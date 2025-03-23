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


static bool pps_vbus_uv = false;
struct buckboost_s  g_buckboost;
int16_t ibus_to_ibat(int16_t ibus,int16_t vbus,int16_t vbat)
{
	int16_t k,b;
	uint32_t actual_effiency;
	int32_t temp_ibat;
	// x1 =5,y1= 970; x2 = 9, y2= 950;
	// xielv k ----- (y2-y1)/(x2-x1) , so k = (950-970)/(9-5) = -5,  k  used as * 100
	// jieju ------y1=kx1+b, b= y1-kx1, so  b = 970 - (-5)*5 = 995
	// y= (k * x) + b; effiency, used as *1000
	if(ibus>0)// buck mode
	{
		k = -500;
		b = 995;
	}
	else // boost mode
	{
		if(vbus> 12000)
		{
            k = -300;
            b = 1000;
		}
		else if(vbus> 9000)
		{
			 k = -333;
			 b = 980;
		}
		else // <9v
		{
			k = -500;
			b = 995;
		}
	}
	actual_effiency = (k*vbus)/100 +b;
	temp_ibat = ((actual_effiency*((vbus*ibus) /1000))/vbat);
    return (int16_t)temp_ibat;
}
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
//	osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_CHARGER_CURRENT);

	g_buckboost.woke_mode = BUCKBOOST_SHUTDOWM_MODE;
	buckboost_ops.set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
	g_buckboost.woke_mode = BUCKBOOST_CHAGER_MODE;
	buckboost_ops.set_work_mode(g_buckboost.woke_mode);
	buckboost_ops.set_chager_ibus_limit(g_buckboost.chager_ibus_limit);
	buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
}

void buckboost_set_work_mode(enum buckboost_mode mode)
{
	//printk("%s = %d\n",__func__,mode);
	g_buckboost.woke_mode = mode;
	//osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SWITCH_WORK_MODE);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		g_buckboost.chager_ibat_limit = 100;
		g_buckboost.chager_ibus_limit = 100;
		buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
		buckboost_ops.set_chager_ibus_limit(g_buckboost.chager_ibus_limit);
	}
	buckboost_ops.set_work_mode(g_buckboost.woke_mode);
}


void buckboost_set_typeca_gate_en(bool en)
{
	g_buckboost.set_typeca_gate_en = en;
	//printk("typeca gate = 0x%x\n",g_buckboost.set_typeca_gate_en);
	//osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECA_GATE_EN);

	buckboost_ops.typca_gate_en(g_buckboost.set_typeca_gate_en);
}

void buckboost_set_typecb_gate_en(bool en)
{
	g_buckboost.set_typecb_gate_en = en;
	//printk("typecb gate = 0x%x\n",g_buckboost.set_typecb_gate_en);
	//osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECB_GATE_EN);
	buckboost_ops.typcb_gate_en(g_buckboost.set_typecb_gate_en);
}

void buckboost_set_usb_a_gate_en(bool en)
{
	g_buckboost.set_usb_a_gate_en = en;
	//printk("typecb gate = 0x%x\n",g_buckboost.set_typecb_gate_en);
	//osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_USB_A_GATE_EN);
	buckboost_ops.usb_a_gate_en(g_buckboost.set_usb_a_gate_en);
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
	//g_buckboost.usba_state =  buckboost_ops.get_a2_state();
	//g_buckboost.adc_tbat = buckboost_ops.get_bat_temperature();
	g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();
#if(CONFIG_USBA_SUPPORT == 1)
	g_buckboost.usba_dectet_en = buckboost_ops.en_a2_detect(true);
#endif
//	buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
//	buckboost_set_bus_iv(5000,3000,0,0);
}



