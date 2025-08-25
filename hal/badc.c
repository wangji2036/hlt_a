/**
  ******************************************************************************
  * @file    badc.c
  * @brief   badc HAL module driver.<br>
  *          This file provides firmware functions to manage the following functions of the generic ADC peripheral: <br>
  *           - Initialization function <br>
  * 		  - Implementation of software filtering <br> 
  *           - ADC Channel read function <br>
  * 		  - BADC Interrupt Handler <br>
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 NuVolta Tech.<br>
  * All rights reserved.<br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS. <br>
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                    ##### BADC Peripheral features #####
  ==============================================================================
  [..] 
	(#) Skannemodus for automatisk overgang fra kanal 0 til kanal x.
	(#) Datajustering for innebygd datakonsistens.
	(#) ADC-konverteringstype (se datablad).
	(#) Valgfri programvarefiltrering i utlesningen.
	(#) Valgfri bruk av avbruddsmodus, der det genereres et avbrudd ved slutten av konverteringen.
	(#) Konfigurerbare referansekilder: VDD, V1P2, V1P1
	(#) BADC-klokke fast på 4M

                     ##### How to use this driver #####
  ==============================================================================  
  [..]
    (#) Configure the BADC(s) using hal_badc_init().
	(#) Read the analog voltage value of the specified channel using hal_badc_meas().
		CHAN_SEL (BADC channel select):
		- AVSS_BG
		- BADC1
		- BADC2
		- BADC3
		- BADC4
		- BADC5
		- BADC6
		- BADC7
		- BADC8
		- BADC9
		- internal v055 BAND_GAP
		- internal V1P1 BAND_GAP
		- internal V1P2 BAND_GAP
		- internal temperature_L
		- internal temperature_H
		- ISNS_VOUT_P, differential signal amplified by ISNS_PGA, ISNS_VOUT_N will auto switch to the negative input of BADC. 
		 PA5 and PB5 need to be set as ISNS_PGA mode, and ISNS_PGA_EN needs to be enabled.
	(#) To use software filtering, call the hal_badc_average_meas() function.
	(#) Use the interrupt handler BADC_IRQHandler() for handling BADC events.
  @endverbatim
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "badc.h"

#define BADC_TRIM_ADDR_VREF_GAIN    (0x00001C9A)
#define BADC_TRIM_ADDR_VREF_BIAS    (0x00001C98)
#define BADC_TRIM_ADDR_ISNS_GAIN    (0x00001C92)
#define BADC_TRIM_ADDR_ISNS_BIAS    (0x00001C90)

#define BADC_ISNS_CHAN_DC_OFFSET    (      1000) //1000mV

static uint16_t badc_vref_gain;
static  int16_t badc_vref_bias;
static uint16_t badc_isns_gain;
static  int16_t badc_isns_bias;

static void hal_badc_cali(void)
{
	uint16_t temp_trim_code;

	badc_vref_gain = __read_16bits(BADC_TRIM_ADDR_VREF_GAIN) ^ 0xFFFF;

	temp_trim_code = __read_16bits(BADC_TRIM_ADDR_VREF_BIAS) ^ 0xFFFF;
	badc_vref_bias = (temp_trim_code > 0x8000) ? (int16_t)(temp_trim_code - 0xFFFF) : temp_trim_code;

	badc_isns_gain = __read_16bits(BADC_TRIM_ADDR_ISNS_GAIN) ^ 0xFFFF;

	temp_trim_code = __read_16bits(BADC_TRIM_ADDR_ISNS_BIAS) ^ 0xFFFF;
	badc_isns_bias = (temp_trim_code > 0x8000) ? (int16_t)(temp_trim_code - 0xFFFF) : temp_trim_code;

	printk("\r\n badc_cali: %d %d %d %d", badc_vref_gain, badc_vref_bias, badc_isns_gain, badc_isns_bias);

	if (badc_vref_gain == 0)
	{
		badc_vref_gain = 10000;
		printk("\r\n badc_cali_vref error ...");
	}

	if (badc_isns_gain == 0)
	{
		badc_isns_gain = 10000;
		printk("\r\n badc_cali_isns error ...");
	}
}

/**
 * @brief  Initialize the configuration of the hardware ADC module.
 * 
 * This function configures the ADC's sample average count,
 * sample delay,sample clock,and reference voltage,and enables the ADC module.
 * Ensure that the ADC will function properly in subsequent operations.
 * 
 * @param  None.
 * @return None.
 */
