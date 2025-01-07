/**
  ******************************************************************************
  * @file    epwm.c
  * @brief   This file provides firmware functions to manage the functionalities<br>
  *          of the Enhanced Pulse Width Modulation (EPWM) peripheral:<br>
  *           - PWM Start and Stop functions
  *           - PWM Update functions
  *           - AFD Start and Stop functions
  ******************************************************************************
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
                    ##### EPWM Peripheral features #####
  ==============================================================================
  [..]
  (#) Enhanced Pulse Width Modulation (EPWM) allows for precise control of 
      PWM signals with configurable parameters.
  (#) Supports duty cycle and phase angle adjustments.
  (#) Enables multiple output channels for concurrent signal generation.
  (#) Integration with other peripheral features for advanced functionalities.
  
                    *** How to use this driver ***
  ==============================================================================
  [..]
  (#) Start the PWM by calling hal_epwm_pwm_start() with the desired 
      parameters for period, duty cycle, and phase angle.
  (#) Update the PWM configuration using hal_epwm_pwm_update() to adjust 
      duty cycle and phase without restarting.
  (#) Stop the PWM using hal_epwm_pwm_stop() when the output is no longer needed.

                    *** Execution of EPWM operations ***
  ==============================================================================
  [..]
  (#) The driver can operate in the following modes:
  
     *** PWM Start ***
     ============================
     [..]
       (+) Start the EPWM peripheral using hal_epwm_pwm_start().
       (+) Specify the period, duty cycle, and phase angle.
  
     *** PWM Update ***
     ============================
     [..]
       (+) Update the existing PWM settings with hal_epwm_pwm_update().
       (+) Adjust parameters dynamically without stopping the PWM.
  
     *** PWM Stop ***
     ============================
     [..]
       (+) Stop the EPWM output using hal_epwm_pwm_stop().
  
     *** AFD (Automatic Frequency Detection) Start ***
     ============================
     [..]
       (+) Start AFD functionality with hal_epwm_afd_start() specifying cycle 
           and count parameters.
  
     *** AFD Stop ***
     ============================
     [..]
       (+) Stop AFD functionality using hal_epwm_afd_stop().

  @endverbatim
*/

/* Includes ------------------------------------------------------------------*/
#include "regdef.h"
#include "printk.h"
#include "epwm.h"

/* Function Definitions -----------------------------------------------------*/

/**
  * @brief  Starts the EPWM output.
  * @param  epwm Pointer to the EPWM instance.
  * @param  perd_cycle Period of the PWM signal.
  * @param  duty_ratio Duty cycle ratio.
  * @param  phas_angle Phase angle.
  * @retval void
  */
void hal_epwm_pwm_start(TS_EPWM *epwm, uint16_t perd_cycle, uint16_t duty_ratio, uint16_t phas_angle)
{
    hal_epwm_pwm_update(epwm, perd_cycle, duty_ratio, phas_angle);
    epwm->PWM_CTRL.WORD = (_EPWM_OUT_NORMAL_PWM << EPWM_PWM_CTRL_CH1_OUT_Pos) |
                          (_EPWM_OUT_NORMAL_PWM << EPWM_PWM_CTRL_CH0_OUT_Pos) |
                          EPWM_PWM_CTRL_EPWM_EN_Msk;
}

/**
  * @brief  Updates the EPWM output settings.
  * @param  epwm Pointer to the EPWM instance.
  * @param  perd_cycle Period of the PWM signal.
  * @param  duty_ratio Duty cycle ratio.
  * @param  phas_angle Phase angle.
  * @retval void
  */
