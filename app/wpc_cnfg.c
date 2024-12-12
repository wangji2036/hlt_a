#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "osal.h"
#include "delay.h"
#include "qdt.h"
#include "pid.h"
#include "pfod.h"
#include "fsk.h"
#include "_wpc.h"
#include "epp.h"
#include "pkt_type.h"
#include "wpc_ping.h"
#include "wpc_cnfg.h"
#include "debug.h"


static uint8_t is_cnfg_phase_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x01 || hdr == 0x02 || hdr == 0x03 || hdr == 0x04 || hdr == 0x05 || hdr == 0x07 || hdr == 0x09 || hdr == 0x15 || hdr == 0x16 || hdr == 0x17 ||
    	hdr == 0x20 || hdr == 0x22 || hdr == 0x25 || hdr == 0x26 || hdr == 0x27 || hdr == 0x31 || hdr == 0x36 || hdr == 0x37 || hdr == 0x46 || hdr == 0x47 ||
    	hdr == 0x54 || hdr == 0x55 || hdr == 0x56 || hdr == 0x57 || hdr == 0x66 || hdr == 0x67 || hdr == 0x71 || hdr == 0x76 || hdr == 0x77 || hdr == 0x81)
    {
    	return 1;
    }
	return 0;
}

static uint8_t get_prx_type(uint16_t prmc)
{
	switch (prmc)
	{
		case 0x0042:
			gd->rx_infos.rx_type = EPRX_TYPE_SAMSUNG;
			break;
		case 0x0056:
			if (gd->rx_infos.device_id == 0x44035445 && gd->rx_infos.qi_version == 0x12)//YBZ BPP RX FIXTURE 71 12 00 56 44 03 54 45 63
			{
				gd->rx_infos.rx_type = ERX_TYPE_YBZ_BPP_FIXTURE;
			}
			break;
		case 0x0058:
			if (gd->rx_infos.device_id == 0x33035445 && gd->rx_infos.qi_version == 0x12)//YBZ EPP RX FIXTURE 71 12 00 56 44 03 54 45 63
			{
				gd->rx_infos.rx_type = ERX_TYPE_YBZ_EPP_FIXTURE;
			}
			break;
		case 0x425A:
			if (gd->rx_infos.device_id == 0x00035464 && gd->rx_infos.qi_version == 0x59)//YBZ PPDE RX FIXTURE 71 59 42 5A 00 03 54 64 03
			{
				gd->rx_infos.rx_type = ERX_TYPE_YBZ_PPDE_FIXTURE;
			}
			break;
		case 0x005A:
			gd->rx_infos.rx_type = EPRX_TYPE_APPLE_STD;
			if (gd->rx_infos.device_id == 0xEE54986A && gd->rx_infos.qi_version == 0x20)//YBZ MPP RX FIXTURE 71 20 00 5A EE 54 98 6A 43
			{
				gd->rx_infos.rx_type = ERX_TYPE_YBZ_MPP_FIXTURE;
			}
			break;
		case 0x005C:
			gd->rx_infos.rx_type = EPRX_TYPE_NUVOLTA;
			if (gd->rx_infos.device_id == 0x804A444D && gd->rx_infos.qi_version == 0x20)//NVT JDM RX FIXTURE 71 20 00 5C 80 4A 44 4D CE
			{
				gd->rx_infos.rx_type = ERX_TYPE_NVT_MPP;
			}
			break;
		case 0x0059:
			gd->rx_infos.rx_type = EPRX_TYPE_MEIZU;
			break;
		case 0x0060:
		case 0x0105:
			gd->rx_infos.rx_type = EPRX_TYPE_HUAWEI;
			break;
		case 0x006E:
			gd->rx_infos.rx_type = EPRX_TYPE_XIAOMI_BPP;
			break;
		case 0x0072:
			gd->rx_infos.rx_type = EPRX_TYPE_GOOGLE;
			break;
		default:
			gd->rx_infos.rx_type = EPRX_TYPE_UNKNOWN;
			break;
	}

	return 0;
}

