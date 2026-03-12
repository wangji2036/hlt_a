#include "regdef.h"
#include "tcpc.h"
#include "dpdm.h"
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "tcpm.h"
#include "pd.h"
#include "pdlib.h"
#include "nu6801.h"
#include "ufcs.h"
#include "afc_scp.h"
#include "usb_qc.h"
#include "port_manager.h"

void dpdm_source_init(void);
uint16_t qc_volt = 5000;
uint8_t bc12_type = 0;
uint8_t dpdm_map = 0xff;
uint8_t dpdm_snk_support = 0;

uint8_t dpdm_snk_qc_volt = 0;

//static enum dpdm_state_e dpdm_state = DPDM_OFF_STATE;

void usb_dpdm_task_init(void)
{
	osal_task_handler_reg(USB_DPDM_TASK, usb_dpdm_task_event_handler);
	osal_start_timerEx(USB_BC12_TIMER, 100, 100, USB_DPDM_TASK, DPDM_EVT_TIMER_PERIOD);

	dpdm_source_init();
}

void dpdm_source_init(void)
{
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM->HVDCP_CTRL.BITS.ENTER_DCP_INT_MASK = 1;
	DPDM->HVDCP_CTRL.BITS.ENTER_HVDCP_INT_MASK = 1;

	uint32_t *dpdm_ovrd_offset0 = (uint32_t *)(0x4000c0bc);
	uint32_t *dpdm_ovrd_offset4 = (uint32_t *)(0x4000c0bc + 4);

	*dpdm_ovrd_offset0 = 0x09;
	*dpdm_ovrd_offset4 |= 0x09;

	*dpdm_ovrd_offset0 = 0x0a;
	*dpdm_ovrd_offset4 |= ((*dpdm_ovrd_offset4) & ~0x30);

	*dpdm_ovrd_offset0 = 0x0b;
	*dpdm_ovrd_offset4 |= (((*dpdm_ovrd_offset4) & ~0x30) | 0x30);
}

void usb_dpdm_port0_switch(bool en)
{
	VIC_vModuleDisable();
	if(en)
	{
		GPA->MODE.BITS.PIN0 = 3; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C
		GPA->MODE.BITS.PIN1 = 3; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
	}
	else
	{
		// GPA->I_EN.BITS.PIN0 = 1;
		GPA->MODE.BITS.PIN0 = 0; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C
		// GPA->I_EN.BITS.PIN1 = 1;
		GPA->MODE.BITS.PIN1 = 0; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
	}
	VIC_vModuleEnable();
}

void usb_dpdm_select(uint8_t tc_index)
{
// 	if(tc_index == 0)
// 	{
// 		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 1;
// 	}
// 	else if(tc_index == 1)
// 	{
// 		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 3;  //
// 	}
// 	else if(tc_index == 2)
// 	{
// 		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 2; //DPDM-A
// 	}
// 	else
// 	{
// 		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 0;
// 	}

// 	dpdm_sink_deinit();

// //	DPDM->SOURCE_CTRL.BITS.PORT1_CTRL = 0;
// //	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 0;
// //	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;

// 	DPDM->SOURCE_CTRL.BITS.PORT1_CTRL = 1;
// 	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 0;
// 	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;

// 	bc12_type = 0;
// 	dpdm_map = tc_index;
// 	printk("dpdm_map=%d\n",dpdm_map);
}

void usb_dpdm_autodcp_en(void)
{
	//
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 1;
	DPDM->SOURCE_CTRL.BITS.EN_HVDCP_DET = 1;
	DPDM->SOURCE_CTRL.BITS.EN_QC_SRC_DET = 2;
	DPDM->SOURCE_CTRL.BITS.EN_AUTO_DCP = 1;
	DPDM->SOURCE_CTRL.BITS.EN_AFC_SRC_DET = 1;
	DPDM->SOURCE_CTRL.BITS.EN_SCP_SRC_DET = 1;
#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)
	DPDM->SOURCE_CTRL.BITS.EN_UFCS_SRC_DET = 1;
#endif
	DPDM->SOURCE_CTRL.BITS.EN_900K_PD = 1;

	//

	DPDM->HVDCP_CTRL.BITS.DP_FAIL_DEG = 3;

	DPDM->HVDCP_CTRL.BITS.ENTER_DCP_INT_MASK = 0;
	DPDM->HVDCP_CTRL.BITS.ENTER_HVDCP_INT_MASK = 0;

	DPDM->QC_SRC_CTRL.BITS.TACTIVE = 1;
	DPDM->QC_SRC_CTRL.BITS.FIXED_5V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.FIXED_9V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.FIXED_12V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.CONTINUOUS_MODE_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.QC_PULSE_DEC_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.QC_PULSE_INC_INT_MASK = 0;

	DPDM->AFC_CTRL.BITS.AFC_RX_DATA_MASK = 0;
	DPDM->AFC_CTRL.BITS.SCP_RX_DATA_MASK = 0;
	//printk("AFC_CTRL=0x%x\n",(uint32_t)(&DPDM->AFC_CTRL));
	//printk("AFC_CTRL=0x%x\n",DPDM->AFC_CTRL.WORD);
}