void hal_badc_init(void)
{
	//(4+1)*(3+3)*250ns = 7.5us
	BADC->CTRL.WORD = (_BADC_SAMPLE_AVG_4 << BADC_CTRL_SAMPLE_AVG_SEL_Pos) | (_BADC_SAMPLE_DLY_1 << BADC_CTRL_SAMPLE_DLY_SEL_Pos) |
			          (_BADC_SAMPLE_CLK_3 << BADC_CTRL_SAMPLE_CLK_SEL_Pos) | (_BADC_VREF_V3P3 << BADC_CTRL_VREF_SEL_Pos) | BADC_CTRL_ADC_EN_Msk;

	hal_badc_cali();
}

static uint16_t hal_badc_vref_update(void)
{
	uint32_t tmp;

	BADC->CTRL.BITS.CHAN_SEL = _BADC_CH_INR_V1P2;
	delay_1us(20); //for channel stable
	BADC->FLAG.WORD = BADC_FLAG_DONE_FLAG_Msk;   //clear flag
	BADC->CTRL.WORD |= BADC_CTRL_CONV_START_Msk; //hardware clear automatically
	while ((BADC->FLAG.WORD & BADC_FLAG_DONE_FLAG_Msk) == 0);

	if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
	{
		tmp = BADC->DATA.BITS.CONV_DATA;
	}
	else
	{
		tmp = (BADC->DATA.BITS.CONV_DATA & 0x80) ? BADC->DATA.BITS.CONV_DATA + 2 : BADC->DATA.BITS.CONV_DATA - 2;
	}
	tmp = tmp * badc_vref_gain / 1000;

	return (tmp != 0) ? (12000 - badc_vref_bias) * 4096 / tmp : 3300;
}

/**
 * @brief  Measures the average ADC value over a specified number of samples.
 * Measures the ADC value from the specified channel 
 * and calculates the average of multiple measurements.
 * 
 * This function takes multiple measurements from the provided analog signal channels 
 * and calculates their average value. The measurement improves the reliability 
 * of the result by * excluding extreme values (maximum and minimum) to minimize noise effects.
 *
 * @param  channel The channel to be measured, type `badc_chan_t` enumeration value.
 * @param  times   The number of measurements performed with a minimum value of 1.
 * @return Returns the average value of the measurement, of type int16_t, representing the measurement value.
 */
static int16_t hal_badc_average_meas(enum badc_chan_t channel, uint8_t times)
{
	int16_t tmp, max = -4095, min = 4095, sum = 0;

	//10us + 7.5us * 6 = 55us
	for (int i=0; i<times; i++)
	{
		if (BADC->CTRL.BITS.CHAN_SEL != channel)
		{
			delay_1us(10); //for channel stable
			BADC->CTRL.BITS.CHAN_SEL = channel;
		}
		BADC->FLAG.WORD = BADC_FLAG_DONE_FLAG_Msk;   //clear flag
		BADC->CTRL.WORD |= BADC_CTRL_CONV_START_Msk; //hardware clear automatically
		while ((BADC->FLAG.WORD & BADC_FLAG_DONE_FLAG_Msk) == 0);

		if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
		{
			tmp = (BADC->DATA.WORD & BADC_DATA_NEGA_SIGN_Msk) ? 0 - ((BADC->DATA.BITS.CONV_DATA ^ 0xFFF) + 1) : (BADC->DATA.BITS.CONV_DATA);
		}
		else
		{
			tmp = (BADC->DATA.WORD & BADC_DATA_NEGA_SIGN_Msk) ? 0 - ((BADC->DATA.BITS.CONV_DATA ^ 0xFFF) + 1) : (BADC->DATA.BITS.CONV_DATA);
			tmp = (tmp & 0x80) ? tmp + 2 : tmp - 2;
		}

		if (tmp > max) max = tmp;
		if (tmp < min) min = tmp;

		sum += tmp;
	}

	if (times > 2)
	{
		tmp = (sum - max - min) / (times - 2);
	}
	else if (times > 0)
	{
		tmp = sum / times;
	}
	else
	{
		tmp = 0;
	}
	//printk("\r\nraw_tmp%d",tmp);
	return tmp;
}

