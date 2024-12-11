#ifndef VIC_H_
#define VIC_H_

/**
 * @brief Enable and set the priority of each interrupt.
 * @details The priority of each interrupt as below(Lower value has higher priority):
 * 			- IRQn_TMR0:    3
 * 			- IRQn_TMR1:    1
 * 			- IRQn_TMR2:    3
 * 			- IRQn_TMR3:    3
 * 			- IRQn_EADC:    2
 * 			- IRQn_ECAP1:   2
 * 			- IRQn_ECAP2:   2
 * 			- IRQn_ECAP3:   2
 * 			- IRQn_ECAP4:   2
 * 			- IRQn_ECAP5:   2
 * 			- IRQn_UART1:   3
 * 			- IRQn_I2CS:    3
 * 			- IRQn_USBPD:   0
 * 			- IRQn_FSK1:    1
 * 			- IRQn_FSK2:    1
 * @param  None
 * @retval	None	  
 * @note   This function should be called in the main function after the clocks are enabled.
 */
void hal_vic_init(void);

#endif /* VIC_H_ */
