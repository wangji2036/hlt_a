/**
  ******************************************************************************
  * @file    ../fml/fsk.c
  * @brief   This file provides firmware functions to manage the functionalities<br>
  *          of the FSK modulation:<br>
  *           - FSK status check
  *           - FSK parameters set
  *           - FSK pattern data send
  *           - FSK packet data send
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 NuVolta Technologies. <br>
  * All rights reserved. <br>
  *
  * This software is licensed under terms that can be found in the LICENSE file <br>
  * in the root directory of this software component. <br>
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  @verbatim
  ==============================================================================
                      ##### FSK module driver features #####
  ==============================================================================
  [..]
  (#) FSK status check
  (#) FSK parameters set
  (#) FSK pattern data send
  (#) FSK packet data send

                      *** How to use this driver ***
  ==============================================================================
  [..]
  (#) should call these function during power transfer, the EPWM is enable.

                      *** Execution of FSK operations ***
  ==============================================================================
  [..]
  (#) The driver can operate in the following modes:

     *** parameters set ***
     ============================
     [..]
       (+) call fml_fsk_param_set when it is necessary to set or change FSK parameters.

     *** status check ***
     ============================
     [..]
       (+) call fml_fsk_is_busy to check FSK is sending or not during power transfer.

     *** FSK pattern send ***
     ============================
     [..]
       (+) call fml_fsk_patt_send to send a FSK pattern during power transfer.

     *** FSK packet send ***
     ============================
     [..]
       (+) call fml_fsk_data_send to send a FSK packet data buffer during power transfer.

  @endverbatim
*/


#include "regdef.h"
#include "printk.h"
#include "_wpc.h"
#include "g_data.h"
#include "osal.h"
#include "fsk.h"

#define FSK_PKT_MAX_LEN    28 //header + message, not include checksum

static uint8_t  fsk_is_busy[2];
static uint8_t  fsk_dither_status[2];
static uint8_t  fsk_need_preamble[2];
static uint8_t  fsk_need_send_cnt[2];
static uint8_t  fsk_have_send_cnt[2];
static uint8_t  fsk_original_data_buff[2][FSK_PKT_MAX_LEN + 1]; //1-checksum
static uint32_t fsk_encoding_data_buff[2][FSK_PKT_MAX_LEN + 1]; //1-checksum

/**
 * @brief  check FSK is sending or not.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 *
 * @return 0-idle 1-busy.
 */
uint8_t fml_fsk_is_busy(TS_EPWM *epwm)
{
	if (epwm->PWM_CTRL.BITS.EPWM_EN == 0 || epwm->PWM_PERD.BITS.PWM_PERD == 0)
	{
		return 0;
	}
	return (epwm == EPWM1) ? fsk_is_busy[0] : fsk_is_busy[1];
}

/**
 * @brief  Set the parameters for FSK transmission
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  This parameter determines whether the difference between f_mod and f_op is positive or negative,
 *         positive(0) or negative(1).
 * @param  This parameter determines the magnitude of the difference between f_op and f_mod.
 *         depth needs to be within [0,3].
 * @param  EPWM number of cycles per bit, please use _FSK_BIT_CYCLES_512 or _FSK_BIT_CYCLES_128.
 * @param  FSK send data packet need add 4-bit-preamble or not.
 *
 * @return None.
 */
void fml_fsk_param_set(TS_EPWM *epwm, uint8_t polarity, uint8_t depth, uint8_t cycles, uint8_t preamble)
{
	uint8_t idx = (epwm == EPWM1) ? 0 : 1;

	switch (depth)
	{
		case 0: depth =  7; break;
		case 1: depth = 11; break;
		case 2: depth = 21; break;
		case 3: depth = 38; break;
		default:depth =  7; break;
	}

	epwm->FSK_CTRL.WORD &= ~(EPWM_FSK_CTRL_POLAR_SEL_Msk | EPWM_FSK_CTRL_BIT_CYCLE_Msk | EPWM_FSK_CTRL_DEPTH_SEL_Msk);
	epwm->FSK_CTRL.WORD |=  (polarity << EPWM_FSK_CTRL_POLAR_SEL_Pos) | (depth << EPWM_FSK_CTRL_DEPTH_SEL_Pos) | (cycles << EPWM_FSK_CTRL_BIT_CYCLE_Pos);

	fsk_need_preamble[idx] = preamble;
}

