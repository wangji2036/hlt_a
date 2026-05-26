#include "tcpm.h"
#include "printk.h"
#include "regdef.h"
#include "tcpc.h"
#include "pd_tc.h"
#include "typec.h"
#include "pd.h"
#include "osal.h"
#include "usbpd_config.h"
#include "_wpc.h"
#include "g_data.h"
#include "port_manager.h"
#include "nu6801.h"
#include "usb_qc.h"

struct tc_s g_tc[TYPEC_PORT_MAX_N] = {};

extern volatile uint32_t tc_sys_ticks;

void usb_tc_init(void)
{
	osal_mem_clear(&g_tc,sizeof(struct tc_s));
	hal_tcpc_init();
	g_tc[TYPEC_PORT_A].tc_index = TYPEC_PORT_A;
	g_tc[TYPEC_PORT_A].is_in_prswap = 0;

	g_tc[TYPEC_PORT_B].tc_index = TYPEC_PORT_B;
	g_tc[TYPEC_PORT_B].is_in_prswap = 0;

	g_buckboost.adc_vbat = buckboost_ops.get_bat_voltage();

	lib_printk("typec vbat =%d\n",g_buckboost.adc_vbat);
	extern bool tc_power_on;
	if(g_buckboost.adc_vbat < dead_battery_voltage || tc_power_on)
	{
		g_tc[TYPEC_PORT_A].is_deadbattery = 1;
		g_tc[TYPEC_PORT_B].is_deadbattery = 1;
	}

	tc_power_on = false;

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

bool tc_acc_is_connected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
    return ((cc1== TYPEC_CC_RA && cc2 == TYPEC_CC_RA));
}

bool tc_acc_is_disconnected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
    return ((cc1!= TYPEC_CC_RA || cc2 != TYPEC_CC_RA));
}


bool tc_debug_is_disconnected(enum tc_cc_status cc1,enum tc_cc_status cc2)
{
	return ((cc1!=TYPEC_CC_RD || cc2 != TYPEC_CC_RD));
}

void usb_tc_set_state(struct tc_s * tc,enum usb_tc_state_e tc_state,enum usb_tc_substate_e tc_substate)
{
	tc->usb_tc_state = tc_state;
	tc->usb_tc_substate = tc_substate;
	tc->is_in_prswap = 0;
	lib_printk("tc[%d]_state = %d,%d\n",tc->tc_index, tc_state,tc_substate);
}

static void TC_Disable_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_OPEN);
	if(tc->tc_index == PORT0_INDEX) gd->tc0_lighting_mode= 0;
	if(tc->tc_index == PORT1_INDEX) gd->tc1_lighting_mode= 0;
	if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(false); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
	usb_tc_set_state(tc,TC_Disable,exit_state);
}
static void TC_Disable_Exit(struct tc_s * tc)
{
	if(tc->typec_delay_ms != 0xffff)
	{
		if(tc->typec_delay_ms  > 0) tc->typec_delay_ms--;
		if(tc->typec_delay_ms == 0) usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
	}
}

#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)

static void TC_SNK_Unattached_Entry(struct tc_s * tc)
{
	if(gd->tc0_lighting_mode && tc->tc_index == 0)
	{
		lib_printk("[SNK-U] p0 light=%d -> DRP!\n", gd->tc0_lighting_mode);
		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
		return;
	}

	if(gd->tc1_lighting_mode && tc->tc_index == 1)
	{
		lib_printk("[SNK-U] p1 light=%d -> DRP!\n", gd->tc1_lighting_mode);
		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
		return;
	}

	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
	lib_printk("[SNK-U] p%d set CC=RD ok\n", tc->tc_index);

    tc->tc_timer_cnt = tc_sys_ticks;
    tc->try_snk_cnt = 0;
    usb_tc_set_state(tc,TC_SNK_Unattached,exit_state);
}
static void TC_SNK_Unattached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//lib_printk("cc1 = %d cc2= %d\n", cc1,cc2);
    if (tc_snk_is_connected(cc1,cc2))
    {
        if (cc1 != TYPEC_CC_OPEN)
        	tc->polarity = TYPEC_POLARITY_CC1;
        else
        	tc->polarity = TYPEC_POLARITY_CC2;
        usb_tc_set_state(tc,TC_SNK_AttachWait,enter_state);
        hal_tcpc_port_dummyload_en(tc->tc_index,true);
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_TRY_CONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_TRY_CONNECT);
    }
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
    else
    {
    	/* USB comm mode: stay in SNK, don't fall back to DRP */
    	if (gd->usb_comm_activated && tc->tc_index == PORT0_INDEX) {
    		tc->tc_timer_cnt = tc_sys_ticks;  /* reset timer, keep waiting for Rp */
    	}
    	else if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) >= 28)
    	{
    		tc->tc_timer_cnt = tc_sys_ticks;
    		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
    	}
    }
