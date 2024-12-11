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
uint16_t port_vbus = 5000;
uint16_t port_defualt_voltage = 5000;
uint8_t wpc_work_mode = TCPM_WPC_WORK_FIX5V;

static uint8_t usba_state = 0;
static uint8_t usba_cnt = 0;
static uint8_t qi_state = 0;
static uint8_t qi_cnt = 0;

#define UBSA_GATA_INDEX 		2

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
	[0] = PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),
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


void tcpm_update_wpc_work_mode(enum wpc_work_mode mode)
{
	wpc_work_mode = mode;

	switch(mode)
	{
		case TCPM_WPC_WORK_FIX5V:
			gd->dig_ping_volt = 5000;
			gd->dig_ping_perd = 144000000/360000;
			gd->dig_ping_duty = 500;
			gd->dig_ping_phas = 0;
			pid_set_volt_limit(5000, 5000, 5000);
			pid_set_freq_limit(144000000/360000, 144000000/360000, 144000000/360000);
			pid_set_duty_limit(500, 500, 500);
			pid_set_phas_limit( 50,  40,   0);
			break;
		case TCPM_WPC_WORK_BOOST:
			gd->dig_ping_volt = 5000;
			gd->dig_ping_perd = 144000000/360000;
			gd->dig_ping_duty = 500;
			gd->dig_ping_phas = 0;
			pid_set_volt_limit(20000, 5000, 5000);
			pid_set_freq_limit(144000000/360000, 144000000/360000, 144000000/360000);
			pid_set_duty_limit(500, 500, 500);
			pid_set_phas_limit( 50,  40,   0);
			break;
		case TCPM_WPC_WORK_ADP_FIX:
		case TCPM_WPC_WORK_PD_PPS:
			break;
	}

	printk("wpc_work_mode= %d\n",mode);
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
				if(g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE) hal_tcpc_set_phy_port(0);

				if(g_tc[1].usb_tc_state != TC_SNK_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
					hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
					osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECA_SNK_ATTACHED);
				}

				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);

				if(g_tc[1].usb_tc_state == TC_SRC_Attached)
				{
					usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
				}

				if(g_tc[1].usb_tc_state == TC_SNK_Attached)
				{
					if(g_tc[1].tc_index == g_tcpc.tc_port_map && usba_state == 0)
					{
						usb_pd_requsrt_voltage(2,9000,2000);
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
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE) hal_tcpc_set_phy_port(1);

				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);

				if(g_tc[0].usb_tc_state != TC_SNK_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
					hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
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
					}

					if(g_tc[0].usb_tc_state == TC_SNK_Attached)
					{
			    		if(g_tc[0].tc_index == g_tcpc.tc_port_map && usba_state == 0)
						{
			    			usb_pd_requsrt_voltage(2,9000,2000);
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

				if(qi_state == 1)
				{
					//wpc stop

					//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);

					if(usba_state == 1)
					{
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
					}
					else if(g_tc[1].usb_tc_state == TC_SNK_Attached ||g_tc[1].usb_tc_state == TC_DRP_TOGGLE)
					{
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						//
						//tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
					}
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
			    	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
			    	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
			    	//updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_PDO_LEVEL1);
				}
			    else
			    {
			    	if(g_tc[1].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
			    		hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
			    		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
						usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
						//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
						updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
			    	}

			    	if(g_tc[1].usb_tc_state == TC_SNK_Attached)
			    	{
			    		if(g_tc[1].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(2,9000,2000);
			    		updata_pdo_of_sink((uint32_t *)sink_pdo_level_1,SIZEOF_SINK_PDO_LEVEL1);

			    		osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECB_SNK_ATTACHED);
			    	}
			    }
		    }

		    if(tc->tc_index == 1)
		    {
			    hal_tcpc_set_gate_en(tc->tc_index,false);
			    if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE)
				{
			    	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
			    	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
				}
			    else
			    {
			    	if(g_tc[0].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
			    		hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
			    		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
						usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
						//osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
						updata_pdo_of_source((uint32_t *)source_pdo_level_1,SIZEOF_SOURCE_PDO_LEVEL1);
			    	}
			    	if(g_tc[0].usb_tc_state == TC_SNK_Attached)
			    	{
			    		//usb_tc_set_state(&g_tc[0],TC_SNK_Attached,enter_state);
			    		if(g_tc[0].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(2,9000,2000);
			    		updata_pdo_of_sink((uint32_t *)sink_pdo_level_1,SIZEOF_SINK_PDO_LEVEL1);
			    		osal_start_timerEx(TCPM_PORT0_TIMER, 50, 0, USB_TASK, TCPM_EVT_TYPECA_SNK_ATTACHED);
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
			hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);

			if(tc->tc_index == 0)
			{
			    if(g_tc[1].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
					usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
					usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
					updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}


			    if(g_tc[1].usb_tc_state == TC_SNK_Attached)
				{
			    	if(g_tc[1].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(1,5000,2000);
		    		osal_start_timerEx(TCPM_CHG_TIMER, 50, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}

			}
			if(tc->tc_index == 1)
			{
			    if(g_tc[0].usb_tc_state == TC_SRC_Attached)
				{
					hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
					usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
					usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);
					updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}

			    if(g_tc[0].usb_tc_state == TC_SNK_Attached)
				{
			    	if(g_tc[0].tc_index == g_tcpc.tc_port_map) usb_pd_requsrt_voltage(1,5000,2000);
		    		osal_start_timerEx(TCPM_CHG_TIMER, 50, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}
			}

		    if(usba_state == 1)
		    {
		    	updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
		    }

			break;
		case TC_SRC_Attached:
			if(tc->tc_index == 0)
			{
				if(g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE)
				{
					hal_tcpc_set_phy_port(0);
					usb_dpdm_select(0);
				}

				if(g_tc[1].usb_tc_state ==  TC_DRP_TOGGLE && usba_state == 0)
				{
					osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
				}
				hal_tcpc_set_gate_en(tc->tc_index,true);

			}
			if(tc->tc_index == 1)
			{
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE)
				{
					hal_tcpc_set_phy_port(1);
					usb_dpdm_select(1);
				}
				if(g_tc[0].usb_tc_state ==  TC_DRP_TOGGLE && usba_state == 0)
				{
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

			break;
		case TCPM_EVT_TYPECB_PORT_STATE_CHANGE:
		#ifdef MULTI_PORT_ALT_MODE
			temp_port_state_change_handle(&g_tc[1]);
		#endif
			break;
		case TCPM_EVT_TYPECA_SNK_ATTACHED:
			if(g_tc[1].usb_tc_state != TC_SNK_Attached)
			{
				printk("typec a sink attached\n");
				hal_tcpc_set_gate_en(0,true);
				osal_start_timerEx(TCPM_CHG_TIMER, 1000, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
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
			usb_pd_snk_dump_pdoinfo();
			uint16_t ibus_limit;
			uint16_t ibat_limit;
			if(g_tc[g_tcpc.tc_port_map].usb_tc_state == TC_SNK_Attached )
			{
				ibus_limit = rdo_op_current(g_usb_pd_s.snk_rdo);
				ibat_limit = 5000;//ibus_limit * pdo_fixed_voltage(g_usb_pd_s.snk_rx_source_cap[rdo_index(g_usb_pd_s.snk_rdo) -1])/ 4000;
				//if(ibat_limit > 1500) ibat_limit = 1500;
			}
			else
			{
				ibus_limit = 500;
				ibat_limit = 500;
			}

			printk("ibat_limit =%d ibus_limit =%d\n",ibat_limit,ibus_limit);
			hal_tcpc_set_snk_charge_current(ibat_limit,ibus_limit);
			break;

		case TCPM_EVT_PD_READY:
			if(g_tcpc.pwr_role == TYPEC_SINK)
			{
				if(g_tc[0].usb_tc_state !=  TC_SRC_Attached && g_tc[1].usb_tc_state !=  TC_SRC_Attached && usba_state == 0)
				{
					//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
					if(port_vbus != 9000) usb_pd_requsrt_voltage(2,9000,pdo_max_current(g_usb_pd_s.snk_rx_source_cap[1]));
					osal_start_timerEx(TCPM_CHG_TIMER, 1000, 0, USB_TASK, TCPM_EVT_SNK_START_CHARGER);
				}
			}
			else
			{
				//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
			}
			break;

		case TCPM_EVT_USBA_SCAN:
			if(g_buckboost.usba_state)
			{
				usba_state = 1;
				osal_set_event(USB_TASK,TCPM_EVT_USBA_PLUG);
			}

			if(usba_state)
			{
				if(g_buckboost.adc_ibus < 160 && g_buckboost.adc_ibus >= 0 )
				{
					usba_cnt++;
					if(usba_cnt >= 10)
					{
						usba_cnt = 0;
						usba_state = 0;
						osal_set_event(USB_TASK,TCPM_EVT_USBA_PLUG);
					}
				}
				else
					usba_cnt = 0;
			}

			if(qi_state == 1 && gd->ptx_protocol_phase <= WPC_PHASE_PING)
			{
				qi_cnt++;
				if(qi_cnt >= 20)
				{
					qi_state = 0;
					qi_cnt = 0;
				}
			}
			else
				qi_cnt = 0;
			//


			printk("qi_state= %d usba_state =%d wpc_mode=%d \n",qi_state,usba_state,wpc_work_mode);
			break;

		case TCPM_EVT_USBA_PLUG:
			if(usba_state)
			{
				if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
				{
					usb_pd_requsrt_voltage(1,5000,2000);
				}
				else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
				{
		    		//hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
		    		hal_tcpc_pd_set_bus_iv(UBSA_GATA_INDEX,5000,3000,0,0);

			    	if(g_tc[0].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[0].tc_index,false);
						usb_pd_set_event(g_tc[0].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[0],TC_SRC_AttachWait,enter_state);

			    	}
			    	if(g_tc[1].usb_tc_state == TC_SRC_Attached)
			    	{
			    		hal_tcpc_set_gate_en(g_tc[1].tc_index,false);
						usb_pd_set_event(g_tc[1].tc_index,USB_PD_EVT_SRC_UNATTACH);
						usb_tc_set_state(&g_tc[1],TC_SRC_AttachWait,enter_state);
			    	}
			    	updata_pdo_of_source((uint32_t *)source_pdo_level_0,SIZEOF_SOURCE_PDO_LEVEL0);
				}
				//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				osal_start_timerEx(TCPM_USB_A_TIMER, 500, 0, USB_TASK, TCPM_EVT_USBA_WORK);
			}
			else
			{
				osal_set_event(USB_TASK,TCPM_EVT_USBA_WORK);
				osal_start_timerEx(TCPM_USB_A_TIMER, 1000, 0, USB_TASK, TCPM_EVT_USBA_DETEN);
				buckboost_ops.usb_a_dischg_en(true);

			}
			break;

		case TCPM_EVT_USBA_WORK:
			if(usba_state)
			{
				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,true);
			}
			else
			{
				hal_tcpc_set_gate_en(UBSA_GATA_INDEX,false);
			    if(g_tc[0].usb_tc_state == TC_DRP_TOGGLE && g_tc[1].usb_tc_state == TC_DRP_TOGGLE && usba_state == 0)
			    {
			    	//wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
			    	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			    }
			}
			break;
		case TCPM_EVT_USBA_DETEN:
			buckboost_ops.usb_a_dischg_en(false);
			buckboost_ops.en_a2_detect();
			break;

		case TCPM_EVT_QI_WORK:
			qi_state = 1;
			qi_cnt = 0;
			break;
#endif
		default:
			break;

	}
}
