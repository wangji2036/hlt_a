/**
  ******************************************************************************
  * @file    fmc.c
  * @brief   Flash HAL module driver.
  *          This file provides firmware functions to manage the following <br>
  *          The functionalities of the Flash peripherals as below: <br>
  *           - Erase one page of the Flash memory. <br>
  *           - Write a word to the Flash memory. <br>
  *          
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Novltatech. <br>
  * All rights reserved. <br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### Flash Peripheral Features #####
  ==============================================================================
  [..] 
  @todo Need to add the description of the Flash features when IC datasheet is available.

  [..] 
  The Flash has the following features:
  (+) 64K Flash memory organized into 256 pages of 128 bytes each.
  (+) 16-bit data bus.
  (+) 128-bit flash erase command.
  (+) 32-bit flash program command.

  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Erase the page of the Flash memory using the hal_fmc_erase_page() function.
    (#) Write a word to the Flash memory using the hal_fmc_write_word() function.
		-	Before writing a word to the Flash memory, the erease operation should be performed.


  @endverbatim
  ******************************************************************************
  */ 

#include "regdef.h"
#include "fmc.h"

/**	@cond HIDDEN_SYMBOLS */
static void FMC_vClose(void)
{
	FMC->FMC_CMD_CTRL.WORD = _FMC_CMD_ALL_CTRL_DISABLE;
}

static void FMC_vEraseEnable(void)
{
	FMC->FMC_CMD_CTRL.WORD = _FMC_CMD_PAG_ERASE_ENABLE;
}

static void FMC_vWriteEnable(void)
{
	FMC->FMC_CMD_CTRL.WORD = _FMC_CMD_AHB_WRITE_ENABLE;
}

void __attribute__((isr)) DMA_IRQHandler(void)
{
}
/**	@endcond */

/**
  * @brief     Erase a page. The page size is 512 bytes.
  * @param addr   Flash page address. Must be a 512-byte aligned address.
  * @retval    void
  */
void hal_fmc_erase_page(uint32_t addr)
{
	FMC_vEraseEnable();
	__write_32bits(addr, 0); //a dummy write operation trigger FMC erase.
	FMC_vClose();
}

/**
  * @brief     Writes a word data to specified flash address.
  * @param addr  Destination address, Must be a 4-byte aligned address.
  * @param data  Word data to be written
  * @return   void
  */
void hal_fmc_write_word(uint32_t addr, uint32_t data)
{
	FMC_vWriteEnable();
	__write_32bits(addr, data);
	FMC_vClose();
}
