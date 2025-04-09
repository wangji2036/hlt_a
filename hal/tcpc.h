#ifndef TCPC_H_
#define TCPC_H_

#include "pd.h"
#include "buckboost.h"
#include "usbpd_config.h"


#define RP_VALUE_DEFAULT            (0)
#define RP_VALUE_1A5                (1)
#define RP_VALUE_3A0                (2)
#define RP_VALUE_RESERVED           (3)

#define CC_STATE_RA            		(0)
#define CC_STATE_RP                	(1)
#define CC_STATE_RD                	(2)
#define CC_STATE_OPEN           	(3)

#define EN_SOP           			(0x01<<0)
#define EN_SOP1           			(0x01<<1)
#define EN_SOP2           			(0x01<<2)
#define EN_SOP1DB           		(0x01<<3)
#define EN_SOP2DB           		(0x01<<4)
#define EN_HARD_RESET           	(0x01<<5)
#define EN_CABLE_RESET           	(0x01<<6)

enum transmit_frame_type
{
	Transmit_SOP = 0,
	Transmit_SOP1,
	Transmit_SOP2,
	Transmit_SOP_DBG1,
	Transmit_SOP_DBG2,
	Transmit_HardReset,
	Transmit_CableReset,
	Transmit_BIST_CarrierMode2,
};

enum rx_vref
{
	VREF_0P30 = 0,
	VREF_0P36 = 1,
	VREF_0P43 = 2,
	VREF_0P50 = 3,
	VREF_0P56 = 4,
	VREF_0P62 = 5,
	VREF_0P68 = 6,
	VREF_0P75 = 7,
};

#define V_VBUS_PRESENT_TH			3800

enum data_role_e
{
	TYPEC_DEVICE = 0,
	TYPEC_HOST,
};

enum pwr_role_e
{
	TYPEC_SINK = 0,
	TYPEC_SOURCE,
};

enum tc_cc_status
{
    TYPEC_CC_OPEN,
    TYPEC_CC_RA,
    TYPEC_CC_RD,
    TYPEC_CC_RP_DEF,
    TYPEC_CC_RP_1_5,
    TYPEC_CC_RP_3_0,
    TYPEC_CC_TOGGLE,
};

enum tc_drp_reult
{
	TYPEC_DRP_NO_CONNECT = 0,
	TYPEC_DRP_SNK_CONNECTED,
	TYPEC_DRP_SRC_CONNECTED,
};


enum tc_cc_polarity
{
    TYPEC_POLARITY_CC1,
    TYPEC_POLARITY_CC2,
};

struct tcpc_s
{
	uint8_t tc_port_map;
	enum data_role_e data_role;
	enum pwr_role_e pwr_role;

};


extern struct usb_pd_pkt_t transmit_pkt;
extern uint8_t tcpc_transmit_retry_cnt;
extern struct tcpc_s g_tcpc;
void hal_tcpc_init(void);
bool hal_tcpc_vbus_is_present(uint8_t tc_index);

void hal_tcpc_set_data_role(uint8_t tc_index,enum data_role_e role);
void hal_tcpc_set_pwr_role(uint8_t tc_index,enum pwr_role_e role);
void hal_tcpc_set_cc(uint8_t tc_index,enum tc_cc_status cc);
void hal_tcpc_set_vconn(uint8_t tc_index,bool en);
void hal_tcpc_set_gate_en(uint8_t tc_index,bool en);
enum tc_drp_reult hal_get_drp_toggle_result(uint8_t tc_index);
void hal_tcpc_set_pd_rx(uint8_t tc_index,uint32_t sop,bool en);
void hal_tcpc_set_roles(uint8_t tc_index,enum pwr_role_e pwr_role,enum data_role_e data_role);
void hal_tcpc_set_phy_port(uint8_t tc_index);

void hal_tcpc_pkt_transmit(enum transmit_frame_type frame, struct usb_pd_pkt_t *pkt);
void hal_tcpc_set_polarity(uint8_t tc_index,enum tc_cc_polarity polarity);
void hal_tcpc_get_cc(uint8_t tc_index,enum tc_cc_status *cc1, enum tc_cc_status *cc2);
void hal_tcpc_reset_pd_phy(void);
void hal_tcpc_pd_phy_enable(void);
void hal_tcpc_pd_phy_disable(void);
void hal_tcpc_set_bist_data(bool on);
bool hal_tcpc_vbus_is_vsafe5v(void);
bool hal_tcpc_vbus_is_removed(uint8_t tc_index);
bool hal_tcpc_vbus_is_vsfae0v(uint8_t tc_index);

void hal_tcpc_pd_send_revision(void);
void hal_tcpc_send_request_mgs(uint32_t rdo);
void hal_tcpc_send_hardreset(void);
void hal_tcpc_send_bistdata(void);
void hal_tcpc_send_ctrl_mgs(enum pd_ctrl_msg_type msg_type);
void hal_tcpc_send_source_caps(uint32_t * pdos,uint32_t pdo_n);
void hal_tcpc_send_snk_caps(uint32_t * pdos,uint32_t pdo_n);
void hal_tcpc_send_sink_caps_ext(void);
void tcpc_pd_send_bat_capability(uint8_t bat_index);
void tcpc_pd_send_pps_status(void);
void hal_tcpc_port_dummyload_en(uint8_t tc_index,bool en);
void hal_tcpc_pd_set_bus_iv(uint8_t tc_index,uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay);
bool hal_tcpc_pd_bus_ready(uint8_t tc_index);
void hal_tcpc_set_source_mode(enum buckboost_mode mode);
void hal_tcpc_set_snk_charge_current(uint16_t ibat,uint16_t ibus);
void hal_tcpc_set_phy_rx_vref(enum rx_vref vref);
void hal_tcpc_pd_send_batt_status(void);
#endif /* TCPC_H_ */
