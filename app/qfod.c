#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "config.h"
#include "delay.h"
#include "debug.h"
#include "_wpc.h"
#include "app.h"
#include "pfod.h"
#include "qfod.h"

static uint8_t cali_max_cnt;
static uint8_t cali_ready;
static uint8_t tool_remove_cnt;

static uint16_t q_sum, f_sum;


void qfod_qdt_cali_init(void)
{
	cali_max_cnt = 3;
	cali_ready = 0;
	tool_remove_cnt = 0;
}

void qfod_qdt_cali_process(void)
{
	if (cali_ready == 0)
	{
		if (gd->tx_infos.q_fact + ap->q_factor_reco_value +20 > ap->q_factor_base_value && gd->tx_infos.q_fact < ap->q_factor_limH_value &&
		    gd->tx_infos.f_self + ap->fs_reco_value > ap->fs_base_value && gd->tx_infos.f_self < ap->fs_limH_value)
		{
			if (++tool_remove_cnt >= 10)
			{
				tool_remove_cnt = 0;
				q_sum = f_sum = 0;
				cali_ready = 1;
				printk(" cali ready");
			}
		}
		else
		{
			tool_remove_cnt = 0;
		}
	}
	else
	{
		q_sum += gd->tx_infos.q_fact;
		f_sum += gd->tx_infos.f_self;
		if (++tool_remove_cnt >= 8)
		{
			q_sum >>= 3;
			f_sum >>= 3;
			gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
			printk(" cali success: %d %d", q_sum, f_sum);

			uint32_t u32Tmp;
			hal_fmc_erase_page(AP_CFG_ROM_ADDR_BASE);
			u32Tmp = switch_big_little_endian(q_sum);
			hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE, u32Tmp);
			u32Tmp = switch_big_little_endian(f_sum);
			hal_fmc_write_word((AP_CFG_ROM_ADDR_BASE + 4), u32Tmp);
		}
	}
}

uint8_t qfod_nego(uint8_t ref_q, uint8_t ref_f)
{
	uint8_t result = 0;

	///MP.TPR#MP3
	if (ref_q >= 0x7C && ref_q <= 0x8F && ref_f >= 0x79 && ref_f <= 0x85)
	{
		gd->rx_infos.rx_type = EPRX_TYPE_NOK9_EPP_FOD_TPR_MP3;
		if (gd->tx_infos.q_fact + 110 < ap->q_factor_base_value && gd->tx_infos.f_self + 50 > ap->fs_base_value)
		{
			result = 1;
		}
		printk("\r\n TPR.MP3");
	}
	///MP.TPR#7
//	else if (ref_q == 0x81)
//	{
//		if (gd->tx_infos.q_fact + 80 < ap->q_factor_base_value)
//		{
//			result = 1;
//		}
//	}
	///MP.TPR#MP4
	else if (ref_q >= 0x25 && ref_q <= 0x28 && ref_f >= 0x73 && ref_f <= 0x75)
	{
		gd->rx_infos.rx_type = EPRX_TYPE_NOK9_EPP_FOD_TPR_MP4;
		if (gd->tx_infos.q_fact + 110 < ap->q_factor_base_value && gd->tx_infos.f_self + 50 > ap->fs_base_value)
		{
			result = 1;
		}
		printk("\r\n TPR.MP4");
	}
	///MP.TPR#MP1B
	else if (ref_q >= 0x85 && ref_q <= 0x8C && ref_f >= 0x6B && ref_f <= 0x6F)
	{
		gd->rx_infos.rx_type = EPRX_TYPE_NOK9_EPP_FOD_TPR_MP1B;
		if (gd->tx_infos.q_fact + 110 < ap->q_factor_base_value && gd->tx_infos.f_self > ap->fs_base_value + 50)
		{
			result = 1;
		}
		printk("\r\n TPR.MP1B");
	}
	///MP.TPR#1F
	else if (ref_q >= 0x7C && ref_q <= 0x83 && ref_f >= 0x70 && ref_f <= 0x74)
	{
		gd->rx_infos.rx_type = EPRX_TYPE_NOK9_EPP_FOD_TPR_1F;
		if (gd->tx_infos.q_fact + 100 < ap->q_factor_base_value && gd->tx_infos.f_self > ap->fs_base_value + 10)
		{
			result = 1;
		}
		printk("\r\n TPR.#1F");
	}

	return result;
}
