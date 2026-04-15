#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "fm1210.h"
#include "pkt_type.h"
#include "delay.h"
#include "debug.h"
#include "fsk.h"
#include "_wpc.h"
#include "algo.h"
#include "osal.h"
#include "wpc_xfer.h"
#include "wpc_5_xfer_4_dstrm.h"

#define PU_CERT_LEN_OFS	    (2 + 32 + 328)
#define PU_CERT_LEN_MAX     (400)
#define CERT_CHAIN_LEN      (PU_CERT_LEN_OFS + PU_CERT_LEN_MAX)
#define DIGEST_LENGTH       (32)
#define CHALL_LENGTH        (64)
#define CRC_INITIAL_VALUE   (0xFFFF)

#define AUTH_PORTOCOL_VER   (0x1)
#define AUTH_SOLT_NUM_0     (0x0)

#define DS_EVEN             0
#define DS_ODD              1

enum auth_header_t
{
	RSP_DIGESTS        = 0x11,
	RSP_CERTIFICATE    = 0x12,
	RSP_CHALLENGE_AUTH = 0x13,
	RSP_ERROR          = 0x17,
	GET_DIGESTS        = 0x19,
	GET_CERTIFICATE    = 0x1A,
	GET_CHALLENGE_AUTH = 0x1B,
};

enum auth_error_code_t
{
	RSP_ERROR_CODE_INVALID_REQUEST      = 0x01,
	RSP_ERROR_CODE_UNSUPPORTED_PROTOCOL = 0x02,
	RSP_ERROR_CODE_BUSY                 = 0x03,
	RSP_ERROR_CODE_UNSPECIFIED          = 0x04,
};

enum ds_status_t
{
	DS_STS_STANDBY = 0,
	DS_STS_OPENING = 1,
	DS_STS_SENDING = 2,
	DS_STS_CLOSING = 3,
};

uint8_t array_digest[1 + DIGEST_LENGTH] =
{
	0x11,
};

uint8_t array_chall[CHALL_LENGTH];

uint8_t cert_chain[CERT_CHAIN_LEN] =
{
		    0x01, 0x6B,

			0xA1, 0x75, 0x9E, 0xCC, 0xA0, 0xBE, 0x3B, 0x85, 0x01, 0x18, 0x18, 0x3E, 0xD6,
			0xCD, 0xD6, 0xD4, 0xA5, 0xDB, 0x7D, 0x83, 0xE6, 0xFD, 0x0E, 0x6F, 0x47, 0x5C,
			0xE4, 0xBB, 0x6E, 0xA0, 0x14, 0x24,

			0x30, 0x82, 0x01, 0x44, 0x30, 0x81, 0xEB, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 
			0x08, 0x66, 0x07, 0x0F, 0x3C, 0x28, 0x48, 0x6F, 0x79, 0x30, 0x0A, 0x06, 0x08, 
			0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x11, 0x31, 0x0F, 0x30, 
			0x0D, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x06, 0x57, 0x50, 0x43, 0x43, 0x41, 
			0x31, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x35, 0x30, 0x37, 0x32, 0x34, 0x30, 0x39, 
			0x32, 0x30, 0x33, 0x36, 0x5A, 0x18, 0x0F, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32, 
			0x33, 0x31, 0x32, 0x33, 0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x12, 0x31, 0x10, 
			0x30, 0x0E, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x07, 0x30, 0x31, 0x44, 0x31, 
			0x2D, 0x31, 0x44, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 
			0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 
			0x03, 0x42, 0x00, 0x04, 0x3E, 0x12, 0x80, 0xAF, 0xE2, 0x8B, 0x1B, 0xE5, 0xAF, 
			0x94, 0xC3, 0xA3, 0xA1, 0xD6, 0xB6, 0xE9, 0x0C, 0xE4, 0xFD, 0x95, 0x5E, 0x20, 
			0x8E, 0xC4, 0x45, 0xEA, 0x95, 0x4E, 0x4A, 0x17, 0x23, 0x53, 0x90, 0xB4, 0x97, 
			0x5F, 0x78, 0x12, 0x0C, 0x33, 0x5E, 0x2F, 0x3D, 0xD5, 0xB1, 0x27, 0xE1, 0xBC, 
			0xB3, 0x9A, 0xF2, 0x48, 0x86, 0xB0, 0xA9, 0x3F, 0x47, 0x14, 0x65, 0x4B, 0x35, 
			0x92, 0xB2, 0xC5, 0xA3, 0x2A, 0x30, 0x28, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1D, 
			0x13, 0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01, 
			0x00, 0x30, 0x12, 0x06, 0x05, 0x67, 0x81, 0x14, 0x01, 0x01, 0x01, 0x01, 0xFF, 
			0x04, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x03, 0x30, 0x0A, 0x06, 0x08, 0x2A, 
			0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 
			0x20, 0x64, 0xF3, 0xCB, 0x20, 0xB3, 0x9E, 0x22, 0x7A, 0xD2, 0xB6, 0x6E, 0x9C, 
			0x78, 0x93, 0x0F, 0x2D, 0xAE, 0xFB, 0xBA, 0xA1, 0x4C, 0x50, 0x3F, 0x1B, 0x1D, 
			0x3B, 0xF8, 0x0C, 0x4E, 0x25, 0x5B, 0xEA, 0x02, 0x21, 0x00, 0xF0, 0x9C, 0xAD, 
			0xEF, 0xED, 0x8A, 0x1F, 0xA9, 0xE2, 0x79, 0xC1, 0x64, 0xEF, 0xDB, 0x5B, 0x14, 
			0x13, 0x83, 0x15, 0x86, 0x7B, 0x9C, 0xDB, 0xA2, 0x6B, 0x49, 0xAD, 0x4A, 0x49, 
			0xBA, 0x92, 0xB9,


	//Product Unit Certificate
};

