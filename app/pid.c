#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "g_data.h"
#include "debug.h"
#include "adp.h"
#include "pid.h"
#include "_wpc.h"
#include "delay.h"
#include "pfod.h"

enum pid_ctrl_mode_t
{
	EPID_CTRL_MODE_VOLT = 0,
	EPID_CTRL_MODE_FREQ = 1,
	EPID_CTRL_MODE_DUTY = 2,
	EPID_CTRL_MODE_PHAS = 3,
	EPID_CTRL_MODE_SCAP = 4,
};

enum pid_ctrl_evnt_t
{
	EPID_EVENT_NUL = 0x00,
	EPID_EVENT_SAK = 0x01,
	EPID_EVENT_OVP = 0x02,
	EPID_EVENT_UVP = 0x04,
};

static uint8_t m_u8CtrlErrShakeCnt, m_u8OVPCount, m_u8UVPCount;
static uint8_t m_u8PositiveCevSum;
static enum pid_ctrl_evnt_t m_pid_ctrl_evnt;
static enum pid_ctrl_mode_t m_pid_ctrl_mode;
static void pid_ctrl_mode_sel(int8_t cep);

#define _Ctx1 0
#define _Ctx2 82
#define _Ctx3 400
#define _Ctx4 47

#define cap_s1_enable() \
	do                  \
	{                   \
	} while (0) //not use, spec 33nF
#define cap_s1_disable() \
	do                   \
	{                    \
	} while (0)                                                      //not use, spec 33nF
#define cap_s3_enable() fml_nu103x_config(_1030_CFG_DRVH1_TURN_ON_)  //400nF
#define cap_s3_disable() fml_nu103x_config(_1030_CFG_DRVH1_TURN_OFF) //400nF
#define cap_s2_enable() fml_nu103x_config(_1030_CFG_DRVH2_TURN_ON_)  //47nF
#define cap_s2_disable() fml_nu103x_config(_1030_CFG_DRVH2_TURN_OFF) //47nF

//#define cap_s2_enable()					fml_nu103x_config(_1030_CFG_DRVH1_TURN_ON_)//400nF
//#define cap_s2_disable()				fml_nu103x_config(_1030_CFG_DRVH1_TURN_OFF)//400nF
//#define cap_s3_enable()					fml_nu103x_config(_1030_CFG_DRVH2_TURN_ON_)//47nF
//#define cap_s3_disable()				fml_nu103x_config(_1030_CFG_DRVH2_TURN_OFF)//47nF

/**
 * Actual ctx_ind  0: Ctx=68nF, 1: Ctx=68nF,  2: Ctx=115nF, 3: Ctx=468nF, 4: Ctx=515nF
 * Spec   ctx_ind  0: Ctx=68nF, 1: Ctx=101nF, 2: Ctx=134nF, 3: Ctx=468nF, 4: Ctx=501nF
 */
void ctx_switch(uint8_t ctx_ind)
{
#if ONLY7_5W_ENALBE
	gd->ctx = 468;
	gd->ctx_ind = 0;
	return;
#else
	gd->ctx_ind = ctx_ind;
#endif

	VIC_vModuleDisable();
	switch (ctx_ind)
	{
	case 0:
		//			cap_s1_disable();
		cap_s2_disable();
		//			cap_s3_disable();
		gd->ctx = 82;
		break;
	case 1:
		//			cap_s1_enable();
		cap_s2_disable();
		//			cap_s3_disable();
		gd->ctx = 82;
		break;
	case 2:
		//			cap_s1_enable();
		cap_s2_disable();
		//			cap_s3_enable();
		gd->ctx = 82;
		break;
	case 3:
		//			cap_s1_disable();
		cap_s2_enable();
		//			cap_s3_disable();
		gd->ctx = 482;
		break;
	case 4:
		//			cap_s1_enable();
		cap_s2_enable();
		//			cap_s3_enable();
		gd->ctx = 482;
		break;
	default:
		break;
	}
	VIC_vModuleEnable();
}

