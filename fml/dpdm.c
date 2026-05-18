#include "regdef.h"
#include "tcpc.h"
#include "dpdm.h"
#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "osal.h"
#include "tcpm.h"
#include "pd.h"
#include "pdlib.h"
#include "nu6801.h"
#include "ufcs.h"
#include "afc_scp.h"
#include "usb_qc.h"
#include "port_manager.h"

void dpdm_source_init(void);
uint16_t qc_volt = 5000;
uint8_t bc12_type = 0;
uint8_t dpdm_map = 0xff;
uint8_t dpdm_snk_support = 0;

uint8_t dpdm_snk_qc_volt = 0;

//static enum dpdm_state_e dpdm_state = DPDM_OFF_STATE;

void usb_dpdm_task_init(void)
{
	osal_task_handler_reg(USB_DPDM_TASK, usb_dpdm_task_event_handler);
	osal_start_timerEx(USB_BC12_TIMER, 100, 100, USB_DPDM_TASK, DPDM_EVT_TIMER_PERIOD);

	dpdm_source_init();
}

void dpdm_source_init(void)
{
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 0;
	DPDM->HVDCP_CTRL.BITS.ENTER_DCP_INT_MASK = 1;
	DPDM->HVDCP_CTRL.BITS.ENTER_HVDCP_INT_MASK = 1;

	uint32_t *dpdm_ovrd_offset0 = (uint32_t *)(0x4000c0bc);
	uint32_t *dpdm_ovrd_offset4 = (uint32_t *)(0x4000c0bc + 4);

	*dpdm_ovrd_offset0 = 0x09;
	*dpdm_ovrd_offset4 |= 0x09;

	*dpdm_ovrd_offset0 = 0x0a;
	*dpdm_ovrd_offset4 |= ((*dpdm_ovrd_offset4) & ~0x30);

	*dpdm_ovrd_offset0 = 0x0b;
	*dpdm_ovrd_offset4 |= (((*dpdm_ovrd_offset4) & ~0x30) | 0x30);
}

void usb_dpdm_port0_switch(bool en)
{
	VIC_vModuleDisable();
	if (en)
	{
		GPA->MODE.BITS.PIN0 = 3; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C
		GPA->MODE.BITS.PIN1 = 3; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
	}
	else
	{
		// GPA->I_EN.BITS.PIN0 = 1;
		GPA->MODE.BITS.PIN0 = 0; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C
		// GPA->I_EN.BITS.PIN1 = 1;
		GPA->MODE.BITS.PIN1 = 0; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
	}
	VIC_vModuleEnable();
}

void usb_dpdm_port1_switch(bool en)
{
	VIC_vModuleDisable();
	if (en)
	{
		GPB->MODE.BITS.PIN2 = 3; //00:PB2 01:BPWM3 10:BADC2 11:DP_C2
		GPD->MODE.BITS.PIN0 = 2; //00:PD0 01:BADC8 10:DM_C2 11:RESERVED
	}
	else
	{
		/* GPIO high-Z: release PB2/PD0 for WB7720 USB data */
		GPB->I_EN.BITS.PIN2 = 0;
		GPB->O_EN.BITS.PIN2 = 0;
		GPB->MODE.BITS.PIN2 = 0; //PB2 GPIO
		GPD->I_EN.BITS.PIN0 = 0;
		GPD->MODE.BITS.PIN0 = 0; //PD0 GPIO
	}
	VIC_vModuleEnable();
}

