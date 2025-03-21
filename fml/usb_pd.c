#include "usb_pd.h"
#include "tcpc.h"
#include "pd.h"
#include "typec.h"
#include "printk.h"
#include "osal.h"
#include "regdef.h"
#include "usb_pd.h"
#include "usbpd_config.h"
#include "port_manager.h"
#include "ntc.h"

#define SINK_PDO_MATCH_MODE_VOLTAGE					0
#define SINK_PDO_MATCH_MODE_VOLTAGE_CURRENT			1

extern const struct usb_pd_state_task_t usb_pd_tasks_table[];

static uint32_t usb_pd_event = 0;
static uint8_t usb_pd_state = 0;
static uint8_t usb_pd_substate = 0;
uint32_t pd_rx_buff[32];
uint8_t rx_cnt = 0;

uint8_t softreset_reason = 0;

struct usb_pd_s g_usb_pd_s;
struct usb_pd_pkt_t g_pd_packet;

static uint8_t usb_pd_disable = 0;

static uint8_t need_rechager = 0;

void usb_set_disable(void)
{
	usb_pd_disable = 1;
}

void usb_set_enable(void)
{
	usb_pd_disable = 0;
}




#if(BUCKBOOST_USED_SW7201 == 1)
const uint32_t source_pdo[] =
{
	#define SOURCE_PDO_FIXED_FLAGS     			(PDO_FIXED_UNCONSTRAINED_POWER)
	[0] = PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),
	[1] = PDO_FIXED(9000, 3000, 0),
	[2] = PDO_FIXED(12000, 3000, 0),
	[3] = PDO_FIXED(15000, 3000, 0),
	[4] = PDO_PPS_APDO(5000,16000,3000),
};
#elif(BUCKBOOST_USED_NU6801 == 1)
const uint32_t source_pdo[] =
{
	#define SOURCE_PDO_FIXED_FLAGS     			(PDO_FIXED_UNCONSTRAINED_POWER)
	[0] = PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),
	[1] = PDO_FIXED(9000, 2000, 0),
	[2] = PDO_FIXED(12000, 1500, 0),
	[3] = PDO_PPS_APDO(5000,11000,2000),
};

const uint32_t source_pdo_ntc[] =
{
	[0] = PDO_FIXED(5000, 2000, SOURCE_PDO_FIXED_FLAGS),
};
#endif

const uint32_t sink_pdo[] =
{
	#define SINK_PDO_FIXED_FLAGS     			(0)
	[0] = PDO_FIXED(5000, 3000, SINK_PDO_FIXED_FLAGS),
	[1] = PDO_FIXED(9000, 2000, 0),

	//[2] = PDO_FIXED(15000, 3000, 0),
};

static union usb_pd_timer_u usb_pd_timers[USBPD_TIMER_MAX];

void updata_pdo_of_source(uint32_t * pdo,uint8_t n_pdo)
{
	osal_mem_clear((void*)g_usb_pd_s.src_source_pdo,28);
	for(uint8_t i = 0; i< n_pdo;i++)
	{
		g_usb_pd_s.src_source_pdo[i] = pdo[i];
	}
	g_usb_pd_s.src_tx_pdo_n = n_pdo;

	printk("update source caps:%d\n",n_pdo);
}

void updata_pdo_of_sink(uint32_t * pdo,uint8_t n_pdo)
{
	osal_mem_clear((void*)g_usb_pd_s.snk_sink_pdo,28);
	for(uint8_t i = 0; i< n_pdo;i++)
	{
		g_usb_pd_s.snk_sink_pdo[i] = pdo[i];
	}
	g_usb_pd_s.snk_tx_pdo_n = n_pdo;

	printk("update sink caps:%d\n",n_pdo);
}

void usb_pd_init(void)
{
	osal_mem_clear(&g_usb_pd_s,sizeof(struct usb_pd_s));
	usb_pd_reset_prl();
	g_usb_pd_s.snk_rdo = RDO_FIXED(1, 500, 500,0);
	g_usb_pd_s.src_tx_pdo_n = sizeof(source_pdo) / 4;
	for(uint8_t i = 0; i< (sizeof(source_pdo) /4);i++)
	{
		g_usb_pd_s.src_source_pdo[i] = source_pdo[i];
	}

	g_usb_pd_s.snk_tx_pdo_n = sizeof(sink_pdo) / 4;
	for(uint8_t i = 0; i< (sizeof(sink_pdo) /4);i++)
	{
		g_usb_pd_s.snk_sink_pdo[i] = sink_pdo[i];
	}
	hal_tcpc_pd_phy_disable();
	usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
}

void usb_pd_snk_dump_pdoinfo(void)
{
	//struct usb_pd_source_cap_packet_t * source_pdo =  (struct usb_pd_source_cap_packet_t *) &g_usb_pd_s.snk_rx_source_cap;
	uint32_t source_pdo;
	usbpd_printk("\n");
	for(uint8_t i= 0; i<g_usb_pd_s.snk_rx_pdo_n;i++)
	{
		source_pdo = (uint32_t)g_usb_pd_s.snk_rx_source_cap[i];
		if(pdo_type(source_pdo) == PDO_TYPE_FIXED)
		{
			printk("PDO%d FIX: %dmV %dmA \n",i+1,
					pdo_fixed_voltage(source_pdo),pdo_max_current(source_pdo));
		}
		else if(pdo_type(source_pdo) == PDO_TYPE_APDO)
		{
			printk("PDO%d PPS: %dmV-%dmV %dmA \n",i+1,pdo_pps_apdo_min_voltage(source_pdo),
							pdo_pps_apdo_max_voltage(source_pdo),pdo_pps_apdo_max_current(source_pdo));
		}
	}
}

void usb_pd_requsrt_voltage(uint32_t pdo_position,uint16_t voltage,uint16_t current)
{
	if(pdo_position == 0 || pdo_position >7) return;

	uint32_t source_pdo = g_usb_pd_s.snk_rx_source_cap[pdo_position-1];

	if(pdo_type(source_pdo) == PDO_TYPE_APDO)
	{
		uint16_t max_current = pdo_pps_apdo_max_current(source_pdo);
		max_current = current < max_current ? current : max_current;
		g_usb_pd_s.snk_rdo = RDO_PROG(pdo_position, voltage, max_current, 0);
		g_usb_pd_s.is_in_pps = 1;
	}
	else
	{
		uint16_t max_current = pdo_max_current(source_pdo);
		max_current = current < max_current ? current : max_current;
		g_usb_pd_s.snk_rdo = RDO_FIXED(pdo_position, max_current, max_current, 0);
		g_usb_pd_s.is_in_pps = 0;
	}
	usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_SNK_SET_VOLTAGE);
}


uint32_t usb_pd_snk_match_pdo(void * source_cap,uint8_t src_pdo_n,uint8_t match_mode)
{
	struct usb_pd_source_cap_packet_t * source_pdo =  (struct usb_pd_source_cap_packet_t *) source_cap;
	struct usb_pd_sink_cap_packet_t * snk_pdo =  (struct usb_pd_sink_cap_packet_t *) g_usb_pd_s.snk_sink_pdo;
	uint16_t op_current,max_current;
	uint8_t index = 0;

	if(match_mode == SINK_PDO_MATCH_MODE_VOLTAGE)
	{
		//for(uint8_t n=sizeof(sink_pdo) / 4; n != 0 ; n--)
		for(uint8_t n=g_usb_pd_s.snk_tx_pdo_n; n != 0 ; n--)
		{
			for(uint8_t m = 0; m< src_pdo_n;m++)
			{
				if(source_pdo->source_pdo[m].BITS.FIX_BITS.fixed == 0 )
				{
					if(source_pdo->source_pdo[m].BITS.FIX_BITS.voltage == snk_pdo->sink_pdo[n-1].BITS.FIX_BITS.voltage)
					{
						max_current = source_pdo->source_pdo[m].BITS.FIX_BITS.max_current * 10;
						op_current = snk_pdo->sink_pdo[n-1].BITS.FIX_BITS.op_current * 10;
						if(op_current > max_current) op_current = max_current;
						index = m + 1;
						break;
					}
				}
			}
			if(index != 0) break;  //match ok
		}

	}
	else
	{
		return RDO_FIXED(1, 500, 500, 0);
	}

	if(index != 0)
	{
		return RDO_FIXED(index, op_current, max_current, 0);
	}
	else
	{
		return RDO_FIXED(1, 500, 500, 0);
	}

}



void usb_pd_reset_prl(void)
{
	g_usb_pd_s.rx_sop_msgid = -1;
	g_usb_pd_s.rx_sop1_msgid = -1;
	g_usb_pd_s.tx_sop_msgid = 0;
	g_usb_pd_s.tx_sop1_msgid = 0;
	g_usb_pd_s.communitcate_capable = 0;
	//g_usb_pd_s.hardreset_counter = 0;
	g_usb_pd_s.sink_request_index = 1;
	g_usb_pd_s.pe_prl_busy = 0;
	g_usb_pd_s.is_in_pps = 0;
	g_usb_pd_s.caps_counter = 0;
	g_usb_pd_s.snk_rx_pdo_n = 0;
	g_usb_pd_s.in_bist_mode = 0;
	//timer all reset
	for(uint8_t i = 0; i< USBPD_TIMER_MAX;i++)
	{
		usb_pd_timers[i].word = 0;
	}
}

// period unit is mS, Max 32767
void usb_pd_timer_all_reset(void)
{
	for(uint8_t i = 0; i< USBPD_TIMER_MAX;i++)
	{
		usb_pd_timers[i].word = 0;
	}
}

void usb_pd_timer_start(enum usb_pd_timer_e timer_id,uint16_t period)
{
	usb_pd_timers[timer_id].timer.timeout = false;
	usb_pd_timers[timer_id].timer.time_cnt = period;
	usb_pd_timers[timer_id].timer.state = TIMER_RUNNING;
}

void usb_pd_timer_stop(enum usb_pd_timer_e timer_id)
{
	usb_pd_timers[timer_id].timer.time_cnt = 0;
	usb_pd_timers[timer_id].timer.state = TIMER_STOP;
}

bool usb_pd_timer_is_timeout(enum usb_pd_timer_e timer_id)
{
	if(usb_pd_timers[timer_id].timer.state == TIMER_RUNNING)
	{
		return usb_pd_timers[timer_id].timer.timeout;
	}
	return false;
}



