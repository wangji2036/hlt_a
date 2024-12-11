/**
  ******************************************************************************
  * @file    ask.c
  * @brief   This file provides firmware functions to manage the functionalities<br>
  *          of the ASK demodulation:<br>
  *           - ask enable and disable
  *           - ask decode
  *           - ECAP channel interrupt call back handler
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
                      ##### ASK module driver features #####
  ==============================================================================
  [..]
  (#) enable ask demodulation.
  (#) disable ask demodulation.
  (#) ask bit decode.
  (#) ask bit interrupt call back handler.

                      *** How to use this driver ***
  ==============================================================================
  [..]
  (#) before power transfer, need enable ask demodulation function
  (#) when remove power transfer, it's better disable the ask demodulation function
  (#) when receive the ask bit received event, need call the ask decode function
  (#) you can call fml_ask_decode_check function to check ask demodulation

                      *** Execution of ASK operations ***
  ==============================================================================
  [..]
  (#) The driver can operate in the following modes:

     *** enable ***
     ============================
     [..]
       (+) when you are ready start EPWM for power transfer, please call enable function.

     *** decode ***
     ============================
     [..]
       (+) when received ECAP edge interrupt, please call enable function.

     *** disable ***
     ============================
     [..]
       (+) when removed power signal, please call disable function.

     *** ask check ***
     ============================
     [..]
       (+) during power transfer, you can call check function to check ask status.

  @endverbatim
*/

#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "nu103x.h"
#include "debug.h"
#include "algo.h"
#include "osal.h"
#include "_fml.h"
#include "ask.h"

extern volatile uint16_t sys_ticks;
static volatile uint16_t str_ticks, hdr_ticks, pkt_ticks;

void fml_ask_int_handler(uint8_t ask_chan, uint16_t cnt);

#define ASK_DM_CHAN_MAX     3

#define	WPC_PKT_LEN_MAX    29
#define	TIM_CAP_BUF_MSK    63 //must be 2^n-1. 7, 15, 31, 63, ...

#define BIT_ZERO            0
#define BIT_ONE             1

#define	PRMBL_CNT_MIN       5
#define	PRMBL_CNT_MAX      50

///1cnt <-> 1/1.125 = 0.8889us
//#define	FULL_BIT_0_MAX    900 //900/1.125 -> 800us
//#define	FULL_BIT_0_MIN    450 //450/1.125 -> 400us
//#define	STAR_BIT_0_MIN    405 //405/1.125 -> 360us
//#define	HALF_BIT_1_MAX    338 //338/1.125 -> 300us
//#define	HALF_BIT_1_MIN     99 // 99/1.125 ->  88us
#define	FULL_BIT_0_MAX    (800 * 1125 / 1000) //900/1.125 -> 800us
#define	FULL_BIT_0_MIN    (400 * 1125 / 1000) //450/1.125 -> 400us
#define	STAR_BIT_0_MIN    (360 * 1125 / 1000) //405/1.125 -> 360us
#define	HALF_BIT_1_MAX    (300 * 1125 / 1000) //338/1.125 -> 300us
#define	HALF_BIT_1_MIN    ( 88 * 1125 / 1000) // 99/1.125 ->  88us

/**
 * @enum pkt_error_t
 * @brief Packet Error Definition
 * @note  0: No Error
 *        1: Preamble Error
 *        2: checksum Error
 *        3: Length Error
 */
enum pkt_error_t {
	PKT_ERR_NUL = 0,
	PKT_ERR_PRM = 1,
	PKT_ERR_CHS = 2,
	PKT_ERR_LEN = 3,
};

/**
 * @brief Packet Phase Definition
 * @note  0: Preamble
 *        1: Header
 *        2: Message
 *        3: Checksum
 */
enum pkt_phase_t {
	PKT_PHS_PRM = 0,
	PKT_PHS_HDR = 1,
	PKT_PHS_MSG = 2,
	PKT_PHS_CHS = 3,
};

/**
 * @enum byt_error_t
 * @brief Byte Error Definition
 * @note  0: No Error
 *        1: Start Bit Error
 *        2: Parity Bit Error
 *        3: Stop Bit Error
 */
enum byt_error_t {
	BYT_ERR_NUL = 0,
	BYT_ERR_STR = 1,
	BYT_ERR_PTY = 2,
	BYT_ERR_STP = 3,
};

