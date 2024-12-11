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

		auth_init();
		pfod_init();
		idle_qfod_init();
	}
	else
	{
		gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
		wpc_stop_to_idle(ESYS_ERR_CODE_PING_PHASE_1ST_PKT_TYPE_ERR);
	}
}
