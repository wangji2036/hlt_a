/**
  ******************************************************************************
  * @file    uart.c
  * @brief   Uart HAL module driver.<br>
  *          The functionalities of the UART peripheral:<br>
  *           - Initialization and de-initialization functions. <br>
  *           - Uart transmit a character. <br>
  * 		      - Register the interrupt callback function. <br>
  * 		      - Retarget the fputc function to the uart transmit function. <br>
  * 		      - UART1 and UART2 Interrupt handler. <br>
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech.<br>
  * All rights reserved. <br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### UART Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the UART features when IC datasheet is available.

  [..] 
  The UART has the following features:
  -# Configurable baud rate.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Configure the Uart1 or UART2 using the hal_uart_init() function.
    (#) Register the interrupt callback function using the hal_uart_reg_int_cb() function before using the UART.
    (#) Use the hal_uart_putc() function to transmit a character.
    (#) Use the retarget_fputc() function to redirect the fputc() function to the hal_uart_putc() function.	
	
  @endverbatim
  ******************************************************************************
  */ 
#include "regdef.h"
#include "debug.h"
#include "uart.h"

typedef void (*pFunc)(void);
static volatile pFunc m_pfn_UART1_RxIntHandler = NULL;
static volatile pFunc m_pfn_UART2_RxIntHandler = NULL;

/**
 * @brief Initialize the UART. Mason 17111 have two UARTs, UART1 and UART2. 
 * 		-	The UART is configured as a USCI_A0 module.
 * 		-	The baud rate is set to 250K.	
 * @param uart UART1 or UART2
 * @return void
 * @note This function should be called before using the UART.
 */
void hal_uart_init(TS_UART *uart)
{
	//buad_rate = 36M/clk_div/clk_cnt;
	uint16_t clk_div = 16;
	uint16_t clk_cnt =  9;
	uart->GEN_CTRL.WORD = (_UART_USCI_MODE_1 << UART_GEN_CTRL_USCI_MOD_Pos) | UART_GEN_CTRL_MUTI_MOD_Msk | UART_GEN_CTRL_RXD_EN_Msk; //TX&RX
	uart->BRG_CTRL.WORD = ((clk_div - 1) << UART_BRG_CTRL_CLK_DIV_Pos) | ((0x7FFF - (clk_cnt - 1)) << UART_BRG_CTRL_CLK_CNT_Pos) | UART_BRG_CTRL_BRG_EN_Msk; //250K
}

/**
 * @brief Uart transmit a character.
 * @param uart: UART1 or UART2
 * @param chr: 	The character to be transmitted.
 * @retval void
 */
void hal_uart_putc(TS_UART *uart, uint8_t chr)
{
	uart->DAT_BUFF.TXDB.DATA = chr;
	while (!(uart->STS_FLAG.WORD & UART_STS_FLAG_TXEND_FLAG_Msk));
	uart->STS_FLAG.WORD = UART_STS_FLAG_TXEND_FLAG_Msk;
}

/**
 * @brief Register the interrupt callback function.
 * @param uart UART1 or UART2 of TS_UART type.
 * @param func The callback function to be registered.
 */
void hal_uart_reg_int_cb(TS_UART *uart, void (*func)(void))
{
	if (uart == UART1)
	{
		m_pfn_UART1_RxIntHandler = func;
	}

	if (uart == UART2)
	{
		m_pfn_UART2_RxIntHandler = func;
	}
}

/**
 * @brief Retaget the fputc function to the uart transmit function.
 * @param chr: 	The character to be transmitted.
 * @retval void
 */
void retarget_fputc(uint8_t chr)
{
	hal_uart_putc(DEBUG_PORT, chr);
}



/**
 * @brief UART1 Interrupt handler.
 *        This function handles the UART1 interrupt request. 	
 *        - Here, we check if the RXEND flag is set and call the registered callback function if it is.
 * @param  void
 * @retval void
 * @note This function should be called in the UART1 interrupt handler.
 * 		 The interrupt handler should be registered using the hal_uart_reg_int_cb() function.
 */
void __attribute__((isr)) UART1_IRQHandler(void)
{
	if (UART1->STS_FLAG.WORD & UART_STS_FLAG_RXEND_FLAG_Msk)
	{
		if (m_pfn_UART1_RxIntHandler != NULL)
		{
			m_pfn_UART1_RxIntHandler();
		}
		UART1->STS_FLAG.WORD = UART_STS_FLAG_RXEND_FLAG_Msk;
	}
}

/**
 * @brief UART1 Interrupt handler.
 *        This function handles the UART2 interrupt request. 	
 *        - Here, we check if the RXEND flag is set and call the registered callback function if it is.
 * @param  void
 * @retval void
 * @note This function should be called in the UART2 interrupt handler.
 * 		 The interrupt handler should be registered using the hal_uart_reg_int_cb() function.
 */
void __attribute__((isr)) UART2_IRQHandler(void)
{
	if (UART2->STS_FLAG.WORD & UART_STS_FLAG_RXEND_FLAG_Msk)
	{
		if (m_pfn_UART2_RxIntHandler != NULL)
		{
			m_pfn_UART2_RxIntHandler();
		}
		UART2->STS_FLAG.WORD = UART_STS_FLAG_RXEND_FLAG_Msk;
	}
}
