
/**
  ******************************************************************************
  * @file    wdt.c
  * @brief  
  *   This file provides code for the watchdog timer (WDT) module driver.<br>
  *   The functions for this module are as list as bleow: <br>
  *           - Wacthdog timer initialization function. <br>
  *           - Clock internal interrupt handler, not used and implemented as empty function. <br>
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS. <br>
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### WDT Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the WDT features when IC datasheet is available.

  [..] 
  The WDT (Watchdog Timer) has the following features:
  (+) Enable the watchdog timer.
  (+) Select the watchdog timer timeout.
  (+) Enable the watchdog timer reset.
  (+) Enable the watchdog timer clock interrupt.
  (+) Enable the watchdog timer load.
  (+) Feed the watchdog timer to prevent a system reset.
  (+) Stop the watchdog timer.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
  (#) Enable and configuring the watchdog timer timeout to 1000ms with hal_wdt_init.
  (#) Feed the watchdog timer to prevent a system reset with hal_wdt_feed.
  (#) Stop the watchdog timer with hal_wdt_stop.
	
  @endverbatim
  ******************************************************************************
*/ 


#include "regdef.h"
#include "wdt.h"

#define WDT_TIMEOUT    (64 * 1000 - 1) //64000 * 0.015625 = 1000ms
#define WDT_TIMEOUT_5MS    (64 * 5 - 1) //

/**
 * @brief 	Enable and configure the Watchdog timer timeout to 1000ms(1 second).
 * @details This function initializes the watchdog timer with a timeout of 1000ms(1 second). 
 * 			- If the watchdog timer is not fed within this time, the system will reset.
 * @param  void
 * @retval void
 */
void hal_wdt_init(void) //64K
{

	if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
	{
		WDT->CTRL.WORD = (WDT_TIMEOUT << WDT_CTRL_WDT_CNT_Pos) | WDT_CTRL_RST_EN_Msk | WDT_CTRL_LOAD_EN_Msk | WDT_CTRL_MODU_EN_Msk;
	}
}
void hal_wdt_init_to_reset(void) //64K
{
	if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
	{
		WDT->CTRL.WORD = (WDT_TIMEOUT_5MS << WDT_CTRL_WDT_CNT_Pos) | WDT_CTRL_RST_EN_Msk | WDT_CTRL_LOAD_EN_Msk | WDT_CTRL_MODU_EN_Msk;
	}
}

/**
 * @brief 	Watchdog timer feed function.
 * @details This function feeds the watchdog timer to prevent a system reset.
 * @param  void
 * @retval void
 */
void hal_wdt_feed(void)
{
	if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
	{
		WDT->CTRL.WORD |= WDT_CTRL_LOAD_EN_Msk;
	}
}


/**
 * @brief 	Stop the watchdog timer.
 * @details This function stops the watchdog timer.
 * @param  	void
 * @retval 	void
 */
void hal_wdt_stop(void)
{
	WDT->CTRL.WORD = 0;
}


/**
 * @cond HIDDEN_SYMBOLS
*/
/**
 * @brief 	Watchdog timer clock internal interrupt handler.
 * @details This function is not used and implemented as empty function.
 * @param  	void
 * @retval 	void
 */
void __attribute__((isr)) WDT_IRQHandler(void)
{
}
