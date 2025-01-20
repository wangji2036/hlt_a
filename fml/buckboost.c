#include "regdef.h"
#include "buckboost.h"
#include "sw7201.h"
#include "nu6801.h"
#include "printk.h"
#include "tcpm.h"
#include "typec.h"
#include "port_manager.h"
#include "tcpm.h"

struct buckboost_s  g_buckboost;

void buckboost_set_bus_iv(uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay)
{
	printk("OUT = %d %d\n",voltage,current);
	osal_stop_timerEx(BUCKBOOST_REGULATOR_TIMER);
	g_buckboost.out_voltage_wait = wait;
	g_buckboost.out_voltage_delay = delay;
	g_buckboost.buckboost_out_voltage = voltage;
	g_buckboost.buckboost_out_current = current;
	g_buckboost.regulator_state = 0;
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT);
}

void buckboost_set_charge_current(uint16_t ibat,uint16_t ibus)
{
	g_buckboost.chager_ibus_limit = ibus;
	g_buckboost.chager_ibat_limit = ibat;
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_CHARGER_CURRENT);
}

void buckboost_set_work_mode(enum buckboost_mode mode)
{
	//printk("%s = %d\n",__func__,mode);
	g_buckboost.woke_mode = mode;
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SWITCH_WORK_MODE);
}


void buckboost_set_typeca_gate_en(bool en)
{
	g_buckboost.set_typeca_gate_en = en;
	//printk("typeca gate = 0x%x\n",g_buckboost.set_typeca_gate_en);
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECA_GATE_EN);
}

void buckboost_set_typecb_gate_en(bool en)
{
	g_buckboost.set_typecb_gate_en = en;
	//printk("typecb gate = 0x%x\n",g_buckboost.set_typecb_gate_en);
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECB_GATE_EN);
}

void buckboost_set_usb_a_gate_en(bool en)
{
	g_buckboost.set_usb_a_gate_en = en;
	//printk("typecb gate = 0x%x\n",g_buckboost.set_typecb_gate_en);
	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_USB_A_GATE_EN);
}

bool buckboost_regulator_done(void)
{
	return g_buckboost.regulator_state;
}

void buckboost_task_init(void)
{
	osal_mem_set(&g_buckboost,0,sizeof(struct buckboost_s));
	osal_task_handler_reg(BUCKBOOST_TASK, buckboost_task_event_handler);
	osal_start_timerEx(BUCKBOOST_PERIOD_TIMER, BUCKBOOST_TIME_PERIOD, BUCKBOOST_TIME_PERIOD, BUCKBOOST_TASK, BUCKBOOST_EVT_TIME_PERIOD);
	osal_start_timerEx(BUCKBOOST_VBUS_TIMER, BUCKBOOST_VBUS_PERIOD, BUCKBOOST_VBUS_PERIOD, BUCKBOOST_TASK, BUCKBOOST_EVT_VBUS_PERIOD);

	buckboost_ops.init();

	g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();
	g_buckboost.adc_ibat = buckboost_ops.get_bat_current();
	g_buckboost.adc_ibus = buckboost_ops.get_bus_current();
	g_buckboost.usba_state =  buckboost_ops.get_a2_state();
	g_buckboost.adc_tbat = buckboost_ops.get_bat_temperature();
	g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();

	g_buckboost.usba_dectet_en = buckboost_ops.en_a2_detect(true);
//	buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
//	buckboost_set_bus_iv(5000,3000,0,0);
}



void buckboost_protection_handle(void)
{
	#define VBUS_FUALT_VBUS_OCP			BIT(1)
	#define VBUS_FUALT_VBUS_SCP			BIT(2)
	#define VBUS_FUALT_VBAT_UVP			BIT(3)
	#define VBUS_FUALT_VBAT_OVP			BIT(4)
	#define VBUS_FUALT_VBUS_OVP			BIT(5)

	#define VBUS_OVP_TH					21500

	static uint8_t buckboost_protection_flag = false;

	uint8_t status = 0;

	status = buckboost_ops.get_protect_status();

	if(g_buckboost.adc_vbus > VBUS_OVP_TH) status |= VBUS_FUALT_VBUS_OVP;

	if(status != 0)
	{
		if(status & (VBUS_FUALT_VBUS_SCP | VBUS_FUALT_VBUS_OVP | VBUS_FUALT_VBUS_OCP))
		{
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			hal_tcpc_set_gate_en(PORT2_INDEX,false);

			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
			tcpm_stop_wpc(WPC_DELAY);
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			tcpm_disable_usba_detect();
			buckboost_protection_flag = 1;
			printk("protect lock =%d\n",status);
		}
	}
	else
	{
		if(buckboost_protection_flag)
		{
			buckboost_protection_flag = 0;
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
			osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
			tcpm_stop_wpc(WPC_DELAY);
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			printk("protect unlock\n");
		}
	}

	g_buckboost.protect_status = status;
}

