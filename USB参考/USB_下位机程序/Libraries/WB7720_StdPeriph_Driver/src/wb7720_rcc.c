/**
 * @file    wb7720_rcc.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the RCC firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup RCC
  * @brief RCC driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup RCC_Private_Functions
  * @{
  */

/**
 * @brief  Configures the system clock source.
 * @param  RCC_SYSCLKSource: specifies the system clock source.
 *    This parameter can be one of the following values:
 *      @arg RCC_SYSCLKSource_HSI2: HSI2 selected as system clock
 *      @arg RCC_SYSCLKSource_HSI48_DIV6: HSI48/6 selected as system clock
 *      @arg RCC_SYSCLKSource_HSI48: HSI48 selected as system clock
 *      @arg RCC_SYSCLKSource_HSE: HSE selected as system clock
 *      @arg RCC_SYSCLKSource_PLL: PLL selected as system clock
 * @return None
 */
void RCC_SYSCLKSourceConfig(uint32_t RCC_SYSCLKSource)
{
  RCC->SYSCLKSRC = RCC_SYSCLKSource;
  RCC->SYSCLKUEN = RCC_SYSCLKUEN_ENA;
  while (RCC->SYSCLKUEN != 0);
}

/**
 * @brief  Configures the system clock prescaler.
 * @param  RCC_SYSCLKPrescaler: defines the system clock prescaler.
 *    This parameter can be RCC_SYSCLKPrescaler_Divx where x:[1, 64]
 * @return None
 */
void RCC_SYSCLKPrescalerConfig(uint32_t RCC_SYSCLKPrescaler)
{
  RCC->SYSCLKPRE1 = RCC_SYSCLKPrescaler;
}

/**
 * @brief  Configures the USB clock source (USBCLK).
 * @param  RCC_USBCLKSource: specifies the USB clock source.
 *    This parameter can be one of the following values:
 *      @arg RCC_USBCLKSource_None: no clock selected as USB clock source
 *      @arg RCC_USBCLKSource_HSI48: HSI48 selected as USB clock source
 *      @arg RCC_USBCLKSource_PLL2USB: PLL to USB clock selected as USB clock source
 * @return None
 */
void RCC_USBCLKConfig(uint32_t RCC_USBCLKSource)
{
  RCC->USBCLKSRC = RCC_USBCLKSource;
}

/**
 * @brief  Configures the clock to output on MCO pin.
 * @note   Before configuring MCO, the LSI oscillator must be enabled and is ready.
 * @param  RCC_MCOSource: specifies the clock source to output.
 *    This parameter can be one of the following values:
 *      @arg RCC_MCOSource_LSI: LSI oscillator clock selected
 *      @arg RCC_MCOSource_PLL: PLL clock selected
 *      @arg RCC_MCOSource_HSI48: HSI48 oscillator clock selected
 *      @arg RCC_MCOSource_SYSCLKSRC: The source of system clock selected
 * @param  RCC_MCOPrescaler: defines the MCO clock prescaler.
 *    This parameter can be RCC_MCOPrescaler_Divx where x:[1, 64]
 * @param  NewState: new state of the MCO clock.
 *    This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void RCC_MCOConfig(uint32_t RCC_MCOSource, uint32_t RCC_MCOPrescaler, FunctionalState NewState)
{
  volatile uint32_t tmp;

  if (NewState != DISABLE) {
    RCC->MCOSEL = RCC_MCOSource;
    RCC->MCOUEN = RCC_MCOUEN_ENA;
    tmp = 2000;
    while (tmp--);
    RCC->MCOENR = RCC_MCOENR_OUTEN;
    RCC->MCOPRE = RCC_MCOPrescaler;
    RCC->MCOUEN = RCC_MCOUEN_ENA;
  }
  else {
    RCC->MCOENR = 0x00;
    RCC->MCOPRE = 0x00;
  }
}

/**
 * @brief  Configures the FCU clock source (FCUCLK).
 * @param  RCC_FCUCLKSource: specifies the FCU clock source.
 *    This parameter can be one of the following values:
 *      @arg RCC_FCUCLKSource_None: no clock selected as FCU clock source
 *      @arg RCC_FCUCLKSource_HSI48: HSI48 selected as FCU clock source
 *      @arg RCC_FCUCLKSource_PLL: PLL clock selected as FCU clock source
 * @param  RCC_FCUCLKPrescaler: defines the FCU clock prescaler.
 *    This parameter can be RCC_FCUCLKPrescaler_Divx where x:[1, 64]
 * @return None
 */
