#include "regdef.h"
#include "printk.h"
#include "tcpc.h"
#include "isr.h"
#include "bsp.h"
#include "g_data.h"
#include "pd.h"
#include "osal.h"
#include "usb_pd.h"
#include "buckboost.h"
#include "usbpd_config.h"
#include "tcpm.h"
#include "typec.h"
#include "pdlib.h"
#include "pd_tc.h"

extern void tcpm_update_pdo_for_normal(void);
const uint32_t source_pdo_default[] =
{
	#define SOURCE_PDO_FIXED_FLAGS     			(PDO_FIXED_UNCONSTRAINED_POWER | PDO_FIXED_DUAL_ROLE | PDO_FIXED_SUSPEND )
		// === 修改 默认 Sink PDO 2024-06-22 Victor ===
	[0] = PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),   // 5 V 3 A
	[1] = PDO_FIXED(9000, 3000, 0),                      // 9 V 3 A
	[2] = PDO_FIXED(12000, 2500, 0),                      // 12 V 2.5 A
	[3] = PDO_FIXED(15000, 2000, 0),                      // 15 V 2 A
	[4] = PDO_FIXED(20000, 1500, 0),                      // 20 V 1.5 A
	[5] = PDO_PPS_APDO(5000,11000,2700),  // 5V-11V 2.7A
	   // ====  修改 Victor 2024-06-22 end ====
};

const uint32_t sink_pdo_default[] =
{
	#define SINK_PDO_FIXED_FLAGS     			(PDO_FIXED_DUAL_ROLE | PDO_FIXED_UNCONSTRAINED_POWER | PDO_HIGH_CAPABILITY)
	[0] = PDO_FIXED(5000, 3000, SINK_PDO_FIXED_FLAGS),   // 5 V 3 A
	[1] = PDO_FIXED(9000, 3000, 0),                      // 9 V 3 A
	[2] = PDO_FIXED(12000, 2500, 0),                     // 12 V 2.5 A
	[3] = PDO_FIXED(15000, 2000, 0),                     // 15 V 2 A
	[4] = PDO_FIXED(20000, 1500, 0),                     // 20V 1.5 A
	[5] = PDO_PPS_APDO(5000,11000,2700),	// 5V-11V 2.70A
};


void usb_pdlib_timer_update()
{
	usb_pd_timer_update();
}

void pdlib_init(void)
{
	usb_tc_init();
	usb_pd_init();

	pdlib_update_source_pdo(source_pdo_default,sizeof(source_pdo_default)/4);
	pdlib_update_sink_pdo(sink_pdo_default,sizeof(sink_pdo_default) /4);
}
extern bool typec_ntc_ot_flag;
void pdlib_run(void)
{
	static uint8_t pre_flag = 0,pre_flag2 = 0,pre_flag3 = 0,soft_flag = 0;
	usb_pdevt_run();
	usb_pd_run();
	usb_tc_run();
	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		if(pre_flag != typec_ntc_ot_flag || pre_flag2 != gd->bat_ntc_dischg_reduce_flag)
		{
			pre_flag = typec_ntc_ot_flag;
			pre_flag2 = gd->bat_ntc_dischg_reduce_flag;
			if (typec_ntc_ot_flag||gd->bat_ntc_dischg_reduce_flag)
			{
				soft_flag = 1;
				tcpm_update_pdo_for_limit();
			}
			else
			{
				soft_flag = 0;
				tcpm_update_pdo_for_normal();
			}
			if(soft_flag != pre_flag3) pdlib_set_pd_event(pdlib_get_port_map(), USB_PD_EVT_SOURCE_SOFTRESET);
			pre_flag3 = soft_flag;
		}
	}
}

bool pdlib_is_connect(void)
{
	return g_usb_pd_s.explicit_contract;
}

bool pdlib_is_pps_sink(void)
{
	return (g_usb_pd_s.explicit_contract && g_usb_pd_s.is_in_pps && g_tcpc.pwr_role == TYPEC_SINK);
}

