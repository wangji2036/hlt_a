#ifndef PD_TC_H_
#define PD_TC_H_

#include "pd.h"
#include "usbpd_config.h"
#include "tcpm.h"

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



extern struct usb_pd_pkt_t transmit_pkt;
extern uint8_t tcpc_transmit_retry_cnt;
extern struct tcpc_s g_tcpc;


void hal_tcpc_init(void);
void hal_tcpc_set_phy_port(uint8_t tc_index);
void hal_tcpc_set_phy_rx_vref(enum rx_vref vref);
void hal_tcpc_set_cc(uint8_t tc_index,enum tc_cc_status cc);
enum tc_cc_status hal_tcpc_to_typec_cc(uint32_t cc, bool sink);
void hal_tcpc_get_cc(uint8_t tc_index,enum tc_cc_status *cc1, enum tc_cc_status *cc2);
enum tc_drp_reult hal_get_drp_toggle_result(uint8_t tc_index);
void hal_tcpc_set_data_role(uint8_t tc_index,enum data_role_e role);
void hal_tcpc_set_pwr_role(uint8_t tc_index,enum pwr_role_e role);
void hal_tcpc_set_vconn(uint8_t tc_index,bool en);
void hal_tcpc_set_pd_rx(uint8_t tc_index,uint32_t sop,bool en);
void hal_tcpc_set_roles(uint8_t tc_index,enum pwr_role_e pwr_role,enum data_role_e data_role);
void hal_tcpc_set_polarity(uint8_t tc_index,enum tc_cc_polarity polarity);
void hal_tcpc_set_bist_data(bool on);
void hal_tcpc_reset_pd_phy(void);
void hal_tcpc_pd_phy_enable(void);
void hal_tcpc_pd_phy_disable(void);
void hal_tcpc_send_hardreset(void);
void hal_tcpc_send_bistdata(void);
void hal_tcpc_pkt_transmit(enum transmit_frame_type frame, struct usb_pd_pkt_t *pkt);
void hal_tcpc_send_request_mgs(uint32_t rdo);
void hal_tcpc_send_ctrl_mgs(enum pd_ctrl_msg_type msg_type);
void hal_tcpc_pd_send_revision(void);
void hal_tcpc_send_source_caps(uint32_t * pdos,uint32_t pdo_n);
void hal_tcpc_send_snk_caps(uint32_t * pdos,uint32_t pdo_n);
void hal_tcpc_send_sink_caps_ext(void);
void tcpc_pd_send_pps_status(void);
void hal_tcpc_pd_send_batt_status(void);
void tcpc_pd_send_bat_capability(uint8_t bat_index);
void hal_tcpc_pd_send_Alert(void);
void hal_tcpc_send_discover_Identity_Ack(void);
void hal_tcpc_send_discover_SVID_Ack(void);
void hal_tcpc_uvdm_analyze(struct usb_pd_pkt_t *pkt);
void hal_tcpc_uvdm_send_PowerBankBattery_Realtime_Info(void);
void hal_tcpc_uvdm_send_bat_data(void);
extern bool is_power_z;
void hal_tcpc_uvdm_analyze_for_powerz(struct usb_pd_pkt_t *pkt);

#endif /* TCPC_H_ */
