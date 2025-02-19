#ifndef TCPM_H_
#define TCPM_H_

#include "typdef.h"
#include "osal.h"

#define VOLTAGE_5V 	 	5000
#define VOLTAGE_9V  	9000
#define VOLTAGE_12V  	12000
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

#endif /* FML_H_ */