static int fml_fsk_data_encoding(TS_EPWM *epwm, uint8_t *data, uint8_t len)
{
	if (len == 0)
	{
		return -1;
	}

	if (epwm->PWM_CTRL.BITS.EPWM_EN == 0 || epwm->PWM_PERD.BITS.PWM_PERD == 0)
	{
		return -2;
	}

	if (len > FSK_PKT_MAX_LEN) len = FSK_PKT_MAX_LEN;

	uint8_t tmp, idx = (epwm == EPWM1) ? 0 : 1;

	for (int i=0; i<len; i++)
	{
		fsk_original_data_buff[idx][i] = *(data + i);
	}

	if (len == 1) //pattern
	{
		fsk_encoding_data_buff[idx][0] = (8 << 26) | (fsk_original_data_buff[idx][len - 1] << 0);
		fsk_need_send_cnt[idx] = len;
	}
	else //data packet
	{
		//calc_chs
		fsk_original_data_buff[idx][len] = 0;
		for (int i=0; i<len; i++)
		{
			fsk_original_data_buff[idx][len] ^= fsk_original_data_buff[idx][i];
		}

		//byte encode
		for (int i=0; i<len + 1; i++)
		{
			tmp = fsk_original_data_buff[idx][i];
			tmp ^= tmp >> 1;
			tmp ^= tmp >> 2;
			tmp ^= tmp >> 4;
			tmp &= 1;
			fsk_encoding_data_buff[idx][i] = (11 << 26) | (1 << 10) | (tmp << 9) | (fsk_original_data_buff[idx][i] << 1) | (0 << 0);
		}
		fsk_need_send_cnt[idx] = len + 1;
	}

	//MPP data packet need add 4-bit preamble
	//EPP also have 128-cycles now.
	if ((len > 1) && (epwm->FSK_CTRL.BITS.BIT_CYCLE == _FSK_BIT_CYCLES_128) && (fsk_need_preamble[idx] != 0))
	{
		uint32_t tmp_data = fsk_encoding_data_buff[idx][0];
		fsk_encoding_data_buff[idx][0] = ((tmp_data & 0x3FFFFFFUL) << 4) | 0xF;
		tmp_data >>= 26;
		tmp_data += 4;
		fsk_encoding_data_buff[idx][0] |= (tmp_data << 26);
	}

	fsk_have_send_cnt[idx] = 0;

	return 0;
}

static void fml_fsk_data_response(TS_EPWM *epwm, uint16_t delay_ms)
{
	uint8_t tmp, idx = (epwm == EPWM1) ? 0 : 1;

/*+++++++++++++++++++++ EPWM design issue workaround +++++++++++++++++++++*/
	tmp = epwm->FSK_CTRL.BITS.DEPTH_SEL;
	uint16_t perd_tmp, perd_min, perd_max;
	uint16_t duty_tmp, duty_min, duty_max;
	perd_tmp = epwm->PWM_PERD.BITS.PWM_PERD;
	duty_tmp = epwm->PWM_DUTY.BITS.CH0_DUTY;
	perd_min = (epwm->FSK_CTRL.WORD & EPWM_FSK_CTRL_POLAR_SEL_Msk) ? perd_tmp : perd_tmp - tmp;
	perd_max = (epwm->FSK_CTRL.WORD & EPWM_FSK_CTRL_POLAR_SEL_Msk) ? perd_tmp + tmp : perd_tmp;
	duty_min = (perd_min / 2) - epwm->PWM_PERD.BITS.PWM_PHAS - 3;
	duty_max = (perd_max / 2) - epwm->PWM_PERD.BITS.PWM_PHAS - 1;
	if (duty_tmp > duty_min && duty_tmp < duty_max)
	{
		epwm->PWM_DUTY.BITS.CH0_DUTY = (epwm->FSK_CTRL.WORD & EPWM_FSK_CTRL_POLAR_SEL_Msk) ? duty_min : duty_max;
	}
/*--------------------- EPWM design issue workaround ---------------------*/

	if (SYS->PID_INFO.BITS.VER != CHIP_VER_A0)
	{
		uint32_t delay_cycle = delay_ms * 144000 / (epwm->PWM_PERD.BITS.PWM_PERD + 1);
		if (delay_cycle > 0xFFFF) delay_cycle = 0xFFFF;
		epwm->FSK_DLY_.WORD = delay_cycle;
		epwm->FSK_BUFF.WORD = fsk_encoding_data_buff[idx][0];
		epwm->FSK_CTRL.WORD |= EPWM_FSK_CTRL_INT_EN_Msk | EPWM_FSK_CTRL_FSK_EN_Msk;
	}
	else
	{
		epwm->FSK_DLY_.WORD = 0;
		epwm->FSK_BUFF.WORD = fsk_encoding_data_buff[idx][0];
		epwm->FSK_CTRL.WORD |= EPWM_FSK_CTRL_INT_EN_Msk;
		gd->fsk_silence = delay_ms;
		gd->sys_infos.tim3_evnt |= 4; //4-fsk_response_dealy
		hal_timer_init(TMR3);
	}

	fsk_is_busy[idx] = 1;
//	GPA->DOUT.BITS.PIN4 ^= 1;

	if (epwm->AFD_CTRL.BITS.AFD_EN)
	{
		fsk_dither_status[idx] = epwm->AFD_CTRL.BITS.AFD_EN;
		epwm->AFD_CTRL.BITS.AFD_EN = 0;
	}

	wpc_printk(" $:");
	for (int i=0; i<fsk_need_send_cnt[idx]; i++)
	{
		wpc_printk(" %02X", fsk_original_data_buff[idx][i]);
	}
}

