/**
 * @file    wb7720_uart0.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the UART0 firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_UART0_H
#define __WB7720_UART0_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup UART0
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  UART0 Init Structure definition  
  */
typedef struct
{
  uint32_t UART0_BaudRate;            /*!< This member configures the UART0 communication baud rate.
                                           The baud rate is computed using the following formula:
                                            - IntegerDivider = ((PCLKx) / (16 * (UART0_InitStruct->UART0_BaudRate)))
                                            - FractionalDivider = ((IntegerDivider - ((uint32_t) IntegerDivider)) * 16) + 0.5 */

  uint32_t UART0_WordLength;          /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref UART0_Word_Length */

  uint32_t UART0_StopBits;            /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref UART0_Stop_Bits */

  uint32_t UART0_Parity;              /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref UART0_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */
 
  uint32_t UART0_Mode;                /*!< Specifies wether the Receive or Transmit mode is enabled or disabled.
                                           This parameter can be a value of @ref UART0_Mode */
} UART0_InitTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup UART0_Exported_Constants
  * @{
  */

/** @defgroup UART0_Word_Length 
  * @{
  */

#define UART0_WordLength_8b                  ((uint32_t)0x00000000)
#define UART0_WordLength_9b                  UART0_CR1_M /* should be ((uint32_t)0x00001000) */
#define IS_UART0_WORD_LENGTH(LENGTH) (((LENGTH) == UART0_WordLength_8b) || \
                                      ((LENGTH) == UART0_WordLength_9b))
/**
  * @}
  */


/** @defgroup UART0_Stop_Bits 
  * @{
  */

#define UART0_StopBits_1                     ((uint32_t)0x00000000)
#define UART0_StopBits_2                     (0x2U << UART0_CR2_STOP_Pos)
#define IS_UART0_STOPBITS(STOPBITS) (((STOPBITS) == UART0_StopBits_1) || \
                                     ((STOPBITS) == UART0_StopBits_2))
/**
  * @}
  */


/** @defgroup UART0_Parity 
  * @{
  */

#define UART0_Parity_No                      ((uint32_t)0x00000000)
#define UART0_Parity_Even                    UART0_CR1_PCE
#define UART0_Parity_Odd                     (UART0_CR1_PCE | UART0_CR1_PS) 
#define IS_UART0_PARITY(PARITY) (((PARITY) == UART0_Parity_No) || \
                                 ((PARITY) == UART0_Parity_Even) || \
                                 ((PARITY) == UART0_Parity_Odd))
/**
  * @}
  */


/** @defgroup UART0_Mode 
  * @{
  */
#define UART0_Mode_Rx                        UART0_CR1_RE
#define UART0_Mode_Tx                        UART0_CR1_TE
#define IS_UART0_MODE(MODE) ((((MODE) & (uint32_t)0xFFFFFFF3) == 0x00) && \
                              ((MODE) != (uint32_t)0x00))
/**
  * @}
  */


/** @defgroup UART0_Inversion_Pins 
  * @{
  */

#define UART0_InvPin_Tx                      UART0_CR2_TXINV
#define UART0_InvPin_Rx                      UART0_CR2_RXINV
#define IS_UART0_INVERSTION_PIN(PIN) ((((PIN) & (uint32_t)0xFFFCFFFF) == 0x00) && \
                                       ((PIN) != (uint32_t)0x00))
/**
  * @}
  */


/** @defgroup UART0_AutoBaudRate_Mode 
  * @{
  */
#define UART0_AutoBaudRate_StartBit          ((uint32_t)0x00000000)
#define UART0_AutoBaudRate_FallingEdge       (0x1U << UART0_CR2_ABRMODE_Pos)
#define IS_UART0_AUTOBAUDRATE_MODE(MODE) (((MODE) == UART0_AutoBaudRate_StartBit) || \
                                          ((MODE) == UART0_AutoBaudRate_FallingEdge))
/**
  * @}
  */


/** @defgroup UART0_OVR_DETECTION
  * @{
  */
