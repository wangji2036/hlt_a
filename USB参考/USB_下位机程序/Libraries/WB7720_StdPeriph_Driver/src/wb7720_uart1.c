/**
 * @file    wb7720_uart1.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the UART1 firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_uart1.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup UART1
  * @brief UART1 driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup UART1_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the UART1 peripheral registers to their default reset values.
 * @return None
 */
void UART1_DeInit(void)
{
  RCC_APBPeriphResetCmd(RCC_APBPeriph_UART1, ENABLE);
  RCC_APBPeriphResetCmd(RCC_APBPeriph_UART1, DISABLE);
}

/**
 * @brief  Initializes the UART1 peripheral according to the specified
 *         parameters.
 * @param  UART1_BaudRate: specifies the communication baud rate.
 * @param  UART1_Mode: specifies whether the Receive or Transmit mode is enabled or disabled.
*          This parameter can be any combination of the following values:
*            @arg @ref UART1_Mode_TX
*            @arg @ref UART1_Mode_RX
 * @return None
 */
void UART1_Init(uint32_t UART1_BaudRate, uint32_t UART1_Mode)
{
  uint32_t divider, apbclock;
  RCC_ClocksTypeDef RCC_Clocks;

  RCC_GetClocksFreq(&RCC_Clocks);
  apbclock = RCC_Clocks.APBCLK_Frequency;

  if (UART1_BaudRate != 0)
  {
    // round off
    divider = (apbclock + (UART1_BaudRate >> 1)) / UART1_BaudRate;
  }
  else
  {
    divider = 0;
  }

  UART1->BDR = divider;
  UART1->CR = (UART1->CR & (~(UART1_CR_TXE | UART1_CR_RXE))) | UART1_Mode;
}


/**
 * @brief  Write data to data register.
 * @param  Data: the data to write.
 * @return None
 */
void UART1_WriteData(uint8_t Data)
{
  UART1->DR = Data;
}

/**
 * @brief  Read data from data register.
 * @param  None
 * @return The received data
 */
uint8_t UART1_ReadData(void)
{
  return (uint8_t)UART1->DR;
}

/**
 * @brief  Enables or disables the UART1's interrupts.
 * @param  UART1_IT: specifies the UART1 interrupt sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_IT_TX: Transmit interrupt.
 *         @arg @ref UART1_IT_RX: Receive interrupt.
 *         @arg @ref UART1_IT_TXO: Transmit buffer overrun interrupt.
 *         @arg @ref UART1_IT_RXO: Receive buffer overrun interrupt.
 * @param  NewState: new state of the interrupts.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void UART1_ITConfig(uint32_t UART1_IT, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    UART1->CR |= (UART1_IT << 2);
  }
  else {
    UART1->CR &= (~(UART1_IT << 2));
  }
}

/**
 * @brief  Checks whether the UART1 flag is set or not.
 * @param  UART1_FLAG: specifies the flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref UART1_FLAG_TXF: Transmit buffer full flag.
 *         @arg @ref UART1_FLAG_RXF: Receive buffer full flag.
 *         @arg @ref UART1_FLAG_TXO: Transmit buffer overrun flag.
 *         @arg @ref UART1_FLAG_RXO: Receive buffer overrun flag.
 * @return The new state of UART1_FLAG (SET or RESET).
 */
