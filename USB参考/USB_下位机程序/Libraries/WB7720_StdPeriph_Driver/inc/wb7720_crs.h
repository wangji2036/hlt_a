/**
 * @file    wb7720_crs.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the CRS firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_CRS_H
#define __WB7720_CRS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup CRS
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup CRS_Exported_Constants
  * @{
  */

/** @defgroup CRS_Interrupt_Sources
  * @{
  */
#define CRS_IT_SYNCOK             CRS_ISR_SYNCOKF     /*!< SYNC event OK */
#define CRS_IT_SYNCWARN           CRS_ISR_SYNCWARNF   /*!< SYNC warning */
#define CRS_IT_ERR                CRS_ISR_ERRF        /*!< error */
#define CRS_IT_ESYNC              CRS_ISR_ESYNCF      /*!< Expected SYNC */
#define CRS_IT_TRIMOVF            CRS_ISR_TRIMOVF     /*!< Trimming overflow or underflow */
#define CRS_IT_SYNCERR            CRS_ISR_SYNCERR     /*!< SYNC error */
#define CRS_IT_SYNCMISS           CRS_ISR_SYNCMISS    /*!< SYNC missed*/

#define IS_CRS_IT(IT) (((IT) == CRS_IT_SYNCOK) || ((IT) == CRS_IT_SYNCWARN) || \
                       ((IT) == CRS_IT_ERR)    || ((IT) == CRS_IT_ESYNC))
                       
#define IS_CRS_GET_IT(IT) (((IT) == CRS_IT_SYNCOK)  || ((IT) == CRS_IT_SYNCWARN) || \
                           ((IT) == CRS_IT_ERR)     || ((IT) == CRS_IT_ESYNC) || \
                           ((IT) == CRS_IT_TRIMOVF) || ((IT) == CRS_IT_SYNCERR) || \
                           ((IT) == CRS_IT_SYNCMISS))

#define IS_CRS_CLEAR_IT(IT) ((IT) != 0x00)

/**
  * @}
  */

/** @defgroup CRS_Flags
  * @{
  */
#define CRS_FLAG_SYNCOK             CRS_ISR_SYNCOKF       /*!< SYNC event OK */
#define CRS_FLAG_SYNCWARN           CRS_ISR_SYNCWARNF     /*!< SYNC warning */
#define CRS_FLAG_ERR                CRS_ISR_ERRF          /*!< error */
#define CRS_FLAG_ESYNC              CRS_ISR_ESYNCF        /*!< Expected SYNC */
#define CRS_FLAG_TRIMOVF            CRS_ISR_TRIMOVF       /*!< Trimming overflow or underflow */
#define CRS_FLAG_SYNCERR            CRS_ISR_SYNCERR       /*!< SYNC error */
#define CRS_FLAG_SYNCMISS           CRS_ISR_SYNCMISS      /*!< SYNC missed*/

#define IS_CRS_FLAG(FLAG) (((FLAG) == CRS_FLAG_SYNCOK) || ((FLAG) == CRS_FLAG_SYNCWARN) || \
                           ((FLAG) == CRS_FLAG_ERR) || ((FLAG) == CRS_FLAG_ESYNC) || \
                           ((FLAG) == CRS_FLAG_TRIMOVF) || ((FLAG) == CRS_FLAG_SYNCERR) || \
                           ((FLAG) == CRS_FLAG_SYNCMISS))

/**
  * @}
  */
  
/** @defgroup CRS_Synchro_Source
  * @{
  */
#define CRS_SYNCSource_GPIO         CRS_CFGR_SYNCSRC_GPIO     /*!< Synchro Signal soucre GPIO (PA0 only) */
#define CRS_SYNCSource_USB          CRS_CFGR_SYNCSRC_USB      /*!< Synchro Signal source USB SOF */
#define CRS_SYNCSource_EXTI5        CRS_CFGR_SYNCSRC_EXTI5    /*!< Synchro Signal source EXTI5 interrupt */

