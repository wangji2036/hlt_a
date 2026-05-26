#include "regdef.h"
#include "ufcs.h"
#include "dpdm.h"
#include "delay.h"
#include "printk.h"
#include "tcpm.h"
#include "port_manager.h"
#include "config.h"

#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)

uint8_t ufcs_rx_buffer[64];
uint8_t ufcs_tx_buffer[64];

uint8_t ufcs_msg_id = 0;
uint8_t ufcs_tx_len = 0;
uint8_t ufcs_rx_msgid = 0;
uint8_t ufcs_tx_index = 0;

void dpdm_ufcs_init(void)
{
	DPDM_UFCS->VERSION_MASK.BITS.RX_FULL_FLAG_MASK = 0;
	DPDM_UFCS->VERSION_MASK.BITS.TX_EMPETY_FLAG_MASK = 0;
	//DPDM_UFCS->VERSION_MASK.BITS.NACK_RECEIVED_FLAG_MASK = 0;
	DPDM_UFCS->VERSION_MASK.BITS.DATA_READY_MASK = 0;
	DPDM_UFCS->VERSION_MASK.BITS.HARD_RESET_MASK = 0;
	//DPDM_UFCS->VERSION_MASK.BITS.UFCS_HANDSHAKE_SUCC_MASK = 0;
	//DPDM_UFCS->VERSION_MASK.BITS.SENT_PACKET_COMPLETE_MASK = 0;
	//DPDM_UFCS->VERSION_MASK.BITS.ACK_RECEIVE_TIMEOUT_MASK = 0;
	DPDM_UFCS->SOURCE_CTRL.BITS.ACK_DEV_ADDR = 0x01;

	ufcs_msg_id = 0;
}

void dpdm_ufcs_deinit(void)
{
	DPDM_UFCS->VERSION_MASK.BITS.RX_FULL_FLAG_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.TX_EMPETY_FLAG_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.NACK_RECEIVED_FLAG_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.DATA_BYTE_TMOUT_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.HARD_RESET_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.UFCS_HANDSHAKE_SUCC_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.SENT_PACKET_COMPLETE_MASK = 1;
	DPDM_UFCS->VERSION_MASK.BITS.ACK_RECEIVE_TIMEOUT_MASK = 1;
}

void ufcs_send_msg(uint8_t ufcs_tx_len)
{
	ufcs_tx_index = 0;
	ufcs_msg_id++;

	DPDM_UFCS->TX_BUFFER.WORD = *((uint32_t *)&ufcs_tx_buffer[ufcs_tx_index]);
	DPDM_UFCS->TX_LENGTH.WORD = ufcs_tx_len;
	DPDM_UFCS->SOURCE_CTRL.BITS.SND_CMD = 1;

	ufcs_tx_index += 4;
}


