/**
  ******************************************************************************
  * @file    eadc.c
  * @brief   eadc HAL module driver.<br>
  *          This file provides firmware functions to manage the functionalities<br>
  *          of the Enhanced Analog-to-Digital Converter (EADC) peripheral:<br>
  *          - EADC digital demodulation Mode Initialization
  *          - EADC analog demodulation Mode Initialization
  *          - EADC Interrupt Handler
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
                    ##### EADC Peripheral features #####
  ==============================================================================
  [..] 
  (#) The EADC allows for enhanced conversion of analog signals to digital values
      with flexible configurations including reference voltage, clock source,
      and channel selection.
                     ##### How to use this driver #####
  ==============================================================================  
  [..]
  (#) Initialize the EADC for digital demodulation using hal_eadc_sho().
  (#) Initialize the EADC for analog demodulation mode using hal_eadc_init().
  (#) Use the interrupt handler EADC_IRQHandler() for handling EADC events.
  
  @endverbatim
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/
#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "algo.h"
#include "eadc.h"

#define EADC_TRIM_ADDR_VREF_GAIN    (0x00001C9C)
#define EADC_TRIM_ADDR_VREF_BIAS    (0x00001CA2)
#define EADC_TRIM_ADDR_VCAP_GAIN    (0x00001C96)
#define EADC_TRIM_ADDR_VCAP_BIAS    (0x00001C94)

#define EADC_VCAP_CHAN_DC_OFFSET    (      1650)
#define EADC_VCAP_CHAN_FIXD_GAIN    (      1385)
#define EADC_VCAP_CHAN_FIXD_BIAS    (         0)

#define ICAP_MAX_VAULE_360K_GAIN    (       129)
#define ICAP_MAX_VAULE_360K_BIAS    (       -48)
#define ICAP_MAX_VAULE_128K_GAIN    (        68)
#define ICAP_MAX_VAULE_128K_BIAS    (       448)

#define ICAP_RMS_VAULE_360K_GAIN    (       127)
#define ICAP_RMS_VAULE_360K_BIAS    (      -100)
#define ICAP_RMS_VAULE_128K_GAIN    (       129)
#define ICAP_RMS_VAULE_128K_BIAS    (      -368)

static uint16_t eadc_vref_gain;
static  int16_t eadc_vref_bias;
static uint16_t eadc_vcap_gain;
static  int16_t eadc_vcap_bias;

void hal_eadc_init(void)
{
	uint16_t temp_trim_code;

	eadc_vref_gain = __read_16bits(EADC_TRIM_ADDR_VREF_GAIN) ^ 0xFFFF;

	temp_trim_code = __read_16bits(EADC_TRIM_ADDR_VREF_BIAS) ^ 0xFFFF;
	eadc_vref_bias = (temp_trim_code > 0x8000) ? (int16_t)(temp_trim_code - 0xFFFF) : temp_trim_code;

	eadc_vcap_gain = __read_16bits(EADC_TRIM_ADDR_VCAP_GAIN) ^ 0xFFFF;

	temp_trim_code = __read_16bits(EADC_TRIM_ADDR_VCAP_BIAS) ^ 0xFFFF;
	eadc_vcap_bias = (temp_trim_code > 0x8000) ? (int16_t)(temp_trim_code - 0xFFFF) : temp_trim_code;

	printk("\r\n eadc_cali: %d %d %d %d", eadc_vref_gain, eadc_vref_bias, eadc_vcap_gain, eadc_vcap_bias);

	if (eadc_vref_gain == 0)
	{
		eadc_vref_gain = 10000;
		printk("\r\n eadc_cali_vref error ...");
	}

	if (eadc_vcap_gain == 0)
	{
		eadc_vcap_gain = 10000;
		printk("\r\n eadc_cali_vcap error ...");
	}
}

static uint16_t hal_eadc_vref_update(void)
{
	uint32_t tmp;

	EADC->CTRL.WORD = (_EADC_MODE_DIG_DDM << EADC_CTRL_ADC_MODE_Pos) | (_EADC_CH_INR_V1P2 << EADC_CTRL_CHAN_SEL_Pos) | (_EADC_VREF_V3P3 << EADC_CTRL_VREF_SEL_Pos) |
			(_EADC_DIG_DDM_LPF_RC_200ns << EADC_CTRL_VCAP_LPF_RC_SEL_Pos) | (_EADC_SAMPLE_DLY_3 << EADC_CTRL_SAMPLE_DLY_SEL_Pos) | EADC_CTRL_ADC_EN_Msk;

	if ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 400) //360K MPP
	{
		EADC->CTRL.WORD |= (_EADC_SOURCE_CLK_40x360K << EADC_CTRL_SOURCE_CLK_SEL_Pos);
	}
	else
	{
		EADC->CTRL.WORD |= (_EADC_SOURCE_CLK_40xEPWM << EADC_CTRL_SOURCE_CLK_SEL_Pos);
		EADC->CTRL.WORD |= (((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) / 20) << EADC_CTRL_CLK_DIV_Pos);
	}

	delay_1us(20); //for channel stable
	EADC->FLAG.WORD = EADC_FLAG_DONE_FLAG_Msk;
	EADC->CTRL.WORD |= EADC_CTRL_VCAP_DETECT_EN_Msk | EADC_CTRL_CONV_START_Msk;
	while ((EADC->FLAG.WORD & EADC_FLAG_DONE_FLAG_Msk) == 0);

	tmp = 0;
	for (int i=0; i<20; i++)
	{
		tmp += (EADC->DATA[i].BITS.CONV_DATA & 0x80) ? EADC->DATA[i].BITS.CONV_DATA + 2 : EADC->DATA[i].BITS.CONV_DATA - 2;
	}

	tmp /= 20;
	tmp = tmp * eadc_vref_gain / 1000;

	return (tmp != 0) ? (12000 - eadc_vref_bias) * 4096 / tmp : 3300;
}

uint16_t hal_eadc_meas(enum eadc_chan_t channel)
{
	if (channel != _EADC_CH_INR_VCAP || EPWM1->PWM_CTRL.BITS.EPWM_EN == 0)
	{
		return 0;
	}

	int32_t tmp[20], vctx_max, vctx_min;
	uint32_t delta_abs[20], delta_max, delta_sum;
	uint16_t icol_max_buff[4], icol_rms_buff[4], vctx_p2p_buff[4];

	uint16_t EDAC_VREF_V3P3 = hal_eadc_vref_update();

	EADC->CTRL.WORD = (_EADC_MODE_DIG_DDM << EADC_CTRL_ADC_MODE_Pos) | (_EADC_CH_INR_VCAP << EADC_CTRL_CHAN_SEL_Pos) |  (_EADC_VREF_V3P3 << EADC_CTRL_VREF_SEL_Pos) |
			(_EADC_DIG_DDM_LPF_RC_200ns << EADC_CTRL_VCAP_LPF_RC_SEL_Pos) | (_EADC_SAMPLE_DLY_3 << EADC_CTRL_SAMPLE_DLY_SEL_Pos) | EADC_CTRL_ADC_EN_Msk;

	if ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 400) //360K MPP
	{
		EADC->CTRL.WORD |= (_EADC_SOURCE_CLK_40x360K << EADC_CTRL_SOURCE_CLK_SEL_Pos);
	}
	else
	{
		EADC->CTRL.WORD |= (_EADC_SOURCE_CLK_40xEPWM << EADC_CTRL_SOURCE_CLK_SEL_Pos);
		tmp[0] = (EPWM1->PWM_PERD.BITS.PWM_PERD + 1) / 20;
		EADC->CTRL.WORD |= (tmp[0] << EADC_CTRL_CLK_DIV_Pos);
	}

	delay_1us(20); //for channel stable

	for (int times=0; times<4; times++)
	{
		EADC->FLAG.WORD = EADC_FLAG_DONE_FLAG_Msk;
		EADC->CTRL.WORD |= EADC_CTRL_VCAP_DETECT_EN_Msk | EADC_CTRL_CONV_START_Msk;
		while ((EADC->FLAG.WORD & EADC_FLAG_DONE_FLAG_Msk) == 0);

		for(int i=0; i<20; i++)
		{
			tmp[i] = (EADC->DATA[i].BITS.CONV_DATA & 0x80) ? EADC->DATA[i].BITS.CONV_DATA + 2 : EADC->DATA[i].BITS.CONV_DATA - 2;
			tmp[i] = (tmp[i] * EDAC_VREF_V3P3) >> 12;
			tmp[i] = tmp[i] * eadc_vcap_gain / 10000 + eadc_vcap_bias / 10;
			tmp[i] = tmp[i] - EADC_VCAP_CHAN_DC_OFFSET;
//			printk(" %d", tmp[i]);
			tmp[i] = tmp[i] * EADC_VCAP_CHAN_FIXD_GAIN / 100 + EADC_VCAP_CHAN_FIXD_BIAS;
//			printk(" %d", tmp[i]);
		}

		vctx_max = vctx_min = tmp[0];
		delta_abs[0] = (tmp[0] > tmp[19]) ? (tmp[0] - tmp[19]) : (tmp[19] - tmp[0]);
		delta_max = delta_abs[0];
		delta_sum = delta_abs[0] * delta_abs[0];
		for (int i=1; i<20; i++)
		{
			if (tmp[i] > vctx_max) vctx_max = tmp[i];
			if (tmp[i] < vctx_min) vctx_min = tmp[i];
			delta_abs[i] = (tmp[i] > tmp[i-1]) ? (tmp[i] - tmp[i-1]) : (tmp[i-1] - tmp[i]);
			if (delta_abs[i] > delta_max)
			{
				delta_max = delta_abs[i];
			}
			delta_sum += delta_abs[i] * delta_abs[i];
		}
		delta_sum /= 20;
		delta_sum = quick_sqrt(delta_sum);

		if ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 400) //360K MPP
		{
			icol_max_buff[times] = ICAP_MAX_VAULE_360K_GAIN * (gd->ctx * 9 * delta_max) / ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) * 625) + ICAP_MAX_VAULE_360K_BIAS;
			icol_rms_buff[times] = ICAP_RMS_VAULE_360K_GAIN * (gd->ctx * 9 * delta_sum) / ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) * 625) + ICAP_RMS_VAULE_360K_BIAS;
			vctx_p2p_buff[times] = vctx_max - vctx_min;
		}
		else
		{
			icol_max_buff[times] = ICAP_MAX_VAULE_128K_GAIN * (gd->ctx * 9 * delta_max) / ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) * 625) + ICAP_MAX_VAULE_128K_BIAS;
			icol_rms_buff[times] = ICAP_RMS_VAULE_128K_GAIN * (gd->ctx * 9 * delta_sum) / ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) * 625) + ICAP_RMS_VAULE_128K_BIAS;
			vctx_p2p_buff[times] = vctx_max - vctx_min;
		}
	}

	delta_abs[0] = delta_abs[1] = delta_abs[2] = 0;
	for (int times=0; times<4; times++)
	{
		delta_abs[0] += icol_max_buff[times];
		delta_abs[1] += icol_rms_buff[times];
		delta_abs[2] += vctx_p2p_buff[times];
	}

	gd->icol_max = delta_abs[0] >> 2;
	gd->icol_rms = delta_abs[1] >> 2;
	gd->vctx_pp  = delta_abs[2] >> 2;

//	gd->icol_rms = gd->icol_rms * gd->pid_duty / 500;

//	printk(" icol_rms: %d", gd->icol_rms);
//	printk(" icol_max: %d", gd->icol_max);

	return 0;
}

void hal_eadc_ddm_init(void)
{
	EADC->CTRL.WORD = (_EADC_MODE_DIG_DDM << EADC_CTRL_ADC_MODE_Pos) | (_EADC_CH_INR_VCAP << EADC_CTRL_CHAN_SEL_Pos) |  (_EADC_VREF_V3P3 << EADC_CTRL_VREF_SEL_Pos) |
			(_EADC_SOURCE_CLK_40x360K << EADC_CTRL_SOURCE_CLK_SEL_Pos) | (_EADC_DIG_DDM_LPF_RC_200ns << EADC_CTRL_VCAP_LPF_RC_SEL_Pos) |
			(_EADC_SAMPLE_DLY_3 << EADC_CTRL_SAMPLE_DLY_SEL_Pos) | EADC_CTRL_CONV_START_Msk |
			/*EADC_CTRL_INT_EN_Msk |*/ EADC_CTRL_ADC_EN_Msk;
}

void hal_eadc_stop(void)
{
	EADC->CTRL.WORD = 0;
}

void __attribute__((isr)) EADC_IRQHandler(void)
{
}
