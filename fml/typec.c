#include "tcpm.h"
#include "printk.h"
#include "regdef.h"
#include "tcpc.h"
#include "typec.h"
#include "pd.h"
#include "osal.h"
#include "usbpd_config.h"
#include "_wpc.h"
#include "port_manager.h"

struct tc_s g_tc[TYPEC_PORT_MAX_N] = {};

void usb_tc_init(void)
{
	osal_mem_clear(&g_tc,sizeof(struct tc_s));
	hal_tcpc_init();
	g_tc[TYPEC_PORT_A].tc_index = TYPEC_PORT_A;
	g_tc[TYPEC_PORT_A].is_in_prswap = 0;

	g_tc[TYPEC_PORT_B].tc_index = TYPEC_PORT_B;
	g_tc[TYPEC_PORT_B].is_in_prswap = 0;

	g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();

	printk("typec vbat =%d\n",g_buckboost.adc_vbat);

	if(g_buckboost.adc_vbat < 6000)
	{
		g_tc[TYPEC_PORT_A].is_deadbattery = 1;
		g_tc[TYPEC_PORT_B].is_deadbattery = 1;
	}

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_tc_set_state(&g_tc[TYPEC_PORT_A],TC_SRC_Unattached,enter_state);
	usb_tc_set_state(&g_tc[TYPEC_PORT_B],TC_SRC_Unattached,enter_state);
#else
	usb_tc_set_state(&g_tc[TYPEC_PORT_A],TC_SNK_Unattached,enter_state);
	usb_tc_set_state(&g_tc[TYPEC_PORT_B],TC_SNK_Unattached,enter_state);
#endif



}

bool tc_snk_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
    return (cc1 != TYPEC_CC_OPEN || cc2 != TYPEC_CC_OPEN);
}

bool tc_snk_is_disconnected(struct tc_s * tc)
{
    return (tc->polarity==TYPEC_POLARITY_CC1 && tc->cc1 == TYPEC_CC_OPEN) || (tc->polarity==TYPEC_POLARITY_CC2 && tc->cc2==TYPEC_CC_OPEN);
}

bool tc_src_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2)   //one cc is rd
{
    return ((cc1==TYPEC_CC_RD && cc2 !=TYPEC_CC_RD) || (cc2==TYPEC_CC_RD && cc1 !=TYPEC_CC_RD));
}

bool tc_src_is_disconnected(struct tc_s * tc)
{
    return (tc->polarity==TYPEC_POLARITY_CC1 && tc->cc1!=TYPEC_CC_RD) || (tc->polarity==TYPEC_POLARITY_CC2 && tc->cc2!=TYPEC_CC_RD);
}

bool tc_debug_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
    return ((cc1== TYPEC_CC_RD && cc2 == TYPEC_CC_RD));
}

bool tc_debug_is_disconnected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
	return ((cc1!=TYPEC_CC_RD || cc2 != TYPEC_CC_RD));
}

void usb_tc_set_state(struct tc_s * tc,enum usb_tc_state_e tc_state,enum usb_tc_substate_e tc_substate)
{
	tc->usb_tc_state = tc_state;
	tc->usb_tc_substate = tc_substate;

	usbpd_printk("tc[%d]_state = %d,%d\n",tc->tc_index, tc_state,tc_substate);
}

static void TC_Disable_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_OPEN);
	usb_tc_set_state(tc,TC_Disable,exit_state);
}
static void TC_Disable_Exit(struct tc_s * tc)
{

}

#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)

static void TC_SNK_Unattached_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);

#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,9000,3000,0,0);
	hal_tcpc_set_gate_en(tc->tc_index,false);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
    tc->tc_timer_cnt = 0;
    tc->try_snk_cnt = 0;
    usb_tc_set_state(tc,TC_SNK_Unattached,exit_state);
}
static void TC_SNK_Unattached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//usbpd_printk("cc1 = %d cc2= %d\n", cc1,cc2);
    if (tc_snk_is_connected(cc1,cc2))
    {
        if (cc1 != TYPEC_CC_OPEN)
        	tc->polarity = TYPEC_POLARITY_CC1;
        else
        	tc->polarity = TYPEC_POLARITY_CC2;
        usb_tc_set_state(tc,TC_SNK_AttachWait,enter_state);
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_TRY_CONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_TRY_CONNECT);
    }
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
    else
    {
    	tc->tc_timer_cnt++;
    	if(tc->tc_timer_cnt >= 40)
    	{
    		tc->tc_timer_cnt = 0;
    		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
    	}
    }
