#ifndef WPC_XFER_H_
#define WPC_XFER_H_

#include "osal.h"
#include "_wpc.h"
#include "pkt_type.h"

void mpp_get_pkt_process(struct mpp_prx_ask_pkt_t *mpp);

void wpc_xfer_phase_process(struct com_prx_ask_pkt_t *com_ask);
void wpc_cloak_phase_process(struct com_prx_ask_pkt_t *com_pkt);

extern uint8_t need_atn_cnt;//1:have gotten auth IC data
extern uint8_t need_atn_evt;
void auth_init(void);

#endif /* WPC_XFER_H_ */
