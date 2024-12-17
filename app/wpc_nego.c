#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "fsk.h"
#include "pkt_type.h"
#include "wpc_xfer.h"
#include "epp.h"
#include "pfod.h"
#include "debug.h"

//__attribute__((weak)) void wpc_epp_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
//{
//
//}

enum mpp_prx_get_request_type_t
{
	GET_PTx_XID  = 0,
	GET_PTx_INV  = 2,
	GET_PTx_PLAP = 3,
	GET_PTx_ECAP = 4,
	GET_PTx_RCS  = 5,
	GET_PTx_CHS  = 6,
	GET_PTx_KEST = 7,
	GET_PTx_ERR  = 9,
	GET_POWER_MODES 		= 10,
	GET_MODE_XCAP			= 11,
	GET_MATED_Q_RESULT		= 12,
	GET_PTx_PLAP2 			= 13,
	GET_Reserved_14 		= 14,
	GET_Reserved_15 		= 15,//G3
	GET_PTx_CAL_CAP 		= 16,

};

enum mpp_prx_srq_request_type_t
{
	SRQ_END_00                = 0x00,
	SRQ_RP_04                 = 0x04,
	SRQ_REP_05                = 0x05,
	SRQ_FREQ_SEL_F0           = 0xF0,
	SRQ_EPGL_F3               = 0xF3,
	SRQ_EPGH_F4               = 0xF4,
	SRQ_CLOAK_DIG_PING_LSB_F5 = 0xF5,
	SRQ_PCP_F6                = 0xF6,
	SRQ_CLOAK_DIG_PING_MSB_F7 = 0xF7,
	SRQ_CLOAK_DET_PING_F8     = 0xF8,
	SRQ_PLA_SEL_A0            = 0xA0,
};

//static uint8_t mpp_tx_cap, mpp_tx_cur_cap, mpp_tx_is_nego_cap = 0;


static uint8_t is_neg_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x01 || hdr == 0x05 || hdr == 0x06 || hdr == 0x09 || hdr == 0x15 || hdr == 0x48 ||
    	hdr == 0x38 || hdr == 0x26 || hdr == 0x51 || hdr == 0x71 || hdr == 0x81 || hdr == 0x19 ||
    	hdr == 0x58 || hdr == 0x03)
    {
    	return 1;
    }
	return 0;
}

void mpp_srq_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	switch (mpp_ask->msg.srq.request)
	{
		case SRQ_END_00:
			//TODO: if PRx Not received ACK successfully, and retry 20 00 packet, need to consider this situation. --Sean
			osal_stop_timerEx(WPC_NEXT_TIMER);
			osal_stop_timerEx(WPC_NEGO_TIMER);
			osal_start_timerEx(WPC_CEP_TIMER, T_MPP_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
			osal_start_timerEx(WPC_RPP_TIMER, T_MPP_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
			gd->ptx_protocol_phase = WPC_PHASE_XFER;
			gd->ptx_end_nego_event |= MPP_END_NEGO_FLAG;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);

			if (gd->rx_infos.mpp_restricted_mode == 1)
			{
				gd->rx_infos.mpp_restricted_mode = 0;
			}

			if (gd->pid_perd == 1127)//128K
			{
				gd->tx_infos._128_nego_gd |= 2;
			}
			else
			{
				gd->tx_infos._128_nego_gd = 0;
#if DIG_DDM_ENABLE
				if (144000 / (EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 360 && gd->nu103x_sts_curr.BITS.DMO2_OUT_MODE == _NU1030_DMO2_OUT_MODE_CAP)
				{
//					hal_ecap_dig_ddm_init();
//					hal_eadc_ddm_init();
					hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_DDM);
					hal_ecap_open(ECAP2);
					hal_ecap_close(ECAP4);
					fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_DDM);

					printk("\r\n ----- disable digital ddm");
				}
#endif
			}
			break;
		case SRQ_REP_05:
			gd->tx_infos.t_re_ping = (mpp_ask->msg.srq.parameter & 0x3F) * 200;
//			gd->tx_infos.dig_ping_type = _360K_FB;//TODO: I think it's not appropriate to set dig_ping_type here
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_FREQ_SEL_F0:
			if ((mpp_ask->msg.srq.parameter & 0x3) == 1)
			{
				gd->tx_infos.dig_ping_type = _360K_FB;
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
				printk("\r\n 360k-2");
			}
			else
			{
				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			}
			break;
		case SRQ_RP_04:
		case SRQ_EPGL_F3:
		case SRQ_EPGH_F4://TODO: compare load power value
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_CLOAK_DIG_PING_LSB_F5:
			gd->tx_infos.cloak_dig_ping_delay &= ~0x00FF;
			gd->tx_infos.cloak_dig_ping_delay |= mpp_ask->msg.srq.parameter;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_PCP_F6:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_CLOAK_DIG_PING_MSB_F7:
			gd->tx_infos.cloak_dig_ping_delay &= ~0x0300;
			gd->tx_infos.cloak_dig_ping_delay |= (mpp_ask->msg.srq.parameter & 0x03) << 8;
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_CLOAK_DET_PING_F8:
			gd->tx_infos.cloak_det_ping_delay = mpp_ask->msg.srq.parameter;//unit: 100mS_Cloak_Ping
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case SRQ_PLA_SEL_A0:
			printk(" [SRQ_PLA_ID %d]", mpp_ask->msg.srq.parameter & 0x03);
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		default:
			break;
	}