uint8_t auth_request_type;

uint16_t get_cert_ofs;
uint16_t get_cert_len;

uint8_t  ds_incoming_status[4];
uint8_t  ds_incoming_parity[4];
uint16_t adt_buff_recv_crc;
uint16_t adt_have_recv_len;
uint16_t adt_need_recv_len;
uint8_t  adt_data_recv_buf[18];

uint8_t  ds_outgoing_status[4];
uint8_t  ds_outgoing_parity[4];
uint16_t adt_buff_send_crc;
uint16_t adt_need_send_len;
uint16_t adt_have_send_len;
uint8_t  adt_last_send_len;
uint8_t  adt_buff_send_log;
uint8_t  adt_buff_send_ofs;
uint8_t* adt_buff_send_ptr;
uint8_t  adt_buff_send_cpy[8];

void ds_init(void)
{
	for (int i=0; i<4; i++)
	{
		ds_incoming_status[i] = DS_STS_STANDBY;
		ds_outgoing_status[i] = DS_STS_STANDBY;
	}
}

static void ds_record_ptx_fsk_data(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
//	for (int i=0; i<sizeof(adt_buff_send_cpy); i++)
//	{
//		 adt_buff_send_cpy[i] = fsk_pkt->mpp_fsk.data[i];
//	}
	osal_mem_copy(&adt_buff_send_cpy[0], &fsk_pkt->mpp_fsk.data[0], sizeof(adt_buff_send_cpy));
}

static void ds_resend_ptx_fsk_data(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
//	for(int i=0; i <sizeof(adt_buff_send_cpy); i++)
//	{
//		fsk_pkt->mpp_fsk.data[i] = adt_buff_send_cpy[i];
//	}
	osal_mem_copy(&fsk_pkt->mpp_fsk.data[0], &adt_buff_send_cpy[0], sizeof(adt_buff_send_cpy));
	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt->mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt->mpp_fsk.data[0]) + 1);
}

