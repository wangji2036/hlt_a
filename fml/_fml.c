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
#define USBD_WB7720_ADDR	0x21
extern uint8_t power_on_cnt;

void fml_task_init(void)
{
	osal_task_handler_reg(FML_TASK, fml_task_event_handler);
	osal_start_timerEx(GAUGE_TIMER, 0,   T_GAUGE, FML_TASK, APL_EVT_GAUGE);
	//osal_start_timerEx(USB_WB7720_TIMER, 0,   47, FML_TASK, APL_HID_REPORT);  // wb IIC通讯已注释
}
void ubsd_wb7720_report_update(void);


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

#define REG_SLEEP           				 0x44    // ????????
#define REG_WAKEUP          			 	 0x45    // ????????

	uint16_t write_buf;

	static uint8_t cnt = 0;

	if(cnt == 0)
	{
		write_buf = gd->real_soc_show;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,SOC_pct,(uint8_t*)&write_buf,1);
	}
	else if(cnt == 1)
	{
		write_buf = 5200;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,Capacity_mAh,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 2)
	{
		write_buf = g_buckboost.adc_vbat;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,VBAT_mV,(uint8_t*)&write_buf,2);
	}
	else if(cnt == 3)
	{
		write_buf = g_buckboost.adc_ibat;
		hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR,IBAT_mA,(uint8_t*)&write_buf,2);
		printk("adc_ibat = %d\n",g_buckboost.adc_ibat);
	}
	else if(cnt == 4)
	{
		write_buf = 300;
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
	}

	cnt++;
	if(cnt >= 10) cnt = 0;
}


void ubsd_wb7720_sleep(void)
{
	hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR,REG_SLEEP,0x01);
}