void buckboost_ir_drop_handle(void)
{

	uint16_t ir_drop = 0;
	static uint8_t cnt_delay = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE && wpc_mode != TCPM_WPC_WORK_BOOST && !g_usb_pd_s.is_in_pps)
	{
		ir_drop = -g_buckboost.adc_ibus * 100 / 1000 ;    //1A +100mV
		ir_drop = ir_drop / 20 * 20;
		if(ir_drop >= 300) ir_drop = 300;
		if(ir_drop != g_buckboost.ir_drop)
		{
			cnt_delay++;
			if(cnt_delay >= 5)
			{
				g_buckboost.ir_drop = ir_drop;
				printk("ir drop = %d\n",g_buckboost.ir_drop);
				buckboost_ops.set_out(g_buckboost.buckboost_out_voltage + g_buckboost.ir_drop,g_buckboost.buckboost_out_current);
			}
		}
		else
		{
			cnt_delay = 0;
		}
	}
	else
	{
		cnt_delay = 0;
		g_buckboost.ir_drop = 0;;
	}
}

void buckboost_task_event_handler(uint32_t event)
{
	static uint8_t get_info_step = 0;
	switch (event)
	{
		case BUCKBOOST_EVT_TIME_PERIOD:
			//g_buckboost.adc_dischg_adc_ibus = buckboost_ops.get_bus_current();
			//printk("DISCHG IBUS= %d\n",g_buckboost.adc_dischg_adc_ibus);

			if(get_info_step == 0)
			{
				g_buckboost.adc_ibat = buckboost_ops.get_bat_current();
				g_buckboost.adc_ibus = buckboost_ops.get_bus_current();
				g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();
				g_buckboost.adc_tbat = buckboost_ops.get_bat_temperature();
			}
			else if(get_info_step == 1)
			{
				g_buckboost.usba_state =  buckboost_ops.get_a2_state();
				if(g_buckboost.adc_vbat < BAT_DEAD_BATTER_V)
				{
					g_tc[TYPEC_PORT_A].is_deadbattery = 1;
					g_tc[TYPEC_PORT_B].is_deadbattery = 1;
				}
				else if(g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V)
				{
					if(g_tc[TYPEC_PORT_A].is_deadbattery)
					{
						g_tc[TYPEC_PORT_A].is_deadbattery = 0;
						g_tc[TYPEC_PORT_B].is_deadbattery = 0;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
					}
				}
				osal_set_event(USB_TASK,TCPM_EVT_USBA_SCAN);
				buckboost_ir_drop_handle();
			}
			else if(get_info_step == 2)
			{
				buckboost_protection_handle();
			}
			get_info_step++;
			if(get_info_step > 2) get_info_step = 0;
			break;
		case BUCKBOOST_EVT_VBUS_PERIOD:
			g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();
			break;
		case BUCKBOOST_EVT_SWITCH_WORK_MODE:  //
			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
			{
				g_buckboost.chager_ibat_limit = 100;
				buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
			}
			buckboost_ops.set_work_mode(g_buckboost.woke_mode);
			//printk("%s\n","BUCKBOOST_EVT_SWITCH_WORK_MODE");
			break;
		case BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT:
			//printk("%s\n","BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT");
			g_buckboost.regulator_state = 0;
			osal_start_timerEx(BUCKBOOST_REGULATOR_TIMER, g_buckboost.out_voltage_wait, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_REGULATOR_WAITDONE);
			break;
		case BUCKBOOST_EVT_REGULATOR_WAITDONE:
			//printk("%s\n","BUCKBOOST_EVT_REGULATOR_WAITDONE");
			//buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,g_buckboost.buckboost_out_current);
			buckboost_ops.set_out(g_buckboost.buckboost_out_voltage + g_buckboost.ir_drop,g_buckboost.buckboost_out_current);
			if(g_buckboost.out_voltage_delay != 0)
				osal_start_timerEx(BUCKBOOST_REGULATOR_TIMER, g_buckboost.out_voltage_delay, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_REGULATOR_DELAYDONE);
			break;
		case BUCKBOOST_EVT_REGULATOR_DELAYDONE:
			//printk("%s\n","BUCKBOOST_EVT_REGULATOR_DELAYDONE");
			g_buckboost.regulator_state = 1;
			g_buckboost.out_voltage_wait = 0;
			g_buckboost.out_voltage_delay = 0;
			osal_stop_timerEx(BUCKBOOST_REGULATOR_TIMER);
			break;
		case BUCKBOOST_EVT_SET_CHARGER_CURRENT:
			//printk("%s\n","BUCKBOOST_EVT_SET_CHARGER_CURRENT");
			g_buckboost.woke_mode = BUCKBOOST_CHAGER_MODE;
			buckboost_ops.set_chager_ibus_limit(g_buckboost.chager_ibus_limit);
			buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
			buckboost_ops.set_work_mode(g_buckboost.woke_mode);
			break;
		case BUCKBOOST_EVT_SET_TYPECA_GATE_EN:
			//printk("%s\n","BUCKBOOST_EVT_SET_TYPECA_GATE_EN");
			buckboost_ops.typca_gate_en(g_buckboost.set_typeca_gate_en);
			break;
		case BUCKBOOST_EVT_SET_TYPECB_GATE_EN:
			//printk("%s\n","BUCKBOOST_EVT_SET_TYPECB_GATE_EN");
			buckboost_ops.typcb_gate_en(g_buckboost.set_typecb_gate_en);
			break;
		case BUCKBOOST_EVT_SET_USB_A_GATE_EN:
			buckboost_ops.usb_a_gate_en(g_buckboost.set_usb_a_gate_en);
			break;
		case BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_EN:
			buckboost_ops.typca_dischg_en(true);
			break;
		case BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_EN:
			buckboost_ops.typcb_dischg_en(true);
			break;
		case BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_DIS:
			buckboost_ops.typca_dischg_en(false);
			break;
		case BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_DIS:
			buckboost_ops.typcb_dischg_en(false);
			break;
		default:
			break;
	}

}

