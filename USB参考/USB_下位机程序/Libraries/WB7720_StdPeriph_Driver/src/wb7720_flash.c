/**
 * @file    wb7720_flash.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the FLASH firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_flash.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup FLASH
  * @brief FLASH driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup FLASH_Private_Functions
  * @{
  */

/**
 * @brief  Unlocks the FLASH controller.
 * @return None
 */
void FLASH_Unlock(void)
{
  FLASH->KEYR = 0x45670123;
  FLASH->KEYR = 0xCDEF89AB;
}

/**
 * @brief  Locks the FLASH controller.
 * @return None
 */
void FLASH_Lock(void)
{
  FLASH->CR |= FLASH_CR_LOCK;
}

/**
 * @brief  Execute flash command.
 * @param  Cmd: Flash command.
 * @param  Address: The address to be programmed or erased.
 * @param  Data: The data to be programmed.
 * @return 0 - OK,  1 - Failed
 */
uint32_t FLASH_CMD_EXEC(uint32_t Cmd, uint32_t Address, uint32_t Data)
{
  int state, prftbe;
  state = __get_PRIMASK();
  __disable_irq();
  prftbe = 0;
  if (FLASH->ACR & FLASH_ACR_PRFTBE) {
    prftbe = 1;
    FLASH->ACR &= ~FLASH_ACR_PRFTBE;
  }

  FLASH->SR = (FLASH_SR_EOP | FLASH_SR_PGERR);
  FLASH->CR = (FLASH->CR & ~FLASH_CR_CMD_Msk) | (Cmd & FLASH_CR_CMD_Msk);
  FLASH->AR = Address;
  FLASH->DR = Data;
  FLASH->CR |= FLASH_CR_START;
  while (FLASH->SR & FLASH_SR_BSY);
  FLASH->CR &= ~FLASH_CR_CMD_Msk;
  FLASH->SR = FLASH_SR_EOP;

  if (prftbe) {
    FLASH->ACR |= FLASH_ACR_PRFTBE;
  }
  if (!state) {
    __enable_irq();
  }

  if (FLASH->SR & FLASH_SR_PGERR) {
    return 1;
  }
  return 0;
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