void usb_pd_set_state(enum usb_pd_state_e pe_state,enum usb_pd_substate_e pe_substate)
{
	usb_pd_state = pe_state;
	usb_pd_substate = pe_substate;
}

void usb_pd_set_event(uint8_t tc_index,uint32_t event)
{
	if(tc_index != g_tcpc.tc_port_map) return;
	usb_pd_event |= event;
}

void usb_pd_clear_event(uint32_t event)
{
	usb_pd_event &= ~event;
}


static void PE_SNK_RSC_Disable_Entry(void)
{
	g_usb_pd_s.explicit_contract = 0;
	usb_pd_reset_prl();
	hal_tcpc_reset_pd_phy();
	hal_tcpc_pd_phy_disable();
	hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET ,false);
	usb_pd_set_state(PE_SNK_RSC_Disable,exit_state);
}

static void PE_SNK_RSC_Disable_Exit(void)
{

}

#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)

static void PE_SNK_Startup_Entry(void)
{
	usb_pd_reset_prl();
	hal_tcpc_reset_pd_phy();
	hal_tcpc_pd_phy_enable();
	hal_tcpc_set_phy_rx_vref(VREF_0P36);
	//g_usb_pd_s.snk_rdo = RDO_FIXED(1, 500, 500,0);
	g_usb_pd_s.nego_revision = PD_REV30;
	g_usb_pd_s.explicit_contract = 0;
	hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET ,false);
	usb_pd_set_state(PE_SNK_Startup,exit_state);
}

static void PE_SNK_Startup_Exit(void)
{
	usb_pd_set_state(PE_SNK_Discovery,enter_state);
}

static void PE_SNK_Discovery_Entry(void)
{
	usb_pd_set_state(PE_SNK_Discovery,exit_state);
}

static void PE_SNK_Discovery_Exit(void)
{
	if(hal_tcpc_vbus_is_present(g_tcpc.tc_port_map))
	{
		hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET ,true);
		usb_pd_set_state(PE_SNK_Wait_for_Capabilities,enter_state);
	}
}

static void PE_SNK_Wait_for_Capabilities_Entry(void)
{
	usb_pd_timer_start(SinkWaitCapTimer,tSinkWaitCapTime);
	usb_pd_set_state(PE_SNK_Wait_for_Capabilities,exit_state);
}

static void PE_SNK_Wait_for_Capabilities_Exit(void)
{
	if(usb_pd_timer_is_timeout(SinkWaitCapTimer))
	{
		//usbpd_printk("SenderResponseTimer timeout\n");
		if(g_usb_pd_s.hardreset_counter <= N_HARDRESET_COUNTER)
			usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
		else
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
	}
}



static void PE_SNK_Evaluate_Capability_Entry(void)
{
	g_usb_pd_s.hardreset_counter = 0;
	g_usb_pd_s.sink_request_index = 2;
	usb_pd_timer_stop(SinkWaitCapTimer);

	if(g_usb_pd_s.explicit_contract)  need_rechager = 1;

	for(uint8_t i= 0; i<7;i++)
	{
		g_usb_pd_s.snk_rx_source_cap[i] = 0;
	}

	for(uint8_t i= 0; i<g_pd_packet.hdr.BITS.n_data_object;i++)
	{
		g_usb_pd_s.snk_rx_source_cap[i] = g_pd_packet.msg.source_cap.source_pdo[i].WORD;
	}
	g_usb_pd_s.snk_rx_pdo_n = g_pd_packet.hdr.BITS.n_data_object;

	usb_pd_set_state(PE_SNK_Evaluate_Capability,exit_state);
}

static void PE_SNK_Evaluate_Capability_Exit(void)
{
	//if(g_usb_pd_s.explicit_contract == 0)
	{
		g_usb_pd_s.snk_rdo = RDO_FIXED(1, pdo_max_current(g_usb_pd_s.snk_rx_source_cap[0]), pdo_max_current(g_usb_pd_s.snk_rx_source_cap[0]), 0);//usb_pd_snk_match_pdo(g_usb_pd_s.snk_rx_source_cap,g_usb_pd_s.snk_rx_pdo_n,SINK_PDO_MATCH_MODE_VOLTAGE);
	}
	usb_pd_set_state(PE_SNK_Select_Capability,enter_state);
}


static void PE_SNK_Select_Capability_Entry(void)
{
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REQUEST;
	//usbpd_printk("snk rdo =%x\n",g_usb_pd_s.snk_rdo);
	hal_tcpc_send_request_mgs(g_usb_pd_s.snk_rdo);
}

static void PE_SNK_Select_Capability_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
	}
}

static void PE_SNK_Transition_Sink_Entry(void)
{
	usb_pd_timer_start(PSTransitionTimer,tPSTransitionTime);
	usb_pd_set_state(PE_SNK_Transition_Sink,exit_state);
}

static void PE_SNK_Transition_Sink_Exit(void)
{
	if(usb_pd_timer_is_timeout(PSTransitionTimer))
	{
		usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
	}
}

static void PE_SNK_Ready_Entry(void)
{
	usb_pd_timer_all_reset();
	if(g_usb_pd_s.is_in_pps)
	{
		usb_pd_timer_start(SinkPPSPeriodicTimer,1500);
	}
	g_usb_pd_s.explicit_contract = 1;



	usb_pd_set_state(PE_SNK_Ready,exit_state);

	if(need_rechager == 1)
	{
		port_manager_set_event(PORT_EVENT_RESET_CHARGE);
		need_rechager = 0;
	}

	printk("%s\n",__func__);

	//osal_set_event(USB_TASK,TCPM_EVT_PD_READY);
	osal_start_timerEx(TCPM_PSREADY_TIMER, 500, 0, USB_TASK, TCPM_EVT_PD_READY);
}

static void PE_SNK_Ready_Exit(void)
{
	if(g_usb_pd_s.is_in_pps && usb_pd_timer_is_timeout(SinkPPSPeriodicTimer))
	{
		usb_pd_set_state(PE_SNK_Select_Capability,enter_state);
	}

}

static void PE_SNK_Hard_Reset_Entry(void)
{
	g_usb_pd_s.hardreset_counter++;
	hal_tcpc_send_hardreset();
	usb_pd_set_state(PE_SNK_Hard_Reset,exit_state);
}

static void PE_SNK_Hard_Reset_Exit(void)
{
	usb_pd_set_state(PE_SNK_Transition_to_default,enter_state);
}

static void PE_SNK_Transition_to_default_Entry(void)
{
	hal_tcpc_set_data_role(g_tcpc.tc_port_map,TYPEC_DEVICE);
	usb_pd_reset_prl();
	hal_tcpc_reset_pd_phy();
	usb_pd_set_state(PE_SNK_Transition_to_default,exit_state);
}

static void PE_SNK_Transition_to_default_Exit(void)
{
	usb_pd_set_state(PE_SNK_Wait_for_Capabilities,enter_state);

	need_rechager = 1;
}

static void PE_SNK_Send_Soft_Reset_Entry(void)
{
	usb_pd_reset_prl();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_SOFTRESET;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_SOFT_RESET);
	printk("softreset reason = %d\n",softreset_reason);
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
}

static void PE_SNK_Send_Soft_Reset_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
	}
}

static void PE_SNK_Soft_Reset_Entry(void)
{
	usb_pd_reset_prl();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_ACCEPT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_ACCEPT);
}

static void PE_SNK_Soft_Reset_Exit(void)
{
	usb_pd_set_state(PE_SNK_Wait_for_Capabilities,enter_state);
}

static void PE_SNK_Not_Supported_Received_Entry(void)
{

}

static void PE_SNK_Not_Supported_Received_Exit(void)
{

}

static void PE_SNK_Send_Not_Supported_Entry(void)
{
	if(g_usb_pd_s.nego_revision == PD_REV30)
	{
		hal_tcpc_send_ctrl_mgs(PD_CTRL_NOT_SUPP);
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_NOTSUPPORT;
	}
	else
	{
		hal_tcpc_send_ctrl_mgs(PD_CTRL_REJECT);
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REJECT;
	}
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
	usb_pd_set_state(PE_SNK_Send_Not_Supported,exit_state);
}

static void PE_SNK_Send_Not_Supported_Exit(void)
{

}

static void PE_SNK_Give_Sink_Cap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_GIVESNKCAP;
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
	usb_pd_set_state(PE_SNK_Give_Sink_Cap,exit_state);
	hal_tcpc_send_snk_caps(g_usb_pd_s.snk_sink_pdo,g_usb_pd_s.snk_tx_pdo_n);
}

static void PE_SNK_Give_Sink_Cap_Exit(void)
{

}

static void PE_SNK_Give_Sink_Cap_Ext_Entry(void)
{
	hal_tcpc_send_sink_caps_ext();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_GIVESNKCAP_EXT;
	usb_pd_set_state(PE_SNK_Give_Sink_Cap_Ext,exit_state);
}

static void PE_SNK_Give_Sink_Cap_Ext_Exit(void)
{

}

static void PE_SRC_Give_PPS_Status_Entry(void)
{
	tcpc_pd_send_pps_status();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_GIVEPPS_STA;
	usb_pd_set_state(PE_SRC_Give_PPS_Status,exit_state);
}

static void PE_SRC_Give_PPS_Status_Exit(void)
{

}

#endif


#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
static void PE_SRC_Startup_Entry(void)
{
	usbpd_printk("%s\n",__func__);
	g_usb_pd_s.caps_counter = 0;
	g_usb_pd_s.explicit_contract = 0;
	hal_tcpc_set_pwr_role(g_tcpc.tc_port_map,TYPEC_SOURCE);
	usb_pd_reset_prl();
	hal_tcpc_reset_pd_phy();
	hal_tcpc_pd_phy_enable();
	hal_tcpc_set_phy_rx_vref(VREF_0P62);
	g_usb_pd_s.nego_revision = PD_REV30;
	hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET ,true);
	usb_pd_set_state(PE_SRC_Startup,exit_state);
}

static void PE_SRC_Startup_Exit(void)
{
	usb_pd_set_state(PE_SRC_Discovery,enter_state);
}

static void PE_SRC_Discovery_Entry(void)
{
	usb_pd_timer_start(SourceCapabilityTimer,tSourceCapabilityTime);
	usb_pd_set_state(PE_SRC_Discovery,exit_state);
}

