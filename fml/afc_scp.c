#include "tcpm.h"
#include "tcpc.h"
#include "pd.h"
#include "typec.h"
#include "printk.h"
#include "osal.h"
#include "regdef.h"
#include "usb_pd.h"
#include "dpdm.h"
#include "afc_scp.h"
#include "buckboost.h"

union scp_packet_t scp_tx;
union scp_packet_t scp_packet;

uint16_t scp_vout = 5000;
uint16_t scp_iout = 2400;

uint8_t SCP_REG[256] =
{
	[FCP_REG_DEVICE_TYPE] 					= SCP_ADP_TYPE1,
	[FCP_REG_SPEC_VER] 						= 0x00,
	[FCP_REG_SLAVE_CTRL] 					= 0x00,
	[FCP_REG_SLAVE_STATE] 					= 0x00,
	[FCP_REG_ID_OUT0] 						= 0xA2,
	[FCP_REG_CAPABILITIES] 					= FCP_DISCRETE_VOUT_SUPPORT,
	[FCP_REG_DISCRETE_CAPABILITIES] 		= FCP_DISCRETE_CAPABILITIES,
	[FPC_REG_MAX_PWR] 						= FCP_MAX_POWER,
	[FCP_REG_ADAPTER_STATUS] 				= 0x00,
	[FCP_REG_VOUT_STATUS] 					= 0x00,
	[FCP_REG_OUTPUT_CTRL] 					= SCP_ADP_TYPE1,
	[FCP_REG_VOUT_CONFIG] 					= 0x32,
	[FCP_REG_DISCRETE_VOUT0] 				= FCP_DISCRETE_VOUT0,
	[FCP_REG_DISCRETE_VOUT1] 				= FCP_DISCRETE_VOUT1,
	[FCP_REG_DISCRETE_VOUT2] 				= FCP_DISCRETE_VOUT2,
	[FCP_REG_DISCRETE_VOUT3] 				= 0x00,
	[FCP_REG_DISCRETE_VOUT4] 				= 0x00,
	[FCP_REG_DISCRETE_VOUT5] 				= 0x00,
	[FCP_REG_DISCRETE_VOUT6] 				= 0x00,
	[FCP_REG_DISCRETE_VOUT7] 				= 0x00,

	[SCP_REG_ADP_TYPE0] 					= SCP_ADP_TYPE1,
	[SCP_REG_ADP_TYPE1] 					= 0x90,
	[SCP_REG_B_ADP_TYPE] 					= SCP_B_ADP_TYPE,
	[SCP_REG_VENDER_ID_H] 					= SCP_VENDER_ID_H,
	[SCP_REG_VENDER_ID_L] 					= SCP_VENDER_ID_L,
	[SCP_REG_MODULE_ID_H] 					= SCP_MODULE_ID_H,
	[SCP_REG_MODULE_ID_L] 					= SCP_MODULE_ID_L,
	[SCP_REG_SERIAL_NUM_H] 					= SCP_SERIAL_NUM_H,
	[SCP_REG_SERIAL_NUM_L] 					= SCP_SERIAL_NUM_L,
	[SCP_REG_CHIP_ID] 						= SCP_PROTOCOL_CHIP_ID,
	[SCP_REG_HW_VER] 						= SCP_HW_VER,
	[SCP_REG_FW_VER_H] 						= SCP_SW_VER_H,
	[SCP_REG_FW_VER_L] 						= SCP_SW_VER_L,
	[SCP_REG_B_ADP_TYPE1] 					= SCP_ADP_B_TYPE1,
	[SCP_REG_FACTORY_ID] 					= SCP_FACTORY_ID,
	[SCP_REG_RESEVED0] 						= 0x00,
	[SCP_REG_MAX_PWR] 						= SCP_MAX_PWR,
	[SCP_REG_CNT_PWR] 						= SCP_CNT_PWR,
	[SCP_REG_MIN_VOUT] 						= SCP_MIN_VOUT,
	[SCP_REG_MAX_VOUT] 						= SCP_MAX_VOUT,
	[SCP_REG_MIN_IOUT] 						= SCP_MIN_IOUT,
	[SCP_REG_MAX_IOUT] 						= SCP_MAX_IOUT,
	[SCP_REG_VSTEP] 						= SCP_VSTEP,
	[SCP_REG_ISTEP] 						= SCP_ISTEP,
	[SCP_REG_MAX_VERR] 						= SCP_MAX_VERR,
	[SCP_REG_MAX_IERR] 						= SCP_MAX_IERR,
	[SCP_REG_MAX_STTIME] 					= SCP_MAX_STTIME,
	[SCP_REG_MAX_RSPTIME] 					= SCP_MAX_RSPTIME,
	[SCP_REG_RESEVED1] 						= 0x3C,
	[SCP_REG_CTRL_BYTE0] 					= 0xC0,
	[SCP_REG_CTRL_BYTE1] 					= 0x00,
	[SCP_REG_STATUS_BYTE0] 					= 0xC0,
	[SCP_REG_STATUS_BYTE1] 					= 0x00,
	[SCP_REG_STATUS_BYTE2] 					= 0x00,
	[SCP_REG_SSTS] 							= 0x00,
	[SCP_REG_INSIDE_TMP] 					= 50,
	[SCP_REG_PORT_TMP] 						= 50,
	[SCP_REG_READ_VOUT_H] 					= 0x00,
	[SCP_REG_READ_VOUT_L] 					= 0x00,
	[SCP_REG_READ_IOUT_H] 					= 0x03,
	[SCP_REG_READ_IOUT_L] 					= 0xE8,
	[SCP_REG_DAC_VSET_H] 					= 0x00,
	[SCP_REG_DAC_VSET_L] 					= 0x00,
	[SCP_REG_DAC_ISET_H] 					= 3000 >> 8,
	[SCP_REG_DAC_ISET_L] 					= (uint8_t)3000,
	[SCP_REG_VSET_BOUNDARY_H] 				= 10000 >> 8,
	[SCP_REG_VSET_BOUNDARY_L] 				= (uint8_t)10000,
	[SCP_REG_ISET_BOUNDARY_H] 				= 2400 >>8,
	[SCP_REG_ISET_BOUNDARY_L] 				= (uint8_t)2400,
	[SCP_REG_MAX_VSET_OFFSET] 				= 0x00,
	[SCP_REG_MAX_ISET_OFFSET] 				= 0x00,
	[SCP_REG_VSET_H] 						= 5000 >> 8,
	[SCP_REG_VSET_L] 						= (uint8_t)5000,
	[SCP_REG_ISET_H] 						= 2400 >> 8,
	[SCP_REG_ISET_L] 						= (uint8_t)2400,
	[SCP_REG_VSET_OFFSET_H] 				= 0x00,
	[SCP_REG_VSET_OFFSET_L] 				= 0x00,
	[SCP_REG_ISET_OFFSET_H] 				= 0x00,
	[SCP_REG_ISET_OFFSET_L] 				= 0x00,
	[SCP_REG_SREAD_VOUT] 					= 0x00,
	[SCP_REG_SREAD_IOUT] 					= 0x00,
	[SCP_REG_VSSET] 						= 0x00,
	[SCO_REG_ISSET] 						= 0x80,
	[SCP_REG_STEP_VSET_OFFSET] 				= 0x00,
	[SCP_REG_STEP_ISET_OFFSET] 				= 0x00,
	[SCP_REG_SPEC_FUN1] 					= 0x00,
	[SCP_REG_SPEC_FUN2] 					= 0xC0,
	[SCP_TEST_S_REG_SS] 					= 0x00,
};

