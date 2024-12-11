#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "badc.h"
#include "prot.h"
#include "g_data.h"
#include "_wpc.h"

#define NTC_TEMP_BUFF_SIZE_Max    (                         8)
#define NTC_TEMP_BUFF_SIZE_Msk    (NTC_TEMP_BUFF_SIZE_Max - 1)

#define DIE_TEMP_BUFF_SIZE_Max    (                         8)
#define DIE_TEMP_BUFF_SIZE_Msk    (DIE_TEMP_BUFF_SIZE_Max - 1)

static const uint16_t ntc_tbl[] =
{
	 3022, 3008, 2994, 2980, 2965, 2949, 2933, 2917, 2899, 2882, //-29 ~ -20
	 2864, 2845, 2826, 2806, 2786, 2765, 2743, 2722, 2699, 2676, //-19 ~ -10
	 2653, 2628, 2604, 2579, 2553, 2527, 2501, 2474, 2446, 2419, // -9 ~   0
	 2390, 2362, 2333, 2304, 2274, 2244, 2214, 2183, 2153, 2122, //  1 ~  10
	 2091, 2059, 2028, 1997, 1965, 1933, 1902, 1870, 1838, 1807, // 11 ~  20
	 1775, 1744, 1712, 1681, 1650, 1619, 1588, 1558, 1528, 1498, // 21 ~  30
	 1468, 1438, 1409, 1380, 1352, 1324, 1296, 1268, 1241, 1215, // 31 ~  40
	 1188, 1162, 1137, 1112, 1087, 1063, 1039, 1015,  992,  969, // 41 ~  50
	  947,  926,  904,  883,  863,  843,  823,  804,  785,  766, // 51 ~  60
	  748,  730,  713,  696,  679,  663,  647,  632,  617,  602, // 61 ~  70
	  588,  574,  560,  547,  534,  521,  508,  496,  484,  473, // 71 ~  80
	  462,  451,  440,  429,  419,  409,  399,  390,  381,  372, // 81 ~  90
	  363,  355,  346,  338,  330,  322,  315,  308,  301,  293, // 91 ~ 100
	  287,  280,  274,  267,  261,  255,  250,  244,  238,  233, //101 ~ 110
	  228,  223,  218,  213,  208,  203,  199,  194,  190,  186, //111 ~ 120
};

int16_t fml_ntc_temp_get(void)
{
	static uint8_t  vntc_idx = 0;
	static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = { 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650 };

	uint16_t i, v_ntc = 0;

	vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PB2_ADC2);
	vntc_idx &= NTC_TEMP_BUFF_SIZE_Msk;

	for (i=0; i<NTC_TEMP_BUFF_SIZE_Max; ++i)
	{
		v_ntc += vntc_buf[i];
	}

	v_ntc /= NTC_TEMP_BUFF_SIZE_Max;

	i = 0;
	while (i < sizeof(ntc_tbl)/sizeof(ntc_tbl[0]))
	{
		if (v_ntc >= ntc_tbl[i])
		{
			break;
		}
		++i;
	}

	return ((int)i - 29);
}

int16_t fml_die_temp_get(void)
{
	static uint8_t tdie_idx = 0;
	static int16_t tdie_buf[DIE_TEMP_BUFF_SIZE_Max] = { 25, 25, 25, 25,  25, 25, 25, 25 };

	int16_t i, t_die = 0;

	tdie_buf[tdie_idx++] = (int16_t)hal_badc_meas(_BADC_CH_INR_TJ_L);
	tdie_idx &= DIE_TEMP_BUFF_SIZE_Msk;

	for (i=0; i<DIE_TEMP_BUFF_SIZE_Max; ++i)
	{
		t_die += tdie_buf[i];
	}

	t_die /= DIE_TEMP_BUFF_SIZE_Max;

	return t_die;
}

/*+++++++++++++++++++++++++++++++++++++++++++ TNTC_OTP +++++++++++++++++++++++++++++++++++++++++++*/
static struct tntc_otp_t
{
	uint8_t  writ_idx;
	uint8_t  over_cnt;
	uint8_t  reco_cnt;
	uint16_t temp_buf[NTC_TEMP_BUFF_SIZE_Max];
} tntc_otp_ctrl;

void fml_tntc_otp_init(void)
{
	tntc_otp_ctrl.over_cnt = 0;
	tntc_otp_ctrl.reco_cnt = 0;

	gd->prot_sts.tntc_otp_flag = 0;
}