extern union scp_packet_t scp_tx;
bool is_enter_dpdm_prot = false;
#if(BUCKBOOST_USED_NU6801 == 1)
uint16_t adc_input = 0;
#endif
void usb_dpdm_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case DPDM_EVT_SRC_ATTACHED:
			usb_dpdm_autodcp_en();
		#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)
			dpdm_ufcs_init();
		#endif
			is_enter_dpdm_prot = false;
			//printk("SOURCE_CTRL=0x%x\n",DPDM->SOURCE_CTRL.WORD);
			break;
		case DPDM_EVT_SRC_UNATTCHED:
			is_enter_dpdm_prot = false;
			dpdm_source_init();
		#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)
			dpdm_ufcs_deinit();
		#endif
			break;
		case DPDM_EVT_ENTER_DCP:
			if(is_enter_dpdm_prot) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
			printk("enter dcp\n");
			//usb_dpdm_autodcp_en();
			break;
		case DPDM_EVT_ENTER_HVDCP://if enter dpdm,buck to 5v
			if(is_enter_dpdm_prot) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
			printk("hvdcp\n");
			break;
		case DPDM_EVT_TIMER_PERIOD:
			//printk("SOURCE_CTRL=0x%x\n",(uint32_t)(&DPDM->SOURCE_CTRL));
			//printk("DPDM_RESULT=0x%x\n",DPDM->SOURCE_STAT.WORD);
			break;
		case DPDM_EVT_QC_FIXED_5V:
		case DPDM_EVT_QC_FIXED_9V:
		case DPDM_EVT_QC_FIXED_12V:
		case DPDM_EVT_QC_FIXED_20V:
			is_enter_dpdm_prot = true;
			if(pdlib_is_connect() && pdlib_get_source_supply_voltage() != 5000)
			{
				printk("pd has work,qc should not work\n");
				return;
			}

			switch(DPDM->QC_SRC_FLAG.BITS.QC_SRC_STAT)
			{
				case QC_NOT_MODE:
					break;
				case QC_FIX_5V_MODE:
					qc_volt = 5000;
					break;
				case QC_FIX_9V_MODE:
					qc_volt = 9000;
					break;
				case QC_FIX_12V_MODE:
					qc_volt = 12000;
					break;
				case QC_FIX_20V_MODE:
					qc_volt = 20000;  //not support qc 20v
					break;
				case QC_CONTINUOUS_MODE:
					break;
			}
			uint16_t qc_current = 18000 * 1000/ qc_volt;

			qc_current = qc_current > 3000? 3000 : qc_current;

			hal_tcpc_pd_set_bus_iv(0,qc_volt,qc_current + 300,0,10);

			printk("qc2 v= %d i= %d\n",qc_volt,qc_current);
			break;
		case DPDM_EVT_QC_CONTINUES:
			break;

		case DPDM_EVT_AFC_SCP_OUT:
			is_enter_dpdm_prot = true;
			hal_tcpc_pd_set_bus_iv(0,scp_vout,scp_iout,0,10);
			break;

		case DPDM_EVT_QC_PLUSE_INC:
		case DPDM_EVT_QC_PLUSE_DEC:
			is_enter_dpdm_prot = true;
			if(pdlib_is_connect() && pdlib_get_source_supply_voltage() != 5000)
			{
				printk("pd has work,qc should not work\n");
				return;
			}
			if(DPDM->QC_SRC_FLAG.BITS.QC_SRC_STAT == QC_CONTINUOUS_MODE)
			{
				uint16_t qc3_current = 18000 * 1000/ qc_volt;
				qc3_current = qc3_current > 3000? 3000 : qc3_current;
				hal_tcpc_pd_set_bus_iv(0,qc_volt,qc3_current + 300,0,10);

				printk("qc3 v= %d i= %d\n",qc_volt,qc3_current);
			}

			break;
		case DPDM_EVT_AFC_RX_DATA:
			//printk("afc rx = 0x%x\n",DPDM->AFC_RX_0.WORD);
			break;
		case DPDM_EVT_SCP_RX_DATA:
			printk("\n");
			printk("SCP RX:");
			for(uint8_t i = 0; i < scp_packet.bytes.msg_len;i++)
			{
				printk(" 0x%x",((uint8_t *)(&scp_packet.bytes.msg_0))[i]);
			}
			printk("\n");

			break;
		case DPDM_EVT_SCP_TX_DATA:

			printk("SCP TX:");
			for(uint8_t i = 0; i < scp_tx.bytes.msg_len;i++)
			{
				printk(" 0x%x",((uint8_t *)(&scp_tx.bytes.msg_0))[i]);
			}
			printk("\n");
			break;

		case DPDM_EVT_SNK_ATTACHED:
			osal_stop_timerEx(DPDM_SINK_TIMER);
			dpdm_sink_init();
			break;
		case DPDM_EVT_SNK_UNATTCHED:
			dpdm_sink_deinit();
			break;
		case DPDM_EVT_SNK_BC12DONE:
			printk("\r\n  BC12 bc12_type=0x%x  UNSTANDARD_TYPE = 0x%x \r\n",DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE,DPDM_QC_SINK->BC1P2_STAT.BITS.UNSTANDARD_TYPE);
			if(DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x02)
				bc12_type = BC1P2_CDP;
			else if(DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x03)
				bc12_type = BC1P2_DCP;
			else if(DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x06)
			{
				bc12_type = BC1P2_APPLE; //APPLE 2.4A 2.1A
			}
			else
				bc12_type = BC1P2_SDP;
			if(DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x03) //DCP
				osal_start_timerEx(DPDM_SINK_TIMER, 25, 0, USB_DPDM_TASK, DPDM_EVT_SNK_HVDCP_START);
			break;
		case DPDM_EVT_SNK_HVDCP_START:
			osal_stop_timerEx(DPDM_SINK_TIMER);
			printk("hvdcp start\n");
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_EN = 1;
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x03;
			DPDM->SOURCE_CTRL.BITS.EN_900K_PD = 1;
			break;
		case DPDM_EVT_SNK_HVDCP_DONE:
			printk("hvdcp done\n");
			//osal_set_event(USB_TASK,TCPM_EVT_HVDCP_DONE);

			if(g_port.snk_5v_only == 0 && pdlib_is_connect() == false)
			{
				osal_start_timerEx(DPDM_SINK_TIMER, 50, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC_START);
			}
			else
			{
				bc12_type = BC1P2_HVDCP;
				osal_set_event(USB_TASK,TCPM_EVT_DPDM_DONE);
			}
			break;
		case DPDM_EVT_SNK_HVDCP_FAIL:
			osal_set_event(USB_TASK,TCPM_EVT_DPDM_DONE);
			break;
		case DPDM_EVT_SNK_QC_START:
			buckboost_ops.set_ovp(21000);//20000
			bc12_type = BC1P2_HVDCP;
			qc2_set_volt(12000);
			osal_start_timerEx(DPDM_SINK_TIMER, 200, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC12V_DONE);
			break;
		case DPDM_EVT_SNK_QC12V_DONE:
		#if(BUCKBOOST_USED_NU6801 == 1 && 0)
			if(dpdm_map == 0)
				adc_input = hal_nu6801_buckboost_typeca_vbus_present();
			else
				adc_input = hal_nu6801_buckboost_typecb_vbus_present();
			printk("Set Qc 12V=%d\n",adc_input);
			if(adc_input >= 10500)
		#else
			printk("Set Qc 12V=%d\n",g_buckboost.adc_vbus);
			if(g_buckboost.adc_vbus >= 10500)
		#endif
			{
				bc12_type = BC1P2_QC12V;
				qc2_set_volt(5000);
				pdlib_disable_usbpd();
			}
			else
			{
				qc2_set_volt(9000);
				osal_start_timerEx(DPDM_SINK_TIMER, 200, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC_DONE);
			}
			osal_set_event(USB_TASK,TCPM_EVT_DPDM_DONE);
			break;

		case DPDM_EVT_SNK_QC_DONE:
		#if(BUCKBOOST_USED_NU6801 == 1 && 0)
			if(dpdm_map == 0)
				adc_input = hal_nu6801_buckboost_typeca_vbus_present();
			else
				adc_input = hal_nu6801_buckboost_typecb_vbus_present();
			printk("Set Qc 9V=%d\n",adc_input);
			if(adc_input >= 7500)
		#else
			printk("Set Qc 9V=%d\n",g_buckboost.adc_vbus);
			if(g_buckboost.adc_vbus >= 7500)
		#endif
			{
				bc12_type = BC1P2_QC9V;
				pdlib_disable_usbpd();
			}
			qc2_set_volt(5000);
			osal_set_event(USB_TASK,TCPM_EVT_DPDM_DONE);
			break;
