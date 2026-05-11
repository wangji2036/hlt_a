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
uint8_t fod_enable;
uint8_t pfo_en_reco;//fixture and bpp set 0

uint8_t fod_count_filter;
uint8_t fod_count;
uint8_t mpla_count;

uint16_t pfo_thd;
uint16_t pfo_thd_reco;

uint16_t p_rect_max;

uint8_t pfo_values_index;
int32_t pfo_values[5];
int32_t pfo;
int32_t pfo_avg;


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

	fod_enable = MPP_25W_FOD_ENABLE;
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

uint32_t ploss_calc(uint8_t mode)
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

	wpc_wpc_printk("\r\n ploss:%d coil:%d fm:%d mos:%d snb:%d", temp_ploss, Pcoil, Pfm, Pmos, Psnuber);

	return  temp_ploss;
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

	wpc_wpc_printk("\r\n CAL_CAP %d %d %d %d %d * %d %d ",\
			gd->dploss_cal.index_cnt, gd->dploss_cal.preceived, gd->dploss_cal.prect, gd->dploss_cal.vrect, gd->dploss_cal.irect, gd->icol_rms, gd->tx_power);
#if 0
	wpc_wpc_printk(" * %d %d * %d %d %d %d %d %d %d * %d %d %d %d %d",
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

	//wpc_printk(" u64_tmp0:%d u64_tmp1:%d u32_tmp:%d", u64_tmp0, u64_tmp1, u32_tmp);

	u64_tmp0 = mlp_32bit_x_32bit(Sum_V2_Plc, Sum_I4);
	u64_tmp1 = mlp_32bit_x_32bit(Sum_V2_I2, Sum_I2_Plc);
	u64_tmp2 = u64_tmp0 - u64_tmp1;

	//wpc_printk(" u64_tmp0:%d u64_tmp1:%d u64_tmp2:%d", u64_tmp0, u64_tmp1, u64_tmp2);

	*alpha = u64_tmp2 / u32_tmp;//unit 1/1000000

	u64_tmp0 = mlp_32bit_x_32bit(Sum_I2_Plc, Sum_V4);
	u64_tmp1 = mlp_32bit_x_32bit(Sum_V2_I2, Sum_V2_Plc);
	u64_tmp2 = u64_tmp0 - u64_tmp1;

	//wpc_printk(" u64_tmp0:%d u64_tmp1:%d u64_tmp2:%d", u64_tmp0, u64_tmp1, u64_tmp2);
	
	*beta = u64_tmp2 / u32_tmp;//unit 1/1000

	wpc_wpc_printk("\r\n\r\n alpha:%d beta:%d", *alpha, *beta);//iphone alpha=0.0016*1000000, beta=1.3*1000
	wpc_wpc_printk(" * %d %d %d %d %d", Sum_V2_Plc, Sum_I4, Sum_V2_I2, Sum_I2_Plc, Sum_V4);
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

	wpc_wpc_printk("\r\n a:%d b:%d pcircu:%d plc:%d pcal:%d",gd->tx_infos.dp_alpha, gd->tx_infos.dp_beta, Pcircuit, Plc, Plc_cal);

	return pfod_action();
}