void ufcs_send_ctrl_msg(uint8_t msg_type)
{
    uint16_t header;
    header = UFCS_HEADER(UFCS_CTRL_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
    ufcs_tx_buffer[0] = header >> 8;
    ufcs_tx_buffer[1] = header;
    ufcs_tx_buffer[2] = msg_type;
	ufcs_tx_len = 3;
	ufcs_send_msg(ufcs_tx_len);
}

void ufcs_exit_handle(void)
{
	hal_tcpc_pd_set_bus_iv(0,5000,3000,0,10);
	DPDM->SOURCE_CTRL.BITS.SOFT_RESET = 1;
	lib_printk("UFCS EXIT\n");
}

void ufcs_psread_handle(void)
{
	ufcs_send_ctrl_msg(UFCS_POWER_READY);
}


void ufcs_ctrl_handle(uint8_t msg_cmd)
{

	uint16_t header = 0;

    switch(msg_cmd)
    {
        case UFCS_PING:

            break;
        case UFCS_ACK:
            break;

        case UFCS_ACCEPT:
            break;

        case UFCS_SOFT_RESET:
            break;

        case UFCS_GET_OUTPUT_CAP:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_OUTPUT_CAPS;
        	ufcs_tx_buffer[3] = 0x08;
        	ufcs_tx_buffer[4] = 0x11;
        	ufcs_tx_buffer[5] = (uint8_t)((CONFIG_UFCS_MAX_VOLTAGE / 10) >>8);
        	ufcs_tx_buffer[6] = (uint8_t)(CONFIG_UFCS_MAX_VOLTAGE / 10);
        	ufcs_tx_buffer[7] = (uint8_t)((CONFIG_UFCS_MIN_VOLTAGE / 10) >>8);
        	ufcs_tx_buffer[8] = (uint8_t)(CONFIG_UFCS_MIN_VOLTAGE / 10);
        	ufcs_tx_buffer[9] = (uint8_t)((CONFIG_UFCS_MAX_CURRENT / 10) >>8);
        	ufcs_tx_buffer[10] = (uint8_t)(CONFIG_UFCS_MAX_CURRENT / 10);
        	ufcs_tx_buffer[11] = 0x1e;
        	ufcs_tx_len = 12;
        	ufcs_send_msg(ufcs_tx_len);
            break;

        case UFCS_GET_SOURCEINFO:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_SOURCEINFO;
            ufcs_tx_buffer[3] = 0x08;
            ufcs_tx_buffer[4] = 0x00;
            ufcs_tx_buffer[5] = 0x00;
            ufcs_tx_buffer[6] = 0x00;
            ufcs_tx_buffer[7] = 0x00;
            ufcs_tx_buffer[8] = (g_buckboost.adc_vbus / 10) >> 8;
            ufcs_tx_buffer[9] = (uint8_t)(g_buckboost.adc_vbus / 10);
            ufcs_tx_buffer[10] = ((-g_buckboost.adc_ibus) / 10) >> 8;;
            ufcs_tx_buffer[11] = (uint8_t)((-g_buckboost.adc_ibus) / 10);
        	ufcs_tx_len = 12;
        	ufcs_send_msg(ufcs_tx_len);
            break;
        case UFCS_GET_DEVICEINFO:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_DEVICE_INFO;
            ufcs_tx_buffer[3] = 0x08;
            ufcs_tx_buffer[4] = 0x00;
            ufcs_tx_buffer[5] = 0x00;
            ufcs_tx_buffer[6] = 0x00;
            ufcs_tx_buffer[7] = 0x00;
            ufcs_tx_buffer[8] = 0x00;
            ufcs_tx_buffer[9] = 0x00;
            ufcs_tx_buffer[10] = 0x00;
            ufcs_tx_buffer[11] = 0x00;
        	ufcs_tx_len = 12;
        	ufcs_send_msg(ufcs_tx_len);
            break;

        case UFCS_GET_ERRINFO:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_ERROR_INFO;
            ufcs_tx_buffer[3] = 0x04;
            ufcs_tx_buffer[4] = 0x00;
            ufcs_tx_buffer[5] = 0x00;
            ufcs_tx_buffer[6] = 0x00;
            ufcs_tx_buffer[7] = 0x00;
        	ufcs_tx_len = 8;
        	ufcs_send_msg(ufcs_tx_len);
            break;
        case UFCS_DETECT_CABLEINFO:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_REFUSE;
            ufcs_tx_buffer[3] = 0x04;
            ufcs_tx_buffer[4] = (ufcs_rx_msgid) & 0xf;
            ufcs_tx_buffer[5] = 0;
            ufcs_tx_buffer[6] = 0;
            ufcs_tx_buffer[7] = 0;
        	ufcs_tx_len = 8;
        	ufcs_send_msg(ufcs_tx_len);
            break;

        case UFCS_START_CABLEDET:
            break;

        case UFCS_END_CABLEDET:
            break;
        case UFCS_EXIT_MODE:
        	ufcs_exit_handle();
            break;

        default:  // 无效命令
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_REFUSE;
            ufcs_tx_buffer[3] = 0x04;
            ufcs_tx_buffer[4] = (ufcs_rx_msgid) & 0xf;
            ufcs_tx_buffer[5] = 0;
            ufcs_tx_buffer[6] = 0;
            ufcs_tx_buffer[7] = 0;
        	ufcs_tx_len = 8;
        	ufcs_send_msg(ufcs_tx_len);
            break;
    }
}

void ufcs_data_handle(uint8_t msg_cmd)
{
	uint16_t supply_vlotage = 0;
	uint16_t supply_current = 0;

	uint16_t header = 0;
    switch(msg_cmd)
    {
        case UFCS_OUTPUT_CAPS:
            break;

        case UFCS_RESQT:  //
            supply_vlotage = ((ufcs_rx_buffer[8] << 8)  | ufcs_rx_buffer[9]) * 10;
            supply_current = ((ufcs_rx_buffer[10] << 8) | ufcs_rx_buffer[11]) * 10;
            lib_printk("UFSC V=%d I=%d\n",supply_vlotage,supply_current);
            if(supply_vlotage <= CONFIG_UFCS_MAX_VOLTAGE && supply_vlotage >= CONFIG_UFCS_MIN_VOLTAGE)
            {
                ufcs_send_ctrl_msg(UFCS_ACCEPT);
                hal_tcpc_pd_set_bus_iv(PORT0_INDEX,supply_vlotage,supply_current,20,0);
                osal_start_timerEx(DPDM_SINK_TIMER, 200, 0, USB_DPDM_TASK, DPDM_EVT_UFCS_PSREADY);
            }
            else
            {
				header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
				ufcs_tx_buffer[0] = header >> 8;
				ufcs_tx_buffer[1] = header;
				ufcs_tx_buffer[2] = UFCS_REFUSE;
				ufcs_tx_buffer[3] = 0x04;
				ufcs_tx_buffer[4] = (ufcs_rx_msgid) & 0xf;
				ufcs_tx_buffer[5] = UFCS_DATA_MSG;
				ufcs_tx_buffer[6] = UFCS_RESQT;
				ufcs_tx_buffer[7] = REFUSE_REASON_PowerOutRange;
				ufcs_tx_len = 8;
				ufcs_send_msg(ufcs_tx_len);
            }
            break;


        case UFCS_CONFIG_WATCHDOG:
        	ufcs_send_ctrl_msg(UFCS_ACCEPT);
            break;

        case UFCS_VERIFY_REQUEST:
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_REFUSE;
            ufcs_tx_buffer[3] = 0x04;
            ufcs_tx_buffer[4] = (ufcs_rx_msgid) & 0xf;
            ufcs_tx_buffer[5] = UFCS_DATA_MSG;
            ufcs_tx_buffer[6] = UFCS_VERIFY_REQUEST;
            ufcs_tx_buffer[7] = REFUSE_REASON_Other;
        	ufcs_tx_len = 8;
        	ufcs_send_msg(ufcs_tx_len);
            break;

        default:  // 无效命令
        	header = UFCS_HEADER(UFCS_DATA_MSG,UFCS_VSERION,(ufcs_msg_id&0xf),UFCS_CHG_DEV);
        	ufcs_tx_buffer[0] = header >> 8;
        	ufcs_tx_buffer[1] = header;
        	ufcs_tx_buffer[2] = UFCS_REFUSE;
            ufcs_tx_buffer[3] = 0x04;
            ufcs_tx_buffer[4] = (ufcs_rx_msgid) & 0xf;
            ufcs_tx_buffer[5] = UFCS_DATA_MSG;
            ufcs_tx_buffer[6] = msg_cmd;
            ufcs_tx_buffer[7] = REFUSE_REASON_Other;
        	ufcs_tx_len = 8;
        	ufcs_send_msg(ufcs_tx_len);
            break;
    }
}

uint8_t ufcs_rx_cnt = 0;



void ufcs_rx_packet_handle(void)
{
    uint8_t msg_type = ufcs_rx_buffer[1] & 0x7 ;
    uint8_t msg_cmd = ufcs_rx_buffer[2];
    ufcs_rx_msgid = (ufcs_rx_buffer[0] >> 1) & 0x0F;
	lib_printk("ufcs [%d] =",ufcs_rx_cnt);

	for(uint8_t i= 0; i< ufcs_rx_cnt;i++)
	{
		lib_printk("0x%x ",ufcs_rx_buffer[i]);
	}

	lib_printk("\n");

	if(msg_type == 0)
		ufcs_ctrl_handle(msg_cmd);
	else if(msg_type == 1)
		ufcs_data_handle(msg_cmd);



}

uint8_t ufcs_rx_index = 0;

void __attribute__((isr)) UFCS_IRQHandler(void)
{

	uint32_t int_mask = BIT(4) | BIT(8) | BIT(18) | BIT(19);
	uint32_t ufcs_int = DPDM_UFCS->INT_FLAG.WORD;

    do
    {
    	if(ufcs_int & BIT(19))
    	{
    		DPDM_UFCS->INT_FLAG.BITS.RX_FULL_FLAG = 1;
    		uint32_t date = DPDM_UFCS->RX_BUFFER.WORD;
    		ufcs_rx_buffer[ufcs_rx_index] = date >> 0;
    		ufcs_rx_buffer[ufcs_rx_index + 1] = date >> 8;
    		ufcs_rx_buffer[ufcs_rx_index + 2] = date >> 16;
    		ufcs_rx_buffer[ufcs_rx_index + 3] = date >> 24;
    		ufcs_rx_index += 4;
    	}

    	if(ufcs_int & BIT(18))
    	{
    		DPDM_UFCS->INT_FLAG.BITS.TX_EMPETY_FLAG = 1;
    		DPDM_UFCS->TX_BUFFER.WORD = *((uint32_t *)&ufcs_tx_buffer[ufcs_tx_index]);
    		ufcs_tx_index += 4;
    	}

    	if(ufcs_int & BIT(4))
    	{
    		DPDM_UFCS->INT_FLAG.BITS.DATA_READY_FLAG = 1;
    		uint32_t date = DPDM_UFCS->RX_BUFFER.WORD;
    		ufcs_rx_buffer[ufcs_rx_index] = date >> 0;
    		ufcs_rx_buffer[ufcs_rx_index + 1] = date >> 8;
    		ufcs_rx_buffer[ufcs_rx_index + 2] = date >> 16;
    		ufcs_rx_buffer[ufcs_rx_index + 3] = date >> 24;
    		ufcs_rx_index += 4;
    		ufcs_rx_cnt = DPDM_UFCS->RX_LENGTH.WORD;
    		ufcs_rx_index = 0;
    		osal_set_event(USB_DPDM_TASK,DPDM_EVT_UFCS_RX_PACKET);
    	}

    	if(ufcs_int & BIT(8))
    	{
    		DPDM_UFCS->INT_FLAG.BITS.HARD_RESET_FLAG = 1;
    		osal_set_event(USB_DPDM_TASK,DPDM_EVT_UFCS_RX_RESET);
    	}

    	ufcs_int = DPDM_UFCS->INT_FLAG.WORD;

    }while(int_mask & ufcs_int);
}
#endif