void pid_init(void)
{
	gd->rx_infos.cep_pre = 0;
	m_u8CtrlErrShakeCnt = 0;
	m_u8OVPCount = m_u8UVPCount = 0;
	m_pid_ctrl_evnt = EPID_EVENT_NUL;

	switch (gd->adp.adp_type)
	{
	case EADP_TYPE_QC3P0_12V:
	case EADP_TYPE_QC3P0_20V:
	case EADP_TYPE_PD3P0_10W:
	case EADP_TYPE_PD3P0_20W:
	case EADP_TYPE_PD3P0_30W:
	case EADP_TYPE_PD3P0_50W:
		break;
	case EADP_TYPE_DCSRC_05V:
		pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
		pid_set_freq_limit(144000000 / 127772, 144000000 / 127772, 144000000 / 127772);
		pid_set_duty_limit(500, 350, 150);
		pid_set_phas_limit(0, 0, 0);
		break;
	case EADP_TYPE_POWERBANK_05V:
		pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
		pid_set_freq_limit(144000000 / 110500, 144000000 / 127772, 144000000 / 147000);
		pid_set_duty_limit(500, 350, 150);
		pid_set_phas_limit(0, 0, 0);
		break;
	case EADP_TYPE_QC2P0_09V:
	case EADP_TYPE_DCSRC_09V:
	case EADP_TYPE_PD2P0_09V:
	case EADP_TYPE_PD2P0_12V:
	case EADP_TYPE_POWERBANK_WIRELESS_ONLY:
	case EADP_TYPE_POWERBANK_09V:
	case EADP_TYPE_POWERBANK_PPS:
		pid_set_volt_limit(gd->adp.volt_max, gd->adp.volt_min, gd->adp.volt_min);
		pid_set_freq_limit(144000000 / 127772, 144000000 / 127772, 144000000 / 127772);
		pid_set_duty_limit(500, 350, 100);
		pid_set_phas_limit(0, 0, 0);
		break;
	case EADP_TYPE_DCSRC_12V:
		break;
	default:
		break;
	}

#ifdef _PRINT_PID_MSG
	wpc_printk("\r\n pid_lim [%d %d %d] [%d %d %d] [%d %d %d] [%d %d %d]",
	           gd->pid_limit.volt_lim_hi, gd->pid_limit.volt_lim_mi, gd->pid_limit.volt_lim_lo,
	           gd->pid_limit.perd_lim_hi, gd->pid_limit.perd_lim_mi, gd->pid_limit.perd_lim_lo,
	           gd->pid_limit.duty_lim_hi, gd->pid_limit.duty_lim_mi, gd->pid_limit.duty_lim_lo,
	           gd->pid_limit.phas_lim_hi, gd->pid_limit.phas_lim_mi, gd->pid_limit.phas_lim_lo);
#endif
}