static void PE_SRC_Discovery_Exit(void)
{
	if(usb_pd_timer_is_timeout(SourceCapabilityTimer))
	{
		if(g_usb_pd_s.caps_counter > N_CAPS_COUNT)
			usb_pd_set_state(PE_SRC_Disabled,enter_state);
		else
			usb_pd_set_state(PE_SRC_Send_Capabilities,enter_state);

		usbpd_printk("caps counter =%d \n",g_usb_pd_s.caps_counter);
	}
}

static void PE_SRC_Send_Capabilities_Entry(void)
{
	g_usb_pd_s.caps_counter++;
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_SOURCECAPS;

	if(ntc_ut_flag | ntc_ot_flag)
		updata_pdo_of_source((uint32_t *)source_pdo_ntc,sizeof(source_pdo_ntc) / 4);
	else
		updata_pdo_of_source((uint32_t *)source_pdo,sizeof(source_pdo) / 4);

	hal_tcpc_send_source_caps(g_usb_pd_s.src_source_pdo,g_usb_pd_s.src_tx_pdo_n);
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
}
static void PE_SRC_Send_Capabilities_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
	}
}



uint32_t usb_pd_check_request(struct usb_pd_request_packet_t *rqt)
{
	#define 	check_index_error  				0x01
	#define 	check_current_error  			0x02
	#define 	check_pps_current_error  		0x03
	#define 	check_pps_voltage_error  		0x04
    uint32_t rdo_op_current, pdo_max_current, index;
    uint32_t voltage = 5000, current = 1000;

    index = rqt->request.FIX_BITS.object_posiotion;//ReqPos

    usbpd_printk("rdo=0x%x\n",rqt->request.WORD);
    usbpd_printk("object_posiotion=%d pdo_n=%d\n",index,g_usb_pd_s.src_tx_pdo_n);

    if (!index || index > g_usb_pd_s.src_tx_pdo_n)  return check_index_error;

    struct usb_pd_source_cap_packet_t *pdo = (struct usb_pd_source_cap_packet_t *)g_usb_pd_s.src_source_pdo;

    switch (pdo->source_pdo[index - 1].BITS.FIX_BITS.fixed)
    {
    	case PDO_TYPE_VAR:
    	case PDO_TYPE_BATT:
    		break;
        case PDO_TYPE_FIXED:
        	rdo_op_current = rqt->request.FIX_BITS.op_current * 10;
        	pdo_max_current = pdo->source_pdo[index - 1].BITS.FIX_BITS.max_current * 10;
            if (rdo_op_current > pdo_max_current) return check_current_error;
            voltage = pdo->source_pdo[index - 1].BITS.FIX_BITS.voltage * 50;
            current = pdo_max_current * 11 / 10 + 100;
            g_usb_pd_s.is_in_pps = 0;
            break;
        case PDO_TYPE_APDO:
        	rdo_op_current = rqt->request.PPS_BITS.op_current * 50;
            pdo_max_current = pdo->source_pdo[index - 1].BITS.PPS_BITS.max_current * 50;
            if (rdo_op_current > pdo_max_current ) return check_pps_current_error;
            if(rdo_op_current < 1000) rdo_op_current = 1000;
            voltage = rqt->request.PPS_BITS.output_voltage * 20;
            if ((voltage > pdo->source_pdo[index - 1].BITS.PPS_BITS.max_voltage * 100) || (voltage < pdo->source_pdo[index - 1].BITS.PPS_BITS.min_voltage * 100)) return check_pps_voltage_error;
			current = rdo_op_current + 250;
			g_usb_pd_s.is_in_pps = 1;
            break;
    }
    g_usb_pd_s.supply_current = current;
    g_usb_pd_s.supply_voltage = voltage;
    usbpd_printk("V = %dmV I = %dmA\n",g_usb_pd_s.supply_voltage,g_usb_pd_s.supply_current);
    return 0;
}

static void PE_SRC_Negotiate_Capability_Entry(void)
{
	usbpd_printk("%s\n",__func__);
	usb_pd_set_state(PE_SRC_Negotiate_Capability,exit_state);
}

static void PE_SRC_Negotiate_Capability_Exit(void)
{
	usbpd_printk("%s\n",__func__);
	uint32_t ret = usb_pd_check_request(&g_pd_packet.msg.request);
	usbpd_printk("request pdo =0x%x \n",g_pd_packet.msg.request.request.WORD);
	usbpd_printk("check_ret =%d \n",ret);
	if(ret == 0)//check success
	{
		usb_pd_set_state(PE_SRC_Transition_Supply,enter_state);
	}
	else
	{
		usb_pd_set_state(PE_SRC_Capability_Response,enter_state);
	}
}

static void PE_SRC_Transition_Supply_Entry(void)
{
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
	hal_tcpc_send_ctrl_mgs(PD_CTRL_ACCEPT);
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_ACCEPT;
}

static void PE_SRC_Transition_Supply_Exit(void)
{
	if(hal_tcpc_pd_bus_ready(g_tcpc.tc_port_map))
	{
		//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
		hal_tcpc_send_ctrl_mgs(PD_CTRL_PS_RDY);
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_PSREADY;
	}
}

static void PE_SRC_Ready_Entry(void)
{
	usb_pd_timer_all_reset();
	g_usb_pd_s.explicit_contract = 1;
	usb_pd_timer_start(SourcePPSCommTimer,tSinkPPSPeriodicTime);
	usb_pd_set_state(PE_SRC_Ready,exit_state);
	g_usb_pd_s.pe_timer_cnt = 0;
	//osal_set_event(USB_TASK,TCPM_EVT_PD_READY);
	osal_start_timerEx(TCPM_PSREADY_TIMER, 500, 0, USB_TASK, TCPM_EVT_PD_READY);

	//usbpd_printk("pps cnt =%d \n",usb_pd_timers[SourcePPSCommTimer].timer.time_cnt);
}

static void PE_SRC_Ready_Exit(void)
{
	if(g_usb_pd_s.is_in_pps && usb_pd_timer_is_timeout(SourcePPSCommTimer))
	{
		usbpd_printk("pps cnt =%d \n",usb_pd_timers[SourcePPSCommTimer].timer.time_cnt);
		usbpd_printk("pps timeout  \n");
		usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
	}

	/*
	g_usb_pd_s.pe_timer_cnt++;
	if(g_usb_pd_s.pe_timer_cnt >= 1000)
	{
		usb_pd_set_state(PE_PRS_SRC_SNK_Send_Swap,enter_state);
	}
	*/
}


static void PE_SRC_Disabled_Entry(void)
{
	usb_pd_reset_prl();
	hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP  ,false);
	usb_pd_set_state(PE_SRC_Disabled,exit_state);
}

static void PE_SRC_Disabled_Exit(void)
{

}

static void PE_SRC_Capability_Response_Entry(void)
{
	//usb_pd_event &= ~(usb_pd_EVT_TX_SUCCESSED | usb_pd_EVT_TX_FAIL);
	hal_tcpc_send_ctrl_mgs(PD_CTRL_REJECT);
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REJECT;
}

static void PE_SRC_Capability_Response_Exit(void)
{

}

static void PE_SRC_Hard_Reset_Entry(void)
{
	usbpd_printk("%s\n",__func__);
	hal_tcpc_send_hardreset();
	usb_pd_timer_start(PSHardResetTimer,tPSHardResetTime);
	usb_pd_set_state(PE_SRC_Hard_Reset,exit_state);
}

static void PE_SRC_Hard_Reset_Exit(void)
{
	if(usb_pd_timer_is_timeout(PSHardResetTimer))
	{
		usbpd_printk("%s\n",__func__);
		usb_pd_set_state(PE_SRC_Transition_to_default,enter_state);
	}
}

static void PE_SRC_Hard_Reset_Received_Entry(void)
{
	usbpd_printk("%s\n",__func__);
	usb_pd_timer_start(PSHardResetTimer,tPSHardResetTime);
	usb_pd_set_state(PE_SRC_Hard_Reset,exit_state);
}

static void PE_SRC_Hard_Reset_Received_Exit(void)
{
	if(usb_pd_timer_is_timeout(PSHardResetTimer))
	{
		usbpd_printk("%s\n","PSHardResetTimer Timeout");
		usb_pd_set_state(PE_SRC_Transition_to_default,enter_state);
	}
}

static void PE_SRC_Transition_to_default_Entry(void)
{
	hal_tcpc_set_gate_en(g_tcpc.tc_port_map,false);
	hal_tcpc_port_dummyload_en(g_tcpc.tc_port_map,true);
	hal_tcpc_set_vconn(g_tcpc.tc_port_map,false);
	hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,5000,3000,0,0);
	hal_tcpc_pd_phy_disable();
	hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET,false);
	hal_tcpc_set_roles(g_tcpc.tc_port_map,TYPEC_SOURCE,TYPEC_HOST);
	usb_pd_timer_start(SourceHardResetRecoverTimer,tSourceHardResetRecoverTime);
	usbpd_printk("%s\n",__func__);
	usb_pd_set_state(PE_SRC_Transition_to_default,exit_state);
}

static void PE_SRC_Transition_to_default_Exit(void)
{
	if(usb_pd_timer_is_timeout(SourceHardResetRecoverTimer))
	{
		hal_tcpc_set_gate_en(g_tcpc.tc_port_map,true);
		hal_tcpc_set_vconn(g_tcpc.tc_port_map,true);
		hal_tcpc_reset_pd_phy();
		hal_tcpc_pd_phy_enable();
		hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP | EN_HARD_RESET,true);
		usb_pd_timer_stop(SourceHardResetRecoverTimer);
		usb_pd_set_state(PE_SRC_Startup,enter_state);
		usbpd_printk("%s timeout\n",__func__);
	}
}

static void PE_SRC_Give_Source_Cap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_SOURCECAPS;
	hal_tcpc_send_source_caps(g_usb_pd_s.src_source_pdo,g_usb_pd_s.src_tx_pdo_n);
}

static void PE_SRC_Give_Source_Cap_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
    {
		usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
    }
}


static void PE_SRC_Wait_New_Capabilities_Entry(void)
{

}

static void PE_SRC_Wait_New_Capabilities_Exit(void)
{

}

static void PE_SRC_Soft_Reset_Entry(void)
{
	usb_pd_reset_prl();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_ACCEPT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_ACCEPT);
}

static void PE_SRC_Soft_Reset_Exit(void)
{
	usb_pd_set_state(PE_SRC_Send_Capabilities,enter_state);
}

static void PE_SRC_Send_Soft_Reset_Entry(void)
{
	usb_pd_reset_prl();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_SOFTRESET;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_SOFT_RESET);
}

