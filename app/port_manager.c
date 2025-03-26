#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "port_manager.h"
#include "pd.h"
#include "typec.h"
#include "buckboost.h"
#include "usb_pd.h"
#include "adp.h"
#include "_wpc.h"
#include "g_data.h"
#include "pid.h"
#include "usb_qc.h"
#include "tcpm.h"
#include "nu6801.h"
#include "ntc.h"

void port_manager_set_event(uint32_t event)
{
	g_port.port_event |= event;

	gd->idle_to_sleep_cnt = 0;

	printk("%s=0x%x!\n",__func__,event);
}


void port_manager_set_state(enum port_state_e state)
{
	g_port.state = state;
}

void port_manager_task_init(void)
{
	osal_mem_clear(&g_port,sizeof(struct port_infos));
	osal_task_handler_reg(PORT_MANAGER_TASK, port_manager_event_handle);
	osal_start_timerEx(PORT_ENUM_TIMER, PORT_ENUM_PERIOD, PORT_ENUM_PERIOD, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT_SCAN);

	hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);

	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);

}


void port_enum_port0_connect_closed(void)
{
	printk("%s!\n",__func__);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	tcpm_disable_usba_detect();
	hal_tcpc_set_gate_en(g_port.inhandle_port,false);
	g_port.port_state[g_port.inhandle_port] = PORT_STATE_NONE;

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.incharge_port == PORT0_INDEX)
		{
			if(g_port.port_state[PORT1_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT1_INDEX,false);
				usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
				usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
				usb_dpdm_select(DPDM_PHY_OFF);
				g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			}

			if(g_port.port_state[PORT2_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT2_INDEX,false);
				usb_dpdm_select(DPDM_PHY_OFF);
				port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			}

			port_manager_set_state(PORT_IDLE_OR_READY);
		}
		else
		{
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK &&  g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_SINK_SETVOLT);
				g_port.inhandle_port = 1;
			}
			else
			{
				port_manager_set_state(PORT_IDLE_OR_READY);
			}
		}

	}
	else
	{
		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_SRC_Unattached,enter_state);
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			usb_dpdm_select(DPDM_PHY_OFF);
		}

		if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
			usb_dpdm_select(DPDM_PHY_OFF);
		}

		port_manager_set_state(PORT_IDLE_OR_READY);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE  && g_tc[PORT0_INDEX].usb_tc_state == TC_Disable)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_tc[PORT1_INDEX].usb_tc_state == TC_Disable)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)
	{
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
	{
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	}

	tcpm_stop_wpc(WPC_DELAY);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
				&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)

		{
			if(g_usb_pd_s.explicit_contract)
			{
				if( g_usb_pd_s.is_in_pps)  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if(bc12_type > BC1P2_CDP)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}

		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}

	}
	else
	{
		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE
				&& g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}
	}
}

void port_enum_port1_connect_closed(void)
{
	printk("%s!\n",__func__);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	hal_tcpc_set_gate_en(g_port.inhandle_port,false);
	tcpm_disable_usba_detect();
	g_port.port_state[g_port.inhandle_port] = PORT_STATE_NONE;

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.incharge_port == PORT1_INDEX)
		{
			if(g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT0_INDEX,false);
				usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
				usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
				usb_dpdm_select(DPDM_PHY_OFF);
				g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			}

			if(g_port.port_state[PORT2_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT2_INDEX,false);
				usb_dpdm_select(DPDM_PHY_OFF);
				port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			}
			port_manager_set_state(PORT_IDLE_OR_READY);
		}
		else
		{
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK &&  g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
				g_port.inhandle_port = 0;
			}
			else
			{
				port_manager_set_state(PORT_IDLE_OR_READY);
			}
		}
	}
	else
	{
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_SRC_Unattached,enter_state);
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			usb_dpdm_select(DPDM_PHY_OFF);
		}

		if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
			usb_dpdm_select(DPDM_PHY_OFF);
		}
		port_manager_set_state(PORT_IDLE_OR_READY);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)
	{
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	}

	tcpm_stop_wpc(WPC_DELAY);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
				&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)

		{
			if(g_usb_pd_s.explicit_contract)
			{
				if( g_usb_pd_s.is_in_pps)  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if( rdo_index(g_usb_pd_s.snk_rdo) == PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if( rdo_op_current(g_usb_pd_s.snk_rdo) >= 1500 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if(bc12_type > BC1P2_CDP)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}

		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}

	}
	else
	{
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE
				&& g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}
	}
}

