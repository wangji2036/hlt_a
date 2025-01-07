#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "g_data.h"
#include "osal.h"
#include "_wpc.h"
#include "fsk.h"
#include "pid.h"
#include "pfod.h"
#include "fm1210.h"
#include "pkt_type.h"
#include "wpc_idle.h"
#include "wpc_ping.h"
#include "wpc_cnfg.h"
#include "wpc_nego.h"
#include "wpc_xfer.h"
#include "debug.h"
#include "usb_pd.h"
#include "t91206.h"
#include "wpc_5_xfer_4_dstrm.h"

static uint8_t special_cep_cnt;
uint8_t wpc_msg_size_get(uint8_t hdr)
{
	uint8_t len;

	if (hdr <= 0x1F)
	{
		len = 1 + ((hdr - 0) >> 5);
	}
	else if (hdr <= 0x7F)
	{
		len = 2 + ((hdr - 32) >> 4);
	}
	else if (hdr <= 0xDF)
	{
		len = 8 + ((hdr - 128) >> 3);
	}
	else
	{
		len = 20 + ((hdr - 224) >> 2);
	}

	return len;
}

static void wpc_pkt_print(void)
{
	uint8_t len;
	if (gd->wpc_pkt.data[0] <= 0x1F)
	{
		len = 1;
	}
	else if (gd->wpc_pkt.data[0] <= 0x7F)
	{
		len = gd->wpc_pkt.data[0] >> 4;
	}
	else if (gd->wpc_pkt.data[0] <= 0xDF)
	{
		len = (gd->wpc_pkt.data[0] >> 3) - 8;
	}
	else
	{
		len = (gd->wpc_pkt.data[0]) - 36;
	}
	len += 2;
	if (len != gd->wpc_pkt.len)
	{
		printk("\r\n error ---------------> %d %d", len, gd->wpc_pkt.len);
	}

#if 0
	printk("\r\n @:");
	fml_test_ask_info_print(gd->wpc_pkt.src);
#else
	printk("\r\n @:[%d]", gd->wpc_pkt.src);
#endif
	for (int i=0; i<gd->wpc_pkt.len; i++)
	{
		printk(" %02X", gd->wpc_pkt.data[i]);
	}
}

void wpc_stop_to_idle(uint8_t err_code)
{
	gd->sys_err_code = err_code;
	osal_set_event(WPC_TASK, WPC_EVT_STOP_POWER);
}

extern uint8_t cnt_cloak_dig_ping;
extern uint8_t cnt_cloak_det_ping;

void wpc_stop_power(void)
{
	if (gd->ptx_protocol_phase == WPC_PHASE_IDLE)
	{
		return;
	}

	if (EPWM1->PWM_PERD.BITS.PWM_PERD + 1 == 144000 / 360)
	{
		if (gd->tx_infos.flg_mode_cloak != TRUE)
		{
			gd->tx_infos.dig_ping_type = _128K_HB;
		}
	}

	hal_epwm_pwm_stop(EPWM1);
	fml_ask_disable();

	fml_nu103x_por_rst();

	if(gd->tx_infos.flg_mode_cloak == TRUE)
	{
		gd->ptx_protocol_phase = WPC_PHASE_CLOAK;

		printk("\r\n cloak_1: %d %d", cnt_cloak_det_ping, cnt_cloak_dig_ping);
		cnt_cloak_dig_ping = 0;
		cnt_cloak_det_ping = 0;
	}
	else
	{
		if (gd->ptx_protocol_phase > WPC_PHASE_PING)
		{
			if (gd->ptx_idle_phase_status == WPC_IDLE_STAT_STANDBY)
			{
				if (gd->sys_err_code == ESYS_ERR_CODE_IDCFG_PHASE_MPP_RESTRICTED_REP)
				{
					gd->tx_infos.t_next_ping = 500;
				}
				else
				{
					gd->tx_infos.t_next_ping = ap->t_next_ping;
				}
			}
			else if (gd->ptx_idle_phase_status == WPC_IDLE_STAT_EPT_REP) //should set the ping type
			{
//				gd->tx_infos.t_next_ping = gd->tx_infos.t_re_ping;
				gd->tx_infos.t_next_ping = 100;
			}
			else if (gd->ptx_idle_phase_status == WPC_IDLE_STAT_EPT_RES && gd->tx_infos.power_mode_trans_eptr == 1)
			{
				gd->tx_infos.power_mode_trans_eptr = 0;
				gd->tx_infos.t_next_ping = 1000;
			}
			else
			{
				gd->tx_infos.t_next_ping = ap->t_next_ping;
			}
		}
		else
		{
			gd->tx_infos.t_next_ping = ap->t_next_ping;
		}
		gd->ptx_protocol_phase = WPC_PHASE_IDLE;

		osal_start_timerEx(WPC_PING_TIMER, gd->tx_infos.t_next_ping, ap->t_next_ping, WPC_TASK, WPC_EVT_DIG_PING);
	}

	osal_stop_timerEx(WPC_NEXT_TIMER);

//	gd->dig_ping_volt = 11000;
//	if (usb_pd_9v_flag)
//	{
//		USBPD_vSetVolt(9000);
//	}

	ctx_switch(4);//for qdt

	if (gd->pid_volt != gd->dig_ping_volt)
	{
		gd->pid_volt = gd->dig_ping_volt;
		fml_adp_volt_set(gd->pid_volt);
	}

	gd->rx_power = 0;
	gd->tx_power = 0;
	gd->atl_test_tpr1c_coil_flag = 0;
	gd->atl_test_ldstp_epp_N60 = 0;
	gd->atl_test_ldstp_bpp_N60 = 0;
	gd->atl_test_ldstp_bpp_P60 = 0;
	gd->alt_test_resv_rp8_cnt = 0;
	gd->alt_test_last_rp8_value = 0;
	gd->alt_test_1st_rp8_value = 0;
	gd->alt_test_continous_cnt = 0;

	special_cep_cnt = 0;

	osal_stop_timerEx(WPC_CEP_TIMER);
	osal_stop_timerEx(WPC_RPP_TIMER);

	printk("\r\n ---------------------power removed-> %02X %d %d", gd->sys_err_code, gd->pid_volt, gd->dig_ping_volt);
}

