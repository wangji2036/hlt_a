/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   GPIO HAL module driver.<br>
  *          The functionalities of the General Purpose Input/Output (GPIO) peripheral:<br>
  *           - Initialization and de-initialization functions <br>
  *           - IO operation functions
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech. <br>
  * All rights reserved.  <br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### GPIO Peripheral features #####
  ==============================================================================
  [..] 
  Subject to the specific hardware characteristics of each I/O port listed in the datasheet, each
  port bit of the General Purpose IO (GPIO) Ports, can be individually configured by software
  in several modes:
  (+) Input mode 
  (+) Analog mode
  (+) Output mode
  (+) Alternate function mode
  (+) External interrupt/event lines

  [..]  
  During and just after reset, the alternate functions and external interrupt  
  lines are not active and the I/O ports are configured in input floating mode.
  
  [..]   
  The GPIO pins are programmable general-purpose I/O pins whose functions can be assigned. 
  These pins can be configured both as inputs and outputs (either push-pull or open-drain) 
  according to the selected function. The GPIN pins can only be configured as inputs.

  [..]
  In Output or Alternate mode, each IO can be configured on open-drain or push-pull
  type and the IO speed can be selected depending on the VDD value.

  [..]  
  23x GPIO+7x GPIN, keep every GPIO/GPIN channel independent.
  The minimum Logic High is 1.4V(VDD =2.5V~5V),1.5V(VDD=5.5V).The maximum Logic Low is 0.6V. 
  
  [..]
  For the GPIO PA,PB,PC,the structure is shown as the below figure.
  All the PAs,PBs,PCs have the same structure except from the function in the red frame.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Configure the GPIO pin(s) using hal_gpio_init().
		(++) Only the default configuration works for the GPIO pins.
		(++) Other GPIO function will be release in furture version.
	(#) ...
  @endverbatim
  ******************************************************************************
  */
#include "regdef.h"
#include "gpio.h"

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
void hal_gpio_init(void)
{
	/* PA0 */
	GPA->I_EN.BITS.PIN0 = 1;
	GPA->O_EN.BITS.PIN0 = 0;
	GPA->DOUT.BITS.PIN0 = 0;
	GPA->ODEN.BITS.PIN0 = 1;
	GPA->PUEN.BITS.PIN0 = 0;
	GPA->PDEN.BITS.PIN0 = 0;
	GPA->MODE.BITS.PIN0 = 0; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C

	/* PA1 */
	GPA->I_EN.BITS.PIN1 = 1;
	GPA->O_EN.BITS.PIN1 = 0;
	GPA->DOUT.BITS.PIN1 = 0;
	GPA->ODEN.BITS.PIN1 = 1;
	GPA->PUEN.BITS.PIN1 = 0;
	GPA->PDEN.BITS.PIN1 = 0;
	GPA->MODE.BITS.PIN1 = 0; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
	GPA->ITEN.BITS.PIN1 = 0;
	GPA->ITTP.BITS.PIN1 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge

	/* PA4 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPA->I_EN.BITS.PIN4 = 0;
		GPA->O_EN.BITS.PIN4 = 0;
		GPA->DOUT.BITS.PIN4 = 0;
		GPA->ODEN.BITS.PIN4 = 0;
		GPA->PUEN.BITS.PIN4 = 0;
		GPA->PDEN.BITS.PIN4 = 0;
		GPA->MODE.BITS.PIN4 = 0; //00:PA4 01:SDA3_S 10:RESERVED 11:RESERVED
	}
	else
	{
		GPA->I_EN.BITS.PIN4 = 1;
		GPA->O_EN.BITS.PIN4 = 0;
		GPA->DOUT.BITS.PIN4 = 0;
		GPA->ODEN.BITS.PIN4 = 0;
		GPA->PUEN.BITS.PIN4 = 0;
		GPA->PDEN.BITS.PIN4 = 0;
		GPA->MODE.BITS.PIN4 = 0; //00:PA4 01:SDA3_S 10:RESERVED 11:RESERVED
	}

	/* PA5 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPA->I_EN.BITS.PIN5 = 0;
		GPA->O_EN.BITS.PIN5 = 1;
		GPA->DOUT.BITS.PIN5 = 1;
		GPA->ODEN.BITS.PIN5 = 0;
		GPA->PUEN.BITS.PIN5 = 0;
		GPA->PDEN.BITS.PIN5 = 0;
		GPA->MODE.BITS.PIN5 = 0; //00:PA5 01:SCL3_S 10:LS_ISNS_PGA_N 11:RESERVED
	}
	else
	{
		GPA->I_EN.BITS.PIN5 = 0;
		GPA->O_EN.BITS.PIN5 = 1;
		GPA->DOUT.BITS.PIN5 = 0;
		GPA->ODEN.BITS.PIN5 = 0;
		GPA->PUEN.BITS.PIN5 = 0;
		GPA->PDEN.BITS.PIN5 = 0;
		GPA->MODE.BITS.PIN5 = 0; //00:PA5 01:SCL3_S 10:LS_ISNS_PGA_N 11:RESERVED
	}

	/* PA6 */
	GPA->I_EN.BITS.PIN6 = 0;
	GPA->O_EN.BITS.PIN6 = 1;
	GPA->DOUT.BITS.PIN6 = 1;
	GPA->ODEN.BITS.PIN6 = 0;
	GPA->PUEN.BITS.PIN6 = 0;
	GPA->PDEN.BITS.PIN6 = 0;
	GPA->MODE.BITS.PIN6 = 0; //00:PA6 01:SCL2_M 10:RESERVED 11:RESERVED

	/* PA7 */
	GPA->I_EN.BITS.PIN7 = 0;
	GPA->O_EN.BITS.PIN7 = 1;
	GPA->DOUT.BITS.PIN7 = 1;
	GPA->ODEN.BITS.PIN7 = 0;
	GPA->PUEN.BITS.PIN7 = 0;
	GPA->PDEN.BITS.PIN7 = 0;
	GPA->MODE.BITS.PIN7 = 0; //00:PA7 01:SDA2_M 10:RESERVED 11:RESERVED


	/* PB0 */
	GPB->I_EN.BITS.PIN0 = 0;
	GPB->O_EN.BITS.PIN0 = 0;
	GPB->DOUT.BITS.PIN0 = 0;
	GPB->ODEN.BITS.PIN0 = 0;
	GPB->PUEN.BITS.PIN0 = 0;
	GPB->PDEN.BITS.PIN0 = 0;
	GPB->MODE.BITS.PIN0 = 1; //00:PB0 01:OSC_IN 10:RESERVED 11:RESERVED

	/* PB1 */
	GPB->I_EN.BITS.PIN1 = 0;
	GPB->O_EN.BITS.PIN1 = 0;
	GPB->DOUT.BITS.PIN1 = 0;
	GPB->ODEN.BITS.PIN1 = 0;
	GPB->PUEN.BITS.PIN1 = 0;
	GPB->PDEN.BITS.PIN1 = 0;
	GPB->MODE.BITS.PIN1 = 1; //00:PB1 01:OSC_OUT 10:RESERVED 11:RESERVED

	/* PB2 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPB->I_EN.BITS.PIN2 = 0;
		GPB->O_EN.BITS.PIN2 = 1;
		GPB->DOUT.BITS.PIN2 = 1;
		GPB->ODEN.BITS.PIN2 = 0;
		GPB->PUEN.BITS.PIN2 = 0;
		GPB->PDEN.BITS.PIN2 = 0;
		GPB->MODE.BITS.PIN2 = 1; //00:PB2 01:BPWM3 10:BADC2 11:DP_C2
	}
	else
	{
		GPB->I_EN.BITS.PIN2 = 1;
		GPB->O_EN.BITS.PIN2 = 0;
		GPB->DOUT.BITS.PIN2 = 0;
		GPB->ODEN.BITS.PIN2 = 0;
		GPB->PUEN.BITS.PIN2 = 0;
		GPB->PDEN.BITS.PIN2 = 0;
		GPB->MODE.BITS.PIN2 = 2; //00:PB2 01:BPWM3 10:BADC2 11:DP_C2
	}

	/* PB3 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPB->I_EN.BITS.PIN3 = 0;
		GPB->O_EN.BITS.PIN3 = 0;
		GPB->DOUT.BITS.PIN3 = 0;
		GPB->ODEN.BITS.PIN3 = 0;
		GPB->PUEN.BITS.PIN3 = 0;
		GPB->PDEN.BITS.PIN3 = 0;
		GPB->MODE.BITS.PIN3 = 0; //00:PB3 01:JTAG_CLK 10:BPWM7 11:RESERVED
	}
	else
	{
		GPB->I_EN.BITS.PIN3 = 1;
		GPB->O_EN.BITS.PIN3 = 0;
		GPB->DOUT.BITS.PIN3 = 0;
		GPB->ODEN.BITS.PIN3 = 0;
		GPB->PUEN.BITS.PIN3 = 0;
		GPB->PDEN.BITS.PIN3 = 0;
		GPB->MODE.BITS.PIN3 = 0; //00:PB3 01:JTAG_CLK 10:BPWM7 11:RESERVED
	}

	/* PB4 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPB->I_EN.BITS.PIN4 = 0;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->DOUT.BITS.PIN4 = 0;
		GPB->ODEN.BITS.PIN4 = 0;
		GPB->PUEN.BITS.PIN4 = 0;
		GPB->PDEN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 0;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge
	}
	else
	{
		GPB->I_EN.BITS.PIN4 = 1;
		GPB->O_EN.BITS.PIN4 = 0;
		GPB->DOUT.BITS.PIN4 = 0;
		GPB->ODEN.BITS.PIN4 = 0;
		GPB->PUEN.BITS.PIN4 = 0;
		GPB->PDEN.BITS.PIN4 = 0;
		GPB->MODE.BITS.PIN4 = 0; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		GPB->ITEN.BITS.PIN4 = 0;
		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge
	}

	/* PB5 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPB->I_EN.BITS.PIN5 = 1;
		GPB->O_EN.BITS.PIN5 = 0;
		GPB->DOUT.BITS.PIN5 = 0;
		GPB->ODEN.BITS.PIN5 = 0;
		GPB->PUEN.BITS.PIN5 = 0;
		GPB->PDEN.BITS.PIN5 = 0;
		GPB->MODE.BITS.PIN5 = 2; //00:PB5 01:JTAG_RST 10:BADC6 11:LS_ISNS_PGA_P
	}
	else
	{
		GPB->I_EN.BITS.PIN5 = 0;
		GPB->O_EN.BITS.PIN5 = 1;
		GPB->DOUT.BITS.PIN5 = 0;
		GPB->ODEN.BITS.PIN5 = 0;
		GPB->PUEN.BITS.PIN5 = 0;
		GPB->PDEN.BITS.PIN5 = 0;
		GPB->MODE.BITS.PIN5 = 0; //00:PB5 01:JTAG_RST 10:BADC6 11:LS_ISNS_PGA_P
	}

	/* PB6 */
	GPB->I_EN.BITS.PIN6 = 1;
	GPB->O_EN.BITS.PIN6 = 0;
	GPB->DOUT.BITS.PIN6 = 0;
	GPB->ODEN.BITS.PIN6 = 0;
	GPB->PUEN.BITS.PIN6 = 0;
	GPB->PDEN.BITS.PIN6 = 0;
	GPB->MODE.BITS.PIN6 = 0; //00:PB6 01:BADC7 10:RESERVED 11:RESERVED

	/* PB7 */
	GPB->I_EN.BITS.PIN7 = 0;
	GPB->O_EN.BITS.PIN7 = 1;
	GPB->DOUT.BITS.PIN7 = 1;
	GPB->ODEN.BITS.PIN7 = 0;
	GPB->PUEN.BITS.PIN7 = 1;
	GPB->PDEN.BITS.PIN7 = 0;
	GPB->MODE.BITS.PIN7 = 1; //00:PB7 01:UART1_TXD 10:RESERVED 11:RESERVED


	/* PC0 */
	GPC->I_EN.BITS.PIN0 = 0;
	GPC->O_EN.BITS.PIN0 = 1;
	GPC->DOUT.BITS.PIN0 = 0;
	GPC->ODEN.BITS.PIN0 = 0;
	GPC->PUEN.BITS.PIN0 = 0;
	GPC->PDEN.BITS.PIN0 = 0;
	GPC->MODE.BITS.PIN0 = 1; //00:PC0 01:EPWM1 10:RESERVED 11:RESERVED

	/* PC1 */
	GPC->I_EN.BITS.PIN1 = 0;
	GPC->O_EN.BITS.PIN1 = 1;
	GPC->DOUT.BITS.PIN1 = 0;
	GPC->ODEN.BITS.PIN1 = 0;
	GPC->PUEN.BITS.PIN1 = 0;
	GPC->PDEN.BITS.PIN1 = 0;
	GPC->MODE.BITS.PIN1 = 1; //00:PC1 01:EPWM2 10:RESERVED 11:RESERVED

	/* PC2 */
	GPC->I_EN.BITS.PIN2 = 1;
	GPC->O_EN.BITS.PIN2 = 0;
	GPC->DOUT.BITS.PIN2 = 0;
	GPC->ODEN.BITS.PIN2 = 0;
	GPC->PUEN.BITS.PIN2 = 0;
	GPC->PDEN.BITS.PIN2 = 0;
	GPC->MODE.BITS.PIN2 = 0; //00:PC2 01:RESERVED 10:RESERVED 11:RESERVED
	GPC->ITEN.BITS.PIN2 = 0;
	GPC->ITTP.BITS.PIN2 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge

	/* PC3 */
	GPC->I_EN.BITS.PIN3 = 0;
	GPC->O_EN.BITS.PIN3 = 0;
	GPC->DOUT.BITS.PIN3 = 0;
	GPC->ODEN.BITS.PIN3 = 0;
	GPC->PUEN.BITS.PIN3 = 0;
	GPC->PDEN.BITS.PIN3 = 0;
	GPC->MODE.BITS.PIN3 = 2; //00:PC3 01:BPWM4 10:DP_A1 11:RESERVED

	/* PC4 */
	GPC->I_EN.BITS.PIN4 = 0;
	GPC->O_EN.BITS.PIN4 = 0;
	GPC->DOUT.BITS.PIN4 = 0;
	GPC->ODEN.BITS.PIN4 = 0;
	GPC->PUEN.BITS.PIN4 = 0;
	GPC->PDEN.BITS.PIN4 = 0;
	GPC->MODE.BITS.PIN4 = 2; //00:PC4 01:EPWM5 10:DM_A1 11:RESERVED

	/* PC5 */
	GPC->I_EN.BITS.PIN5 = 1;
	GPC->O_EN.BITS.PIN5 = 0;
	GPC->DOUT.BITS.PIN5 = 0;
	GPC->ODEN.BITS.PIN5 = 0;
	GPC->PUEN.BITS.PIN5 = 0;
	GPC->PDEN.BITS.PIN5 = 0;
	GPC->MODE.BITS.PIN5 = 0; //00:PC5 01:EPWM6 10:RESERVED 11:RESERVED

	/* PC6 */
	GPC->I_EN.BITS.PIN6 = 1;
	GPC->O_EN.BITS.PIN6 = 0;
	GPC->DOUT.BITS.PIN6 = 0;
	GPC->ODEN.BITS.PIN6 = 0;
	GPC->PUEN.BITS.PIN6 = 1;
	GPC->PDEN.BITS.PIN6 = 0;
	GPC->MODE.BITS.PIN6 = 0; //00:PC6 01:BADC1 10:ECAP4 11:RESERVED
	GPC->ITEN.BITS.PIN6 = 0;
	GPC->ITTP.BITS.PIN6 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge

	/* PC7 */
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		GPC->I_EN.BITS.PIN7 = 0;
		GPC->O_EN.BITS.PIN7 = 0;
		GPC->DOUT.BITS.PIN7 = 0;
		GPC->ODEN.BITS.PIN7 = 0;
		GPC->PUEN.BITS.PIN7 = 0;
		GPC->PDEN.BITS.PIN7 = 0;
		GPC->MODE.BITS.PIN7 = 0; //00:PC7 01:BADC4 10:RESERVED 11:RESERVED
	}
	else
	{
		GPC->I_EN.BITS.PIN7 = 1;
		GPC->O_EN.BITS.PIN7 = 0;
		GPC->DOUT.BITS.PIN7 = 0;
		GPC->ODEN.BITS.PIN7 = 0;
		GPC->PUEN.BITS.PIN7 = 0;
		GPC->PDEN.BITS.PIN7 = 0;
		GPC->MODE.BITS.PIN7 = 1; //00:PC7 01:BADC4 10:RESERVED 11:RESERVED
	}

	/* PC8 */
	GPC->I_EN.BITS.PIN8 = 0;
	GPC->O_EN.BITS.PIN8 = 0;
	GPC->DOUT.BITS.PIN8 = 0;
	GPC->ODEN.BITS.PIN8 = 0;
	GPC->PUEN.BITS.PIN8 = 0;
	GPC->PDEN.BITS.PIN8 = 0;
	GPC->MODE.BITS.PIN8 = 0; //00:CC2_L 01:PC8 10:BADC5 11:RESERVED

	/* PD0 */
	GPD->I_EN.BITS.PIN0 = 1;
	GPD->MODE.BITS.PIN0 = 1; //00:PD0 01:BADC8 10:DM_C2 11:RESERVED

	/* PD1 */
	GPD->I_EN.BITS.PIN1 = 0;
	GPD->MODE.BITS.PIN1 = 0; //00:PD1 01:UART1_RXD 10:ECAP5 11:RESERVED
	GPD->ITEN.BITS.PIN1 = 0;
	GPD->ITTP.BITS.PIN1 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge

	/* PD2 */
	GPD->I_EN.BITS.PIN2 = 0;
	GPD->MODE.BITS.PIN2 = 0; //00:CC1_L 01:PD2 10:ECAP3 11:RESERVED

	/* PD3 */
	GPD->I_EN.BITS.PIN3 = 0;
	GPD->MODE.BITS.PIN3 = 1; //00:PD3 01:BADC9 10:RESERVED 11:RESERVED
	GPD->ITEN.BITS.PIN3 = 0;
	GPD->ITTP.BITS.PIN3 = 0; //00:Falling Edge 01:Rising Edge 1x:both edge

	/* PD4 */
	GPD->I_EN.BITS.PIN4 = 1;
	GPD->MODE.BITS.PIN4 = 1; //00:PD4 01:ECAP1 10:RESERVED 11:RESERVED

	/* PD5 */
	GPD->I_EN.BITS.PIN5 = 1;
	GPD->MODE.BITS.PIN5 = 1; //00:PD5 01:ECAP2 10:RESERVED 11:RESERVED

	/* PD6 */
	GPD->I_EN.BITS.PIN6 = 1;
	GPD->MODE.BITS.PIN6 = 1; //00:PD6 01:BADC3 10:RESERVED 11:RESERVED
}

void __attribute__((isr)) GPIO_IRQHandler(void)
{
}
