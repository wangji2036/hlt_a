#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "fsk.h"
#include "algo.h"
#include "pkt_type.h"
#include "wpc_xfer.h"
#include "fm1210.h"
#include "debug.h"
#include "pfod.h"

static uint8_t cnt_cep0 = 0;
static uint8_t cnt_cloak_pkt = 0;

#define PU_CERT_LEN_OFS	    (2 + 32 + 328)
#define PU_CERT_LEN_MAX     (400)
#define CERT_CHAIN_LEN      (PU_CERT_LEN_OFS + PU_CERT_LEN_MAX)
#define DIGEST_LENGTH       (32)
#define CHALL_LENGTH        (64)
#define CRC_INITIAL_VALUE   (0xFFFF)

uint8_t array_digest[1 + DIGEST_LENGTH] =
{
	0x11,
};

uint8_t cert_chain[CERT_CHAIN_LEN] =
{
	0x02, 0xAA, //2 bytes length,  The length is the total number of bytes in the Certificate Chain including the Length field.

	0xA1, 0x75, 0x9E, 0xCC, 0xA0, 0xBE, 0x3B, 0x85, 0x01, 0x18, 0x18, 0x3E, 0xD6, 0xCD, 0xD6, 0xD4, //32 bytes, Root Certificate Hash
	0xA5, 0xDB, 0x7D, 0x83, 0xE6, 0xFD, 0x0E, 0x6F, 0x47, 0x5C, 0xE4, 0xBB, 0x6E, 0xA0, 0x14, 0x24,

	0x30, 0x82, 0x01, 0x44, 0x30, 0x81, 0xEB, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x7F, 0x14, //328 bytes, Manufacturer CA Certificate
	0x7F, 0x22, 0xC4, 0x7F, 0x75, 0x6D, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04,
	0x03, 0x02, 0x30, 0x11, 0x31, 0x0F, 0x30, 0x0D, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x06, 0x57,
	0x50, 0x43, 0x43, 0x41, 0x31, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x33, 0x30, 0x36, 0x32, 0x30, 0x30,
	0x39, 0x32, 0x30, 0x30, 0x39, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 0x33, 0x31,
	0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x12, 0x31, 0x10, 0x30, 0x0E, 0x06, 0x03, 0x55,
	0x04, 0x03, 0x0C, 0x07, 0x30, 0x30, 0x35, 0x43, 0x2D, 0x46, 0x45, 0x30, 0x59, 0x30, 0x13, 0x06,
	0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03,
	0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x79, 0x14, 0x43, 0x12, 0x1D, 0x7C, 0x0A, 0x2D, 0x7E, 0xA2,
	0xF7, 0x79, 0x32, 0x53, 0x3A, 0xA3, 0xC8, 0x00, 0xBA, 0x1C, 0x93, 0x1D, 0x4F, 0xB1, 0x97, 0x15,
	0x1D, 0xCF, 0xDB, 0xF4, 0xBF, 0xC7, 0x8C, 0x11, 0xCF, 0xD0, 0xAD, 0xEB, 0xF3, 0x5D, 0xB6, 0x43,
	0xD5, 0x85, 0x91, 0x9C, 0x50, 0x64, 0x63, 0x37, 0x6D, 0xA6, 0x63, 0x0D, 0x5D, 0x13, 0x16, 0xD9,
	0x14, 0xCD, 0x49, 0xF0, 0xCF, 0xD8, 0xA3, 0x2A, 0x30, 0x28, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1D,
	0x13, 0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01, 0x00, 0x30, 0x12,
	0x06, 0x05, 0x67, 0x81, 0x14, 0x01, 0x01, 0x01, 0x01, 0xFF, 0x04, 0x06, 0x04, 0x04, 0x00, 0x00,
	0x00, 0x01, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48,
	0x00, 0x30, 0x45, 0x02, 0x20, 0x6F, 0xA9, 0x9F, 0x0A, 0x95, 0x88, 0x58, 0xCE, 0x91, 0x99, 0xA3,
	0x58, 0xBD, 0x86, 0xD1, 0x76, 0x95, 0xE5, 0x7D, 0x9B, 0x74, 0x89, 0xEA, 0x38, 0xD5, 0x13, 0xBB,
	0x55, 0x28, 0x8D, 0xA3, 0xA4, 0x02, 0x21, 0x00, 0x92, 0x69, 0x78, 0x5E, 0xB1, 0xF1, 0x26, 0xFF,
	0x29, 0x11, 0x2C, 0x09, 0xF5, 0xF8, 0xD3, 0xC9, 0x31, 0xA8, 0xE6, 0xA8, 0xC7, 0x68, 0x40, 0x08,
	0x9F, 0x8B, 0x2B, 0xB2, 0x68, 0x34, 0xB6, 0xAD,

	//Product Unit Certificate
};

enum auth_header_t
{
	RSP_DIGESTS     = 0x11,
	RSP_CERTIFICATE = 0x12,
	RSP_CHALLENGE   = 0x13,
};

enum ds_status_t
{
	DS_STS_IDLE = 0,
	DS_STS_OPEN = 1,
	DS_STS_TRANS = 2,
	DS_STS_CLOSE = 3,
};


uint8_t need_atn_cnt;
uint8_t need_atn_evt;

uint8_t ds_incoming_status[4];
uint8_t ds_incoming_odd_even[4];
uint16_t adt_have_rcv_len;
uint16_t adt_need_rcv_len;
uint8_t  adt_rcv_buff[18];


uint8_t ds_outgoing_status[4];
uint8_t ds_outgoing_odd_even[4];
uint8_t auth_request_type;
uint8_t send_log_idx;
uint16_t cert_ofs; //GET_CERTIFICATE request's offset
uint16_t cert_len; //GET_CERTIFICATE request's length
uint16_t cert_left; //the left cert data need to trans
uint16_t cert_chain0_len; //the total chain0 length, over 600 bytes

uint16_t ds_send_crc;
uint16_t adt_need_send_len;
uint16_t adt_have_send_len;
uint8_t adt_last_send_len;
uint8_t  *auth_send_ptr = NULL;
uint8_t  adt_send_buff[130]; //reserved 2 bytes