void usb_dpdm_select(uint8_t tc_index)
{
	/* IP162_GB 单 C 口配置：物理 TypeC-B (PB2/PD0)，软件 PORT0_INDEX 映射到 TypeC-B。
	 * 必须设置 DPDM block 内部 MUX 路由到对应物理 pin，否则 BC1.2/QC 检测悬空 pin。
	 * 历史回归：c617026 把本函数体整体注释掉，导致 QC/AFC/FCP 全部失效。 */
	dpdm_printk("[DPDM] select port=%d\n", tc_index);
	if (tc_index == 0)
	{
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 3; // TypeC-B (PB2=DP_C2 / PD0=DM_C2)
	}
	else if (tc_index == 1)
	{
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 1; // TypeC-A (本项目禁用)
	}
	else if (tc_index == 2)
	{
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 2; // USB-A
	}
	else
	{
		DPDM->SOURCE_CTRL.BITS.MUX_PORT_NUM = 0;
	}

	dpdm_sink_deinit();

	DPDM->SOURCE_CTRL.BITS.PORT1_CTRL = 0; // TypeC-A 关闭
	DPDM->SOURCE_CTRL.BITS.PORT2_CTRL = 0; // USB-A 关闭
	DPDM->SOURCE_CTRL.BITS.PORT3_CTRL = 1; // TypeC-B 启用（本项目唯一 C 口）
	bc12_type = 0;
	dpdm_map = tc_index;
	dpdm_printk("[DPDM] dpdm_map=%d PORT3_CTRL=1\n", dpdm_map);
}

void usb_dpdm_autodcp_en(void)
{
	/* Source 模式一次打开 DCP/HVDCP/QC/AFC/SCP/UFCS 检测，
	 * 中断只负责投递事件，具体改压和协议处理留给 USB_DPDM_TASK。 */
	//
	DPDM->SOURCE_CTRL.BITS.EN_SRC_PROTOCOL = 1;
	DPDM->SOURCE_CTRL.BITS.EN_HVDCP_DET = 1;
	DPDM->SOURCE_CTRL.BITS.EN_QC_SRC_DET = 2;
	DPDM->SOURCE_CTRL.BITS.EN_AUTO_DCP = 1;
	DPDM->SOURCE_CTRL.BITS.EN_AFC_SRC_DET = 1;
	DPDM->SOURCE_CTRL.BITS.EN_SCP_SRC_DET = 1;
#if (CONFIG_UFCS_SOURCE_SUPPORT == 1)
	DPDM->SOURCE_CTRL.BITS.EN_UFCS_SRC_DET = 1;
#endif
	DPDM->SOURCE_CTRL.BITS.EN_900K_PD = 1;

	//

	DPDM->HVDCP_CTRL.BITS.DP_FAIL_DEG = 3;

	DPDM->HVDCP_CTRL.BITS.ENTER_DCP_INT_MASK = 0;
	DPDM->HVDCP_CTRL.BITS.ENTER_HVDCP_INT_MASK = 0;

	DPDM->QC_SRC_CTRL.BITS.TACTIVE = 1;
	DPDM->QC_SRC_CTRL.BITS.FIXED_5V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.FIXED_9V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.FIXED_12V_REQ_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.CONTINUOUS_MODE_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.QC_PULSE_DEC_INT_MASK = 0;
	DPDM->QC_SRC_CTRL.BITS.QC_PULSE_INC_INT_MASK = 0;

	DPDM->AFC_CTRL.BITS.AFC_RX_DATA_MASK = 0;
	DPDM->AFC_CTRL.BITS.SCP_RX_DATA_MASK = 0;
	//dpdm_printk("AFC_CTRL=0x%x\n",(uint32_t)(&DPDM->AFC_CTRL));
	//dpdm_printk("AFC_CTRL=0x%x\n",DPDM->AFC_CTRL.WORD);
}

