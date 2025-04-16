#ifndef TCPC_H_
#define TCPC_H_

#include "buckboost.h"


bool hal_tcpc_vbus_is_present(uint8_t tc_index);
void hal_tcpc_pd_set_bus_iv(uint8_t tc_index,uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay);
bool hal_tcpc_pd_bus_ready(uint8_t tc_index);
bool hal_tcpc_vbus_is_removed(uint8_t tc_index);
bool hal_tcpc_vbus_is_vsfae0v(uint8_t tc_index);
bool hal_tcpc_vbus_is_vsafe5v(void);
void hal_tcpc_port_dummyload_en(uint8_t tc_index,bool en);
void hal_tcpc_set_gate_en(uint8_t tc_index,bool en);
void hal_tcpc_set_source_mode(enum buckboost_mode mode);
void hal_tcpc_set_snk_charge_current(uint16_t ibat,uint16_t ibus);

#endif /* TCPC_H_ */
