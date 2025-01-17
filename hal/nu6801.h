#ifndef NU6801_H_
#define NU6801_H_

#include "buckboost.h"

#define NU6801_I2C_DEV_ADDR		0x66

extern int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
extern int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);


#define REG_INT_FLAG					0x00
#define REG_INT_MASK					0x01
#define REG_VAC_PLUG_IN_STAT			0x02
#define REG_MAIN_STAT					0x03
#define REG_TEMP_STAT					0x04
#define REG_CHG_FLAG					0x05
#define REG_BUBO_FAULT_FLAG				0x06
#define REG_AC_DET_CTRL					0x07
#define REG_VAC_DRV_CTRL				0x08
#define REG_BUBO_CTRL					0x09
#define REG_VBAT_CTRL					0x0A
#define REG_IBAT_CTRL					0x0B
#define REG_VBUS_SET_H					0x0C
#define REG_VBUS_SET_L					0x0D
#define REG_IBUS_SET					0x0E
#define REG_IR_COMP						0x0F
#define REG_MISC_CTRL					0x10
#define REG_AMUX_CTRL					0x11
#define REG_REVISION					0x12

void hal_nu6801_buckboost_init(void);
void hal_nu6801_buckboost_set_cv(void);
void hal_nu6801_buckboost_typeca_dischg(bool en);
void hal_nu6801_buckboost_typecb_dischg(bool en);
void hal_nu6801_buckboost_usb_a_dischg(bool en);
void hal_nu6801_buckboost_vbus_dischg(bool en);
bool hal_nu6801_buckboost_usba_detect_enable(bool en);
bool hal_nu6801_buckboost_get_usba_state(void);
void hal_nu6801_buckboost_set_mode(enum buckboost_mode woke_mode);
void hal_nu6801_buckboost_set_busiv(uint16_t vbus,uint16_t ibus);
void hal_nu6801_buckboost_typeca_gate_en(bool en);
void hal_nu6801_buckboost_usb_a_gate_en(bool en);
void hal_nu6801_buckboost_typecb_gate_en(bool en);
void hal_nu6801_buckboost_charge_ibus_limit(uint16_t ibus_limit);
void hal_nu6801_buckboost_charge_ibat_limit(uint16_t ibat_limit);
int16_t hal_nu6801_buckboost_get_bus_current(void);
int16_t hal_nu6801_buckboost_get_bat_current(void);
uint16_t hal_nu6801_buckboost_get_bat_voltage(void);
uint16_t hal_nu6801_buckboost_get_bus_voltage(void);
uint16_t hal_nu6801_buckboost_get_bat_temperature(void);
uint8_t hal_nu6801_buckboost_get_verision(void);
void hal_nu6801_buckboost_write_reset_check(void);
void hal_nu6801_buckboost_dis_indetb(void);
void hal_nu6801_buckboost_charge_vbus_uv(uint16_t vbus_uv);
void hal_nu6801_buckboost_charge_target_volt(uint16_t volt);
void hal_nu6801_buckboost_charge_set_trickle_volt(uint16_t volt);
void hal_nu6801_buckboost_discharge_set_bat_uv_volt(uint16_t volt);
void hal_nu6801_buckboost_wake_up(void);
uint8_t hal_nu6801_buckboost_get_protect(void);

#endif /* SW7201_H_ */
