#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "port_manager.h"
#include "pd.h"
#include "config.h"
#include "buckboost.h"
#include "pdlib.h"
#include "adp.h"
#include "_wpc.h"
#include "g_data.h"
#include "pid.h"
#include "usb_qc.h"
#include "tcpm.h"
#include "nu6801.h"
#include "ntc.h"
#include "pd_tc.h"
void port_manager_set_event(uint32_t event)
{
	g_port.port_event |= event;

	gd->idle_to_sleep_cnt = 0;

	printk("%s=0x%x!\n",__func__,event);
}
extern uint8_t charge_led_finish;
extern uint8_t charge_led_run;

void port_manager_set_state(enum port_state_e state)
{
	if (state == PORT_INHANDLING) printk("[ST:IH]\n");
	else if (g_port.state == PORT_INHANDLING) printk("[ST:RDY]\n");
	g_port.state = state;
}

void port_manager_task_init(void)
{
	osal_mem_clear(&g_port,sizeof(struct port_infos));
	osal_task_handler_reg(PORT_MANAGER_TASK, port_manager_event_handle);
	osal_start_timerEx(PORT_ENUM_TIMER, PORT_ENUM_PERIOD, PORT_ENUM_PERIOD, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT_SCAN);

	hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);

	//tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);

}

extern volatile uint32_t time_ticks;
void port_enum_port0_connect_closed(void)
{
	printk("%s!\n",__func__);
	gd->flag11 = 1;
	gd->timer_cnt = time_ticks;
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
				pdlib_delayms_restart_typec(PORT1_INDEX,200);
				pdlib_disable_usbpd();
				usb_dpdm_select(DPDM_PHY_OFF);
				g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			}
#if(CONFIG_USBA_SUPPORT == 1)
			if(g_port.port_state[PORT2_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT2_INDEX,false);
				usb_dpdm_select(DPDM_PHY_OFF);
				port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			}
#endif
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
			pdlib_disable_usbpd();
			//pdlib_restart_typec(PORT1_INDEX);
			pdlib_delayms_restart_typec(PORT1_INDEX,200);
			usb_dpdm_select(DPDM_PHY_OFF);
		}
#if(CONFIG_USBA_SUPPORT == 1)
		if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
			usb_dpdm_select(DPDM_PHY_OFF);
		}
#endif
		port_manager_set_state(PORT_IDLE_OR_READY);
		printk("[IH-]\n");
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE  && pdlib_get_tc_state(PORT0_INDEX)  == TC_Disable)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT0_INDEX);
		pdlib_delayms_restart_typec(PORT0_INDEX,200);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE  && pdlib_get_tc_state(PORT1_INDEX)  == TC_Disable)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT1_INDEX);
		pdlib_delayms_restart_typec(PORT1_INDEX,200);
	}
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}
#endif
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
			if(pdlib_is_connect())
			{
				if(pdlib_is_pps_sink())  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
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
			//pdlib_restart_typec(PORT1_INDEX);
			pdlib_delayms_restart_typec(PORT1_INDEX,200);

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
#if(CONFIG_TYPECA_SUPPORT == 1)
	lib_para.typec_a_support = 1;
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
	lib_para.typec_b_support = 1;
#endif
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
				//pdlib_restart_typec(PORT0_INDEX);
				pdlib_delayms_restart_typec(PORT0_INDEX,200);
				pdlib_disable_usbpd();
				usb_dpdm_select(DPDM_PHY_OFF);
				g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
			}
#if(CONFIG_USBA_SUPPORT == 1)
			if(g_port.port_state[PORT2_INDEX] != PORT_STATE_NONE )
			{
				hal_tcpc_set_gate_en(PORT2_INDEX,false);
				usb_dpdm_select(DPDM_PHY_OFF);
				port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
				g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			}
#endif
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
			//pdlib_restart_typec(PORT0_INDEX);
			pdlib_delayms_restart_typec(PORT0_INDEX,200);
			pdlib_disable_usbpd();
			usb_dpdm_select(DPDM_PHY_OFF);
		}
#if(CONFIG_USBA_SUPPORT == 1)
		if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
			usb_dpdm_select(DPDM_PHY_OFF);
		}
