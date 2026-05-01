#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "osal.h"
#include "qdt.h"
#include "pid.h"
#include "gui.h"
#include "_wpc.h"
#include "qfod.h"
#include "wpc_idle.h"
#include "wpc_ping.h"
#include "debug.h"
// #include "BMS_FixPoint.h"  // gauge removed
#include "mpp.h"
#include "tcpm.h"
#include"sleep.h"
#include "port_manager.h"
#include "wpc_nego.h"


static uint8_t rx_may_still_be_flag;
static uint8_t qdt_try_ping_count;
static uint8_t qdt_have_obj_count;
static uint8_t qdt_obj_remove_count;
static uint8_t qdt_large_metal_count;
static uint16_t qdt_just_count;

static uint8_t xfer_fod_remove_delta_Q;
static uint8_t xfer_fod_remove_delta_F;
static uint8_t xfer_fod_obj_remove_cnt;

static uint16_t pre_q[4], pre_f[4], delta_q_pre, delta_f_pre;

uint32_t delta_abs(uint32_t a, uint32_t b)
{
	return (a > b) ? a - b: b - a;
}

void enter_buff(uint16_t q, uint16_t f)
{
	delta_q_pre = delta_abs(q, pre_q[3]);
	delta_f_pre = delta_abs(f, pre_f[3]);

	pre_q[0] = pre_q[1];
	pre_q[1] = pre_q[2];
	pre_q[2] = pre_q[3];
	pre_q[3] = q;

	pre_f[0] = pre_f[1];
	pre_f[1] = pre_f[2];
	pre_f[2] = pre_f[3];
	pre_f[3] = f;
}

uint8_t is_stable(void)
{
	uint16_t q_max, q_min, f_max, f_min;

	q_max = q_min = pre_q[0];
	f_max = f_min = pre_f[0];

	for (int i=0; i<4; i++)
	{
		if (pre_q[i] > q_max) q_max = pre_q[i];
		if (pre_q[i] < q_min) q_min = pre_q[i];
		if (pre_f[i] > f_max) f_max = pre_f[i];
		if (pre_f[i] < f_min) f_min = pre_f[i];

//		printk("\r\n %d %d", pre_q[i], pre_q[i]);
	}

//	printk("\r\n max_min %d %d %d %d", q_max, q_min, f_max, f_min);

	return (delta_abs(q_max, q_min) < 30) && (delta_abs(f_max, f_min) < 30);
}

void idle_qfod_init(void)
{
	// for (int i=0; i<4; i++)
	// {
	// 	pre_q[i] = ap->q_factor_base_value;
	// 	pre_f[i] = ap->fs_base_value;
	// }

	qdt_try_ping_count = 0;
	qdt_have_obj_count = 0;
	qdt_obj_remove_count = 0;
	qdt_large_metal_count = 0;
	qdt_just_count = 0;
	xfer_fod_remove_delta_Q = 4;
	xfer_fod_remove_delta_F = 50;
	xfer_fod_obj_remove_cnt = 0;
	gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
	gd->dig_ping_continuous_cnt = 0;
	gd->tx_infos.reping_cnt = 2;
}

uint8_t idle_qdt_back_to_normal(void)
{
	return (gd->tx_infos.q_fact + ap->q_factor_reco_value > ap->q_factor_base_value && gd->tx_infos.q_fact < ap->q_factor_limH_value &&
			gd->tx_infos.f_self + ap->fs_reco_value > ap->fs_base_value && gd->tx_infos.f_self < ap->fs_limH_value);
}

static void idle_obj_remove_detect(void)
{
	if (idle_qdt_back_to_normal())
	{
		if (++qdt_obj_remove_count > 3)
		{
			idle_qfod_init();
			gd->tx_infos.ept_attempt_cnt = 0;
		}
	}
	else
	{
		qdt_obj_remove_count = 0;
	}
}

extern void mpp_mate_q_detect(void);

