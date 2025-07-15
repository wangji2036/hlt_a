#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "fsk.h"
#include "pkt_type.h"
#include "wpc_nego.h"
#include "wpc_xfer.h"
#include "epp.h"
#include "pfod.h"
#include "debug.h"

struct power_transfer_contract_t ptx_power_contract, prx_power_contract;

void power_contract_init(void)
{
	osal_mem_set((uint8_t *)&ptx_power_contract, 0, sizeof(struct power_transfer_contract_t));

	ptx_power_contract.rep_delay = 5;
	ptx_power_contract.pch_delay = 5;
	ptx_power_contract.freq_sel = 0;
	ptx_power_contract.ext_nego_power = 50;
	ptx_power_contract.power_control_profile = 0;
	ptx_power_contract.cloak_det_ping_delay = 0;
	ptx_power_contract.cloak_dig_ping_delay = 5;

	osal_mem_copy((uint8_t *)&prx_power_contract, (uint8_t *)&ptx_power_contract, sizeof(struct power_transfer_contract_t));
}

uint8_t power_contract_change_cnt(struct power_transfer_contract_t *ptx_contract, struct power_transfer_contract_t *prx_contract)
{
	uint8_t change_cnt = 0;

	if (prx_contract->rep_delay != ptx_contract->rep_delay) change_cnt++;

	if (prx_contract->pch_delay != ptx_contract->pch_delay) change_cnt++;

	if (prx_contract->freq_sel != ptx_contract->freq_sel) change_cnt++;

	if (prx_contract->ext_nego_power != ptx_contract->ext_nego_power) change_cnt++;

	if (prx_contract->power_control_profile != ptx_contract->power_control_profile) change_cnt++;

	if (prx_contract->cloak_det_ping_delay != ptx_contract->cloak_det_ping_delay) change_cnt++;

	if (prx_contract->cloak_dig_ping_delay != ptx_contract->cloak_dig_ping_delay) change_cnt++;

	return change_cnt;
}

enum mpp_prx_get_request_type_t
{
	GET_PTx_XID  = 0,
	GET_PTx_INV  = 2,
	GET_PTx_PLAP = 3,
	GET_PTx_ECAP = 4,
	GET_PTx_RCS  = 5,
	GET_PTx_CHS  = 6,
	GET_PTx_KEST = 7,
	GET_PTx_ERR  = 9,
};

enum mpp_prx_srq_request_type_t
{
	SRQ_END_00                = 0x00,
	SRQ_REP_05                = 0x05,
	SRQ_PCH_07                = 0x07,
	SRQ_FREQ_SEL_F0           = 0xF0,
	SRQ_EPGL_F3               = 0xF3,
	SRQ_CLOAK_DIG_PING_LSB_F5 = 0xF5,
	SRQ_PCP_F6                = 0xF6,
	SRQ_CLOAK_DIG_PING_MSB_F7 = 0xF7,
	SRQ_CLOAK_DET_PING_F8     = 0xF8,
};

static uint8_t is_neg_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x01 || hdr == 0x05 || hdr == 0x06 || hdr == 0x09 || hdr == 0x15 || hdr == 0x48 ||
    	hdr == 0x38 || hdr == 0x26 || hdr == 0x51 || hdr == 0x71 || hdr == 0x81 || hdr == 0x19 ||
    	hdr == 0x58 || hdr == 0x03)
    {
    	return 1;
    }
	return 0;
}