void port_enum_port2_connect_closed(void)
{
	printk("%s!\n",__func__);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	tcpm_disable_usba_detect();
	hal_tcpc_set_gate_en(g_port.inhandle_port,false);
	g_port.port_state[g_port.inhandle_port] = PORT_STATE_NONE;

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)
	{
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	}

	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE &&  g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)
	{
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(g_port.incharge_port == PORT0_INDEX)
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
			}
			else
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_SINK_SETVOLT);
			}
			g_port.inhandle_port = g_port.incharge_port;
		}
	}



	tcpm_stop_wpc(WPC_DELAY);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
				&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)

		{
			if(g_usb_pd_s.explicit_contract)
			{
				if( g_usb_pd_s.is_in_pps)  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if( rdo_index(g_usb_pd_s.snk_rdo) == PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if( rdo_op_current(g_usb_pd_s.snk_rdo) >= 1500 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if(bc12_type > BC1P2_CDP)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}

		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}

	}
	else
	{

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE
				&& g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}
	}

	port_manager_set_state(PORT_IDLE_OR_READY);
}

void port_enum_port3_connect_closed(void)
{
	printk("%s!\n",__func__);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	hal_tcpc_set_gate_en(g_port.inhandle_port,false);
	tcpm_disable_usba_detect();
	g_port.port_state[g_port.inhandle_port] = PORT_STATE_NONE;

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)
	{
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	}

	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE &&  g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)
	{
		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(g_port.incharge_port == PORT0_INDEX)
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
			}
			else
			{
				osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_SINK_SETVOLT);
			}
			g_port.inhandle_port = g_port.incharge_port;
		}
	}

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
				&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)
		{
			if(g_usb_pd_s.explicit_contract)
			{
				if( g_usb_pd_s.is_in_pps)  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if( rdo_index(g_usb_pd_s.snk_rdo) == PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if( rdo_op_current(g_usb_pd_s.snk_rdo) >= 1500 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else if(bc12_type > BC1P2_CDP)
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}

	}
	else
	{
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE
				&& g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);

		}
		else
		{
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
		}
	}


	port_manager_set_state(PORT_IDLE_OR_READY);
}

