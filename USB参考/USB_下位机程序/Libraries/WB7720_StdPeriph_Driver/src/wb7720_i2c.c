/**
 * @file    wb7720_i2c.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the I2C firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_i2c.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup I2C
  * @brief I2C driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup I2C_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes the I2Cx peripheral registers to their default reset values.
 * @param  None
 * @return None
 */
void I2C_DeInit(void)
{
    RCC_APBPeriphResetCmd(RCC_APBPeriph_I2C, ENABLE);
    RCC_APBPeriphResetCmd(RCC_APBPeriph_I2C, DISABLE);
}

/**
 * @brief  Initializes the I2C peripheral according to the specified 
 * @param  I2C_Clock_Div: Configure the clock frequency for the I2C.
 *         This parameter can be one of the values of the I2C_ClockRate_Divider.
 * @return None
 */
void I2C_Init(uint8_t I2C_Clock_Div)
{
  I2C->CON = I2C_Clock_Div | I2C_CON_I2CEN;
}


/**
 * @brief  Configures the specified I2C own address.
 * @param  Address: specifies the I2C own address.
 * @return None
 */
void I2C_OwnAddressConfig(uint8_t Address)
{
  I2C->ADR = Address;
}

/**
 * @brief  Enables or disables the specified I2C peripheral.
 * @param  NewState: new state of the I2C peripheral.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void I2C_Cmd(FunctionalState NewState)
{
  if (NewState != DISABLE) {
    I2C->CON |= I2C_CON_I2CEN;
  }
  else {
    I2C->CON &= ~I2C_CON_I2CEN;
  }
}

/**
 * @brief  Read data from DAT register.
 * @param  None
 * @return The received data.
 */
uint8_t I2C_ReadData(void)
{
  return (uint8_t)I2C->DAT;
}

/**
 * @brief  Write data to DAT register..
 * @param  data: Data written to DAT.
 * @return None.
 */
void I2C_WriteData(uint8_t data)
{
  I2C->DAT = data;
}


/**
 * @brief  Gets the current I2C bus status.
 * @param  None
 * @return Now status of I2C bus.
 */
uint8_t I2C_GetBusStatus(void)
{
  return  I2C->STAT;
}

/**
 * @brief  Gets the I2C SI suspension flag.
 * @param  None
 * @return I2C_SI flag.
 */
uint8_t I2C_GetSIFlag(void)
{
  return  I2C->CON & I2C_CON_SI;
}

/**
 * @brief  I2C tranmission start.
 * @param  None
 * @return None
 */
void I2C_Start(void)
{
  I2C->CON = (I2C->CON & ~I2C_CON_SI) | I2C_CON_STA;
}


/**
 * @brief  Enable or disable I2C function, Write 0x0 close all fubction.
 * @param  I2C_Con: Functions to be enabled.
 *         This parameter can be a combination of the following values:
 *         @arg @ref I2C_CON_AA
 *         @arg @ref I2C_CON_SI
 *         @arg @ref I2C_CON_STO
 *         @arg @ref I2C_CON_STA

 * @return None.
 */
void I2C_Config(uint8_t I2C_Con)
{
  I2C->CON = I2C_Con | (I2C->CON & ~0x3C);
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