void mpp_srq_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	switch (mpp_ask->msg.srq.request)
	{
		case SRQ_END_00:
			if (mpp_ask->msg.srq.parameter == power_contract_change_cnt(&ptx_power_contract, &prx_power_contract))
			{
				osal_mem_copy((uint8_t *)&ptx_power_contract, (uint8_t *)&prx_power_contract, sizeof(struct power_transfer_contract_t));
				//TODO: if PRx Not received ACK successfully, and retry 20 00 packet, need to consider this situation. --Sean
				osal_stop_timerEx(WPC_NEXT_TIMER);
				osal_stop_timerEx(WPC_NEGO_TIMER);
				osal_start_timerEx(WPC_CEP_TIMER, T_MPP_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
				osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
				gd->ptx_protocol_phase = WPC_PHASE_XFER;
				gd->ptx_end_nego_event |= MPP_END_NEGO_FLAG;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				if (gd->rx_infos.mpp_restricted_mode == 1)
				{
					gd->rx_infos.mpp_restricted_mode = 0;
				}
			}
			else
			{
				osal_mem_copy((uint8_t *)&prx_power_contract, (uint8_t *)&ptx_power_contract, sizeof(struct power_transfer_contract_t));
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);
			}
			printk(" [SRQ/end]");

			if (gd->pid_perd == 1127)//128K
			{
				gd->tx_infos._128_nego_gd |= 2;
			}
			else
			{
				gd->tx_infos._128_nego_gd = 0;
#if DIG_DDM_ENABLE
				if (144000 / (EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 360 && gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE == _NU1030_DMO2_OUT_MODE_CAP)
				{
//					hal_ecap_dig_ddm_init();
//					hal_eadc_ddm_init();
					hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_DDM);
					hal_ecap_open(ECAP2);
					hal_ecap_close(ECAP4);
					fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);

					printk("\r\n ----- disable digital ddm");
				}
#endif
			}
			break;
		case SRQ_REP_05:
			gd->tx_infos.t_re_ping = (mpp_ask->msg.srq.parameter & 0x3F) * 200;
			gd->tx_infos.reping_cnt = (mpp_ask->msg.srq.parameter & 0x3F) * 200 / 100;
			if (gd->tx_infos.reping_cnt != 0)
			{
				prx_power_contract.rep_delay = gd->tx_infos.reping_cnt;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);
			}
			printk(" [SRQ/rep]");
			break;
		case SRQ_PCH_07:
			if (gd->rx_infos.qi_version >= 0x21)
			{
				if (mpp_ask->msg.srq.parameter < T_PCH_TIME_MIN || mpp_ask->msg.srq.parameter > T_PCH_TIME_MAX)
				{
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);
				}
				else
				{
					prx_power_contract.pch_delay = mpp_ask->msg.srq.parameter;
					gd->rx_infos.pch_t_delay = prx_power_contract.pch_delay;
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				}
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			}
			printk(" [SRQ/pch]");
			break;
		case SRQ_FREQ_SEL_F0:
			if ((mpp_ask->msg.srq.parameter & 0x3) == 1)
			{
				prx_power_contract.freq_sel = 1;
				gd->tx_infos.dig_ping_type = _360K_FB;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			}
			printk(" [SRQ/freqsel]");
			break;
		case SRQ_EPGL_F3:
			if (mpp_ask->msg.srq.parameter > 150)
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_NAK);
			}
			else
			{
				prx_power_contract.ext_nego_power = mpp_ask->msg.srq.parameter;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			}
			printk(" [SRQ/egpl]");
			break;
		case SRQ_CLOAK_DIG_PING_LSB_F5:
			prx_power_contract.cloak_dig_ping_delay &= ~0x00FF;
			prx_power_contract.cloak_dig_ping_delay |= mpp_ask->msg.srq.parameter;
			gd->tx_infos.cloak_dig_ping_delay &= ~0x00FF;
			gd->tx_infos.cloak_dig_ping_delay |= mpp_ask->msg.srq.parameter;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			printk(" [SRQ/cloakl]");
			break;
		case SRQ_PCP_F6:
			prx_power_contract.power_control_profile = (mpp_ask->msg.srq.parameter & 0x01);
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			printk(" [SRQ/pcp]");
			break;
		case SRQ_CLOAK_DIG_PING_MSB_F7:
			prx_power_contract.cloak_dig_ping_delay &= ~0x0300;
			prx_power_contract.cloak_dig_ping_delay |= (mpp_ask->msg.srq.parameter & 0x03) << 8;
			gd->tx_infos.cloak_dig_ping_delay &= ~0x0300;
			gd->tx_infos.cloak_dig_ping_delay |= (mpp_ask->msg.srq.parameter & 0x03) << 8;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			printk(" [SRQ/cloakh]");
			break;
		case SRQ_CLOAK_DET_PING_F8:
			prx_power_contract.cloak_det_ping_delay = mpp_ask->msg.srq.parameter;
			gd->tx_infos.cloak_det_ping_delay = mpp_ask->msg.srq.parameter;//unit: 100mS_Cloak_Ping
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			printk(" [SRQ/detect]");
			break;
		default:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			break;
	}
}

