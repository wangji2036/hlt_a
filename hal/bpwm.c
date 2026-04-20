/**
  ******************************************************************************
  * @file    bpwm.c
  * @brief   bpwm HAL module driver.<br>
  *          This file provides firmware functions to manage the functionalities<br>
  *          of the Basic PWM (BPWM) peripheral:<br>
  *           - BPWM Initialization and Configuration
  *           - BPWM Start, Update, and Stop functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech.<br>
  * All rights reserved.<br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS. <br>
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### BPWM Peripheral features #####
  ==============================================================================
  [..] 
  (#) The Basic PWM (BPWM) allows for generating PWM signals with configurable duty cycles and periods.
    (++) PERD data determines the PWM period.
    (++) PWM frequency = BPWM_CLK/(PERD+1)
    (++) [30:16] |DUTY |BPWMx duty cycle counter register
    (++) BPWMx duty ratio = (DUTY+1)/(PERD+1)
    (++) DUTY >= PERD: PWM output is always high
    (++) DUTY < PERD: PWM low width = (PERD-DUTY+1) unit; PWM high width = (DUTY) unit.
    (++) DUTY = 0: PWM low width = (PERD+1) unit; PWM high width = 0 unit.
    (++) DUTY = 0: PWM low width = (PERD+1) unit; PWM high width = 0 unit.

                     ##### How to use this driver #####
  ==============================================================================  
  [...]
    (#) Configure BADC startup using hal_bpwm_start().
        (++) Configure bpwm mode using the ¡°bpwm¡± member of the TS_BPWM structure.
        (++) Use perd_cycle to configure the time value of the bpwm cycle.
	[...]
    (#) Update BADC using hal_bpwm_update().
      (++) Use perd_cycle to configure the time value of the bpwm cycle.
      (++) Use duty_cycle to configure the time value of the bpwm duty cycle.
	[...]
    (#) Stop the BPWM using hal_bpwm_stop() when no longer needed.
      (++) Stop the BPWM module.
  @endverbatim
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "regdef.h"
#include "bpwm.h"

/* Function Definitions -----------------------------------------------------*/

/**
 * @brief Initializes the BPWM peripheral according to the specified parameters * in the bpwm.
 *  @param bpwm Pointer to an instance of the BPWM module.
 *  @param perd_cycle The time value of the cycle.
 *  @param duty_cycle The time value of the duty cycle.
 *  @note This function should be called before hal_bpwm_start() to initialize the BPWM module.
 *  @retval void
 */
void hal_bpwm_start(TS_BPWM *bpwm, uint16_t perd_cycle, uint16_t duty_cycle)
{
	hal_bpwm_update(bpwm, perd_cycle, duty_cycle);
	bpwm->GEN_CTRL.WORD = (_BPWM_CLK_SRC_HCLK << BPWM_GEN_CTRL_CLK_SRC_Pos) | (_BPWM_MODE_PERIODIC << BPWM_GEN_CTRL_MODE_Pos) | BPWM_GEN_CTRL_EN_Msk;
}

/**
 *  @brief Updates the BPWM settings for the specified period and duty cycle.
 *  @note This function should be called after a call to hal_bpwm_start() to update the parameters of the BPWM module.
 *  @param bpwm Pointer to the structure of the BPWM module.
 *  @param perd_cycle The length of the cycle, in units that can be the number of clock cycles.
 *  @param duty_cycle The duty cycle, which indicates how long the signal will be high during a cycle, in clock cycles.
 *  @retval void
 */
void hal_bpwm_update(TS_BPWM *bpwm, uint16_t perd_cycle, uint16_t duty_cycle)
{
	if (perd_cycle < 1) perd_cycle = 1;
	if (duty_cycle < 1) duty_cycle = 1;

	bpwm->PWM_CTRL.WORD = ((duty_cycle - 1) << BPWM_PWM_CTRL_DUTY_Pos) | ((perd_cycle - 1) << BPWM_PWM_CTRL_PERD_Pos);
}

/**
 *  @brief Stops the BPWM operation.
 *  @note This function should be called after hal_bpwm_update() to stop the BPWM module.
 *  @param bpwm Pointer to the structure of the BPWM module.
 *  @retval void
 */
void hal_bpwm_stop(TS_BPWM* bpwm)
{
	bpwm->GEN_CTRL.WORD &= ~BPWM_GEN_CTRL_EN_Msk;
}