static void PE_SRC_Send_Soft_Reset_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
	}
}



static void PE_SRC_Not_Supported_Received_Entry(void)
{

}

static void PE_SRC_Not_Supported_Received_Exit(void)
{

}

static void PE_SRC_Send_Not_Supported_Entry(void)
{
	if(g_usb_pd_s.nego_revision == PD_REV30)
	{
		hal_tcpc_send_ctrl_mgs(PD_CTRL_NOT_SUPP);
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_NOTSUPPORT;
	}
	else
	{
		hal_tcpc_send_ctrl_mgs(PD_CTRL_REJECT);
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REJECT;
	}
	usb_pd_set_state(PE_SRC_Send_Not_Supported,exit_state);
}

static void PE_SRC_Send_Not_Supported_Exit(void)
{

}

#endif

static void PE_Give_Revision_Entry(void)
{
	hal_tcpc_pd_send_revision();
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REVISION;
	usb_pd_set_state(PE_Give_Revision,exit_state);
}

static void PE_Give_Revision_Exit(void)
{

}

static void PE_SRC_SNK_Chunk_Received_Entry(void)
{
	usb_pd_timer_start(ChunkingNotSupportedTimer,tChunkingNotSupportedTime);
	usb_pd_set_state(PE_SRC_SNK_Chunk_Received,exit_state);
}

static void PE_SRC_SNK_Chunk_Received_Exit(void)
{
	if(usb_pd_timer_is_timeout(ChunkingNotSupportedTimer))
	{
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		if(g_tcpc.pwr_role == TYPEC_SINK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		else
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
		usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
		usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
	#endif
		usb_pd_timer_stop(ChunkingNotSupportedTimer);
	}
}

static void PE_BIST_Carrier_Mode_Entry(void)
{
    usbpd_printk("Bist Carry!\n");
    usb_pd_timer_start(BISTContModeTimer,tBISTContModeTime);
    hal_tcpc_send_bistdata();
    usb_pd_set_state(PE_BIST_Carrier_Mode,exit_state);
}

static void PE_BIST_Carrier_Mode_Exit(void)
{
	if(usb_pd_timer_is_timeout(BISTContModeTimer))
    {
		usb_pd_timer_stop(BISTContModeTimer);
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Ready,enter_state);
			else
				usb_pd_set_state(PE_SRC_Ready,enter_state);
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Ready,enter_state);
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Ready,enter_state);
	#endif

    }
}

static void PE_BIST_Test_Mode_Entry(void)
{
	usbpd_printk("Bist test data!\n");
    hal_tcpc_set_pd_rx(g_tcpc.tc_port_map,EN_SOP1, false);
    hal_tcpc_set_bist_data(true);
    usb_pd_set_state(PE_BIST_Test_Mode,enter_state);
}

static void PE_BIST_Test_Mode_Exit(void)
{

}

static void PE_Give_Battery_Status_Entry(void)
{
	uint8_t bat_index;
	bat_index = g_pd_packet.msg.ext_msg.data[0];
	tcpc_pd_send_bat_capability(bat_index);
	usb_pd_set_state(PE_Give_Battery_Status,exit_state);
}

static void PE_Give_Battery_Status_Exit(void)
{
	if(g_tcpc.pwr_role == TYPEC_SINK)
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	else
		usb_pd_set_state(PE_SRC_Ready,enter_state);
}

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
static void PE_PRS_SRC_SNK_Evaluate_Swap_Entry(void)
{

	usb_pd_set_state(PE_PRS_SRC_SNK_Evaluate_Swap,exit_state);
}

static void PE_PRS_SRC_SNK_Evaluate_Swap_Exit(void)
{
	usb_pd_set_state(PE_PRS_SRC_SNK_Accept_Swap,enter_state);
}

static void PE_PRS_SRC_SNK_Accept_Swap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_ACCEPT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_ACCEPT);
}

static void PE_PRS_SRC_SNK_Accept_Swap_Exit(void)
{
	usb_pd_set_state(PE_PRS_SRC_SNK_Transition_to_off,enter_state);
	hal_tcpc_set_gate_en(g_tcpc.tc_port_map,false);
}

static void PE_PRS_SRC_SNK_Transition_to_off_Entry(void)
{
	//usb_pd_timer_start(PSSourceOffTimer,tBISTContModeTime);
	g_tc[g_tcpc.tc_port_map].is_in_prswap = 1;
	hal_tcpc_set_gate_en(g_tcpc.tc_port_map,false);
	hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,5000,3000,0,0);
	g_usb_pd_s.pe_timer_cnt = 0;
	usb_pd_set_state(PE_PRS_SRC_SNK_Transition_to_off,exit_state);
}

static void PE_PRS_SRC_SNK_Transition_to_off_Exit(void)
{
	g_usb_pd_s.pe_timer_cnt++;
	if(g_usb_pd_s.pe_timer_cnt >= 50) // wait 50ms for vbus discharge
	{
		usb_pd_set_state(PE_PRS_SRC_SNK_Assert_Rd,enter_state);
	}
}

static void PE_PRS_SRC_SNK_Assert_Rd_Entry(void)
{
	hal_tcpc_set_cc(g_tcpc.tc_port_map,TYPEC_CC_RD);
	hal_tcpc_set_pwr_role(g_tcpc.tc_port_map,TYPEC_SINK);
	g_usb_pd_s.pe_timer_cnt = 0;
	usb_pd_set_state(PE_PRS_SRC_SNK_Assert_Rd,exit_state);
}

static void PE_PRS_SRC_SNK_Assert_Rd_Exit(void)
{
	g_usb_pd_s.pe_timer_cnt++;
	if(g_usb_pd_s.pe_timer_cnt >= 10) // wait 50ms for vbus discharge
	{
		usb_pd_set_state(PE_PRS_SRC_SNK_Wait_Source_on,enter_state);
	}
}

static void PE_PRS_SRC_SNK_Wait_Source_on_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_PSREADY;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_PS_RDY);
}

static void PE_PRS_SRC_SNK_Wait_Source_on_Exit(void)
{
	if(usb_pd_timer_is_timeout(PSSourceOnTimer))
	{
		g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_ErrorRecovery,enter_state);
	}
}


static void PE_PRS_SRC_SNK_Send_Swap_Entry(void)
{
	//
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_PRSWAP;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_PR_SWAP);
}

static void PE_PRS_SRC_SNK_Send_Swap_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SRC_Ready,enter_state);
	}
}

static void PE_PRS_SRC_SNK_Reject_PR_Swap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REJECT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_REJECT);
}

static void PE_PRS_SRC_SNK_Reject_PR_Swap_Exit(void)
{
	usb_pd_set_state(PE_SRC_Ready,enter_state);
}

static void PE_PRS_SNK_SRC_Evaluate_Swap_Entry(void)
{
	usb_pd_set_state(PE_PRS_SNK_SRC_Evaluate_Swap,exit_state);
}

static void PE_PRS_SNK_SRC_Evaluate_Swap_Exit(void)
{
	usb_pd_set_state(PE_PRS_SNK_SRC_Accept_Swap,enter_state);
}

static void PE_PRS_SNK_SRC_Accept_Swap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_ACCEPT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_ACCEPT);
}

static void PE_PRS_SNK_SRC_Accept_Swap_Exit(void)
{
	usb_pd_set_state(PE_PRS_SNK_SRC_Transition_to_off,enter_state);
	hal_tcpc_set_gate_en(g_tcpc.tc_port_map,false);
}

static void PE_PRS_SNK_SRC_Transition_to_off_Entry(void)
{
	usb_pd_timer_start(PSSourceOffTimer,tPSSourceOffTime);
	g_tc[g_tcpc.tc_port_map].is_in_prswap = 1;
	hal_tcpc_set_pwr_role(g_tcpc.tc_port_map,TYPEC_SOURCE);
	usb_pd_set_state(PE_PRS_SNK_SRC_Transition_to_off,exit_state);
}

static void PE_PRS_SNK_SRC_Transition_to_off_Exit(void)
{
	if(usb_pd_timer_is_timeout(PSSourceOffTimer))
	{
		g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_ErrorRecovery,enter_state);
	}
}

static void PE_PRS_SNK_SRC_Assert_Rp_Entry(void)
{
	hal_tcpc_set_cc(g_tcpc.tc_port_map,TYPEC_CC_RP_3_0);
	usb_pd_set_state(PE_PRS_SNK_SRC_Assert_Rp,exit_state);
	hal_tcpc_set_source_mode(BUCKBOOST_DISCHG_MODE);
	hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,5000,3000,0,0);
	g_usb_pd_s.pe_timer_cnt = 0;
}

static void PE_PRS_SNK_SRC_Assert_Rp_Exit(void)
{
	g_usb_pd_s.pe_timer_cnt++;
	if(g_usb_pd_s.pe_timer_cnt >= 20) // wait 10ms for Rp
	{
		usb_pd_set_state(PE_PRS_SNK_SRC_Source_on,enter_state);
	}
}

static void PE_PRS_SNK_SRC_Source_on_Entry(void)
{
	hal_tcpc_set_gate_en(g_tcpc.tc_port_map,true);
	hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,5000,3000,0,0);
	g_usb_pd_s.pe_timer_cnt = 0;
	usb_pd_set_state(PE_PRS_SNK_SRC_Source_on,exit_state);
}

static void PE_PRS_SNK_SRC_Source_on_Exit(void)
{
	g_usb_pd_s.pe_timer_cnt++;
	if(g_usb_pd_s.pe_timer_cnt >= 50) // wait 50ms for Vbus
	{
		g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_PSREADY;
		hal_tcpc_send_ctrl_mgs(PD_CTRL_PS_RDY);
	}
}

static void PE_PRS_SNK_SRC_Reject_Swap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_REJECT;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_REJECT);
}

static void PE_PRS_SNK_SRC_Reject_Swap_Exit(void)
{

}

static void PE_PRS_SNK_SRC_Send_Swap_Entry(void)
{
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_PRSWAP;
	hal_tcpc_send_ctrl_mgs(PD_CTRL_PR_SWAP);
}

static void PE_PRS_SNK_SRC_Send_Swap_Exit(void)
{
	if(usb_pd_timer_is_timeout(SenderResponseTimer))
	{
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	}
}

#endif


