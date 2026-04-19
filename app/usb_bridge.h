#ifndef USB_BRIDGE_H_
#define USB_BRIDGE_H_

#include "typdef.h"
#include "config.h"

#if CONFIG_USB_BRIDGE_ENABLE

/* ===== I2C Address ===== */
#define USBD_WB7720_ADDR        0x21    // WB7720 I2C 7-bit address (spec: 0x42)

/* ===== Telemetry Data Region (NU17112 -> WB7720) ===== */
#define REG_SOC_PCT             0x00
#define REG_CAPACITY_MAH        0x01    // u32 LE, 4 bytes
#define REG_VBAT_MV             0x05    // u16 LE
#define REG_IBAT_MA             0x07    // s16 LE
#define REG_TEMP_DC             0x09    // s16 LE, 0.1 deg C
#define REG_CYCLE_COUNT         0x0B    // u16 LE
#define REG_R_INTERNAL_MOHM     0x0D    // u16 LE
#define REG_SOH_PCT_X100        0x0F    // u16 LE
#define REG_ERR_OVERTEMP_CNT    0x11    // u16 LE
#define REG_ERR_OVERVOLT_CNT    0x13    // u16 LE
#define REG_ERR_OVERCURR_CNT    0x15    // u16 LE
#define REG_CHARGE_STATE        0x17    // u8: 0=idle, 1=charging, 2=discharging
#define REG_CELL_COUNT          0x2C    // u8: fixed 1 (single cell)
#define REG_CELL1_VOLTAGE_MV    0x2D    // u16 LE
#define REG_CELL2_VOLTAGE_MV    0x2F    // u16 LE
#define REG_PCB_TEMP_DC         0x35    // s16 LE, 0.1 deg C

/* ===== Exception Log Rolling Write Region (NU17112 -> WB7720) ===== */
#define REG_EXC_TOTAL_COUNT     0x37    // u8: 0~5
#define REG_EXC_CURRENT_IDX     0x38    // u8: 0~4
#define REG_EXC_READY           0x39    // u8: 0xA5=数据有效可读, 0x00=写入中
#define REG_EXC_RECORD          0x3A    // 20 bytes: BatteryExceptionRecord_t

/* ===== Sleep/Wake Commands ===== */
#define REG_SLEEP_CMD           0x4E    // write 0x01 -> WB7720 Stop sleep
#define REG_WAKEUP_CMD          0x4F    // write 0x01 -> WB7720 USB re-enumerate

/* ===== Time Sync Register ===== */
#define REG_TIME_SYNC           0x51    // u8: host/WB7720 置 0xCA；MCU 读到后按 REG_ENG_CURRENT_DATE 同步 Bat_RTC_Seconds 并写回 0
#define TIME_SYNC_MAGIC         0xCA
#define REG_RTC_SECONDS         0x52    // u32 LE: Bat_RTC_Seconds (powerbank → WB7720 → host)

/* ===== Engineering Mode Registers ===== */
#define REG_WORK_MODE           0x50    // u8: 0x00=user, 0xA5=eng mode
#define REG_ENG_CURRENT_DATE    0x60    // 7 bytes: YY YY MM DD HH mm ss
#define REG_ENG_PROD_DATE       0x70    // 4 bytes: YY YY MM DD
#define REG_ENG_CYCLE_COUNT     0x80    // u16 LE
#define REG_ENG_VIRTUAL_CELL1   0x82    // u16 LE, 0xFFFF=use real value
#define REG_ENG_VIRTUAL_CELL2   0x84    // u16 LE, 0xFFFF=use real value
#define REG_ENG_VIRTUAL_TEMP    0x86    // s16 LE, 0x7FFF=use real value
#define REG_ENG_ERASE_CMD       0x88    // u8: 0xEE=erase, 0xAA=refresh
#define REG_ENG_CMD_STATUS      0x89    // u8: 0x00=idle, 0x01=busy, 0x02=ok, 0xFF=fail

/* ===== Production Mode Registers ===== */
#define PROD_MODE_FLAG          0x90    // u8: 0xB5=enter production mode
#define PROD_WRITE_STATUS       0x91    // u8: 0x01=busy, 0x02=ok, 0xFF=fail
#define PROD_MANUFACTURER       0x92    // 20 bytes
#define PROD_MODEL              0xA6    // 20 bytes
#define PROD_BATTERY_MFR        0xBA    // 20 bytes
#define PROD_BATTERY_MODEL      0xCE    // 20 bytes
#define PROD_PROD_DATE          0xE2    // 20 bytes
#define PROD_SERIAL             0x18    // 20 bytes (serial number)

/* ===== Engineering Mode Magic Numbers ===== */
#define ENG_MODE_MAGIC          0xA5
#define PROD_MODE_MAGIC         0xB5
#define ENG_CMD_ERASE_ALL       0xEE
#define ENG_CMD_REFRESH         0xAA
#define ENG_STATUS_IDLE         0x00
#define ENG_STATUS_BUSY         0x01
#define ENG_STATUS_OK           0x02
#define ENG_STATUS_FAIL         0xFF

/* ===== Virtual Parameter Sentinel Values ===== */
#define VIRTUAL_CELL_SENTINEL   0xFFFF  // do not override, use real ADC
#define VIRTUAL_TEMP_SENTINEL   0x7FFF  // do not override, use real NTC

/* ===== Public API ===== */

/** Initialize USB Bridge module (called in fml_task_init) */
void usb_bridge_init(void);

/** 47ms periodic call (FML_TASK APL_HID_REPORT event) */
void usb_bridge_periodic_update(void);

/** WB7720 sleep (USB disconnect) */
void usb_bridge_sleep(void);

/** WB7720 wakeup (USB re-enumerate) + ProductInfo resync */
void usb_bridge_wakeup(void);

/** Write ProductInfo from Flash to i2c_buff (0x92~0xF5 + 0x18~0x2B) */
void usb_bridge_reset_product_info(void);

#endif /* CONFIG_USB_BRIDGE_ENABLE */
#endif /* USB_BRIDGE_H_ */