void buckboost_protection_handle(void)
{
#if(BUCKBOOST_USED_SW7201 == 1)
	#define VBUS_FUALT_VBUS_OCP			BIT(1)
	#define VBUS_FUALT_VBUS_SCP			BIT(2)
	#define VBUS_FUALT_VBAT_UVP			BIT(3)
	#define VBUS_FUALT_VBAT_OVP			BIT(4)
	#define VBUS_FUALT_VBUS_OVP			BIT(5)
#elif(BUCKBOOST_USED_NU6801 == 1)
	#define URB_DET						BIT(0)
	#define BST_UV_FLAG					BIT(1)
	#define VBAT_OV_FLAG				BIT(2)
	#define VBAT_LOW_FLAG				BIT(3)
	#define VBUS_OV_FLAG				BIT(4)
	#define VBUS_REL_LOW_FLAG			BIT(5)
	#define VBUS_UV_FLAG				BIT(6)
	#define HFET_OCP					BIT(7)

	#define DIS_VBAT_LOW				BIT(8)
	#define PPS_UV						BIT(9)
	#define NTC_PCT					BIT(10)
	static uint8_t cnt = 0;
#endif

	#define SW7201_VBUS_OVP_TH					21500
	#define NU6801_VBUS_OVP_TH					20000

	static uint8_t buckboost_protection_flag = false;


	uint16_t status = 0;

	status = buckboost_ops.get_protect_status();

#if(BUCKBOOST_USED_SW7201 == 1)
	if(g_buckboost.adc_vbus > SW7201_VBUS_OVP_TH) status |= VBUS_FUALT_VBUS_OVP;
#elif(BUCKBOOST_USED_NU6801 == 1)
	if(g_buckboost.adc_vbus > NU6801_VBUS_OVP_TH )
	{
		status |= VBUS_OV_FLAG;
	}
#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
	if(ntc_lock_flag) status |= NTC_PCT;
#endif

	if(g_buckboost.adc_vbat < 3000)// && g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE)// && !g_tc[TYPEC_PORT_A].is_deadbattery)
	{
		cnt++;
		if(cnt >= 10)
		{
			if(g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE)
			{
				gd->bat_dead_flag = 1;
				status |= DIS_VBAT_LOW;

			}
			else
			{
				if(g_tc[TYPEC_PORT_A].is_deadbattery == 0)
				{
					port_manager_set_event(PORT_EVENT_RESET_CHARGE);
				}
			}
			g_tc[TYPEC_PORT_A].is_deadbattery = 1;
			g_tc[TYPEC_PORT_B].is_deadbattery = 1;
			cnt = 0;
		}
	}
	else
	{
		cnt = 0;
	}

	if(pps_vbus_uv)
	{
		pps_vbus_uv = false;
		status |= PPS_UV;

	}
#endif

	if(status != 0)
	{
#if(BUCKBOOST_USED_SW7201 == 1)
		if(status & (VBUS_FUALT_VBUS_SCP | VBUS_FUALT_VBUS_OVP | VBUS_FUALT_VBUS_OCP))
		{
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			buckboost_set_bus_iv(5000,3000,0,0);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
			tcpm_stop_wpc(WPC_DELAY);
			qi_state = 0;
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			tcpm_disable_usba_detect();
			buckboost_protection_flag = 1;
			printk("protect lock =0x%x\n",status);
		}
#elif(BUCKBOOST_USED_NU6801 == 1)
		if(status & (URB_DET  | VBAT_OV_FLAG | VBUS_OV_FLAG | HFET_OCP | VBUS_UV_FLAG  | DIS_VBAT_LOW  | PPS_UV | NTC_PCT))
		{
			printk("protect lock =0x%x\n",status);
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			g_port.port_state[PORT3_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			buckboost_set_bus_iv(5000,3000,0,0);
			g_tc[PORT0_INDEX].is_in_prswap = 0;
			g_tc[PORT1_INDEX].is_in_prswap = 0;
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_Disable,enter_state);
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			tcpm_stop_wpc(WPC_DELAY);
			qi_state = 0;
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			tcpm_disable_usba_detect();
			buckboost_ops.init();
			buckboost_protection_flag = 1;
		}
#endif
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
			buckboost_ops.init();
			buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
			buckboost_set_bus_iv(5000,3000,0,0);
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
	static uint8_t pps_uv_cnt = 0;
	uint32_t row;
	switch (event)
	{
		case BUCKBOOST_EVT_TIME_PERIOD:
			//g_buckboost.adc_dischg_adc_ibus = buckboost_ops.get_bus_current();
			//printk("DISCHG IBUS= %d\n",g_buckboost.adc_dischg_adc_ibus);

			if(get_info_step == 0)
			{
			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IAC1);
			#endif

			}
			else if(get_info_step == 1)
			{
			#if(CONFIG_USBA_SUPPORT == 1)
				g_buckboost.usba_state =  buckboost_ops.get_a2_state();
			#endif
			#if(BUCKBOOST_USED_SW7201 == 1)
				g_buckboost.adc_ibat = buckboost_ops.get_bat_current();
			#else
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IBAT);
			#endif
			#if(BUCKBOOST_USED_SW7201 == 1)
				if(g_buckboost.adc_vbat < BAT_DEAD_BATTER_V)
				{
					g_tc[TYPEC_PORT_A].is_deadbattery = 1;
					g_tc[TYPEC_PORT_B].is_deadbattery = 1;
				}
				else
			#endif

			#if(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V && !nu6801_dead_bat)
			#else
				if(g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V)
			#endif
				{

					if(g_tc[TYPEC_PORT_A].is_deadbattery)
					{
						g_tc[TYPEC_PORT_A].is_deadbattery = 0;
						g_tc[TYPEC_PORT_B].is_deadbattery = 0;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						printk("%s\n",__func__);
					}
				}
				osal_set_event(USB_TASK,TCPM_EVT_USBA_SCAN);

			}
			else if(get_info_step == 2)
			{
			#if(BUCKBOOST_USED_SW7201 == 1)
				g_buckboost.adc_ibus = buckboost_ops.get_bus_current();
			#else
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_IBUS);
			#endif
				buckboost_protection_handle();

			#if(BUCKBOOST_USED_NU6801 == 1)
				if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					uint8_t flag = buckboost_ops.get_charge_flag();

					printk("charge flag = 0x%x\n",flag);

					if(flag & 0x02) g_buckboost.bat_full_flag = 1;
					if(g_buckboost.bat_full_flag && flag & 0x01)
					{
						g_buckboost.bat_full_flag = 0;
						port_manager_set_event(PORT_EVENT_RESET_CHARGE);
						printk("------------------------- rechage \n");
					}
				}
				else
				{
					g_buckboost.bat_full_flag = 0;
				}

			#endif
			}
			else if(get_info_step == 3)
			{

			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_deadbat_patch();
			#endif
			#if(BUCKBOOST_USED_SW7201 == 1)
				buckboost_ir_drop_handle();
				g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();
			#else
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VBAT);
			#endif
			}
			else if(get_info_step == 4)
			{
				//hal_nu6801_buckboost_get_main_state();

			#if(BUCKBOOST_USED_NU6801 == 1 && CONFIG_USE_NTC_FOR_CHAGER == 1)
				g_buckboost.adc_tbat = buckboost_ops.get_bat_temperature();
				buckboost_ntc_handle();
			#endif
			#if(BUCKBOOST_USED_NU6801 == 1)
				hal_nu6801_get_charge_state();
				hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VREF);
			#endif
			}
			if(get_info_step ++ > 4) get_info_step = 0;
			break;
		case BUCKBOOST_EVT_VBUS_PERIOD:
		#if(BUCKBOOST_USED_SW7201 == 1)
			g_buckboost.adc_vbus = buckboost_ops.get_bus_voltage();
		#else
			hal_nu6801_buckboost_set_adc_channel(NU6801_ADC_VBUS);
		#endif
			//if(g_usb_pd_s.is_in_pps && g_usb_pd_s.explicit_contract)
			if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE && (g_tc[0].usb_tc_state == TC_SRC_Attached || g_tc[1].usb_tc_state == TC_SRC_Attached))
			{
				g_buckboost.ibus_cc_flag =  buckboost_ops.is_ibus_loop();
				if(g_buckboost.adc_vbus < 4400)
				{
					pps_uv_cnt++;
					if(pps_uv_cnt >= 5)
					{
						pps_uv_cnt = 0;
						pps_vbus_uv = true;
					}
				}
				else
					pps_uv_cnt = 0;
			}
			else
			{
				pps_uv_cnt = 0;
				pps_vbus_uv = false;
			}
			break;
		case BUCKBOOST_EVT_SWITCH_WORK_MODE:  //
