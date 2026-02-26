/**
 * @file    wb7720_pct.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the PCT firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_pct.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup PCT
  * @brief PCT driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup PCT_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the PCTx peripheral registers to their default reset values.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @return None
 */
void PCT_DeInit(PCT_TypeDef* PCTx)
{
  if (PCTx == PCT0)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT0, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT0, DISABLE);
  }
  else if (PCTx == PCT1)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT1, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT1, DISABLE);
  }
  else if (PCTx == PCT2)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT2, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT2, DISABLE);
  }
  else if (PCTx == PCT3)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT3, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT3, DISABLE);
  }
  else if (PCTx == PCT4)
  {
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT4, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_PCT4, DISABLE);
  }
}

/**
 * @brief  Initializes the PCTx peripheral time base.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_Prescaler: specifies the Prescaler configuration.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_Prescaler_4
 *         @arg @ref PCT_Prescaler_8
 *         @arg @ref PCT_Prescaler_12
 *         @arg @ref PCT_Prescaler_16
 *         @arg @ref PCT_Prescaler_24
 *         @arg @ref PCT_Prescaler_32
 *         @arg @ref PCT_Prescaler_64
 * @param  Autoreload: specifies the Autoreload register value.
 * @return None
 */
void PCT_TimeBaseInit(PCT_TypeDef* PCTx, uint8_t PCT_Prescaler, uint16_t Autoreload)
{
  PCTx->CFGR = (PCTx->CFGR & 0x81) | PCT_Prescaler;
  PCTx->ARRL = Autoreload;
  PCTx->ARRH = Autoreload >> 8;
}

/**
 * @brief  Configures the PCTx Prescaler.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Prescaler: specifies the Prescaler configuration.
 * @return None
 */
void PCT_PrescalerConfig(PCT_TypeDef* PCTx, uint8_t PCT_Prescaler)
{
  PCTx->CFGR = (PCTx->CFGR & 0x81) | PCT_Prescaler;
}

/**
 * @brief  Sets the PCTx Autoreload Register value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Autoreload: specifies the Autoreload register new value.
 * @return None
 */
void PCT_SetAutoreload(PCT_TypeDef* PCTx, uint16_t Autoreload)
{
  PCTx->ARRL = Autoreload;
  PCTx->ARRH = Autoreload >> 8;
}

/**
 * @brief  Configures the PCTx's One Pulse Mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  NewState: new state of the One Pulse Mode.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void PCT_OnePulseModeConfig(PCT_TypeDef* PCTx, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    PCTx->CFGR2 |= PCT_CFGR2_OPM;
  }
  else {
    PCTx->CFGR2 &= ~PCT_CFGR2_OPM;
  }
}

/**
 * @brief  Enables or disables the PCTx peripheral Preload register on CCRx.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  NewState: new state of the Preload register on CCRx.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void PCT_CCRPreloadConfig(PCT_TypeDef* PCTx, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    PCTx->CFGR2 |= PCT_CFGR2_CCRPE;
  }
  else {
    PCTx->CFGR2 &= ~PCT_CFGR2_CCRPE;
  }
}

/**
 * @brief  Sets the PCTx Counter Register value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Counter: specifies the Counter register new value.
 * @return None
 */
void PCT_SetCounter(PCT_TypeDef* PCTx, uint16_t Counter)
{
  PCTx->CNTL = Counter;
  PCTx->CNTH = Counter >> 8;
}

/**
 * @brief  Gets the PCTx Counter value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @return Counter Register value.
 */
uint16_t PCT_GetCounter(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CNT;
}