void hal_epwm_pwm_update(TS_EPWM *epwm, uint16_t perd_cycle, uint16_t duty_ratio, uint16_t phas_angle)
{
    TS_EPWM_PWM_PERD perd_ctrl;
    TS_EPWM_PWM_DUTY duty_ctrl;

    uint16_t duty_cycle = duty_ratio * perd_cycle / 500;
    uint16_t phas_shift = phas_angle * perd_cycle / 360;

    if (duty_cycle < 1) duty_cycle = 1;
    if (phas_shift < 1) phas_shift = 1;

    perd_ctrl.WORD = ((phas_shift - 1) << EPWM_PWM_PERD_PWM_PHAS_Pos) | ((perd_cycle - 1) << EPWM_PWM_PERD_PWM_PERD_Pos);
    if (duty_cycle > (perd_cycle >> 1))
    {
    	duty_ctrl.WORD = ((duty_cycle - (perd_cycle >> 1) - 1) << EPWM_PWM_DUTY_CH1_DUTY_Pos) | (((perd_cycle >> 1) - 1) << EPWM_PWM_DUTY_CH0_DUTY_Pos);
    }
    else
    {
    	duty_ctrl.WORD = (0 << EPWM_PWM_DUTY_CH1_DUTY_Pos) | ((duty_cycle - 1) << EPWM_PWM_DUTY_CH0_DUTY_Pos);
    }
//    duty_ctrl.WORD = (((perd_cycle >> 1) - 1) << EPWM_PWM_DUTY_CH1_DUTY_Pos) | (((perd_cycle >> 1) - 1) << EPWM_PWM_DUTY_CH0_DUTY_Pos);

    /*+++++++++++++++++++++ EPWM design issue workaround +++++++++++++++++++++*/
    if (perd_ctrl.BITS.PWM_PERD != epwm->PWM_PERD.BITS.PWM_PERD && perd_ctrl.BITS.PWM_PERD != 0 && epwm->PWM_PERD.BITS.PWM_PERD != 0)
    {
    	uint16_t perd_min, perd_max, duty_min, duty_max;
    	perd_min = (perd_ctrl.BITS.PWM_PERD > epwm->PWM_PERD.BITS.PWM_PERD) ? epwm->PWM_PERD.BITS.PWM_PERD : perd_ctrl.BITS.PWM_PERD;
    	perd_max = (perd_ctrl.BITS.PWM_PERD > epwm->PWM_PERD.BITS.PWM_PERD) ? perd_ctrl.BITS.PWM_PERD : epwm->PWM_PERD.BITS.PWM_PERD;
    	duty_min = ((perd_min / 2) > perd_ctrl.BITS.PWM_PHAS + 3) ? (perd_min / 2) - perd_ctrl.BITS.PWM_PHAS - 3 : 0;
    	duty_max = ((perd_max / 2) > perd_ctrl.BITS.PWM_PHAS + 1) ? (perd_max / 2) - perd_ctrl.BITS.PWM_PHAS - 1 : 0;
    	if (duty_ctrl.BITS.CH0_DUTY > duty_min && duty_ctrl.BITS.CH0_DUTY < duty_max)
    	{
    		duty_ctrl.BITS.CH0_DUTY = (perd_ctrl.BITS.PWM_PERD < epwm->PWM_PERD.BITS.PWM_PERD) ? duty_min : duty_max;
    		printk("\r\n xxxxxxxxxxxxxxxxxxxxxxxxx");
    	}
    }
    /*--------------------- EPWM design issue workaround ---------------------*/
    
    epwm->PWM_DUTY.WORD = duty_ctrl.WORD;
    epwm->PWM_PERD.WORD = perd_ctrl.WORD;
}

/**
  * @brief  Stops the EPWM output.
  * @param  epwm Pointer to the EPWM instance.
  * @retval void
  */
void hal_epwm_pwm_stop(TS_EPWM *epwm)
{
    epwm->AFD_CTRL.WORD = 0;
    epwm->FSK_CTRL.WORD &= ~EPWM_FSK_CTRL_FSK_EN_Msk;
    epwm->PWM_CTRL.WORD = 0;
    epwm->PWM_PERD.WORD = 0;
    epwm->PWM_DUTY.WORD = 0;
}

/**
  * @brief  Starts the Automatic Frequency Dither (AFD) functionality.
  * @param  epwm Pointer to the EPWM instance.
  * @param  cycle Cycle count for AFD.
  * @param  count Step count for AFD.
  * @retval void
  */
void hal_epwm_afd_start(TS_EPWM *epwm, uint8_t cycle, uint8_t count)
{
    epwm->AFD_CTRL.WORD = (cycle << EPWM_AFD_CTRL_STEP_CNT_Pos) |
                          (count << EPWM_AFD_CTRL_STEP_CYC_Pos) |
                          EPWM_AFD_CTRL_AFD_EN_Msk;
}

/**
  * @brief  Stops the Automatic Frequency Dither (AFD) functionality.
  * @param  epwm Pointer to the EPWM instance.
  * @retval void
  */
void hal_epwm_afd_stop(TS_EPWM *epwm)
{
    epwm->AFD_CTRL.WORD = 0;
}