void port_enum_port_enum_done(void)
{
	printk("%s!\n",__func__);


	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE)
	{
		if(g_tcpc.tc_port_map != PORT0_INDEX || dpdm_map != PORT0_INDEX) tcpm_set_port_sdp(PORT0_INDEX);  // 500mA放电
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		hal_tcpc_set_gate_en(PORT0_INDEX,true);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE)
	{
		if(g_tcpc.tc_port_map != PORT1_INDEX || dpdm_map != PORT1_INDEX) tcpm_set_port_sdp(PORT1_INDEX);  // 500mA放电
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		hal_tcpc_set_gate_en(PORT1_INDEX,true);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
	{
		if(dpdm_map != PORT2_INDEX) tcpm_set_port_sdp(PORT2_INDEX);  // 500mA放电s
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		hal_tcpc_set_gate_en(PORT2_INDEX,true);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT0_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 重新开启toogle
	{
		usb_tc_set_state(&g_tc[PORT1_INDEX],TC_DRP_TOGGLE,enter_state);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 重新开启A口检测
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}

	if(g_port.inhandle_port == USBA_INDEX && g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE &&
			g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
	{
		usb_dpdm_select(USBA_INDEX);
		osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
	}

	if(g_port.inhandle_port != WPC_INDEX)
	{
		tcpm_stop_wpc(WPC_DELAY);

		if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		{
			if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
					&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)

			{
				if(g_usb_pd_s.explicit_contract)
				{
					if( g_usb_pd_s.is_in_pps)
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
					else
					{
						if( rdo_index(g_usb_pd_s.snk_rdo) >= PDO_INDEX_2 )
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
						else
							tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
					}

				}
				else
				{
					if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
					else
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
				}

			}
			else
			{
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}

		}
		else
		{
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE
					&& g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
			{
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			}
			else
			{
				tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
		}
	}
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && g_port.inhandle_port == WPC_INDEX )
	{
	    port_manager_set_event(PORT_EVENT_RESET_CHARGE);
	    printk("%s\n",__func__);
	}

	port_manager_set_state(PORT_IDLE_OR_READY);
}

void port_enum_port_snk_setcharge(void)
{
	printk("%s!\n",__func__);

	hal_tcpc_set_gate_en(g_port.incharge_port,true);

	gd->bat_dead_flag = 0;

	if(g_port.port_state[PORT3_INDEX] == PORT_STATE_SOURCE)// wireless present.
	{
		if(g_buckboost.adc_vbus<5500)// 5v
		{
			g_port.ibat_limit = (g_port.adpater_power > 8000)? (g_port.adpater_power - 8000)/5:500;
		}
		else if (g_buckboost.adc_vbus<9500)// 9v
		{
			g_port.ibat_limit = (g_port.adpater_power > 11000)? (g_port.adpater_power - 11000)/9:500;
		}
		else//12v, reserved for future 12 use.
		{
			g_port.ibat_limit = (g_port.adpater_power > 12000)? (g_port.adpater_power - 12000)/12:500;
		}
		//g_port.ibat_limit = g_port.ibat_limit < 500 ? g_port.ibat_limit : 500;
		g_port.ibus_limit = g_port.ibus_limit* 95 / 100;
		g_port.ibat_limit = g_port.ibat_limit < 2000 ? g_port.ibat_limit : 2000;

	#if(BUCKBOOST_USED_NU6801 == 1)
		g_port.ibus_limit = g_port.ibus_limit < 2000 ? g_port.ibus_limit : 2000;
	#endif
	}
	else
	{
		g_port.ibat_limit = g_port.ibat_limit;
		g_port.ibus_limit = g_port.ibus_limit;

		if(g_buckboost.adc_vbus<5500)// 5v
		{
		#if(BUCKBOOST_USED_NU6801 == 1)
			g_port.ibus_limit = g_port.ibus_limit < 3000 ? g_port.ibus_limit : 3000;
		#endif
		}
		else if(g_buckboost.adc_vbus < 9500)// 5v
		{
		#if(BUCKBOOST_USED_NU6801 == 1)
			g_port.ibus_limit = g_port.ibus_limit < 2000 ? g_port.ibus_limit : 2000;
		#endif
		}
		else
		{
		#if(BUCKBOOST_USED_NU6801 == 1)
			g_port.ibus_limit = g_port.ibus_limit < 1500 ? g_port.ibus_limit : 1500;
		#endif
		}
	}

#if(BUCKBOOST_USED_NU6801 == 1)
	g_port.ibat_limit = g_port.ibat_limit < 5000 ? g_port.ibat_limit : 5000;
#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
	if(ntc_ut_flag | ntc_ot_flag)
	{
		g_port.ibat_limit = g_port.ibat_limit / 2;
		g_port.ibus_limit = g_port.ibus_limit / 2;
	}
#endif
#endif
	if(g_tc[TYPEC_PORT_A].is_deadbattery) g_port.ibus_limit =  g_port.ibus_limit < 500 ? g_port.ibus_limit : 500;

	buckboost_set_charge_current(g_port.ibat_limit,g_port.ibus_limit);

	if(g_port.inhandle_port == PORT0_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 100, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_ENUM_DONE);
	else if(g_port.inhandle_port == PORT1_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 100, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);


#if(BUCKBOOST_USED_NU6801 == 1)
	if(nu6801_dead_bat) hal_nu6801_buckboost_enter_force_trickle(true);
	#if(CONFIG_USE_NTC_FOR_CHAGER == 1)
	if(ntc_stop_chrg_flag) hal_nu6801_disable_bubo();
	#endif
#endif

#if(BUCKBOOST_USED_NU6801 == 1)

	buckboost_ops.set_ovp(g_port.snk_set_volt);
#endif

	printk("[%d]Power=%dmW I[bat]=%dmA I[bus]=%dmA!\n",g_port.inhandle_port,g_port.adpater_power,g_port.ibat_limit,g_port.ibus_limit);

}

void port_enum_port_snk_setvolt(void)
{

	uint32_t source_pdo;
	hal_tcpc_set_gate_en(g_port.incharge_port,false);
	printk("[%d]%s!\n",g_port.inhandle_port,__func__);

	g_port.ibus_limit = 1000;
	g_port.ibat_limit = 1000;

	buckboost_ops.set_ovp(20000);

	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
			&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE && (!g_tc[TYPEC_PORT_A].is_deadbattery))
	{

		if(g_usb_pd_s.explicit_contract)
		{
#if(BUCKBOOST_USED_SW7201 == 1)
			source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - 1];
			if(pdo_type(source_pdo) == PDO_TYPE_APDO && pdo_pps_apdo_max_voltage(source_pdo) >= 16000 && pdo_pps_apdo_max_current(source_pdo) >= 2500)
			{
				usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n,VOLTAGE_PPS,pdo_pps_apdo_max_current(source_pdo));
				g_port.ibus_limit =  pdo_pps_apdo_max_current(source_pdo);

				//g_port.adpater_power =  (uint32_t)g_port.ibus_limit * pdo_pps_apdo_max_voltage(source_pdo) / 1000;
				g_port.adpater_power =  (uint32_t)g_port.ibus_limit * 11000 / 1000;
			}
			else
#endif
			for(uint8_t i = 0; i< g_usb_pd_s.snk_rx_pdo_n; i++)
			{
				source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[g_usb_pd_s.snk_rx_pdo_n - i - 1];

				if(pdo_type(source_pdo) == PDO_TYPE_FIXED)
				{
					if(pdo_fixed_voltage(source_pdo) <= VOLTAGE_12V)
					{
						usb_pd_requsrt_voltage(g_usb_pd_s.snk_rx_pdo_n - i,pdo_fixed_voltage(source_pdo),pdo_max_current(source_pdo));
						g_port.snk_set_volt = pdo_fixed_voltage(source_pdo);
						g_port.ibus_limit = pdo_max_current(source_pdo);
						g_port.adpater_power =  (uint32_t)g_port.ibus_limit * pdo_fixed_voltage(source_pdo) / 1000;
						break;
					}
				}
			}

			g_port.ibat_limit = 5500;
		}
		else if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
		{
			if(bc12_type == BC1P2_QC12V)
			{
				qc2_set_volt(VOLTAGE_12V);
				g_port.ibus_limit = 1500;
				g_port.snk_set_volt = VOLTAGE_12V;
				g_port.adpater_power =  (uint32_t)2000 * VOLTAGE_12V / 1000;
			}
			else
			{
				qc2_set_volt(VOLTAGE_9V);
				g_port.snk_set_volt = VOLTAGE_9V;
				g_port.ibus_limit = 2000;
				g_port.adpater_power =  (uint32_t)2000 * VOLTAGE_9V / 1000;
			}
			g_port.ibat_limit = 5000;
		}
		else
		{
			if(bc12_type >= BC1P2_HVDCP)
			{
				g_port.adpater_power =  (uint32_t)3000 * VOLTAGE_5V / 1000;
				g_port.ibus_limit = 3000;
				g_port.ibat_limit = 5000;
			}
			else if(bc12_type > BC1P2_CDP)
				g_port.adpater_power =  (uint32_t)1500 * VOLTAGE_5V / 1000;
			else
				g_port.adpater_power =  (uint32_t)500 * VOLTAGE_5V / 1000;

			g_port.snk_set_volt = VOLTAGE_5V;
		}
	}
	else
	{
		if(g_usb_pd_s.explicit_contract)
		{
			source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[0];
			g_port.adpater_power =  pdo_max_current(source_pdo) * VOLTAGE_5V / 1000;
		}
		else
		{
			if(bc12_type >= BC1P2_HVDCP)
			{
				g_port.adpater_power =  (uint32_t)3000 * VOLTAGE_5V / 1000;
			}
			else if(bc12_type > BC1P2_CDP)
				g_port.adpater_power =  (uint32_t)1500 * VOLTAGE_5V / 1000;
			else
				g_port.adpater_power =  (uint32_t)500 * VOLTAGE_5V / 1000;
		}
		g_port.snk_set_volt = VOLTAGE_5V;
		g_port.ibus_limit = 1000;
		g_port.ibat_limit = 1000;
	}

	if(g_port.inhandle_port == PORT0_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETCHARGE);
	else
		osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_SINK_SETCHARGE);

	printk("sdp_type = %d\n",bc12_type);
	//printk("I[bat]=%dmA I[bus]=%dmA!\n",g_port.ibat_limit,g_port.ibus_limit);
}

