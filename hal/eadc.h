#ifndef EADC_H_
#define EADC_H_

/**
  * @brief  Initializes the EADC in analog demodulation mode.
  * @retval None
  */
void hal_eadc_init(void);
void hal_eadc_stop(void);
uint16_t hal_eadc_meas(enum eadc_chan_t channel);
void hal_eadc_ddm_init(void);

#endif /* EADC_H_ */
