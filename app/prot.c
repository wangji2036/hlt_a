#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "badc.h"
#include "prot.h"
#include "g_data.h"
#include "_wpc.h"
#include "config.h"
#include "tcpm.h"
#include "buckboost.h"
#include "usb_bridge.h"
#include "bat_record.h"

extern volatile uint32_t tc_sys_ticks;

#define NTC_TEMP_BUFF_SIZE_Max (8)
#define NTC_TEMP_BUFF_SIZE_Msk (NTC_TEMP_BUFF_SIZE_Max - 1)

#define DIE_TEMP_BUFF_SIZE_Max (8)
#define DIE_TEMP_BUFF_SIZE_Msk (DIE_TEMP_BUFF_SIZE_Max - 1)

static uint8_t DeltaTemp_N_1 __attribute__((unused));

#if IC_PN_17111
//MM customer: Nu17111 IC NTC resistance is 82/NTC
static const uint16_t ntc_tbl[] =
    {
        3146,
        3137,
        3127,
        3117,
        3106,
        3095,
        3083,
        3071,
        3058,
        3044, //-29 ~ -20
        3030,
        3015,
        3000,
        2984,
        2968,
        2950,
        2933,
        2914,
        2895,
        2875, //-19 ~ -10
        2854,
        2833,
        2811,
        2789,
        2765,
        2741,
        2717,
        2691,
        2665,
        2638, // -9 ~   0
        2611,
        2583,
        2554,
        2525,
        2495,
        2465,
        2434,
        2402,
        2370,
        2338, //  1 ~  10
        2305,
        2271,
        2238,
        2203,
        2169,
        2134,
        2099,
        2064,
        2028,
        1993, // 11 ~  20
        1957,
        1921,
        1885,
        1849,
        1813,
        1777,
        1742,
        1706,
        1670,
        1635, // 21 ~  30
        1600,
        1565,
        1530,
        1496,
        1462,
        1429,
        1395,
        1362,
        1330,
        1298, // 31 ~  40
        1266,
        1235,
        1205,
        1175,
        1145,
        1116,
        1087,
        1059,
        1032,
        1005, // 41 ~  50
        978,
        952,
        927,
        902,
        878,
        854,
        831,
        808,
        786,
        764, // 51 ~  60
        743,
        723,
        703,
        683,
        664,
        646,
        628,
        610,
        593,
        576, // 61 ~  70
        560,
        544,
        529,
        514,
        499,
        485,
        472,
        458,
        445,
        433, // 71 ~  80
        421,
        409,
        397,
        386,
        375,
        365,
        355,
        345,
        335,
        326, // 81 ~  90
        317,
        308,
        299,
        291,
        283,
        275,
        268,
        260,
        253,
        246, // 91 ~ 100
        239,
        233,
        227,
        220,
        214,
        209,
        203,
        198,
        192,
        187, //101 ~ 110
        182,
        177,
        173,
        168,
        164,
        159,
        155,
        151,
        147,
        143, //111 ~ 120
};
#else
/* Typec NTC: 100K@25°C (B=3950) 接 GND, 100K 上拉 VDD=3.3V (V_pull 与 Vref 同源).
 * 表存名义 3300mV 下的 V_ntc(mV); 查表前用 flash 校准 Vref 把 hal_badc_meas 返回的实测 mV
 * 归一回 3300mV 名义刻度。
 * 公式: V_ntc(mV) = round(3300 × R/(R+100K)), R(T)=100K × exp(3950 × (1/(T+273.15) - 1/298.15)). */
