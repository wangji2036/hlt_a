/**
 * @file    wb7720_adc.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the ADC firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_adc.h"
#include "wb7720_rcc.h"
#include "wb7720_anctl.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup ADC
  * @brief ADC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup ADC_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the ADC peripheral registers to their default reset values.
 * @param  None
 * @return None
 */
void ADC_DeInit(void)
{
  RCC_APBPeriphResetCmd(RCC_APBPeriph_ADC, ENABLE);
  RCC_APBPeriphResetCmd(RCC_APBPeriph_ADC, DISABLE);
}

/**
 * @brief  Initializes the ADC peripheral according to the specified 
 *         parameters in the ADC_InitStruct.
 * @param  ADC_InitStruct: pointer to a ADC_InitTypeDef structure
 *         that contains the configuration information for the specified 
 *         ADC peripheral.
 * @return None
 */
void ADC_Init(ADC_InitTypeDef* ADC_InitStruct)
{
  uint32_t tmp = ADC->CR;

  tmp &= ~(ADC_CR_SMP_Msk | ADC_CR_PRERATIO_Msk | ADC_CR_TRGSRC | ADC_CR_CMPSEL | ADC_CR_CMPPOL);

  tmp |= (ADC_InitStruct->ADC_CMPIndex | 
          ADC_InitStruct->ADC_CMPPolarity |
          ADC_InitStruct->ADC_TriggerSource |
          ADC_InitStruct->ADC_PrescalerDivision |
          ADC_InitStruct->ADC_SampleCycles |
          ADC_CR_PREDIVEN |
          ADC_CR_PRESRCEN);
  
  ADC->CR = tmp;
}

/**
 * @brief  Fills each ADC_InitStruct member with its default value.
 * @param  ADC_InitStruct: pointer to a ADC_InitTypeDef structure
 *         which will be initialized.
 * @return None
 */
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct)
{
  ADC_InitStruct->ADC_CMPIndex = ADC_CMP_CMP0;
  ADC_InitStruct->ADC_CMPPolarity = ADC_CMPPolarity_Positive;
  ADC_InitStruct->ADC_TriggerSource = ADC_TriggerSource_Software;
  ADC_InitStruct->ADC_PrescalerDivision = ADC_PrescalerDivision_2;
  ADC_InitStruct->ADC_SampleCycles = ADC_SampleCycles_64;
}

/**
 * @brief  Enables or disables the ADC peripheral.
 * @param  NewState: new state of the ADCx peripheral.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void ADC_Cmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    ADC->CR |= ADC_CR_ADEN;
  }
  else {
    ADC->CR &= ~ADC_CR_ADEN;
  }
}

/**
 * @brief  Read the result of the ADC conversion
 * @param  None
 * @return The read data.
 */
uint32_t ADC_ReadData(void)
{
  return ADC->DR;
}

/**
 * @brief Start an ADC conversion
 * @param  None
 */
void ADC_StartConversion(void)
{
  ADC->CR |= ADC_CR_SWSTART;
}

/**
 * @brief  Enables or disables the ADC interrupt.
 * @param  ADC_IT: specifies the ADC interrupt sources to be enabled or disabled.
 *         This parameter can be any combination of the following values:
 *         @arg @ref ADC_IT_EOC: ADC end of conversion interrupt.
 *         @arg @ref ADC_IT_OVF: ADC data overflow interrupt.
 * @param  NewState: new state of the ADC interrupt.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None.
 */
void ADC_ITConfig(uint32_t ADC_IT, FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    ADC->IER |= ADC_IT;
  }
  else
  {
    ADC->IER &= ~ADC_IT;
  }
}

/**
 * @brief  Checks whether the specified ADC flag is set or not.
 * @param  ADC_FLAG: specifies the ADC flag to check.
 *         This parameter can be one of the following values:
 *         @arg @ref ADC_FLAG_EOC: ADC end of conversion flag.
 *         @arg @ref ADC_FLAG_OVF: ADC data overflow flag.
 *         @arg @ref ADC_FLAG_BUSY: ADC busy flag.
 * @return The new state of ADC_FLAG (SET or RESET).
 */
FlagStatus ADC_GetFlagStatus(uint32_t ADC_FLAG)
{
  FlagStatus bitstatus = RESET;

  /* Check the status of the specified ADC flag */
  if (ADC->ISR & ADC_FLAG)
  {
    /* ADC_FLAG is set */
    bitstatus = SET;
  }
  else
  {
    /* ADC_FLAG is reset */
    bitstatus = RESET;
  }
  /* Return the ADC_FLAG status */
  return bitstatus;
}

/**
 * @brief  Clears the ADC IT pending flags.
 * @param  ADC_FLAG: specifies the flag to clear.
 *         This parameter can be any combination of the following values:
 *         @arg @ref ADC_IT_EOC: adc end of conversion interrupt.
 *         @arg @ref ADC_IT_OVF: ADC data overflow interrupt.
 * @return None.
 */
void ADC_ClearITFlag(uint32_t ADC_IT)
{
  ADC->ICR |= ~ADC_IT;
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