uint8_t array_chall[CHALL_LENGTH];

void auth_init(void)
{
	need_atn_cnt = 0;
	need_atn_evt = 0;

	for (int i=0; i<4; i++)
	{
		ds_incoming_status[i] = DS_STS_IDLE;
		ds_incoming_odd_even[i] = 0;
	}
	adt_have_rcv_len = 0;
	adt_need_rcv_len = 0;

	for (int i=0; i<4; i++)
	{
		ds_outgoing_status[i] = DS_STS_IDLE;
		ds_outgoing_odd_even[i] = 0;
	}
	auth_request_type = 0;
	send_log_idx = 0;
	cert_ofs = 0;
	cert_len = 0;
	cert_left = 0;
	cert_chain0_len = (cert_chain[0] << 8) | cert_chain[1];

	adt_need_send_len = 0;
	adt_have_send_len = 0;
}

static uint8_t is_bpp_xfer_phase_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x01 || hdr == 0x06 || hdr == 0x07 || hdr == 0x09 || hdr == 0x15 || hdr == 0x16 || hdr == 0x17 || hdr == 0x20 || hdr == 0x22 || hdr == 0x25 ||
		hdr == 0x26 || hdr == 0x27 || hdr == 0x31 || hdr == 0x36 || hdr == 0x37 || hdr == 0x46 || hdr == 0x47 || hdr == 0x51 || hdr == 0x54 || hdr == 0x55 ||
		hdr == 0x56 || hdr == 0x57 || hdr == 0x66 || hdr == 0x67 || hdr == 0x71 || hdr == 0x76 || hdr == 0x77 || hdr == 0x81)
    {
    	return 1;
    }
	return 0;
}

void bpp_epp_prop_pkt_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (com_ask->hdr)
	{
		case 0x18:
			break;
		default:
			break;
	}
}

static uint8_t is_bpp_epp_prop_pkt(uint8_t hdr)
{
    if (hdr == 0x18 || hdr == 0x19 || hdr == 0x28 || hdr == 0x29 || hdr == 0x38 || hdr == 0x48 || hdr == 0x58 ||
    	hdr == 0x68 || hdr == 0x78 || hdr == 0x84 || hdr == 0xA4 || hdr == 0xC4 || hdr == 0xE2)
    {
    	return 1;
    }
	return 0;
}

//static uint8_t is_epp_adt_pkt(uint8_t hdr)
//{
//    if (hdr == 0x16 || hdr == 0x17 || hdr == 0x26 || hdr == 0x27 || hdr == 0x36 || hdr == 0x37 || hdr == 0x46 ||
//    	hdr == 0x47 || hdr == 0x56 || hdr == 0x57 || hdr == 0x66 || hdr == 0x67 || hdr == 0x76 || hdr == 0x77)
//    {
//    	return 1;
//    }
//	return 0;
//}

static uint8_t is_mpp_sadt_pkt(uint8_t hdr)
{
    if (hdr == 0x26 || hdr == 0x27 || hdr == 0x36 || hdr == 0x37 || hdr == 0x46 || hdr == 0x47 ||
    	hdr == 0x56 || hdr == 0x57 || hdr == 0x66 || hdr == 0x67 || hdr == 0x76 || hdr == 0x77)
    {
    	return 1;
    }
	return 0;
}

static uint8_t is_cloak_phase_illegal_pkt(uint8_t hdr)
{
	if (hdr == 0x18 || hdr == 0x28 || hdr == 0x58)
	{
		return 0;
	}
	return 1;
}

//void wpc_xfer_ptx_power_update(void)
//{
//	gd->isns_avg = hal_badc_meas(_BADC_CH_PD6_ADC3);
//	gd->vpwr_avg = hal_badc_meas(_BADC_CH_PD0_ADC8);
//	gd->tx_power = gd->isns_avg * gd->vpwr_avg / 1000;
//}

void wpc_bpp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (com_ask->hdr)
	{
		case WPC_PRx_PKT_TYP_CE_03:
			gd->rx_infos.cep_val = com_ask->msg.cep.ce_value;
			printk("epp ce= %d\n",gd->rx_infos.cep_val);
			if (gd->rx_power > 6500)
			{
				gd->rx_infos.mpp_restricted_power_limit = 1;
			}
			else if (gd->rx_power < 6200)
			{
				gd->rx_infos.mpp_restricted_power_limit = 0;
			}

			if (gd->rx_infos.mpp_restricted_power_limit && gd->rx_infos.cep_val > 0)
			{
				//gd->rx_infos.cep_val = 0;
				printk("#");
			}

			osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
			osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
			break;
		case WPC_PRx_PKT_TYP_RP8_04:
			gd->rx_power = (com_ask->msg.rp8.rp_value * (uint32_t)gd->rx_infos.max_power * 1000) >> 8;
			osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
			osal_start_timerEx(WPC_NEXT_TIMER, 0, 0, WPC_TASK, WPC_EVT_PFOD);
			break;
		case WPC_PRx_PKT_TYP_CHS_05:
			gd->rx_infos.chr_status = com_ask->msg.chs.chs_value;
			printk(" %%: [charge status %d%%]", gd->rx_infos.chr_status);
			break;
		default:
			if (is_bpp_epp_prop_pkt(com_ask->hdr))
			{
				bpp_epp_prop_pkt_process(com_ask);
				break;
			}

			if (is_bpp_xfer_phase_illegal_pkt(com_ask->hdr))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT);
				goto __XFER_PHASE_ERR__;
			}
			break;
	}

__XFER_PHASE_ERR__:
	return;
}

__attribute__((weak)) void wpc_epp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask)
{

}

void mpp_report_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	uint8_t res;
	if (mpp_ask->msg.report_pla.select == 1)//TODO: add fod func here, add a timer print log to avoid the FSK window
	{
		gd->rx_power = mpp_ask->msg.report_pla.rcvd_power_msb << 8 | mpp_ask->msg.report_pla.rcvd_power_lsb;
		gd->rx_prect = gd->rx_infos.pla_prect = mpp_ask->msg.report_pla.rect_power_msb << 8 | mpp_ask->msg.report_pla.rect_power_lsb;
		osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);//RPP timerout

		gd->tx_infos.fsk_done_event |= 8;//set reported event print long log

		res = pfod_mpla();
		printk("\r\n ---> res-> %d", res);
		if (res == 0)
		{
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
		}
		else if (res == 1)
		{
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);

			gd->rx_infos.cep_val = -5;
			osal_start_timerEx(WPC_NEXT_TIMER, T_XCE_RESP_TO + gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
		}
		else
		{
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ATN);
		}

	}
	else if (mpp_ask->msg.report_pla.select == 0)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
	}
	else
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
	}
}