static void wpc_ept_pkt_process(struct com_prx_ask_pkt_t *com_ask)
{
	gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
	gd->tx_infos.flg_mode_cloak = FALSE;

	switch (com_ask->msg.ept.ept_code)
	{
		case EPT_CODE_00_Unknown:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_RES;
			break;
		case EPT_CODE_02_InternalFault:
		case EPT_CODE_01_ChargeComplete:
		case EPT_CODE_03_OverTemperature:
		case EPT_CODE_04_OverVoltage:
		case EPT_CODE_05_OverCurrent:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_XER_COM;
			break;
		case EPT_CODE_06_BatteryFailure:
		case EPT_CODE_07_NA:
		case EPT_CODE_09_NA:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_ERR;
			break;
		case EPT_CODE_08_NoResponse:
		case EPT_CODE_0A_NegotiationFailure:
			gd->tx_infos.ept_attempt_cnt++; //EPT_NR3
			gd->tx_infos.reping_cnt = 2;
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_RES;
			break;
		case EPT_CODE_0B_RestartPowerTransfer:
			gd->tx_infos.reping_cnt = 2;
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_RES;
			break;
		case EPT_CODE_0C_RePing:
			printk("\r\n re_ping_cnt: %d", gd->tx_infos.reping_cnt);
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_REP;
			break;
		default:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_EPT_ERR;
			break;
	}

	wpc_stop_to_idle(ESYS_ERR_CODE_RECVD_EPT_PKT);
}

static void wpc_ptx_end_nego_check(struct com_prx_ask_pkt_t *ask_pkt)
{
	if (gd->ptx_end_nego_event)
	{
		if (ask_pkt->hdr == WPC_PRx_PKT_TYP_SRQ_20 && ask_pkt->msg.srq.request == 0x00)
		{
			gd->ptx_protocol_phase = WPC_PHASE_NEGO;
			if (gd->ptx_end_nego_event & EPP_END_NEGO_FLAG)
			{
				gd->rx_infos.power_profile_mode = EPP;
				if (gd->ptx_protocol_phase == WPC_PHASE_XFER)
				{
					gd->nego_flag = 2;
				}
				else
				{
					gd->nego_flag = 1;
				}
			}
			if (gd->ptx_end_nego_event & MPP_END_NEGO_FLAG)
			{
				gd->rx_infos.power_profile_mode = MPP;
			}
		}
		gd->ptx_end_nego_event = 0;
	}
}