#endif
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_SNK_AttachWait_Entry(struct tc_s * tc)
{
	tc->tc_timer_cnt = tc_sys_ticks;
	usb_tc_set_state(tc,TC_SNK_AttachWait,exit_state);
	hal_tcpc_port_dummyload_en(tc->tc_index,true);
}

static void TC_SNK_AttachWait_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_SNK_AttachWait,enter_state);
    }
    else if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > 3 && (tc_snk_is_disconnected(tc)))
    {
    	/* USB comm mode on Port0: don't escape to SRC, go back to SNK_Unattached and retry */
    	if (gd->usb_comm_activated && tc->tc_index == PORT0_INDEX)
    		usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
    	else
    		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
    }
    else if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
    {
    	hal_tcpc_port_dummyload_en(tc->tc_index,false);
        if(hal_tcpc_vbus_is_present(tc->tc_index) && hal_tcpc_vbus_is_vsafe5v())
        {
			#if(CONFIG_TC_TRY_SOURCE_SUPPORT_EN)
				if(tc->try_src_cnt >= 3 || tc->is_deadbattery
				   || (gd->usb_comm_activated && tc->tc_index == PORT0_INDEX))
				{
					usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
					lib_printk("try cnt= %d d=%d\n", tc->try_src_cnt,tc->is_deadbattery);
				}
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
    tc->tc_timer_cnt = tc_sys_ticks;
	hal_tcpc_set_polarity(tc->tc_index,tc->polarity);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SINK,TYPEC_DEVICE);
    usb_tc_set_state(tc,TC_SNK_Attached,exit_state);

    tc->try_snk_cnt = 5;
    tc->try_src_cnt = 0;
    if(tc->tc_index == PORT0_INDEX)
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS);
    else
    	osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS);

    if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(true); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
}


static void TC_SNK_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	/* USB comm mode on Port0: skip VBUS check, rely on CC only.
	 * External device (PC/phone) needs time to provide VBUS after CC connect. */
	{
		bool vbus_low = (g_buckboost.adc_vbus <= 4000);
		if (gd->usb_comm_activated && tc->tc_index == PORT0_INDEX)
			vbus_low = false;

		if(tc_snk_is_disconnected(tc) || vbus_low)
		{
			lib_printk("[SNK-D] p%d cc1=%d cc2=%d pol=%d vbus=%d disc=%d\n",
			       tc->tc_index, cc1, cc2, tc->polarity,
			       g_buckboost.adc_vbus, tc_snk_is_disconnected(tc));
			//tc->tc_timer_cnt++;
			if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
			{

				if(hal_tcpc_vbus_is_removed(tc->tc_index))
				{
					lib_printk("[SNK-DISC] p%d vbus_removed\n", tc->tc_index);
					usb_pd_set_event(tc->tc_index,USB_PD_EVT_SNK_UNATTACH);
					usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
					hal_tcpc_port_dummyload_en(tc->tc_index,true);
					hal_tcpc_set_gate_en(tc->tc_index,false);
					if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(false); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
					hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
					//osal_set_event(USB_DPDM_TASK, DPDM_EVT_SNK_UNATTCHED);
					if(tc->tc_index == 0)
					{
						lib_printk("[UC-A]\n");
						port_manager_set_event(PORT0_EVENT_UNCONNECT);
					}
					else
						port_manager_set_event(PORT1_EVENT_UNCONNECT);
				}
			}
		}
		else
		{
			tc->tc_timer_cnt = tc_sys_ticks;
		}
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
    tc->tc_timer_cnt = tc_sys_ticks;
    usb_tc_set_state(tc,TC_SRC_Unattached,exit_state);
}
static void TC_SRC_Unattached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);

	//lib_printk("cc1= %d cc2=%d\n", cc1,cc2);
    if(tc_src_is_connected(cc1,cc2) || tc_acc_is_connected(cc1,cc2))
    {
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_TRY_CONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_TRY_CONNECT);
        usb_tc_set_state(tc,TC_SRC_AttachWait,enter_state);
    }
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
    else
    {
    	//tc->tc_timer_cnt++;
    	if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) >= 58)
    	{
    		tc->tc_timer_cnt = tc_sys_ticks;
    		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
    	}
    }