static void ds_mpp_ptx_sadc_open(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
	enum { PTX_SADC_RST_ALL_STREAMS = 0, PTX_SADC_RST_SPEC_STREAM = 1, PTX_SADC_CLOSE_AND_ABORT = 2, PTX_SADC_CLOSE_COMPLETE = 3, PTX_SADC_OPEN_TRANSPORT = 4, };

	switch (auth_request_type)
	{
		case GET_DIGESTS:
			adt_buff_send_ptr = &array_digest[1];
			adt_need_send_len = (adt_data_recv_buf[1] & 0x01) ? DIGEST_LENGTH + 2 : 2; //slot mask, the digest of the Certificate Chain stored in slot 0
			break;
		case GET_CERTIFICATE:
			get_cert_ofs = ((adt_data_recv_buf[1] & 0xFF) >> 5) * 256 + adt_data_recv_buf[2];
			get_cert_len = ((adt_data_recv_buf[1] & 0x1C) >> 2) * 256 + adt_data_recv_buf[3];

			if (get_cert_ofs >= 0x600)
			{
				get_cert_ofs = 2 + 32 + 4 + (cert_chain[36] << 8) + cert_chain[37] + get_cert_ofs - 0x600;
			}

			if (get_cert_len == 0)
			{
				get_cert_len = ((cert_chain[0] << 8) | cert_chain[1]) > get_cert_ofs ? ((cert_chain[0] << 8) | cert_chain[1]) - get_cert_ofs : (cert_chain[0] << 8) | cert_chain[1];
			}

			if (((cert_chain[0] << 8) | cert_chain[1]) < get_cert_ofs + get_cert_len)
			{
				wpc_printk("\r\n error %d %d %d", (cert_chain[0] << 8) | cert_chain[1], get_cert_ofs, get_cert_len);
				auth_request_type = RSP_ERROR;
				array_chall[0] = RSP_ERROR_CODE_INVALID_REQUEST;
				array_chall[1] = 0x00;
				adt_need_send_len = 3;
			}
			else
			{
				adt_buff_send_ptr = &cert_chain[get_cert_ofs];
				adt_need_send_len = get_cert_len + 1;
			}
			break;
		case GET_CHALLENGE_AUTH:
			if (adt_data_recv_buf[1] != AUTH_SOLT_NUM_0)
			{
				auth_request_type = RSP_ERROR;
				array_chall[0] = RSP_ERROR_CODE_INVALID_REQUEST;
				array_chall[1] = 0x00;
				adt_need_send_len = 3;
			}
			else
			{
				adt_buff_send_ptr = &array_chall[0];
				adt_need_send_len = CHALL_LENGTH + 3;
			}
			break;
		default:
			if ((auth_request_type >> 4) != AUTH_PORTOCOL_VER)
			{
				array_chall[0] = RSP_ERROR_CODE_UNSUPPORTED_PROTOCOL;
				array_chall[1] = AUTH_PORTOCOL_VER;
			}
			else
			{
				array_chall[0] = RSP_ERROR_CODE_INVALID_REQUEST;
				array_chall[1] = 0x00;
			}
			auth_request_type = RSP_ERROR;
			adt_need_send_len = 3;
			break;
	}

	adt_have_send_len = 0;
	adt_last_send_len = 0;
	ds_outgoing_parity[1] = DS_ODD;
	adt_buff_send_log = 0;

	fsk_pkt->mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
	fsk_pkt->mpp_fsk.sadc.request = PTX_SADC_OPEN_TRANSPORT;
	fsk_pkt->mpp_fsk.sadc.stream_num = 0x01;
	fsk_pkt->mpp_fsk.sadc.param_msb = (adt_need_send_len >> 8) & 0xFF;
	fsk_pkt->mpp_fsk.sadc.param_lsb = (adt_need_send_len >> 0) & 0xFF;
	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt->mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt->mpp_fsk.data[0]) + 1);

	ds_record_ptx_fsk_data(fsk_pkt);

	switch (auth_request_type)
	{
		case GET_DIGESTS:
			wpc_printk(" [RSP_DIGESTS]");
			break;
		case GET_CERTIFICATE:
			wpc_printk(" [RSP_CERTIFICATE] [cert: %d %d]", get_cert_ofs, get_cert_len);
			break;
		case GET_CHALLENGE_AUTH:
			wpc_printk(" [RSP_CHALLENGE_AUTH]");
			break;
		default:
			wpc_printk(" [RSP_ERROR]");
			break;
	}
}

static void ds_mpp_ptx_sadc_close(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
	enum { PTX_SADC_RST_ALL_STREAMS = 0, PTX_SADC_RST_SPEC_STREAM = 1, PTX_SADC_CLOSE_AND_ABORT = 2, PTX_SADC_CLOSE_COMPLETE = 3, PTX_SADC_OPEN_TRANSPORT = 4, };

	fsk_pkt->mpp_fsk.sadc.hdr_4F = MPP_PTx_PKT_TYP_SADC_4F;
	fsk_pkt->mpp_fsk.sadc.request = PTX_SADC_CLOSE_COMPLETE;
	fsk_pkt->mpp_fsk.sadc.stream_num = 0x01;
	fsk_pkt->mpp_fsk.sadc.param_msb = (adt_buff_send_crc >> 8) & 0xFF;
	fsk_pkt->mpp_fsk.sadc.param_lsb = (adt_buff_send_crc >> 0) & 0xFF;
	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt->mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt->mpp_fsk.data[0]) + 1);

	ds_record_ptx_fsk_data(fsk_pkt);
}