static const uint16_t ntc_tbl[] =
    {
        3220, //-40
        3214,
        3208,
        3201,
        3194,
        3187,
        3179,
        3171,
        3162,
        3153,
        3143, //-39 ~ -30
        3133,
        3122,
        3111,
        3099,
        3086,
        3073,
        3059,
        3045,
        3030,
        3014, //-29 ~ -20
        2998,
        2980,
        2963,
        2944,
        2925,
        2904,
        2884,
        2862,
        2840,
        2816, //-19 ~ -10
        2793,
        2768,
        2742,
        2716,
        2689,
        2661,
        2633,
        2604,
        2574,
        2543, //-9 ~ 0
        2512,
        2480,
        2448,
        2415,
        2381,
        2347,
        2313,
        2278,
        2242,
        2206, //1 ~ 10
        2170,
        2134,
        2097,
        2060,
        2023,
        1985,
        1948,
        1911,
        1873,
        1836, //11 ~ 20
        1798,
        1761,
        1724,
        1687,
        1650,
        1613,
        1577,
        1541,
        1506,
        1470, //21 ~ 30
        1436,
        1401,
        1367,
        1334,
        1301,
        1268,
        1236,
        1205,
        1174,
        1143, //31 ~ 40
        1114,
        1084,
        1056,
        1028,
        1000,
        973,
        947,
        921,
        896,
        871, //41 ~ 50
        847,
        824,
        801,
        779,
        757,
        736,
        716,
        696,
        676,
        657, //51 ~ 60
        639,
        621,
        603,
        586,
        570,
        554,
        538,
        523,
        508,
        494, //61 ~ 70
        480,
        466,
        453,
        441,
        428,
        416,
        405,
        394,
        383,
        372, //71 ~ 80
        362,
        352,
        342,
        333,
        323,
        315,
        306,
        298,
        290,
        282, //81 ~ 90
        274,
        267,
        260,
        253,
        246,
        239,
        233,
        227,
        221,
        215, //91 ~ 100
        210,
        204,
        199,
        194,
        189,
        184,
        179,
        175,
        170,
        166, //101 ~ 110
        162,
        158,
        154,
        150,
        146,
        143,
        139,
        136,
        132,
        129, //111 ~ 120
        126,
        123,
        120,
        117,
        114,
        112,
        109,
        106,
        104,
        101, //121 ~ 130
        99,
        97,
        95,
        92,
        90,
        88,
        86,
        84,
        82,
        81, //131 ~ 140
        79,
        77,
        75,
        74,
        72,
        70,
        69,
        67,
        66,
        65, //141 ~ 150
};

static const uint16_t ntc_100r_5v_tbl_rev[] =
    {
        6055,                                                       //-40
        6044, 6032, 6019, 6006, 5993, 5978, 5962, 5946, 5929, 5910, //-39 ~ -30
        5891, 5871, 5849, 5827, 5804, 5779, 5753, 5726, 5698, 5668, //-29 ~ -20
        5637, 5605, 5571, 5536, 5500, 5462, 5423, 5382, 5340, 5297, //-19 ~ -10
        5252, 5205, 5158, 5108, 5058, 5005, 4952, 4897, 4841, 4784, //-9 ~ 0
        4725, 4665, 4604, 4542, 4479, 4415, 4349, 4284, 4217, 4149, //1 ~ 10
        4081, 4013, 3943, 3874, 3804, 3734, 3663, 3593, 3522, 3452, //11 ~ 20
        3381, 3311, 3241, 3171, 3102, 3034, 2965, 2898, 2831, 2764, //21 ~ 30
        2699, 2634, 2570, 2507, 2445, 2384, 2323, 2264, 2206, 2149, //31 ~ 40
        2093, 2038, 1984, 1931, 1879, 1829, 1779, 1731, 1683, 1637, //41 ~ 50
        1592, 1548, 1505, 1463, 1423, 1383, 1344, 1306, 1270, 1234, //51 ~ 60
        1200, 1166, 1133, 1101, 1070, 1039, 1010, 982, 954, 927,    //61 ~ 70
        901, 876, 851, 827, 804, 782, 759, 739, 718, 698,           //71 ~ 80
        679, 660, 642, 624, 607, 590, 574, 558, 543, 529,           //81 ~ 90
        514, 501, 487, 474, 461, 449, 437, 426, 415, 404,           //91 ~ 100
        393, 383, 373, 363, 353, 345, 336, 327, 319, 311,           //101 ~ 110
        303, 295, 288, 281, 274, 267, 261, 254, 248, 242,           //111 ~ 120
        236, 230, 225, 219, 214, 209, 204, 199, 195, 190,           //121 ~ 130
        186, 182, 177, 173, 169, 165, 162, 158, 154, 151,           //131 ~ 140
        148, 144, 141, 138, 135, 132, 129, 126, 124, 121            //141 ~ 150
};
#endif

