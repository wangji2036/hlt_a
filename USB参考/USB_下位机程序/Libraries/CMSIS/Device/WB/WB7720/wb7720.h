/******************************************************************************
 * @file     wb7720.h
 * @brief    CMSIS Core Peripheral Access Layer Header File for
 *           wb7720 Device Series
 * @version  V0.1.1
 * @date     13-January-2025
 * @attention
 * 
 * Copyright (c) 2020 - 2023 Westberry Technology (ChangZhou) Corp., Ltd
 * All rights reserved.
 * 
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, WESTBERRY TECHNOLOGY SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 ******************************************************************************/

/** @addtogroup CMSIS
  * @{
  */

/** @addtogroup wb7720
  * @{
  */

#ifndef __WB7720_H__
#define __WB7720_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup Library_configuration_section
  * @{
  */

#if !defined  USE_STDPERIPH_DRIVER
/**
 * @brief Comment the line below if you will not use the peripherals drivers.
 * In this case, these drivers will not be included and the application code will 
 * be based on direct access to peripherals registers 
 */
  /*#define USE_STDPERIPH_DRIVER*/
#endif


/**
 * @brief In the following line adjust the value of External High Speed oscillator (HSE)
 *        used in your application.
 * 
 * Tip: To avoid modifying this file each time you need to use different HSE, you
 *      can define the HSE value in your toolchain compiler preprocessor.
 */
#if !defined  HSE_VALUE
  #define HSE_VALUE     (8000000)  /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */


/**
 * @brief WB7720 Standard Peripheral Library version number
 */
#define __WB7720_STDPERIPH_VERSION_MAIN   (0x00) /*!< [31:24] main version */
#define __WB7720_STDPERIPH_VERSION_SUB1   (0x00) /*!< [23:16] sub1 version */
#define __WB7720_STDPERIPH_VERSION_SUB2   (0x01) /*!< [15:8]  sub2 version */
#define __WB7720_STDPERIPH_VERSION_RC     (0x00) /*!< [7:0]  release candidate */ 
#define __WB7720_STDPERIPH_VERSION        ( (__WB7720_STDPERIPH_VERSION_MAIN << 24)\
                                             |(__WB7720_STDPERIPH_VERSION_SUB1 << 16)\
                                             |(__WB7720_STDPERIPH_VERSION_SUB2 << 8)\
                                             |(__WB7720_STDPERIPH_VERSION_RC))

/**
  * @}
  */

#define HSI2_VALUE      (1600000)     /*!< Value of the Internal 1.6MHz oscillator in Hz*/
#define HSI48_VALUE     (48000000)    /*!< Value of the Internal 48MHz oscillator in Hz*/


/* -------------------------  Interrupt Number Definition  ------------------------ */

typedef enum IRQn
{
/* -------------------  Cortex-M0 Processor Exceptions Numbers  ------------------- */
  NonMaskableInt_IRQn           = -14,      /*  2 Non Maskable Interrupt */
  HardFault_IRQn                = -13,      /*  3 HardFault Interrupt */
  SVCall_IRQn                   =  -5,      /* 11 SV Call Interrupt */
  PendSV_IRQn                   =  -2,      /* 14 Pend SV Interrupt */
  SysTick_IRQn                  =  -1,      /* 15 System Tick Interrupt */

/* ----------------------  WB7720 Specific Interrupt Numbers  --------------------- */
  BOD_IRQn                      =   0,      /*!< Brown-Out Detection through EXTI Line detection Interrupt */
  EXTI1_0_IRQn                  =   1,      /*!< External Line[1:0] Interrupts */
  EXTI3_2_IRQn                  =   2,      /*!< External Line[3:2] Interrupts */
  FLASH_IRQn                    =   3,      /*!< FLASH Interrupt */
  EXTI5_4_IRQn                  =   4,      /*!< External Line[5:4] Interrupts */
  EXTI7_6_IRQn                  =   5,      /*!< External Line[7:6] Interrupts */
  EXTI15_8_IRQn                 =   6,      /*!< External Line[15:8] Interrupts */
  UART0_IRQn                    =   7,      /*!< UART0 Interrupt */
  UART1_IRQn                    =   8,      /*!< UART1 Interrupt */
  SPIM_IRQn                     =   9,      /*!< SPIM Interrupt */
  SPI1_IRQn                     =   10,     /*!< SPI1 Interrupt */
  I2C_IRQn                      =   11,     /*!< I2C Interrupt */
  ARGB_IRQn                     =   12,     /*!< ARGB Interrupt */
  TIM6_IRQn                     =   13,     /*!< TIM6 Interrupt */
  PCT0_IRQn                     =   14,     /*!< PCT0 Interrupt */
  PCT1_IRQn                     =   15,     /*!< PCT1 Interrupt */
  PCT2_IRQn                     =   16,     /*!< PCT2 Interrupt */
  PCT3_IRQn                     =   17,     /*!< PCT3 Interrupt */
  PCT4_IRQn                     =   18,     /*!< PCT4 Interrupt */
  USBP_WKUP_IRQn                =   19,     /*!< USB PIN Interrupt */
  RTC_IRQn                      =   20,     /*!< RTC Interrupt */
  ADC_IRQn                      =   21,     /*!< ADC Interrupt */
  USB_IRQn                      =   22,     /*!< USB Interrupt */
  CMP0_IRQn                     =   23,     /*!< CMP0 Interrupt */
  CMP1_IRQn                     =   24,     /*!< CMP1 Interrupt */
  CRS_IRQn                      =   25,     /*!< CRS Interrupt */
} IRQn_Type;


/* ================================================================================ */
/* ================      Processor and Core Peripheral Section     ================ */
/* ================================================================================ */

/* -------  Start of section using anonymous unions and disabling warnings  ------- */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif


/* --------  Configuration of Core Peripherals  ----------------------------------- */
#define __CM0_REV                 0x0000U   /* Core revision r0p0 */
#define __MPU_PRESENT             0U        /* no MPU present */
#define __VTOR_PRESENT            0U        /* no VTOR present */
#define __NVIC_PRIO_BITS          2U        /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */

#include "core_cm0.h"                       /* Processor and core peripherals */
#include "system_wb7720.h"                /* System Header */



typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;


/* ================================================================================ */
/* ================       Device Specific Peripheral Section       ================ */
/* ================================================================================ */


/* ================================================================================ */
/* ================          Reset and Clock Control (RCC)         ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t PLLPRE1;         /*!< RCC PLL clock prescaler register 1,                  Address offset: 0x000 */
  __IOM uint32_t PLLPRE2;         /*!< RCC PLL clock prescaler register 2,                  Address offset: 0x004 */
  __IOM uint32_t PLLSRC;          /*!< RCC PLL clock source select register,                Address offset: 0x008 */
  __IOM uint32_t SYSCLKPRE1;      /*!< RCC SYSCLK clock prescaler register 1,               Address offset: 0x00C */
        uint32_t RESERVED0;       /*!< Reserved,                                                            0x010 */
  __IOM uint32_t SYSCLKSRC;       /*!< RCC SYSCLK clock source select register,             Address offset: 0x014 */
  __IOM uint32_t SYSCLKUEN;       /*!< RCC SYSCLK clock source update enable register,      Address offset: 0x018 */
  __IOM uint32_t FCUCLKPRE1;      /*!< RCC FCU clock prescaler register 1,                  Address offset: 0x01C */
  __IOM uint32_t FCUCLKPRE2;      /*!< RCC FCU clock prescaler register 2,                  Address offset: 0x020 */
  __IOM uint32_t FCUCLKSRC;       /*!< RCC FCU clock source select register,                Address offset: 0x024 */
  __IOM uint32_t PLL2USBPRE1;     /*!< RCC PLL to USB clock prescaler register 1,           Address offset: 0x028 */
  __IOM uint32_t PLL2USBPRE2;     /*!< RCC PLL to USB clock prescaler register 2,           Address offset: 0x02C */
  __IOM uint32_t USBCLKSRC;       /*!< RCC USBCLK clock source select register,             Address offset: 0x030 */
  __IOM uint32_t MCOPRE;          /*!< RCC MCO clock prescaler register,                    Address offset: 0x034 */
  __IOM uint32_t MCOSEL;          /*!< RCC MCO clock source select register,                Address offset: 0x038 */
  __IOM uint32_t MCOUEN;          /*!< RCC MCO clock source update enable register,         Address offset: 0x03C */
  __IOM uint32_t MCOENR;          /*!< RCC MCO output enable register,                      Address offset: 0x040 */
        uint32_t RESERVED1;       /*!< Reserved,                                                            0x044 */
  __IOM uint32_t APBENR;          /*!< RCC APB peripheral clock enable register,            Address offset: 0x048 */
        uint32_t RESERVED2;       /*!< Reserved,                                                            0x04C */
  __IOM uint32_t APBRSTR;         /*!< RCC APB peripheral reset register,                   Address offset: 0x050 */
  __IOM uint32_t SPI1SCLKCR;      /*!< RCC SPI1 slave clock control register,               Address offset: 0x054 */
        uint32_t RESERVED3;       /*!< Reserved,                                                            0x058 */
  __IOM uint32_t CLRRSTSTAT;      /*!< RCC clear reset status register,                     Address offset: 0x05C */
  __IOM uint32_t RSTSTAT;         /*!< RCC reset status register,                           Address offset: 0x060 */
  __IOM uint32_t GPREG0;          /*!< General purpose register 0,                          Address offset: 0x064 */
  __IOM uint32_t GPREG1;          /*!< General purpose register 1,                          Address offset: 0x068 */
  __IOM uint32_t GPREG2;          /*!< General purpose register 2,                          Address offset: 0x06C */
} RCC_TypeDef;


/* ================================================================================ */
/* ================              Power Control (PWR)               ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t SLEEP_CR;        /*!< SLEEP mode control register,                         Address offset: 0x000 */
  __IOM uint32_t STOP_CR0;        /*!< STOP mode control register 0,                        Address offset: 0x004 */
  __IOM uint32_t STOP_CR1;        /*!< STOP mode control register 1,                        Address offset: 0x008 */
  __IOM uint32_t STOP_CR2;        /*!< STOP mode control register 2,                        Address offset: 0x00C */
  __IOM uint32_t STOP_CR3;        /*!< STOP mode control register 3,                        Address offset: 0x010 */
  __IOM uint32_t STOP_CR4;        /*!< STOP mode control register 4,                        Address offset: 0x014 */
  __IOM uint32_t STOP_CR5;        /*!< STOP mode control register 5,                        Address offset: 0x018 */
  __IOM uint32_t SR;              /*!< Status register,                                     Address offset: 0x01C */
  __IOM uint32_t ANAKEY1;         /*!< ANCTL write enable key register 1,                   Address offset: 0x020 */
  __IOM uint32_t ANAKEY2;         /*!< ANCTL write enable key register 2,                   Address offset: 0x024 */
} PWR_TypeDef;


/* ================================================================================ */
/* ================             Analog Control (ANCTL)             ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t HPLDOEN;         /*!< HPLDO enable register,                               Address offset: 0x000 */
  __IOM uint32_t HPLDOCON;        /*!< HPLDO control register,                              Address offset: 0x004 */
  __IOM uint32_t LPLDOEN;         /*!< LPLDO enable register,                               Address offset: 0x008 */
  __IOM uint32_t LPLDOCON;        /*!< LPLDO control register,                              Address offset: 0x00C */
  __IOM uint32_t HPBIASEN;        /*!< HPBIAS enable register,                              Address offset: 0x010 */
  __IOM uint32_t BODCON;          /*!< Brown-Out Detection control register,                Address offset: 0x014 */
  __IOM uint32_t BODSR;           /*!< Brown-Out Detection status register,                 Address offset: 0x018 */
        uint32_t RESERVED0[3];    /*!< Reserved,                                                    0x01C - 0x024 */
  __IOM uint32_t HSI2ENR;         /*!< HSI2 enable register,                                Address offset: 0x028 */
  __IOM uint32_t CMP0CON;         /*!< Comparator 0 control register,                       Address offset: 0x02C */
  __IOM uint32_t CMP0SEL;         /*!< Comparator 0 input select register,                  Address offset: 0x030 */
        uint32_t RESERVED1;       /*!< Reserved,                                                            0x034 */
  __IOM uint32_t CMP0OUT;         /*!< Comparator 0 output state register,                  Address offset: 0x038 */
  __IOM uint32_t CMP1CON;         /*!< Comparator 1 control register,                       Address offset: 0x03C */
  __IOM uint32_t CMP1SEL;         /*!< Comparator 1 input select register,                  Address offset: 0x040 */
        uint32_t RESERVED2;       /*!< Reserved,                                                            0x044 */
  __IOM uint32_t CMP1OUT;         /*!< Comparator 1 output state register,                  Address offset: 0x048 */
  __IOM uint32_t DACCON;          /*!< DAC control register,                                Address offset: 0x04C */
  __IOM uint32_t DACDAT;          /*!< DAC data register,                                   Address offset: 0x050 */
  __IOM uint32_t DACTRIM;         /*!< DAC trim register,                                   Address offset: 0x054 */
  __IOM uint32_t PLLENR;          /*!< PLL enable register,                                 Address offset: 0x058 */
  __IOM uint32_t PLLCR1;          /*!< PLL control register 1,                              Address offset: 0x05C */
  __IOM uint32_t PLLCR2;          /*!< PLL control register 2,                              Address offset: 0x060 */
  __IOM uint32_t PLLTEST;         /*!< PLL test register,                                   Address offset: 0x064 */
  __IOM uint32_t PLLSR;           /*!< PLL status register,                                 Address offset: 0x068 */
  __IOM uint32_t LSICR;           /*!< LSI control register,                                Address offset: 0x06C */
  __IOM uint32_t LSITRIM;         /*!< LSI trim register,                                   Address offset: 0x070 */
  __IOM uint32_t LSITEST;         /*!< LSI test register,                                   Address offset: 0x074 */
  __IOM uint32_t LSISR;           /*!< LSI status register,                                 Address offset: 0x078 */
  __IOM uint32_t HSI48ENR;        /*!< HSI48 enable register,                               Address offset: 0x07C */
  __IOM uint32_t HSI48CR;         /*!< HSI48 control register,                              Address offset: 0x080 */
  __IOM uint32_t HSI48TRIM;       /*!< HSI48 trim register,                                 Address offset: 0x084 */
        uint32_t RESERVED4;       /*!< Reserved,                                                            0x088 */
  __IOM uint32_t HSI48TEST;       /*!< HSI48 test register,                                 Address offset: 0x08C */
  __IOM uint32_t HSI48SR;         /*!< HSI48 status register,                               Address offset: 0x090 */
  __IOM uint32_t USBPDSR;         /*!< USB port state detect status register,               Address offset: 0x094 */
  __IOM uint32_t USBPDCR;         /*!< USB port state detect control register,              Address offset: 0x098 */
  __IOM uint32_t HSECR;           /*!< HSE control register,                                Address offset: 0x09C */
} ANCTL_TypeDef;


/* ================================================================================ */
/* ================           General-purpose I/Os (GPIO)          ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t MODER;           /*!< GPIO port mode register,                             Address offset: 0x000 */
  __IOM uint32_t OTYPER;          /*!< GPIO port output type register,                      Address offset: 0x004 */
  __IOM uint32_t OSPEEDR;         /*!< GPIO port output speed register,                     Address offset: 0x008 */
  __IOM uint32_t PUPDR;           /*!< GPIO port pull-up/pull-down register,                Address offset: 0x00C */
  __IM  uint32_t IDR;             /*!< GPIO port input data register,                       Address offset: 0x010 */
  __IOM uint32_t ODR;             /*!< GPIO port output data register,                      Address offset: 0x014 */
  __OM  uint32_t BSRR;            /*!< GPIO port bit set/reset register,                    Address offset: 0x018 */
  __IOM uint32_t LCKR;            /*!< GPIO port configuration lock register,               Address offset: 0x01C */
  __IOM uint32_t AFRL;            /*!< GPIO alternate function low register,                Address offset: 0x020 */
  __IOM uint32_t AFRH;            /*!< GPIO alternate function high register,               Address offset: 0x024 */
  __OM  uint32_t BRR;             /*!< GPIO port bit reset register,                        Address offset: 0x028 */
  __IOM uint32_t CSR;             /*!< GPIO port CMOS/Schmitt register,                     Address offset: 0x02C */
  __IOM uint32_t DRR;             /*!< GPIO port drive strength register,                   Address offset: 0x030 */
} GPIO_TypeDef;


/* ================================================================================ */
/* ================   External Interrupt/Event Controller (EXTI)   ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t IMR;             /*!< Interrupt mask register,                             Address offset: 0x000 */
  __IOM uint32_t EMR;             /*!< Event mask register,                                 Address offset: 0x004 */
  __IOM uint32_t RTSR;            /*!< Rising trigger selection register,                   Address offset: 0x008 */
  __IOM uint32_t FTSR;            /*!< Falling trigger selection register,                  Address offset: 0x00C */
  __IOM uint32_t SWIER;           /*!< Software interrupt event register,                   Address offset: 0x010 */
  __IOM uint32_t PR;              /*!< Pending register,                                    Address offset: 0x014 */
} EXTI_TypeDef;


/* ================================================================================ */
/* ================          System Configuration (SYSCFG)         ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CFGR1;           /*!< SYSCFG configuration register 1,                                     Address offset: 0x000 */
  __IOM uint32_t EXTICR[4];       /*!< SYSCFG external interrupt configuration register 1 to 4,     Address offset: 0x004 - 0x010 */
        uint32_t RESERVED0[2];    /*!< Reserved,                                                                    0x014 - 0x018 */
  __IOM uint32_t CFGR2;           /*!< SYSCFG configuration register 2,                                     Address offset: 0x01C */
  __IOM uint32_t CFGR3;           /*!< SYSCFG configuration register 3,                                     Address offset: 0x020 */
  __IOM uint32_t CFGR4;           /*!< SYSCFG configuration register 4,                                     Address offset: 0x024 */
} SYSCFG_TypeDef;


/* ================================================================================ */
/* ================          Clock Recovery System (CRS)           ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR;              /*!< CRS control register,                                Address offset: 0x000 */
  __IOM uint32_t CFGR;            /*!< CRS configuration register,                          Address offset: 0x004 */
  __IOM uint32_t ISR;             /*!< CRS interrupt and status register,                   Address offset: 0x008 */
  __IOM uint32_t ICR;             /*!< CRS interrupt flag clear register,                   Address offset: 0x00C */
} CRS_TypeDef;


/* ================================================================================ */
/* ================          Cyclic Redundancy Check (CRC)         ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t MODE;            /*!< CRC Mode register,                                   Address offset: 0x000 */
  __IOM uint32_t SEED;            /*!< CRC Seed register,                                   Address offset: 0x004 */
  union {
  __IM uint32_t SUM;              /*!< CRC Sum register,                                    Address offset: 0x008 */
  __OM uint32_t WR_DATA_WORD;     /*!< CRC Data register for word,                          Address offset: 0x008 */
  __OM uint16_t WR_DATA_HALF_WORD;/*!< CRC Data register for half word,                     Address offset: 0x008 */
  __OM uint8_t  WR_DATA_BYTE;     /*!< CRC Data register for byte,                          Address offset: 0x008 */
  };
} CRC_TypeDef;


/* ================================================================================ */
/* ============== Universal Asyncronous Receiver / Transmitter (UART) ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR1;             /*!< Control register 1,                                  Address offset: 0x000 */
  __IOM uint32_t CR2;             /*!< Control register 2,                                  Address offset: 0x004 */
  __IOM uint32_t CR3;             /*!< Control register 3,                                  Address offset: 0x008 */
  __IOM uint32_t BRR;             /*!< Baud rate register,                                  Address offset: 0x00C */
        uint32_t RESERVED0[2];    /*!< Reserved,                                                    0x010 - 0x014 */
  __OM  uint32_t RQR;             /*!< Request register,                                    Address offset: 0x018 */
  __IM  uint32_t ISR;             /*!< Interrupt and status register,                       Address offset: 0x01C */
  __IOM uint32_t ICR;             /*!< Interrupt flag Clear register,                       Address offset: 0x020 */
  __IM  uint32_t RDR;             /*!< Receive Data register,                               Address offset: 0x024 */
  __IOM uint32_t TDR;             /*!< Transmit Data register,                              Address offset: 0x028 */
} UART0_TypeDef;


/* ================================================================================ */
/* ============== Universal Asyncronous Receiver / Transmitter (UART) ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t DR;              /*!< Data register,                                       Address offset: 0x000 */
  __IOM uint32_t SR;              /*!< Status register,                                     Address offset: 0x004 */
  __IOM uint32_t CR;              /*!< Control register,                                    Address offset: 0x008 */
  __IOM uint32_t ISR;             /*!< Interrupt status register,                           Address offset: 0x00C */
  __IOM uint32_t BDR;             /*!< Baud rate divider,                                   Address offset: 0x010 */
        uint32_t RESERVED0[59];   /*!< Reserved,                                                    0x014 - 0x0FC */
  __IOM uint32_t ABRCON;          /*!< Auto Baud Rate Control Register,                     Address offset: 0x100 */
  __IM  uint32_t ABRVAL;          /*!< Auto Baud Rate Value Register,                       Address offset: 0x104 */
  __IM  uint32_t ABRSR;           /*!< Auto Baud Rate Status Register,                      Address offset: 0x108 */
  __IOM uint32_t ABRIE;           /*!< Auto Baud Rate Interrupt Enable Register,            Address offset: 0x10C */
  __OM  uint32_t ABRIC;           /*!< Auto Baud Rate Interrupt Clear Register,             Address offset: 0x110 */
} UART1_TypeDef;