enum byt_phase_t {
	BYT_PHS_STR = 0,
	BYT_PHS_DAT = 1,
	BYT_PHS_PTY = 2,
	BYT_PHS_STP = 3,
};

enum bit_phase_t {
	BIT_PHS_NOR = 0,
	BIT_PHS_STR = 1,
	BIT_PHS_STP = 2,
};

enum bit_error_t
{
	BIT_ERR_NUL = 0,
	BIT_ERR_MAX = 1,
	BIT_ERR_MIN = 2,
	BIT_ERR_MIS = 3,
};

struct ask_dm_t {
	struct {
		uint16_t read_idx : 8;
		uint16_t writ_idx : 8;
		uint16_t tim_curr;
		uint16_t tim_last[3];
		uint16_t tim_buff[TIM_CAP_BUF_MSK + 1];
	} tim;

	struct {
		uint16_t bit_error : 4;
		uint16_t bit_phase : 4;
		uint16_t haf_1_flg : 1;
		uint16_t fuzzy_flg : 1;
		uint16_t fuzzy_cnt : 5;
		uint16_t bit_value : 1;
	} bit;

	struct {
		uint32_t byt_error : 4;
		uint32_t byt_phase : 4;
		uint32_t bit_index : 4;
		uint32_t byt_parity: 1;
		uint32_t byt_value : 8;
	} byt;

	struct {
		uint8_t pkt_error : 4;
		uint8_t pkt_phase : 4;
		uint8_t pkt_src;
		uint8_t prm_cnt;
		uint8_t byt_idx;
		uint8_t pkt_hdr;
		uint8_t pkt_len;
		uint8_t pkt_chs;
		uint8_t pkt_msg[29];
	} pkt;

	uint16_t pkt_time_stamp;
	void (*ddm_param_chose)(void);
};

static uint8_t pkt_len_get(uint8_t hdr)
{
	uint8_t len;

	if (hdr <= 0x1F)
	{
		len = 1;
	}
	else if (hdr <= 0x7F)
	{
		len = hdr >> 4;
	}
	else if (hdr <= 0xDF)
	{
		len = (hdr >> 3) - 8;
	}
	else
	{
		len = (hdr >> 2) - 36;
	}

	return len + 2;
}

static struct ask_dm_t ask_dm[ASK_DM_CHAN_MAX];

uint16_t buff[64] =
{
//		282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, //17
//		562, 281, 281, 281, 281, 563, 564, 565, 566, 567, 568, 283, 284, 285, 286, //15
//		455, 456, 457, 458, 459, 460, 461, 462, 463, 287, 288, 289, 290, //13
//		662, 181, 182, 183, 184, 663, 664, 665, 666, 667, 668, 185, 186, 187, 188, //15

//		 308, 260, 302, 257, 302, 257, 304, 255, 302, 260, 299, 260, 301, 258, 299, 262,
//		 299, 260, 300, 483, 352, 279, 471, 567, 543, 582, 536, 578, 540, 580, 363, 272,
//		 475, 566, 545, 580, 358, 274, 472, 571, 539, 360, 306, 251, 306, 262, 299, 484,
//		 350, 278, 473, 563, 365, 276, 476, 565, 541, 358, 310, 478, 352, 278, 415, 852,

//		 308, 260, 302, 257, 302, 257, 304, 255, 302, 260, 299, 260, 301, 258, 299, 262, 299,
//		 483, 352, 279, 471, 567, 543, 582, 536, 578, 540, 580, 363, 272,
//		 475, 566, 545, 580, 358, 274, 472, 571, 539, 360, 306, 251, 306, 262, 299,
//		 484, 350, 278, 473, 563, 365, 276, 476, 565, 541, 358, 310, 478, 352, 278, 415, 852,

//		 308, 260, 302, 257, 302, 257, 304, 255, 302, 260, 299, 260, 301, 258, 299, 262, 299,
//		 406, 352, 279, 471, 567, 543, 582, 536, 578, 540, 580, 363, 272,
//		 404, 566, 545, 580, 358, 274, 472, 571, 539, 360, 306, 251, 306, 262, 299,
//		 406, 350, 278, 473, 563, 365, 276, 476, 565, 541, 358, 310, 478, 352, 278, 415, 852,

//		 308, 260, 302, 257, 302, 257, 304, 255, 302, 260, 299, 260, 301, 258, 299, 262, 299,
//		 406, 352, 279, 471, 567, 543, 582, 536, 578, 540, 580, 363, 272,
//		 404, 566, 545, 580, 358, 274, 472, 571, 539, 360, 306, 251, 306, 262, 299,
//		 406, 350, 278, 473, 563, 365, 276, 476, 565, 541, 358, 310, 450, 352, 478, 415, 852,

		 308, 260, 302, 257, 302, 257, 304, 255, 302, 260, 299, 260, 301, 258, 299, 262, 299,
		 406, 352, 279, 471, 567, 543, 582, 536, 578, 540, 580, 363, 272,
		 404, 566, 545, 580, 358, 274, 472, 571, 539, 360, 306, 251, 306, 262, 299,
		 406, 350, 278, 473, 563, 365, 276, 476, 565, 541, 358, 310, 450, 352, 478, 415, 852,
};

