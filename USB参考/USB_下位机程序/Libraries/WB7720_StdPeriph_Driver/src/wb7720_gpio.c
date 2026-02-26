/**
 * @file    wb7720_gpio.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the GPIO firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_gpio.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup GPIO
  * @brief GPIO driver modules
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup GPIO_Private_Functions
  * @{
  */

/**
 * @brief  Deinitializes all GPIO peripheral registers to their default reset values.
 * @return None
 */
void GPIO_DeInit(void)
{
  RCC->APBRSTR |= RCC_APBRSTR_GPIORST;
  RCC->APBRSTR &= ~RCC_APBRSTR_GPIORST;
}

/**
 * @brief  Initializes the GPIOx peripheral according to the PinConfig.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be initialized.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @param  PinConfig: the port bit configuration.
 *         This parameter must be any combination of one GPIO_MODE_x flag, one GPIO_OTYPE_x flag,
 *         one GPIO_PUPD_x flag, one GPIO_SPEED_x flag and one GPIO_AFx flag.
 * @return None
 */
void GPIO_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t PinConfig)
{
  uint32_t tmp, pinmask, pinmask2, afl_mask, afh_mask;
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  pinmask = GPIO_Pin;
  pinmask2 = 0;
  afl_mask = 0;
  afh_mask = 0;
  for (tmp = 0 ; tmp < 0x10; tmp++)
  {
    if (pinmask & (0x1U << tmp))
    {
      pinmask2 |= 0x3U << (tmp << 1);
      if (tmp < 8) {
        afl_mask |= 0xFU << (tmp << 2);
      }
      else {
        afh_mask |= 0xFU << ((tmp-8) << 2);
      }
    }
  }

  GPIOx->OSPEEDR = (GPIOx->OSPEEDR & ~pinmask) | ((((PinConfig >> 20) & 0x1) * 0xFFFFFFFFU) & pinmask);
  GPIOx->OTYPER = (GPIOx->OTYPER & ~pinmask) | ((((PinConfig >> 24) & 0x1) * 0xFFFFFFFFU) & pinmask);
  GPIOx->MODER = (GPIOx->MODER & ~pinmask2) | ((((PinConfig >> 28) & 0x3) * 0x55555555U) & pinmask2);
  GPIOx->PUPDR = (GPIOx->PUPDR & ~pinmask2) | ((((PinConfig >> 16) & 0x3) * 0x55555555U) & pinmask2);

  tmp = (PinConfig & 0xF) * 0x11111111U;
  GPIOx->AFRL = (GPIOx->AFRL & ~afl_mask) | (tmp & afl_mask);
  GPIOx->AFRH = (GPIOx->AFRH & ~afh_mask) | (tmp & afh_mask);
}

/**
 * @brief  Configure gpio alternate function of any pins for a dedicated port.
 * @note   Possible values are from AF0 to AF15 depending on target.
 * @note   Warning: only one pin can be passed as parameter.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bits to configure.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @param  Alternate: This parameter can be GPIO_AFx where x can be (0..15).
 * @return None
 */
void GPIO_SetAFPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t Alternate)
{
  uint32_t tmp, afl_mask, afh_mask;

  afl_mask = 0;
  afh_mask = 0;
  for (tmp = 0 ; tmp < 0x10; tmp++)
  {
    if (GPIO_Pin & (0x1U << tmp))
    {
      if (tmp < 8) {
        afl_mask |= 0xFU << (tmp << 2);
      }
      else {
        afh_mask |= 0xFU << ((tmp-8) << 2);
      }
    }
  }

  tmp = Alternate * 0x11111111U;
  GPIOx->AFRL = (GPIOx->AFRL & ~afl_mask) | (tmp & afl_mask);
  GPIOx->AFRH = (GPIOx->AFRH & ~afh_mask) | (tmp & afh_mask);
}

/**
 * @brief  Reads the specified input port pin.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to read.
 *         This parameter can be GPIO_Pin_x where x can be (0..15).
 * @return The input port pin value.
 */
uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  uint8_t bitstatus = 0x00;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

  if ((GPIOx->IDR & GPIO_Pin) != (uint32_t)Bit_RESET)
  {
    bitstatus = (uint8_t)Bit_SET;
  }
  else
  {
    bitstatus = (uint8_t)Bit_RESET;
  }
  return bitstatus;
}

/**
 * @brief  Reads the specified GPIO input data port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @return GPIO input data port value.
 */
uint16_t GPIO_ReadInputData(GPIO_TypeDef* GPIOx)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

  return ((uint16_t)GPIOx->IDR);
}

/**
  * @brief  Reads the specified output data port bit.
  * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
  * @param  GPIO_Pin: specifies the port bit to read.
  *         This parameter can be GPIO_Pin_x where x can be (0..15).
  * @return The output port pin value.
  */