void port_enum_port0_connect_success(void)
{
	printk("%s!\n",__func__);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_tc[PORT0_INDEX].usb_tc_state == TC_SNK_Attached)
		{
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!g_usb_pd_s.explicit_contract && bc12_type != BC1P2_QC9V)
			{
				usb_dpdm_select(PORT0_INDEX);
				hal_tcpc_set_phy_port(PORT0_INDEX);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				usb_pd_set_event(PORT0_INDEX,USB_PD_EVT_SNK_ATTACHED);

				if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK) hal_tcpc_set_gate_en(PORT1_INDEX,false);
				//hal_tcpc_set_gate_en(PORT0_INDEX,true);
				g_port.incharge_port = PORT0_INDEX;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_ENUM_DONE);
		}
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		if(g_tc[PORT0_INDEX].usb_tc_state == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);

			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!g_usb_pd_s.explicit_contract && bc12_type != BC1P2_QC9V)
			{
				usb_dpdm_select(PORT0_INDEX);
				hal_tcpc_set_phy_port(PORT0_INDEX);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				usb_pd_set_event(PORT0_INDEX,USB_PD_EVT_SNK_ATTACHED);
				//hal_tcpc_set_gate_en(PORT0_INDEX,true);
				if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK) hal_tcpc_set_gate_en(PORT1_INDEX,false);
				g_port.incharge_port = PORT0_INDEX;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			hal_tcpc_set_gate_en(PORT0_INDEX,true);
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
			{
				usb_dpdm_select(PORT0_INDEX);
				hal_tcpc_set_phy_port(PORT0_INDEX);
				usb_pd_set_event(PORT0_INDEX,USB_PD_EVT_SRC_ATTACHED);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 10, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);
		}
	}

	if(g_tc[PORT0_INDEX].usb_tc_state == TC_SNK_Attached) g_port.port_state[PORT0_INDEX] = PORT_STATE_SINK;
	if(g_tc[PORT0_INDEX].usb_tc_state == TC_SRC_Attached) g_port.port_state[PORT0_INDEX] = PORT_STATE_SOURCE;
}

