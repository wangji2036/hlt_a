#ifndef NU6801_H_
#define NU6801_H_

#include "buckboost.h"

#define NU6801_I2C_DEV_ADDR		0x66

extern uint8_t nu6801_adc_chennel;
extern uint16_t nu6801_vref;
enum
{
	NU6801_ADC_VBAT = 0,
	NU6801_ADC_IBAT,
	NU6801_ADC_VBUS,
	NU6801_ADC_IBUS,
	NU6801_ADC_IAC1,
	NU6801_ADC_IAC2,
	NU6801_ADC_RNTC,
	NU6801_ADC_VREF,
	NU6801_ADC_OTHER,
};

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
void hal_nu6801_buckboost_set_cv(uint16_t volt);
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
uint16_t hal_nu6801_buckboost_typeca_vbus_present(void);
uint16_t hal_nu6801_buckboost_typecb_vbus_present(void);
uint8_t hal_nu6801_buckboost_get_charge_flag(void);
uint16_t hal_nu6801_buckboost_get_iac1(void);//iac1;
uint8_t hal_nu6801_buckboost_is_ibus_loop(void);
void hal_nu6801_buckboost_enter_force_trickle(bool enter);
void hal_nu6801_buckboost_set_ovp(uint16_t set_volt);
uint8_t hal_nu6801_buckboost_get_main_stat(void);
void hal_nu6801_disable_bubo(void);
void hal_nu6801_buckboost_set_adc_channel(uint8_t channel);
void hal_nu6801_deadbat_patch(void);
void hal_nu6801_get_charge_state(void);
void nu6801_deadbat_patch(void);

extern bool nu6801_dead_bat;

#endif /* NU6805_H_ */
