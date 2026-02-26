/**
 * @file    wb7720_flash.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the FLASH firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_FLASH_H
#define __WB7720_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup FLASH
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void FLASH_Unlock(void);
void FLASH_Lock(void);

uint32_t FLASH_CMD_EXEC(uint32_t Cmd, uint32_t Address, uint32_t Data);

/**
 * @brief  Erase a specified FLASH page.
 * @param  Page_Address: The page address to be erased.
 * @return 0 - OK,  1 - Failed
 */
#define FLASH_ErasePage(Page_Address)       FLASH_CMD_EXEC(FLASH_CR_CMD_PER, Page_Address, 0)

/**
 * @brief  Erase main flash memory.
 * @return 0 - OK,  1 - Failed
 */
#define FLASH_EraseBulk()                   FLASH_CMD_EXEC(FLASH_CR_CMD_MER, 0x08000000, 0)

/**
 * @brief  Programs the data to the specified address.
 * @param  Address: The address to be programmed.
 * @param  Data: The data to be programmed.
 * @return 0 - OK,  1 - Failed
 */
#define FLASH_ProgramWord(Address, Data)    FLASH_CMD_EXEC(FLASH_CR_CMD_PG, Address, Data)

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_FLASH_H */
