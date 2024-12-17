#ifndef TCPM_H_
#define TCPM_H_

#include "typdef.h"
#include "osal.h"


#define MULTI_PORT_ALT_MODE

#define TCPM_EVT_TIME_PERIOD    			osal_event_declare(0)

#define TCPM_EVT_TYPECA_SNK_ATTACHED    	osal_event_declare(1)   // TYPEC1
#define TCPM_EVT_TYPECA_SRC_ATTACHED    	osal_event_declare(2)
#define TCPM_EVT_TYPECB_SNK_ATTACHED    	osal_event_declare(3)
#define TCPM_EVT_TYPECB_SRC_ATTACHED    	osal_event_declare(4)
#define TCPM_EVT_PORTA_ATTACHED    			osal_event_declare(5)
#define TCPM_EVT_WPC_ATTACHED    			osal_event_declare(7)

#define TCPM_EVT_TYPECA_PORT_STATE_CHANGE   osal_event_declare(8)
#define TCPM_EVT_TYPECB_PORT_STATE_CHANGE   osal_event_declare(9)
#define TCPM_EVT_SNK_START_CHARGER   		osal_event_declare(10)


#define TCPM_EVT_USBA_SCAN			   		osal_event_declare(11)
#define TCPM_EVT_USBA_PLUG			   		osal_event_declare(12)

#define TCPM_EVT_USBA_WORK			   		osal_event_declare(13)
#define TCPM_EVT_USBA_DETEN			   		osal_event_declare(14)

#define TCPM_EVT_QI_WORK			   		osal_event_declare(15)

#define TCPM_EVT_PD_READY			   		osal_event_declare(16)
#define TCPM_EVT_QI_SET_VOLT			   	osal_event_declare(17)

void tcpm_task_init(void);
void tcpm_task_event_handler(uint32_t event);

extern uint16_t port_vbus;
extern uint8_t tcpm_qi_work_delay;
extern uint16_t qi_volt;

enum wpc_work_mode
{
	TCPM_WPC_WORK_FIX5V = 0,
	TCPM_WPC_WORK_BOOST,
	TCPM_WPC_WORK_ADP_FIX,
	TCPM_WPC_WORK_PD_PPS,
};

extern uint8_t wpc_work_mode;
#endif /* FML_H_ */