#endif
		port_manager_set_state(PORT_IDLE_OR_READY);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT0_INDEX);
		pdlib_delayms_restart_typec(PORT0_INDEX,200);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT1_INDEX);
		pdlib_delayms_restart_typec(PORT1_INDEX,200);
	}
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}
#endif
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
			if(pdlib_is_connect())
			{
				if(pdlib_is_pps_sink())  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
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
			//pdlib_restart_typec(PORT0_INDEX);
			pdlib_delayms_restart_typec(PORT0_INDEX,200);
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
#if(CONFIG_TYPECA_SUPPORT == 1)
	lib_para.typec_a_support = 1;
#endif
#if(CONFIG_TYPECB_SUPPORT == 1)
	lib_para.typec_b_support = 1;
#endif
}

void port_enum_port2_connect_closed(void)
{
	printk("%s!\n",__func__);
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	tcpm_disable_usba_detect();
	hal_tcpc_set_gate_en(g_port.inhandle_port,false);
	g_port.port_state[g_port.inhandle_port] = PORT_STATE_NONE;

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT0_INDEX);
		pdlib_delayms_restart_typec(PORT0_INDEX,200);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT1_INDEX);
		pdlib_delayms_restart_typec(PORT1_INDEX,200);
	}
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}
#endif
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
			if(pdlib_is_connect())
			{
				if( pdlib_is_pps_sink())  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if( pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2 ) tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
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
			//pdlib_restart_typec(PORT0_INDEX);
			pdlib_delayms_restart_typec(PORT0_INDEX,200);

		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			//pdlib_restart_typec(PORT1_INDEX);
			pdlib_delayms_restart_typec(PORT1_INDEX,200);
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
#if(CONFIG_TYPECA_SUPPORT == 1)
	lib_para.typec_a_support = 1;
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
	lib_para.typec_b_support = 1;
#endif
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

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT0_INDEX);
		pdlib_delayms_restart_typec(PORT0_INDEX,200);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT1_INDEX);
		pdlib_delayms_restart_typec(PORT1_INDEX,200);
	}

	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
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
			if(pdlib_is_connect())
			{
				if(pdlib_is_pps_sink())  tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
				if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2 )
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_ADP_FIX);
				else
					tcpm_update_wpc_work_mode(TCPM_WPC_WORK_FIX5V);
			}
			else
			{
				if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V)
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
			//pdlib_restart_typec(PORT0_INDEX);
			pdlib_delayms_restart_typec(PORT0_INDEX,200);
		}

		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)
		{
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			//pdlib_restart_typec(PORT1_INDEX);
			pdlib_delayms_restart_typec(PORT1_INDEX,200);
		}
#if(CONFIG_USBA_SUPPORT == 1)
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
		{
			g_port.port_state[PORT2_INDEX] = PORT_STATE_NONE;
			hal_tcpc_set_gate_en(PORT2_INDEX,false);
			port_manager_set_event(PORT2_EVENT_TRY_CONNECT);
		}
#endif
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
#if(CONFIG_TYPECA_SUPPORT == 1)
	lib_para.typec_a_support = 1;
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
	lib_para.typec_b_support = 1;
#endif
	port_manager_set_state(PORT_IDLE_OR_READY);
}

void port_enum_port_enum_done(void)
{
	printk("%s!\n",__func__);

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && !g_buckboost.set_typeca_gate_en)
	{
		//if(g_tcpc.tc_port_map != PORT0_INDEX || dpdm_map != PORT0_INDEX) tcpm_set_port_sdp(PORT0_INDEX);  // 500mA锟脚碉拷
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,6500);
		printk("mos0\n");
		hal_tcpc_set_gate_en(PORT0_INDEX,true);
		buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,g_buckboost.buckboost_out_current_actual);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && !g_buckboost.set_typeca_gate_en)
	{
		//if(g_tcpc.tc_port_map != PORT1_INDEX || dpdm_map != PORT1_INDEX) tcpm_set_port_sdp(PORT1_INDEX);  // 500mA锟脚碉拷
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		hal_tcpc_set_gate_en(PORT1_INDEX,true);
	}
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE && !g_buckboost.set_usb_a_gate_en)
	{
		//if(dpdm_map != PORT2_INDEX) tcpm_set_port_sdp(PORT2_INDEX);  // 500mA锟脚碉拷s
		//if(!(g_port.adpater_power < 7500 && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE))
		hal_tcpc_set_gate_en(PORT2_INDEX,true);
	}
