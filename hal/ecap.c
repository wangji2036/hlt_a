/**
  ******************************************************************************
  * @file    ecap.c
  * @brief   This file provides firmware functions to manage the functionalities<br>
  *          of the Enhanced Capture (ECAP) peripheral:<br>
  *           - ECAP Initialization and Configuration
  *           - Callback Registration for Interrupts
  *           - ECAP Open and Close functions
  *           - Interrupt Handlers for ECAP channels
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
                      ##### ECAP Peripheral features #####
  ==============================================================================
  [..]
  (#) Enhanced Capture (ECAP) allows for precise measurement of input signals
      through configurable modes and interrupt handling.
  (#) Supports different functional modes including Dual-Mode and QDT.
  (#) Enables edge detection and overflow notifications through interrupts.
  
                      *** How to use this driver ***
  ==============================================================================
  [..]
  (#) Initialize the ECAP peripheral using hal_ecap_init() with the desired 
      functional mode.
  (#) Register a callback function for edge detection interrupts using 
      hal_ecap_reg_int_cb().
  (#) Enable or disable the capture functionality with hal_ecap_open() and 
      hal_ecap_close().
  
                      *** Execution of ECAP operations ***
  ==============================================================================
  [..]
  (#) The driver can operate in the following modes:
  
     *** Initialization ***
     ============================
     [..]
       (+) Call hal_ecap_init() to set up the ECAP peripheral.
       (+) Choose between DDM and QDT modes.
  
     *** Register Callback ***
     ============================
     [..]
       (+) Register a callback function using hal_ecap_reg_int_cb() to handle 
           edge detection events.
  
     *** Capture Open ***
     ============================
     [..]
       (+) Start capturing events by calling hal_ecap_open().
  
     *** Capture Close ***
     ============================
     [..]
       (+) Stop capturing events by calling hal_ecap_close().

     *** Interrupt Handling ***
     ============================
     [..]
       (+) The ECAP interrupt handlers will call the registered callback 
           functions upon edge detection.
  
  @endverbatim
*/

/* Includes ------------------------------------------------------------------*/
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "ecap.h"
#include "ask.h"

/* Typedefs and Macros ------------------------------------------------------*/
typedef void (*pfn_ecap_cb)(uint8_t, uint16_t);
static volatile pfn_ecap_cb ecap_callback = NULL;

/* Function Definitions -----------------------------------------------------*/

/**
  * @brief  Registers a callback function for ECAP interrupts.
  * @param  func: Pointer to the callback function.
  * @retval void
  */
void hal_ecap_reg_int_cb(void (*func)(uint8_t, uint16_t))
{
	ecap_callback = func;
}

/**
  * @brief  Initializes the ECAP peripheral.
  * @param  ecap: Pointer to the ECAP instance.
  * @param  mode: Operating mode of the ECAP.
  * @retval void
  */
void hal_ecap_init(TS_ECAP *ecap, enum ECAP_FUNC_MODE mode)
{
	if (mode == _ECAP_FUNC_MODE_DDM)
	{
		ecap->OVER_CNT.WORD = (1125 * 2 << ECAP_OVER_CNT_OVERFLOW_CNT_Pos);
		ecap->DDM_CTRL.WORD = (_ECAP_DDMCAP_TIMER_CLKDIV_32 << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos) | (_ECAP_EDGE_TYPE_RISEING_FALLING << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos);
		ecap->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_DDM << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_EXT_PAD_PIN << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
				          (_ECAP_DEGILTC_TIME_80us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos) | ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk | ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk;
		ecap->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;
	}
	else
	{
		if (ecap != ECAP4)
		{
			if (ecap == ECAP1 || ecap == ECAP5)
			{
				ecap->QDT_CTRL.WORD = (_ECAP_QDT_VDM_PLUSE_END_TYPE_LOW << ECAP_QDT_CTRL_VDM_PLUSE_END_TYPE_Pos);
			}
			else if (ecap == ECAP2 || ecap == ECAP3)
			{
				ecap->QDT_CTRL.WORD = (0 << ECAP_QDT_CTRL_NQM_SKIP_TIMES_SET_Pos) | (4 << ECAP_QDT_CTRL_NQM_MEAS_TIMES_SET_Pos);
			}
			ecap->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_QDT << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_EXT_PAD_PIN << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
					          (_ECAP_DEGILTC_TIME_00us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos);
			ecap->STS_FLAG.WORD = ECAP_STS_FLAG_QDT_DONE_FLAG_Msk;
		}
	}
}


