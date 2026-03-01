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
#include "usb_bridge.h"
#include "nu6805.h"
#define USBD_WB7720_ADDR	0x21
extern uint8_t power_on_cnt;
extern uint8_t bat_cell_num;
uint16_t cell2_voltage;

void fml_task_init(void)
{
	osal_task_handler_reg(FML_TASK, fml_task_event_handler);
	osal_start_timerEx(GAUGE_TIMER, 0,   T_GAUGE, FML_TASK, APL_EVT_GAUGE);
	osal_start_timerEx(USB_WB7720_TIMER, 0,   47, FML_TASK, APL_HID_REPORT); 
	wb7720_init();
}
void ubsd_wb7720_report_update(void);
void wb7720_init(void);

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
#if(BUCKBOOST_USED_NU6805 == 1)
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

	#define SOC_pct 				0x00
	#define Capacity_mAh 			0x01
	#define VBAT_mV 				0x05
	#define IBAT_mA 				0x07
	#define TEMP_dC 				0x09
	#define CycleCount 				0x0b
	#define R_internal_mOhm 		0x0d
	
	#define SOH_pct_x100 			0x0f
	#define CELL1_VOLTAGE_MV 		0x2D	
    #define CELL2_VOLTAGE_MV 		0x2F

	#define PCB_Temp_dC				0x35

	#define REG_SLEEP           	0x44    // ????????
	#define REG_WAKEUP          	0x45    // ????????

	uint16_t write_buf;
	

	static uint8_t cnt = 0;

	if(cnt == 0)
	{
		write_buf = gd->real_soc_show;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,SOC_pct,(uint8_t*)&write_buf,1);
	}
	else if(cnt == 1)
	{
		write_buf = CONFIG_BATTERY_CAPACITY_MAH;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,Capacity_mAh,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 2)
	{
		write_buf = g_buckboost.adc_vbat;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,VBAT_mV,(uint8_t*)&write_buf,2);
		printk("adc_vbat = %d %d %d\n",g_buckboost.adc_ibat,g_buckboost.adc_vbat,write_buf);
	}
	else if(cnt == 3)
	{
		write_buf = g_buckboost.adc_ibat;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,IBAT_mA,(uint8_t*)&write_buf,2);
		printk("adc_ibat = %d\n",g_buckboost.adc_ibat);
	}
	else if(cnt == 4)
	{
		g_buckboost.batTemp = (int16_t)binary_search((uint16_t *)ntc_3435_tbl, sizeof(ntc_3435_tbl) / sizeof(ntc_3435_tbl[0]), g_buckboost.adc_tbat1) - 19;
		write_buf =  g_buckboost.batTemp;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,TEMP_dC,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 5)
	{
		write_buf = gd->Battery_cycle_count;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,CycleCount,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 6)
	{
		write_buf = gd->Bat_Rdc;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,R_internal_mOhm,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 7)
	{
		write_buf = gd->Bat_SoH;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,SOH_pct_x100,(uint8_t*)&write_buf,2);
	}else if(cnt == 8)
	{
		cell2_voltage = hal_badc_meas(_BADC_CH_PC7_ADC4) * 1.5;
		printk("cell2_voltage = %d\n",cell2_voltage);
		write_buf = cell2_voltage;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,CELL2_VOLTAGE_MV,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 9)
	{
		write_buf = g_buckboost.adc_vbat - cell2_voltage;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,CELL1_VOLTAGE_MV,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 10)
	{
		g_buckboost.batTemp = gd->sys_infos.ntc_temp_typec;
		write_buf =  g_buckboost.batTemp;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,PCB_Temp_dC,(uint8_t*)&write_buf,2);
	}
#if CONFIG_USB_BRIDGE_ENABLE
	else if(cnt == 11)
	{
		usb_bridge_write_exception_counts();
	}
	else if(cnt == 12)
	{
		usb_bridge_write_charge_state();
	}
	else if(cnt == 13)
	{
		usb_bridge_write_exception_record();
	}
	else if(cnt == 14)
	{
		usb_bridge_check_engineering_mode();
	}
	else if(cnt == 15)
	{
		usb_bridge_check_production_mode();
	}
	else if(cnt == 16)
	{
		usb_bridge_ensure_product_info();
	}
	else if(cnt == 17)
	{
		usb_bridge_check_eng_test_cmds();
	}
#endif

	cnt++;
#if CONFIG_USB_BRIDGE_ENABLE
	if(cnt >= 18) cnt = 0;
#else
	if(cnt >= 11) cnt = 0;
#endif

	static bool is_usb_enable = false;
	static uint8_t qc_delay_cnt = 0;
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if(gd->usb_comm_activated && g_port.port_state[1] != PORT_STATE_NONE)
#else
	if(g_port.port_state[1] != PORT_STATE_NONE)
#endif
	{
		if(g_usb_pd_s.explicit_contract || gd->force_usb_mode)
		{
			DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 0;
			DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;  //
			DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;

			if(is_usb_enable == 0)
			{
				ubsd_wb7720_wakeup();
				is_usb_enable = 1;
			}
		}
		else
		{
			qc_delay_cnt++;
			if(qc_delay_cnt == 10)
			{
				if(g_port.port_state[1] == PORT_STATE_SOURCE )
				{
					usb_dpdm_select(PORT1_INDEX);
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
				}
			}

			if(qc_delay_cnt >= 100) qc_delay_cnt = 100;
		}

	}
	else
	{
		if(is_usb_enable == 1)
		{
			ubsd_wb7720_sleep();
			is_usb_enable = 0;
		}

		qc_delay_cnt = 0;
	}
}


void ubsd_wb7720_sleep(void)
{
	hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR,REG_SLEEP,0x01);
	printk("enter sleep mode\n");
}


void ubsd_wb7720_wakeup(void)
{
	hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR,REG_WAKEUP,0x01);
	printk("enter wake mode\n");
}