void mpp_get_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

	switch (mpp_ask->msg.get.param)
	{
		case GET_PTx_XID:
			fsk_pkt.mpp_fsk.xid.hdr_8F = MPP_PTx_PKT_TYP_XID_8F;
			fsk_pkt.mpp_fsk.xid.selector = 0;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_msb = 0x73;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_mid = 0x2F;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_lsb = 0x01;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_msb = 0x00;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_mid = 0x01;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_lsb = 0x11;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_INV:
			fsk_pkt.mpp_fsk.inv.hdr_3F = MPP_PTx_PKT_TYP_INV_3F;
			fsk_pkt.mpp_fsk.inv.selector = 0x00;
			fsk_pkt.mpp_fsk.inv.vinv_msb = (gd->vpwr >> 9) & 0x3F;
			fsk_pkt.mpp_fsk.inv.vinv_lsb = (gd->vpwr >> 1) & 0xFF;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_PLAP:
			fsk_pkt.mpp_fsk.plap.hdr_5F = MPP_PTx_PKT_TYP_PLAP_5F;
			fsk_pkt.mpp_fsk.plap.g_coil_rx_msb = 0x27;
			fsk_pkt.mpp_fsk.plap.g_coil_rx_lsb = 0x10;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_ECAP:
//			fsk_pkt.mpp_fsk.ecap.cal_support = 1;
			fsk_pkt.mpp_fsk.ecap.cal_support = 0;
			if (gd->tx_infos.fo_exist)
			{
				printk("\r\n xxxx");
				gd->tx_infos.tar_cap_fod = 100;
				gd->tx_infos.nego_cap = 100;
				gd->tx_infos.power_limit_reason = 2;
			}
			else if (gd->dploss_cal.success == 1)
			{
				gd->tx_infos.tar_cap_cali = 250;
				gd->tx_infos.nego_cap = 250;
				gd->tx_infos.power_limit_reason = 0;
			}
			else
			{
				gd->tx_infos.tar_cap_cali = 150;
				gd->tx_infos.nego_cap = 150;
				gd->tx_infos.power_limit_reason = 10;//calibration limit
			}

			fsk_pkt.mpp_fsk.ecap.hdr_8F = MPP_PTx_PKT_TYP_ECAP_8F;
			fsk_pkt.mpp_fsk.ecap.selector = 0x01;
			fsk_pkt.mpp_fsk.ecap.ptx_potential_power = gd->tx_infos.max_cap;
			fsk_pkt.mpp_fsk.ecap.prx_negotiable_power = gd->tx_infos.nego_cap;
			fsk_pkt.mpp_fsk.ecap.power_limit_reason = gd->tx_infos.power_limit_reason;
			fsk_pkt.mpp_fsk.ecap.concurrent_data_stream = 2;
			fsk_pkt.mpp_fsk.ecap.data_stream_buffer_size = 3;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_RCS:
			fsk_pkt.mpp_fsk.rcs.hdr_1E = MPP_PTx_PKT_TYP_RCS_1E;
			fsk_pkt.mpp_fsk.rcs.selector = 0x03;
			fsk_pkt.mpp_fsk.rcs.status = 0x00; //normal operation
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_CHS:
			fsk_pkt.mpp_fsk.chs.hdr_1F = MPP_PTx_PKT_TYP_CHS_1F;
			fsk_pkt.mpp_fsk.chs.chs_value = 0xFF;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_KEST:
			fsk_pkt.mpp_fsk.kest.hdr_3F = MPP_PTx_PKT_TYP_KEST_3F;
			fsk_pkt.mpp_fsk.kest.selector = 0x02;
			fsk_pkt.mpp_fsk.kest.kest_msb = ((gd->k_est * 4095 / 10000) >> 8) & 0x0F;
			fsk_pkt.mpp_fsk.kest.kest_lsb = ((gd->k_est * 4095 / 10000) >> 0) & 0xFF;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_ERR:
			fsk_pkt.mpp_fsk.err.hdr_01 = MPP_PTx_PKT_TYP_ERR_01;
			fsk_pkt.mpp_fsk.err.error = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		default:
			fsk_pkt.mpp_fsk.data[0] = 0x00;
			fsk_pkt.mpp_fsk.data[1] = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
	}

//	osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

