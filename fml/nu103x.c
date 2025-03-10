#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "nu103x.h"
#include "delay.h"
#include "_wpc.h"

void fml_nu103x_config(enum nu103x_cmd_t cmd)
{
	uint8_t comm_pulse = (uint8_t)cmd;

	GPC->O_EN.BITS.PIN2 = 0; GPC->DOUT.BITS.PIN2 = 0; GPC->I_EN.BITS.PIN2 = 1;
//	delay_1us(1);

	GPC->I_EN.BITS.PIN2 = 0; GPC->DOUT.BITS.PIN2 = 0; GPC->O_EN.BITS.PIN2 = 1;
//	delay_1us(1);

	for (int i=0; i<comm_pulse; i++)
	{
		GPC->DOUT.BITS.PIN2 = 1;
//		delay_1us(1);
		GPC->DOUT.BITS.PIN2 = 0;
//		delay_1us(1);
	}

	GPC->O_EN.BITS.PIN2 = 0; GPC->DOUT.BITS.PIN2 = 0; GPC->I_EN.BITS.PIN2 = 1;

	gd->nu103x_sts_last.WORD = gd->nu103x_sts_curr.WORD;

	switch (cmd)
	{
		case _1030_CFG_ALL_RST:                     gd->nu103x_sts_curr.WORD                   = 0;                                        break;
		case _1030_CFG_OCP_08A:                     gd->nu103x_sts_curr.BITS.OCP_THD           = _NU1030_OCP_THD_08A;                      break;
		case _1030_CFG_OCP_10A:                     gd->nu103x_sts_curr.BITS.OCP_THD           = _NU1030_OCP_THD_10A;                      break;
		case _1030_CFG_LPM_EN_:                     gd->nu103x_sts_curr.BITS.LPM_STS           = _NU1030_LPM_STS_EN_;                      break;
		case _1030_CFG_LPM_DIS:                     gd->nu103x_sts_curr.BITS.LPM_STS           = _NU1030_LPM_STS_DIS;                      break;
		case _1030_CFG_QDT_EN_:                                                                                                            break;
		case _1030_CFG_VDD_LDO_V4P8_ON_:            gd->nu103x_sts_curr.BITS.VDD_LDO_V4P8_STS  = _NU1030_VDD_LDO_V4P8_STS_ON_;             break;
		case _1030_CFG_VDD_LDO_V4P8_OFF:            gd->nu103x_sts_curr.BITS.VDD_LDO_V4P8_STS  = _NU1030_VDD_LDO_V4P8_STS_OFF;             break;
		case _1030_CFG_VDD_V5V_BUCK_DIS:            gd->nu103x_sts_curr.BITS.VDD_V5V_BUCK_STS  = _NU1030_VDD_V5V_BUCK_STS_OFF;             break;
		case _1030_CFG_VDD_V5V_BUCK_EN_:            gd->nu103x_sts_curr.BITS.VDD_V5V_BUCK_STS  = _NU1030_VDD_V5V_BUCK_STS_ON_;             break;
		case _1030_CFG_DRVH2_CONN_SW2:              gd->nu103x_sts_curr.BITS.DRVH2_CONN_STS    = _NU1030_DRVH2_CONN_STS_SW2;               break;
		case _1030_CFG_DRVH2_CONN_VIN:              gd->nu103x_sts_curr.BITS.DRVH2_CONN_STS    = _NU1030_DRVH2_CONN_STS_VIN;               break;
		case _1030_CFG_DRVH2_TURN_OFF:              gd->nu103x_sts_curr.BITS.DRVH2_TURN_STS    = _NU1030_DRVH2_TURN_STS_OFF;               break;
		case _1030_CFG_DRVH2_TURN_ON_:              gd->nu103x_sts_curr.BITS.DRVH2_TURN_STS    = _NU1030_DRVH2_TURN_STS_ON_;               break;
		case _1030_CFG_DRVH1_TURN_OFF:              gd->nu103x_sts_curr.BITS.DRVH1_TURN_STS    = _NU1030_DRVH1_TURN_STS_OFF;               break;
		case _1030_CFG_DRVH1_TURN_ON_:              gd->nu103x_sts_curr.BITS.DRVH1_TURN_STS    = _NU1030_DRVH1_TURN_STS_ON_;               break;
		case _1030_CFG_DRVH_SLEW_RATE_FAST:         gd->nu103x_sts_curr.BITS.DRVHx_SLEW_RATE   = _NU1030_DRVHx_SLEW_RATE_10ns;             break;
		case _1030_CFG_DRVH_SLEW_RATE_SLOW:         gd->nu103x_sts_curr.BITS.DRVHx_SLEW_RATE   = _NU1030_DRVHx_SLEW_RATE_40ns;             break;
		case _1030_CFG_DRVH_DEAD_TIME_AUTO:         gd->nu103x_sts_curr.BITS.DRVHx_DEAD_TIME   = _NU1030_DRVHx_DEAD_TIME_Self_Adaptive;    break;
		case _1030_CFG_DRVH_DEAD_TIME_FIXD:         gd->nu103x_sts_curr.BITS.DRVHx_DEAD_TIME   = _NU1030_DRVHx_DEAD_TIME_Fixed_to_10ns;    break;
		case _1030_CFG_VDM_PIN_VCAP_OUT:            gd->nu103x_sts_curr.BITS.VDM_PIN_MFP_STS   = _NU1030_VDM_PIN_MFC_STS_VCAP_OUT;         break;
		case _1030_CFG_VDM_PIN_EVDM_IN_:            gd->nu103x_sts_curr.BITS.VDM_PIN_MFP_STS   = _NU1030_VDM_PIN_MFC_STS_EVDM_IN_;         break;
		case _1030_CFG_DMO1_OUT_MODE_DDM:           gd->nu103x_sts_curr.BITS.DMO1_OUT_MODE     = _NU1030_DMO1_OUT_MODE_DDM;                break;
		case _1030_CFG_DMO1_OUT_MODE_QDT:           gd->nu103x_sts_curr.BITS.DMO1_OUT_MODE     = _NU1030_DMO1_OUT_MODE_QDT;                break;
		case _1030_CFG_DMO1_DDM_SRC_IAVG:           gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC      = _NU1030_DMO1_DDM_SRC_IAVG;                break;
		case _1030_CFG_DMO1_DDM_SRC_EVDM:           gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC      = _NU1030_DMO1_DDM_SRC_EVDM;                break;
		case _1030_CFG_DMO1_DDM_BPF_2ORD:           gd->nu103x_sts_curr.BITS.DMO1_DDM_BPF      = _NU1030_DMO1_DDM_BPF_2ord;                break;
		case _1030_CFG_DMO1_DDM_BPF_1ORD:           gd->nu103x_sts_curr.BITS.DMO1_DDM_BPF      = _NU1030_DMO1_DDM_BPF_1ord;                break;
		case _1030_CFG_DMO1_DDM_GAIN_MODE_AUTO:     gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_MOD = _NU1030_DMO1_DDM_GAIN_MODE_AUTO;          break;
		case _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD:     gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_MOD = _NU1030_DMO1_DDM_GAIN_MODE_FIXD;          break;
		case _1030_CFG_DMO1_DDM_FIXED_GAIN_X36:     gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_FIX = _NU1030_DMO1_DDM_GAIN_FIXED_X36;          break;
		case _1030_CFG_DMO1_DDM_FIXED_GAIN_X60:     gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_FIX = _NU1030_DMO1_DDM_GAIN_FIXED_X60;          break;
		case _1030_CFG_DMO2_OUT_MODE_DDM:           gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE     = _NU1030_DMO2_OUT_MODE_DDM;                break;
		case _1030_CFG_DMO2_OUT_MODE_QDT:           gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE     = _NU1030_DMO2_OUT_MODE_QDT;                break;
		case _1030_CFG_DMO2_OUT_MODE_CAP:           gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE     = _NU1030_DMO2_OUT_MODE_CAP;                break;
		case _1030_CFG_DMO2_VCAP_RATIO_K2:          gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K = _NU1030_DMO2_VCAP_RATIO_K2;               break;
		case _1030_CFG_DMO2_VCAP_RATIO_K3:          gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K = _NU1030_DMO2_VCAP_RATIO_K3;               break;
		case _1030_CFG_DMO2_VCAP_RATIO_K1:          gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K = _NU1030_DMO2_VCAP_RATIO_K1;               break;
		case _1030_CFG_DMO2_DDM_SRC_VCAP:           gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC      = _NU1030_DMO2_DDM_SRC_VCAP;                break;
		case _1030_CFG_DMO2_DDM_SRC_PHAS:           gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC      = _NU1030_DMO2_DDM_SRC_PHAS;                break;
		case _1030_CFG_DMO2_DDM_BPF_2ORD:           gd->nu103x_sts_curr.BITS.DMO2_DDM_BPF      = _NU1030_DMO2_DDM_BPF_2ord;                break;
		case _1030_CFG_DMO2_DDM_BPF_1ORD:           gd->nu103x_sts_curr.BITS.DMO2_DDM_BPF      = _NU1030_DMO2_DDM_BPF_1ord;                break;
		case _1030_CFG_DMO2_DDM_GAIN_MODE_AUTO:     gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_MOD = _NU1030_DMO2_DDM_GAIN_MODE_AUTO;          break;
		case _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD:     gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_MOD = _NU1030_DMO2_DDM_GAIN_MODE_FIXD;          break;
		case _1030_CFG_DMO2_DDM_FIXED_GAIN_X36:     gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_FIX = _NU1030_DMO2_DDM_GAIN_FIXED_X36;          break;
		case _1030_CFG_DMO2_DDM_FIXED_GAIN_X60:     gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_FIX = _NU1030_DMO2_DDM_GAIN_FIXED_X60;          break;
		case _1030_CFG_DMOx_DDM_CMP_HYST_12P5mV:    gd->nu103x_sts_curr.BITS.DMOx_DDM_CMP_HYST = _NU1030_DMOx_DDM_CMP_HYST_12P5mV;         break;
		case _1030_CFG_DMOx_DDM_CMP_HYST_30P0mV:    gd->nu103x_sts_curr.BITS.DMOx_DDM_CMP_HYST = _NU1030_DMOx_DDM_CMP_HYST_30P0mV;         break;
		default:
			break;
	}
}