uint8_t GPIO_ReadOutputDataBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  uint8_t bitstatus = 0x00;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GET_GPIO_PIN(GPIO_Pin));

  if (((GPIOx->ODR) & GPIO_Pin) != (uint32_t)Bit_RESET)
  {
    bitstatus = (uint8_t)Bit_SET;
  }
  else
  {
    bitstatus = (uint8_t)Bit_RESET;
  }
  return bitstatus;
}

/**
 * @brief  Reads the specified GPIO output data port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @return GPIO output data port value.
 */
uint16_t GPIO_ReadOutputData(GPIO_TypeDef* GPIOx)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

  return ((uint16_t)GPIOx->ODR);
}

/**
 * @brief  Sets the selected data port bits.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bits to be written.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @return None
 */
void GPIO_SetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  GPIOx->BSRR = GPIO_Pin;
}

/**
 * @brief  Clears the selected data port bits.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bits to be written.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @return None
 */
void GPIO_ResetBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  GPIOx->BRR = GPIO_Pin;
}

/**
 * @brief  Sets or clears the selected data port bit.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bit to be written.
 *         This parameter can be one of GPIO_Pin_x where x can be (0..15).
 * @param  BitVal: specifies the value to be written to the selected bit.
 *         This parameter can be one of the BitAction enum values:
 *         @arg Bit_RESET: to clear the port pin
 *         @arg Bit_SET: to set the port pin
 * @return None
 */
void GPIO_WriteBit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, BitAction BitVal)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GET_GPIO_PIN(GPIO_Pin));
  assert_param(IS_GPIO_BIT_ACTION(BitVal));

  if (BitVal != Bit_RESET)
  {
    GPIOx->BSRR = GPIO_Pin;
  }
  else
  {
    GPIOx->BRR = GPIO_Pin;
  }
}

/**
 * @brief  Writes data to the specified GPIO data port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  PortVal: specifies the value to be written to the port output data register.
 * @return None
 */
void GPIO_Write(GPIO_TypeDef* GPIOx, uint16_t PortVal)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

  GPIOx->ODR = PortVal;
}

/**
 * @brief  Toggle data value for several pin of specified port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the pins to be toggled.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @return None
 */
void GPIO_ToggleBits(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));

  GPIOx->ODR ^= GPIO_Pin;
}

/**
 * @brief  Lock configuration of several pins for a specified port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the pins to be locked.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @return None
 * @note   When the lock sequence has been applied on a port bit, the
 *         value of this port bit can no longer be modified until the
 *         next reset.
 */
void GPIO_LockPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  uint32_t tmp = GPIO_LCKR_LCKK;

  /* Check the parameters */
  assert_param(IS_GPIO_ALL_PERIPH(GPIOx));
  assert_param(IS_GPIO_PIN(GPIO_Pin));

  tmp |= GPIO_Pin;
  /* Set LCKK bit */
  GPIOx->LCKR = tmp;
  /* Reset LCKK bit */
  GPIOx->LCKR = GPIO_Pin;
  /* Set LCKK bit */
  GPIOx->LCKR = tmp;
  /* Read LCKK bit*/
  (void) GPIOx->LCKR;
}

/**
 * @brief  Return 1 if all pins passed as parameter, of a specified port, are locked. else Return 0.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the pins to check lock.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @return State of bit (1 or 0).
 */
uint32_t GPIO_IsPinLocked(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  return ((GPIOx->LCKR & GPIO_Pin) == GPIO_Pin);
}

/**
 * @brief  Configure gpio input buffer of any pins for a dedicated port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bits to configure.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @param  GPIO_InputBuffer: specifies the input buffer.
 *         This parameter can be one of the following values:
 *         @arg @ref GPIO_InputBuffer_Schmitt
 *         @arg @ref GPIO_InputBuffer_CMOS
 * @return None
 */
void GPIO_InputBufferConfig(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_InputBuffer)
{
  if (GPIO_InputBuffer == GPIO_InputBuffer_Schmitt)
  {
    GPIOx->CSR &= ~GPIO_Pin;
  }
  else
  {
    GPIOx->CSR |= GPIO_Pin;
  }
}

/**
 * @brief  Configure gpio drive strength of any pins for a dedicated port.
 * @param  GPIOx: where x can be (A..D) to select the GPIO peripheral.
 * @param  GPIO_Pin: specifies the port bits to configure.
 *         This parameter can be any combination of GPIO_Pin_x where x can be (0..15).
 * @param  GPIO_DriveStrength: specifies the drive strength.
 *         This parameter can be one of the following values:
 *         @arg @ref GPIO_DriveStrength_High
 *         @arg @ref GPIO_DriveStrength_Low
 * @return None
 */
void GPIO_DriveStrengthConfig(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t GPIO_DriveStrength)
{
  if (GPIO_DriveStrength == GPIO_DriveStrength_High)
  {
    GPIOx->DRR &= ~GPIO_Pin;
  }
  else
  {
    GPIOx->DRR |= GPIO_Pin;
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