/**
 * @brief  send FSK pattern data.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  FSK response delay time, unit ms.
 * @param  8bit FSK pattern.
 *
 * @return None.
 */
void fml_fsk_patt_send(TS_EPWM *epwm, uint16_t delay_ms, uint8_t pattern)
{
	uint8_t data[2] = { pattern, 0};
	fml_fsk_data_send(epwm, delay_ms, data, 1);
}

/**
 * @brief  send FSK packet data.
 *
 * @param  which EPWM is being used for FSK, must be EPWM1 or EPWM2.
 * @param  FSK response delay time, unit ms.
 * @param  packet buffer need to be send.
 * @param  the length of packet buffer need to be send.
 *
 * @return None.
 */
void fml_fsk_data_send(TS_EPWM *epwm, uint16_t delay_ms, uint8_t *data, uint8_t len)
{
	if (fml_fsk_data_encoding(epwm, data, len) < 0)
	{
		return;
	}
	fml_fsk_data_response(epwm, delay_ms);
}

static void fml_fsk_int_callback(TS_EPWM *epwm)
{
	volatile uint32_t idx = (epwm == EPWM1) ? 0 : 1;

	if (epwm->FSK_FLAG.WORD & EPWM_FSK_FLAG_BMC_BUFF_EMPTY_FLAG_Msk)
	{
		if (++fsk_have_send_cnt[idx] < fsk_need_send_cnt[idx])
		{
			epwm->FSK_BUFF.WORD = fsk_encoding_data_buff[idx][fsk_have_send_cnt[idx]];
		}
		else
		{
			epwm->FSK_BUFF.WORD = 0;
		}
		epwm->FSK_FLAG.WORD = EPWM_FSK_FLAG_BMC_BUFF_EMPTY_FLAG_Msk;
//		GPA->DOUT.BITS.PIN4 ^= 1;
	}

	if (epwm->FSK_FLAG.WORD & EPWM_FSK_FLAG_LAST_BUFF_DONE_FLAG_Msk)
	{
		osal_set_event(WPC_TASK, WPC_EVT_FSK_RESP_DONE);
		fsk_is_busy[idx] = 0;

		epwm->FSK_CTRL.WORD &= ~EPWM_FSK_CTRL_FSK_EN_Msk;
		epwm->FSK_FLAG.WORD = EPWM_FSK_FLAG_LAST_BUFF_DONE_FLAG_Msk;
//		GPA->DOUT.BITS.PIN5 ^= 1;

		if (fsk_dither_status[idx])
		{
			epwm->AFD_CTRL.BITS.AFD_EN = 1;
		}
	}
}

void __attribute__((isr)) FSK1_IRQHandler(void)
{
	fml_fsk_int_callback(EPWM1);
}

void __attribute__((isr)) FSK2_IRQHandler(void)
{
	fml_fsk_int_callback(EPWM2);
}
