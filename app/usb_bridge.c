/**
 * @file usb_bridge.c
 * @brief USB Bridge (WB7720) extended telemetry, exception log, engineering mode,
 *        and production mode implementation for NU17112 PowerBank.
 *
 * This module handles I2C communication between NU17112 and WB7720 USB Bridge MCU.
 * Functions are called from the FML_TASK round-robin (ubsd_wb7720_report_update).
 *
 * Register map reference: .claude/references/specs/WB7720_USB_Bridge_接口规范.md
 */

#include "usb_bridge.h"

#if CONFIG_USB_BRIDGE_ENABLE

#include "g_data.h"
#include "i2cm.h"
#include "printk.h"
#include "buckboost.h"
#include <string.h>
#include "bat_record.h"

/*===================== External References =====================*/
extern struct buckboost_s g_buckboost;

/*===================== Static Variables =====================*/

/* Engineering mode state */
static bool eng_mode_active = false;
static uint32_t eng_saved_rtc_seconds;       /* Real RTC seconds before override */
static uint8_t  eng_saved_cycle_count;       /* Real cycle count before override */
static uint32_t eng_entry_virtual_seconds;   /* Virtual RTC seconds at entry */
static uint8_t  eng_entry_virtual_cycle;     /* Virtual cycle count at entry */

/* Exception record rotation */
static uint16_t exc_cycle_cnt = 0;
static uint8_t  exc_rotate_idx = 0;

/* Engineering mode virtual override values */
static uint16_t eng_virtual_cell1 = 0;  /* Virtual Cell1 voltage (mV), 0=disabled */
static uint16_t eng_virtual_cell2 = 0;  /* Virtual Cell2 voltage (mV), 0=disabled */
static int16_t  eng_virtual_temp  = 0;  /* Virtual temperature (0.1°C), 0=disabled */

/*===================== Helper Functions =====================*/

/**
 * @brief Convert date (Year, Month, Day) to seconds since 2026-01-01 00:00:00.
 * @param year  Year (u16), must be >= 2026
 * @param month Month (u8), 1-12
 * @param day   Day (u8), 1-31
 * @return Seconds since 2026-01-01, or 0 if year < 2026
 */
static uint32_t date_to_seconds(uint16_t year, uint8_t month, uint8_t day)
{
    if (year < 2026) {
        return 0;
    }

    uint32_t total_days = 0;
    uint16_t y;
    uint8_t m;

    /* Add days for complete years from 2026 to (year-1) */
    for (y = 2026; y < year; y++) {
        total_days += (y % 4 == 0) ? 366 : 365;
    }

    /* Days per month (non-leap) */
    static const uint8_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    /* Add days for complete months in current year */
    for (m = 1; m < month; m++) {
        total_days += month_days[m - 1];
        if (m == 2 && (year % 4 == 0)) {
            total_days += 1; /* Leap year February */
        }
    }

    /* Add days in current month (day-1 because day 1 = offset 0) */
    total_days += (day - 1);

    return total_days * 86400UL;
}

/**
 * @brief Check if a ProductInfo buffer is valid (not all 0x00 or all 0xFF).
 * @param data  Pointer to data buffer
 * @param len   Length in bytes
 * @return true if at least one byte is neither 0x00 nor 0xFF
 */
static bool is_product_info_valid(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    bool all_zero = true;
    bool all_ff   = true;

    for (i = 0; i < len; i++) {
        if (data[i] != 0x00) all_zero = false;
        if (data[i] != 0xFF) all_ff = false;
        if (!all_zero && !all_ff) return true;
    }

    return (!all_zero && !all_ff);
}

/*===================== Public Functions =====================*/

/**
 * @brief Initialize USB Bridge module.
 *
 * Called from wb7720_init() at boot.
 * Reads ProductInfo from Flash; if valid, writes 100 bytes to WB7720
 * registers 0x92-0xF5 so the PC can read device info via HID.
 */
void usb_bridge_init(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    usb_bridge_ensure_product_info();
#endif
}

/**
 * @brief Ensure ProductInfo has been written to WB7720.
 *
 * Called from round-robin step 16 (~every 752ms).
 * If usb_bridge_init() I2C write failed (WB7720 not ready at boot),
 * this function retries until success, then stops (done=1).
 */
void usb_bridge_ensure_product_info(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    static uint8_t done = 0;
    if (done) return;
    ProductInfo_t info;
    product_info_read(&info);
    if (!is_product_info_valid((const uint8_t *)&info, sizeof(ProductInfo_t))) {
        done = 1;
        return;
    }
    if (hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                   REG_PROD_MANUFACTURER,
                                   (uint8_t *)&info,
                                   sizeof(ProductInfo_t)) == 0) {
        done = 1;
    }
#endif
}