void wpc_mpp_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	struct mpp_prx_ask_pkt_t *mpp_ask = (struct mpp_prx_ask_pkt_t *)com_ask;
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };
	struct com_ptx_fsk_pkt_t _fsk = { };

	switch (com_ask->hdr)//TODO: need add pkt 0x13, 0x18, 0xA8, 0x90, maybe 0x19/0x58/0x88
	{
		case WPC_PRx_PKT_TYP_GRQ_07:
			if (mpp_ask->msg.data[0] == 0x30)
			{
				_fsk.com_fsk.id.hdr_30 = 0x30;
				_fsk.com_fsk.id.qi_version = 0x21;
				_fsk.com_fsk.id.ptmc_msb = 0x00;
				_fsk.com_fsk.id.ptmc_lsb = 0x5C;
			}
			else if (mpp_ask->msg.data[0] == 0x31)
			{
				_fsk.com_fsk.cap.hdr_31 = 0x31;
				_fsk.com_fsk.cap.neg_power = _fsk.com_fsk.cap.pot_power = 30;//25W
				_fsk.com_fsk.cap.wpid = 0;
				_fsk.com_fsk.cap.buff_size = 7;
			}
			else
			{
				_fsk.com_fsk.null.hdr_00 = 0x00;
				_fsk.com_fsk.null.invalid_data = 0x00;
			}
			fml_fsk_data_send(EPWM1, T_RESPONSE, &_fsk.com_fsk.data[0], wpc_msg_size_get(_fsk.com_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_SRQ_20:
			mpp_srq_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_GET_28:
			mpp_get_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_EDS_29:
			fsk_pkt.mpp_fsk.eds.hdr_2F = MPP_PTx_PKT_TYP_EDS_2F;
			fsk_pkt.mpp_fsk.eds.streams_bitmask_msb = 0x00;
			fsk_pkt.mpp_fsk.eds.streams_bitmask_lsb = 0x02;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_REPORT_58:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_PLAP_78:
			gd->rx_infos.gcoil_tx = (int16_t)(((mpp_ask->msg.plap.g_coil_tx.msb<<8) + mpp_ask->msg.plap.g_coil_tx.lsb)/10000);
			gd->rx_infos.alpha_fm = (int16_t)((mpp_ask->msg.plap.alpha_fm.msb<<8) + mpp_ask->msg.plap.alpha_fm.lsb);//unit 0.5mW
			gd->rx_infos.alpha_fm_dc = (int16_t)((mpp_ask->msg.plap.alpha_fm_dc.msb<<8) + mpp_ask->msg.plap.alpha_fm_dc.lsb);//unit 0.5mW

			printk(" gcoil_tx:%d fm:%d fm_dc:%d", gd->rx_infos.gcoil_tx, gd->rx_infos.alpha_fm, gd->rx_infos.alpha_fm_dc);

			if ((gd->rx_infos.alpha_fm == 112 && gd->rx_infos.alpha_fm_dc == 419) || (gd->rx_infos.alpha_fm == 67 && gd->rx_infos.alpha_fm_dc == 237))//@: 78 00 00 70 01 A3 27 10 9D, SGS NOK9 gcoil_tx:1 fm:56 112 fm_dc:209 419
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_SGS;
			else if ((gd->rx_infos.alpha_fm == 93 && gd->rx_infos.alpha_fm_dc == 309) || (gd->rx_infos.alpha_fm == 74 && gd->rx_infos.alpha_fm_dc == 247))// @: 78 00 00 5D 01 35 27 10 26, Hongk NOK9 gcoil_tx:1 fm:46 93 fm_dc:154 309  C: @: 78 00 00 4A 00 F7 27 10 F2
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_HONK;
			else if ((gd->rx_infos.alpha_fm == 82 && gd->rx_infos.alpha_fm_dc == 248) || (gd->rx_infos.alpha_fm == 66 && gd->rx_infos.alpha_fm_dc == 198))//@: 78 00 00 52 00 F8 27 10 E5, WeiCe NOK9  gcoil_tx:1 fm:41 82 fm_dc:124 248
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_MICRO;
			else if ((gd->rx_infos.alpha_fm == 98 && gd->rx_infos.alpha_fm_dc == 191) || (gd->rx_infos.alpha_fm == 76 && gd->rx_infos.alpha_fm_dc == 152))// @: 78 00 00 62 00 BF 27 10 92, GRL Hongbiao gcoil_tx:1 fm:49 98 fm_dc:95 191
				gd->rx_infos.rx_type = ERX_TYPE_GRL_MPP;
			//pfod_mpla_init();

				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_ECAP_84:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_PROP_1A:
	    case MPP_PRx_PKT_TYP_PROP_1B:
	    case MPP_PRx_PKT_TYP_PROP_2A:
	    case MPP_PRx_PKT_TYP_PROP_2B:
	    case MPP_PRx_PKT_TYP_PROP_39:
	    case MPP_PRx_PKT_TYP_PROP_49:
	    case MPP_PRx_PKT_TYP_PROP_59:
	    	fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			break;
		default:
			if (is_neg_illegal_pkt(mpp_ask->hdr))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT);
				osal_stop_timerEx(WPC_NEXT_TIMER);
				goto __NEGO_PHASE_ERR__;
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			}
			break;
	}

	if (gd->ptx_protocol_phase == WPC_PHASE_XFER)
	{
		osal_stop_timerEx(WPC_NEXT_TIMER);
	}
	else
	{
		osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
	}

__NEGO_PHASE_ERR__:
	return;
}

void wpc_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (gd->rx_infos.power_profile_mode)
	{
		case BPP:
			break;
		case EPP:
			wpc_epp_nego_phase_process(com_ask);
			break;
		case MPP:
			wpc_mpp_nego_phase_process(com_ask);
			break;
		default:
			break;
	}
}
