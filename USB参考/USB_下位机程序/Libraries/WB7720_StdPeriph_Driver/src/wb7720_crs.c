/**
 * @file    wb7720_crs.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the CRS firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_crs.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup CRS
  * @brief CRS driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup CRS_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes CRS peripheral registers to their default reset values.
 * @return None
 */
void CRS_DeInit(void)
{
  RCC->APBRSTR |= RCC_APBRSTR_CRSRST;
  RCC->APBRSTR &= ~RCC_APBRSTR_CRSRST;
}

/**
 * @brief  Adjusts the Internal 48MHz oscillator (HSI48) calibration value.
 * @note   The calibration is used to compensate for the variations in voltage
 *         and temperature that influence the frequency of the internal HSI48 RC.
 * @param  CRS_HSI48CalibrationValue: specifies the calibration value to set.
 * @return None
 */
void CRS_AdjustHSI48CalibrationValue(uint16_t CRS_HSI48CalibrationValue)
{
  CRS->CR = (CRS->CR & ~CRS_CR_TRIM_Msk) | (CRS_HSI48CalibrationValue << CRS_CR_TRIM_Pos);
}

/**
 * @brief  Enables or disables the oscillator clock for frequency error counter.
 * @param  NewState: new state of the frequency error counter.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void CRS_FrequencyErrorCounterCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    CRS->CR |= CRS_CR_CEN;
  }
  else
  {
    CRS->CR &= ~CRS_CR_CEN;
  }
}

/**
 * @brief  Enables or disables the automatic hardware adjustement of TRIM bits.
 * @param  NewState: new state of the automatic trimming.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void CRS_AutomaticCalibrationCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    CRS->CR |= CRS_CR_AUTOTRIMEN;
  }
  else
  {
    CRS->CR &= ~CRS_CR_AUTOTRIMEN;
  }
}

/**
 * @brief  Generate the software synchronization event.
 * @return None
 */
void CRS_SoftwareSynchronizationGenerate(void)
{
  CRS->CR |= CRS_CR_SWSYNC;
}

/**
 * @brief  Sets the frequency error counter reload value.
 * @param  CRS_ReloadValue: specifies the reload value.
 * @return None
 */
void CRS_FrequencyErrorCounterReload(uint16_t CRS_ReloadValue)
{
  CRS->CFGR = (CRS->CFGR & ~CRS_CFGR_RELOAD_Msk) | ((uint32_t)CRS_ReloadValue);
}

/**
 * @brief  Sets the frequency error limit.
 * @param  CRS_ErrorLimitValue: specifies the frequency error limit value.
 * @return None
 */
void CRS_FrequencyErrorLimitConfig(uint8_t CRS_ErrorLimitValue)
{
  CRS->CFGR = (CRS->CFGR & ~CRS_CFGR_FELIM_Msk) | (((uint32_t)CRS_ErrorLimitValue) << CRS_CFGR_FELIM_Pos);
}

/**
 * @brief  Configures the synchronization prescaler division factor.
 * @param  CRS_Prescaler: specifies the synchronization division factor
 *         This parameter can be one of the following values:
 *         @arg @ref CRS_SYNC_Div1
 *         @arg @ref CRS_SYNC_Div2
 *         @arg @ref CRS_SYNC_Div4
 *         @arg @ref CRS_SYNC_Div8
 *         @arg @ref CRS_SYNC_Div16
 *         @arg @ref CRS_SYNC_Div32
 *         @arg @ref CRS_SYNC_Div64
 *         @arg @ref CRS_SYNC_Div128
 * @return None
 */
void CRS_SynchronizationPrescalerConfig(uint32_t CRS_Prescaler)
{
  /* Check the parameters */
  assert_param(IS_CRS_SYNC_DIV(CRS_Prescaler));

  CRS->CFGR = (CRS->CFGR & ~CRS_CFGR_SYNCDIV_Msk) | CRS_Prescaler;
}

/**
 * @brief  Configures the synchronization source.
 * @param  CRS_SYNCSource: specifies the synchronization source
 *         This parameter can be one of the following values:
 *         @arg @ref CRS_SYNCSource_GPIO: Synchro Signal soucre GPIO (PA0 only).
 *         @arg @ref CRS_SYNCSource_USB: Synchro Signal source USB SOF.
 *         @arg @ref CRS_SYNCSource_EXTI5: Synchro Signal source EXTI5 interrupt.
 * @return None
 */
void CRS_SynchronizationSourceConfig(uint32_t CRS_SYNCSource)
{
  /* Check the parameters */
  assert_param(IS_CRS_SYNC_SOURCE(CRS_SYNCSource));

  CRS->CFGR = (CRS->CFGR & ~CRS_CFGR_SYNCSRC_Msk) | CRS_SYNCSource;
}

/**
 * @brief  Configures the synchronization signal polarity.
 * @param  CRS_SYNCPolarity: specifies the synchronization signal polarity
 *         This parameter can be one of the following values:
 *         @arg @ref CRS_SYNCPolarity_Rising
 *         @arg @ref CRS_SYNCPolarity_Falling
 * @return None
 */
void CRS_SynchronizationPolarityConfig(uint32_t CRS_SYNCPolarity)
{
  /* Check the parameters */
  assert_param(IS_CRS_SYNC_POLARITY(CRS_SYNCPolarity));
  
  CRS->CFGR = (CRS->CFGR & ~CRS_CFGR_SYNCPOL) | CRS_SYNCPolarity;
}