#define UART0_OVRDetection_Enable            ((uint32_t)0x00000000)
#define UART0_OVRDetection_Disable           UART0_CR3_OVRDIS
#define IS_UART0_OVRDETECTION(OVR) (((OVR) == UART0_OVRDetection_Enable)|| \
                                    ((OVR) == UART0_OVRDetection_Disable))
/**
  * @}
  */


/** @defgroup UART0_Request 
  * @{
  */
#define UART0_Request_ABRRQ                  UART0_RQR_ABRRQ
#define UART0_Request_SBKRQ                  UART0_RQR_SBKRQ
#define UART0_Request_RXFRQ                  UART0_RQR_RXFRQ

#define IS_UART0_REQUEST(REQUEST) (((REQUEST) == UART0_Request_RXFRQ) || \
                                   ((REQUEST) == UART0_Request_SBKRQ) || \
                                   ((REQUEST) == UART0_Request_ABRRQ))
/**
  * @}
  */


/** @defgroup UART0_Flags 
  * @{
  */
#define UART0_FLAG_REACK                     UART0_ISR_REACK
#define UART0_FLAG_TEACK                     UART0_ISR_TEACK
#define UART0_FLAG_SBK                       UART0_ISR_SBKF
#define UART0_FLAG_BUSY                      UART0_ISR_BUSY
#define UART0_FLAG_ABRF                      UART0_ISR_ABRF
#define UART0_FLAG_ABRE                      UART0_ISR_ABRE
#define UART0_FLAG_LBD                       UART0_ISR_LBD
#define UART0_FLAG_TXE                       UART0_ISR_TXE
#define UART0_FLAG_TC                        UART0_ISR_TC
#define UART0_FLAG_RXNE                      UART0_ISR_RXNE
#define UART0_FLAG_IDLE                      UART0_ISR_IDLE
#define UART0_FLAG_ORE                       UART0_ISR_ORE
#define UART0_FLAG_NE                        UART0_ISR_NE
#define UART0_FLAG_FE                        UART0_ISR_FE
#define UART0_FLAG_PE                        UART0_ISR_PE
#define IS_UART0_FLAG(FLAG) (((FLAG) == UART0_FLAG_PE) || ((FLAG) == UART0_FLAG_TXE) || \
                             ((FLAG) == UART0_FLAG_TC) || ((FLAG) == UART0_FLAG_RXNE) || \
                             ((FLAG) == UART0_FLAG_IDLE) || ((FLAG) == UART0_FLAG_LBD) || \
                             ((FLAG) == UART0_FLAG_ORE) || ((FLAG) == UART0_FLAG_NE) || \
                             ((FLAG) == UART0_FLAG_FE) || ((FLAG) == UART0_FLAG_ABRE) || \
                             ((FLAG) == UART0_FLAG_ABRF) || ((FLAG) == UART0_FLAG_BUSY) || \
                             ((FLAG) == UART0_FLAG_SBK) || ((FLAG) == UART0_FLAG_TEACK) || \
                             ((FLAG) == UART0_FLAG_REACK))

#define IS_UART0_CLEAR_FLAG(FLAG) (((FLAG) == UART0_FLAG_TC) || ((FLAG) == UART0_FLAG_IDLE) || \
                                   ((FLAG) == UART0_FLAG_ORE) || ((FLAG) == UART0_FLAG_NE) || \
                                   ((FLAG) == UART0_FLAG_FE) || ((FLAG) == UART0_FLAG_LBD) || \
                                   ((FLAG) == UART0_FLAG_PE))
/**
  * @}
  */


/** @defgroup UART0_Interrupt_definition 
  * @brief UART0 Interrupt definition
  * UART0_IT possible values
  * Elements values convention: 0xZZZZYYXX
  *   XX: Position of the corresponding Interrupt
  *   YY: Register index
  *   ZZZZ: Flag position
  * @{
  */
