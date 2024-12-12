#ifndef WPC_5_XFER_4_DSTRM_H_
#define WPC_5_XFER_4_DSTRM_H_

void ds_init(void);
void ds_mpp_prx_sadc_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask);
void ds_mpp_prx_sadt_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask);
void ds_mpp_prx_sdsr_pkt_handler(struct mpp_prx_ask_pkt_t *mpp_ask);
void ds_mpp_prx_dsr_poll_handler(struct mpp_ptx_fsk_pkt_t *fsk_pkt);

extern uint8_t array_digest[];
extern uint8_t array_chall[];
extern uint8_t adt_data_recv_buf[18];

#endif /* WPC_5_XFER_4_DSTRM_H_ */