/**
 * @brief  Measures the analog voltage value of the specified channel.
 * 
 * This function takes a channel as an input parameter and 
 * averages six measurements of the analog signal of that channel.
 * It converts the raw measurement to a voltage value based on a reference
 * voltage. The result is scaled to a 12-bit resolution.
 *
 * @param  channel The specified channel to measure. The channel should be
 *         one of the defined enumeration values of type `badc_chan_t`.
 * @return Returns the measured voltage value in millivolts (mV).
 */
uint16_t hal_badc_meas(enum badc_chan_t channel)
{
	uint16_t rst = 1;

	uint16_t BDAC_VREF_V3P3 = hal_badc_vref_update();

	int tmp = (BDAC_VREF_V3P3 * hal_badc_average_meas(channel, 4)) >> 12; //100us
	//int tmp = hal_badc_average_meas(channel, 4);
	switch (channel)
	{
		case _BADC_CH_PC6_ADC1:
			break;
		case _BADC_CH_PB2_ADC2:
			if (tmp > 0)
			{
				rst = tmp;
				if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_PD6_ADC3:
			if (tmp > BADC_ISNS_CHAN_DC_OFFSET)
			{
				tmp -= BADC_ISNS_CHAN_DC_OFFSET;
				tmp = tmp * badc_isns_gain / 10000;
				tmp = ((EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 400) ? tmp * 1039 / 1000 : tmp * 1012 / 1000;
				tmp = tmp + badc_isns_bias;
				rst = (tmp > 0) ? tmp : 1;
			}
			break;
		case _BADC_CH_PC7_ADC4:
			break;
		case _BADC_CH_PC8_ADC5:
			if (tmp > 0)
			{
				rst = tmp;
				if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_PB5_ADC6:
			if (tmp > 0)
			{
				rst = tmp;
				if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_PB6_ADC7:
			if (tmp > 0)
			{
				//rst = tmp * 72 / 10;
				rst = tmp * 84 / 10;
				if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_PD0_ADC8:
			if (tmp > 0)
			{
				rst = tmp * 72 / 10;
				if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_PD3_ADC9:
			if (tmp > 0)
			{
			rst = tmp;
			if (rst < 1) rst = 1;
			}
			break;
		case _BADC_CH_INR_V055:
			break;
		case _BADC_CH_INR_V1P1:
			break;
		case _BADC_CH_INR_V1P2:
			break;
		case _BADC_CH_INR_TJ_L: //180us
		case _BADC_CH_INR_TJ_H:
		{
			int32_t tmp_p, tmp_n;
			if (channel == _BADC_CH_INR_TJ_H)
			{
				tmp_p = tmp;
				tmp_n = (BDAC_VREF_V3P3 * hal_badc_average_meas(_BADC_CH_INR_TJ_L, 6)) >> 12;
			}
			else
			{
				tmp_n = tmp;
				tmp_p = (BDAC_VREF_V3P3 * hal_badc_average_meas(_BADC_CH_INR_TJ_H, 6)) >> 12;
			}
			tmp = (tmp_p - tmp_n - 437) * 2 / 3 + 25;
			rst = (uint16_t)(tmp & 0xFFFF);
			break;
		}
		case _BADC_CH_PGA_ISNS:
		     rst = tmp;
			//rst = tmp1;
			//rst = (tmp1*25)/10;// gain = 40,  sense R:10 mohm, and changed to be mA. so Ibus = adc value*2.5 ( mA)
			break;
		default:
			break;
	}

	return rst;
}

/**
 * @brief  Update BADC internal ISNS channel offset.
 *
 * This function update internal ISNS channel DC bias.
 * please call this function before EPWM start driver power stage.
 *
 * @param  None.
 * @return None.
 */
void hal_badc_isns_chan_offest_update(void)
{
	uint16_t BDAC_VREF_V3P3 = hal_badc_vref_update();

	int32_t tmp = (BDAC_VREF_V3P3 * hal_badc_average_meas(_BADC_CH_PD6_ADC3, 4)) >> 12; //100us

	badc_isns_bias = (int16_t)(1000 - tmp);

//	printk("\r\n isns_ofs: %d", badc_isns_bias);
}

/**
  * @brief  BADC interrupt handler.
  * @retval None
  */
void __attribute__((isr)) BADC_IRQHandler(void)
{
}