#endif
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}
static void TC_SRC_AttachWait_Entry(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	tc->tc_timer_cnt = tc_sys_ticks;
	hal_tcpc_port_dummyload_en(tc->tc_index,true);
    if(cc1 == TYPEC_CC_RD)
        tc->polarity = TYPEC_POLARITY_CC1;
    else
    	tc->polarity = TYPEC_POLARITY_CC2;

	usb_tc_set_state(tc,TC_SRC_AttachWait,exit_state);
}
static void TC_SRC_AttachWait_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_SRC_AttachWait,enter_state);
    }
    else if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > 3 && (tc_src_is_disconnected(tc)) && !tc_acc_is_connected(cc1,cc2))
    {
    	usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
    }
    else if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_CC_DEBOUNCE)
    {
    	hal_tcpc_port_dummyload_en(tc->tc_index,false);
        if(tc_debug_is_connected(cc1,cc2))
        {
        	usb_tc_set_state(tc,TC_DEBUG_Attached,enter_state);
        }
        else if(tc_acc_is_connected(cc1,cc2))
        {
        	usb_tc_set_state(tc,TC_ACCESSORY_Attached,enter_state);
        }
        else if(tc_src_is_connected(cc1,cc2))
        {
			#if(CONFIG_TC_TRY_SINK_SUPPORT_EN)
				if(tc->try_snk_cnt >= 5)
					usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
				else
					usb_tc_set_state(tc,TC_Try_SNK,enter_state);
			#else
				if(hal_tcpc_vbus_is_vsafe5v() &&hal_tcpc_vbus_is_vsfae0v(tc->tc_index))
					usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
			#endif
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}
static void TC_SRC_Attached_Entry(struct tc_s * tc)
{
	tc->tc_timer_cnt = tc_sys_ticks;
	hal_tcpc_set_polarity(tc->tc_index,tc->polarity);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SOURCE,TYPEC_HOST);
    usb_tc_set_state(tc,TC_SRC_Attached,exit_state);
    //tc->try_src_cnt = 0;
    if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(true); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
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

    if(gd->tc0_lighting_mode && tc->tc_index == 0)
    {
		usb_pd_set_event(tc->tc_index,USB_PD_EVT_SRC_UNATTACH);
		osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
		hal_tcpc_set_gate_en(tc->tc_index,false);
        lib_printk("[UC-B]\n");
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_UNCONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_UNCONNECT);
    }

    if(gd->tc1_lighting_mode && tc->tc_index == 1)
    {
		usb_pd_set_event(tc->tc_index,USB_PD_EVT_SRC_UNATTACH);
		osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
		usb_tc_set_state(tc,TC_DRP_TOGGLE,enter_state);
		hal_tcpc_set_gate_en(tc->tc_index,false);
        lib_printk("[UC-C]\n");
        if(tc->tc_index == 0)
        	port_manager_set_event(PORT0_EVENT_UNCONNECT);
        else
        	port_manager_set_event(PORT1_EVENT_UNCONNECT);
    }

    if(pdlib_is_connect()) tc->try_src_cnt = 0;

	if(tc_src_is_disconnected(tc)) //CC断开
	{
		//tc->tc_timer_cnt++;
		uint32_t timeout = 0;

		timeout = g_buckboost.adc_vbus > 5500 ? TC_T_PD_DEBOUNCE :(TC_T_PD_DEBOUNCE + 250);

		if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > timeout)
		{
			usb_pd_set_event(tc->tc_index,USB_PD_EVT_SRC_UNATTACH);
			osal_set_event(USB_DPDM_TASK,DPDM_EVT_SRC_UNATTCHED);
			if(tc->try_src_cnt >= 3) usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
			else usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
			if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(false); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
			hal_tcpc_port_dummyload_en(tc->tc_index,true);
			hal_tcpc_set_gate_en(tc->tc_index,false);
			hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
            lib_printk("[UC-D]\n");
            if(tc->tc_index == 0)
            	port_manager_set_event(PORT0_EVENT_UNCONNECT);
            else
            	port_manager_set_event(PORT1_EVENT_UNCONNECT);
		}
	}
	else
	{
		tc->tc_timer_cnt = tc_sys_ticks;
	}
}


