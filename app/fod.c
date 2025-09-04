/*
 * lib.c
 *
 *  Created on: Jan 21, 2025
 *      Author: NVT10114
 */

#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "debug.h"
#include "algo.h"
#include "app.h"
#include "bsp.h"
#include "pid.h"
#include "pfod.h"
#include "adp.h"
#include "_wpc.h"
#include "wpc_xfer.h"
#include "g_data.h"
#include "gui.h"
#define YBZ_MPP_THD         490
#define YBZ_MPP_RECO         470														
static const uint16_t u16_kp_tbl[][3] =
{
	//Unknown PRx, default coeff 0
	{750,	750,	750},//4W,8W,12W

	//SAMSUNG 1
	{500,	500,	750},

	//APPLE_STD 2
	{500,	900,	900},

	//APPLE_MAG 3
	{500,	 500,	 500},
};

static uint32_t ploss;


uint8_t pfod_action(void)
{
	uint8_t res = 0;
	int32_t pfo_sum;

	if (mpla_count++ >= 5)
	{
		mpla_count = 5;
	}

	/* calculate pfo_avg*/
	pfo_values[pfo_values_index++] = pfo;
	if (pfo_values_index >= 5)
	{
		pfo_values_index = 0;
	}

	pfo_sum = 0;
	for (uint8_t i=0; i<5; i++)
	{
		pfo_sum += pfo_values[i];
	}
	pfo_avg = pfo_sum / 5;
    
    if (fod_enable && (pfo > pfo_thd || (pfo >= pfo_thd_reco && fod_count > 0)))
    {
		/* pfo over threshold */
		fod_count_filter++;

		if (fod_count_filter > 3)
		{
			fod_count_filter = 3;
			fod_count++;

			if (((fod_count >= FOD_MAX_CNT && gd->rx_prect < 5000) || fod_count >= 40 || !pfo_en_reco))
			{
				gd->prot_sts.xfer_fod_flag = 1;
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_XER_FOD;
				wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_POWER_LOSS_FOD);
			}
			else
			{
				gd->power_limit_sts.fop_flag = 1;
				res = 1;//will throttle after rpp and do not ack cep
			}
		}

    	if (pfo > (pfo_thd + 150))//IOC 10.4.05
    	{
			gd->power_limit_sts.fop_flag = 1;
			res = 1;//will throttle after rpp and do not ack cep
//			printk("\r\n IOC#10.4.05");
		}
    }

    else
    {
		/* pfo below threshold */
        if (fod_count_filter)
        {
			fod_count_filter--;
        }

		if (gd->rx_prect > p_rect_max)
		{
            p_rect_max = gd->rx_prect;
		}

		if (mpla_count >= 5)
		{
			printk("\r\n 1");
            if ((fod_count == 0) && (pfo_avg < pfo_thd_reco))
            {
            	printk("\r\n 2");
            	//p_target is safe
                if (p_rect_max + 200 >= gd->tx_infos.nego_cap)
                {
                	printk("\r\n 3");
					if (gd->tx_infos.tar_cap_fod == gd->tx_infos.nego_cap && gd->tx_infos.tar_cap_fod < gd->tx_infos.max_cap)
					{
						printk("\r\n 4");
						/* prect is close to p_target and power limit reason is fod, should increase p_target */
						if (gd->tx_infos.tar_cap_fod + 2 < gd->tx_infos.max_cap)
						{
							printk("\r\n 5");
							gd->tx_infos.tar_cap_fod += 2;
							gd->tx_infos.power_limit_reason = 2;
						}
						else
						{
							printk("\r\n 6");
							gd->tx_infos.tar_cap_fod = gd->tx_infos.max_cap;
							gd->tx_infos.power_limit_reason = 0;
							if (gd->tx_infos.fo_exist)
							{
								printk("\r\n 7");
								gd->tx_infos.fo_exist = 0;
							}
						}

						if (pfo_en_reco)
						{
						//	printk("\r\n 8");
							gd->tx_infos.nego_cap = gd->tx_infos.tar_cap_fod;
							gd->tx_infos.need_renego_cap = 1;
							res = 2;//go to send ATN and increase the nego cap
						}
					}
                }
            }
            else if ((fod_count >= 3) || ((fod_count >= 1) && (pfo_avg > pfo_thd)))
            {
            	//p_target is not safe
				gd->tx_infos.tar_cap_fod = (gd->tx_infos.tar_cap_fod > 32) ? (gd->tx_infos.tar_cap_fod - 2) : 30;

				printk("\r\n A: %d %d", gd->tx_infos.tar_cap_fod, p_rect_max);

				if (gd->tx_infos.tar_cap_fod > p_rect_max / 100)
				{
					printk("\r\n B");
					gd->tx_infos.tar_cap_fod = p_rect_max / 100;
				}

				if (gd->tx_infos.nego_cap > gd->tx_infos.tar_cap_fod)
				{
					printk("\r\n C");
					gd->tx_infos.nego_cap = gd->tx_infos.tar_cap_fod;
					gd->tx_infos.need_renego_cap = 1;
					gd->tx_infos.power_limit_reason = 2;
					res = 3;//go to send ATN and decrease the nego cap
				}
            }

			// clear counts
			fod_count_filter = 0;
			fod_count = 0;
			mpla_count = 0;
			gd->power_limit_sts.fop_flag = 0;
        }
    }

    if (gd->pid_volt >= gd->pid_limit.volt_lim_hi && gd->pid_duty >= gd->pid_limit.duty_lim_hi && gd->pid_phas <= gd->pid_limit.phas_lim_lo)
    {
    	if (gd->pid_perd == 400 && gd->rx_infos.cep_val > 0)
    	{
        	//p_target is not safe
			gd->tx_infos.tar_cap_fod = (gd->tx_infos.tar_cap_fod > 32) ? (gd->tx_infos.tar_cap_fod - 2) : 30;

			printk("\r\n A1");

			if (gd->tx_infos.tar_cap_fod > p_rect_max / 100)
			{
				printk("\r\n B1");
				gd->tx_infos.tar_cap_fod = p_rect_max / 100;
			}

			if (gd->tx_infos.nego_cap > gd->tx_infos.tar_cap_fod)
			{
				printk("\r\n C1");
				gd->tx_infos.nego_cap = gd->tx_infos.tar_cap_fod;
				gd->tx_infos.need_renego_cap = 1;
				gd->tx_infos.power_limit_reason = 2;
				res = 3;//go to send ATN and decrease the nego cap
			}
    	}
    }
