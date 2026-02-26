/**
 * @file    wb7720_spim.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the SPIM firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_spim.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup SPIM
  * @brief SPIM driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup SPIM_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the SPIM peripheral registers to their default reset values.
 * @return None
 */
void SPIM_DeInit(void)
{
}

/**
 * @brief  Initializes the SPIM peripheral according to the specified 
 *         parameters in the SPIM_InitStruct.
 * @param  SPIM_InitStruct: pointer to a SPIM_InitTypeDef structure
 *         that contains the configuration information for the SPIM peripheral.
 * @return None
 */
void SPIM_Init(SPIM_InitTypeDef* SPIM_InitStruct)
{
  SPIM->CR0 = (SPIM_InitStruct->SPIM_DataSize | SPIM_InitStruct->SPIM_FrameFormat |
        SPIM_InitStruct->SPIM_CPHA | SPIM_InitStruct->SPIM_CPOL |
        SPIM_InitStruct->SPIM_TransferMode);

  SPIM->BAUDR = SPIM_InitStruct->SPIM_BaudRatePrescaler;
}

/**
 * @brief  Fills each SPIM_InitStruct member with its default value.
 * @param  SPIM_InitStruct: pointer to a SPIM_InitTypeDef structure
 *         which will be initialized.
 * @return None
 */
void SPIM_StructInit(SPIM_InitTypeDef* SPIM_InitStruct)
{
  SPIM_InitStruct->SPIM_TransferMode = SPIM_TransferMode_TxAndRx;
  SPIM_InitStruct->SPIM_DataSize = SPIM_DataSize_8b;
  SPIM_InitStruct->SPIM_CPOL = SPIM_CPOL_Low;
  SPIM_InitStruct->SPIM_CPHA = SPIM_CPHA_1Edge;
  SPIM_InitStruct->SPIM_BaudRatePrescaler = 2;
  SPIM_InitStruct->SPIM_FrameFormat = SPIM_FrameFormat_SPI;
}

/**
 * @brief  Enables or disables the SPIM peripheral.
 * @param  NewState: new state of the SPIM peripheral.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SPIM_Cmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    SPIM->SPIENR = 0x01;
  }
  else {
    SPIM->SPIENR = 0x00;
  }
}

/**
 * @brief  Configures the transfer mode.
 * @param  SPIM_TransferMode: specifies the transfer mode.
 *         This parameter can be one of the following values:
 *         @arg @ref SPIM_TransferMode_TxAndRx
 *         @arg @ref SPIM_TransferMode_TxOnly
 *         @arg @ref SPIM_TransferMode_RxOnly
 *         @arg @ref SPIM_TransferMode_EepromRead
 * @return None
 */
void SPIM_TransferModeConfig(uint16_t SPIM_TransferMode)
{
  SPIM->CR0 = (SPIM->CR0 & (~SPIM_CR0_TMOD_Msk)) | SPIM_TransferMode;
}

/**
 * @brief  Enables or disables the slave select line.
 * @param  NewState: new state of the slave select line.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SPIM_NSSConfig(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    SPIM->SER |= 0x01;
  }
  else {
    SPIM->SER &= ~0x01;
  }
}

/**
 * @brief  Enables or disables the slave select toggle mode.
 * @param  NewState: new state of the slave select toggle mode.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SPIM_NSSToggleModeCmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    SPIM->CR0 |= SPIM_CR0_SSTE;
  }
  else {
    SPIM->CR0 &= ~SPIM_CR0_SSTE;
  }
}

/**
 * @brief  Configures the number of data frames to be continuously received.
 * @param  DataLength: specifies the number of data frames to be continuously received.
 * @return None
 */
void SPIM_ReceiveDataLengthConfig(uint32_t DataLength)
{
  SPIM->CR1 = DataLength - 1;
}

/**
 * @brief  Configures the Microwire frame format according to the
 *         specified parameters in the SPIM_MicrowireInitStruct.
 * @param  SPIM_MicrowireInitStruct: pointer to a SPIM_MicrowireInitTypeDef structure
 *         that contains the configuration information for the Microwire frame format. 
 * @return None
 */
void SPIM_MicrowireConfig(SPIM_MicrowireInitTypeDef* SPIM_MicrowireInitStruct)
{
  SPIM->CR0 = (SPIM->CR0 & (~SPIM_CR0_CFS_Msk)) | SPIM_MicrowireInitStruct->SPIM_MicrowireControlFrameSize;
  SPIM->MWCR = (SPIM->MWCR & 0xFFFFFFF8) | SPIM_MicrowireInitStruct->SPIM_MicrowireTransferMode |
    SPIM_MicrowireInitStruct->SPIM_MicrowireDirection | SPIM_MicrowireInitStruct->SPIM_MicrowireHandshaking;
}