static void TC_DEBUG_Attached_Entry(struct tc_s * tc)
{
	usb_tc_set_state(tc,TC_DEBUG_Attached,exit_state);
}

static void TC_DEBUG_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	if(tc_debug_is_disconnected(cc1,cc2)) //CC�Ͽ�
	{
		//tc->tc_timer_cnt++;
		if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
		{
			usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
		}
	}
	else
	{
		tc->tc_timer_cnt = tc_sys_ticks;
	}
}

static void TC_ACCESSORY_Attached_Entry(struct tc_s * tc)
{
	usb_tc_set_state(tc,TC_ACCESSORY_Attached,exit_state);
}

static void TC_ACCESSORY_Attached_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
    tc->cc1 = cc1;
    tc->cc2 = cc2;
	if(tc_acc_is_disconnected(cc1,cc2)) //CC�Ͽ�
	{
		//tc->tc_timer_cnt++;
		if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
		{
			usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
		}
	}
	else
	{
		tc->tc_timer_cnt = tc_sys_ticks;
	}
}

#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
static uint8_t a_toggle_rp_or_rd = 0;
static uint8_t a_toggle_rd_cnt = 0;
static uint8_t a_toggle_rp_cnt = 0;
static uint8_t b_toggle_rp_or_rd = 0;
static uint8_t b_toggle_rd_cnt = 0;
static uint8_t b_toggle_rp_cnt = 0;


static uint16_t tc0_delay = 0;
static uint16_t tc1_delay = 0;