#if OPTION_FOD_ENABLE
	return res;
#else
	return 0;
#endif
}
uint8_t pfod_mpla(void)
{
	// uint8_t res = 0;
	// int32_t pfo_sum;
    uint32_t temp_power;
    
	ploss = ploss_calc(gd->rx_infos.pla_type);

	if (gd->rx_power < 4000 && ploss > 1600)
	{
		ploss = 1600;
	}
	else if (gd->rx_power < 8000 && ploss > 2000)
	{
		ploss = 2000;
	}
	else if (gd->rx_power < 12000 && ploss > 2800)
	{
		ploss = 2800;
	}
	else if (ploss > 4000)
	{
		ploss = 4000;
	}

    if (gd->rx_infos.rx_type == ERX_TYPE_YBZ_MPP_FIXTURE)
    {
        if (gd->rx_power > 10000)
        {
            temp_power = gd->rx_power * 815 / 1000;
        }
        else
        {
        	temp_power = gd->rx_power*800/1000;
        }
        pfo = gd->tx_power - ploss - temp_power;
    }
    else
    {
	    pfo = gd->tx_power - ploss - gd->rx_power - 300;
    }

	pfo_en_reco = 1; //pfo_en_reco = (gd->extend.k > 6200)? 1 : 0;
	if (gd->rx_infos.rx_type == ERX_TYPE_APPLE_MPP)
	{
		pfo_thd = PFO_THD_APL_MPP;
		pfo_thd_reco = PFO_RECO_APL_MPP;
	}
	else
	{
		if (gd->tx_infos.fo_exist)
		{// || gd->rx_infos.gcoil_tx < 1
			pfo_thd = PFO_10W_THD;
			pfo_thd_reco = PFO_10W_RECO;
		//	printk("\r\n pfo: %d %d", pfo_thd, pfo_thd_reco);
		}
		else
		{
			pfo_thd = PFO_15W_THD;
			pfo_thd_reco = PFO_15W_RECO;
		}

		//Fixtures' FOD adjust
		if (gd->rx_infos.rx_type == ERX_TYPE_YBZ_MPP_FIXTURE)
		{
			pfo_thd = YBZ_MPP_THD;
			pfo_thd_reco = YBZ_MPP_RECO;
			pfo_en_reco = 0;
		}
		else if (gd->rx_infos.rx_type == ERX_TYPE_NVT_MPP)
		{
			pfo_en_reco = 0;
			if (gd->rx_power > 8000)
			{
				pfo += 650;
			}
			else if (gd->rx_power > 4000)
			{
				pfo += 450;
			}
			else
			{
				pfo += 450;
			}
		}
		else
		{

			if (gd->rx_power < 1000)
			{
				pfo += 650;
			}
			else if  (gd->rx_power < 3000)
			{
				pfo += 490;
			}
			else if (gd->rx_power < 4000)
			{
				pfo += 450;
			}
			else if(gd->rx_power < 6000){
				pfo += 300;
			}
			else if (gd->rx_power < 9000)
			{
				pfo += 450;
			}
			else if (gd->rx_power < 13000)
			{
				pfo += 400;
			}
			else
			{
				pfo += 250;//200;
			}
			printk ("\r\n pfo value %d %d", gd->rx_power,pfo);
		}
	}

	return pfod_action();
}