/**
 * @brief Write exception counts (OT/OV/OC) to WB7720 registers 0x11-0x16.
 *
 * Scans ap->record_storage.records[] and counts error types:
 *   error_type 0x01 = Overvoltage  -> REG_ERR_OVERVOLT_CNT (0x13)
 *   error_type 0x02 = Overtemp     -> REG_ERR_OVERTEMP_CNT (0x11)
 *   error_type 0x03 = Undertemp    -> REG_ERR_OVERCURR_CNT (0x15) (mapped to OC slot)
 *
 * Called from round-robin step 11.
 */
void usb_bridge_write_exception_counts(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    uint16_t ot_count = 0;
    uint16_t ov_count = 0;
    uint16_t oc_count = 0; /* maps undertemp to OC slot (reserved) */
    uint8_t i;
    uint8_t valid_count;
    uint8_t buf[6];

    /* Determine how many valid records exist */
    valid_count = ap->record_storage.exception_counter;
    if (valid_count > MAX_RECORDS) {
        valid_count = MAX_RECORDS;
    }

    /* Count error types from stored records */
    for (i = 0; i < valid_count; i++) {
        switch (ap->record_storage.records[i].error_type) {
            case 0x01: /* Overvoltage */
                ov_count++;
                break;
            case 0x02: /* Over-temperature */
                ot_count++;
                break;
            case 0x03: /* Under-temperature -> mapped to OC slot */
                oc_count++;
                break;
            default:
                break;
        }
    }

    /* Pack 3 x u16 LE into buffer: OT(0x11), OV(0x13), OC(0x15) */
    buf[0] = (uint8_t)(ot_count & 0xFF);
    buf[1] = (uint8_t)((ot_count >> 8) & 0xFF);
    buf[2] = (uint8_t)(ov_count & 0xFF);
    buf[3] = (uint8_t)((ov_count >> 8) & 0xFF);
    buf[4] = (uint8_t)(oc_count & 0xFF);
    buf[5] = (uint8_t)((oc_count >> 8) & 0xFF);

    hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                               REG_ERR_OVERTEMP_CNT,
                               buf, 6);
#endif
}

/**
 * @brief Write charge/discharge state to WB7720 register 0x17.
 *
 * Maps g_buckboost.woke_mode to:
 *   BUCKBOOST_SHUTDOWM_MODE (0) -> 0 (standby)
 *   BUCKBOOST_CHAGER_MODE   (1) -> 1 (charging)
 *   BUCKBOOST_DISCHG_MODE   (2) -> 2 (discharging)
 *
 * Called from round-robin step 12.
 */
void usb_bridge_write_charge_state(void)
{
    uint8_t state;

    switch (g_buckboost.woke_mode) {
        case BUCKBOOST_CHAGER_MODE:
            state = 1;
            break;
        case BUCKBOOST_DISCHG_MODE:
            state = 2;
            break;
        default:
            state = 0; /* Standby / shutdown */
            break;
    }

    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_CHARGE_STATE, state);
}

/**
 * @brief Write one exception record to WB7720 registers 0x37-0x4D.
 *
 * Uses internal rate control: only writes one record every EXC_WRITE_INTERVAL
 * (64) calls. Rotates through valid records in ap->record_storage.
 *
 * Writes 23 bytes:
 *   [0]    = total valid record count (exception_counter, capped at MAX_RECORDS)
 *   [1]    = current record index being written
 *   [2]    = reserved (0x00)
 *   [3-22] = 20-byte BatteryExceptionRecord_t
 *
 * Called from round-robin step 13.
 */
void usb_bridge_write_exception_record(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    uint8_t valid_count;
    uint8_t buf[23];

    exc_cycle_cnt++;
    if (exc_cycle_cnt < EXC_WRITE_INTERVAL) {
        return;
    }
    exc_cycle_cnt = 0;

    /* Determine how many valid records exist */
    valid_count = ap->record_storage.exception_counter;
    if (valid_count > MAX_RECORDS) {
        valid_count = MAX_RECORDS;
    }

    /* If no records, write zeros */
    if (valid_count == 0) {
        memset(buf, 0, sizeof(buf));
        hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                   REG_EXC_TOTAL_COUNT,
                                   buf, sizeof(buf));
        return;
    }

    /* Wrap rotation index within valid record range */
    if (exc_rotate_idx >= valid_count) {
        exc_rotate_idx = 0;
    }

    /* Build 23-byte payload */
    buf[0] = valid_count;        /* Total valid record count */
    buf[1] = exc_rotate_idx;     /* Current record index */
    buf[2] = 0x00;               /* Reserved */

    /* Copy 20-byte record (BatteryExceptionRecord_t) */
    memcpy(&buf[3],
           (const void *)&ap->record_storage.records[exc_rotate_idx],
           sizeof(BatteryExceptionRecord_t));

    hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                               REG_EXC_TOTAL_COUNT,
                               buf, sizeof(buf));

    /* Advance rotation for next write */
    exc_rotate_idx++;
    if (exc_rotate_idx >= valid_count) {
        exc_rotate_idx = 0;
    }
