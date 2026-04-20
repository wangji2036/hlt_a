#ifndef GPIO_H_
#define GPIO_H_

/**
 * @brief  Initialization function for the GPIO peripheral.
 * 		  - PIN0: input enable, output disable, output value 0, open drain enable, pull-up disable, pull-down disable, mode 00:SCL1_S
 * 		  - PIN1: input enable, output disable, output value 0, open drain enable, pull-up disable, pull-down disable, mode 00:SDA1_S, interrupt disable, trigger type 00:Falling Edge
 * 		  - PIN4: input disable, output enable, output value 0, open drain disable, pull-up disable, pull-down disable, mode 00:PA4
 * 		  - PIN5: input disable, output enable, output value 0, open drain disable, pull-up disable, pull-down disable, mode 00:PA5	, interrupt disable, trigger type 00:Falling Edge
 * 		  - PIN6: input disable, output enable, output value 1, open drain disable, pull-up disable, pull-down disable, mode 00:PA6
 * 		  - PIN7: input disable, output enable, output value 1, open drain disable, pull-up disable, pull-down disable, mode 00:PA7
 * 		  - PB0:  input disable, output enable, output value 0, open drain disable, pull-up disable, pull-down disable, mode 00:PB0
 * 		  - PB1:  input disable, output enable, output value 0, open drain disable, pull-up disable, pull-down disable, mode 00:PB1	, interrupt disable, trigger type 00:Falling Edge	
 *
 * @param  void.
 * @retval void.
 */
void hal_gpio_init(void);
void hal_gpio_init_default(void);
#endif /* GPIO_H_ */
