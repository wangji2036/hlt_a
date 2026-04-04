#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "ask.h"
#include "_fml.h"
#include"led.h"
#include "typdef.h"
#include "BMS_FixPoint.h"
#include "BMS_FixPoint_private.h"
#include "SOC.h"
#include "port_manager.h"
#include "g_data.h"
#include "usb_pd.h"
#include "typec.h"
#include "usb_bridge.h"
#include "nu6805.h"
#define USBD_WB7720_ADDR	0x21
extern uint8_t power_on_cnt;
extern uint8_t bat_cell_num;
uint16_t cell2_voltage;
uint8_t g_wb7720_awake;    // WB7720 wakeup state (1=awake, 0=sleep), separate from gd_t to preserve struct layout

void wb7720_init(void);
void ubsd_wb7720_report_update(void);

void fml_task_init(void)
{
	osal_task_handler_reg(FML_TASK, fml_task_event_handler);
	osal_start_timerEx(GAUGE_TIMER, 0,   T_GAUGE, FML_TASK, APL_EVT_GAUGE);
	osal_start_timerEx(USB_WB7720_TIMER, 0,   47, FML_TASK, APL_HID_REPORT);
	wb7720_init();
}

void wb7720_init(void)
{
	#define CELL_COUNT 				0x2c
	uint8_t write_buf = bat_cell_num;
	hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,CELL_COUNT,(uint8_t*)&write_buf,1);
#if CONFIG_USB_BRIDGE_ENABLE
	usb_bridge_init();
#endif
}

static const uint16_t ntc_3435_tbl[] =
{
	663, 631, 601, 572, 545, 553, 527, 502, 457, 436, //-19 ~ -10
	416, 397, 380, 363, 346, 331, 317, 303, 290, 277, // -9 ~   0
	265, 254, 243, 233, 223, 214, 205, 196, 188, 181, //  1 ~  10
	173, 166, 160, 153, 147, 141, 136, 131, 126, 121, // 11 ~  20
	116, 112, 107, 103, 100, 96,  92,  89,  86,  82, // 21 ~  30
	79,  77,  74,  71,  69,  66,  64,  62,  60,  57, // 31 ~  40
	55,  54,  52,  50,  48,  47,  45,  44,  42,  41, // 41 ~  50
	39,  38,  37,  36,  35,  34,  32,  31,  30,  29, // 51 ~  60
	29,  28,  27,  26,  25,  25,  24,  23,  22,  22, // 61 ~  70
	21,  21,  20,  20,  19,  19,  18,  18,  17,  17, // 71 ~  80
	16,  16,  15,  15,  14,  14,  13,  13,  13,  12, // 81 ~  90
};

int binary_search(uint16_t arr[], uint16_t size, uint16_t target)
{
    int left = 0;
    int right = size - 1;

    while (left <= right) {
    int mid = (left + right) / 2;

    if (arr[mid] == target) {
        return mid;  // 找到目标值
    } else
    if (arr[mid] < target) {
        right = mid - 1;  // 目标值在左半部分
    } else {
        left = mid + 1;   // 目标值在右半部分
    }
    }

    return left;  // 未找到目标值
}


void fml_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case FML_EVT_ASK_INT_RECVD:
			fml_ask_decode();
			break;
		case APL_EVT_GAUGE:
			if(power_on_cnt)
			{
				power_on_cnt--;
			}
			else
			{
				SigPr_CellTemps_C_s = 25;
				SigPr_CellVolts_mV_s = g_buckboost.adc_vbat;
				SigPr_PackCurr_mA_s   = g_buckboost.adc_ibat;
				Cyclic();


				if(gd->Battery_cycle_count <= 50 )
				{
					gd->Bat_Rdc = P_R0Dsg_mOhm[0];
					gd->Bat_SoH = 100;
				}
				else
				{
					gd->Bat_Rdc = P_R0Dsg_mOhm[0] + P_R0Dsg_mOhm[0] * ( gd->Battery_cycle_count - 50) * 5 / 10000;
					gd->Bat_SoH = 100 - ( gd->Battery_cycle_count - 50) * 5 / 100;
					if(gd->Bat_SoH < 0) gd->Bat_SoH = 0;
				}

				// Cycle-based CV voltage adjustment
#if(CONFIG_CYCLE_CV_REDUCTION_ENABLE == 1 && BUCKBOOST_USED_NU6805 == 1)
				{
					uint16_t cv_offset_mv = 0;
					if (gd->Battery_cycle_count >= CYCLE_CV_TIER3_COUNT) {
						cv_offset_mv = CYCLE_CV_TIER3_OFFSET;
					} else if (gd->Battery_cycle_count >= CYCLE_CV_TIER2_COUNT) {
						cv_offset_mv = CYCLE_CV_TIER2_OFFSET;
					} else if (gd->Battery_cycle_count >= CYCLE_CV_TIER1_COUNT) {
						cv_offset_mv = CYCLE_CV_TIER1_OFFSET;
					}

					static uint16_t last_cv_offset = 0xFFFF;  // force first update
					if (cv_offset_mv != last_cv_offset) {
						uint16_t adjusted_cv_pack = (BATTERY_CV_VALUE - cv_offset_mv) * CONFIG_BATTERY_CELL_COUNT;
						hal_nu6805_buckboost_charge_target_volt(adjusted_cv_pack);
						last_cv_offset = cv_offset_mv;
					}
				}
#endif

				//printk("Cycle = %d Rdc = %d SoH = %d RTC_Timer = %d\n",gd->Battery_cycle_count,gd->Bat_Rdc,gd->Bat_SoH,gd->Bat_RTC_Timer);
			}
			break;
		case APL_HID_REPORT:
			ubsd_wb7720_report_update();
			break;
		default:
			break;
	}
}


void ubsd_wb7720_report_update(void)
{
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	/* CC-driven force_usb_mode gate:
	 * Triple-click sets usb_comm_activated, CC presence confirms cable.
	 * usb_bridge_periodic_update() uses force_usb_mode to gate telemetry. */
	{
		bool want_awake = false;
		if (gd->usb_comm_activated) {
			enum tc_cc_status cc1, cc2;
			hal_tcpc_get_cc(1, &cc1, &cc2);
#if (CONFIG_USB_COM_FORCE_SINK == 1)
			if (cc1 >= TYPEC_CC_RP_DEF || cc2 >= TYPEC_CC_RP_DEF)
				want_awake = true;
#else
			if (cc1 == TYPEC_CC_RD || cc2 == TYPEC_CC_RD)
				want_awake = true;
#endif
		}
		gd->force_usb_mode = want_awake ? 1 : 0;
		g_wb7720_awake = gd->force_usb_mode;
	}

	/* DPDM MUX: keep routed when awake */
	if (gd->force_usb_mode) {
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 0;
		DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;
		DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	}
#endif

	/* All telemetry + eng/prod/time now handled inside usb_bridge */
#if CONFIG_USB_BRIDGE_ENABLE
	usb_bridge_periodic_update();
#endif
}


/* ubsd_wb7720_sleep/wakeup moved to usb_bridge.c (usb_bridge_sleep/wakeup) */


