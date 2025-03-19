#ifndef BUCK_BOOST_H_
#define BUCK_BOOST_H_

#include "typdef.h"
#include "osal.h"

#define BUCKBOOST_USED_SW7201		0
#define BUCKBOOST_USED_NU6801		1

#if(BUCKBOOST_USED_SW7201 == 1)
	#define BAT_DEAD_BATTER_V   	6000
	#define BAT_ACTIVE_RBATTER_V   	6500
#elif(BUCKBOOST_USED_NU6801 == 1)
	#define BAT_DEAD_BATTER_V   	3000
	#define BAT_ACTIVE_RBATTER_V   	3250
#endif

enum buckboost_mode
{
	BUCKBOOST_SHUTDOWM_MODE = 0,
	BUCKBOOST_CHAGER_MODE = 1,
	BUCKBOOST_DISCHG_MODE,
};

struct buckboost_s
{
	enum buckboost_mode woke_mode;
	bool set_typeca_gate_en;
	bool set_typecb_gate_en;
	bool set_usb_a_gate_en;
	bool bat_full_flag;
	bool usba_dectet_en;
	bool usba_state;

	uint16_t buckboost_out_voltage;
	uint16_t buckboost_out_current;
	uint16_t out_voltage_wait;
	uint16_t out_voltage_delay;
	uint16_t regulator_state;
	uint16_t buckboost_chager_current;
	uint16_t chager_ibus_limit;
	uint16_t chager_ibat_limit;

	int16_t adc_ibus;
	int16_t adc_ibat;

	uint16_t adc_vbat;
	uint16_t adc_tbat;
	uint16_t adc_vbus;
	uint16_t ir_drop;
	uint8_t ibus_cc_flag;
#if(BUCKBOOST_USED_NU6801 == 1)
	uint16_t adc_iac1;
	uint8_t charging_stat; //ÊÇ·ñÔÚ³äµç
#endif
	uint8_t protect_status;
};

struct buckboost_operations
{

	void (*init)(void);
	void (*set_work_mode)(enum buckboost_mode woke_mode);
	void (*set_out)(uint16_t out_voltage,uint16_t out_current);
	void (*typca_gate_en)(bool en);
	void (*typcb_gate_en)(bool en);
	void (*usb_a_gate_en)(bool en);

	void (*typca_dischg_en)(bool en);
	void (*typcb_dischg_en)(bool en);
	void (*usb_a_dischg_en)(bool en);
	void (*vbus_dischg_en)(bool en);
	bool (*en_a2_detect)(bool en);


	void (*set_chager_current)(uint16_t current);
	void (*set_chager_ibus_limit)(uint16_t current);
	void (*set_chager_ibat_limit)(uint16_t current);
	int16_t (*get_bus_current)(void);
	int16_t (*get_bat_current)(void);
	uint16_t (*get_bat_voltage)(void);
	uint16_t (*get_bus_voltage)(void);
	bool (*get_a2_state)(void);
	uint16_t (*get_bat_temperature)(void);
	uint8_t (*get_protect_status)(void);
	uint8_t (*is_ibus_loop)(void);

#if(BUCKBOOST_USED_NU6801 == 1)
	uint16_t (*get_typeca_vbus_present)(void);
	uint16_t (*get_typecb_vbus_present)(void);
	uint8_t (*get_charge_flag)(void);
	uint16_t (*get_adc_iac1)(void);
	void (*set_ovp)(void);
#endif

};


#define BUCKBOOST_TIME_PERIOD									23
#define BUCKBOOST_VBUS_PERIOD									10

#define BUCKBOOST_EVT_SWITCH_WORK_MODE    						osal_event_declare(0)
#define BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT    					osal_event_declare(1)
#define BUCKBOOST_EVT_SET_CHARGER_CURRENT    					osal_event_declare(2)
#define BUCKBOOST_EVT_SET_DISCHG_IBUS_LIMIT    					osal_event_declare(3)
#define BUCKBOOST_EVT_SET_CHAGER_IBUS_LIMIT    					osal_event_declare(4)
#define BUCKBOOST_EVT_SET_CHAGER_IBAT_LIMIT    					osal_event_declare(5)
#define BUCKBOOST_EVT_SET_VBUS_DUMMYLOAD_DISCHG    				osal_event_declare(6)
#define BUCKBOOST_EVT_SET_TYPECA_GATE_EN    					osal_event_declare(7)
#define BUCKBOOST_EVT_REGULATOR_WAITDONE    					osal_event_declare(8)
#define BUCKBOOST_EVT_REGULATOR_DELAYDONE    					osal_event_declare(9)
#define BUCKBOOST_EVT_SET_TYPECB_GATE_EN    					osal_event_declare(10)
#define BUCKBOOST_EVT_SET_USB_A_GATE_EN    						osal_event_declare(11)

#define BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_EN    				osal_event_declare(12)
#define BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_EN    				osal_event_declare(13)
#define BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_DIS    				osal_event_declare(14)
#define BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_DIS    				osal_event_declare(15)

#define BUCKBOOST_EVT_VBUS_PERIOD    							osal_event_declare(30)
#define BUCKBOOST_EVT_TIME_PERIOD    							osal_event_declare(31)


void buckboost_task_init(void);
void buckboost_task_event_handler(uint32_t event);

void buckboost_set_bus_iv(uint16_t voltage,uint16_t current,uint16_t wait, uint16_t delay);
void buckboost_set_charge_current(uint16_t ibat,uint16_t ibus);
void buckboost_set_work_mode(enum buckboost_mode mode);
void buckboost_set_gate_en(uint8_t tc_index,bool en);
void buckboost_set_typeca_gate_en(bool en);
void buckboost_set_typecb_gate_en(bool en);
void buckboost_set_usb_a_gate_en(bool en);
bool buckboost_regulator_done(void);

extern struct buckboost_s  g_buckboost;
extern const struct buckboost_operations buckboost_ops;
extern bool ntc_ut_flag;
extern bool ntc_ot_flag;
extern bool ntc_stop_chrg_flag;

#endif /* BUCK_BOOST_H_ */