void fml_nu103x_por_rst(void)
{
	//don't delete, need more test -- Sean
	hal_epwm_pwm_start(EPWM1, 144000/1800, 500, 180);
	delay_1us(1);
	hal_epwm_pwm_stop(EPWM1);

	fml_nu103x_config(_1030_CFG_ALL_RST);

	fml_nu103x_config(_1030_CFG_OCP_08A);
	if (SYS->PID_INFO.BITS.VER == CHIP_VER_A0)
	{
	fml_nu103x_config(_1030_CFG_VDD_LDO_V4P8_ON_);
	}
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_EN_);

	fml_nu103x_config(_1030_CFG_DRVH2_CONN_SW2);
	fml_nu103x_config(_1030_CFG_VDM_PIN_EVDM_IN_);
}

void fml_nu103x_por_init(void)
{
	/*
	 * After power up, before the switching of SW1 and SW2 can be controlled by PWM1 and PWM2,
	 * the PWM1 and PWM2 must be toggled LOW-HIGH-LOW to initiate the control logic circuit.
	 */
	hal_epwm_pwm_start(EPWM1, 144000/1800, 500, 180);
	delay_1us(1);
	hal_epwm_pwm_stop(EPWM1);

	fml_nu103x_config(_1030_CFG_ALL_RST);

	fml_nu103x_config(_1030_CFG_OCP_08A);
	if (SYS->PID_INFO.BITS.VER == CHIP_VER_A0)
	{
	fml_nu103x_config(_1030_CFG_VDD_LDO_V4P8_ON_);
	}
	fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_EN_);

	fml_nu103x_config(_1030_CFG_DRVH2_CONN_SW2);
	fml_nu103x_config(_1030_CFG_VDM_PIN_EVDM_IN_);

	fml_nu103x_config(_1030_CFG_DRVH1_TURN_OFF);
	fml_nu103x_config(_1030_CFG_DRVH2_TURN_OFF);
}

