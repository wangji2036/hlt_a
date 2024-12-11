#include "regdef.h"
#include "tcpc.h"
#include "dpdm.h"
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "tcpm.h"
#include "pd.h"
#include "typec.h"
#include "afc_scp.h"
#include "usb_qc.h"
#include "usb_pd.h"
void dpdm_init(void);

uint16_t qc_volt = 5000;



//static enum dpdm_state_e dpdm_state = DPDM_OFF_STATE;

void usb_dpdm_task_init(void)
{
	osal_task_handler_reg(USB_DPDM_TASK, usb_dpdm_task_event_handler);
	osal_start_timerEx(USB_BC12_TIMER, 100, 100, USB_DPDM_TASK, DPDM_EVT_TIMER_PERIOD);

	dpdm_init();
}

void dpdm_init(void)
{
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM->HVDCP_CTRL.BITS.ENTER_DCP_INT_MASK = 1;
	DPDM->HVDCP_CTRL.BITS.ENTER_HVDCP_INT_MASK = 1;
}

void usb_dpdm_select(uint8_t tc_index)
{
	if(tc_index == 0)
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 2;
	else if(tc_index == 1)
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 3;
	else
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 1;
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
	DPDM->SOURCE_CTRL.BITS.EN_900K_PD = 1;
	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 1;
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
	printk("AFC_CTRL=0x%x\n",(uint32_t)(&DPDM->AFC_CTRL));
	printk("AFC_CTRL=0x%x\n",DPDM->AFC_CTRL.WORD);
}

extern union scp_packet_t scp_tx;

void usb_dpdm_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case DPDM_EVT_SRC_ATTACHED:
			usb_dpdm_autodcp_en();
			printk("SOURCE_CTRL=0x%x\n",DPDM->SOURCE_CTRL.WORD);
			break;
		case DPDM_EVT_SRC_UNATTCHED:
			dpdm_init();
			break;
		case DPDM_EVT_ENTER_DCP:
			printk("enter dcp\n");
			usb_dpdm_autodcp_en();
			break;
		case DPDM_EVT_ENTER_HVDCP://if enter dpdm,buck to 5v
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

			if(g_usb_pd_s.explicit_contract && g_usb_pd_s.supply_voltage != 5000)
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
					//qc_volt = 20000;  //not support qc 20v
					break;
				case QC_CONTINUOUS_MODE:
					break;
			}
			hal_tcpc_pd_set_bus_iv(0,qc_volt,3000,0,0);
			printk("qc2 volt = %d\n",qc_volt);
			break;
		case DPDM_EVT_QC_CONTINUES:
			break;

		case DPDM_EVT_AFC_SCP_OUT:
			hal_tcpc_pd_set_bus_iv(0,scp_vout,scp_iout,0,0);
			break;

		case DPDM_EVT_QC_PLUSE_INC:
		case DPDM_EVT_QC_PLUSE_DEC:
			if(g_usb_pd_s.explicit_contract && g_usb_pd_s.supply_voltage != 5000)
			{
				printk("pd has work,qc should not work\n");
				return;
			}
			if(DPDM->QC_SRC_FLAG.BITS.QC_SRC_STAT == QC_CONTINUOUS_MODE)
				hal_tcpc_pd_set_bus_iv(0,qc_volt,3000,0,0);
			printk("qc3 volt = %d\n",qc_volt);
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
			qc_init();
			break;
		case DPDM_EVT_SNK_UNATTCHED:
			qc_deinit();
			break;
		case DPDM_EVT_SNK_BC12DONE:
			osal_start_timerEx(DPDM_SINK_TIMER, 25, 25, USB_DPDM_TASK, DPDM_EVT_SNK_HVDCP_START);
			printk("bc12_type = %d\n",DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE);
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
			osal_start_timerEx(DPDM_SINK_TIMER, 50, 50, USB_DPDM_TASK, DPDM_EVT_SNK_QC_START);
			break;
		case DPDM_EVT_SNK_QC_START:
			printk("Set Qc 9V\n");
			osal_stop_timerEx(DPDM_SINK_TIMER);
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x01;
			break;
		default:
			break;
	}
}

void dpdm_src_afc_handle(void)
{
	#define AFC_CMD_RESET                       (0x01)                  //afc 5v
	#define AFC_CMD_5V                          (0x08)                  //afc 5v
	#define AFC_CMD_9V                          (0x46)                  //afc 9v
	#define AFC_CMD_12V                         (0x79)                  //afc 12v
	static uint16_t afc_volt = 5000;

	switch(DPDM->AFC_RX_0.BITS.RX_BUFFER_0)
	{
		case AFC_CMD_RESET:
			afc_volt = 5000;
			break;
		case AFC_CMD_5V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_5V << 8) | 0x01);
			afc_volt = 5000;
			break;
		case AFC_CMD_9V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_9V << 8) | 0x01);
			afc_volt = 9000;
			break;
		case AFC_CMD_12V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_12V << 8) | 0x01);
			afc_volt = 12000;
			break;
	}

	hal_tcpc_pd_set_bus_iv(0,afc_volt,3000,2,0);
}

void __attribute__((isr)) DCP_HVDCP_IRQHandler(void)
{
    uint32_t int_flag = (DPDM->HVDCP_FLAG.WORD) & 0x0300;
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

		int_flag = (DPDM->HVDCP_FLAG.WORD) & 0x0300;

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
			if(qc_volt <= 3600) qc_volt = 3600;
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