/**
 * @brief  Enables or disables the specified PCTx Counter.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  NewState: new state of the PCTx Counter.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void PCT_Cmd(PCT_TypeDef* PCTx, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    PCTx->CSR |= PCT_CSR_CEN;
  }
  else {
    PCTx->CSR &= ~PCT_CSR_CEN;
  }
}

/**
 * @brief  Enables or disables the PCTx's interrupts.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_IT: specifies the PCT interrupt sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *         @arg @ref PCT_IT_OVF: Counter overflow interrupt.
 *         @arg @ref PCT_IT_CC0: Compare/Capture 0 interrupt.
 *         @arg @ref PCT_IT_CC1: Compare/Capture 1 interrupt.
 *         @arg @ref PCT_IT_CC2: Compare/Capture 2 interrupt.
 *         @arg @ref PCT_IT_CC3: Compare/Capture 3 interrupt.
 *         @arg @ref PCT_IT_CC4: Compare/Capture 4 interrupt.
 *         @arg @ref PCT_IT_CC5: Compare/Capture 5 interrupt.
 * @note   PCT3 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1, PCT_IT_CC2 or PCT_IT_CC3.
 * @note   PCT4 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1 or PCT_IT_CC2.
 * @param  NewState: new state of the interrupts.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void PCT_ITConfig(PCT_TypeDef* PCTx, uint8_t PCT_IT, FunctionalState NewState)
{
  int bitnum;
  bitnum = 0;
  while (PCT_IT != 0)
  {
    if ((PCT_IT & 0x01) != 0)
    {
      if (bitnum < 6)
      {
        if (NewState != DISABLE) {
          *((__IO uint32_t*)(((uint32_t)&PCTx->CCMR0) + bitnum*4)) |= PCT_CCMR_CCIE;
        }
        else {
          *((__IO uint32_t*)(((uint32_t)&PCTx->CCMR0) + bitnum*4)) &= ~PCT_CCMR_CCIE;
        }
      }
      else if (bitnum == 7)
      {
        if (NewState != DISABLE) {
          PCTx->CFGR |= PCT_CFGR_OVFIE;
        }
        else {
          PCTx->CFGR &= ~PCT_CFGR_OVFIE;
        }
      }
    }
    PCT_IT >>= 1;
    bitnum++;
  }
}

/**
 * @brief  Checks whether the specified PCT flag is set or not.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_FLAG: specifies the flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_FLAG_OVF: Counter overflow flag.
 *         @arg @ref PCT_FLAG_CC0: Compare/Capture 0 interrupt flag.
 *         @arg @ref PCT_FLAG_CC1: Compare/Capture 1 interrupt flag.
 *         @arg @ref PCT_FLAG_CC2: Compare/Capture 2 interrupt flag.
 *         @arg @ref PCT_FLAG_CC3: Compare/Capture 3 interrupt flag.
 *         @arg @ref PCT_FLAG_CC4: Compare/Capture 4 interrupt flag.
 *         @arg @ref PCT_FLAG_CC5: Compare/Capture 5 interrupt flag.
 * @note   PCT3 can have only PCT_FLAG_OVF, PCT_FLAG_CC0, PCT_FLAG_CC1, PCT_FLAG_CC2 or PCT_FLAG_CC3.
 * @note   PCT4 can have only PCT_FLAG_OVF, PCT_FLAG_CC0, PCT_FLAG_CC1 or PCT_FLAG_CC2.
 * @return The new state of PCT_FLAG (SET or RESET).
 */