#endif
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_SNK_AttachWait_Entry(struct tc_s * tc)
{
	tc->tc_timer_cnt = 0;
	usb_tc_set_state(tc,TC_SNK_AttachWait,exit_state);
	hal_tcpc_port_dummyload_en(tc->tc_index,true);
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
}

static void TC_SNK_AttachWait_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_SNK_AttachWait,enter_state);
    }
    else if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE && (tc_snk_is_disconnected(tc)))
    {
    	usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
    }
    else if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
    {
    	hal_tcpc_port_dummyload_en(tc->tc_index,false);
        if(hal_tcpc_vbus_is_present(tc->tc_index))
        {
			#if(CONFIG_TC_TRY_SOURCE_SUPPORT_EN)
				if(tc->try_src_cnt >= 5 || tc->is_deadbattery)
					usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
				else
				{
					tc->try_src_cnt++;
					usb_tc_set_state(tc,TC_Try_SRC,enter_state);
				}
			#else
				usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
			#endif
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_SNK_Attached_Entry(struct tc_s * tc)
{


    tc->tc_timer_cnt = 0;
#ifndef MULTI_PORT_ALT_MODE
    hal_tcpc_set_gate_en(tc->tc_index,true);
#endif
	hal_tcpc_set_polarity(tc->tc_index,tc->polarity);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SINK,TYPEC_DEVICE);
    usb_tc_set_state(tc,TC_SNK_Attached,exit_state);

    if(tc->tc_index == PORT0_INDEX)
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS);
    else
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS);
}


static void TC_SNK_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	if(tc_snk_is_disconnected(tc)) //CC¶Ï¿ª
	{
		tc->tc_timer_cnt++;
		if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
		#ifndef MULTI_PORT_ALT_MODE
			hal_tcpc_set_gate_en(tc->tc_index,false);
		#endif
			usb_pd_set_event(tc->tc_index,USB_PD_EVT_SNK_UNATTACH);
			usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_SNK_UNATTCHED);
            if(tc->tc_index == 0)
            	port_manager_set_event(PORT0_EVENT_UNCONNECT);
            else
            	port_manager_set_event(PORT1_EVENT_UNCONNECT);
		}
	}
	else
	{
		tc->tc_timer_cnt = 0;
	}
}

#endif


#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
static void TC_SRC_Unattached_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index, TYPEC_CC_RP_3_0);

    hal_tcpc_set_vconn(tc->tc_index,false);
	hal_tcpc_set_pd_rx(tc->tc_index,EN_SOP | EN_HARD_RESET | EN_SOP1 ,false);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SOURCE,TYPEC_HOST);
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_gate_en(tc->tc_index,false);
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,9000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
    tc->tc_timer_cnt = 0;
    tc->try_snk_cnt = 0;
    tc->try_src_cnt = 0;
    usb_tc_set_state(tc,TC_SRC_Unattached,exit_state);
}
static void TC_SRC_Unattached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    if(tc_src_is_connected(cc1,cc2))
    {
        if (cc1 == TYPEC_CC_RD)
            tc->polarity = TYPEC_POLARITY_CC1;
        else
        	tc->polarity = TYPEC_POLARITY_CC2;
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_TRY_CONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_TRY_CONNECT);
        usb_tc_set_state(tc,TC_SRC_AttachWait,enter_state);
    }
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
    else
    {
    	tc->tc_timer_cnt++;
    	if(tc->tc_timer_cnt >= 40)
    	{
    		tc->tc_timer_cnt = 0;
    		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
    	}
    }
#endif
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}
static void TC_SRC_AttachWait_Entry(struct tc_s * tc)
{
	tc->tc_timer_cnt = 0;
	hal_tcpc_port_dummyload_en(tc->tc_index,true);
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
	usb_tc_set_state(tc,TC_SRC_AttachWait,exit_state);
}
static void TC_SRC_AttachWait_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_SRC_AttachWait,enter_state);
    }
    else if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE && (tc_src_is_disconnected(tc)))
    {
    	usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
    }
    else if(tc->tc_timer_cnt > TC_T_CC_DEBOUNCE)
    {
    	hal_tcpc_port_dummyload_en(tc->tc_index,false);
        if(tc_debug_is_connected(cc1,cc2))
        {
        	usb_tc_set_state(tc,TC_DEBUG_Attached,enter_state);
        }
        else if(tc_src_is_connected(cc1,cc2))
        {
			#if(CONFIG_TC_TRY_SINK_SUPPORT_EN)
				if(tc->try_snk_cnt >= 5)
					usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
				else
					usb_tc_set_state(tc,TC_Try_SNK,enter_state);
			#else
				if(hal_tcpc_vbus_is_vsafe5v()) usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
			#endif
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}
static void TC_SRC_Attached_Entry(struct tc_s * tc)
{
	tc->tc_timer_cnt = 0;
	hal_tcpc_set_polarity(tc->tc_index,tc->polarity);
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_gate_en(tc->tc_index,true);
	osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_ATTACHED);
#endif
    usb_tc_set_state(tc,TC_SRC_Attached,exit_state);

    if(tc->tc_index == PORT0_INDEX)
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS);
    else
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS);
}
static void TC_SRC_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index, &cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	if(tc_src_is_disconnected(tc)) //CC¶Ï¿ª
	{
		tc->tc_timer_cnt++;
		if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
			usb_pd_set_event(tc->tc_index,USB_PD_EVT_SRC_UNATTACH);
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
			usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
            if(tc->tc_index == 0)
            	port_manager_set_event(PORT0_EVENT_UNCONNECT);
            else
            	port_manager_set_event(PORT1_EVENT_UNCONNECT);
		}
	}
	else
	{
		tc->tc_timer_cnt = 0;
	}
}