//	osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

void mpp_get_pkt_process(struct mpp_prx_ask_pkt_t *mpp_ask)
{
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };

//	for (int i=0; i<sizeof(fsk_pkt); i++)
//	{
//		if (fsk_pkt.mpp_fsk.data[i] != 0)
//		{
//			printk("\r\n fsk ram data init err ...");
//			while (1);
//		}
//	}

	switch (mpp_ask->msg.get.param)
	{
		case GET_PTx_XID:
			fsk_pkt.mpp_fsk.xid.hdr_8F = MPP_PTx_PKT_TYP_XID_8F;
			fsk_pkt.mpp_fsk.xid.selector = 0;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_msb = 0x73;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_mid = 0x2F;
			fsk_pkt.mpp_fsk.xid.ptx_bdid_lsb = 0x01;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_msb = 0x00;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_mid = 0x01;
			fsk_pkt.mpp_fsk.xid.mfg_rsvd_lsb = 0x11;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
//		{
//			struct ptx_fsk_pkt_t fsk = { };
//			fsk.pkt.mpp_fsk.xid.hdr_8F = MPP_PTx_PKT_TYP_XID_8F;
//			fsk.pkt.mpp_fsk.xid.selector = 0;
//			fsk.pkt.mpp_fsk.xid.ptx_bdid_msb = 0x73;
//			fsk.pkt.mpp_fsk.xid.ptx_bdid_mid = 0x2F;
//			fsk.pkt.mpp_fsk.xid.ptx_bdid_lsb = 0x01;
//			fsk.pkt.mpp_fsk.xid.mfg_rsvd_msb = 0x00;
//			fsk.pkt.mpp_fsk.xid.mfg_rsvd_mid = 0x01;
//			fsk.pkt.mpp_fsk.xid.mfg_rsvd_lsb = 0x11;
//			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk.pkt.data[0], wpc_msg_size_get(fsk.pkt.data[0]) + 1);
//			break;
//		}
		case GET_PTx_INV:
			fsk_pkt.mpp_fsk.inv.hdr_3F = MPP_PTx_PKT_TYP_INV_3F;
			fsk_pkt.mpp_fsk.inv.selector = 0x00;
			fsk_pkt.mpp_fsk.inv.vinv_msb = (gd->vpwr >> 9) & 0x3F;
			fsk_pkt.mpp_fsk.inv.vinv_lsb = (gd->vpwr >> 1) & 0xFF;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_PLAP:
			fsk_pkt.mpp_fsk.plap.hdr_5F = MPP_PTx_PKT_TYP_PLAP_5F;
			fsk_pkt.mpp_fsk.plap.g_coil_rx_msb = 0x27;
			fsk_pkt.mpp_fsk.plap.g_coil_rx_lsb = 0x10;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_ECAP:
//			fsk_pkt.mpp_fsk.ecap.cal_support = 1;
			fsk_pkt.mpp_fsk.ecap.cal_support = 0;
			if (gd->tx_infos.fo_exist)
			{
				gd->tx_infos.tar_cap_fod = 250;
				gd->tx_infos.nego_cap = 150;
				gd->tx_infos.power_limit_reason = 2;
			}
			else if (gd->dploss_cal.success == 1)
			{
				gd->tx_infos.tar_cap_cali = 250;
				gd->tx_infos.nego_cap = 250;
				gd->tx_infos.power_limit_reason = 0;
			}
			else
			{
				gd->tx_infos.tar_cap_cali = 150;
				gd->tx_infos.nego_cap = 150;
				gd->tx_infos.power_limit_reason = 10;//calibration limit
			}

			fsk_pkt.mpp_fsk.ecap.hdr_8F = MPP_PTx_PKT_TYP_ECAP_8F;
			fsk_pkt.mpp_fsk.ecap.selector = 0x01;
			fsk_pkt.mpp_fsk.ecap.potential_power_bit0_1 = (gd->tx_infos.max_cap>>8)&0b11;
			fsk_pkt.mpp_fsk.ecap.potential_power_bit2_9 = gd->tx_infos.max_cap&0xFF;
			fsk_pkt.mpp_fsk.ecap.nego_power_bit0_1 = (gd->tx_infos.nego_cap>>8)&0b11;
			fsk_pkt.mpp_fsk.ecap.nego_power_bit2_9 = gd->tx_infos.nego_cap&0xFF;
			fsk_pkt.mpp_fsk.ecap.pow_limit_reason = gd->tx_infos.power_limit_reason;
			fsk_pkt.mpp_fsk.ecap.concurrent_data_stream = 2;
			fsk_pkt.mpp_fsk.ecap.buffer_size = 3;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_RCS:
			fsk_pkt.mpp_fsk.rcs.hdr_1E = MPP_PTx_PKT_TYP_RCS_1E;
			fsk_pkt.mpp_fsk.rcs.selector = 0x03;
			fsk_pkt.mpp_fsk.rcs.status = 0x00; //normal operation
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_CHS:
			fsk_pkt.mpp_fsk.chs.hdr_1F = MPP_PTx_PKT_TYP_CHS_1F;
			fsk_pkt.mpp_fsk.chs.chs_value = 0xFF;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_KEST:
			fsk_pkt.mpp_fsk.kest.hdr_3F = MPP_PTx_PKT_TYP_KEST_3F;
			fsk_pkt.mpp_fsk.kest.selector = 0x02;
			fsk_pkt.mpp_fsk.kest.kest_msb = ((gd->k_est * 4095 / 10000) >> 8) & 0x0F; //need k
			fsk_pkt.mpp_fsk.kest.kest_lsb = ((gd->k_est * 4095 / 10000) >> 0) & 0xFF; //need k
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_ERR:
			fsk_pkt.mpp_fsk.err.hdr_01 = MPP_PTx_PKT_TYP_ERR_01;
			fsk_pkt.mpp_fsk.err.error = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_POWER_MODES:
			printk(" [GET_MODE]");
			fsk_pkt.mpp_fsk.mode_info.hdr_0x5A = 0x5A;//mode info
			fsk_pkt.mpp_fsk.mode_info.active_aux = 0;//active aux mode ID is 0:default 1:Auxiliary mode

			fsk_pkt.mpp_fsk.mode_info.active_main_mode = gd->power_mode;
#if 0
			if (gd->power_mode == high)
				fsk_pkt.mpp_fsk.mode_info.active_main_mode = 3;//active main mode ID: 1 nominal, 2 light load, 3 high power
			else
				fsk_pkt.mpp_fsk.mode_info.active_main_mode = 1;//NOK9 power mode trans re-ping
#endif
			printk(" active mode: %d", gd->power_mode);

#if (MPP_25W_POWER_MODE_CPM_ENABLE || (!MPP_25W_POWER_MODE_TRANS_W_EPTR && !MPP_25W_POWER_MODE_TRANS_W_EPTR))
			fsk_pkt.mpp_fsk.mode_info.cpm = 1;
			fsk_pkt.mpp_fsk.mode_info.cpm_aux = 1;

			fsk_pkt.mpp_fsk.mode_info.npm = 0;//1/ /nominal mode is support
			fsk_pkt.mpp_fsk.mode_info.npm_aux = 0;//nominal GainM (auxiliary mode) is support
			fsk_pkt.mpp_fsk.mode_info.llpm = 0;//1 //light mode is support
			fsk_pkt.mpp_fsk.mode_info.hpm = 0;//0 //high power mode is support
			fsk_pkt.mpp_fsk.mode_info.hpm_aux = 0;//high power GianM is support
#else
			fsk_pkt.mpp_fsk.mode_info.cpm = 0;
			fsk_pkt.mpp_fsk.mode_info.cpm_aux = 0;

			fsk_pkt.mpp_fsk.mode_info.npm = 1;//nominal mode is support
			fsk_pkt.mpp_fsk.mode_info.npm_aux = 0;//nominal GainM (auxiliary mode) is support
			fsk_pkt.mpp_fsk.mode_info.llpm = 1;//light mode is support
			fsk_pkt.mpp_fsk.mode_info.hpm = 1;//high power mode is support
			fsk_pkt.mpp_fsk.mode_info.hpm_aux = 0;//high power GianM is support
#endif
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_MATED_Q_RESULT:
			printk(" [GET_MATE_Q]");
			fsk_pkt.mpp_fsk.data[0] = 0x40;
			if (gd->tx_infos.fo_exist)
				fsk_pkt.mpp_fsk.data[1] = 0x02;
			else
				fsk_pkt.mpp_fsk.data[1] = 0x01;

			fsk_pkt.mpp_fsk.data[2] = 0x00;
			fsk_pkt.mpp_fsk.data[3] = 0x00;
			fsk_pkt.mpp_fsk.data[4] = 0x00;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_PLAP2://TODO: packtet structure and params need re-write
			printk(" [GET_PLA2]");
			fsk_pkt.mpp_fsk.plap2.hdr_0x88 = 0x88;
			//fsk_pkt.mpp_fsk.plap2.reserved0 = 00;
			fsk_pkt.mpp_fsk.plap2.g_coil_rx2 = 0x204E;// range [0,2] resolution 5e-5, 200000/200000=1,
			//fsk_pkt.mpp_fsk.plap2.reserved1 = 0;
			fsk_pkt.mpp_fsk.plap2.g_alfa_fm_irect_bit16 = 0;
			fsk_pkt.mpp_fsk.plap2.g_alfa_fm_irect_bit0_15 = 0x50C3;//range [0,10] resolution 1e-4, 50000/10000=5
			//fsk_pkt.mpp_fsk.plap2.reserved2 = 0;
			fsk_pkt.mpp_fsk.plap2.g_alfa_fm_vrect_bit16 = 0;
			fsk_pkt.mpp_fsk.plap2.g_alfa_fm_vrect_bit0_15 = 0x50C3;//range[0,10] resolution 1e-4, 50000/10000=5
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_PTx_CAL_CAP:
			printk(" [GET_CALI_CAP]");
			fsk_pkt.mpp_fsk.data[0] = 0x43;

			if (gd->dploss_cal.index_cnt == 0)
				fsk_pkt.mpp_fsk.data[1] = 0x00;//b2_b0: no calibration data
			else //TODO:
				fsk_pkt.mpp_fsk.data[1] = 0x04;//b2_b0: success full power unlock

			fsk_pkt.mpp_fsk.data[2] = 0x01;//b0: CAL_M0 support
			fsk_pkt.mpp_fsk.data[3] = 0x00;//reserved
			fsk_pkt.mpp_fsk.data[4] = 0x00;//reserved
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case GET_MODE_XCAP:
			break;
		default:
			break;
	}

