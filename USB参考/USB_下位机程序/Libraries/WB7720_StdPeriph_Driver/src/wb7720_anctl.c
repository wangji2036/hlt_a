/**
 * @file    wb7720_anctl.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the ANCTL firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_anctl.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup ANCTL
  * @brief ANCTL driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DAC_Config_Msk      ((uint32_t)(0x0F << 1))
#define DAC_Trim_Msk        ((uint32_t)(0xFF << 0))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup ANCTL_Private_Functions
  * @{
  */

/**
 * @brief  Enables or disables the Internal 1.6MHz oscillator (HSI2).
 * @param  NewState: new state of the HSI2.
 *         This parameter can be: ENABLE or DISABLE.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_HSI2Cmd(FunctionalState NewState)
{
  ANCTL->HSI2ENR = (uint32_t)NewState;
}

/**
 * @brief  Enables or disables the Internal 48MHz oscillator (HSI48).
 * @param  NewState: new state of the HSI48.
 *         This parameter can be: ENABLE or DISABLE.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_HSI48Cmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    ANCTL->HSI48CR |= 0x01;
    ANCTL->HSI48ENR = 0x01;
    while ((ANCTL->HSI48SR & 0x03) != 0x01);    // Wait HSI48 ready
    ANCTL->HSI48CR &= ~0x01;
    __NOP();  __NOP();
    ANCTL->HSI48CR |= 0x01;
  }
  else {
    ANCTL->HSI48CR &= ~0x01;
    ANCTL->HSI48ENR = 0x00;
    while ((ANCTL->HSI48SR & 0x03) != 0x02);    // Wait HSI48 disabled
  }
}

/**
 * @brief  Gets the HSI48 Ready status.
 * @param  None
 * @return The new state of the HSI48 Ready status (SET or RESET).
 */
FlagStatus ANCTL_GetHSI48ReadyStatus(void)
{
  FlagStatus readystatus = RESET;

  if ((ANCTL->HSI48SR & 0x03) == 0x01)
  {
    readystatus = SET;
  }
  else
  {
    readystatus = RESET;
  }

  return readystatus;
}

/**
 * @brief  Adjusts the Internal 48MHz oscillator (HSI48) calibration value.
 * @param  HSI48CalibrationValue: specifies the HSI48 calibration trimming value.
 *         This parameter must be a number between 0 and 0x3FF.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_AdjustHSI48CalibrationValue(uint16_t HSI48CalibrationValue)
{
  ANCTL->HSI48TRIM = HSI48CalibrationValue;
}

/**
 * @brief  Returns the HSI48 Calibration value.
 * @param  None
 * @return The calibration value.
 */
uint16_t ANCTL_GetHSI48CalibrationValue(void)
{
  return ((uint16_t)ANCTL->HSI48TRIM);
}


/**
 * @brief  Enables or disables the Internal Low Speed oscillator (LSI).
 * @param  NewState: new state of the LSI.
 *         This parameter can be: ENABLE or DISABLE.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_LSICmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    ANCTL->LSICR |= ANCTL_LSICR_LSION;
  }
  else {
    ANCTL->LSICR &= ~ANCTL_LSICR_LSION;
  }
}

/**
 * @brief  Gets the LSI Ready status.
 * @param  None
 * @return The new state of the LSI Ready status (SET or RESET).
 */
FlagStatus ANCTL_GetLSIReadyStatus(void)
{
  FlagStatus readystatus = RESET;

  if ((ANCTL->LSISR & 0x01) == 0x01)
  {
    readystatus = SET;
  }
  else
  {
    readystatus = RESET;
  }

  return readystatus;
}

/**
 * @brief  Adjusts the Internal Low Speed oscillator (LSI) calibration value.
 * @param  LSICalibrationValue: specifies the LSI calibration trimming value.
 *         This parameter must be a number between 0 and 0x7F.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_AdjustLSICalibrationValue(uint8_t LSICalibrationValue)
{
  ANCTL->LSITRIM = LSICalibrationValue;
}

/**
 * @brief  Returns the LSI Calibration value.
 * @param  None
 * @return The calibration value.
 */
uint8_t ANCTL_GetLSICalibrationValue(void)
{
  return ((uint8_t)ANCTL->LSITRIM);
}

/**
 * @brief  Configures the voltage threshold detected by the Brown-Out Detector (BOD).
 * @param  ANCTL_BODLevel: specifies the BOD detection level.
 *    This parameter can be one of the following values:
 *      @arg ANCTL_BODLevel_0
 *      @arg ANCTL_BODLevel_1
 *      @arg ANCTL_BODLevel_2
 *      @arg ANCTL_BODLevel_3
 *      @arg ANCTL_BODLevel_4
 *      @arg ANCTL_BODLevel_5
 *      @arg ANCTL_BODLevel_6
 *      @arg ANCTL_BODLevel_7
 * @note   Refer to the electrical characteristics of your device datasheet for
 *         more details about the voltage threshold corresponding to each 
 *         detection level.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_BODLevelConfig(uint32_t ANCTL_BODLevel)
{
  ANCTL->BODCON = ((ANCTL->BODCON & ~ANCTL_BODCON_BLS_Msk) | ANCTL_BODLevel);
}

/**
 * @brief  Enables or disables the Brown-Out Detector (BOD).
 * @param  NewState: new state of the BOD.
 *         This parameter can be: ENABLE or DISABLE.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_BODCmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    ANCTL->BODCON |= ANCTL_BODCON_BODEN;
  }
  else {
    ANCTL->BODCON &= ~ANCTL_BODCON_BODEN;
  }
}

/**
 * @brief  Gets the BOD status.
 * @param  None
 * @return The new state of the BOD (SET or RESET).
 */