void add(void)
{
	for (int i=0; i<64; i++)
	{
		ask_dm[0].tim.tim_buff[i] = buff[i];
	}
	ask_dm[0].tim.read_idx = 0;
	ask_dm[0].tim.writ_idx = 63;
}

void ask_decode_init(struct ask_dm_t *dm_chan);

void pkt_decode_init(struct ask_dm_t *dm_chan)
{
	dm_chan->pkt.prm_cnt = 0;
	dm_chan->pkt.pkt_error = PKT_ERR_NUL;
	dm_chan->pkt.pkt_phase = PKT_PHS_PRM;
}

void pkt_decode(struct ask_dm_t *dm_chan)
{
	switch (dm_chan->pkt.pkt_phase)
	{
		case PKT_PHS_PRM:
			break;
		case PKT_PHS_HDR:
			dm_chan->pkt.byt_idx = 0;
			dm_chan->pkt.pkt_hdr = dm_chan->byt.byt_value;
			dm_chan->pkt.pkt_len = pkt_len_get(dm_chan->pkt.pkt_hdr);
			dm_chan->pkt.pkt_chs = dm_chan->pkt.pkt_hdr;
			dm_chan->pkt.pkt_msg[dm_chan->pkt.byt_idx++] = dm_chan->pkt.pkt_hdr;
			dm_chan->pkt.pkt_phase = PKT_PHS_MSG;
			if ((uint16_t)(sys_ticks - hdr_ticks) > 25 && (uint16_t)(sys_ticks - pkt_ticks) > 10)
			{
				hdr_ticks = sys_ticks;
				gd->wpc_pkt.hdr = dm_chan->pkt.pkt_hdr;
				gd->wpc_pkt.len = dm_chan->pkt.pkt_len;
				gd->wpc_pkt.src = dm_chan->pkt.pkt_src;
				osal_set_event(WPC_TASK, WPC_EVT_HDR_RECVD);
			}
			break;
		case PKT_PHS_MSG:
			dm_chan->pkt.pkt_chs ^= dm_chan->byt.byt_value;
			dm_chan->pkt.pkt_msg[dm_chan->pkt.byt_idx++] = dm_chan->byt.byt_value;
			if (dm_chan->pkt.byt_idx == dm_chan->pkt.pkt_len - 1)
			{
				dm_chan->pkt.pkt_phase = PKT_PHS_CHS;
			}
			break;
		case PKT_PHS_CHS:
			dm_chan->pkt.pkt_msg[dm_chan->pkt.byt_idx++] = dm_chan->byt.byt_value;
			if (dm_chan->pkt.pkt_chs != dm_chan->byt.byt_value)
			{
				dm_chan->pkt.pkt_error = PKT_ERR_CHS;
			}
			else
			{
				if ((uint16_t)(sys_ticks - pkt_ticks) > 20) //due to the unsigned subtraction feature, there will be no problem when data overflow
				{
					pkt_ticks = sys_ticks;
					gd->wpc_pkt.hdr = dm_chan->pkt.pkt_hdr;
					gd->wpc_pkt.len = dm_chan->pkt.pkt_len;
					gd->wpc_pkt.src = dm_chan->pkt.pkt_src;
					for (int i=0; i<dm_chan->pkt.pkt_len; i++)
					{
						gd->wpc_pkt.data[i] = dm_chan->pkt.pkt_msg[i];
					}
					osal_set_event(WPC_TASK, WPC_EVT_PKT_RECVD);
//					GPA->DOUT.BITS.PIN4 ^= 1;
				}
				ask_decode_init(dm_chan);
				dm_chan->pkt_time_stamp = sys_ticks;

//				printk("\r\n [dmo-%d(%d)]", dm_chan->pkt.pkt_src, gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE);
//				for (int i=0; i<dm_chan->pkt.pkt_len; i++)
//				{
//					printk(" %02X", dm_chan->pkt.pkt_msg[i]);
//				}

//				if (dm_chan->pkt.pkt_src == 2)
//				{
//					GPC->DOUT.BITS.PIN6 ^= 1;
//				}
//
//				if (dm_chan->pkt.pkt_src == 3)
//				{
//					GPB->DOUT.BITS.PIN5 ^= 1;
//				}
			}
			break;
		default:
			break;
	}
}