void RCC_FCUCLKConfig(uint32_t RCC_FCUCLKSource, uint32_t RCC_FCUCLKPrescaler)
{
  RCC->FCUCLKSRC = RCC_FCUCLKSRC_NONE;
  if (RCC_FCUCLKSource != RCC_FCUCLKSource_None)
  {
    RCC->FCUCLKPRE2 = RCC_FCUCLKPRE2_SRCEN;
    RCC->FCUCLKPRE1 = RCC_FCUCLKPrescaler;
    RCC->FCUCLKSRC = RCC_FCUCLKSource;
  }
}

/**
 * @brief  Returns the frequencies of different on chip clocks.
 * @param  RCC_Clocks: pointer to a RCC_ClocksTypeDef structure which will hold
 *         the clocks frequencies.
 * @return None
 */
void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks)
{
  uint32_t sysclkfreq;
  uint32_t sysclkprediv, sysclksrcfreq;

  switch (RCC->SYSCLKSRC)
  {
  case 0x00:    /* HSI2 used as system clock */
    sysclksrcfreq = HSI2_VALUE;
    break;
  case 0x01:    /* HSI48/6 used as system clock */
    sysclksrcfreq = (HSI48_VALUE / 6);
    break;
  case 0x02:    /* HSI48 used as system clock */
    sysclksrcfreq = HSI48_VALUE;
    break;
  default:
    sysclksrcfreq = HSI2_VALUE;
    break;
  }

  sysclkprediv = (((RCC->SYSCLKPRE1 & (RCC_SYSCLKPRE1_RATIO_Msk | RCC_SYSCLKPRE1_DIVEN)) + 1) >> 1) + 1;
  sysclkfreq = sysclksrcfreq / sysclkprediv;

  RCC_Clocks->SYSCLK_Frequency = sysclkfreq;
  RCC_Clocks->APBCLK_Frequency = sysclkfreq;
}

