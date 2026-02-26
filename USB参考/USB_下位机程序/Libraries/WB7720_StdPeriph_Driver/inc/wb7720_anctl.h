/**
 * @file    wb7720_anctl.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the ANCTL firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_ANCTL_H
#define __WB7720_ANCTL_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup ANCTL
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup ANCTL_Exported_Constants 
  * @{
  */

/** @defgroup BOD_detection_level 
  * @{
  */
#define ANCTL_BODLevel_0                  ANCTL_BODCON_BLS_LEV0
#define ANCTL_BODLevel_1                  ANCTL_BODCON_BLS_LEV1
#define ANCTL_BODLevel_2                  ANCTL_BODCON_BLS_LEV2
#define ANCTL_BODLevel_3                  ANCTL_BODCON_BLS_LEV3
#define ANCTL_BODLevel_4                  ANCTL_BODCON_BLS_LEV4
#define ANCTL_BODLevel_5                  ANCTL_BODCON_BLS_LEV5
#define ANCTL_BODLevel_6                  ANCTL_BODCON_BLS_LEV6
#define ANCTL_BODLevel_7                  ANCTL_BODCON_BLS_LEV7
/**
  * @}
  */


/** @defgroup CMP_OP_GAIN
  * @{
  */
#define CMP_OP_GAIN_1                     ((uint32_t)(0x0 << 9))      
#define CMP_OP_GAIN_2                     ((uint32_t)(0x01 << 9))      
#define CMP_OP_GAIN_4                     ((uint32_t)(0x02 << 9))      
#define CMP_OP_GAIN_8                     ((uint32_t)(0x03 << 9))      
#define CMP_OP_GAIN_10                    ((uint32_t)(0x04 << 9))      
#define CMP_OP_GAIN_16                    ((uint32_t)(0x05 << 9))      
#define CMP_OP_GAIN_32                    ((uint32_t)(0x06 << 9))      
#define CMP_OP_GAIN_64                    ((uint32_t)(0x07 << 9))      
/**
  * @}
  */

/** @defgroup CMP_HYS
  * @{
  */
#define CMP_Hysteresis_Enable             ((uint32_t)(0x1 << 8))      
#define CMP_Hysteresis_Disable            ((uint32_t)(0x0 << 8))        
/**
  * @}
  */

/** @defgroup CMP_CHOP_POL
  * @{
  */
#define CMP_Input_Exchange                ((uint32_t)(0x1 << 7))      
#define CMP_Input_NoExchange              ((uint32_t)(0x0 << 7))        
/**
  * @}
  */

/** @defgroup CMP_CHOP_EN
  * @{
  */
#define CMP_CHOP_Enable                   ((uint32_t)(0x1 << 6))      
#define CMP_CHOP_Disable                  ((uint32_t)(0x0 << 6))        
/**
  * @}
  */

/** @defgroup CMP_BYPASS
  * @{
  */
#define CMP_Bypass_Enable                 ((uint32_t)(0x1 << 5))      
#define CMP_Bypass_Disable                ((uint32_t)(0x0 << 5))        
/**
  * @}
  */

 /** @defgroup CMP_OP_EXT
  * @{
  */
#define CMP_OP_External                   ((uint32_t)(0x1 << 4))      
#define CMP_OP_Internal                   ((uint32_t)(0x0 << 4))        
/**
  * @}
  */

/** @defgroup CMP_OP_OE
  * @{
  */
#define CMP_OP_Output_Enable              ((uint32_t)(0x1 << 3))      
#define CMP_OP_Output_Disable             ((uint32_t)(0x0 << 3))        
/**
  * @}
  */

/** @defgroup CMP_OP_EN
  * @{
  */
#define CMP_Mode_OP                       ((uint32_t)(0x1 << 2))      
#define CMP_Mode_Comparator               ((uint32_t)(0x0 << 2))        
/**
  * @}
  */

/** @defgroup CMP_OE
  * @{
  */
#define CMP_Compare_Output_Enable         ((uint32_t)(0x1 << 1))      
#define CMP_Compare_Output_Disable        ((uint32_t)(0x0 << 1))        
/**
  * @}
  */

/** @defgroup CMP_EN
  * @{
  */
#define CMP_Enable                        ((uint32_t)(0x1 << 0))      
#define CMP_Disable                       ((uint32_t)(0x0 << 1))        
/**
  * @}
  */