/**
 * @brief Initialize the byte decode opreation.
 * 		  - Clear byte error flag.
 * 		  - Set byte phase to start state.
 * @param dm_chan 
 */
void byt_decode_init(struct ask_dm_t *dm_chan)
{
	dm_chan->byt.byt_error = BYT_ERR_NUL;
	dm_chan->byt.byt_phase = BYT_ERR_STR;
}

/**
 * @brief Byte decode function.
 * 		  - Start bit: 0, must be 0.
 * 		  - Data bits: 8bits
 * 		  - Parity bit: 1
 * 		  - Parity check: Even parity
 * 		  - Stop bit: 1
 * @param dm_chan data message channel
 * @return void
 */
void byt_decode(struct ask_dm_t *dm_chan)
{
	if (dm_chan->pkt.pkt_phase == PKT_PHS_PRM)
	{
		if (dm_chan->bit.bit_value == 1)
		{
			if (++dm_chan->pkt.prm_cnt > PRMBL_CNT_MAX)
			{
				dm_chan->pkt.pkt_error = PKT_ERR_PRM;
			}
		}
		else
		{
			if (dm_chan->pkt.prm_cnt < PRMBL_CNT_MIN)
			{
				dm_chan->pkt.pkt_error = PKT_ERR_PRM;
			}
			else
			{
				dm_chan->pkt.pkt_phase = PKT_PHS_HDR;
				dm_chan->byt.byt_phase = BYT_PHS_STR;
				if ((uint16_t)(sys_ticks - str_ticks) > 25 && (uint16_t)(sys_ticks - pkt_ticks) > 10)
				{
					str_ticks = sys_ticks;
//					wpc_evnt |= WPC_EVNT_HDR_START;
					osal_set_event(WPC_TASK, WPC_EVT_HDR_START);
				}
			}
		}
		if (dm_chan->pkt.pkt_phase == PKT_PHS_PRM)
		{
			return;
		}
	}

	switch (dm_chan->byt.byt_phase)
	{
		case BYT_PHS_STR:
			if (dm_chan->bit.bit_value == 0)
			{
				dm_chan->byt.bit_index = 0;
				dm_chan->byt.byt_value = 0;
				dm_chan->byt.byt_parity = 1;
				dm_chan->byt.byt_phase = BYT_PHS_DAT;
			}
			else
			{
				dm_chan->byt.byt_error = BYT_ERR_STR;
			}
			break;
		case BYT_PHS_DAT:
			dm_chan->byt.byt_value |= (dm_chan->bit.bit_value << dm_chan->byt.bit_index);
			dm_chan->byt.byt_parity ^= dm_chan->bit.bit_value;
			if (++dm_chan->byt.bit_index >= 8)
			{
				dm_chan->byt.byt_phase = BYT_PHS_PTY;
			}
			break;
		case BYT_PHS_PTY:
			if (dm_chan->bit.bit_value == dm_chan->byt.byt_parity)
			{
				dm_chan->byt.byt_phase = BYT_PHS_STP;
			}
			else
			{
				dm_chan->byt.byt_error = BYT_ERR_PTY;
			}
			break;
		case BYT_PHS_STP:
			if (dm_chan->bit.bit_value == 1)
			{
				dm_chan->byt.byt_phase = BYT_PHS_STR;
				pkt_decode(dm_chan);
			}
			else
			{
				dm_chan->byt.byt_error = BYT_ERR_STP;
			}
			break;
		default:
			break;
	}
}

void bit_decode_init(struct ask_dm_t *dm_chan)
{
	dm_chan->bit.bit_error = BIT_ERR_NUL;
	dm_chan->bit.bit_phase = BIT_PHS_NOR;
	dm_chan->bit.haf_1_flg = 0;
	dm_chan->bit.fuzzy_flg = 0;
}