void mpp_pla2_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	uint8_t res;

	gd->rx_infos.pla_type = 2;
	//gd->rx_infos.rpp_tick++;

	gd->rx_power = mpp_ask->msg.pla2.rcvd_power_msb << 8 | mpp_ask->msg.pla2.rcvd_power_lsb;
	gd->rx_prect = gd->rx_infos.pla_prect = mpp_ask->msg.report_pla.rect_power_msb << 8 | mpp_ask->msg.report_pla.rect_power_lsb;
	gd->rx_infos.pla_vrect = (mpp_ask->msg.pla2.vrect_msb << 8) + mpp_ask->msg.pla2.vrect_lsb;
	gd->rx_infos.pla_irect = (mpp_ask->msg.pla2.irect_h << 8) + mpp_ask->msg.pla2.irect_l;
	osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);//RPP timerout

	gd->tx_infos.fsk_done_event |= 8;//set reported event print long log

	if (gd->dploss_cal.success == 1)
		res = pfod_dploss();
	else
		res = pfod_mpla();

	if (res == 0)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
	}
	else if (res == 1)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);

		gd->rx_infos.cep_val = -5;
		osal_start_timerEx(WPC_NEXT_TIMER, T_XCE_RESP_TO + gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
	}
	else
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ATN);
	}
}


void mpp_dsr_poll_handler(void)
{
	enum { PTX_SADC_RST_ALL_STREAMS = 0, PTX_SADC_RST_SPEC_STREAM = 1, PTX_SADC_CLOSE_AND_ABORT = 2, PTX_SADC_CLOSE_COMPLETE = 3, PTX_SADC_OPEN_TRANSPORT = 4, };

	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	uint8_t tmp_len;

	need_atn_cnt = 0;

	if (gd->tx_infos.need_renego_cap == 1)
	{
		gd->tx_infos.need_renego_cap = 0;
		//gd->power_limit_sts.fop_flag = 0;

		fsk_pkt.mpp_fsk.ecap.hdr_8F = MPP_PTx_PKT_TYP_ECAP_8F;
		fsk_pkt.mpp_fsk.ecap.selector = 0x01;
		fsk_pkt.mpp_fsk.ecap.potential_power_bit0_1 = (gd->tx_infos.max_cap>>8)&0b11;//100mW/bit
		fsk_pkt.mpp_fsk.ecap.potential_power_bit2_9 = gd->tx_infos.max_cap&0xFF;
		fsk_pkt.mpp_fsk.ecap.nego_power_bit0_1 = (gd->tx_infos.nego_cap>>8)&0b11;
		fsk_pkt.mpp_fsk.ecap.nego_power_bit2_9 =  gd->tx_infos.nego_cap&0xFF;
		fsk_pkt.mpp_fsk.ecap.pow_limit_reason = gd->tx_infos.power_limit_reason;//fo presence
		fsk_pkt.mpp_fsk.ecap.concurrent_data_stream = 2;//TODO:
		fsk_pkt.mpp_fsk.ecap.buffer_size = 3;
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
	}
	else if (need_atn_evt == 1)
	{
		switch (ds_outgoing_status[1])
		{
			case DS_STS_IDLE:
				ds_outgoing_status[1] = DS_STS_OPEN;
			case DS_STS_OPEN:
//				fsk_pkt.mpp_fsk.data[0] = 0x00;
//				fsk_pkt.mpp_fsk.data[1] = 0x00;
//				fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

				fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
				fsk_pkt.mpp_fsk.sadc.request = PTX_SADC_OPEN_TRANSPORT;
				fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
				fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
				fsk_pkt.mpp_fsk.sadc.request = PTX_SADC_OPEN_TRANSPORT;
				fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
				if (auth_request_type == 0x19)
				{
					auth_send_ptr = &adt_send_buff[0];
					adt_send_buff[0] = RSP_DIGESTS;
					for (int i=0; i<DIGEST_LENGTH+1; i++)
					{
						adt_send_buff[1 + i] = array_digest[i];
					}
					if (adt_rcv_buff[1] & 0x01)
					{
						adt_need_send_len = DIGEST_LENGTH + 2;
					}
					else
					{
						adt_need_send_len = 2;
					}
					ds_send_crc = crc16_ccitt(adt_send_buff, adt_need_send_len, CRC_INITIAL_VALUE);
					fsk_pkt.mpp_fsk.sadc.param_msb = (adt_need_send_len >> 8) & 0xFF;
					fsk_pkt.mpp_fsk.sadc.param_lsb = (adt_need_send_len >> 0) & 0xFF;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
				}
				else if (auth_request_type == 0x1A)
				{
					auth_send_ptr = &adt_send_buff[0];
					adt_send_buff[0] = RSP_CERTIFICATE;
					for (int i=0; i<cert_len; i++)
					{
						adt_send_buff[1 + i] = cert_chain[cert_ofs + i];
					}
					adt_need_send_len = cert_len + 1;
					ds_send_crc = crc16_ccitt(adt_send_buff, adt_need_send_len, CRC_INITIAL_VALUE);
					fsk_pkt.mpp_fsk.sadc.param_msb = (adt_need_send_len >> 8) & 0xFF;
					fsk_pkt.mpp_fsk.sadc.param_lsb = (adt_need_send_len >> 0) & 0xFF;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
				}
				else if (auth_request_type == 0x1B)
				{
					auth_send_ptr = &adt_send_buff[0];
					adt_send_buff[0] = RSP_CHALLENGE;
					adt_send_buff[1] = 0x11;
					adt_send_buff[2] = array_digest[32];
					for (int i=0; i<CHALL_LENGTH+2; i++)
					{
						adt_send_buff[3+i] = array_chall[i];
					}
					adt_need_send_len = CHALL_LENGTH + 3;
					ds_send_crc = crc16_ccitt(adt_send_buff, adt_need_send_len, CRC_INITIAL_VALUE);
					fsk_pkt.mpp_fsk.sadc.param_msb = (adt_need_send_len >> 8) & 0xFF;
					fsk_pkt.mpp_fsk.sadc.param_lsb = (adt_need_send_len >> 0) & 0xFF;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
				}
				ds_outgoing_odd_even[1] = 0;
				break;
			case DS_STS_TRANS:
				tmp_len = adt_need_send_len - adt_have_send_len;
				if (++tmp_len > 7) tmp_len = 7;
				if (0 != ds_outgoing_odd_even[1])
				{
					fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x07;
				}
				else
				{
					fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x06;
				}

				fsk_pkt.mpp_fsk.sadt.stream_num = 0x01;

				for(int i=0; i <(tmp_len-1); i++)
				{
					fsk_pkt.mpp_fsk.sadt.data[i] = *(auth_send_ptr + adt_have_send_len + i);
				}

				fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

				printk(" [%2d,%3d]", send_log_idx, adt_have_send_len);
				//PTx 应该重发上一个data stream
				break;
			case DS_STS_CLOSE:
				fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
				fsk_pkt.mpp_fsk.sadc.request = 0x03;
				fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
				fsk_pkt.mpp_fsk.sadc.param_msb = (ds_send_crc >> 8) & 0xFF;
				fsk_pkt.mpp_fsk.sadc.param_lsb = (ds_send_crc >> 0) & 0xFF;
				fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
				break;
			default:
				break;
		}
	}
	else if (gd->tx_infos.power_mode_trans_atn == 1)
	{
		gd->tx_infos.power_mode_trans_atn = 0;
#if MPP_25W_POWER_MODE_TRANS_W_EPTR
		fsk_pkt.mpp_fsk.data[0] = 0x0A;
		fsk_pkt.mpp_fsk.data[1] = 0x00;
#elif MPP_25W_POWER_MODE_TRANS_W_CLOAK
		fsk_pkt.mpp_fsk.data[0] = 0x1E;
		fsk_pkt.mpp_fsk.data[1] = 0x05;//cloak reason: power mode change
#endif
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

	}
#if MPP_25W_POWER_MODE_TRANS_W_CLOAK
	else if (gd->tx_infos.power_mode_trans_cloak == 1)
	{
		gd->tx_infos.power_mode_trans_cloak = 0;
		fsk_pkt.mpp_fsk.mss.hdr_0x23 = MPP_PTx_PKT_TYP_MSS_23;
		fsk_pkt.mpp_fsk.mss.error_code = 0;
		fsk_pkt.mpp_fsk.mss.status = 0;//success	
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
	}
#endif
}

