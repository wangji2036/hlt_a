#include "regdef.h"
#include "printk.h"
#include "tcpc.h"
#include "isr.h"
#include "bsp.h"
#include "g_data.h"
#include "pd.h"
#include "osal.h"
//#include "usb_pd.h"
#include "buckboost.h"
#include "usbpd_config.h"
#include "tcpm.h"

bool hal_tcpc_vbus_is_present(uint8_t tc_index)
{
	static uint8_t cnt = 0;
#if(BUCKBOOST_USED_NU6801 == 1)
	cnt++;
	if(cnt >= 10)
	{
		cnt = 0;

		if(tc_index == 0)
		{
			if(buckboost_ops.get_typeca_vbus_present() >= 3800 ) return true;
		}
		else if(tc_index == 1)
		{
			if(buckboost_ops.get_typecb_vbus_present() >= 3800 ) return true;
		}
	}

	return false;
#else
	return true;
#endif
}

void hal_tcpc_pd_set_bus_iv(uint8_t tc_index,uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay)
{
	//
	buckboost_set_bus_iv(voltage,current,wait,delay);

	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		port_vbus = voltage;
	}
}

/*
 * hal_tcpc_pd_set_bus_iv   need return the state,
 * if regulation complete return true,other flase
 */
bool hal_tcpc_pd_bus_ready(uint8_t tc_index)
{
	//if(tc_index != 0) return true;
	return buckboost_regulator_done();
}

bool hal_tcpc_vbus_is_removed(uint8_t tc_index)
{
#if(BUCKBOOST_USED_NU6801 == 1)
#if(CONFIG_NU6801_A0 == 1)
	return true;
#else
	static uint8_t delay_cnt = 0;
	static uint8_t timeout0 = 0;
	static uint8_t timeout1 = 0;
	if(delay_cnt == 0)
	{
		if(tc_index == 0)
		{
			if(buckboost_ops.get_typeca_vbus_present() < 2000 )
			{
				timeout0 = 0;
				return true;
			}
			else
			{
				timeout0++;
				if(timeout0 >= 100)
				{
					timeout0 = 0;
					//printk("vbus0 timeout\n");
					return true;
				}
			}
		}
		else if(tc_index == 1)
		{
			if(buckboost_ops.get_typecb_vbus_present() < 2000 )
			{
				timeout1 = 0;
				return true;
			}
			else
			{
				timeout0++;
				if(timeout0 >= 100)
				{
					timeout0 = 0;
					//printk("vbus1 timeout\n");
					return true;
				}
			}

		}
		return false;
	}
	delay_cnt++;
	if(delay_cnt >= 10) delay_cnt = 0;
	return false;
#endif
#else
	return true;
#endif
}

bool hal_tcpc_vbus_is_vsfae0v(uint8_t tc_index)
{
#if(BUCKBOOST_USED_NU6801 == 1)
	if(tc_index == 0)
	{
		if(buckboost_ops.get_typeca_vbus_present() < 800 ) return true;
	}
	else if(tc_index == 1)
	{
		if(buckboost_ops.get_typecb_vbus_present() < 800 ) return true;
	}
	return false;
#else
	return true;
#endif
}

bool hal_tcpc_vbus_is_vsafe5v(void)
{
	if(g_buckboost.adc_vbus <= 5500) return true;
	return false;
}


void hal_tcpc_port_dummyload_en(uint8_t tc_index,bool en)
{
	if(tc_index == 0)
	{
		buckboost_ops.typcb_dischg_en(en);
	}

	if(tc_index == 1)
	{
		buckboost_ops.typca_dischg_en(en);
	}
}



void hal_tcpc_set_gate_en(uint8_t tc_index,bool en)
{
	//printk("gate[%d]:%d\n",tc_index,en);
	if(tc_index == 0)
	buckboost_set_typecb_gate_en(en);
	else if(tc_index == 1)
	buckboost_set_typeca_gate_en(en);
		
	else if(tc_index == 2)
		buckboost_set_usb_a_gate_en(en);
}

void hal_tcpc_set_source_mode(enum buckboost_mode mode)
{
	//printk("set buckboost mode = %d\n",mode);
	buckboost_set_work_mode(mode);
}
void hal_tcpc_set_snk_charge_current(uint16_t ibat,uint16_t ibus)
{
	buckboost_set_charge_current(ibat,ibus);
}