uint8_t qfod_detect(void)
{
	uint8_t no_obj = 1;

	static uint8_t rx_may_still_be_remove_cnt = 0;
	if (rx_may_still_be_flag != 0)
	{
		if (idle_qdt_back_to_normal())
		{
			if (++rx_may_still_be_remove_cnt > 3)
			{
				rx_may_still_be_flag = 0;
				rx_may_still_be_remove_cnt = 0;
				gd->tx_infos.fo_exist = 0;
				gd->power_mode = nominal;
			}
		}
		else
		{
			rx_may_still_be_remove_cnt = 0;
		}
	}
	else
	{
		rx_may_still_be_remove_cnt = 0;
	}

//	if (rx_may_still_be_flag != 0 && idle_qdt_back_to_normal())
//	{
//		rx_may_still_be_flag = 0;
//	}

	wpc_printk("\r\n idle: [%d] [q:%d,%d,%d,%d] [f:%d,%d,%d,%d] [t:%d,%d] [fo:%d,%d]",
			gd->ptx_idle_phase_status,
			gd->tx_infos.q_fact, ap->q_factor_base_value, gd->tx_infos.q_fact - ap->q_factor_base_value, delta_q_pre,
			gd->tx_infos.f_self, ap->fs_base_value, gd->tx_infos.f_self - ap->fs_base_value, delta_f_pre,
			gd->sys_infos.ntc_temp_wpc, gd->sys_infos.die_temp, qdt_have_obj_count, ap->pin_fod_cnt);

	switch (gd->ptx_idle_phase_status)
	{
		case WPC_IDLE_STAT_STANDBY:
			if (gd->tx_infos.q_fact < ap->q_factor_limL_value || gd->tx_infos.q_fact > ap->q_factor_limH_value ||
					gd->tx_infos.f_self < ap->fs_limL_value || gd->tx_infos.f_self > ap->fs_limH_value)
			{
				qdt_try_ping_count = 0;
				qdt_have_obj_count = 0;
				if (++qdt_large_metal_count >= 3)
				{
					gd->ptx_idle_phase_status = WPC_IDLE_STAT_LAR_MET;
				}
			
			}
			else
			{
				qdt_large_metal_count = 0;

				if (qdt_have_obj_count)
				{
					if (idle_qdt_back_to_normal())
					{
						qdt_have_obj_count = 0;

					}
					else
					{
						if (++qdt_have_obj_count > ap->pin_fod_cnt)
						{
							idle_qfod_init();
							gd->ptx_idle_phase_status = WPC_IDLE_STAT_QDT_FOD;
						}
					}
	
				}
				else
				{
					if ((gd->tx_infos.q_fact + 0 + rx_may_still_be_flag * 10 + ap->q_factor_obj_value < ap->q_factor_base_value) ||
						(gd->tx_infos.f_self > ap->fs_limL_value && gd->tx_infos.f_self + ap->fs_obj_value + rx_may_still_be_flag * 160 < ap->fs_base_value))
					{
						++qdt_have_obj_count;
						qdt_try_ping_count = 0;
					}
					else
					{
						qdt_try_ping_count += (1 + rx_may_still_be_flag * ap->pin_max_cnt);
						gd->tx_infos.fo_exist = 0;
					}
					//printk("333333333");
				}
			}
			break;
		case WPC_IDLE_STAT_XER_COM:
			idle_obj_remove_detect();
			if ((++qdt_just_count * ap->t_next_ping) > 11 * 60 * 1000) //11min
			{
				idle_qfod_init();
			}
			break;
		case WPC_IDLE_STAT_XER_FOD:
			idle_obj_remove_detect();
			break;
		case WPC_IDLE_STAT_QDT_FOD:
			idle_obj_remove_detect();
			break;
		case WPC_IDLE_STAT_LAR_MET:
			idle_obj_remove_detect();
			break;
		case WPC_IDLE_STAT_EPT_ERR:
			idle_obj_remove_detect();
			if ((++qdt_just_count * ap->t_next_ping) > 11 * 60 * 1000) //Refer to other TX, 11min is enough, IOC test 10min.
			{
				idle_qfod_init();
			}
			break;
		case WPC_IDLE_STAT_EPT_RES:
			idle_obj_remove_detect();
			if (gd->tx_infos.ept_attempt_cnt < 3)
			{
				if (++qdt_just_count >= gd->tx_infos.reping_cnt)
				{
					idle_qfod_init();
					qdt_have_obj_count = 2; //force a digital ping
				}
			}
			break;
		case WPC_IDLE_STAT_EPT_REP:
			idle_obj_remove_detect();
			if (++qdt_just_count >= gd->tx_infos.reping_cnt)
			{
				idle_qfod_init();
				qdt_have_obj_count = 2; //force a digital ping
			}
			break;
		case WPC_IDLE_STAT_QDT_CALI:
			qfod_qdt_cali_process();
			break;
		default:
			idle_obj_remove_detect();
			break;
	}
	// modified to force digital ping after several Q, even though no objects was detected.
//	if (qdt_have_obj_count > 1)
	if (qdt_try_ping_count >= ap->pin_max_cnt || qdt_have_obj_count > 1)

	{
		no_obj = 0;
		qdt_try_ping_count = 0;
	}

	if (ap->pin_fod_dis)
	{
		no_obj = 0;
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
	}

	if (gd->ptx_idle_phase_status == WPC_IDLE_STAT_STANDBY && no_obj == 0)
	{
		gd->dig_ping_continuous_cnt++;
	}
	else
	{
		gd->dig_ping_continuous_cnt = 0;
	}
//	printk("\r\n dig_ping_continuous_cnt-> %d", gd->dig_ping_continuous_cnt);

	return no_obj;
}
/*----------------------------------- IDLE -----------------------------------*/