void pid_cep_handler(int8_t cep)
{
	pid_ctrl_mode_sel(cep);

	//wpc_printk("\r\n ce=%d ctrl=%d\n",cep,m_pid_ctrl_mode);

	if (cep == 0)
		return;

	switch (m_pid_ctrl_mode)
	{
	case EPID_CTRL_MODE_VOLT:
		if (cep > 0)
		{
			switch (gd->adp.adp_type)
			{
			case EADP_TYPE_QC3P0_12V:
			case EADP_TYPE_QC3P0_20V:
				if (cep > 30)
					cep = 30;
				gd->pid_volt += 200 * (cep / 10 + 1);
				break;
			case EADP_TYPE_PD3P0_10W:
			case EADP_TYPE_PD3P0_20W:
			case EADP_TYPE_PD3P0_30W:
			case EADP_TYPE_PD3P0_50W:
				if (cep > 24)
					cep = 24;
				gd->pid_volt += 20 * ((cep >> 0) + 1);
				break;
			case EADP_TYPE_DCSRC_05V:
			case EADP_TYPE_QC2P0_09V:
			case EADP_TYPE_DCSRC_09V:
			case EADP_TYPE_DCSRC_12V:
			case EADP_TYPE_PD2P0_09V:
			case EADP_TYPE_PD2P0_12V:
			case EADP_TYPE_POWERBANK_WIRELESS_ONLY:
			case EADP_TYPE_POWERBANK_PPS:
				if (cep > 24)
					cep = 24;
				gd->pid_volt += 20 * ((cep >> 0) + 1);
				/*						if (gd->atl_test_ldstp_bpp_P60 == 1)
						{
							gd->pid_volt += 900;
						}*/
				break;
			default:
				break;
			}
			if (gd->pid_volt > gd->pid_limit.volt_lim_hi)
			{
				gd->pid_volt = gd->pid_limit.volt_lim_hi;
			}
		}
		else
		{
			switch (gd->adp.adp_type)
			{
			case EADP_TYPE_QC3P0_12V:
			case EADP_TYPE_QC3P0_20V:
				if (cep < -30)
					cep = -30;
				cep *= -1;
				gd->pid_volt -= 200 * (cep / 10 + 1);
				break;
			case EADP_TYPE_PD3P0_10W:
			case EADP_TYPE_PD3P0_20W:
			case EADP_TYPE_PD3P0_30W:
			case EADP_TYPE_PD3P0_50W:
				if (cep < -24)
					cep = -24;
				cep *= -1;
				gd->pid_volt -= 20 * ((cep >> 0) + 1);
				break;
			case EADP_TYPE_DCSRC_05V:
			case EADP_TYPE_QC2P0_09V:
			case EADP_TYPE_DCSRC_09V:
			case EADP_TYPE_DCSRC_12V:
			case EADP_TYPE_PD2P0_09V:
			case EADP_TYPE_PD2P0_12V:
			case EADP_TYPE_POWERBANK_WIRELESS_ONLY:
			case EADP_TYPE_POWERBANK_PPS:
				if (cep < -24)
					cep = -24;
				cep *= -1;
				gd->pid_volt -= 20 * ((cep >> 0) + 1);
				if (gd->atl_test_ldstp_epp_N60 == 1 || gd->atl_test_ldstp_bpp_N60 == 1)
				{
					gd->pid_volt -= 1000;
				}
				break;
			default:
				break;
			}
			if (gd->pid_volt < gd->pid_limit.volt_lim_lo)
			{
				gd->pid_volt = gd->pid_limit.volt_lim_lo;
			}
		}

		if (gd->atl_test_ldstp_epp_N60 == 1 || gd->atl_test_ldstp_bpp_N60 == 1)
		{
		}
		else if (gd->atl_test_ldstp_bpp_P60 == 1)
		{

			uint16_t temp = gd->pid_volt + 400;
			uint16_t i = gd->pid_volt;
			for (i = gd->pid_volt; i < temp; i += 80)
			{
				fml_adp_volt_set(i);
				delay_1us(5);
			}
			gd->pid_volt = i;
			wpc_printk("bpp_ldstp %d", gd->pid_volt);
			// power bank application,needs special process. for IOC.
			/*				uint16_t tmp_duty, tmp;
				tmp_duty = (20091 - gd->pid_volt) * 100 / 1263;
				if (tmp_duty > 900) tmp_duty = 900;
				if (tmp_duty <   1) tmp_duty =   1;

				tmp = BPWM8->PWM_CTRL.BITS.DUTY;

				wpc_printk("\r\n tmp_duty,tmp: %d %d", tmp_duty, BPWM8->PWM_CTRL.BITS.DUTY);

				if (tmp < tmp_duty)
				{
					for (int i=tmp; i<tmp_duty; i++)
					{
						hal_bpwm_update(BPWM8, BPWM8->PWM_CTRL.BITS.PERD + 1, i);
						delay_1us(10);
					}
				}
				else
				{
					for (int i=tmp; i>tmp_duty; i--)
					{
						hal_bpwm_update(BPWM8, BPWM8->PWM_CTRL.BITS.PERD + 1, i);
						delay_1us(10);
					}
				}*/
		}
		else
		{
			fml_adp_volt_set(gd->pid_volt);
		}
		break;
	case EPID_CTRL_MODE_FREQ:
		if (cep > 0)
		{
			if (cep > 30)
				cep = 30;
			gd->pid_perd += cep / 3 + 1;

			if (gd->pid_volt >= gd->pid_limit.volt_lim_hi)
			{
				if (gd->pid_perd > gd->pid_limit.perd_lim_hi)
				{
					gd->pid_perd = gd->pid_limit.perd_lim_hi;
				}
			}
			else
			{
				if (gd->pid_perd > gd->pid_limit.perd_lim_mi)
				{
					gd->pid_perd = gd->pid_limit.perd_lim_mi;
				}
			}
		}
		else
		{
			if (cep < -30)
				cep = -30;
			cep *= -1;
			gd->pid_perd -= cep / 3 + 1;

			if (gd->pid_perd < gd->pid_limit.perd_lim_lo)
			{
				gd->pid_perd = gd->pid_limit.perd_lim_lo;
			}
		}

		hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
		break;
	case EPID_CTRL_MODE_DUTY:
		if (cep > 0)
		{
			if (cep > 30)
				cep = 30;
			gd->pid_duty += cep / 2 + 1;
			if (gd->pid_duty > gd->pid_limit.duty_lim_hi)
			{
				gd->pid_duty = gd->pid_limit.duty_lim_hi;
			}
		}
		else
		{
			if (cep < -30)
				cep = -30;
			cep *= -1;
			gd->pid_duty -= cep / 2 + 1;
			if (gd->pid_volt <= gd->pid_limit.volt_lim_lo && gd->pid_perd <= gd->pid_limit.perd_lim_lo)
			{
				if (gd->pid_duty < gd->pid_limit.duty_lim_lo)
				{
					gd->pid_duty = gd->pid_limit.duty_lim_lo;
				}
			}
			else
			{
				if (gd->pid_duty < gd->pid_limit.duty_lim_mi)
				{
					gd->pid_duty = gd->pid_limit.duty_lim_mi;
				}
			}
		}

		//			if (gd->atl_test_ldstp_bpp_P60 == 1)
		//			{
		//				wpc_printk("\r\n xxxxxx-> %d %d", tmp_duty, gd->pid_duty);
		//				for (int i=tmp_duty; i<=gd->pid_duty; i++)
		//				{
		//					hal_epwm_pwm_update(EPWM1, gd->pid_perd, i, gd->pid_phas);
		//					delay_1us(50);
		//				}
		//			}
		//			else
		{
			hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
		}
		break;
	case EPID_CTRL_MODE_PHAS:
		if (cep > 0)
		{
			if (cep > 20)
				cep = 20;
			gd->pid_phas = (gd->pid_phas > (cep / 4 + 1)) ? gd->pid_phas - (cep / 4 + 1) : 0;
			if (gd->pid_phas < gd->pid_limit.phas_lim_lo)
			{
				gd->pid_phas = gd->pid_limit.phas_lim_lo;
			}
		}
		else
		{
			if (cep < -20)
				cep = -20;
			cep *= -1;
			gd->pid_phas += cep / 4 + 1;
			if (gd->pid_phas > gd->pid_limit.phas_lim_hi)
			{
				gd->pid_phas = gd->pid_limit.phas_lim_hi;
			}
		}
		hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
		break;
	default:
		break;
	}

	gd->atl_test_ldstp_epp_N60 = 0;
	gd->atl_test_ldstp_bpp_N60 = 0;
	gd->atl_test_ldstp_bpp_P60 = 0;

#ifdef _PRINT_PID_MSG
	wpc_printk(" #:[%02x] %5d %6d %3d %2d", (m_pid_ctrl_evnt << 4) | m_pid_ctrl_mode, gd->pid_volt, 144000000 / gd->pid_perd, gd->pid_duty, gd->pid_phas);
#endif
}

