/**
  ******************************************************************************
  * @file    vic.c
  * @brief   Interrup vector table and interrupt handler for the VIC HAL module driver. <br>
  *          The functionalities of the UART peripheral:<br>
  *           - Initialization and set the priority of each interrupt <br>
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
                    ##### Intertupt Vector Features #####
  ==============================================================================
  [..] 
  The VIC has the following features:
  (+) Enable the interrupt sources
  (+) Set the priority of each interrupt
  (+) Set the interrupt vector table base address

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
  (#) Enable and set the priority of each interrupt using the hal_vic_init() function.
	
  @endverbatim
  ******************************************************************************
  */ 
#include "regdef.h"
#include "vic.h"

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
 * @param  void
 * @retval	void	  
 * @note   This function should be called in the main function after the clocks are enabled.
 */
void hal_vic_init(void)
{
	VIC_vModuleEnable();

	VIC->IPTR = 0x00000000;
	VIC->IABR = 0x00000000;

//	VIC_vEnableIRQ(IRQn_PROT  ); VIC_vSetPriority(IRQn_PROT,   0);
//	VIC_vEnableIRQ(IRQn_WDT   ); VIC_vSetPriority(IRQn_WDT,    3);
	VIC_vEnableIRQ(IRQn_TMR0  ); VIC_vSetPriority(IRQn_TMR0,   3);
	VIC_vEnableIRQ(IRQn_TMR1  ); VIC_vSetPriority(IRQn_TMR1,   1);
	VIC_vEnableIRQ(IRQn_TMR2  ); VIC_vSetPriority(IRQn_TMR2,   3);
	VIC_vEnableIRQ(IRQn_TMR3  ); VIC_vSetPriority(IRQn_TMR3,   3);
//	VIC_vEnableIRQ(IRQn_EADC  ); VIC_vSetPriority(IRQn_EADC,   2);
//	VIC_vEnableIRQ(IRQn_BADC  ); VIC_vSetPriority(IRQn_BADC,   3);
	VIC_vEnableIRQ(IRQn_ECAP1 ); VIC_vSetPriority(IRQn_ECAP1,  2);
	VIC_vEnableIRQ(IRQn_ECAP2 ); VIC_vSetPriority(IRQn_ECAP2,  2);
	VIC_vEnableIRQ(IRQn_ECAP3 ); VIC_vSetPriority(IRQn_ECAP3,  2);
	VIC_vEnableIRQ(IRQn_ECAP4 ); VIC_vSetPriority(IRQn_ECAP4,  2);
	VIC_vEnableIRQ(IRQn_ECAP5 ); VIC_vSetPriority(IRQn_ECAP5,  2);
//	VIC_vEnableIRQ(IRQn_GPIO  ); VIC_vSetPriority(IRQn_GPIO,   3);
	VIC_vEnableIRQ(IRQn_UART1 ); VIC_vSetPriority(IRQn_UART1,  3);
//	VIC_vEnableIRQ(IRQn_UART2 ); VIC_vSetPriority(IRQn_UART2,  3);
	VIC_vEnableIRQ(IRQn_I2CS  ); VIC_vSetPriority(IRQn_I2CS,   3);
//	VIC_vEnableIRQ(IRQn_I2CM  ); VIC_vSetPriority(IRQn_I2CM,   1);
	VIC_vEnableIRQ(IRQn_USBPD ); VIC_vSetPriority(IRQn_USBPD,  0);
//	VIC_vEnableIRQ(IRQn_UFCS  ); VIC_vSetPriority(IRQn_UFCS,   2);
	VIC_vEnableIRQ(IRQn_DPDM_SINK); VIC_vSetPriority(IRQn_DPDM_SINK, 1);
	VIC_vEnableIRQ(IRQn_DCP_HVDCP); VIC_vSetPriority(IRQn_DCP_HVDCP, 1);
	VIC_vEnableIRQ(IRQn_QC_SRC); VIC_vSetPriority(IRQn_QC_SRC, 1);
	VIC_vEnableIRQ(IRQn_AFC_SCP_SRC); VIC_vSetPriority(IRQn_AFC_SCP_SRC, 1);
	VIC_vEnableIRQ(IRQn_FSK1  ); VIC_vSetPriority(IRQn_FSK1,   1);
	VIC_vEnableIRQ(IRQn_FSK2  ); VIC_vSetPriority(IRQn_FSK2,   1);
//	VIC_vEnableIRQ(IRQn_DMA   ); VIC_vSetPriority(IRQn_DMA,    3);
//	VIC_vEnableIRQ(IRQn_TCPC  ); VIC_vSetPriority(IRQn_TCPC,   2);
}
