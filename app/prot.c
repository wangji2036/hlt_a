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

extern volatile uint32_t tc_sys_ticks;

#define NTC_TEMP_BUFF_SIZE_Max    (                         8)
#define NTC_TEMP_BUFF_SIZE_Msk    (NTC_TEMP_BUFF_SIZE_Max - 1)

#define DIE_TEMP_BUFF_SIZE_Max    (                         8)
#define DIE_TEMP_BUFF_SIZE_Msk    (DIE_TEMP_BUFF_SIZE_Max - 1)

static uint8_t DeltaTemp_N_1 __attribute__((unused));


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
//Nu17112 IC NTC resistance is 100/NTC
static const uint16_t ntc_tbl[] =
{
3996, //-40
3989, 3981, 3973, 3964, 3955, 3945, 3935, 3924, 3913, 3901, //-39 ~ -30
3888, 3875, 3861, 3846, 3830, 3814, 3797, 3779, 3760, 3741, //-29 ~ -20
3721, 3699, 3677, 3654, 3630, 3605, 3579, 3552, 3525, 3496, //-19 ~ -10
3466, 3436, 3404, 3371, 3338, 3304, 3268, 3232, 3195, 3157, //-9 ~ 0
3118, 3079, 3039, 2998, 2956, 2914, 2871, 2827, 2783, 2739, //1 ~ 10
2694, 2648, 2603, 2557, 2511, 2464, 2418, 2371, 2325, 2278, //11 ~ 20
2232, 2185, 2139, 2093, 2048, 2002, 1957, 1912, 1868, 1824, //21 ~ 30
1781, 1738, 1696, 1655, 1614, 1573, 1533, 1494, 1456, 1418, //31 ~ 40
1381, 1345, 1309, 1274, 1240, 1207, 1174, 1142, 1111, 1081, //41 ~ 50
1051, 1022, 993, 966, 939, 913, 887, 862, 838, 815, //51 ~ 60
792, 769, 748, 727, 706, 686, 667, 648, 630, 612, //61 ~ 70
595, 578, 562, 546, 531, 516, 501, 488, 474, 461, //71 ~ 80
448, 436, 424, 412, 401, 390, 379, 369, 359, 349, //81 ~ 90
340, 330, 322, 313, 305, 296, 289, 281, 274, 266, //91 ~ 100
259, 253, 246, 240, 234, 228, 222, 216, 211, 205, //101 ~ 110
200, 195, 190, 185, 181, 176, 172, 168, 164, 160, //111 ~ 120
156, 152, 148, 145, 141, 138, 135, 132, 129, 126, //121 ~ 130
123, 120, 117, 114, 112, 109, 107, 104, 102, 100, //131 ~ 140
97, 95, 93, 91, 89, 87, 85, 83, 82, 80 //141 ~ 150
};

static const uint16_t ntc_100r_5v_tbl_rev[] =
{
    6055, //-40
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
    1200, 1166, 1133, 1101, 1070, 1039, 1010, 982, 954, 927, //61 ~ 70
    901, 876, 851, 827, 804, 782, 759, 739, 718, 698, //71 ~ 80
    679, 660, 642, 624, 607, 590, 574, 558, 543, 529, //81 ~ 90
    514, 501, 487, 474, 461, 449, 437, 426, 415, 404, //91 ~ 100
    393, 383, 373, 363, 353, 345, 336, 327, 319, 311, //101 ~ 110
    303, 295, 288, 281, 274, 267, 261, 254, 248, 242, //111 ~ 120
    236, 230, 225, 219, 214, 209, 204, 199, 195, 190, //121 ~ 130
    186, 182, 177, 173, 169, 165, 162, 158, 154, 151, //131 ~ 140
    148, 144, 141, 138, 135, 132, 129, 126, 124, 121 //141 ~ 150
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
		vntc_buf[vntc_idx++] = hal_badc_meas(_BADC_CH_PC8_ADC5);
	}


	vntc_idx &= NTC_TEMP_BUFF_SIZE_Msk;

	for (i=0; i<NTC_TEMP_BUFF_SIZE_Max; ++i)
	{
		v_ntc += vntc_buf[i];
	}

	v_ntc /= NTC_TEMP_BUFF_SIZE_Max;
	//printk("\r\n typec_adc = %d",v_ntc);
	i = 0;
	while (i < sizeof(ntc_tbl)/sizeof(ntc_tbl[0]))
	{
		if (v_ntc >= ntc_tbl[i])
		{
			break;
		}
		++i;
	}
	return ((int)i - 40);
}