/* ================================================================================ */
/* ================             FLASH Registers (FLASH)            ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t ACR;             /*!< Flash access control register,                       Address offset: 0x000 */
  __IOM uint32_t KEYR;            /*!< Flash key register,                                  Address offset: 0x004 */
  __IOM uint32_t CR;              /*!< Flash control register,                              Address offset: 0x008 */
  __IOM uint32_t AR;              /*!< Flash address register,                              Address offset: 0x00C */
  __IOM uint32_t DR;              /*!< Flash data register,                                 Address offset: 0x010 */
  __IOM uint32_t SR;              /*!< Flash status register,                               Address offset: 0x014 */
  __IOM uint32_t OBR1;            /*!< Option byte register 1,                              Address offset: 0x018 */
  __IOM uint32_t OBR2;            /*!< Option byte register 2,                              Address offset: 0x01C */
} FLASH_TypeDef;


/* ================================================================================ */
/* ================           Independent watchdog (IWDG)          ================ */
/* ================================================================================ */
typedef struct
{
  __OM  uint32_t KR;              /*!< Key register,                                        Address offset: 0x000 */
  __IOM uint32_t PR;              /*!< Prescaler register,                                  Address offset: 0x004 */
  __IOM uint32_t RLR;             /*!< Reload register,                                     Address offset: 0x008 */
  __IM  uint32_t SR;              /*!< Status register,                                     Address offset: 0x00C */
} IWDG_TypeDef;


/* ================================================================================ */
/* ================              Real-Time Clock (RTC)             ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint16_t CRH;             /*!< RTC control register high,                           Address offset: 0x000 */    uint16_t RESERVED0;
  __IOM uint16_t CRL;             /*!< RTC control register low,                            Address offset: 0x004 */    uint16_t RESERVED1;
  __OM  uint16_t PRLH;            /*!< RTC prescaler load register high,                    Address offset: 0x008 */    uint16_t RESERVED2;
  union {
  __IM  uint32_t PRL;             /*!< RTC prescaler load register (read only),             Address offset: 0x00C */
  __OM  uint16_t PRLL;            /*!< RTC prescaler load register low,                     Address offset: 0x00C */
  };
  __IM  uint16_t DIVH;            /*!< RTC prescaler divider register high,                 Address offset: 0x010 */    uint16_t RESERVED4;
  union {
  __IM  uint32_t DIV;             /*!< RTC prescaler divider register,                      Address offset: 0x014 */
  __IM  uint16_t DIVL;            /*!< RTC prescaler divider register low,                  Address offset: 0x014 */
  };
  __IOM uint16_t CNTH;            /*!< RTC counter register high,                           Address offset: 0x018 */    uint16_t RESERVED5;
  union {
  __IM  uint32_t CNT;             /*!< RTC counter register (read only),                    Address offset: 0x01C */
  __IOM uint16_t CNTL;            /*!< RTC counter register low,                            Address offset: 0x01C */
  };
  __OM  uint16_t ALRH;            /*!< RTC alarm register high,                             Address offset: 0x020 */    uint16_t RESERVED6;
  union {
  __IM  uint32_t ALR;             /*!< RTC alarm register (read only),                      Address offset: 0x024 */
  __OM  uint16_t ALRL;            /*!< RTC alarm register low,                              Address offset: 0x024 */
  };
} RTC_TypeDef;


/* ================================================================================ */
/* ================   Pulse Width Modulator/Capture Timer (PCT)    ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CSR;             /*!< Control and status register,                         Address offset: 0x000 */
  __IOM uint32_t CFGR;            /*!< Configuration register,                              Address offset: 0x004 */
  __IOM uint32_t CNTH;            /*!< 16-bit counter register high,                        Address offset: 0x008 */
  union {
  __IOM uint32_t CNTL;            /*!< 16-bit counter register low,                         Address offset: 0x00C */
  __IM  uint32_t CNT;             /*!< 16-bit counter register (read only),                 Address offset: 0x00C */
  };
  __IOM uint32_t CFGR2;           /*!< Configuration register 2,                            Address offset: 0x010 */
  __IOM uint32_t ARRL;            /*!< Auto-reload register low,                            Address offset: 0x014 */
  __IOM uint32_t ARRH;            /*!< Auto-reload register high,                           Address offset: 0x018 */
  __IOM uint32_t FTCMPAL;         /*!< PWM final compare value register A low,              Address offset: 0x01C */
  __IOM uint32_t FTCMPAH;         /*!< PWM final compare value register A high,             Address offset: 0x020 */
  __IOM uint32_t FTCMPBL;         /*!< PWM final compare value register B low,              Address offset: 0x024 */
  __IOM uint32_t FTCMPBH;         /*!< PWM final compare value register B high,             Address offset: 0x028 */
        uint32_t RESERVED0;       /*!< Reserved,                                                            0x02C */
  __IOM uint32_t CCR0H;           /*!< Capture/Compare value register 0 high,               Address offset: 0x030 */
  union {
  __IOM uint32_t CCR0L;           /*!< Capture/Compare value register 0 low,                Address offset: 0x034 */
  __IM  uint32_t CCR0;            /*!< Capture/Compare value register 0 (read only),        Address offset: 0x034 */
  };
  __IOM uint32_t CCR1H;           /*!< Capture/Compare value register 1 high,               Address offset: 0x038 */
  union {
  __IOM uint32_t CCR1L;           /*!< Capture/Compare value register 1 low,                Address offset: 0x03C */
  __IM  uint32_t CCR1;            /*!< Capture/Compare value register 1 (read only),        Address offset: 0x03C */
  };
  __IOM uint32_t CCR2H;           /*!< Capture/Compare value register 2 high,               Address offset: 0x040 */
  union {
  __IOM uint32_t CCR2L;           /*!< Capture/Compare value register 2 low,                Address offset: 0x044 */
  __IM  uint32_t CCR2;            /*!< Capture/Compare value register 2 (read only),        Address offset: 0x044 */
  };
  __IOM uint32_t CCR3H;           /*!< Capture/Compare value register 3 high,               Address offset: 0x048 */
  union {
  __IOM uint32_t CCR3L;           /*!< Capture/Compare value register 3 low,                Address offset: 0x04C */
  __IOM uint32_t CCR3;            /*!< Capture/Compare value register 3 (read only),        Address offset: 0x04C */
  };
  __IOM uint32_t CCR4H;           /*!< Capture/Compare value register 4 high,               Address offset: 0x050 */
  union {
  __IOM uint32_t CCR4L;           /*!< Capture/Compare value register 4 low,                Address offset: 0x054 */
  __IM  uint32_t CCR4;            /*!< Capture/Compare value register 4 (read only),        Address offset: 0x054 */
  };
  __IOM uint32_t CCR5H;           /*!< Capture/Compare value register 5 high,               Address offset: 0x058 */
  union {
  __IOM uint32_t CCR5L;           /*!< Capture/Compare value register 5 low,                Address offset: 0x05C */
  __IM  uint32_t CCR5;            /*!< Capture/Compare value register 5 (read only),        Address offset: 0x05C */
  };
  __IOM uint32_t CCMR0;           /*!< Capture/Compare mode register 0,                     Address offset: 0x060 */
  __IOM uint32_t CCMR1;           /*!< Capture/Compare mode register 1,                     Address offset: 0x064 */
  __IOM uint32_t CCMR2;           /*!< Capture/Compare mode register 2,                     Address offset: 0x068 */
  __IOM uint32_t CCMR3;           /*!< Capture/Compare mode register 3,                     Address offset: 0x06C */
  __IOM uint32_t CCMR4;           /*!< Capture/Compare mode register 4,                     Address offset: 0x070 */
  __IOM uint32_t CCMR5;           /*!< Capture/Compare mode register 5,                     Address offset: 0x074 */
} PCT_TypeDef;


/* ================================================================================ */
/* ================                   Timer (TIM)                  ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR1;             /*!< Control register 1,                                  Address offset: 0x000 */
  __IOM uint32_t CR2;             /*!< Control register 2,                                  Address offset: 0x004 */
        uint32_t RESERVED0;       /*!< Reserved,                                                            0x008 */
  __IOM uint32_t DIER;            /*!< interrupt enable register,                           Address offset: 0x00C */
  __IOM uint32_t SR;              /*!< Status register,                                     Address offset: 0x010 */
  __OM  uint32_t EGR;             /*!< Event generation register,                           Address offset: 0x014 */
        uint32_t RESERVED1[3];    /*!< Reserved,                                                    0x018 - 0x020 */
  __IOM uint32_t CNT;             /*!< Counter,                                             Address offset: 0x024 */
  __IOM uint32_t PSC;             /*!< Prescaler,                                           Address offset: 0x028 */
  __IOM uint32_t ARR;             /*!< Auto-reload register,                                Address offset: 0x02C */
} TIM_TypeDef;


/* ================================================================================ */
/* ================         Inter-Integrated Circuit (I2C)         ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CON;             /*!< I2C control register,                                Address offset: 0x000 */
  __IOM uint32_t ADR;             /*!< I2C slave address register,                          Address offset: 0x004 */
        uint32_t RESERVED0;       /*!< Reserved,                                                            0x008 */
  __IM  uint32_t STAT;            /*!< I2C status register,                                 Address offset: 0x00C */
  __IOM uint32_t DAT;             /*!< I2C data register,                                   Address offset: 0x010 */
} I2C_TypeDef;


/* ================================================================================ */
/* ==============          Serial Peripheral Interface (SPIM)         ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR0;             /*!< Control Register 0,                                  Address offset: 0x000 */
  __IOM uint32_t CR1;             /*!< Control Register 1,                                  Address offset: 0x004 */
  __IOM uint32_t SPIENR;          /*!< SPI Enable Register,                                 Address offset: 0x008 */
  __IOM uint32_t MWCR;            /*!< Microwire Control Register,                          Address offset: 0x00C */
  __IOM uint32_t SER;             /*!< Slave Enable Register,                               Address offset: 0x010 */
  __IOM uint32_t BAUDR;           /*!< Baud Rate Select,                                    Address offset: 0x014 */
  __IOM uint32_t TXFTLR;          /*!< Transmit FIFO Threshold Level,                       Address offset: 0x018 */
  __IOM uint32_t RXFTLR;          /*!< Receive FIFO Threshold Level,                        Address offset: 0x01C */
  __IM  uint32_t TXFLR;           /*!< Transmit FIFO Level Register,                        Address offset: 0x020 */
  __IM  uint32_t RXFLR;           /*!< Receive FIFO Level Register,                         Address offset: 0x024 */
  __IM  uint32_t SR;              /*!< Status Register,                                     Address offset: 0x028 */
  __IOM uint32_t IER;             /*!< Interrupt Enable Register,                           Address offset: 0x02C */
  __IM  uint32_t ISR;             /*!< Interrupt Status Register,                           Address offset: 0x030 */
  __IM  uint32_t RISR;            /*!< Raw Interrupt Status Register,                       Address offset: 0x034 */
  __IM  uint32_t TXOICR;          /*!< Transmit FIFO Overflow Interrupt Clear Register,     Address offset: 0x038 */
  __IM  uint32_t RXOICR;          /*!< Receive FIFO Overflow Interrupt Clear Register,      Address offset: 0x03C */
  __IM  uint32_t RXUICR;          /*!< Receive FIFO Underflow Interrupt Clear Register,     Address offset: 0x040 */
  __IM  uint32_t MSTICR;          /*!< Multi-Master Interrupt Clear Register,               Address offset: 0x044 */
  __IM  uint32_t ICR;             /*!< Interrupt Clear Register,                            Address offset: 0x048 */
        uint32_t RESERVED0[5];    /*!< Reserved,                                                    0x04C - 0x05C */
  __IOM uint32_t DR;              /*!< Data Register,                                       Address offset: 0x060 */
        uint32_t RESERVED1[35];   /*!< Reserved,                                                    0x064 - 0x0EC */
  __IOM uint32_t RX_SAMPLE_DLY;   /*!< RX Sample Delay Register,                            Address offset: 0x0F0 */
} SPIM_TypeDef;


/* ================================================================================ */
/* ==============          Serial Peripheral Interface (SPI)          ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR;              /*!< Control Register,                                    Address offset: 0x000 */
  __IOM uint32_t SR;              /*!< Status Register,                                     Address offset: 0x004 */
  __IOM uint32_t DR;              /*!< Data Register,                                       Address offset: 0x008 */
  __IOM uint32_t CR2;             /*!< Control Register 2,                                  Address offset: 0x00C */
} SPI_TypeDef;


/* ================================================================================ */
/* ================       Analog to Digital Converter (ADC)        ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR;              /*!< ADC control register,                                Address offset: 0x000 */
  __IOM uint32_t DR;              /*!< ADC data register,                                   Address offset: 0x004 */
  __IOM uint32_t ISR;             /*!< ADC interrupt status register,                       Address offset: 0x008 */
  __IOM uint32_t IER;             /*!< ADC interrupt enable register,                       Address offset: 0x00C */
  __IOM uint32_t ICR;             /*!< ADC interrupt clear register,                        Address offset: 0x010 */
  __IOM uint32_t CAL;             /*!< ADC calibration register,                            Address offset: 0x014 */
} ADC_TypeDef;


/* ================================================================================ */
/* ==============        Addressable RGB LED Controller (ARGB)        ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint8_t CTRL;          /*!< Control Register,                                       Address offset: 0x000 */ uint8_t RESERVED0[3];
  __IOM uint8_t STATUS;        /*!< Status Register,                                        Address offset: 0x004 */ uint8_t RESERVED1[3];
  __IOM uint8_t DAT;           /*!< Data Register,                                          Address offset: 0x008 */ uint8_t RESERVED2[3];
  __IOM uint8_t PERIODCNT;     /*!< Data Period Count Register,                             Address offset: 0x00C */ uint8_t RESERVED3[3];
  __IOM uint8_t T1HCNT;        /*!< T1H Count Register,                                     Address offset: 0x010 */ uint8_t RESERVED4[3];
  __IOM uint8_t TRSTCNTL;      /*!< Treset Count Low Register,                              Address offset: 0x014 */ uint8_t RESERVED5[3];
  __IOM uint8_t TRSTCNTH;      /*!< Treset Count High Register,                             Address offset: 0x018 */
} ARGB_TypeDef;


/* ================================================================================ */
/* ==============             Universal Serial Bus (USB)              ============= */
/* ================================================================================ */
typedef struct
{
  __IOM uint8_t FADDR;         /*!< Function address register,                                     Address offset: 0x000 */
  __IOM uint8_t POWER;         /*!< Power management register,                                     Address offset: 0x001 */
  __IM  uint8_t INTRIN;        /*!< Interrupt register for Endpoint 0 plus IN Endpoints 1 to 4,    Address offset: 0x002 */
        uint8_t RESERVED0;     /*!< Reserved,                                                                      0x003 */
  __IM  uint8_t INTROUT;       /*!< Interrupt register for OUT Endpoints 1 to 4,                   Address offset: 0x004 */
        uint8_t RESERVED1;     /*!< Reserved,                                                                      0x005 */
  __IM  uint8_t INTRUSB;       /*!< Interrupt register for common USB interrupts,                  Address offset: 0x006 */
  __IOM uint8_t INTRINE;       /*!< Interrupt enable register for IntrIn,                          Address offset: 0x007 */
        uint8_t RESERVED2;     /*!< Reserved,                                                                      0x008 */
  __IOM uint8_t INTROUTE;      /*!< Interrupt enable register for IntrOut,                         Address offset: 0x009 */
        uint8_t RESERVED3;     /*!< Reserved,                                                                      0x00A */
  __IOM uint8_t INTRUSBE;      /*!< Interrupt enable register for IntrUSB,                         Address offset: 0x00B */
  __IM  uint8_t FRAMEL;        /*!< Frame number bits 0 to 7,                                      Address offset: 0x00C */
  __IM  uint8_t FRAMEH;        /*!< Frame number bits 8 to 10,                                     Address offset: 0x00D */
  __IOM uint8_t INDEX;         /*!< Index register,                                                Address offset: 0x00E */
        uint8_t RESERVED4;     /*!< Reserved,                                                                      0x00F */

  __IOM uint8_t INMAXP;        /*!< Maximum packet size for IN endpoint,                           Address offset: 0x010 */
  union {
  __IOM uint8_t CSR0;          /*!< Control Status register for Endpoint 0,                        Address offset: 0x011 */
  __IOM uint8_t INCSR1;        /*!< Control Status register 1 for IN endpoint,                     Address offset: 0x011 */
  };
  __IOM uint8_t INCSR2;        /*!< Control Status register 2 for IN endpoint,                     Address offset: 0x012 */
  __IOM uint8_t OUTMAXP;       /*!< Maximum packet size for OUT endpoint,                          Address offset: 0x013 */
  __IOM uint8_t OUTCSR1;       /*!< Control Status register 1 for OUT endpoint,                    Address offset: 0x014 */
  __IOM uint8_t OUTCSR2;       /*!< Control Status register 2 for OUT endpoint,                    Address offset: 0x015 */
  union {
  __IM  uint8_t COUNT0;        /*!< Number of received bytes in Endpoint 0 FIFO,                   Address offset: 0x016 */
  __IM  uint8_t OUTCOUNTL;     /*!< Number of bytes in OUT endpoint FIFO (lower byte),             Address offset: 0x016 */
  };
  __IM  uint8_t OUTCOUNTH;     /*!< Number of bytes in OUT endpoint FIFO (upper byte),             Address offset: 0x017 */
        uint8_t RESERVED5[8];  /*!< Reserved,                                                              0x018 - 0x01F */

  __IOM uint32_t FIFO[16];        /*!< FIFOs for Endpoints 0 to 15 (must accessed by bytes),       Address offset: 0x020 - 0x05F */
} USB_TypeDef;


/* ================================================================================ */
/* ================               Debug MCU (DBGMCU)               ================ */
/* ================================================================================ */
typedef struct
{
  __IOM uint32_t CR;            /*!< Debug MCU configuration register,                      Address offset: 0x000 */
} DBGMCU_TypeDef;


/* --------  End of section using anonymous unions and disabling warnings  -------- */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif




/* ================================================================================ */
/* ================              Peripheral memory map             ================ */
/* ================================================================================ */
#define PCT0_BASE             ((uint32_t)0x40000000UL)
#define PCT1_BASE             ((uint32_t)0x40000400UL)
#define PCT2_BASE             ((uint32_t)0x40001400UL)
#define PCT3_BASE             ((uint32_t)0x40002000UL)
#define PCT4_BASE             ((uint32_t)0x40002C00UL)

#define TIM6_BASE             ((uint32_t)0x40001000UL)

#define RTC_BASE              ((uint32_t)0x40002800UL)

#define IWDG_BASE             ((uint32_t)0x40003000UL)

#define I2C_BASE              ((uint32_t)0x40005400UL)

#define SPIM_BASE             ((uint32_t)0x40003800UL)

#define SPI1_BASE             ((uint32_t)0x40011400UL)

#define ADC_BASE              ((uint32_t)0x40011800UL)

#define ARGB0_BASE            ((uint32_t)0x40011C00UL)
#define ARGB1_BASE            ((uint32_t)0x40011C01UL)
#define ARGB2_BASE            ((uint32_t)0x40011C02UL)
#define ARGB3_BASE            ((uint32_t)0x40011C03UL)

#define ARGB4_BASE            ((uint32_t)0x40012400UL)
#define ARGB5_BASE            ((uint32_t)0x40012401UL)

#define GPIOA_BASE            ((uint32_t)0x48000000UL)
#define GPIOB_BASE            ((uint32_t)0x48000400UL)
#define GPIOC_BASE            ((uint32_t)0x48000800UL)
#define GPIOD_BASE            ((uint32_t)0x48000C00UL)

#define EXTI_BASE             ((uint32_t)0x40010400UL)

#define SYSCFG_BASE           ((uint32_t)0x40010000UL)

#define CRS_BASE              ((uint32_t)0x40006C00UL)

#define CRC_BASE              ((uint32_t)0x40023000UL)

#define UART0_BASE            ((uint32_t)0x40004400UL)
#define UART1_BASE            ((uint32_t)0x40005800UL)

#define PWR_BASE              ((uint32_t)0x40007000UL)

#define ANCTL_BASE            ((uint32_t)0x40020000UL)
#define RCC_BASE              ((uint32_t)0x40021000UL)
#define FLASH_R_BASE          ((uint32_t)0x40022000UL)

#define USB_BASE              ((uint32_t)0x20008000UL)

#define DBGMCU_BASE           ((uint32_t)0x40015800UL)



/* ================================================================================ */
/* ================             Peripheral declaration             ================ */
/* ================================================================================ */
#define PCT0                      ((        PCT_TypeDef *)         PCT0_BASE)
#define PCT1                      ((        PCT_TypeDef *)         PCT1_BASE)
#define PCT2                      ((        PCT_TypeDef *)         PCT2_BASE)
#define PCT3                      ((        PCT_TypeDef *)         PCT3_BASE)
#define PCT4                      ((        PCT_TypeDef *)         PCT4_BASE)

#define TIM6                      ((        TIM_TypeDef *)         TIM6_BASE)

#define RTC                       ((        RTC_TypeDef *)          RTC_BASE)

#define IWDG                      ((       IWDG_TypeDef *)         IWDG_BASE)

#define I2C                       ((        I2C_TypeDef *)          I2C_BASE)

#define SPIM                      ((       SPIM_TypeDef *)         SPIM_BASE)

#define SPI1                      ((        SPI_TypeDef *)         SPI1_BASE)

#define ADC                       ((        ADC_TypeDef *)          ADC_BASE)

#define ARGB0                     ((       ARGB_TypeDef *)        ARGB0_BASE)
#define ARGB1                     ((       ARGB_TypeDef *)        ARGB1_BASE)
#define ARGB2                     ((       ARGB_TypeDef *)        ARGB2_BASE)
#define ARGB3                     ((       ARGB_TypeDef *)        ARGB3_BASE)

#define ARGB4                     ((       ARGB_TypeDef *)        ARGB4_BASE)
#define ARGB5                     ((       ARGB_TypeDef *)        ARGB5_BASE)