static void TC_DEBUG_Attached_Entry(struct tc_s * tc)
{
	usb_tc_set_state(tc,TC_SRC_Attached,exit_state);
}
static void TC_DEBUG_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	if(tc_debug_is_disconnected(cc1,cc2)) //CC¶Ï¿ª
	{
		tc->tc_timer_cnt++;
		if(tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
			usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
		}
	}
	else
	{
		tc->tc_timer_cnt = 0;
	}
}

#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
static void TC_DRP_TOGGLE_Entry(struct tc_s * tc)
{
#ifndef MULTI_PORT_ALT_MODE
    hal_tcpc_set_gate_en(tc->tc_index,false);
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,9000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
    hal_tcpc_set_vconn(tc->tc_index,false);
	hal_tcpc_set_pd_rx(tc->tc_index,EN_SOP | EN_HARD_RESET | EN_SOP1 ,false);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SINK,TYPEC_DEVICE);
    hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_TOGGLE);
    usb_tc_set_state(tc,TC_DRP_TOGGLE,exit_state);

    if(g_port.inhandle_port == tc->tc_index && g_port.state == PORT_INHANDLING)
    {
    	if(tc->tc_index == 0)
    		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_CLOSED);
    	else
    		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_CLOSED);
    }
}

static void TC_DRP_TOGGLE_Exit(struct tc_s * tc)
{
	if(hal_get_drp_toggle_result(tc->tc_index) == TYPEC_DRP_SNK_CONNECTED)
	{
		usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
	}
	else if(hal_get_drp_toggle_result(tc->tc_index) == TYPEC_DRP_SRC_CONNECTED)
	{
		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
	}
}

static void TC_Try_SNK_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index, TYPEC_CC_RD);
	tc->tc_timer_cnt = 0;
	usb_tc_set_state(tc,TC_Try_SNK,exit_state);
}
static void TC_Try_SNK_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_Try_SNK,enter_state);
    }
    else
    {
        if(tc_snk_is_connected(cc1,cc2) && tc->tc_timer_cnt > TC_T_TRY_CC_DEBOUNCE)
        {
        	usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
        }

        if(tc_snk_is_disconnected(tc) &&  tc->tc_timer_cnt > TC_T_DRP_TRYWAIT)
        {
        	usb_tc_set_state(tc,TC_TryWAIT_SRC,enter_state);
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_TryWAIT_SRC_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RP_3_0);
	tc->tc_timer_cnt = 0;
	usb_tc_set_state(tc,TC_TryWAIT_SRC,exit_state);
}
static void TC_TryWAIT_SRC_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_TryWAIT_SRC,enter_state);
    }
    else
    {
    	if(tc_src_is_disconnected(tc) && tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
    		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
		}
		else if(tc_src_is_connected(cc1,cc2) && tc->tc_timer_cnt > TC_T_CC_DEBOUNCE)
		{
			usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
		}
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_Try_SRC_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RP_3_0);
	tc->tc_timer_cnt = 0;
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
	usb_tc_set_state(tc,TC_Try_SRC,exit_state);
}


