#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "badc.h"
#include "prot.h"
#include "g_data.h"
#include "_wpc.h"
#include "config.h"

#define NTC_TEMP_BUFF_SIZE_Max    (                         8)
#define NTC_TEMP_BUFF_SIZE_Msk    (NTC_TEMP_BUFF_SIZE_Max - 1)

#define DIE_TEMP_BUFF_SIZE_Max    (                         8)
#define DIE_TEMP_BUFF_SIZE_Msk    (DIE_TEMP_BUFF_SIZE_Max - 1)

static uint8_t DeltaTemp_N_1;


#if IC_PN_17111
//MM customer: Nu17111 IC NTC resistance is 82/NTC
static const uint16_t ntc_tbl[] =
{
	 3146, 3137, 3127, 3117, 3106, 3095, 3083, 3071, 3058, 3044,  //-29 ~ -20
	 3030, 3015, 3000, 2984, 2968, 2950, 2933, 2914, 2895, 2875,  //-19 ~ -10
	 2854, 2833, 2811, 2789, 2765, 2741, 2717, 2691, 2665, 2638,  // -9 ~   0
	 2611, 2583, 2554, 2525, 2495, 2465, 2434, 2402, 2370, 2338,  //  1 ~  10
	 2305, 2271, 2238, 2203, 2169, 2134, 2099, 2064, 2028, 1993,  // 11 ~  20
	 1957, 1921, 1885, 1849, 1813, 1777, 1742, 1706, 1670, 1635,  // 21 ~  30
	 1600, 1565, 1530, 1496, 1462, 1429, 1395, 1362, 1330, 1298,  // 31 ~  40
	 1266, 1235, 1205, 1175, 1145, 1116, 1087, 1059, 1032, 1005,  // 41 ~  50
	 978, 952, 927, 902, 878, 854, 831, 808, 786, 764,  // 51 ~  60
	 743, 723, 703, 683, 664, 646, 628, 610, 593, 576,  // 61 ~  70
	 560, 544, 529, 514, 499, 485, 472, 458, 445, 433,  // 71 ~  80
	 421, 409, 397, 386, 375, 365, 355, 345, 335, 326,  // 81 ~  90
	 317, 308, 299, 291, 283, 275, 268, 260, 253, 246,  // 91 ~ 100
	 239, 233, 227, 220, 214, 209, 203, 198, 192, 187,  //101 ~ 110
	 182, 177, 173, 168, 164, 159, 155, 151, 147, 143,  //111 ~ 120
};
#else
//Nu17112 IC NTC resistance is 10/NTC
static const uint16_t ntc_tbl[] =
{
3937, //-40
3927, 3917, 3906, 3895, 3883, 3870, 3857, 3843, 3829, 3814, //-39 ~ -30
3799, 3783, 3766, 3748, 3730, 3711, 3691, 3671, 3650, 3628, //-29 ~ -20
3605, 3582, 3558, 3533, 3508, 3481, 3454, 3426, 3398, 3369, //-19 ~ -10
3338, 3308, 3276, 3244, 3211, 3178, 3144, 3109, 3074, 3038, //-9 ~ 0
3001, 2965, 2927, 2889, 2851, 2812, 2773, 2734, 2694, 2654, //1 ~ 10
2614, 2574, 2533, 2493, 2452, 2411, 2370, 2330, 2289, 2248, //11 ~ 20
2208, 2167, 2127, 2087, 2048, 2008, 1969, 1930, 1892, 1853, //21 ~ 30
1816, 1778, 1741, 1705, 1669, 1633, 1598, 1564, 1529, 1496, //31 ~ 40
1463, 1430, 1398, 1367, 1336, 1306, 1276, 1247, 1218, 1190, //41 ~ 50
1163, 1136, 1109, 1083, 1058, 1033, 1009, 985, 962, 940, //51 ~ 60
917, 896, 875, 854, 834, 814, 795, 776, 757, 740, //61 ~ 70
722, 705, 688, 672, 656, 641, 626, 611, 597, 583, //71 ~ 80
569, 556, 543, 530, 518, 506, 495, 483, 472, 461, //81 ~ 90
451, 440, 430, 421, 411, 402, 393, 384, 376, 367, //91 ~ 100
359, 351, 343, 336, 328, 321, 314, 307, 301, 294, //101 ~ 110
288, 282, 276, 270, 264, 259, 253, 248, 243, 238, //111 ~ 120
233, 228, 223, 219, 214, 210, 205, 200, 195, 190, //121 ~ 130
186, 182, 178, 175, 172, 168, 165, 162, 159, 156, //131 ~ 140
153, 150, 147, 144, 142, 139, 136, 134, 131 //141 ~ 150
};
#endif

//type c
int16_t fml_ntc_temp_get_typec(void)
{
	static uint8_t  vntc_idx = 0;

#if IC_PN_17111
		static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = { 1813, 1813, 1813, 1813, 1813, 1813, 1813, 1813 };
#else
		static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = { 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650 };
#endif

	uint16_t i, v_ntc = 0;

	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PB5_ADC6);
	}
	else
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PB5_ADC6);
	}


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
	return ((int)i - 40-11);
}

//wpc
int16_t fml_ntc_temp_get_wpc(void)
{
	static uint8_t  vntc_idx = 0;

#if IC_PN_17111
		static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = { 1813, 1813, 1813, 1813, 1813, 1813, 1813, 1813 };
#else
		static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = { 1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650 };
#endif

	uint16_t i, v_ntc = 0;

	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PB5_ADC6);
	}
	else
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PD3_ADC9);
	}


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
	return ((int)i - 50+18);
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
uint8_t wirless_ntc_power_reduce = 0;

/*+++++++++++++++++++++++++++++++++++++++++++ TNTC_OTP +++++++++++++++++++++++++++++++++++++++++++*/
void fml_tntc_otp_limit_power(int16_t tntc)
{
	uint8_t DeltaTemp = 0;
	//flg_action = 0;//default value, no action.
	//flg_action = 1;//cep=-5, reduce power
	//flg_action = 2;//go to send ATN and update the nego cap
//	uint8_t flg_action = 0;
	static uint8_t cnt = 0;
	if(!gd->wirless_ntc_lock)
	{
		if (tntc >= 75|| tntc <= 2)
		{
			gd->wirless_ntc_lock = 1;
			gd->wpc_disable = 1;
			tcpm_stop_wpc(10);
			return;
		}
	}
	else
	{
		if(tntc<66&&tntc>5)
		{
			gd->wirless_ntc_lock = 0;
			gd->wpc_disable = 0;
		}
	}
	if(!gd->wirless_ntc_lock)
	{
		if(!wirless_ntc_power_reduce)
		{
			if(tntc>=64)
			{
				if(cnt++>10)
				wirless_ntc_power_reduce = 1;
				// tcpm_stop_wpc(10);
			}
			else
			{
				cnt = 0;
			}
		}
		else
		{
			if(tntc<=40)
			{
				if(cnt++>10)
				{
					wirless_ntc_power_reduce = 0;
					// tcpm_stop_wpc(10);
				}
			}
			else
			{
				cnt = 0;
			}
		}
	}
}

/*
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
*/
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