extern union scp_packet_t scp_tx;
bool is_enter_dpdm_prot = false;
#if (BUCKBOOST_USED_NU6801 == 1)
uint16_t adc_input = 0;
#endif
void usb_dpdm_task_event_handler(uint32_t event)
{
	/* DPDM 任务按 source/sink 事件串行处理：进入 DCP/HVDCP 先回 5V，
	 * QC fixed/pulse 再调整 VBUS，AFC/SCP 收包由对应协议处理函数继续解析。 */
	switch (event)
	{
	case DPDM_EVT_SRC_ATTACHED:
		usb_dpdm_autodcp_en();
#if (CONFIG_UFCS_SOURCE_SUPPORT == 1)
		dpdm_ufcs_init();
#endif
		is_enter_dpdm_prot = false;
		//dpdm_printk("SOURCE_CTRL=0x%x\n",DPDM->SOURCE_CTRL.WORD);
		break;
	case DPDM_EVT_SRC_UNATTCHED:
		is_enter_dpdm_prot = false;
		dpdm_source_init();
#if (CONFIG_UFCS_SOURCE_SUPPORT == 1)
		dpdm_ufcs_deinit();
#endif
		break;
	case DPDM_EVT_ENTER_DCP:
		if (is_enter_dpdm_prot)
			hal_tcpc_pd_set_bus_iv(PORT0_INDEX, 5000, 3500, 0, 0);
		dpdm_printk("enter dcp\n");
		//usb_dpdm_autodcp_en();
		break;
	case DPDM_EVT_ENTER_HVDCP: //if enter dpdm,buck to 5v
		if (is_enter_dpdm_prot)
			hal_tcpc_pd_set_bus_iv(PORT0_INDEX, 5000, 3500, 0, 0);
		dpdm_printk("hvdcp\n");
		break;
	case DPDM_EVT_TIMER_PERIOD:
		//dpdm_printk("SOURCE_CTRL=0x%x\n",(uint32_t)(&DPDM->SOURCE_CTRL));
		//dpdm_printk("DPDM_RESULT=0x%x\n",DPDM->SOURCE_STAT.WORD);
		break;
	case DPDM_EVT_QC_FIXED_5V:
	case DPDM_EVT_QC_FIXED_9V:
	case DPDM_EVT_QC_FIXED_12V:
	case DPDM_EVT_QC_FIXED_20V:
		is_enter_dpdm_prot = true;
		if (pdlib_is_connect() && pdlib_get_source_supply_voltage() != 5000)
		{
			dpdm_printk("pd has work,qc should not work\n");
			return;
		}

		switch (DPDM->QC_SRC_FLAG.BITS.QC_SRC_STAT)
		{
		case QC_NOT_MODE:
			break;
		case QC_FIX_5V_MODE:
			qc_volt = 5000;
			break;
		case QC_FIX_9V_MODE:
			qc_volt = 9000;
			break;
		case QC_FIX_12V_MODE:
			qc_volt = 12000;
			break;
		case QC_FIX_20V_MODE:
			qc_volt = 20000; //not support qc 20v
			break;
		case QC_CONTINUOUS_MODE:
			break;
		}
		uint16_t qc_current = 18000 * 1000 / qc_volt;

		qc_current = qc_current > 3000 ? 3000 : qc_current;

		hal_tcpc_pd_set_bus_iv(0, qc_volt, qc_current + 300, 0, 10);

		dpdm_printk("qc2 v= %d i= %d\n", qc_volt, qc_current);
		break;
	case DPDM_EVT_QC_CONTINUES:
		break;

	case DPDM_EVT_AFC_SCP_OUT:
		is_enter_dpdm_prot = true;
		hal_tcpc_pd_set_bus_iv(0, scp_vout, scp_iout, 0, 10);
		break;

	case DPDM_EVT_QC_PLUSE_INC:
	case DPDM_EVT_QC_PLUSE_DEC:
		is_enter_dpdm_prot = true;
		if (pdlib_is_connect() && pdlib_get_source_supply_voltage() != 5000)
		{
			dpdm_printk("pd has work,qc should not work\n");
			return;
		}
		if (DPDM->QC_SRC_FLAG.BITS.QC_SRC_STAT == QC_CONTINUOUS_MODE)
		{
			uint16_t qc3_current = 18000 * 1000 / qc_volt;
			qc3_current = qc3_current > 3000 ? 3000 : qc3_current;
			hal_tcpc_pd_set_bus_iv(0, qc_volt, qc3_current + 300, 0, 10);

			dpdm_printk("qc3 v= %d i= %d\n", qc_volt, qc3_current);
		}

		break;
	case DPDM_EVT_AFC_RX_DATA:
		//dpdm_printk("afc rx = 0x%x\n",DPDM->AFC_RX_0.WORD);
		break;
	case DPDM_EVT_SCP_RX_DATA:
		dpdm_printk("\n");
		dpdm_printk("SCP RX:");
		for (uint8_t i = 0; i < scp_packet.bytes.msg_len; i++)
		{
			dpdm_printk(" 0x%x", ((uint8_t *)(&scp_packet.bytes.msg_0))[i]);
		}
		dpdm_printk("\n");

		break;
	case DPDM_EVT_SCP_TX_DATA:

		dpdm_printk("SCP TX:");
		for (uint8_t i = 0; i < scp_tx.bytes.msg_len; i++)
		{
			dpdm_printk(" 0x%x", ((uint8_t *)(&scp_tx.bytes.msg_0))[i]);
		}
		dpdm_printk("\n");
		break;

	case DPDM_EVT_SNK_ATTACHED:
		osal_stop_timerEx(DPDM_SINK_TIMER);
		dpdm_sink_init();
		break;
	case DPDM_EVT_SNK_UNATTCHED:
		dpdm_sink_deinit();
		break;
	case DPDM_EVT_SNK_BC12DONE:
		dpdm_printk("\r\n  BC12 bc12_type=0x%x  UNSTANDARD_TYPE = 0x%x \r\n", DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE, DPDM_QC_SINK->BC1P2_STAT.BITS.UNSTANDARD_TYPE);
		/* === DPDM 直接电压诊断（确认适配器是否短接 D+/D-） ===
			 * VDP_RD / VDM_RD 是 3-bit 阈值编码 (0..7)，越大代表电压越高。
			 * - DCP 适配器：D+ 被注入 ~0.6V 电流源后，由于 D+/D- 短接，D- 也升高 → 两值相近且 > 0
			 * - SDP/无连接：D- 浮空 → VDM_RD ≈ 0
			 * - 线缆 D+/D- 断开：两值都 ≈ 0
			 */
		{
			uint32_t bc12_raw = DPDM_QC_SINK->BC1P2_STAT.WORD;
			uint32_t ctrl_raw = DPDM_QC_SINK->BC1P2_INTMSK_CTRL.WORD;
			dpdm_printk("[DPDM-DBG] BC1P2_STAT=0x%08X CTRL=0x%08X dpdm_map=%d\n",
			            bc12_raw, ctrl_raw, dpdm_map);
			/* 启用 D+/D- 读回（manual mode 用于诊断，读完关掉避免影响后续 QC 流程）*/
			DPDM_QC_SINK->DPDM_MANUAL.BITS.DP_RD_EN = 1;
			DPDM_QC_SINK->DPDM_MANUAL.BITS.DM_RD_EN = 1;
			delay_1ms(2);
			uint8_t vdp = DPDM_QC_SINK->DPDM_MANUAL.BITS.VDP_RD;
			uint8_t vdm = DPDM_QC_SINK->DPDM_MANUAL.BITS.VDM_RD;
			DPDM_QC_SINK->DPDM_MANUAL.BITS.DP_RD_EN = 0;
			DPDM_QC_SINK->DPDM_MANUAL.BITS.DM_RD_EN = 0;
			dpdm_printk("[DPDM-DBG] VDP_RD=%d VDM_RD=%d (3bit code, equal&>0 => DCP short)\n", vdp, vdm);
		}
		if (DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x02)
			bc12_type = BC1P2_CDP;
		else if (DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x03)
			bc12_type = BC1P2_DCP;
		else if (DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x06)
		{
			bc12_type = BC1P2_APPLE; //APPLE 2.4A 2.1A
		}
		else
			bc12_type = BC1P2_SDP;
		if (DPDM_QC_SINK->BC1P2_STAT.BITS.BC1P2_TYPE == 0x03) //DCP
			osal_start_timerEx(DPDM_SINK_TIMER, 25, 0, USB_DPDM_TASK, DPDM_EVT_SNK_HVDCP_START);
		break;
	case DPDM_EVT_SNK_HVDCP_START:
		osal_stop_timerEx(DPDM_SINK_TIMER);
		dpdm_printk("hvdcp start\n");
		DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_EN = 1;
		DPDM_QC_SINK->QC_INTMSK_CTRL.BITS.QC_MODE = 0x03;
		DPDM->SOURCE_CTRL.BITS.EN_900K_PD = 1;
		break;
	case DPDM_EVT_SNK_HVDCP_DONE:
		dpdm_printk("hvdcp done\n");
		//osal_set_event(USB_TASK,TCPM_EVT_HVDCP_DONE);

		if (g_port.snk_5v_only == 0 && pdlib_is_connect() == false)
		{
			osal_start_timerEx(DPDM_SINK_TIMER, 50, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC_START);
		}
		else
		{
			bc12_type = BC1P2_HVDCP;
			osal_set_event(USB_TASK, TCPM_EVT_DPDM_DONE);
		}
		break;
	case DPDM_EVT_SNK_HVDCP_FAIL:
		osal_set_event(USB_TASK, TCPM_EVT_DPDM_DONE);
		break;
	case DPDM_EVT_SNK_QC_START:
		buckboost_ops.set_ovp(21000); //20000
		bc12_type = BC1P2_HVDCP;
		qc2_set_volt(12000);
		osal_start_timerEx(DPDM_SINK_TIMER, 200, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC12V_DONE);
		break;
	case DPDM_EVT_SNK_QC12V_DONE:
#if (BUCKBOOST_USED_NU6801 == 1 && 0)
		if (dpdm_map == 0)
			adc_input = hal_nu6801_buckboost_typeca_vbus_present();
		else
			adc_input = hal_nu6801_buckboost_typecb_vbus_present();
		dpdm_printk("Set Qc 12V=%d\n", adc_input);
		if (adc_input >= 10500)
#else
		dpdm_printk("Set Qc 12V=%d\n", g_buckboost.adc_vbus);
		if (g_buckboost.adc_vbus >= 10500)
#endif
		{
			bc12_type = BC1P2_QC12V;
			qc2_set_volt(5000);
			pdlib_disable_usbpd();
		}
		else
		{
			qc2_set_volt(9000);
			osal_start_timerEx(DPDM_SINK_TIMER, 200, 0, USB_DPDM_TASK, DPDM_EVT_SNK_QC_DONE);
		}
		osal_set_event(USB_TASK, TCPM_EVT_DPDM_DONE);
		break;

	case DPDM_EVT_SNK_QC_DONE:
#if (BUCKBOOST_USED_NU6801 == 1 && 0)
		if (dpdm_map == 0)
			adc_input = hal_nu6801_buckboost_typeca_vbus_present();
		else
			adc_input = hal_nu6801_buckboost_typecb_vbus_present();
		dpdm_printk("Set Qc 9V=%d\n", adc_input);
		if (adc_input >= 7500)
#else
		dpdm_printk("Set Qc 9V=%d\n", g_buckboost.adc_vbus);
		if (g_buckboost.adc_vbus >= 7500)
#endif
		{
			bc12_type = BC1P2_QC9V;
			pdlib_disable_usbpd();
		}
		qc2_set_volt(5000);
		osal_set_event(USB_TASK, TCPM_EVT_DPDM_DONE);
		break;
#if (CONFIG_UFCS_SOURCE_SUPPORT == 1)
	case DPDM_EVT_UFCS_INT:
		break;
	case DPDM_EVT_UFCS_RX_PACKET:
		ufcs_rx_packet_handle();
		break;
	case DPDM_EVT_UFCS_PSREADY:
		ufcs_psread_handle();
		break;
	case DPDM_EVT_UFCS_RX_RESET:
		dpdm_printk("UFCS HARDRESET\n");
		ufcs_exit_handle();
		break;
#endif
	default:
		break;
	}
}

