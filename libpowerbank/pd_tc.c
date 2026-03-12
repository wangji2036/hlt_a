#include "regdef.h"
#include "printk.h"
#include "pd_tc.h"
#include "isr.h"
#include "bsp.h"
#include "g_data.h"
#include "pd.h"
#include "osal.h"
#include "usb_pd.h"
#include "buckboost.h"
#include "usbpd_config.h"
#include "tcpm.h"
#include "config.h"
#include "pd_tc.h"

uint8_t tcpc_transmit_retry_cnt;
uint8_t tcpc_transmit_byte_index;
struct usb_pd_pkt_t transmit_pkt;

struct tcpc_s g_tcpc;

void hal_tcpc_init(void)
{
if(lib_para.typec_a_support)
{
	TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	TCPC->CCA_CTRL.BITS.CC_DCSRC_DRP = 1; // 50%
	TCPC->CCA_CTRL.BITS.CC_DB_RD_DIS = 1; // dead battary off
	TCPC->CCA_CTRL.BITS.CC_LPMODE_RP = 0;
	TCPC->CCA_CTRL.BITS.CC_LPMODE_EN = 0;
}

if(lib_para.typec_b_support)
{
	TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 0; // enable cc block
	TCPC->CCB_CTRL.BITS.CC_DCSRC_DRP = 1; // 50%
	TCPC->CCB_CTRL.BITS.CC_DB_RD_DIS = 1; // dead battary off
	TCPC->CCB_CTRL.BITS.CC_LPMODE_RP = 0;
	TCPC->CCB_CTRL.BITS.CC_LPMODE_EN = 0;
}


#if(CONFIG_TYPECA_SUPPORT != 1)
	TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 1; // disable cc block
	printk("\n Dis_CCA_BLOCK \n");
#endif

#if(CONFIG_TYPECB_SUPPORT != 1)
	TCPC->CCB_CTRL.BITS.CC_BLOCK_DIS = 1; // disable cc block
	printk("\n Dis_CCB_BLOCK \n");
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
	transmit_pkt.msg.WORDS[0] = 0x32110000;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
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
	transmit_pkt.msg.ext_msg.data[16] = 0x01;
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

void hal_tcpc_pd_send_batt_status(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_BAT_STATUS, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 1);
	transmit_pkt.msg_len = 1;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;

	if(g_tcpc.pwr_role == TYPEC_SOURCE)
		transmit_pkt.msg.WORDS[0] = 0xFFFF0100;
	else
		transmit_pkt.msg.WORDS[0] = 0xFFFF0100;
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
	if(bat_index == 0 && g_tcpc.pwr_role == TYPEC_SOURCE)
		transmit_pkt.msg.ext_msg.data[8] = 0x00;
	else
		transmit_pkt.msg.ext_msg.data[8] = 0x01;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_pd_send_Alert(void)
{

	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_ALERT, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 1);
	transmit_pkt.msg_len = 1;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0x02100000;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}


void hal_tcpc_send_discover_Identity_Ack(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 5);
	transmit_pkt.msg_len = 5;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0xFF00A841;
    if(g_usb_pd_s.nego_revision ==PD_REV20)
    	transmit_pkt.msg.WORDS[1] =  0x19800000 | USBPD_VID;
    else
    	transmit_pkt.msg.WORDS[1] =  0x19C00000 | USBPD_VID;
    transmit_pkt.msg.WORDS[2] = 0x00000000;
    transmit_pkt.msg.WORDS[3] = 0x26810000;
    transmit_pkt.msg.WORDS[4] = 0x40000000;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_send_discover_SVID_Ack(void)
{

	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 2);
	transmit_pkt.msg_len = 2;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0xFF00A842;
    transmit_pkt.msg.WORDS[1] = 0xFF010000;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

#define UVDM_GET		0b00
#define UVDM_Response	0b01
#define UVDM_NAK		0b10
#define UVDM_Wait		0b11

#define PowerBankBattery_Basic_Info 	0x0000
#define PowerBankBattery_Realtime_Info	0x0001
#define PowerBankBattery_Abnormal_Info	0x0002