void hal_ecap_dig_ddm_init(void)
{
	ECAP2->OVER_CNT.WORD = (1125 * 2 << ECAP_OVER_CNT_OVERFLOW_CNT_Pos);
	ECAP2->DDM_CTRL.WORD = (_ECAP_DDMCAP_TIMER_CLKDIV_32 << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos) | (_ECAP_EDGE_TYPE_RISEING_FALLING << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos);
	ECAP2->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_DDM << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_INR_DDM_OUT << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
			          (_ECAP_DEGILTC_TIME_80us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos) | ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk | ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk;
	ECAP2->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;

	ECAP4->OVER_CNT.WORD = (1125 * 2 << ECAP_OVER_CNT_OVERFLOW_CNT_Pos);
	ECAP4->DDM_CTRL.WORD = (_ECAP_DDMCAP_TIMER_CLKDIV_32 << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos) | (_ECAP_EDGE_TYPE_RISEING_FALLING << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos);
	ECAP4->GEN_CTRL.WORD = (_ECAP_FUNC_MODE_DDM << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos) | (_ECAP_INPUT_CHAN_INR_DDM_OUT << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos) |
			          (_ECAP_DEGILTC_TIME_80us << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos) | ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk | ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk;
	ECAP4->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;

	hal_ecap_open(ECAP2);
	hal_ecap_open(ECAP4);
}

/**
  * @brief  Opens the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_open(TS_ECAP *ecap)
{
	ecap->GEN_CTRL.WORD |= ECAP_GEN_CTRL_CAP_EN_Msk;
}

/**
  * @brief  Closes the ECAP capture functionality.
  * @param  ecap: Pointer to the ECAP instance.
  * @retval void
  */
void hal_ecap_close(TS_ECAP *ecap)
{
	/*+++++++++++++++++++++ ECAP design issue workaround +++++++++++++++++++++*/
	if ((ecap == ECAP1 || ecap == ECAP5) && ecap->GEN_CTRL.BITS.FUNC_WORKING_MODE == _ECAP_FUNC_MODE_DDM)
	{
		//this will have an impact on the QDT function after EPWM power remove
		delay_1us(100); //at least 80us
	}
	/*--------------------- ECAP design issue workaround ---------------------*/

	ecap->GEN_CTRL.WORD &= ~ECAP_GEN_CTRL_CAP_EN_Msk;
	ecap->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk | ECAP_STS_FLAG_OVERFLOW_FLAG_Msk;

	/*+++++++++++++++++++++ ECAP design issue workaround +++++++++++++++++++++*/
	if ((ecap == ECAP1 || ecap == ECAP5) && ecap->GEN_CTRL.BITS.FUNC_WORKING_MODE == _ECAP_FUNC_MODE_DDM)
	{
		//this will have an impact on the QDT function after EPWM power remove
		delay_1us(100); //at least 80us
	}
	/*--------------------- ECAP design issue workaround ---------------------*/
}

/**
  * @brief  ECAP1 Interrupt Handler.
  * @retval void
  */
void __attribute__((isr)) ECAP1_IRQHandler(void)
{
	if (ECAP1->STS_FLAG.WORD & ECAP_STS_FLAG_EDGE_DET_FLAG_Msk)
	{
		GPA->DOUT.BITS.PIN4 ^= 1;//debug toggle SDA pin
		if (ecap_callback != NULL)
		{
			ecap_callback(0, ECAP1->EDGE_CNT.BITS.EDGE_DET_CNT);
		}
		ECAP1->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk;
	}
}

/**
  * @brief  ECAP2 Interrupt Handler.
  * @retval void
  */
void __attribute__((isr)) ECAP2_IRQHandler(void)
{
	if (ECAP2->STS_FLAG.WORD & ECAP_STS_FLAG_EDGE_DET_FLAG_Msk)
	{
		GPC->DOUT.BITS.PIN6 ^= 1;//debug toggle SCL pin
		if (ecap_callback != NULL)
		{
			ecap_callback(1, ECAP2->EDGE_CNT.BITS.EDGE_DET_CNT);
		}
		ECAP2->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk;
	}
}

/**
  * @brief  ECAP4 Interrupt Handler.
  * @retval void
  */
void __attribute__((isr)) ECAP4_IRQHandler(void)
{
	if (ECAP4->STS_FLAG.WORD & ECAP_STS_FLAG_EDGE_DET_FLAG_Msk)
	{
//		GPA->DOUT.BITS.PIN5 ^= 1;
//		GPC->DOUT.BITS.PIN7 ^= 1;
		if (ecap_callback != NULL)
		{
			ecap_callback(2, ECAP4->EDGE_CNT.BITS.EDGE_DET_CNT);
		}
		ECAP4->STS_FLAG.WORD = ECAP_STS_FLAG_EDGE_DET_FLAG_Msk;
	}
}

/**
  * @brief  ECAP3 Interrupt Handler.
  * @retval void
  */
void __attribute__((isr)) ECAP3_IRQHandler(void)
{
    // Handle ECAP3 interrupt if needed
}

/**
  * @brief  ECAP5 Interrupt Handler.
  * @retval void
  */
void __attribute__((isr)) ECAP5_IRQHandler(void)
{
    // Handle ECAP5 interrupt if needed
}
