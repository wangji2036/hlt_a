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
bool typec_ntc_ot_flag = false;
extern const uint32_t source_pdo[];
extern const uint32_t source_pdo_ntc[];

#if(BUCKBOOST_USED_NU6805 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)


void buckboost_ntc_handle(void)
{
	static uint8_t bat_ntc_lock_cnt = 0;
	static uint8_t bat_ntc_ut_cnt = 0;
	static uint8_t bat_ntc_ot_cnt = 0;
	static uint8_t ntc_stop_chg_cnt = 0;
	static uint8_t typec_ntc_ot_cnt = 0;
	//bat
	{
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			gd->bat_ntc_lock_flag = 0;
			if(!ntc_stop_chrg_flag)
			{
				//3度  52度
				if(g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_50 || g_buckboost.adc_tbat1> NTC_10K_3435_REAL_RT_3 || g_buckboost.adc_tbat1 == 470)
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
				//8度  47度
				if(g_buckboost.adc_tbat1 > NTC_10K_3435_REAL_RT_47 && g_buckboost.adc_tbat1< NTC_10K_3435_REAL_RT_8)
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
				//15度
				if(g_buckboost.adc_tbat1>NTC_10K_3435_REAL_RT_15)
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
				//20度
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
				//45度
				if(g_buckboost.adc_tbat1<NTC_10K_3435_REAL_RT_43)
				{
					if(g_buckboost.adc_vbat>=8400)
					{
						if(bat_ntc_ot_cnt++>10)
						{
							bat_ntc_ot_cnt = 0;
							bat_charge_ntc_ot_flag = 1;
							// hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x00);
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
					bat_ntc_ot_cnt = 0;
				}
			}
			else
			{
				//40度
				if(g_buckboost.adc_tbat1>NTC_10K_3435_REAL_RT_40||g_buckboost.adc_vbat<8400)
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
		}

		if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
		{
			ntc_stop_chrg_flag = 0;
			bat_charge_ntc_ot_flag = 0;
			if(gd->bat_ntc_lock_flag == 0)
			{
				//-15度 57度
				if(g_buckboost.adc_tbat1 < NTC_10K_3435_REAL_RT_54 || g_buckboost.adc_tbat1> NTC_10K_3435_REAL_RT_N15||g_buckboost.adc_tbat1==470)
				{
					bat_ntc_lock_cnt++;
					if(bat_ntc_lock_cnt >= 20)
					{
						bat_ntc_lock_cnt = 0;
						gd->bat_ntc_lock_flag = 1;
					}
				}
				else
				{
					bat_ntc_lock_cnt = 0;
				}
			}
			else
			{
				//-10度 52度
				if(g_buckboost.adc_tbat1 > NTC_10K_3435_REAL_RT_52 && g_buckboost.adc_tbat1< NTC_10K_3435_REAL_RT_N10)
				{
					bat_ntc_lock_cnt++;
					if(bat_ntc_lock_cnt >= 20)
					{
						bat_ntc_lock_cnt = 0;
						gd->bat_ntc_lock_flag = 0;
						gd->ntc_led_off = 1;
					}
				}
				else
				{
					bat_ntc_lock_cnt = 0;
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
					gd->typec_ntc_lock = 1;
					gd->recharge_flag = 1;
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec <= 82 && gd->sys_infos.ntc_temp_typec >-15)
				{
					gd->ntc_led_off = 1;
					gd->typec_ntc_lock = 0;
					gd->recharge_flag = 0;
				}
			}
			
			if(!typec_ntc_ot_flag)
			{
				if(gd->sys_infos.ntc_temp_typec >= 81)
				{
					if(typec_ntc_ot_cnt++>10)
					{
						typec_ntc_ot_flag = 1;
					}
				}
				else
				{
					typec_ntc_ot_cnt = 0;
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec < 35)
				{
					if(typec_ntc_ot_cnt++>10)
					{
						typec_ntc_ot_flag = 0;
					}
				}
				else
				{
					typec_ntc_ot_cnt = 0;
				}
			}
		}
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(!gd->typec_charge_ntc_lock)
			{
				if(gd->sys_infos.ntc_temp_typec >81 || gd->sys_infos.ntc_temp_typec<-15)
				{
					gd->typec_charge_ntc_lock = 1;
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec <= 68 && gd->sys_infos.ntc_temp_typec>-15)
				{
					gd->typec_charge_ntc_lock = 0;
					hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,0x10);
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}

			if(!typec_ntc_ot_flag)
			{
				if(gd->sys_infos.ntc_temp_typec > 68)
				{
					if(typec_ntc_ot_cnt++>10)
					{
						typec_ntc_ot_flag = 1;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					typec_ntc_ot_cnt = 0;
				}
			}
			else
			{
				if(gd->sys_infos.ntc_temp_typec < 30)
				{
					if(typec_ntc_ot_cnt++>10)
					{
						typec_ntc_ot_flag = 0;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				else
				{
					typec_ntc_ot_cnt = 0;
				}
			}
		}
	}


}


#endif