void mpp_dsr_pkt_handler(struct com_prx_ask_pkt_t *com_ask)
{
	enum { DSR_nak = 0x00, DSR_poll = 0x33, DSR_nd = 0x55, DSR_ack = 0xFF, };

	switch (com_ask->msg.dsr.type)
	{
		case DSR_nak:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case DSR_poll:
			mpp_dsr_poll_handler();
			break;
		case DSR_nd:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case DSR_ack:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		default:
			break;
	}
}

void get_auth_ic_data(void)
{
	need_atn_cnt = 100;
}

void mpp_sdsr_pkt_handler(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	enum { SDSR_ACK = 0, SDSR_UNEXPECTED = 1, SDSR_ERR_BUSY = 2, SDSR_ERR_CRC = 3, };
	enum { PTX_SADC_RST_ALL_STREAMS = 0, PTX_SADC_RST_SPEC_STREAM = 1, PTX_SADC_CLOSE_AND_ABORT = 2, PTX_SADC_CLOSE_COMPLETE = 3, PTX_SADC_OPEN_TRANSPORT = 4, };

	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	uint8_t tmp_len;

	switch (ds_outgoing_status[1])
	{
		case DS_STS_IDLE:
			fsk_pkt.mpp_fsk.data[0] = 0x00;
			fsk_pkt.mpp_fsk.data[1] = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case DS_STS_OPEN:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
					ds_outgoing_status[1] = DS_STS_TRANS;

					adt_have_send_len = 0;
					adt_last_send_len = 0;
					send_log_idx = 0;

					adt_have_send_len = adt_have_send_len +adt_last_send_len;

					tmp_len = adt_need_send_len - adt_have_send_len;
					if (++tmp_len > 7) tmp_len = 7;

					if (0 != ds_outgoing_odd_even[1])
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x07;
					}
					else
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x06;
					}

					fsk_pkt.mpp_fsk.sadt.stream_num = 0x01;

					for(int i=0; i <(tmp_len-1); i++)
					{
						fsk_pkt.mpp_fsk.sadt.data[i] = *(auth_send_ptr + adt_have_send_len + i);
					}

					adt_last_send_len = tmp_len - 1;
//					adt_have_send_len = adt_have_send_len + tmp_len - 1;

					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

					printk(" [%2d,%3d]", ++send_log_idx, adt_have_send_len + adt_last_send_len);
					break;
				case SDSR_UNEXPECTED:
				case SDSR_ERR_BUSY:
					//PTx 应该重发上一个data stream
					fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
					fsk_pkt.mpp_fsk.sadc.request = PTX_SADC_OPEN_TRANSPORT;
					fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
					fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
					fsk_pkt.mpp_fsk.sadc.request = PTX_SADC_OPEN_TRANSPORT;
					fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
					fsk_pkt.mpp_fsk.sadc.param_msb = (adt_need_send_len >> 8) & 0xFF;
					fsk_pkt.mpp_fsk.sadc.param_lsb = (adt_need_send_len >> 0) & 0xFF;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

					ds_outgoing_odd_even[1] = 0;
					break;
				case SDSR_ERR_CRC:
					//PTx 应该重发整个data stream
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
					break;
				default:
					break;
			}
			break;
		case DS_STS_TRANS:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
					adt_have_send_len = adt_have_send_len + adt_last_send_len;

					if (adt_have_send_len >= adt_need_send_len)
					{
						ds_outgoing_status[1] = DS_STS_CLOSE;

						fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
						fsk_pkt.mpp_fsk.sadc.request = 0x03;
						fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
						fsk_pkt.mpp_fsk.sadc.param_msb = (ds_send_crc >> 8) & 0xFF;
						fsk_pkt.mpp_fsk.sadc.param_lsb = (ds_send_crc >> 0) & 0xFF;
						fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
						return;
					}

					ds_outgoing_odd_even[1] ^= 1;

					tmp_len = adt_need_send_len - adt_have_send_len;
					if (++tmp_len > 7) tmp_len = 7;
					if (0 != ds_outgoing_odd_even[1])
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x07;
					}
					else
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x06;
					}

					fsk_pkt.mpp_fsk.sadt.stream_num = 0x01;

					for(int i=0; i <(tmp_len-1); i++)
					{
						fsk_pkt.mpp_fsk.sadt.data[i] = *(auth_send_ptr + adt_have_send_len + i);
					}

					adt_last_send_len = tmp_len - 1;
