#include "mpp.h"
#include "printk.h"
#include "g_data.h"
#include "_wpc.h"

void mpp_power_limit_init(void)
{
//	gd->tx_infos.max_cap = 250;
//
//	gd->tx_infos.tar_cap_fod = 250;
//    gd->tx_infos.tar_cap_cali = 150;
//
//    gd->tx_infos.nego_cap = 150;//TODO:
//
//    gd->tx_infos.power_limit_reason = 0;//TODO:
//	gd->tx_infos.need_renego_cap = 0;

	gd->tx_infos.max_cap = (gd->adp.pwr_high / 2) * 10;

	gd->tx_infos.tar_cap_fod = (gd->adp.pwr_high / 2) * 10;
    gd->tx_infos.tar_cap_cali = (gd->adp.pwr_high / 2) * 10;

    gd->tx_infos.nego_cap = (gd->adp.pwr_high / 2) * 10;

    gd->tx_infos.power_limit_reason = 0;//TODO:
	gd->tx_infos.need_renego_cap = 0;
}