/* ===== WB7720 NTC 硬件参数 =====
 * WB7720 ADC: 10-bit (0..1023), Vref=4.5V; NTC分压: VDDH(4.84V) → 100K上拉 → ADC → 100K@25°C NTC → GND
 * 查表基准为旧 12-bit/Vref=VDDH, 需两步归一化: ×4095/1023 再 ×4500/4840 */
#define WB7720_ADC_VREF_MV      4500U
#define WB7720_NTC_VDDH_MV      4840U
#define WB7720_ADC_FULL_SCALE   1023U
#define LEGACY_ADC_FULL_SCALE   4095U

/* WPC 线圈 NTC 锚点反查表 (TODO: 生产前实测 25+60°C 两点校准) */
static const int16_t  ntc_wb7720_anchors_temp[] = { -20,  0,    25,   50,   75,  100,  125  };
static const uint16_t ntc_wb7720_anchors_raw[]  = { 3580, 3135, 2048, 1085, 528,  260,  135  };
#define NTC_WB7720_ANCHORS_N (sizeof(ntc_wb7720_anchors_temp)/sizeof(ntc_wb7720_anchors_temp[0]))

static int16_t ntc_wb7720_raw_to_temp_c(uint16_t raw)
{
	if (raw >= ntc_wb7720_anchors_raw[0]) return ntc_wb7720_anchors_temp[0];
	if (raw <= ntc_wb7720_anchors_raw[NTC_WB7720_ANCHORS_N-1]) return ntc_wb7720_anchors_temp[NTC_WB7720_ANCHORS_N-1];
	for (uint8_t i = 0; i < NTC_WB7720_ANCHORS_N - 1; i++) {
		uint16_t r_hi = ntc_wb7720_anchors_raw[i];
		uint16_t r_lo = ntc_wb7720_anchors_raw[i+1];
		if (raw <= r_hi && raw >= r_lo) {
			int16_t t_lo = ntc_wb7720_anchors_temp[i];
			int16_t t_hi = ntc_wb7720_anchors_temp[i+1];
			int32_t dt = t_hi - t_lo;
			int32_t dr = r_hi - r_lo;
			int32_t di = r_hi - raw;
			return (int16_t)(t_lo + (di * dt) / dr);
		}
	}
	return 25;
}

/* WPC线圈NTC: WB7720 I2C采样 → 三条件校验(I2C/STATUS/SEQ) → 归一化 → 锚点反查 → 8深滑动平均
 * Fail-safe: 任一条件异常时返回上次有效值(首次25°C)并置coil_ntc_source_fault */
#define NTC_WPC_SEQ_TIMEOUT_MS  1000

