/**
 * @file     system_wb7720.c
 * @brief    CMSIS Device System Source File for
 *           WB7720 Device Series
 * @version  V0.1.1
 * @date     13-January-2025
 */

#include "wb7720.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/

/*!< Uncomment the line corresponding to the desired system clock (SYSCLK)
   frequency (after reset, the HSI2 is used as SYSCLK source)

   Tip: To avoid modifying this file each time you need to use different system clock,
       you can define the SYSCLK_FREQ_x in your toolchain compiler preprocessor.

   IMPORTANT NOTE:
   ============== 
   1. After reset the HSI2 is used as system clock source.

   2. If none of the following definitions is enabled, the HSI2 is used as 
      system clock source.

   3. Please make sure that the selected system clock doesn't exceed your device's
      maximum frequency.
 */

/* #define SYSCLK_FREQ_HSI2     */
/* #define SYSCLK_FREQ_HSI48d6  */
/* #define SYSCLK_FREQ_HSI48    */

#if ((defined(SYSCLK_FREQ_HSI2) + defined(SYSCLK_FREQ_HSI48d6) + defined(SYSCLK_FREQ_HSI48)) > 1)
  #error "Only one SYSCLK_FREQ_xx macro can be defined!"
#endif


/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
#ifdef SYSCLK_FREQ_HSI2
  uint32_t SystemCoreClock = HSI2_VALUE;          /*!< System Clock Frequency (Core Clock) */
#elif defined SYSCLK_FREQ_HSI48d6
  uint32_t SystemCoreClock = (HSI48_VALUE / 6);   /*!< System Clock Frequency (Core Clock) */
#elif defined SYSCLK_FREQ_HSI48
  uint32_t SystemCoreClock = HSI48_VALUE;         /*!< System Clock Frequency (Core Clock) */
#else
  uint32_t SystemCoreClock = HSI2_VALUE;          /*!< System Clock Frequency (Core Clock) */
#endif


static void SetSysClock(void);

#ifdef SYSCLK_FREQ_HSI2
  static void SetSysClockToHSI2(void);
#elif defined SYSCLK_FREQ_HSI48d6
  static void SetSysClockToHSI48d6(void);
#elif defined SYSCLK_FREQ_HSI48
  static void SetSysClockToHSI48(void);
#endif


/**
 * @brief  Setup the microcontroller system
 * @note   This function should be used only after reset.
 * @param  None
 * @return None
 */
void SystemInit (void)
{
  SetSysClock();
}

/**
 * @brief  Update SystemCoreClock variable according to Clock Register Values.
 *         The SystemCoreClock variable contains the core clock (HCLK), it can
 *         be used by the user application to setup the SysTick timer or configure
 *         other parameters.
 * @note   Each time the core clock (HCLK) changes, this function must be called
 *         to update SystemCoreClock variable value. Otherwise, any configuration
 *         based on this variable will be incorrect.   
 * @param  None
 * @return None
 */
void SystemCoreClockUpdate (void)
{
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
  SystemCoreClock = sysclksrcfreq / sysclkprediv;
}

/**
 * @brief  Configures the system clock frequency.
 * @note   This function should be used only after reset.
 * @param  None
 * @return None
 */
static void SetSysClock(void)
{
#ifdef SYSCLK_FREQ_HSI2
  SetSysClockToHSI2();
#elif defined SYSCLK_FREQ_HSI48d6
  SetSysClockToHSI48d6();
#elif defined SYSCLK_FREQ_HSI48
  SetSysClockToHSI48();
#else
  /* If none of the above definitions is enabled, the HSI2 is used as 
     system clock source (this is default after reset) */
#endif
}


#ifdef SYSCLK_FREQ_HSI2
/**
 * @brief  Selects HSI2 as system clock source.
 * @note   This function should be used only after reset.
 * @param  None
 * @return None
 */
static void SetSysClockToHSI2(void)
{
  /* Unlocks write to ANCTL registers */
  PWR->ANAKEY1 = 0x03;
  PWR->ANAKEY2 = 0x0C;

  RCC->SYSCLKPRE1 = 0x00;
  RCC->SYSCLKSRC = RCC_SYSCLKSRC_HSI2;
  RCC->SYSCLKUEN = RCC_SYSCLKUEN_ENA;
  while (RCC->SYSCLKUEN != 0);

  FLASH->ACR = FLASH_ACR_PRFTBE;

  /* Locks write to ANCTL registers */
  PWR->ANAKEY1 = 0x00;
  PWR->ANAKEY2 = 0x00;
}

#elif defined SYSCLK_FREQ_HSI48d6
/**
 * @brief  Selects HSI48/6 as system clock source.
 * @note   This function should be used only after reset.
 * @param  None
 * @return None
 */
static void SetSysClockToHSI48d6(void)
{
  /* Unlocks write to ANCTL registers */
  PWR->ANAKEY1 = 0x03;
  PWR->ANAKEY2 = 0x0C;

  if ((ANCTL->HSI48SR & 0x03) != 0x01)
  {
    ANCTL->HSI48CR &= ~0x01;
    ANCTL->HSI48ENR = 0x00;
    while ((ANCTL->HSI48SR & 0x03) != 0x02);    // Wait HSI48 disabled
    ANCTL->HSI48CR |= 0x01;
    ANCTL->HSI48ENR = 0x01;
    while ((ANCTL->HSI48SR & 0x03) != 0x01);    // Wait HSI48 ready
  }
  ANCTL->HSI48CR &= ~0x01;
  __NOP();  __NOP();
  ANCTL->HSI48CR |= 0x01;

  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
  RCC->SYSCLKPRE1 = 0x00;
  RCC->SYSCLKSRC = RCC_SYSCLKSRC_HSI48_DIV6;
  RCC->SYSCLKUEN = RCC_SYSCLKUEN_ENA;
  while (RCC->SYSCLKUEN != 0);

  /* Locks write to ANCTL registers */
  PWR->ANAKEY1 = 0x00;
  PWR->ANAKEY2 = 0x00;
}

#elif defined SYSCLK_FREQ_HSI48
/**
 * @brief  Selects HSI48 as system clock source.
 * @note   This function should be used only after reset.
 * @param  None
 * @return None
 */
static void SetSysClockToHSI48(void)
{
  /* Unlocks write to ANCTL registers */
  PWR->ANAKEY1 = 0x03;
  PWR->ANAKEY2 = 0x0C;

  if ((ANCTL->HSI48SR & 0x03) != 0x01)
  {
    ANCTL->HSI48CR &= ~0x01;
    ANCTL->HSI48ENR = 0x00;
    while ((ANCTL->HSI48SR & 0x03) != 0x02);    // Wait HSI48 disabled
    ANCTL->HSI48CR |= 0x01;
    ANCTL->HSI48ENR = 0x01;
    while ((ANCTL->HSI48SR & 0x03) != 0x01);    // Wait HSI48 ready
  }
  ANCTL->HSI48CR &= ~0x01;
  __NOP();  __NOP();
  ANCTL->HSI48CR |= 0x01;

  FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;
  RCC->SYSCLKPRE1 = 0x00;
  RCC->SYSCLKSRC = RCC_SYSCLKSRC_HSI48;
  RCC->SYSCLKUEN = RCC_SYSCLKUEN_ENA;
  while (RCC->SYSCLKUEN != 0);

  /* Locks write to ANCTL registers */
  PWR->ANAKEY1 = 0x00;
  PWR->ANAKEY2 = 0x00;
}

#endif