/**
 * @brief  Fills each SPIM_MicrowireInitStruct member with its default value.
 * @param  SPIM_MicrowireInitStruct: pointer to a SPIM_MicrowireInitTypeDef structure
 *         which will be initialized.
 * @return None
 */
void SPIM_MicrowireStructInit(SPIM_MicrowireInitTypeDef* SPIM_MicrowireInitStruct)
{
  SPIM_MicrowireInitStruct->SPIM_MicrowireControlFrameSize = SPIM_MicrowireControlFrameSize_8b;
  SPIM_MicrowireInitStruct->SPIM_MicrowireTransferMode = SPIM_MicrowireTransferMode_NonSequential;
  SPIM_MicrowireInitStruct->SPIM_MicrowireDirection = SPIM_MicrowireDirection_Transmit;
  SPIM_MicrowireInitStruct->SPIM_MicrowireHandshaking = SPIM_MicrowireHandshaking_Disable;
}

/**
 * @brief  Read one data from Rx FIFO.
 * @return The read data.
 */
uint16_t SPIM_ReadData(void)
{
  return SPIM->DR;
}

/**
 * @brief  Write one data to Tx FIFO.
 * @param  Data: The data to write.
 * @return None
 */
void SPIM_WriteData(uint16_t Data)
{
  SPIM->DR = Data;
}

/**
 * @brief  Checks whether the specified SPIM flag is set or not.
 * @param  SPIM_FLAG: specifies the SPIM flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref SPIM_FLAG_BUSY: SPIM busy flag.
 *         @arg @ref SPIM_FLAG_TFNF: Transmit FIFO not full.
 *         @arg @ref SPIM_FLAG_TFE: Transmit FIFO empty.
 *         @arg @ref SPIM_FLAG_RFNE: Receive FIFO not empty.
 *         @arg @ref SPIM_FLAG_RFF: Receive FIFO full.
 *         @arg @ref SPIM_FLAG_DCOL: Data collision error.
 * @return The new state of SPIM_FLAG (SET or RESET).
 */
FlagStatus SPIM_GetFlagStatus(uint8_t SPIM_FLAG)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified SPIM flag */
  if (SPIM->SR & SPIM_FLAG)
  {
    /* SPIM_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* SPIM_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the SPIM_FLAG status */
  return  bitstatus;
}

/**
 * @brief  Enables or disables the specified SPIM interrupts.
 * @param  SPIM_IT: specifies the SPIM interrupt source to be enabled or disabled. 
 *         This parameter can be any combination of the following values:
 *         @arg @ref SPIM_IT_TXE: Transmit FIFO empty interrupt mask
 *         @arg @ref SPIM_IT_TXO: Transmit FIFO overflow interrupt mask
 *         @arg @ref SPIM_IT_RXU: Receive FIFO underflow interrupt mask
 *         @arg @ref SPIM_IT_RXO: Receive FIFO overflow interrupt mask
 *         @arg @ref SPIM_IT_RXF: Receive FIFO full interrupt mask
 *         @arg @ref SPIM_IT_MST: Multi-Master contention interrupt mask
 * @param  NewState: new state of the specified SPIM interrupt.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None.
 */
void SPIM_ITConfig(uint8_t SPIM_IT, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    /* Enable the selected SPIM interrupt */
    SPIM->IER |= SPIM_IT;
  }
  else
  {
    /* Disable the selected SPIM interrupt */
    SPIM->IER &= ~SPIM_IT;
  }
}

/**
 * @brief  Checks whether the specified SPIM raw interrupt status.
 * @param  SPIM_IT: specifies the SPIM interrupt source to check.
 *         This parameter can be one of the following values:
 *         @arg @ref SPIM_IT_TXE: Transmit FIFO empty interrupt
 *         @arg @ref SPIM_IT_TXO: Transmit FIFO overflow interrupt
 *         @arg @ref SPIM_IT_RXU: Receive FIFO underflow interrupt
 *         @arg @ref SPIM_IT_RXO: Receive FIFO overflow interrupt
 *         @arg @ref SPIM_IT_RXF: Receive FIFO full interrupt
 *         @arg @ref SPIM_IT_MST: Multi-Master contention interrupt
 * @return The new state of raw SPIM_IT (SET or RESET).
 */