void fml_nu103x_ddm_init(void)
{
	// fml_nu103x_config(_1030_CFG_ALL_RST);
	// fml_nu103x_config(_1030_CFG_OCP_08A);
	// fml_nu103x_config(_1030_CFG_VDD_LDO_V4P8_ON_);
	// fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_EN_);

	// fml_nu103x_config(_1030_CFG_DRVH2_CONN_SW2);
	// fml_nu103x_config(_1030_CFG_VDM_PIN_EVDM_IN_);

	// fml_nu103x_config(_1030_CFG_DRVH1_TURN_OFF);
	// fml_nu103x_config(_1030_CFG_DRVH2_TURN_OFF);

	fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
	fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);

	fml_nu103x_config(_1030_CFG_DMO1_DDM_SRC_EVDM);
	fml_nu103x_config(_1030_CFG_DMO2_DDM_SRC_VCAP);
}

void fml_nu103x_qdt_init(void)
{
	// fml_nu103x_config(_1030_CFG_ALL_RST);
	// fml_nu103x_config(_1030_CFG_OCP_08A);
	// fml_nu103x_config(_1030_CFG_VDD_LDO_V4P8_ON_);
	// fml_nu103x_config(_1030_CFG_VDD_V5V_BUCK_EN_);

	// fml_nu103x_config(_1030_CFG_DRVH2_CONN_SW2);
	// fml_nu103x_config(_1030_CFG_VDM_PIN_EVDM_IN_);

	fml_nu103x_config(_1030_CFG_DRVH1_TURN_ON_);
	fml_nu103x_config(_1030_CFG_DRVH2_TURN_ON_);

	fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_QDT);
	fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_QDT);
	fml_nu103x_config(_1030_CFG_QDT_EN_);
}