#define GPIOA                     ((       GPIO_TypeDef *)        GPIOA_BASE)
#define GPIOB                     ((       GPIO_TypeDef *)        GPIOB_BASE)
#define GPIOC                     ((       GPIO_TypeDef *)        GPIOC_BASE)
#define GPIOD                     ((       GPIO_TypeDef *)        GPIOD_BASE)

#define EXTI                      ((       EXTI_TypeDef *)         EXTI_BASE)

#define SYSCFG                    ((     SYSCFG_TypeDef *)       SYSCFG_BASE)

#define CRS                       ((        CRS_TypeDef *)          CRS_BASE)

#define CRC                       ((        CRC_TypeDef *)          CRC_BASE)
#define UART0                     ((      UART0_TypeDef *)        UART0_BASE)
#define UART1                     ((      UART1_TypeDef *)        UART1_BASE)

#define PWR                       ((        PWR_TypeDef *)          PWR_BASE)

#define ANCTL                     ((      ANCTL_TypeDef *)        ANCTL_BASE)
#define RCC                       ((        RCC_TypeDef *)          RCC_BASE)
#define FLASH                     ((      FLASH_TypeDef *)      FLASH_R_BASE)

#define USB                       ((        USB_TypeDef *)          USB_BASE)

#define DBGMCU                    ((     DBGMCU_TypeDef *)       DBGMCU_BASE)





/*------------------------------------------------------------------------------------------------------*/
/*---                                  General-purpose I/Os (GPIO)                                   ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for GPIO_MODER register  ******************************/
#define GPIO_MODER_MODER0_Pos         (0U)
#define GPIO_MODER_MODER0_Msk         (0x3U << GPIO_MODER_MODER0_Pos)
#define GPIO_MODER_MODER1_Pos         (2U)
#define GPIO_MODER_MODER1_Msk         (0x3U << GPIO_MODER_MODER1_Pos)
#define GPIO_MODER_MODER2_Pos         (4U)
#define GPIO_MODER_MODER2_Msk         (0x3U << GPIO_MODER_MODER2_Pos)
#define GPIO_MODER_MODER3_Pos         (6U)
#define GPIO_MODER_MODER3_Msk         (0x3U << GPIO_MODER_MODER3_Pos)
#define GPIO_MODER_MODER4_Pos         (8U)
#define GPIO_MODER_MODER4_Msk         (0x3U << GPIO_MODER_MODER4_Pos)
#define GPIO_MODER_MODER5_Pos         (10U)
#define GPIO_MODER_MODER5_Msk         (0x3U << GPIO_MODER_MODER5_Pos)
#define GPIO_MODER_MODER6_Pos         (12U)
#define GPIO_MODER_MODER6_Msk         (0x3U << GPIO_MODER_MODER6_Pos)
#define GPIO_MODER_MODER7_Pos         (14U)
#define GPIO_MODER_MODER7_Msk         (0x3U << GPIO_MODER_MODER7_Pos)
#define GPIO_MODER_MODER8_Pos         (16U)
#define GPIO_MODER_MODER8_Msk         (0x3U << GPIO_MODER_MODER8_Pos)
#define GPIO_MODER_MODER9_Pos         (18U)
#define GPIO_MODER_MODER9_Msk         (0x3U << GPIO_MODER_MODER9_Pos)
#define GPIO_MODER_MODER10_Pos        (20U)
#define GPIO_MODER_MODER10_Msk        (0x3U << GPIO_MODER_MODER10_Pos)
#define GPIO_MODER_MODER11_Pos        (22U)
#define GPIO_MODER_MODER11_Msk        (0x3U << GPIO_MODER_MODER11_Pos)
#define GPIO_MODER_MODER12_Pos        (24U)
#define GPIO_MODER_MODER12_Msk        (0x3U << GPIO_MODER_MODER12_Pos)
#define GPIO_MODER_MODER13_Pos        (26U)
#define GPIO_MODER_MODER13_Msk        (0x3U << GPIO_MODER_MODER13_Pos)
#define GPIO_MODER_MODER14_Pos        (28U)
#define GPIO_MODER_MODER14_Msk        (0x3U << GPIO_MODER_MODER14_Pos)
#define GPIO_MODER_MODER15_Pos        (30U)
#define GPIO_MODER_MODER15_Msk        (0x3U << GPIO_MODER_MODER15_Pos)

/********************************  Bit definition for GPIO_OTYPER register  ******************************/
#define GPIO_OTYPER_OT0               (0x1U << 0)
#define GPIO_OTYPER_OT1               (0x1U << 1)
#define GPIO_OTYPER_OT2               (0x1U << 2)
#define GPIO_OTYPER_OT3               (0x1U << 3)
#define GPIO_OTYPER_OT4               (0x1U << 4)
#define GPIO_OTYPER_OT5               (0x1U << 5)
#define GPIO_OTYPER_OT6               (0x1U << 6)
#define GPIO_OTYPER_OT7               (0x1U << 7)
#define GPIO_OTYPER_OT8               (0x1U << 8)
#define GPIO_OTYPER_OT9               (0x1U << 9)
#define GPIO_OTYPER_OT10              (0x1U << 10)
#define GPIO_OTYPER_OT11              (0x1U << 11)
#define GPIO_OTYPER_OT12              (0x1U << 12)
#define GPIO_OTYPER_OT13              (0x1U << 13)
#define GPIO_OTYPER_OT14              (0x1U << 14)
#define GPIO_OTYPER_OT15              (0x1U << 15)

/********************************  Bit definition for GPIO_OSPEEDR register  ******************************/
#define GPIO_OSPEEDER_OSPEEDR0        (0x1U << 0)
#define GPIO_OSPEEDER_OSPEEDR1        (0x1U << 1)
#define GPIO_OSPEEDER_OSPEEDR2        (0x1U << 2)
#define GPIO_OSPEEDER_OSPEEDR3        (0x1U << 3)
#define GPIO_OSPEEDER_OSPEEDR4        (0x1U << 4)
#define GPIO_OSPEEDER_OSPEEDR5        (0x1U << 5)
#define GPIO_OSPEEDER_OSPEEDR6        (0x1U << 6)
#define GPIO_OSPEEDER_OSPEEDR7        (0x1U << 7)
#define GPIO_OSPEEDER_OSPEEDR8        (0x1U << 8)
#define GPIO_OSPEEDER_OSPEEDR9        (0x1U << 9)
#define GPIO_OSPEEDER_OSPEEDR10       (0x1U << 10)
#define GPIO_OSPEEDER_OSPEEDR11       (0x1U << 11)
#define GPIO_OSPEEDER_OSPEEDR12       (0x1U << 12)
#define GPIO_OSPEEDER_OSPEEDR13       (0x1U << 13)
#define GPIO_OSPEEDER_OSPEEDR14       (0x1U << 14)
#define GPIO_OSPEEDER_OSPEEDR15       (0x1U << 15)

/********************************  Bit definition for GPIO_PUPDR register  ******************************/
#define GPIO_PUPDR_PUPDR0_Pos         (0U)
#define GPIO_PUPDR_PUPDR0_Msk         (0x3U << GPIO_PUPDR_PUPDR0_Pos)
#define GPIO_PUPDR_PUPDR1_Pos         (2U)
#define GPIO_PUPDR_PUPDR1_Msk         (0x3U << GPIO_PUPDR_PUPDR1_Pos)
#define GPIO_PUPDR_PUPDR2_Pos         (4U)
#define GPIO_PUPDR_PUPDR2_Msk         (0x3U << GPIO_PUPDR_PUPDR2_Pos)
#define GPIO_PUPDR_PUPDR3_Pos         (6U)
#define GPIO_PUPDR_PUPDR3_Msk         (0x3U << GPIO_PUPDR_PUPDR3_Pos)
#define GPIO_PUPDR_PUPDR4_Pos         (8U)
#define GPIO_PUPDR_PUPDR4_Msk         (0x3U << GPIO_PUPDR_PUPDR4_Pos)
#define GPIO_PUPDR_PUPDR5_Pos         (10U)
#define GPIO_PUPDR_PUPDR5_Msk         (0x3U << GPIO_PUPDR_PUPDR5_Pos)
#define GPIO_PUPDR_PUPDR6_Pos         (12U)
#define GPIO_PUPDR_PUPDR6_Msk         (0x3U << GPIO_PUPDR_PUPDR6_Pos)
#define GPIO_PUPDR_PUPDR7_Pos         (14U)
#define GPIO_PUPDR_PUPDR7_Msk         (0x3U << GPIO_PUPDR_PUPDR7_Pos)
#define GPIO_PUPDR_PUPDR8_Pos         (16U)
#define GPIO_PUPDR_PUPDR8_Msk         (0x3U << GPIO_PUPDR_PUPDR8_Pos)
#define GPIO_PUPDR_PUPDR9_Pos         (18U)
#define GPIO_PUPDR_PUPDR9_Msk         (0x3U << GPIO_PUPDR_PUPDR9_Pos)
#define GPIO_PUPDR_PUPDR10_Pos        (20U)
#define GPIO_PUPDR_PUPDR10_Msk        (0x3U << GPIO_PUPDR_PUPDR10_Pos)
#define GPIO_PUPDR_PUPDR11_Pos        (22U)
#define GPIO_PUPDR_PUPDR11_Msk        (0x3U << GPIO_PUPDR_PUPDR11_Pos)
#define GPIO_PUPDR_PUPDR12_Pos        (24U)
#define GPIO_PUPDR_PUPDR12_Msk        (0x3U << GPIO_PUPDR_PUPDR12_Pos)
#define GPIO_PUPDR_PUPDR13_Pos        (26U)
#define GPIO_PUPDR_PUPDR13_Msk        (0x3U << GPIO_PUPDR_PUPDR13_Pos)
#define GPIO_PUPDR_PUPDR14_Pos        (28U)
#define GPIO_PUPDR_PUPDR14_Msk        (0x3U << GPIO_PUPDR_PUPDR14_Pos)
#define GPIO_PUPDR_PUPDR15_Pos        (30U)
#define GPIO_PUPDR_PUPDR15_Msk        (0x3U << GPIO_PUPDR_PUPDR15_Pos)

/********************************  Bit definition for GPIO_IDR register  ******************************/
#define GPIO_IDR_IDR0                 (0x1U << 0)        /*!< Port input data, bit 0 */
#define GPIO_IDR_IDR1                 (0x1U << 1)        /*!< Port input data, bit 1 */
#define GPIO_IDR_IDR2                 (0x1U << 2)        /*!< Port input data, bit 2 */
#define GPIO_IDR_IDR3                 (0x1U << 3)        /*!< Port input data, bit 3 */
#define GPIO_IDR_IDR4                 (0x1U << 4)        /*!< Port input data, bit 4 */
#define GPIO_IDR_IDR5                 (0x1U << 5)        /*!< Port input data, bit 5 */
#define GPIO_IDR_IDR6                 (0x1U << 6)        /*!< Port input data, bit 6 */
#define GPIO_IDR_IDR7                 (0x1U << 7)        /*!< Port input data, bit 7 */
#define GPIO_IDR_IDR8                 (0x1U << 8)        /*!< Port input data, bit 8 */
#define GPIO_IDR_IDR9                 (0x1U << 9)        /*!< Port input data, bit 9 */
#define GPIO_IDR_IDR10                (0x1U << 10)       /*!< Port input data, bit 10 */
#define GPIO_IDR_IDR11                (0x1U << 11)       /*!< Port input data, bit 11 */
#define GPIO_IDR_IDR12                (0x1U << 12)       /*!< Port input data, bit 12 */
#define GPIO_IDR_IDR13                (0x1U << 13)       /*!< Port input data, bit 13 */
#define GPIO_IDR_IDR14                (0x1U << 14)       /*!< Port input data, bit 14 */
#define GPIO_IDR_IDR15                (0x1U << 15)       /*!< Port input data, bit 15 */

/********************************  Bit definition for GPIO_ODR register  ******************************/
#define GPIO_ODR_ODR0                 (0x1U << 0)        /*!< Port output data, bit 0 */
#define GPIO_ODR_ODR1                 (0x1U << 1)        /*!< Port output data, bit 1 */
#define GPIO_ODR_ODR2                 (0x1U << 2)        /*!< Port output data, bit 2 */
#define GPIO_ODR_ODR3                 (0x1U << 3)        /*!< Port output data, bit 3 */
#define GPIO_ODR_ODR4                 (0x1U << 4)        /*!< Port output data, bit 4 */
#define GPIO_ODR_ODR5                 (0x1U << 5)        /*!< Port output data, bit 5 */
#define GPIO_ODR_ODR6                 (0x1U << 6)        /*!< Port output data, bit 6 */
#define GPIO_ODR_ODR7                 (0x1U << 7)        /*!< Port output data, bit 7 */
#define GPIO_ODR_ODR8                 (0x1U << 8)        /*!< Port output data, bit 8 */
#define GPIO_ODR_ODR9                 (0x1U << 9)        /*!< Port output data, bit 9 */
#define GPIO_ODR_ODR10                (0x1U << 10)       /*!< Port output data, bit 10 */
#define GPIO_ODR_ODR11                (0x1U << 11)       /*!< Port output data, bit 11 */
#define GPIO_ODR_ODR12                (0x1U << 12)       /*!< Port output data, bit 12 */
#define GPIO_ODR_ODR13                (0x1U << 13)       /*!< Port output data, bit 13 */
#define GPIO_ODR_ODR14                (0x1U << 14)       /*!< Port output data, bit 14 */
#define GPIO_ODR_ODR15                (0x1U << 15)       /*!< Port output data, bit 15 */

/********************************  Bit definition for GPIO_BSRR register  ******************************/
#define GPIO_BSRR_BS0                 (0x1U << 0)        /*!< Port x Set bit 0 */
#define GPIO_BSRR_BS1                 (0x1U << 1)        /*!< Port x Set bit 1 */
#define GPIO_BSRR_BS2                 (0x1U << 2)        /*!< Port x Set bit 2 */
#define GPIO_BSRR_BS3                 (0x1U << 3)        /*!< Port x Set bit 3 */
#define GPIO_BSRR_BS4                 (0x1U << 4)        /*!< Port x Set bit 4 */
#define GPIO_BSRR_BS5                 (0x1U << 5)        /*!< Port x Set bit 5 */
#define GPIO_BSRR_BS6                 (0x1U << 6)        /*!< Port x Set bit 6 */
#define GPIO_BSRR_BS7                 (0x1U << 7)        /*!< Port x Set bit 7 */
#define GPIO_BSRR_BS8                 (0x1U << 8)        /*!< Port x Set bit 8 */
#define GPIO_BSRR_BS9                 (0x1U << 9)        /*!< Port x Set bit 9 */
#define GPIO_BSRR_BS10                (0x1U << 10)       /*!< Port x Set bit 10 */
#define GPIO_BSRR_BS11                (0x1U << 11)       /*!< Port x Set bit 11 */
#define GPIO_BSRR_BS12                (0x1U << 12)       /*!< Port x Set bit 12 */
#define GPIO_BSRR_BS13                (0x1U << 13)       /*!< Port x Set bit 13 */
#define GPIO_BSRR_BS14                (0x1U << 14)       /*!< Port x Set bit 14 */
#define GPIO_BSRR_BS15                (0x1U << 15)       /*!< Port x Set bit 15 */

#define GPIO_BSRR_BR0                 (0x1U << 16)       /*!< Port x Reset bit 0 */
#define GPIO_BSRR_BR1                 (0x1U << 17)       /*!< Port x Reset bit 1 */
#define GPIO_BSRR_BR2                 (0x1U << 18)       /*!< Port x Reset bit 2 */
#define GPIO_BSRR_BR3                 (0x1U << 19)       /*!< Port x Reset bit 3 */
#define GPIO_BSRR_BR4                 (0x1U << 20)       /*!< Port x Reset bit 4 */
#define GPIO_BSRR_BR5                 (0x1U << 21)       /*!< Port x Reset bit 5 */
#define GPIO_BSRR_BR6                 (0x1U << 22)       /*!< Port x Reset bit 6 */
#define GPIO_BSRR_BR7                 (0x1U << 23)       /*!< Port x Reset bit 7 */
#define GPIO_BSRR_BR8                 (0x1U << 24)       /*!< Port x Reset bit 8 */
#define GPIO_BSRR_BR9                 (0x1U << 25)       /*!< Port x Reset bit 9 */
#define GPIO_BSRR_BR10                (0x1U << 26)       /*!< Port x Reset bit 10 */
#define GPIO_BSRR_BR11                (0x1U << 27)       /*!< Port x Reset bit 11 */
#define GPIO_BSRR_BR12                (0x1U << 28)       /*!< Port x Reset bit 12 */
#define GPIO_BSRR_BR13                (0x1U << 29)       /*!< Port x Reset bit 13 */
#define GPIO_BSRR_BR14                (0x1U << 30)       /*!< Port x Reset bit 14 */
#define GPIO_BSRR_BR15                (0x1U << 31)       /*!< Port x Reset bit 15 */

/********************************  Bit definition for GPIO_LCKR register  *******************************/
#define GPIO_LCKR_LCK0                (0x1U << 0)        /*!< Port x Lock bit 0 */
#define GPIO_LCKR_LCK1                (0x1U << 1)        /*!< Port x Lock bit 1 */
#define GPIO_LCKR_LCK2                (0x1U << 2)        /*!< Port x Lock bit 2 */
#define GPIO_LCKR_LCK3                (0x1U << 3)        /*!< Port x Lock bit 3 */
#define GPIO_LCKR_LCK4                (0x1U << 4)        /*!< Port x Lock bit 4 */
#define GPIO_LCKR_LCK5                (0x1U << 5)        /*!< Port x Lock bit 5 */
#define GPIO_LCKR_LCK6                (0x1U << 6)        /*!< Port x Lock bit 6 */
#define GPIO_LCKR_LCK7                (0x1U << 7)        /*!< Port x Lock bit 7 */
#define GPIO_LCKR_LCK8                (0x1U << 8)        /*!< Port x Lock bit 8 */
#define GPIO_LCKR_LCK9                (0x1U << 9)        /*!< Port x Lock bit 9 */
#define GPIO_LCKR_LCK10               (0x1U << 10)       /*!< Port x Lock bit 10 */
#define GPIO_LCKR_LCK11               (0x1U << 11)       /*!< Port x Lock bit 11 */
#define GPIO_LCKR_LCK12               (0x1U << 12)       /*!< Port x Lock bit 12 */
#define GPIO_LCKR_LCK13               (0x1U << 13)       /*!< Port x Lock bit 13 */
#define GPIO_LCKR_LCK14               (0x1U << 14)       /*!< Port x Lock bit 14 */
#define GPIO_LCKR_LCK15               (0x1U << 15)       /*!< Port x Lock bit 15 */
#define GPIO_LCKR_LCKK                (0x1U << 16)       /*!< Lock key */

/********************************  Bit definition for GPIO_AFRL register  *******************************/
#define GPIO_AFRL_AFR0_Pos            (0U)
#define GPIO_AFRL_AFR0_Msk            (0xFU << GPIO_AFRL_AFR0_Pos)
#define GPIO_AFRL_AFR1_Pos            (4U)
#define GPIO_AFRL_AFR1_Msk            (0xFU << GPIO_AFRL_AFR1_Pos)
#define GPIO_AFRL_AFR2_Pos            (8U)
#define GPIO_AFRL_AFR2_Msk            (0xFU << GPIO_AFRL_AFR2_Pos)
#define GPIO_AFRL_AFR3_Pos            (12U)
#define GPIO_AFRL_AFR3_Msk            (0xFU << GPIO_AFRL_AFR3_Pos)
#define GPIO_AFRL_AFR4_Pos            (16U)
#define GPIO_AFRL_AFR4_Msk            (0xFU << GPIO_AFRL_AFR4_Pos)
#define GPIO_AFRL_AFR5_Pos            (20U)
#define GPIO_AFRL_AFR5_Msk            (0xFU << GPIO_AFRL_AFR5_Pos)
#define GPIO_AFRL_AFR6_Pos            (24U)
#define GPIO_AFRL_AFR6_Msk            (0xFU << GPIO_AFRL_AFR6_Pos)
#define GPIO_AFRL_AFR7_Pos            (28U)
#define GPIO_AFRL_AFR7_Msk            (0xFU << GPIO_AFRL_AFR7_Pos)

/********************************  Bit definition for GPIO_AFRH register  *******************************/
#define GPIO_AFRH_AFR8_Pos            (0U)
#define GPIO_AFRH_AFR8_Msk            (0xFU << GPIO_AFRH_AFR8_Pos)
#define GPIO_AFRH_AFR9_Pos            (4U)
#define GPIO_AFRH_AFR9_Msk            (0xFU << GPIO_AFRH_AFR9_Pos)
#define GPIO_AFRH_AFR10_Pos           (8U)
#define GPIO_AFRH_AFR10_Msk           (0xFU << GPIO_AFRH_AFR10_Pos)
#define GPIO_AFRH_AFR11_Pos           (12U)
#define GPIO_AFRH_AFR11_Msk           (0xFU << GPIO_AFRH_AFR11_Pos)
#define GPIO_AFRH_AFR12_Pos           (16U)
#define GPIO_AFRH_AFR12_Msk           (0xFU << GPIO_AFRH_AFR12_Pos)
#define GPIO_AFRH_AFR13_Pos           (20U)
#define GPIO_AFRH_AFR13_Msk           (0xFU << GPIO_AFRH_AFR13_Pos)
#define GPIO_AFRH_AFR14_Pos           (24U)
#define GPIO_AFRH_AFR14_Msk           (0xFU << GPIO_AFRH_AFR14_Pos)
#define GPIO_AFRH_AFR15_Pos           (28U)
#define GPIO_AFRH_AFR15_Msk           (0xFU << GPIO_AFRH_AFR15_Pos)

