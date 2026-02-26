/**
 * @file    wb7720_argb.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the ARGB firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_ARGB_H
#define __WB7720_ARGB_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup ARGB
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  ws2812 color data structure
  */
typedef struct
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
} ws2812_color_t;

/* Exported constants --------------------------------------------------------*/

/** @defgroup ARGB_Exported_Constants
  * @{
  */

/* Exported macro ------------------------------------------------------------*/

/** @defgroup ARGB_flags_definition 
  * @{
  */
#define ARGB_FLAG_EMPTY   ((uint8_t)ARGB_STATUS_EMPTY)
#define ARGB_FLAG_FULL    ((uint8_t)ARGB_STATUS_FULL )
#define ARGB_FLAG_GRSTF   ((uint8_t)ARGB_STATUS_GRSTF)

/* Exported functions --------------------------------------------------------*/
void ARGB_DeInit(void);
void ARGB_Init(ARGB_TypeDef* ARGBx, uint8_t Periodcnt, uint8_t T1h_cnt, uint16_t Treset_cnt);
void ARGB_Cmd(ARGB_TypeDef* ARGBx, uint8_t Control_enable, FunctionalState NewState);
FlagStatus ARGB_GetControlStatus(ARGB_TypeDef* ARGBx, uint8_t ARGB_Control);
void ARGB_WriteData(ARGB_TypeDef* ARGBx, uint8_t Data);
void ARGB_WriteResetSignal(ARGB_TypeDef* ARGBx);
FlagStatus ARGB_GetFlagStatus(ARGB_TypeDef* ARGBx, uint8_t ARGB_FLAG);
void ARGB_ClearFlag(ARGB_TypeDef* ARGBx, uint32_t ARGB_FLAG);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_ARGB_H */