#endif
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT0_INDEX);
		pdlib_delayms_restart_typec(PORT0_INDEX,200);
	}

	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷toogle
	{
		//pdlib_restart_typec(PORT1_INDEX);
		pdlib_delayms_restart_typec(PORT1_INDEX,200);
	}
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE)  // 锟斤拷锟铰匡拷锟斤拷A锟节硷拷锟�
	{
		osal_set_event(USB_TASK, TCPM_EVT_USBA_REDETECT);
	}
#endif
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
				if(pdlib_is_connect())
				{
					if(pdlib_is_pps_sink())
						tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
					else
					{
						if(pdlib_snk_get_work_pdo_index() >= PDO_INDEX_2 )
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

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE && g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE)
	{
		g_buckboost.buckboost_out_current = 3000;
		g_buckboost.buckboost_out_current_actual = 3000;
		buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,g_buckboost.buckboost_out_current);
	}
#if(CONFIG_TYPECA_SUPPORT == 1)
	lib_para.typec_a_support = 1;
#endif
#if(CONFIG_TYPECB_SUPPORT == 1)
	lib_para.typec_b_support = 1;
#endif

	port_manager_set_state(PORT_IDLE_OR_READY);
}

void port_enum_port_snk_setcharge(void)
{
	printk("%s vbus=%d!\n",__func__,g_buckboost.adc_vbus);

	gd->bat_dead_flag = 0;

	if(g_port.port_state[PORT3_INDEX] == PORT_STATE_SOURCE)// wireless present.
	{
		if(g_buckboost.adc_vbus<5500)// 5v
		{
			g_port.ibat_limit = (g_port.adpater_power > 8000)? (g_port.adpater_power - 8000)/5:500;
			g_port.ibus_limit = (g_port.adpater_power > 8000)? (g_port.adpater_power - 8000)/5:500;
		}
		else if (g_buckboost.adc_vbus<9500)// 9v
		{
			g_port.ibat_limit = (g_port.adpater_power > 11000)? (g_port.adpater_power - 11000)/9:500;
			g_port.ibus_limit = (g_port.adpater_power > 11000)? (g_port.adpater_power - 11000)/9:500;
		}
		else//12v, reserved for future 12 use.
		{
			g_port.ibat_limit = (g_port.adpater_power > 12000)? (g_port.adpater_power - 12000)/12:500;
			g_port.ibus_limit = (g_port.adpater_power > 12000)? (g_port.adpater_power - 12000)/12:500;
		}
		//g_port.ibat_limit = g_port.ibat_limit < 500 ? g_port.ibat_limit : 500;
		g_port.ibus_limit = g_port.ibus_limit* 95 / 100;
		g_port.ibat_limit = g_port.ibat_limit < 2000 ? g_port.ibat_limit : 2000;

		g_port.ibus_limit = g_port.ibus_limit < 2000 ? g_port.ibus_limit : 2000;
	}
	else
	{
		g_port.ibat_limit = g_port.ibat_limit;
		g_port.ibus_limit = g_port.ibus_limit;

		if(g_buckboost.adc_vbus < 5500)// 5v
		{
			g_port.ibus_limit = g_port.ibus_limit < 3000 ? g_port.ibus_limit : 3000;
		}
		else if(g_buckboost.adc_vbus < 9500)// 5v
		{
			g_port.ibus_limit = g_port.ibus_limit < 3000 ? g_port.ibus_limit : 3000;
		}
		else if(g_buckboost.adc_vbus < 12500)
		{
			g_port.ibus_limit = g_port.ibus_limit < 2500 ? g_port.ibus_limit : 2500;
		}
		else if(g_buckboost.adc_vbus < 15500)
		{
			g_port.ibus_limit = g_port.ibus_limit < 2000 ? g_port.ibus_limit : 2000;
		}
		else
		{
			g_port.ibus_limit = g_port.ibus_limit < 1500 ? g_port.ibus_limit : 1500;
		}
	}

	if(typec_ntc_ot_flag)
	{
		g_port.ibat_limit = g_port.ibat_limit<(20000*1000/g_buckboost.adc_vbat)?g_port.ibat_limit:20000*1000/g_buckboost.adc_vbat;
		g_port.ibus_limit = g_port.ibus_limit<(20000*1000/g_buckboost.adc_vbus)?g_port.ibus_limit:20000*1000/g_buckboost.adc_vbus;
	}
	if(bat_ntc_ut_flag)
	{
		g_port.ibat_limit = g_port.ibat_limit<(7000*1000/g_buckboost.adc_vbat)?g_port.ibat_limit:7000*1000/g_buckboost.adc_vbat;
		g_port.ibus_limit = g_port.ibus_limit<(7000*1000/g_buckboost.adc_vbus)?g_port.ibus_limit:7000*1000/g_buckboost.adc_vbus;
	}
	if (gd->bat_ntc_dischg_reduce_flag)
	{
		g_port.ibat_limit = g_port.ibat_limit<(20000*1000/g_buckboost.adc_vbat)?g_port.ibat_limit:20000*1000/g_buckboost.adc_vbat;
		g_port.ibus_limit = g_port.ibus_limit<(20000*1000/g_buckboost.adc_vbus)?g_port.ibus_limit:20000*1000/g_buckboost.adc_vbus;
	}

	if(pdlib_get_deadbat()) g_port.ibus_limit =  g_port.ibus_limit < 500 ? g_port.ibus_limit : 500;
	printk("charg set %d %d", g_port.ibus_limit,g_port.ibat_limit);

	//if(g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE) hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
	hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
	printk("chager mode=%d vbus=%d ovp=%d\n", g_buckboost.woke_mode, g_buckboost.adc_vbus, g_buckboost.ovp_value);
	g_port.ibus_limit = g_port.ibus_limit * 95 / 100;

	buckboost_set_charge_current(g_port.ibat_limit,g_port.ibus_limit);

	if(g_port.inhandle_port == PORT0_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 100, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_ENUM_DONE);
	else if(g_port.inhandle_port == PORT1_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 100, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);
	gd->bat_ov_forbid_flag = 0;  // DEBUG: force clear OV forbid for testing
	printk("PROT ntc_stop=%d bat_ntc_ot=%d tc_ntc_lock=%d deadbat=%d soc=%d ov_forbid=%d\n", ntc_stop_chrg_flag, bat_charge_ntc_ot_flag, gd->typec_charge_ntc_lock, pdlib_get_deadbat(), gd->real_soc_show, gd->bat_ov_forbid_flag);
	if(ntc_stop_chrg_flag||bat_charge_ntc_ot_flag||gd->typec_charge_ntc_lock) {
		printk("\r\n [CHRG_BLOCK] ntc_stop=%d bat_ot=%d tc_lock=%d", ntc_stop_chrg_flag, bat_charge_ntc_ot_flag, gd->typec_charge_ntc_lock);
		buckboost_ops.set_work_mode(0x00);
	}

	if(pdlib_is_pps_sink())
		buckboost_ops.set_ovp(20000);
	else
		buckboost_ops.set_ovp(g_port.snk_set_volt);

	printk("[%d]Power=%dmW I[bat]=%dmA I[bus]=%dmA V[bat] = %d  V[set] = %d !\n",g_port.inhandle_port,g_port.adpater_power,g_port.ibat_limit,
			g_port.ibus_limit,g_buckboost.adc_vbat,g_port.snk_set_volt);

}