//type c
int16_t fml_ntc_temp_get_typec(void)
{
	static uint8_t vntc_idx = 0;

#if IC_PN_17111
	static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = {1813, 1813, 1813, 1813, 1813, 1813, 1813, 1813};
#else
	static uint16_t vntc_buf[NTC_TEMP_BUFF_SIZE_Max] = {1650, 1650, 1650, 1650, 1650, 1650, 1650, 1650};
#endif

	uint16_t i, v_ntc = 0;

	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PB5_ADC6);
	}
	else
	{
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PC8_ADC5);
		// printk("_BADC_CH_PC8_ADC5=%d",hal_badc_meas(_BADC_CH_PC8_ADC5));
	}

	vntc_idx &= NTC_TEMP_BUFF_SIZE_Msk;

	for (i = 0; i < NTC_TEMP_BUFF_SIZE_Max; ++i)
	{
		v_ntc += vntc_buf[i];
	}

	v_ntc /= NTC_TEMP_BUFF_SIZE_Max;
	//printk("\r\n typec_adc = %d",v_ntc);
	i = 0;
	while (i < sizeof(ntc_tbl) / sizeof(ntc_tbl[0]))
	{
		if (v_ntc >= ntc_tbl[i])
		{
			break;
		}
		++i;
	}
	return ((int)i - 40 - 2);
}

/* WPC 线圈 NTC: 100K@25°C (B=3950) 接 GND, 100K 上拉 VDDH=4.8V, WB7720 ADC 10-bit Vref=4.5V.
 * 按 1°C 步进, tbl[0]=-20°C, raw 单调递减. 查不到时高端钳位 91°C, 低端钳位 -20°C.
 * 公式: raw = round( 4.8 × R/(R+100K) × 1023/4.5 ).
 * TODO: 生产前两点实测校准(25/60°C). */
static const uint16_t ntc_wpc_raw_tbl[] =
    {
        997,                                              //-20
        991, 986, 980, 974, 967, 960, 953, 946, 939, 931, //-19 ~ -10
        924, 915, 907, 898, 889, 880, 871, 861, 851, 841, // -9 ~   0
        831, 820, 809, 799, 787, 776, 765, 753, 741, 729, //  1 ~  10
        717, 705, 694, 681, 669, 657, 644, 632, 619, 607, // 11 ~  20
        594, 582, 570, 558, 546, 534, 522, 510, 498, 486, // 21 ~  30
        475, 463, 452, 441, 430, 419, 409, 398, 388, 378, // 31 ~  40
        368, 359, 349, 340, 331, 322, 313, 305, 296, 288, // 41 ~  50
        280, 272, 265, 258, 250, 244, 237, 230, 224, 217, // 51 ~  60
        211, 205, 200, 194, 188, 183, 178, 173, 168, 163, // 61 ~  70
        159, 154, 150, 146, 142, 138, 134, 130, 127, 123, // 71 ~  80
        120, 116, 113, 110, 107, 104, 101, 98, 96, 93     // 81 ~  90
};

int16_t fml_ntc_temp_get_wpc(void)
{
	static int16_t last_temp = 25;
	static uint32_t outlier_start_tk = 0;
	static bool outlier_pending = false;

	uint16_t raw;
	if (usb_bridge_read_ntc_raw(NTC_CH_COIL, &raw, NULL, NULL) != 0)
		return last_temp;
	uint16_t i = 0;
	while (i < sizeof(ntc_wpc_raw_tbl) / sizeof(ntc_wpc_raw_tbl[0]))
	{
		if (raw >= ntc_wpc_raw_tbl[i])
			break;
		++i;
	}
	int16_t temp = (int16_t)i - 20 - 3;

	/* 偏差 > 10°C 延迟 3s 再接受 (避免瞬态抖动/采样异常被采信) */
	int16_t delta = temp - last_temp;
	if (delta > 10 || delta < -10)
	{
		if (!outlier_pending)
		{
			outlier_pending = true;
			outlier_start_tk = tc_sys_ticks;
		}
		if ((uint32_t)(tc_sys_ticks - outlier_start_tk) < 3000)
			return last_temp;
	}
	else
	{
		outlier_pending = false;
	}
	last_temp = temp;
	return temp;
}

