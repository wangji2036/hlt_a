/**
  ******************************************************************************
  * @file    i2cs.c
  * @brief   I2C Slave HAL module driver.<br>
  *          The functionalities of the I2C Slave peripheral as below:<br>
  *           - Initialization and de-initialization functions. <br>
  *           - Uart transmit a character. <br>
  * 		      - Register the interrupt callback function. <br>
  * 		      - Retarget the fputc function to the uart transmit function. <br>
  * 		      - UART1 and UART2 Interrupt handler. <br>
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech. <br>
  * All rights reserved. <br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### I2CS Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the I2CS features when IC datasheet is available.

  [..] 
  The I2C Salve (I2CS) has the following features:
  (+) Supports 7-bit addressing.
  (+) Supports 100KHz and 400KHz bus speeds.
  (+) Supports Continue mode.
  (+) Supports 16-bit data width.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Configure the I2CS using the hal_i2cs_init() function.

	
  @endverbatim
  ******************************************************************************
  */ 
#include "regdef.h"
#include "i2cs.h"

typedef void (*pFunc)(void);
static volatile pFunc m_pfn_I2CS_GR03IntHandler = NULL;

/**
 * @brief Enable I2CS interrupt.
 * @param  void
 * @return void
 */
void hal_i2cs_init(void)
{
	I2CS->CTRL.BITS.GR03_INT_EN = 1;
//	I2CS->CTRL.BITS.SRAM_INT_EN = 1;
}

/**
 * @brief Register the interrupt call back function.
 * @param func The interrupt call back function.
 * @return void	
 */
void hal_i2cs_reg_int_cb(void (*func)(void))
{
	m_pfn_I2CS_GR03IntHandler = func;
}

void __attribute__((isr)) I2CS_IRQHandler(void)
{
	if (I2CS->FLAG.BITS.GR03_FLAG)
	{
		if (m_pfn_I2CS_GR03IntHandler != NULL)
		{
			m_pfn_I2CS_GR03IntHandler();
		}
		I2CS->FLAG.BITS.GR03_FLAG = 1;
	}

//	if (I2CS->FLAG.BITS.SRAM_FLAG == 1)
//	{
//		I2CS->FLAG.BITS.SRAM_FLAG = 1;
//	}
}