void wpc_idle_dping_select(void)
{
	switch (gd->adp.adp_type)
	{
		case EADP_TYPE_QC3P0_12V:
		case EADP_TYPE_QC3P0_20V:
		case EADP_TYPE_PD3P0_10W:
		case EADP_TYPE_PD3P0_20W:
		case EADP_TYPE_PD3P0_30W:
		case EADP_TYPE_PD3P0_50W:
			gd->dig_ping_volt = ap->dig_ping_volt_6v;
			gd->dig_ping_perd = ap->dig_ping_perd_6v;
			gd->dig_ping_duty = ap->dig_ping_duty_6v;
			gd->dig_ping_phas = ap->dig_ping_phas_6v;
			break;
		case EADP_TYPE_POWERBANK_09V:
			if(gd->vpwr <7500)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_5v;
				gd->dig_ping_perd = ap->dig_ping_perd_5v;
				gd->dig_ping_duty = ap->dig_ping_duty_5v;
				gd->dig_ping_phas = ap->dig_ping_phas_5v;
			}
			else if(gd->vpwr <10000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_9v;
				gd->dig_ping_perd = ap->dig_ping_perd_9v;
				gd->dig_ping_duty = ap->dig_ping_duty_9v;
				gd->dig_ping_phas = ap->dig_ping_phas_9v;
			}
			else if(gd->vpwr <13000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_11v;
				gd->dig_ping_perd = ap->dig_ping_perd_11v;
				gd->dig_ping_duty = ap->dig_ping_duty_11v;
				gd->dig_ping_phas = ap->dig_ping_phas_11v;
			}
			else//<15v
			{
				gd->dig_ping_volt = gd->vpwr;//12000;
				gd->dig_ping_perd = 144000000 / 147772;
				gd->dig_ping_duty = 100;
				gd->dig_ping_phas = 50;
			}
			break;
		case EADP_TYPE_QC2P0_09V:
		case EADP_TYPE_PD2P0_09V:
		case EADP_TYPE_PD2P0_12V:
		case EADP_TYPE_DCSRC_09V:
		case EADP_TYPE_POWERBANK_PPS:
			if(gd->vpwr <7500)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_5v;
				gd->dig_ping_perd = ap->dig_ping_perd_5v;
				gd->dig_ping_duty = ap->dig_ping_duty_5v;
				gd->dig_ping_phas = ap->dig_ping_phas_5v;
			}
			else if(gd->vpwr <10000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_9v;
				gd->dig_ping_perd = ap->dig_ping_perd_9v;
				gd->dig_ping_duty = ap->dig_ping_duty_9v;
				gd->dig_ping_phas = ap->dig_ping_phas_9v;
			}
			else if(gd->vpwr <13000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_11v;
				gd->dig_ping_perd = ap->dig_ping_perd_11v;
				gd->dig_ping_duty = ap->dig_ping_duty_11v;
				gd->dig_ping_phas = ap->dig_ping_phas_11v;
			}
			else//<15v
			{
				gd->dig_ping_volt = gd->vpwr;//12000;
				gd->dig_ping_perd = 144000000 / 147772;
				gd->dig_ping_duty = 100;
				gd->dig_ping_phas = 50;
			}
			break;
		case EADP_TYPE_POWERBANK_WIRELESS_ONLY:
//			gd->dig_ping_volt = ap->dig_ping_volt_11v;
//			gd->dig_ping_perd = ap->dig_ping_perd_11v;
//			gd->dig_ping_duty = ap->dig_ping_duty_11v;
//			gd->dig_ping_phas = ap->dig_ping_phas_11v;

			gd->dig_ping_volt = ap->dig_ping_volt_6v;
			gd->dig_ping_perd = ap->dig_ping_perd_6v;
			gd->dig_ping_duty = ap->dig_ping_duty_6v;
			gd->dig_ping_phas = ap->dig_ping_phas_6v;

			break;
//		case EADP_TYPE_PD2P0_12V:
//			gd->dig_ping_volt = ap->dig_ping_volt_12v;
//			gd->dig_ping_perd = ap->dig_ping_perd_12v;
//			gd->dig_ping_duty = ap->dig_ping_duty_12v;
//			gd->dig_ping_phas = ap->dig_ping_phas_12v;
//			break;
		default:
			if(gd->vpwr <7500)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_5v;
				gd->dig_ping_perd = ap->dig_ping_perd_5v;
				gd->dig_ping_duty = ap->dig_ping_duty_5v;
				gd->dig_ping_phas = ap->dig_ping_phas_5v;
			}
			else if(gd->vpwr <10000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_9v;
				gd->dig_ping_perd = ap->dig_ping_perd_9v;
				gd->dig_ping_duty = ap->dig_ping_duty_9v;
				gd->dig_ping_phas = ap->dig_ping_phas_9v;
			}
			else if(gd->vpwr <13000)
			{
				gd->dig_ping_volt = ap->dig_ping_volt_11v;
				gd->dig_ping_perd = ap->dig_ping_perd_11v;
				gd->dig_ping_duty = ap->dig_ping_duty_11v;
				gd->dig_ping_phas = ap->dig_ping_phas_11v;
			}
			else//<15v
			{
				gd->dig_ping_volt = gd->vpwr;//12000;
				gd->dig_ping_perd = 144000000 / 147772;
				gd->dig_ping_duty = 100;
				gd->dig_ping_phas = 50;
			}
			break;
	}
}

void wpc_idle_dig_ping_init_128K(void)
{
	/*
	//wpc_idle_dping_select();
	gd->dig_ping_volt = 11000;
	gd->dig_ping_perd = 1127;//127.77K
	gd->dig_ping_duty = 125; // 250;
	gd->dig_ping_phas = 0;
*/
	wpc_idle_dping_select();
	pid_init();
	mpp_power_limit_init();

	if (gd->pid_volt != gd->dig_ping_volt || wpc_mode != wpc_mode_pre)
	{
		gd->pid_volt = gd->dig_ping_volt;
		fml_adp_volt_set(gd->pid_volt);
		wpc_mode_pre = wpc_mode;
	}

	if(gd->adp.adp_type == EADP_TYPE_POWERBANK_WIRELESS_ONLY )
	{
		gd->pid_volt = gd->dig_ping_volt;
		fml_adp_volt_set(gd->pid_volt);
	}


	fml_nu103x_por_rst();
	ctx_switch(4);

	delay_1ms(5);

	if (gd->dig_ping_continuous_cnt % 5 == 0) //for IOC test, TPR#1C, 6.2.09 Test#23
	{
//		printk("\r\n dig_ping_continuous_cnt: %d", gd->dig_ping_continuous_cnt);
/*		if(gd->dig_ping_volt == ap->dig_ping_volt_11v)
		{
			gd->dig_ping_volt = 11000;
			gd->dig_ping_perd = 1000;//144K
			gd->dig_ping_duty = 125; // 250;
			gd->dig_ping_phas = 0;
		}*/
		fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
//		fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_EVDM, _1030_CFG_DMO1_DDM_GAIN_MODE_AUTO, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
		fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
		gd->dmo1_phase = _NU103x_DM_PHASE_DIG_PING;

		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
//		fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K1);
		fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X36, _1030_CFG_DMO2_VCAP_RATIO_K1);
		gd->dmo2_phase = _NU103x_DM_PHASE_DIG_PING;
	}
	else
	{
/*		if(gd->dig_ping_volt == ap->dig_ping_volt_11v)
		{
			gd->dig_ping_volt = 11000;
			gd->dig_ping_perd = 1127;//127.772K
			gd->dig_ping_duty = 125; // 250;
			gd->dig_ping_phas = 0;
		}*/
		fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);

		gd->dmo1_phase = _NU103x_DM_PHASE_DIG_PING;
		gd->dmo2_phase = _NU103x_DM_PHASE_DIG_PING;

		static uint8_t ddm_ping_cfg = 0;
		switch (ddm_ping_cfg)
		{
		case 0:
			ddm_ping_cfg = 1;
			fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
			fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K1);
			break;
		case 1:
		default:
			ddm_ping_cfg = 0;
			fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_EVDM, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
			fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K2);
			break;
		}
	}
	gd->renego_flag = 0;
	gd->pid_volt = gd->dig_ping_volt;
	gd->pid_perd = gd->dig_ping_perd;
	if(gd->dig_ping_volt<6000)
	{
		gd->pid_duty = 260;
	}
	else
	{
	    gd->pid_duty = 50;
	}
	gd->pid_phas = gd->dig_ping_phas;

	gd->sys_infos.tim3_evnt |= 1; //duty ramp up
	hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
	hal_timer_init(TMR3);//1ms
}