int16_t fml_ntc_temp_get_wpc(void)
{
	static uint16_t s_hist[8] = {0};
	static uint8_t  s_hist_idx = 0;
	static uint8_t  s_hist_primed = 0;
	static int16_t  s_last_good_temp = 25;

	static uint8_t  s_last_seq = 0xFF;
	static uint32_t s_last_seq_tk = 0;
	static bool     s_seq_ever = false;

	static bool     s_get_logged    = false;
	static int      s_last_log_r    = -999;
	static uint8_t  s_last_log_st   = 0xFF;
	static int16_t  s_last_log_temp = 0x7FFF;

	uint16_t raw = 0;
	uint8_t  st  = 0;
	uint8_t  seq = 0;
	int r = usb_bridge_read_ntc_raw(NTC_CH_COIL, &raw, &st, &seq);

	printk("[WLS_NTC_RAW] r=%d raw=%d st=0x%02X seq=%d\n", r, raw, st, seq);

	/* Condition 1: I2C 通讯失败 */
	if (r != 0) {
#if (CONFIG_WLS_NTC_FAIL_MASK == 1)
		gd->prot_sts.coil_ntc_source_fault = 0;
#else
		gd->prot_sts.coil_ntc_source_fault = 1;
#endif
		if (!s_get_logged || r != s_last_log_r) {
			printk("[WLS_NTC_LOCK] GET r=%d st=%d seq=%d temp=%d path=i2c_fail\n", r, st, seq, s_last_good_temp);
			s_get_logged = true; s_last_log_r = r; s_last_log_st = st;
		}
		return s_last_good_temp;
	}

	/* Condition 2: STATUS 异常 */
	if (st == COIL_NTC_STATUS_ERR) {
#if (CONFIG_WLS_NTC_FAIL_MASK == 1)
		gd->prot_sts.coil_ntc_source_fault = 0;
#else
		gd->prot_sts.coil_ntc_source_fault = 1;
#endif
		if (!s_get_logged || st != s_last_log_st) {
			printk("[WLS_NTC_LOCK] GET r=%d st=%d seq=%d temp=%d path=status_err\n", r, st, seq, s_last_good_temp);
			s_get_logged = true; s_last_log_r = r; s_last_log_st = st;
		}
		return s_last_good_temp;
	}
	if (st != COIL_NTC_STATUS_VALID) {
		if (!s_get_logged || st != s_last_log_st) {
			printk("[WLS_NTC_LOCK] GET r=%d st=%d seq=%d temp=%d path=status_invalid\n", r, st, seq, s_last_good_temp);
			s_get_logged = true; s_last_log_r = r; s_last_log_st = st;
		}
		return s_last_good_temp;
	}

	/* Condition 3: SEQ 冻死 (>1000ms 不变视为 WB7720 卡死) */
	uint32_t now = tc_sys_ticks;
	if (s_seq_ever && seq == s_last_seq) {
		if ((uint32_t)(now - s_last_seq_tk) > NTC_WPC_SEQ_TIMEOUT_MS) {
#if (CONFIG_WLS_NTC_FAIL_MASK == 1)
			gd->prot_sts.coil_ntc_source_fault = 0;
#else
			gd->prot_sts.coil_ntc_source_fault = 1;
#endif
			printk("[WLS_NTC_LOCK] GET seq_freeze seq=%d age=%lu\n", seq, (unsigned long)(now - s_last_seq_tk));
			return s_last_good_temp;
		}
	} else {
		s_last_seq = seq;
		s_last_seq_tk = now;
		s_seq_ever = true;
	}

	/* 三条件全过: 清 fault, 消费 RAW */
	gd->prot_sts.coil_ntc_source_fault = 0;

	/* 8深滑动平均 */
	s_hist[s_hist_idx & 0x07] = raw;
	s_hist_idx++;
	if (s_hist_idx >= 8) s_hist_primed = 1;

	uint16_t raw_avg;
	if (s_hist_primed) {
		uint32_t sum = 0;
		for (uint8_t i = 0; i < 8; i++) sum += s_hist[i];
		raw_avg = (uint16_t)(sum >> 3);
	} else {
		raw_avg = raw;
	}

	/* 两步归一化: 10-bit→12-bit 位宽扩展 + Vref/VDDH 电压校正 */
	uint32_t raw_12bit = ((uint32_t)raw_avg * LEGACY_ADC_FULL_SCALE) / WB7720_ADC_FULL_SCALE;
	uint16_t raw_norm  = (uint16_t)((raw_12bit * WB7720_ADC_VREF_MV) / WB7720_NTC_VDDH_MV);

	int16_t temp = ntc_wb7720_raw_to_temp_c(raw_norm);

	if (!s_get_logged || temp != s_last_log_temp) {
		printk("[WLS_NTC_RAW] raw=%u norm=%u temp=%d\n", raw_avg, raw_norm, temp);
		s_get_logged = true; s_last_log_r = r; s_last_log_st = st; s_last_log_temp = temp;
	}

	s_last_good_temp = temp;
	return temp;
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
void fml_bat_ov_forbid_check(void) {
    static uint8_t ov_forbid_consec_cnt = 0;
    // if (gd->forbid_bypass_flag) return;
	printk("gd->bat_ov_forbid_flag=%d\n",gd->bat_ov_forbid_flag);
    if (gd->bat_ov_forbid_flag) {
        if (g_buckboost.woke_mode != BUCKBOOST_SHUTDOWM_MODE) {
            buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
        }
        printk("\r\n[OV_FORBID] Active (flag=1)");
        return;
    }

    uint16_t cell1 = g_buckboost.adc_vcell1;
    uint16_t cell2 = g_buckboost.adc_vcell2;
    uint16_t total = cell1 + cell2;
    uint16_t max_cell = (cell1 > cell2) ? cell1 : cell2;

    printk("\r\n[OV_CHK] c1=%d c2=%d t=%d max=%d", cell1, cell2, total, max_cell);

    /* 可疑高读数去抖：max_cell > 5200mV 时延时 5s (50 × 100ms) 后再判定，
     * 避免 ADC 瞬态毛刺直接触发 forbid。延时窗口内 return，窗口结束后放行到正常判定。*/
    static uint8_t suspect_delay_left = 0;
    static uint8_t suspect_delay_done = 0;
    if (max_cell > 5200) {
        if (!suspect_delay_done) {
            if (suspect_delay_left == 0) {
                suspect_delay_left = 50;
                printk("\r\n[OV_CHK] suspect max=%d>5200, delay 5s", max_cell);
                return;
            }
            suspect_delay_left--;
            if (suspect_delay_left > 0) {
                return;
            }
            suspect_delay_done = 1;
            printk("\r\n[OV_CHK] 5s elapsed, max=%d, proceed", max_cell);
        }
        /* suspect_delay_done==1 → 继续走下面的正常判定 */
    } else {
        suspect_delay_left = 0;
        suspect_delay_done = 0;
    }

    if (max_cell >= OVER_VOLTAGE_FORBID_THRESHOLD) {
        ov_forbid_consec_cnt++;
        printk("\r\n[OV_FORBID] %dmV >= %dmV, cnt=%d",
               max_cell, OVER_VOLTAGE_FORBID_THRESHOLD, ov_forbid_consec_cnt);
        if (ov_forbid_consec_cnt >= OVER_VOLTAGE_FORBID_CONSEC_COUNT) {
            gd->bat_ov_forbid_flag = 1;
#if OV_FORBID_FLASH_PERSIST
            cycle_count_save_to_flash();
            printk("\r\n[OV_FORBID] Persisted to Flash.");
#else
            printk("\r\n[OV_FORBID] TRIGGERED! Forbidden until power cycle.");
#endif
            buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
        }
    } else {
        ov_forbid_consec_cnt = 0;
    }
}
/*------------------------------------------- BAT_OV_FORBID --------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++ BAT_UV_FORBID +++++++++++++++++++++++++++++++++++++++*/
void fml_bat_uv_forbid_check(void) {
    static uint8_t uv_forbid_consec_cnt = 0;
    static uint8_t uv_log_cnt = 0;
    uint8_t do_log = (++uv_log_cnt >= 100);
    if (do_log) uv_log_cnt = 0;

    if (gd->forbid_bypass_flag) return;

    if (gd->bat_uv_forbid_flag) {
        if (g_buckboost.woke_mode != BUCKBOOST_SHUTDOWM_MODE) {
            buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
        }
        if (do_log) printk("\r\n[UV_FORBID] Active (flag=1)");
        return;
    }

    uint16_t cell1 = g_buckboost.adc_vcell1;
    uint16_t cell2 = g_buckboost.adc_vcell2;
    uint16_t total = cell1 + cell2;
    if (total == 0) return;
    uint16_t min_cell = (cell1 < cell2) ? cell1 : cell2;

    if (do_log) printk("\r\n[UV_CHK] c1=%d c2=%d t=%d min=%d", cell1, cell2, total, min_cell);

    if (min_cell > 0 && min_cell <= UNDER_VOLTAGE_FORBID_THRESHOLD) {
        uv_forbid_consec_cnt++;
        printk("\r\n[UV_FORBID] %dmV <= %dmV, cnt=%d",
               min_cell, UNDER_VOLTAGE_FORBID_THRESHOLD, uv_forbid_consec_cnt);
        if (uv_forbid_consec_cnt >= UV_FORBID_CONSEC_COUNT) {
            gd->bat_uv_forbid_flag = 1;
            printk("\r\n[UV_FORBID] TRIGGERED! Forbidden until power cycle.");
            buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
        }
    } else {
        uv_forbid_consec_cnt = 0;
    }
}
/*------------------------------------------- BAT_UV_FORBID --------------------------------------*/