/** @defgroup CMP0_PSEL_NSEL 
  * @{
  */
#define CMP0_PSEL_NONE      0x0000
#define CMP0_PSEL_PD5       0x0001
#define CMP0_PSEL_PD4       0x0002
#define CMP0_PSEL_DAC       0x0010

#define CMP0_NSEL_NONE      0x0000
#define CMP0_NSEL_PA1       0x0020
#define CMP0_NSEL_PA0       0x0040
#define CMP0_NSEL_DAC       0x0200
/**
  * @}
  */


/** @defgroup CMP1_PSEL_NSEL 
  * @{
  */
#define CMP1_PSEL_NONE      0x0000
#define CMP1_PSEL_PD4       0x0001
#define CMP1_PSEL_PD5       0x0002
#define CMP1_PSEL_DAC       0x0010

#define CMP1_NSEL_NONE      0x0000
#define CMP1_NSEL_PA0       0x0020
#define CMP1_NSEL_PA1       0x0040
#define CMP1_NSEL_DAC       0x0200
/**
  * @}
  */

/** @defgroup DAC_VCAS 
  * @{
  */
#define DAC_CURRENT_CALL_BIAS_WI          ((uint32_t)(0x0 << 3))
#define DAC_CURRENT_CALL_BIAS_WO          ((uint32_t)(0x1 << 3))
#define DAC_CURRENT_CALL_BIAS_Normal      ((uint32_t)(0x2 << 3))
/**
  * @}
  */

/** @defgroup DAC_IOUT 
  * @{
  */
#define DAC_CURRENT_MODE_OUT_Disable      ((uint32_t)(0x0 << 2))
#define DAC_CURRENT_MODE_OUT_Enable       ((uint32_t)(0x1 << 2))
/**
  * @}
  */

/** @defgroup DAC_BUFEN 
  * @{
  */
#define DAC_BUFFER_OUT_Disable            ((uint32_t)(0x0 << 1))
#define DAC_BUFFER_OUT_Enable             ((uint32_t)(0x1 << 1))
/**
  * @}
  */

/** @defgroup DAC_EN 
  * @{
  */
#define DAC_Disable                       ((uint32_t)(0x0 << 0))
#define DAC_Enable                        ((uint32_t)(0x1 << 0))
/**
  * @}
  */

/** @defgroup DAC_VOLTAGE_RANGE 
  * @{
  */
#define DAC_Voltage_2V5                   ((uint32_t)(0x0 << 4))
#define DAC_Voltage_3V0                   ((uint32_t)(0x1 << 4))
#define DAC_Voltage_3V5                   ((uint32_t)(0x2 << 4))
#define DAC_Voltage_4V0                   ((uint32_t)(0x3 << 4))
#define DAC_Voltage_4V5                   ((uint32_t)(0x4 << 4))
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void ANCTL_HSI2Cmd(FunctionalState NewState);
void ANCTL_HSI48Cmd(FunctionalState NewState);
FlagStatus ANCTL_GetHSI48ReadyStatus(void);
void ANCTL_AdjustHSI48CalibrationValue(uint16_t HSI48CalibrationValue);
uint16_t ANCTL_GetHSI48CalibrationValue(void);
void ANCTL_LSICmd(FunctionalState NewState);
FlagStatus ANCTL_GetLSIReadyStatus(void);
void ANCTL_AdjustLSICalibrationValue(uint8_t LSICalibrationValue);
uint8_t ANCTL_GetLSICalibrationValue(void);
void ANCTL_BODLevelConfig(uint32_t ANCTL_BODLevel);
void ANCTL_BODCmd(FunctionalState NewState);
FlagStatus ANCTL_GetBODStatus(void);
uint8_t ANCTL_CMP0GetValue(void);
uint8_t ANCTL_CMP1GetValue(void);
void ANCTL_CMP0Config(uint32_t CMP_Config);
void ANCTL_CMP1Config(uint32_t CMP_Config);
void ANCTL_CMP0InputConfig(uint32_t CMP0_PSEL, uint32_t CMP0_NSEL);
void ANCTL_CMP1InputConfig(uint32_t CMP1_PSEL, uint32_t CMP1_NSEL);

void ANCTL_DACSetValue(uint32_t DAC_Value);
void ANCTL_DACCmd(FunctionalState NewState);
void ANCTL_DACConfig(uint32_t DAC_Voltage, uint32_t DAC_Config);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_ANCTL_H */
