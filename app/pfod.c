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

#define pcoil_factor		160//156->NU226  //360K-156, 128K-63, ACR
#define pcircuit_factor		 40//Rcircuit    //

#define PFO_10W_THD 		265
#define PFO_10W_RECO 		251//THD*0.95
#define PFO_15W_THD 		385
#define PFO_15W_RECO 		365//THD*0.95

#define PFO_THD_APL_MPP		385//750
#define PFO_RECO_APL_MPP 	365//500

#define FOD_MAX_CNT			30

static uint8_t fod_enable;
static uint8_t pfo_en_reco;//fixture and bpp set 0

static uint8_t fod_count_filter;
static uint8_t fod_count;
static uint8_t mpla_count;

static uint16_t pfo_thd;
static uint16_t pfo_thd_reco;

static uint16_t p_rect_max;

static uint8_t pfo_values_index;
static int32_t pfo_values[5];
static int32_t pfo;
static int32_t pfo_avg;
static uint32_t ploss;

void pfod_init(void)
{
	gd->tx_infos.pfod_event = EXFER_FOD_EVENT_NONE;
	// gd->tx_infos.max_cap = 250;
	// gd->tx_infos.nego_cap = 150;//TODO:
	// gd->tx_infos.tar_cap_fod = 250;
	gd->rx_infos.pla_type = 1; //TODO: depend on rx
	// gd->tx_infos.power_limit_reason = 0;//TODO:
	// gd->tx_infos.need_renego_cap = 0;
	
	gd->power_limit_sts.fop_flag = 0;
	gd->rx_prect = 0;

	fod_enable = MPP_25W_FOD_LOOSE_PFO;
	fod_count_filter = 0;
	fod_count = 0;
	mpla_count = 0;
	p_rect_max = 0;
	pfo_values_index = 0;
	pfo_avg = 0;
	osal_mem_clear(pfo_values, sizeof(pfo_values));
}

void pfod_mpla_init(void)//after XID/CFG/0x78
{

}

static uint32_t ploss_calc(uint8_t mode)
{
	register uint32_t tmp = 0;
	register uint32_t Pcoil, Pfm, Pmos, Psnuber;

	uint16_t freq_khz, vpwr, icoil;
	uint32_t temp_ploss, temp_alpha_fm, temp_alpha_fm_dc;

	freq_khz = 144000 / (EPWM1->PWM_PERD.BITS.PWM_PERD + 1);
	vpwr = gd->vpwr_avg;
	icoil = gd->icol_rms;

	/* icoil^2 */
	tmp = icoil * icoil;
	tmp = tmp / 1000;//tmp is icoil^2*1000, unit is mw^2/1000

	/* Pcoil */
	if (mode)
	{
		Pcoil = pcoil_factor * tmp / 1000;
	}
	else
	{
		Pcoil = 63 * tmp / 1000;
	}


	/*
	 * Pfm = g_fm_dc*alpha_fm_dc + g_fm*alpha_fm*icoil^2 
	 * g_fm_dc = alpha_fm_dc_tg/alpha_fm_dc_gg => apromax: alpha_fm_dc/alpha_fm_dc_gg
	 * g_fm = alpha_fm_tg/alpha_fm_gg => apromax: alpha_dc/alpha_fm_gg
	 * Pfm = alpha_fm_dc*alpha_fm_dc(unit 0.5*0.5)/alpha_fm_dc_gg + alpha_fm*alpha_fm(unit 0.5*0.5)/alpha_fm_gg*icoil^2
	 * Pfm = alpha_fm_dc*alpha_fm_dc/602 + alpha_fm*alpha_fm/235*icoil^2 (alpha_fm_dc_gg = 0.1505, alpha_fm_gg = 0.0588)
	 * Pfm = alpha_fm_dc*alpha_fm_dc/492 + alpha_fm*alpha_fm/267*icoil^2 (alpha_fm_dc_gg = 0.123, alpha_fm_gg = 0.0669)
	*/
	if (mode)
	{
	#if 1//GRL send zero data
		if (gd->rx_infos.alpha_fm == 0 && gd->rx_infos.alpha_fm_dc == 0)
		{
			gd->rx_infos.alpha_fm = 100;
			gd->rx_infos.alpha_fm_dc = 200;
		}
	#endif
		temp_alpha_fm = gd->rx_infos.alpha_fm * gd->rx_infos.alpha_fm / 267;
		temp_alpha_fm_dc = gd->rx_infos.alpha_fm_dc * gd->rx_infos.alpha_fm_dc / 492;
		Pfm = temp_alpha_fm*tmp / 1000;
		Pfm += temp_alpha_fm_dc;
	}
	else
	{
		Pfm = 0;
	}

	/* 
	 * Pmos = 2*R_ds_on*Icoil^2 + 4*Vin*Icoil*t*freq/6, R_ds_on = 0.04 Ohm, t=33ns
	 * Pmos = 80*Icoil^2(mW) + 4*33/6/1000000000*Vin*Icoil*freq, freq = 360Khz
	 * Pmos = 80*tmp/1000 + 22*360/1000/1000*vpwr*icoil/1000(mW)
	 * Pmos = (80*tmp + 792/100*vpwr*icoil/1000)/1000
	*/
	if (mode)
	{
		Pmos = 792 * vpwr / 100; //KirmsPra = 22; 22*360/1000=792/100
	}
	else
	{
		Pmos = 3 * vpwr / 100; //KirmsPra = 30;
		Pmos *= freq_khz;
	}
	
	Pmos *= icoil;
	Pmos /= 1000;
	Pmos += 80 * tmp;
	Pmos /= 1000;

	/* Psnuber = 1188*isns/1000*vpwr/1000000 */
	Psnuber = 1188 * vpwr;
	Psnuber /= 1000;
	Psnuber *= vpwr;
	Psnuber /= 1000000;

	/* ploss = Pcoil+Pfm+Pmos+Psnuber */
	temp_ploss = Pcoil + Pfm + Pmos + Psnuber;

	printk("\r\n ploss:%d coil:%d fm:%d mos:%d snb:%d", temp_ploss, Pcoil, Pfm, Pmos, Psnuber);

	return  temp_ploss;
}