void hal_tcpc_uvdm_send_PowerBankBattery_Realtime_Info(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_EXT_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 7);
	transmit_pkt.hdr.BITS.externed = 1;
	transmit_pkt.msg_len = 7;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.data_size = 24;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.request_chunk = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunk_num = 0;
	transmit_pkt.msg.ext_msg.ext_hrd.BITS.chunked = 1;
	//vdm header
	transmit_pkt.msg.ext_msg.data[0] = PowerBankBattery_Realtime_Info << 2 | UVDM_Response | 0xC0;
	transmit_pkt.msg.ext_msg.data[1] = 0x00;
	transmit_pkt.msg.ext_msg.data[2] = (uint8_t)USBPD_VID;
	transmit_pkt.msg.ext_msg.data[3] = (uint8_t)(USBPD_VID >> 8);

	//message0
	transmit_pkt.msg.ext_msg.data[4] = g_buckboost.adc_vbat;
	transmit_pkt.msg.ext_msg.data[5] = g_buckboost.adc_vbat >> 8;
	transmit_pkt.msg.ext_msg.data[6] = 0x00;
	transmit_pkt.msg.ext_msg.data[7] = 0x00;
	//message1
	transmit_pkt.msg.ext_msg.data[8] = g_buckboost.adc_ibat;
	transmit_pkt.msg.ext_msg.data[9] = g_buckboost.adc_ibat >> 8;
	transmit_pkt.msg.ext_msg.data[10] = 0x00;
	transmit_pkt.msg.ext_msg.data[11] = 0x00;
	//message2
	transmit_pkt.msg.ext_msg.data[12] = (uint8_t)300;
	transmit_pkt.msg.ext_msg.data[13] = (uint8_t)(300 >> 8);
	transmit_pkt.msg.ext_msg.data[14] = 0x00;
	transmit_pkt.msg.ext_msg.data[15] = 0x00;
	//message3
	transmit_pkt.msg.ext_msg.data[16] = 98; // 98%
	transmit_pkt.msg.ext_msg.data[17] = 0x01; //2
	transmit_pkt.msg.ext_msg.data[18] = 0xFF;
	transmit_pkt.msg.ext_msg.data[19] = 0xFF;
	//message4
	transmit_pkt.msg.ext_msg.data[20] = gd->real_soc_show;
	transmit_pkt.msg.ext_msg.data[21] = 0xFF;
	transmit_pkt.msg.ext_msg.data[22] = 0xFF;
	transmit_pkt.msg.ext_msg.data[23] = 0xFF;
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_uvdm_analyze(struct usb_pd_pkt_t *pkt)
{

	//uint16_t ext_head = pkt->msg.ext_msg.ext_hrd.WORD;
	uint32_t vdm_head = (uint32_t)(pkt->msg.ext_msg.data[0] | pkt->msg.ext_msg.data[1] << 8 | pkt->msg.ext_msg.data[2] << 16 | pkt->msg.ext_msg.data[3] << 24);

	#define XIAMI_VID  0x0000
	//if(vdm_head >> 16 == XIAMI_VID)
	#ifndef BIT
	#define BIT(n) (0x01ul << (n))
	#endif
	if((vdm_head & 0x03) == UVDM_GET && (vdm_head & BIT(15)) == 0)  //UVDM
	{
		uint32_t batinfo_type = vdm_head >>2  & 0x0f;
		switch(batinfo_type)
		{
			case PowerBankBattery_Basic_Info:

				break;
			case PowerBankBattery_Realtime_Info:
				hal_tcpc_uvdm_send_PowerBankBattery_Realtime_Info();
				break;
			case PowerBankBattery_Abnormal_Info:
				break;
		}
	}

}

typedef union
{
uint32_t object[5];
uint8_t byte[20];
struct
{
uint16_t  PresentCapacity; //���ꨮ����?��?�㨴��?���� �̣�??0.1%
uint16_t  Voltage;         //��?3?��??1      �̣�??mV
int16_t   Current;         //��?3?��?���¨�D��??����??y?a3?��??o?a��?��?      �̣�??mA
int16_t   BatteryTemperature;   //��?3????��      �̣�??0.01?��
uint16_t  Cycle;           //?-?����?��y
uint16_t  AbnormalAlarmCount; //����3�����?����?��y
uint8_t   Cell;   			// ��?3?��?��a��y��?
uint8_t   Reserved1[7];   // ���ꨢ??��
} bat_byte;
}USBPD_VDM_BatteryData_TypeDef;