/**
 * @brief  Returns the Relaod value.
 * @return The reload value.
 */
uint16_t CRS_GetReloadValue(void)
{
  return ((uint16_t)(CRS->CFGR & CRS_CFGR_RELOAD_Msk));
}

/**
 * @brief  Returns the HSI48 Calibration value.
 * @return The calibration value.
 */
uint16_t CRS_GetHSI48CalibrationValue(void)
{
  return ((uint16_t)((CRS->CR & CRS_CR_TRIM_Msk) >> CRS_CR_TRIM_Pos));
}

/**
 * @brief  Returns the frequency error capture.
 * @return The frequency error capture value.
 */
uint16_t CRS_GetFrequencyErrorValue(void)
{
  return ((uint16_t)((CRS->ISR & CRS_ISR_FECAP_Msk) >> CRS_ISR_FECAP_Pos));
}

/**
 * @brief  Returns the frequency error direction.
 * @return The frequency error direction. The returned value can be one 
 *         of the following values:
 *           - 0x0000: Up counting
 *           - 0x8000: Down counting
 */
uint32_t CRS_GetFrequencyErrorDirection(void)
{
  return ((uint32_t)(CRS->ISR & CRS_ISR_FEDIR));
}

/**
 * @brief  Enables or disables the specified CRS interrupts.
 * @param  CRS_IT: specifies the CRS interrupt sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *         @arg @ref CRS_IT_SYNCOK
 *         @arg @ref CRS_IT_SYNCWARN
 *         @arg @ref CRS_IT_ERR
 *         @arg @ref CRS_IT_ESYNC
 * @param  NewState: new state of the specified CRS interrupts.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None.
 */
void CRS_ITConfig(uint32_t CRS_IT, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_CRS_IT(CRS_IT));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    CRS->CR |= CRS_IT;
  }
  else
  {
    CRS->CR &= ~CRS_IT;
  }
}

/**
 * @brief  Checks whether the specified CRS flag is set or not.
 * @param  CRS_FLAG: specifies the flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref CRS_FLAG_SYNCOK
 *         @arg @ref CRS_FLAG_SYNCWARN
 *         @arg @ref CRS_FLAG_ERR
 *         @arg @ref CRS_FLAG_ESYNC
 *         @arg @ref CRS_FLAG_TRIMOVF
 *         @arg @ref CRS_FLAG_SYNCERR
 *         @arg @ref CRS_FLAG_SYNCMISS
 * @return The new state of CRS_FLAG (SET or RESET).
 */
FlagStatus CRS_GetFlagStatus(uint32_t CRS_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_CRS_FLAG(CRS_FLAG));

  if ((CRS->ISR & CRS_FLAG) != RESET)
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
 * @brief  Clears the CRS specified Flag.
 * @param  CRS_FLAG: specifies the flag bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref CRS_FLAG_SYNCOK
 *         @arg @ref CRS_FLAG_SYNCWARN
 *         @arg @ref CRS_FLAG_ERR
 *         @arg @ref CRS_FLAG_ESYNC
 *         @arg @ref CRS_FLAG_TRIMOVF
 *         @arg @ref CRS_FLAG_SYNCERR
 *         @arg @ref CRS_FLAG_SYNCMISS
 * @return None.
 */
void CRS_ClearFlag(uint32_t CRS_FLAG)
{
  /* Check the parameters */
  assert_param(IS_CRS_FLAG(CRS_FLAG));

  if ((CRS_FLAG & 0x0700) != 0)
  {
    CRS_FLAG |= CRS_ICR_ERRC;
  }
  CRS->ICR = CRS_FLAG;
}

/**
 * @brief  Checks whether the specified CRS IT pending bit is set or not.
 * @param  CRS_IT: specifies the IT pending bit to check.
 *         This parameter can be one of the following values:
 *         @arg @ref CRS_IT_SYNCOK
 *         @arg @ref CRS_IT_SYNCWARN
 *         @arg @ref CRS_IT_ERR
 *         @arg @ref CRS_IT_ESYNC
 *         @arg @ref CRS_IT_TRIMOVF
 *         @arg @ref CRS_IT_SYNCERR
 *         @arg @ref CRS_IT_SYNCMISS
 * @return The new state of CRS_IT (SET or RESET).
 */
ITStatus CRS_GetITStatus(uint32_t CRS_IT)
{
  ITStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_CRS_GET_IT(CRS_IT));

  if ((CRS->ISR & CRS_IT) != RESET)
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
 * @brief  Clears the CRS specified interrupt pending bit.
 * @param  CRS_IT: specifies the pending bit to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref CRS_IT_SYNCOK
 *         @arg @ref CRS_IT_SYNCWARN
 *         @arg @ref CRS_IT_ERR
 *         @arg @ref CRS_IT_ESYNC
 *         @arg @ref CRS_IT_TRIMOVF
 *         @arg @ref CRS_IT_SYNCERR
 *         @arg @ref CRS_IT_SYNCMISS
 * @return None.
 */
void CRS_ClearITPendingBit(uint32_t CRS_IT)
{
  /* Check the parameters */
  assert_param(IS_CRS_CLEAR_IT(CRS_IT));

  if ((CRS_IT & 0x0700) != 0)
  {
    CRS_IT |= CRS_ICR_ERRC;
  }
  CRS->ICR = CRS_IT;
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