/********************************  Bit definition for GPIO_BRR register  ******************************/
#define GPIO_BRR_BR0                  (0x1U << 0)         /*!< Port x Reset bit 0 */
#define GPIO_BRR_BR1                  (0x1U << 1)         /*!< Port x Reset bit 1 */
#define GPIO_BRR_BR2                  (0x1U << 2)         /*!< Port x Reset bit 2 */
#define GPIO_BRR_BR3                  (0x1U << 3)         /*!< Port x Reset bit 3 */
#define GPIO_BRR_BR4                  (0x1U << 4)         /*!< Port x Reset bit 4 */
#define GPIO_BRR_BR5                  (0x1U << 5)         /*!< Port x Reset bit 5 */
#define GPIO_BRR_BR6                  (0x1U << 6)         /*!< Port x Reset bit 6 */
#define GPIO_BRR_BR7                  (0x1U << 7)         /*!< Port x Reset bit 7 */
#define GPIO_BRR_BR8                  (0x1U << 8)         /*!< Port x Reset bit 8 */
#define GPIO_BRR_BR9                  (0x1U << 9)         /*!< Port x Reset bit 9 */
#define GPIO_BRR_BR10                 (0x1U << 10)        /*!< Port x Reset bit 10 */
#define GPIO_BRR_BR11                 (0x1U << 11)        /*!< Port x Reset bit 11 */
#define GPIO_BRR_BR12                 (0x1U << 12)        /*!< Port x Reset bit 12 */
#define GPIO_BRR_BR13                 (0x1U << 13)        /*!< Port x Reset bit 13 */
#define GPIO_BRR_BR14                 (0x1U << 14)        /*!< Port x Reset bit 14 */
#define GPIO_BRR_BR15                 (0x1U << 15)        /*!< Port x Reset bit 15 */

/********************************  Bit definition for GPIO_CSR register  ******************************/
#define GPIO_CSR_CS0                  (0x1U << 0)
#define GPIO_CSR_CS1                  (0x1U << 1)
#define GPIO_CSR_CS2                  (0x1U << 2)
#define GPIO_CSR_CS3                  (0x1U << 3)
#define GPIO_CSR_CS4                  (0x1U << 4)
#define GPIO_CSR_CS5                  (0x1U << 5)
#define GPIO_CSR_CS6                  (0x1U << 6)
#define GPIO_CSR_CS7                  (0x1U << 7)
#define GPIO_CSR_CS8                  (0x1U << 8)
#define GPIO_CSR_CS9                  (0x1U << 9)
#define GPIO_CSR_CS10                 (0x1U << 10)
#define GPIO_CSR_CS11                 (0x1U << 11)
#define GPIO_CSR_CS12                 (0x1U << 12)
#define GPIO_CSR_CS13                 (0x1U << 13)
#define GPIO_CSR_CS14                 (0x1U << 14)
#define GPIO_CSR_CS15                 (0x1U << 15)

/********************************  Bit definition for GPIO_DRR register  ******************************/
#define GPIO_DRR_DR0                  (0x1U << 0)
#define GPIO_DRR_DR1                  (0x1U << 1)
#define GPIO_DRR_DR2                  (0x1U << 2)
#define GPIO_DRR_DR3                  (0x1U << 3)
#define GPIO_DRR_DR4                  (0x1U << 4)
#define GPIO_DRR_DR5                  (0x1U << 5)
#define GPIO_DRR_DR6                  (0x1U << 6)
#define GPIO_DRR_DR7                  (0x1U << 7)
#define GPIO_DRR_DR8                  (0x1U << 8)
#define GPIO_DRR_DR9                  (0x1U << 9)
#define GPIO_DRR_DR10                 (0x1U << 10)
#define GPIO_DRR_DR11                 (0x1U << 11)
#define GPIO_DRR_DR12                 (0x1U << 12)
#define GPIO_DRR_DR13                 (0x1U << 13)
#define GPIO_DRR_DR14                 (0x1U << 14)
#define GPIO_DRR_DR15                 (0x1U << 15)




/*------------------------------------------------------------------------------------------------------*/
/*---                           External Interrupt/Event Controller (EXTI)                           ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for EXTI_IMR register  ********************************/
#define  EXTI_IMR_IM0               ((uint32_t)0x00000001)        /*!< Interrupt Mask on line 0 */
#define  EXTI_IMR_IM1               ((uint32_t)0x00000002)        /*!< Interrupt Mask on line 1 */
#define  EXTI_IMR_IM2               ((uint32_t)0x00000004)        /*!< Interrupt Mask on line 2 */
#define  EXTI_IMR_IM3               ((uint32_t)0x00000008)        /*!< Interrupt Mask on line 3 */
#define  EXTI_IMR_IM4               ((uint32_t)0x00000010)        /*!< Interrupt Mask on line 4 */
#define  EXTI_IMR_IM5               ((uint32_t)0x00000020)        /*!< Interrupt Mask on line 5 */
#define  EXTI_IMR_IM6               ((uint32_t)0x00000040)        /*!< Interrupt Mask on line 6 */
#define  EXTI_IMR_IM7               ((uint32_t)0x00000080)        /*!< Interrupt Mask on line 7 */
#define  EXTI_IMR_IM8               ((uint32_t)0x00000100)        /*!< Interrupt Mask on line 8 */
#define  EXTI_IMR_IM9               ((uint32_t)0x00000200)        /*!< Interrupt Mask on line 9 */
#define  EXTI_IMR_IM10              ((uint32_t)0x00000400)        /*!< Interrupt Mask on line 10 */
#define  EXTI_IMR_IM11              ((uint32_t)0x00000800)        /*!< Interrupt Mask on line 11 */
#define  EXTI_IMR_IM12              ((uint32_t)0x00001000)        /*!< Interrupt Mask on line 12 */
#define  EXTI_IMR_IM13              ((uint32_t)0x00002000)        /*!< Interrupt Mask on line 13 */
#define  EXTI_IMR_IM14              ((uint32_t)0x00004000)        /*!< Interrupt Mask on line 14 */
#define  EXTI_IMR_IM15              ((uint32_t)0x00008000)        /*!< Interrupt Mask on line 15 */
#define  EXTI_IMR_IM16              ((uint32_t)0x00010000)        /*!< Interrupt Mask on line 16 */
#define  EXTI_IMR_IM17              ((uint32_t)0x00020000)        /*!< Interrupt Mask on line 17 */
#define  EXTI_IMR_IM18              ((uint32_t)0x00040000)        /*!< Interrupt Mask on line 18 */

/********************************  Bit definition for EXTI_EMR register  ********************************/
#define  EXTI_EMR_EM0               ((uint32_t)0x00000001)        /*!< Event Mask on line 0 */
#define  EXTI_EMR_EM1               ((uint32_t)0x00000002)        /*!< Event Mask on line 1 */
#define  EXTI_EMR_EM2               ((uint32_t)0x00000004)        /*!< Event Mask on line 2 */
#define  EXTI_EMR_EM3               ((uint32_t)0x00000008)        /*!< Event Mask on line 3 */
#define  EXTI_EMR_EM4               ((uint32_t)0x00000010)        /*!< Event Mask on line 4 */
#define  EXTI_EMR_EM5               ((uint32_t)0x00000020)        /*!< Event Mask on line 5 */
#define  EXTI_EMR_EM6               ((uint32_t)0x00000040)        /*!< Event Mask on line 6 */
#define  EXTI_EMR_EM7               ((uint32_t)0x00000080)        /*!< Event Mask on line 7 */
#define  EXTI_EMR_EM8               ((uint32_t)0x00000100)        /*!< Event Mask on line 8 */
#define  EXTI_EMR_EM9               ((uint32_t)0x00000200)        /*!< Event Mask on line 9 */
#define  EXTI_EMR_EM10              ((uint32_t)0x00000400)        /*!< Event Mask on line 10 */
#define  EXTI_EMR_EM11              ((uint32_t)0x00000800)        /*!< Event Mask on line 11 */
#define  EXTI_EMR_EM12              ((uint32_t)0x00001000)        /*!< Event Mask on line 12 */
#define  EXTI_EMR_EM13              ((uint32_t)0x00002000)        /*!< Event Mask on line 13 */
#define  EXTI_EMR_EM14              ((uint32_t)0x00004000)        /*!< Event Mask on line 14 */
#define  EXTI_EMR_EM15              ((uint32_t)0x00008000)        /*!< Event Mask on line 15 */
#define  EXTI_EMR_EM16              ((uint32_t)0x00010000)        /*!< Event Mask on line 16 */
#define  EXTI_EMR_EM17              ((uint32_t)0x00020000)        /*!< Event Mask on line 17 */
#define  EXTI_EMR_EM18              ((uint32_t)0x00040000)        /*!< Event Mask on line 18 */

/********************************  Bit definition for EXTI_RTSR register  ********************************/
#define  EXTI_RTSR_TR0              ((uint32_t)0x00000001)        /*!< Rising trigger event configuration bit of line 0 */
#define  EXTI_RTSR_TR1              ((uint32_t)0x00000002)        /*!< Rising trigger event configuration bit of line 1 */
#define  EXTI_RTSR_TR2              ((uint32_t)0x00000004)        /*!< Rising trigger event configuration bit of line 2 */
#define  EXTI_RTSR_TR3              ((uint32_t)0x00000008)        /*!< Rising trigger event configuration bit of line 3 */
#define  EXTI_RTSR_TR4              ((uint32_t)0x00000010)        /*!< Rising trigger event configuration bit of line 4 */
#define  EXTI_RTSR_TR5              ((uint32_t)0x00000020)        /*!< Rising trigger event configuration bit of line 5 */
#define  EXTI_RTSR_TR6              ((uint32_t)0x00000040)        /*!< Rising trigger event configuration bit of line 6 */
#define  EXTI_RTSR_TR7              ((uint32_t)0x00000080)        /*!< Rising trigger event configuration bit of line 7 */
#define  EXTI_RTSR_TR8              ((uint32_t)0x00000100)        /*!< Rising trigger event configuration bit of line 8 */
#define  EXTI_RTSR_TR9              ((uint32_t)0x00000200)        /*!< Rising trigger event configuration bit of line 9 */
#define  EXTI_RTSR_TR10             ((uint32_t)0x00000400)        /*!< Rising trigger event configuration bit of line 10 */
#define  EXTI_RTSR_TR11             ((uint32_t)0x00000800)        /*!< Rising trigger event configuration bit of line 11 */
#define  EXTI_RTSR_TR12             ((uint32_t)0x00001000)        /*!< Rising trigger event configuration bit of line 12 */
#define  EXTI_RTSR_TR13             ((uint32_t)0x00002000)        /*!< Rising trigger event configuration bit of line 13 */
#define  EXTI_RTSR_TR14             ((uint32_t)0x00004000)        /*!< Rising trigger event configuration bit of line 14 */
#define  EXTI_RTSR_TR15             ((uint32_t)0x00008000)        /*!< Rising trigger event configuration bit of line 15 */
#define  EXTI_RTSR_TR16             ((uint32_t)0x00010000)        /*!< Rising trigger event configuration bit of line 16 */
#define  EXTI_RTSR_TR17             ((uint32_t)0x00020000)        /*!< Rising trigger event configuration bit of line 17 */
#define  EXTI_RTSR_TR18             ((uint32_t)0x00040000)        /*!< Rising trigger event configuration bit of line 18 */

/********************************  Bit definition for EXTI_FTSR register  ********************************/
#define  EXTI_FTSR_TR0              ((uint32_t)0x00000001)        /*!< Falling trigger event configuration bit of line 0 */
#define  EXTI_FTSR_TR1              ((uint32_t)0x00000002)        /*!< Falling trigger event configuration bit of line 1 */
#define  EXTI_FTSR_TR2              ((uint32_t)0x00000004)        /*!< Falling trigger event configuration bit of line 2 */
#define  EXTI_FTSR_TR3              ((uint32_t)0x00000008)        /*!< Falling trigger event configuration bit of line 3 */
#define  EXTI_FTSR_TR4              ((uint32_t)0x00000010)        /*!< Falling trigger event configuration bit of line 4 */
#define  EXTI_FTSR_TR5              ((uint32_t)0x00000020)        /*!< Falling trigger event configuration bit of line 5 */
#define  EXTI_FTSR_TR6              ((uint32_t)0x00000040)        /*!< Falling trigger event configuration bit of line 6 */
#define  EXTI_FTSR_TR7              ((uint32_t)0x00000080)        /*!< Falling trigger event configuration bit of line 7 */
#define  EXTI_FTSR_TR8              ((uint32_t)0x00000100)        /*!< Falling trigger event configuration bit of line 8 */
#define  EXTI_FTSR_TR9              ((uint32_t)0x00000200)        /*!< Falling trigger event configuration bit of line 9 */
#define  EXTI_FTSR_TR10             ((uint32_t)0x00000400)        /*!< Falling trigger event configuration bit of line 10 */
#define  EXTI_FTSR_TR11             ((uint32_t)0x00000800)        /*!< Falling trigger event configuration bit of line 11 */
#define  EXTI_FTSR_TR12             ((uint32_t)0x00001000)        /*!< Falling trigger event configuration bit of line 12 */
#define  EXTI_FTSR_TR13             ((uint32_t)0x00002000)        /*!< Falling trigger event configuration bit of line 13 */
#define  EXTI_FTSR_TR14             ((uint32_t)0x00004000)        /*!< Falling trigger event configuration bit of line 14 */
#define  EXTI_FTSR_TR15             ((uint32_t)0x00008000)        /*!< Falling trigger event configuration bit of line 15 */
#define  EXTI_FTSR_TR16             ((uint32_t)0x00010000)        /*!< Falling trigger event configuration bit of line 16 */
#define  EXTI_FTSR_TR17             ((uint32_t)0x00020000)        /*!< Falling trigger event configuration bit of line 17 */
#define  EXTI_FTSR_TR18             ((uint32_t)0x00040000)        /*!< Falling trigger event configuration bit of line 18 */

/********************************  Bit definition for EXTI_SWIER register  ********************************/
#define  EXTI_SWIER_SWIER0          ((uint32_t)0x00000001)        /*!< Software Interrupt on line 0 */
#define  EXTI_SWIER_SWIER1          ((uint32_t)0x00000002)        /*!< Software Interrupt on line 1 */
#define  EXTI_SWIER_SWIER2          ((uint32_t)0x00000004)        /*!< Software Interrupt on line 2 */
#define  EXTI_SWIER_SWIER3          ((uint32_t)0x00000008)        /*!< Software Interrupt on line 3 */
#define  EXTI_SWIER_SWIER4          ((uint32_t)0x00000010)        /*!< Software Interrupt on line 4 */
#define  EXTI_SWIER_SWIER5          ((uint32_t)0x00000020)        /*!< Software Interrupt on line 5 */
#define  EXTI_SWIER_SWIER6          ((uint32_t)0x00000040)        /*!< Software Interrupt on line 6 */
#define  EXTI_SWIER_SWIER7          ((uint32_t)0x00000080)        /*!< Software Interrupt on line 7 */
#define  EXTI_SWIER_SWIER8          ((uint32_t)0x00000100)        /*!< Software Interrupt on line 8 */
#define  EXTI_SWIER_SWIER9          ((uint32_t)0x00000200)        /*!< Software Interrupt on line 9 */
#define  EXTI_SWIER_SWIER10         ((uint32_t)0x00000400)        /*!< Software Interrupt on line 10 */
#define  EXTI_SWIER_SWIER11         ((uint32_t)0x00000800)        /*!< Software Interrupt on line 11 */
#define  EXTI_SWIER_SWIER12         ((uint32_t)0x00001000)        /*!< Software Interrupt on line 12 */
#define  EXTI_SWIER_SWIER13         ((uint32_t)0x00002000)        /*!< Software Interrupt on line 13 */
#define  EXTI_SWIER_SWIER14         ((uint32_t)0x00004000)        /*!< Software Interrupt on line 14 */
#define  EXTI_SWIER_SWIER15         ((uint32_t)0x00008000)        /*!< Software Interrupt on line 15 */
#define  EXTI_SWIER_SWIER16         ((uint32_t)0x00010000)        /*!< Software Interrupt on line 16 */
#define  EXTI_SWIER_SWIER17         ((uint32_t)0x00020000)        /*!< Software Interrupt on line 17 */
#define  EXTI_SWIER_SWIER18         ((uint32_t)0x00040000)        /*!< Software Interrupt on line 18 */

/********************************  Bit definition for EXTI_PR register  ********************************/
#define  EXTI_PR_PR0                ((uint32_t)0x00000001)        /*!< Pending bit for line 0 */
#define  EXTI_PR_PR1                ((uint32_t)0x00000002)        /*!< Pending bit for line 1 */
#define  EXTI_PR_PR2                ((uint32_t)0x00000004)        /*!< Pending bit for line 2 */
#define  EXTI_PR_PR3                ((uint32_t)0x00000008)        /*!< Pending bit for line 3 */
#define  EXTI_PR_PR4                ((uint32_t)0x00000010)        /*!< Pending bit for line 4 */
#define  EXTI_PR_PR5                ((uint32_t)0x00000020)        /*!< Pending bit for line 5 */
#define  EXTI_PR_PR6                ((uint32_t)0x00000040)        /*!< Pending bit for line 6 */
#define  EXTI_PR_PR7                ((uint32_t)0x00000080)        /*!< Pending bit for line 7 */
#define  EXTI_PR_PR8                ((uint32_t)0x00000100)        /*!< Pending bit for line 8 */
#define  EXTI_PR_PR9                ((uint32_t)0x00000200)        /*!< Pending bit for line 9 */
#define  EXTI_PR_PR10               ((uint32_t)0x00000400)        /*!< Pending bit for line 10 */
#define  EXTI_PR_PR11               ((uint32_t)0x00000800)        /*!< Pending bit for line 11 */
#define  EXTI_PR_PR12               ((uint32_t)0x00001000)        /*!< Pending bit for line 12 */
#define  EXTI_PR_PR13               ((uint32_t)0x00002000)        /*!< Pending bit for line 13 */
#define  EXTI_PR_PR14               ((uint32_t)0x00004000)        /*!< Pending bit for line 14 */
#define  EXTI_PR_PR15               ((uint32_t)0x00008000)        /*!< Pending bit for line 15 */
#define  EXTI_PR_PR16               ((uint32_t)0x00010000)        /*!< Pending bit for line 16 */
#define  EXTI_PR_PR17               ((uint32_t)0x00020000)        /*!< Pending bit for line 17 */
#define  EXTI_PR_PR18               ((uint32_t)0x00040000)        /*!< Pending bit for line 18 */



/*------------------------------------------------------------------------------------------------------*/
/*---                                 System Configuration (SYSCFG)                                  ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for SYSCFG_CFGR1 register  ********************************/
#define SYSCFG_CFGR1_MEM_MODE_Pos       (0U)
#define SYSCFG_CFGR1_MEM_MODE_Msk       (0x3U << SYSCFG_CFGR1_MEM_MODE_Pos)

#define SYSCFG_CFGR1_SETUP              (0x1U << 2)

/******************************** Bit definition for SYSCFG_EXTICR1 register  ********************************/
#define SYSCFG_EXTICR1_EXTI0_Pos        (0U)
#define SYSCFG_EXTICR1_EXTI0_Msk        (0xFU << SYSCFG_EXTICR1_EXTI0_Pos)

#define SYSCFG_EXTICR1_EXTI1_Pos        (4U)
#define SYSCFG_EXTICR1_EXTI1_Msk        (0xFU << SYSCFG_EXTICR1_EXTI1_Pos)

#define SYSCFG_EXTICR1_EXTI2_Pos        (8U)
#define SYSCFG_EXTICR1_EXTI2_Msk        (0xFU << SYSCFG_EXTICR1_EXTI2_Pos)

#define SYSCFG_EXTICR1_EXTI3_Pos        (12U)
#define SYSCFG_EXTICR1_EXTI3_Msk        (0xFU << SYSCFG_EXTICR1_EXTI3_Pos)

/******************************** Bit definition for SYSCFG_EXTICR2 register  ********************************/
#define SYSCFG_EXTICR2_EXTI4_Pos        (0U)
#define SYSCFG_EXTICR2_EXTI4_Msk        (0xFU << SYSCFG_EXTICR2_EXTI4_Pos)

#define SYSCFG_EXTICR2_EXTI5_Pos        (4U)
#define SYSCFG_EXTICR2_EXTI5_Msk        (0xFU << SYSCFG_EXTICR2_EXTI5_Pos)

#define SYSCFG_EXTICR2_EXTI6_Pos        (8U)
#define SYSCFG_EXTICR2_EXTI6_Msk        (0xFU << SYSCFG_EXTICR2_EXTI6_Pos)

#define SYSCFG_EXTICR2_EXTI7_Pos        (12U)
#define SYSCFG_EXTICR2_EXTI7_Msk        (0xFU << SYSCFG_EXTICR2_EXTI7_Pos)

/******************************** Bit definition for SYSCFG_EXTICR3 register  ********************************/
#define SYSCFG_EXTICR3_EXTI8_Pos        (0U)
#define SYSCFG_EXTICR3_EXTI8_Msk        (0xFU << SYSCFG_EXTICR3_EXTI8_Pos)

#define SYSCFG_EXTICR3_EXTI9_Pos        (4U)
#define SYSCFG_EXTICR3_EXTI9_Msk        (0xFU << SYSCFG_EXTICR3_EXTI9_Pos)

#define SYSCFG_EXTICR3_EXTI10_Pos       (8U)
#define SYSCFG_EXTICR3_EXTI10_Msk       (0xFU << SYSCFG_EXTICR3_EXTI10_Pos)

#define SYSCFG_EXTICR3_EXTI11_Pos       (12U)
#define SYSCFG_EXTICR3_EXTI11_Msk       (0xFU << SYSCFG_EXTICR3_EXTI11_Pos)

