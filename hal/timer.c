/**
  ******************************************************************************
  * @file    timer.c
  * @brief   Timer HAL module driver. <br>
  *          The functionalities of the Timer peripherals: <br>
  *           - Initialization and de-initialization of the timer. <br>
  *           - Start and stop of the timer. <br>
  *           - Timer interrupt handling. <br>	
  *           - Timer counter read and write. <br>
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
                    ##### Timer Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the Timer features when IC datasheet is available.

  [..] 
  The Timer has the following features:
  (+) 

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Configure the Timer0/1/2/3 using the hal_timer_init() function.
	(#)	Start the timer using the hal_timer_start() function.
	(#)	Stop the timer using the hal_timer_stop() function.
    (#) TIM0 interrupt handling is done in the TMR0_IRQHandler() function for every 250ms.
	(#) TIM1 interrupt handling is done in the TMR1_IRQHandler() function for every 1ms.
	(#) TIM2 interrupt handling is done in the TMR2_IRQHandler() function for every 250ms.
	(#) TIM3 interrupt handling is done in the TMR3_IRQHandler() function for every 1ms.

  @endverbatim
  ******************************************************************************
  */ 
#include "regdef.h"
#include "timer.h"
#include "g_data.h"
#include"led.h"

/**
 * @brief Timer 0/1/2/3 initialization. 
 * 		  You can initialize one of the timer according to your needs.
 * @param timer TIMER0/1/2/3
 * @retval void
 * @note  
 * -#	The timer clock source is LIRC, and the timer clock frequency is 64K.
 * -#	You must call this function before using the timer.
 */
void hal_timer_init(TS_TMR *timer)
{
	if (timer == TMR0)
	{
		timer->LOAD_CNT.WORD = 4000 - 1; //250ms
		timer->SPL_CTRL.WORD = (_TMR_CLK_SRC_LIRC << TMR_SPL_CTRL_CLK_SRC_Pos) | TMR_SPL_CTRL_WKUP_EN_Msk; //LIRC: 64K
		timer->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_ONE_SHOT << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_INT_EN_Msk | TMR_GEN_CTRL_CNT_EN_Msk; //16K
	}

	if (timer == TMR1)
	{
		timer->LOAD_CNT.WORD = 9000 - 1; //1ms
		timer->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_PERIODIC << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_INT_EN_Msk | TMR_GEN_CTRL_CNT_EN_Msk; //9MHz
	}

	if (timer == TMR2)
	{
		timer->LOAD_CNT.WORD = 1125 * 250 - 1; //250ms
		timer->GEN_CTRL.WORD = (5 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_PERIODIC << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_INT_EN_Msk | TMR_GEN_CTRL_CNT_EN_Msk;; //1.125MHz
	}

	if (timer == TMR3)
	{
		timer->LOAD_CNT.WORD = 9000 - 1; //1ms
		timer->GEN_CTRL.WORD = (2 << TMR_GEN_CTRL_CLK_PSC_Pos) | (_TMR_OP_MODE_PERIODIC << TMR_GEN_CTRL_OP_MODE_Pos) | TMR_GEN_CTRL_INT_EN_Msk | TMR_GEN_CTRL_CNT_EN_Msk; //9MHz
	}
}

/**
 * @brief Stops the input timer.
 * @param timer TS_TMR
 * @retval void
 */
void hal_timer_stop(TS_TMR *timer)
{
	timer->GEN_CTRL.WORD &= ~TMR_GEN_CTRL_CNT_EN_Msk;
}

/**
 * @brief TM0 interrupt handler.
 * @param 	void
 * @retval	void
 * @todo This function is not implemented yet.
 */
void __attribute__((isr)) TMR0_IRQHandler(void) //250ms
{
}

volatile uint8_t g_u8Tmr0IntHaved_USBPD;
volatile uint16_t g_u16Tmr0IntCnt_USBPD;
volatile uint8_t  g_u8Tmr0IntHaved;
volatile uint16_t g_u16Tmr0IntCnt;

volatile uint16_t sys_ticks;
/**
 * @brief	TIM1 interrupt handler.
 * @note	This function is called every 1ms.
 * @param  	void
 * @return 	void
 */
void __attribute__((isr)) TMR1_IRQHandler(void) //1ms
{
	sys_ticks++;

	if (g_u8Tmr0IntHaved)
	{
		++g_u16Tmr0IntCnt;
	}
	else
	{
		g_u8Tmr0IntHaved = 1;
		g_u16Tmr0IntCnt  = 1;
	}

	if (g_u8Tmr0IntHaved_USBPD)
	{
		++g_u16Tmr0IntCnt_USBPD;
	}
	else
	{
		g_u8Tmr0IntHaved_USBPD = 1;
		g_u16Tmr0IntCnt_USBPD  = 1;
	}

//	GPA->DOUT.BITS.PIN4 ^= 1;
//	GPA->DOUT.BITS.PIN5 ^= 1;

	//ui_display();
}

/**
 * @brief 	TMI2 interrupt handler.
 * @note	This function is called every 250ms setted by TMR2.
 * @param  	void
 * @return 	void
 */
volatile uint8_t tmr2_250ms_int_flag;
void __attribute__((isr)) TMR2_IRQHandler(void) //250ms
{
	tmr2_250ms_int_flag++;
}

/**
 * @brief 	TMI3 interrupt handler.
 * 				
 * @note	This function is called every 1ms setted by TMR3.
 * @param  	void
 * @retval 	void
 * @todo 	Update the details for this function.
 */
void __attribute__((isr)) TMR3_IRQHandler(void)
{
	if (gd->sys_infos.tim3_evnt & 1)//duty ramp up
	{
		if (gd->pid_duty < gd->dig_ping_duty)
		{
			gd->pid_duty += 50;
			if (gd->pid_duty >= gd->dig_ping_duty)
			{
				gd->pid_duty = gd->dig_ping_duty;
				gd->sys_infos.tim3_evnt &= ~1;
			}
			hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
		}
	}

	if (gd->sys_infos.tim3_evnt & 4)//fsk response
	{
		if (gd->fsk_silence)
		{
			if (--gd->fsk_silence == 0)
			{
				EPWM1->FSK_CTRL.WORD |= EPWM_FSK_CTRL_FSK_EN_Msk;
				gd->sys_infos.tim3_evnt &= ~4;
//				GPA->DOUT.BITS.PIN4 ^= 1;
			}
		}
	}

	if (gd->sys_infos.tim3_evnt == 0)
	{
		hal_timer_stop(TMR3);
	}
}
