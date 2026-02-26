/**
 * @file    wb7720_syscfg.c
 * @author  Westberry Application Team
 * @version V0.1.1
 * @date    13-January-2025
 * @brief   This file provides all the SYSCFG firmware functions.
 */

/* Includes ------------------------------------------------------------------*/
#include "wb7720_syscfg.h"
#include "wb7720_rcc.h"

/** @addtogroup WB7720_StdPeriph_Driver
  * @{
  */

/** @defgroup SYSCFG 
  * @brief SYSCFG driver modules
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup SYSCFG_Private_Functions
  * @{
  */ 

/**
  * @brief  Deinitializes the SYSCFG registers to their default reset values.
  * @param  None
  * @return None
  */
void SYSCFG_DeInit(void)
{
  SYSCFG->CFGR1 = 0x0;
  SYSCFG->CFGR2 = 0x0;
  SYSCFG->CFGR3 = 0x0;
  SYSCFG->CFGR4 = 0x0;
  SYSCFG->EXTICR[0] = 0;
  SYSCFG->EXTICR[1] = 0;
  SYSCFG->EXTICR[2] = 0;
  SYSCFG->EXTICR[3] = 0;
}

/**
  * @brief  Configures the memory mapping at address 0x00000000.
  * @param  SYSCFG_MemoryRemap: selects the memory remapping.
  *         This parameter can be one of the following values:
  *           @arg SYSCFG_MemoryRemap_Flash: Main Flash memory mapped at 0x00000000  
  *           @arg SYSCFG_MemoryRemap_SystemMemory: System Flash memory mapped at 0x00000000
  *           @arg SYSCFG_MemoryRemap_SRAM: Embedded SRAM mapped at 0x00000000
  * @return None
  */
void SYSCFG_MemoryRemapConfig(uint32_t SYSCFG_MemoryRemap)
{
  uint32_t tmpcfgr = 0;

  /* Check the parameter */
  assert_param(IS_SYSCFG_MEMORY_REMAP(SYSCFG_MemoryRemap));

  /* Get CFGR1 register value */
  tmpcfgr = SYSCFG->CFGR1;

  /* Clear MEM_MODE bits */
  tmpcfgr &= (uint32_t) (~SYSCFG_CFGR1_MEM_MODE_Msk);
  
  /* Set the new MEM_MODE bits value */
  tmpcfgr |= (uint32_t) SYSCFG_MemoryRemap;

  /* Set CFGR1 register with the new memory remap configuration */
  SYSCFG->CFGR1 = tmpcfgr;
}

/**
 * @brief  Enables or disables the automatic trim of crs.
 * @param  NewState: new state of the automatic trimming.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_CRS_TrimCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR3 |= SYSCFG_CFGR3_CRS_TRIM_EN;
  }
  else
  {
    SYSCFG->CFGR3 &= ~SYSCFG_CFGR3_CRS_TRIM_EN;
  }
}

/**
  * @brief  Selects the GPIO pin used as EXTI Line.
  * @param  EXTI_PortSourceGPIOx: selects the GPIO port to be used as source 
  *         for EXTI lines where x can be (A, B, C, D, E or F).    
  * @param  EXTI_PinSourcex: specifies the EXTI line to be configured.
  *         This parameter can be EXTI_PinSourcex where x can be (0..15).
  * @retval None
  */
void SYSCFG_EXTILineConfig(uint8_t EXTI_PortSourceGPIOx, uint8_t EXTI_PinSourcex)
{
  uint32_t tmp = 0x00;

  /* Check the parameters */
  assert_param(IS_EXTI_PORT_SOURCE(EXTI_PortSourceGPIOx));
  assert_param(IS_EXTI_PIN_SOURCE(EXTI_PinSourcex));
  
  tmp = ((uint32_t)0x0F) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03));
  SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] &= ~tmp;
  SYSCFG->EXTICR[EXTI_PinSourcex >> 0x02] |= (((uint32_t)EXTI_PortSourceGPIOx) << (0x04 * (EXTI_PinSourcex & (uint8_t)0x03)));
}

/**
 * @brief  Enables or disables the usb D+ pull up resistor.
 * @param  NewState: new state of the pull up resistor.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_USB_PullUPCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR2 |= SYSCFG_CFGR2_DPPUEN;
  }
  else
  {
    SYSCFG->CFGR2 &= ~SYSCFG_CFGR2_DPPUEN;
  }
}

/**
 * @brief  Enables or disables the SINK function config.
 * @param  SINK_Index: sink pin index.
 *         @arg SYSCFG_SINK_PC0: PC0
 *         @arg SYSCFG_SINK_PC1: PC1
 *         @arg SYSCFG_SINK_PC2: PC2
 *         @arg SYSCFG_SINK_PC3: PC3
 *         @arg SYSCFG_SINK_PC4: PC4
 *         @arg SYSCFG_SINK_PC5: PC5
 * @param  NewState: new state of the Sink function.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_SINK_ConfigCmd(uint8_t SINK_Index, FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR2 |= (0x01 << (12 + SINK_Index));
  }
  else
  {
    SYSCFG->CFGR2 &= ~(0x01 << (12 + SINK_Index));
  }
}

/**
 * @brief  Enables or disables the SINK function of SINK pin.
 * @param  SINK_Index: sink pin index.
 *         @arg SYSCFG_SINK_PC0: PC0
 *         @arg SYSCFG_SINK_PC1: PC1
 *         @arg SYSCFG_SINK_PC2: PC2
 *         @arg SYSCFG_SINK_PC3: PC3
 *         @arg SYSCFG_SINK_PC4: PC4
 *         @arg SYSCFG_SINK_PC5: PC5
 * @param  NewState: new state of the SINK pin.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_SINK_SINKCmd(uint8_t SINK_Index, FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR2 |= (0x01 << (2 * SINK_Index + 1));
  }
  else
  {
    SYSCFG->CFGR2 &= ~(0x01 << (2 * SINK_Index + 1));
  }
}

/**
 * @brief  Enables or disables the open source function of SINK pin.
 * @param  SINK_Index: sink pin index.
 *         @arg SYSCFG_SINK_PC0: PC0
 *         @arg SYSCFG_SINK_PC1: PC1
 *         @arg SYSCFG_SINK_PC2: PC2
 *         @arg SYSCFG_SINK_PC3: PC3
 *         @arg SYSCFG_SINK_PC4: PC4
 *         @arg SYSCFG_SINK_PC5: PC5
 * @param  NewState: new state of the SINK pin.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_SINK_OpenSourceCmd(uint8_t SINK_Index, FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR2 |= (0x01 << (2 * SINK_Index));
  }
  else
  {
    SYSCFG->CFGR2 &= ~(0x01 << (2 * SINK_Index));
  }
}

/**
 * @brief  Enables or disables the NRST PIN function.
 * @param  NewState: new state of the NRST pin.
 *         This parameter can be: ENABLE or DISABLE.
 * @return None
 */
void SYSCFG_NRST_PINCmd(FunctionalState NewState)
{
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    SYSCFG->CFGR3 |= SYSCFG_CFGR3_NRST_DIS;
  }
  else
  {
    SYSCFG->CFGR3 &= SYSCFG_CFGR3_NRST_DIS;
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

