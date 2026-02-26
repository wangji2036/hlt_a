/**
 * @file    wb7720_misc.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the miscellaneous firmware functions (add-on
 *          to CMSIS functions).
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_misc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup MISC 
  * @brief MISC driver modules
  * @{
  */

/** @defgroup MISC_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */ 

/** @defgroup MISC_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup MISC_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup MISC_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup MISC_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup MISC_Private_Functions
  * @{
  */

/**
 * @brief  Delay n ticks.
 * @param  n_ticks: Number of ticks to delay.
 * @note   Since SysTick_LOAD is a 24-bit register, 
 *         the n_ticks must be less than 16777216
 * @return None
 */
void SysTick_DelayNticks(uint32_t n_ticks)
{
  uint32_t status;
  /* Configure the reload value */
  SysTick->LOAD = n_ticks;
  /* Clear the counter */
  SysTick->VAL = 0x00;
  /* The core clock as clock source and enable SysTick */
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
  /* Wait for count to 0 */
  do {
    status = SysTick->CTRL;
  } while((status & SysTick_CTRL_ENABLE_Msk) && (!(status & SysTick_CTRL_COUNTFLAG_Msk)));
  /* Disable SysTick */
  SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/**
 * @brief  Delay n us.
 * @param  ms: Number of us to delay.
 * @note   None
 * @return None
 */
void SysTick_DelayUs(uint32_t us)
{
  uint32_t status;

  while (us--)
  {
    /* Configure the reload value */
    SysTick->LOAD = SystemCoreClock / 1000000;
    /* Clear the counter */
    SysTick->VAL = 0x00;
    /* The core clock as clock source and enable SysTick */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    /* Wait for count to 0 */
    do {
      status = SysTick->CTRL;
    } while((status & SysTick_CTRL_ENABLE_Msk) && (!(status & SysTick_CTRL_COUNTFLAG_Msk)));
    /* Disable SysTick */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  }
}

/**
 * @brief  Delay n ms.
 * @param  ms: Number of ms to delay.
 * @note   None
 * @return None
 */
void SysTick_DelayMs(uint32_t ms)
{
  uint32_t status;

  while (ms--)
  {
    /* Configure the reload value */
    SysTick->LOAD = SystemCoreClock / 1000;
    /* Clear the counter */
    SysTick->VAL = 0x00;
    /* The core clock as clock source and enable SysTick */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    /* Wait for count to 0 */
    do {
      status = SysTick->CTRL;
    } while((status & SysTick_CTRL_ENABLE_Msk) && (!(status & SysTick_CTRL_COUNTFLAG_Msk)));
    /* Disable SysTick */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
  }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
