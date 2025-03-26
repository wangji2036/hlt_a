#include "regdef.h"
#include "printk.h"
#include "tcpc.h"
#include "isr.h"
#include "bsp.h"
#include "g_data.h"
#include "pd.h"
#include "osal.h"
#include "usb_pd.h"
#include "buckboost.h"
#include "usbpd_config.h"
#include "tcpm.h"

uint8_t tcpc_transmit_retry_cnt;
uint8_t tcpc_transmit_byte_index;
struct usb_pd_pkt_t transmit_pkt;

struct tcpc_s g_tcpc;


void hal_tcpc_init(void)
{
#if(CONFIG_USBTC_PORT_SELECT & TC_PORT_CCA)
	TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	TCPC->CCA_CTRL.BITS.CC_DCSRC_DRP = 1; // 50%
	TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1; // dead battary off
	TCPC->CCA_CTRL.BITS.CC_LPMODE_RP = 0;
	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
#endif

#if(CONFIG_USBTC_PORT_SELECT & TC_PORT_CCB)
	TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	TCPC->CCB_CTRL.BITS.CC_DCSRC_DRP = 1; // 50%
	TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1; // dead battary off
	TCPC->CCB_CTRL.BITS.CC_LPMODE_RP = 0;
	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
#endif
	TCPC->PHY_CTRL.BITS.PD_PHY_EN = 0x01;
	TCPC->PHY_CTRL.BITS.PD_RX_VREF_SEL = 0x05;

#if(PD_PORT_MAP == TYPEC_PORT_A)
	g_tcpc.tc_port_map = TYPEC_PORT_A;
	TCPC->PHY_CTRL.BITS.PD_CC_PORT_SEL = 0x01;
#else
	g_tcpc.tc_port_map = TYPEC_PORT_B;
	TCPC->PHY_CTRL.BITS.PD_CC_PORT_SEL = 0x02;
#endif
	tcpc_transmit_retry_cnt = 0;
}

void hal_tcpc_set_phy_port(uint8_t tc_index)
{
	if(tc_index == 0)
	{
		g_tcpc.tc_port_map = TYPEC_PORT_A;
		TCPC->PHY_CTRL.BITS.PD_CC_PORT_SEL = 0x01;
	}
	else
	{
		g_tcpc.tc_port_map = TYPEC_PORT_B;
		TCPC->PHY_CTRL.BITS.PD_CC_PORT_SEL = 0x02;
	}

	printk("pd phy sel = %d\n",tc_index);
}

void hal_tcpc_set_phy_rx_vref(enum rx_vref vref)
{
	TCPC->PHY_CTRL.BITS.PD_RX_VREF_SEL = vref;
}

void hal_tcpc_set_cc(uint8_t tc_index,enum tc_cc_status cc)
{
	if(tc_index == 0)
	{
	    switch (cc)
	    {
	        case TYPEC_CC_RA:
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RA;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RA;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RD:
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RD;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RD;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_DEF:
	        	TCPC->CCA_ROLE.BITS.RP_VALUE = RP_VALUE_DEFAULT;
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_1_5:
	        	TCPC->CCA_ROLE.BITS.RP_VALUE = RP_VALUE_1A5;
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_3_0://5
	        	TCPC->CCA_ROLE.BITS.RP_VALUE = RP_VALUE_3A0;
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_TOGGLE:
	        	TCPC->CCA_ROLE.BITS.RP_VALUE = RP_VALUE_DEFAULT;
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_RD;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_RD;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 1;
	        	TCPC->CCA_CTRL.BITS.CC_DCSRC_DRP = 2;
	        	TCPC->CCA_CTRL.BITS.CC_T_DRP_SEL = 3;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 1;
	        	TCPC->CCA_CMD_.BITS.CMD_TYPE = 0x99;
	        	break;
	        case TYPEC_CC_OPEN:
	        default:
	        	TCPC->CCA_ROLE.BITS.CC1_ROLE = CC_STATE_OPEN;
	        	TCPC->CCA_ROLE.BITS.CC2_ROLE = CC_STATE_OPEN;
	        	TCPC->CCA_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	    }
	    //printk("CCA_ROLE = 0x%x\n",TCPC->CCA_ROLE.WORD);
	    //printk("CCA_CTRL = 0x%x\n",TCPC->CCA_CTRL.WORD);
	}
	else if(tc_index == 1)
	{
	    switch (cc)
	    {
	        case TYPEC_CC_RA:
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RA;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RA;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RD:
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RD;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RD;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_DEF:
	        	TCPC->CCB_ROLE.BITS.RP_VALUE = RP_VALUE_DEFAULT;
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_1_5:
	        	TCPC->CCB_ROLE.BITS.RP_VALUE = RP_VALUE_1A5;
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_RP_3_0://5
	        	TCPC->CCB_ROLE.BITS.RP_VALUE = RP_VALUE_3A0;
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RP;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	        case TYPEC_CC_TOGGLE:
	        	TCPC->CCB_ROLE.BITS.RP_VALUE = RP_VALUE_DEFAULT;
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_RD;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_RD;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 1;
	        	TCPC->CCB_CTRL.BITS.CC_DCSRC_DRP = 2;
	        	TCPC->CCB_CTRL.BITS.CC_T_DRP_SEL = 3;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 1;
	        	TCPC->CCB_CMD_.BITS.CMD_TYPE = 0x99;
	        	break;
	        case TYPEC_CC_OPEN:
	        default:
	        	TCPC->CCB_ROLE.BITS.CC1_ROLE = CC_STATE_OPEN;
	        	TCPC->CCB_ROLE.BITS.CC2_ROLE = CC_STATE_OPEN;
	        	TCPC->CCB_ROLE.BITS.DRP_MODE = 0;
	        	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
	            break;
	    }
	    //printk("CCB_ROLE = 0x%x\n",TCPC->CCB_ROLE.WORD);
	    //printk("CCB_CTRL = 0x%x\n",TCPC->CCB_CTRL.WORD);
	}
}