void port_enum_port1_connect_success(void)
{
	printk("%s!\n",__func__);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_tc[PORT1_INDEX].usb_tc_state == TC_SNK_Attached)
		{
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!g_usb_pd_s.explicit_contract && bc12_type != BC1P2_QC9V)
			{
				usb_dpdm_select(PORT1_INDEX);
				hal_tcpc_set_phy_port(PORT1_INDEX);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				usb_pd_set_event(PORT1_INDEX,USB_PD_EVT_SNK_ATTACHED);

				if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK) hal_tcpc_set_gate_en(PORT0_INDEX,false);
				//hal_tcpc_set_gate_en(PORT1_INDEX,true);
				g_port.incharge_port = PORT1_INDEX;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);
		}
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		if(g_tc[PORT1_INDEX].usb_tc_state == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);

			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!g_usb_pd_s.explicit_contract && bc12_type != BC1P2_QC9V)
			{
				usb_dpdm_select(PORT1_INDEX);
				hal_tcpc_set_phy_port(PORT1_INDEX);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				usb_pd_set_event(PORT1_INDEX,USB_PD_EVT_SNK_ATTACHED);
				//hal_tcpc_set_gate_en(PORT1_INDEX,true);
				if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SINK) hal_tcpc_set_gate_en(PORT0_INDEX,false);
				g_port.incharge_port = PORT1_INDEX;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			hal_tcpc_set_gate_en(PORT1_INDEX,true);
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
			{
				usb_dpdm_select(PORT1_INDEX);
				hal_tcpc_set_phy_port(PORT1_INDEX);
				usb_pd_set_event(PORT1_INDEX,USB_PD_EVT_SRC_ATTACHED);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 10, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);
		}
	}

	if(g_tc[PORT1_INDEX].usb_tc_state == TC_SNK_Attached) g_port.port_state[PORT1_INDEX] = PORT_STATE_SINK;
	if(g_tc[PORT1_INDEX].usb_tc_state == TC_SRC_Attached) g_port.port_state[PORT1_INDEX] = PORT_STATE_SOURCE;
}

