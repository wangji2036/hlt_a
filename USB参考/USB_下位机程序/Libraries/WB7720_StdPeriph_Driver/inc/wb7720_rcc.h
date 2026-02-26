/**
 * @file    wb7720_rcc.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the RCC firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_RCC_H
#define __WB7720_RCC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup RCC
  * @{
  */

/* Exported types ------------------------------------------------------------*/

typedef struct
{
  uint32_t SYSCLK_Frequency;    /*!< returns SYSCLK clock frequency expressed in Hz */
  uint32_t APBCLK_Frequency;    /*!< returns APBCLK clock frequency expressed in Hz */
} RCC_ClocksTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup RCC_Exported_Constants 
  * @{
  */

/** @defgroup SYSCLK_source 
  * @{
  */
#define RCC_SYSCLKSource_HSI2           ((uint32_t)0x00000000)
#define RCC_SYSCLKSource_HSI48_DIV6     ((uint32_t)0x00000001)
#define RCC_SYSCLKSource_HSI48          ((uint32_t)0x00000002)
#define RCC_SYSCLKSource_HSE            ((uint32_t)0x00000003)
#define RCC_SYSCLKSource_PLL            ((uint32_t)0x00000004)
/**
  * @}
  */


/** @defgroup RCC_SYSCLKPrescaler 
  * @{
  */
#define RCC_SYSCLKPrescaler_Div1        ((uint32_t)0x00000000)
#define RCC_SYSCLKPrescaler_Div2        ((uint32_t)0x00000001)
#define RCC_SYSCLKPrescaler_Div3        ((uint32_t)0x00000003)
#define RCC_SYSCLKPrescaler_Div4        ((uint32_t)0x00000005)
#define RCC_SYSCLKPrescaler_Div5        ((uint32_t)0x00000007)
#define RCC_SYSCLKPrescaler_Div6        ((uint32_t)0x00000009)
#define RCC_SYSCLKPrescaler_Div7        ((uint32_t)0x0000000B)
#define RCC_SYSCLKPrescaler_Div8        ((uint32_t)0x0000000D)
#define RCC_SYSCLKPrescaler_Div9        ((uint32_t)0x0000000F)
#define RCC_SYSCLKPrescaler_Div10       ((uint32_t)0x00000011)
#define RCC_SYSCLKPrescaler_Div11       ((uint32_t)0x00000013)
#define RCC_SYSCLKPrescaler_Div12       ((uint32_t)0x00000015)
#define RCC_SYSCLKPrescaler_Div13       ((uint32_t)0x00000017)
#define RCC_SYSCLKPrescaler_Div14       ((uint32_t)0x00000019)
#define RCC_SYSCLKPrescaler_Div15       ((uint32_t)0x0000001B)
#define RCC_SYSCLKPrescaler_Div16       ((uint32_t)0x0000001D)
#define RCC_SYSCLKPrescaler_Div17       ((uint32_t)0x0000001F)
#define RCC_SYSCLKPrescaler_Div18       ((uint32_t)0x00000021)
#define RCC_SYSCLKPrescaler_Div19       ((uint32_t)0x00000023)
#define RCC_SYSCLKPrescaler_Div20       ((uint32_t)0x00000025)
#define RCC_SYSCLKPrescaler_Div21       ((uint32_t)0x00000027)
#define RCC_SYSCLKPrescaler_Div22       ((uint32_t)0x00000029)
#define RCC_SYSCLKPrescaler_Div23       ((uint32_t)0x0000002B)
#define RCC_SYSCLKPrescaler_Div24       ((uint32_t)0x0000002D)
#define RCC_SYSCLKPrescaler_Div25       ((uint32_t)0x0000002F)
#define RCC_SYSCLKPrescaler_Div26       ((uint32_t)0x00000031)
#define RCC_SYSCLKPrescaler_Div27       ((uint32_t)0x00000033)
#define RCC_SYSCLKPrescaler_Div28       ((uint32_t)0x00000035)
#define RCC_SYSCLKPrescaler_Div29       ((uint32_t)0x00000037)
#define RCC_SYSCLKPrescaler_Div30       ((uint32_t)0x00000039)
#define RCC_SYSCLKPrescaler_Div31       ((uint32_t)0x0000003B)
#define RCC_SYSCLKPrescaler_Div32       ((uint32_t)0x0000003D)
#define RCC_SYSCLKPrescaler_Div33       ((uint32_t)0x0000003F)
#define RCC_SYSCLKPrescaler_Div34       ((uint32_t)0x00000041)
#define RCC_SYSCLKPrescaler_Div35       ((uint32_t)0x00000043)
#define RCC_SYSCLKPrescaler_Div36       ((uint32_t)0x00000045)
#define RCC_SYSCLKPrescaler_Div37       ((uint32_t)0x00000047)
#define RCC_SYSCLKPrescaler_Div38       ((uint32_t)0x00000049)
#define RCC_SYSCLKPrescaler_Div39       ((uint32_t)0x0000004B)
#define RCC_SYSCLKPrescaler_Div40       ((uint32_t)0x0000004D)
#define RCC_SYSCLKPrescaler_Div41       ((uint32_t)0x0000004F)
#define RCC_SYSCLKPrescaler_Div42       ((uint32_t)0x00000051)
#define RCC_SYSCLKPrescaler_Div43       ((uint32_t)0x00000053)
#define RCC_SYSCLKPrescaler_Div44       ((uint32_t)0x00000055)
#define RCC_SYSCLKPrescaler_Div45       ((uint32_t)0x00000057)
#define RCC_SYSCLKPrescaler_Div46       ((uint32_t)0x00000059)
#define RCC_SYSCLKPrescaler_Div47       ((uint32_t)0x0000005B)
#define RCC_SYSCLKPrescaler_Div48       ((uint32_t)0x0000005D)
#define RCC_SYSCLKPrescaler_Div49       ((uint32_t)0x0000005F)
#define RCC_SYSCLKPrescaler_Div50       ((uint32_t)0x00000061)
#define RCC_SYSCLKPrescaler_Div51       ((uint32_t)0x00000063)
#define RCC_SYSCLKPrescaler_Div52       ((uint32_t)0x00000065)
#define RCC_SYSCLKPrescaler_Div53       ((uint32_t)0x00000067)
#define RCC_SYSCLKPrescaler_Div54       ((uint32_t)0x00000069)
#define RCC_SYSCLKPrescaler_Div55       ((uint32_t)0x0000006B)
#define RCC_SYSCLKPrescaler_Div56       ((uint32_t)0x0000006D)
#define RCC_SYSCLKPrescaler_Div57       ((uint32_t)0x0000006F)
#define RCC_SYSCLKPrescaler_Div58       ((uint32_t)0x00000071)
#define RCC_SYSCLKPrescaler_Div59       ((uint32_t)0x00000073)
#define RCC_SYSCLKPrescaler_Div60       ((uint32_t)0x00000075)
#define RCC_SYSCLKPrescaler_Div61       ((uint32_t)0x00000077)
#define RCC_SYSCLKPrescaler_Div62       ((uint32_t)0x00000079)
#define RCC_SYSCLKPrescaler_Div63       ((uint32_t)0x0000007B)
#define RCC_SYSCLKPrescaler_Div64       ((uint32_t)0x0000007D)
/**
  * @}
  */


/** @defgroup USB_clock_source 
  * @{
  */
#define RCC_USBCLKSource_None           ((uint32_t)0x00000000)
#define RCC_USBCLKSource_HSI48          ((uint32_t)0x00000001)
#define RCC_USBCLKSource_PLL2USB        ((uint32_t)0x00000002)
/**
  * @}
  */


/** @defgroup Clock_source_to_output_on_MCO_pin 
  * @{
  */
#define RCC_MCOSource_LSI               ((uint32_t)0x00000000)
#define RCC_MCOSource_PLL               ((uint32_t)0x00000002)
#define RCC_MCOSource_HSI48             ((uint32_t)0x00000003)
#define RCC_MCOSource_SYSCLKSRC         ((uint32_t)0x00000004)
/**
  * @}
  */


/** @defgroup RCC_MCOPrescaler 
  * @{
  */
#define RCC_MCOPrescaler_Div1           ((uint32_t)0x00000000)
#define RCC_MCOPrescaler_Div2           ((uint32_t)0x00000001)
#define RCC_MCOPrescaler_Div3           ((uint32_t)0x00000003)
#define RCC_MCOPrescaler_Div4           ((uint32_t)0x00000005)
#define RCC_MCOPrescaler_Div5           ((uint32_t)0x00000007)
#define RCC_MCOPrescaler_Div6           ((uint32_t)0x00000009)
#define RCC_MCOPrescaler_Div7           ((uint32_t)0x0000000B)
#define RCC_MCOPrescaler_Div8           ((uint32_t)0x0000000D)
#define RCC_MCOPrescaler_Div9           ((uint32_t)0x0000000F)
#define RCC_MCOPrescaler_Div10          ((uint32_t)0x00000011)
#define RCC_MCOPrescaler_Div11          ((uint32_t)0x00000013)
#define RCC_MCOPrescaler_Div12          ((uint32_t)0x00000015)
#define RCC_MCOPrescaler_Div13          ((uint32_t)0x00000017)
#define RCC_MCOPrescaler_Div14          ((uint32_t)0x00000019)
#define RCC_MCOPrescaler_Div15          ((uint32_t)0x0000001B)
#define RCC_MCOPrescaler_Div16          ((uint32_t)0x0000001D)
#define RCC_MCOPrescaler_Div17          ((uint32_t)0x0000001F)
#define RCC_MCOPrescaler_Div18          ((uint32_t)0x00000021)
#define RCC_MCOPrescaler_Div19          ((uint32_t)0x00000023)
#define RCC_MCOPrescaler_Div20          ((uint32_t)0x00000025)
#define RCC_MCOPrescaler_Div21          ((uint32_t)0x00000027)
#define RCC_MCOPrescaler_Div22          ((uint32_t)0x00000029)
#define RCC_MCOPrescaler_Div23          ((uint32_t)0x0000002B)
#define RCC_MCOPrescaler_Div24          ((uint32_t)0x0000002D)
#define RCC_MCOPrescaler_Div25          ((uint32_t)0x0000002F)
#define RCC_MCOPrescaler_Div26          ((uint32_t)0x00000031)
#define RCC_MCOPrescaler_Div27          ((uint32_t)0x00000033)
#define RCC_MCOPrescaler_Div28          ((uint32_t)0x00000035)
#define RCC_MCOPrescaler_Div29          ((uint32_t)0x00000037)
#define RCC_MCOPrescaler_Div30          ((uint32_t)0x00000039)
#define RCC_MCOPrescaler_Div31          ((uint32_t)0x0000003B)
#define RCC_MCOPrescaler_Div32          ((uint32_t)0x0000003D)
#define RCC_MCOPrescaler_Div33          ((uint32_t)0x0000003F)
#define RCC_MCOPrescaler_Div34          ((uint32_t)0x00000041)
#define RCC_MCOPrescaler_Div35          ((uint32_t)0x00000043)
#define RCC_MCOPrescaler_Div36          ((uint32_t)0x00000045)
#define RCC_MCOPrescaler_Div37          ((uint32_t)0x00000047)
#define RCC_MCOPrescaler_Div38          ((uint32_t)0x00000049)
#define RCC_MCOPrescaler_Div39          ((uint32_t)0x0000004B)
#define RCC_MCOPrescaler_Div40          ((uint32_t)0x0000004D)
#define RCC_MCOPrescaler_Div41          ((uint32_t)0x0000004F)
#define RCC_MCOPrescaler_Div42          ((uint32_t)0x00000051)
#define RCC_MCOPrescaler_Div43          ((uint32_t)0x00000053)
#define RCC_MCOPrescaler_Div44          ((uint32_t)0x00000055)
#define RCC_MCOPrescaler_Div45          ((uint32_t)0x00000057)
#define RCC_MCOPrescaler_Div46          ((uint32_t)0x00000059)
#define RCC_MCOPrescaler_Div47          ((uint32_t)0x0000005B)
#define RCC_MCOPrescaler_Div48          ((uint32_t)0x0000005D)
#define RCC_MCOPrescaler_Div49          ((uint32_t)0x0000005F)
#define RCC_MCOPrescaler_Div50          ((uint32_t)0x00000061)
#define RCC_MCOPrescaler_Div51          ((uint32_t)0x00000063)
#define RCC_MCOPrescaler_Div52          ((uint32_t)0x00000065)
#define RCC_MCOPrescaler_Div53          ((uint32_t)0x00000067)
#define RCC_MCOPrescaler_Div54          ((uint32_t)0x00000069)
#define RCC_MCOPrescaler_Div55          ((uint32_t)0x0000006B)
#define RCC_MCOPrescaler_Div56          ((uint32_t)0x0000006D)
#define RCC_MCOPrescaler_Div57          ((uint32_t)0x0000006F)
#define RCC_MCOPrescaler_Div58          ((uint32_t)0x00000071)
#define RCC_MCOPrescaler_Div59          ((uint32_t)0x00000073)
#define RCC_MCOPrescaler_Div60          ((uint32_t)0x00000075)
#define RCC_MCOPrescaler_Div61          ((uint32_t)0x00000077)
#define RCC_MCOPrescaler_Div62          ((uint32_t)0x00000079)
#define RCC_MCOPrescaler_Div63          ((uint32_t)0x0000007B)
#define RCC_MCOPrescaler_Div64          ((uint32_t)0x0000007D)
/**
  * @}
  */


/** @defgroup FCU_clock_source 
  * @{
  */
#define RCC_FCUCLKSource_None           ((uint32_t)0x00000000)
#define RCC_FCUCLKSource_HSI48          ((uint32_t)0x00000001)
#define RCC_FCUCLKSource_PLL            ((uint32_t)0x00000002)
/**
  * @}
  */


/** @defgroup RCC_FCUCLKPrescaler 
  * @{
  */
#define RCC_FCUCLKPrescaler_Div1        ((uint32_t)0x00000000)
#define RCC_FCUCLKPrescaler_Div2        ((uint32_t)0x00000001)
#define RCC_FCUCLKPrescaler_Div3        ((uint32_t)0x00000003)
#define RCC_FCUCLKPrescaler_Div4        ((uint32_t)0x00000005)
#define RCC_FCUCLKPrescaler_Div5        ((uint32_t)0x00000007)
#define RCC_FCUCLKPrescaler_Div6        ((uint32_t)0x00000009)
#define RCC_FCUCLKPrescaler_Div7        ((uint32_t)0x0000000B)
#define RCC_FCUCLKPrescaler_Div8        ((uint32_t)0x0000000D)
#define RCC_FCUCLKPrescaler_Div9        ((uint32_t)0x0000000F)
#define RCC_FCUCLKPrescaler_Div10       ((uint32_t)0x00000011)
#define RCC_FCUCLKPrescaler_Div11       ((uint32_t)0x00000013)
#define RCC_FCUCLKPrescaler_Div12       ((uint32_t)0x00000015)
#define RCC_FCUCLKPrescaler_Div13       ((uint32_t)0x00000017)
#define RCC_FCUCLKPrescaler_Div14       ((uint32_t)0x00000019)
#define RCC_FCUCLKPrescaler_Div15       ((uint32_t)0x0000001B)
#define RCC_FCUCLKPrescaler_Div16       ((uint32_t)0x0000001D)
#define RCC_FCUCLKPrescaler_Div17       ((uint32_t)0x0000001F)
#define RCC_FCUCLKPrescaler_Div18       ((uint32_t)0x00000021)
#define RCC_FCUCLKPrescaler_Div19       ((uint32_t)0x00000023)
#define RCC_FCUCLKPrescaler_Div20       ((uint32_t)0x00000025)
#define RCC_FCUCLKPrescaler_Div21       ((uint32_t)0x00000027)
#define RCC_FCUCLKPrescaler_Div22       ((uint32_t)0x00000029)
#define RCC_FCUCLKPrescaler_Div23       ((uint32_t)0x0000002B)
#define RCC_FCUCLKPrescaler_Div24       ((uint32_t)0x0000002D)
#define RCC_FCUCLKPrescaler_Div25       ((uint32_t)0x0000002F)
#define RCC_FCUCLKPrescaler_Div26       ((uint32_t)0x00000031)
#define RCC_FCUCLKPrescaler_Div27       ((uint32_t)0x00000033)
#define RCC_FCUCLKPrescaler_Div28       ((uint32_t)0x00000035)
#define RCC_FCUCLKPrescaler_Div29       ((uint32_t)0x00000037)
#define RCC_FCUCLKPrescaler_Div30       ((uint32_t)0x00000039)
#define RCC_FCUCLKPrescaler_Div31       ((uint32_t)0x0000003B)
#define RCC_FCUCLKPrescaler_Div32       ((uint32_t)0x0000003D)
#define RCC_FCUCLKPrescaler_Div33       ((uint32_t)0x0000003F)
#define RCC_FCUCLKPrescaler_Div34       ((uint32_t)0x00000041)
#define RCC_FCUCLKPrescaler_Div35       ((uint32_t)0x00000043)
#define RCC_FCUCLKPrescaler_Div36       ((uint32_t)0x00000045)
#define RCC_FCUCLKPrescaler_Div37       ((uint32_t)0x00000047)
#define RCC_FCUCLKPrescaler_Div38       ((uint32_t)0x00000049)
#define RCC_FCUCLKPrescaler_Div39       ((uint32_t)0x0000004B)
#define RCC_FCUCLKPrescaler_Div40       ((uint32_t)0x0000004D)
#define RCC_FCUCLKPrescaler_Div41       ((uint32_t)0x0000004F)
#define RCC_FCUCLKPrescaler_Div42       ((uint32_t)0x00000051)
#define RCC_FCUCLKPrescaler_Div43       ((uint32_t)0x00000053)
#define RCC_FCUCLKPrescaler_Div44       ((uint32_t)0x00000055)
#define RCC_FCUCLKPrescaler_Div45       ((uint32_t)0x00000057)
#define RCC_FCUCLKPrescaler_Div46       ((uint32_t)0x00000059)
#define RCC_FCUCLKPrescaler_Div47       ((uint32_t)0x0000005B)
#define RCC_FCUCLKPrescaler_Div48       ((uint32_t)0x0000005D)
#define RCC_FCUCLKPrescaler_Div49       ((uint32_t)0x0000005F)
#define RCC_FCUCLKPrescaler_Div50       ((uint32_t)0x00000061)
#define RCC_FCUCLKPrescaler_Div51       ((uint32_t)0x00000063)
#define RCC_FCUCLKPrescaler_Div52       ((uint32_t)0x00000065)
#define RCC_FCUCLKPrescaler_Div53       ((uint32_t)0x00000067)
#define RCC_FCUCLKPrescaler_Div54       ((uint32_t)0x00000069)
#define RCC_FCUCLKPrescaler_Div55       ((uint32_t)0x0000006B)
#define RCC_FCUCLKPrescaler_Div56       ((uint32_t)0x0000006D)
#define RCC_FCUCLKPrescaler_Div57       ((uint32_t)0x0000006F)
#define RCC_FCUCLKPrescaler_Div58       ((uint32_t)0x00000071)
#define RCC_FCUCLKPrescaler_Div59       ((uint32_t)0x00000073)
#define RCC_FCUCLKPrescaler_Div60       ((uint32_t)0x00000075)
#define RCC_FCUCLKPrescaler_Div61       ((uint32_t)0x00000077)
#define RCC_FCUCLKPrescaler_Div62       ((uint32_t)0x00000079)
#define RCC_FCUCLKPrescaler_Div63       ((uint32_t)0x0000007B)
#define RCC_FCUCLKPrescaler_Div64       ((uint32_t)0x0000007D)
/**
  * @}
  */


/** @defgroup APB_peripheral 
  * @{
  */
#define RCC_APBPeriph_CRC         (0x1U << 0)
#define RCC_APBPeriph_CRS         (0x1U << 1)
#define RCC_APBPeriph_UART0       (0x1U << 2)
#define RCC_APBPeriph_UART1       (0x1U << 3)
#define RCC_APBPeriph_ARGB03      (0x1U << 4)
#define RCC_APBPeriph_ARGB45      (0x1U << 5)
#define RCC_APBPeriph_IWDG        (0x1U << 6)
#define RCC_APBPeriph_RTC         (0x1U << 7)
#define RCC_APBPeriph_I2C         (0x1U << 8)
#define RCC_APBPeriph_GPIO        (0x1U << 9)
#define RCC_APBPeriph_TIM6        (0x1U << 10)
#define RCC_APBPeriph_SPIM        (0x1U << 11)
#define RCC_APBPeriph_SPI1        (0x1U << 12)
#define RCC_APBPeriph_PCT0        (0x1U << 13)
#define RCC_APBPeriph_PCT1        (0x1U << 14)
#define RCC_APBPeriph_PCT2        (0x1U << 15)
#define RCC_APBPeriph_PCT3        (0x1U << 16)
#define RCC_APBPeriph_PCT4        (0x1U << 17)
#define RCC_APBPeriph_ADC         (0x1U << 18)
#define RCC_APBPeriph_EXTI        (0x1U << 19)
#define RCC_APBPeriph_USB         (0x1U << 20)
/**
  * @}
  */


/** @defgroup RCC_SPI1_Slave_Clock_Path 
  * @{
  */
#define RCC_SPI1SlaveClockPath_None     ((uint32_t)0x00000000)
#define RCC_SPI1SlaveClockPath_PA6      ((uint32_t)0x00000001)
#define RCC_SPI1SlaveClockPath_PB12     ((uint32_t)0x00000002)
/**
  * @}
  */


/** @defgroup RCC_Reset_Flag 
  * @{
  */
#define RCC_RSTFLAG_LPWRRST     ((uint32_t)0x00000001)
#define RCC_RSTFLAG_IWDGRST     ((uint32_t)0x00000002)
#define RCC_RSTFLAG_SFTRST      ((uint32_t)0x00000004)
#define RCC_RSTFLAG_PORRST      ((uint32_t)0x00000008)
#define RCC_RSTFLAG_LVRRST      ((uint32_t)0x00000010)
#define RCC_RSTFLAG_PINRST      ((uint32_t)0x00000020)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void RCC_SYSCLKSourceConfig(uint32_t RCC_SYSCLKSource);
void RCC_SYSCLKPrescalerConfig(uint32_t RCC_SYSCLKPrescaler);
void RCC_USBCLKConfig(uint32_t RCC_USBCLKSource);
void RCC_MCOConfig(uint32_t RCC_MCOSource, uint32_t RCC_MCOPrescaler, FunctionalState NewState);
void RCC_FCUCLKConfig(uint32_t RCC_FCUCLKSource, uint32_t RCC_FCUCLKPrescaler);
void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks);
void RCC_APBPeriphClockCmd(uint32_t RCC_APBPeriph, FunctionalState NewState);
void RCC_APBPeriphResetCmd(uint32_t RCC_APBPeriph, FunctionalState NewState);
void RCC_SPI1SlaveClockPathConfig(uint32_t RCC_SPI1SlaveClockPath);
FlagStatus RCC_GetResetFlagStatus(uint32_t RCC_RSTFLAG);
void RCC_ClearResetFlags(void);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_RCC_H */