/******************************** Bit definition for SYSCFG_EXTICR4 register  ********************************/
#define SYSCFG_EXTICR4_EXTI12_Pos       (0U)
#define SYSCFG_EXTICR4_EXTI12_Msk       (0xFU << SYSCFG_EXTICR4_EXTI12_Pos)

#define SYSCFG_EXTICR4_EXTI13_Pos       (4U)
#define SYSCFG_EXTICR4_EXTI13_Msk       (0xFU << SYSCFG_EXTICR4_EXTI13_Pos)

#define SYSCFG_EXTICR4_EXTI14_Pos       (8U)
#define SYSCFG_EXTICR4_EXTI14_Msk       (0xFU << SYSCFG_EXTICR4_EXTI14_Pos)

#define SYSCFG_EXTICR4_EXTI15_Pos       (12U)
#define SYSCFG_EXTICR4_EXTI15_Msk       (0xFU << SYSCFG_EXTICR4_EXTI15_Pos)

/******************************** Bit definition for SYSCFG_CFGR2 register  ********************************/
#define SYSCFG_CFGR2_PC0_OS_EN          (0x1U << 0)
#define SYSCFG_CFGR2_PC0_SINK_EN        (0x1U << 1)
#define SYSCFG_CFGR2_PC1_OS_EN          (0x1U << 2)
#define SYSCFG_CFGR2_PC1_SINK_EN        (0x1U << 3)
#define SYSCFG_CFGR2_PC2_OS_EN          (0x1U << 4)
#define SYSCFG_CFGR2_PC2_SINK_EN        (0x1U << 5)
#define SYSCFG_CFGR2_PC3_OS_EN          (0x1U << 6)
#define SYSCFG_CFGR2_PC3_SINK_EN        (0x1U << 7)
#define SYSCFG_CFGR2_PC4_OS_EN          (0x1U << 8)
#define SYSCFG_CFGR2_PC4_SINK_EN        (0x1U << 9)
#define SYSCFG_CFGR2_PC5_OS_EN          (0x1U << 10)
#define SYSCFG_CFGR2_PC5_SINK_EN        (0x1U << 11)

#define SYSCFG_CFGR2_PC0_CFG_EN         (0x1U << 12)
#define SYSCFG_CFGR2_PC1_CFG_EN         (0x1U << 13)
#define SYSCFG_CFGR2_PC2_CFG_EN         (0x1U << 14)
#define SYSCFG_CFGR2_PC3_CFG_EN         (0x1U << 15)
#define SYSCFG_CFGR2_PC4_CFG_EN         (0x1U << 16)
#define SYSCFG_CFGR2_PC5_CFG_EN         (0x1U << 17)

#define SYSCFG_CFGR2_DPPUEN             (0x1U << 18)

/******************************** Bit definition for SYSCFG_CFGR3 register  ********************************/
#define SYSCFG_CFGR3_CRS_TRIM_EN        (0x1U << 0)
#define SYSCFG_CFGR3_NRST_DIS           (0x1U << 1)

/******************************** Bit definition for SYSCFG_CFGR4 register  ********************************/
#define SYSCFG_CFGR4_PC0_DLY            (0x1U << 0)
#define SYSCFG_CFGR4_PC0_SINK_DLY       (0x1U << 1)
#define SYSCFG_CFGR4_PC1_DLY            (0x1U << 2)
#define SYSCFG_CFGR4_PC1_SINK_DLY       (0x1U << 3)
#define SYSCFG_CFGR4_PC2_DLY            (0x1U << 4)
#define SYSCFG_CFGR4_PC2_SINK_DLY       (0x1U << 5)
#define SYSCFG_CFGR4_PC3_DLY            (0x1U << 6)
#define SYSCFG_CFGR4_PC3_SINK_DLY       (0x1U << 7)
#define SYSCFG_CFGR4_PC4_DLY            (0x1U << 8)
#define SYSCFG_CFGR4_PC4_SINK_DLY       (0x1U << 9)
#define SYSCFG_CFGR4_PC5_DLY            (0x1U << 10)
#define SYSCFG_CFGR4_PC5_SINK_DLY       (0x1U << 11)



/*------------------------------------------------------------------------------------------------------*/
/*---                                  Clock Recovery System (CRS)                                   ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for CRS_CR register  ********************************/
#define CRS_CR_SYNCOKIE         (0x1U << 0)                       /*!< SYNC event OK interrupt enable */
#define CRS_CR_SYNCWARNIE       (0x1U << 1)                       /*!< SYNC warning interrupt enable */
#define CRS_CR_ERRIE            (0x1U << 2)                       /*!< SYNC error interrupt enable */
#define CRS_CR_ESYNCIE          (0x1U << 3)                       /*!< Expected SYNC(ESYNCF) interrupt Enable */
#define CRS_CR_CEN              (0x1U << 5)                       /*!< Frequency error counter enable */
#define CRS_CR_AUTOTRIMEN       (0x1U << 6)                       /*!< Automatic trimming enable */
#define CRS_CR_SWSYNC           (0x1U << 7)                       /*!< A Software SYNC event is generated */
#define CRS_CR_TRIM_Pos         (8U)
#define CRS_CR_TRIM_Msk         (0x3FFU << CRS_CR_TRIM_Pos)       /*!< HSI48 oscillator smooth trimming */

/******************************** Bit definition for CRS_CFGR register  ********************************/
#define CRS_CFGR_RELOAD_Pos     (0U)
#define CRS_CFGR_RELOAD_Msk     (0xFFFFU << CRS_CFGR_RELOAD_Pos)  /*!< Counter reload value */
#define CRS_CFGR_FELIM_Pos      (16U)
#define CRS_CFGR_FELIM_Msk      (0xFFU << CRS_CFGR_FELIM_Pos)     /*!< Frequency error limit */

#define CRS_CFGR_SYNCDIV_Pos    (24U)
#define CRS_CFGR_SYNCDIV_Msk    (0x7U << CRS_CFGR_SYNCDIV_Pos)    /*!< SYNC divider */

#define CRS_CFGR_SYNCDIV_1      (0x0U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_2      (0x1U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_4      (0x2U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_8      (0x3U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_16     (0x4U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_32     (0x5U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_64     (0x6U << CRS_CFGR_SYNCDIV_Pos)
#define CRS_CFGR_SYNCDIV_128    (0x7U << CRS_CFGR_SYNCDIV_Pos)

#define CRS_CFGR_SYNCSRC_Pos    (28U)
#define CRS_CFGR_SYNCSRC_Msk    (0x3U << CRS_CFGR_SYNCSRC_Pos)    /*!< SYNC signal source selection */

#define CRS_CFGR_SYNCSRC_GPIO   (0x0U << CRS_CFGR_SYNCSRC_Pos)
#define CRS_CFGR_SYNCSRC_LSI    (0x1U << CRS_CFGR_SYNCSRC_Pos)
#define CRS_CFGR_SYNCSRC_USB    (0x2U << CRS_CFGR_SYNCSRC_Pos)
#define CRS_CFGR_SYNCSRC_EXTI5  (0x3U << CRS_CFGR_SYNCSRC_Pos)

#define CRS_CFGR_SYNCPOL        (0x1U << 31)                      /*!< SYNC polarity selection */

/******************************** Bit definition for CRS_ISR register  ********************************/
#define CRS_ISR_SYNCOKF         (0x1U << 0)                       /*!< SYNC event OK flag */
#define CRS_ISR_SYNCWARNF       (0x1U << 1)                       /*!< SYNC warning */
#define CRS_ISR_ERRF            (0x1U << 2)                       /*!< SYNC error flag */
#define CRS_ISR_ESYNCF          (0x1U << 3)                       /*!< Expected SYNC flag */
#define CRS_ISR_SYNCERR         (0x1U << 8)                       /*!< SYNC error */
#define CRS_ISR_SYNCMISS        (0x1U << 9)                       /*!< SYNC missed */
#define CRS_ISR_TRIMOVF         (0x1U << 10)                      /*!< Trimming overflow or underflow */
#define CRS_ISR_FEDIR           (0x1U << 15)                      /*!< Frequency error direction */
#define CRS_ISR_FECAP_Pos       (16U)
#define CRS_ISR_FECAP_Msk       (0xFFFFU << CRS_ISR_FECAP_Pos)    /*!< Frequency error capture */

/******************************** Bit definition for CRS_ICR register  ********************************/
#define CRS_ICR_SYNCOKC         (0x1U << 0)                       /*!< SYNC event OK clear flag */
#define CRS_ICR_SYNCWARNC       (0x1U << 1)                       /*!< SYNC warning clear flag */
#define CRS_ICR_ERRC            (0x1U << 2)                       /*!< Error clear flag */
#define CRS_ICR_ESYNCC          (0x1U << 3)                       /*!< Expected SYNC clear flag */



/*------------------------------------------------------------------------------------------------------*/
/*---                                 Cyclic Redundancy Check (CRC)                                  ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for CRC_MODE register  ********************************/
#define CRC_MODE_CRC_POLY_Msk     (0x3U << 0)          /*!< CRC polynomial field mask bit */
#define CRC_MODE_CRC_POLY_CRC8    (0x0U << 0)          /*!< CRC-8 polynomial */
#define CRC_MODE_CRC_POLY_CCITT   (0x1U << 0)          /*!< CRC-CCITT polynomial */
#define CRC_MODE_CRC_POLY_CRC16   (0x2U << 0)          /*!< CRC-CRC16 polynomial */
#define CRC_MODE_CRC_POLY_CRC32   (0x3U << 0)          /*!< CRC-CRC32 polynomial */

#define CRC_MODE_BIT_RVS_WR       (0x1U << 2)          /*!< Select bit order for CRC_WR_DATA */
#define CRC_MODE_CMPL_WR          (0x1U << 3)          /*!< Select one's complement for CRC_WR_DATA */
#define CRC_MODE_BIT_RVS_SUM      (0x1U << 4)          /*!< Select bit order revers for CRC_SUM */
#define CRC_MODE_CMPL_SUM         (0x1U << 5)          /*!< Select one's complement for CRC_SUM */
#define CRC_MODE_SEED_OP          (0x1U << 6)          /*!< CRC seed option set */
#define CRC_MODE_SEED_SET         (0x1U << 7)          /*!< Load seed to CRC generator */


/*------------------------------------------------------------------------------------------------------*/
/*---                      Universal Asyncronous Receiver / Transmitter (UART0)                      ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for UART0_CR1 register  ********************************/
#define UART0_CR1_UE              (0x1U << 0)           /*!< UART0 Enable */
#define UART0_CR1_RE              (0x1U << 2)           /*!< Receiver Enable */
#define UART0_CR1_TE              (0x1U << 3)           /*!< Transmitter Enable */
#define UART0_CR1_IDLEIE          (0x1U << 4)           /*!< IDLE Interrupt Enable */
#define UART0_CR1_RXNEIE          (0x1U << 5)           /*!< RXNE Interrupt Enable */
#define UART0_CR1_TCIE            (0x1U << 6)           /*!< Transmission Complete Interrupt Enable */
#define UART0_CR1_TXEIE           (0x1U << 7)           /*!< TXE Interrupt Enable */
#define UART0_CR1_PEIE            (0x1U << 8)           /*!< PE Interrupt Enable */
#define UART0_CR1_PS              (0x1U << 9)           /*!< Parity Selection */
#define UART0_CR1_PCE             (0x1U << 10)          /*!< Parity Control Enable */
#define UART0_CR1_M               (0x1U << 12)          /*!< Word length */
#define UART0_CR1_OVER8           (0x1U << 15)          /*!< Oversampling by 8-bit or 16-bit mode */

/******************************** Bit definition for UART0_CR2 register  ********************************/
#define UART0_CR2_LBDL            (0x1U << 5)           /*!< LIN Break Detection Length */
#define UART0_CR2_LBDIE           (0x1U << 6)           /*!< LIN Break Detection Interrupt Enable */

#define UART0_CR2_STOP_Pos        (12U)
#define UART0_CR2_STOP_Msk        (0x3U << UART0_CR2_STOP_Pos)               /*!< STOP bits Msk bit */

#define UART0_CR2_LINEN           (0x1U << 14)          /*!< LIN mode enable */
#define UART0_CR2_RXINV           (0x1U << 16)          /*!< RX pin active level inversion */
#define UART0_CR2_TXINV           (0x1U << 17)          /*!< TX pin active level inversion */
#define UART0_CR2_DATAINV         (0x1U << 18)          /*!< Binary data inversion */
#define UART0_CR2_MSBFIRST        (0x1U << 19)          /*!< Most Significant Bit First */
#define UART0_CR2_ABREN           (0x1U << 20)          /*!< Auto Baud-Rate Enable*/

#define UART0_CR2_ABRMODE_Pos     (21U)                              
#define UART0_CR2_ABRMODE_Msk     (0x3U << UART0_CR2_ABRMODE_Pos)            /*!< Auto baud rate mode Msk bit*/

/********************************  Bit definition for UART0_CR3 register  *******************************/
#define UART0_CR3_EIE             (0x1U << 0)           /*!< Error Interrupt Enable */
#define UART0_CR3_OVRDIS          (0x1U << 12)          /*!< Overrun Disable */

/********************************  Bit definition for UART0_BRR register  *******************************/
#define UART0_BRR_DIV_LOW         (0xFU)                                     /*!< Fraction of UART0DIV */
#define UART0_BRR_DIV_HIGH_Pos    (4U)        
#define UART0_BRR_DIV_HIGH        (0xFFFU << UART0_BRR_DIV_HIGH_Pos)         /*!< Mantissa of UART0DIV Msk bit */

/********************************  Bit definition for UART0_RQR register  *******************************/
#define UART0_RQR_ABRRQ           (0x1U << 0)            /*!< Auto-Baud Rate Request */
#define UART0_RQR_SBKRQ           (0x1U << 1)            /*!< Send Break Request */
#define UART0_RQR_RXFRQ           (0x1U << 3)            /*!< Receive Data flush Request */

/********************************  Bit definition for UART0_ISR register  *******************************/
#define UART0_ISR_PE              (0x1U << 0)            /*!< Parity Error */
#define UART0_ISR_FE              (0x1U << 1)            /*!< Framing Error */
#define UART0_ISR_NE              (0x1U << 2)            /*!< Noise detected Flag */
#define UART0_ISR_ORE             (0x1U << 3)            /*!< OverRun Error */
#define UART0_ISR_IDLE            (0x1U << 4)            /*!< IDLE line detected */
#define UART0_ISR_RXNE            (0x1U << 5)            /*!< Read Data Register Not Empty */
#define UART0_ISR_TC              (0x1U << 6)            /*!< Transmission Complete */
#define UART0_ISR_TXE             (0x1U << 7)            /*!< Transmit Data Register Empty */
#define UART0_ISR_LBD             (0x1U << 8)            /*!< LIN Break Detection Flag */
#define UART0_ISR_ABRE            (0x1U << 14)           /*!< Auto-Baud Rate Error */
#define UART0_ISR_ABRF            (0x1U << 15)           /*!< Auto-Baud Rate Flag */
#define UART0_ISR_BUSY            (0x1U << 16)           /*!< Busy Flag */
#define UART0_ISR_SBKF            (0x1U << 18)           /*!< Send Break Flag */
#define UART0_ISR_TEACK           (0x1U << 21)           /*!< Transmit Enable Acknowledge Flag */
#define UART0_ISR_REACK           (0x1U << 22)           /*!< Receive Enable Acknowledge Flag */

/********************************  Bit definition for UART0_ICR register  *******************************/
#define UART0_ICR_PECF            (0x1U << 0)            /*!< Parity Error Clear Flag */
#define UART0_ICR_FECF            (0x1U << 1)            /*!< Framing Error Clear Flag */
#define UART0_ICR_NCF             (0x1U << 2)            /*!< Noise detected Clear Flag */
#define UART0_ICR_ORECF           (0x1U << 3)            /*!< OverRun Error Clear Flag */
#define UART0_ICR_IDLECF          (0x1U << 4)            /*!< IDLE line detected Clear Flag */
#define UART0_ICR_TCCF            (0x1U << 6)            /*!< Transmission Complete Clear Flag */
#define UART0_ICR_LBDCF           (0x1U << 8)            /*!< LIN Break Detection Clear Flag */

/********************************  Bit definition for UART0_RDR register  *******************************/
#define UART0_RDR_RDR             (0x1FFU)               /*!< RDR[8:0] bits (Receive Data value) */

/********************************  Bit definition for UART0_TDR register  *******************************/
#define UART0_TDR_TDR             (0x1FFU)               /*!< TDR[8:0] bits (Transmit Data value) */


/*------------------------------------------------------------------------------------------------------*/
/*---                      Universal Asyncronous Receiver / Transmitter (UART1)                      ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for UART1_SR register  ********************************/
#define UART1_SR_TXF            (0x1U << 0)           /*!< Transmit buffer full */
#define UART1_SR_RXF            (0x1U << 1)           /*!< Receive buffer full */
#define UART1_SR_TXO            (0x1U << 2)           /*!< Transmit buffer overrun */
#define UART1_SR_RXO            (0x1U << 3)           /*!< Receive buffer overrun */

/******************************** Bit definition for UART1_CR register  ********************************/
#define UART1_CR_TXE            (0x1U << 0)           /*!< Transmitter enable */
#define UART1_CR_RXE            (0x1U << 1)           /*!< Receiver enable */
#define UART1_CR_TXIE           (0x1U << 2)           /*!< Transmit interrupt enable */
#define UART1_CR_RXIE           (0x1U << 3)           /*!< Receive interrupt enable */
#define UART1_CR_TXOIE          (0x1U << 4)           /*!< Transmit buffer overrun interrupt enable */
#define UART1_CR_RXOIE          (0x1U << 5)           /*!< Receive buffer overrun interrupt enable */

/******************************** Bit definition for UART1_ISR register  ********************************/
#define UART1_ISR_TXIF          (0x1U << 0)           /*!< Transmit interrupt flag */
#define UART1_ISR_RXIF          (0x1U << 1)           /*!< Receive interrupt flag */
#define UART1_ISR_TXOIF         (0x1U << 2)           /*!< Transmit buffer overrun interrupt flag */
#define UART1_ISR_RXOIF         (0x1U << 3)           /*!< Receive buffer overrun interrupt flag */

/******************************** Bit definition for UART1_ABRCON register  ********************************/
#define UART1_ABRCON_ABREN           (0x1U << 0)          /*!< Auto Baud Rate Detect enable */
#define UART1_ABRCON_ABRUPEN         (0x1U << 1)          /*!< Auto Baud Rate Update enable */
#define UART1_ABRCON_LPWIDTH_Pos     (2U)
#define UART1_ABRCON_LPWIDTH_Msk     (0x3U << UART1_ABRCON_LPWIDTH_Pos)
#define UART1_ABRCON_LPWIDTH_1BIT    (0x0U << UART1_ABRCON_LPWIDTH_Pos)
#define UART1_ABRCON_LPWIDTH_2BIT    (0x1U << UART1_ABRCON_LPWIDTH_Pos)
#define UART1_ABRCON_LPWIDTH_4BIT    (0x2U << UART1_ABRCON_LPWIDTH_Pos)
#define UART1_ABRCON_LPWIDTH_8BIT    (0x3U << UART1_ABRCON_LPWIDTH_Pos)

#define UART1_ABRCON_CHWIDTH_Pos     (4U)
#define UART1_ABRCON_CHWIDTH_Msk     (0xFU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_5BIT    (0x5U << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_6BIT    (0x6U << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_7BIT    (0x7U << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_8BIT    (0x8U << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_9BIT    (0x9U << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_10BIT   (0xAU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_11BIT   (0xBU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_12BIT   (0xCU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_13BIT   (0xDU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_14BIT   (0xEU << UART1_ABRCON_CHWIDTH_Pos)
#define UART1_ABRCON_CHWIDTH_15BIT   (0xFU << UART1_ABRCON_CHWIDTH_Pos)

#define UART1_ABRCON_RXINABREN       (0x1U << 8)          /*!< Enable UART to receive the character during auto baud rate detection */

/******************************** Bit definition for UART1_ABRSR register  ********************************/
#define UART1_ABRSR_ABRD             (0x1U << 0)          /*!< Auto Baud Rate dectection done flag */
#define UART1_ABRSR_ABROV            (0x1U << 1)          /*!< Auto Baud Rate overflow flag */

/******************************** Bit definition for UART1_ABRIE register  ********************************/
#define UART1_ABRIE_ABRDIE           (0x1U << 0)          /*!< Auto Baud Rate dectection done interrupt enable */
#define UART1_ABRIE_ABROVIE          (0x1U << 1)          /*!< Auto Baud Rate overflow interrupt enable */

/******************************** Bit definition for UART1_ABRIC register  ********************************/
#define UART1_ABRIC_ABRDIC           (0x1U << 0)          /*!< Auto Baud Rate dectection done interrupt clear */
#define UART1_ABRIC_ABROVIC          (0x1U << 1)          /*!< Auto Baud Rate overflow interrupt clear */



/*------------------------------------------------------------------------------------------------------*/
/*---                                    FLASH Registers (FLASH)                                     ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************    Bit definition for FLASH_ACR register *******************************/
#define FLASH_ACR_LATENCY               (0x1U << 0)         /*!< LATENCY bit (Latency) */
#define FLASH_ACR_PRFTBE                (0x1U << 4)         /*!< Prefetch Buffer Enable */
#define FLASH_ACR_PRFTBS                (0x1U << 5)         /*!< Prefetch Buffer Status */

/******************************    Bit definition for FLASH_CR register *******************************/
#define FLASH_CR_CMD_Msk                (0x3U << 0)         /*!< CMD field mask bit */

#define FLASH_CR_CMD_PG                 (0x1U << 0)         /*!< CMD Word Program */
#define FLASH_CR_CMD_PER                (0x2U << 0)         /*!< CMD Page Erase */
#define FLASH_CR_CMD_MER                (0x3U << 0)         /*!< CMD Mass Erase */