ITStatus SPIM_GetRawITStatus(uint8_t SPIM_IT)
{
  ITStatus bitstatus = RESET;

  /* Check the raw status of the specified SPIM interrupt */
  if (SPIM->RISR & SPIM_IT)
  {
    /* SPIM_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* SPIM_IT is reset */
    bitstatus = RESET;
  }
  /* Return the SPIM_IT raw status */
  return bitstatus;
}

/**
 * @brief  Checks whether the specified SPIM interrupt has occurred or not.
 * @param  SPIM_IT: specifies the SPIM interrupt source to check.
 *         This parameter can be one of the following values:
 *         @arg @ref SPIM_IT_TXE: Transmit FIFO empty interrupt
 *         @arg @ref SPIM_IT_TXO: Transmit FIFO overflow interrupt
 *         @arg @ref SPIM_IT_RXU: Receive FIFO underflow interrupt
 *         @arg @ref SPIM_IT_RXO: Receive FIFO overflow interrupt
 *         @arg @ref SPIM_IT_RXF: Receive FIFO full interrupt
 *         @arg @ref SPIM_IT_MST: Multi-Master contention interrupt
 * @return The new state of SPIM_IT (SET or RESET).
 */
ITStatus SPIM_GetITStatus(uint8_t SPIM_IT)
{
  ITStatus bitstatus = RESET;

  /* Check the status of the specified SPIM interrupt */
  if (SPIM->ISR & SPIM_IT)
  {
    /* SPIM_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* SPIM_IT is reset */
    bitstatus = RESET;
  }
  /* Return the SPIM_IT status */
  return bitstatus;
}

/**
 * @brief  Clears the SPIM's interrupt pending bits.
 * @param  SPIM_IT: specifies the interrupt pending bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref SPIM_IT_TXO: Transmit FIFO overflow interrupt
 *         @arg @ref SPIM_IT_RXU: Receive FIFO underflow interrupt
 *         @arg @ref SPIM_IT_RXO: Receive FIFO overflow interrupt
 *         @arg @ref SPIM_IT_MST: Multi-Master contention interrupt
 * @return None.
 */
void SPIM_ClearITPendingBit(uint8_t SPIM_IT)
{
  if ((SPIM_IT & (SPIM_IT_TXO | SPIM_IT_RXU | SPIM_IT_RXO | SPIM_IT_MST)) ==
               (SPIM_IT_TXO | SPIM_IT_RXU | SPIM_IT_RXO | SPIM_IT_MST))
  {
    (void)SPIM->ICR;
  }
  else
  {
    if(SPIM_IT & SPIM_IT_TXO) (void)SPIM->TXOICR;
    if(SPIM_IT & SPIM_IT_RXU) (void)SPIM->RXUICR;
    if(SPIM_IT & SPIM_IT_RXO) (void)SPIM->RXOICR;
    if(SPIM_IT & SPIM_IT_MST) (void)SPIM->MSTICR;
  }
}

/**
 * @brief  Configures the Rx FIFO threshold.
 * @param  Threshold: The Rx FIFO threshold to set.
 * @note   The Receive FIFO Full interrupt will trigger when the level > Threshold.
 * @return None.
 */
void SPIM_RxFIFOThresholdConfig(uint8_t Threshold)
{
  SPIM->RXFTLR = Threshold;
}

/**
 * @brief  Configures the Tx FIFO threshold.
 * @param  Threshold: The Tx FIFO threshold to set.
 * @note   The Transmit FIFO Empty interrupt will trigger when the level <= Threshold.
 * @return None.
 */
void SPIM_TxFIFOThresholdConfig(uint8_t Threshold)
{
  SPIM->TXFTLR = Threshold;
}

/**
 * @brief  Returns the number of data can be read from Rx FIFO.
 * @return The number of data can be read from Rx FIFO.
 */
uint8_t SPIM_GetRxFIFOLevel(void)
{
  return SPIM->RXFLR;
}

/**
 * @brief  Returns the number of data left in Tx FIFO.
 * @return The number of data left in Tx FIFO.
 */
uint8_t SPIM_GetTxFIFOLevel(void)
{
  return SPIM->TXFLR;
}

/**
 * @brief  Configures the rxd sample delay value.
 * @param  DelayValue: specifies the rxd sample delay value.
 * @return None.
 */
void SPIM_RxdSampleDelayConfig(uint8_t DelayValue)
{
  SPIM->RX_SAMPLE_DLY = DelayValue;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
