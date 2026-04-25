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

bool bat_ntc_ut_flag = false;
bool bat_charge_ntc_ot_flag = false;
bool ntc_stop_chrg_flag = false;
uint8_t ntc_lock_flag = 0;
bool typec_ntc_ot_dischg_flag = false;
bool typec_ntc_ot_chrg_flag = false;
bool bat_ntc_dual_dischg_lock = false;
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
	// printk("adc_tbat1 = %d , ntc_temp_typec = %d",g_buckboost.adc_tbat1,gd->sys_infos.ntc_temp_typec);
	{
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(!ntc_stop_chrg_flag)
			{
				//3搴� 52搴�
				if(g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_52 || g_buckboost.adc_tbat1> NTC_10K_3435_REAL_RT_3 || g_buckboost.adc_tbat1 == 470)
				{
					ntc_stop_chg_cnt++;
					if(ntc_stop_chg_cnt >= 20)
					{
						ntc_stop_chg_cnt = 0;
						ntc_stop_chrg_flag = 1;
						// hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x00);
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
				//8搴� 47搴�
				if(g_buckboost.adc_tbat1 > NTC_10K_3435_REAL_RT_47 && g_buckboost.adc_tbat1< NTC_10K_3435_REAL_RT_5)
				{
					ntc_stop_chg_cnt++;
					if(ntc_stop_chg_cnt >= 20)
					{
						ntc_stop_chg_cnt = 0;
						ntc_stop_chrg_flag = 0;
						hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x10);
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					ntc_stop_chg_cnt = 0;
				}
			}
			if(!bat_ntc_ut_flag)
			{
				//15搴�
				if(g_buckboost.adc_tbat1>NTC_10K_3435_REAL_RT_18)
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
				//20搴�
				if(g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_20)
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
			if(!bat_charge_ntc_ot_flag)
			{
				//43°
				if(g_buckboost.adc_tbat1<NTC_10K_3435_REAL_RT_43)
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
				//40搴�
				if(g_buckboost.adc_tbat1>NTC_10K_3435_REAL_RT_38)
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
			ntc_stop_chrg_flag = 0;
			bat_charge_ntc_ot_flag = 0;

			if(!bat_ntc_dual_dischg_lock)
			{
				if(g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_45 || g_buckboost.adc_tbat1 > NTC_10K_3435_REAL_RT_0)
				{
					if(dual_dischg_lock_cnt++>=10)
					{
						dual_dischg_lock_cnt = 0;
						bat_ntc_dual_dischg_lock = 1;
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
			}
			else
			{
				if(g_buckboost.adc_tbat1 > NTC_10K_3435_REAL_RT_40 && g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_5)
				{
					if(dual_dischg_lock_cnt++>=10)
					{
						dual_dischg_lock_cnt = 0;
						bat_ntc_dual_dischg_lock = 0;
					}
				}
				else
				{
					dual_dischg_lock_cnt = 0;
				}
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
				if(gd->sys_infos.ntc_temp_typec <= 82 && gd->sys_infos.ntc_temp_typec >-10)
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
			
			if(!typec_ntc_ot_dischg_flag)
			{
				if(gd->sys_infos.ntc_temp_typec >= 81)
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
		gd->bat_ntc_dischg_reduce_flag = 0;
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

	// 电池NTC降功率：tbat>=43°C或<=-3°C降7.5W，回到[0,38]区间恢复15W
	if (!gd->bat_ntc_dischg_reduce_flag)
	{
		if (tbat >= 43 || tbat <= -3)
		{
			if (++bat_reduce_cnt > 10)
			{
				bat_reduce_cnt = 0;
				gd->bat_ntc_dischg_reduce_flag = 1;
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
		if (tbat <= 38 && tbat >= 0)
		{
			if (++bat_reduce_rec_cnt > 10)
			{
				bat_reduce_rec_cnt = 0;
				gd->bat_ntc_dischg_reduce_flag = 0;
				printk("\r\n[BAT_NTC] dischg restore tbat=%d", tbat);
			}
		}
		else
		{
			bat_reduce_rec_cnt = 0;
		}
	}
}
