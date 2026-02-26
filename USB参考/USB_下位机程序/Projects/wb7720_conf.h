/**
  * @file    Projects/WB7720_StdPeriph_Template/wb7720_conf.h
  * @author  Westberry Application Team
  * @version V0.1.1
  * @date    13-January-2025
  * @brief   Library configuration file.
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_CONF_H
#define __WB7720_CONF_H

/* Includes ------------------------------------------------------------------*/
/* Uncomment/Comment the line below to enable/disable peripheral header file inclusion */
#include "wb7720_misc.h"
#include "wb7720_anctl.h"
#include "wb7720_argb.h"
#include "wb7720_crc.h"
#include "wb7720_crs.h"
#include "wb7720_exti.h"
#include "wb7720_flash.h"
#include "wb7720_gpio.h"
#include "wb7720_i2c.h"
#include "wb7720_iwdg.h"
#include "wb7720_pct.h"
#include "wb7720_pwr.h"
#include "wb7720_rcc.h"
#include "wb7720_rtc.h"
#include "wb7720_spi.h"
#include "wb7720_spim.h"
#include "wb7720_tim.h"
#include "wb7720_uart0.h"
#include "wb7720_uart1.h"
#include "wb7720_syscfg.h"
#include "wb7720_adc.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Uncomment the line below to expanse the "assert_param" macro in the 
   Standard Peripheral Library drivers code */
/* #define USE_FULL_ASSERT    1 */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

#endif /* __WB7720_CONF_H */