void wpc_idle_dig_ping_init_360K(void)
{
//	wpc_idle_dping_select();
	gd->dig_ping_volt = 11000;
	gd->dig_ping_perd = 144000000/360000;
	gd->dig_ping_duty = 500;
	if (TRUE == gd->tx_infos.flg_mode_cloak)
	{
		gd->dig_ping_phas =  35; //NOK9 IOC CLOAKING TEST
	}
	else
	{
		gd->dig_ping_phas =  40;
	}

	pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
	pid_set_freq_limit(144000000/360000, 144000000/360000, 144000000/360000);
	pid_set_duty_limit(500, 500, 500);
	pid_set_phas_limit( 60,  50,   0);

	fml_nu103x_por_rst();

	ctx_switch(2);

	delay_1ms(5);

	pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);

	if (gd->pid_volt != gd->dig_ping_volt || wpc_mode != wpc_mode_pre)
	{
		gd->pid_volt = gd->dig_ping_volt;
		fml_adp_volt_set(gd->pid_volt);
		wpc_mode_pre = wpc_mode;
	}

		wpc_printk(" [ctx:%d k:%d pid-v %d]", gd->ctx, gd->k_est,gd->pid_volt);

		//config_1
		fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
//		fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_EVDM, _1030_CFG_DMO1_DDM_GAIN_MODE_AUTO, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
		fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
		gd->dmo1_phase = _NU103x_DM_PHASE_DIG_PING;

		fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
//		fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K1);
		fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K1);
		gd->dmo2_phase = _NU103x_DM_PHASE_DIG_PING;

		gd->pid_perd = gd->dig_ping_perd;
		gd->pid_duty = gd->dig_ping_duty;
		gd->pid_phas = 180;

//		gd->sys_infos.tim3_evnt = 2; //phase ramp up
		while (gd->pid_phas > gd->dig_ping_phas)//phase ramp up, ~250us
		{
			if (gd->pid_phas > gd->dig_ping_phas + 10)
			{
				gd->pid_phas -= 10;
			}
			else
			{
				gd->pid_phas = gd->dig_ping_phas;
			}
			hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
//			delay_1us(2);
		}
		if (ap->mpp_dither_en)
		{
			hal_epwm_afd_start(EPWM1, 4, 2);
		}

#if DIG_DDM_ENABLE
		if (144000 / (EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 360)
		{
			hal_ddm_dig_ping();
			hal_ecap_dig_ddm_init();
			hal_eadc_ddm_init();
			fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K1);
			fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_CAP);
			wpc_printk("\r\n ----- enable digital ddm");
		}
#endif
}

