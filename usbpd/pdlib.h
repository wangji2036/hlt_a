#ifndef PDLIB_H_
#define PDLIB_H_

#include "tcpm.h"

#define USB_PD_EVT_SNK_ATTACHED  					(0x01ul<<0)
#define USB_PD_EVT_SNK_UNATTACH  					(0x01ul<<1)
#define USB_PD_EVT_SRC_ATTACHED  					(0x01ul<<2)
#define USB_PD_EVT_SRC_UNATTACH  					(0x01ul<<3)
#define USB_PD_EVT_SNK_SET_VOLTAGE  				(0x01ul<<8)  //include tx_discard tx_fald tx_timeout
#define USB_PD_EVT_SOURCE_SOFTRESET 				(0x01ul<<10)

void usb_pdlib_timer_update();
void pdlib_init(void);
void pdlib_run(void);
bool pdlib_is_connect(void);
bool pdlib_is_pps_sink(void);
bool pdlib_is_pps_source(void);
void pdlib_disable_typec(uint8_t index);
void pdlib_restart_typec(uint8_t index);
void pdlib_disable_usbpd();

bool pdlib_get_deadbat(void);
void pdlib_set_deadbat(bool dead);

enum usb_tc_state_e pdlib_get_tc_state(uint8_t index);
void pdlib_clear_typec_prswap(uint8_t index);
uint16_t pdlib_get_source_supply_voltage(void);
uint16_t pdlib_get_source_supply_current(void);
void pdlib_set_pd_event(uint8_t tc_index,uint32_t event);
enum pwr_role_e pdlib_get_pwr_role(void);
enum data_role_e pdlib_get_date_role(void);
void pdlib_update_source_pdo(const uint32_t* pdo,uint8_t pdo_n);
void pdlib_update_sink_pdo(const uint32_t* pdo,uint8_t pdo_n);
void pdlib_snk_requsrt_voltage(uint8_t pdo_index,uint16_t voltage,uint16_t current);
uint8_t pdlib_snk_get_work_pdo_index(void);
uint32_t pdlib_snk_get_work_pdo(void);
uint8_t pdlib_get_port_map(void);
uint8_t pdlib_snk_get_pdo_amount(void);
uint32_t pdlib_snk_get_pdo_by_index(uint8_t index);

void pdlib_tcpc_get_cc(uint8_t tc_index,enum tc_cc_status *cc1, enum tc_cc_status *cc2);
enum tc_drp_reult pdlib_get_drp_toggle_result(uint8_t tc_index);
void pdlib_tcpc_set_cc(uint8_t tc_index,enum tc_cc_status cc);
void pdlib_set_pd_port(uint8_t tc_index);

#endif /* TCPC_H_ */