#endif
}

/**
 * @brief Check and handle engineering mode (virtual value override).
 *
 * Engineering mode allows the PC to inject virtual date, production date,
 * and cycle count into the running system WITHOUT writing to Flash.
 * On exit, real values are restored with elapsed deltas preserved.
 *
 * Enter  (0x50 == 0xA5 && !eng_mode_active):
 *   - Save real gd->Bat_RTC_Seconds and gd->Battery_cycle_count
 *   - Read eng values from I2C: 0x60-0x63 (date), 0x70-0x73 (prod date), 0x80-0x81 (cycle)
 *   - Override gd fields with virtual values
 *   - Write eng production date to I2C 0xE2 area
 *
 * Maintain (0x50 == 0xA5 && eng_mode_active):
 *   - Do nothing (system runs with virtual values)
 *
 * Exit   (0x50 != 0xA5 && eng_mode_active):
 *   - Compute elapsed delta, restore real values + delta
 *   - Reload real ProductInfo from Flash to I2C 0x92-0xF5
 *
 * Called from round-robin step 14.
 */
void usb_bridge_check_engineering_mode(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    uint8_t work_mode = 0;
    uint8_t date_buf[4];

    /* Read work mode register from WB7720 */
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_WORK_MODE, &work_mode);

    if (work_mode == 0xA5) {
        if (!eng_mode_active) {
            /*--- ENTER engineering mode ---*/

            /* Save real values */
            eng_saved_rtc_seconds = gd->Bat_RTC_Seconds;
            eng_saved_cycle_count = gd->Battery_cycle_count;

            /* Read engineering current date from 0x60-0x63: Year(u16 LE) + Month(u8) + Day(u8) */
            hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                      REG_ENG_CURRENT_DATE,
                                      date_buf, 4);
            {
                uint16_t eng_year  = (uint16_t)date_buf[0] | ((uint16_t)date_buf[1] << 8);
                uint8_t  eng_month = date_buf[2];
                uint8_t  eng_day   = date_buf[3];

                /* Override RTC seconds with virtual date */
                gd->Bat_RTC_Seconds = date_to_seconds(eng_year, eng_month, eng_day);
                eng_entry_virtual_seconds = gd->Bat_RTC_Seconds;

            }

            /* Read engineering production date from 0x70-0x73 */
            hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                      REG_ENG_PRODUCTION_DATE,
                                      date_buf, 4);
            {
                /* Write engineering production date to I2C 0xE2 area (20B ASCII) */
                /* Format: "YYYY-MM-DD" padded with 0x00 to 20 bytes */
                uint16_t prod_year  = (uint16_t)date_buf[0] | ((uint16_t)date_buf[1] << 8);
                uint8_t  prod_month = date_buf[2];
                uint8_t  prod_day   = date_buf[3];
                char prod_date_str[PRODUCT_INFO_FIELD_SIZE];
                uint8_t idx = 0;

                memset(prod_date_str, 0, sizeof(prod_date_str));

                /* Simple integer-to-ASCII for YYYY-MM-DD */
                prod_date_str[idx++] = '0' + (prod_year / 1000) % 10;
                prod_date_str[idx++] = '0' + (prod_year / 100) % 10;
                prod_date_str[idx++] = '0' + (prod_year / 10) % 10;
                prod_date_str[idx++] = '0' + prod_year % 10;
                prod_date_str[idx++] = '-';
                prod_date_str[idx++] = '0' + (prod_month / 10) % 10;
                prod_date_str[idx++] = '0' + prod_month % 10;
                prod_date_str[idx++] = '-';
                prod_date_str[idx++] = '0' + (prod_day / 10) % 10;
                prod_date_str[idx++] = '0' + prod_day % 10;

                hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                           REG_PROD_PROD_DATE,
                                           (uint8_t *)prod_date_str,
                                           PRODUCT_INFO_FIELD_SIZE);

                /* prod_date written to WB7720 */
            }

            /* Read 0x80-0x87 in one 8-byte I2C read: cycle(2B) + cell1(2B) + cell2(2B) + temp(2B) */
            {
                uint8_t rbuf[8];
                hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                          REG_ENG_CYCLE_CHG_COUNT, rbuf, 8);
                gd->Battery_cycle_count = rbuf[0];
                eng_entry_virtual_cycle = rbuf[0];
                eng_virtual_cell1 = (uint16_t)rbuf[2] | ((uint16_t)rbuf[3] << 8);
                eng_virtual_cell2 = (uint16_t)rbuf[4] | ((uint16_t)rbuf[5] << 8);
                eng_virtual_temp  = (int16_t)((uint16_t)rbuf[6] | ((uint16_t)rbuf[7] << 8));
            }

            eng_mode_active = true;
        }
        /* else: Maintain mode - do nothing, system runs with virtual values */
    } else {
        if (eng_mode_active) {
            /*--- EXIT engineering mode ---*/
            uint32_t elapsed;
            uint8_t cycles_added;

            /* Calculate how much time elapsed during engineering mode */
            elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds;

            /* Restore real RTC seconds + elapsed delta */
            gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed;

            /* Calculate cycles added during engineering mode */
            cycles_added = gd->Battery_cycle_count - eng_entry_virtual_cycle;

            /* Restore real cycle count + added cycles */
            gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added;

            /* Reload real ProductInfo from Flash to I2C 0x92-0xF5 */
            {
                ProductInfo_t info;
                product_info_read(&info);

                if (is_product_info_valid((const uint8_t *)&info, sizeof(ProductInfo_t))) {
                    hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                               REG_PROD_MANUFACTURER,
                                               (uint8_t *)&info,
                                               sizeof(ProductInfo_t));
                    /* PI restored */
                }
            }

            /* Clear virtual override values */
            eng_virtual_cell1 = 0;
            eng_virtual_cell2 = 0;
            eng_virtual_temp  = 0;

            eng_mode_active = false;
        }
    }