FlagStatus ANCTL_GetBODStatus(void)
{
  FlagStatus bodstatus = RESET;

  if ((ANCTL->BODSR & 0x01) == 0x01)
  {
    bodstatus = SET;
  }
  else
  {
    bodstatus = RESET;
  }

  return bodstatus;
}

/**
 * @brief  Configures the CMP0.
 * @param  CMP_Config: Specifies the configuration parameters for CMP.
 * @return None
 */
void ANCTL_CMP0Config(uint32_t CMP_Config)
{
  ANCTL->CMP0CON = CMP_Config;
}

/**
 * @brief  Configures the CMP1.
 * @param  CMP1_Config: Specifies the configuration parameters for CMP.
 * @return None
 */
void ANCTL_CMP1Config(uint32_t CMP_Config)
{
  ANCTL->CMP1CON = CMP_Config;
}

/**
 * @brief  Get comparison results for CMP0.
 * @param  None
 * @return None
 */
uint8_t ANCTL_CMP0GetValue(void)
{
  return ANCTL->CMP0OUT;
}

/**
 * @brief  Get comparison results for CMP.
 * @param  None
 * @return None
 */
uint8_t ANCTL_CMP1GetValue(void)
{
  return ANCTL->CMP1OUT;
}

/**
 * @brief  Configures the Comparator 0 input.
 * @param  CMP0_PSEL: specifies the comparator positive input.
 * @param  CMP0_NSEL: specifies the comparator negative input.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_CMP0InputConfig(uint32_t CMP0_PSEL, uint32_t CMP0_NSEL)
{
  uint32_t tmpreg;
  tmpreg = (CMP0_PSEL & 0x0013) | (CMP0_NSEL & 0x0260);
  ANCTL->CMP0SEL = tmpreg;
}

/**
 * @brief  Configures the Comparator 1 input.
 * @param  CMP1_PSEL: specifies the comparator positive input.
 * @param  CMP1_NSEL: specifies the comparator negative input.
 * @note   The ANCTL register write-protection function should be disabled before using this function.
 * @return None
 */
void ANCTL_CMP1InputConfig(uint32_t CMP1_PSEL, uint32_t CMP1_NSEL)
{
  uint32_t tmpreg;
  tmpreg = (CMP1_PSEL & 0x0013) | (CMP1_NSEL & 0x0260);
  ANCTL->CMP1SEL = tmpreg;
}

/**
 * @brief  Configures the DAC.
 * @param  DAC_Voltage: Specifies the maximum voltage output by the DAC.
 *         This parameter can be one of the following values:
 *           @arg DAC_Voltage_2V5: The maximum output voltage of the DAC is 2.5V.
 *           @arg DAC_Voltage_3V0: The maximum output voltage of the DAC is 3.0V.
 *           @arg DAC_Voltage_3V5: The maximum output voltage of the DAC is 3.5V.
 *           @arg DAC_Voltage_4V0: The maximum output voltage of the DAC is 4.0V.
 *           @arg DAC_Voltage_4V5: The maximum output voltage of the DAC is 4.5V.
 * @param  DAC_Config: Specifies the configuration parameters for DAC.
 * @return None
 */
void ANCTL_DACConfig(uint32_t DAC_Voltage, uint32_t DAC_Config)
{
  uint32_t tmp = 0;

  tmp = (ANCTL->DACCON & ~DAC_Config_Msk);
  tmp |= DAC_Config;
  ANCTL->DACCON = tmp;

  tmp = (ANCTL->DACTRIM & ~DAC_Trim_Msk);
  tmp |= DAC_Voltage;
  tmp |= (0x05 << 0);
  ANCTL->DACTRIM = tmp;
}

/**
 * @brief  Sets the value of the DAC output.
 * @param  DAC_Value: DAC output value.
 * @return None
 */
void ANCTL_DACSetValue(uint32_t DAC_Value)
{
  ANCTL->DACDAT = DAC_Value;
}

/**
 * @brief  Enables or disables the DAC.
 * @param  NewState: new state of the DAC.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void ANCTL_DACCmd(FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    ANCTL->DACCON |= ANCTL_DACCON_DACEN;
  }
  else
  {
    ANCTL->DACCON &= ~ANCTL_DACCON_DACEN;
  } 
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