static void TC_DRP_TOGGLE_Entry(struct tc_s * tc)
{
	if (gd->usb_comm_activated)
		lib_printk("[DRP-E] p%d comm=%d light0=%d\n", tc->tc_index, gd->usb_comm_activated, gd->tc0_lighting_mode);

	if(gd->tc0_lighting_mode && tc->tc_index == PORT0_INDEX)
	{
		static uint16_t cnt0 = 0;
		enum tc_cc_status cc1,cc2;
		hal_tcpc_get_cc(tc->tc_index, &cc1,&cc2);
	    tc->cc1 = cc1;
	    tc->cc2 = cc2;
		//if(tc_src_is_disconnected(tc))// || gd->dp_result != tcpm_dp_get_result())
		if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
		{
			cnt0++;
			if(cnt0 > 1000) gd->tc0_lighting_mode = 0;
		}
		else cnt0 = 0;
		return;
	}

	if(gd->tc1_lighting_mode && tc->tc_index == PORT1_INDEX)
	{
		static uint16_t cnt1 = 0;
		enum tc_cc_status cc1,cc2;
		hal_tcpc_get_cc(tc->tc_index, &cc1,&cc2);
	    tc->cc1 = cc1;
	    tc->cc2 = cc2;
		//if(tc_src_is_disconnected(tc))// || gd->dp_result != tcpm_dp_get_result())
		if(cc1 != TYPEC_CC_RD && cc2 != TYPEC_CC_RD)
		{
			cnt1++;
			if(cnt1 > 1000)gd->tc1_lighting_mode = 0;
		}
		else cnt1 = 0;
		return;
	}
	DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 0;
	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 0;  //
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.DPDM_EN = 0;
	DPDM_QC_SINK->BC1P2_INTMSK_CTRL.BITS.BC1P2_EN = 0;
    hal_tcpc_set_vconn(tc->tc_index,false);
	hal_tcpc_set_pd_rx(tc->tc_index,EN_SOP | EN_HARD_RESET | EN_SOP1 ,false);
	hal_tcpc_set_roles(tc->tc_index,TYPEC_SINK,TYPEC_DEVICE);
    hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_OPEN);
    //tc->try_snk_cnt = 0;
    //tc->try_src_cnt = 0;
    usb_tc_set_state(tc,TC_DRP_TOGGLE,exit_state);
    hal_tcpc_port_dummyload_en(tc->tc_index,false);
    if(tc->tc_index == PORT0_INDEX) usb_dpdm_port1_switch(false); /* PORT0_INDEX = 物理 TypeC-B (PB2/PD0) */
    if(g_port.inhandle_port == tc->tc_index && g_port.state == PORT_INHANDLING)
    {
//    	if(tc->tc_index == 0)
//    		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT0_CONNECT_CLOSED);
//    	else
//    		osal_set_event(PORT_MANAGER_TASK,PORT_ENUM_EVT_PORT1_CONNECT_CLOSED);

//		if(tc->tc_index == 0)
//			port_manager_set_event(PORT0_EVENT_UNCONNECT);
//		else
//			port_manager_set_event(PORT1_EVENT_UNCONNECT);
    }

    a_toggle_rp_or_rd = 0;
    a_toggle_rd_cnt = 0;
    a_toggle_rp_cnt = 0;

    b_toggle_rp_or_rd = 0;
    b_toggle_rd_cnt = 0;
    b_toggle_rp_cnt = 0;

	if(tc0_delay > 0) lib_printk("[DRP-EN] d0=%d\n", tc0_delay);
	tc0_delay = 0;
	tc1_delay = 0;
}