#endif
}

/**
 * @brief Check and handle production mode (Flash ProductInfo write).
 *
 * When REG_PROD_MODE_FLAG (0x90) == 0xB5:
 *   1. Write 0x91 = 0x01 (busy)
 *   2. Read 100 bytes from 0x92-0xF5 into ProductInfo_t
 *   3. Validate (at least one field non-zero)
 *   4. Call product_info_write() to write to Flash
 *   5. Read back from Flash and verify with memcmp
 *   6. Write 0x91 = 0x02 (success) or 0xFF (fail)
 *   7. Clear 0x90 = 0x00
 *
 * Called from round-robin step 15.
 */
void usb_bridge_check_production_mode(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    uint8_t flag = 0;

    /* Read production mode flag */
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_MODE_FLAG, &flag);

    if (flag != 0xB5) {
        return;
    }

    /* production mode triggered */

    /* Step 1: Set status to busy */
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_WRITE_STATUS, 0x01);

    /* Step 2: Read 100 bytes from I2C registers 0x92-0xF5 */
    ProductInfo_t info;
    memset(&info, 0, sizeof(ProductInfo_t));

    hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                              REG_PROD_MANUFACTURER,
                              (uint8_t *)&info,
                              sizeof(ProductInfo_t));

    /* Step 3: Validate - at least one field must be non-zero */
    if (!is_product_info_valid((const uint8_t *)&info, sizeof(ProductInfo_t))) {
        /* invalid data */
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_WRITE_STATUS, 0xFF);
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_MODE_FLAG, 0x00);
        return;
    }

    /* Step 4: Write to Flash */
    product_info_write(&info);

    /* Step 5: Read back from Flash and verify */
    ProductInfo_t verify;
    product_info_read(&verify);

    if (memcmp(&info, &verify, sizeof(ProductInfo_t)) == 0) {
        /* Step 6a: Success */
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_WRITE_STATUS, 0x02);

        /* Also update the live I2C buffer with the newly written data */
        /* (registers 0x92-0xF5 already contain the correct data from the PC write) */
    } else {
        /* Step 6b: Fail - verification mismatch */
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_WRITE_STATUS, 0xFF);
    }

    /* Step 7: Clear production mode flag */
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_PROD_MODE_FLAG, 0x00);
#endif
}

/**
 * @brief Query whether engineering mode is currently active.
 * @return true if engineering mode is active (virtual values in use)
 */
bool usb_bridge_is_eng_mode(void)
{
    return eng_mode_active;
}

uint16_t usb_bridge_get_eng_cell1(void) { return eng_virtual_cell1; }
uint16_t usb_bridge_get_eng_cell2(void) { return eng_virtual_cell2; }
int16_t  usb_bridge_get_eng_temp(void)  { return eng_virtual_temp; }

/**
 * @brief Check and handle engineering test commands (erase all).
 * Called from round-robin step 17. Only processes when engineering mode is active.
 */
void usb_bridge_check_eng_test_cmds(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    if (!eng_mode_active) return;

    uint8_t erase_cmd = 0;
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_ERASE_ALL_CMD, &erase_cmd);

    if (erase_cmd == 0xEE) {
        battery_record_erase_all();
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_CMD_STATUS, 0x02);
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_ERASE_ALL_CMD, 0x00);
    }
#endif
}

#endif /* CONFIG_USB_BRIDGE_ENABLE */