static uint8_t pfod_action(void)
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
							printk("\r\n 8");
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

	return res;
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
			printk("\r\n pfo: %d %d", pfo_thd, pfo_thd_reco);
		}
		else
		{
			pfo_thd = PFO_15W_THD;
			pfo_thd_reco = PFO_15W_RECO;
		}

		//Fixtures' FOD adjust
		if (gd->rx_infos.rx_type == ERX_TYPE_YBZ_MPP_FIXTURE)
		{
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
			//1.12.1.4 nok9 +458mW offset: 896(877)->439(878), 3930(3733)->3489(3750), GRL 0mm +485mW offset: 915(566)->455(564), 3648(3382)->3577(3745)
//			if ((gd->rx_power < 3700) && (gd->rx_prect > gd->rx_power + 100))
//			{
//				pfo += 150;
//			}
//
//			if (gd->rx_power > 10000)
//			{
////				if (pfo > 150)
//				{
//					pfo -= 150;
//				}
//			}

			if (gd->rx_power < 3000)
			{
				pfo += 300;
			}
			else if (gd->rx_power < 4000)
			{
				pfo += 300;
			}
			else if (gd->rx_power < 9000)
			{
				pfo += 200;
			}
			else if (gd->rx_power < 13000)
			{
				pfo += 150;
			}
			else
			{
				pfo += 50;
			}
		}
	}

	return pfod_action();
}

static uint32_t Sum_V2_Plc, Sum_I4, Sum_V2_I2, Sum_I2_Plc, Sum_V4;

void pfod_dploss_init(void)
{
	Sum_V2_Plc = 0;
	Sum_I4 = 0;
	Sum_V2_I2 = 0;
	Sum_I2_Plc = 0;
	Sum_V4 = 0;
}

void pfod_dploss_cal(void)
{
	uint16_t Pcircuit, Plc, Vrect2, Irect2, Vrect2_x_Plc, Irect2_x_Plc;
	uint32_t Vrect4, Irect4, Vrec2_x_Irect2;

	//Pcircuit mW
	Pcircuit = pcircuit_factor * gd->icol_rms * gd->icol_rms/1000000;

	//Plc, unit mW
	Plc = gd->tx_power - gd->dploss_cal.prect - Pcircuit;
	
	//Vrect2, unit W
	Vrect2 = gd->dploss_cal.vrect * gd->dploss_cal.vrect / 1000000;

	//Vrect4, unit W^2
	Vrect4 = Vrect2 * Vrect2;

	//Irect2, unit A^2/1000
	Irect2 =  gd->dploss_cal.irect *  gd->dploss_cal.irect / 1000;

	//Irect4, unit A^4/1000000
	Irect4 = Irect2 * Irect2;

	Vrect2_x_Plc = Vrect2 * Plc / 1000;
	Irect2_x_Plc = Irect2 * Plc / 1000;
	Vrec2_x_Irect2 = Vrect2 * Irect2;

	Sum_V2_Plc += Vrect2_x_Plc;
	Sum_I4 += Irect4;
	Sum_V2_I2 += Vrec2_x_Irect2;
	Sum_I2_Plc += Irect2_x_Plc;
	Sum_V4 += Vrect4;

	printk("\r\n CAL_CAP %d %d %d %d %d * %d %d ",\
			gd->dploss_cal.index_cnt, gd->dploss_cal.preceived, gd->dploss_cal.prect, gd->dploss_cal.vrect, gd->dploss_cal.irect, gd->icol_rms, gd->tx_power);
#if 0
	printk(" * %d %d * %d %d %d %d %d %d %d * %d %d %d %d %d",
			Pcircuit, Plc, Vrect2, Vrect4, Irect2, Irect4, Vrect2_x_Plc, Irect2_x_Plc, Vrec2_x_Irect2,
		    Sum_V2_Plc, Sum_I4, Sum_V2_I2, Sum_I2_Plc, Sum_V4);
#endif
}