enum tc_cc_status hal_tcpc_to_typec_cc(uint32_t cc, bool sink)
{
    switch(cc)
    {
        case 0x00:
            return sink ? TYPEC_CC_OPEN : TYPEC_CC_OPEN;
        case 0x01:
            return sink ? TYPEC_CC_RP_DEF : TYPEC_CC_RA;
        case 0x02:
            return sink ? TYPEC_CC_RP_1_5 : TYPEC_CC_RD;
        case 0x03:
            return sink ? TYPEC_CC_RP_3_0 : TYPEC_CC_OPEN;
        default:
            return TYPEC_CC_OPEN;
    }
}

void hal_tcpc_get_cc(uint8_t tc_index,enum tc_cc_status *cc1, enum tc_cc_status *cc2)
{
	bool sink = 0;
	uint32_t cc_status = 0;

	if(tc_index == 0)
	{
		sink = (TCPC->CCA_ROLE.WORD & 0x03) == CC_STATE_RD? true : false;
		cc_status = TCPC->CCA_STAT.WORD;
	}
	else
	{
		sink = (TCPC->CCB_ROLE.WORD & 0x03) == CC_STATE_RD? true : false;
		cc_status = TCPC->CCB_STAT.WORD;
	}
    *cc1 = hal_tcpc_to_typec_cc(cc_status & 0x03,sink);
    *cc2 = hal_tcpc_to_typec_cc((cc_status & 0x0c) >>2,sink);
}

bool hal_tcpc_vbus_is_present(uint8_t tc_index)
{
	static uint8_t cnt = 0;
#if(BUCKBOOST_USED_NU6801 == 1)
	cnt++;
	if(cnt >= 10)
	{
		cnt = 0;

		if(tc_index == 0)
		{
			if(buckboost_ops.get_typeca_vbus_present() >= 3800 ) return true;
		}
		else if(tc_index == 1)
		{
			if(buckboost_ops.get_typecb_vbus_present() >= 3800 ) return true;
		}
	}

	return false;
#else
	return true;
#endif
}

bool hal_tcpc_vbus_is_removed(uint8_t tc_index)
{
#if(BUCKBOOST_USED_NU6801 == 1)
	static uint8_t delay_cnt = 0;
	if(delay_cnt == 0)
	{
		if(tc_index == 0)
		{
			if(buckboost_ops.get_typeca_vbus_present() < 2000 ) return true;
		}
		else if(tc_index == 1)
		{
			if(buckboost_ops.get_typecb_vbus_present() < 2000 ) return true;
		}
		return false;
	}
	delay_cnt++;
	if(delay_cnt >= 23) delay_cnt = 0;
	return false;
#else
	return true;
#endif
}