void __attribute__((isr)) DCP_HVDCP_IRQHandler(void)
{
	/* ISR 只清硬件 flag 并投递 OSAL 事件，避免在中断上下文直接改母线电压。 */
	uint32_t int_flag = (DPDM->HVDCP_FLAG.WORD) & 0b00001100;
	do
	{
		if (int_flag & (0x01 << 2))
		{
			DPDM->HVDCP_FLAG.BITS.ENTER_DCP = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_ENTER_DCP);
		}

		if (int_flag & (0x01 << 3))
		{
			DPDM->HVDCP_FLAG.BITS.ENTER_HVDCP = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_ENTER_HVDCP);
		}

		int_flag = (DPDM->HVDCP_FLAG.WORD) & 0b00001100;

	} while (int_flag);
}

void __attribute__((isr)) QC_SRC_IRQHandler(void)
{
	/* QC 中断会同步更新请求电压影子值，但实际设置仍由任务事件执行。 */

	uint32_t int_flag = (DPDM->QC_SRC_FLAG.WORD) & 0x3F80;

	//dpdm_printk("qc3 int_flag = 0x%x\n",DPDM->QC_SRC_FLAG.WORD);
	do
	{
		if (int_flag & (0x01 << 7))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_5V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_FIXED_5V);
		}

		if (int_flag & (0x01 << 8))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_9V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_FIXED_9V);
		}

		if (int_flag & (0x01 << 9))
		{
			DPDM->QC_SRC_FLAG.BITS.FIXED_12V_REQ_INT = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_FIXED_12V);
		}

		// if(int_flag & (0x01<<10))
		// {
		// 	DPDM->QC_SRC_FLAG.BITS.FIXED_20V_REQ_INT = 1;
		// 	osal_set_event(USB_DPDM_TASK,DPDM_EVT_QC_FIXED_20V);
		// }

		if (int_flag & (0x01 << 11))
		{
			DPDM->QC_SRC_FLAG.BITS.CONTINUOUS_MODE_INT = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_CONTINUES);
		}

		if (int_flag & (0x01 << 12))
		{
			DPDM->QC_SRC_FLAG.BITS.QC_PULSE_INC_INT = 1;
			qc_volt += 200;
			if (qc_volt >= 12000)
				qc_volt = 12000;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_PLUSE_INC);
		}

		if (int_flag & (0x01 << 13))
		{
			qc_volt -= 200;
			if (qc_volt <= 5000)
				qc_volt = 5000;
			DPDM->QC_SRC_FLAG.BITS.QC_PULSE_DEC_INT = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_QC_PLUSE_DEC);
		}

		int_flag = (DPDM->QC_SRC_FLAG.WORD) & 0x3F80;

	} while (int_flag);
}

void __attribute__((isr)) AFC_SCP_SRC_IRQHandler(void)
{
	/* AFC/SCP 收包中断需要立即清 flag，并唤醒协议解析，避免 FIFO 数据被下一帧覆盖。 */
	uint32_t int_flag = (DPDM->AFC_INT_FLAG.WORD) & 0x0060;

	do
	{
		if (int_flag & (0x01 << 5))
		{
			DPDM->AFC_INT_FLAG.BITS.AFC_RX_DATA_READY_FLAG = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_AFC_RX_DATA);
			dpdm_src_afc_handle();
		}

		if (int_flag & (0x01 << 6))
		{
			DPDM->AFC_INT_FLAG.BITS.SCP_RX_DATA_READY_FLAG = 1;
			osal_set_event(USB_DPDM_TASK, DPDM_EVT_SCP_RX_DATA);
			extern void dpdm_src_scp_handle(void);
			dpdm_src_scp_handle();
		}

		int_flag = (DPDM->AFC_INT_FLAG.WORD) & 0x0060;

	} while (int_flag);
}