#define IS_CRS_SYNC_SOURCE(SOURCE) (((SOURCE) == CRS_SYNCSource_GPIO) || \
                                    ((SOURCE) == CRS_SYNCSource_USB)  || \
                                    ((SOURCE) == CRS_SYNCSource_EXTI5))
/**
  * @}
  */

/** @defgroup CRS_SynchroDivider
  * @{
  */
#define CRS_SYNC_Div1        CRS_CFGR_SYNCDIV_1               /*!< Synchro Signal not divided */
#define CRS_SYNC_Div2        CRS_CFGR_SYNCDIV_2               /*!< Synchro Signal divided by 2 */
#define CRS_SYNC_Div4        CRS_CFGR_SYNCDIV_4               /*!< Synchro Signal divided by 4 */
#define CRS_SYNC_Div8        CRS_CFGR_SYNCDIV_8               /*!< Synchro Signal divided by 8 */
#define CRS_SYNC_Div16       CRS_CFGR_SYNCDIV_16              /*!< Synchro Signal divided by 16 */
#define CRS_SYNC_Div32       CRS_CFGR_SYNCDIV_32              /*!< Synchro Signal divided by 32 */
#define CRS_SYNC_Div64       CRS_CFGR_SYNCDIV_64              /*!< Synchro Signal divided by 64 */
#define CRS_SYNC_Div128      CRS_CFGR_SYNCDIV_128             /*!< Synchro Signal divided by 128 */

#define IS_CRS_SYNC_DIV(DIV) (((DIV) == CRS_SYNC_Div1) || ((DIV) == CRS_SYNC_Div2)   || \
                              ((DIV) == CRS_SYNC_Div4) || ((DIV) == CRS_SYNC_Div8)   || \
                              ((DIV) == CRS_SYNC_Div16) || ((DIV) == CRS_SYNC_Div32) || \
                              ((DIV) == CRS_SYNC_Div64) || ((DIV) == CRS_SYNC_Div128))
/**
  * @}
  */

/** @defgroup CRS_SynchroPolarity
  * @{
  */
#define CRS_SYNCPolarity_Rising       ((uint32_t)0x00)      /*!< Synchro Active on rising edge */
#define CRS_SYNCPolarity_Falling      CRS_CFGR_SYNCPOL      /*!< Synchro Active on falling edge */

#define IS_CRS_SYNC_POLARITY(POLARITY) (((POLARITY) == CRS_SYNCPolarity_Rising) || \
                                        ((POLARITY) == CRS_SYNCPolarity_Falling))
/**
  * @}
  */


/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void CRS_DeInit(void);
void CRS_AdjustHSI48CalibrationValue(uint16_t CRS_HSI48CalibrationValue);
void CRS_FrequencyErrorCounterCmd(FunctionalState NewState);
void CRS_AutomaticCalibrationCmd(FunctionalState NewState); 
void CRS_SoftwareSynchronizationGenerate(void);
void CRS_FrequencyErrorCounterReload(uint16_t CRS_ReloadValue);
void CRS_FrequencyErrorLimitConfig(uint8_t CRS_ErrorLimitValue);
void CRS_SynchronizationPrescalerConfig(uint32_t CRS_Prescaler);
void CRS_SynchronizationSourceConfig(uint32_t CRS_Source);
void CRS_SynchronizationPolarityConfig(uint32_t CRS_Polarity);
uint16_t CRS_GetReloadValue(void);
uint16_t CRS_GetHSI48CalibrationValue(void);
uint16_t CRS_GetFrequencyErrorValue(void);
uint32_t CRS_GetFrequencyErrorDirection(void);

void CRS_ITConfig(uint32_t CRS_IT, FunctionalState NewState);
FlagStatus CRS_GetFlagStatus(uint32_t CRS_FLAG);
void CRS_ClearFlag(uint32_t CRS_FLAG);
ITStatus CRS_GetITStatus(uint32_t CRS_IT);
void CRS_ClearITPendingBit(uint32_t CRS_IT);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_CRS_H */
