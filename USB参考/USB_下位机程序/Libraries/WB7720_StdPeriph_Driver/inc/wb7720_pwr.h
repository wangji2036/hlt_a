/**
 * @file    wb7720_pwr.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the PWR firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_PWR_H
#define __WB7720_PWR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup PWR
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup PWR_Exported_Constants 
  * @{
  */

/** @defgroup PWR_entry_mode 
  * @{
  */
#define PWR_EntryMode_WFI         ((uint8_t)0x01)
#define PWR_EntryMode_WFE         ((uint8_t)0x02)
/**
  * @}
  */



/** @defgroup PWR_Flag 
  * @{
  */
#define PWR_FLAG_SL     ((uint32_t)0x00000001)
#define PWR_FLAG_SP     ((uint32_t)0x00000002)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG);
void PWR_ClearFlag(uint32_t PWR_FLAG);
void PWR_UnlockANA(void);
void PWR_LockANA(void);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_PWR_H */
