#ifndef EPWM_H_
#define EPWM_H_

//frequency : 360K, (110K, 148K)
//duty_ratio: 0-500
//phase: 0-180deg

/**
  * @brief  Starts the EPWM output.
  * @param  epwm: Pointer to the EPWM instance.
  * @param  perd_cycle: Period of the PWM signal.
  * @param  duty_ratio: Duty cycle ratio.
  * @param  phas_angle: Phase angle.
  * @retval void
  */
void hal_epwm_pwm_start(TS_EPWM *epwm, uint16_t perd_cycle, uint16_t duty_ratio, uint16_t phas_angle);

/**
  * @brief  Updates the EPWM output settings.
  * @param  epwm: Pointer to the EPWM instance.
  * @param  perd_cycle: Period of the PWM signal.
  * @param  duty_ratio: Duty cycle ratio.
  * @param  phas_angle: Phase angle.
  * @retval void
  */
void hal_epwm_pwm_update(TS_EPWM *epwm, uint16_t perd_cycle, uint16_t duty_ratio, uint16_t phas_angle);

/**
  * @brief  Stops the EPWM output.
  * @param  epwm: Pointer to the EPWM instance.
  * @retval void
  */
void hal_epwm_pwm_stop(TS_EPWM *epwm);

/**
  * @brief  Starts the Automatic Frequency Detection (AFD) functionality.
  * @param  epwm: Pointer to the EPWM instance.
  * @param  cycle: Cycle count for AFD.
  * @param  count: Step count for AFD.
  * @retval void
  */
void hal_epwm_afd_start(TS_EPWM *epwm, uint8_t cycle, uint8_t count);

/**
  * @brief  Stops the Automatic Frequency Detection (AFD) functionality.
  * @param  epwm: Pointer to the EPWM instance.
  * @retval void
  */
void hal_epwm_afd_stop(TS_EPWM *epwm);

#endif /* EPWM_H_ */