//					adt_have_send_len = adt_have_send_len + tmp_len - 1;

					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

					printk(" [%2d,%3d]", ++send_log_idx, adt_have_send_len + adt_last_send_len);
					break;
				case SDSR_UNEXPECTED:
				case SDSR_ERR_BUSY:
					tmp_len = adt_need_send_len - adt_have_send_len;
					if (++tmp_len > 7) tmp_len = 7;
					if (0 != ds_outgoing_odd_even[1])
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x07;
					}
					else
					{
						fsk_pkt.mpp_fsk.sadt.hdr = tmp_len << 4 | 0x06;
					}

					fsk_pkt.mpp_fsk.sadt.stream_num = 0x01;

					for(int i=0; i <(tmp_len-1); i++)
					{
						fsk_pkt.mpp_fsk.sadt.data[i] = *(auth_send_ptr + adt_have_send_len + i);
					}

//					adt_have_send_len = adt_have_send_len + tmp_len - 1;

					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);

					printk(" [%2d,%3d]", send_log_idx, adt_have_send_len);
					//PTx 应该重发上一个data stream
					break;
				case SDSR_ERR_CRC:
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
					break;
				default:
					break;
			}
			break;
		case DS_STS_CLOSE:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
				{
					ds_outgoing_status[1] = DS_STS_IDLE;

					struct com_ptx_fsk_pkt_t fsk_pkt = { };
					fsk_pkt.com_fsk.null.hdr_00 = WPC_PTx_PKT_TYP_NULL_00;
					fsk_pkt.com_fsk.null.invalid_data = 0x00;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.com_fsk.data[0], wpc_msg_size_get(fsk_pkt.com_fsk.data[0]) + 1);
					break;
				}
				case SDSR_UNEXPECTED:
					//PTx 应该重发上一个data stream
//					break;
				case SDSR_ERR_BUSY:
					fsk_pkt.mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
					fsk_pkt.mpp_fsk.sadc.request = 0x03;
					fsk_pkt.mpp_fsk.sadc.stream_num = 0x01;
					fsk_pkt.mpp_fsk.sadc.param_msb = (ds_send_crc >> 8) & 0xFF;
					fsk_pkt.mpp_fsk.sadc.param_lsb = (ds_send_crc >> 0) & 0xFF;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
					break;
				case SDSR_ERR_CRC:
					//PTx 应该重发整个data stream
					if (auth_request_type == 0x19 || auth_request_type == 0x1A)
					{
						get_auth_ic_data();
					}
					else if (auth_request_type == 0x1B)
					{
						osal_start_timerEx(WPC_AUTH_TIMER, 10, 0, WPC_TASK, WPC_EVT_SE_IC_TBS_AUTH);
					}
					ds_outgoing_status[1] = DS_STS_IDLE;
					need_atn_evt = 1;//auth
					break;
				default:
					break;
			}
			break;
		default:
			fsk_pkt.mpp_fsk.data[0] = 0x00;
			fsk_pkt.mpp_fsk.data[1] = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
	}
}



void mpp_sadc_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	enum { SDSR_ACK = 0, SDSR_UNEXPECTED = 1, SDSR_ERR_BUSY = 2, SDSR_ERR_CRC = 3, };
	enum { SADC_RST_ALL_STREAMS = 0, SADC_RST_SPEC_STREAM = 1, SADC_CLOSE_AND_ABORT = 2, SADC_CLOSE_AND_CMPLT = 3, SADC_OPEN_DATA_TRANS = 4, };

	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	if (mpp_ask->msg.sadc.stream_num > 3)
	{
		fsk_pkt.mpp_fsk.data[0] = 0x00;
		fsk_pkt.mpp_fsk.data[1] = 0x00;
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
		return;
	}

	fsk_pkt.mpp_fsk.sdsr.hdr_3F = MPP_PTx_PKT_TYP_SDSR_3F;
	fsk_pkt.mpp_fsk.sdsr.selector = 0x01;
	fsk_pkt.mpp_fsk.sdsr.stream_num = mpp_ask->msg.sadc.stream_num;
	fsk_pkt.mpp_fsk.sdsr.type = SDSR_ACK;

	switch (mpp_ask->msg.sadc.request)
	{
		case SADC_RST_ALL_STREAMS:
		case SADC_RST_SPEC_STREAM:
		case SADC_CLOSE_AND_ABORT:
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_IDLE;
			break;
		case SADC_CLOSE_AND_CMPLT:
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_IDLE;
			if (adt_need_rcv_len == 0 || crc16_ccitt(adt_rcv_buff, adt_need_rcv_len, CRC_INITIAL_VALUE) != (mpp_ask->msg.sadc.param_msb << 8 | mpp_ask->msg.sadc.param_lsb))
			{
				fsk_pkt.mpp_fsk.sdsr.type = SDSR_ERR_CRC;
			}
			else
			{
				if ((auth_request_type >> 4) != 1)
				{
					fsk_pkt.mpp_fsk.data[0] = 0x17;
					fsk_pkt.mpp_fsk.data[1] = 0x02;
					fsk_pkt.mpp_fsk.data[2] = 0x01;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], 3);
					return;
				}
				else if (auth_request_type == 0x1B && adt_rcv_buff[1] != 0)
				{
					fsk_pkt.mpp_fsk.data[0] = 0x17;
					fsk_pkt.mpp_fsk.data[1] = 0x01;
					fsk_pkt.mpp_fsk.data[2] = 0x00;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], 3);
					return;
				}