uint16_t cnt_cloak_dig_ping = 0;
uint16_t cnt_cloak_det_ping = 0;
void wpc_idle_cloak_phase_process(void)
{
	if (TRUE == gd->tx_infos.flg_mode_cloak)
	{
		wpc_printk("\r\n cloak_2: %d %d %d %d", cnt_cloak_det_ping, cnt_cloak_dig_ping, gd->tx_infos.cloak_dig_ping_delay, gd->tx_infos.cloak_det_ping_delay);
		cnt_cloak_dig_ping++;
		cnt_cloak_det_ping++;

		if (cnt_cloak_dig_ping >= gd->tx_infos.cloak_dig_ping_delay)
		{
			//digital ping
			cnt_cloak_dig_ping = 0;
			cnt_cloak_det_ping = 0;

			fml_ask_enable();

			wpc_idle_dig_ping_init_360K();

			wpc_printk("\r\n dig_ping [%d %d %d %d %d][%d %d %d %d]", gd->vbus, gd->vpwr, gd->isns, gd->sys_infos.ntc_temp_wpc, gd->sys_infos.die_temp,
					gd->pid_volt, 144000000/gd->pid_perd, gd->dig_ping_duty, gd->pid_phas);

			gd->ptx_protocol_phase = WPC_PHASE_CLOAK;
			osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
			osal_stop_timerEx(WPC_PING_TIMER);
		}
		else if (cnt_cloak_det_ping >= gd->tx_infos.cloak_det_ping_delay)
		{
			fml_nu103x_por_rst();

			ctx_switch(4);

			delay_1ms(5);

			fml_qdt_detect((uint32_t *)&gd->tx_infos.q_fact, (uint32_t *)&gd->tx_infos.f_self);

			ctx_switch(2);

			delay_1ms(5);

			wpc_printk("\r\n idle:%d [q:%d,%d,%d,%d] [f:%d,%d,%d,%d]", gd->ptx_idle_phase_status,
					gd->tx_infos.q_fact, ap->q_factor_base_value, gd->tx_infos.q_fact - ap->q_factor_base_value, delta_q_pre,
					gd->tx_infos.f_self, ap->fs_base_value, gd->tx_infos.f_self - ap->fs_base_value, delta_f_pre);

			if ((0 == gd->tx_infos.q_fact_air) && (0 == gd->tx_infos.f_self_air))
			{
				gd->tx_infos.q_fact_air = gd->tx_infos.q_fact;
				gd->tx_infos.f_self_air = gd->tx_infos.f_self;
				wpc_printk("\r\n air_q [%d %d]", gd->tx_infos.q_fact_air, gd->tx_infos.f_self_air);
			}

			enter_buff(gd->tx_infos.q_fact, gd->tx_infos.f_self);

			static uint8_t rx_may_still_be_remove_cnt = 0;
			if (idle_qdt_back_to_normal())
			{
				if (++rx_may_still_be_remove_cnt > 3)
				{
					rx_may_still_be_flag = 0;
					rx_may_still_be_remove_cnt = 0;
					rx_may_still_be_remove_cnt = 0;
					gd->tx_infos.fo_exist = 0;
					gd->ptx_protocol_phase = WPC_PHASE_IDLE;
					gd->tx_infos.flg_mode_cloak = FALSE;
					osal_stop_timerEx(WPC_NEXT_TIMER);
					osal_start_timerEx(WPC_PING_TIMER, gd->tx_infos.t_next_ping, ap->t_next_ping, WPC_TASK, WPC_EVT_DIG_PING);
					return;
				}
			}
			else
			{
				rx_may_still_be_remove_cnt = 0;
			}

			//short ping
			cnt_cloak_det_ping = 0;

			fml_nu103x_ddm_init();

			gd->dig_ping_volt = 11000;
			gd->dig_ping_perd = 144000000/360000;
			gd->dig_ping_duty = 500;
			gd->dig_ping_phas =  40;

			gd->pid_perd = gd->dig_ping_perd;
			gd->pid_duty = gd->dig_ping_duty;
			gd->pid_phas = 180;

			while (gd->pid_phas > gd->dig_ping_phas)//phase ramp up, ~250us
			{
				if (gd->pid_phas > gd->dig_ping_phas + 10)
				{
					gd->pid_phas -= 10;
				}
				else
				{
					gd->pid_phas = gd->dig_ping_phas;
				}
				hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
//				delay_1us(2);
			}
			delay_1us(2000);
			hal_epwm_pwm_stop(EPWM1);
		}
	}
}