void usb_pd_sop_data_msg_handle(void)
{

	switch(g_pd_packet.hdr.BITS.message_type)
	{
		case PD_DATA_SOURCE_CAP:
		#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
			if(g_tcpc.pwr_role != TYPEC_SINK) return;
			g_usb_pd_s.nego_revision = g_pd_packet.hdr.BITS.spec_revision < g_usb_pd_s.nego_revision ? g_pd_packet.hdr.BITS.spec_revision : g_usb_pd_s.nego_revision;
			usb_pd_set_state(PE_SNK_Evaluate_Capability,enter_state);
		#else
			usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif
			break;
		case PD_DATA_REQUEST:
		#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
			if(usb_pd_state == PE_SRC_Send_Capabilities || usb_pd_state == PE_SRC_Ready || usb_pd_state == PE_SRC_Give_Source_Cap)
			{
				g_usb_pd_s.nego_revision = g_pd_packet.hdr.BITS.spec_revision < g_usb_pd_s.nego_revision ? g_pd_packet.hdr.BITS.spec_revision : g_usb_pd_s.nego_revision;
				usb_pd_set_state(PE_SRC_Negotiate_Capability,enter_state);
			}
			else
			{
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				if(g_tcpc.pwr_role == TYPEC_SINK)
				{
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
					softreset_reason = 1;
				}
				else
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
			#endif
			}
		#else
			usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
		#endif
			break;
		case  PD_DATA_BIST:
			switch(g_pd_packet.msg.bist.bist.BITS.bist_mode)
			{
				case BIST_Carrier_Mode:
					usb_pd_set_state(PE_BIST_Carrier_Mode,enter_state);
					break;
				case BIST_Test_Data:
					g_usb_pd_s.in_bist_mode = 1;
					usb_pd_set_state(PE_BIST_Test_Mode,enter_state);
					break;
			}
			break;
		case  PD_DATA_SINK_CAP:
		case  PD_DATA_BAT_STATUS:
		case  PD_DATA_ALERT:
			break;
		case  PD_DATA_EPR_REQUEST:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
			else
				usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
		#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
		#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
		#endif
			break;
		case  PD_DATA_VENDOR_DEF:
			if(g_usb_pd_s.nego_revision == PD_REV30)
			{
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				if(g_tcpc.pwr_role == TYPEC_SINK)
					usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
				else
					usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			#endif
			}
			break;
		case  PD_DATA_SOURCE_INFO:
		case  PD_DATA_REVISION:
		/* 7-14 Reserved */
		case  PD_DATA_GET_COUNTRY_INFO:
		case  PD_DATA_ENTER_USB:
		case  PD_DATA_EPR_MODE:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
			break;
		default:
			break;
	}
}

void usb_pd_sop_ctrl_msg_handle(void)
{
	switch(g_pd_packet.hdr.BITS.message_type)
	{
		case PD_CTRL_GOOD_CRC:
			break;
		case PD_CTRL_GOTO_MIN:
			break;
		case PD_CTRL_ACCEPT:
			switch(usb_pd_state)
			{
			#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				case PE_SNK_Select_Capability:
					usb_pd_set_state(PE_SNK_Transition_Sink,enter_state);
					break;
				case PE_SNK_Send_Soft_Reset:
					usb_pd_set_state(PE_SNK_Wait_for_Capabilities,enter_state);
					break;
			#endif

			#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
				case PE_SRC_Send_Soft_Reset:
					usb_pd_set_state(PE_SRC_Send_Capabilities,enter_state);
					break;
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				case PE_PRS_SRC_SNK_Send_Swap:
					usb_pd_set_state(PE_PRS_SRC_SNK_Transition_to_off,enter_state);
					break;
			#endif
				default:
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
					if(g_tcpc.pwr_role == TYPEC_SINK)
					{
						usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
						softreset_reason = 2;
					}
					else
						usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif

				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif

				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				#endif
					break;
			}
			break;
		case PD_CTRL_REJECT:
			switch(usb_pd_state)
			{
			#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				case PE_SNK_Select_Capability:
					usb_pd_set_state(PE_SNK_Ready,enter_state);
					break;
			#endif
				default:
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
					if(g_tcpc.pwr_role == TYPEC_SINK)
					{
						usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
						softreset_reason = 3;
					}
					else
						usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				#endif
					break;
			}
			break;
		case PD_CTRL_PING:
			break;
		case PD_CTRL_PS_RDY:
			switch(usb_pd_state)
			{
			#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				case PE_SNK_Transition_Sink:
					usb_pd_set_state(PE_SNK_Ready,enter_state);
					break;
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				case PE_PRS_SRC_SNK_Wait_Source_on:
					g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
					usb_pd_set_state(PE_SNK_Startup,enter_state);
					usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_SNK_Attached,exit_state);
					break;

				case PE_PRS_SNK_SRC_Transition_to_off:
					usb_pd_set_state(PE_PRS_SNK_SRC_Assert_Rp,enter_state);
					break;
			#endif
				default:
					#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
//						if(g_tcpc.pwr_role == TYPEC_SINK)
//							usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
//						else
//							usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);

						if(g_tcpc.pwr_role == TYPEC_SINK)
							usb_pd_set_state(PE_SNK_Ready,enter_state);
						else
							usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
					#endif
					#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
						usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
					#endif
					#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
						usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
					#endif
					break;
			}
			break;
		case PD_CTRL_GET_SOURCE_CAP:
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(usb_pd_state == PE_SRC_Ready || usb_pd_state == PE_SNK_Ready)
		#elif(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			if(usb_pd_state == PE_SRC_Ready)
		#endif
			{
				usb_pd_set_state(PE_SRC_Give_Source_Cap,enter_state);
			}
			else
	#else
			if(usb_pd_state == PE_SNK_Ready)
			{
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			}
			else
	#endif

			{
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				if(g_tcpc.pwr_role == TYPEC_SINK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				else
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
			#endif
			}
			break;
		case PD_CTRL_GET_SINK_CAP:
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(usb_pd_state == PE_SRC_Ready || usb_pd_state == PE_SNK_Ready)
		#else
			if(usb_pd_state == PE_SNK_Ready)
		#endif
            {
            	usb_pd_set_state(PE_SNK_Give_Sink_Cap,enter_state);
            }
            else
	#else
    		if(usb_pd_state == PE_SRC_Ready)
			{
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			}
			else
	#endif
            {
			#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
                if(usb_pd_state == PE_SNK_Transition_Sink)
                {
                	usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
                }
                else
			#endif
                {
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
					if(g_tcpc.pwr_role == TYPEC_SINK)
						usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
					else
						usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
				#endif
				#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				#endif
                }
            }
            break;
		case PD_CTRL_PR_SWAP:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(usb_pd_state == PE_SRC_Ready)
				usb_pd_set_state(PE_PRS_SRC_SNK_Evaluate_Swap,enter_state);
			else if(usb_pd_state == PE_SNK_Ready)
				usb_pd_set_state(PE_PRS_SNK_SRC_Evaluate_Swap,enter_state);
			else
			{
				if(g_tcpc.pwr_role == TYPEC_SINK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				else
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			}
		#else
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			#endif
		#endif
			break;
		case PD_CTRL_DR_SWAP:
		case PD_CTRL_VCONN_SWAP:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
			break;
		case PD_CTRL_WAIT:
			break;
		case PD_CTRL_SOFT_RESET:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Soft_Reset,enter_state);
			else
				usb_pd_set_state(PE_SRC_Soft_Reset,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Soft_Reset,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Soft_Reset,enter_state);
		#endif
			break;
		/* 14-15 Reserved */
		case PD_CTRL_DATA_RESET:
		case PD_CTRL_DATA_RET_COMPLETE:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
			break;
		// CONFIG_USB_PD_REV30
		case PD_CTRL_NOT_SUPP:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Ready,enter_state);
			else
				usb_pd_set_state(PE_SRC_Ready,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Ready,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Ready,enter_state);
		#endif
			break;
		case PD_CTRL_GET_REVISION:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(usb_pd_state == PE_SRC_Ready || usb_pd_state == PE_SNK_Ready)
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			if(usb_pd_state == PE_SRC_Ready)
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			if(usb_pd_state == PE_SNK_Ready)
		#endif
			{
				usb_pd_set_state(PE_Give_Revision,enter_state);
			}
			else
			{
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				if(g_tcpc.pwr_role == TYPEC_SINK)
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				else
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#endif
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
			#endif
			}
			break;
		case PD_CTRL_GET_SINK_CAP_EXT:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#else
			usb_pd_set_state(PE_SNK_Give_Sink_Cap_Ext,enter_state);
		#endif
			break;
		case PD_CTRL_GET_PPS_STATUS:
			if(g_tcpc.pwr_role == TYPEC_SOURCE) usb_pd_set_state(PE_SRC_Give_PPS_Status,enter_state);
			break;
		case PD_CTRL_GET_SOURCE_CAP_EXT:
		case PD_CTRL_GET_STATUS:
		case PD_CTRL_FR_SWAP:
		case PD_CTRL_GET_COUNTRY_CODES:
		case PD_CTRL_GET_SOURCE_INFO:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
			break;
		default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
			break;
	}
}

void usb_pd_sop_ext_msg_handle(void)
{

    if (!g_pd_packet.msg.ext_msg.ext_hrd.BITS.chunked)//Chunked=0
    {
    	usb_pd_set_state(PE_SRC_SNK_Chunk_Received,enter_state); //unchunk not support
        return;
    }

    if ((g_pd_packet.msg.ext_msg.ext_hrd.BITS.data_size > 26))
    {
    	usb_pd_set_state(PE_SRC_SNK_Chunk_Received,enter_state);// unchunk not support
        return;
    }

    switch (g_pd_packet.hdr.BITS.message_type)
    {
		case PD_EXT_GET_BATT_CAP:
			usb_pd_set_state(PE_Give_Battery_Status,enter_state);
			break;
        case PD_EXT_EPR_SOURCE_CAPABILITIES:
        case PD_EXT_EXTENDED_CTRL:
        case PD_EXT_STATUS://2
        case PD_EXT_PPS_STATUS://12
        case PD_EXT_SOURCE_CAP_EXT:
        case PD_EXT_GET_BATT_STATUS:
        case PD_EXT_BATT_CAP:
        case PD_EXT_GET_MANUFACTURER_INFO://6
        case PD_EXT_MANUFACTURER_INFO://7
        case PD_EXT_SECURITY_REQUEST:
        case PD_EXT_SECURITY_RESPONSE:
        case PD_EXT_FW_UPDATE_REQUEST:
        case PD_EXT_FW_UPDATE_RESPONSE:
        case PD_EXT_COUNTRY_INFO:
        case PD_EXT_COUNTRY_CODES:
        default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
			else
				usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Not_Supported,enter_state);
		#endif
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Not_Supported,enter_state);
		#endif
            break;
    }
}

