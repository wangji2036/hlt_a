/**
 * @file    wb7720_spi.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the SPI firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_spi.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup SPI
  * @brief SPI driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup SPI_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the SPIx peripheral registers to their default reset values.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @return None
 */
void SPI_DeInit(SPI_TypeDef* SPIx)
{
  if (SPIx == SPI1)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_SPI1, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_SPI1, DISABLE);
  }
}

/**
 * @brief  Initializes the SPIx peripheral according to the specified 
 *         parameters in the SPI_InitStruct.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure
 *         that contains the configuration information for the specified 
 *         SPI peripheral.
 * @return None
 */
void SPI_Init(SPI_TypeDef* SPIx, SPI_InitTypeDef* SPI_InitStruct)
{
  SPIx->CR = (SPIx->CR & 0xC0) | SPI_InitStruct->SPI_Mode |
        SPI_InitStruct->SPI_CPOL | SPI_InitStruct->SPI_CPHA |
        SPI_InitStruct->SPI_FirstBit | (SPI_InitStruct->SPI_ClockRateDivider & 0x03);
  SPIx->CR2 = SPI_InitStruct->SPI_ClockRateDivider >> 4;
}

/**
 * @brief  Fills each SPI_InitStruct member with its default value.
 * @param  SPI_InitStruct: pointer to a SPI_InitTypeDef structure
 *         which will be initialized.
 * @return None
 */
void SPI_StructInit(SPI_InitTypeDef* SPI_InitStruct)
{
  SPI_InitStruct->SPI_Mode = SPI_Mode_Master;
  SPI_InitStruct->SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStruct->SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStruct->SPI_ClockRateDivider = SPI_ClockRateDivider_8;
  SPI_InitStruct->SPI_FirstBit = SPI_FirstBit_MSB;
}

/**
 * @brief  Enables or disables the specified SPI peripheral.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  NewState: new state of the SPIx peripheral.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SPI_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    SPIx->CR |= SPI_CR_SPE;
  }
  else {
    SPIx->CR &= ~SPI_CR_SPE;
  }
}

/**
 * @brief  Read one data from Rx buffer.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @return The read data.
 */
uint8_t SPI_ReadData(SPI_TypeDef* SPIx)
{
  return SPIx->DR;
}

/**
 * @brief  Write one data to Tx buffer.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  Data: The data to write.
 * @return None
 */
void SPI_WriteData(SPI_TypeDef* SPIx, uint8_t Data)
{
  SPIx->DR = Data;
}

/**
 * @brief  Enables or disables the SPI interrupt.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  NewState: new state of the SPI interrupt.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None.
 */
void SPI_ITConfig(SPI_TypeDef* SPIx, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    SPIx->CR |= SPI_CR_IE;
  }
  else
  {
    SPIx->CR &= ~SPI_CR_IE;
  }
}

/**
 * @brief  Checks whether the specified SPIx flag is set or not.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  SPI_FLAG: specifies the SPI flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref SPI_FLAG_WCOL: Write collision flag.
 *         @arg @ref SPI_FLAG_SPIF: SPI data transfer complete flag.
 * @return The new state of SPI_FLAG (SET or RESET).
 */
FlagStatus SPI_GetFlagStatus(SPI_TypeDef* SPIx, uint8_t SPI_FLAG)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified SPI flag */
  if (SPIx->SR & SPI_FLAG)
  {
    /* SPI_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* SPI_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the SPI_FLAG status */
  return bitstatus;
}

/**
 * @brief  Clears the SPIx's pending flags.
 * @param  SPIx: Pointer to selected SPI peripheral.
 *         This parameter can be one of the following values:
 *         SPI1.
 * @param  SPI_FLAG: specifies the flag to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref SPI_FLAG_WCOL: Write collision flag.
 *         @arg @ref SPI_FLAG_SPIF: SPI data transfer complete flag.
 * @return None.
 */
void SPI_ClearFlag(SPI_TypeDef* SPIx, uint8_t SPI_FLAG)
{
  SPIx->SR &= ~SPI_FLAG;
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