void update_scp_reg(void)
{
	SCP_REG[SCP_REG_READ_VOUT_H] = (uint8_t)(g_buckboost.buckboost_out_voltage >> 8);
	SCP_REG[SCP_REG_READ_VOUT_L] = (uint8_t)g_buckboost.buckboost_out_voltage;
	SCP_REG[SCP_REG_SREAD_IOUT] = ( - g_buckboost.adc_ibus) / 50;
	SCP_REG[SCP_REG_SREAD_VOUT] = (g_buckboost.buckboost_out_voltage - 3000) / 10;

}


void fcp_single_read_handle(void)
{
	update_scp_reg();
	DPDM->AFC_TX_0.WORD =( (FCP_ACK << 8)| (SCP_REG[scp_packet.bytes.msg_1] << 16) | 0x02);

	osal_set_event(USB_DPDM_TASK,DPDM_EVT_SCP_TX_DATA);
	scp_tx.words[0] = ( (FCP_ACK << 8)| (SCP_REG[scp_packet.bytes.msg_1] << 16) | 0x02);
	scp_tx.words[1] = 00;
	scp_tx.words[2] = 00;
}

void fcp_single_write_handle(void)
{

	DPDM->AFC_TX_0.WORD =( (FCP_ACK << 8) | 0x01);
	SCP_REG[scp_packet.bytes.msg_1] = scp_packet.bytes.msg_2;

	osal_set_event(USB_DPDM_TASK,DPDM_EVT_SCP_TX_DATA);
	scp_tx.words[0] = ( (FCP_ACK << 8) | 0x01);
	scp_tx.words[1] = 00;
	scp_tx.words[2] = 00;
	switch(scp_packet.bytes.msg_1)
	{
		case FCP_REG_OUTPUT_CTRL:
			if(SCP_REG[scp_packet.bytes.msg_1] & 0x01)
			{
				scp_vout = (uint16_t)SCP_REG[FCP_REG_VOUT_CONFIG]* 100;
				//if(scp_vout >= 10000) scp_vout = 10000;
				scp_iout = 24000000 / scp_vout;
				if(scp_iout >= 2400) scp_iout = 2400;
				//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
	        	//usb_pd_set_state(PE_SRC_Disabled,enter_state);
	            hal_tcpc_set_pd_rx(0,EN_HARD_RESET,false);
			}
			break;
		case FCP_REG_VOUT_CONFIG:
        	scp_vout = (uint16_t)SCP_REG[FCP_REG_VOUT_CONFIG]* 100;
        	//if(scp_vout >= 10000) scp_vout = 10000;
			scp_iout = 24000000 / scp_vout;
			if(scp_iout >= 2400) scp_iout = 2400;
        	//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
        	osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
        	//usb_pd_set_state(PE_SRC_Disabled,enter_state);
            hal_tcpc_set_pd_rx(0,EN_HARD_RESET,false);
        	break;


        case SCP_REG_VSET_L:
        	scp_vout = ((uint16_t)SCP_REG[SCP_REG_VSET_H] << 8) | SCP_REG[SCP_REG_VSET_L];
        	if(scp_vout >= 10000) scp_vout = 10000;
			scp_iout = 24000000 / scp_vout;
			if(scp_iout >= 2400) scp_iout = 2400;
        	//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
        	osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
        	//usb_pd_set_state(PE_SRC_Disabled,enter_state);
            hal_tcpc_set_pd_rx(0,EN_HARD_RESET,false);
			break;
        case SCP_REG_SPEC_FUN2://0xCE 调节电流步进
        	SCP_REG[SCP_REG_VSET_H] = 5000 >> 8;
        	SCP_REG[SCP_REG_VSET_L] = (uint8_t)5000;
        	//usb_pd_set_state(PE_SRC_Disabled,enter_state);
            hal_tcpc_set_pd_rx(0,EN_HARD_RESET,false);
            scp_vout = ((uint16_t)SCP_REG[SCP_REG_VSET_H] << 8) | SCP_REG[SCP_REG_VSET_L];
            if(scp_vout >= 10000) scp_vout = 10000;
			scp_iout = 24000000 / scp_vout;
			if(scp_iout >= 2400) scp_iout = 2400;
        	//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
        	osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
            break;

        case SCP_REG_CTRL_BYTE0://0xA0
            if(!(SCP_REG[SCP_REG_CTRL_BYTE0] & 0x40))
            {
            	SCP_REG[FCP_REG_VOUT_CONFIG] = 0x32;
            	SCP_REG[SCP_REG_VSET_H] = 5000 >> 8;
            	SCP_REG[SCP_REG_VSET_L] = (uint8_t)5000;
            	scp_vout = SCP_REG[SCP_REG_VSET_H] << 8 | SCP_REG[SCP_REG_VSET_L];
            	if(scp_vout >= 10000) scp_vout = 10000;
				scp_iout = 24000000 / scp_vout;
				if(scp_iout >= 2400) scp_iout = 2400;
            	//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
            	osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
            	DPDM->SOURCE_CTRL.BITS.SOFT_RESET = 1;
            }
            break;
	}
}