bool hal_tcpc_vbus_is_vsfae0v(uint8_t tc_index)
{
#if(BUCKBOOST_USED_NU6801 == 1)
	if(tc_index == 0)
	{
		if(buckboost_ops.get_typeca_vbus_present() < 800 ) return true;
	}
	else if(tc_index == 1)
	{
		if(buckboost_ops.get_typecb_vbus_present() < 800 ) return true;
	}
	return false;
#else
	return true;
#endif
}

bool hal_tcpc_vbus_is_vsafe5v(void)
{
	if(g_buckboost.adc_vbus <= 5500) return true;
	return false;
}


void hal_tcpc_port_dummyload_en(uint8_t tc_index,bool en)
{
	if(tc_index == 0)
	{
		if(en)
			osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_EN);
		else
			osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_DIS);
	}

	if(tc_index == 1)
	{
		if(en)
			osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_EN);
		else
			osal_set_event(BUCKBOOST_TASK,BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_DIS);
	}
}

enum tc_drp_reult hal_get_drp_toggle_result(uint8_t tc_index)
{

	uint32_t  cc_stat = 0;

	if(tc_index == 0)
		cc_stat = TCPC->FSM_STAT.BITS.CCA_STAT;
	else
		cc_stat = TCPC->FSM_STAT.BITS.CCB_STAT;

	//printk("cc_stat=0x%x\n",cc_stat);

	switch(cc_stat)
	{
		case 0x03:
			return TYPEC_DRP_SNK_CONNECTED;
		case 0x04:
			return TYPEC_DRP_SRC_CONNECTED;
		default:
			return TYPEC_DRP_NO_CONNECT;
	}


}

void hal_tcpc_set_data_role(uint8_t tc_index,enum data_role_e role)
{
	if(tc_index != g_tcpc.tc_port_map) return;
	g_tcpc.data_role = role;

	TCPC->RXD_CTRL.BITS.PHY_GDCRC_PDR = role;

}

void hal_tcpc_set_pwr_role(uint8_t tc_index,enum pwr_role_e role)
{
	if(tc_index != g_tcpc.tc_port_map) return;
	g_tcpc.pwr_role = role;

	TCPC->RXD_CTRL.BITS.PHY_GDCRC_PPR = role;
}

void hal_tcpc_set_gate_en(uint8_t tc_index,bool en)
{
	printk("gate[%d]:%d\n",tc_index,en);
	if(tc_index == 0)
		buckboost_set_typeca_gate_en(en);
	else if(tc_index == 1)
		buckboost_set_typecb_gate_en(en);
	else if(tc_index == 2)
		buckboost_set_usb_a_gate_en(en);
}

void hal_tcpc_set_vconn(uint8_t tc_index,bool en)
{

}

void hal_tcpc_set_pd_rx(uint8_t tc_index,uint32_t sop,bool en)
{
	if(tc_index != g_tcpc.tc_port_map) return;

	if(en)
		TCPC->RXD_CTRL.WORD |= sop;
	else
		TCPC->RXD_CTRL.WORD &= ~sop;
}
void hal_tcpc_set_roles(uint8_t tc_index,enum pwr_role_e pwr_role,enum data_role_e data_role)
{
	if(tc_index != g_tcpc.tc_port_map) return;

	g_tcpc.data_role = data_role;
	g_tcpc.pwr_role = pwr_role;
	TCPC->RXD_CTRL.BITS.PHY_GDCRC_PDR = data_role;
	TCPC->RXD_CTRL.BITS.PHY_GDCRC_REV = 0x01;
	TCPC->RXD_CTRL.BITS.PHY_GDCRC_PPR = pwr_role;
}

void hal_tcpc_set_polarity(uint8_t tc_index,enum tc_cc_polarity polarity)
{
	if(tc_index == 0)
	{
		if(polarity == TYPEC_POLARITY_CC1)
			TCPC->CCA_CTRL.BITS.CC_PD_CH_SEL = 0x00;
		else
			TCPC->CCA_CTRL.BITS.CC_PD_CH_SEL = 0x01;
	}
	else
	{
		if(polarity == TYPEC_POLARITY_CC1)
			TCPC->CCB_CTRL.BITS.CC_PD_CH_SEL = 0x00;
		else
			TCPC->CCB_CTRL.BITS.CC_PD_CH_SEL = 0x01;
	}
}

void hal_tcpc_set_bist_data(bool on)
{

}

void hal_tcpc_reset_pd_phy(void)
{
	TCPC->PHY_CTRL.BITS.PD_PHY_EN = 0x00;
	TCPC->PHY_CTRL.BITS.PD_PHY_EN = 0x01;
}

