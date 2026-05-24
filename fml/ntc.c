#include "regdef.h"
#include "buckboost.h"
#include "nu6805.h"
#include "nu6801.h"
#include "printk.h"
#include "tcpm.h"
#include "pdlib.h"
#include "port_manager.h"
#include "config.h"
#include "tcpm.h"
#include "g_data.h"
#include "ntc.h"
#include "_fml.h"

bool bat_ntc_charge_ut_reduce5W_flag = false;
bool bat_ntc_charge_ot_reduce12W_flag = false;
// bool bat_ntc_stop_chrg_flag = false;
bool typec_ntc_lock = false;
bool typec_charge_ntc_lock = false;
bool wirless_ntc_lock = false;
// bool bat_ntc_lock_flag = false;
bool typec_ntc_dischg_ot_reduce20W_flag = false;
bool typec_ntc_charge_ot_reduce20W_flag = false;
// bool bat_ntc_dischg_lock = false;
bool bat_ntc_dual_dischg_inhibit = false; // C 口+无线充同时放电温度抑制：<0/≥45 锁，[5,40] 恢复 max 5V/3A
bool bat_ntc_prot_reverse = false;
uint8_t bat_low_volt_reduce = 0;
extern const uint32_t source_pdo[];
extern const uint32_t source_pdo_ntc[];

#if (BUCKBOOST_USED_NU6805 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)