void fcp_multi_read_handle(void)
{
	update_scp_reg();
	union scp_packet_t pkt;
	uint8_t copy_len = scp_packet.bytes.msg_2 > 10? 10: scp_packet.bytes.msg_2;
	osal_mem_copy(&pkt.bytes.msg_1,&SCP_REG[scp_packet.bytes.msg_1],copy_len); //msg:0  1c  1: reg 2: len 3...: data
	pkt.bytes.msg_len = scp_packet.bytes.msg_2 + 1;
	pkt.bytes.msg_0 = FCP_ACK;
	DPDM->AFC_TX_2.WORD = pkt.words[2];
	DPDM->AFC_TX_1.WORD = pkt.words[1];
	DPDM->AFC_TX_0.WORD = pkt.words[0];

	osal_set_event(USB_DPDM_TASK,DPDM_EVT_SCP_TX_DATA);
	scp_tx.words[0] = pkt.words[0];
	scp_tx.words[1] = pkt.words[1];
	scp_tx.words[2] = pkt.words[2];
}

void fcp_multi_write_handle(void)
{
	DPDM->AFC_TX_0.WORD =( (FCP_ACK << 8) | 0x01);
	SCP_REG[scp_packet.bytes.msg_1] = scp_packet.bytes.msg_2;

	osal_set_event(USB_DPDM_TASK,DPDM_EVT_SCP_TX_DATA);
	scp_tx.words[0] = ( (FCP_ACK << 8) | 0x01);
	scp_tx.words[1] = 00;
	scp_tx.words[2] = 00;
	uint8_t copy_len = scp_packet.bytes.msg_2 > 10? 10: scp_packet.bytes.msg_2;
	osal_mem_copy(&SCP_REG[scp_packet.bytes.msg_1],&scp_packet.bytes.msg_3,copy_len);//msg 0:1b  1:reg  2:len  3...: data

	switch(scp_packet.bytes.msg_1)
	{
		case SCP_REG_VSET_H:
			scp_vout = ((uint16_t)SCP_REG[SCP_REG_VSET_H] << 8) | SCP_REG[SCP_REG_VSET_L];
			//printk("SCP_REG_VSET_H = %d\n",SCP_REG[SCP_REG_VSET_H]);
			//printk("SCP_REG_VSET_L = %d\n",SCP_REG[SCP_REG_VSET_L]);
			//printk("scp_vout = %d\n",scp_vout);
        	if(scp_vout >= 10000) scp_vout = 10000;
			scp_iout = 24000000 / scp_vout;
			if(scp_iout >= 2400) scp_iout = 2400;
			//hal_tcpc_pd_set_bus_iv(0,scp_vout,3000,20,0);
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
        	//usb_pd_set_state(PE_SRC_Disabled,enter_state);
            hal_tcpc_set_pd_rx(0,EN_HARD_RESET,false);
			break;
	}
}

