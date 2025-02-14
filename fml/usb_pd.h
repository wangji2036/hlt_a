#ifndef USB_PD_H_
#define USB_PD_H_

#include "typdef.h"
#include "usbpd_config.h"

#define USB_PD_EVT_SNK_ATTACHED  					(0x01ul<<0)
#define USB_PD_EVT_SNK_UNATTACH  					(0x01ul<<1)
#define USB_PD_EVT_SRC_ATTACHED  					(0x01ul<<2)
#define USB_PD_EVT_SRC_UNATTACH  					(0x01ul<<3)
#define USB_PD_EVT_RX_SOP_PACKET 					(0x01ul<<4)
#define USB_PD_EVT_RX_HARDRESET  					(0x01ul<<5)
#define USB_PD_EVT_TX_SUCCESSED  					(0x01ul<<6)
#define USB_PD_EVT_TX_FAIL  						(0x01ul<<7)  //include tx_discard tx_fald tx_timeout
#define USB_PD_EVT_SNK_SET_VOLTAGE  				(0x01ul<<8)  //include tx_discard tx_fald tx_timeout
#define USB_PD_EVT_PS_TRANST 						(0x01ul<<9)

struct usb_pd_state_task_t
{
	void (*enter_cb)(void);
	void (*exit_cb)(void);
};

#define N_HARDRESET_COUNTER						50
#define N_CAPS_COUNT							50

#define tSinkWaitCapTime						2380
#define tChunkingNotSupportedTime				40
#define tSenderResponseTime 					27
#define tPSTransitionTime						500
#define tSinkPPSPeriodicTime					14000
#define tSourceCapabilityTime					150
#define tSourceHardResetRecoverTime            	1000
#define tPSHardResetTime						28
#define tBISTContModeTime						60
#define tPSSourceOffTime						835
#define tPSSourceOnTime							435


enum usb_pd_state_e
{
	PE_SNK_RSC_Disable = 0,
// snk port
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SNK)
	PE_SNK_Startup,
	PE_SNK_Discovery,
	PE_SNK_Wait_for_Capabilities,
	PE_SNK_Evaluate_Capability,
	PE_SNK_Select_Capability,
	PE_SNK_Transition_Sink,
	PE_SNK_Ready,//7
	PE_SNK_Hard_Reset, //8
	PE_SNK_Transition_to_default,
	PE_SNK_Give_Sink_Cap,
	PE_SNK_Send_Soft_Reset,
	PE_SNK_Soft_Reset,//12
	PE_SNK_Not_Supported_Received,
	PE_SNK_Send_Not_Supported,//14
	PE_SNK_Give_Sink_Cap_Ext,//15
#endif

// source port
#if(CONFIG_USBPD_POWER_ROLR & USBPD_POWER_ROLR_SRC)
	PE_SRC_Startup,
	PE_SRC_Discovery,
	PE_SRC_Send_Capabilities,
	PE_SRC_Negotiate_Capability,
	PE_SRC_Transition_Supply,
	PE_SRC_Ready,
	PE_SRC_Disabled,
	PE_SRC_Capability_Response,
	PE_SRC_Hard_Reset,
	PE_SRC_Hard_Reset_Received,
	PE_SRC_Transition_to_default,
	PE_SRC_Give_Source_Cap,
	PE_SRC_Wait_New_Capabilities,
	PE_SRC_Send_Soft_Reset,
	PE_SRC_Soft_Reset,
	PE_SRC_Not_Supported_Received,
	PE_SRC_Send_Not_Supported,
	PE_SRC_Give_PPS_Status,
#endif
	PE_Give_Revision,//32
	PE_SRC_SNK_Chunk_Received,//33
	PE_BIST_Carrier_Mode,//34
	PE_BIST_Test_Mode,//35

#if(CONFIG_USBPD_POWER_ROLR == USBPD_POWER_ROLR_DRP)
	PE_PRS_SRC_SNK_Evaluate_Swap,
	PE_PRS_SRC_SNK_Accept_Swap,
	PE_PRS_SRC_SNK_Transition_to_off,
	PE_PRS_SRC_SNK_Assert_Rd,
	PE_PRS_SRC_SNK_Wait_Source_on,
	PE_PRS_SRC_SNK_Send_Swap,
	PE_PRS_SRC_SNK_Reject_PR_Swap,
	PE_PRS_SNK_SRC_Evaluate_Swap,
	PE_PRS_SNK_SRC_Accept_Swap,
	PE_PRS_SNK_SRC_Transition_to_off,
	PE_PRS_SNK_SRC_Assert_Rp,
	PE_PRS_SNK_SRC_Source_on,
	PE_PRS_SNK_SRC_Reject_Swap,
	PE_PRS_SNK_SRC_Send_Swap,
