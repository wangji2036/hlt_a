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
#include "_wpc.h"

extern volatile uint16_t sys_ticks;
static volatile uint16_t str_ticks, hdr_ticks, pkt_ticks;

void fml_ask_int_handler(uint8_t ask_chan, uint16_t cnt);

#define ASK_DM_CHAN_MAX     3
#define MAX_SUB_DCODE		2

#define	WPC_PKT_LEN_MAX    29
#define	TIM_CAP_BUF_MSK    63 //must be 2^n-1. 7, 15, 31, 63, ...

#define BIT_ZERO            0
#define BIT_ONE             1

#define	PRMBL_CNT_MIN       8
#define	PRMBL_CNT_MAX      30

#define	FULL_BIT_0_MAX_THD1    (880 * 1125 / 1000) //900/1.125 -> 800us
#define	FULL_BIT_0_MIN_THD1    (390 * 1125 / 1000) //450/1.125 -> 400us//fuzzy max//390 OK
#define	STAR_BIT_0_MIN_THD1    (370 * 1125 / 1000) //405/1.125 -> 360us//375 OK//380 OK
#define	HALF_BIT_1_MAX_THD1    (300 * 1125 / 1000) //338/1.125 -> 300us//fuzzy min//290 OK//310 OK
#define	HALF_BIT_1_MIN_THD1    (135 * 1125 / 1000) //135/1.125 -> 120us//160//180 OK//premble
#define	DATA_BIT_1_MIN_THD1    (60  * 1125 / 1000)

#define	FULL_BIT_0_MAX_THD2    (880 * 1125 / 1000) //900/1.125 -> 800us
#define	FULL_BIT_0_MIN_THD2    (388 * 1125 / 1000) //450/1.125 -> 400us//fuzzy max//390 OK
#define	STAR_BIT_0_MIN_THD2    (375 * 1125 / 1000) //405/1.125 -> 360us//375 OK//380 OK
#define	HALF_BIT_1_MAX_THD2    (300 * 1125 / 1000) //338/1.125 -> 300us//fuzzy min//290 OK//310 OK
#define	HALF_BIT_1_MIN_THD2    (150 * 1125 / 1000) //135/1.125 -> 120us//160//180 OK//
#define	DATA_BIT_1_MIN_THD2    (60  * 1125 / 1000)
/*
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
#define	HALF_BIT_1_MIN    (120 * 1125 / 1000) //120/1.125 -> 120us
*/
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

struct decode_t
{
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
	uint16_t bit_zero_max, bit_zero_min, bit_one_max, bit_one_min, preamble_bit_min, start_bit_min;
};

struct ask_dm_t {
	struct {
		uint16_t read_idx : 8;
		uint16_t writ_idx : 8;
		uint16_t tim_curr;
		uint16_t tim_last[3];
		uint16_t tim_buff[TIM_CAP_BUF_MSK + 1];
	} tim;

	struct decode_t decode[MAX_SUB_DCODE];

