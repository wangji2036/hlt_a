/**
  * @file    Projects/WB7720_StdPeriph_Template/wb7720_it.h
  * @author  Westberry Application Team
  * @version V0.1.1
  * @date    13-January-2025
  * @brief   This file contains the headers of the interrupt handlers.
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_IT_H
#define __WB7720_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_IT_H */