void port_enum_port2_connect_success(void)
{
	printk("%s!\n",__func__);
	osal_start_timerEx(PORT_CONNECT_TIMER, 200, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT2_ENUM_DONE);
	g_port.port_state[PORT2_INDEX] = PORT_STATE_SOURCE;
}

void port_enum_port3_connect_success(void)
{
	printk("%s!\n",__func__);
	osal_start_timerEx(PORT_CONNECT_TIMER, 200, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT3_ENUM_DONE);
	g_port.port_state[PORT3_INDEX] = PORT_STATE_SOURCE;
}



void port_enum_port0_connect_start(void)
{
	printk("PORT0 START! PORT1=[%d] PORT2=[%d] PORT3=[%d]\n",g_port.port_state[1],g_port.port_state[2],g_port.port_state[3]);

	uint32_t source_pdo = 0;
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	tcpm_disable_usba_detect();
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT1_INDEX],TC_Disable,enter_state);
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT1_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[0];
		if(g_usb_pd_s.explicit_contract) usb_pd_requsrt_voltage(1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_snk_charge_current(CHG_IBUS_MIN,CHG_IBAT_MIN);    //设置充电电流到最小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没有snk
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_dpdm_select(DPDM_PHY_OFF);
		hal_tcpc_set_gate_en(PORT1_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	}

}


void port_enum_port1_connect_start(void)
{
	printk("PORT1 START! PORT0=[%d] PORT2=[%d] PORT3=[%d]\n",g_port.port_state[0],g_port.port_state[2],g_port.port_state[3]);

	uint32_t source_pdo = 0;
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_disable_usba_detect();
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT0_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[0];
		if(g_usb_pd_s.explicit_contract) usb_pd_requsrt_voltage(1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_snk_charge_current(CHG_IBUS_MIN,CHG_IBAT_MIN);    //设置充电电流到最小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没有snk
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_dpdm_select(DPDM_PHY_OFF);
		hal_tcpc_set_gate_en(PORT1_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	}
}

void port_enum_port2_connect_start(void)
{
	printk("PORT2 START! PORT0=[%d] PORT1=[%d] PORT3=[%d]\n",g_port.port_state[0],g_port.port_state[1],g_port.port_state[3]);

	uint32_t source_pdo = 0;
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_disable_usba_detect();
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT1_INDEX],TC_Disable,enter_state);
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[0];
		if(g_usb_pd_s.explicit_contract) usb_pd_requsrt_voltage(1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,100);
		hal_tcpc_set_snk_charge_current(CHG_IBUS_MIN,CHG_IBAT_MIN);    //设置充电电流到最小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没有snk
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_dpdm_select(DPDM_PHY_OFF);
		hal_tcpc_set_gate_en(PORT1_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	}

	osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT2_CONNECT_SUCCESS);
}