#endif

	PE_STATE_MAX,
};


enum usb_pd_timer_e
{
	SenderResponseTimer,
	SinkWaitCapTimer,
	PSTransitionTimer,
	SinkPPSPeriodicTimer,

	SourceCapabilityTimer,
	SourcePPSCommTimer,
	PSHardResetTimer,
	SourceHardResetRecoverTimer,
	ChunkingNotSupportedTimer,
	BISTContModeTimer,
	PSSourceOnTimer,
	PSSourceOffTimer,
	USBPD_TIMER_MAX,
};

enum usb_pd_substate_e
{
	enter_state,
	exit_state,
};

enum tcpc_pe_transmit_type
{
	TRANSMITE_TYPE_HARDRESER,
	TRANSMITE_TYPE_BISTCARRYMODE,
	TRANSMITE_TYPE_REQUEST,
	TRANSMITE_TYPE_ACCEPT,
	TRANSMITE_TYPE_SOFTRESET,
	TRANSMITE_TYPE_NOTSUPPORT,
	TRANSMITE_TYPE_SOURCECAPS,
	TRANSMITE_TYPE_PSREADY,
	TRANSMITE_TYPE_REJECT,
	TRANSMITE_TYPE_REVISION,
	TRANSMITE_TYPE_PRSWAP,
	TRANSMITE_TYPE_GIVESNKCAP,
	TRANSMITE_TYPE_GIVESNKCAP_EXT,
	TRANSMITE_TYPE_GIVEPPS_STA,
	TRANSMITE_TYPE_MAX,
};

struct usb_pd_s
{
	int8_t rx_sop_msgid;
	int8_t rx_sop1_msgid;
	uint8_t tx_sop_msgid;
	uint8_t tx_sop1_msgid;
	uint8_t communitcate_capable;
	uint8_t explicit_contract;
	uint8_t hardreset_counter;
	uint8_t sink_request_index;
	uint8_t is_in_pps;
	uint8_t pe_prl_busy;
	uint8_t in_bist_mode;
	uint16_t pe_timer_cnt;

	uint8_t caps_counter;
	uint8_t nego_revision;
	uint8_t src_tx_pdo_n;
	uint8_t snk_rx_pdo_n;
	uint8_t snk_tx_pdo_n;
	enum tcpc_pe_transmit_type pe_tran_cb_type;



	uint16_t supply_voltage;
	uint16_t supply_current;

	uint32_t snk_rdo;
	uint32_t snk_rx_source_cap[7];

	uint32_t snk_sink_pdo[7];
	uint32_t src_source_pdo[7];

};

enum usb_pd_timer_state
{
	TIMER_STOP = 0,
	TIMER_RUNNING,
};

union usb_pd_timer_u
{
	struct usb_pd_timer_t
	{
		uint16_t time_cnt;    //timer remain cnt   timer period is 500uS
		enum usb_pd_timer_state state;		  //0: stop 		1: running
		bool timeout;	  //false or 0: not timeout 	true or 1: time out
	} timer;
	uint32_t word;
};

extern struct usb_pd_s g_usb_pd_s;
void usb_pd_timer_all_reset(void);
void usb_pd_set_event(uint8_t tc_index,uint32_t event);
void usb_pd_clear_event(uint32_t event);
void usb_pd_set_state(enum usb_pd_state_e pe_state,enum usb_pd_substate_e pe_substate);
void usb_pd_requsrt_voltage(uint32_t pdo_position,uint16_t voltage,uint16_t current);
void updata_pdo_of_source(uint32_t * pdo,uint8_t n_pdo);
void updata_pdo_of_sink(uint32_t * pdo,uint8_t n_pdo);
void usb_pd_snk_dump_pdoinfo(void);
void usb_pd_reset_prl(void);
void usb_pd_init(void);
void usb_pdevt_run(void);
void usb_pd_run(void);
#endif