void port_enum_port_snk_setvolt(void)
{

	uint32_t source_pdo;
	//hal_tcpc_set_gate_en(g_port.incharge_port,false);
	printk("[%d %d]%s!\n",g_port.inhandle_port,g_port.incharge_port,__func__);

	hal_tcpc_set_gate_en(g_port.incharge_port,true);

	g_port.ibus_limit = 3000;
	g_port.ibat_limit = 3000;
	buckboost_ops.set_ovp(20000);
	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE
			&& g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE && (!pdlib_get_deadbat()))
	{

		if(pdlib_is_connect())
		{
			{
				for(uint8_t i = 0; i< pdlib_snk_get_pdo_amount(); i++)
				{

					source_pdo = pdlib_snk_get_pdo_by_index(pdlib_snk_get_pdo_amount() - i);

					if(pdo_type(source_pdo) == PDO_TYPE_FIXED)
					{
						if(gd->sigle_clicked)
						{
							if(pdo_fixed_voltage(source_pdo) <= VOLTAGE_5V)
							{
								pdlib_snk_requsrt_voltage(pdlib_snk_get_pdo_amount() - i,pdo_fixed_voltage(source_pdo),pdo_max_current(source_pdo));
								g_port.snk_set_volt = pdo_fixed_voltage(source_pdo);
								g_port.ibus_limit = pdo_max_current(source_pdo);
								g_port.adpater_power =  (uint32_t)g_port.ibus_limit * pdo_fixed_voltage(source_pdo) / 1000;
								break;
							}
							//gd->sigle_clicked = 0;

						}
						else
						{
							if(pdo_fixed_voltage(source_pdo) <= VOLTAGE_20V)
							{
								pdlib_snk_requsrt_voltage(pdlib_snk_get_pdo_amount() - i,pdo_fixed_voltage(source_pdo),pdo_max_current(source_pdo));
								g_port.snk_set_volt = pdo_fixed_voltage(source_pdo);
								g_port.ibus_limit = pdo_max_current(source_pdo);
								if(g_port.snk_set_volt >=18000) g_port.ibus_limit = g_port.ibus_limit >1500? 1500:g_port.ibus_limit;
								else if(g_port.snk_set_volt >=14000) g_port.ibus_limit = g_port.ibus_limit >2000? 2000:g_port.ibus_limit;
								g_port.adpater_power = (uint32_t)g_port.ibus_limit * pdo_fixed_voltage(source_pdo) / 1000;
								break;
							}

						}
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
				g_port.ibus_limit = 1500;  // 12V 2.5A
				g_port.snk_set_volt = VOLTAGE_12V;
				g_port.adpater_power =  (uint32_t)2500 * VOLTAGE_12V / 1000;
			}
			else
			{
				qc2_set_volt(VOLTAGE_9V);
				g_port.snk_set_volt = VOLTAGE_9V;
				g_port.ibus_limit = 2000;  // 9V 3A
				g_port.adpater_power =  (uint32_t)3000 * VOLTAGE_9V / 1000;
			}
			g_port.ibat_limit = 5000;
		}
		else
		{
//			if(bc12_type >= BC1P2_HVDCP)
//			{
//				g_port.adpater_power =  (uint32_t)3000 * VOLTAGE_5V / 1000;
//				g_port.ibus_limit = 3000;
//				g_port.ibat_limit = 5000;
//			}
//			else if(bc12_type > BC1P2_CDP)
//			{
//
//				if(bc12_type == BC1P2_DCP)
//				{
//					g_port.ibus_limit = 1500;
//					g_port.ibat_limit = 5000;
//					g_port.adpater_power =  (uint32_t)1500 * VOLTAGE_5V / 1000;
//				}
//				else if(bc12_type == BC1P2_APPLE)
//				{
//					g_port.ibus_limit = 1950;
//					g_port.ibat_limit = 5000;
//					g_port.adpater_power =  (uint32_t)1950 * VOLTAGE_5V / 1000;
//				}
//			}
//			else
//				g_port.adpater_power =  (uint32_t)500 * VOLTAGE_5V / 1000;
			g_port.ibus_limit = 3000;
			g_port.ibat_limit = 5000;
			g_port.adpater_power =  (uint32_t)500 * VOLTAGE_5V / 1000;
			g_port.snk_set_volt = VOLTAGE_5V;
		}
	}
	else
	{
		if(pdlib_is_connect())
		{
			source_pdo = pdlib_snk_get_pdo_by_index(PDO_INDEX_1);
			g_port.adpater_power =  pdo_max_current(source_pdo) * VOLTAGE_5V / 1000;
			pdlib_snk_requsrt_voltage(PDO_INDEX_1,VOLTAGE_5V,pdo_max_current(source_pdo));
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

	g_port.prot_ibus = g_port.ibus_limit;

	if(g_port.inhandle_port == PORT0_INDEX)
		osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETCHARGE);
	else
		osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_SINK_SETCHARGE);

	printk("sdp_type = %d\n",bc12_type);
	//printk("I[bat]=%dmA I[bus]=%dmA!\n",g_port.ibat_limit,g_port.ibus_limit);
}

void port_enum_port0_connect_success(void)
{
	printk("%s! woke=%d tc=%d comm=%d\n",__func__, g_buckboost.woke_mode, pdlib_get_tc_state(PORT0_INDEX), gd->usb_comm_activated);
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) return;
#endif
	gd->typec_scp = 0;
	gd->vbus_ovp = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(pdlib_get_tc_state(PORT0_INDEX) == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_SHUTDOWM_MODE);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);

			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!pdlib_is_connect() && !(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V))
			{
				usb_dpdm_select(PORT0_INDEX);
				pdlib_set_pd_port(PORT0_INDEX);
				hal_tcpc_set_roles(PORT0_INDEX,TYPEC_SINK,TYPEC_DEVICE);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				pdlib_set_pd_event(PORT0_INDEX,USB_PD_EVT_SNK_ATTACHED);
				g_port.incharge_port = PORT0_INDEX;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_ENUM_DONE);
		}
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE || g_buckboost.woke_mode == BUCKBOOST_SHUTDOWM_MODE)
	{
		if(pdlib_get_tc_state(PORT0_INDEX) == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_SHUTDOWM_MODE);
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!pdlib_is_connect() && !(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V))
			{
				usb_dpdm_select(PORT0_INDEX);
				pdlib_set_pd_port(PORT0_INDEX);
				hal_tcpc_set_roles(PORT0_INDEX,TYPEC_SINK,TYPEC_DEVICE);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				pdlib_set_pd_event(PORT0_INDEX,USB_PD_EVT_SNK_ATTACHED);
				//hal_tcpc_set_gate_en(PORT0_INDEX,true);
				g_port.incharge_port = PORT0_INDEX;
			}
			if (charge_led_finish == 0)
			{
				charge_led_run = 1;
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 2000, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_SINK_SETVOLT);
		}
		else  //TC_SRC_Attached
		{
			if(g_buckboost.woke_mode == BUCKBOOST_SHUTDOWM_MODE)
			{
				hal_tcpc_pd_set_bus_iv(PORT0_INDEX, g_buckboost.buckboost_out_voltage, 3500, 0, 0);
				hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
			}
			buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,6500);
			printk("mos-1\n");
			hal_tcpc_set_gate_en(PORT0_INDEX,true);
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT2_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
			{
				usb_dpdm_select(PORT0_INDEX);
				pdlib_set_pd_port(PORT0_INDEX);
				hal_tcpc_set_roles(PORT0_INDEX,TYPEC_SOURCE,TYPEC_HOST);
				pdlib_set_pd_event(PORT0_INDEX,USB_PD_EVT_SRC_ATTACHED);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 200, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_ENUM_DONE);
			buckboost_ops.set_out(g_buckboost.buckboost_out_voltage,3500);
		}
	}


	if(pdlib_get_tc_state(PORT0_INDEX) == TC_SNK_Attached) g_port.port_state[PORT0_INDEX] = PORT_STATE_SINK;
	if(pdlib_get_tc_state(PORT0_INDEX) == TC_SRC_Attached) g_port.port_state[PORT0_INDEX] = PORT_STATE_SOURCE;
}

