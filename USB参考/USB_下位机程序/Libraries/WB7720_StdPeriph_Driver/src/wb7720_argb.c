/**
 * @file    wb7720_argb.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the ARGB firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_argb.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup ARGB
  * @brief ARGB driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup ARGB_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes all ARGB peripheral registers to their default reset values.
 * @param  None
 * @return None
 */
void ARGB_DeInit(void)
{
  RCC->APBRSTR |= (RCC_APBRSTR_ARGB03RST | RCC_APBRSTR_ARGB45RST);
  RCC->APBRSTR &= ~(RCC_APBRSTR_ARGB03RST | RCC_APBRSTR_ARGB45RST);
}

/**
 * @brief  Initializes the ARGBx peripheral according to the PinConfig.
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @param  Periodcnt: Configure a one-bit data width.
 *         The calculation method is N(us) = Periodcnt * system clock.
 * @param  T1h_cnt: Configures the high level duration in one-bit data width.
 *         The calculation method is N(us) = T1HCNT * system clock.
 * @param  Treset_cnt: Configures reset the period width of the signal.
 *         The calculation method is N(us) = Treset_cnt * system clock.
 * @return None
 */
void ARGB_Init(ARGB_TypeDef* ARGBx, uint8_t Periodcnt, uint8_t T1h_cnt, uint16_t Treset_cnt)
{
  ARGBx->PERIODCNT = Periodcnt;
  ARGBx->T1HCNT = T1h_cnt;
  ARGBx->TRSTCNTH = (uint8_t)(Treset_cnt >> 8);
  ARGBx->TRSTCNTL = (uint8_t)(Treset_cnt);
}

/**
 * @brief  Initializes the ARGBx peripheral according to the PinConfig.
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @param  Control_enable: Enable required functions.
 *    This parameter can be one of the following values:
 *      @arg ARGB_CTRL_ENABLE: LSI oscillator clock selected
 *      @arg ARGB_CTRL_TFEIE: PLL clock selected
 *      @arg ARGB_CTRL_FIFOR: HSI48 oscillator clock selected
 *      @arg ARGB_CTRL_GRSTIE: The source of system clock selected
 *      @arg ARGB_CTRL_GRST: The source of system clock selected
 * @param  NewState: new state of the MCO clock.
 *    This parameter can be: ENABLE or DISABLE.
 * @return None
 */

void ARGB_Cmd(ARGB_TypeDef* ARGBx, uint8_t Control_enable, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    ARGBx->CTRL |= Control_enable;
    if (Control_enable & 0x80)
    {
      ARGBx->CTRL |= 0x80;
    }
    while (ARGBx->CTRL & 0x80);
  }
  else
  {
    ARGBx->CTRL &= ~Control_enable;
  }
}

/**
 * @brief  Get whether the function is enabled
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @return Now ARGB control register value.
 */
FlagStatus ARGB_GetControlStatus(ARGB_TypeDef* ARGBx, uint8_t ARGB_Control)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified SPI flag */
  if (ARGBx->CTRL & ARGB_Control)
  {
    /* ARGB_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ARGB_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ARGB_FLAG status */
  return bitstatus;
}

/**
 * @brief  Write one data to Tx buffer.
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @param  Data: The data to write.
 * @return None
 */
void ARGB_WriteData(ARGB_TypeDef* ARGBx, uint8_t Data)
{
  ARGBx->DAT = Data;
}

/**
 * @brief  Generate a reset signal
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @return None
 */
void ARGB_WriteResetSignal(ARGB_TypeDef* ARGBx)
{
  ARGBx->CTRL |= 0x80;
  while (ARGBx->CTRL & 0x80);
}

/**
 * @brief  Checks whether the specified ARGBx flag is set or not.
 * @param  ARGBx: where x can be (0..5) to select the ARGB peripheral.
 * @param  ARGB_FLAG: specifies the ARGB flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref ARGB_FLAG_EMPTY: TX FIFO empty.
 *         @arg @ref ARGB_FLAG_FULL : TX FIFO full.
 *         @arg @ref ARGB_FLAG_GRSTF: Generate reset code finish flag.
 * @return The new state of ARGB_FLAG (SET or RESET).
 */
FlagStatus ARGB_GetFlagStatus(ARGB_TypeDef* ARGBx, uint8_t ARGB_FLAG)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified SPI flag */
  if (ARGBx->STATUS & ARGB_FLAG)
  {
    /* ARGB_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ARGB_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ARGB_FLAG status */
  return bitstatus;
}

/**
  * @brief  Clears the ARGB's pending flags.
  * @param  ARGB_FLAG: specifies the flag to clear.
  *          This parameter can be any combination of the following values:
  *            @arg ARGB_FLAG_RESET:  Generate reset code finish flag.
  * @retval None
  */
void ARGB_ClearFlag(ARGB_TypeDef* ARGBx, uint32_t ARGB_FLAG)
{
  ARGBx->STATUS = ARGB_FLAG;
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