void bit_decode(struct ask_dm_t *dm_chan)
{
	if (dm_chan->pkt.pkt_phase == PKT_PHS_CHS && dm_chan->byt.byt_phase == BYT_PHS_STP)
	{
		if (dm_chan->bit.haf_1_flg == 1 || dm_chan->bit.fuzzy_flg == 1)
		{
			dm_chan->bit.haf_1_flg = 0;
			dm_chan->bit.fuzzy_flg = 0;
			dm_chan->bit.bit_value = 1;
			byt_decode(dm_chan);
		}
		else
		{
			dm_chan->bit.haf_1_flg = 1;
		}
		return;
	}

	if (dm_chan->tim.tim_curr > FULL_BIT_0_MAX)
	{
		dm_chan->bit.bit_error = BIT_ERR_MAX;
	}
	else if (dm_chan->tim.tim_curr >= FULL_BIT_0_MIN)
	{
		if (dm_chan->pkt.pkt_phase == PKT_PHS_PRM) //Preamble state allow half ONE bit or suspect bit
		{
			dm_chan->bit.haf_1_flg = 0;
			dm_chan->bit.fuzzy_flg = 0;
			dm_chan->bit.bit_value = 0;
			byt_decode(dm_chan);
		}
		else
		{
			if (dm_chan->bit.haf_1_flg == 1) //there is half ONE between two bit, which shows error
			{
				dm_chan->bit.bit_error = BIT_ERR_MIS;
			}
			else
			{
				if (dm_chan->bit.fuzzy_flg == 1)
				{
					if (dm_chan->bit.fuzzy_cnt == 1)
					{
						dm_chan->bit.fuzzy_flg = 0;
						dm_chan->bit.bit_value = 0; //previous suspect bit is judged as ZERO
						byt_decode(dm_chan);
					}
					else if (dm_chan->bit.fuzzy_cnt == 2)
					{
						dm_chan->bit.fuzzy_flg = 0;
						dm_chan->bit.bit_value = 1; //previous 2 suspect bits is judged as ONE
						byt_decode(dm_chan);
					}
					else if (dm_chan->bit.fuzzy_cnt == 3)
					{
						dm_chan->bit.fuzzy_flg = 0;
						dm_chan->bit.bit_value = 0; //1st suspect bits is judged as ZER0
						byt_decode(dm_chan);
						dm_chan->bit.bit_value = 1; //2nd/3rd suspect bits is judged as ONE
						byt_decode(dm_chan);
					}
				}
				dm_chan->bit.bit_value = 0;
				byt_decode(dm_chan);
			}
		}
	}
	else if (dm_chan->tim.tim_curr >= HALF_BIT_1_MAX) //300us <= width < 440us, unsure pulse width
	{
		if (dm_chan->byt.byt_phase == BYT_PHS_STR || dm_chan->pkt.pkt_phase == PKT_PHS_PRM)
		{
			if (dm_chan->tim.tim_curr > STAR_BIT_0_MIN)
			{
				dm_chan->bit.haf_1_flg = 0;
				dm_chan->bit.fuzzy_flg = 0;
				dm_chan->bit.bit_value = 0;
				byt_decode(dm_chan);
			}
			else
			{
				if (dm_chan->bit.haf_1_flg == 1)
				{
					dm_chan->bit.haf_1_flg = 0;
					dm_chan->bit.fuzzy_flg = 0;
					dm_chan->bit.bit_value = 1;
					byt_decode(dm_chan);
				}
				else
				{
					if (dm_chan->bit.fuzzy_flg == 1)
					{
						dm_chan->bit.haf_1_flg = 0;
						dm_chan->bit.fuzzy_flg = 0;
						dm_chan->bit.bit_value = 1;
						byt_decode(dm_chan);
					}
					else
					{
						dm_chan->bit.fuzzy_flg = 1;
						dm_chan->bit.fuzzy_cnt = 1;
					}
				}
			}
		}
		else
		{
			if (dm_chan->bit.haf_1_flg == 1)
			{
				dm_chan->bit.haf_1_flg = 0;
				dm_chan->bit.fuzzy_flg = 0;
				dm_chan->bit.bit_value = 1;
				byt_decode(dm_chan);
			}
			else
			{
				if (dm_chan->bit.fuzzy_flg == 1)
				{
					if (++dm_chan->bit.fuzzy_cnt >= 4)
					{
						dm_chan->bit.haf_1_flg = 0;
						dm_chan->bit.fuzzy_cnt -= 2;
						dm_chan->bit.bit_value = 1;
						byt_decode(dm_chan);
					}
//					dm_chan->bit.bit_value = 1;
//					byt_decode(dm_chan);
				}
				else
				{
					dm_chan->bit.fuzzy_flg = 1;
					dm_chan->bit.fuzzy_cnt = 1;
				}
			}
		}
	}
	else if (dm_chan->tim.tim_curr >= HALF_BIT_1_MIN) // 88us <= width < 300us
	{
		if (dm_chan->bit.haf_1_flg == 1)
		{
			dm_chan->bit.haf_1_flg = 0;
			dm_chan->bit.fuzzy_flg = 0;
			dm_chan->bit.bit_value = 1;
			byt_decode(dm_chan);
		}
		else
		{
			if (dm_chan->bit.fuzzy_flg == 1)
			{
				if (dm_chan->bit.fuzzy_cnt == 1)
				{
					dm_chan->bit.fuzzy_flg = 0;
					dm_chan->bit.bit_value = 1; //previous suspect bit is judged as half ONE
					byt_decode(dm_chan);
				}
				else if (dm_chan->bit.fuzzy_cnt == 2)
				{
					dm_chan->bit.fuzzy_flg = 0;
					dm_chan->bit.bit_value = 0; //1st suspect bits is judged as ZERO
					byt_decode(dm_chan);
					dm_chan->bit.bit_value = 1; //2nd suspect bit is judged as half ONE
					byt_decode(dm_chan);
				}
				else if (dm_chan->bit.fuzzy_cnt == 3)
				{
					dm_chan->bit.fuzzy_flg = 0;
					dm_chan->bit.bit_value = 1; //1st-2nd suspect bits is judged as ONE
					byt_decode(dm_chan);
					dm_chan->bit.bit_value = 1; //3rd suspect bits is judged as half ONE
					byt_decode(dm_chan);
				}
			}
			else
			{
				dm_chan->bit.haf_1_flg = 1;
			}
		}
	}
	else
	{
		dm_chan->bit.bit_error = BIT_ERR_MIN;
	}
}