void dpdm_src_afc_handle(void)
{
	#define AFC_CMD_RESET                       (0x01)                  //afc 5v
	#define AFC_CMD_5V                          (0x08)                  //afc 5v
	#define AFC_CMD_9V                          (0x46)                  //afc 9v
	#define AFC_CMD_12V                         (0x79)                  //afc 12v


	switch(DPDM->AFC_RX_0.BITS.RX_BUFFER_0)
	{
		case AFC_CMD_RESET:
			scp_vout = 5000;
			break;
		case AFC_CMD_5V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_5V << 8) | 0x01);
			scp_vout = 5000;
			break;
		case AFC_CMD_9V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_9V << 8) | 0x01);
			scp_vout = 9000;
			break;
		case AFC_CMD_12V:
			DPDM->AFC_TX_0.WORD =( (AFC_CMD_12V << 8) | 0x01);
			scp_vout = 12000;
			break;
	}

	scp_iout = 3000;
	osal_set_event(USB_DPDM_TASK,DPDM_EVT_AFC_SCP_OUT);
}


void dpdm_src_scp_handle(void)
{
	scp_packet.words[0] = DPDM->AFC_RX_0.WORD;
	scp_packet.words[1] = DPDM->AFC_RX_1.WORD;
	scp_packet.words[2] = DPDM->AFC_RX_2.WORD;


    switch(scp_packet.bytes.msg_0)
    {
		case CMD_SINGLE_READ:
			fcp_single_read_handle();
			break;
		case CMD_SINGLE_WRITE:
			fcp_single_write_handle();
			break;
		case CMD_MULTI_READ:
			fcp_multi_read_handle();
			break;
		case CMD_MULTI_WRITE:
			fcp_multi_write_handle();
			break;
    }
}