//	osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

/* using this func if the mpp_mate_q_detect() is in-effective */
//static void temp_mate_q_detect(void)
//{
//	if (gd->tx_infos.q_fact < 100)
//		gd->tx_infos.fo_exist = 1;
//}

void mpp_mate_q_detect(void)
{
	const uint8_t mate_q_enalbe = MPP_25W_MATE_Q_ENABLE;
	const uint8_t mate_q_thd = 0;//large mate_q_thd to loose mate_q

	int32_t temp_a0, temp_a1, temp_a2;
	int16_t mate_q_result;

	/* 68nF: a0=-0.2698, a1=0.9041, a2=-1, 515nF: a0=0.35, a1=0.361, a2=-1
	   g0 expect to 0, g1 expect to 1, g2 expect to 1

	   result = (g0+a0) + (g1*a1)*F_def + (g2*a2)*Q_def  --> g0 unit 0.02, a0/a1/a2/g1/g2 unit 0.01
	   		  = (g0*2+a0) + (g1*a1)*f/f_base/100 + (g2*a2)*q/q_base/100

	   result is expect < 0
	*/
#if 1//515nF a_n params
	gd->tx_infos.mate_q_a0 = 35;
	gd->tx_infos.mate_q_a1 = 36;
	gd->tx_infos.mate_q_a2 = 100;//a2 negtive
#elif 0//68nF a_n params
	gd->tx_infos.mate_q_a0 = -27;
	gd->tx_infos.mate_q_a1 = 90;
	gd->tx_infos.mate_q_a2 = 100;//a2 negtive
#endif

//	printk(" mate_q a0:%d a1:%d a2:%d", gd->tx_infos.mate_q_a0, gd->tx_infos.mate_q_a1, gd->tx_infos.mate_q_a2);

	temp_a0 = (gd->tx_infos.mate_q_g0<<1) + gd->tx_infos.mate_q_a0; //g0+a0
	temp_a1 = (gd->tx_infos.mate_q_g1 * gd->tx_infos.mate_q_a1 * gd->tx_infos.f_self)/ap->fs_base_value;//g1*a1*f/f_base
	temp_a1 /= 100;
	temp_a2 = gd->tx_infos.mate_q_g2 * gd->tx_infos.q_fact/ap->q_factor_base_value;//g2*a2*q/q_base

	mate_q_result = temp_a0 + temp_a1 - temp_a2 ;

	if (mate_q_enalbe && (mate_q_result > mate_q_thd)) {
		gd->tx_infos.fo_exist = 1;
	} else {
		gd->tx_infos.fo_exist = 0;
	}
	printk(" --->mate_q res:%d=%d+%d-%d", mate_q_result, temp_a0, temp_a1, temp_a2);
}

