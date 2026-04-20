#ifndef SYS_H_
#define SYS_H_

/**
 * @brief 	   Clock initialization function for clock source selection and frequency range selection.
 * @details    This function initializes the clock system. 
 * 			   - It sets the CPU clock to 36MHz,
 *             - It sets the PLL source to XTAL, the XTAL pre-divider to 3, 
 *             - And enables the XTAL(external crystal).
 * @param  		void
 * @retval		void
 */
void hal_sys_init(void);

#endif /* SYS_H_ */