void PID_vDDMEventHandler(void) // l 147kk ; h 112k
{
	if (gd->pid_perd >= PID_PERD_LIM_L + 20) // > 979+20
	{
		gd->pid_perd -= 20;
		gd->pid_duty += 10;
	}
	else if (gd->pid_perd > PID_PERD_LIM_L) // > 979
	{
		gd->pid_perd = PID_PERD_LIM_L;
		gd->pid_duty -= 30;
	}
	else if (gd->pid_perd < PID_PERD_LIM_H) // < 1300
	{
		gd->pid_perd += 20;
		gd->pid_duty -= 20;
	}
	else
	{
		if (gd->pid_duty > 250)
		{
			gd->pid_perd = PID_PERD_LIM_H - 100; //PID_PERD_LIM_L;
			gd->pid_duty -= 50;
		}
		else
		{
			gd->pid_perd = PID_PERD_LIM_H - 100; //PID_PERD_LIM_L;
			gd->pid_duty += 40;
		}
	}

	gd->pid_perd = (gd->pid_perd > PID_PERD_LIM_M) ? PID_PERD_LIM_M : gd->pid_perd;
	gd->pid_perd = (gd->pid_perd < PID_PERD_LIM_L) ? PID_PERD_LIM_L : gd->pid_perd;
	gd->pid_duty = (gd->pid_duty > PID_DUTY_LIM_H) ? PID_DUTY_LIM_H : gd->pid_duty;
	gd->pid_duty = (gd->pid_duty < PID_DUTY_LIM_L) ? PID_DUTY_LIM_L : gd->pid_duty;

	hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);

#ifdef _PRINT_PID_MSG
	wpc_printk("\r\n &&&& %d %d %d", gd->pid_volt, gd->pid_perd, gd->pid_duty);
#endif
}

void pid_set_volt_limit(uint16_t hi, uint16_t mi, uint16_t lo)
{
	gd->pid_limit.volt_lim_hi = hi;
	gd->pid_limit.volt_lim_mi = mi;
	gd->pid_limit.volt_lim_lo = lo;
}

void pid_set_freq_limit(uint16_t hi, uint16_t mi, uint16_t lo)
{
	gd->pid_limit.perd_lim_hi = hi;
	gd->pid_limit.perd_lim_mi = mi;
	gd->pid_limit.perd_lim_lo = lo;
}

