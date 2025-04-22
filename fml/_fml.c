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

void fml_task_init(void)
{
	osal_task_handler_reg(FML_TASK, fml_task_event_handler);
	osal_start_timerEx(GAUGE_TIMER, 0,   T_GAUGE, FML_TASK, APL_EVT_GAUGE);
}

void fml_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case FML_EVT_ASK_INT_RECVD:
			fml_ask_decode();
			break;
		case APL_EVT_GAUGE:
			SigPr_CellTemps_C_s = 25;
			SigPr_CellVolts_mV_s = g_buckboost.adc_vbat;
			SigPr_PackCurr_mA_s   = g_buckboost.adc_ibat;
	//		printk("\r\n gauge T/V/I=%d  %d  %d\n",SigPr_CellTemps_C_s, SigPr_CellVolts_mV_s,SigPr_PackCurr_mA_s);
			Cyclic();
		//	printk("\r\n calu ibat =%d", temp);
			//printk("\r\n gauge T=%d  Vbat=%d  Ibat=%d  Ibus=%d\n",SigPr_CellTemps_C_s, SigPr_CellVolts_mV_s,SigPr_PackCurr_mA_s,g_buckboost.adc_ibus);

//			printk("\r\n gauge  Vbat=%d  Ibat=%d  Ibus=%d\n", SigPr_CellVolts_mV_s,SigPr_PackCurr_mA_s,g_buckboost.adc_ibus);

/*			printk("\r\n  gauge  Vbat=%d  Ibat=%d  Ibus=%d  PORT0= %x PORT1= %x PORT2= %x PORT3= %x  \n",
	               SigPr_CellVolts_mV_s,SigPr_PackCurr_mA_s,g_buckboost.adc_ibus,
	               g_port.port_state[PORT0_INDEX],g_port.port_state[PORT1_INDEX],g_port.port_state[PORT2_INDEX],g_port.port_state[PORT3_INDEX]);*/

			break;
		default:
			break;
	}
}