bool pdlib_is_pps_source(void)
{
	return (g_usb_pd_s.explicit_contract && g_usb_pd_s.is_in_pps && g_tcpc.pwr_role == TYPEC_SOURCE);
}

void pdlib_disable_typec(uint8_t index)
{
	usb_tc_set_state(&g_tc[index],TC_Disable,enter_state);
	g_tc[index].typec_delay_ms = 0xffff;
}

void pdlib_restart_typec(uint8_t index)
{
	usb_tc_set_state(&g_tc[index],TC_DRP_TOGGLE,enter_state);
	g_tc[index].typec_delay_ms = 0x00;
}

void pdlib_delayms_restart_typec(uint8_t index,uint16_t delay_ms)
{
	g_tc[index].typec_delay_ms = delay_ms;
	usb_tc_set_state(&g_tc[index],TC_Disable,enter_state);
}

void pdlib_disable_usbpd(void)
{
	usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
}

bool pdlib_get_deadbat(void)
{
	return g_tc[TYPEC_PORT_A].is_deadbattery;
}

void pdlib_set_deadbat(bool dead)
{
	g_tc[TYPEC_PORT_A].is_deadbattery = dead;
}

void pdlib_clear_typec_prswap(uint8_t index)
{
	g_tc[index].is_in_prswap = 0;
}

enum usb_tc_state_e pdlib_get_tc_state(uint8_t index)
{
	return g_tc[index].usb_tc_state;
}

uint16_t pdlib_get_source_supply_voltage(void)
{
	return g_usb_pd_s.supply_voltage;
}

uint16_t pdlib_get_source_supply_current(void)
{
	return g_usb_pd_s.supply_current;
}

enum pwr_role_e pdlib_get_pwr_role(void)
{
	return g_tcpc.pwr_role;
}

enum data_role_e pdlib_get_date_role(void)
{
	return g_tcpc.data_role;
}

void pdlib_set_pd_event(uint8_t tc_index,uint32_t event)
{
	usb_pd_set_event(g_tcpc.tc_port_map,event);
}

void pdlib_update_source_pdo(const uint32_t* pdo,uint8_t pdo_n)
{
	updata_pdo_of_source(pdo,pdo_n);
}

void pdlib_update_sink_pdo(const uint32_t* pdo,uint8_t pdo_n)
{
	updata_pdo_of_sink(pdo,pdo_n);
}

uint32_t pdlib_snk_get_work_pdo(void)
{
	return (uint32_t)g_usb_pd_s.snk_rx_source_cap[rdo_index(g_usb_pd_s.snk_rdo) - 1];
}

uint8_t pdlib_snk_get_work_pdo_index(void)
{
	return rdo_index(g_usb_pd_s.snk_rdo);
}

uint32_t pdlib_snk_get_pdo_by_index(uint8_t index)
{
	if(index == 0 || index > 7) return 0;
	return (uint32_t)g_usb_pd_s.snk_rx_source_cap[index - 1];
}


uint8_t pdlib_snk_get_pdo_amount(void)
{
	return g_usb_pd_s.snk_rx_pdo_n;
}

void pdlib_snk_requsrt_voltage(uint8_t pdo_index,uint16_t voltage,uint16_t current)
{
	usb_pd_requsrt_voltage(pdo_index,voltage,current);
}

uint8_t pdlib_get_port_map(void)
{
	return g_tcpc.tc_port_map;
}

void pdlib_tcpc_get_cc(uint8_t tc_index,enum tc_cc_status *cc1, enum tc_cc_status *cc2)
{
	hal_tcpc_get_cc(tc_index,cc1,cc2);
}
enum tc_drp_reult pdlib_get_drp_toggle_result(uint8_t tc_index)
{
	return hal_get_drp_toggle_result(tc_index);
}

void pdlib_tcpc_set_cc(uint8_t tc_index,enum tc_cc_status cc)
{
	hal_tcpc_set_cc(tc_index,cc);
}

void pdlib_set_pd_port(uint8_t tc_index)
{
	hal_tcpc_set_phy_port(tc_index);
}





