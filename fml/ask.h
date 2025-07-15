#ifndef ASK_H_
#define ASK_H_

struct ask_packet_t {
	uint8_t src;
	uint8_t hdr;
	uint8_t len;
	uint8_t data[29]; //header + message + checksum
};

/**
 * @brief  ask decode function.
 *
 * When receiving the capture signal from ECAP, call this function can decode bit-byte-packet.
 * If the package is successfully received, an event notification will be sent to the application layer.
 *
 * @param  None.
 * @return None.
 */
void fml_ask_decode(void);

/**
 * @brief  enable ask decode function.
 *
 * when ready to start EPWM for power transfer, call enable function to initialize the hal_layer demodulation driver
 *
 * @param  None.
 * @return None.
 */
void fml_ask_enable(void);

/**
 * @brief  disable ask decode function.
 *
 * when removed power signal, call disable function to disable the hal_layer demodulation driver
 *
 * @param  None.
 * @return None.
 */
void fml_ask_disable(void);

/**
 * @brief  check ask decode function.
 *
 * during power transfer, call the check function to check the demodulation status
 * of each channel and perform some processing.
 *
 * @param  None.
 * @return None.
 */
void fml_ask_decode_check(void);

//void fml_test_ask_info_print(uint8_t chan);

//void add(void);

#endif /* ASK_H_ */
