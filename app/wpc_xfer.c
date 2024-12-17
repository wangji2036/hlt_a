#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "fsk.h"
#include "algo.h"
#include "pfod.h"
#include "pkt_type.h"
#include "wpc_xfer.h"
#include "fm1210.h"
#include "debug.h"
#include "qfod.h"
#include "epp.h"
#include "wpc_5_xfer_4_dstrm.h"

static uint8_t need_cloak_atn;
static uint8_t cnt_cep0 = 0;
static uint8_t cnt_cloak_pkt = 0;

uint8_t need_atn_cnt;
uint8_t need_atn_evt;

void auth_init(void)
{
	need_atn_cnt = 0;
	need_atn_evt = 0;
	ds_init();

	gd->tx_infos.cloak_dig_ping_delay = 0; //TODO: don't zero it here
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
		case 0x28:
			if (gd->rx_infos.prmc == 0x005C && gd->rx_infos.device_id == 0x16197510)
			{
				if ((com_ask->msg.prop.data[0] == 0x12 && com_ask->msg.prop.data[1] == 0x34) ||
				    (com_ask->msg.prop.data[0] == 0x43 && com_ask->msg.prop.data[1] == 0x21))
				{
					qfod_qdt_cali_init();
					gd->ptx_idle_phase_status = WPC_IDLE_STAT_QDT_CAL;
					wpc_stop_to_idle(ESYS_ERR_CODE_NEED_QDT_CALIBRATION);
				}
			}
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

static uint8_t is_mpp_prx_sadt_pkt(uint8_t hdr)
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

void wpc_bpp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (com_ask->hdr)
	{
		case WPC_PRx_PKT_TYP_CE_03:
			gd->rx_infos.cep_val = com_ask->msg.cep.ce_value;

			if (gd->rx_power > 6500 && gd->rx_infos.power_profile_mode == MPP)
			{
				gd->rx_infos.mpp_restricted_power_limit = 1;
			}
			else if (gd->rx_power < 6200)
			{
				gd->rx_infos.mpp_restricted_power_limit = 0;
			}

			if (gd->rx_infos.mpp_restricted_power_limit && gd->rx_infos.cep_val > 0)
			{
				gd->rx_infos.cep_val = 0;
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

void mpp_report_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	uint8_t res = 0;
	if (mpp_ask->msg.report_pla.select == 1)//TODO: add fod func here, add a timer print log to avoid the FSK window
	{
		gd->rx_power = mpp_ask->msg.report_pla.rcvd_power_msb << 8 | mpp_ask->msg.report_pla.rcvd_power_lsb;
		gd->rx_prect = gd->rx_infos.pla_prect = mpp_ask->msg.report_pla.rect_power_msb << 8 | mpp_ask->msg.report_pla.rect_power_lsb;
		osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);

		gd->tx_infos.fsk_done_event |= 8;//set reported event print long log

		//res = pfod_mpla();
		printk("\r\n ---> res-> %d", res);
		if (res == 0)
		{
			if (1 == gd->power_limit_sts.tntc_ot_flag)
			{// ntc ot
				if ((((gd->rx_prect +200)/100) <= (gd->tx_infos.nego_cap)) && (1 == gd->tntc_ot_flag_atn))
				{
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ATN);
					gd->power_limit_sts.tntc_ot_flag = 0;
					gd->tx_infos.need_renego_cap = 1;
					gd->tntc_ot_flag_atn = 2;
				}
			}
			else
			{//no ntc ot, no fod
				gd->p_rect_max_ntc_ot = 0;

				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			}
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
	uint8_t res = 0;

	gd->rx_infos.pla_type = 2;
	//gd->rx_infos.rpp_tick++;

	gd->rx_power = mpp_ask->msg.pla2.rcvd_power_msb << 8 | mpp_ask->msg.pla2.rcvd_power_lsb;
	gd->rx_prect = gd->rx_infos.pla_prect = mpp_ask->msg.report_pla.rect_power_msb << 8 | mpp_ask->msg.report_pla.rect_power_lsb;
	gd->rx_infos.pla_vrect = (mpp_ask->msg.pla2.vrect_msb << 8) + mpp_ask->msg.pla2.vrect_lsb;
	gd->rx_infos.pla_irect = (mpp_ask->msg.pla2.irect_h << 8) + mpp_ask->msg.pla2.irect_l;
	osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);

	gd->tx_infos.fsk_done_event |= 8;//set reported event print long log

	//if (gd->dploss_cal.success == 1)
	//	res = pfod_dploss();
	//else
	//	res = pfod_mpla();

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
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

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
		ds_mpp_prx_dsr_poll_handler(&fsk_pkt);
	}
	else if (gd->tx_infos.power_mode_trans_atn == 1)
	{
		gd->tx_infos.power_mode_trans_atn = 0;
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
	}
	else if (need_cloak_atn == 1)
	{
		need_cloak_atn = 0;
		fsk_pkt.mpp_fsk.cloak.hdr_1E = 0x1E;
		fsk_pkt.mpp_fsk.cloak.selector = 0x00;
		fsk_pkt.mpp_fsk.cloak.reason = 0x02;
		fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
	}
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
					need_cloak_atn = 1;
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
					if (gd->power_limit_sts.tntc_ot_flag == 1)
					{
						if (1 == gd->tntc_ot_flag_atn)
						{
							if (gd->rx_infos.cep_val >= 0)
								gd->rx_infos.cep_val = -10;//-5;
						}
						else if (2 == gd->tntc_ot_flag_atn)
						{
							if (gd->rx_infos.cep_val > 0)
								gd->rx_infos.cep_val = 0;
						}
						else
						{
							;//do nothing
						}
					}
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				}

				osal_start_timerEx(WPC_NEXT_TIMER, T_XCE_RESP_TO + gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
			}
			break;
		case WPC_PRx_PKT_TYP_NEGO_09:
			osal_stop_timerEx(WPC_CEP_TIMER);
			osal_start_timerEx(WPC_NEGO_TIMER, T_RENEGO_TO, 0, WPC_TASK, WPC_EVT_RENEGO_TO);
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

				gd->tx_infos.cloak_dig_ping_delay = (gd->tx_infos.cloak_dig_ping_delay > 0) ? gd->tx_infos.cloak_dig_ping_delay : 5;
				gd->tx_infos.cloak_det_ping_delay = (gd->tx_infos.cloak_det_ping_delay > 0) ? gd->tx_infos.cloak_det_ping_delay : 1;
			}
			break;
		case MPP_PRx_PKT_TYP_GET_28:
			mpp_get_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_SDSR_38:
			ds_mpp_prx_sdsr_pkt_handler(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_SADC_48:
			ds_mpp_prx_sadc_pkt_process(mpp_ask);
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
			if (is_mpp_prx_sadt_pkt(mpp_ask->hdr))
			{
				ds_mpp_prx_sadt_pkt_process(mpp_ask);
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

	osal_stop_timerEx(WPC_PING_TIMER);
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
			osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT_EX, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
			gd->tx_infos.flg_cloak_tx_exit = FALSE;


			uint32_t tmp_id = 0;
			uint32_t tmp_base_id = 0;
			tmp_id = ((mpp->msg.report_xid.prx_byteid0 << 16 | mpp->msg.report_xid.prx_byteid1 << 8 | mpp->msg.report_xid.prx_byteid2)>>3) & 0xFFFFF;
			tmp_base_id = (gd->rx_infos.device_id >> 11) & 0xFFFFF;

			if (tmp_base_id != tmp_id)
			{
				printk("\r\n %x %x", tmp_base_id, tmp_id);
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
			else
			{
				//TODO: response ACK?
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
