/**
 * @file    wb7720_i2c.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the I2C firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_I2C_H
#define __WB7720_I2C_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup I2C
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/** @defgroup I2C_Exported_Constants
  * @{
  */

/**
  * @}
  */

/** @defgroup I2C_ClockRate_Divider 
  * @{
  */
#define I2C_ClockRateDivider_60        (I2C_CON_CRSEL_DIV60 )
#define I2C_ClockRateDivider_120       (I2C_CON_CRSEL_DIV120)
#define I2C_ClockRateDivider_160       (I2C_CON_CRSEL_DIV160)
#define I2C_ClockRateDivider_192       (I2C_CON_CRSEL_DIV192)
#define I2C_ClockRateDivider_224       (I2C_CON_CRSEL_DIV224)
#define I2C_ClockRateDivider_256       (I2C_CON_CRSEL_DIV256)
#define I2C_ClockRateDivider_960       (I2C_CON_CRSEL_DIV960)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void I2C_DeInit(void);
void I2C_Init(uint8_t I2C_Clock_Div);
void I2C_OwnAddressConfig(uint8_t Address);
void I2C_Cmd(FunctionalState NewState);
uint8_t I2C_ReadData(void);
void I2C_WriteData(uint8_t data);
uint8_t I2C_GetBusStatus(void);
uint8_t I2C_GetSIFlag(void);
void I2C_Start(void);
void I2C_Config(uint8_t I2C_Con);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_I2C_H */
