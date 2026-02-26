/**
 * @file    wb7720_spim.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the SPIM firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_SPIM_H
#define __WB7720_SPIM_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup SPIM
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  SPIM Init structure definition  
  */
typedef struct
{
  uint16_t SPIM_TransferMode;
  uint16_t SPIM_DataSize;
  uint16_t SPIM_CPOL;
  uint16_t SPIM_CPHA;
  uint16_t SPIM_BaudRatePrescaler;
  uint16_t SPIM_FrameFormat;
} SPIM_InitTypeDef;


/** 
  * @brief  SPIM Microwire Init structure definition  
  */
typedef struct
{
  uint16_t SPIM_MicrowireControlFrameSize;
  uint16_t SPIM_MicrowireTransferMode;
  uint16_t SPIM_MicrowireDirection;
  uint16_t SPIM_MicrowireHandshaking;
} SPIM_MicrowireInitTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup SPIM_Exported_Constants
  * @{
  */

/** @defgroup SPIM_transfer_mode 
  * @{
  */
#define SPIM_TransferMode_TxAndRx        ((uint16_t)SPIM_CR0_TMOD_TX_AND_RX)
#define SPIM_TransferMode_TxOnly         ((uint16_t)SPIM_CR0_TMOD_TX_ONLY)
#define SPIM_TransferMode_RxOnly         ((uint16_t)SPIM_CR0_TMOD_RX_ONLY)
#define SPIM_TransferMode_EepromRead     ((uint16_t)SPIM_CR0_TMOD_EEPROM_READ)
/**
  * @}
  */


/** @defgroup SPIM_data_size 
  * @{
  */
#define SPIM_DataSize_4b                 ((uint16_t)SPIM_CR0_DFS_4BITS)
#define SPIM_DataSize_5b                 ((uint16_t)SPIM_CR0_DFS_5BITS)
#define SPIM_DataSize_6b                 ((uint16_t)SPIM_CR0_DFS_6BITS)
#define SPIM_DataSize_7b                 ((uint16_t)SPIM_CR0_DFS_7BITS)
#define SPIM_DataSize_8b                 ((uint16_t)SPIM_CR0_DFS_8BITS)
#define SPIM_DataSize_9b                 ((uint16_t)SPIM_CR0_DFS_9BITS)
#define SPIM_DataSize_10b                ((uint16_t)SPIM_CR0_DFS_10BITS)
#define SPIM_DataSize_11b                ((uint16_t)SPIM_CR0_DFS_11BITS)
#define SPIM_DataSize_12b                ((uint16_t)SPIM_CR0_DFS_12BITS)
#define SPIM_DataSize_13b                ((uint16_t)SPIM_CR0_DFS_13BITS)
#define SPIM_DataSize_14b                ((uint16_t)SPIM_CR0_DFS_14BITS)
#define SPIM_DataSize_15b                ((uint16_t)SPIM_CR0_DFS_15BITS)
#define SPIM_DataSize_16b                ((uint16_t)SPIM_CR0_DFS_16BITS)
/**
  * @}
  */


/** @defgroup SPIM_Clock_Polarity 
  * @{
  */
#define SPIM_CPOL_Low                    ((uint16_t)0x00)
#define SPIM_CPOL_High                   ((uint16_t)0x80)
/**
  * @}
  */


/** @defgroup SPIM_Clock_Phase 
  * @{
  */
#define SPIM_CPHA_1Edge                  ((uint16_t)0x00)
#define SPIM_CPHA_2Edge                  ((uint16_t)0x40)
/**
  * @}
  */


/** @defgroup SPIM_Frame_Format 
  * @{
  */
#define SPIM_FrameFormat_SPI             ((uint16_t)SPIM_CR0_FRF_SPI)
#define SPIM_FrameFormat_SSP             ((uint16_t)SPIM_CR0_FRF_SSP)
#define SPIM_FrameFormat_Microwire       ((uint16_t)SPIM_CR0_FRF_NS)
/**
  * @}
  */


/** @defgroup SPIM_Microwire_Control_Frame_Size 
  * @{
  */
#define SPIM_MicrowireControlFrameSize_1b      ((uint16_t)SPIM_CR0_CFS_01_BIT)
#define SPIM_MicrowireControlFrameSize_2b      ((uint16_t)SPIM_CR0_CFS_02_BIT)
#define SPIM_MicrowireControlFrameSize_3b      ((uint16_t)SPIM_CR0_CFS_03_BIT)
#define SPIM_MicrowireControlFrameSize_4b      ((uint16_t)SPIM_CR0_CFS_04_BIT)
#define SPIM_MicrowireControlFrameSize_5b      ((uint16_t)SPIM_CR0_CFS_05_BIT)
#define SPIM_MicrowireControlFrameSize_6b      ((uint16_t)SPIM_CR0_CFS_06_BIT)
#define SPIM_MicrowireControlFrameSize_7b      ((uint16_t)SPIM_CR0_CFS_07_BIT)
#define SPIM_MicrowireControlFrameSize_8b      ((uint16_t)SPIM_CR0_CFS_08_BIT)
#define SPIM_MicrowireControlFrameSize_9b      ((uint16_t)SPIM_CR0_CFS_09_BIT)
#define SPIM_MicrowireControlFrameSize_10b     ((uint16_t)SPIM_CR0_CFS_10_BIT)
#define SPIM_MicrowireControlFrameSize_11b     ((uint16_t)SPIM_CR0_CFS_11_BIT)
#define SPIM_MicrowireControlFrameSize_12b     ((uint16_t)SPIM_CR0_CFS_12_BIT)
#define SPIM_MicrowireControlFrameSize_13b     ((uint16_t)SPIM_CR0_CFS_13_BIT)
#define SPIM_MicrowireControlFrameSize_14b     ((uint16_t)SPIM_CR0_CFS_14_BIT)
#define SPIM_MicrowireControlFrameSize_15b     ((uint16_t)SPIM_CR0_CFS_15_BIT)
#define SPIM_MicrowireControlFrameSize_16b     ((uint16_t)SPIM_CR0_CFS_16_BIT)
/**
  * @}
  */


/** @defgroup SPIM_Microwire_Transfer_Mode 
  * @{
  */
#define SPIM_MicrowireTransferMode_NonSequential   ((uint16_t)0x00)
#define SPIM_MicrowireTransferMode_Sequential      ((uint16_t)0x01)
/**
  * @}
  */


/** @defgroup SPIM_Microwire_Direction 
  * @{
  */
#define SPIM_MicrowireDirection_Receive        ((uint16_t)0x00)
#define SPIM_MicrowireDirection_Transmit       ((uint16_t)0x02)
/**
  * @}
  */


/** @defgroup SPIM_Microwire_Handshaking 
  * @{
  */
#define SPIM_MicrowireHandshaking_Enable       ((uint16_t)0x04)
#define SPIM_MicrowireHandshaking_Disable      ((uint16_t)0x00)
/**
  * @}
  */


/** @defgroup SPIM_flags_definition 
  * @{
  */
#define SPIM_FLAG_BUSY   ((uint8_t)SPIM_SR_BUSY)
#define SPIM_FLAG_TFNF   ((uint8_t)SPIM_SR_TFNF)
#define SPIM_FLAG_TFE    ((uint8_t)SPIM_SR_TFE)
#define SPIM_FLAG_RFNE   ((uint8_t)SPIM_SR_RFNE)
#define SPIM_FLAG_RFF    ((uint8_t)SPIM_SR_RFF)
#define SPIM_FLAG_TXERR  ((uint8_t)SPIM_SR_TXERR)
#define SPIM_FLAG_DCOL   ((uint8_t)SPIM_SR_DCOL)
/**
  * @}
  */


/** @defgroup SPIM_interrupts_definition
  * @{
  */
#define SPIM_IT_TXE      (0x1 << 0)
#define SPIM_IT_TXO      (0x1 << 1)
#define SPIM_IT_RXU      (0x1 << 2)
#define SPIM_IT_RXO      (0x1 << 3)
#define SPIM_IT_RXF      (0x1 << 4)
#define SPIM_IT_MST      (0x1 << 5)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void SPIM_DeInit(void);
void SPIM_Init(SPIM_InitTypeDef* SPIM_InitStruct);
void SPIM_StructInit(SPIM_InitTypeDef* SPIM_InitStruct);
void SPIM_Cmd(FunctionalState NewState);
void SPIM_TransferModeConfig(uint16_t SPIM_TransferMode);
void SPIM_NSSConfig(FunctionalState NewState);
void SPIM_NSSToggleModeCmd(FunctionalState NewState);
void SPIM_ReceiveDataLengthConfig(uint32_t DataLength);
void SPIM_MicrowireConfig(SPIM_MicrowireInitTypeDef* SPIM_MicrowireInitStruct);
void SPIM_MicrowireStructInit(SPIM_MicrowireInitTypeDef* SPIM_MicrowireInitStruct);
uint16_t SPIM_ReadData(void);
void SPIM_WriteData(uint16_t data);
FlagStatus SPIM_GetFlagStatus(uint8_t SPIM_FLAG);
void SPIM_ITConfig(uint8_t SPIM_IT, FunctionalState NewState);
ITStatus SPIM_GetRawITStatus(uint8_t SPIM_IT);
ITStatus SPIM_GetITStatus(uint8_t SPIM_IT);
void SPIM_ClearITPendingBit(uint8_t SPIM_IT);
void SPIM_RxFIFOThresholdConfig(uint8_t Threshold);
void SPIM_TxFIFOThresholdConfig(uint8_t Threshold);
uint8_t SPIM_GetRxFIFOLevel(void);
uint8_t SPIM_GetTxFIFOLevel(void);
void SPIM_RxdSampleDelayConfig(uint8_t DelayValue);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_SPIM_H */
