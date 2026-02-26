/**
 * @file    wb7720_pct.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the PCT firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_PCT_H
#define __WB7720_PCT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup PCT
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup PCT_Exported_Constants 
  * @{
  */

/** @defgroup PCT_Prescaler 
  * @{
  */
#define PCT_Prescaler_4       ((uint8_t)PCT_CFGR_PSC_DIV4)
#define PCT_Prescaler_8       ((uint8_t)PCT_CFGR_PSC_DIV8)
#define PCT_Prescaler_12      ((uint8_t)PCT_CFGR_PSC_DIV12)
#define PCT_Prescaler_16      ((uint8_t)PCT_CFGR_PSC_DIV16)
#define PCT_Prescaler_24      ((uint8_t)PCT_CFGR_PSC_DIV24)
#define PCT_Prescaler_32      ((uint8_t)PCT_CFGR_PSC_DIV32)
#define PCT_Prescaler_64      ((uint8_t)PCT_CFGR_PSC_DIV64)
/**
  * @}
  */


/** @defgroup PCT_flags_definition 
  * @{
  */
#define PCT_FLAG_OVF    ((uint8_t)0x80)
#define PCT_FLAG_CC0    ((uint8_t)0x01)
#define PCT_FLAG_CC1    ((uint8_t)0x02)
#define PCT_FLAG_CC2    ((uint8_t)0x04)
#define PCT_FLAG_CC3    ((uint8_t)0x08)
#define PCT_FLAG_CC4    ((uint8_t)0x10)
#define PCT_FLAG_CC5    ((uint8_t)0x20)
/**
  * @}
  */


/** @defgroup PCT_interrupts_definition
  * @{
  */
#define PCT_IT_OVF      ((uint8_t)0x80)
#define PCT_IT_CC0      ((uint8_t)0x01)
#define PCT_IT_CC1      ((uint8_t)0x02)
#define PCT_IT_CC2      ((uint8_t)0x04)
#define PCT_IT_CC3      ((uint8_t)0x08)
#define PCT_IT_CC4      ((uint8_t)0x10)
#define PCT_IT_CC5      ((uint8_t)0x20)
/**
  * @}
  */


/** @defgroup PCT_Capture_and_Compare_modes_definition
  * @{
  */
#define PCT_CCMode_Disable              ((uint8_t)0x00)
#define PCT_CCMode_PWM1                 ((uint8_t)(PCT_CCMR_ECOM | PCT_CCMR_PWM | PCT_CCMR_MAT))
#define PCT_CCMode_PWM2                 ((uint8_t)(PCT_CCMR_ECOM | PCT_CCMR_PWM | PCT_CCMR_MAT | PCT_CCMR_POL))
#define PCT_CCMode_Match                ((uint8_t)(PCT_CCMR_ECOM | PCT_CCMR_MAT))
#define PCT_CCMode_Toggle               ((uint8_t)(PCT_CCMR_ECOM | PCT_CCMR_MAT | PCT_CCMR_TOG))
#define PCT_CCMode_Capture_Rising       ((uint8_t)(PCT_CCMR_CAPP))
#define PCT_CCMode_Capture_Falling      ((uint8_t)(PCT_CCMR_CAPN))
#define PCT_CCMode_Capture_BothEdge     ((uint8_t)(PCT_CCMR_CAPP | PCT_CCMR_CAPN))
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/* PCT base management ********************************************************/
void PCT_DeInit(PCT_TypeDef* PCTx);
void PCT_TimeBaseInit(PCT_TypeDef* PCTx, uint8_t PCT_Prescaler, uint16_t Autoreload);
void PCT_PrescalerConfig(PCT_TypeDef* PCTx, uint8_t PCT_Prescaler);
void PCT_SetAutoreload(PCT_TypeDef* PCTx, uint16_t Autoreload);
void PCT_OnePulseModeConfig(PCT_TypeDef* PCTx, FunctionalState NewState);
void PCT_CCRPreloadConfig(PCT_TypeDef* PCTx, FunctionalState NewState);
void PCT_SetCounter(PCT_TypeDef* PCTx, uint16_t Counter);
uint16_t PCT_GetCounter(PCT_TypeDef* PCTx);
void PCT_Cmd(PCT_TypeDef* PCTx, FunctionalState NewState);

/* Interrupts and flags management ********************************************/
void PCT_ITConfig(PCT_TypeDef* PCTx, uint8_t PCT_IT, FunctionalState NewState);
FlagStatus PCT_GetFlagStatus(PCT_TypeDef* PCTx, uint8_t PCT_FLAG);
void PCT_ClearFlag(PCT_TypeDef* PCTx, uint8_t PCT_FLAG);
ITStatus PCT_GetITStatus(PCT_TypeDef* PCTx, uint8_t PCT_IT);
void PCT_ClearITPendingBit(PCT_TypeDef* PCTx, uint8_t PCT_IT);

/* Input Capture and Output Compare management ********************************/
void PCT_CC0ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_CC1ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_CC2ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_CC3ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_CC4ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_CC5ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode);
void PCT_SetFinalCompareA(PCT_TypeDef* PCTx, uint16_t CompareA);
void PCT_SetFinalCompareB(PCT_TypeDef* PCTx, uint16_t CompareB);
void PCT_SetCompare0(PCT_TypeDef* PCTx, uint16_t Compare0);
void PCT_SetCompare1(PCT_TypeDef* PCTx, uint16_t Compare1);
void PCT_SetCompare2(PCT_TypeDef* PCTx, uint16_t Compare2);
void PCT_SetCompare3(PCT_TypeDef* PCTx, uint16_t Compare3);
void PCT_SetCompare4(PCT_TypeDef* PCTx, uint16_t Compare4);
void PCT_SetCompare5(PCT_TypeDef* PCTx, uint16_t Compare5);
uint16_t PCT_GetCapture0(PCT_TypeDef* PCTx);
uint16_t PCT_GetCapture1(PCT_TypeDef* PCTx);
uint16_t PCT_GetCapture2(PCT_TypeDef* PCTx);
uint16_t PCT_GetCapture3(PCT_TypeDef* PCTx);
uint16_t PCT_GetCapture4(PCT_TypeDef* PCTx);
uint16_t PCT_GetCapture5(PCT_TypeDef* PCTx);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_PCT_H */
