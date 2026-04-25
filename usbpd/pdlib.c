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
	[4] = PDO_FIXED(20000, 1500, 0),                     // 20 V 1.5 A
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
extern bool typec_ntc_ot_dischg_flag;

#define PDO_STATE_NORMAL  0
#define PDO_STATE_LIMIT   1   // 9V2.22A/12V1.67A 限档
#define PDO_STATE_NTC     2   // 严格 5V/2A 单档

void pdlib_run(void)
{
	static uint8_t pre_state = PDO_STATE_NORMAL;
	usb_pdevt_run();
	usb_pd_run();
	usb_tc_run();
	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		uint8_t state;
		// 优先级：C 口 bat NTC 降功率(5V/2A 严格) > typec NTC OT / wpc bat 降功率(limit) > 正常
		if (gd->bat_ntc_cport_dischg_reduce_flag)
			state = PDO_STATE_NTC;
		else if (typec_ntc_ot_dischg_flag || gd->bat_ntc_dischg_reduce_flag)
			state = PDO_STATE_LIMIT;
		else
			state = PDO_STATE_NORMAL;

		if (state != pre_state)
		{
			pre_state = state;
			// 切档：在 PD 合约内→soft reset 让对端重协商；非 PD（BC1.2/无协议）→直接物理钳到 PDO[0] 的 V/I
			switch (state)
			{
				case PDO_STATE_NTC:
					tcpm_update_pdo_for_ntc();
					if (pdlib_is_connect())
						pdlib_set_pd_event(pdlib_get_port_map(), USB_PD_EVT_SOURCE_SOFTRESET);
					else
						buckboost_set_bus_iv(5000, 2000, 500, 0);   // source_pdo_ntc 5V/2A
					break;
				case PDO_STATE_LIMIT:
					tcpm_update_pdo_for_limit();
					if (pdlib_is_connect())
						pdlib_set_pd_event(pdlib_get_port_map(), USB_PD_EVT_SOURCE_SOFTRESET);
					else
						buckboost_set_bus_iv(5000, 3000, 500, 0);   // source_pdo1[0] 5V/3A
					break;
				default:
					tcpm_update_pdo_for_normal();
					if (pdlib_is_connect())
						pdlib_set_pd_event(pdlib_get_port_map(), USB_PD_EVT_SOURCE_SOFTRESET);
					else
						buckboost_set_bus_iv(5000, 3000, 500, 0);   // source_pdo[0] 5V/3A 还原默认
					break;
			}
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