void port_enum_port1_connect_success(void)
{
	printk("%s!\n",__func__);
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) return;
#endif

	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(pdlib_get_tc_state(PORT1_INDEX) == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_SHUTDOWM_MODE);
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!pdlib_is_connect() && !(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V))
			{
				usb_dpdm_select(PORT1_INDEX);
				pdlib_set_pd_port(PORT1_INDEX);
				hal_tcpc_set_roles(PORT1_INDEX,TYPEC_SINK,TYPEC_DEVICE);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				pdlib_set_pd_event(PORT1_INDEX,USB_PD_EVT_SNK_ATTACHED);
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
		if(pdlib_get_tc_state(PORT1_INDEX) == TC_SNK_Attached)
		{
			hal_tcpc_set_source_mode(BUCKBOOST_SHUTDOWM_MODE);
			hal_tcpc_set_gate_en(PORT0_INDEX,false);

			if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE || g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE)
				g_port.snk_5v_only = 1;
			else
				g_port.snk_5v_only = 0;

			if(!pdlib_is_connect() && !(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V))
			{
				usb_dpdm_select(PORT1_INDEX);
				pdlib_set_pd_port(PORT1_INDEX);
				hal_tcpc_set_roles(PORT1_INDEX,TYPEC_SINK,TYPEC_DEVICE);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SNK_ATTACHED);
				pdlib_set_pd_event(PORT1_INDEX,USB_PD_EVT_SNK_ATTACHED);
				//hal_tcpc_set_gate_en(PORT1_INDEX,true);
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
				pdlib_set_pd_port(PORT1_INDEX);
				hal_tcpc_set_roles(PORT1_INDEX,TYPEC_SOURCE,TYPEC_HOST);
				pdlib_set_pd_event(PORT1_INDEX,USB_PD_EVT_SRC_ATTACHED);
				osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
			}
			osal_start_timerEx(PORT_CONNECT_TIMER, 200, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT1_ENUM_DONE);
		}
	}

	if(pdlib_get_tc_state(PORT1_INDEX) == TC_SNK_Attached) g_port.port_state[PORT1_INDEX] = PORT_STATE_SINK;
	if(pdlib_get_tc_state(PORT1_INDEX) == TC_SRC_Attached) g_port.port_state[PORT1_INDEX] = PORT_STATE_SOURCE;
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
	gd->ntc_led_off = 0;
	g_port.snk_set_volt = VOLTAGE_5V;
	tcpm_stop_wpc(WPC_DELAY);
	tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
	tcpm_disable_usba_detect();
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  pdlib_disable_typec(PORT1_INDEX);
	lib_para.typec_b_support = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT1_INDEX,false);
