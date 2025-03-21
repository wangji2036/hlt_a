#include "regdef.h"
#include "buckboost.h"
#include "sw7201.h"
#include "nu6801.h"
#include "printk.h"
#include "tcpm.h"
#include "typec.h"
#include "port_manager.h"
#include "config.h"
#include "tcpm.h"
#include "g_data.h"
#include "ntc.h"

uint8_t ntc_lock_flag = 0;
bool ntc_ut_flag = false;
bool ntc_ot_flag = false;
bool ntc_stop_chrg_flag = false;

#if(BUCKBOOST_USED_NU6801 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)


void buckboost_ntc_handle(void)
{
	static uint8_t ntc_lock_cnt = 0;
	static uint8_t ntc_ut_cnt = 0;
	static uint8_t ntc_ot_cnt = 0;
	static uint8_t ntc_stop_chg_cnt = 0;
	//static uint8_t ntc_lock_cnt = 0;
	printk("\nR_ntc=%d %d %d\n",g_buckboost.adc_tbat,ntc_ut_flag,ntc_ot_flag);

	uint16_t ntc_ut_value;
	uint16_t ntc_ut_restore_value;
	uint16_t ntc_ot_value;
	uint16_t ntc_ot_restore_value;

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		ntc_ut_value = 			CHRG_NTC_UT_VALUE;
		ntc_ut_restore_value = 	CHRG_NTC_UT_RESTORE_VALUE;
		ntc_ot_value = 			CHRG_NTC_OT_VALUE;
		ntc_ot_restore_value = 	CHRG_NTC_OT_RESTORE_VALUE;
	}
	else
	{
		ntc_ut_value = 			DISG_NTC_UT_VALUE;
		ntc_ut_restore_value = 	DISG_NTC_UT_RESTORE_VALUE;
		ntc_ot_value = 			DISG_NTC_OT_VALUE;
		ntc_ot_restore_value = 	DISG_NTC_OT_RESTORE_VALUE;
	}

	if(ntc_ut_flag == 0)
	{
		if(g_buckboost.adc_tbat > ntc_ut_value)
		{
			ntc_ut_cnt++;
			if(ntc_ut_cnt >= 5)
			{
				ntc_ut_cnt = 0;
				ntc_ut_flag = 1;

				if(g_usb_pd_s.explicit_contract && g_tcpc.pwr_role == TYPEC_SOURCE)
				{
					//port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_SOURCE_SOFTRESET);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}

				printk("\nR_ntc ut\n");
			}
		}
		else
		{
			ntc_ut_cnt = 0;
		}
	}
	else
	{
		if(g_buckboost.adc_tbat < ntc_ut_restore_value)
		{
			ntc_ut_cnt++;
			if(ntc_ut_cnt >= 5)
			{
				ntc_ut_cnt = 0;
				ntc_ut_flag = 0;
				if(g_usb_pd_s.explicit_contract && g_tcpc.pwr_role == TYPEC_SOURCE)
				{
					//port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_SOURCE_SOFTRESET);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
				printk("\nR_ntc utrestore\n");
			}
		}
		else
		{
			ntc_ut_cnt = 0;
		}
	}

	if(ntc_ot_flag == 0)
	{
		if(g_buckboost.adc_tbat < ntc_ot_value)
		{
			ntc_ot_cnt++;
			if(ntc_ot_cnt >= 5)
			{
				ntc_ot_cnt = 0;
				ntc_ot_flag = 1;

				if(g_usb_pd_s.explicit_contract && g_tcpc.pwr_role == TYPEC_SOURCE)
				{
					//port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_SOURCE_SOFTRESET);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}

				printk("\nR_ntc Ot\n");
			}
		}
		else
		{
			ntc_ot_cnt = 0;
		}
	}
	else
	{
		if(g_buckboost.adc_tbat > ntc_ot_restore_value)
		{
			ntc_ot_cnt++;
			if(ntc_ot_cnt >= 5)
			{
				ntc_ot_cnt = 0;
				ntc_ot_flag = 0;
				if(g_usb_pd_s.explicit_contract && g_tcpc.pwr_role == TYPEC_SOURCE)
				{
					//port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_SOURCE_SOFTRESET);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
				printk("\nR_ntc otrestore\n");
			}
		}
		else
		{
			ntc_ot_cnt = 0;
		}
	}

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE) //ntc_stop_chg_cnt
	{
		if(ntc_stop_chrg_flag == 0)
		{
			if(g_buckboost.adc_tbat < CHRG_NTC_OT_LOCK_VALUE || g_buckboost.adc_tbat> CHRG_NTC_UT_LOCK_VALUE)
			{
				ntc_stop_chg_cnt++;
				if(ntc_stop_chg_cnt >= 20)
				{
					ntc_stop_chg_cnt = 0;
					ntc_stop_chrg_flag = 1;
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
		}
		else
		{
			if(g_buckboost.adc_tbat > CHRG_NTC_OT_LOCK_RESTORE_VALUE && g_buckboost.adc_tbat< CHRG_NTC_UT_LOCK_RESTORE_VALUE)
			{
				ntc_stop_chg_cnt++;
				if(ntc_stop_chg_cnt >= 20)
				{
					ntc_stop_chg_cnt = 0;
					ntc_stop_chrg_flag = 0;
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
		}
	}

	if(ntc_lock_flag == 0)
	{
		if(g_buckboost.adc_tbat < DISG_NTC_OT_LOCK_VALUE || g_buckboost.adc_tbat> DISG_NTC_UT_LOCK_VALUE)
		{
			ntc_lock_cnt++;
			if(ntc_lock_cnt >= 20)
			{
				ntc_lock_cnt = 0;
				ntc_lock_flag = 1;
			}
		}
	}
	else
	{
		if(g_buckboost.adc_tbat > DISG_NTC_OT_LOCK_RESTORE_VALUE && g_buckboost.adc_tbat< DISG_NTC_UT_LOCK_RESTORE_VALUE)
		{
			ntc_lock_cnt++;
			if(ntc_lock_cnt >= 20)
			{
				ntc_lock_cnt = 0;
				ntc_lock_flag = 0;
			}
		}
	}
}


#endif