static void TC_DRP_TOGGLE_Exit(struct tc_s * tc)
{

	if(tc->tc_index == 0)
	{
		static uint8_t a_delay_cnt = 0;

		if(a_toggle_rp_or_rd == 0)
		{
			if(a_toggle_rd_cnt == 0)
			{
				hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
				//lib_printk("Set Rd\n");
			}
			else if(a_toggle_rd_cnt < 35)
			{
				enum tc_cc_status cc1,cc2;
				hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
				if (tc_snk_is_connected(cc1,cc2))
				{
					//lib_printk("SNK [%x %x %x %x %x]\n",cc1,cc2,TCPC->CCA_STAT.WORD,TCPC->CCA_ROLE.WORD,a_delay_cnt);
					a_delay_cnt++;
					a_toggle_rd_cnt--;
					if(a_delay_cnt >= 10)
					{
						usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
						a_delay_cnt = 0;
					}
				}
				else
					a_delay_cnt = 0;
			}
			else
			{
				a_toggle_rp_cnt = 0;
				a_toggle_rp_or_rd = 1;
			}

			a_toggle_rd_cnt++; if(a_toggle_rd_cnt >= 100) a_toggle_rd_cnt = 100;
		}
		else
		{
			if(a_toggle_rp_cnt == 0)
			{
				hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RP_3_0);
//				lib_printk("Set Rp\n");
			}
			else if(a_toggle_rp_cnt < 45)
			{
				enum tc_cc_status cc1,cc2;
				hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
				if(tc_src_is_connected(cc1,cc2) || tc_acc_is_connected(cc1,cc2))
				{
					//lib_printk("SRC [%x %x %x %x %x]\n",cc1,cc2,TCPC->CCA_STAT.WORD,TCPC->CCA_ROLE.WORD,a_delay_cnt);
					a_delay_cnt++;
					a_toggle_rp_cnt--;
					if(a_delay_cnt >= 10)
					{
						usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
						a_delay_cnt = 0;
					}
				}
				else
					a_delay_cnt = 0;
			}
			else
			{
				a_toggle_rd_cnt = 0;
				a_toggle_rp_or_rd = 0;
			}

			a_toggle_rp_cnt++; if(a_toggle_rp_cnt >= 100) a_toggle_rp_cnt = 100;
		}
	}
	else
	{
		static uint8_t b_delay_cnt = 0;

		if(b_toggle_rp_or_rd == 0)
		{
			if(b_toggle_rd_cnt == 0)
			{
				hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
				//lib_printk("Set Rd\n");
			}
			else if(b_toggle_rd_cnt < 35)
			{
				enum tc_cc_status cc1,cc2;
				hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
				if (tc_snk_is_connected(cc1,cc2))
				{
					//lib_printk("SNK [%x %x %x %x %x]\n",cc1,cc2,TCPC->CCA_STAT.WORD,TCPC->CCA_ROLE.WORD,delay_cnt);
					b_delay_cnt++;
					b_toggle_rd_cnt--;
					if(b_delay_cnt >= 10)
					{
						usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
						b_delay_cnt = 0;
					}
				}
				else
					b_delay_cnt = 0;
			}
			else
			{
				b_toggle_rp_cnt = 0;
				b_toggle_rp_or_rd = 1;
			}

			b_toggle_rd_cnt++; if(b_toggle_rd_cnt >= 100) b_toggle_rd_cnt = 100;
		}
		else
		{
			if(b_toggle_rp_cnt == 0)
			{
				hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RP_3_0);
				//lib_printk("Set Rp\n");
			}
			else if(b_toggle_rp_cnt < 45)
			{
				enum tc_cc_status cc1,cc2;
				hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
				if(tc_src_is_connected(cc1,cc2) || tc_acc_is_connected(cc1,cc2))
				{
					//lib_printk("SRC [%x %x %x %x %x]\n",cc1,cc2,TCPC->CCA_STAT.WORD,TCPC->CCA_ROLE.WORD,delay_cnt);
					b_delay_cnt++;
					b_toggle_rp_cnt--;
					if(b_delay_cnt >= 10)
					{
						usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
						b_delay_cnt = 0;
					}
				}
				else
					b_delay_cnt = 0;
			}
			else
			{
				b_toggle_rd_cnt = 0;
				b_toggle_rp_or_rd = 0;
			}

			b_toggle_rp_cnt++; if(b_toggle_rp_cnt >= 100) b_toggle_rp_cnt = 100;
		}
	}



    //LJJ ADD Delay

    if(g_port.inhandle_port == tc->tc_index && g_port.state == PORT_INHANDLING)
    {
		if(tc->tc_index == 0)
		{
			tc0_delay++;
			if(tc0_delay <= 3 || tc0_delay == 500 || tc0_delay == 999)
				lib_printk("[UC-E] d=%d ih=%d st=%d\n", tc0_delay, g_port.inhandle_port, g_port.state);
			if(tc0_delay > 1000)
			{
				port_manager_set_event(PORT0_EVENT_UNCONNECT);
				tc0_delay = 0;
			}
		}
		else
		{
			tc1_delay++;
			if(tc1_delay > 1000)
			{
				port_manager_set_event(PORT1_EVENT_UNCONNECT);
				tc1_delay = 0;
			}
		}
    }
	//		if(tc->tc_index == 0)
	//			port_manager_set_event(PORT0_EVENT_UNCONNECT);
	//		else
	//			port_manager_set_event(PORT1_EVENT_UNCONNECT);
}