void ask_decode_init(struct ask_dm_t *dm_chan)
{
	bit_decode_init(dm_chan);
	byt_decode_init(dm_chan);
	pkt_decode_init(dm_chan);
}

void tim_decode(struct ask_dm_t *dm_chan)
{
	while (dm_chan->tim.read_idx != dm_chan->tim.writ_idx)
	{
		dm_chan->tim.tim_curr = dm_chan->tim.tim_buff[dm_chan->tim.read_idx++];
		dm_chan->tim.read_idx &= TIM_CAP_BUF_MSK;
		bit_decode(dm_chan);
		if (dm_chan->bit.bit_error != BIT_ERR_NUL || dm_chan->byt.byt_error != BYT_ERR_NUL || dm_chan->pkt.pkt_error != PKT_ERR_NUL)
		{
//			printk("\r\n error-> %d %d %d %d", dm_chan->tim.tim_curr, dm_chan->bit.bit_error, dm_chan->byt.byt_error, dm_chan->pkt.pkt_error);
			ask_decode_init(dm_chan);
		}
	}
}

/**
 * @brief  ask decode function.
 *
 * When receiving the capture signal from ECAP, call this function can decode bit-byte-packet.
 * If the package is successfully received, an event notification will be sent to the application layer.
 *
 * @param  None.
 * @return None.
 */
void fml_ask_decode(void)
{
	int i;
	for (i=0; i<sizeof(ask_dm)/sizeof(ask_dm[0]); i++)
	{
		ask_dm[i].pkt.pkt_src = i + 1;
		tim_decode(&ask_dm[i]);
	}
}

/**
 * @brief  enable ask decode function.
 *
 * when ready to start EPWM for power transfer, call enable function to initialize the hal_layer demodulation driver
 *
 * @param  None.
 * @return None.
 */
void fml_ask_enbale(void)
{
	hal_ecap_init(ECAP1, _ECAP_FUNC_MODE_DDM);
	hal_ecap_open(ECAP1);
	hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_DDM);
	hal_ecap_open(ECAP2);
	hal_ecap_reg_int_cb(fml_ask_int_handler);

	for (int i=0; i<sizeof(ask_dm)/sizeof(ask_dm[0]); i++)
	{
		ask_dm[i].pkt_time_stamp = sys_ticks;
	}

	ask_dm[0].ddm_param_chose = fml_nu103x_dmo1_xfer_param_chose;
	ask_dm[1].ddm_param_chose = fml_nu103x_dmo2_xfer_param_chose;
	ask_dm[2].ddm_param_chose = NULL;
}