#if(CONFIG_USBA_SUPPORT == 1)
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
#endif
		source_pdo = pdlib_snk_get_pdo_by_index(PDO_INDEX_1);
		if(pdlib_is_connect()) pdlib_snk_requsrt_voltage(PDO_INDEX_1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		//
		hal_tcpc_set_snk_charge_current(CHG_IBAT_MIN,CHG_IBUS_MIN);    //锟斤拷锟矫筹拷锟斤拷锟斤拷锟斤拷锟斤拷小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没锟斤拷snk
	{
		pdlib_disable_usbpd();
		usb_dpdm_select(DPDM_PHY_OFF);
		hal_tcpc_set_gate_en(PORT1_INDEX,false);
	#if(CONFIG_USBA_SUPPORT == 1)
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
	#endif
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
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  pdlib_disable_typec(PORT0_INDEX);
	lib_para.typec_a_support = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT0_INDEX,false);
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		source_pdo = pdlib_snk_get_pdo_by_index(PDO_INDEX_1);
		if(pdlib_is_connect()) pdlib_snk_requsrt_voltage(PDO_INDEX_1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		//hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
		hal_tcpc_set_snk_charge_current(CHG_IBAT_MIN,CHG_IBUS_MIN);    //锟斤拷锟矫筹拷锟斤拷锟斤拷锟斤拷锟斤拷小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没锟斤拷snk
	{
		pdlib_disable_usbpd();
		usb_dpdm_select(DPDM_PHY_OFF);
		hal_tcpc_set_gate_en(PORT0_INDEX,false);
	#if(CONFIG_USBA_SUPPORT == 1)
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
	#endif
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
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  pdlib_disable_typec(PORT0_INDEX);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  pdlib_disable_typec(PORT1_INDEX);
	lib_para.typec_a_support = 0;
	lib_para.typec_b_support = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_tcpc_set_gate_en(PORT2_INDEX,false);
		source_pdo = pdlib_snk_get_pdo_by_index(PDO_INDEX_1);
		if(pdlib_is_connect()) pdlib_snk_requsrt_voltage(PDO_INDEX_1,VOLTAGE_5V,pdo_max_current(source_pdo));
		if(bc12_type == BC1P2_QC9V || bc12_type == BC1P2_QC12V) qc2_set_volt(VOLTAGE_5V);
		hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,100);
		hal_tcpc_set_snk_charge_current(CHG_IBAT_MIN,CHG_IBUS_MIN);    //锟斤拷锟矫筹拷锟斤拷锟斤拷锟斤拷锟斤拷小
	}
	else if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)  //没锟斤拷snk
	{
		pdlib_disable_usbpd();
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
	gd->touch_to_weakup = 0;
	tcpm_disable_usba_detect();
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE)  {pdlib_disable_typec(PORT0_INDEX);lib_para.typec_a_support = 0;}
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_NONE)  {pdlib_disable_typec(PORT1_INDEX);lib_para.typec_b_support = 0;}


	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_tcpc_set_snk_charge_current(CHG_IBAT_MIN,CHG_IBUS_MIN);    //锟斤拷锟矫筹拷锟斤拷锟斤拷锟斤拷锟斤拷小
	}
	else
	{
		pdlib_disable_usbpd();
		usb_dpdm_select(DPDM_PHY_OFF);
	}

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,2000,0,0);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT1_INDEX,5000,2000,0,0);
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE) hal_tcpc_pd_set_bus_iv(PORT0_INDEX,5000,3500,0,0);
#endif

	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT0_INDEX,false);
	if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT1_INDEX,false);