uint8_t pfod_common(void)
{
	uint8_t res;
	uint8_t u8Idx0, u8Idx1;
	uint16_t u16LargeLossTHD;
	u8Idx0 = (uint8_t)gd->rx_infos.rx_type;
	if (u8Idx0 >= sizeof(u16_kp_tbl) / sizeof(u16_kp_tbl[0])) u8Idx0 = 0;
	u8Idx1 = gd->rx_power / 4000;
	if (u8Idx1 > 2) u8Idx1 = 2;

	u16LargeLossTHD = (u8Idx1 + 2) * 1000 + 1000;//3W,4W,5W
	if (u16LargeLossTHD > 5000) u16LargeLossTHD = 5000;

	if (gd->rx_infos.mpp_restricted_mode && gd->rx_power > 4900)//IOC
	{
	    pfo_thd = 500;
	}
	else
	{
	    pfo_thd = (gd->rx_infos.qi_version >= 0x20) ? 0 : u16_kp_tbl[u8Idx0][u8Idx1];
	}

	ploss = ploss_calc(0);

	if (gd->rx_infos.rx_type == EPRX_TYPE_APPLE_STD && gd->rx_infos.qi_version < 0x20)
	{
		//Loose FOD for IPX/IP8
		ploss <<= 1;
	}
	
	if (gd->rx_infos.rx_type == EPRX_TYPE_APPLE_MAG && ploss > 1000)
	{
		ploss = 1000;
	}
	else if (ploss > 3500)
	{
		ploss = 3500;
	}

	if ((gd->rx_infos.rx_type == ERX_TYPE_YBZ_BPP_FIXTURE) || ((gd->rx_infos.rx_type == ERX_TYPE_YBZ_EPP_FIXTURE)))
	{
	    pfo_thd = 0;
        pfo = gd->tx_power - ploss - (gd->rx_power * 810 / 1000);
	}
	else if (gd->rx_infos.rx_type == ERX_TYPE_YBZ_PPDE_FIXTURE)
	{
        pfo = gd->tx_power - ploss - gd->rx_power - 2500;
	}
	else if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_BPP_FOD_TPR_5)
	{
		pfo = gd->tx_power - ploss - gd->rx_power;
	}else if (gd->rx_infos.rx_type == EPRX_TYPE_SAMSUNG)
	{
		pfo = gd->tx_power - ploss - gd->rx_power + 1000;
		printk("\r\n SAMSUNG");
	}
	else
	{
	    pfo = gd->tx_power - ploss - gd->rx_power;

	    if (gd->rx_infos.power_profile_mode == EPP)
	    {
	    	if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_EPP_FOD_TPR_7)
			{
	    		pfo += 200;
			}
	    	else if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_EPP_FOD_TPR_MP3)
	    	{
	    		pfo += 200;
	    	}
	    	else if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_EPP_FOD_TPR_MP4)
	    	{
	    		pfo += 200;
	    	}
	    	else if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_EPP_FOD_TPR_MP1B)
	    	{
	    		pfo += 200;
	    	}
	    	else if (gd->rx_infos.rx_type == EPRX_TYPE_NOK9_EPP_FOD_TPR_1F)
	    	{
	    		pfo += 200;
	    	}
	    	else
	    	{
	    		pfo -= 1200;
	    	}
	    }
	    else
	    {
	    	pfo -= 1000;
	    }

	    if (gd->rx_power > 5000)
	    {
	    	pfo -= 300;
	    }
	    else if (gd->rx_power > 4000)
	    {
	    	pfo -= 200;
	    }
	    else if (gd->rx_power > 3000)
	    {
	    	pfo -= 30;
	    }
	}

	res = (pfo > pfo_thd) ? 1 : 0;

	if (res == 1)
	{
		fod_count++;
		if (gd->tx_power > gd->rx_power + u16LargeLossTHD)
		{
			if(gd->tx_power > 4000) fod_count++;
			if(gd->tx_power > 5000) fod_count++;
		}
		if (fod_count > 200) fod_count = 200;
		if (fod_count < ap->rpp_fod_cnt || ap->rpp_fod_dis) res = 0;
	}
	else
	{
		fod_count = 0;
	}

	pfod_log_print();

#if OPTION_FOD_ENABLE
	return res;
#else
	return 0;
#endif

}


void pfod_log_print(void)// print long log and avoid the fsk window
{
#ifdef _PRINT_FOD_MSG
	printk("\r\n FOD-> %d %d %d %d t:%d %d ctx:%d v:%d %d i:%d %d p:%d %d(%d) r:%d %d pfo:%d %d %d",
		   gd->rx_infos.rx_type, gd->tx_infos.nego_cap, 144000000 / gd->pid_perd, gd->pid_phas,
		   gd->sys_infos.die_temp, gd->sys_infos.ntc_temp_wpc, gd->ctx,
		   gd->vbus, gd->vpwr_avg, gd->isns_avg, gd->icol_rms,
		   gd->tx_power, gd->rx_power, gd->rx_prect,
		   gd->rx_infos.pla_vrect, gd->rx_infos.pla_irect,
		   pfo, pfo_thd, fod_count);
#endif



}