static void TC_Try_SRC_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_Try_SRC,enter_state);
    }
    else
    {
        if(tc_src_is_connected(cc1,cc2) && tc->tc_timer_cnt > TC_T_TRY_CC_DEBOUNCE)
        {

        	usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
        }

        if(tc_src_is_disconnected(tc) &&  tc->tc_timer_cnt > TC_T_DRP_TRY)
        {
        	usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_TryWAIT_SNK_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
	tc->tc_timer_cnt = 0;
	usb_tc_set_state(tc,TC_TryWAIT_SNK,exit_state);
#ifndef MULTI_PORT_ALT_MODE
	hal_tcpc_set_source_mode(BUCKBOOST_CHAGER_MODE);
	hal_tcpc_pd_set_bus_iv(tc->tc_index,5000,3000,0,0);
	wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
#endif
}
static void TC_TryWAIT_SNK_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt++;

    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
    }
    else
    {
    	if(tc_snk_is_disconnected(tc) && tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
    		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
		}
		else if(tc_snk_is_connected(cc1,cc2) && tc->tc_timer_cnt > TC_T_PD_DEBOUNCE)
		{
			if(hal_tcpc_vbus_is_present(tc->tc_index))
			{
				usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
			}
		}
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}
#endif

static void TC_ErrorRecovery_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_OPEN);
//    hal_tcpc_set_gate_en(tc->tc_index,false);
//    hal_tcpc_set_vconn(tc->tc_index,false);
    hal_tcpc_set_pd_rx(tc->tc_index,EN_SOP | EN_HARD_RESET | EN_SOP1 ,false);
    tc->try_snk_cnt = 0;
    tc->try_src_cnt = 0;
    hal_tcpc_set_roles(tc->tc_index,TYPEC_SINK,TYPEC_DEVICE);
	tc->tc_timer_cnt = 0;
	usb_tc_set_state(tc,TC_ErrorRecovery,exit_state);
}
static void TC_ErrorRecovery_Exit(struct tc_s * tc)
{
	tc->tc_timer_cnt++;
	if(tc->tc_timer_cnt > TC_T_ERROR_RECOVERY)
	{
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
	#else
		usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
	#endif
	}
}

const static struct usb_tc_state_task_t usb_tc_table[TC_STATE_MAX]  =
{
	{TC_Disable_Entry,TC_Disable_Exit},						//TC_Disable = 0,
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
	{TC_SNK_Unattached_Entry,TC_SNK_Unattached_Exit},		//TC_SNK_Unattached,
	{TC_SNK_AttachWait_Entry,TC_SNK_AttachWait_Exit},		//TC_SNK_AttachWait,
	{TC_SNK_Attached_Entry,TC_SNK_Attached_Exit},			//TC_SNK_Attached,
#endif

#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
	{TC_SRC_Unattached_Entry,TC_SRC_Unattached_Exit},		//TC_SRC_Unattached,
	{TC_SRC_AttachWait_Entry,TC_SRC_AttachWait_Exit},		//TC_SRC_AttachWait,
	{TC_SRC_Attached_Entry,TC_SRC_Attached_Exit},			//TC_SRC_Attached,
	{TC_DEBUG_Attached_Entry,TC_DEBUG_Attached_Exit}, 		//TC_DEBUG_Attached
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	{TC_DRP_TOGGLE_Entry,TC_DRP_TOGGLE_Exit}, 				//TC_DRP_TOGGLE,
	{TC_Try_SNK_Entry,TC_Try_SNK_Exit}, 					//TC_Try_SNK,
	{TC_TryWAIT_SRC_Entry,TC_TryWAIT_SRC_Exit}, 			//TC_TryWAIT_SRC,
	{TC_Try_SRC_Entry,TC_Try_SRC_Exit}, 					//TC_Try_SRC,
	{TC_TryWAIT_SNK_Entry,TC_TryWAIT_SNK_Exit}, 			//TC_TryWAIT_SNK,
#endif
	{TC_ErrorRecovery_Entry,TC_ErrorRecovery_Exit},		//TC_ErrorRecovery
															//TC_STATE_MAX,
};


void usb_tc_run(void)
{


#if(CONFIG_USBTC_PORT_SELECT & TC_PORT_CCA)
	if(g_tc[TYPEC_PORT_A].is_in_prswap) return;

	if(g_tc[TYPEC_PORT_A].usb_tc_substate == enter_state)
	{
		//if(usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].enter_cb != NULL)
			usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].enter_cb(&g_tc[TYPEC_PORT_A]);
	}
	else
	{
		//if(usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].exit_cb != NULL)
			usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].exit_cb(&g_tc[TYPEC_PORT_A]);
	}
#endif

#if(CONFIG_USBTC_PORT_SELECT & TC_PORT_CCB)
	if(g_tc[TYPEC_PORT_B].is_in_prswap) return;

	if(g_tc[TYPEC_PORT_B].usb_tc_substate == enter_state)
	{
		//if(usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].enter_cb != NULL)
			usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].enter_cb(&g_tc[TYPEC_PORT_B]);
	}
	else
	{
		//if(usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].exit_cb != NULL)
			usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].exit_cb(&g_tc[TYPEC_PORT_B]);
	}
#endif


}