void wpc_protocol_sm(void)
{
	struct com_prx_ask_pkt_t *ask_pkt = (struct com_prx_ask_pkt_t *)&gd->wpc_pkt.data;

	if (ask_pkt->hdr == WPC_PRx_PKT_TYP_EPT_02)
	{
		wpc_ept_pkt_process(ask_pkt);
		return;
	}

	wpc_ptx_end_nego_check(ask_pkt);

	switch (gd->ptx_protocol_phase)
	{
		case WPC_PHASE_IDLE:
			break;
		case WPC_PHASE_PING:
			wpc_ping_phase_process(ask_pkt);
			break;
		case WPC_PHASE_CNFG:
			wpc_cnfg_phase_process(ask_pkt);
			break;
		case WPC_PHASE_NEGO:
			wpc_nego_phase_process(ask_pkt);
			break;
		case WPC_PHASE_XFER:
			wpc_xfer_phase_process(ask_pkt);
			break;
		case WPC_PHASE_CLOAK:
			wpc_cloak_phase_process(ask_pkt);
			break;
		default:
			break;
	}
}

void wpc_pkt_hdr_handler(void)
{
	if (gd->ptx_protocol_phase == WPC_PHASE_PING)
	{
//		osal_start_timerEx(WPC_NEXT_TIMER, T_FIRST_LIMIT, 0, WPC_TASK, WPC_EVT_PING_1st_PKT_TO);
		osal_start_timerEx(WPC_NEXT_TIMER, (gd->wpc_pkt.len - 1) * 10, 0, WPC_TASK, WPC_EVT_PING_1st_PKT_TO);
		printk("\r\n ping_xfer-> %02X %d %d", gd->wpc_pkt.hdr, (gd->wpc_pkt.len - 1) * 10, gd->wpc_pkt.src);
	}
	else if (gd->ptx_protocol_phase == WPC_PHASE_CNFG)
	{
//		osal_start_timerEx(WPC_NEXT_TIMER, T_MAX_LIMIT, 0, WPC_TASK, WPC_EVT_CNFG_NEXT_PKT_TO);
		osal_start_timerEx(WPC_NEXT_TIMER, (gd->wpc_pkt.len - 1) * 10, 0, WPC_TASK, WPC_EVT_PING_1st_PKT_TO);
		printk("\r\n cnfg_xfer-> %02X %d %d", gd->wpc_pkt.hdr, (gd->wpc_pkt.len - 1) * 10, gd->wpc_pkt.src);
	}
	else if ((gd->ptx_protocol_phase == WPC_PHASE_NEGO) || (WPC_PHASE_XFER == gd->ptx_protocol_phase))
	{
		if (gd->wpc_pkt.hdr == WPC_PRx_PKT_TYP_CE_03)
		{
			osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
		}

		if (gd->wpc_pkt.hdr == WPC_PRx_PKT_TYP_RP8_04 || gd->wpc_pkt.hdr == WPC_PRx_PKT_TYP_RP_31)
		{
			osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
		}
	}
	else if (WPC_PHASE_CLOAK == gd->ptx_protocol_phase)
	{
//		osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT_EX, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
//		printk("\r\n cloak_xfer-> %02X %d %d", gd->wpc_pkt.hdr, (gd->wpc_pkt.len - 1) * 10, gd->wpc_pkt.src);
		if (gd->wpc_pkt.hdr == 0x18 || gd->wpc_pkt.hdr == 0x28 || gd->wpc_pkt.hdr == 0x58)
		{
//			osal_start_timerEx(WPC_NEXT_TIMER, (gd->wpc_pkt.len - 1) * 10, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
			osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT_EX, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
			printk("\r\n cloak_xfer-> %02X %d %d", gd->wpc_pkt.hdr, (gd->wpc_pkt.len - 1) * 10, gd->wpc_pkt.src);
		}
	}
}

void wpc_task_init(void)
{
	osal_task_handler_reg(WPC_TASK, wpc_task_event_handler);
	osal_start_timerEx(WPC_PING_TIMER, ap->t_next_ping, ap->t_next_ping, WPC_TASK, WPC_EVT_DIG_PING);

#ifdef	_CLOAK_TX_INIT
	gd->tx_infos.flg_cloak_tx_init = TRUE;
#endif
}