#define FLASH_CR_START                  (0x1U << 2)         /*!< Start */
#define FLASH_CR_LOCK                   (0x1U << 3)         /*!< Lock */
#define FLASH_CR_EOPIE                  (0x1U << 4)         /*!< End of operation interrupt enable */
#define FLASH_CR_ERRIE                  (0x1U << 5)         /*!< Error Interrupt Enable */

/******************************    Bit definition for FLASH_SR register *******************************/
#define FLASH_SR_BSY                    (0x1U << 0)         /*!< Busy */
#define FLASH_SR_EOP                    (0x1U << 1)         /*!< End of operation */
#define FLASH_SR_PGERR                  (0x1U << 2)         /*!< Programming Error */

/******************************    Bit definition for FLASH_OBR1 register *******************************/
#define FLASH_OBR1_RDP_Msk              (0x3U << 0)         /*!< Read protection Level field mask bit */
#define FLASH_OBR1_IWDG_SW              (0x1U << 16)        /*!< IWDG SW */

/******************************    Bit definition for FLASH_OBR2 register *******************************/
#define FLASH_OBR2_nRST_STOP            (0x1U << 0)         /*!< nRST_STOP */
#define FLASH_OBR2_nBOOT0               (0x1U << 16)        /*!< nBOOT0 */
#define FLASH_OBR2_nBOOT1               (0x1U << 17)        /*!< nBOOT1 */
#define FLASH_OBR2_BOOT0_SEL            (0x1U << 18)        /*!< BOOT0_SEL */



/*------------------------------------------------------------------------------------------------------*/
/*---                                  Independent watchdog (IWDG)                                   ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for IWDG_KR register  ********************************/
#define  IWDG_KR_KEY                         ((uint16_t)0xFFFF)            /*!< Key value (write only, read 0000h) */

/********************************  Bit definition for IWDG_PR register  ********************************/
#define  IWDG_PR_PR                          ((uint8_t)0x07)               /*!< PR[2:0] (Prescaler divider) */
#define  IWDG_PR_PR_0                        ((uint8_t)0x01)               /*!< Bit 0 */
#define  IWDG_PR_PR_1                        ((uint8_t)0x02)               /*!< Bit 1 */
#define  IWDG_PR_PR_2                        ((uint8_t)0x04)               /*!< Bit 2 */

/********************************  Bit definition for IWDG_RLR register  ********************************/
#define  IWDG_RLR_RL                         ((uint16_t)0x0FFF)            /*!< Watchdog counter reload value */

/********************************  Bit definition for IWDG_SR register  ********************************/
#define  IWDG_SR_PVU                         ((uint8_t)0x01)               /*!< Watchdog prescaler value update */
#define  IWDG_SR_RVU                         ((uint8_t)0x02)               /*!< Watchdog counter reload value update */



/*------------------------------------------------------------------------------------------------------*/
/*---                                      Real-Time Clock (RTC)                                     ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for RTC_CRH register  ********************************/
#define  RTC_CRH_SECIE                       ((uint8_t)0x01)               /*!< Second Interrupt Enable */
#define  RTC_CRH_ALRIE                       ((uint8_t)0x02)               /*!< Alarm Interrupt Enable */
#define  RTC_CRH_OWIE                        ((uint8_t)0x04)               /*!< OverfloW Interrupt Enable */

/********************************  Bit definition for RTC_CRL register  ********************************/
#define  RTC_CRL_SECF                        ((uint8_t)0x01)               /*!< Second Flag */
#define  RTC_CRL_ALRF                        ((uint8_t)0x02)               /*!< Alarm Flag */
#define  RTC_CRL_OWF                         ((uint8_t)0x04)               /*!< OverfloW Flag */
#define  RTC_CRL_RSF                         ((uint8_t)0x08)               /*!< Registers Synchronized Flag */
#define  RTC_CRL_CNF                         ((uint8_t)0x10)               /*!< Configuration Flag */
#define  RTC_CRL_RTOFF                       ((uint8_t)0x20)               /*!< RTC operation OFF */

/********************************  Bit definition for RTC_PRLH register  ********************************/
#define  RTC_PRLH_PRL                        ((uint16_t)0x000F)            /*!< RTC Prescaler Reload Value High */

/********************************  Bit definition for RTC_PRLL register  ********************************/
#define  RTC_PRLL_PRL                        ((uint16_t)0xFFFF)            /*!< RTC Prescaler Reload Value Low */

/********************************  Bit definition for RTC_DIVH register  ********************************/
#define  RTC_DIVH_RTC_DIV                    ((uint16_t)0x000F)            /*!< RTC Clock Divider High */

/********************************  Bit definition for RTC_DIVL register  ********************************/
#define  RTC_DIVL_RTC_DIV                    ((uint16_t)0xFFFF)            /*!< RTC Clock Divider Low */

/********************************  Bit definition for RTC_CNTH register  ********************************/
#define  RTC_CNTH_RTC_CNT                    ((uint16_t)0xFFFF)            /*!< RTC Counter High */

/********************************  Bit definition for RTC_CNTL register  ********************************/
#define  RTC_CNTL_RTC_CNT                    ((uint16_t)0xFFFF)            /*!< RTC Counter Low */

/********************************  Bit definition for RTC_ALRH register  ********************************/
#define  RTC_ALRH_RTC_ALR                    ((uint16_t)0xFFFF)            /*!< RTC Alarm High */

/********************************  Bit definition for RTC_ALRL register  ********************************/
#define  RTC_ALRL_RTC_ALR                    ((uint16_t)0xFFFF)            /*!< RTC Alarm Low */



/*------------------------------------------------------------------------------------------------------*/
/*---                           Pulse Width Modulator/Capture Timer (PCT)                            ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for PCT_CSR register  ********************************/
#define PCT_CSR_CC0IF          (0x1U << 0)           /*!< Compare/Capture 0 interrupt flag */
#define PCT_CSR_CC1IF          (0x1U << 1)           /*!< Compare/Capture 1 interrupt flag */
#define PCT_CSR_CC2IF          (0x1U << 2)           /*!< Compare/Capture 2 interrupt flag */
#define PCT_CSR_CC3IF          (0x1U << 3)           /*!< Compare/Capture 3 interrupt flag */
#define PCT_CSR_CC4IF          (0x1U << 4)           /*!< Compare/Capture 4 interrupt flag */
#define PCT_CSR_CC5IF          (0x1U << 5)           /*!< Compare/Capture 5 interrupt flag */
#define PCT_CSR_CEN            (0x1U << 6)           /*!< Counter enable */
#define PCT_CSR_OVF            (0x1U << 7)           /*!< Counter overflow flag */

/******************************** Bit definition for PCT_CFGR register  ********************************/
#define PCT_CFGR_OVFIE         (0x1U << 0)           /*!< Counter overflow interrupt enable */

#define PCT_CFGR_PSC_Pos       (1U)
#define PCT_CFGR_PSC_Msk       (0x1FU << PCT_CFGR_PSC_Pos)

#define PCT_CFGR_PSC_DIV4      (0x1U << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV8      (0x5U << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV12     (0x0U << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV16     (0x3U << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV24     (0x4U << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV32     (0xBU << PCT_CFGR_PSC_Pos)
#define PCT_CFGR_PSC_DIV64     (0x13U << PCT_CFGR_PSC_Pos)

#define PCT_CFGR_CSLP          (0x1U << 7)           /*!< Counter sleep control */

/******************************** Bit definition for PCT_CFGR2 register  ********************************/
#define PCT_CFGR2_CCRPE        (0x1U << 0)           /*!< Preload register on CCRx enable */
#define PCT_CFGR2_OPM          (0x1U << 1)           /*!< One-pulse mode */


/******************************** Bit definition for PCT_CCMR register  ********************************/
#define PCT_CCMR_CCIE          (0x1U << 0)           /*!< Compare/Capture interrupt enable */
#define PCT_CCMR_PWM           (0x1U << 1)           /*!< PWM mode enable */
#define PCT_CCMR_TOG           (0x1U << 2)           /*!< Toggle enable */
#define PCT_CCMR_MAT           (0x1U << 3)
#define PCT_CCMR_CAPN          (0x1U << 4)
#define PCT_CCMR_CAPP          (0x1U << 5)
#define PCT_CCMR_ECOM          (0x1U << 6)
#define PCT_CCMR_POL           (0x1U << 7)


/*------------------------------------------------------------------------------------------------------*/
/*---                                           Timer (TIM)                                          ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for TIM_CR1 register  ********************************/
#define TIM_CR1_CEN             (0x1U << 0)          /*!< Counter enable */
#define TIM_CR1_UDIS            (0x1U << 1)          /*!< Update disable */
#define TIM_CR1_URS             (0x1U << 2)          /*!< Update request source */
#define TIM_CR1_OPM             (0x1U << 3)          /*!< One pulse mode */

#define TIM_CR1_ARPE            (0x1U << 7)          /*!< Auto-reload preload enable */

/********************************  Bit definition for TIM_CR2 register  ********************************/
#define TIM_CR2_MMS             (0x7U << 4)          /*!< MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS_0           (0x1U << 4)          /*!< Bit 0 */
#define TIM_CR2_MMS_1           (0x1U << 5)          /*!< Bit 1 */
#define TIM_CR2_MMS_2           (0x1U << 6)          /*!< Bit 2 */

/********************************  Bit definition for TIM_DIER register  ********************************/
#define TIM_DIER_UIE            (0x1U << 0)          /*!< Update interrupt enable */
#define TIM_DIER_UDE            (0x1U << 8)          /*!< Update DMA request enable */

/********************************  Bit definition for TIM_SR register  ********************************/
#define TIM_SR_UIF              (0x1U << 0)          /*!< Update interrupt Flag */

/********************************  Bit definition for TIM_EGR register  ********************************/
#define TIM_EGR_UG              (0x1U << 0)          /*!< Update Generation */

/********************************  Bit definition for TIM_CNT register  ********************************/
#define TIM_CNT_CNT             (0xFFFFFU)           /*!< Counter Value */

/********************************  Bit definition for TIM_PSC register  ********************************/
#define TIM_PSC_PSC             (0xFFFFU)            /*!< Prescaler Value */

/********************************  Bit definition for TIM_ARR register  ********************************/
#define TIM_ARR_ARR             (0xFFFFFU)           /*!< actual auto-reload Value */



/*------------------------------------------------------------------------------------------------------*/
/*---                                 Inter-Integrated Circuit (I2C)                                 ---*/
/*------------------------------------------------------------------------------------------------------*/
/****************************** Bit definition for I2C_CON register *******************************/
#define I2C_CON_CRSEL_Msk       (0x83U)

#define I2C_CON_CRSEL_DIV60     (0x82U)
#define I2C_CON_CRSEL_DIV120    (0x81U)
#define I2C_CON_CRSEL_DIV160    (0x03U)
#define I2C_CON_CRSEL_DIV192    (0x02U)
#define I2C_CON_CRSEL_DIV224    (0x01U)
#define I2C_CON_CRSEL_DIV256    (0x00U)
#define I2C_CON_CRSEL_DIV960    (0x80U)

#define I2C_CON_AA              (0x1U << 2)
#define I2C_CON_SI              (0x1U << 3)
#define I2C_CON_STO             (0x1U << 4)
#define I2C_CON_STA             (0x1U << 5)
#define I2C_CON_I2CEN           (0x1U << 6)

/****************************** Bit definition for I2C_ADR register *******************************/
#define I2C_ADR_GC              (0x1U << 0)

#define I2C_ADR_ADDR_Pos        (1U)
#define I2C_ADR_ADDR_Msk        (0x7FU << I2C_ADR_ADDR_Pos)




/*------------------------------------------------------------------------------------------------------*/
/*---                                     Analog Control (ANCTL)                                     ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for ANCTL_BODCON register  ***************************/
#define ANCTL_BODCON_BODEN            (0x1U << 0)

#define ANCTL_BODCON_BLS_Pos          (1U)
#define ANCTL_BODCON_BLS_Msk          (0x7U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV0         (0x0U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV1         (0x1U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV2         (0x2U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV3         (0x3U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV4         (0x4U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV5         (0x5U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV6         (0x6U << ANCTL_BODCON_BLS_Pos)
#define ANCTL_BODCON_BLS_LEV7         (0x7U << ANCTL_BODCON_BLS_Pos)

/********************************  Bit definition for ANCTL_BODSR register  ***************************/
#define ANCTL_BODSR_BODO              (0x1U << 0)

/********************************  Bit definition for ANCTL_HSI2ENR register  ***************************/
#define ANCTL_HSI2ENR_HSI2ON          (0x1U << 0)

/********************************  Bit definition for ANCTL_DACCON register  ***************************/
#define ANCTL_DACCON_DACEN            (0x1U << 0)
#define ANCTL_DACCON_BUFEN            (0x1U << 1)

/********************************  Bit definition for ANCTL_PLLENR register  ***************************/
#define ANCTL_PLLENR_PLLON            (0x1U << 0)    /*!< PLL enable */

/********************************  Bit definition for ANCTL_PLLCR2 register  ***************************/
#define ANCTL_PLLCR2_PLLMUL_Pos       (0U)

#define ANCTL_PLLCR2_PLLMUL_Msk       (0xFU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_2         (0x1U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_3         (0x2U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_4         (0x3U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_5         (0x4U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_6         (0x5U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_7         (0x6U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_8         (0x7U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_9         (0x8U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_10        (0x9U << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_11        (0xAU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_12        (0xBU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_13        (0xCU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_14        (0xDU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_15        (0xEU << ANCTL_PLLCR2_PLLMUL_Pos)
#define ANCTL_PLLCR2_PLLMUL_16        (0xFU << ANCTL_PLLCR2_PLLMUL_Pos)

/********************************  Bit definition for ANCTL_PLLSR register  ***************************/
#define ANCTL_PLLSR_PLLRDY            (0x1U << 0)

/********************************  Bit definition for ANCTL_LSICR register  ***************************/
#define ANCTL_LSICR_LSION             (0x1U << 0)   /*!< Internal low-speed oscillator enable */

/********************************  Bit definition for ANCTL_LSISR register  ***************************/
#define ANCTL_LSISR_LSIRDY            (0x1U << 0)   /*!< Internal low-speed oscillator ready flag */

/********************************  Bit definition for ANCTL_HSI48ENR register  ***************************/
#define ANCTL_HSI48ENR_HSI48ON        (0x1U << 0)    /*!< Internal 48M high-speed clock enable */

/********************************  Bit definition for ANCTL_HSI48SR register  ***************************/
#define ANCTL_HSI48SR_HSI48RDY        (0x1U << 0)    /*!< Internal 48M high-speed clock ready flag */

/********************************  Bit definition for ANCTL_USBPDSR register  ***************************/
#define ANCTL_USBPDSR_SE0             (0x1U << 0)
#define ANCTL_USBPDSR_SE1             (0x1U << 1)
#define ANCTL_USBPDSR_JSTAT           (0x1U << 2)
#define ANCTL_USBPDSR_KSTAT           (0x1U << 3)
#define ANCTL_USBPDSR_SETUP           (0x1U << 4)

/********************************  Bit definition for ANCTL_USBPDCR register  ***************************/
#define ANCTL_USBPDCR_SE0IE           (0x1U << 0)
#define ANCTL_USBPDCR_SE1IE           (0x1U << 1)
#define ANCTL_USBPDCR_JSTATIE         (0x1U << 2)
#define ANCTL_USBPDCR_KSTATIE         (0x1U << 3)

/********************************  Bit definition for ANCTL_HSECR register  ***************************/
#define ANCTL_HSECR_DISABLE           (0x0U)
#define ANCTL_HSECR_BYPASS            (0x1U)
#define ANCTL_HSECR_ENABLE            (0x3U)



/*------------------------------------------------------------------------------------------------------*/
/*---                                 Reset and Clock Control (RCC)                                  ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for RCC_PLLPRE1 register  ***************************/
#define RCC_PLLPRE1_DIVEN           (0x1U << 0)

#define RCC_PLLPRE1_RATIO_Msk       (0x3FU << 1)
#define RCC_PLLPRE1_RATIO_2         (0x0U << 1)
#define RCC_PLLPRE1_RATIO_3         (0x1U << 1)
#define RCC_PLLPRE1_RATIO_4         (0x2U << 1)
#define RCC_PLLPRE1_RATIO_5         (0x3U << 1)
#define RCC_PLLPRE1_RATIO_6         (0x4U << 1)
#define RCC_PLLPRE1_RATIO_7         (0x5U << 1)
#define RCC_PLLPRE1_RATIO_8         (0x6U << 1)
#define RCC_PLLPRE1_RATIO_9         (0x7U << 1)
#define RCC_PLLPRE1_RATIO_10        (0x8U << 1)
#define RCC_PLLPRE1_RATIO_11        (0x9U << 1)
#define RCC_PLLPRE1_RATIO_12        (0xAU << 1)
#define RCC_PLLPRE1_RATIO_13        (0xBU << 1)
#define RCC_PLLPRE1_RATIO_14        (0xCU << 1)
#define RCC_PLLPRE1_RATIO_15        (0xDU << 1)
#define RCC_PLLPRE1_RATIO_16        (0xEU << 1)
#define RCC_PLLPRE1_RATIO_17        (0xFU << 1)
#define RCC_PLLPRE1_RATIO_18        (0x10U << 1)
#define RCC_PLLPRE1_RATIO_19        (0x11U << 1)
#define RCC_PLLPRE1_RATIO_20        (0x12U << 1)
#define RCC_PLLPRE1_RATIO_21        (0x13U << 1)
#define RCC_PLLPRE1_RATIO_22        (0x14U << 1)
#define RCC_PLLPRE1_RATIO_23        (0x15U << 1)
#define RCC_PLLPRE1_RATIO_24        (0x16U << 1)
#define RCC_PLLPRE1_RATIO_25        (0x17U << 1)
#define RCC_PLLPRE1_RATIO_26        (0x18U << 1)
#define RCC_PLLPRE1_RATIO_27        (0x19U << 1)
#define RCC_PLLPRE1_RATIO_28        (0x1AU << 1)
#define RCC_PLLPRE1_RATIO_29        (0x1BU << 1)
#define RCC_PLLPRE1_RATIO_30        (0x1CU << 1)
#define RCC_PLLPRE1_RATIO_31        (0x1DU << 1)
#define RCC_PLLPRE1_RATIO_32        (0x1EU << 1)
#define RCC_PLLPRE1_RATIO_33        (0x1FU << 1)
#define RCC_PLLPRE1_RATIO_34        (0x20U << 1)
#define RCC_PLLPRE1_RATIO_35        (0x21U << 1)
#define RCC_PLLPRE1_RATIO_36        (0x22U << 1)
#define RCC_PLLPRE1_RATIO_37        (0x23U << 1)
#define RCC_PLLPRE1_RATIO_38        (0x24U << 1)
#define RCC_PLLPRE1_RATIO_39        (0x25U << 1)
#define RCC_PLLPRE1_RATIO_40        (0x26U << 1)
#define RCC_PLLPRE1_RATIO_41        (0x27U << 1)
#define RCC_PLLPRE1_RATIO_42        (0x28U << 1)
#define RCC_PLLPRE1_RATIO_43        (0x29U << 1)
#define RCC_PLLPRE1_RATIO_44        (0x2AU << 1)
#define RCC_PLLPRE1_RATIO_45        (0x2BU << 1)
#define RCC_PLLPRE1_RATIO_46        (0x2CU << 1)
#define RCC_PLLPRE1_RATIO_47        (0x2DU << 1)
#define RCC_PLLPRE1_RATIO_48        (0x2EU << 1)
#define RCC_PLLPRE1_RATIO_49        (0x2FU << 1)
#define RCC_PLLPRE1_RATIO_50        (0x30U << 1)
#define RCC_PLLPRE1_RATIO_51        (0x31U << 1)
#define RCC_PLLPRE1_RATIO_52        (0x32U << 1)
#define RCC_PLLPRE1_RATIO_53        (0x33U << 1)
#define RCC_PLLPRE1_RATIO_54        (0x34U << 1)
#define RCC_PLLPRE1_RATIO_55        (0x35U << 1)
#define RCC_PLLPRE1_RATIO_56        (0x36U << 1)
#define RCC_PLLPRE1_RATIO_57        (0x37U << 1)
#define RCC_PLLPRE1_RATIO_58        (0x38U << 1)
#define RCC_PLLPRE1_RATIO_59        (0x39U << 1)
#define RCC_PLLPRE1_RATIO_60        (0x3AU << 1)
#define RCC_PLLPRE1_RATIO_61        (0x3BU << 1)
#define RCC_PLLPRE1_RATIO_62        (0x3CU << 1)
#define RCC_PLLPRE1_RATIO_63        (0x3DU << 1)
#define RCC_PLLPRE1_RATIO_64        (0x3EU << 1)

/********************************  Bit definition for RCC_PLLPRE2 register  ***************************/
#define RCC_PLLPRE2_SRCEN           (0x1U << 0)

/********************************  Bit definition for RCC_PLLSRC register  ***************************/
#define RCC_PLLSRC_NONE             (0x0U)
#define RCC_PLLSRC_HSI48            (0x1U)
#define RCC_PLLSRC_HSE              (0x2U)

/********************************  Bit definition for RCC_SYSCLKPRE1 register  ***************************/
#define RCC_SYSCLKPRE1_DIVEN            (0x1U << 0)

