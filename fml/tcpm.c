#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "tcpm.h"
#include "pd.h"
#include "typec.h"
#include "buckboost.h"
#include "usb_pd.h"
#include "adp.h"
#include "_wpc.h"
#include "g_data.h"
#include "pid.h"
#include "usb_qc.h"
uint16_t port_vbus = 5000;
uint16_t qi_volt = 5000;
uint16_t port_defualt_voltage = 5000;
uint8_t wpc_mode = TCPM_WPC_WORK_BOOST;
uint8_t wpc_mode_pre = TCPM_WPC_WORK_BOOST;
uint8_t tcpm_qi_work_delay = 0;

static uint8_t usba_state = 0;
static uint8_t usba_cnt = 0;
static uint8_t qi_state = 0;
static uint8_t qi_cnt = 0;

#define WPC_DELAY				10
#define UBSA_GATA_INDEX 		2
#define WPC_INDEX 				3

#define VOLTAGE_5V  5000

void tcpm_task_init(void)
{
	osal_task_handler_reg(USB_TASK, tcpm_task_event_handler);
	osal_start_timerEx(USB_TC_PD_TIMER, 1, 1, USB_TASK, TCPM_EVT_TIME_PERIOD);
	usb_tc_init();
	usb_pd_init();

	//fml_adp_type_set(EADP_TYPE_DCSRC_09V,  9000, 19500, 15 * 2);
}

const uint32_t source_pdo_level_0[] =
{
	#define SOURCE_PDO_FIXED_FLAGS     			(PDO_FIXED_UNCONSTRAINED_POWER)
	[0] = PDO_FIXED(5000, 1500, SOURCE_PDO_FIXED_FLAGS),
	#define SIZEOF_SOURCE_PDO_LEVEL0					sizeof(source_pdo_level_0) / 4
};

const uint32_t source_pdo_level_1[] =
{
	#define SOURCE_PDO_FIXED_FLAGS     			(PDO_FIXED_UNCONSTRAINED_POWER)
	[0] = PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),
	[1] = PDO_FIXED(9000, 2000, 0),
	[2] = PDO_PPS_APDO(5000,11000,2000),
	#define SIZEOF_SOURCE_PDO_LEVEL1					sizeof(source_pdo_level_1) / 4
};

const uint32_t sink_pdo_level_0[] =
{
	#define SINK_PDO_FIXED_FLAGS     			(0)
	[0] = PDO_FIXED(5000, 3000, SINK_PDO_FIXED_FLAGS),
	#define SIZEOF_SINK_PDO_LEVEL0					sizeof(sink_pdo_level_0) / 4
	//[2] = PDO_FIXED(15000, 3000, 0),
};

const uint32_t sink_pdo_level_1[] =
{
	#define SINK_PDO_FIXED_FLAGS     			(0)
	[0] = PDO_FIXED(5000, 3000, SINK_PDO_FIXED_FLAGS),
	[1] = PDO_FIXED(9000, 3000, 0),
	#define SIZEOF_SINK_PDO_LEVEL1					sizeof(sink_pdo_level_1) / 4
};

void tcpm_tc_set_state(struct tc_s * tc,enum usb_tc_state_e tc_state,enum usb_tc_substate_e tc_substate)
{
	tc->usb_tc_state = tc_state;
	tc->usb_tc_substate = tc_substate;
}

void tcpm_stop_wpc(uint8_t delay_ping_unit)
{
	tcpm_qi_work_delay = delay_ping_unit;
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
	printk("wpc stop = %d\n",delay_ping_unit);
}


void tcpm_update_wpc_work_mode(enum wpc_work_mode mode)
{
	wpc_mode = mode;
	switch(mode)
	{
		case TCPM_WPC_WORK_FIX5V:
			fml_adp_type_set(EADP_TYPE_POWERBANK_05V,  5000, 5000, 5 * 2);
		    pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			break;
		case TCPM_WPC_WORK_ADP_FIX:
			fml_adp_type_set(EADP_TYPE_POWERBANK_09V,  9000, 9000, 10 * 2);
			pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			break;
		case TCPM_WPC_WORK_BOOST:
		case TCPM_WPC_WORK_PD_PPS:
			fml_adp_type_set(EADP_TYPE_POWERBANK_WIRELESS_ONLY,  9000, 19500, 15 * 2);
			pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
			printk("\r\n adapter updated! PPS");
			break;
		case TCPM_WPC_WORK_DISABLE:
			break;
	}


	printk("wpc_mode= %d\n",mode);
}