void wpc_cnfg_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (com_ask->hdr)
	{
		case WPC_PRx_PKT_TYP_ID_71:
			if (gd->rx_infos.opt_cnt == 0)
			{
				gd->rx_infos.qi_version = com_ask->msg.id.major_ver << 4 | com_ask->msg.id.minor_ver;
				gd->rx_infos.prmc = com_ask->msg.id.prmc_msb << 8 | com_ask->msg.id.prmc_lsb;
				gd->rx_infos.device_id = com_ask->msg.id.bdid0_msb << 24 | com_ask->msg.id.bdid0_lsb << 16 | com_ask->msg.id.bdid1_msb << 8 | com_ask->msg.id.bdid1_lsb;
				gd->rx_infos.pch_t_delay = T_PCH_TIME_MIN;
				get_prx_type(gd->rx_infos.prmc);
				
				if (gd->rx_infos.qi_version >= 0x20 && (gd->rx_infos.device_id & PRX_ID71_PKT_EXT_BIT_MSK))//add ext bit
				{
					if (gd->pid_perd != 144000/360)
					{
						if (gd->pid_duty < 250)
						{
							gd->pid_duty = 250;
							hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
						}
					}
				}

				//MPP_25W_POWER_MODE
				if (gd->rx_infos.qi_version < 0x20)
				{
					gd->power_mode = light;
				}
				else if (gd->rx_infos.qi_version < 0x22)
				{
					gd->power_mode = nominal;
				}
				if (gd->rx_infos.rx_type == ERX_TYPE_NVT_MPP)
				{
//					gd->power_mode = high;//fixture test
					gd->power_mode = nominal;
				}
			}
			else
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR);
				goto __CNFG_PHASE_ERR__;
			}
			++gd->rx_infos.opt_cnt;
			break;
		case WPC_PRx_PKT_TYP_XID_81:
			if (gd->rx_infos.opt_cnt == 1 && (gd->rx_infos.device_id & PRX_ID71_PKT_EXT_BIT_MSK))
			{
				++gd->rx_infos.opt_cnt;
				if (gd->rx_infos.rx_type == EPRX_TYPE_APPLE_STD)
				{
					gd->rx_infos.rx_type = EPRX_TYPE_APPLE_MAG;
				}

				if (com_ask->msg.xid.selector == 0xFE)
				{
					gd->rx_infos.power_profile_mode = MPP;

					if (gd->pid_perd != 144000/360)
					{
						int32_t temp = 0;

						uint32_t alpha0 = com_ask->msg.xid.alpha0_rx;
						uint32_t alpha1 = com_ask->msg.xid.alpha0_rx;
						uint32_t vrect = 20 * (com_ask->msg.xid.vrect_msb << 8 | com_ask->msg.xid.vrect_lsb);

						gd->vpwr = hal_badc_meas(_BADC_CH_PD0_ADC8);
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

						temp = vrect * alpha0;
						temp /= (gd->vctx_pp + gd->vpwr);
						temp *= 15926;
						temp += (1043 * alpha1);
						temp /= 100;

						gd->k_est = temp*107/100;

						printk(" {%d,%d,%d,%d,%d,k=%d}", alpha0, alpha1, vrect, gd->vpwr, gd->vctx_pp, gd->k_est);

						gd->tx_infos._128_nego_gd = 1;
					}
					else
					{
						gd->tx_infos._128_nego_gd = 0;
					}

					gd->rx_infos.mpp_restricted_mode = com_ask->msg.xid.resticted;

					if (gd->rx_infos.mpp_restricted_mode)
					{
						if (gd->pid_perd != 144000/360)
						{
							gd->tx_infos.dig_ping_type = _360K_FB;//OK, restricted mode reping
							gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
							wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_MPP_RESTRICTED_REP);
							goto __CNFG_PHASE_ERR__;
						}
					}
				}
			}
			else
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR);
				goto __CNFG_PHASE_ERR__;
			}
			break;
		case WPC_PRx_PKT_TYP_PCH_06:
			if (gd->rx_infos.opt_cnt == 0 || (gd->rx_infos.opt_cnt == 1 && (gd->rx_infos.device_id & PRX_ID71_PKT_EXT_BIT_MSK)))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR);
				goto __CNFG_PHASE_ERR__;
			}
			gd->rx_infos.pch_t_delay = com_ask->msg.pch.pch_time;
			if (((gd->rx_infos.qi_version >= 0x13) && (gd->rx_infos.pch_t_delay < T_PCH_TIME_MIN || gd->rx_infos.pch_t_delay > T_PCH_TIME_MAX)) ||
			    (gd->rx_infos.pch_t_delay < T_PCH_TIME_MIN || gd->rx_infos.pch_t_delay > 205))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PCH_PKT_TIME_ERR);
				goto __CNFG_PHASE_ERR__;
			}
			++gd->rx_infos.opt_cnt;
			break;
		case WPC_PRx_PKT_TYP_CFG_51:
			--gd->rx_infos.opt_cnt; //0x71
			if (gd->rx_infos.device_id & PRX_ID71_PKT_EXT_BIT_MSK) //0x81
			{
				--gd->rx_infos.opt_cnt;
			}
			if (com_ask->msg.cfg.count != gd->rx_infos.opt_cnt)
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_CFG_PKT_CNT_ERR);
				goto __CNFG_PHASE_ERR__;
			}

			gd->rx_infos.neg = com_ask->msg.cfg.is_nego;
			gd->rx_infos.max_power = com_ask->msg.cfg.max_power;
			gd->rx_infos.wnd_size = com_ask->msg.cfg.wind_size << 2;
			gd->rx_infos.wnd_size = 16;
			gd->rx_infos.fsk_param = com_ask->msg.cfg.fsk_pol << 2 | com_ask->msg.cfg.fsk_dep;

			gd->fsk_cfg.polar = com_ask->msg.cfg.fsk_pol;
			gd->fsk_cfg.depth = com_ask->msg.cfg.fsk_dep;
			gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_512;
			gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;
			fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
			printk("\r\n --------------> %d %d %d", gd->rx_infos.power_profile_mode, gd->adp.pwr_high, gd->rx_infos.mpp_restricted_mode);
			if (gd->rx_infos.power_profile_mode == MPP &&/* gd->adp.pwr_high >= 20 && */0 == gd->rx_infos.mpp_restricted_mode)
			{
				gd->rx_infos.opt_cnt = 0;
				gd->rx_infos.phase_state = 0;
				gd->rx_infos.gant_power_temp = gd->rx_infos.guaranteed_power;
				gd->rx_infos.max_power_temp = gd->rx_infos.max_power;
				if (gd->pid_perd != 144000/360)
				{
					gd->fsk_cfg.polar = 1;
					gd->fsk_cfg.depth = 2;
					gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_512;
					gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;
					fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
				}
				else
				{
					gd->fsk_cfg.polar = 0;
					gd->fsk_cfg.depth = 0;
					gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_512;
					gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;
					fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
				}
				gd->rx_infos.power_profile_mode = MPP;
				gd->ptx_protocol_phase = WPC_PHASE_NEGO;

				gd->tx_infos.fsk_done_event = 2;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_MPP);
				osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