void pid_set_duty_limit(uint16_t hi, uint16_t mi, uint16_t lo)
{
	gd->pid_limit.duty_lim_hi = hi;
	gd->pid_limit.duty_lim_mi = mi;
	gd->pid_limit.duty_lim_lo = lo;
}

void pid_set_phas_limit(uint16_t hi, uint16_t mi, uint16_t lo)
{
	gd->pid_limit.phas_lim_hi = hi;
	gd->pid_limit.phas_lim_mi = mi;
	gd->pid_limit.phas_lim_lo = lo;
}

static void pid_ctrl_mode_sel(int8_t cep)
{
	do
	{
		if (cep > 0)
		{
			if (gd->pid_phas > gd->pid_limit.phas_lim_lo)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_PHAS;
				break;
			}
			if (gd->pid_duty < gd->pid_limit.duty_lim_hi)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_DUTY;
				break;
			}
			if (gd->pid_perd < gd->pid_limit.perd_lim_mi)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
				break;
			}
			if (gd->pid_volt < gd->pid_limit.volt_lim_hi)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_VOLT;
				break;
			}
			if (gd->pid_perd < gd->pid_limit.perd_lim_hi)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
				break;
			}
		}

		if (cep < 0)
		{
			m_u8PositiveCevSum = 0;
			if (gd->rx_infos.rx_type == EPRX_TYPE_SAMSUNG)
			{
				if (gd->pid_volt > gd->pid_limit.volt_lim_lo)
				{
					m_pid_ctrl_mode = EPID_CTRL_MODE_VOLT;
					break;
				}
				if (gd->pid_perd > gd->pid_limit.perd_lim_mi)
				{
					m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
					break;
				}
			}
			else
			{
				if (gd->pid_perd > gd->pid_limit.perd_lim_mi)
				{
					m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
					break;
				}
				if (gd->pid_volt > gd->pid_limit.volt_lim_lo)
				{
					m_pid_ctrl_mode = EPID_CTRL_MODE_VOLT;
					break;
				}
			}
			if (gd->pid_perd > gd->pid_limit.perd_lim_lo)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
				break;
			}
			if (gd->pid_duty > gd->pid_limit.duty_lim_lo)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_DUTY;
				break;
			}
			if (gd->pid_phas < gd->pid_limit.phas_lim_hi)
			{
				m_pid_ctrl_mode = EPID_CTRL_MODE_PHAS;
				break;
			}
		}
	} while (0);

	if ((m_pid_ctrl_evnt & EPID_EVENT_SAK) && (cep < 0))
	{
		if (gd->pid_perd > gd->pid_limit.perd_lim_lo)
		{
			m_pid_ctrl_mode = EPID_CTRL_MODE_FREQ;
		}
		else if (gd->pid_duty > gd->pid_limit.duty_lim_mi)
		{
			m_pid_ctrl_mode = EPID_CTRL_MODE_DUTY;
		}
	}

	if (gd->atl_test_ldstp_epp_N60 == 1 || gd->atl_test_ldstp_bpp_N60 == 1 || gd->atl_test_ldstp_bpp_P60 == 1)
	{
		if (gd->pid_volt > gd->pid_limit.volt_lim_lo)
		{
			m_pid_ctrl_mode = EPID_CTRL_MODE_VOLT;
		}
	}
	if (gd->atl_test_ldstp_bpp_P60 == 1)
	{
		m_pid_ctrl_mode = EPID_CTRL_MODE_VOLT;
	}

	//	if (gd->atl_test_ldstp_bpp_P60 == 1)
	//	{
	//		if (gd->pid_duty < gd->pid_limit.duty_lim_hi)
	//		{
	//			m_pid_ctrl_mode = EPID_CTRL_MODE_DUTY;
	//		}
	//	}
}

void PID_vCtrlAccuracyCheck(int8_t cep)
{
	//	if (gd->cep_prev * cep < 0)
	//	{
	//		if (++m_u8CtrlErrShakeCnt > 4)
	//		{
	//			m_u8CtrlErrShakeCnt = 4;
	//			m_pid_ctrl_evnt |= EPID_EVENT_SAK;
	//		}
	//	}
	//
	//	if (cep == 0)
	//	{
	//		m_pid_ctrl_evnt &= ~EPID_EVENT_SAK;
	//		m_u8CtrlErrShakeCnt = 0;
	//	}

	gd->rx_infos.cep_pre = cep;
}
