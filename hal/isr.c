/**
  ******************************************************************************
  * @file    isr.c
  * @brief   Interrupt service routine(ISR) is called by the processor when an interrupt occurs.<br>
  *          This file provides firmware functions to manage the following <br>
  *           - deafaul_IRQHandler() - default <br>
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech.<br>
  * All rights reserved.<br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### Interrupt Service Routine Features #####
  ==============================================================================
  [..] 
  The ISR has the following features:
  (+) defaul_IRQHandler() - default ISR handler for unhandled(issues like NMI, HardFault, etc) interrupt vectors.
  (+) USBPD_IRQHandler() - USBPD interrupt handler.
  (+) UFCS_IRQHandler() - UFCS interrupt handler.
  (+) DPDM_SINK_IRQHandler() - DPDM SINK interrupt handler.
  (+) DCP_HVDCP_IRQHandler() - DCP HVDCP interrupt handler.
  (+) QC_SRC_IRQHandler() - QC SRC interrupt handler.
  (+) AFC_SCP_SRC_IRQHandler() - AFC SCP SRC interrupt handler.
  (+) TCPC_IRQHandler() - TCPC interrupt handler.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) When an interrupt occurs, the corresponding ISR is called by the processor.
    (#) The ISR handler should be implemented in the corresponding file (isr.c) and 
		should be declared as an ISR using the __attribute__((isr)) keyword.
    (#) The ISR handler should be defined as void __attribute__((isr)) ISR_Handler(void) 
		and should be placed in the interrupt vector table (IVT).
    (#) The ISR handler should call the corresponding function to handle the interrupt.
    (#) The ISR handler should not return any value.
	  (#) When an abnormal issue(like NMI, HardFault, stack overflow, etc) occurs, 
		the system will go to default_IRQHandler() to handle the interrupt.
	
  @endverbatim
  ******************************************************************************
  */ 
#include "typdef.h"
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "tcpc.h"
#include "isr.h"

void __attribute__((isr)) default_IRQHandler(uint32_t vector, uint32_t pc, uint32_t sp)
{
	printk("\r\n IRQn-> default_exception_handler: %d %x %x", vector, pc, sp);
	while (1);
}

void __attribute__((isr)) UFCS_IRQHandler(void)
{
}

void __attribute__((isr)) TCPC_IRQHandler(void)
{
}