#define RCC_SYSCLKPRE1_RATIO_Msk        (0x3FU << 1)
#define RCC_SYSCLKPRE1_RATIO_2          (0x0U << 1)
#define RCC_SYSCLKPRE1_RATIO_3          (0x1U << 1)
#define RCC_SYSCLKPRE1_RATIO_4          (0x2U << 1)
#define RCC_SYSCLKPRE1_RATIO_5          (0x3U << 1)
#define RCC_SYSCLKPRE1_RATIO_6          (0x4U << 1)
#define RCC_SYSCLKPRE1_RATIO_7          (0x5U << 1)
#define RCC_SYSCLKPRE1_RATIO_8          (0x6U << 1)
#define RCC_SYSCLKPRE1_RATIO_9          (0x7U << 1)
#define RCC_SYSCLKPRE1_RATIO_10         (0x8U << 1)
#define RCC_SYSCLKPRE1_RATIO_11         (0x9U << 1)
#define RCC_SYSCLKPRE1_RATIO_12         (0xAU << 1)
#define RCC_SYSCLKPRE1_RATIO_13         (0xBU << 1)
#define RCC_SYSCLKPRE1_RATIO_14         (0xCU << 1)
#define RCC_SYSCLKPRE1_RATIO_15         (0xDU << 1)
#define RCC_SYSCLKPRE1_RATIO_16         (0xEU << 1)
#define RCC_SYSCLKPRE1_RATIO_17         (0xFU << 1)
#define RCC_SYSCLKPRE1_RATIO_18         (0x10U << 1)
#define RCC_SYSCLKPRE1_RATIO_19         (0x11U << 1)
#define RCC_SYSCLKPRE1_RATIO_20         (0x12U << 1)
#define RCC_SYSCLKPRE1_RATIO_21         (0x13U << 1)
#define RCC_SYSCLKPRE1_RATIO_22         (0x14U << 1)
#define RCC_SYSCLKPRE1_RATIO_23         (0x15U << 1)
#define RCC_SYSCLKPRE1_RATIO_24         (0x16U << 1)
#define RCC_SYSCLKPRE1_RATIO_25         (0x17U << 1)
#define RCC_SYSCLKPRE1_RATIO_26         (0x18U << 1)
#define RCC_SYSCLKPRE1_RATIO_27         (0x19U << 1)
#define RCC_SYSCLKPRE1_RATIO_28         (0x1AU << 1)
#define RCC_SYSCLKPRE1_RATIO_29         (0x1BU << 1)
#define RCC_SYSCLKPRE1_RATIO_30         (0x1CU << 1)
#define RCC_SYSCLKPRE1_RATIO_31         (0x1DU << 1)
#define RCC_SYSCLKPRE1_RATIO_32         (0x1EU << 1)
#define RCC_SYSCLKPRE1_RATIO_33         (0x1FU << 1)
#define RCC_SYSCLKPRE1_RATIO_34         (0x20U << 1)
#define RCC_SYSCLKPRE1_RATIO_35         (0x21U << 1)
#define RCC_SYSCLKPRE1_RATIO_36         (0x22U << 1)
#define RCC_SYSCLKPRE1_RATIO_37         (0x23U << 1)
#define RCC_SYSCLKPRE1_RATIO_38         (0x24U << 1)
#define RCC_SYSCLKPRE1_RATIO_39         (0x25U << 1)
#define RCC_SYSCLKPRE1_RATIO_40         (0x26U << 1)
#define RCC_SYSCLKPRE1_RATIO_41         (0x27U << 1)
#define RCC_SYSCLKPRE1_RATIO_42         (0x28U << 1)
#define RCC_SYSCLKPRE1_RATIO_43         (0x29U << 1)
#define RCC_SYSCLKPRE1_RATIO_44         (0x2AU << 1)
#define RCC_SYSCLKPRE1_RATIO_45         (0x2BU << 1)
#define RCC_SYSCLKPRE1_RATIO_46         (0x2CU << 1)
#define RCC_SYSCLKPRE1_RATIO_47         (0x2DU << 1)
#define RCC_SYSCLKPRE1_RATIO_48         (0x2EU << 1)
#define RCC_SYSCLKPRE1_RATIO_49         (0x2FU << 1)
#define RCC_SYSCLKPRE1_RATIO_50         (0x30U << 1)
#define RCC_SYSCLKPRE1_RATIO_51         (0x31U << 1)
#define RCC_SYSCLKPRE1_RATIO_52         (0x32U << 1)
#define RCC_SYSCLKPRE1_RATIO_53         (0x33U << 1)
#define RCC_SYSCLKPRE1_RATIO_54         (0x34U << 1)
#define RCC_SYSCLKPRE1_RATIO_55         (0x35U << 1)
#define RCC_SYSCLKPRE1_RATIO_56         (0x36U << 1)
#define RCC_SYSCLKPRE1_RATIO_57         (0x37U << 1)
#define RCC_SYSCLKPRE1_RATIO_58         (0x38U << 1)
#define RCC_SYSCLKPRE1_RATIO_59         (0x39U << 1)
#define RCC_SYSCLKPRE1_RATIO_60         (0x3AU << 1)
#define RCC_SYSCLKPRE1_RATIO_61         (0x3BU << 1)
#define RCC_SYSCLKPRE1_RATIO_62         (0x3CU << 1)
#define RCC_SYSCLKPRE1_RATIO_63         (0x3DU << 1)
#define RCC_SYSCLKPRE1_RATIO_64         (0x3EU << 1)

/********************************  Bit definition for RCC_SYSCLKSRC register  ***************************/
#define RCC_SYSCLKSRC_HSI2              (0x0U)
#define RCC_SYSCLKSRC_HSI48_DIV6        (0x1U)
#define RCC_SYSCLKSRC_HSI48             (0x2U)
#define RCC_SYSCLKSRC_HSE               (0x3U)
#define RCC_SYSCLKSRC_PLL               (0x4U)

/********************************  Bit definition for RCC_SYSCLKUEN register  ***************************/
#define RCC_SYSCLKUEN_ENA               (0x1U)

/********************************  Bit definition for RCC_FCUCLKPRE1 register  ***************************/
#define RCC_FCUCLKPRE1_DIVEN            (0x1U << 0)

#define RCC_FCUCLKPRE1_RATIO_Msk        (0xFU << 1)
#define RCC_FCUCLKPRE1_RATIO_2          (0x0U << 1)
#define RCC_FCUCLKPRE1_RATIO_3          (0x1U << 1)
#define RCC_FCUCLKPRE1_RATIO_4          (0x2U << 1)
#define RCC_FCUCLKPRE1_RATIO_5          (0x3U << 1)
#define RCC_FCUCLKPRE1_RATIO_6          (0x4U << 1)
#define RCC_FCUCLKPRE1_RATIO_7          (0x5U << 1)
#define RCC_FCUCLKPRE1_RATIO_8          (0x6U << 1)
#define RCC_FCUCLKPRE1_RATIO_9          (0x7U << 1)
#define RCC_FCUCLKPRE1_RATIO_10         (0x8U << 1)
#define RCC_FCUCLKPRE1_RATIO_11         (0x9U << 1)
#define RCC_FCUCLKPRE1_RATIO_12         (0xAU << 1)
#define RCC_FCUCLKPRE1_RATIO_13         (0xBU << 1)
#define RCC_FCUCLKPRE1_RATIO_14         (0xCU << 1)
#define RCC_FCUCLKPRE1_RATIO_15         (0xDU << 1)
#define RCC_FCUCLKPRE1_RATIO_16         (0xEU << 1)

/********************************  Bit definition for RCC_FCUCLKPRE2 register  ***************************/
#define RCC_FCUCLKPRE2_SRCEN            (0x1U << 0)

/********************************  Bit definition for RCC_FCUCLKSRC register  ***************************/
#define RCC_FCUCLKSRC_NONE              (0x0U)
#define RCC_FCUCLKSRC_HSI48             (0x1U)
#define RCC_FCUCLKSRC_PLL               (0x2U)

/********************************  Bit definition for RCC_PLL2USBPRE1 register  ***************************/
#define RCC_PLL2USBPRE1_DIVEN           (0x1U << 0)

#define RCC_PLL2USBPRE1_RATIO_Msk       (0xFU << 1)
#define RCC_PLL2USBPRE1_RATIO_2         (0x0U << 1)
#define RCC_PLL2USBPRE1_RATIO_3         (0x1U << 1)
#define RCC_PLL2USBPRE1_RATIO_4         (0x2U << 1)
#define RCC_PLL2USBPRE1_RATIO_5         (0x3U << 1)
#define RCC_PLL2USBPRE1_RATIO_6         (0x4U << 1)
#define RCC_PLL2USBPRE1_RATIO_7         (0x5U << 1)
#define RCC_PLL2USBPRE1_RATIO_8         (0x6U << 1)
#define RCC_PLL2USBPRE1_RATIO_9         (0x7U << 1)
#define RCC_PLL2USBPRE1_RATIO_10        (0x8U << 1)
#define RCC_PLL2USBPRE1_RATIO_11        (0x9U << 1)
#define RCC_PLL2USBPRE1_RATIO_12        (0xAU << 1)
#define RCC_PLL2USBPRE1_RATIO_13        (0xBU << 1)
#define RCC_PLL2USBPRE1_RATIO_14        (0xCU << 1)
#define RCC_PLL2USBPRE1_RATIO_15        (0xDU << 1)
#define RCC_PLL2USBPRE1_RATIO_16        (0xEU << 1)

/********************************  Bit definition for RCC_PLL2USBPRE2 register  ***************************/
#define RCC_PLL2USBPRE2_SRCEN           (0x1U << 0)

/********************************  Bit definition for RCC_USBCLKSRC register  ***************************/
#define RCC_USBCLKSRC_NONE              (0x0U)
#define RCC_USBCLKSRC_HSI48             (0x1U)
#define RCC_USBCLKSRC_PLL2USB           (0x2U)

/********************************  Bit definition for RCC_MCOPRE register  ***************************/
#define RCC_MCOPRE_DIVEN            (0x1U << 0)

#define RCC_MCOPRE_RATIO_Msk        (0x3FU << 1)
#define RCC_MCOPRE_RATIO_2          (0x0U << 1)
#define RCC_MCOPRE_RATIO_3          (0x1U << 1)
#define RCC_MCOPRE_RATIO_4          (0x2U << 1)
#define RCC_MCOPRE_RATIO_5          (0x3U << 1)
#define RCC_MCOPRE_RATIO_6          (0x4U << 1)
#define RCC_MCOPRE_RATIO_7          (0x5U << 1)
#define RCC_MCOPRE_RATIO_8          (0x6U << 1)
#define RCC_MCOPRE_RATIO_9          (0x7U << 1)
#define RCC_MCOPRE_RATIO_10         (0x8U << 1)
#define RCC_MCOPRE_RATIO_11         (0x9U << 1)
#define RCC_MCOPRE_RATIO_12         (0xAU << 1)
#define RCC_MCOPRE_RATIO_13         (0xBU << 1)
#define RCC_MCOPRE_RATIO_14         (0xCU << 1)
#define RCC_MCOPRE_RATIO_15         (0xDU << 1)
#define RCC_MCOPRE_RATIO_16         (0xEU << 1)
#define RCC_MCOPRE_RATIO_17         (0xFU << 1)
#define RCC_MCOPRE_RATIO_18         (0x10U << 1)
#define RCC_MCOPRE_RATIO_19         (0x11U << 1)
#define RCC_MCOPRE_RATIO_20         (0x12U << 1)
#define RCC_MCOPRE_RATIO_21         (0x13U << 1)
#define RCC_MCOPRE_RATIO_22         (0x14U << 1)
#define RCC_MCOPRE_RATIO_23         (0x15U << 1)
#define RCC_MCOPRE_RATIO_24         (0x16U << 1)
#define RCC_MCOPRE_RATIO_25         (0x17U << 1)
#define RCC_MCOPRE_RATIO_26         (0x18U << 1)
#define RCC_MCOPRE_RATIO_27         (0x19U << 1)
#define RCC_MCOPRE_RATIO_28         (0x1AU << 1)
#define RCC_MCOPRE_RATIO_29         (0x1BU << 1)
#define RCC_MCOPRE_RATIO_30         (0x1CU << 1)
#define RCC_MCOPRE_RATIO_31         (0x1DU << 1)
#define RCC_MCOPRE_RATIO_32         (0x1EU << 1)
#define RCC_MCOPRE_RATIO_33         (0x1FU << 1)
#define RCC_MCOPRE_RATIO_34         (0x20U << 1)
#define RCC_MCOPRE_RATIO_35         (0x21U << 1)
#define RCC_MCOPRE_RATIO_36         (0x22U << 1)
#define RCC_MCOPRE_RATIO_37         (0x23U << 1)
#define RCC_MCOPRE_RATIO_38         (0x24U << 1)
#define RCC_MCOPRE_RATIO_39         (0x25U << 1)
#define RCC_MCOPRE_RATIO_40         (0x26U << 1)
#define RCC_MCOPRE_RATIO_41         (0x27U << 1)
#define RCC_MCOPRE_RATIO_42         (0x28U << 1)
#define RCC_MCOPRE_RATIO_43         (0x29U << 1)
#define RCC_MCOPRE_RATIO_44         (0x2AU << 1)
#define RCC_MCOPRE_RATIO_45         (0x2BU << 1)
#define RCC_MCOPRE_RATIO_46         (0x2CU << 1)
#define RCC_MCOPRE_RATIO_47         (0x2DU << 1)
#define RCC_MCOPRE_RATIO_48         (0x2EU << 1)
#define RCC_MCOPRE_RATIO_49         (0x2FU << 1)
#define RCC_MCOPRE_RATIO_50         (0x30U << 1)
#define RCC_MCOPRE_RATIO_51         (0x31U << 1)
#define RCC_MCOPRE_RATIO_52         (0x32U << 1)
#define RCC_MCOPRE_RATIO_53         (0x33U << 1)
#define RCC_MCOPRE_RATIO_54         (0x34U << 1)
#define RCC_MCOPRE_RATIO_55         (0x35U << 1)
#define RCC_MCOPRE_RATIO_56         (0x36U << 1)
#define RCC_MCOPRE_RATIO_57         (0x37U << 1)
#define RCC_MCOPRE_RATIO_58         (0x38U << 1)
#define RCC_MCOPRE_RATIO_59         (0x39U << 1)
#define RCC_MCOPRE_RATIO_60         (0x3AU << 1)
#define RCC_MCOPRE_RATIO_61         (0x3BU << 1)
#define RCC_MCOPRE_RATIO_62         (0x3CU << 1)
#define RCC_MCOPRE_RATIO_63         (0x3DU << 1)
#define RCC_MCOPRE_RATIO_64         (0x3EU << 1)

/********************************  Bit definition for RCC_MCOSEL register  ***************************/
#define RCC_MCOSEL_LSI              (0x0U)
#define RCC_MCOSEL_PLL              (0x2U)
#define RCC_MCOSEL_HSI48            (0x3U)
#define RCC_MCOSEL_SYSCLKSRC        (0x4U)

/********************************  Bit definition for RCC_MCOUEN register  ***************************/
#define RCC_MCOUEN_ENA              (0x1U)

/********************************  Bit definition for RCC_MCOENR register  ***************************/
#define RCC_MCOENR_OUTEN            (0x1U)

/********************************  Bit definition for RCC_APBENR register  ***************************/
#define RCC_APBENR_CRCEN          (0x1U << 0)
#define RCC_APBENR_CRSEN          (0x1U << 1)
#define RCC_APBENR_UART0EN        (0x1U << 2)
#define RCC_APBENR_UART1EN        (0x1U << 3)
#define RCC_APBENR_ARGB03EN       (0x1U << 4)
#define RCC_APBENR_ARGB45EN       (0x1U << 5)
#define RCC_APBENR_IWDGEN         (0x1U << 6)
#define RCC_APBENR_RTCEN          (0x1U << 7)
#define RCC_APBENR_I2CEN          (0x1U << 8)
#define RCC_APBENR_GPIOEN         (0x1U << 9)
#define RCC_APBENR_TIM6EN         (0x1U << 10)
#define RCC_APBENR_SPIMEN         (0x1U << 11)
#define RCC_APBENR_SPI1EN         (0x1U << 12)
#define RCC_APBENR_PCT0EN         (0x1U << 13)
#define RCC_APBENR_PCT1EN         (0x1U << 14)
#define RCC_APBENR_PCT2EN         (0x1U << 15)
#define RCC_APBENR_PCT3EN         (0x1U << 16)
#define RCC_APBENR_PCT4EN         (0x1U << 17)
#define RCC_APBENR_ADCEN          (0x1U << 18)
#define RCC_APBENR_EXTIEN         (0x1U << 19)
#define RCC_APBENR_USBEN          (0x1U << 20)

/********************************  Bit definition for RCC_APBRSTR register  ***************************/
#define RCC_APBRSTR_CRCRST        (0x1U << 0)
#define RCC_APBRSTR_CRSRST        (0x1U << 1)
#define RCC_APBRSTR_UART0RST      (0x1U << 2)
#define RCC_APBRSTR_UART1RST      (0x1U << 3)
#define RCC_APBRSTR_ARGB03RST     (0x1U << 4)
#define RCC_APBRSTR_ARGB45RST     (0x1U << 5)
#define RCC_APBRSTR_IWDGRST       (0x1U << 6)
#define RCC_APBRSTR_RTCRST        (0x1U << 7)
#define RCC_APBRSTR_I2CRST        (0x1U << 8)
#define RCC_APBRSTR_GPIORST       (0x1U << 9)
#define RCC_APBRSTR_TIM6RST       (0x1U << 10)
#define RCC_APBRSTR_SPIMRST       (0x1U << 11)
#define RCC_APBRSTR_SPI1RST       (0x1U << 12)
#define RCC_APBRSTR_PCT0RST       (0x1U << 13)
#define RCC_APBRSTR_PCT1RST       (0x1U << 14)
#define RCC_APBRSTR_PCT2RST       (0x1U << 15)
#define RCC_APBRSTR_PCT3RST       (0x1U << 16)
#define RCC_APBRSTR_PCT4RST       (0x1U << 17)
#define RCC_APBRSTR_ADCRST        (0x1U << 18)
#define RCC_APBRSTR_EXTIRST       (0x1U << 19)
#define RCC_APBRSTR_USBRST        (0x1U << 20)

/********************************  Bit definition for RCC_SPI1SCLKCR register  ***************************/
#define RCC_SPI1SCLKCR_CLKEN_PA6   (0x1U << 0)
#define RCC_SPI1SCLKCR_CLKEN_PB12  (0x1U << 1)

/********************************  Bit definition for RCC_CLRRSTSTAT register  ***************************/
#define RCC_CLRRSTSTAT_CLR        (0x1U)

/********************************  Bit definition for RCC_RSTSTAT register  ***************************/
#define RCC_RSTSTAT_LPWRRSTF      (0x1U << 0)
#define RCC_RSTSTAT_IWDGRSTF      (0x1U << 1)
#define RCC_RSTSTAT_SFTRSTF       (0x1U << 2)
#define RCC_RSTSTAT_PORRSTF       (0x1U << 3)
#define RCC_RSTSTAT_LVRRSTF       (0x1U << 4)
#define RCC_RSTSTAT_PINRSTF       (0x1U << 5)


/*------------------------------------------------------------------------------------------------------*/
/*---                               Serial Peripheral Interface (SPIM)                               ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************    Bit definition for SPIM_CR0 register *******************************/
#define SPIM_CR0_DFS_Msk               (0xFU << 0)         /*!< Data Frame Size field mask */
#define SPIM_CR0_DFS_4BITS             (0x3U << 0)         /*!< 4-bit serial data transfer */
#define SPIM_CR0_DFS_5BITS             (0x4U << 0)         /*!< 5-bit serial data transfer */
#define SPIM_CR0_DFS_6BITS             (0x5U << 0)         /*!< 6-bit serial data transfer */
#define SPIM_CR0_DFS_7BITS             (0x6U << 0)         /*!< 7-bit serial data transfer */
#define SPIM_CR0_DFS_8BITS             (0x7U << 0)         /*!< 8-bit serial data transfer */
#define SPIM_CR0_DFS_9BITS             (0x8U << 0)         /*!< 9-bit serial data transfer */
#define SPIM_CR0_DFS_10BITS            (0x9U << 0)         /*!< 10-bit serial data transfer */
#define SPIM_CR0_DFS_11BITS            (0xAU << 0)         /*!< 11-bit serial data transfer */
#define SPIM_CR0_DFS_12BITS            (0xBU << 0)         /*!< 12-bit serial data transfer */
#define SPIM_CR0_DFS_13BITS            (0xCU << 0)         /*!< 13-bit serial data transfer */
#define SPIM_CR0_DFS_14BITS            (0xDU << 0)         /*!< 14-bit serial data transfer */
#define SPIM_CR0_DFS_15BITS            (0xEU << 0)         /*!< 15-bit serial data transfer */
#define SPIM_CR0_DFS_16BITS            (0xFU << 0)         /*!< 16-bit serial data transfer */

#define SPIM_CR0_FRF_Msk               (0x3U << 4)         /*!< Frame Format field mask */
#define SPIM_CR0_FRF_SPI               (0x0U << 4)         /*!< Motorolla SPI Frame Format */
#define SPIM_CR0_FRF_SSP               (0x1U << 4)         /*!< Texas Instruments SSP Frame Format */
#define SPIM_CR0_FRF_NS                (0x2U << 4)         /*!< National Microwire Frame Format */

#define SPIM_CR0_CPHA                  (0x1U << 6)         /*!< Serial Clock Phase */
#define SPIM_CR0_CPOL                  (0x1U << 7)         /*!< Serial Clock Polarity */

#define SPIM_CR0_TMOD_Msk              (0x3U << 8)         /*!< Transfer Mode field mask */
#define SPIM_CR0_TMOD_TX_AND_RX        (0x0U << 8)         /*!< Transfer & receive */
#define SPIM_CR0_TMOD_TX_ONLY          (0x1U << 8)         /*!< Transmit only mode */
#define SPIM_CR0_TMOD_RX_ONLY          (0x2U << 8)         /*!< Receive only mode */
#define SPIM_CR0_TMOD_EEPROM_READ      (0x3U << 8)         /*!< EEPROM Read mode */

#define SPIM_CR0_SRL                   (0x1U << 11)        /*!< Shift Register Loop */