static void TC_Try_SNK_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index, TYPEC_CC_RD);
	tc->tc_timer_cnt = tc_sys_ticks;
	usb_tc_set_state(tc,TC_Try_SNK,exit_state);
}
static void TC_Try_SNK_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_Try_SNK,enter_state);
    }
    else
    {
        if(tc_snk_is_connected(cc1,cc2) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_TRY_CC_DEBOUNCE)
        {
        	usb_tc_set_state(tc,TC_SNK_Attached,enter_state);
        }

        if(tc_snk_is_disconnected(tc) &&  (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_DRP_TRYWAIT)
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
	tc->tc_timer_cnt = tc_sys_ticks;
	usb_tc_set_state(tc,TC_TryWAIT_SRC,exit_state);
}
static void TC_TryWAIT_SRC_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_TryWAIT_SRC,enter_state);
    }
    else
    {
    	if(tc_src_is_disconnected(tc) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
		{
    		usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
		}
		else if(tc_src_is_connected(cc1,cc2) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_CC_DEBOUNCE)
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
	tc->tc_timer_cnt = tc_sys_ticks;

	usb_tc_set_state(tc,TC_Try_SRC,exit_state);
}


static void TC_Try_SRC_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;
    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_Try_SRC,enter_state);
    }
    else
    {
        if(tc_src_is_connected(cc1,cc2) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_TRY_CC_DEBOUNCE)
        {

        	usb_tc_set_state(tc,TC_SRC_Attached,enter_state);
        }

        if(tc_src_is_disconnected(tc) &&  (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_DRP_TRY)
        {
        	//usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
			if(tc->try_src_cnt >= 3) usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
			else usb_tc_set_state(tc,TC_SRC_Unattached,enter_state);
        }
    }
    tc->cc1 = cc1;
    tc->cc2 = cc2;
}

static void TC_TryWAIT_SNK_Entry(struct tc_s * tc)
{
	hal_tcpc_set_cc(tc->tc_index,TYPEC_CC_RD);
	tc->tc_timer_cnt = tc_sys_ticks;
	usb_tc_set_state(tc,TC_TryWAIT_SNK,exit_state);
}
static void TC_TryWAIT_SNK_Exit(struct tc_s * tc)
{
	enum tc_cc_status cc1,cc2;
	hal_tcpc_get_cc(tc->tc_index,&cc1,&cc2);
	//tc->tc_timer_cnt++;

    if ((cc1 != tc->cc1 && tc->polarity == TYPEC_POLARITY_CC1) || (cc2 != tc->cc2  && tc->polarity == TYPEC_POLARITY_CC2))   //CC changes
    {
    	usb_tc_set_state(tc,TC_TryWAIT_SNK,enter_state);
    }
    else
    {
    	if(tc_snk_is_disconnected(tc) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
		{
    		usb_tc_set_state(tc,TC_SNK_Unattached,enter_state);
		}
		else if(tc_snk_is_connected(cc1,cc2) && (uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_PD_DEBOUNCE)
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
	tc->tc_timer_cnt = tc_sys_ticks;
	usb_tc_set_state(tc,TC_ErrorRecovery,exit_state);
}
static void TC_ErrorRecovery_Exit(struct tc_s * tc)
{
	//tc->tc_timer_cnt++;
	if((uint32_t)(tc_sys_ticks - tc->tc_timer_cnt) > TC_T_ERROR_RECOVERY)
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
	{TC_ACCESSORY_Attached_Entry,TC_ACCESSORY_Attached_Exit}, 		//TC_ACCESSORY_Attached
	{TC_ErrorRecovery_Entry,TC_ErrorRecovery_Exit},		//TC_ErrorRecovery
															//TC_STATE_MAX,
};


void usb_tc_run(void)
{


if(lib_para.typec_a_support)
{
	if(g_tc[TYPEC_PORT_A].is_in_prswap) return;

	if(g_tc[TYPEC_PORT_A].usb_tc_substate == enter_state)
	{
		usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].enter_cb(&g_tc[TYPEC_PORT_A]);
	}
	else
	{
		usb_tc_table[g_tc[TYPEC_PORT_A].usb_tc_state].exit_cb(&g_tc[TYPEC_PORT_A]);
	}
}
if(lib_para.typec_b_support)
{
	if(g_tc[TYPEC_PORT_B].is_in_prswap) return;

	if(g_tc[TYPEC_PORT_B].usb_tc_substate == enter_state)
	{
		usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].enter_cb(&g_tc[TYPEC_PORT_B]);
	}
	else
	{
		usb_tc_table[g_tc[TYPEC_PORT_B].usb_tc_state].exit_cb(&g_tc[TYPEC_PORT_B]);
	}
}
}