uint8_t pfod_dploss_cal_cmt(uint16_t *alpha, uint16_t *beta)
{
	uint32_t u32_tmp;
	uint64_t u64_tmp0, u64_tmp1, u64_tmp2;

	u64_tmp0 = mlp_32bit_x_32bit(Sum_V4, Sum_I4);
	u64_tmp1 = mlp_32bit_x_32bit(Sum_V2_I2, Sum_V2_I2);
	u32_tmp = (u64_tmp0 - u64_tmp1)/1000000;

	if (u32_tmp == 0) u32_tmp = 10000;

	//printk(" u64_tmp0:%d u64_tmp1:%d u32_tmp:%d", u64_tmp0, u64_tmp1, u32_tmp);

	u64_tmp0 = mlp_32bit_x_32bit(Sum_V2_Plc, Sum_I4);
	u64_tmp1 = mlp_32bit_x_32bit(Sum_V2_I2, Sum_I2_Plc);
	u64_tmp2 = u64_tmp0 - u64_tmp1;

	//printk(" u64_tmp0:%d u64_tmp1:%d u64_tmp2:%d", u64_tmp0, u64_tmp1, u64_tmp2);

	*alpha = u64_tmp2 / u32_tmp;//unit 1/1000000

	u64_tmp0 = mlp_32bit_x_32bit(Sum_I2_Plc, Sum_V4);
	u64_tmp1 = mlp_32bit_x_32bit(Sum_V2_I2, Sum_V2_Plc);
	u64_tmp2 = u64_tmp0 - u64_tmp1;

	//printk(" u64_tmp0:%d u64_tmp1:%d u64_tmp2:%d", u64_tmp0, u64_tmp1, u64_tmp2);
	
	*beta = u64_tmp2 / u32_tmp;//unit 1/1000

	printk("\r\n\r\n alpha:%d beta:%d", *alpha, *beta);//iphone alpha=0.0016*1000000, beta=1.3*1000
	printk(" * %d %d %d %d %d", Sum_V2_Plc, Sum_I4, Sum_V2_I2, Sum_I2_Plc, Sum_V4);
	return 0;
}

uint8_t pfod_dploss(void)
{
	uint16_t Pcircuit, Plc, Plc_cal;
	uint64_t u64_tmp0, u64_tmp1;

	//Pcircuit mW
	Pcircuit = 34 * gd->icol_rms * gd->icol_rms/1000000;

	//Plc, unit mW
	Plc = gd->tx_power - gd->rx_infos.pla_prect - Pcircuit;

	u64_tmp0 = mlp_32bit_x_32bit(gd->tx_infos.dp_alpha , gd->rx_infos.pla_vrect * gd->rx_infos.pla_vrect);
	u64_tmp1 = mlp_32bit_x_32bit(1000, 1000000);
	Plc_cal = u64_tmp0 / u64_tmp1;

	u64_tmp0 = mlp_32bit_x_32bit(gd->tx_infos.dp_beta, gd->rx_infos.pla_irect * gd->rx_infos.pla_irect);
	u64_tmp1 = u64_tmp0 / 1000000;
	Plc_cal = Plc_cal + u64_tmp1;

//	pfo = Plc - Plc_cal;
	pfo = Plc - Plc_cal - 300;//TODO: need tuning FOD

	printk("\r\n a:%d b:%d pcircu:%d plc:%d pcal:%d",gd->tx_infos.dp_alpha, gd->tx_infos.dp_beta, Pcircuit, Plc, Plc_cal);

	return pfod_action();
}

void pfod_log_print(void)// print long log and avoid the fsk window
{
#ifdef _PRINT_FOD_MSG
	printk("\r\n FOD-> %d %d %d %d t:%d %d ctx:%d v:%d %d i:%d %d p:%d %d(%d) r:%d %d pfo:%d %d",
		   gd->rx_infos.rx_type, gd->tx_infos.nego_cap, 144000000 / gd->pid_perd, gd->pid_phas,
		   gd->sys_infos.die_temp, gd->sys_infos.ntc_temp, gd->ctx,
		   gd->vbus, gd->vpwr_avg, gd->isns_avg, gd->icol_rms,
		   gd->tx_power, gd->rx_power, gd->rx_prect,
		   gd->rx_infos.pla_vrect, gd->rx_infos.pla_irect,
		   pfo, fod_count);
#endif

}

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

//	return res;
	return 0;
}
