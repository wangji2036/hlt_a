#ifndef WPC_NEGO_H_
#define WPC_NEGO_H_

#include "osal.h"
#include "pkt_type.h"
#include "_wpc.h"

struct power_transfer_contract_t
{
//	uint8_t ref_power;
//	uint8_t wnd_size;
//	uint8_t wnd_ofs;
//	uint8_t holdoff_time;
//	uint8_t rpp_type;
//	uint8_t fsk_config;
//	uint8_t pot_power;
//	uint8_t gtd_power;

	uint8_t rep_delay; //unit: 100ms, default value is 5.
	uint8_t pch_delay; //unit:   1ms, default value is 5.
	uint8_t freq_sel;  //default value is 0.
	uint8_t ext_nego_power; //unit: 100mW, default value is 50.
	uint8_t power_control_profile; //default value is 0.
	uint8_t cloak_det_ping_delay; //unit: 100ms, default value is 0.
   uint16_t cloak_dig_ping_delay; //unit: 100ms, default value is 5.
};
void power_contract_init(void);
extern struct power_transfer_contract_t ptx_power_contract, prx_power_contract;

void wpc_nego_phase_process(struct com_prx_ask_pkt_t *com_ask);

#endif /* WPC_NEGO_H_ */