static void ds_mpp_ptx_sadt_xfer(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
	ds_outgoing_parity[1] ^= 1;

	if (adt_have_send_len == 0)
	{
		uint8_t tmp_buff[3];
		switch (auth_request_type)
		{
			case GET_DIGESTS:
				tmp_buff[0] = RSP_DIGESTS;
				tmp_buff[1] = 0x11;
				adt_last_send_len = adt_buff_send_ofs = 2;
				break;
			case GET_CERTIFICATE:
				tmp_buff[0] = RSP_CERTIFICATE;
				adt_last_send_len = adt_buff_send_ofs = 1;
				break;
			case GET_CHALLENGE_AUTH:
				tmp_buff[0] = RSP_CHALLENGE_AUTH;
				tmp_buff[1] = 0x11;
				tmp_buff[2] = array_digest[32];
				adt_last_send_len = adt_buff_send_ofs = 3;
				break;
			default:
				tmp_buff[0] = RSP_ERROR;
				tmp_buff[1] = array_chall[0]; //auth_error_code
				tmp_buff[2] = array_chall[1]; //auth_error_data
				adt_last_send_len = adt_buff_send_ofs = 3;
				break;
		}

//		for(int i=0; i <adt_last_send_len; i++)
//		{
//			fsk_pkt->mpp_fsk.sadt.data[i] = tmp_buff[i];
//		}
		osal_mem_copy(&fsk_pkt->mpp_fsk.sadt.data[0], &tmp_buff[0], adt_last_send_len);
	}
	else
	{
		adt_last_send_len = (adt_need_send_len - adt_have_send_len > 6) ? 6 : adt_need_send_len - adt_have_send_len;
//		for(int i=0; i <adt_last_send_len; i++)
//		{
//			fsk_pkt->mpp_fsk.sadt.data[i] = *(adt_buff_send_ptr + adt_have_send_len - adt_buff_send_ofs + i);
//		}
		osal_mem_copy(&fsk_pkt->mpp_fsk.sadt.data[0], adt_buff_send_ptr + adt_have_send_len - adt_buff_send_ofs, adt_last_send_len);
	}

	fsk_pkt->mpp_fsk.sadt.hdr = (ds_outgoing_parity[1] == DS_EVEN) ? (adt_last_send_len + 1) << 4 | 0x06 : (adt_last_send_len + 1) << 4 | 0x07;
	fsk_pkt->mpp_fsk.sadt.stream_num = 0x01;
	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt->mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt->mpp_fsk.data[0]) + 1);

	adt_buff_send_crc = crc16_ccitt(&fsk_pkt->mpp_fsk.sadt.data[0], adt_last_send_len, adt_have_send_len == 0 ? CRC_INITIAL_VALUE : adt_buff_send_crc);
	ds_record_ptx_fsk_data(fsk_pkt);

	wpc_printk(" [%02d,%03d]", ++adt_buff_send_log, adt_have_send_len + adt_last_send_len);
}

