/**
 * @file    wb7720_misc.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the miscellaneous
 *          firmware library functions (add-on to CMSIS functions).
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_MISC_H
#define __WB7720_MISC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup MISC
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void SysTick_DelayUs(uint32_t us);
void SysTick_DelayMs(uint32_t ms);
void SysTick_DelayNticks(uint32_t n_ticks);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_MISC_H */