void wpc_task_event_handler(uint32_t event)
{
	switch (event)
	{
		case WPC_EVT_CLOAK_PING:
			wpc_idle_cloak_phase_process();
			break;
		case WPC_EVT_DIG_PING:
			wpc_idle_phase_process();
			break;
		case WPC_EVT_PIN_NO_PKT:
		case WPC_EVT_PING_1st_PKT_TO:
			gd->tx_infos.flg_mode_cloak = FALSE;
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
			wpc_stop_to_idle(ESYS_ERR_CODE_PING_PHASE_WAIT_1ST_PKT_TIMEOUT);
			break;
		case WPC_EVT_CNFG_NEXT_1ST_TO:
			wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_WAIT_NEXT_PKT_ERR);
			break;
		case WPC_EVT_CNFG_NEXT_PKT_TO:
			wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_WAIT_NEXT_PKT_TIMEOUT);
			break;
		case WPC_EVT_CEP_TO:
			wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_CEP_TIMEOUT);
			break;
		case WPC_EVT_RPP_TO:
			wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_RPP_TIMEOUT);
			break;
		case WPC_EVT_PCH_TO:
			if (gd->rx_infos.cep_val == 5 && gd->isns < 150)
			{
				if (++special_cep_cnt >= 5)
				{
					special_cep_cnt = 5;
					gd->rx_infos.cep_val = 0;
				}
			}
			else
			{
				special_cep_cnt = 0;
			}
			pid_cep_handler(gd->rx_infos.cep_val);
			osal_start_timerEx(WPC_NEXT_TIMER, T_ACTIVE, 0, WPC_TASK, WPC_EVT_1ST_WND);// + gd->rx_infos.wnd_size
			break;
		case WPC_EVT_1ST_WND:
			osal_start_timerEx(WPC_NEXT_TIMER, T_WINDOW, 0, WPC_TASK, WPC_EVT_2ND_WND);
//			wpc_xfer_ptx_power_update();
			gd->isns_avg = hal_badc_meas(_BADC_CH_PD6_ADC3);
			gd->vpwr_avg = g_buckboost.adc_vbus;//hal_badc_meas(_BADC_CH_PB6_ADC7);

			uint8_t last_k = gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K;
			fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_CAP);
			fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K3);
			hal_eadc_meas(_EADC_CH_INR_VCAP);
			switch (last_k)
			{
				case _NU1030_DMO2_VCAP_RATIO_K1:
					fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K1);
					break;
				case _NU1030_DMO2_VCAP_RATIO_K2:
					fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K2);
					break;
				case _NU1030_DMO2_VCAP_RATIO_K3:
					fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K3);
					break;
				default:
					break;
			}
			fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
			break;
		case WPC_EVT_2ND_WND:
			osal_start_timerEx(WPC_NEXT_TIMER, T_WINDOW, 0, WPC_TASK, WPC_EVT_3RD_WND);
//			wpc_xfer_ptx_power_update();
			gd->isns_avg = (gd->isns_avg + hal_badc_meas(_BADC_CH_PD6_ADC3)) >> 1;
			gd->vpwr_avg = (gd->vpwr_avg + g_buckboost.adc_vbus) >> 1;//; hal_badc_meas(_BADC_CH_PB6_ADC7)
			break;
		case WPC_EVT_3RD_WND:
			osal_start_timerEx(WPC_NEXT_TIMER, T_WINDOW, 0, WPC_TASK, WPC_EVT_4TH_WND);
//			wpc_xfer_ptx_power_update();
			gd->isns_avg = (gd->isns_avg + hal_badc_meas(_BADC_CH_PD6_ADC3)) >> 1;
			gd->vpwr_avg = (gd->vpwr_avg + g_buckboost.adc_vbus) >> 1;//; hal_badc_meas(_BADC_CH_PB6_ADC7)
			break;
		case WPC_EVT_4TH_WND:
//			wpc_xfer_ptx_power_update();
			gd->isns_avg = (gd->isns_avg + hal_badc_meas(_BADC_CH_PD6_ADC3)) >> 1;
			gd->vpwr_avg = (gd->vpwr_avg + g_buckboost.adc_vbus) >> 1;//; hal_badc_meas(_BADC_CH_PB6_ADC7)
			gd->tx_power = gd->isns_avg * gd->vpwr_avg / 1000;
			printk("  vpwr:%d iavg:%d irms:%d imax:%d vctx:%d fo_exist:%d", gd->vpwr_avg, gd->isns_avg, gd->icol_rms, gd->icol_max, gd->vctx_pp,gd->tx_infos.fo_exist);
			break;
		case WPC_EVT_FSK_RESP_DONE:
//			if (gd->tx_infos.fsk_done_event & 1)
//			{
//				gd->tx_infos.fsk_done_event &= ~1;
//
//				if (gd->rx_infos.cep_val > 0 || gd->rx_infos.cep_val < 0)
//				{
//					osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
//				}
//				else
//				{
//					osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.wnd_size, 0, WPC_TASK, WPC_EVT_1ST_WND);
//				}
//			}

			if (gd->tx_infos.fsk_done_event & 2)
			{
				gd->tx_infos.fsk_done_event &= ~2;
				if (gd->rx_infos.power_profile_mode == MPP)
				{
					gd->fsk_cfg.polar = gd->fsk_cfg.polar;
					gd->fsk_cfg.depth = gd->fsk_cfg.depth;
					gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_128;
					gd->fsk_cfg.prmbl = FSK_PRMBL_NEED;
					fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
				}
			}

			if (gd->tx_infos.fsk_done_event & 4)
			{
				gd->tx_infos.fsk_done_event &= ~4;

				if (need_atn_evt != 0)
				{
					need_atn_cnt = 100;
//					ds_outgoing_stream_stop(1);
					ds_init();
				}

				osal_start_timerEx(WPC_NEXT_TIMER, T_TERMINATE, 0, WPC_TASK, WPC_EVT_STOP_AFTER_FSK);
				osal_start_timerEx(WPC_PING_TIMER, T_CLOAK_PING, T_CLOAK_PING, WPC_TASK, WPC_EVT_CLOAK_PING);
				printk("\r\n need stop after fsk");
			}

			if (gd->tx_infos.fsk_done_event & 8)
			{
				gd->tx_infos.fsk_done_event &= ~8;
				osal_set_event(WPC_TASK, WPC_EVT_FOD_REPORTED);
			}

			if (gd->tx_infos.fsk_done_event & 0x10)
			{
				gd->tx_infos.fsk_done_event &= ~0x10;
				osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO); //480 ms
			}

			if (gd->tx_infos.fsk_done_event & 0x20)
			{
				gd->tx_infos.fsk_done_event &= ~0x20;
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_QDT_FOD;
				osal_start_timerEx(WPC_NEXT_TIMER, T_TERMINATE, 0, WPC_TASK, WPC_EVT_STOP_AFTER_FSK);
			}
			break;
		case WPC_EVT_STOP_AFTER_FSK:
//			gd->ptx_idle_phase_status = WPC_IDLE_STAT_CLOAK_DET_PING;
			wpc_stop_to_idle(ESYS_ERR_CODE_CLOAK_SWITCH);
			break;
		case WPC_EVT_NEGO_NEXT_PKT_TO:
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
			wpc_stop_to_idle(ESYS_ERR_CODE_NEGO_PHASE_WAIT_NEXT_PKT_TIMEOUT);
			break;
		case WPC_EVT_PFOD:
			//pfod_common();
			//if (pfod_common())
		//	{
              //  gd->ptx_idle_phase_status = WPC_IDLE_STAT_XER_FOD;
			//	wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_POWER_LOSS_FOD);
			//	printk("\r\n FOD happen");
			//}
			if (pfod_common())
			{
                gd->ptx_idle_phase_status = WPC_IDLE_STAT_XER_FOD;
				wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_POWER_LOSS_FOD);
			}
			break;
		case WPC_EVT_HDR_START:
			break;
		case WPC_EVT_HDR_RECVD:
			wpc_pkt_hdr_handler();
			break;
		case WPC_EVT_PKT_RECVD:
			wpc_pkt_print();
			wpc_protocol_sm();
			break;
		case WPC_EVT_STOP_POWER:
			wpc_stop_power();
			break;
		case WPC_EVT_SE_IC_TBS_AUTH:
			need_atn_cnt = 100;
			if (ap->auth_seic_type == 1)
			{
				t91206_get_tbs_auth(array_chall, adt_data_recv_buf + 2);
			}
			else
			{
				fm1210_get_tbs_auth(array_chall);
			}
//			printk("\r\n tbs_hash:");
//			for (int i=0; i<64; i++)
//			{
//				printk(" %02X", array_chall[i]);
//			}
//			printk("\r\n");
			break;
		case WPC_EVT_FOD_REPORTED:
			pfod_log_print();
			break;
		case WPC_EVT_RENEGO_TO:
			wpc_stop_to_idle(ESYS_ERR_CODE_RENOGO_PHASE_TIMEOUT_ERR);
			break;
		default:
			break;
	}
}