void hal_tcpc_pd_phy_enable(void)
{
	TCPC->PHY_CTRL.BITS.PD_PHY_EN = 0x01;
	TCPC->INT_CTRL.BITS.PHY_RX_SUCCESSFUL_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_RX_HARD_RESET_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_TX_NO_GOODCRC_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_TX_CC_DISCARD_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_TX_SUCCESSFUL_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_RX_BUFF_UPDAT_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_TX_BUFF_EMPTY_INTE = 1;
	TCPC->INT_CTRL.BITS.PHY_RX_DATA_ERROR_INTE = 1;
}

void hal_tcpc_pd_phy_disable(void)
{
	TCPC->PHY_CTRL.BITS.PD_PHY_EN = 0x00;
	TCPC->INT_CTRL.BITS.PHY_RX_SUCCESSFUL_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_RX_HARD_RESET_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_TX_NO_GOODCRC_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_TX_CC_DISCARD_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_TX_SUCCESSFUL_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_RX_BUFF_UPDAT_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_TX_BUFF_EMPTY_INTE = 0;
	TCPC->INT_CTRL.BITS.PHY_RX_DATA_ERROR_INTE = 0;
}

void hal_tcpc_send_hardreset(void)
{
	usbpd_printk("HARDRESET SENT\n");
	transmit_pkt.msg_len = 0;
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_HARDRESER;
	hal_tcpc_pkt_transmit(Transmit_HardReset,&transmit_pkt);
}

void hal_tcpc_send_bistdata(void)
{
	transmit_pkt.msg_len = 0;
	g_usb_pd_s.pe_tran_cb_type = TRANSMITE_TYPE_BISTCARRYMODE;
	hal_tcpc_pkt_transmit(Transmit_BIST_CarrierMode2,&transmit_pkt);
}


void hal_tcpc_pkt_transmit(enum transmit_frame_type frame, struct usb_pd_pkt_t *pkt)
{
	if(frame == Transmit_SOP || frame == Transmit_SOP1)
	{
		g_usb_pd_s.pe_prl_busy = 1;
		tcpc_transmit_byte_index = 0;
		TCPC->TXD_CTRL.BITS.TXD_SOP_TYP = frame;   // sop
		TCPC->TXD_BUFF.WORD = pkt->msg.WORDS[0];
		TCPC->TXD_INFO.WORD = BIG_LITTLE_SWAP16(pkt->hdr.WORD) | (((pkt->msg_len * 4) + 2) << 16);   // sop
	}
	else
	{
		TCPC->TXD_CTRL.BITS.TXD_SOP_TYP = frame;   // sop
	}
}

void hal_tcpc_send_request_mgs(uint32_t rdo)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_REQUEST,g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 1);
	transmit_pkt.msg_len = 1;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.request.request.WORD = rdo;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_send_ctrl_mgs(enum pd_ctrl_msg_type msg_type)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(msg_type, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 0);
	transmit_pkt.msg_len = 0;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_pd_send_revision(void)
{

	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_REVISION, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 1);
	transmit_pkt.msg_len = 1;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0x31180000;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}


/*
voltage: target voltage
current: target current
wait:  neet wait some times then regulation
delay:  neet delay some times for regulation loop stable
*/
void hal_tcpc_pd_set_bus_iv(uint8_t tc_index,uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay)
{
	//
	buckboost_set_bus_iv(voltage,current,wait,delay);

	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		port_vbus = voltage;
	}
}

/*
 * hal_tcpc_pd_set_bus_iv   need return the state,
 * if regulation complete return true,other flase
 */
bool hal_tcpc_pd_bus_ready(uint8_t tc_index)
{
	//if(tc_index != 0) return true;
	return buckboost_regulator_done();
}

void hal_tcpc_set_source_mode(enum buckboost_mode mode)
{
	printk("set buckboost mode = %d\n",mode);
	buckboost_set_work_mode(mode);
}

void hal_tcpc_set_snk_charge_current(uint16_t ibat,uint16_t ibus)
{
	buckboost_set_charge_current(ibat,ibus);
}