void usb_pd_sop_msg_handle(void)
{
	//usbpd_printk("%s\n",__func__);
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if (g_tc[g_tcpc.tc_port_map].usb_tc_state == TC_SNK_Attached || g_tc[g_tcpc.tc_port_map].usb_tc_state == TC_SRC_Attached)
#endif
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	if (g_tc[g_tcpc.tc_port_map].usb_tc_state == TC_SRC_Attached)
#endif
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	if (g_tc[g_tcpc.tc_port_map].usb_tc_state == TC_SNK_Attached)
#endif

    {
        if(g_pd_packet.hdr.BITS.message_id ==  g_usb_pd_s.rx_sop_msgid && g_pd_packet.hdr.BITS.message_type != PD_CTRL_SOFT_RESET) return;
        g_usb_pd_s.rx_sop_msgid = g_pd_packet.hdr.BITS.message_id;

		if (g_pd_packet.hdr.BITS.externed)
		{
			usb_pd_sop_ext_msg_handle();
			//usbpd_printk("ext msg\n");
		}
		else if (g_pd_packet.hdr.BITS.n_data_object)
		{
			//usbpd_printk("data msg\n");
			usb_pd_sop_data_msg_handle();
		}
		else
		{
			//usbpd_printk("ctrl msg\n");
			usb_pd_sop_ctrl_msg_handle();
		}
    }

}

void usb_pd_timer_update(void)
{
	for(uint8_t i = 0; i< USBPD_TIMER_MAX;i++)
	{
		if(usb_pd_timers[i].timer.state == TIMER_STOP) continue;
		if(usb_pd_timers[i].timer.time_cnt) usb_pd_timers[i].timer.time_cnt--;
		if(usb_pd_timers[i].timer.time_cnt == 0) usb_pd_timers[i].timer.timeout = true;
	}
}

void usb_pdevt_run(void)
{
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
	if(usb_pd_event & USB_PD_EVT_SNK_ATTACHED)
	{
		usb_pd_set_state(PE_SNK_Startup,enter_state);
		usbpd_printk("usb_pd_EVT_SNK_ATTACHED\n");
		g_usb_pd_s.pe_prl_busy = 0;
		need_rechager = 0;
		usb_pd_event = 0;
	}
	if(usb_pd_event & USB_PD_EVT_SNK_UNATTACH)
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usbpd_printk("usb_pd_EVT_SNK_UNATTACH\n");
		g_usb_pd_s.pe_prl_busy = 0;
		usb_pd_event = 0;
		need_rechager = 0;
	}
#endif

#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
	if(usb_pd_event & USB_PD_EVT_SRC_ATTACHED)
	{
		usb_pd_set_state(PE_SRC_Startup,enter_state);
		usbpd_printk("usb_pd_EVT_SRC_ATTACHED\n");
		g_usb_pd_s.pe_prl_busy = 0;
		usb_pd_event = 0;
		need_rechager = 0;
	}
	if(usb_pd_event & USB_PD_EVT_SRC_UNATTACH)
	{
		usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		usbpd_printk("usb_pd_EVT_SRC_UNATTACH\n");
		g_usb_pd_s.pe_prl_busy = 0;
		usb_pd_event = 0;
		need_rechager = 0;
	}
#endif

	if(usb_pd_event & USB_PD_EVT_RX_HARDRESET)
	{
		usbpd_printk("usb_pd_EVT_RX_HARDRESET\n");
		g_usb_pd_s.pe_prl_busy = 0;
		usb_pd_event &= ~USB_PD_EVT_RX_HARDRESET;
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		if(g_tcpc.pwr_role == TYPEC_SINK)
			usb_pd_set_state(PE_SNK_Transition_to_default,enter_state);
		else
			usb_pd_set_state(PE_SRC_Hard_Reset_Received,enter_state);
	#endif
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
		usb_pd_set_state(PE_SRC_Hard_Reset_Received,enter_state);
	#endif
	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
		usb_pd_set_state(PE_SNK_Transition_to_default,enter_state);
	#endif

	}
	else if(usb_pd_event & USB_PD_EVT_PS_TRANST)
	{
		#define abs(a,b) a>b?a-b:b-a
		usb_pd_event &= ~USB_PD_EVT_PS_TRANST;
		if(g_usb_pd_s.is_in_pps && (abs(g_usb_pd_s.supply_voltage,g_buckboost.buckboost_out_voltage)<= 500))
			hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,g_usb_pd_s.supply_voltage ,g_usb_pd_s.supply_current ,0,24);
		else
			hal_tcpc_pd_set_bus_iv(g_tcpc.tc_port_map,g_usb_pd_s.supply_voltage ,g_usb_pd_s.supply_current ,30,180);
	}
	else if(usb_pd_event & USB_PD_EVT_RX_SOP_PACKET)
	{

		usb_pd_event &= ~USB_PD_EVT_RX_SOP_PACKET;
		if(g_usb_pd_s.in_bist_mode == 1) return;
		usbpd_printk("header= 0x%x \n", g_pd_packet.hdr.WORD);
		usbpd_printk("msg_type = %d \n ", g_pd_packet.hdr.BITS.message_type);

		for(uint8_t i = 0; i < g_pd_packet.msg_len; i++)
		{
			usbpd_printk("0x%x ", pd_rx_buff[i]);
		}
		usbpd_printk("\n");

		if(g_usb_pd_s.pe_prl_busy) return;
		usbpd_printk("msg_len = %d \n ", g_pd_packet.msg_len);
		usb_pd_sop_msg_handle();
	}
	else if(usb_pd_event & USB_PD_EVT_SNK_SET_VOLTAGE)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		if(usb_pd_state == PE_SNK_Ready)
		{
			usb_pd_set_state(PE_SNK_Select_Capability,enter_state);
			usb_pd_event &= ~USB_PD_EVT_SNK_SET_VOLTAGE;
		}
	#endif
	}
	else if(usb_pd_event & USB_PD_EVT_SOURCE_SOFTRESET)
	{
		if(usb_pd_state == PE_SRC_Ready)
		{
			usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			usb_pd_event &= ~USB_PD_EVT_SOURCE_SOFTRESET;
		}
	}
}

void transmit_timeout_cb(void)
{

	g_usb_pd_s.tx_sop_msgid++;
	g_usb_pd_s.tx_sop_msgid = g_usb_pd_s.tx_sop_msgid & 0x07;

	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		case PE_SNK_Send_Soft_Reset:
			usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		case PE_SRC_Send_Soft_Reset:
			usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		case PE_PRS_SRC_SNK_Wait_Source_on:
		case PE_PRS_SNK_SRC_Source_on:
			g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_ErrorRecovery,enter_state);
			break;
	#endif
		default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
			{
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				softreset_reason = 5;
			}
			else
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
		#endif
			break;
	}
	g_usb_pd_s.pe_prl_busy = 0;
}


void usb_pd_run(void)
{

	static uint8_t usb_pd_state_last = 0;
	static uint8_t usb_pd_substate_last = 0;
	static uint8_t prl_busy_cnt = 0;

	if(usb_pd_disable)
	{
		if(usb_pd_state != PE_SNK_RSC_Disable)
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
		return;
	}
	
	if(g_usb_pd_s.pe_prl_busy)
	{
		prl_busy_cnt++;
		if(prl_busy_cnt >= 30)
		{
			prl_busy_cnt = 0;
			//transmit_timeout_cb();
			usbpd_printk("tx timeout !\n");
		}
		return;
	}
	else
	{
		prl_busy_cnt = 0;
	}




	//usbpd_printk("tx_discard_cnt = %d \n ", tx_discard_cnt);

	if(g_usb_pd_s.pe_prl_busy) return;

	uint8_t pd_prv_state;
	uint8_t pd_prv_substate;
	do
	{
		if(usb_pd_state_last != usb_pd_state || usb_pd_substate != usb_pd_substate_last) usbpd_printk("pe_state = %d %d\n", usb_pd_state,usb_pd_substate);
		usb_pd_state_last = usb_pd_state;
		usb_pd_substate_last = usb_pd_substate;

		pd_prv_state = usb_pd_state;
		pd_prv_substate = usb_pd_substate;
		if(usb_pd_substate == enter_state)
		{
			//if(usb_pd_tasks_table[usb_pd_state].enter_cb != NULL)
				usb_pd_tasks_table[usb_pd_state].enter_cb();
		}
		else
		{
			//if(usb_pd_tasks_table[usb_pd_state].exit_cb != NULL)
				usb_pd_tasks_table[usb_pd_state].exit_cb();
		}

	} while(usb_pd_state != pd_prv_state ||  usb_pd_substate != pd_prv_substate);
}

typedef void (*callback)(void);

void usb_pd_pkts_transmit_request_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
	usb_pd_timer_start(SenderResponseTimer,tSenderResponseTime);
	usb_pd_set_state(PE_SNK_Select_Capability,exit_state);
#endif
}

void usb_pd_pkts_transmit_accept_callback(void)
{
	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		case PE_SNK_Soft_Reset:
			usb_pd_set_state(PE_SNK_Soft_Reset,exit_state);
			break;
	#endif
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		case PE_SRC_Transition_Supply:
			g_buckboost.regulator_state = 0;
			usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_PS_TRANST);
			usb_pd_set_state(PE_SRC_Transition_Supply,exit_state);
			break;
		case PE_SRC_Soft_Reset:
			usb_pd_set_state(PE_SRC_Soft_Reset,exit_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		case PE_PRS_SRC_SNK_Accept_Swap:
			usb_pd_set_state(PE_PRS_SRC_SNK_Accept_Swap,exit_state);
			break;
		case PE_PRS_SNK_SRC_Accept_Swap:
			usb_pd_set_state(PE_PRS_SNK_SRC_Accept_Swap,exit_state);
			break;
	#endif
		default:
			break;
	}
}