uint8_t adp_ready;
uint32_t rrlen;
extern uint8_t array_digest[];
extern uint8_t adt_data_recv_buf[18];
extern uint8_t cert_chain[];
extern bool bat_ntc_dual_dischg_inhibit;

void wpc_idle_phase_process(void)
{
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if (gd->usb_comm_activated) return;
#endif
	static uint8_t bat_low_sleep = 0;

	if(gd->bat_dead_flag)
	{
		bat_low_sleep++;
		if(g_port.port_state[0] == PORT_STATE_SINK) {gd->bat_dead_flag = 0;bat_low_sleep =0;}

		wpc_printk("low cnt [%d]",bat_low_sleep);
		if(bat_low_sleep >= 50)
		{
			bat_low_sleep = 0;
			if(SYS->PID_INFO.BITS.VER != CHIP_VER_A0)SLP_vNormalToSleep();
		}
	}
	else
	{
		bat_low_sleep = 0;
	}
	/* SOC lock with hysteresis: <=1% disable (stop + mode disable), >=2% re-enable */
	static uint8_t wpc_dualsrc_low_soc_lock = 0;
	if (!wpc_dualsrc_low_soc_lock)
	{
		if (gd->real_soc_show <= 1)
		{
			gd->wpc_disable = 0x01;
			tcpm_stop_wpc(WPC_DELAY);
			tcpm_update_wpc_work_mode(TCPM_WPC_WORK_DISABLE);
			wpc_printk("\r\n WPC disabled: SOC<=1%% (SOC=%d)", gd->real_soc_show);
			wpc_dualsrc_low_soc_lock = 1;
		}
	}
	else
	{
		if (gd->real_soc_show >= 2)
		{
			gd->wpc_disable = 0x00;
		tcpm_update_wpc_work_mode(TCPM_WPC_WORK_BOOST);
			wpc_printk("\r\n WPC re-enabled: SOC>=2%% (SOC=%d)", gd->real_soc_show);
			wpc_dualsrc_low_soc_lock = 0;
		}
	}
	
	//printk("sigle click %d \r\n",gd->sigle_clicked);
	// if(gd->vpwr >13000){
	// 	printk("no wpc due to vbus %d \r\n",gd->vpwr);
	// 	return;
	// }
	if(gd->bat_dead_flag) return;
	if(gd->adp.adp_type == EADP_TYPE_POWERBANK_WIRELESS_ONLY && (gd->ptx_idle_phase_status == WPC_IDLE_STAT_STANDBY
		|| 	gd->ptx_idle_phase_status == WPC_IDLE_STAT_XER_FOD || 	gd->ptx_idle_phase_status == WPC_IDLE_STAT_QDT_FOD
		|| 	gd->ptx_idle_phase_status == WPC_IDLE_STAT_LAR_MET))
	{
		if(gd->idle_to_sleep_cnt >300)
		{
			gd->idle_to_sleep_cnt = 0;
			if(SYS->PID_INFO.BITS.VER != CHIP_VER_A0)SLP_vNormalToSleep();
		}
		else
		{
			gd->idle_to_sleep_cnt++;
			wpc_printk("idle cnt [%d]",gd->idle_to_sleep_cnt);
		}
	}
	else
	{
		gd->idle_to_sleep_cnt = 0;
	}
	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE)
	{
		if(!gd->sigle_clicked)
		{
			return;
		}
	}
	if (gd->ptx_protocol_phase != WPC_PHASE_IDLE)
	{
		return;
	}
	if(tcpm_qi_work_delay)
	{
		tcpm_qi_work_delay--;
		return;
	}
	if(wpc_mode == TCPM_WPC_WORK_DISABLE || gd->wpc_disable == 0x01||gd->wirless_ntc_lock||gd->bat_ntc_lock_flag) return;
	if (gd->prot_sts.tdie_otp_flag || gd->prot_sts.tdie_utp_flag || gd->prot_sts.tntc_otp_flag || gd->prot_sts.tntc_utp_flag ||
		gd->prot_sts.isns_ocp_flag || gd->prot_sts.vbus_ovp_flag || gd->prot_sts.vbus_uvp_flag || gd->prot_sts.vbus_dpl_flag ||
		gd->prot_sts.vpwr_ovp_flag || gd->prot_sts.pout_opp_flag || gd->bat_ov_forbid_flag)
	{
		wpc_printk("\r\n system protection ");
		if (gd->prot_sts.tntc_otp_flag) wpc_printk("[tntc_otp:%d]", gd->sys_infos.ntc_temp_wpc);
		if (gd->prot_sts.tntc_utp_flag) wpc_printk("[tntc_utp:%d]", gd->sys_infos.ntc_temp_wpc);
		if (gd->prot_sts.tdie_otp_flag) wpc_printk("[tdie_otp:%d]", gd->sys_infos.die_temp);
		if (gd->prot_sts.tdie_utp_flag) wpc_printk("[tdie_utp:%d]", gd->sys_infos.die_temp);
		if (gd->prot_sts.isns_ocp_flag) wpc_printk("[isns_ocp:%d]", gd->isns);
		if (gd->prot_sts.vbus_ovp_flag) wpc_printk("[vbus_ovp:%d]", gd->vbus);
		if (gd->prot_sts.vbus_uvp_flag) wpc_printk("[vbus_uvp:%d]", gd->vbus);
		if (gd->prot_sts.vbus_dpl_flag) wpc_printk("[vbus_dpl:%d]", gd->vbus);
		if (gd->prot_sts.vpwr_ovp_flag) wpc_printk("[vpwr_ovp:%d]", gd->vpwr);
		if (gd->prot_sts.pout_opp_flag) wpc_printk("[pout_opp:%d %d]", gd->vpwr, gd->isns);
		if (gd->bat_ov_forbid_flag) wpc_printk("[bat_ov_forbid]");
		return;
	}

	switch (gd->tx_infos.ping_type)
	{
		case qdt_ping:
			break;
		case dig_ping:
			break;
		case det_ping:
			break;
		default:
			break;
	}

	hal_badc_isns_chan_offest_update();

	fml_nu103x_por_rst();

	ctx_switch(4);//for qdt

	delay_1ms(5);

	fml_qdt_detect((uint32_t *)&gd->tx_infos.q_fact, (uint32_t *)&gd->tx_infos.f_self);

	if ((0 == gd->tx_infos.q_fact_air) && (0 == gd->tx_infos.f_self_air))
	{
		gd->tx_infos.q_fact_air = gd->tx_infos.q_fact;
		gd->tx_infos.f_self_air = gd->tx_infos.f_self;
		wpc_printk("\r\n air_q [%d %d]", gd->tx_infos.q_fact_air, gd->tx_infos.f_self_air);
	}

	enter_buff(gd->tx_infos.q_fact, gd->tx_infos.f_self);

	if (qfod_detect())
	{
		return;
	}
	gd->ntc_led_off = 0;
	fml_ask_enable();

	if (gd->tx_infos.dig_ping_type == _128K_HB)
	{
		wpc_idle_dig_ping_init_128K();
	}
	else if (gd->tx_infos.dig_ping_type == _360K_FB)
	{
		gd->tx_infos.dig_ping_type = _128K_HB;
		wpc_printk("back to 128k-2\r\n");
		wpc_idle_dig_ping_init_360K();
	}
	fml_nu103x_config(_1030_CFG_QDT_PRECHARGE_V1P8);// set to 1.8v again, for better DDM
	wpc_printk("\r\n ping: [%d] [%d %d] [%d %d %d %d]", gd->tx_infos.dig_ping_type, gd->vbus, gd->vpwr,
			gd->pid_volt, 144000000 / gd->pid_perd, gd->dig_ping_duty, gd->pid_phas);

	wpc_printk(" <dmo1-%d-%d%d%d>", gd->dmo1_phase, gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC,
			gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_MOD, gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_FIX);

	wpc_printk(" <dmo2-%d-%d%d%d%d>", gd->dmo2_phase, gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC,
		gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_MOD, gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_FIX, gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K);

//	fml_ask_enable();
	power_contract_init();
	gd->ptx_protocol_phase = WPC_PHASE_PING;
	osal_start_timerEx(WPC_NEXT_TIMER, T_PING, 0, WPC_TASK, WPC_EVT_PIN_NO_PKT);
	osal_stop_timerEx(WPC_PING_TIMER);
}