goto __CNFG_PHASE_ERR__;
			}
			else if (gd->rx_infos.qi_version >= 0x12 && gd->rx_infos.neg == 1 && gd->adp.pwr_high >= 20
					&& (gd->tx_infos.master_adaptor_cap != 1)) //EPP before negotiation send ACK to power receiver
			{
				if (com_ask->msg.cfg.max_power > 10)
				{
					//error
					gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				}

				gd->rx_infos.opt_cnt = 0;
				gd->rx_infos.phase_state = 0;
				gd->rx_infos.gant_power_temp = gd->rx_infos.guaranteed_power;;
				gd->rx_infos.max_power_temp = gd->rx_infos.max_power;

				gd->rx_infos.power_profile_mode = EPP; 
				gd->ptx_protocol_phase = WPC_PHASE_NEGO; 	//negotiate

				gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_512;
				gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;

				initializePTC();

				fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);

				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
goto __CNFG_PHASE_ERR__;
			}
			else
			{
				if (gd->rx_infos.mpp_restricted_mode)
				{
					gd->fsk_cfg.polar = 0;
					gd->fsk_cfg.depth = 0;
					gd->fsk_cfg.cycle = _FSK_BIT_CYCLES_512;
					gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;

					fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
					gd->tx_infos.fsk_done_event = 2;
					fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_MPP);
					osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
					osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
				}
				else
				{
//					if (gd->rx_infos.rx_type != EPRX_TYPE_APPLE_STD && gd->rx_infos.rx_type != EPRX_TYPE_APPLE_MAG)
					{
						pid_set_freq_limit(144000000/90000, 144000000/147000, 144000000/147000);
					}
					if (gd->rx_infos.ssp_value > 180 && gd->rx_infos.qi_version >= 0x20) //for IOC test
					{
						pid_set_freq_limit(144000000/90000, 144000000/147000, 144000000/180000);
					}

					//even MPP/EPP, but input power is limit, enter BPP
					//BPP
//					gd->rx_infos.power_profile_mode = BPP; //if MPP, force to BPP
					osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
					osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
				}
				gd->ptx_protocol_phase = WPC_PHASE_XFER;
				osal_stop_timerEx(WPC_NEXT_TIMER);
				goto __CNFG_PHASE_ERR__;
			}
			break;
		default:
			if (is_cnfg_phase_illegal_pkt(com_ask->hdr))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_NO_THIS_PKT);
				goto __CNFG_PHASE_ERR__;
			}

			if (gd->rx_infos.opt_cnt == 0)
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR);
				goto __CNFG_PHASE_ERR__;
			}

			if (gd->rx_infos.opt_cnt == 1 && (gd->rx_infos.device_id & PRX_ID71_PKT_EXT_BIT_MSK))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR);
				goto __CNFG_PHASE_ERR__;
			}

			++gd->rx_infos.opt_cnt;
			break;
	}

	osal_start_timerEx(WPC_NEXT_TIMER, T_NEXT + 50, 0, WPC_TASK, WPC_EVT_CNFG_NEXT_1ST_TO);

__CNFG_PHASE_ERR__:
	return;
}
