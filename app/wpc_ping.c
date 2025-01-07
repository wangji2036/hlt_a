#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "osal.h"
#include "delay.h"
#include "qdt.h"
#include "pid.h"
#include "_wpc.h"
#include "pkt_type.h"
#include "wpc_ping.h"
#include "wpc_xfer.h"
#include "wpc_idle.h"
#include "pfod.h"
#include "tcpm.h"

void wpc_ping_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	idle_qfod_init();

	if (com_ask->hdr == WPC_PRx_PKT_TYP_SIG_01)
	{
		osal_set_event(USB_TASK,TCPM_EVT_QI_WORK);
		gd->rx_infos.ssp_value = com_ask->msg.sig.ss_value;
		gd->ptx_protocol_phase = WPC_PHASE_CNFG;
		osal_start_timerEx(WPC_NEXT_TIMER, T_NEXT + 50, 0, WPC_TASK, WPC_EVT_CNFG_NEXT_1ST_TO);//TODO:temp change for bad ddm
		gd->rx_infos.opt_cnt = 0;
		gd->rx_infos.power_profile_mode = BPP;
		gd->rx_infos.mpp_restricted_mode = 0;
		gd->tx_infos.rx_status = 1;

		gd->nego_flag = 0;

		auth_init();
		pfod_init();
		idle_qfod_init();

		gd->alt_test_resv_rp8_cnt = 0;

		/*+++++++++++++++++++++ ATL TPR#1C 6.2.09 Test#23 workaround +++++++++++++++++++++*/
		if (gd->rx_infos.ssp_value < 200)
		{
			if (gd->pid_perd == PLL_CLK / 144000)
			{
				wpc_stop_to_idle(ESYS_ERR_CODE_DIGITAL_REPING);
			}
		}
		else
		{
			if (gd->pid_perd != PLL_CLK / 360000)
			{
				gd->pid_perd = 144000/144;
				hal_epwm_pwm_update(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);

				fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_DDM);
				fml_nu103x_dmo1_param_set(_1030_CFG_DMO1_DDM_SRC_IAVG, _1030_CFG_DMO1_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO1_DDM_FIXED_GAIN_X36);
				gd->dmo1_phase = _NU103x_DM_PHASE_DIG_PING;

				fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);
				fml_nu103x_dmo2_param_set(_1030_CFG_DMO2_DDM_SRC_PHAS, _1030_CFG_DMO2_DDM_GAIN_MODE_FIXD, _1030_CFG_DMO2_DDM_FIXED_GAIN_X36, _1030_CFG_DMO2_VCAP_RATIO_K1);
				gd->dmo2_phase = _NU103x_DM_PHASE_DIG_PING;
			}
		}
		/*--------------------- ATL TPR#1C 6.2.09 Test#23 workaround ---------------------*/
	}
	else
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_PING_PHASE_1ST_PKT_TYPE_ERR);
	}
}
