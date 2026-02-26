/**
 * @file    wb7720_adc.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the ADC firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_ADC_H
#define __WB7720_ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup ADC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  ADC Init structure definition  
  */
typedef struct
{
  uint32_t ADC_SampleCycles;             /*!< Specifies the number of ADC sampling cycles.
                                              This parameter can be a value of @ref ADC_SampleCycles. */

  uint32_t ADC_PrescalerDivision;        /*!< Specifies the frequency division factor for the ADC.
                                              This parameter can be a value of @ref ADC_PrescalerDivision */

  uint32_t ADC_TriggerSource;            /*!< Specifies the trigger for the ADC conversion
                                              This parameter can be a value of @ref ADC_TriggerSource */

  uint32_t ADC_CMPIndex;                 /*!< Specifies the CMP that the ADC works with.
                                              This parameter can be a value of @ref ADC_CMP_Index */

  uint32_t ADC_CMPPolarity;              /*!< Configure the polarity of the ADC.
                                              This parameter can be a value of @ref ADC_CMPPolarity */
} ADC_InitTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup ADC_Exported_Constants
  * @{
  */

/** @defgroup ADC_SampleCycles 
  * @{
  */
#define ADC_SampleCycles_32          ((uint32_t)ADC_CR_SMP_32)
#define ADC_SampleCycles_64          ((uint32_t)ADC_CR_SMP_64)
#define ADC_SampleCycles_128         ((uint32_t)ADC_CR_SMP_128)
#define ADC_SampleCycles_256         ((uint32_t)ADC_CR_SMP_256)
/**
  * @}
  */


/** @defgroup ADC_PrescalerDivision 
  * @{
  */
#define ADC_PrescalerDivision_2      ((uint32_t)ADC_CR_PRERATIO_2 )
#define ADC_PrescalerDivision_3      ((uint32_t)ADC_CR_PRERATIO_3 )
#define ADC_PrescalerDivision_4      ((uint32_t)ADC_CR_PRERATIO_4 )
#define ADC_PrescalerDivision_5      ((uint32_t)ADC_CR_PRERATIO_5 )
#define ADC_PrescalerDivision_6      ((uint32_t)ADC_CR_PRERATIO_6 )
#define ADC_PrescalerDivision_7      ((uint32_t)ADC_CR_PRERATIO_7 )
#define ADC_PrescalerDivision_8      ((uint32_t)ADC_CR_PRERATIO_8 )
#define ADC_PrescalerDivision_9      ((uint32_t)ADC_CR_PRERATIO_9 )
#define ADC_PrescalerDivision_10     ((uint32_t)ADC_CR_PRERATIO_10)
#define ADC_PrescalerDivision_11     ((uint32_t)ADC_CR_PRERATIO_11)
#define ADC_PrescalerDivision_12     ((uint32_t)ADC_CR_PRERATIO_12)
#define ADC_PrescalerDivision_13     ((uint32_t)ADC_CR_PRERATIO_13)
#define ADC_PrescalerDivision_14     ((uint32_t)ADC_CR_PRERATIO_14)
#define ADC_PrescalerDivision_15     ((uint32_t)ADC_CR_PRERATIO_15)
#define ADC_PrescalerDivision_16     ((uint32_t)ADC_CR_PRERATIO_16)
#define ADC_PrescalerDivision_17     ((uint32_t)ADC_CR_PRERATIO_17)
#define ADC_PrescalerDivision_18     ((uint32_t)ADC_CR_PRERATIO_18)
#define ADC_PrescalerDivision_19     ((uint32_t)ADC_CR_PRERATIO_19)
#define ADC_PrescalerDivision_20     ((uint32_t)ADC_CR_PRERATIO_20)
#define ADC_PrescalerDivision_21     ((uint32_t)ADC_CR_PRERATIO_21)
#define ADC_PrescalerDivision_22     ((uint32_t)ADC_CR_PRERATIO_22)
#define ADC_PrescalerDivision_23     ((uint32_t)ADC_CR_PRERATIO_23)
#define ADC_PrescalerDivision_24     ((uint32_t)ADC_CR_PRERATIO_24)
#define ADC_PrescalerDivision_25     ((uint32_t)ADC_CR_PRERATIO_25)
#define ADC_PrescalerDivision_26     ((uint32_t)ADC_CR_PRERATIO_26)
#define ADC_PrescalerDivision_27     ((uint32_t)ADC_CR_PRERATIO_27)
#define ADC_PrescalerDivision_28     ((uint32_t)ADC_CR_PRERATIO_28)
#define ADC_PrescalerDivision_29     ((uint32_t)ADC_CR_PRERATIO_29)
#define ADC_PrescalerDivision_30     ((uint32_t)ADC_CR_PRERATIO_30)
#define ADC_PrescalerDivision_31     ((uint32_t)ADC_CR_PRERATIO_31)
#define ADC_PrescalerDivision_32     ((uint32_t)ADC_CR_PRERATIO_32)
#define ADC_PrescalerDivision_33     ((uint32_t)ADC_CR_PRERATIO_33)
#define ADC_PrescalerDivision_34     ((uint32_t)ADC_CR_PRERATIO_34)
#define ADC_PrescalerDivision_35     ((uint32_t)ADC_CR_PRERATIO_35)
#define ADC_PrescalerDivision_36     ((uint32_t)ADC_CR_PRERATIO_36)
#define ADC_PrescalerDivision_37     ((uint32_t)ADC_CR_PRERATIO_37)
#define ADC_PrescalerDivision_38     ((uint32_t)ADC_CR_PRERATIO_38)
#define ADC_PrescalerDivision_39     ((uint32_t)ADC_CR_PRERATIO_39)
#define ADC_PrescalerDivision_40     ((uint32_t)ADC_CR_PRERATIO_40)
#define ADC_PrescalerDivision_41     ((uint32_t)ADC_CR_PRERATIO_41)
#define ADC_PrescalerDivision_42     ((uint32_t)ADC_CR_PRERATIO_42)
#define ADC_PrescalerDivision_43     ((uint32_t)ADC_CR_PRERATIO_43)
#define ADC_PrescalerDivision_44     ((uint32_t)ADC_CR_PRERATIO_44)
#define ADC_PrescalerDivision_45     ((uint32_t)ADC_CR_PRERATIO_45)
#define ADC_PrescalerDivision_46     ((uint32_t)ADC_CR_PRERATIO_46)
#define ADC_PrescalerDivision_47     ((uint32_t)ADC_CR_PRERATIO_47)
#define ADC_PrescalerDivision_48     ((uint32_t)ADC_CR_PRERATIO_48)
#define ADC_PrescalerDivision_49     ((uint32_t)ADC_CR_PRERATIO_49)
#define ADC_PrescalerDivision_50     ((uint32_t)ADC_CR_PRERATIO_50)
#define ADC_PrescalerDivision_51     ((uint32_t)ADC_CR_PRERATIO_51)
#define ADC_PrescalerDivision_52     ((uint32_t)ADC_CR_PRERATIO_52)
#define ADC_PrescalerDivision_53     ((uint32_t)ADC_CR_PRERATIO_53)
#define ADC_PrescalerDivision_54     ((uint32_t)ADC_CR_PRERATIO_54)
#define ADC_PrescalerDivision_55     ((uint32_t)ADC_CR_PRERATIO_55)
#define ADC_PrescalerDivision_56     ((uint32_t)ADC_CR_PRERATIO_56)
#define ADC_PrescalerDivision_57     ((uint32_t)ADC_CR_PRERATIO_57)
#define ADC_PrescalerDivision_58     ((uint32_t)ADC_CR_PRERATIO_58)
#define ADC_PrescalerDivision_59     ((uint32_t)ADC_CR_PRERATIO_59)
#define ADC_PrescalerDivision_60     ((uint32_t)ADC_CR_PRERATIO_60)
#define ADC_PrescalerDivision_61     ((uint32_t)ADC_CR_PRERATIO_61)
#define ADC_PrescalerDivision_62     ((uint32_t)ADC_CR_PRERATIO_62)
#define ADC_PrescalerDivision_63     ((uint32_t)ADC_CR_PRERATIO_63)
#define ADC_PrescalerDivision_64     ((uint32_t)ADC_CR_PRERATIO_64)
/**
  * @}
  */


/** @defgroup ADC_TriggerSource 
  * @{
  */
#define ADC_TriggerSource_Software   ((uint32_t)ADC_CR_TRGSRC)
#define ADC_TriggerSource_External   ((uint32_t)0x0)
/**
  * @}
  */


/** @defgroup ADC_CMPPolarity 
  * @{
  */
#define ADC_CMPPolarity_Positive     ((uint32_t)ADC_CR_CMPPOL)
#define ADC_CMPPolarity_Negative     ((uint32_t)0x0)
/**
  * @}
  */


/** @defgroup ADC_Flag
  * @{
  */
#define ADC_FLAG_EOC                 ((uint32_t)ADC_ISR_EOC)
#define ADC_FLAG_OVF                 ((uint32_t)ADC_ISR_OVF)
#define ADC_FLAG_BUSY                ((uint32_t)ADC_ISR_BUSY)
/**
  * @}
  */


/** @defgroup ADC_IT
  * @{
  */
#define ADC_IT_EOC                ((uint32_t)ADC_IER_EOCIE)
#define ADC_IT_OVF                ((uint32_t)ADC_IER_OVFIE)
/**
  * @}
  */


/** @defgroup ADC_CMP_Index
  * @{
  */
#define ADC_CMP_CMP0                ((uint32_t)0x0)
#define ADC_CMP_CMP1                ((uint32_t)ADC_CR_CMPSEL)
/**
  * @}
  */


/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void ADC_DeInit(void);
void ADC_Init(ADC_InitTypeDef* ADC_InitStruct);
void ADC_StructInit(ADC_InitTypeDef* ADC_InitStruct);
void ADC_Cmd(FunctionalState NewState);
uint32_t ADC_ReadData(void);
void ADC_StartConversion(void);
void ADC_ITConfig(uint32_t ADC_IT, FunctionalState NewState);
FlagStatus ADC_GetFlagStatus(uint32_t ADC_FLAG);
void ADC_ClearITFlag(uint32_t ADC_IT);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_ADC_H */
