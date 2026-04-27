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

bool bat_ntc_ut_flag = false;
bool bat_charge_ntc_ot_flag = false;
bool bat_ntc_stop_chrg_flag = false;
uint8_t ntc_lock_flag = 0;
bool typec_ntc_ot_dischg_flag = false;
bool typec_ntc_ot_chrg_flag = false;
bool bat_ntc_dischg_lock = false;
bool bat_ntc_dual_dischg_inhibit = false;   // C 口+无线充同时放电温度抑制：<0/≥45 锁，[5,40] 恢复 max 5V/3A
uint8_t bat_low_volt_reduce = 0;
extern const uint32_t source_pdo[];
extern const uint32_t source_pdo_ntc[];

#if(BUCKBOOST_USED_NU6805 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)


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
	static bool ot_full_stop = false;   // 4.1V 停充闭锁：在 OT 区间 vbat≥8200 触发，OT 解时清；vbat 回落不解锁
	int16_t bat_temp = ntc_to_temp(g_buckboost.adc_tbat1);
	// printk("bat_temp = %d , ntc_temp_typec = %d",bat_temp,gd->sys_infos.ntc_temp_typec);
	{
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			// 43-52°C 区间 vbat≥8.2V(4.1V/cell) 触发停充并闭锁；vbat 回落到 4.1V 以下不解，
			// 必须等温度降到 ≤30°C 才解
			if (bat_charge_ntc_ot_flag && g_buckboost.adc_vbat >= 8200) {
				ot_full_stop = true;
			}
			if (bat_temp <= 30) {
				ot_full_stop = false;
			}

			// 充电禁充：<3°C 锁 / ≥5°C 解；>52°C 锁 / ≤47°C 解；43-52°C 段满 4.1V 闭锁
			if(!bat_ntc_stop_chrg_flag)
			{
				if(bat_temp > 52 || bat_temp < 3 || ot_full_stop)
				{
					ntc_stop_chg_cnt++;
					if(ntc_stop_chg_cnt >= 20)
					{
						ntc_stop_chg_cnt = 0;
						bat_ntc_stop_chrg_flag = 1;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					ntc_stop_chg_cnt = 0;
				}
			}
			else
			{
				if(bat_temp >= 5 && bat_temp <= 47 && !ot_full_stop)
				{
					ntc_stop_chg_cnt++;
					if(ntc_stop_chg_cnt >= 20)
					{
						ntc_stop_chg_cnt = 0;
						bat_ntc_stop_chrg_flag = 0;
						hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x10);
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					ntc_stop_chg_cnt = 0;
				}
			}
			// 充电限 5W：<18°C 触发，≥20°C 恢复 30W
			if(!bat_ntc_ut_flag)
			{
				if(bat_temp < 18)
				{
					if(bat_ntc_ut_cnt++>10)
					{
						bat_ntc_ut_cnt = 0;
						bat_ntc_ut_flag = 1;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					bat_ntc_ut_cnt = 0;
				}
			}
			else
			{
				if(bat_temp >= 20)
				{
					if(bat_ntc_ut_cnt++>10)
					{
						bat_ntc_ut_cnt = 0;
						bat_ntc_ut_flag = 0;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					bat_ntc_ut_cnt = 0;
				}
			}
			// 充电限 12W：≥43°C 触发，≤30°C 恢复 30W
			if(!bat_charge_ntc_ot_flag)
			{
				if(bat_temp >= 43)
				{
					if(bat_ntc_ot_cnt++>10)
					{
						bat_ntc_ot_cnt = 0;
						bat_charge_ntc_ot_flag = 1;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					bat_ntc_ot_cnt = 0;
				}
			}
			else
			{
				if(bat_temp <= 30)
				{
					if(bat_ntc_ot_cnt++>10)
					{
						bat_ntc_ot_cnt = 0;
						bat_charge_ntc_ot_flag = 0;
						hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x10);
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
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
					bat_low_volt_reduce = 1;
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
			else
			{
				if (g_buckboost.adc_vbat >= 7500)
				{
					bat_low_volt_reduce = 0;
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
		}

		if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
		{
			bat_ntc_stop_chrg_flag = 0;
			bat_charge_ntc_ot_flag = 0;

			// 放电锁：≤-15°C 或 ≥55°C 锁，[-10, 50] 解锁
			if(!bat_ntc_dischg_lock)
			{
				if(bat_temp <= -15 || bat_temp >= 55)
				{
					if(dual_dischg_lock_cnt++>=10)
					{
						dual_dischg_lock_cnt = 0;
						bat_ntc_dischg_lock = 1;
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
			}
			else
			{
				if(bat_temp >= -10 && bat_temp <= 50)
				{
					if(dual_dischg_lock_cnt++>=10)
					{
						dual_dischg_lock_cnt = 0;
						bat_ntc_dischg_lock = 0;
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
			}

			
			// 放电限 10W (PD 5V 2A)：≤-3°C 或 ≥43°C 触发，[0, 30] 恢复 30W
			if(!gd->bat_ntc_cport_dischg_reduce_flag)
			{
				if(bat_temp <= -3 || bat_temp >= 43)
				{
					if(bat_dischg_reduce_cnt++>10)
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
				if(bat_temp >= 0 && bat_temp <= 30)
				{
					if(bat_dischg_reduce_cnt++>10)
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
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE)
			{
				if(!bat_ntc_dual_dischg_inhibit)
				{
					if(bat_temp < 0 || bat_temp >= 45)
					{
						if(dual_dischg_inhibit_cnt++>=10)
						{
							dual_dischg_inhibit_cnt = 0;
							bat_ntc_dual_dischg_inhibit = 1;
						}
					}
					else
					{
						dual_dischg_inhibit_cnt = 0;
					}
				}
				else
				{
					if(bat_temp >= 5 && bat_temp <= 40)
					{
						if(dual_dischg_inhibit_cnt++>=10)
						{
							dual_dischg_inhibit_cnt = 0;
							bat_ntc_dual_dischg_inhibit = 0;
						}
					}
					else
					{
						dual_dischg_inhibit_cnt = 0;
					}
				}
			}
			else
			{
				dual_dischg_inhibit_cnt = 0;
			}

		}
	}
	//typec
	{
		if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
		{
			if(!gd->typec_ntc_lock)
			{
				if(gd->sys_infos.ntc_temp_typec >105 || gd->sys_infos.ntc_temp_typec<-15)
				{
					if(typec_ntc_lock_cnt++>=10)
					{
						typec_ntc_lock_cnt = 0;
						gd->typec_ntc_lock = 1;
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
				if(gd->sys_infos.ntc_temp_typec <= 80 && gd->sys_infos.ntc_temp_typec >-10)
				{
					if(typec_ntc_lock_cnt++>=10)
					{
						typec_ntc_lock_cnt = 0;
						gd->ntc_led_off = 1;
						gd->typec_ntc_lock = 0;
						gd->recharge_flag = 0;
					}
				}
				else
				{
					typec_ntc_lock_cnt = 0;
				}
			}

			// 放电 OT 限 20W (最大 12V)：≥80°C 触发，<35°C 恢复 30W
			if(!typec_ntc_ot_dischg_flag)
			{
				if(gd->sys_infos.ntc_temp_typec >= 80)
				{
					if(typec_ntc_ot_dischg_cnt++>10)
					{
						typec_ntc_ot_dischg_flag = 1;
					}
				}
				else
				{
					typec_ntc_ot_dischg_cnt = 0;
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec < 35)
				{
					if(typec_ntc_ot_dischg_cnt++>10)
					{
						typec_ntc_ot_dischg_flag = 0;
					}
				}
				else
				{
					typec_ntc_ot_dischg_cnt = 0;
				}
			}
		}
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(!gd->typec_charge_ntc_lock)
			{
				if(gd->sys_infos.ntc_temp_typec >105 || gd->sys_infos.ntc_temp_typec<-15)
				{
					if(typec_chrg_lock_cnt++>=10)
					{
						typec_chrg_lock_cnt = 0;
						gd->typec_charge_ntc_lock = 1;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					typec_chrg_lock_cnt = 0;
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec <= 68 && gd->sys_infos.ntc_temp_typec>-10)
				{
					if(typec_chrg_lock_cnt++>=10)
					{
						typec_chrg_lock_cnt = 0;
						gd->typec_charge_ntc_lock = 0;
						hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x10);
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					typec_chrg_lock_cnt = 0;
				}
			}

			if(!typec_ntc_ot_chrg_flag)
			{
				if(gd->sys_infos.ntc_temp_typec > 68)
				{
					if(typec_ntc_ot_chrg_cnt++>10)
					{
						typec_ntc_ot_chrg_flag = 1;
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
				if(gd->sys_infos.ntc_temp_typec < 30)
				{
					if(typec_ntc_ot_chrg_cnt++>10)
					{
						typec_ntc_ot_chrg_flag = 0;
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
	if (!gd->wirless_ntc_lock)
	{
		if (tntc >= 75 || tntc <= -10 )
		{
			if (++otp_cnt >= 5)
			{
				otp_cnt = 0;
				gd->wirless_ntc_lock = 1;
				gd->wpc_disable = 1;
				tcpm_stop_wpc(10);
				printk("\r\n[WPC_NTC] OTP lock tntc=%d", tntc);
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
				gd->wirless_ntc_lock = 0;
				gd->wpc_disable = 0;
				printk("\r\n[WPC_NTC] OTP unlock tntc=%d", tntc);
			}
		}
		else
		{
			otp_rec_cnt = 0;
		}
		return;
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
				printk("\r\n[WPC_NTC] power reduce tntc=%d", tntc);
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
				printk("\r\n[WPC_NTC] power restore tntc=%d", tntc);
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
		gd->bat_ntc_lock_flag = 0;
		gd->bat_ntc_wpc_dischg_reduce_flag = 0;
		bat_lock_cnt = 0;
		bat_lock_rec_cnt = 0;
		bat_reduce_cnt = 0;
		bat_reduce_rec_cnt = 0;
		return;
	}

	// 电池NTC保护：tbat<=-15°C或>=55°C锁，回到(-10,50)区间解锁
	if (!gd->bat_ntc_lock_flag)
	{
		if (tbat <= -15 || tbat >= 55)
		{
			if (++bat_lock_cnt >= 5)
			{
				bat_lock_cnt = 0;
				gd->bat_ntc_lock_flag = 1;
				tcpm_stop_wpc(10);
				printk("\r\n[BAT_NTC] lock tbat=%d", tbat);
			}
		}
		else
		{
			bat_lock_cnt = 0;
		}
	}
	else
	{
		// -10°C/50°C解锁，解锁后tbat仍在降功率区间→自然落到7.5W
		if (tbat > -10 && tbat < 50)
		{
			if (++bat_lock_rec_cnt >= 5)
			{
				bat_lock_rec_cnt = 0;
				gd->bat_ntc_lock_flag = 0;
				gd->ntc_led_off = 1;
				printk("\r\n[BAT_NTC] unlock tbat=%d", tbat);
			}
		}
		else
		{
			bat_lock_rec_cnt = 0;
		}
		return;
	}

	// 电池NTC降功率：tbat>=43°C或<=-3°C降7.5W，回到[0,30]区间恢复15W
	if (!gd->bat_ntc_wpc_dischg_reduce_flag)
	{
		if (tbat >= 43 || tbat <= -3)
		{
			if (++bat_reduce_cnt > 10)
			{
				bat_reduce_cnt = 0;
				gd->bat_ntc_wpc_dischg_reduce_flag = 1;
				printk("\r\n[BAT_NTC] dischg reduce tbat=%d", tbat);
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
				printk("\r\n[BAT_NTC] dischg restore tbat=%d", tbat);
			}
		}
		else
		{
			bat_reduce_rec_cnt = 0;
		}
	}
}