//				else if (auth_request_type == 0x1A && (cert_chain0_len >= cert_ofs || cert_chain0_len < cert_ofs + cert_len))
				else if (auth_request_type == 0x1A && (/*cert_chain0_len >= cert_ofs || */cert_chain0_len < cert_ofs + cert_len))
				{
					printk("\r\n *** %d %d %d", cert_chain0_len, cert_ofs, cert_len);
					fsk_pkt.mpp_fsk.data[0] = 0x17;
					fsk_pkt.mpp_fsk.data[1] = 0x01;
					fsk_pkt.mpp_fsk.data[2] = 0x00;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], 3);
					return;
				}
				else if (mpp_ask->msg.sadc.stream_num == 1)
				{
					if (auth_request_type == 0x19 || auth_request_type == 0x1A)
					{
						get_auth_ic_data();
					}
					else if (auth_request_type == 0x1B)
					{
						osal_start_timerEx(WPC_AUTH_TIMER, 10, 0, WPC_TASK, WPC_EVT_SE_IC_TBS_AUTH);
					}
					ds_outgoing_status[1] = DS_STS_IDLE;
					need_atn_evt = 1;//auth
				}
			}
			break;
		case SADC_OPEN_DATA_TRANS:
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_OPEN;
			ds_incoming_odd_even[mpp_ask->msg.sadc.stream_num] = 0;
			adt_have_rcv_len = 0;
			adt_need_rcv_len = mpp_ask->msg.sadc.param_msb << 8 | mpp_ask->msg.sadc.param_lsb;
			auth_request_type = 0;
			break;
		default:
			fsk_pkt.mpp_fsk.sdsr.type = SDSR_UNEXPECTED;
			break;
	}

	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
}

void mpp_sadt_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	if (mpp_ask->msg.sadt.stream_num != 1)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
		return;
	}

	if (ds_incoming_status[mpp_ask->msg.sadt.stream_num] == DS_STS_IDLE)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
		return;
	}

	if (ds_incoming_status[mpp_ask->msg.sadt.stream_num] == DS_STS_OPEN)
	{
		ds_incoming_status[mpp_ask->msg.sadt.stream_num] = DS_STS_TRANS;
		auth_request_type = mpp_ask->msg.sadt.data[0];
	}

	if ((mpp_ask->hdr & 0x01) == ds_incoming_odd_even[mpp_ask->msg.sadt.stream_num])
	{
		ds_incoming_odd_even[mpp_ask->msg.sadt.stream_num] ^= 1;

		uint8_t len = (mpp_ask->hdr >> 4) - 1;

		if (adt_have_rcv_len + len > adt_need_rcv_len)
		{
			auth_request_type = 0;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			return;
		}

		for (int i=0; i<len; i++)
		{
			adt_rcv_buff[adt_have_rcv_len+i] = mpp_ask->msg.sadt.data[i];
		}
		adt_have_rcv_len += len;

		if (adt_have_rcv_len == adt_need_rcv_len)
		{
			switch (auth_request_type)
			{
				case 0x19:
					break;
				case 0x1A:
					cert_ofs = ((adt_rcv_buff[1] & 0xFF) >> 5) * 256 + adt_rcv_buff[2];
					cert_len = ((adt_rcv_buff[1] & 0x1C) >> 2) * 256 + adt_rcv_buff[3];

					if (cert_ofs >= 0x600)
					{
						cert_ofs = PU_CERT_LEN_OFS + cert_ofs - 0x600; //cali the read cert offset
					}

					if (cert_len == 0 && cert_chain0_len > cert_ofs)
					{
						cert_len = cert_chain0_len - cert_ofs;//cali the read cert length
					}

					if (cert_ofs + cert_len > CERT_CHAIN_LEN)
					{
						printk("\r\n out of range!!!");
						cert_len = CERT_CHAIN_LEN - cert_ofs;
					}
					printk("\r\n ------------------------auth-> %d %d", cert_ofs, cert_len);
					break;
				case 0x1B:
					break;
				default:
					break;
			}
		}
	}

	fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
}

static uint8_t is_mpp_xfer_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x31 || hdr == 0x04 || hdr == 0x01 || hdr == 0x71 || hdr == 0x81 || hdr == 0x06 ||
    	hdr == 0x51)
    {
    	return 1;
    }
	return 0;
}

void wpc_mpp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask)
{
	struct mpp_prx_ask_pkt_t *mpp_ask = (struct mpp_prx_ask_pkt_t *)com_ask;
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };
	uint8_t operation;//temp value

	switch (mpp_ask->hdr)
	{
		case WPC_PRx_PKT_TYP_CE_03:
			if (gd->rx_infos.mpp_restricted_mode)
			{
				gd->rx_infos.cep_val = com_ask->msg.cep.ce_value;

				if (gd->tx_power > 7500)
				{
					gd->rx_infos.mpp_restricted_power_limit = 1;
				}
				else if (gd->tx_power < 7000)
				{
					gd->rx_infos.mpp_restricted_power_limit = 0;
				}

				if (gd->rx_infos.mpp_restricted_power_limit && gd->rx_infos.cep_val >= 0)
				{
					gd->rx_infos.cep_val = -2;
					printk("#");
				}

				osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
				osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
				if (gd->rx_infos.pch_t_delay >= 20)
				{
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_MPP);
				}
			}
			else
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_MPP_ILLEGAL_PKT);
			}
			break;
		case WPC_PRx_PKT_TYP_RP8_04:
			if (gd->rx_infos.mpp_restricted_mode)
			{
				osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
				gd->rx_power = (com_ask->msg.rp8.rp_value * (uint32_t)gd->rx_infos.max_power * 1000) >> 8;
				osal_start_timerEx(WPC_NEXT_TIMER, 0, 0, WPC_TASK, WPC_EVT_PFOD);
			}
			else
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_MPP_ILLEGAL_PKT);
			}
			break;
		case WPC_PRx_PKT_TYP_CHS_05:
			gd->rx_infos.chr_status = com_ask->msg.chs.chs_value;
			printk(" %%: [charge status %d%%]", gd->rx_infos.chr_status);
			break;
		case MPP_PRx_PKT_TYP_XCE_19:
			gd->rx_infos.cep_val = mpp_ask->msg.xce.xce_value;

			if ((0 == gd->rx_infos.cep_val) && (TRUE == gd->tx_infos.flg_cloak_tx_init))//TODO: tx init cloak here, should not happen in a normal process
			{
				if (cnt_cep0++ > 20)
				{
					cnt_cep0 = 0;
					gd->tx_infos.flg_cloak_tx_enter = TRUE;
					need_atn_cnt = 100;
				}
			}

			osal_start_timerEx(WPC_CEP_TIMER, T_MPP_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);

			if ((need_atn_cnt != 0) || (TRUE == gd->tx_infos.flg_cloak_tx_enter) || (gd->tx_infos.need_renego_cap == 1) || \
				(gd->tx_infos.power_mode_trans_atn == 1) || (gd->tx_infos.power_mode_trans_cloak == 1))
			{
				if (need_atn_cnt > 0) need_atn_cnt--;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ATN);//tx init attention
			}
			else
			{
				if (gd->power_limit_sts.fop_flag == 1 && gd->rx_infos.cep_val > 0)
				{
					gd->rx_infos.cep_val = 0;
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);
				}
				else
				{
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				}
				