void port_enum_port3_connect_start(void)
{

	//uint32_t source_pdo = 0;

	printk("PORT3 START! PORT0=[%d] PORT1=[%d] PORT2=[%d]\n",g_port.port_state[0],g_port.port_state[1],g_port.port_state[2]);
	tcpm_disable_usba_detect();
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  usb_tc_set_state(&g_tc[PORT1_INDEX],TC_Disable,enter_state);

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_tcpc_set_snk_charge_current(CHG_IBUS_MIN,CHG_IBAT_MIN);    //设置充电电流到最小
	}
	else
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_dpdm_select(DPDM_PHY_OFF);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT0_INDEX,false);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT1_INDEX,false);
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT2_INDEX,false);

	osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT3_CONNECT_SUCCESS);
}


void port_enum_scan_handle(void)
{

	if(g_port.state != PORT_IDLE_OR_READY)
	{
		if(g_port.port_event & PORT0_EVENT_UNCONNECT && g_port.inhandle_port != PORT0_INDEX)				//TTPEC0
		{
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			usb_tc_set_state(&g_tc[PORT0_INDEX],TC_Disable,enter_state);
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
		}
		else if(g_port.port_event & PORT1_EVENT_UNCONNECT && g_port.inhandle_port != PORT1_INDEX) 			//TYPEC1
		{
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			usb_tc_set_state(&g_tc[PORT1_INDEX],TC_Disable,enter_state);
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
		}
		return;
	}

	if(g_port.port_event & PORT0_EVENT_UNCONNECT)				//TTPEC0
	{
		g_port.inhandle_port = 0;
		g_port.port_event &= ~PORT0_EVENT_UNCONNECT;
		port_manager_set_state(PORT_INHANDLING);
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_CLOSED);
	}
	else if(g_port.port_event & PORT1_EVENT_UNCONNECT) 			//TYPEC1
	{
		g_port.inhandle_port = 1;
		g_port.port_event &= ~PORT1_EVENT_UNCONNECT;
		port_manager_set_state(PORT_INHANDLING);
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_CLOSED);
	}
	else if(g_port.port_event & PORT2_EVENT_UNCONNECT) 			//TYPEC1
	{
		g_port.inhandle_port = 2;
		port_manager_set_state(PORT_INHANDLING);
		g_port.port_event &= ~PORT2_EVENT_UNCONNECT;
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT2_CONNECT_CLOSED);
	}
	else if(g_port.port_event & PORT3_EVENT_UNCONNECT) 			//TYPEC1
	{
		g_port.inhandle_port = 3;
		port_manager_set_state(PORT_INHANDLING);
		g_port.port_event &= ~PORT3_EVENT_UNCONNECT;
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT3_CONNECT_CLOSED);
	}


	else if(g_port.port_event & PORT0_EVENT_TRY_CONNECT)				//TTPEC0
	{
		g_port.port_event &= ~PORT0_EVENT_TRY_CONNECT;
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)
		{
			port_manager_set_state(PORT_INHANDLING);
			g_port.inhandle_port = 0;
			osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_START);
		}
	}
	else if(g_port.port_event & PORT1_EVENT_TRY_CONNECT) 			//TYPEC1
	{
		g_port.port_event &= ~PORT1_EVENT_TRY_CONNECT;
		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)
		{
			port_manager_set_state(PORT_INHANDLING);
			g_port.inhandle_port = 1;
			osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_START);
		}
	}
	else if(g_port.port_event & PORT2_EVENT_TRY_CONNECT)			//USB-A
	{
		g_port.port_event &= ~PORT2_EVENT_TRY_CONNECT;
		port_manager_set_state(PORT_INHANDLING);
		g_port.inhandle_port = 2;
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT2_CONNECT_START);
	}
	else if(g_port.port_event & PORT3_EVENT_TRY_CONNECT)  			//WPC
	{
		g_port.port_event &= ~PORT3_EVENT_TRY_CONNECT;
		g_port.inhandle_port = 3;
		port_manager_set_state(PORT_INHANDLING);
		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT3_CONNECT_START);
	}
	else if(g_port.port_event & PORT_EVENT_RESET_CHARGE)
	{
		g_port.port_event &= ~PORT_EVENT_RESET_CHARGE;
		g_port.inhandle_port = g_port.incharge_port;
		tcpm_disable_usba_detect();
		port_manager_set_state(PORT_INHANDLING);
		//if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE &&  g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE)
		{
			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
			{
				if(g_port.incharge_port == PORT0_INDEX)
				{
					osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
				}
				else
				{
					osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_SINK_SETVOLT);
				}
				//g_port.inhandle_port = g_port.incharge_port;
			}
		}
	}

}

