#include "regdef.h"
#include "usb_qc.h"
#include "dpdm.h"
#include "delay.h"
#include "printk.h"
#include "tcpm.h"

void dpdm_sink_init(void)
{
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.DPDM_EN = 1;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_EN = 1;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_DET_DONE_INT_MASK = 0;
	DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.HVDCP_DET_OK_INT_MASK = 0x0;
	DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.HVDCP_DET_FAIL_INT_MASK = 0x0;
	bc12_type = 0;
	printk("bc12_init\n");

	uint32_t *dpdm_ovrd_offset4 = (uint32_t *)(0x4000c0bc + 4);
	*dpdm_ovrd_offset4 |= 0x80;
}

void dpdm_sink_deinit(void)
{
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.DPDM_EN = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_EN = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_DET_DONE_INT_MASK = 1;
	DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.HVDCP_DET_OK_INT_MASK = 0x1;
	DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.HVDCP_DET_FAIL_INT_MASK = 0x1;

	printk("bc12_deinit\n");
}

void qc2_set_volt(uint16_t qc_volt)
{
	switch(qc_volt)
	{
		case VOLTAGE_5V:
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x03;
			break;
		case VOLTAGE_9V:
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x01;
			break;
		case VOLTAGE_12V:
			DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x00;
			break;
	}
	printk("qc set =%d\n",qc_volt);
}


void qc3_set_volt(void)
{

}

void qc_handshake_handle(void)
{

}

void __attribute__((isr)) DPDM_SINK_IRQHandler(void)
{
	uint32_t int0 = DPDM_QC_SINK->BC1P2_INT_FLAG.WORD & 0x80;
	uint32_t int1 = DPDM_QC_SINK->QC_INT_FLAG.WORD & 0xC0;
	do
	{
		if(int0 & (0x80))
		{
			DPDM_QC_SINK->BC1P2_INT_FLAG.BITS.BC1P2_DET_DONE_INT_FLAG = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_BC12DONE);
		}

		if(int1 & (0x80))
		{
			DPDM_QC_SINK->QC_INT_FLAG.BITS.HVDCP_DET_OK_INT_FLAG = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_HVDCP_DONE);
		}

		if(int1 & (0x40))
		{
			DPDM_QC_SINK->QC_INT_FLAG.BITS.HVDCP_DET_FAIL_INT_FLAG = 1;
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_HVDCP_FAIL);
		}

		int0 = DPDM_QC_SINK->BC1P2_INT_FLAG.BITS.BC1P2_DET_DONE_INT_FLAG;
		int1 = DPDM_QC_SINK->QC_INT_FLAG.BITS.HVDCP_DET_OK_INT_FLAG;

	}while(int0 &0x80 || int1 & 0xC0);

}










