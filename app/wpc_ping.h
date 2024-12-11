#ifndef WPC_PING_H_
#define WPC_PING_H_

#include "osal.h"
#include "_wpc.h"
#include "pkt_type.h"

void wpc_ping_phase_process(struct com_prx_ask_pkt_t *com_ask);

#endif /* WPC_PING_H_ */
