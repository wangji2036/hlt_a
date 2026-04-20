/**
  ******************************************************************************
  * @file    sys.c
  * @brief   This file provides code for the PLL(system clock) HAL module driver.<br>
  *          The functions for PLL functions list as bleow: <br>
  *           - Clock initialization function. <br>
  *           - Clock internal interrupt handler, not used and implemented as empty function. <br>
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
                    ##### PLL Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the PLL features when IC datasheet is available.

  [..] 
  The PLL has the following features:
  (+) Select the clock source for the PLL.
  (+) Select the PLL multiplier.
  (+) Select the PLL output frequency range.
  (+) Select the PLL output clock divider.	
  (+) Enable the PLL.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
  (#) Initialize the clock using the hal_sys_init() function to <br>
  (#) Select the clock source and frequency range by calling the hal_sys_init() function.
	
  @endverbatim
  ******************************************************************************
  */ 
#include "regdef.h"
#include "sys.h"
#include"config.h"

/**
 * @brief 	   Clock initialization function for clock source selection and frequency range selection.
 * @details    This function initializes the clock system. 
 * 			   - It sets the CPU clock to 36MHz,
 *             - It sets the PLL source to XTAL, the XTAL pre-divider to 3, 
 *             - And enables the XTAL(external crystal).
 * @param  		void
 * @retval		void
 */
void hal_sys_init(void)
{
#if ONLY7_5W_ENALBE
	SYS->CLK_CTRL.WORD = (_SYS_CPU_CLK_36M << SYS_CLK_CTRL_CPU_CLK_SEL_Pos) | (_SYS_PLL_SRC_HIRC << SYS_CLK_CTRL_PLL_SRC_SEL_Pos);
#else
	SYS->CLK_CTRL.WORD = (_SYS_CPU_CLK_36M << SYS_CLK_CTRL_CPU_CLK_SEL_Pos) | (_SYS_PLL_SRC_XTAL << SYS_CLK_CTRL_PLL_SRC_SEL_Pos) |
			           (_SYS_XTAL_PREDIV_3 << SYS_CLK_CTRL_XTAL_PREDIV_Pos) | SYS_CLK_CTRL_XTAL_EN_Msk;
#endif
//	SYS->CLK_CTRL.WORD = (_SYS_CPU_CLK_36M << SYS_CLK_CTRL_CPU_CLK_SEL_Pos) | (_SYS_PLL_SRC_XTAL << SYS_CLK_CTRL_PLL_SRC_SEL_Pos) |
//			           (_SYS_XTAL_PREDIV_1 << SYS_CLK_CTRL_XTAL_PREDIV_Pos) | SYS_CLK_CTRL_XTAL_EN_Msk;
	SYS->PRO_CTRL.WORD = (_SYS_TSD_THD_125C << SYS_PRO_CTRL_TSD_THD_SEL_Pos) | (_SYS_PVD_THD_V2P4 << SYS_PRO_CTRL_PVD_THD_SEL_Pos) | SYS_PRO_CTRL_PVD_EN_Msk;
}

/**
 * @brief 	   Clock internal interrupt handler.
 * @details    This function is the clock internal interrupt handler. 
 * 			       It is used to handle the clock interrupt. Currently, it is not used.
 * @param  		 void
 * @retval		 void
 */
void __attribute__((isr)) PROT_IRQHandler(void)
{
}