void buckboost_ntc_handle(void)
{
	static uint8_t bat_ntc_ut_cnt = 0;
	static uint8_t bat_ntc_ot_cnt = 0;
	static uint8_t ntc_stop_chg_cnt = 0;
	static uint8_t typec_ntc_ot_dischg_cnt = 0;
	static uint8_t typec_ntc_ot_chrg_cnt = 0;
	static uint8_t dual_dischg_lock_cnt = 0;
	static uint8_t typec_ntc_lock_cnt = 0;
	static uint8_t typec_chrg_lock_cnt = 0;
	static uint8_t bat_dischg_reduce_cnt = 0;
	static uint8_t dual_dischg_inhibit_cnt = 0;
	static uint8_t bat_low_volt_cnt = 0;
	static bool ot_full_stop = false; // 4.1V 停充闭锁：在 OT 区间 vbat≥8200 触发，OT 解时清；vbat 回落不解锁
	int16_t bat_temp = ntc_to_temp(g_buckboost.adc_tbat1);
	// printk("bat_temp = %d , ntc_temp_typec = %d",bat_temp,gd->sys_infos.ntc_temp_typec);
	{
		// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		// {
			// 43-52°C 区间 vbat≥8.2V(4.1V/cell) 触发停充并闭锁；vbat 回落到 4.1V 以下不解，
			// 必须等温度降到 ≤30°C 才解
			if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
			{
				gd->bat_ntc_dischg_lock = 0;
				if (bat_ntc_charge_ot_reduce12W_flag && g_buckboost.adc_vbat >= 8200)
				{
					ot_full_stop = true;
				}
				if (bat_temp <= 30)
				{
					ot_full_stop = false;
				}
			}
			// 充电禁充：<3°C 锁 / ≥5°C 解；>52°C 锁 / ≤47°C 解；43-52°C 段满 4.1V 闭锁
			if (gd->bat_ntc_stop_chrg_flag == 0)
			{
				if (bat_temp > 52 || bat_temp < 3 || ot_full_stop)
				{
					ntc_stop_chg_cnt++;
					if (ntc_stop_chg_cnt >= 5)
					{
						ntc_stop_chg_cnt = 0;
						// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
						{
							ntc_stop_chg_cnt = 0;
							gd->bat_ntc_stop_chrg_flag = 1;
							port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						}
					}
				}
				else
				{
					ntc_stop_chg_cnt = 0;
				}
			}
			else
			{
				if (bat_temp >= 5 && bat_temp <= 47 && !ot_full_stop)
				{
					ntc_stop_chg_cnt++;
					if (ntc_stop_chg_cnt >= 5)
					{
						ntc_stop_chg_cnt = 0;
						// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
						{
							gd->bat_ntc_stop_chrg_flag = 0;
							gd->flash_times = 0;
							port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						}
					}
				}
				else
				{
					ntc_stop_chg_cnt = 0;
				}
			}

			if (gd->bat_ntc_stop_chrg_flag == 0)
			{
				// 充电限 5W：<18°C 触发，≥20°C 恢复 30W
				if (!bat_ntc_charge_ut_reduce5W_flag)
				{
					if (bat_temp < 18)
					{
						if (bat_ntc_ut_cnt++ > 5)
						{
							bat_ntc_ut_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_ntc_charge_ut_reduce5W_flag = 1;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_ntc_ut_cnt = 0;
					}
				}
				else
				{
					if (bat_temp >= 20)
					{
						if (bat_ntc_ut_cnt++ > 5)
						{
							bat_ntc_ut_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_ntc_charge_ut_reduce5W_flag = 0;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_ntc_ut_cnt = 0;
					}
				}

				// 充电限 12W：≥43°C 触发，≤30°C 恢复 30W
				if (!bat_ntc_charge_ot_reduce12W_flag)
				{
					if (bat_temp >= 43)
					{
						if (bat_ntc_ot_cnt++ > 5)
						{
							bat_ntc_ot_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_ntc_charge_ot_reduce12W_flag = 1;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_ntc_ot_cnt = 0;
					}
				}
				else
				{
					if (bat_temp <= 30)
					{
						if (bat_ntc_ot_cnt++ > 5)
						{
							bat_ntc_ot_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_ntc_charge_ot_reduce12W_flag = 0;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_ntc_ot_cnt = 0;
					}
				}
				if (!bat_low_volt_reduce)
				{
					if (g_buckboost.adc_vbat < 7400)
					{
						if (bat_low_volt_cnt++ > 5)
						{
							bat_low_volt_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_low_volt_reduce = 1;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_low_volt_cnt = 0;
					}
				}
				else
				{
					if (g_buckboost.adc_vbat >= 7800)
					{
						if (bat_low_volt_cnt++ > 5)
						{
							bat_low_volt_cnt = 0;
							// if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
							if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK)
							{
								bat_low_volt_reduce = 0;
								port_manager_set_event(PORT_EVENT_RESET_CHARGE);
							}
						}
					}
					else
					{
						bat_low_volt_cnt = 0;
					}
				}
			}
		// }
		//end
		if (g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
		{
			gd->bat_ntc_stop_chrg_flag = 0;
			bat_ntc_charge_ot_reduce12W_flag = 0;

			// 放电锁：≤-15°C 或 ≥55°C 锁，[-10, 50] 解锁
			{
				static uint8_t ntc_dbg_cnt = 0;
				if (++ntc_dbg_cnt >= 30) {
					ntc_dbg_cnt = 0;
					printk("[NTC] bat_t=%d dlock=%d cnt=%d ihport=%d pst=%d klf2=%d\n",
					       (int)bat_temp,
					       gd->bat_ntc_dischg_lock,
					       dual_dischg_lock_cnt,
					       g_port.inhandle_port,
					       g_port.port_state[g_port.inhandle_port],
					       gd->key_led_fault2);
				}
			}
			if (gd->bat_ntc_dischg_lock == 0)
			{
                if (bat_temp <= -15 || bat_temp >= 55)
				{
					if (dual_dischg_lock_cnt++ >= 5)
					{
						dual_dischg_lock_cnt = 0;
						if (g_port.port_state[g_port.inhandle_port] != PORT_STATE_NONE)
						{
							gd->bat_ntc_dischg_lock = 1;
						}
						else
						{
							gd->key_led_fault2 = 1;
						}
						
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
			}
			else
			{
				if (bat_temp >= -10 && bat_temp <= 50)
				{
					if (dual_dischg_lock_cnt++ >= 5)
					{
						dual_dischg_lock_cnt = 0;
						gd->bat_ntc_dischg_lock = 0;
						gd->air_protect_ntc1 = 0;
						printk("NTC_CLR klf2 was %d cnt=%d bat_t=%d\r\n", gd->key_led_fault2, dual_dischg_lock_cnt, (int)bat_temp);
						gd->key_led_fault2 = 0;
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
			}

			// 放电限 10W (PD 5V 2A)：≤-3°C 或 ≥43°C 触发，[0, 30] 恢复 30W
			if (!gd->bat_ntc_cport_dischg_reduce_flag)
			{
				if (bat_temp <= -3 || bat_temp >= 43)
				{
					if (bat_dischg_reduce_cnt++ > 10)
					{
						bat_dischg_reduce_cnt = 0;
						gd->bat_ntc_cport_dischg_reduce_flag = 1;
					}
				}
				else
				{
					bat_dischg_reduce_cnt = 0;
				}
			}
			else
			{
				if (bat_temp >= 0 && bat_temp <= 30)
				{
					if (bat_dischg_reduce_cnt++ > 10)
					{
						bat_dischg_reduce_cnt = 0;
						gd->bat_ntc_cport_dischg_reduce_flag = 0;
					}
				}
				else
				{
					bat_dischg_reduce_cnt = 0;
				}
			}

			// 同放温度抑制：仅当 C 口 + 无线充同时 SOURCE 才判定
			// <0°C 或 ≥45°C 禁止同放（仅留 C 口），[5, 40] 恢复同放（max 5V/3A）
			// if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE)
			// {
			if (!bat_ntc_dual_dischg_inhibit)
			{
				if (bat_temp < 0 || bat_temp >= 45)
				{
					if (dual_dischg_inhibit_cnt++ >= 5)
					{
						dual_dischg_inhibit_cnt = 0;
						bat_ntc_dual_dischg_inhibit = 1;
						ntc_printk("\r\nbat_ntc_dual_dischg_inhibit=1 trigger\n");
					}
				}
				else
				{
					dual_dischg_inhibit_cnt = 0;
				}
			}
			else
			{
				if (bat_temp >= 5 && bat_temp <= 40)
				{
					if (dual_dischg_inhibit_cnt++ >= 5)
					{
						dual_dischg_inhibit_cnt = 0;
						bat_ntc_dual_dischg_inhibit = 0;
						ntc_printk("\r\nbat_ntc_dual_dischg_inhibit=0 trigger\n");
					}
				}
				else
				{
					dual_dischg_inhibit_cnt = 0;
				}
			}
			// }
			// else
			// {
			// 	dual_dischg_inhibit_cnt = 0;
			// }
		}
	}
	//typec
	{
		if (g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
		{
			if (!typec_ntc_lock)
			{
				if (gd->sys_infos.ntc_temp_typec > 105 || gd->sys_infos.ntc_temp_typec < 10)
				{
					if (typec_ntc_lock_cnt++ >= 5)
					{
						typec_ntc_lock_cnt = 0;
						typec_ntc_lock = 1;
						gd->recharge_flag = 1;
					}
				}
				else
				{
					typec_ntc_lock_cnt = 0;
				}
			}
			else
			{
				// 105°C 锁解：温度回落到 ≤80°C（OT 仍在 20W 降功率档，到 35°C 才彻底恢复）
				if (gd->sys_infos.ntc_temp_typec <= 80 && gd->sys_infos.ntc_temp_typec > 15)
				{
					if (typec_ntc_lock_cnt++ >= 5)
					{
						typec_ntc_lock_cnt = 0;
						gd->ntc_led_off = 1;
						typec_ntc_lock = 0;
						gd->recharge_flag = 0;
					}
				}
				else
				{
					typec_ntc_lock_cnt = 0;
				}
			}

			// 放电 OT 限 20W (最大 12V)：≥80°C 触发，<35°C 恢复 30W
			if (!typec_ntc_dischg_ot_reduce20W_flag)
			{
				if (gd->sys_infos.ntc_temp_typec >= 80)
				{
					if (typec_ntc_ot_dischg_cnt++ > 10)
					{
						typec_ntc_dischg_ot_reduce20W_flag = 1;
					}
				}
				else
				{
					typec_ntc_ot_dischg_cnt = 0;
				}
			}
			else
			{
				if (gd->sys_infos.ntc_temp_typec < 35)
				{
					if (typec_ntc_ot_dischg_cnt++ > 10)
					{
						typec_ntc_dischg_ot_reduce20W_flag = 0;
					}
				}
				else
				{
					typec_ntc_ot_dischg_cnt = 0;
				}
			}
		}
		if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if (!typec_charge_ntc_lock)
			{
				if (gd->sys_infos.ntc_temp_typec > 105 || gd->sys_infos.ntc_temp_typec < 10)
				{
					if (typec_chrg_lock_cnt++ >= 5)
					{
						typec_chrg_lock_cnt = 0;
						typec_charge_ntc_lock = 1;
					}
				}
				else
				{
					typec_chrg_lock_cnt = 0;
				}
			}
			else
			{
				if (gd->sys_infos.ntc_temp_typec <= 68 && gd->sys_infos.ntc_temp_typec > 15)
				{
					if (typec_chrg_lock_cnt++ >= 5)
					{
						typec_chrg_lock_cnt = 0;
						typec_charge_ntc_lock = 0;
					}
				}
				else
				{
					typec_chrg_lock_cnt = 0;
				}
			}
			if (!typec_charge_ntc_lock)
			{
				if (!typec_ntc_charge_ot_reduce20W_flag)
				{
					if (gd->sys_infos.ntc_temp_typec > 68)
					{
						if (typec_ntc_ot_chrg_cnt++ > 5)
						{
							typec_ntc_charge_ot_reduce20W_flag = 1;
							port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						}
					}
					else
					{
						typec_ntc_ot_chrg_cnt = 0;
					}
				}
				else
				{
					if (gd->sys_infos.ntc_temp_typec < 30)
					{
						if (typec_ntc_ot_chrg_cnt++ > 5)
						{
							typec_ntc_charge_ot_reduce20W_flag = 0;
							port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						}
					}
					else
					{
						typec_ntc_ot_chrg_cnt = 0;
					}
				}
			}
		}
	}

	ntc_printk("\r\n[NTC_FLAG] tbat=%d mode=%d chg[stop=%d ut5=%d ot12=%d otfull=%d lowv=%d] dischg[lock=%d cport=%d dual=%d] tc_chg[lock=%d ot20=%d] tc_disc[lock=%d ot20=%d]",
	           bat_temp, g_buckboost.woke_mode,
	           gd->bat_ntc_stop_chrg_flag, bat_ntc_charge_ut_reduce5W_flag, bat_ntc_charge_ot_reduce12W_flag, ot_full_stop, bat_low_volt_reduce,
	           gd->bat_ntc_dischg_lock, gd->bat_ntc_cport_dischg_reduce_flag, bat_ntc_dual_dischg_inhibit,
	           typec_charge_ntc_lock, typec_ntc_charge_ot_reduce20W_flag,
	           typec_ntc_lock, typec_ntc_dischg_ot_reduce20W_flag);
}

#endif

uint8_t wpc_ntc_power_reduce_flag = 0;

void wpc_power_handle(int16_t tntc, int16_t tbat)
{
	static uint8_t otp_cnt = 0;
	static uint8_t otp_rec_cnt = 0;
	static uint8_t reduce_cnt = 0;
	static uint8_t reduce_rec_cnt = 0;
	static uint8_t bat_lock_cnt = 0;
	static uint8_t bat_lock_rec_cnt = 0;
	static uint8_t bat_reduce_cnt = 0;
	static uint8_t bat_reduce_rec_cnt = 0;

	// 75°C OTP + NTC开路(<=2)保护
	if (!wirless_ntc_lock)
	{
		if (tntc >= 75 || tntc <= -10)
		{
			if (++otp_cnt >= 5)
			{
				otp_cnt = 0;
				wirless_ntc_lock = 1;
				gd->wpc_disable = 1;
				tcpm_stop_wpc(WPC_DELAY);
				ntc_printk("\r\n[WPC_NTC] OTP lock tntc=%d", tntc);
			}
		}
		else
		{
			otp_cnt = 0;
		}
	}
	else
	{
		// 65°C解锁，65>43故wpc_ntc_power_reduce_flag仍置位→恢复7.5W放电
		if (tntc < 65 && tntc > 5)
		{
			if (++otp_rec_cnt >= 5)
			{
				otp_rec_cnt = 0;
				wirless_ntc_lock = 0;
				gd->wpc_disable = 0;
				ntc_printk("\r\n[WPC_NTC] OTP unlock tntc=%d", tntc);
			}
		}
		else
		{
			otp_rec_cnt = 0;
		}
	}

	// 43°C降7.5W，30°C恢复15W
	if (!wpc_ntc_power_reduce_flag)
	{
		if (tntc >= 43)
		{
			if (++reduce_cnt > 10)
			{
				reduce_cnt = 0;
				wpc_ntc_power_reduce_flag = 1;
				ntc_printk("\r\n[WPC_NTC] power reduce tntc=%d", tntc);
			}
		}
		else
		{
			reduce_cnt = 0;
		}
	}
	else
	{
		if (tntc <= 30)
		{
			if (++reduce_rec_cnt > 10)
			{
				reduce_rec_cnt = 0;
				wpc_ntc_power_reduce_flag = 0;
				bat_ntc_prot_reverse = 1;
				ntc_printk("\r\n[WPC_NTC] power restore tntc=%d", tntc);
			}
		}
		else
		{
			reduce_rec_cnt = 0;
		}
	}

	// 仅放电模式才处理电池NTC
	if (g_buckboost.woke_mode != BUCKBOOST_DISCHG_MODE)
	{
		// bat_ntc_lock_flag = 0;
		gd->bat_ntc_wpc_dischg_reduce_flag = 0;
		bat_lock_cnt = 0;
		bat_lock_rec_cnt = 0;
		bat_reduce_cnt = 0;
		bat_reduce_rec_cnt = 0;
		return;
	}
#if 0
	// // 电池NTC保护：tbat<=-15°C或>=55°C锁，回到(-10,50)区间解锁
	// if (!gd->bat_ntc_lock_flag)
	// {
	// 	if (tbat <= -15 || tbat >= 55)
	// 	{
	// 		if (++bat_lock_cnt >= 5)
	// 		{
	// 			bat_lock_cnt = 0;
	// 			gd->bat_ntc_lock_flag = 1;
	// 			tcpm_stop_wpc(WPC_DELAY);
	// 			ntc_printk("\r\n[BAT_NTC] lock tbat=%d", tbat);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		bat_lock_cnt = 0;
	// 	}
	// }
	// else
	// {
	// 	// -10°C/50°C解锁，解锁后tbat仍在降功率区间→自然落到7.5W
	// 	if (tbat > -10 && tbat < 50)
	// 	{
	// 		if (++bat_lock_rec_cnt >= 5)
	// 		{
	// 			bat_lock_rec_cnt = 0;
	// 			gd->bat_ntc_lock_flag = 0;
	// 			gd->ntc_led_off = 1;
	// 			ntc_printk("\r\n[BAT_NTC] unlock tbat=%d", tbat);
	// 		}
	// 	}
	// 	else
	// 	{
	// 		bat_lock_rec_cnt = 0;
	// 	}
	// 	return;
	// }
#endif
	// 电池NTC降功率：tbat>=43°C或<=-3°C降7.5W，回到[0,30]区间恢复15W
	if (!gd->bat_ntc_wpc_dischg_reduce_flag)
	{
		if (tbat >= 43 || tbat <= -3)
		{
			if (++bat_reduce_cnt > 10)
			{
				bat_reduce_cnt = 0;
				gd->bat_ntc_wpc_dischg_reduce_flag = 1;
				ntc_printk("\r\n[BAT_NTC] dischg reduce tbat=%d", tbat);
			}
		}
		else
		{
			bat_reduce_cnt = 0;
		}
	}
	else
	{
		if (tbat <= 30 && tbat >= 0)
		{
			if (++bat_reduce_rec_cnt > 10)
			{
				bat_reduce_rec_cnt = 0;
				gd->bat_ntc_wpc_dischg_reduce_flag = 0;
				bat_ntc_prot_reverse = 1;
				ntc_printk("\r\n[BAT_NTC] dischg restore tbat=%d", tbat);
			}
		}
		else
		{
			bat_reduce_rec_cnt = 0;
		}
	}
}

/* C 口在 SOURCE 时，bat_ntc_dual_dischg_inhibit（<0 / ≥45）禁止同时放电，仅留 C 口
	 * 极端温度锁（≤-15 / ≥55）由 bat_ntc_dischg_lock 走 buckboost.c VBUS_FAULT_VBUS_NTC 硬锁路径 */
void wpc_typec_cowork_otputpcheck(void)
{
	// static uint8_t wpc_dual_temp_lock = 0;
	// if (!wpc_dual_temp_lock)
	// {
	if (bat_ntc_dual_dischg_inhibit && (g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE))
	{
		gd->wpc_disable = 0x01;
		tcpm_stop_wpc(WPC_DELAY);
		tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
		ntc_printk("\r\n WPC disabled: dual dischg inhibit=%d\n", bat_ntc_dual_dischg_inhibit);
		// wpc_dual_temp_lock = 1;
	}
	// }
	// else
	// {
	if (!bat_ntc_dual_dischg_inhibit)
	{
		gd->wpc_disable = 0x00;
		ntc_printk("\r\n WPC re-enabled: dual dischg unlock\n");
		// wpc_dual_temp_lock = 0;
	}
	// }
	if (g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE)
	{
		tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		ntc_printk("\r\nTCPM_WPC_WORK_FIX5V\n");
	}
}
