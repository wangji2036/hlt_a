#ifndef FSK_H_
#define FSK_H_

#include "regdef.h"

enum fsk_cyc_t
{
	_FSK_BIT_CYCLES_512 = 0,
	_FSK_BIT_CYCLES_256 = 1,
	_FSK_BIT_CYCLES_128 = 2,
	_FSK_BIT_CYCLES_064 = 3,
};

#define _FSK_ACK    0xFF
#define _FSK_NAK    0x00
#define _FSK_N_D    0x55
#define _FSK_ATN    0x33
#define _FSK_MPP    0x11
#define _FSK_APP    0x0F

struct fsk_cfg_t {
	uint8_t polar : 1;
	uint8_t depth : 2;
	uint8_t cycle : 3;
	uint8_t prmbl : 1;
	uint8_t       : 1;
};

/**
 * @brief  check FSK is sending or not.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 *
 * @return 0-idle 1-busy.
 */
uint8_t fml_fsk_is_busy(TS_EPWM *epwm);

/**
 * @brief  Set the parameters for FSK transmission
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  This parameter determines whether the difference between f_mod and f_op is positive or negative,
 *         positive(0) or negative(1).
 * @param  This parameter determines the magnitude of the difference between f_op and f_mod.
 *         depth needs to be within [0,3].
 * @param  EPWM number of cycles per bit, please use _FSK_BIT_CYCLES_512 or _FSK_BIT_CYCLES_128.
 * @param  FSK send data packet need add 4-bit-preamble or not.
 *
 * @return None.
 */
void fml_fsk_param_set(TS_EPWM *epwm, uint8_t polarity, uint8_t depth, uint8_t cycles, uint8_t preamble);

/**
 * @brief  send FSK pattern data.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  FSK response delay time, unit ms.
 * @param  8bit FSK pattern.
 *
 * @return None.
 */
void fml_fsk_patt_send(TS_EPWM *epwm, uint16_t delay_ms, uint8_t pattern);

/**
 * @brief  send FSK packet data.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  FSK response delay time, unit ms.
 * @param  packet buffer need to be send.
 * @param  the length of packet buffer need to be send.
 *
 * @return None.
 */
void fml_fsk_data_send(TS_EPWM *epwm, uint16_t delay_ms, uint8_t *data, uint8_t len);

#endif /* FSK_H_ */