void wpc_mpp_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	struct mpp_prx_ask_pkt_t *mpp_ask = (struct mpp_prx_ask_pkt_t *)com_ask;
	struct mpp_ptx_fsk_pkt_t fsk_pkt = { };
	struct com_ptx_fsk_pkt_t _fsk = { };

	switch (com_ask->hdr)//TODO: need add pkt 0x13, 0x18, 0xA8, 0x90, maybe 0x19/0x58/0x88
	{
		case 0x07:
			if (mpp_ask->msg.data[0] == 0x30)
			{
				_fsk.com_fsk.id.hdr_30 = 0x30;
				_fsk.com_fsk.id.qi_version = 0x22;//0x20
				_fsk.com_fsk.id.ptmc_msb = 0x00;
				_fsk.com_fsk.id.ptmc_lsb = 0x5C;
			}
			else if (mpp_ask->msg.data[0] == 0x31)
			{
				_fsk.com_fsk.cap.hdr_31 = 0x31;
				_fsk.com_fsk.cap.neg_power = _fsk.com_fsk.cap.pot_power = 50;//25W
				_fsk.com_fsk.cap.wpid = 0;
				_fsk.com_fsk.cap.buff_size = 7;
			}
			else
			{
				_fsk.com_fsk.null.hdr_00 = 0x00;
				_fsk.com_fsk.null.invalid_data = 0x00;
			}
			fml_fsk_data_send(EPWM1, T_RESPONSE, &_fsk.com_fsk.data[0], wpc_msg_size_get(_fsk.com_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_SRQ_20:
			mpp_srq_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_GET_28:
			mpp_get_pkt_process(mpp_ask);
			break;
		case MPP_PRx_PKT_TYP_EDS_29:
			fsk_pkt.mpp_fsk.eds.hdr_2F = MPP_PTx_PKT_TYP_EDS_2F;
			fsk_pkt.mpp_fsk.eds.streams_bitmask_msb = 0x00;
			fsk_pkt.mpp_fsk.eds.streams_bitmask_lsb = 0x02;
			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_PLAP_78:
			gd->rx_infos.gcoil_tx = (int16_t)(((mpp_ask->msg.plap.g_coil_tx.msb<<8) + mpp_ask->msg.plap.g_coil_tx.lsb)/10000);
			gd->rx_infos.alpha_fm = (int16_t)((mpp_ask->msg.plap.alpha_fm.msb<<8) + mpp_ask->msg.plap.alpha_fm.lsb);//unit 0.5mW
			gd->rx_infos.alpha_fm_dc = (int16_t)((mpp_ask->msg.plap.alpha_fm_dc.msb<<8) + mpp_ask->msg.plap.alpha_fm_dc.lsb);//unit 0.5mW

			printk(" gcoil_tx:%d fm:%d fm_dc:%d", gd->rx_infos.gcoil_tx, gd->rx_infos.alpha_fm, gd->rx_infos.alpha_fm_dc);

			if ((gd->rx_infos.alpha_fm == 112 && gd->rx_infos.alpha_fm_dc == 419) || (gd->rx_infos.alpha_fm == 67 && gd->rx_infos.alpha_fm_dc == 237))//@: 78 00 00 70 01 A3 27 10 9D, SGS NOK9 gcoil_tx:1 fm:56 112 fm_dc:209 419
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_SGS;
			else if ((gd->rx_infos.alpha_fm == 93 && gd->rx_infos.alpha_fm_dc == 309) || (gd->rx_infos.alpha_fm == 74 && gd->rx_infos.alpha_fm_dc == 247))// @: 78 00 00 5D 01 35 27 10 26, Hongk NOK9 gcoil_tx:1 fm:46 93 fm_dc:154 309  C: @: 78 00 00 4A 00 F7 27 10 F2
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_HONK;
			else if ((gd->rx_infos.alpha_fm == 82 && gd->rx_infos.alpha_fm_dc == 248) || (gd->rx_infos.alpha_fm == 66 && gd->rx_infos.alpha_fm_dc == 198))//@: 78 00 00 52 00 F8 27 10 E5, WeiCe NOK9  gcoil_tx:1 fm:41 82 fm_dc:124 248
				gd->rx_infos.rx_type = ERX_TYPE_NOK9_MPP_MICRO;
			else if ((gd->rx_infos.alpha_fm == 98 && gd->rx_infos.alpha_fm_dc == 191) || (gd->rx_infos.alpha_fm == 76 && gd->rx_infos.alpha_fm_dc == 152))// @: 78 00 00 62 00 BF 27 10 92, GRL Hongbiao gcoil_tx:1 fm:49 98 fm_dc:95 191
				gd->rx_infos.rx_type = ERX_TYPE_GRL_MPP;
			//pfod_mpla_init();

				fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_ECAP_84:
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_PROP_1A:
	    case MPP_PRx_PKT_TYP_PROP_1B:
	    case MPP_PRx_PKT_TYP_PROP_2A:
	    case MPP_PRx_PKT_TYP_PROP_2B:
	    case MPP_PRx_PKT_TYP_PROP_39:
	    case MPP_PRx_PKT_TYP_PROP_49:
	    case MPP_PRx_PKT_TYP_PROP_59:
	    	fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_N_D);
			break;
		case MPP_PRx_PKT_TYP_MSR_13:
			gd->power_mode = mpp_ask->msg.msr.main_mode; //high;
			printk(" [MSR select:%d aux:%d]", mpp_ask->msg.msr.main_mode, mpp_ask->msg.msr.aux);

			fsk_pkt.mpp_fsk.mss.hdr_0x23 = MPP_PTx_PKT_TYP_MSS_23;
			fsk_pkt.mpp_fsk.mss.error_code = 0;
			if (144000 / (EPWM1->PWM_PERD.BITS.PWM_PERD + 1) == 360) {
				fsk_pkt.mpp_fsk.mss.status = 0;
			} else {
				if (gd->power_mode == high)
					fsk_pkt.mpp_fsk.mss.status = 1;//pending
				else
					fsk_pkt.mpp_fsk.mss.status = 0;//success	
			}
			printk(" MSS pending:%d", fsk_pkt.mpp_fsk.mss.status);

			fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.mpp_fsk.data[0], wpc_msg_size_get(fsk_pkt.mpp_fsk.data[0]) + 1);
			break;
		case MPP_PRx_PKT_TYP_PLAP2_90:
			gd->rx_infos.gcoil_tx2 = (int16_t)(((mpp_ask->msg.plap2.g_coil_tx2.msb<<8) + mpp_ask->msg.plap2.g_coil_tx2.lsb)/50000);//unit 5e-5, range [0,2]
			gd->rx_infos.alpha_fm_itx = (int16_t)(((mpp_ask->msg.plap2.alpha_fm_itx.msb<<8) + mpp_ask->msg.plap2.alpha_fm_itx.lsb));//unit 1e-4, range [-2,2]
			gd->rx_infos.alpha_fm_irect = (int16_t)(((mpp_ask->msg.plap2.alpha_fm_irect.msb<<8) + mpp_ask->msg.plap2.alpha_fm_irect.lsb));//unit 1e-4, range [-2,2]
			gd->rx_infos.alpha_fm_vrect = (int16_t)(((mpp_ask->msg.plap2.alpha_fm_vrect.msb_h<<16) + (mpp_ask->msg.plap2.alpha_fm_vrect.msb_l<<8) + mpp_ask->msg.plap2.alpha_fm_vrect.lsb));//unit 1e-6, range [-0.2,0.2]
			printk(" PLAP2 gcoil_tx2:%d fm_itx:%d fm_irect:%d fm_vrect:%d", gd->rx_infos.gcoil_tx2, gd->rx_infos.alpha_fm_itx, gd->rx_infos.alpha_fm_irect, gd->rx_infos.alpha_fm_vrect);
			
			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		case MPP_PRx_PKT_TYP_MATE_Q_A8://Mated Q coefficients
			gd->tx_infos.mate_q_g0 = (int16_t)((mpp_ask->msg.mate_q.g0.msb << 8) + mpp_ask->msg.mate_q.g0.lsb);//unit 0.02
			gd->tx_infos.mate_q_g1 = (int16_t)(mpp_ask->msg.mate_q.g1.msb << 8) + mpp_ask->msg.mate_q.g1.lsb;//unit 0.01
			gd->tx_infos.mate_q_g2 = (int16_t)(mpp_ask->msg.mate_q.g2.msb << 8) + mpp_ask->msg.mate_q.g2.lsb;//unit 0.01
			printk(" mate_q g0:%d g1:%d g2:%d", gd->tx_infos.mate_q_g0, gd->tx_infos.mate_q_g1, gd->tx_infos.mate_q_g2);

			mpp_mate_q_detect();

			fml_fsk_patt_send(EPWM1, T_RESPONSE, _FSK_ACK);
			break;
		default:
			if (is_neg_illegal_pkt(mpp_ask->hdr))
			{
				gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
				wpc_stop_to_idle(ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT);
				osal_stop_timerEx(WPC_NEXT_TIMER);
				goto __NEGO_PHASE_ERR__;
			}
			break;
	}

	if (gd->ptx_protocol_phase == WPC_PHASE_XFER)
	{
		osal_stop_timerEx(WPC_NEXT_TIMER);
	}
	else
	{
		osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
	}

__NEGO_PHASE_ERR__:
	return;
}

void wpc_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
	switch (gd->rx_infos.power_profile_mode)
	{
		case BPP:
			break;
		case EPP:
			wpc_epp_nego_phase_process(com_ask);
			break;
		case MPP:
			wpc_mpp_nego_phase_process(com_ask);
			break;
		default:
			break;
	}
}