//			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
//			{
//				g_buckboost.chager_ibat_limit = 100;
//				buckboost_ops.set_chager_ibat_limit(g_buckboost.chager_ibat_limit);
//			}
//			buckboost_ops.set_work_mode(g_buckboost.woke_mode);
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
			//if(g_buckboost.out_voltage_delay != 0)
			osal_start_timerEx(BUCKBOOST_REGULATOR_TIMER, g_buckboost.out_voltage_delay, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_REGULATOR_DELAYDONE);
			break;
		case BUCKBOOST_EVT_REGULATOR_DELAYDONE:
			//printk("%s\n","BUCKBOOST_EVT_REGULATOR_DELAYDONE");
			g_buckboost.regulator_state = 1;
			g_buckboost.out_voltage_wait = 0;
			g_buckboost.out_voltage_delay = 0;
		#if(BUCKBOOST_USED_NU6801 == 1)
			if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE) buckboost_ops.set_ovp(g_buckboost.buckboost_out_voltage);
		#endif
			osal_stop_timerEx(BUCKBOOST_REGULATOR_TIMER);
			break;
		case BUCKBOOST_EVT_SET_CHARGER_CURRENT:
			break;
		case BUCKBOOST_EVT_SET_TYPECA_GATE_EN:
			break;
		case BUCKBOOST_EVT_SET_TYPECB_GATE_EN:
			break;
		case BUCKBOOST_EVT_SET_USB_A_GATE_EN:
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
		case BUCKBOOST_EVT_ADC_PERIOD:
		#if(BUCKBOOST_USED_NU6801 == 1)
			switch(nu6801_adc_chennel)
			{
				case NU6801_ADC_VBAT:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t vbat = row* 120  * 25 / nu6801_vref;
					g_buckboost.adc_vbat = vbat;
					printk("adc_vbat = %d\n",g_buckboost.adc_vbat);
					break;
				case NU6801_ADC_IBAT:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t ibat = row* 120  * 100 / nu6801_vref;
					if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						g_buckboost.adc_ibat = ibat;
					else
						g_buckboost.adc_ibat = -ibat;
					//printk("adc_ibat = %d\n",g_buckboost.adc_ibat);
					break;
				case NU6801_ADC_VBUS:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t vbus = row* 120  * 100 / nu6801_vref;
					g_buckboost.adc_vbus = vbus;
					//printk("adc_vbus = %d\n",g_buckboost.adc_vbus);
					break;
				case NU6801_ADC_IBUS:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t ibus = row* 120  * 25 / nu6801_vref;
					if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
						g_buckboost.adc_ibus = ibus;
					else
						g_buckboost.adc_ibus = -ibus;
					//printk("adc_ibus = %d\n",g_buckboost.adc_ibus);
					break;
				case NU6801_ADC_IAC1:
					row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
					uint32_t iac1 = row* 30 * 15  / nu6801_vref;
					g_buckboost.adc_iac1 = iac1;
					//printk("adc_iac1 = %d\n",g_buckboost.adc_iac1);
					break;
				case NU6801_ADC_VREF:
					nu6801_vref =  hal_badc_meas(_BADC_CH_PD3_ADC9);
					//printk("adc_vref = %d\n",nu6801_vref);
					if(nu6801_vref < 1000)
					{
						uint8_t read_0x10,read_0x11,read_0x06,read_0x00;
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read_0x11);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,&read_0x10);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_FAULT_FLAG,&read_0x06);
						hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_FLAG,&read_0x00);
						printk("adc_err [0x00]=0x%x [0x06]=0x%x [0x10]=0x%x [0x11]=0x%x\n",read_0x00,read_0x06,read_0x10,read_0x11);
					}
					break;
			}
		#endif
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
	.is_ibus_loop = 			hal_sw7201_buckboost_is_ibus_loop,
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
	.is_ibus_loop = 			hal_nu6801_buckboost_is_ibus_loop,

#if(BUCKBOOST_USED_NU6801 == 1)
	.get_typeca_vbus_present = 	hal_nu6801_buckboost_typeca_vbus_present,
	.get_typecb_vbus_present = 	hal_nu6801_buckboost_typecb_vbus_present,
	.get_charge_flag = 			hal_nu6801_buckboost_get_charge_flag,
	.get_adc_iac1 = 			hal_nu6801_buckboost_get_iac1,
	.set_ovp = 					hal_nu6801_buckboost_set_ovp,
#endif
};

#endif


