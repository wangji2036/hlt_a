/**
 * @file    wb7720_uart1.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the UART1 firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_UART1_H
#define __WB7720_UART1_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup UART1
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  UART1 Auto Baud Rate Init Structure definition  
  */
typedef struct
{
  uint8_t           UART1_ABR_LowPulseWidth;              /*!< Specifies the number of bits of the first low pulse on the UART RX line.
                                                               This parameter can be a value of @ref UART1_AutoBaudRate_LowPulseWidth */

  uint8_t           UART1_ABR_CharacterWidth;             /*!< Specifies the number of bits for the character used in UART auto baud rate detection.
                                                               This parameter can be a value of @ref UART1_AutoBaudRate_CharacterWidth */

  FunctionalState   UART1_ABR_UpdateBaudRate;             /*!< Specifies whether to automatically update the UART1 baud rate configuration after auto baud rate detection.
                                                               This parameter can be set either to ENABLE or DISABLE. */

  FunctionalState   UART1_ABR_ReceiveDuringAutoBaudRate;  /*!< Enable or disable receive the character during auto baud rate detection.
                                                               This parameter can be set either to ENABLE or DISABLE. */
} UART1_ABR_InitTypeDef;


/* Exported constants --------------------------------------------------------*/

/** @defgroup UART1_Exported_Constants
  * @{
  */

/** @defgroup UART1_Mode 
  * @{
  */
#define UART1_Mode_TX         UART1_CR_TXE
#define UART1_Mode_RX         UART1_CR_RXE
/**
  * @}
  */


/** @defgroup UART1_IT 
  * @{
  */
#define UART1_IT_TX             (0x1U << 0)
#define UART1_IT_RX             (0x1U << 1)
#define UART1_IT_TXO            (0x1U << 2)
#define UART1_IT_RXO            (0x1U << 3)
/**
  * @}
  */


/** @defgroup UART1_Flags 
  * @{
  */
#define UART1_FLAG_TXF            UART1_SR_TXF
#define UART1_FLAG_RXF            UART1_SR_RXF
#define UART1_FLAG_TXO            UART1_SR_TXO
#define UART1_FLAG_RXO            UART1_SR_RXO
/**
  * @}
  */


/** @defgroup UART1_AutoBaudRate_LowPulseWidth 
  * @{
  */
#define UART1_ABR_LowPulseWidth_1Bit        UART1_ABRCON_LPWIDTH_1BIT
#define UART1_ABR_LowPulseWidth_2Bit        UART1_ABRCON_LPWIDTH_2BIT
#define UART1_ABR_LowPulseWidth_4Bit        UART1_ABRCON_LPWIDTH_4BIT
#define UART1_ABR_LowPulseWidth_8Bit        UART1_ABRCON_LPWIDTH_8BIT
/**
  * @}
  */


/** @defgroup UART1_AutoBaudRate_CharacterWidth 
  * @{
  */
#define UART1_ABR_CharacterWidth_5Bit       UART1_ABRCON_CHWIDTH_5BIT
#define UART1_ABR_CharacterWidth_6Bit       UART1_ABRCON_CHWIDTH_6BIT
#define UART1_ABR_CharacterWidth_7Bit       UART1_ABRCON_CHWIDTH_7BIT
#define UART1_ABR_CharacterWidth_8Bit       UART1_ABRCON_CHWIDTH_8BIT
#define UART1_ABR_CharacterWidth_9Bit       UART1_ABRCON_CHWIDTH_9BIT
#define UART1_ABR_CharacterWidth_10Bit      UART1_ABRCON_CHWIDTH_10BIT
#define UART1_ABR_CharacterWidth_11Bit      UART1_ABRCON_CHWIDTH_11BIT
#define UART1_ABR_CharacterWidth_12Bit      UART1_ABRCON_CHWIDTH_12BIT
#define UART1_ABR_CharacterWidth_13Bit      UART1_ABRCON_CHWIDTH_13BIT
#define UART1_ABR_CharacterWidth_14Bit      UART1_ABRCON_CHWIDTH_14BIT
#define UART1_ABR_CharacterWidth_15Bit      UART1_ABRCON_CHWIDTH_15BIT
/**
  * @}
  */


/** @defgroup UART1_ABR_IT 
  * @{
  */
#define UART1_ABR_IT_ABRD           (0x1U << 0)
#define UART1_ABR_IT_ABROV          (0x1U << 1)
/**
  * @}
  */


/** @defgroup UART1_ABR_Flags 
  * @{
  */
#define UART1_ABR_FLAG_ABRD         (0x1U << 0)
#define UART1_ABR_FLAG_ABROV        (0x1U << 1)
/**
  * @}
  */


/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void UART1_DeInit(void);
void UART1_Init(uint32_t UART1_BaudRate, uint32_t UART1_Mode);
void UART1_WriteData(uint8_t Data);
uint8_t UART1_ReadData(void);
void UART1_ITConfig(uint32_t UART1_IT, FunctionalState NewState);
FlagStatus UART1_GetFlagStatus(uint32_t UART1_FLAG);
void UART1_ClearFlag(uint32_t UART1_FLAG);
ITStatus UART1_GetITStatus(uint32_t UART1_IT);
void UART1_ClearITPendingBit(uint32_t UART1_IT);

/* Auto baud rate functions ************************/
void UART1_ABR_Init(UART1_ABR_InitTypeDef* UART1_ABR_InitStruct);
void UART1_ABR_StructInit(UART1_ABR_InitTypeDef* UART1_ABR_InitStruct);
void UART1_ABR_Cmd(FunctionalState NewState);
uint32_t UART1_ABR_GetAutoBaudRateValue(void);
void UART1_ABR_ITConfig(uint32_t UART1_ABR_IT, FunctionalState NewState);
FlagStatus UART1_ABR_GetFlagStatus(uint32_t UART1_ABR_FLAG);
void UART1_ABR_ClearFlag(uint32_t UART1_ABR_FLAG);
ITStatus UART1_ABR_GetITStatus(uint32_t UART1_ABR_IT);
void UART1_ABR_ClearITPendingBit(uint32_t UART1_ABR_IT);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_UART1_H */