void hal_tcpc_uvdm_send_bat_data(void)
{
	USBPD_VDM_BatteryData_TypeDef bat_data;
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	osal_mem_clear(&bat_data,sizeof(USBPD_VDM_BatteryData_TypeDef));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 7);
	transmit_pkt.msg_len = 7;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0xFF004220;
    transmit_pkt.msg.WORDS[1] = 0x1234abcd;
    bat_data.bat_byte.PresentCapacity = gd->real_soc_show * 10;
    bat_data.bat_byte.Voltage = g_buckboost.adc_vbat;
    bat_data.bat_byte.Current = g_buckboost.adc_ibat;
    bat_data.bat_byte.BatteryTemperature = 3000;
    bat_data.bat_byte.Cycle = 2;
    bat_data.bat_byte.Cell = 1;
    osal_mem_copy(&transmit_pkt.msg.WORDS[2],&bat_data,sizeof(USBPD_VDM_BatteryData_TypeDef));
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_uvdm_send_vendor_string(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 7);
	transmit_pkt.msg_len = 7;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0xFF004250;
    transmit_pkt.msg.WORDS[1] = 0x1234abcd;
    osal_mem_copy(&transmit_pkt.msg.WORDS[2],"NuVolta",sizeof("NuVolta"));
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}

void hal_tcpc_uvdm_send_product_string(void)
{
	osal_mem_clear(&transmit_pkt,sizeof(struct usb_pd_pkt_t));
	transmit_pkt.hdr.WORD = PD_HEADER_LE(PD_DATA_VENDOR_DEF, g_tcpc.pwr_role, g_tcpc.data_role, g_usb_pd_s.nego_revision, g_usb_pd_s.tx_sop_msgid, 7);
	transmit_pkt.msg_len = 7;
	tcpc_transmit_retry_cnt = (g_usb_pd_s.nego_revision == PD_REV30)? 2 : 3;
	transmit_pkt.msg.WORDS[0] = 0xFF004260;
    transmit_pkt.msg.WORDS[1] = 0x1234abcd;
    osal_mem_copy(&transmit_pkt.msg.WORDS[2],"Nu17113",sizeof("Nu17113"));
	hal_tcpc_pkt_transmit(Transmit_SOP,&transmit_pkt);
}


typedef union
{
    uint32_t object[4];
    uint8_t  byte[16];
    struct
    {
        uint8_t  Nnmber;
        uint8_t  Reserved1;
        uint16_t Voltage;
        uint32_t OverVoltage :1;
        uint32_t UnderVoltage :1;
        uint32_t OverCurrent :1;
        uint32_t OverTemperature :1;
        uint32_t  Reserved2:24;
    } alarm;
}USBPD_VDM_BatteryAbnormalAlarm_TypeDef;

typedef union
{
    uint32_t object[5];
    uint8_t byte[20];
    struct
    {
        uint16_t Cell[10];
    } cell;
}USBPD_VDM_BatteryCell_TypeDef;

typedef union
{
uint32_t object[5];
uint8_t byte[20];
struct
{
uint32_t DesignCapacity;
uint32_t FullCapacity;
uint32_t PresentCapacity;
uint8_t  Day;
uint8_t  Hour;
uint8_t  Minute;
uint8_t  Second;
uint32_t  Reserved1;
} cap;
}USBPD_VDM_BatteryCapacity_TypeDef;


bool is_power_z;

void hal_tcpc_uvdm_analyze_for_powerz(struct usb_pd_pkt_t *pkt)
{

	//uint16_t ext_head = pkt->msg.ext_msg.ext_hrd.WORD;
	uint32_t vdm_head = pkt->msg.WORDS[0];

	switch(vdm_head)
	{
		case 0xFF000220:
			is_power_z = 1;
			break;
		case 0xFF000230:
			//hal_tcpc_uvdm_send_warming_Info();
			break;
		case 0xFF000240:
			//hal_tcpc_uvdm_send_bat0_Info();
			break;
		case 0xFF000241:
			//hal_tcpc_uvdm_send_bat10_Info();
			break;
		case 0xFF000248:
		case 0xFF000249:
		case 0xFF00024a:
		case 0xFF00024b:
		case 0xFF00024c:
		case 0xFF00024d:
		case 0xFF00024e:
		case 0xFF00024f:
			//hal_tcpc_uvdm_send_bat10_Info();
			break;
		case 0xFF000250:
			hal_tcpc_uvdm_send_vendor_string();
			break;
		case 0xFF000260:
			hal_tcpc_uvdm_send_product_string();
			break;
	}

}



