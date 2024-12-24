#ifndef SW7201_H_
#define SW7201_H_

#include "buckboost.h"

#define SW7201_I2C_DEV_ADDR		0x3C

extern int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
extern int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);

#define REG_Version_info						0x01
#define REG_IRQ_EN_1							0x02
#define REG_IRQ_EN_2							0x03
#define REG_IRQ_Event1							0x04
#define REG_IRQ_Event2							0x05
#define REG_System_Status						0x06
#define REG_discharge_Control 					0x0C
#define REG_Mode_Control						0x0D
#define REG_IC_Reset_Check						0x0F
#define REG_ADC_Data_Type						0x10
#define REG_ADC_Data_High						0x11
#define REG_ADC_Data_Low						0x12
#define REG_Indt_Control						0x18
#define REG_Powerpath_Control 					0x19
#define REG_Discharge_Setting1 					0x20
#define REG_Discharge_Setting2 					0x21
#define REG_Discharge_Setting3 					0x22
#define REG_Discharge_Vbus_Vol_High 			0x23
#define REG_Discharge_Vbus_Vol_Low 				0x24
#define REG_Discharge_Ibus_Limit				0x25
#define REG_Discharge_Ibat_Limit				0x26
#define REG_Vin_Uvlo							0x27
#define REG_Vin_Uvlo_Hys 						0x28
#define REG_Charger_Setting1 					0x30
#define REG_Charger_Setting2 					0x31
#define REG_Charger_Setting3 					0x32
#define REG_Charger_Setting4 					0x33
#define REG_Charger_VbatVol_High 				0x34
#define REG_Charger_VbatVol_Low 				0x35
#define REG_Trickle_Vol 						0x36
#define REG_Trickle_Vol_Hys 					0x37
#define REG_Charger_HoldVol 					0x38
#define REG_Charger_Ibus_Limit 					0x39
#define REG_Charger_Ibat_Limit 					0x3A
#define REG_Discharge_Setting4 					0x40
#define REG_Ntc_Setting1 						0x43


void hal_sw7201_buckboost_init(void);
void hal_sw7201_buckboost_set_mode(enum buckboost_mode woke_mode);
void hal_sw7201_buckboost_set_busiv(uint16_t vbus,uint16_t ibus);
void hal_sw7201_buckboost_discharge_set_bat_uv_volt(uint16_t volt);
void hal_sw7201_buckboost_typeca_gate_en(bool en);
void hal_sw7201_buckboost_typecb_gate_en(bool en);
void hal_sw7201_buckboost_charge_ibus_limit(uint16_t ibus_limit);
void hal_sw7201_buckboost_charge_ibat_limit(uint16_t ibat_limit);
void hal_sw7201_buckboost_write_reset_check(void);
void hal_sw7201_buckboost_dis_indetb(void);
void hal_sw7201_buckboost_charge_vbus_uv(uint16_t vbus_uv);
void hal_sw7201_buckboost_charge_target_volt(uint16_t volt);
void hal_sw7201_buckboost_charge_set_trickle_volt(uint16_t volt);
uint8_t hal_sw7201_buckboost_get_verision(void);
uint16_t hal_sw7201_buckboost_get_bus_current(void);
int16_t hal_sw7201_buckboost_get_bat_current(void);
void hal_sw7201_buckboost_usb_a_gate_en(bool en);
uint16_t hal_sw7201_buckboost_get_bat_voltage(void);
uint16_t hal_sw7201_buckboost_get_bus_voltage(void);
void hal_sw7201_buckboost_a2_detect_enable(void);
bool hal_sw7201_buckboost_get_a2_state(void);
void hal_sw7201_buckboost_typeca_dischg(bool en);
void hal_sw7201_buckboost_typecb_dischg(bool en);
void hal_sw7201_buckboost_usb_a_dischg(bool en);
void hal_sw7201_buckboost_vbus_dischg(bool en);
uint16_t hal_sw7201_buckboost_get_bat_temperature(void);

#endif /* SW7201_H_ */