//				if (ptx_open_flag == 0)
				{
					//if (gd->rx_infos.cep_val > 0 || gd->rx_infos.cep_val < 0)
					//{
						osal_start_timerEx(WPC_NEXT_TIMER, T_XCE_RESP_TO + gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);//TODO: why pch delay add more 20ms? after FSK?
					//}
					// else
					// {
					// 	osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.wnd_size, 0, WPC_TASK, WPC_EVT_1ST_WND);
					// }
				}
			}
			break;
		case WPC_PRx_PKT_TYP_NEGO_09:
			osal_start_timerEx(WPC_NEXT_TIMER, T_RENEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
			gd->ptx_protocol_phase = WPC_PHASE_NEGO;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case WPC_PRx_PKT_TYP_DSR_15:
			mpp_dsr_pkt_handler(com_ask);
			break;
		case MPP_PRx_PKT_TYP_CLOAK_18:
			gd->tx_infos.cloak_reason = mpp_ask->msg.cloak.reason;

			if (gd->tx_infos.cloak_reason <= 0x06)
			{
				gd->tx_infos.flg_cloak_tx_enter = FALSE;
				gd->tx_infos.flg_mode_cloak = TRUE;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				gd->tx_infos.fsk_done_event |= 4; //cloak

				gd->tx_infos.cloak_ping_delay = (gd->tx_infos.cloak_ping_delay > 0)?gd->tx_infos.cloak_ping_delay : 5;
				gd->tx_infos.cloak_det_ping_delay = (gd->tx_infos.cloak_det_ping_delay > 0)?gd->tx_infos.cloak_det_ping_delay : 1;
			}
			break;
		case MPP_PRx_PKT_TYP_GET_28:
			mpp_get_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_SDSR_38:
			mpp_sdsr_pkt_handler(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_SADC_48:
			mpp_sadc_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_REPORT_58:
			mpp_report_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_MSR_13:
			fsk_pkt.mpp_fsk.mss.hdr_0x23 = MPP_PTx_PKT_TYP_MSS_23;
			fsk_pkt.mpp_fsk.mss.error_code = 0;

			if (gd->power_mode != mpp_ask->msg.msr.main_mode)
			{
				gd->power_mode = mpp_ask->msg.msr.main_mode; //high;
				printk(" [MSR trans:%d aux:%d]",mpp_ask->msg.msr.main_mode, mpp_ask->msg.msr.aux);

#if MPP_25W_POWER_MODE_TRANS_W_EPTR
				fsk_pkt.mpp_fsk.mss.status = 1;//pending, power mode trans with intteruption
				gd->tx_infos.power_mode_trans_atn = 1;
				gd->tx_infos.power_mode_trans_eptr = 1;
#elif MPP_25W_POWER_MODE_TRANS_W_CLOAK
				fsk_pkt.mpp_fsk.mss.status = 1;//pending, power mode trans with intteruption
				gd->tx_infos.power_mode_trans_atn = 1;
				gd->tx_infos.power_mode_trans_cloak = 1;
#else
				fsk_pkt.mpp_fsk.mss.status = 0;//success, power mode trans without break
#endif
			}
			else
			{
				fsk_pkt.mpp_fsk.mss.status = 0;//success, power mode no change
			}

			if (fsk_pkt.mpp_fsk.mss.status)
				printk(" MSS pending");
			else
				printk(" MSS success");
				
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case 0x2B://CAL_OP
			if (gd->rx_infos.qi_version >= 0x22)
			{
				operation = mpp_ask->msg.data[0] >> 5;
				if (operation == 1)//0: initialize, 1: commit
				{
					printk(" [CAL_OP_CMT]");
					gd->dploss_cal.cmt = 1;
					if (pfod_dploss_cal_cmt((uint16_t *)&gd->tx_infos.dp_alpha, (uint16_t *)&gd->tx_infos.dp_beta) == 0)//TODO: cal success
					{
						gd->dploss_cal.success = 1;
						gd->tx_infos.need_renego_cap = 1;
						gd->tx_infos.tar_cap_cali = 250;
						gd->tx_infos.nego_cap = 250;
						gd->tx_infos.power_limit_reason = 0;
					}
				}
				else
				{
					printk(" [CAL_OP_INIT]");
					osal_mem_clear((void *)&gd->dploss_cal, sizeof(gd->dploss_cal));
					pfod_dploss_init();
				}

				fsk_pkt.mpp_fsk.data[0] = 0x1B;
				fsk_pkt.mpp_fsk.data[1] = 0x00;//accepted no error
				fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			}
			break;
		case 0x2C://CAL_START CAL_ENTER
			printk(" [CAL_START resum:%d]", mpp_ask->msg.data[0]&0x01);
			osal_mem_clear((void *)&gd->dploss_cal, sizeof(gd->dploss_cal));
			pfod_dploss_init();

			fsk_pkt.mpp_fsk.data[0] = 0x34;

			if (gd->tx_infos.fo_exist)
				fsk_pkt.mpp_fsk.data[1] = 0x00;//error and do not accept cal
			else 
				fsk_pkt.mpp_fsk.data[1] = 0x01;//no error and accept cal

			fsk_pkt.mpp_fsk.data[2] = 100;//points
			fsk_pkt.mpp_fsk.data[3] = 120;//seconds
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case 0x2D://CAL_END
			printk(" [CAL_END clear:%d]", mpp_ask->msg.data[0]&0x01);
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_CAL_CAP_96:
			gd->dploss_cal.index = mpp_ask->msg.cal_capture.index;
			gd->dploss_cal.index_cnt++;
			gd->dploss_cal.preceived = (mpp_ask->msg.cal_capture.rcvd_power_msb << 8) + mpp_ask->msg.cal_capture.rcvd_power_lsb;
			gd->dploss_cal.prect = (mpp_ask->msg.cal_capture.prect_msb << 8) + mpp_ask->msg.cal_capture.prect_lsb;
			gd->dploss_cal.vrect = (mpp_ask->msg.cal_capture.vrect_msb << 8) + mpp_ask->msg.cal_capture.vrect_lsb;
			gd->dploss_cal.irect = (mpp_ask->msg.cal_capture.irect_h << 8) + mpp_ask->msg.cal_capture.irect_l;

			// printk("\r\n [CAL_CAPTURE %d,%d,%d,%d,%d]",
			// 			gd->dploss_cal.index_cnt, gd->dploss_cal.preceived, gd->dploss_cal.prect, gd->dploss_cal.vrect, gd->dploss_cal.irect);

			fsk_pkt.mpp_fsk.data[0] = 0x14;
			fsk_pkt.mpp_fsk.data[1] = 0x00;//accepted no error

			pfod_dploss_cal();

			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_PLA2_88:
			mpp_pla2_pkt_process(mpp_ask);
			break;
		default:
			if (is_mpp_sadt_pkt(mpp_ask->hdr))
			{
				mpp_sadt_pkt_process(mpp_ask);
			}

			if (is_mpp_xfer_illegal_pkt(com_ask->hdr))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT);
			}
			break;
	}
}

