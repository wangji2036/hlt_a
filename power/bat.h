#ifndef BAT_H_
#define BAT_H_

#include "config.h"

#if SUPPORT_BAT_LOG
	#define bat_printk 	printk
#else
	#define bat_printk(...)
#endif

struct bat_info
{
    int16_t vbat;   //mV
    int16_t ibat;   //mA
    int16_t rbat;   //mR
    int16_t sbat;   //0: 放电  1: 充电  2: 充满
    uint8_t bat_cycle_n;
    bool bat_soe_calied;
    bool bat_soe_in_cali;
    uint32_t bat_is_inited;
    int8_t bat_level_ocv;
    int8_t bat_level_soe;
    int8_t bat_level_ui;
    int8_t bat_level_disg_ey_s;
    int8_t bat_level_disg_ui_s;
    int8_t bat_level_chng_ey_s;
    int8_t bat_level_chng_ui_s;
    int32_t bat_energy_total;
    int32_t bat_energy_current;
    int32_t bat_energy_disgstart;
    int32_t bat_energy_cali;
};

void battery_task_handle(void);
extern struct bat_info g_bat;

#endif
