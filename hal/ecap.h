#ifndef ECAP_H_
#define ECAP_H_

/**
  * @brief  Initializes the ECAP peripheral.
  * @param  ecap: Pointer to the ECAP instance.
  * @param  mode: Operating mode of the ECAP.
  * @retval void
  */
void hal_ecap_init(TS_ECAP *ecap, enum ECAP_FUNC_MODE mode);

/**
  * @brief  Opens the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_open(TS_ECAP *ecap);

/**
  * @brief  Closes the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_close(TS_ECAP *ecap);

/**
  * @brief  Registers a callback function for ECAP interrupts.
  * @param  func: Pointer to the callback function.
  * @retval void
  */
void hal_ecap_reg_int_cb(void (*func)(uint8_t, uint16_t));

void hal_ecap_dig_ddm_init(void);

typedef void (*pfn_ecap_cb)(uint8_t, uint16_t);
extern volatile pfn_ecap_cb ecap_callback;

#endif /* ECAP_H_ */