void tcpm_set_default_request_fix_volt(uint16_t fix_volt)
{
	port_defualt_voltage = fix_volt;
}

uint8_t temp_port_state_change_handle(struct tc_s *tc)
{
	printk("port[%d] = %d\n",tc->tc_index,tc->usb_tc_state);

	switch(tc->usb_tc_state)
	{
		case TC_Disable:
		case TC_SNK_AttachWait:
			break;
		case TC_SNK_Attached:

			if(tc->tc_index == 0)
			{

				if(g_tc[1].usb_tc_state != TC_SNK_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
					hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
					hal_tcpc_set_phy_port(tc->tc_index);
					//usb_dpdm_select(tc->tc_index);
					//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
					//dpdm_snk_support
					osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECA_SNK_ATTACHED);
				}

				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);

				if(g_tc[1].usb_tc_state == TC_SRC_Attached)
				{
					usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
					hal_tcpc_set_cc((&g_tc[1])->tc_index,TYPEC_CC_RP_DEF);
					tcpm_stop_wpc(WPC_DELAY);
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				}

				if(g_tc[1].usb_tc_state == TC_SNK_Attached)
				{
					if(g_tc[1].tc_index == g_tcpc.tc_port_map && usba_state == 0)
					{
						uint32_t source_pdo = 0;
						if(g_usb_pd_s.explicit_contract)
						{
							source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
						}
						if(g_buckboost.adc_vbat > 6000)
						{
							if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 20000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
							{
								printk("WPC PPS\n");
								usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,9000,pdo_pps_apdo_max_current(source_pdo));
								tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
							}
							else if(port_vbus != 9000)
							{
								if(g_usb_pd_s.snk_rx_pdo_n >= 2)   //support pd 9v
								{
									usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
								}

							}
							if(bc12_type == BC1P2_QC9V)
							{
								qc2_set_volt(VOLTAGE_9V);
								tcpm_stop_wpc(WPC_DELAY);
								tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
							}
						}
					}
					else
					{
						hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
						hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
						osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECA_SNK_ATTACHED);
					}
				}

			}
			if(tc->tc_index == 1)
			{

				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);

				if(g_tc[0].usb_tc_state != TC_SNK_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
					hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
					hal_tcpc_set_phy_port(tc->tc_index);
					usb_dpdm_select(tc->tc_index);
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
					osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECB_SNK_ATTACHED);
				}

				if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE)
				{
					updata_pdo_of_sink((uint32_t *)sink_pdo_level_1,SIZEOF_SINK_PDO_LEVEL1);  //
				}
				else
				{
					updata_pdo_of_sink((uint32_t *)sink_pdo_level_0,SIZEOF_SINK_PDO_LEVEL0);

					if(g_tc[0].usb_tc_state == TC_SRC_Attached)
					{
						usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
						hal_tcpc_set_cc((&g_tc[0])->tc_index,TYPEC_CC_RP_DEF);
						tcpm_stop_wpc(WPC_DELAY);
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
					}

					if(g_tc[0].usb_tc_state == TC_SNK_Attached)
					{
			    		if(g_tc[0].tc_index == g_tcpc.tc_port_map && usba_state == 0)
						{
			    			//if(port_vbus != 9000 && g_buckboost.adc_vbat > 6000) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
							uint32_t source_pdo = 0;
							if(g_usb_pd_s.explicit_contract)
							{
								source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
							}

							if(g_buckboost.adc_vbat > 6000)
							{
								if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 20000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
								{
									printk("WPC PPS\n");
									usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,9000,pdo_pps_apdo_max_current(source_pdo));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
								}
								else if(port_vbus != 9000)
								{
									if(g_usb_pd_s.snk_rx_pdo_n >= 2) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
								}
								if(bc12_type == BC1P2_QC9V)
								{
									qc2_set_volt(VOLTAGE_9V);
									tcpm_stop_wpc(WPC_DELAY);
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
								}
							}
						}
			    		else
			    		{
							hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
							hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
							osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECB_SNK_ATTACHED);
			    		}
			    		updata_pdo_of_sink((uint32_t *)sink_pdo_level_1,SIZEOF_SINK_PDO_LEVEL1);
					}

				}

				tcpm_stop_wpc(WPC_DELAY);

				if(usba_state == 1)
				{
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				}
				else //if(g_tc[1].usb_tc_state == TC_SNK_Attached ||g_tc[1].usb_tc_state == TC_DRP_TOGGLE)
				{
					//tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
					//
					//tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				}
				//osal_start_timerEx(TCPM_PORT1_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECB_SNK_ATTACHED);
			}
			break;
		case TC_SRC_Unattached:
		case TC_SNK_Unattached:
			break;
		case TC_DRP_TOGGLE:
		    if(tc->tc_index == 0)
		    {
			    hal_tcpc_set_gate_en(tc->tc_index,false);

			    if(g_tc[1].usb_tc_state == TC_DRP_TOGGLE)
				{
			    	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3500,0,0);
			    	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
			    	//updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_PDO_LEVEL1);
				}
			    else
			    {
			    	if(g_tc[1].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
			    		hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3500,0,0);
			    		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
						usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
						hal_tcpc_set_cc((&g_tc[1])->tc_index,TYPEC_CC_RP_3_0);
						//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
			    	}

			    	if(g_tc[1].usb_tc_state == TC_SNK_Attached)
			    	{
			    		if(g_tc[1].tc_index == g_tcpc.tc_port_map && usba_state == 0)
						{
							uint32_t source_pdo = 0;
							if(g_usb_pd_s.explicit_contract)
							{
								source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
							}
							if(g_buckboost.adc_vbat > 6000)
							{
								if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 20000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
								{
									printk("WPC PPS\n");
									usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,9000,pdo_pps_apdo_max_current(source_pdo));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
								}
								else if(port_vbus != 9000 && g_usb_pd_s.explicit_contract)
								{
									if(g_usb_pd_s.snk_rx_pdo_n >= 2) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
								}
							}
						}

			    		printk("bc12=%d map =%d\n",bc12_type,dpdm_map);

						if(bc12_type == BC1P2_QC9V  && usba_state == 0 && dpdm_map == 1)
						{
							qc2_set_volt(VOLTAGE_9V);
							tcpm_stop_wpc(WPC_DELAY);
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						}

			    		osal_start_timerEx(TCPM_PORT0_TIMER, 200, 0, USB_TASK, TCPM_EVT_TYPECB_SNK_ATTACHED);
			    	}
			    }
		    }

		    if(tc->tc_index == 1)
		    {
			    hal_tcpc_set_gate_en(tc->tc_index,false);
			    if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE)
				{
			    	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3500,0,0);
			    	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
				}
			    else
			    {
			    	if(g_tc[0].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
			    		hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3500,0,0);
			    		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
						usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
						hal_tcpc_set_cc((&g_tc[0])->tc_index,TYPEC_CC_RP_3_0);
						//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
						//updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
			    	}

			    	if(g_tc[0].usb_tc_state == TC_SNK_Attached)
			    	{
			    		//usb_tc_set_state(&g_tc[0],TC_SNK_Attached,enter_state);
			    		if(g_tc[1].tc_index == g_tcpc.tc_port_map && usba_state == 0)
			    		{
							uint32_t source_pdo = 0;
							if(g_usb_pd_s.explicit_contract)
							{
								source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
							}
							if(g_buckboost.adc_vbat > 6000)
							{
								if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 20000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
								{
									printk("WPC PPS\n");
									usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,9000,pdo_pps_apdo_max_current(source_pdo));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
								}
								else if(port_vbus != 9000)
								{
									if(g_usb_pd_s.snk_rx_pdo_n >= 2) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
									tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
								}

							}
			    		}

						if(bc12_type == BC1P2_QC9V  && usba_state == 0 && dpdm_map == 0)
						{
							qc2_set_volt(VOLTAGE_9V);
							tcpm_stop_wpc(WPC_DELAY);
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						}

			    		osal_start_timerEx(TCPM_PORT0_TIMER, 200, 0, USB_TASK, TCPM_EVT_TYPECA_SNK_ATTACHED);
			    	}
			    }
		    }

		    if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE && g_tc[1].usb_tc_state == TC_DRP_TOGGLE && usba_state == 0)
		    {
		    	//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
		    	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
		    }
			break;
		case TC_Try_SRC:
		case TC_SRC_AttachWait:

			hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3500,0,0);

			if(usba_state)
			{
				if(dpdm_map == UBSA_GATA_INDEX)
				{
					hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
					usb_dpdm_select(DPDM_PHY_OFF);
					hal_tcpc_pd_set_bus_iv(UBSA_GATA_INDEX,5000,3500,0,0);
					osal_start_timerEx(TCPM_USB_A_TIMER, 200, 0, USB_TASK, TCPM_EVT_USBA_WORK);
				}
			}

			if(tc->tc_index == 0)
			{
			    if(g_tc[1].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
					usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
					usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
					hal_tcpc_set_cc((&g_tc[1])->tc_index,TYPEC_CC_RP_3_0);
					//updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}


			    if(g_tc[1].usb_tc_state == TC_SNK_Attached)
				{
			    	if(g_tc[1].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(1,5000,2000);
			    	if(bc12_type == BC1P2_QC9V) qc2_set_volt(VOLTAGE_5V);
		    		osal_start_timerEx(TCPM_CHG_TIMER, 500, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}

			}
			if(tc->tc_index == 1)
			{
			    if(g_tc[0].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
					usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
					usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
					hal_tcpc_set_cc((&g_tc[0])->tc_index,TYPEC_CC_RP_3_0);
					//updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}

			    if(g_tc[0].usb_tc_state == TC_SNK_Attached)
				{
			    	if(g_tc[0].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(1,5000,2000);
			    	if(bc12_type == BC1P2_QC9V) qc2_set_volt(VOLTAGE_5V);
		    		osal_start_timerEx(TCPM_CHG_TIMER, 500, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}
			}

			tcpm_stop_wpc(WPC_DELAY);
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			break;
		case TC_SRC_Attached:

			if(g_tcpc.tc_port_map == tc->tc_index)
			{
				updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
			}

			if(tc->tc_index == 0)
			{
				if(dpdm_map == tc->tc_index) usb_dpdm_select(DPDM_PHY_OFF);
				if(g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
				{
					hal_tcpc_set_phy_port(0);
				}

				if(g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE && usba_state == 0 && qi_state == 0)
				{
					usb_dpdm_select(0);
					updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
				}
				hal_tcpc_set_gate_en(tc->tc_index,true);

			}
			if(tc->tc_index == 1)
			{
				if(dpdm_map == tc->tc_index) usb_dpdm_select(DPDM_PHY_OFF);
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE)
				{
					hal_tcpc_set_phy_port(1);
				}
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && usba_state == 0 && qi_state == 0)
				{
					usb_dpdm_select(1);
					updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
				}
				hal_tcpc_set_gate_en(tc->tc_index,true);
			}

			break;
		case TC_DEBUG_Attached:
		case TC_Try_SNK:
		case TC_TryWAIT_SRC:
		case TC_TryWAIT_SNK:
		case TC_ErrorRecovery:
			break;
		default:
			break;
	}


	return 0;
}




void tcpm_task_event_handler(uint32_t event)
{
	uint16_t ibus_limit;
	uint16_t ibat_limit;

	switch (event)
	{
		case TCPM_EVT_TIME_PERIOD:
			usb_pdevt_run();
			usb_pd_run();
			usb_tc_run();
			break;
#ifdef MULTI_PORT_ALT_MODE
		case TCPM_EVT_TYPECA_PORT_STATE_CHANGE:

			temp_port_state_change_handle(&g_tc[0]);
			if(!usba_state)
			{
				osal_start_timerEx(TCPM_USB_A_TIMER, 5000, 0, USB_TASK, TCPM_EVT_USBA_PLUG);
			}
			break;
		case TCPM_EVT_TYPECB_PORT_STATE_CHANGE:

			temp_port_state_change_handle(&g_tc[1]);
			if(!usba_state)
			{
				osal_start_timerEx(TCPM_USB_A_TIMER, 5000, 0, USB_TASK, TCPM_EVT_USBA_PLUG);
			}
			break;
		case TCPM_EVT_TYPECA_SNK_ATTACHED:
			if(g_tc[1].usb_tc_state != TC_SNK_Attached)
			{
				printk("typec a sink attached\n");
				hal_tcpc_set_gate_en(0,true);
				osal_start_timerEx(TCPM_CHG_TIMER, 1000, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				usb_dpdm_select(0);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				if(usba_state == 0 && g_tc[1].usb_tc_state == TC_DRP_TOGGLE)
					dpdm_snk_support = 1;
				else
					dpdm_snk_support = 0;
				tcpm_stop_wpc(WPC_DELAY);
			}

			if(usba_state == 1) // can only 5v
			{
				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
			}
			break;
//		case TCPM_EVT_TYPEC1_SRC_ATTACHED:
//			break;
		case TCPM_EVT_TYPECB_SNK_ATTACHED:
			if(g_tc[0].usb_tc_state != TC_SNK_Attached)
			{
				hal_tcpc_set_gate_en(1,true);
				osal_start_timerEx(TCPM_CHG_TIMER, 1000, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				usb_dpdm_select(1);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				if(usba_state == 0 && g_tc[0].usb_tc_state == TC_DRP_TOGGLE)
					dpdm_snk_support = 1;
				else
					dpdm_snk_support = 0;
				tcpm_stop_wpc(WPC_DELAY);
			}

			if(usba_state == 1) // can only 5v
			{
				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
			}
			break;
//		case TCPM_EVT_TYPEC2_SRC_ATTACHED:
//			break;
//		case TCPM_EVT_PORTA_ATTACHED:
//			break;
//		case TCPM_EVT_WPC_ATTACHED:
//			break;
		case TCPM_EVT_SNK_START_CHARGER:
			//usb_pd_snk_dump_pdoinfo();

			if(g_usb_pd_s.explicit_contract)  //PD
			{
				if(qi_state == 0 && usba_state == 0 && g_tc[0].usb_tc_state !=  TC_SRC_Attached && g_tc[1].usb_tc_state !=  TC_SRC_Attached )
				{
					ibus_limit = rdo_op_current(g_usb_pd_s.snk_rdo) * 80 / 100;
					ibat_limit = 3000;
				}
				else
				{
					ibus_limit = 500;
					ibat_limit = 500;
				}
				//ibus_limit * pdo_fixed_voltage(g_usb_pd_s.snk_rx_source_cap[rdo_index(g_usb_pd_s.snk_rdo) -1])/ 4000;
				//if(ibat_limit > 1500) ibat_limit = 1500;
			}
			else //QC /SDP /CDP /DCP
			{
				if(bc12_type == BC1P2_QC9V )
				{
					if(qi_state == 0 && usba_state == 0 && g_tc[0].usb_tc_state !=  TC_SRC_Attached && g_tc[1].usb_tc_state !=  TC_SRC_Attached )
					{
						ibus_limit = 1500;
						ibat_limit = 3000;
					}
					else
					{
						ibus_limit = 500;
						ibat_limit = 500;
					}
				}
				else if(bc12_type == BC1P2_DCP ||  bc12_type == BC1P2_CDP || bc12_type == BC1P2_HVDCP)
				{
					if(qi_state == 0 && usba_state == 0 && g_tc[0].usb_tc_state !=  TC_SRC_Attached && g_tc[1].usb_tc_state !=  TC_SRC_Attached )
					{
						ibus_limit = 1200;
						ibat_limit = 3000;
					}
					else
					{
						ibus_limit = 500;
						ibat_limit = 500;
					}
				}
				else// if(bc12_type == BC1P2_SDP )
				{
					ibus_limit = 300;
					ibat_limit = 500;
				}

			}

			printk("ibat_limit =%d ibus_limit =%d\n",ibat_limit,ibus_limit);
			hal_tcpc_set_snk_charge_current(ibat_limit,ibus_limit);

//			if(!usba_state)
//			{
//				osal_start_timerEx(TCPM_USB_A_TIMER, 1000, 0, USB_TASK, TCPM_EVT_USBA_PLUG);
//			}
//			else
//				osal_set_event(USB_TASK,TCPM_EVT_USBA_WORK);
			break;

		case TCPM_EVT_PD_READY:
			if(g_tcpc.pwr_role == TYPEC_SINK)
			{
				//if(g_tc[0].usb_tc_state !=  TC_SRC_Attached && g_tc[1].usb_tc_state !=  TC_SRC_Attached && usba_state == 0)TC_Try_SRC

				if(((g_tc[0].usb_tc_state ==  TC_SNK_Attached && (g_tc[1].usb_tc_state ==  TC_SNK_Attached || g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE  ))
						|| (g_tc[1].usb_tc_state ==  TC_SNK_Attached && (g_tc[0].usb_tc_state ==  TC_SNK_Attached || g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE )))
						&&usba_state == 0)
				{
					uint32_t source_pdo = 0;
					if(g_usb_pd_s.explicit_contract)
					{
						source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
					}
					if(g_buckboost.adc_vbat > 6000)
					{
						if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 20000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
						{
							if(g_usb_pd_s.explicit_contract && !g_usb_pd_s.is_in_pps)
							{
								usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,9000,pdo_pps_apdo_max_current(source_pdo));
								tcpm_stop_wpc(WPC_DELAY);
							}
							printk("WPC PPS\n");
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
						}
						else if(port_vbus != 9000)
						{
							if(g_usb_pd_s.snk_rx_pdo_n >= 2) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
							tcpm_stop_wpc(WPC_DELAY);
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						}
						if(bc12_type == BC1P2_QC9V)
						{
							qc2_set_volt(VOLTAGE_9V);
							tcpm_stop_wpc(WPC_DELAY);
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						}
					}
					osal_start_timerEx(TCPM_CHG_TIMER, 1000, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}
			}
			else
			{
				tcpm_stop_wpc(WPC_DELAY);
				if(g_buckboost.buckboost_out_voltage >= 8000 && g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			break;

		case TCPM_EVT_USBA_SCAN:
			if(g_buckboost.usba_state)
			{
				usba_state = 1;
				usba_cnt = 0;
				osal_set_event(USB_TASK,TCPM_EVT_USBA_PLUG);
			}

			if(usba_state)
			{
				if(g_buckboost.adc_ibus >= -100 && g_buckboost.adc_ibus <= 0 )
				{
					usba_cnt++;
					if(usba_cnt >= 50)
					{
						usba_cnt = 0;
						usba_state = 0;
						buckboost_ops.usb_a_gate_en(false);
						osal_set_event(USB_TASK,TCPM_EVT_USBA_PLUG);
						if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
						{
							updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
						}
					}
				}
				else
					usba_cnt = 0;
			}

			if(qi_state == 1 && gd->ptx_protocol_phase <= WPC_PHASE_PING)
			{
				qi_cnt++;
				if(qi_cnt >= 100)
				{
					qi_state = 0;
					qi_cnt = 0;
					if(g_tc[0].usb_tc_state ==  TC_SNK_Attached || g_tc[1].usb_tc_state ==  TC_SNK_Attached)
						osal_set_event(USB_TASK,TCPM_EVT_SNK_START_CHARGER);
					if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
					{
						updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
					}
				}
			}
			else
				qi_cnt = 0;
			//

			printk("usba_cnt =%d\n",usba_cnt);
			printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_mode);
			break;

		case TCPM_EVT_USBA_PLUG:
			if(usba_state)
			{
				if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					usb_pd_requsrt_voltage(1,5000,2000);
					if(bc12_type == BC1P2_QC9V) qc2_set_volt(VOLTAGE_5V);
					osal_set_event(USB_TASK,TCPM_EVT_SNK_START_CHARGER);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
				{
		    		//hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
		    		hal_tcpc_pd_set_bus_iv(UBSA_GATA_INDEX,5000,3500,0,0);

			    	if(g_tc[0].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
						usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
						hal_tcpc_set_cc((&g_tc[0])->tc_index,TYPEC_CC_RP_3_0);

			    	}
			    	if(g_tc[1].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
						usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
						hal_tcpc_set_cc((&g_tc[1])->tc_index,TYPEC_CC_RP_3_0);
			    	}

			    	updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}
				tcpm_stop_wpc(WPC_DELAY);
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				osal_start_timerEx(TCPM_USB_A_TIMER, 500, 0, USB_TASK, TCPM_EVT_USBA_WORK);
			}
			else
			{
				osal_set_event(USB_TASK,TCPM_EVT_USBA_WORK);
				osal_start_timerEx(TCPM_USB_A_TIMER, 300, 0, USB_TASK, TCPM_EVT_USBA_DETEN);
				buckboost_ops.usb_a_dischg_en(true);
			}

			if(usba_state == 0 && g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
			{
				tcpm_stop_wpc(WPC_DELAY);
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			}

			break;

		case TCPM_EVT_USBA_WORK:
			if(usba_state)
			{
				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,true);
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE && qi_state == 0)
				{
					usb_dpdm_select(UBSA_GATA_INDEX);
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
				}

			}
			else
			{
				//hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
			    if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE && g_tc[1].usb_tc_state == TC_DRP_TOGGLE && usba_state == 0)
			    {
			    	tcpm_stop_wpc(WPC_DELAY);
			    	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			    }
			}
			break;
		case TCPM_EVT_USBA_DETEN:
			buckboost_ops.usb_a_dischg_en(false);
			buckboost_ops.en_a2_detect();
			buckboost_ops.vbus_dischg_en(true);
			buckboost_ops.vbus_dischg_en(false);
			printk("enable A det\n");
			break;

		case TCPM_EVT_QI_WORK:

			if(qi_state == 0)   //无线充接入事件发生
			{
				qi_state = 1;

				if(g_tc[0].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
					usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
					hal_tcpc_set_cc((&g_tc[0])->tc_index,TYPEC_CC_RP_3_0);
				}

				if(g_tc[1].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
					usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
					hal_tcpc_set_cc((&g_tc[1])->tc_index,TYPEC_CC_RP_3_0);
				}

				if(usba_state)
				{
					if(dpdm_map == UBSA_GATA_INDEX)
					{
						hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
						usb_dpdm_select(DPDM_PHY_OFF);
						hal_tcpc_pd_set_bus_iv(UBSA_GATA_INDEX,5000,3500,0,0);
						osal_start_timerEx(TCPM_USB_A_TIMER, 200, 0, USB_TASK, TCPM_EVT_USBA_WORK);
					}
				}
			}
			//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				osal_set_event(USB_TASK,TCPM_EVT_SNK_START_CHARGER);
			else {}

			break;
		case TCPM_EVT_QI_SET_VOLT:
			if(wpc_mode == TCPM_WPC_WORK_BOOST)
			{
				hal_tcpc_pd_set_bus_iv(WPC_INDEX,qi_volt,3500,0,0);
			}
			else if(wpc_mode == TCPM_WPC_WORK_PD_PPS)
			{
				uint32_t source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
				usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,qi_volt,pdo_pps_apdo_max_current(source_pdo));
				printk("pd set volt = %d\n",qi_volt);
			}
			break;
		case TCPM_EVT_DPDM_DONE:
//			osal_set_event(USB_TASK,TCPM_EVT_SNK_START_CHARGER);
			if(g_usb_pd_s.explicit_contract == 0)
			{
				tcpm_stop_wpc(WPC_DELAY);
				if(bc12_type == BC1P2_QC9V )
				{
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				}
				else if(bc12_type == BC1P2_DCP ||  bc12_type == BC1P2_CDP || bc12_type == BC1P2_HVDCP)
				{
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
				}
				else if(bc12_type == BC1P2_SDP )
				{
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
				}
				osal_set_event(USB_TASK,TCPM_EVT_SNK_START_CHARGER);
			}
			break;
#endif
		default:
			break;

	}
}