#define SPIM_CR0_CFS_Msk               (0xFU << 12)        /*!< Control Frame Size field mask */
#define SPIM_CR0_CFS_01_BIT            (0x0U << 12)        /*!< 1-bit Control Word */
#define SPIM_CR0_CFS_02_BIT            (0x1U << 12)        /*!< 2-bit Control Word */
#define SPIM_CR0_CFS_03_BIT            (0x2U << 12)        /*!< 3-bit Control Word */
#define SPIM_CR0_CFS_04_BIT            (0x3U << 12)        /*!< 4-bit Control Word */
#define SPIM_CR0_CFS_05_BIT            (0x4U << 12)        /*!< 5-bit Control Word */
#define SPIM_CR0_CFS_06_BIT            (0x5U << 12)        /*!< 6-bit Control Word */
#define SPIM_CR0_CFS_07_BIT            (0x6U << 12)        /*!< 7-bit Control Word */
#define SPIM_CR0_CFS_08_BIT            (0x7U << 12)        /*!< 8-bit Control Word */
#define SPIM_CR0_CFS_09_BIT            (0x8U << 12)        /*!< 9-bit Control Word */
#define SPIM_CR0_CFS_10_BIT            (0x9U << 12)        /*!< 10-bit Control Word */
#define SPIM_CR0_CFS_11_BIT            (0xAU << 12)        /*!< 11-bit Control Word */
#define SPIM_CR0_CFS_12_BIT            (0xBU << 12)        /*!< 12-bit Control Word */
#define SPIM_CR0_CFS_13_BIT            (0xCU << 12)        /*!< 13-bit Control Word */
#define SPIM_CR0_CFS_14_BIT            (0xDU << 12)        /*!< 14-bit Control Word */
#define SPIM_CR0_CFS_15_BIT            (0xEU << 12)        /*!< 15-bit Control Word */
#define SPIM_CR0_CFS_16_BIT            (0xFU << 12)        /*!< 16-bit Control Word */

#define SPIM_CR0_SPI_MODE_Msk          (0x3U << 21)        /*!< SPI Mode field mask */
#define SPIM_CR0_SPI_MODE_STD          (0x0U << 21)        /*!< Standard SPI Mode */
#define SPIM_CR0_SPI_MODE_DUAL         (0x1U << 21)        /*!< Dual SPI Mode */
#define SPIM_CR0_SPI_MODE_QUAD         (0x2U << 21)        /*!< Quad SPI Mode */
#define SPIM_CR0_SPI_MODE_OCTAL        (0x3U << 21)        /*!< Octal SPI Mode */

#define SPIM_CR0_SSTE                  (0x1U << 24)        /*!< Slave Select Toggle Enable */

/******************************    Bit definition for SPIM_CR1 register *******************************/
#define SPIM_CR1_NDF_Msk               (0xFFFFU)           /*!< Number of Data Frames field mask */

/******************************    Bit definition for SPIM_SPIENR register *******************************/
#define SPIM_SPIENR_SPI_EN             (0x1U << 0)         /*!< SPI Enable */

/******************************    Bit definition for SPIM_MWCR register   *******************************/
#define SPIM_MWCR_MWMOD                (0x1U << 0)         /*!< Microwire Transfer Mode */
#define SPIM_MWCR_MDD                  (0x1U << 1)         /*!< Microwire Control */
#define SPIM_MWCR_MHS                  (0x1U << 2)         /*!< Microwire Handshaking */

/******************************    Bit definition for SPIM_SER register    *******************************/
#define SPIM_SER_Msk                   (0x7U << 0)         /*!< Slave Select Enable Flag field mask */
#define SPIM_SER_SE0                   (0x1U << 0)         /*!< Slave 0 Select Enable Flag */
#define SPIM_SER_SE1                   (0x1U << 1)         /*!< Slave 1 Select Enable Flag */
#define SPIM_SER_SE2                   (0x1U << 2)         /*!< Slave 2 Select Enable Flag */

/******************************    Bit definition for SPIM_BAUDR register  *******************************/
#define SPIM_BAUDR_SCKDV_Msk           (0xFFFFU)           /*!< SPI Clock Divider field mask */

/******************************   Bit definition for SPIM_TXFTLR register  *******************************/

/******************************   Bit definition for SPIM_RXFTLR register  *******************************/

/******************************    Bit definition for SPIM_TXFLR register  *******************************/

/******************************    Bit definition for SPIM_RXFLR register  *******************************/

/******************************     Bit definition for SPIM_SR register    *******************************/
#define SPIM_SR_BUSY                   (0x1U << 0)         /*!< SPI Busy Flag */
#define SPIM_SR_TFNF                   (0x1U << 1)         /*!< Transmit FIFO Not Full */
#define SPIM_SR_TFE                    (0x1U << 2)         /*!< Transmit FIFO Empty */
#define SPIM_SR_RFNE                   (0x1U << 3)         /*!< Receive FIFO Not Empty */
#define SPIM_SR_RFF                    (0x1U << 4)         /*!< Receive FIFO Full */
#define SPIM_SR_TXERR                  (0x1U << 5)         /*!< Transmission Error */
#define SPIM_SR_DCOL                   (0x1U << 6)         /*!< Data Collision Error */

/******************************     Bit definition for SPIM_IER register   *******************************/
#define SPIM_IER_TXEIE                 (0x1U << 0)         /*!< Transmit FIFO Empty Interrupt Enable */
#define SPIM_IER_TXOIE                 (0x1U << 1)         /*!< Transmit FIFO Overflow Interrupt Enable */
#define SPIM_IER_RXUIE                 (0x1U << 2)         /*!< Receive FIFO Underflow Interrupt Enable */
#define SPIM_IER_RXOIE                 (0x1U << 3)         /*!< Receive FIFO Overflow Interrupt Enable */
#define SPIM_IER_RXFIE                 (0x1U << 4)         /*!< Receive FIFO Full Interrupt Enable */
#define SPIM_IER_MSTIE                 (0x1U << 5)         /*!< Multi-Master Contention Interrupt Enable */

/******************************     Bit definition for SPIM_ISR register   *******************************/
#define SPIM_ISR_TXEIS                 (0x1U << 0)         /*!< Transmit FIFO Empty Interrupt Status */
#define SPIM_ISR_TXOIS                 (0x1U << 1)         /*!< Transmit FIFO Overflow Interrupt Status */
#define SPIM_ISR_RXUIS                 (0x1U << 2)         /*!< Receive FIFO Underflow Interrupt Status */
#define SPIM_ISR_RXOIS                 (0x1U << 3)         /*!< Receive FIFO Overflow Interrupt Status */
#define SPIM_ISR_RXFIS                 (0x1U << 4)         /*!< Receive FIFO Full Interrupt Status */
#define SPIM_ISR_MSTIS                 (0x1U << 5)         /*!< Multi-Master Contention Interrupt Status */

/******************************    Bit definition for SPIM_RISR register   *******************************/
#define SPIM_RISR_TXEIR                (0x1U << 0)         /*!< Transmit FIFO Empty Raw Interrupt Status */
#define SPIM_RISR_TXOIR                (0x1U << 1)         /*!< Transmit FIFO Overflow Raw Interrupt Status */
#define SPIM_RISR_RXUIR                (0x1U << 2)         /*!< Receive FIFO Underflow Raw Interrupt Status */
#define SPIM_RISR_RXOIR                (0x1U << 3)         /*!< Receive FIFO Overflow Raw Interrupt Status */
#define SPIM_RISR_RXFIR                (0x1U << 4)         /*!< Receive FIFO Full Raw Interrupt Status */
#define SPIM_RISR_MSTIR                (0x1U << 5)         /*!< Multi-Master Contention Raw Interrupt Status */



/*------------------------------------------------------------------------------------------------------*/
/*---                                Serial Peripheral Interface (SPI)                               ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for SPI_CR register  ********************************/
#define SPI_CR_CPHA                     (0x1U << 2)                 /*!< Clock Phase */
#define SPI_CR_CPOL                     (0x1U << 3)                 /*!< Clock polarity */
#define SPI_CR_MSTR                     (0x1U << 4)                 /*!< Master mode */
#define SPI_CR_LSB                      (0x1U << 5)                 /*!< LSB transmitted first */
#define SPI_CR_SPE                      (0x1U << 6)                 /*!< SPI enable */
#define SPI_CR_IE                       (0x1U << 7)                 /*!< SPI data transfer complete interrupt enable  */

/********************************  Bit definition for SPI_SR register  ********************************/
#define SPI_SR_WCOL                     (0x1U << 6)                 /*!< Write collision flag */
#define SPI_SR_SPIF                     (0x1U << 7)                 /*!< SPI data transfer complete flag */


/*------------------------------------------------------------------------------------------------------*/
/*---                               Analog to Digital Converter (ADC)                                ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for ADC_CR register  ********************************/
#define ADC_CR_ADEN                 (0x1U << 0)           /*!< ADC enable */
#define ADC_CR_CMPPOL               (0x1U << 1)           /*!< CMP polarity */
#define ADC_CR_TRGSRC               (0x1U << 2)           /*!< Start of conversion trigger source selection */
#define ADC_CR_SWSTART              (0x1U << 3)           /*!< Start conversion */
#define ADC_CR_CMPSEL               (0x1U << 4)           /*!< CMP selection */

#define ADC_CR_PREDIVEN             (0x1U << 16)          /*!< ADC clock prescaler divider enable */

#define ADC_CR_PRERATIO_Pos         (17U)
#define ADC_CR_PRERATIO_Msk         (0x3FU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_2           (0x0U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_3           (0x1U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_4           (0x2U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_5           (0x3U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_6           (0x4U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_7           (0x5U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_8           (0x6U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_9           (0x7U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_10          (0x8U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_11          (0x9U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_12          (0xAU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_13          (0xBU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_14          (0xCU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_15          (0xDU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_16          (0xEU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_17          (0xFU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_18          (0x10U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_19          (0x11U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_20          (0x12U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_21          (0x13U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_22          (0x14U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_23          (0x15U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_24          (0x16U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_25          (0x17U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_26          (0x18U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_27          (0x19U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_28          (0x1AU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_29          (0x1BU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_30          (0x1CU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_31          (0x1DU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_32          (0x1EU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_33          (0x1FU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_34          (0x20U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_35          (0x21U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_36          (0x22U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_37          (0x23U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_38          (0x24U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_39          (0x25U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_40          (0x26U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_41          (0x27U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_42          (0x28U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_43          (0x29U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_44          (0x2AU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_45          (0x2BU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_46          (0x2CU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_47          (0x2DU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_48          (0x2EU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_49          (0x2FU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_50          (0x30U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_51          (0x31U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_52          (0x32U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_53          (0x33U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_54          (0x34U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_55          (0x35U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_56          (0x36U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_57          (0x37U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_58          (0x38U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_59          (0x39U << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_60          (0x3AU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_61          (0x3BU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_62          (0x3CU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_63          (0x3DU << ADC_CR_PRERATIO_Pos)
#define ADC_CR_PRERATIO_64          (0x3EU << ADC_CR_PRERATIO_Pos)

#define ADC_CR_PRESRCEN             (0x1U << 23)          /*!< ADC clock prescaler input enable */


#define ADC_CR_SMP_Pos              (24U)
#define ADC_CR_SMP_Msk              (0x3U << ADC_CR_SMP_Pos)     /*!< SMP[1:0] bits */
#define ADC_CR_SMP_32               (0x0U << ADC_CR_SMP_Pos)
#define ADC_CR_SMP_64               (0x1U << ADC_CR_SMP_Pos)
#define ADC_CR_SMP_128              (0x2U << ADC_CR_SMP_Pos)
#define ADC_CR_SMP_256              (0x3U << ADC_CR_SMP_Pos)

/******************************** Bit definition for ADC_ISR register  ********************************/
#define ADC_ISR_EOC                 (0x1U << 0)           /*!< End of conversion */
#define ADC_ISR_OVF                 (0x1U << 1)           /*!< Overflow */
#define ADC_ISR_BUSY                (0x1U << 2)           /*!< Busy */

/******************************** Bit definition for ADC_IER register  ********************************/
#define ADC_IER_EOCIE               (0x1U << 0)           /*!< End of conversion interrupt enable */
#define ADC_IER_OVFIE               (0x1U << 1)           /*!< Overflow interrupt enable */

/******************************** Bit definition for ADC_ICR register  ********************************/
#define ADC_ICR_EOCIC               (0x1U << 0)           /*!< End of conversion interrupt clear */
#define ADC_ICR_OVFIC               (0x1U << 1)           /*!< Overflow interrupt clear */




/*------------------------------------------------------------------------------------------------------*/
/*---                                   Universal Serial Bus (USB)                                   ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for USB_FADDR register  ********************************/
#define USB_FADDR_FADDR_Msk       (0x7FU)              /*!< The function address field mask */
#define USB_FADDR_UPDATE          (0x1U << 7)          /*!< Set when FAddr is written */

/********************************  Bit definition for USB_POWER register  ********************************/
#define USB_POWER_SUSEN           (0x1U << 0)          /*!< No description */
#define USB_POWER_SUSMD           (0x1U << 1)          /*!< No description */
#define USB_POWER_RESUME          (0x1U << 2)          /*!< No description */
#define USB_POWER_USBRST          (0x1U << 3)          /*!< No description */
#define USB_POWER_ISOUD           (0x1U << 7)          /*!< No description */

/********************************  Bit definition for USB_INTRIN register  ********************************/
#define USB_INTRIN_EP0            (0x1U << 0)          /*!< No description */
#define USB_INTRIN_IN1            (0x1U << 1)          /*!< No description */
#define USB_INTRIN_IN2            (0x1U << 2)          /*!< No description */
#define USB_INTRIN_IN3            (0x1U << 3)          /*!< No description */
#define USB_INTRIN_IN4            (0x1U << 4)          /*!< No description */

/********************************  Bit definition for USB_INTROUT register  ********************************/
#define USB_INTROUT_OUT1          (0x1U << 1)          /*!< No description */
#define USB_INTROUT_OUT2          (0x1U << 2)          /*!< No description */
#define USB_INTROUT_OUT3          (0x1U << 3)          /*!< No description */
#define USB_INTROUT_OUT4          (0x1U << 4)          /*!< No description */

/********************************  Bit definition for USB_INTRUSB register  ********************************/
#define USB_INTRUSB_SUSIS         (0x1U << 0)          /*!< No description */
#define USB_INTRUSB_RSUIS         (0x1U << 1)          /*!< No description */
#define USB_INTRUSB_RSTIS         (0x1U << 2)          /*!< No description */
#define USB_INTRUSB_SOFIS         (0x1U << 3)          /*!< No description */

/********************************  Bit definition for USB_INTRINE register  ********************************/
#define USB_INTRINE_EP0E          (0x1U << 0)          /*!< No description */
#define USB_INTRINE_IN1E          (0x1U << 1)          /*!< No description */
#define USB_INTRINE_IN2E          (0x1U << 2)          /*!< No description */
#define USB_INTRINE_IN3E          (0x1U << 3)          /*!< No description */
#define USB_INTRINE_IN4E          (0x1U << 4)          /*!< No description */

/********************************  Bit definition for USB_INTROUTE register  ********************************/
#define USB_INTROUTE_OUT1E        (0x1U << 1)          /*!< No description */
#define USB_INTROUTE_OUT2E        (0x1U << 2)          /*!< No description */
#define USB_INTROUTE_OUT3E        (0x1U << 3)          /*!< No description */
#define USB_INTROUTE_OUT4E        (0x1U << 4)          /*!< No description */

/********************************  Bit definition for USB_INTRUSBE register  ********************************/
#define USB_INTRUSBE_SUSIE        (0x1U << 0)          /*!< No description */
#define USB_INTRUSBE_RSUIE        (0x1U << 1)          /*!< No description */
#define USB_INTRUSBE_RSTIE        (0x1U << 2)          /*!< No description */
#define USB_INTRUSBE_SOFIE        (0x1U << 3)          /*!< No description */

/********************************  Bit definition for USB_CSR0 register  ********************************/
#define USB_CSR0_OUTPKTRDY        (0x1U << 0)          /*!< No description */
#define USB_CSR0_INPKTRDY         (0x1U << 1)          /*!< No description */
#define USB_CSR0_SENTSTALL        (0x1U << 2)          /*!< No description */
#define USB_CSR0_DATAEND          (0x1U << 3)          /*!< No description */
#define USB_CSR0_SETUPEND         (0x1U << 4)          /*!< No description */
#define USB_CSR0_SENDSTALL        (0x1U << 5)          /*!< No description */
#define USB_CSR0_SVDOUTPKTRDY     (0x1U << 6)          /*!< No description */
#define USB_CSR0_SVDSETUPEND      (0x1U << 7)          /*!< No description */

/********************************  Bit definition for USB_INCSR1 register  ********************************/
#define USB_INCSR1_INPKTRDY       (0x1U << 0)          /*!< No description */
#define USB_INCSR1_FIFONE         (0x1U << 1)          /*!< No description */
#define USB_INCSR1_UNDERRUN       (0x1U << 2)          /*!< No description */
#define USB_INCSR1_FLUSHFIFO      (0x1U << 3)          /*!< No description */
#define USB_INCSR1_SENDSTALL      (0x1U << 4)          /*!< No description */
#define USB_INCSR1_SENTSTALL      (0x1U << 5)          /*!< No description */
#define USB_INCSR1_CLRDATATOG     (0x1U << 6)          /*!< No description */

/********************************  Bit definition for USB_INCSR2 register  ********************************/
#define USB_INCSR2_FRCDATATOG     (0x1U << 3)          /*!< No description */
#define USB_INCSR2_DMAEN          (0x1U << 4)          /*!< No description */
#define USB_INCSR2_DIRSEL         (0x1U << 5)          /*!< No description */
#define USB_INCSR2_ISO            (0x1U << 6)          /*!< No description */
#define USB_INCSR2_AUTOSET        (0x1U << 7)          /*!< No description */

/********************************  Bit definition for USB_OUTCSR1 register  ********************************/
#define USB_OUTCSR1_OUTPKTRDY     (0x1U << 0)          /*!< No description */
#define USB_OUTCSR1_FIFOFULL      (0x1U << 1)          /*!< No description */
#define USB_OUTCSR1_OVERRUN       (0x1U << 2)          /*!< No description */
#define USB_OUTCSR1_DATAERROR     (0x1U << 3)          /*!< No description */
#define USB_OUTCSR1_FLUSHFIFO     (0x1U << 4)          /*!< No description */
#define USB_OUTCSR1_SENDSTALL     (0x1U << 5)          /*!< No description */
#define USB_OUTCSR1_SENTSTALL     (0x1U << 6)          /*!< No description */
#define USB_OUTCSR1_CLRDATATOG    (0x1U << 7)          /*!< No description */

/********************************  Bit definition for USB_OUTCSR2 register  ********************************/
#define USB_OUTCSR2_DMAMODE       (0x1U << 4)          /*!< No description */
#define USB_OUTCSR2_DMAEN         (0x1U << 5)          /*!< No description */
#define USB_OUTCSR2_ISO           (0x1U << 6)          /*!< No description */
#define USB_OUTCSR2_AUTOCLR       (0x1U << 7)          /*!< No description */



/*------------------------------------------------------------------------------------------------------*/
/*---                             Addressable RGB LED Controller (ARGB)                              ---*/
/*------------------------------------------------------------------------------------------------------*/
/******************************** Bit definition for ARGB_CTRL register  ********************************/
#define ARGB_CTRL_ENABLE              (0x1U << 0)           /*!< ARGB Enable */
#define ARGB_CTRL_TFEIE               (0x1U << 1)           /*!< TX FIFO empty interrupt enable */
#define ARGB_CTRL_FIFOR               (0x1U << 2)           /*!< TX FIFO Reset */
#define ARGB_CTRL_GRSTIE              (0x1U << 6)           /*!< Generate reset code finish interrupt enable */
#define ARGB_CTRL_GRST                (0x1U << 7)           /*!< Trigger generate reset code */

/******************************** Bit definition for ARGB_STATUS register  ********************************/
#define ARGB_STATUS_EMPTY             (0x1U << 0)           /*!< TX FIFO empty */
#define ARGB_STATUS_FULL              (0x1U << 1)           /*!< TX FIFO full */
#define ARGB_STATUS_GRSTF             (0x1U << 6)           /*!< Generate reset code finish flag */
#define ARGB_STATUS_RSTS              (0x1U << 7)           /*!< Reset code detected */



/*------------------------------------------------------------------------------------------------------*/
/*---                                       Debug MCU (DBGMCU)                                       ---*/
/*------------------------------------------------------------------------------------------------------*/
/********************************  Bit definition for DBGMCU_CR register  ********************************/
#define DBGMCU_CR_DBG_PCT0_STOP         (0x1U << 0)         /*!< PCT0 counter stopped when core is halted */
#define DBGMCU_CR_DBG_PCT1_STOP         (0x1U << 1)         /*!< PCT1 counter stopped when core is halted */
#define DBGMCU_CR_DBG_PCT2_STOP         (0x1U << 2)         /*!< PCT2 counter stopped when core is halted */
#define DBGMCU_CR_DBG_PCT3_STOP         (0x1U << 3)         /*!< PCT3 counter stopped when core is halted */
#define DBGMCU_CR_DBG_PCT4_STOP         (0x1U << 4)         /*!< PCT4 counter stopped when core is halted */
#define DBGMCU_CR_DBG_TIM6_STOP         (0x1U << 5)         /*!< TIM6 counter stopped when core is halted */
#define DBGMCU_CR_DBG_RTC_STOP          (0x1U << 6)         /*!< RTC counter stopped when core is halted */
#define DBGMCU_CR_DBG_IWDG_STOP         (0x1U << 7)         /*!< Debug Independent Watchdog stopped when core is halted */



#ifdef USE_STDPERIPH_DRIVER
  #include "wb7720_conf.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_H__ */

/**
  * @}
  */

/**
  * @}
  */
