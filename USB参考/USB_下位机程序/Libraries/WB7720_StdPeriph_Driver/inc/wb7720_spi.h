/**
 * @file    wb7720_spi.h
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file contains all the functions prototypes for the SPI firmware
 *          library.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WB7720_SPI_H
#define __WB7720_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "wb7720.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @addtogroup SPI
  * @{
  */

/* Exported types ------------------------------------------------------------*/

/** 
  * @brief  SPI Init structure definition  
  */
typedef struct
{
  uint8_t SPI_Mode;                 /*!< Specifies the SPI mode (Master/Slave).
                                         This parameter can be a value of @ref SPI_mode. */

  uint8_t SPI_CPOL;                 /*!< Specifies the serial clock polarity.
                                         This parameter can be a value of @ref SPI_Clock_Polarity */

  uint8_t SPI_CPHA;                 /*!< Specifies the clock active edge for the bit capture.
                                         This parameter can be a value of @ref SPI_Clock_Phase */

  uint8_t SPI_ClockRateDivider;     /*!< Specifies the clock rate divider value which will be used to
                                         configure the transmit and receive SCK clock.
                                         This parameter can be a value of @ref SPI_ClockRate_Divider */

  uint8_t SPI_FirstBit;             /*!< Specifies whether data transfers start from MSB or LSB bit.
                                         This parameter can be a value of @ref SPI_MSB_LSB_transmission */
} SPI_InitTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup SPI_Exported_Constants
  * @{
  */

/** @defgroup SPI_mode 
  * @{
  */
#define SPI_Mode_Master         ((uint8_t)SPI_CR_MSTR)
#define SPI_Mode_Slave          ((uint8_t)0x00)
/**
  * @}
  */


/** @defgroup SPI_Clock_Polarity 
  * @{
  */
#define SPI_CPOL_Low            ((uint8_t)0x00)
#define SPI_CPOL_High           ((uint8_t)SPI_CR_CPOL)
/**
  * @}
  */


/** @defgroup SPI_Clock_Phase 
  * @{
  */
#define SPI_CPHA_1Edge          ((uint8_t)0x00)
#define SPI_CPHA_2Edge          ((uint8_t)SPI_CR_CPHA)
/**
  * @}
  */


/** @defgroup SPI_MSB_LSB_transmission 
  * @{
  */
#define SPI_FirstBit_MSB        ((uint8_t)0x00)
#define SPI_FirstBit_LSB        ((uint8_t)SPI_CR_LSB)
/**
  * @}
  */


/** @defgroup SPI_ClockRate_Divider 
  * @{
  */
#define SPI_ClockRateDivider_4         ((uint8_t)0x10)
#define SPI_ClockRateDivider_8         ((uint8_t)0x00)
#define SPI_ClockRateDivider_16        ((uint8_t)0x11)
#define SPI_ClockRateDivider_32        ((uint8_t)0x01)
#define SPI_ClockRateDivider_64        ((uint8_t)0x12)
#define SPI_ClockRateDivider_128       ((uint8_t)0x02)
#define SPI_ClockRateDivider_256       ((uint8_t)0x03)
/**
  * @}
  */


/** @defgroup SPI_flags_definition 
  * @{
  */
#define SPI_FLAG_WCOL   ((uint8_t)SPI_SR_WCOL)
#define SPI_FLAG_SPIF   ((uint8_t)SPI_SR_SPIF)
/**
  * @}
  */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void SPI_DeInit(SPI_TypeDef* SPIx);
void SPI_Init(SPI_TypeDef* SPIx, SPI_InitTypeDef* SPI_InitStruct);
void SPI_StructInit(SPI_InitTypeDef* SPI_InitStruct);
void SPI_Cmd(SPI_TypeDef* SPIx, FunctionalState NewState);
uint8_t SPI_ReadData(SPI_TypeDef* SPIx);
void SPI_WriteData(SPI_TypeDef* SPIx, uint8_t Data);
void SPI_ITConfig(SPI_TypeDef* SPIx, FunctionalState NewState);
FlagStatus SPI_GetFlagStatus(SPI_TypeDef* SPIx, uint8_t SPI_FLAG);
void SPI_ClearFlag(SPI_TypeDef* SPIx, uint8_t SPI_FLAG);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __WB7720_SPI_H */