void hal_tcpc_send_source_caps(uint32_t * pdos,uint32_t pdo_n)
{

	uint8_t fix_pdo_n = 0,tx_pdo_n = 0;
	struct usb_pd_source_cap_packet_t * p = (struct usb_pd_source_cap_packet_t *)pdos;

	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	if(pdo_n > 7) return;

    for (uint8_t i = 0; i < pdo_n; i++)
    {

    	if(p->source_pdo[i].BITS.FIX_BITS.fixed == 0x00) fix_pdo_n ++;
		transmit_pkt.msg.source_cap.source_pdo[i].WORD = pdos[i];
		//printk("0x%x ",transmit_pkt.msg.source_cap.source_pdo[i].WORD);
    }
    //printk("\n");

    tx_pdo_n = g_usb_pd_s.nego_revision < PD_REV30? fix_pdo_n : pdo_n;
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_SOURCE_CAP, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, tx_pdo_n);
	transmit_pkt.msg_len = tx_pdo_n;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;

	//printk("\n%s :%d\n",__func__,pdo_n);

	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_send_snk_caps(uint32_t * pdos,uint32_t pdo_n)
{

	uint8_t fix_pdo_n = 0,tx_pdo_n = 0;
	struct usb_pd_sink_cap_packet_t * p = (struct usb_pd_sink_cap_packet_t *)pdos;
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	if(pdo_n > 7) return;

    for (uint8_t i = 0; i < pdo_n; i++)
    {
    	if(p->sink_pdo[i].BITS.FIX_BITS.fixed == 0x00) fix_pdo_n ++;
		transmit_pkt.msg.sink_cap.sink_pdo[i].WORD = pdos[i];
		//printk("0x%x ",transmit_pkt.msg.source_cap.source_pdo[i].WORD);
    }
    //printk("\n");

    tx_pdo_n = g_usb_pd_s.nego_revision < PD_REV30? fix_pdo_n : pdo_n;
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_SINK_CAP, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, tx_pdo_n);
	transmit_pkt.msg_len = tx_pdo_n;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;

	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_send_sink_caps_ext(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_EXT_SNK_CAPABILITIES_EXTENDED, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 7);
	transmit_pkt.hdr.BITS.externed = 1;
	transmit_pkt.msg_len = 7;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.data_size = 24;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.request_chunk = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunk_num = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunked = 1;
	transmit_pkt.msg.ext_msg.data[0] = (uint8_t)USBPD_VID;
	transmit_pkt.msg.ext_msg.data[1] = (USBPD_VID >> 8);
	transmit_pkt.msg.ext_msg.data[10] = 0x01;
	transmit_pkt.msg.ext_msg.data[17] = 0x02;
	transmit_pkt.msg.ext_msg.data[18] = 0;
	transmit_pkt.msg.ext_msg.data[19] = 5;
	transmit_pkt.msg.ext_msg.data[20] = 18;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void tcpc_pd_send_pps_status(void)
{

	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_EXT_PPS_STATUS, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 2);
	transmit_pkt.hdr.BITS.externed = 1;
	transmit_pkt.msg_len = 2;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.data_size = 4;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.request_chunk = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunk_num = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunked = 1;
	transmit_pkt.msg.ext_msg.data[0] = PD_PPS_SET_OUTPUT_MV(g_buckboost.adc_vbus) & 0xFF;
	transmit_pkt.msg.ext_msg.data[1] = PD_PPS_SET_OUTPUT_MV(g_buckboost.adc_vbus) >> 8;
	transmit_pkt.msg.ext_msg.data[2] = 0xFF;
	transmit_pkt.msg.ext_msg.data[3] =  0x1 << 1 | ((g_buckboost.ibus_cc_flag) ? (0x1 << 3) : 0x0);
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void tcpc_pd_send_bat_capability(uint8_t bat_index)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_EXT_BATT_CAP, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 3);
	transmit_pkt.hdr.BITS.externed = 1;
	transmit_pkt.msg_len = 3;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.data_size = 9;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.request_chunk = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunk_num = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunked = 1;
	transmit_pkt.msg.ext_msg.data[0] = 0xFF;
	transmit_pkt.msg.ext_msg.data[1] = 0XFF;
	transmit_pkt.msg.ext_msg.data[4] = 0xFF;
	transmit_pkt.msg.ext_msg.data[5] = 0xFF;
	transmit_pkt.msg.ext_msg.data[6] = 0xFF;
	transmit_pkt.msg.ext_msg.data[7] = 0xFF;
	if(bat_index == 0)
		transmit_pkt.msg.ext_msg.data[8] = 0x00;
	else
		transmit_pkt.msg.ext_msg.data[8] = 0x01;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}