void wpc_xfer_phase_process(struct com_prx_ask_pkt_t *com_pkt)
{
	switch (gd->rx_infos.power_profile_mode)
	{
		case BPP:
			wpc_bpp_xfer_phase_protocol_process(com_pkt);
			break;
		case EPP:
			wpc_epp_xfer_phase_protocol_process(com_pkt);
			break;
		case MPP:
			wpc_mpp_xfer_phase_protocol_process(com_pkt);
			break;
		default:
			break;
	}
}

void wpc_mpp_cloak_phase_protocol_process(struct com_prx_ask_pkt_t *com_pkt)
{
	struct mpp_prx_ask_pkt_t *mpp = (struct mpp_prx_ask_pkt_t *)com_pkt;

	switch (mpp->hdr)
	{
		case MPP_PRx_PKT_TYP_CLOAK_18:

			if (TRUE == gd->tx_infos.flg_cloak_tx_init)
			{
				if (cnt_cloak_pkt++ > 5)
				{
					cnt_cloak_pkt = 0;
					gd->tx_infos.flg_cloak_tx_exit = TRUE;
				}
			}

#if MPP_25W_POWER_MODE_TRANS_W_CLOAK
			if (gd->tx_infos.power_mode_trans_cloak == 1)
			{
				if(cnt_cloak_pkt++ >= 0)//2
				{
					cnt_cloak_pkt = 0;
					gd->tx_infos.flg_cloak_tx_exit = TRUE;
				}
			}
#endif

			if (FALSE == gd->tx_infos.flg_cloak_tx_exit)
			{
				gd->tx_infos.cloak_reason = mpp->msg.cloak.reason;
				osal_stop_timerEx(WPC_NEXT_TIMER);
				if (gd->tx_infos.cloak_reason <= 0x06)
				{
					gd->tx_infos.flg_mode_cloak = TRUE;
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
					gd->tx_infos.fsk_done_event |= 4; //cloak
				}
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ATN);
				osal_stop_timerEx(WPC_NEXT_TIMER);
			}
			break;

		case MPP_PRx_PKT_TYP_REPORT_58:
			gd->tx_infos.flg_cloak_tx_exit = FALSE;


			uint32_t tmp_id = 0;
			uint32_t tmp_base_id = 0;
			tmp_id = ((mpp->msg.report_xid.prx_byteid0 << 16 | mpp->msg.report_xid.prx_byteid1 << 8 | mpp->msg.report_xid.prx_byteid2)>>3) & 0xFFFFF;
			tmp_base_id = gd->rx_infos.device_id >> 11 & 0xFFFFF;

			if (tmp_base_id != tmp_id)
			{
//				printk("\r\n%x,%x",tmp_base_id,tmp_id);
				goto _CLOAK_PHASE_ERR_;
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT_EX, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
			}
			break;

		case MPP_PRx_PKT_TYP_GET_28:
			mpp_get_pkt_process(mpp);
			osal_stop_timerEx(WPC_NEXT_TIMER);

			gd->tx_infos.flg_mode_cloak = FALSE;
			gd->ptx_protocol_phase = WPC_PHASE_XFER;
			osal_start_timerEx(WPC_CEP_TIMER, T_MPP_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
			osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
			break;

		default:
			if (is_cloak_phase_illegal_pkt(com_pkt->hdr))
			{
_CLOAK_PHASE_ERR_:
				gd->tx_infos.flg_mode_cloak = FALSE;

				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_CLOAK_PHASE_NO_THIS_PKT);
			}
			break;
	}
}

void wpc_cloak_phase_process(struct com_prx_ask_pkt_t *com_pkt)
{
	switch (gd->rx_infos.power_profile_mode)
	{
		case BPP:
		case EPP:
			break;
		case MPP:
			wpc_mpp_cloak_phase_protocol_process(com_pkt);
			break;
		default:
			break;
	}
}