void usb_pd_pkts_transmit_softreset_callback(void)
{
	usb_pd_timer_start(SenderResponseTimer,tSenderResponseTime);

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(g_tcpc.pwr_role == TYPEC_SINK)
	{
		usb_pd_set_state(PE_SNK_Send_Soft_Reset,exit_state);
		softreset_reason = 5;
	}
	else
		usb_pd_set_state(PE_SRC_Send_Soft_Reset,exit_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_pd_set_state(PE_SRC_Send_Soft_Reset,exit_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	usb_pd_set_state(PE_SNK_Send_Soft_Reset,exit_state);
#endif
}

void usb_pd_pkts_transmit_notsupport_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(g_tcpc.pwr_role == TYPEC_SINK)
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	else
		usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	usb_pd_set_state(PE_SNK_Ready,enter_state);
#endif
}

void usb_pd_pkts_transmit_sourcecap_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
	usb_pd_timer_start(SenderResponseTimer,tSenderResponseTime);
	g_usb_pd_s.hardreset_counter = 0;
	g_usb_pd_s.caps_counter = 0;
	if(usb_pd_state == PE_SRC_Give_Source_Cap)
	{

		if(g_tcpc.pwr_role == TYPEC_SOURCE)
			usb_pd_set_state(PE_SRC_Give_Source_Cap,exit_state);
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		else
			usb_pd_set_state(PE_SNK_Ready,enter_state);
	#endif
	}
	else
		usb_pd_set_state(PE_SRC_Send_Capabilities,exit_state);
#endif
}

void usb_pd_pkts_transmit_psready_callback(void)
{
	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		case PE_SRC_Transition_Supply:
			usb_pd_set_state(PE_SRC_Ready,enter_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		case PE_PRS_SRC_SNK_Wait_Source_on:
			usb_pd_timer_start(PSSourceOnTimer,tPSSourceOnTime);
			usb_pd_set_state(PE_PRS_SRC_SNK_Wait_Source_on,exit_state);
			break;
		case PE_PRS_SNK_SRC_Source_on:
			g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
			usb_pd_set_state(PE_SRC_Startup,enter_state);
			break;
	#endif
		default:
			break;
	}
}

void usb_pd_pkts_transmit_reject_callback(void)
{
	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		case PE_SRC_Capability_Response:
			if(g_usb_pd_s.explicit_contract)
				usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
			else
				usb_pd_set_state(PE_SRC_Wait_New_Capabilities,enter_state);
			break;
	#endif
		default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
				usb_pd_set_state(PE_SNK_Ready,enter_state);
			else
				usb_pd_set_state(PE_SRC_Ready,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Ready,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Ready,enter_state);
		#endif
			break;
	}
}

void usb_pd_pkts_transmit_revision_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(g_tcpc.pwr_role == TYPEC_SINK)
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	else
		usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	usb_pd_set_state(PE_SNK_Ready,enter_state);
#endif
}

void usb_pd_pkts_transmit_givesnkcap_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(g_tcpc.pwr_role == TYPEC_SINK)
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	else
		usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	usb_pd_set_state(PE_SNK_Ready,enter_state);
#endif
}

void usb_pd_pkts_transmit_givesnkcap_ext_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(g_tcpc.pwr_role == TYPEC_SINK)
		usb_pd_set_state(PE_SNK_Ready,enter_state);
	else
		usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
	usb_pd_set_state(PE_SRC_Ready,enter_state);
#endif

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
	usb_pd_set_state(PE_SNK_Ready,enter_state);
#endif
}

void usb_pd_pkts_transmit_givepps_sta_callback(void)
{
	usb_pd_set_state(PE_SRC_Ready,enter_state);
}

void usb_pd_pkts_transmit_prswap_callback(void)
{
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	if(usb_pd_state == PE_PRS_SRC_SNK_Send_Swap)
	{
		usb_pd_timer_start(SenderResponseTimer,tSenderResponseTime);
		usb_pd_set_state(PE_PRS_SRC_SNK_Send_Swap,exit_state);
	}
	if(usb_pd_state == PE_PRS_SNK_SRC_Send_Swap)
	{
		usb_pd_timer_start(SenderResponseTimer,tSenderResponseTime);
		usb_pd_set_state(PE_PRS_SNK_SRC_Send_Swap,exit_state);
	}
#endif
}



void transmit_fail_cb(void)
{
	if(TCPC->TXD_CTRL.BITS.TXD_SOP_TYP > Transmit_SOP1)
	{
		g_usb_pd_s.pe_prl_busy = 0;
		return;
	}

	if(tcpc_transmit_retry_cnt)
	{
		tcpc_transmit_retry_cnt--;
		hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
		return;
	}

	g_usb_pd_s.tx_sop_msgid++;
	g_usb_pd_s.tx_sop_msgid = g_usb_pd_s.tx_sop_msgid & 0x07;
	g_usb_pd_s.pe_prl_busy = 0;
	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		case PE_SNK_Send_Soft_Reset:
			usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
			break;
	#endif
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
		case PE_SRC_Send_Capabilities:
			if(g_usb_pd_s.explicit_contract == 0)
			{
				usb_pd_set_state(PE_SRC_Discovery,enter_state);
			}
			else
			{
			#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
				if(g_tcpc.pwr_role == TYPEC_SINK)
				{
					usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
					softreset_reason = 6;
				}
				else
					usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
			#elif(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
			#endif
			}
			break;
		case PE_SRC_Send_Soft_Reset:
			usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		case PE_PRS_SRC_SNK_Wait_Source_on:
		case PE_PRS_SNK_SRC_Source_on:
			g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_ErrorRecovery,enter_state);
			break;
	#endif
		default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
			{
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				softreset_reason = 7;
			}
			else
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
		#endif
			break;
	}

}

void transmit_discard_cb(void)
{
	if(TCPC->TXD_CTRL.BITS.TXD_SOP_TYP > Transmit_SOP1)
	{
		g_usb_pd_s.pe_prl_busy = 0;
		return;
	}
	g_usb_pd_s.tx_sop_msgid++;
	g_usb_pd_s.tx_sop_msgid = g_usb_pd_s.tx_sop_msgid & 0x07;

	switch(usb_pd_state)
	{
	#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
		case PE_SNK_Send_Soft_Reset:
			usb_pd_set_state(PE_SNK_Hard_Reset,enter_state);
			break;
	#endif

	#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
		case PE_SRC_Send_Soft_Reset:
			usb_pd_set_state(PE_SRC_Hard_Reset,enter_state);
			break;
		case PE_PRS_SRC_SNK_Wait_Source_on:
		case PE_PRS_SNK_SRC_Source_on:
			g_tc[g_tcpc.tc_port_map].is_in_prswap = false;
			usb_pd_set_state(PE_SNK_RSC_Disable,enter_state);
			usb_tc_set_state(&g_tc[g_tcpc.tc_port_map],TC_ErrorRecovery,enter_state);
			break;
	#endif
		default:
		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
			if(g_tcpc.pwr_role == TYPEC_SINK)
			{
				usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
				softreset_reason = 8;
			}
			else
				usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SRC)
			usb_pd_set_state(PE_SRC_Send_Soft_Reset,enter_state);
		#endif

		#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_SNK)
			usb_pd_set_state(PE_SNK_Send_Soft_Reset,enter_state);
		#endif
			break;
	}
	g_usb_pd_s.pe_prl_busy = 0;
}


void usb_pd_pkts_transmit_hardreset_callback(void)
{

}

void usb_pd_pkts_transmit_bistcarrymode_callback(void)
{

}


const static callback transmit_success_cb[TRANSMITE_TYPE_MAX]  =
{
	usb_pd_pkts_transmit_hardreset_callback,		//TRANSMITE_TYPE_HARDRESER,
	usb_pd_pkts_transmit_bistcarrymode_callback,  	//TRANSMITE_TYPE_BISTCARRYMODE,
	usb_pd_pkts_transmit_request_callback,			//TRANSMITE_TYPE_REQUEST
	usb_pd_pkts_transmit_accept_callback,			//TRANSMITE_TYPE_ACCEPT
	usb_pd_pkts_transmit_softreset_callback,		//TRANSMITE_TYPE_SOFTRESET
	usb_pd_pkts_transmit_notsupport_callback,		//TRANSMITE_TYPE_NOTSUPPORT
	usb_pd_pkts_transmit_sourcecap_callback,		//TRANSMITE_TYPE_SOURCECAPS
	usb_pd_pkts_transmit_psready_callback,			//TRANSMITE_TYPE_PSREADY
	usb_pd_pkts_transmit_reject_callback,			//TRANSMITE_TYPE_REJECT
	usb_pd_pkts_transmit_revision_callback, 		//TRANSMITE_TYPE_REVISION
	usb_pd_pkts_transmit_prswap_callback,			//TRANSMITE_TYPE_PRSWAP
	usb_pd_pkts_transmit_givesnkcap_callback,		//TRANSMITE_TYPE_GIVESNKCAP
	usb_pd_pkts_transmit_givesnkcap_ext_callback,   //TRANSMITE_TYPE_GIVESNKCAP_EXT
	usb_pd_pkts_transmit_givepps_sta_callback,		//TRANSMITE_TYPE_GIVEPPS_STA
};

