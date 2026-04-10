#ifndef FML_H_
#define FML_H_

#include "osal.h"

#define FML_EVT_ASK_INT_RECVD    	osal_event_declare(0)
#define APL_EVT_GAUGE          		osal_event_declare(1)
#define APL_HID_REPORT          	osal_event_declare(2)


#define T_GAUGE    100

void fml_task_init(void);
void fml_task_event_handler(uint32_t event);
int16_t ntc_to_temp(uint16_t ntc);

#endif /* FML_H_ */
