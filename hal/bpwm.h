#ifndef BPWM_H_
#define BPWM_H_

//y = -15.18x + 18227

/**
 * @brief Initializes the BPWM peripheral according to the specified parameters * in the bpwm.
 *  @param bpwm Pointer to an instance of the BPWM module.
 *  @param perd_cycle The time value of the cycle.
 *  @param duty_cycle The time value of the duty cycle.
 *  @note This function should be called before hal_bpwm_start() to initialize the BPWM module.
 *  @retval void
 */
void hal_bpwm_start(TS_BPWM *bpwm, uint16_t perd_cycle, uint16_t duty_cycle);

/**
 *  @brief Updates the BPWM settings for the specified period and duty cycle.
 *  @note This function should be called after a call to hal_bpwm_start() to update the parameters of the BPWM module.
 *  @param bpwm Pointer to the structure of the BPWM module.
 *  @param perd_cycle The length of the cycle, in units that can be the number of clock cycles.
 *  @param duty_cycle The duty cycle, which indicates how long the signal will be high during a cycle, in clock cycles.
 *  @retval void
 */
void hal_bpwm_update(TS_BPWM *bpwm, uint16_t perd_cycle, uint16_t duty_cycle);

/**
 *  @brief Stops the BPWM operation.
 *  @note This function should be called after hal_bpwm_update() to stop the BPWM module.
 *  @param bpwm Pointer to the structure of the BPWM module.
 *  @retval void
 */
void hal_bpwm_stop(TS_BPWM* bpwm);


#endif /* BPWM_H_ */