#if(BUCKBOOST_USED_SW7201 == 1)
const struct buckboost_operations buckboost_ops =
{
	.init = 					hal_sw7201_buckboost_init,
	.set_work_mode = 			hal_sw7201_buckboost_set_mode,
	.set_out = 					hal_sw7201_buckboost_set_busiv,
	.typca_gate_en = 			hal_sw7201_buckboost_typeca_gate_en,
	.typcb_gate_en = 			hal_sw7201_buckboost_typecb_gate_en,
	.usb_a_gate_en = 			hal_sw7201_buckboost_usb_a_gate_en,
	.set_chager_ibus_limit = 	hal_sw7201_buckboost_charge_ibus_limit,
	.set_chager_ibat_limit = 	hal_sw7201_buckboost_charge_ibat_limit,
	.get_bus_current = 			hal_sw7201_buckboost_get_bus_current,
	.get_bat_current =  		hal_sw7201_buckboost_get_bat_current,
	.get_bat_voltage =  		hal_sw7201_buckboost_get_bat_voltage,
	.get_bus_voltage =  		hal_sw7201_buckboost_get_bus_voltage,
	.get_a2_state    = 			hal_sw7201_buckboost_get_a2_state,
	.en_a2_detect  = 			hal_sw7201_buckboost_a2_detect_enable,
	.get_bat_temperature =      hal_sw7201_buckboost_get_bat_temperature,
	.typca_dischg_en = 			hal_sw7201_buckboost_typeca_dischg,
	.typcb_dischg_en = 			hal_sw7201_buckboost_typecb_dischg,
	.usb_a_dischg_en = 			hal_sw7201_buckboost_usb_a_dischg,
	.vbus_dischg_en = 			hal_sw7201_buckboost_vbus_dischg,
	.get_protect_status = 		hal_sw7201_buckboost_get_protect,
};
#elif(BUCKBOOST_USED_NU6801 == 1)

const struct buckboost_operations buckboost_ops =
{
	.init = 					hal_nu6801_buckboost_init,
	.set_work_mode = 			hal_nu6801_buckboost_set_mode,
	.set_out = 					hal_nu6801_buckboost_set_busiv,
	.typca_gate_en = 			hal_nu6801_buckboost_typeca_gate_en,
	.typcb_gate_en = 			hal_nu6801_buckboost_typecb_gate_en,
	.usb_a_gate_en = 			hal_nu6801_buckboost_usb_a_gate_en,
	.set_chager_ibus_limit = 	hal_nu6801_buckboost_charge_ibus_limit,
	.set_chager_ibat_limit = 	hal_nu6801_buckboost_charge_ibat_limit,
	.get_bus_current = 			hal_nu6801_buckboost_get_bus_current,
	.get_bat_current =  		hal_nu6801_buckboost_get_bat_current,
	.get_bat_voltage =  		hal_nu6801_buckboost_get_bat_voltage,
	.get_bus_voltage =  		hal_nu6801_buckboost_get_bus_voltage,
	.get_a2_state    = 			hal_nu6801_buckboost_get_usba_state,
	.en_a2_detect  = 			hal_nu6801_buckboost_usba_detect_enable,
	.get_bat_temperature =      hal_nu6801_buckboost_get_bat_temperature,
	.typca_dischg_en = 			hal_nu6801_buckboost_typeca_dischg,
	.typcb_dischg_en = 			hal_nu6801_buckboost_typecb_dischg,
	.usb_a_dischg_en = 			hal_nu6801_buckboost_usb_a_dischg,
	.vbus_dischg_en = 			hal_nu6801_buckboost_vbus_dischg,
	.get_protect_status = 		hal_nu6801_buckboost_get_protect,
};

#endif


