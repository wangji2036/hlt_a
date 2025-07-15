#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "pfod.h"
#include "wpc_6_test_1_ioc.h"

static uint8_t bpp_first_rpp_value;
static uint8_t bpp_continous_count;

void ioc_bpp_fod_init(void)
{
	bpp_continous_count = 0;
	bpp_first_rpp_value = 0;
}

void ioc_bpp_fod_handler(uint16_t curr_rpp, uint16_t last_rpp, uint16_t rpp_count)
{
	if (rpp_count == 1)
	{
		bpp_first_rpp_value = curr_rpp;
	}
	else if (rpp_count < 50)
	{
		if (bpp_first_rpp_value < 0x10 && curr_rpp + 2 >= last_rpp && curr_rpp <= last_rpp + 8)
		{
			if (bpp_continous_count < 200)
			{
				if (++bpp_continous_count >= 3)
				{
					bpp_continous_count = 3;
					gd->rx_infos.rx_type = EPRX_TYPE_NOK9_BPP_FOD_TPR_5;
					printk(" [BPP_FOD_TPR#5]");
				}
			}
		}
		else
		{
			bpp_continous_count = 0xFF;
		}
	}
}