#define UART0_IT_PE                          ((uint32_t)0x00000108)
#define UART0_IT_TXE                         ((uint32_t)0x00070107)
#define UART0_IT_TC                          ((uint32_t)0x00060106)
#define UART0_IT_RXNE                        ((uint32_t)0x00050105)
#define UART0_IT_IDLE                        ((uint32_t)0x00040104)
#define UART0_IT_LBD                         ((uint32_t)0x00080206)
#define UART0_IT_ERR                         ((uint32_t)0x00000300)
#define UART0_IT_ORE                         ((uint32_t)0x00030300)
#define UART0_IT_NE                          ((uint32_t)0x00020300)
#define UART0_IT_FE                          ((uint32_t)0x00010300)

#define IS_UART0_CONFIG_IT(IT) (((IT) == UART0_IT_PE) || ((IT) == UART0_IT_TXE) || \
                                ((IT) == UART0_IT_TC) || ((IT) == UART0_IT_RXNE) || \
                                ((IT) == UART0_IT_IDLE) || ((IT) == UART0_IT_LBD) || \
                                ((IT) == UART0_IT_ERR))

#define IS_UART0_GET_IT(IT) (((IT) == UART0_IT_PE) || ((IT) == UART0_IT_TXE) || \
                             ((IT) == UART0_IT_TC) || ((IT) == UART0_IT_RXNE) || \
                             ((IT) == UART0_IT_IDLE) || ((IT) == UART0_IT_LBD) || \
                             ((IT) == UART0_IT_ORE) || ((IT) == UART0_IT_NE) || \
                             ((IT) == UART0_IT_FE))

#define IS_UART0_CLEAR_IT(IT) (((IT) == UART0_IT_TC) || ((IT) == UART0_IT_PE) || \
                               ((IT) == UART0_IT_FE) || ((IT) == UART0_IT_NE) || \
                               ((IT) == UART0_IT_ORE) || ((IT) == UART0_IT_IDLE) || \
                               ((IT) == UART0_IT_LBD))
/**
  * @}
  */


/** @defgroup UART0_Global_definition 
  * @{
  */
#define IS_UART0_BAUDRATE(BAUDRATE) (((BAUDRATE) > 0) && ((BAUDRATE) < 0x005B8D81))
#define IS_UART0_DATA(DATA) ((DATA) <= 0x1FF)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/* Initialization and Configuration functions *********************************/
void UART0_DeInit(void);
void UART0_Init(UART0_InitTypeDef* UART0_InitStruct);
void UART0_StructInit(UART0_InitTypeDef* UART0_InitStruct);
void UART0_Cmd(FunctionalState NewState);
void UART0_DirectionModeCmd(uint32_t UART0_DirectionMode, FunctionalState NewState);
void UART0_OverSampling8Cmd(FunctionalState NewState);
void UART0_MSBFirstCmd(FunctionalState NewState);
void UART0_DataInvCmd(FunctionalState NewState);
void UART0_InvPinCmd(uint32_t UART0_InvPin, FunctionalState NewState);

/* AutoBaudRate functions *****************************************************/
void UART0_AutoBaudRateCmd(FunctionalState NewState);
void UART0_AutoBaudRateConfig(uint32_t UART0_AutoBaudRate);

/* Data transfers functions ***************************************************/
void UART0_SendData(uint16_t Data);
uint16_t UART0_ReceiveData(void);

/* LIN mode functions *********************************************************/
void UART0_LINCmd(FunctionalState NewState);

/* Interrupts and flags management functions **********************************/
void UART0_ITConfig(uint32_t UART0_IT, FunctionalState NewState);
void UART0_RequestCmd(uint32_t UART0_Request, FunctionalState NewState);
void UART0_OverrunDetectionConfig(uint32_t UART0_OVRDetection);
FlagStatus UART0_GetFlagStatus(uint32_t UART0_FLAG);
void UART0_ClearFlag(uint32_t UART0_FLAG);
ITStatus UART0_GetITStatus(uint32_t UART0_IT);
void UART0_ClearITPendingBit(uint32_t UART0_IT);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_UART0_H */