/**
 * @brief  Enables or disables the APB peripheral clock.
 * @param  RCC_APBPeriph: specifies the APB peripheral to gates its clock.
 *    This parameter can be any combination of the following values:
 *      @arg RCC_APBPeriph_CRC
 *      @arg RCC_APBPeriph_CRS
 *      @arg RCC_APBPeriph_UART0
 *      @arg RCC_APBPeriph_UART1
 *      @arg RCC_APBPeriph_ARGB03
 *      @arg RCC_APBPeriph_ARGB45
 *      @arg RCC_APBPeriph_IWDG
 *      @arg RCC_APBPeriph_RTC
 *      @arg RCC_APBPeriph_I2C
 *      @arg RCC_APBPeriph_GPIO
 *      @arg RCC_APBPeriph_TIM6
 *      @arg RCC_APBPeriph_SPIM
 *      @arg RCC_APBPeriph_SPI1
 *      @arg RCC_APBPeriph_PCT0
 *      @arg RCC_APBPeriph_PCT1
 *      @arg RCC_APBPeriph_PCT2
 *      @arg RCC_APBPeriph_PCT3
 *      @arg RCC_APBPeriph_PCT4
 *      @arg RCC_APBPeriph_ADC
 *      @arg RCC_APBPeriph_EXTI
 *      @arg RCC_APBPeriph_USB
 * @param  NewState: new state of the specified peripheral clock.
 *    This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void RCC_APBPeriphClockCmd(uint32_t RCC_APBPeriph, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    RCC->APBENR |= RCC_APBPeriph;
  }
  else {
    RCC->APBENR &= ~RCC_APBPeriph;
  }
}

/**
 * @brief  Forces or releases APB peripheral reset.
 * @param  RCC_APBPeriph: specifies the APB peripheral to reset.
 *    This parameter can be any combination of the following values:
 *      @arg RCC_APBPeriph_CRC
 *      @arg RCC_APBPeriph_CRS
 *      @arg RCC_APBPeriph_UART0
 *      @arg RCC_APBPeriph_UART1
 *      @arg RCC_APBPeriph_ARGB03
 *      @arg RCC_APBPeriph_ARGB45
 *      @arg RCC_APBPeriph_IWDG
 *      @arg RCC_APBPeriph_RTC
 *      @arg RCC_APBPeriph_I2C
 *      @arg RCC_APBPeriph_GPIO
 *      @arg RCC_APBPeriph_TIM6
 *      @arg RCC_APBPeriph_SPIM
 *      @arg RCC_APBPeriph_SPI1
 *      @arg RCC_APBPeriph_PCT0
 *      @arg RCC_APBPeriph_PCT1
 *      @arg RCC_APBPeriph_PCT2
 *      @arg RCC_APBPeriph_PCT3
 *      @arg RCC_APBPeriph_PCT4
 *      @arg RCC_APBPeriph_ADC
 *      @arg RCC_APBPeriph_EXTI
 *      @arg RCC_APBPeriph_USB
 * @param  NewState: new state of the specified peripheral reset.
 *    This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void RCC_APBPeriphResetCmd(uint32_t RCC_APBPeriph, FunctionalState NewState)
{
  if (NewState != DISABLE) {
    RCC->APBRSTR |= RCC_APBPeriph;
  }
  else {
    RCC->APBRSTR &= ~RCC_APBPeriph;
  }
}

/**
 * @brief  Selects the SPI1 slave clock path.
 * @param  RCC_SPI1SlaveClockPath: specifies the SPI1 slave clock path.
 *    This parameter can be one of the following values:
 *      @arg RCC_SPI1SlaveClockPath_None: No clock path selected
 *      @arg RCC_SPI1SlaveClockPath_PA6: SPI1 slave clock to PA6 path selected
 *      @arg RCC_SPI1SlaveClockPath_PB12: SPI1 slave clock to PB12 path selected
 * @return None
 */
void RCC_SPI1SlaveClockPathConfig(uint32_t RCC_SPI1SlaveClockPath)
{
  RCC->SPI1SCLKCR = RCC_SPI1SlaveClockPath;
}

/**
 * @brief  Checks whether the specified RCC reset flag is set or not.
 * @param  RCC_RSTFLAG: specifies the flag to check.
 *    This parameter can be one of the following values:
 *      @arg RCC_RSTFLAG_LPWRRST: Low Power reset
 *      @arg RCC_RSTFLAG_IWDGRST: Independent Watchdog reset
 *      @arg RCC_RSTFLAG_SFTRST: Software reset
 *      @arg RCC_RSTFLAG_PORRST: POR reset
 *      @arg RCC_RSTFLAG_LVRRST: LVR reset
 *      @arg RCC_RSTFLAG_PINRST: Pin reset
 * @return The new state of RCC_RSTFLAG (SET or RESET).
 */
FlagStatus RCC_GetResetFlagStatus(uint32_t RCC_RSTFLAG)
{
  FlagStatus bitstatus = RESET;

  if ((RCC->RSTSTAT & RCC_RSTFLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }

  /* Return the flag status */
  return bitstatus;
}

/**
 * @brief  Clears the RCC reset flags.
 * @note   The reset flags are RCC_RSTFLAG_LPWRRST, RCC_RSTFLAG_IWDGRST,
 *    RCC_RSTFLAG_SFTRST, RCC_RSTFLAG_PORRST, RCC_RSTFLAG_LVRRST, RCC_RSTFLAG_PINRST
 * @param  None
 * @return None.
 */
void RCC_ClearResetFlags(void)
{
  RCC->CLRRSTSTAT = 0x01;
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