void __attribute__((isr)) USBPD_IRQHandler(void)
{
    uint32_t int_ctrl = TCPC->INT_CTRL.WORD;
    uint32_t int_flag = TCPC->INT_FLAG.WORD;

    do
    {
		if(int_flag & (0x01<<6))
		{
			extern uint8_t tcpc_transmit_byte_index;
			tcpc_transmit_byte_index++;
			if(tcpc_transmit_byte_index >= 6) tcpc_transmit_byte_index = 6;
			TCPC->TXD_BUFF.WORD =transmit_pkt.msg.WORDS[tcpc_transmit_byte_index];
			TCPC->INT_FLAG.BITS.PHY_TX_BUFF_EMPTY_FLAG = 0x01;
		}

		if(int_flag & (0x01<<5))
		{
			pd_rx_buff[rx_cnt&0x0F] = TCPC->RXD_BUFF.WORD;
			rx_cnt++;
			TCPC->INT_FLAG.BITS.PHY_RX_BUFF_UPDAT_FLAG = 0x01;
		}

		if(int_flag & (0x01<<2))
		{
			rx_cnt = 0;
			TCPC->INT_FLAG.BITS.PHY_RX_DATA_ERROR_FLAG = 0x01;
		}

		//if(TCPC->INT_FLAG.WORD & (0x01 << 10))
		if(int_flag & (0x01<<10))
		{
			usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_RX_SOP_PACKET);
			g_pd_packet.hdr.WORD = TCPC->RXD_INFO.WORD;
			for(uint8_t i = 0; i < 7; i++)
				g_pd_packet.msg.WORDS[i] = pd_rx_buff[i];
			g_pd_packet.msg_len = rx_cnt;
			g_pd_packet.sop_type = TCPC->RXD_INFO.BITS.RXD_SOP_TYP;
			rx_cnt = 0;
			TCPC->INT_FLAG.BITS.PHY_RX_SUCCESSFUL_FLAG = 0x01;
		}

		if(int_flag & (0x01<<11))
		{
			usb_pd_set_event(g_tcpc.tc_port_map,USB_PD_EVT_RX_HARDRESET);
			TCPC->INT_FLAG.BITS.PHY_RX_HARD_RESET_FLAG = 0x01;
		}

		if(int_flag & (0x01<<12))
		{
			TCPC->INT_FLAG.BITS.PHY_TX_NO_GOODCRC_FLAG = 0x01;
			transmit_fail_cb();
		}

		if(int_flag & (0x01<<13))
		{
			TCPC->INT_FLAG.BITS.PHY_TX_CC_DISCARD_FLAG = 0x01;
			transmit_discard_cb();
		}

		if(int_flag & (0x01<<14))
		{
			transmit_success_cb[g_usb_pd_s.pe_tran_cb_type]();
			g_usb_pd_s.pe_prl_busy = 0;
			TCPC->INT_FLAG.BITS.PHY_TX_SUCCESSFUL_FLAG = 0x01;
			g_usb_pd_s.tx_sop_msgid++;
			g_usb_pd_s.tx_sop_msgid = g_usb_pd_s.tx_sop_msgid & 0x07;
		}

		int_flag = TCPC->INT_FLAG.WORD;

    } while(int_flag & int_ctrl);

}

//this table must map to usb_pd_enum one by one
const struct usb_pd_state_task_t usb_pd_tasks_table[PE_STATE_MAX]  =
{
	//for snk

	{PE_SNK_RSC_Disable_Entry,PE_SNK_RSC_Disable_Exit},									//PE_SNK_RSC_Disable
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
	{PE_SNK_Startup_Entry,PE_SNK_Startup_Exit},											//PE_SNK_Startup,
	{PE_SNK_Discovery_Entry,PE_SNK_Discovery_Exit},										//PE_SNK_Discovery,
	{PE_SNK_Wait_for_Capabilities_Entry,PE_SNK_Wait_for_Capabilities_Exit},				//PE_SNK_Wait_for_Capabilities,
	{PE_SNK_Evaluate_Capability_Entry,PE_SNK_Evaluate_Capability_Exit},					//PE_SNK_Evaluate_Capability,
	{PE_SNK_Select_Capability_Entry,PE_SNK_Select_Capability_Exit},						//PE_SNK_Select_Capability,
	{PE_SNK_Transition_Sink_Entry,PE_SNK_Transition_Sink_Exit},							//PE_SNK_Transition_Sink,
	{PE_SNK_Ready_Entry,PE_SNK_Ready_Exit},												//PE_SNK_Ready,
	{PE_SNK_Hard_Reset_Entry,PE_SNK_Hard_Reset_Exit},									//PE_SNK_Hard_Reset,
	{PE_SNK_Transition_to_default_Entry,PE_SNK_Transition_to_default_Exit},				//PE_SNK_Transition_to_default,
	{PE_SNK_Give_Sink_Cap_Entry,PE_SNK_Give_Sink_Cap_Exit},								//PE_SNK_Give_Sink_Cap,
	{PE_SNK_Send_Soft_Reset_Entry,PE_SNK_Send_Soft_Reset_Exit},							//PE_SNK_Send_Soft_Reset,
	{PE_SNK_Soft_Reset_Entry,PE_SNK_Soft_Reset_Exit},									//PE_SNK_Soft_Reset,
	{PE_SNK_Not_Supported_Received_Entry,PE_SNK_Not_Supported_Received_Exit},			//PE_SNK_Not_Supported_Received,
	{PE_SNK_Send_Not_Supported_Entry,PE_SNK_Send_Not_Supported_Exit},					//PE_SNK_Send_Not_Supported,
	{PE_SNK_Give_Sink_Cap_Ext_Entry,PE_SNK_Give_Sink_Cap_Ext_Exit},						//PE_SNK_Give_Sink_Cap_Ext

#endif

	//for source
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
	{PE_SRC_Startup_Entry,PE_SRC_Startup_Exit},											//PE_SRC_Startup,
	{PE_SRC_Discovery_Entry,PE_SRC_Discovery_Exit},										//PE_SRC_Discovery,
	{PE_SRC_Send_Capabilities_Entry,PE_SRC_Send_Capabilities_Exit},						//PE_SRC_Send_Capabilities,
	{PE_SRC_Negotiate_Capability_Entry,PE_SRC_Negotiate_Capability_Exit},				//PE_SRC_Negotiate_Capability,
	{PE_SRC_Transition_Supply_Entry,PE_SRC_Transition_Supply_Exit},						//PE_SRC_Transition_Supply,
	{PE_SRC_Ready_Entry,PE_SRC_Ready_Exit},												//PE_SRC_Ready,
	{PE_SRC_Disabled_Entry,PE_SRC_Disabled_Exit},										//PE_SRC_Disabled,
	{PE_SRC_Capability_Response_Entry,PE_SRC_Capability_Response_Exit},					//PE_SRC_Capability_Response,
	{PE_SRC_Hard_Reset_Entry,PE_SRC_Hard_Reset_Exit},									//PE_SRC_Hard_Reset,
	{PE_SRC_Hard_Reset_Received_Entry,PE_SRC_Hard_Reset_Received_Exit},					//PE_SRC_Hard_Reset_Received,
	{PE_SRC_Transition_to_default_Entry,PE_SRC_Transition_to_default_Exit},				//PE_SRC_Transition_to_default,
	{PE_SRC_Give_Source_Cap_Entry,PE_SRC_Give_Source_Cap_Exit},							//PE_SRC_Give_Source_Cap,
	{PE_SRC_Wait_New_Capabilities_Entry,PE_SRC_Wait_New_Capabilities_Exit},				//PE_SRC_Wait_New_Capabilities,
	{PE_SRC_Send_Soft_Reset_Entry,PE_SRC_Send_Soft_Reset_Exit},							//PE_SRC_Send_Soft_Reset,//28
	{PE_SRC_Soft_Reset_Entry,PE_SRC_Soft_Reset_Exit},									//PE_SRC_Soft_Reset,
	{PE_SRC_Not_Supported_Received_Entry,PE_SRC_Not_Supported_Received_Exit},			//PE_SRC_Not_Supported_Received,
	{PE_SRC_Send_Not_Supported_Entry,PE_SRC_Send_Not_Supported_Exit},					//PE_SRC_Send_Not_Supported,
	{PE_SRC_Give_PPS_Status_Entry,PE_SRC_Give_PPS_Status_Exit},							//PE_SRC_Give_PPS_Status
#endif
	{PE_Give_Revision_Entry,PE_Give_Revision_Exit},										//PE_Get_Revision,
	{PE_SRC_SNK_Chunk_Received_Entry,PE_SRC_SNK_Chunk_Received_Exit},					//PE_SRC_SNK_Chunk_Received
	{PE_BIST_Carrier_Mode_Entry,PE_BIST_Carrier_Mode_Exit},								//PE_BIST_Carrier_Mode,
	{PE_BIST_Test_Mode_Entry,PE_BIST_Test_Mode_Exit},									//PE_BIST_Test_Mode,
	{PE_Give_Battery_Status_Entry,PE_Give_Battery_Status_Exit},							//PE_Give_Battery_Status
	//for drp
#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	{PE_PRS_SRC_SNK_Evaluate_Swap_Entry,PE_PRS_SRC_SNK_Evaluate_Swap_Exit},				//PE_PRS_SRC_SNK_Evaluate_Swap,
	{PE_PRS_SRC_SNK_Accept_Swap_Entry,PE_PRS_SRC_SNK_Accept_Swap_Exit},					//PE_PRS_SRC_SNK_Accept_Swap,
	{PE_PRS_SRC_SNK_Transition_to_off_Entry,PE_PRS_SRC_SNK_Transition_to_off_Exit},		//PE_PRS_SRC_SNK_Transition_to_off,
	{PE_PRS_SRC_SNK_Assert_Rd_Entry,PE_PRS_SRC_SNK_Assert_Rd_Exit},						//PE_PRS_SRC_SNK_Assert_Rd,
	{PE_PRS_SRC_SNK_Wait_Source_on_Entry,PE_PRS_SRC_SNK_Wait_Source_on_Exit},			//PE_PRS_SRC_SNK_Wait_Source_on,
	{PE_PRS_SRC_SNK_Send_Swap_Entry,PE_PRS_SRC_SNK_Send_Swap_Exit},						//PE_PRS_SRC_SNK_Send_Swap,
	{PE_PRS_SRC_SNK_Reject_PR_Swap_Entry,PE_PRS_SRC_SNK_Reject_PR_Swap_Exit},			//PE_PRS_SRC_SNK_Reject_PR_Swap,
	{PE_PRS_SNK_SRC_Evaluate_Swap_Entry,PE_PRS_SNK_SRC_Evaluate_Swap_Exit},				//PE_PRS_SNK_SRC_Evaluate_Swap,
	{PE_PRS_SNK_SRC_Accept_Swap_Entry,PE_PRS_SNK_SRC_Accept_Swap_Exit},					//PE_PRS_SNK_SRC_Accept_Swap,
	{PE_PRS_SNK_SRC_Transition_to_off_Entry,PE_PRS_SNK_SRC_Transition_to_off_Exit},		//PE_PRS_SNK_SRC_Transition_to_off,
	{PE_PRS_SNK_SRC_Assert_Rp_Entry,PE_PRS_SNK_SRC_Assert_Rp_Exit},						//PE_PRS_SNK_SRC_Assert_Rp,
	{PE_PRS_SNK_SRC_Source_on_Entry,PE_PRS_SNK_SRC_Source_on_Exit},						//PE_PRS_SNK_SRC_Source_on,
	{PE_PRS_SNK_SRC_Reject_Swap_Entry,PE_PRS_SNK_SRC_Reject_Swap_Exit},					//PE_PRS_SNK_SRC_Reject_Swap,
	{PE_PRS_SNK_SRC_Send_Swap_Entry,PE_PRS_SNK_SRC_Send_Swap_Exit},						//PE_PRS_SNK_SRC_Send_Swap,
#endif
	//other
};