	uint8_t dm_bad_cnt;
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

/*
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
*/

void ask_decode_init(struct ask_dm_t *dm_chan, struct decode_t *decode);

void pkt_decode_init(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	decode->pkt.prm_cnt = 0;
	decode->pkt.pkt_error = PKT_ERR_NUL;
	decode->pkt.pkt_phase = PKT_PHS_PRM;
}

void pkt_decode(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	switch (decode->pkt.pkt_phase)
	{
		case PKT_PHS_PRM:
			break;
		case PKT_PHS_HDR:
			if (decode->byt.byt_value >= 0xBB && gd->pid_perd == _360K_EPWM_PERD)//filter the 360K long header to optimize the demodulation, better at ping phase
			{
				decode->pkt.pkt_error = PKT_ERR_LEN;
			}
			else
			{
				decode->pkt.byt_idx = 0;
				decode->pkt.pkt_hdr = decode->byt.byt_value;
				decode->pkt.pkt_len = pkt_len_get(decode->pkt.pkt_hdr);
				decode->pkt.pkt_chs = decode->pkt.pkt_hdr;
				decode->pkt.pkt_msg[decode->pkt.byt_idx++] = decode->pkt.pkt_hdr;
				decode->pkt.pkt_phase = PKT_PHS_MSG;
				if ((uint16_t)(sys_ticks - hdr_ticks) > 25 && (uint16_t)(sys_ticks - pkt_ticks) > 10)
				{
					hdr_ticks = sys_ticks;
					gd->wpc_pkt.hdr = decode->pkt.pkt_hdr;
					gd->wpc_pkt.len = decode->pkt.pkt_len;
					gd->wpc_pkt.src = decode->pkt.pkt_src;

					if (decode != dm_chan->decode)
					{
						gd->wpc_pkt.mark = 0x01;
					}
					else
					{
						gd->wpc_pkt.mark = 0;
					}
					
					osal_set_event(WPC_TASK, WPC_EVT_HDR_RECVD);
					//GPA->DOUT.BITS.PIN5 ^= 1;
				}
			}
		#if DDM_DEBUG_TOGGLE
			if (decode->pkt.pkt_src == 1)
			{
				GPC->DOUT.BITS.PIN5 ^= 1;
			}
			else if (decode->pkt.pkt_src == 2)
			{
				GPB->DOUT.BITS.PIN3 ^= 1;
			}
		#endif
			break;
		case PKT_PHS_MSG:
			decode->pkt.pkt_chs ^= decode->byt.byt_value;
			decode->pkt.pkt_msg[decode->pkt.byt_idx++] = decode->byt.byt_value;
			if (decode->pkt.byt_idx == decode->pkt.pkt_len - 1)
			{
				decode->pkt.pkt_phase = PKT_PHS_CHS;
			}
			break;
		case PKT_PHS_CHS:
			decode->pkt.pkt_msg[decode->pkt.byt_idx++] = decode->byt.byt_value;
			if (decode->pkt.pkt_chs != decode->byt.byt_value)
			{
				decode->pkt.pkt_error = PKT_ERR_CHS;
				// osal_set_event(WPC_TASK, WPC_EVT_PKT_ERROR);
			}
			else
			{
				if ((uint16_t)(sys_ticks - pkt_ticks) > 20) //due to the unsigned subtraction feature, there will be no problem when data overflow
				{
					pkt_ticks = sys_ticks;
					gd->wpc_pkt.hdr = decode->pkt.pkt_hdr;
					gd->wpc_pkt.len = decode->pkt.pkt_len;
					gd->wpc_pkt.src = decode->pkt.pkt_src;

					if (decode != dm_chan->decode)
					{
						gd->wpc_pkt.mark = 0x01;
					}
					else
					{
						gd->wpc_pkt.mark = 0;
					}

					for (int i=0; i<decode->pkt.pkt_len; i++)
					{
						gd->wpc_pkt.data[i] = decode->pkt.pkt_msg[i];
					}
					osal_set_event(WPC_TASK, WPC_EVT_PKT_RECVD);
//					GPA->DOUT.BITS.PIN4 ^= 1;
				}

#if DDM_DEBUG_TOGGLE//ddm debug print
				if (decode->pkt.pkt_src == 1)
				{
					GPC->DOUT.BITS.PIN5 ^= 1;
					wpc_printk("\r\n #:[%d:%d]", decode->pkt.pkt_src, gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC);		
				}
				else if (decode->pkt.pkt_src == 2)
				{
					GPB->DOUT.BITS.PIN3 ^= 1;
					wpc_printk("\r\n #:[%d:%d]", decode->pkt.pkt_src, gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC);
				}
				else
				{
					wpc_printk("\r\n #:[%d:%d]", decode->pkt.pkt_src, 0);
				}

				for (int i=0; i<decode->pkt.pkt_len; i++)
				{
					wpc_printk(" %02X", decode->pkt.pkt_msg[i]);
				}	
#endif
				ask_decode_init(dm_chan, decode);
				dm_chan->pkt_time_stamp = sys_ticks;
				dm_chan->dm_bad_cnt = 0;

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
void byt_decode_init(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	decode->byt.byt_error = BYT_ERR_NUL;
	decode->byt.byt_phase = BYT_PHS_STR;
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
void byt_decode(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	if (decode->pkt.pkt_phase == PKT_PHS_PRM)
	{
		if (decode->bit.bit_value == 1)
		{
			if (++decode->pkt.prm_cnt > PRMBL_CNT_MAX)
			{
				decode->pkt.pkt_error = PKT_ERR_PRM;
			}
		}
		else
		{
			if (decode->pkt.prm_cnt < PRMBL_CNT_MIN)
			{
				decode->pkt.pkt_error = PKT_ERR_PRM;
			}
			else
			{
				decode->pkt.pkt_phase = PKT_PHS_HDR;
				decode->byt.byt_phase = BYT_PHS_STR;
				if ((uint16_t)(sys_ticks - str_ticks) > 25 && (uint16_t)(sys_ticks - pkt_ticks) > 10)
				{
					str_ticks = sys_ticks;
//					wpc_evnt |= WPC_EVNT_HDR_START;
					osal_set_event(WPC_TASK, WPC_EVT_HDR_START);
				}
			}
		}
		if (decode->pkt.pkt_phase == PKT_PHS_PRM)
		{
			return;
		}
	}

	switch (decode->byt.byt_phase)
	{
		case BYT_PHS_STR:
			if (decode->bit.bit_value == 0)
			{
				decode->byt.bit_index = 0;
				decode->byt.byt_value = 0;
				decode->byt.byt_parity = 1;
				decode->byt.byt_phase = BYT_PHS_DAT;
			}
			else
			{
				decode->byt.byt_error = BYT_ERR_STR;
			}
			break;
		case BYT_PHS_DAT:
			decode->byt.byt_value |= (decode->bit.bit_value << decode->byt.bit_index);
			decode->byt.byt_parity ^= decode->bit.bit_value;
			if (++decode->byt.bit_index >= 8)
			{
				decode->byt.byt_phase = BYT_PHS_PTY;
			}
			break;
		case BYT_PHS_PTY:
			if (decode->bit.bit_value == decode->byt.byt_parity)
			{
				decode->byt.byt_phase = BYT_PHS_STP;
			}
			else
			{
				decode->byt.byt_error = BYT_ERR_PTY;
			}
			break;
		case BYT_PHS_STP:
			if (decode->bit.bit_value == 1)
			{
				decode->byt.byt_phase = BYT_PHS_STR;
				pkt_decode(dm_chan, decode);
			}
			else
			{
				decode->byt.byt_error = BYT_ERR_STP;
			}
			break;
		default:
			break;
	}
}

void bit_decode_init(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	decode->bit.bit_error = BIT_ERR_NUL;
	decode->bit.bit_phase = BIT_PHS_NOR;
	decode->bit.haf_1_flg = 0;
	decode->bit.fuzzy_flg = 0;
}

void bit_decode(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	if (decode->pkt.pkt_phase == PKT_PHS_CHS && decode->byt.byt_phase == BYT_PHS_STP)
	{
		if (decode->bit.haf_1_flg == 1 || decode->bit.fuzzy_flg == 1)
		{
			decode->bit.haf_1_flg = 0;
			decode->bit.fuzzy_flg = 0;
			decode->bit.bit_value = 1;
			byt_decode(dm_chan, decode);
		}
		else
		{
			decode->bit.haf_1_flg = 1;
		}
		return;
	}

	if (dm_chan->tim.tim_curr > decode->bit_zero_max)
	{
		decode->bit.bit_error = BIT_ERR_MAX;
	}
	else if (dm_chan->tim.tim_curr >= decode->bit_zero_min)//FULL_BIT_0_MIN
	{
		if (decode->pkt.pkt_phase == PKT_PHS_PRM) //Preamble state allow half ONE bit or suspect bit
		{
			decode->bit.haf_1_flg = 0;
			decode->bit.fuzzy_flg = 0;
			decode->bit.bit_value = 0;
			byt_decode(dm_chan, decode);
		}
		else
		{
			if (decode->bit.haf_1_flg == 1) //there is half ONE between two bit, which shows error
			{
				decode->bit.bit_error = BIT_ERR_MIS;//TODO: loose to 420 as bit 1?
			}
			else
			{
				if (decode->bit.fuzzy_flg == 1)
				{
					if (decode->bit.fuzzy_cnt == 1)
					{
						decode->bit.fuzzy_flg = 0;
						decode->bit.bit_value = 0; //previous suspect bit is judged as ZERO
						byt_decode(dm_chan, decode);
					}
					else if (decode->bit.fuzzy_cnt == 2)
					{
						decode->bit.fuzzy_flg = 0;
						decode->bit.bit_value = 1; //previous 2 suspect bits is judged as ONE
						byt_decode(dm_chan, decode);
					}
					else if (decode->bit.fuzzy_cnt == 3)
					{
						decode->bit.fuzzy_flg = 0;
						decode->bit.bit_value = 0; //1st suspect bits is judged as ZER0
						byt_decode(dm_chan, decode);
						decode->bit.bit_value = 1; //2nd/3rd suspect bits is judged as ONE
						byt_decode(dm_chan, decode);
					}
				}
				decode->bit.bit_value = 0;
				byt_decode(dm_chan, decode);
			}
		}
	}
	else if (dm_chan->tim.tim_curr >= decode->bit_one_max)//HALF_BIT_1_MAX //300us <= width < 440us, unsure pulse width
	{
		if (decode->byt.byt_phase == BYT_PHS_STR || decode->pkt.pkt_phase == PKT_PHS_PRM)
		{
			if (dm_chan->tim.tim_curr > decode->start_bit_min)//STAR_BIT_0_MIN //>360
			{
				decode->bit.haf_1_flg = 0;
				decode->bit.fuzzy_flg = 0;
				decode->bit.bit_value = 0;
				byt_decode(dm_chan, decode);
			}
			else
			{
				if (decode->bit.haf_1_flg == 1)
				{
					decode->bit.haf_1_flg = 0;
					decode->bit.fuzzy_flg = 0;
					decode->bit.bit_value = 1;
					byt_decode(dm_chan, decode);
				}
				else
				{
					if (decode->bit.fuzzy_flg == 1)
					{
						decode->bit.haf_1_flg = 0;
						decode->bit.fuzzy_flg = 0;
						decode->bit.bit_value = 1;
						byt_decode(dm_chan, decode);
					}
					else
					{
						decode->bit.fuzzy_flg = 1;
						decode->bit.fuzzy_cnt = 1;
					}
				}
			}
		}
		else
		{
			if (decode->bit.haf_1_flg == 1)
			{
				decode->bit.haf_1_flg = 0;
				decode->bit.fuzzy_flg = 0;
				decode->bit.bit_value = 1;
				byt_decode(dm_chan, decode);
			}
			else
			{
				if (decode->bit.fuzzy_flg == 1)
				{
					if (++decode->bit.fuzzy_cnt >= 4)
					{
						decode->bit.haf_1_flg = 0;
						decode->bit.fuzzy_cnt -= 2;
						decode->bit.bit_value = 1;
						byt_decode(dm_chan, decode);
					}
//					dm_chan->bit.bit_value = 1;
//					byt_decode(dm_chan);
				}
				else
				{
					decode->bit.fuzzy_flg = 1;
					decode->bit.fuzzy_cnt = 1;
				}
			}
		}
	}
	else if ((dm_chan->tim.tim_curr >= decode->preamble_bit_min) || \
			((dm_chan->tim.tim_curr >= decode->bit_one_min) && (decode->pkt.pkt_phase > PKT_PHS_PRM))) // 88us <= width < 300us
	{
		if (decode->bit.haf_1_flg == 1)
		{
			decode->bit.haf_1_flg = 0;
			decode->bit.fuzzy_flg = 0;
			decode->bit.bit_value = 1;
			byt_decode(dm_chan, decode);
		}
		else
		{
			if (decode->bit.fuzzy_flg == 1)
			{
				if (decode->bit.fuzzy_cnt == 1)
				{
					decode->bit.fuzzy_flg = 0;
					decode->bit.bit_value = 1; //previous suspect bit is judged as half ONE
					byt_decode(dm_chan, decode);
				}
				else if (decode->bit.fuzzy_cnt == 2)
				{
					decode->bit.fuzzy_flg = 0;
					decode->bit.bit_value = 0; //1st suspect bits is judged as ZERO
					byt_decode(dm_chan, decode);
					decode->bit.bit_value = 1; //2nd suspect bit is judged as half ONE
					byt_decode(dm_chan, decode);
				}
				else if (decode->bit.fuzzy_cnt == 3)
				{
					decode->bit.fuzzy_flg = 0;
					decode->bit.bit_value = 1; //1st-2nd suspect bits is judged as ONE
					byt_decode(dm_chan, decode);
					decode->bit.bit_value = 1; //3rd suspect bits is judged as half ONE
					byt_decode(dm_chan, decode);
				}
			}
			else
			{
				decode->bit.haf_1_flg = 1;
			}
		}
	}
	else
	{
		decode->bit.bit_error = BIT_ERR_MIN;
	}
}

void ask_decode_init(struct ask_dm_t *dm_chan, struct decode_t *decode)
{
	bit_decode_init(dm_chan, decode);
	byt_decode_init(dm_chan, decode);
	pkt_decode_init(dm_chan, decode);
}

void tim_decode(struct ask_dm_t *dm_chan)
{
	while (dm_chan->tim.read_idx != dm_chan->tim.writ_idx)
	{
		dm_chan->tim.tim_curr = dm_chan->tim.tim_buff[dm_chan->tim.read_idx++];
		dm_chan->tim.read_idx &= TIM_CAP_BUF_MSK;

		struct decode_t *decode;
		for (uint8_t k = 0; k < MAX_SUB_DCODE; k++)
		{
			decode = &dm_chan->decode[k];
			bit_decode(dm_chan, decode);

			if (decode->bit.bit_error != BIT_ERR_NUL || decode->byt.byt_error != BYT_ERR_NUL || decode->pkt.pkt_error != PKT_ERR_NUL)
			{
				ask_decode_init(dm_chan, decode);
			}
		}
		#if DDM_DEBUG_TOGGLE
		if (dm_chan->pkt.pkt_phase > PKT_PHS_HDR && dm_chan->pkt.pkt_hdr > 0x80 && dm_chan->pkt.pkt_hdr < 0xB0)
		{
			wpc_printk("\r\n err-> %d %d %d %d %d %d *",dm_chan->pkt.pkt_src, dm_chan->tim.read_idx, dm_chan->tim.tim_curr, dm_chan->bit.bit_error, dm_chan->byt.byt_error, dm_chan->pkt.pkt_error);

			for (int i=0; i<dm_chan->pkt.byt_idx; i++)
			{
				wpc_printk(" %02X", dm_chan->pkt.pkt_msg[i]);
			}
		}
		#endif
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
		for (uint8_t k = 0; k < MAX_SUB_DCODE; k++)
		{
			ask_dm[i].decode[k].pkt.pkt_src = i + 1;
		}
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
void fml_ask_enable(void)
{
	hal_ecap_init(ECAP1, _ECAP_FUNC_MODE_DDM);
	hal_ecap_open(ECAP1);
	hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_DDM);
	hal_ecap_open(ECAP2);
	hal_ecap_reg_int_cb(fml_ask_int_handler);

	for (int i=0; i<sizeof(ask_dm)/sizeof(ask_dm[0]); i++)
	{
		ask_dm[i].pkt_time_stamp = sys_ticks;
		ask_dm[i].dm_bad_cnt = 0;

		for (uint8_t k = 0; k < MAX_SUB_DCODE; k++)
		{
			if(0 == k)
			{
				ask_dm[i].decode[k].bit_zero_max = FULL_BIT_0_MAX_THD1;
				ask_dm[i].decode[k].bit_zero_min = FULL_BIT_0_MIN_THD1;
				ask_dm[i].decode[k].bit_one_max = HALF_BIT_1_MAX_THD1;
				ask_dm[i].decode[k].bit_one_min = HALF_BIT_1_MIN_THD1;
				ask_dm[i].decode[k].start_bit_min = STAR_BIT_0_MIN_THD1;
				ask_dm[i].decode[k].preamble_bit_min = HALF_BIT_1_MIN_THD1;
			}
			else
			{
				ask_dm[i].decode[k].bit_zero_max = FULL_BIT_0_MAX_THD2;
				ask_dm[i].decode[k].bit_zero_min = FULL_BIT_0_MIN_THD2;
				ask_dm[i].decode[k].bit_one_max = HALF_BIT_1_MAX_THD2;
				ask_dm[i].decode[k].bit_one_min = HALF_BIT_1_MIN_THD2;
				ask_dm[i].decode[k].start_bit_min = STAR_BIT_0_MIN_THD2;
				ask_dm[i].decode[k].preamble_bit_min = HALF_BIT_1_MIN_THD2;
			}
		}
	}

	ask_dm[0].ddm_param_chose = fml_ask_dmo1_xfer_cfg;
	ask_dm[1].ddm_param_chose = fml_ask_dmo2_xfer_cfg;
	ask_dm[2].ddm_param_chose = NULL;

	ap->ddm_check_interval_long = 0;
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
		if ((uint16_t)(sys_ticks - ask_dm[i].pkt_time_stamp) > (ap->ddm_check_interval_long ? 650 : 350))//350 //650
		{
			ask_dm[i].pkt_time_stamp = sys_ticks;
			if (ask_dm[i].ddm_param_chose != NULL)
			{
				ask_dm[i].ddm_param_chose();
			}
			ask_dm[i].dm_bad_cnt++;
			if (ask_dm[i].dm_bad_cnt > 20)
			{
				ask_dm[i].dm_bad_cnt = 20;
			}
		}
	}
	
	if ((gd->pid_perd == _360K_EPWM_PERD && ask_dm[0].dm_bad_cnt >= 3 && ask_dm[1].dm_bad_cnt >= 3 && ask_dm[2].dm_bad_cnt >= 3) || 
		(gd->pid_perd != _360K_EPWM_PERD && ask_dm[0].dm_bad_cnt >= 2 && ask_dm[1].dm_bad_cnt >= 2 && ask_dm[2].dm_bad_cnt >= 2))
	{
		wpc_printk("\r\n DM_CR %d %d %d", ask_dm[0].dm_bad_cnt, ask_dm[1].dm_bad_cnt, ask_dm[2].dm_bad_cnt);
		ask_dm[0].dm_bad_cnt = 0;
		ask_dm[1].dm_bad_cnt = 0;
		ask_dm[2].dm_bad_cnt = 0;
		ap->ddm_check_interval_long = 0;
		osal_set_event(WPC_TASK, WPC_EVT_DM_CRITICAL);
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

	wpc_printk("[dmo%d%s]", gd->wpc_pkt.src, s);
}
/**
 * demodulation configuration during Ping and power transfer
 *
 */
typedef enum
{
    dmo1_src_iavg = 0,
    dmo1_src_vdm = 1,
} dmo1_src_t;

typedef enum
{
    dmo2_src_vcap = 0,
    dmo2_src_phase = 1,
	dmo2_src_digital = 2,
} dmo2_src_t;

typedef enum
{
    dmo2_vcap_k1 = 1,
    dmo2_vcap_k2 = 2,
	dmo2_vcap_k3 = 3,
} dmo2_vcap_k_t;

typedef enum
{
	auto_gain 	= 0,
	fix_normal	= 1,
	fix_high 	= 2,
} dmox_gain_t;

typedef enum
{
	no_set = 0,
	bpf_1ord = 1,
	bpf_2ord = 2,
} dmox_bpf_t;

struct ddm_dmo1_cfg_t
{
	dmo1_src_t src;
	dmox_gain_t gain;
	dmox_bpf_t bpf;
};

struct ddm_dmo2_cfg_t
{
	dmo2_src_t src;
	dmox_gain_t gain;
	dmox_bpf_t bpf;
	dmo2_vcap_k_t vcap_k;
};

static struct ddm_dmo1_cfg_t curr_dmo1_cfg;
static struct ddm_dmo2_cfg_t curr_dmo2_cfg;

static uint8_t ddm_128_ping_cfg_keep;

#define DDM_PING_CFG_SIZE		(2)
#define DDM_XFER_CFG_SIZE		(4)

const struct ddm_dmo1_cfg_t dmo1_cfg_128_ping[DDM_PING_CFG_SIZE] = 
{
	{.src = dmo1_src_iavg, .gain = fix_high,   .bpf = bpf_1ord},//ping 1
	{.src = dmo1_src_vdm,  .gain = fix_high,   .bpf = bpf_1ord},//ping 2
};

const struct ddm_dmo2_cfg_t dmo2_cfg_128_ping[DDM_PING_CFG_SIZE] = 
{
	{.src = dmo2_src_vcap, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k1},//ping 1
	{.src = dmo2_src_vcap, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k1},//ping 2
};

const struct ddm_dmo1_cfg_t dmo1_cfg_360_ping[DDM_PING_CFG_SIZE] = 
{
	{.src = dmo1_src_iavg, .gain = auto_gain, .bpf = bpf_1ord},//68nF LOW_k
	{.src = dmo1_src_iavg, .gain = auto_gain, .bpf = bpf_1ord},//101nF
};

const struct ddm_dmo2_cfg_t dmo2_cfg_360_ping[DDM_PING_CFG_SIZE] = 
{
	{.src = dmo2_src_digital, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k1},//68nF LOW_k
	{.src = dmo2_src_digital, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k1},//101nF
};

const struct ddm_dmo1_cfg_t dmo1_cfg_normal_xfer[DDM_XFER_CFG_SIZE] = 
{
	/* BPP and MPP */
	{.src = dmo1_src_vdm,  .gain = fix_high,    .bpf = bpf_1ord},
	{.src = dmo1_src_iavg, .gain = fix_normal,  .bpf = bpf_1ord},
	{.src = dmo1_src_vdm,  .gain = fix_normal,  .bpf = bpf_1ord},
	{.src = dmo1_src_iavg, .gain = auto_gain,   .bpf = bpf_1ord},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_normal_xfer_low[DDM_XFER_CFG_SIZE] = 
{
	/* BPP load <= 2500 */
	{.src = dmo2_src_vcap,    .gain = fix_normal,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_phase,   .gain = fix_normal,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_vcap,    .gain = fix_high,    .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_phase,   .gain = fix_high,    .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_normal_xfer_mid[DDM_XFER_CFG_SIZE] = 
{
	/* BPP load <= 6000 */
	{.src = dmo2_src_phase,   .gain = fix_normal,   .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_vcap,    .gain = fix_normal,   .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_phase,   .gain = fix_high,     .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_vcap,    .gain = fix_high,     .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_normal_xfer_high[DDM_XFER_CFG_SIZE] = 
{
	/* BPP load > 6000 */
	{.src = dmo2_src_vcap,    .gain = fix_normal,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_vcap,    .gain = auto_gain,   .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_vcap,    .gain = fix_high,    .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_phase,   .gain = fix_high,    .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_360_cpm_xfer_low[DDM_XFER_CFG_SIZE] = 
{
	/* load <= 2500 */
	{.src = dmo2_src_vcap,    .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_digital, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_phase,   .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k1},
	{.src = dmo2_src_digital, .gain = fix_high, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_360_cpm_xfer_mid[DDM_XFER_CFG_SIZE] = 
{
	/* load <= 6000 */
	{.src = dmo2_src_phase,   .gain = fix_high,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_digital, .gain = fix_high,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_vcap,    .gain = auto_gain, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_digital, .gain = fix_high,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
};

const struct ddm_dmo2_cfg_t dmo2_cfg_360_cpm_xfer_high[DDM_XFER_CFG_SIZE] = 
{
	/* load > 6000 */
	{.src = dmo2_src_vcap,    .gain = fix_normal, .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_digital, .gain = fix_high,   .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k2},
	{.src = dmo2_src_vcap,    .gain = auto_gain,  .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
	{.src = dmo2_src_digital, .gain = fix_high,   .bpf = bpf_1ord, .vcap_k = dmo2_vcap_k3},
};

void fml_ask_dmo1_cfg(const struct ddm_dmo1_cfg_t *dmo1_cfg)
{
	curr_dmo1_cfg.src = dmo1_cfg->src;
	curr_dmo1_cfg.gain = dmo1_cfg->gain;
	curr_dmo1_cfg.bpf = dmo1_cfg->bpf;

	if (gd->nu103x_sts_curr.BITS.DMO1_OUT_MODE != _NU1030_DMO1_OUT_MODE_DDM)
	{
		fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
	}

	switch (dmo1_cfg->src)
	{
	case dmo1_src_iavg:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_SRC_IAVG);
		break;
	case dmo1_src_vdm:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_SRC_EVDM);
	default:
		break;
	}

	switch (dmo1_cfg->gain)
	{
	case auto_gain:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_GAIN_MODE_AUTO);
		break;
	case fix_normal:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_GAIN_MODE_FIXD);
		fml_nu103x_config(_1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
		break;
	case fix_high:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_GAIN_MODE_FIXD);
		fml_nu103x_config(_1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
		break;
	default:
		break;
	}

	switch (dmo1_cfg->bpf)
	{
	case bpf_1ord:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_BPF_1ORD);
		break;
	case bpf_2ord:
		fml_nu103x_config(_1030_CFG_DMO1_DDM_BPF_2ORD);
		break;
	case no_set:
	default:
		break;
	}
}

void fml_ask_dmo2_cfg(const struct ddm_dmo2_cfg_t *dmo2_cfg)
{
	curr_dmo2_cfg.src = dmo2_cfg->src;
	curr_dmo2_cfg.gain = dmo2_cfg->gain;
	curr_dmo2_cfg.bpf = dmo2_cfg->bpf;
	curr_dmo2_cfg.vcap_k = dmo2_cfg->vcap_k;

	switch (dmo2_cfg->src)
	{
	case dmo2_src_vcap:
		fml_ask_dig_ddm_enable(0);
		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);	
		fml_nu103x_config(_1030_CFG_DMO2_DDM_SRC_VCAP);
		break;
	case dmo2_src_phase:
		fml_ask_dig_ddm_enable(0);
		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);	
		fml_nu103x_config(_1030_CFG_DMO2_DDM_SRC_PHAS);
		break;
	case dmo2_src_digital:
		fml_ask_dig_ddm_enable(1);
		break;
	default:
		break;
	}

	switch (dmo2_cfg->gain)
	{
	case auto_gain:
		fml_nu103x_config(_1030_CFG_DMO2_DDM_GAIN_MODE_AUTO);
		break;
	case fix_normal:
		fml_nu103x_config(_1030_CFG_DMO2_DDM_GAIN_MODE_FIXD);
		fml_nu103x_config(_1030_CFG_DMO2_DDM_FIXED_GAIN_X36);
		break;
	case fix_high:
		fml_nu103x_config(_1030_CFG_DMO2_DDM_GAIN_MODE_FIXD);
		fml_nu103x_config(_1030_CFG_DMO2_DDM_FIXED_GAIN_X60);
		break;
	default:
		break;
	}

	switch (dmo2_cfg->bpf)
	{
	case bpf_1ord:
		fml_nu103x_config(_1030_CFG_DMO2_DDM_BPF_1ORD);
		break;
	case bpf_2ord:
		fml_nu103x_config(_1030_CFG_DMO2_DDM_BPF_2ORD);
		break;
	case 0:
	default:
		break;
	}

	switch (dmo2_cfg->vcap_k)
	{
	case dmo2_vcap_k1:
		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K1);
		break;
	case dmo2_vcap_k2:
		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K2);
		break;
	case dmo2_vcap_k3:
		fml_nu103x_config(_1030_CFG_DMO2_VCAP_RATIO_K3);
		break;
	default:
		break;
	}
}

void fml_ask_128_ping_cfg(void)
{
	static uint8_t cfg_cnt = 0;
	const struct ddm_dmo1_cfg_t *dmo1_cfg;
	const struct ddm_dmo2_cfg_t *dmo2_cfg;

	fml_ask_dig_ddm_enable(0);
	fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
	fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
	//cfg_cnt = (ddm_128_ping_cfg_keep) ? cfg_cnt : (++cfg_cnt)&0x01;
    if(!ddm_128_ping_cfg_keep)
    {
    	cfg_cnt++;
    	cfg_cnt &= 0x01;
    }
	ddm_128_ping_cfg_keep = 0;

	switch (cfg_cnt)
	{
	case 0:
		dmo1_cfg = &dmo1_cfg_128_ping[0];
		dmo2_cfg = &dmo2_cfg_128_ping[0];
		break;
	case 1:
	default:
		dmo1_cfg = &dmo1_cfg_128_ping[1];
		dmo2_cfg = &dmo2_cfg_128_ping[1];
		break;
	}

	fml_nu103x_config(_1030_CFG_DMOx_DDM_CMP_HYST_30P0mV);

	fml_ask_dmo1_cfg(dmo1_cfg);
	fml_ask_dmo2_cfg(dmo2_cfg);

	/*
		DMO1_DDM_SRC   0->iavg     1->vdm
		DDM_GAIN_MODE  0->auto     1->fix_normal  2->fix_high
		DDM_CMP_HYST   0->12.5mV   1->30mV
	*/
	wpc_printk("\r\n <1-%d%d>", curr_dmo1_cfg.src, curr_dmo1_cfg.gain);

	/*
		DMO2_DDM_SRC   0->vcap     1->phase    		2->digital
		DDM_GAIN_MODE  0->auto     1->fix_normal	2->fix_high
		VCAP_RATIO_K
	*/
	wpc_printk(" <2-%d%d-k%d>", curr_dmo2_cfg.src, curr_dmo2_cfg.gain, curr_dmo2_cfg.vcap_k);
}

void fml_ask_128_ping_cfg_keep(void)
{
	ddm_128_ping_cfg_keep = 1;
}

void fml_ask_360_ping_cfg(void)
{
	const struct ddm_dmo1_cfg_t *dmo1_cfg;
	const struct ddm_dmo2_cfg_t *dmo2_cfg;

	if (gd->k_est <= ap->low_k_val)
	{
		dmo1_cfg = &dmo1_cfg_360_ping[0];
		dmo2_cfg = &dmo2_cfg_360_ping[0];
	}
	else
	{
		dmo1_cfg = &dmo1_cfg_360_ping[1];
		dmo2_cfg = &dmo2_cfg_360_ping[1];
	}

	fml_nu103x_config(_1030_CFG_DMOx_DDM_CMP_HYST_12P5mV);
	fml_ask_dmo1_cfg(dmo1_cfg);
	fml_ask_dmo2_cfg(dmo2_cfg);
	/*
		DMO1_DDM_SRC   0->iavg     1->vdm
		DDM_GAIN_MODE  0->auto     1->fix_normal  2->fix_high
		DDM_CMP_HYST   0->12.5mV   1->30mV
	*/
	wpc_printk("\r\n <1-%d%d>", curr_dmo1_cfg.src, curr_dmo1_cfg.gain);
	/*
		DMO2_DDM_SRC   0->vcap     1->phase    		2->digital
		DDM_GAIN_MODE  0->auto     1->fix_normal	2->fix_high
		VCAP_RATIO_K
	*/
	wpc_printk(" <2-%d%d-k%d>", curr_dmo2_cfg.src, curr_dmo2_cfg.gain, curr_dmo2_cfg.vcap_k);
}

void fml_ask_dmo1_xfer_cfg(void)
{
	if (gd->ptx_protocol_phase != WPC_PHASE_XFER || gd->atl_test_tpr1c_coil_flag == 1)
	 	return;

	static uint8_t cfg_cnt = 0;
	const struct ddm_dmo1_cfg_t *dmo1_cfg;

	if (++cfg_cnt >= DDM_XFER_CFG_SIZE)
	{
		cfg_cnt = 0;
	}

	dmo1_cfg = &dmo1_cfg_normal_xfer[cfg_cnt];
	fml_ask_dmo1_cfg(dmo1_cfg);
	/*
		DMO1_DDM_SRC   0->iavg     1->vdm
		DDM_GAIN_MODE  0->auto     1->fix_normal  2->fix_high
		DDM_CMP_HYST   0->12.5mV   1->30mV
	*/
	wpc_printk(" <1-%d%d>", curr_dmo1_cfg.src, curr_dmo1_cfg.gain);
}

void fml_ask_dmo2_xfer_cfg(void)
{
	if (gd->ptx_protocol_phase != WPC_PHASE_XFER || gd->atl_test_tpr1c_coil_flag == 1)
	 	return;

	static uint8_t cfg_cnt = 0;
	const struct ddm_dmo2_cfg_t *dmo2_cfg;

	if (++cfg_cnt >= DDM_XFER_CFG_SIZE)
	{
		cfg_cnt = 0;
	}

	if (gd->pid_perd == _360K_EPWM_PERD)
	{
		if (gd->tx_power <= 2500)
		{
			dmo2_cfg = &dmo2_cfg_360_cpm_xfer_low[cfg_cnt];
		}
		else if (gd->tx_power <= 6000)
		{
			dmo2_cfg = &dmo2_cfg_360_cpm_xfer_mid[cfg_cnt];
		}
		else
		{
			dmo2_cfg = &dmo2_cfg_360_cpm_xfer_high[cfg_cnt];
		}
	}
	else
	{
		if (gd->tx_power <= 2500)
		{
			dmo2_cfg = &dmo2_cfg_normal_xfer_low[cfg_cnt];
		}
		else if (gd->tx_power <= 6000)
		{
			dmo2_cfg = &dmo2_cfg_normal_xfer_mid[cfg_cnt];
		}
		else
		{
			dmo2_cfg = &dmo2_cfg_normal_xfer_high[cfg_cnt];
		}
	}

	fml_ask_dmo2_cfg(dmo2_cfg);

	/*
		DMO2_DDM_SRC   0->vcap     1->phase    		2->digital
		DDM_GAIN_MODE  0->auto     1->fix_normal	2->fix_high
		VCAP_RATIO_K
	*/
	wpc_printk(" <2-%d%d-k%d>", curr_dmo2_cfg.src, curr_dmo2_cfg.gain, curr_dmo2_cfg.vcap_k);
}

void fml_ask_dig_ddm_enable(uint8_t enable)
{
	static uint8_t last_dig_ddm_enable = 0;

	if (last_dig_ddm_enable == enable)
	{
		return;
	}

	last_dig_ddm_enable = enable;
	gd->sys_status.is_dig_ddm_en = enable;

	if (enable)
	{
		hal_ddm_dig_ping();
		hal_ecap_dig_ddm_init();
		hal_eadc_ddm_init();
		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_CAP);
		// printk("\r\n ----- enable digital ddm");
	}
	else
	{
		hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_DDM);
		hal_ecap_open(ECAP2);
		hal_ecap_close(ECAP4);
		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
		// printk("\r\n ----- disable digital ddm");
	}
}
