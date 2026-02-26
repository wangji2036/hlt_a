#ifndef USB_BRIDGE_H_
#define USB_BRIDGE_H_

#include "typdef.h"
#include "config.h"

#if CONFIG_USB_BRIDGE_ENABLE

/*===================== WB7720 I2C Configuration =====================*/
#define USB_BRIDGE_WB7720_ADDR      0x21    // WB7720 I2C 7-bit address (0x42 on wire)

/*===================== I2C Register Addresses =====================*/

/* --- Telemetry Region (NU17112 writes, 0x00-0x36) --- */
#define REG_SOC_PCT                 0x00    // u8, %
#define REG_CAPACITY_MAH            0x01    // u32 LE, mAh
#define REG_VBAT_MV                 0x05    // u16 LE, mV
#define REG_IBAT_MA                 0x07    // s16 LE, mA
#define REG_TEMP_DC                 0x09    // s16 LE, 0.1 degC
#define REG_CYCLE_COUNT             0x0B    // u16 LE, cycles
#define REG_R_INTERNAL_MOHM         0x0D    // u16 LE, mohm
#define REG_SOH_PCT_X100           0x0F    // u16 LE, pct x100
#define REG_ERR_OVERTEMP_CNT        0x11    // u16 LE, count
#define REG_ERR_OVERVOLT_CNT        0x13    // u16 LE, count
#define REG_ERR_OVERCURR_CNT        0x15    // u16 LE, count (reserved=0)
#define REG_CHARGE_STATE            0x17    // u8, 0=standby 1=charge 2=discharge
#define REG_CELL_COUNT              0x2C    // u8, cell count
#define REG_CELL1_VOLTAGE_MV        0x2D    // u16 LE, mV
#define REG_CELL2_VOLTAGE_MV        0x2F    // u16 LE, mV
#define REG_PCB_TEMP_DC             0x35    // s16 LE, 0.1 degC

/* --- Exception Log Region (0x37-0x4D, 23 bytes) --- */
#define REG_EXC_TOTAL_COUNT         0x37    // u8, total valid records (0-5)
#define REG_EXC_CURRENT_IDX         0x38    // u8, current record index (0-4)
#define REG_EXC_RESERVED            0x39    // u8, reserved = 0x00
#define REG_EXC_RECORD_DATA         0x3A    // 20B, BatteryExceptionRecord_t

/* --- Sleep/Wakeup Commands (0x44-0x45) --- */
#define REG_SLEEP                   0x44    // u8, write 0x01 to enter sleep
#define REG_WAKEUP                  0x45    // u8, write 0x01 to wakeup

/* --- Engineering Mode Registers (0x50-0x81) --- */
#define REG_WORK_MODE               0x50    // u8, 0x00=user 0xA5=engineering
#define REG_ENG_CURRENT_DATE        0x60    // 4B: Year(u16 LE) + Month(u8) + Day(u8)
#define REG_ENG_PRODUCTION_DATE     0x70    // 4B: Year(u16 LE) + Month(u8) + Day(u8)
#define REG_ENG_CYCLE_CHG_COUNT     0x80    // u16 LE, cycle count

/* --- Production Mode / Device Info Registers (0x90-0xF5) --- */
#define REG_PROD_MODE_FLAG          0x90    // u8, 0xB5=enter production mode
#define REG_PROD_WRITE_STATUS       0x91    // u8, 0x01=busy 0x02=success 0xFF=fail
#define REG_PROD_MANUFACTURER       0x92    // 20B ASCII
#define REG_PROD_MODEL              0xA6    // 20B ASCII
#define REG_PROD_BATTERY_MFR        0xBA    // 20B ASCII
#define REG_PROD_BATTERY_MODEL      0xCE    // 20B ASCII
#define REG_PROD_PROD_DATE          0xE2    // 20B ASCII

/*===================== Timing Constants =====================*/
#define EXC_WRITE_INTERVAL          64      // Exception log write interval in round-robin steps
                                            // 64 x 47ms ≈ 3 seconds per record rotation

/*===================== Function Declarations =====================*/

/* --- Initialization (called from wb7720_init) --- */
void usb_bridge_init(void);

/* --- Telemetry Extensions (called from round-robin) --- */
void usb_bridge_write_exception_counts(void);   // Step 11: OT/OV/OC counts -> 0x11-0x16
void usb_bridge_write_charge_state(void);        // Step 12: charge state -> 0x17

/* --- Exception Log (called every round-robin step, internal rate control) --- */
void usb_bridge_write_exception_record(void);    // Step 13: rolling exception record

/* --- Mode Handling (called from round-robin) --- */
void usb_bridge_check_engineering_mode(void);    // Step 14: virtual value override/restore
void usb_bridge_check_production_mode(void);     // Step 15: Flash ProductInfo write

/* --- Engineering Mode Query --- */
bool usb_bridge_is_eng_mode(void);

#endif /* CONFIG_USB_BRIDGE_ENABLE */
#endif /* USB_BRIDGE_H_ */