#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)
		case DPDM_EVT_UFCS_INT:
			break;
		case DPDM_EVT_UFCS_RX_PACKET:
			ufcs_rx_packet_handle();
			break;
		case DPDM_EVT_UFCS_PSREADY:
			ufcs_psread_handle();
			break;
		case DPDM_EVT_UFCS_RX_RESET:
			printk("UFCS HARDRESET\n");
			ufcs_exit_handle();
			break;
#endif
		default:
			break;
	}
}



void __attribute__((isr)) DCP_HVDCP_IRQHandler(void)
{
    uint32_t int_flag = (DPDM->HVDCP_FLAG.WORD) & 0b00001100;
    do
    {
		if(int_flag & (0x01<<2))
		{
			DPDM->HVDCP_FLAG.BITS.ENTER_DCP = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_ENTER_DCP);
		}

		if(int_flag & (0x01<<3))
		{
			DPDM->HVDCP_FLAG.BITS.ENTER_HVDCP = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_ENTER_HVDCP);
		}

		int_flag = (DPDM->HVDCP_FLAG.WORD) & 0b00001100;

    } while(int_flag);
}



void __attribute__((isr)) QC_SRC_IRQHandler(void)
{

    uint32_t int_flag = (DPDM->QC_SRC_FLAG.WORD) & 0x3F80;

    //printk("qc3 int_flag = 0x%x\n",DPDM->QC_SRC_FLAG.WORD);
    do
    {
		if(int_flag & (0x01<<7))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_5V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_FIXED_5V);
		}

		if(int_flag & (0x01<<8))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_9V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_FIXED_9V);
		}

		if(int_flag & (0x01<<9))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_12V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_FIXED_12V);
		}

		if(int_flag & (0x01<<10))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_20V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_FIXED_20V);
		}

		if(int_flag & (0x01<<11))
		{
			DPDM->QC_SRC_FLAG.BITS.CONTINUOUS_MODE_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_CONTINUES);
		}

		if(int_flag & (0x01<<12))
		{
			DPDM->QC_SRC_FLAG.BITS.QC_PULSE_INC_INT = 1;
			qc_volt += 200;
			if(qc_volt >= 12000) qc_volt = 12000;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_PLUSE_INC);
		}

		if(int_flag & (0x01<<13))
		{
			qc_volt -= 200;
			if(qc_volt <= 5000) qc_volt = 5000;
			DPDM->QC_SRC_FLAG.BITS.QC_PULSE_DEC_INT = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_PLUSE_DEC);
		}

		int_flag = (DPDM->QC_SRC_FLAG.WORD) & 0x3F80;

    } while(int_flag);
}



void __attribute__((isr)) AFC_SCP_SRC_IRQHandler(void)
{
    uint32_t int_flag = (DPDM->AFC_INT_FLAG.WORD) & 0x0060;

    do
    {
		if(int_flag & (0x01<<5))
		{
			DPDM->AFC_INT_FLAG.BITS.AFC_RX_DATA_READY_FLAG = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_RX_DATA);
			dpdm_src_afc_handle();
		}

		if(int_flag & (0x01<<6))
		{
			DPDM->AFC_INT_FLAG.BITS.SCP_RX_DATA_READY_FLAG = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SCP_RX_DATA);
			extern void dpdm_src_scp_handle(void);
			dpdm_src_scp_handle();
		}

		int_flag = (DPDM->AFC_INT_FLAG.WORD) & 0x0060;

    } while(int_flag);

}