/**
 * @brief  enable ask decode function.
 *
 * when removed power signal, call disable function to disable the hal_layer demodulation driver
 *
 * @param  None.
 * @return None.
 */
void fml_ask_disable(void)
{
	hal_ecap_close(ECAP1);
	hal_ecap_close(ECAP2);
	hal_ecap_close(ECAP4);
	hal_ecap_reg_int_cb(NULL);

	ask_dm[0].ddm_param_chose = NULL;
	ask_dm[1].ddm_param_chose = NULL;
	ask_dm[2].ddm_param_chose = NULL;
}

void fml_ask_int_handler(uint8_t ask_chan, uint16_t cnt)
{
	ask_dm[ask_chan].tim.tim_buff[ask_dm[ask_chan].tim.writ_idx++] = cnt;
	ask_dm[ask_chan].tim.writ_idx &= TIM_CAP_BUF_MSK;
	osal_set_event(FML_TASK, FML_EVT_ASK_INT_RECVD);
}

/**
 * @brief  check ask decode function.
 *
 * during power transfer, call the check function to check the demodulation status
 * of each channel and perform some processing.
 *
 * @param  None.
 * @return None.
 */
void fml_ask_decode_check(void)
{
	for (int i=0; i<sizeof(ask_dm)/sizeof(ask_dm[0]); i++)
	{
		if ((uint16_t)(sys_ticks - ask_dm[i].pkt_time_stamp) > 350)
		{
			ask_dm[i].pkt_time_stamp = sys_ticks;
			if (ask_dm[i].ddm_param_chose != NULL)
			{
				ask_dm[i].ddm_param_chose();
			}
		}
	}
}

void fml_test_ask_info_print(uint8_t chan)
{
	char s[50] = {'\0'};
	char *s1 = "";
	char *s2 = "";
	char *s3 = "";

	if (chan == 1)
	{
		if (gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC == _NU1030_DMO1_DDM_SRC_IAVG)
		{
			s1 = "-iavg";
		}
		else
		{
			s1 = "-evdm";
		}

		if (gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_MOD == _NU1030_DMO1_DDM_GAIN_MODE_AUTO)
		{
			s2 = "-gain_mode_auto";
		}
		else
		{
			if (gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_FIX == _NU1030_DMO1_DDM_GAIN_FIXED_X36)
			{
				s2 = "-gain_fixed_x36";
			}
			else
			{
				s2 = "-gain_fixed_x60";
			}
		}
	}
	else if (chan == 2)
	{
		if (gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE == _NU1030_DMO2_OUT_MODE_DDM)
		{
			if (gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC == _NU1030_DMO2_DDM_SRC_VCAP)
			{
				s1 = "-vcap";

				if (gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K == _NU1030_DMO2_VCAP_RATIO_K2)
				{
					s3 = "-K2";
				}
				else if (gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K == _NU1030_DMO2_VCAP_RATIO_K3)
				{
					s3 = "-K3";
				}
				else if (gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K == _NU1030_DMO2_VCAP_RATIO_K1)
				{
					s3 = "-K1";
				}
			}
			else
			{
				s1 = "-phas";
			}

			if (gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_MOD == _NU1030_DMO2_DDM_GAIN_MODE_AUTO)
			{
				s2 = "-gain_mode_auto";
			}
			else
			{
				if (gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_FIX == _NU1030_DMO2_DDM_GAIN_FIXED_X36)
				{
					s2 = "-gain_fixed_x36";
				}
				else
				{
					s2 = "-gain_fixed_x60";
				}
			}
		}
		else if (gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE == _NU1030_DMO2_OUT_MODE_CAP)
		{
			s1 = "-dig_pha";
		}
	}
	else if (chan == 3)
	{
		s1 = "-dig_mag";
	}

//	printk("[dmo%d%s%s%s]", gd->wpc_pkt.src, s1, s2, s3);

	str_concat(s, s1);
	str_concat(s, s2);
	str_concat(s, s3);

	printk("[dmo%d%s]", gd->wpc_pkt.src, s);
}
