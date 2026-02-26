/**
 * @file    wb7720_pwr.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the PWR firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_pwr.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup PWR
  * @brief PWR driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup PWR_Private_Functions
  * @{
  */

/**
 * @brief  Checks whether the specified PWR flag is set or not.
 * @param  PWR_FLAG: specifies the flag to check.
 *    This parameter can be one of the following values:
 *      @arg PWR_FLAG_SL: Sleep flag
 *      @arg PWR_FLAG_SP: Stop flag
 * @return The new state of PWR_FLAG (SET or RESET).
 */
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG)
{
  FlagStatus bitstatus = RESET;

  if ((PWR->SR & PWR_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
 * @brief  Clears the PWR's pending flags.
 * @param  PWR_FLAG: specifies the flag to clear.
 *    This parameter can be any combination of the following values:
 *      @arg PWR_FLAG_SL: Sleep flag
 *      @arg PWR_FLAG_SP: Stop flag
 * @return None
 */
void PWR_ClearFlag(uint32_t PWR_FLAG)
{
  PWR->SR &= ~PWR_FLAG;
}

/**
 * @brief  Disable ANCTL register write-protection function.
 * @param  None
 * @return None
 */
void PWR_UnlockANA(void)
{
  /* Unlocks write to ANCTL registers */
  PWR->ANAKEY1 = 0x03;
  PWR->ANAKEY2 = 0x0C;
}

/**
 * @brief  Enable ANCTL register write-protection function.
 * @param  None
 * @return None
 */
void PWR_LockANA(void)
{
  /* Locks write to ANCTL registers */
  PWR->ANAKEY1 = 0x00;
  PWR->ANAKEY2 = 0x00;
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