FlagStatus PCT_GetFlagStatus(PCT_TypeDef* PCTx, uint8_t PCT_FLAG)
{
  FlagStatus bitstatus = RESET;

  if ((PCTx->CSR & PCT_FLAG) != (uint32_t)RESET)
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
 * @brief  Clears the PCTx's pending flags.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_FLAG: specifies the flag bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref PCT_FLAG_OVF: Counter overflow flag.
 *         @arg @ref PCT_FLAG_CC0: Compare/Capture 0 interrupt flag.
 *         @arg @ref PCT_FLAG_CC1: Compare/Capture 1 interrupt flag.
 *         @arg @ref PCT_FLAG_CC2: Compare/Capture 2 interrupt flag.
 *         @arg @ref PCT_FLAG_CC3: Compare/Capture 3 interrupt flag.
 *         @arg @ref PCT_FLAG_CC4: Compare/Capture 4 interrupt flag.
 *         @arg @ref PCT_FLAG_CC5: Compare/Capture 5 interrupt flag.
 * @note   PCT3 can have only PCT_FLAG_OVF, PCT_FLAG_CC0, PCT_FLAG_CC1, PCT_FLAG_CC2 or PCT_FLAG_CC3.
 * @note   PCT4 can have only PCT_FLAG_OVF, PCT_FLAG_CC0, PCT_FLAG_CC1 or PCT_FLAG_CC2.
 * @return None.
 */
void PCT_ClearFlag(PCT_TypeDef* PCTx, uint8_t PCT_FLAG)
{
  PCTx->CSR &= ~PCT_FLAG;
}

/**
 * @brief  Checks whether the PCT interrupt has occurred or not.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_IT: specifies the PCT interrupt source to check.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_IT_OVF: Counter overflow interrupt source.
 *         @arg @ref PCT_IT_CC0: Compare/Capture 0 interrupt source.
 *         @arg @ref PCT_IT_CC1: Compare/Capture 1 interrupt source.
 *         @arg @ref PCT_IT_CC2: Compare/Capture 2 interrupt source.
 *         @arg @ref PCT_IT_CC3: Compare/Capture 3 interrupt source.
 *         @arg @ref PCT_IT_CC4: Compare/Capture 4 interrupt source.
 *         @arg @ref PCT_IT_CC5: Compare/Capture 5 interrupt source.
 * @note   PCT3 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1, PCT_IT_CC2 or PCT_IT_CC3.
 * @note   PCT4 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1 or PCT_IT_CC2.
 * @return The new state of PCT_IT (SET or RESET).
 */
ITStatus PCT_GetITStatus(PCT_TypeDef* PCTx, uint8_t PCT_IT)
{
  ITStatus bitstatus = RESET;

  if ((PCTx->CSR & PCT_IT) != (uint32_t)RESET)
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
 * @brief  Clears the PCTx's interrupt pending bits.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_IT: specifies the pending bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref PCT_IT_OVF: Counter overflow interrupt source.
 *         @arg @ref PCT_IT_CC0: Compare/Capture 0 interrupt source.
 *         @arg @ref PCT_IT_CC1: Compare/Capture 1 interrupt source.
 *         @arg @ref PCT_IT_CC2: Compare/Capture 2 interrupt source.
 *         @arg @ref PCT_IT_CC3: Compare/Capture 3 interrupt source.
 *         @arg @ref PCT_IT_CC4: Compare/Capture 4 interrupt source.
 *         @arg @ref PCT_IT_CC5: Compare/Capture 5 interrupt source.
 * @note   PCT3 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1, PCT_IT_CC2 or PCT_IT_CC3.
 * @note   PCT4 can have only PCT_IT_OVF, PCT_IT_CC0, PCT_IT_CC1 or PCT_IT_CC2.
 * @return None.
 */
void PCT_ClearITPendingBit(PCT_TypeDef* PCTx, uint8_t PCT_IT)
{
  PCTx->CSR &= ~PCT_IT;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 0 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC0ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR0 = (PCTx->CCMR0 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 1 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC1ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR1 = (PCTx->CCMR1 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 2 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC2ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR2 = (PCTx->CCMR2 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 3 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC3ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR3 = (PCTx->CCMR3 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 4 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC4ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR4 = (PCTx->CCMR4 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Configures the PCTx Capture/Compare Channel 5 mode.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @param  PCT_CCMode: specifies the Capture/Compare mode.
 *         This parameter can be one of the following values:
 *         @arg @ref PCT_CCMode_Disable: Capture/Compare Channel is disabled.
 *         @arg @ref PCT_CCMode_PWM1: PWM mode 1. CNT<CCRx output low, CNT>=CCRx output high.
 *         @arg @ref PCT_CCMode_PWM2: PWM mode 2. CNT<CCRx output high, CNT>=CCRx output low.
 *         @arg @ref PCT_CCMode_Match: When CNT=CCRx only causes the CCxIF flag to be set.
 *         @arg @ref PCT_CCMode_Toggle: Toggle the channel output when CNT=CCRx.
 *         @arg @ref PCT_CCMode_Capture_Rising: Capture the channel input rising edge.
 *         @arg @ref PCT_CCMode_Capture_Falling: Capture the channel input falling edge.
 *         @arg @ref PCT_CCMode_Capture_BothEdge: Capture the channel input rising and falling edge.
 * @return None.
 */
void PCT_CC5ModeConfig(PCT_TypeDef* PCTx, uint8_t PCT_CCMode)
{
  PCTx->CCMR5 = (PCTx->CCMR5 & 0x01) | PCT_CCMode;
}

/**
 * @brief  Sets the PCTx PWM final compare value register A value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  CompareA: specifies the PWM final compare value register A value.
 * @return None
 */
void PCT_SetFinalCompareA(PCT_TypeDef* PCTx, uint16_t CompareA)
{
  PCTx->FTCMPAL = CompareA;
  PCTx->FTCMPAH = CompareA >> 8;
}

/**
 * @brief  Sets the PCTx PWM final compare value register B value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @param  CompareB: specifies the PWM final compare value register B value.
 * @return None
 */
void PCT_SetFinalCompareB(PCT_TypeDef* PCTx, uint16_t CompareB)
{
  PCTx->FTCMPBL = CompareB;
  PCTx->FTCMPBH = CompareB >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 0 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Compare0: specifies the Capture/Compare value register 0 value.
 * @return None
 */
void PCT_SetCompare0(PCT_TypeDef* PCTx, uint16_t Compare0)
{
  PCTx->CCR0L = Compare0;
  PCTx->CCR0H = Compare0 >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 1 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Compare1: specifies the Capture/Compare value register 1 value.
 * @return None
 */
void PCT_SetCompare1(PCT_TypeDef* PCTx, uint16_t Compare1)
{
  PCTx->CCR1L = Compare1;
  PCTx->CCR1H = Compare1 >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 2 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @param  Compare2: specifies the Capture/Compare value register 2 value.
 * @return None
 */
void PCT_SetCompare2(PCT_TypeDef* PCTx, uint16_t Compare2)
{
  PCTx->CCR2L = Compare2;
  PCTx->CCR2H = Compare2 >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 3 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3.
 * @param  Compare3: specifies the Capture/Compare value register 3 value.
 * @return None
 */
void PCT_SetCompare3(PCT_TypeDef* PCTx, uint16_t Compare3)
{
  PCTx->CCR3L = Compare3;
  PCTx->CCR3H = Compare3 >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 4 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @param  Compare4: specifies the Capture/Compare value register 4 value.
 * @return None
 */
void PCT_SetCompare4(PCT_TypeDef* PCTx, uint16_t Compare4)
{
  PCTx->CCR4L = Compare4;
  PCTx->CCR4H = Compare4 >> 8;
}

/**
 * @brief  Sets the PCTx Capture/Compare value register 5 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @param  Compare5: specifies the Capture/Compare value register 5 value.
 * @return None
 */
void PCT_SetCompare5(PCT_TypeDef* PCTx, uint16_t Compare5)
{
  PCTx->CCR5L = Compare5;
  PCTx->CCR5H = Compare5 >> 8;
}

/**
 * @brief  Gets the PCTx Input Capture 0 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @return Capture/Compare 0 register value.
 */
uint16_t PCT_GetCapture0(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR0;
}

/**
 * @brief  Gets the PCTx Input Capture 1 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @return Capture/Compare 1 register value.
 */
uint16_t PCT_GetCapture1(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR1;
}

/**
 * @brief  Gets the PCTx Input Capture 2 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3, PCT4.
 * @return Capture/Compare 2 register value.
 */
uint16_t PCT_GetCapture2(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR2;
}

/**
 * @brief  Gets the PCTx Input Capture 3 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2, PCT3.
 * @return Capture/Compare 3 register value.
 */
uint16_t PCT_GetCapture3(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR3;
}

/**
 * @brief  Gets the PCTx Input Capture 4 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @return Capture/Compare 4 register value.
 */
uint16_t PCT_GetCapture4(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR4;
}

/**
 * @brief  Gets the PCTx Input Capture 5 value.
 * @param  PCTx: Pointer to selected PCT peripheral.
 *         This parameter can be one of the following values:
 *         PCT0, PCT1, PCT2.
 * @return Capture/Compare 5 register value.
 */
uint16_t PCT_GetCapture5(PCT_TypeDef* PCTx)
{
  return (uint16_t)PCTx->CCR5;
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