void port_manager_event_handle(uint32_t event)
{
	switch (event)
	{
		case PORT_ENUM_EVT_PORT_SCAN:
			port_enum_scan_handle();
			break;
		case PORT_ENUM_EVT_PORT0_CONNECT_START:
			port_enum_port0_connect_start();
			break;
		case PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS:
			g_port.port_event &= ~PORT0_EVENT_UNCONNECT;
			port_enum_port0_connect_success();
			break;
		case PORT_ENUM_EVT_PORT0_CONNECT_CLOSED:
			osal_stop_timerEx(PORT_CONNECT_TIMER);
			port_enum_port0_connect_closed();
			buckboost_ops.typca_dischg_en(true);
			break;
		case PORT_ENUM_EVT_PORT0_SINK_SETVOLT:
			port_enum_port_snk_setvolt();
			break;
		case PORT_ENUM_EVT_PORT0_SINK_SETCHARGE:
			port_enum_port_snk_setcharge();
			break;
		case PORT_ENUM_EVT_PORT0_ENUM_DONE:
			port_enum_port_enum_done();
			break;

		case PORT_ENUM_EVT_PORT1_CONNECT_START:
			port_enum_port1_connect_start();
			break;
		case PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS:
			g_port.port_event &= ~PORT1_EVENT_UNCONNECT;
			port_enum_port1_connect_success();
			break;
		case PORT_ENUM_EVT_PORT1_CONNECT_CLOSED:
			osal_stop_timerEx(PORT_CONNECT_TIMER);
			port_enum_port1_connect_closed();
			buckboost_ops.typcb_dischg_en(true);
			break;
		case PORT_ENUM_EVT_PORT1_SINK_SETVOLT:
			port_enum_port_snk_setvolt();
			break;
		case PORT_ENUM_EVT_PORT1_SINK_SETCHARGE:
			port_enum_port_snk_setcharge();
			break;
		case PORT_ENUM_EVT_PORT1_ENUM_DONE:
			port_enum_port_enum_done();
			break;

		case PORT_ENUM_EVT_PORT2_CONNECT_START:
			port_enum_port2_connect_start();
			break;
		case PORT_ENUM_EVT_PORT2_CONNECT_SUCCESS:
			port_enum_port2_connect_success();
			break;
		case PORT_ENUM_EVT_PORT2_CONNECT_CLOSED:
			port_enum_port2_connect_closed();
			break;
		case PORT_ENUM_EVT_PORT2_ENUM_DONE:
			port_enum_port_enum_done();
			break;
		case PORT_ENUM_EVT_PORT3_CONNECT_START:
			port_enum_port3_connect_start();
			break;
		case PORT_ENUM_EVT_PORT3_CONNECT_SUCCESS:
			port_enum_port3_connect_success();
			break;
		case PORT_ENUM_EVT_PORT3_CONNECT_CLOSED:
			port_enum_port3_connect_closed();
			break;
		case PORT_ENUM_EVT_PORT3_ENUM_DONE:
			port_enum_port_enum_done();
			break;
		default:
			break;
	}
}