void ds_mpp_prx_sdsr_pkt_handler(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	enum { SDSR_ACK = 0, SDSR_UNEXPECTED = 1, SDSR_ERR_BUSY = 2, SDSR_ERR_CRC = 3, };

	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	switch (ds_outgoing_status[1])
	{
		case DS_STS_OPENING:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
					ds_outgoing_status[1] = DS_STS_SENDING;
					ds_mpp_ptx_sadt_xfer(&fsk_pkt);
					break;
				default:
					ds_resend_ptx_fsk_data(&fsk_pkt);
					break;
			}
			break;
		case DS_STS_SENDING:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
				case SDSR_UNEXPECTED:
					adt_have_send_len += adt_last_send_len;
					if (adt_have_send_len >= adt_need_send_len)
					{
						ds_outgoing_status[1] = DS_STS_CLOSING;
						ds_mpp_ptx_sadc_close(&fsk_pkt);
					}
					else
					{
						ds_mpp_ptx_sadt_xfer(&fsk_pkt);
					}
					break;
				default:
					ds_resend_ptx_fsk_data(&fsk_pkt);
					break;
			}
			break;
		case DS_STS_CLOSING:
			switch (mpp_ask->msg.sdsr.type)
			{
				case SDSR_ACK:
					need_atn_evt = 0;
					ds_outgoing_status[1] = DS_STS_STANDBY;
					fsk_pkt.mpp_fsk.data[0] = 0x00;
					fsk_pkt.mpp_fsk.data[1] = 0x00;
					fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
					break;
				case SDSR_ERR_CRC:
					ds_outgoing_status[1] = DS_STS_STANDBY;
					need_atn_evt = 1;
					need_atn_cnt = 100;
					break;
				default:
					ds_resend_ptx_fsk_data(&fsk_pkt);
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

void ds_mpp_prx_sadc_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	enum { SDSR_ACK = 0, SDSR_UNEXPECTED = 1, SDSR_ERR_BUSY = 2, SDSR_ERR_CRC = 3, };
	enum { SADC_RST_ALL_STREAMS = 0, SADC_RST_SPEC_STREAM = 1, SADC_CLOSE_AND_ABORT = 2, SADC_CLOSE_AND_CMPLT = 3, SADC_OPEN_DATA_TRANS = 4, };

	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	//how to handle if stream_num != 1 ???
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
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_STANDBY;
			break;
		case SADC_CLOSE_AND_CMPLT:
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_STANDBY;
			if (adt_need_recv_len == 0 || adt_buff_recv_crc != (mpp_ask->msg.sadc.param_msb << 8 | mpp_ask->msg.sadc.param_lsb))
			{
				fsk_pkt.mpp_fsk.sdsr.type = SDSR_ERR_CRC;
			}
			else
			{
				if (mpp_ask->msg.sadc.stream_num == 1)
				{
					if (auth_request_type == GET_CHALLENGE_AUTH)
					{
						osal_start_timerEx(WPC_AUTH_TIMER, 10, 0, WPC_TASK, WPC_EVT_SE_IC_TBS_AUTH);
					}
					else
					{
						need_atn_cnt = 100;
					}
					ds_outgoing_status[mpp_ask->msg.sadc.stream_num] = DS_STS_STANDBY;
					need_atn_evt = 1;//auth_event
				}
			}
			break;
		case SADC_OPEN_DATA_TRANS:
			ds_incoming_status[mpp_ask->msg.sadc.stream_num] = DS_STS_OPENING;
			ds_incoming_parity[mpp_ask->msg.sadc.stream_num] = DS_EVEN;
			adt_have_recv_len = 0;
			adt_need_recv_len = mpp_ask->msg.sadc.param_msb << 8 | mpp_ask->msg.sadc.param_lsb;
			adt_buff_recv_crc = 0;
			auth_request_type = 0;
			break;
		default:
			fsk_pkt.mpp_fsk.sdsr.type = SDSR_UNEXPECTED;
			break;
	}

	fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
}

void ds_mpp_prx_sadt_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	if (mpp_ask->msg.sadt.stream_num != 1)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
		return;
	}

	if (ds_incoming_status[mpp_ask->msg.sadt.stream_num] == DS_STS_STANDBY)
	{
		fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
		return;
	}

	if ((mpp_ask->hdr & 0x01) == ds_incoming_parity[mpp_ask->msg.sadt.stream_num])
	{
		adt_buff_recv_crc = crc16_ccitt(&mpp_ask->msg.sadt.data[0], (mpp_ask->hdr >> 4) - 1, adt_have_recv_len == 0 ? CRC_INITIAL_VALUE : adt_buff_recv_crc);
		ds_incoming_parity[mpp_ask->msg.sadt.stream_num] ^= 1;
		for (int i=0; i<(mpp_ask->hdr >> 4)-1; i++)
		{
			if (adt_have_recv_len + i < sizeof(adt_data_recv_buf))
			{
				adt_data_recv_buf[adt_have_recv_len+i] = mpp_ask->msg.sadt.data[i];
			}
		}
		adt_have_recv_len += (mpp_ask->hdr >> 4) - 1;
	}

	fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);

	if (ds_incoming_status[mpp_ask->msg.sadt.stream_num] == DS_STS_OPENING)
	{
		ds_incoming_status[mpp_ask->msg.sadt.stream_num] = DS_STS_SENDING;
		auth_request_type = mpp_ask->msg.sadt.data[0];
		switch (auth_request_type)
		{
			case GET_DIGESTS:
				wpc_printk(" [GET_DIGESTS]");
				break;
			case GET_CERTIFICATE:
				wpc_printk(" [GET_CERTIFICATE]");
				break;
			case GET_CHALLENGE_AUTH:
				wpc_printk(" [GET_CHALLENGE_AUTH]");
				break;
			default:
				break;
		}
	}
}

void ds_mpp_prx_dsr_poll_handler(struct mpp_ptx_fsk_pkt_t *fsk_pkt)
{
	switch (ds_outgoing_status[1])
	{
		case DS_STS_STANDBY:
			ds_outgoing_status[1] = DS_STS_OPENING;
		case DS_STS_OPENING:
			ds_mpp_ptx_sadc_open(fsk_pkt);
			break;
		default:
			ds_resend_ptx_fsk_data(fsk_pkt);
			break;
	}
}