#if(CONFIG_USBA_SUPPORT == 1)
	if(g_port.port_state[PORT2_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_gate_en(PORT2_INDEX,false);
#endif
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE) hal_tcpc_set_cc(0,TYPEC_CC_OPEN);
	
	osal_start_timerEx(PORT_CONNECT_TIMER, 500, 0, PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT3_CONNECT_SUCCESS);
}


void port_enum_scan_handle(void)
{
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) return;
#endif
	{
		static uint8_t last_tc = 0xFF, last_st = 0xFF;
		uint8_t tc = pdlib_get_tc_state(PORT0_INDEX);
		uint8_t st = g_port.port_state[PORT0_INDEX];
		if (tc != last_tc || st != last_st) {
			printk("[P0] tc=%d->%d st=%d->%d\n", last_tc, tc, last_st, st);
			last_tc = tc; last_st = st;
		}
	}
	static uint16_t inhandling_stuck_cnt = 0;
	if(gd->flag11&&(time_ticks - gd->timer_cnt>=3000))
	{
		gd->flag11 = 0;
		if(g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)gd->sigle_clicked = 0;
	}
	if(gd->recharge_flag)
	{
		g_port.port_state[PORT0_INDEX] = PORT_STATE_SOURCE;
	}
	if(g_port.state != PORT_IDLE_OR_READY)
	{
		if(++inhandling_stuck_cnt > 5000)  /* 5s watchdog: force recovery from stuck INHANDLING */
		{
			printk("[PM] INHANDLING stuck >5s, force IDLE ih=%d\n", g_port.inhandle_port);
			port_manager_set_state(PORT_IDLE_OR_READY);
			inhandling_stuck_cnt = 0;
		}
		if(g_port.port_event & PORT0_EVENT_UNCONNECT && g_port.inhandle_port != PORT0_INDEX)				//TTPEC0
		{
			g_port.port_event &= ~PORT0_EVENT_UNCONNECT;
			hal_tcpc_set_gate_en(PORT0_INDEX,false);
			pdlib_disable_typec(PORT0_INDEX);
			g_port.port_state[PORT0_INDEX] = PORT_STATE_NONE;
		}
		else if(g_port.port_event & PORT1_EVENT_UNCONNECT && g_port.inhandle_port != PORT1_INDEX) 			//TYPEC1
		{
			g_port.port_event &= ~PORT1_EVENT_UNCONNECT;
			hal_tcpc_set_gate_en(PORT1_INDEX,false);
			pdlib_disable_typec(PORT1_INDEX);
			g_port.port_state[PORT1_INDEX] = PORT_STATE_NONE;
		}
		return;
	}

	inhandling_stuck_cnt = 0;

	if(g_port.port_event & PORT0_EVENT_UNCONNECT)				//TTPEC0
	{
		g_port.inhandle_port = 0;
		g_port.port_event &= ~PORT0_EVENT_UNCONNECT;
		port_manager_set_state(PORT_INHANDLING);
		printk("[IH+]\n");
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
		if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE&&!gd->recharge_flag)
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
			buckboost_ops.typcb_dischg_en(true);
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
			buckboost_ops.typca_dischg_en(true);
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