FlagStatus UART1_GetFlagStatus(uint32_t UART1_FLAG)
{
  FlagStatus bitstatus = RESET;

  if ((UART1->SR & UART1_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
 * @brief  Clears the UART1's pending flags.
 * @param  UART1_FLAG: specifies the flag to clear. 
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_FLAG_TXO: Transmit buffer overrun flag.
 *         @arg @ref UART1_FLAG_RXO: Receive buffer overrun flag.
 * @return None
 */
void UART1_ClearFlag(uint32_t UART1_FLAG)
{
  UART1->SR = UART1_FLAG;
}

/**
 * @brief  Checks whether the specified UART1 interrupt has occurred or not.
 * @param  UART1_IT: specifies the UART1 interrupt source to check.
 *         This parameter can be one of the following values:
 *         @arg @ref UART1_IT_TX: Transmit interrupt
 *         @arg @ref UART1_IT_RX: Receive interrupt
 *         @arg @ref UART1_IT_TXO: Transmit buffer overrun interrupt
 *         @arg @ref UART1_IT_RXO: Receive buffer overrun interrupt
 * @return The new state of UART1_IT (SET or RESET).
 */
ITStatus UART1_GetITStatus(uint32_t UART1_IT)
{
  ITStatus bitstatus = RESET;

  /* Check the status of the specified UART1 interrupt */
  if (UART1->ISR & UART1_IT)
  {
    /* UART1_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* UART1_IT is reset */
    bitstatus = RESET;
  }
  /* Return the UART1_IT status */
  return bitstatus;
}

/**
 * @brief  Clears the UART1's interrupt pending bits.
 * @param  UART1_IT: specifies the UART1 interrupt pending bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_IT_TX: Transmit interrupt
 *         @arg @ref UART1_IT_RX: Receive interrupt
 *         @arg @ref UART1_IT_TXO: Transmit buffer overrun interrupt
 *         @arg @ref UART1_IT_RXO: Receive buffer overrun interrupt
 * @return None
 */
void UART1_ClearITPendingBit(uint32_t UART1_IT)
{
  UART1->ISR = UART1_IT;
}

/**
 * @brief  Initializes the UART1 auto baud rate detection feature according to the specified parameters
 *         in the UART1_ABR_InitStruct.
 * @param  UART1_ABR_InitStruct: pointer to an UART1_ABR_InitTypeDef structure that contains
 *         the configuration information for the UART1 auto baud rate detection feature.
 * @return None
 */
void UART1_ABR_Init(UART1_ABR_InitTypeDef* UART1_ABR_InitStruct)
{
  UART1->ABRCON = (UART1->ABRCON & ~0x01FEU) | UART1_ABR_InitStruct->UART1_ABR_LowPulseWidth |
              UART1_ABR_InitStruct->UART1_ABR_CharacterWidth | 
              (((uint32_t)UART1_ABR_InitStruct->UART1_ABR_UpdateBaudRate) << 1) | 
              (((uint32_t)UART1_ABR_InitStruct->UART1_ABR_ReceiveDuringAutoBaudRate) << 8);
}

/**
 * @brief  Fills each UART1_ABR_InitStruct member with its default value.
 * @param  UART1_ABR_InitStruct: pointer to a UART1_ABR_InitTypeDef structure
 *         which will be initialized.
 * @return None
 */
void UART1_ABR_StructInit(UART1_ABR_InitTypeDef* UART1_ABR_InitStruct)
{
  /* UART1_ABR_InitStruct members default value */
  UART1_ABR_InitStruct->UART1_ABR_LowPulseWidth = UART1_ABR_LowPulseWidth_1Bit;
  UART1_ABR_InitStruct->UART1_ABR_CharacterWidth = UART1_ABR_CharacterWidth_8Bit;
  UART1_ABR_InitStruct->UART1_ABR_UpdateBaudRate = ENABLE;
  UART1_ABR_InitStruct->UART1_ABR_ReceiveDuringAutoBaudRate = DISABLE;
}

/**
 * @brief  Enables or disables the UART1 auto baud rate detection feature.
 * @param  NewState: new state of the UART1 auto baud rate detection feature.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void UART1_ABR_Cmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    UART1->ABRCON |= UART1_ABRCON_ABREN;
  }
  else {
    UART1->ABRCON &= ~UART1_ABRCON_ABREN;
  }
}

/**
 * @brief  Returns the Auto baud rate result.
 * @param  None
 * @return The Auto baud rate result.
 */
uint32_t UART1_ABR_GetAutoBaudRateValue(void)
{
  return UART1->ABRVAL;
}

/**
 * @brief  Enables or disables the UART1 auto baud rate detection interrupts.
 * @param  UART1_ABR_IT: specifies the UART1 auto baud rate detection interrupt sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_ABR_IT_ABRD: Auto Baud Rate dectection done interrupt
 *         @arg @ref UART1_ABR_IT_ABROV: Auto Baud Rate overflow interrupt
 * @param  NewState: new state of the interrupts.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void UART1_ABR_ITConfig(uint32_t UART1_ABR_IT, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    UART1->ABRIE |= UART1_ABR_IT;
  }
  else {
    UART1->ABRIE &= ~UART1_ABR_IT;
  }
}

/**
 * @brief  Checks whether the UART1 auto baud rate detection flag is set or not.
 * @param  UART1_ABR_FLAG: specifies the flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref UART1_ABR_FLAG_ABRD: Auto Baud Rate dectection done flag.
 *         @arg @ref UART1_ABR_FLAG_ABROV: Auto Baud Rate overflow flag.
 * @return The new state of UART1_ABR_FLAG (SET or RESET).
 */
FlagStatus UART1_ABR_GetFlagStatus(uint32_t UART1_ABR_FLAG)
{
  FlagStatus bitstatus = RESET;

  if ((UART1->ABRSR & UART1_ABR_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  return bitstatus;
}

/**
 * @brief  Clears the UART1 auto baud rate detection pending flags.
 * @param  UART1_ABR_FLAG: specifies the flag to clear. 
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_ABR_FLAG_ABRD: Auto Baud Rate dectection done flag.
 *         @arg @ref UART1_ABR_FLAG_ABROV: Auto Baud Rate overflow flag.
 * @return None
 */
void UART1_ABR_ClearFlag(uint32_t UART1_ABR_FLAG)
{
  UART1->ABRIC = UART1_ABR_FLAG;
}

/**
 * @brief  Checks whether the specified UART1 auto baud rate detection interrupt has occurred or not.
 * @param  UART1_ABR_IT: specifies the UART1 auto baud rate detection interrupt source to check.
 *         This parameter can be one of the following values:
 *         @arg @ref UART1_ABR_IT_ABRD: Auto Baud Rate dectection done interrupt
 *         @arg @ref UART1_ABR_IT_ABROV: Auto Baud Rate overflow interrupt
 * @return The new state of UART1_ABR_IT (SET or RESET).
 */
ITStatus UART1_ABR_GetITStatus(uint32_t UART1_ABR_IT)
{
  ITStatus bitstatus = RESET;

  /* Check the status of the specified UART1 auto baud rate detection interrupt */
  if (UART1->ABRSR & UART1_ABR_IT)
  {
    /* UART1_ABR_IT is set */
    bitstatus = SET;
  }
  else
  {
    /* UART1_ABR_IT is reset */
    bitstatus = RESET;
  }
  /* Return the UART1_ABR_IT status */
  return bitstatus;
}

/**
 * @brief  Clears the UART1 auto baud rate detection interrupt pending bits.
 * @param  UART1_ABR_IT: specifies the UART1 auto baud rate detection interrupt pending bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref UART1_ABR_IT_ABRD: Auto Baud Rate dectection done interrupt
 *         @arg @ref UART1_ABR_IT_ABROV: Auto Baud Rate overflow interrupt
 * @return None
 */
void UART1_ABR_ClearITPendingBit(uint32_t UART1_ABR_IT)
{
  UART1->ABRIC = UART1_ABR_IT;
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
