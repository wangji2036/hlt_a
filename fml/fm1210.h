#ifndef FM1210_H_
#define FM1210_H_

#include "typdef.h"

struct fm_head_t {
	uint8_t len; // len include flag + data
	union {
		uint8_t cmd;
		uint8_t sta;
    } flag;
} __attribute__ ((packed));

struct fm_pack_t {
	uint8_t cmd;
	uint8_t apdu_data[256];
} __attribute__ ((packed));

void fm1210_init(void);

int fm1210_i2c_send_frame(uint8_t cmd, uint8_t *sbuf, uint16_t slen);
int fm1210_i2c_recv_frame(uint8_t *rbuf, uint16_t *rlen);
int fm1210_i2c_transceive(uint8_t *sbuf, uint16_t slen, uint8_t *rbuf, uint16_t *rlen);

int fm1210_get_qi_id(uint8_t *rbuf);
int fm1210_read_cert_hash(uint8_t *rbuf);
int fm1210_read_se_cert(uint8_t *rbuf, uint16_t *rlen);
int fm1210_get_cert_chain(uint8_t *wpc_cert_hash, uint8_t *manufacturer_cert, uint16_t manufacturer_cert_len, uint8_t *rbuf, uint16_t *rlen);
int fm1210_get_tbs_auth(uint8_t *rbuf);

#endif /* FM1210_H_ */