void fml_tntc_otp_check(int16_t tntc)
{
	if (!ap->tntc_otp_dis)
	{
		if (gd->prot_sts.tntc_otp_flag)
		{
			if (tntc + ap->tntc_otp_hys < ap->tntc_otp_thd)
			{
				if (++tntc_otp_ctrl.reco_cnt > 10)
				{
					tntc_otp_ctrl.over_cnt = 0;
					gd->prot_sts.tntc_otp_flag = 0;
				}
			}
			else
			{
				tntc_otp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (tntc > ap->tntc_otp_thd)
			{
				if (++tntc_otp_ctrl.over_cnt > 10)
				{
					tntc_otp_ctrl.reco_cnt = 0;
					gd->prot_sts.tntc_otp_flag = 1;
				}
			}
			else
			{
				tntc_otp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.tntc_otp_flag = 0;
	}

	if (gd->prot_sts.tntc_otp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_NTC_OTP);
	}
}
/*------------------------------------------- TNTC_OTP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ TNTC_UTP +++++++++++++++++++++++++++++++++++++++++++*/
static struct tntc_utp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} tntc_utp_ctrl;

void fml_tntc_utp_init(void)
{
	tntc_utp_ctrl.over_cnt = 0;
	tntc_utp_ctrl.reco_cnt = 0;

	gd->prot_sts.tntc_utp_flag = 0;
}

void fml_tntc_utp_check(int16_t tntc)
{
	if (!ap->tntc_utp_dis)
	{
		if (gd->prot_sts.tntc_utp_flag)
		{
			if (tntc > (int16_t)ap->tntc_utp_thd + (int16_t)ap->tntc_utp_hys)
			{
				if (++tntc_utp_ctrl.reco_cnt > 10)
				{
					tntc_utp_ctrl.over_cnt = 0;
					gd->prot_sts.tntc_utp_flag = 0;
				}
			}
			else
			{
				tntc_utp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (tntc < (int16_t)ap->tntc_utp_thd)
			{
				if (++tntc_utp_ctrl.over_cnt > 10)
				{
					tntc_utp_ctrl.reco_cnt = 0;
					gd->prot_sts.tntc_utp_flag = 1;
				}
			}
			else
			{
				tntc_utp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.tntc_utp_flag = 0;
	}

	if (gd->prot_sts.tntc_utp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_NTC_UTP);
	}
}
/*------------------------------------------- TNTC_UTP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ TDIE_OTP +++++++++++++++++++++++++++++++++++++++++++*/
static struct tdie_otp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} tdie_otp_ctrl;

void fml_tdie_otp_init(void)
{
	tdie_otp_ctrl.over_cnt = 0;
	tdie_otp_ctrl.reco_cnt = 0;

	gd->prot_sts.tdie_otp_flag = 0;
}

void fml_tdie_otp_check(int16_t tdie)
{
	if (!ap->tdie_otp_dis)
	{
		if (gd->prot_sts.tdie_otp_flag)
		{
			if (tdie + ap->tdie_otp_hys < ap->tdie_otp_thd)
			{
				if (++tdie_otp_ctrl.reco_cnt > 10)
				{
					tdie_otp_ctrl.over_cnt = 0;
					gd->prot_sts.tdie_otp_flag = 0;
				}
			}
			else
			{
				tdie_otp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (tdie > ap->tdie_otp_thd)
			{
				if (++tdie_otp_ctrl.over_cnt > 10)
				{
					tdie_otp_ctrl.reco_cnt = 0;
					gd->prot_sts.tdie_otp_flag = 1;
				}
			}
			else
			{
				tdie_otp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.tdie_otp_flag = 0;
	}

	if (gd->prot_sts.tdie_otp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_DIE_OTP);
	}
}
/*------------------------------------------- TDIE_OTP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ TDIE_UTP +++++++++++++++++++++++++++++++++++++++++++*/
static struct tdie_utp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} tdie_utp_ctrl;

void fml_tdie_utp_init(void)
{
	tdie_utp_ctrl.over_cnt = 0;
	tdie_utp_ctrl.reco_cnt = 0;

	gd->prot_sts.tdie_utp_flag = 0;
}

void fml_tdie_utp_check(int16_t tdie)
{
	if (!ap->tdie_utp_dis)
	{
		if (gd->prot_sts.tdie_utp_flag)
		{
			if (tdie > (int16_t)ap->tdie_utp_thd + (int16_t)ap->tdie_utp_hys)
			{
				if (++tdie_utp_ctrl.reco_cnt > 10)
				{
					tdie_utp_ctrl.over_cnt = 0;
					gd->prot_sts.tdie_utp_flag = 0;
				}
			}
			else
			{
				tdie_utp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (tdie < (int16_t)ap->tdie_utp_thd)
			{
				if (++tdie_utp_ctrl.over_cnt > 10)
				{
					tdie_utp_ctrl.reco_cnt = 0;
					gd->prot_sts.tdie_utp_flag = 1;
				}
			}
			else
			{
				tdie_utp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.tdie_utp_flag = 0;
	}

	if (gd->prot_sts.tdie_utp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_DIE_UTP);
	}
}
/*------------------------------------------- TDIE_UTP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ ISNS_OCP +++++++++++++++++++++++++++++++++++++++++++*/
static struct isns_ocp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} isns_ocp_ctrl;

void fml_isns_ocp_init(void)
{
	isns_ocp_ctrl.over_cnt = 0;
	isns_ocp_ctrl.reco_cnt = 0;

	gd->prot_sts.isns_ocp_flag = 0;
}

void fml_isns_ocp_check(uint16_t isns)
{
	if (!ap->isns_ocp_dis)
	{
		if (gd->prot_sts.isns_ocp_flag)
		{
			if (isns + ap->isns_ocp_hys < ap->isns_ocp_thd)
			{
				if (++isns_ocp_ctrl.reco_cnt)
				{
					isns_ocp_ctrl.over_cnt = 0;
					gd->prot_sts.isns_ocp_flag = 0;
				}
			}
			else
			{
				isns_ocp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (isns > ap->isns_ocp_thd)
			{
				if (++isns_ocp_ctrl.over_cnt > 5)
				{
					isns_ocp_ctrl.reco_cnt = 0;
					gd->prot_sts.isns_ocp_flag = 1;
				}
			}
			else
			{
				isns_ocp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.isns_ocp_flag = 0;
	}

	if (gd->prot_sts.isns_ocp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OCP);
	}
}
/*------------------------------------------- ISNS_OCP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ VBUS_OVP +++++++++++++++++++++++++++++++++++++++++++*/
static struct vbus_ovp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} vbus_ovp_ctrl;

void fml_vbus_ovp_init(void)
{
	vbus_ovp_ctrl.over_cnt = 0;
	vbus_ovp_ctrl.reco_cnt = 0;

	gd->prot_sts.vbus_ovp_flag = 0;
}

void fml_vbus_ovp_check(uint16_t vbus)
{
	if (!ap->vbus_ovp_dis)
	{
		if (gd->prot_sts.vbus_ovp_flag)
		{
			if (vbus + ap->vbus_ovp_hys < ap->vbus_ovp_thd)
			{
				if (++vbus_ovp_ctrl.reco_cnt > 10)
				{
					vbus_ovp_ctrl.over_cnt = 0;
					gd->prot_sts.vbus_ovp_flag = 0;
				}
			}
			else
			{
				vbus_ovp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (vbus > ap->vbus_ovp_thd)
			{
				if (++vbus_ovp_ctrl.over_cnt > 10)
				{
					vbus_ovp_ctrl.reco_cnt = 0;
					gd->prot_sts.vbus_ovp_flag = 1;
				}
			}
			else
			{
				vbus_ovp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.vbus_ovp_flag = 0;
	}

	if (gd->prot_sts.vbus_ovp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OVP);
	}
}
/*------------------------------------------- VBUS_OVP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ VBUS_UVP +++++++++++++++++++++++++++++++++++++++++++*/
static struct vbus_uvp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} vbus_uvp_ctrl;

void fml_vbus_uvp_init(void)
{
	vbus_uvp_ctrl.over_cnt = 0;
	vbus_uvp_ctrl.reco_cnt = 0;

	gd->prot_sts.vbus_uvp_flag = 0;
}

void fml_vbus_uvp_check(uint16_t vbus)
{
	if (!ap->vbus_uvp_dis)
	{
		if (gd->prot_sts.vbus_uvp_flag)
		{
			if (vbus > ap->vbus_uvp_thd + ap->vbus_uvp_hys)
			{
				if (++vbus_uvp_ctrl.reco_cnt > 10)
				{
					vbus_uvp_ctrl.over_cnt = 0;
					gd->prot_sts.vbus_uvp_flag = 0;
				}
			}
			else
			{
				vbus_uvp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (vbus < ap->vbus_uvp_thd)
			{
				if (++vbus_uvp_ctrl.over_cnt > 10)
				{
					vbus_uvp_ctrl.reco_cnt = 0;
					gd->prot_sts.vbus_uvp_flag = 1;
				}
			}
			else
			{
				vbus_uvp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.vbus_uvp_flag = 0;
	}

	if (gd->prot_sts.vbus_uvp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_FMW_UVP);
	}
}
/*------------------------------------------- VBUS_UVP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ VBUS_DPL +++++++++++++++++++++++++++++++++++++++++++*/
static struct vbus_dpl_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} vbus_dpl_ctrl;

void fml_vbus_dpl_init(void)
{
	vbus_dpl_ctrl.over_cnt = 0;
	vbus_dpl_ctrl.reco_cnt = 0;

	gd->tx_infos.cep_event.dpl = 0;
}

void fml_vbus_dpl_check(uint16_t vbus)
{
	if (!ap->vbus_dpl_dis)
	{
		if (gd->tx_infos.cep_event.dpl)
		{
			if (vbus > ap->vbus_dpl_thd + ap->vbus_dpl_hys)
			{
				if (++vbus_dpl_ctrl.reco_cnt > 5)
				{
					vbus_dpl_ctrl.over_cnt = 0;
					gd->tx_infos.cep_event.dpl = 0;
				}
			}
			else
			{
				vbus_dpl_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (vbus < ap->vbus_dpl_thd)
			{
				if (++vbus_dpl_ctrl.over_cnt > 3)
				{
					vbus_dpl_ctrl.reco_cnt = 0;
					gd->tx_infos.cep_event.dpl = 1;
				}
			}
			else
			{
				vbus_dpl_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->tx_infos.cep_event.dpl = 0;
	}
}
/*------------------------------------------- VBUS_DPL -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ VPWR_OVP +++++++++++++++++++++++++++++++++++++++++++*/
static struct vpwr_ovp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} vpwr_ovp_ctrl;

void fml_vpwr_ovp_init(void)
{
	vpwr_ovp_ctrl.over_cnt = 0;
	vpwr_ovp_ctrl.reco_cnt = 0;

	gd->prot_sts.vpwr_ovp_flag = 0;
}

void fml_vpwr_ovp_check(uint16_t vpwr)
{
	if (!ap->vpwr_ovp_dis)
	{
		if (gd->prot_sts.vpwr_ovp_flag)
		{
			if (vpwr + ap->vpwr_ovp_hys < ap->vpwr_ovp_thd)
			{
				if (++vpwr_ovp_ctrl.reco_cnt > 10)
				{
					vpwr_ovp_ctrl.over_cnt = 0;
					gd->prot_sts.vpwr_ovp_flag = 0;
				}
			}
			else
			{
				vpwr_ovp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (vpwr > ap->vpwr_ovp_thd)
			{
				if (++vpwr_ovp_ctrl.over_cnt > 10)
				{
					vpwr_ovp_ctrl.reco_cnt = 0;
					gd->prot_sts.vpwr_ovp_flag = 1;
				}
			}
			else
			{
				vpwr_ovp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.vpwr_ovp_flag = 0;
	}

	if (gd->prot_sts.vpwr_ovp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OVP);
	}
}
/*------------------------------------------- VPWR_OVP -------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ POUT_OPP +++++++++++++++++++++++++++++++++++++++++++*/
static struct pout_opp_t
{
	uint8_t over_cnt;
	uint8_t reco_cnt;
} pout_opp_ctrl;

void fml_pout_opp_init(void)
{
	pout_opp_ctrl.over_cnt = 0;
	pout_opp_ctrl.reco_cnt = 0;

	gd->prot_sts.pout_opp_flag = 0;
}

void fml_pout_opp_check(uint16_t vpwr, uint16_t isns)
{
	if (!ap->pout_opp_dis)
	{
		if (gd->prot_sts.pout_opp_flag)
		{
			if (vpwr * isns / 1000 + ap->pout_opp_hys < ap->pout_opp_thd)
			{
				if (++pout_opp_ctrl.reco_cnt > 10)
				{
					pout_opp_ctrl.over_cnt = 0;
					gd->prot_sts.pout_opp_flag = 0;
				}
			}
			else
			{
				pout_opp_ctrl.reco_cnt = 0;
			}
		}
		else
		{
			if (vpwr * isns / 1000 > ap->pout_opp_thd)
			{
				if (++pout_opp_ctrl.over_cnt > 10)
				{
					pout_opp_ctrl.reco_cnt = 0;
					gd->prot_sts.pout_opp_flag = 1;
				}
			}
			else
			{
				pout_opp_ctrl.over_cnt = 0;
			}
		}
	}
	else
	{
		gd->prot_sts.pout_opp_flag = 0;
	}

	if (gd->prot_sts.pout_opp_flag)
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OPP);
	}
}
/*------------------------------------------- POUT_OPP -------------------------------------------*/
