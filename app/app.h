#ifndef APP_H_
#define APP_H_

#include "osal.h"

#define APL_EVT_010ms_POLL    osal_event_declare(0)
#define APL_EVT_100ms_POLL    osal_event_declare(1)
#define APL_EVT_250ms_POLL    osal_event_declare(2)

void apl_task_init(void);
void apl_task_event_handler(uint32_t event);

void APP_vInit(void);
void APP_vHandler(void);
void APP_vUART1_RxIntHandler(void);

#endif /* APP_H_ */