void fml_nu103x_dmo1_param_set(enum nu103x_cmd_t ddm_src, enum nu103x_cmd_t gain_mod, enum nu103x_cmd_t gain_amp)
{
	fml_nu103x_config(ddm_src);
	fml_nu103x_config(gain_mod);
	fml_nu103x_config(gain_amp);
}

void fml_nu103x_dmo2_param_set(enum nu103x_cmd_t ddm_src, enum nu103x_cmd_t gain_mod, enum nu103x_cmd_t gain_amp, enum nu103x_cmd_t vcap_k_ratio)
{
	fml_nu103x_config(ddm_src);
	fml_nu103x_config(gain_mod);
	fml_nu103x_config(gain_amp);
	if (ddm_src == _1030_CFG_DMO2_DDM_SRC_VCAP)
	{
		fml_nu103x_config(vcap_k_ratio);
	}
}

void fml_nu103x_dmo1_ping_param_chose(void)
{

}

void fml_nu103x_dmo1_xfer_param_chose(void)
{
	 if (gd->ptx_protocol_phase != WPC_PHASE_XFER || gd->atl_test_tpr1c_coil_flag == 1)
	 	return;

	if (gd->dmo1_phase != _NU103x_DM_PHASE_DIG_PING)
	{
		if (gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC == _NU1030_DMO1_DDM_SRC_IAVG)
		{
			if (gd->dmo1_phase == _NU103x_DM_PHASE_LO_POWER)//<1500mW
			{
				fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_EVDM, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
			}
			else//>1500mW
			{
				fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_EVDM, _1030_CFG_DMO1_DDM_GAIN_MODE_AUTO, _1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
			}
		}
		else
		{
			if (gd->dmo1_phase == _NU103x_DM_PHASE_LO_POWER)//<1500mW
			{
				fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
			}
			else//>1500mW
			{
				fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X60);
			}
		}
		/*
			dmo1_phase     0->dig ping 1->low power 2->high power
			DMO1_DDM_SRC   0->iavg     1->vdm
			DDM_GAIN_MODE  0->auto     1->fixed
			DDM_GAIN_FIX   0->36       1->60
		*/
		printk(" <dmo1-%d-%d%d%d>", gd->dmo1_phase, gd->nu103x_sts_curr.BITS.DMO1_DDM_SRC, gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_MOD, gd->nu103x_sts_curr.BITS.DMO1_DDM_GAIN_FIX);
	}
}

void fml_nu103x_dmo2_ping_param_chose(void)
{

}

void fml_nu103x_dmo2_xfer_param_chose(void)
{
	 if (gd->ptx_protocol_phase != WPC_PHASE_XFER || gd->atl_test_tpr1c_coil_flag == 1)
	 	return;

	if (gd->dmo2_phase != _NU103x_DM_PHASE_DIG_PING)
	{
		if (gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC == _NU1030_DMO2_DDM_SRC_VCAP)
		{
			if (gd->dmo1_phase == _NU103x_DM_PHASE_LO_POWER)
			{
				fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K2);
			}
			else
			{
				fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K3);
			}
		}
		else
		{
			if (gd->dmo1_phase == _NU103x_DM_PHASE_LO_POWER)
			{
				fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K2);
			}
			else
			{
				fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_VCAP, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X60, _1030_CFG_DMO2_VCAP_RATIO_K3);
			}
		}
		/*
			dmo2_phase     0->dig ping 1->low power 2->high power
			DMO2_DDM_SRC   0->vcap     1->phase
			DDM_GAIN_MODE  0->auto     1->fixed
			DDM_GAIN_FIX   0->36       1->60
			VCAP_RATIO_K   0->k2       1->k3        2->k1
		*/
		printk(" <dmo2-%d-%d%d%d%d>", gd->dmo2_phase, gd->nu103x_sts_curr.BITS.DMO2_DDM_SRC,
			gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_MOD, gd->nu103x_sts_curr.BITS.DMO2_DDM_GAIN_FIX, gd->nu103x_sts_curr.BITS.DMO2_VCAP_RATIO_K);
	}
//	GPA->DOUT.BITS.PIN4 ^= 1;
}

void fml_nu103x_ddm_param_change(void)
{
}