int16_t fml_die_temp_get(void)
{
	static uint8_t tdie_idx = 0;
	static int16_t tdie_buf[DIE_TEMP_BUFF_SIZE_Max] = {25, 25, 25, 25, 25, 25, 25, 25};

	int16_t i, t_die = 0;

	tdie_buf[tdie_idx++] = (int16_t)hal_badc_meas(_BADC_CH_INR_TJ_L);
	tdie_idx &= DIE_TEMP_BUFF_SIZE_Msk;

	for (i = 0; i < DIE_TEMP_BUFF_SIZE_Max; ++i)
	{
		t_die += tdie_buf[i];
	}

	t_die /= DIE_TEMP_BUFF_SIZE_Max;

	return t_die;
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

/*+++++++++++++++++++++++++++++++++++++++++++ BAT_OV_FORBID +++++++++++++++++++++++++++++++++++++++*/
void fml_bat_ov_forbid_check(void)
{
	static uint8_t ov_forbid_consec_cnt = 0;
	// if (g_forbid_bypass_flag) return;
	xgb_printk("gd->bat_ov_forbid_flag=%d\n", gd->bat_ov_forbid_flag);
	if (gd->bat_ov_forbid_flag)
	{
		if (g_buckboost.woke_mode != BUCKBOOST_SHUTDOWM_MODE)
		{
			buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
		}
		xgb_printk("\r\n[OV_FORBID] Active (flag=1)");
		return;
	}

	uint16_t cell1 = g_buckboost.adc_vcell1;
	uint16_t cell2 = g_buckboost.adc_vcell2;
	uint16_t total = cell1 + cell2;
	uint16_t max_cell = (cell1 > cell2) ? cell1 : cell2;

	xgb_printk("\r\n[OV_CHK] c1=%d c2=%d t=%d max=%d", cell1, cell2, total, max_cell);

	/* 可疑高读数去抖：max_cell > 5200mV 时延时 5s (50 × 100ms) 后再判定，
     * 避免 ADC 瞬态毛刺直接触发 forbid。延时窗口内 return，窗口结束后放行到正常判定。*/
	static uint8_t suspect_delay_left = 0;
	static uint8_t suspect_delay_done = 0;
	if (max_cell > 5200)
	{
		if (!suspect_delay_done)
		{
			if (suspect_delay_left == 0)
			{
				suspect_delay_left = 50;
				xgb_printk("\r\n[OV_CHK] suspect max=%d>5200, delay 5s", max_cell);
				return;
			}
			suspect_delay_left--;
			if (suspect_delay_left > 0)
			{
				return;
			}
			suspect_delay_done = 1;
			xgb_printk("\r\n[OV_CHK] 5s elapsed, max=%d, proceed", max_cell);
		}
		/* suspect_delay_done==1 → 继续走下面的正常判定 */
	}
	else
	{
		suspect_delay_left = 0;
		suspect_delay_done = 0;
	}

	if (max_cell >= OVER_VOLTAGE_FORBID_THRESHOLD)
	{
		ov_forbid_consec_cnt++;
		xgb_printk("\r\n[OV_FORBID] %dmV >= %dmV, cnt=%d",
		           max_cell, OVER_VOLTAGE_FORBID_THRESHOLD, ov_forbid_consec_cnt);
		if (ov_forbid_consec_cnt >= OVER_VOLTAGE_FORBID_CONSEC_COUNT)
		{
			gd->bat_ov_forbid_flag = 1;
#if OV_FORBID_FLASH_PERSIST
			cycle_count_save_to_flash();
			xgb_printk("\r\n[OV_FORBID] Persisted to Flash.");
#else
			xgb_printk("\r\n[OV_FORBID] TRIGGERED! Forbidden until power cycle.");
#endif
			buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
		}
	}
	else
	{
		ov_forbid_consec_cnt = 0;
	}
}
/*------------------------------------------- BAT_OV_FORBID --------------------------------------*/
