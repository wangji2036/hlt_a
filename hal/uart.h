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
#ifndef UART_H_
#define UART_H_

/**
 * @brief Initialize the UART. Mason 17111 have two UARTs, UART1 and UART2. 
 * 		-	The UART is configured as a USCI_A0 module.
 * 		-	The baud rate is set to 250K.	
 * @param uart UART1 or UART2
 * @return None
 * @note This function should be called before using the UART.
 */
void hal_uart_init(TS_UART *uart);

/**
 * @brief Uart transmit a character.
 * @param uart: UART1 or UART2
 * @param chr: 	The character to be transmitted.
 * @retval None
 */
void hal_uart_putc(TS_UART *uart, uint8_t chr);


/**
 * @brief Register the interrupt callback function.
 * @param uart UART1 or UART2 of TS_UART type.
 * @param func The callback function to be registered.
 */
void hal_uart_reg_int_cb(TS_UART *uart, void (*func)(void));

#endif /* UART_H_ */
