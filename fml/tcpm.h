#ifndef TCPM_H_
#define TCPM_H_

#include "typdef.h"
#include "osal.h"
#include "config.h"

#if SUPPORT_TCPM_LOG
	#define tcpm_printk 	printk
#else
	#define tcpm_printk(...)
#endif

#define V_VBUS_PRESENT_TH			3800

enum usb_tc_state_e
{
	TC_Disable = 0,

	TC_SNK_Unattached,  	//1
	TC_SNK_AttachWait,
	TC_SNK_Attached,

	TC_SRC_Unattached,  	//4
	TC_SRC_AttachWait,
	TC_SRC_Attached,
	TC_DEBUG_Attached,		//7

	TC_DRP_TOGGLE,			//8
	TC_Try_SNK,				//9
	TC_TryWAIT_SRC,			//10
	TC_Try_SRC,				//11
	TC_TryWAIT_SNK,			//12
	TC_ACCESSORY_Attached,		//13
	TC_ErrorRecovery,

	TC_STATE_MAX,
};

enum tc_cc_status
{
    TYPEC_CC_OPEN,
    TYPEC_CC_RA,
    TYPEC_CC_RD,
    TYPEC_CC_RP_DEF,
    TYPEC_CC_RP_1_5,
    TYPEC_CC_RP_3_0,
    TYPEC_CC_TOGGLE,
};

enum tc_drp_reult
{
	TYPEC_DRP_NO_CONNECT = 0,
	TYPEC_DRP_SNK_CONNECTED,
	TYPEC_DRP_SRC_CONNECTED,
};


enum tc_cc_polarity
{
    TYPEC_POLARITY_CC1,
    TYPEC_POLARITY_CC2,
};

enum data_role_e
{
	TYPEC_DEVICE = 0,
	TYPEC_HOST,
};

enum pwr_role_e
{
	TYPEC_SINK = 0,
	TYPEC_SOURCE,
};


struct tcpc_s
{
	uint8_t tc_port_map;
	enum data_role_e data_role;
	enum pwr_role_e pwr_role;

};

#define VOLTAGE_5V 	 	5000
#define VOLTAGE_9V  	9000
#define VOLTAGE_12V  	12000
#define VOLTAGE_15V   	15000   //Victor  15v
#define VOLTAGE_20V   	20000   //Victor  20v
#define VOLTAGE_PPS  	11000


#define PDO_INDEX_1		1
#define PDO_INDEX_2		2
#define PDO_INDEX_3		3
#define PDO_INDEX_4		4
#define PDO_INDEX_5		5
#define PDO_INDEX_6		6
#define PDO_INDEX_7		7

#define WPC_DELAY				10

#define MULTI_PORT_ALT_MODE

#define TCPM_EVT_TIME_PERIOD    			osal_event_declare(0)

#define TCPM_EVT_USBA_SCAN			   		osal_event_declare(11)
#define TCPM_EVT_USBA_REDETECT			   	osal_event_declare(12)

#define TCPM_EVT_USBA_WORK			   		osal_event_declare(13)
#define TCPM_EVT_USBA_DETEN			   		osal_event_declare(14)

#define TCPM_EVT_QI_WORK			   		osal_event_declare(15)

#define TCPM_EVT_PD_READY			   		osal_event_declare(16)
#define TCPM_EVT_QI_SET_VOLT			   	osal_event_declare(17)
#define TCPM_EVT_HVDCP_DONE					osal_event_declare(18)
#define TCPM_EVT_DPDM_DONE					osal_event_declare(19)


extern uint16_t port_vbus;
extern uint8_t tcpm_qi_work_delay;
extern uint16_t qi_volt;

enum wpc_work_mode
{
	TCPM_WPC_WORK_FIX5V = 0,
	TCPM_WPC_WORK_BOOST,
	TCPM_WPC_WORK_ADP_FIX,
	TCPM_WPC_WORK_PD_PPS,
	TCPM_WPC_WORK_DISABLE,
};

extern uint8_t wpc_mode;
extern uint8_t wpc_mode_pre;
extern uint8_t qi_state;
void tcpm_task_init(void);
void tcpm_task_event_handler(uint32_t event);
void tcpm_set_port_sdp(uint8_t tc_index);
void tcpm_stop_wpc(uint8_t delay_ping_unit);
void tcpm_update_wpc_work_mode(enum wpc_work_mode mode);
void tcpm_disable_usba_detect(void);
void tcpm_update_pdo_for_ntc(void);
void tcpm_update_pdo_for_limit(void);
void tcpm_dp_set_10uA(void);
uint32_t tcpm_dp_get_result(void);

#endif /* FML_H_ */
