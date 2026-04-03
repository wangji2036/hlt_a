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

/* Exception record sequential push (cursor-based, migrated from Nanfu A1052) */
static uint8_t exc_cursor_page = 0;
static uint8_t exc_cursor_idx = 0;
static uint8_t exc_records_sent = 0;
static uint8_t exc_page_counts[LOG_PAGE_COUNT];

/* Engineering mode virtual override values (sentinel = no override) */
static uint16_t eng_virtual_cell1 = ENG_SENTINEL_CELL;  /* Virtual Cell1 voltage (mV) */
static uint16_t eng_virtual_cell2 = ENG_SENTINEL_CELL;  /* Virtual Cell2 voltage (mV) */
static int16_t  eng_virtual_temp  = (int16_t)ENG_SENTINEL_TEMP;  /* Virtual temperature (0.1°C) */

/*===================== Helper Functions =====================*/


/**
 * @brief Read current date/time from WB7720 (0x60-0x66) and apply to RTC.
 *        Updates Bat_RTC_Seconds, Bat_RTC_Milliseconds, eng_entry_virtual_seconds.
 *        (datetime_to_seconds inlined to eliminate 6-arg call overhead on CK802)
 *
 * @return true if date was applied, false if sentinel (year==0) was detected
 */
static bool apply_eng_datetime_to_rtc(void)
{
    /* datetime_to_seconds inlined to eliminate 6-arg call overhead on CK802 */
    static const uint8_t days_in_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    uint8_t dt_buf[7];
    hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR, REG_ENG_CURRENT_DATE, dt_buf, 7);
    uint16_t year  = (uint16_t)dt_buf[0] | ((uint16_t)dt_buf[1] << 8);

    /* Sentinel check: year==0 means "don't override RTC" */
    if (year == 0) {
        eng_entry_virtual_seconds = gd->Bat_RTC_Seconds;
        return false;
    }

    uint8_t  month = dt_buf[2];
    uint8_t  day   = dt_buf[3];

    /* Days since 2026-01-01 (formula, no loop; eng mode always provides year>=2026) */
    uint16_t y = year - 2026U;
    uint32_t total_days = (uint32_t)y * 365UL + ((uint32_t)(y + 1) / 4);
    uint8_t m;
    for (m = 1; m < month; m++) {
        total_days += days_in_month[m];
    }
    if (month > 2 && (year % 4 == 0)) total_days++;
    total_days += (uint32_t)(day - 1);

    uint32_t secs = total_days * 86400UL
                    + (uint32_t)dt_buf[4] * 3600UL
                    + (uint32_t)dt_buf[5] * 60UL
                    + (uint32_t)dt_buf[6];

    VIC_vModuleDisable();
    gd->Bat_RTC_Seconds = secs;
    gd->Bat_RTC_Milliseconds = 0;
    VIC_vModuleEnable();
    eng_entry_virtual_seconds = gd->Bat_RTC_Seconds;
    return true;
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
 * @brief Reset ProductInfo write flag so next call re-writes to WB7720.
 *
 * Must be called after WB7720 wakeup (NVIC_SystemReset clears i2c_buff).
 */
static uint8_t product_info_done = 0;

void usb_bridge_reset_product_info(void)
{
    product_info_done = 0;
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
    if (product_info_done) return;
    ProductInfo_t info;
    product_info_read(&info);
    if (!is_product_info_valid((const uint8_t *)&info, sizeof(ProductInfo_t))) {
        printk("[PI] Flash invalid, skip\n");
        product_info_done = 1;
        return;
    }
    uint8_t ret = hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                   REG_PROD_MANUFACTURER,
                                   (uint8_t *)&info,
                                   sizeof(ProductInfo_t));
    printk("[PI] write %dB ret=%d\n", (int)sizeof(ProductInfo_t), ret);
    if (ret == 0) {
        product_info_done = 1;
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
 * @brief Sequential push of exception records to WB7720 (0x37-0x4D).
 *
 * Migrated from Nanfu A1052: cursor-based sequential traversal of all
 * Flash pages. Each call pushes one record if WB7720 has consumed the
 * previous one (handshake via REG_EXC_READY).
 *
 * Called from round-robin step 13.
 */
void usb_bridge_write_exception_record(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    /* Scan current record counts and always update total */
    uint8_t total = 0;
    for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
        exc_page_counts[p] = battery_record_get_page_count(p);
        total += exc_page_counts[p];
    }
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_EXC_TOTAL_COUNT, total);

    /* Handshake: check if WB7720 consumed previous record */
    uint8_t wb_ready = 0;
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_EXC_READY, &wb_ready);
    if (wb_ready == 0xA5) {
        return;  /* WB7720 hasn't consumed yet, skip this cycle */
    }

    if (total == 0) return;

    /* Wrap cursor if out of bounds */
    if (exc_cursor_page >= LOG_PAGE_COUNT) exc_cursor_page = 0;
    if (exc_cursor_idx >= exc_page_counts[exc_cursor_page]) {
        exc_cursor_idx = 0;
    }

    /* Skip empty pages */
    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        if (exc_page_counts[exc_cursor_page] > 0) break;
        exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
        exc_cursor_idx = 0;
    }

    /* Read and push one record from Flash */
    BatteryExceptionRecord_t rec;
    if (battery_record_read_by_page_index(exc_cursor_page, exc_cursor_idx, &rec)
        && rec.record_id != 0) {
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_EXC_CURRENT_IDX, exc_records_sent);
        hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR, REG_EXC_RECORD_DATA,
                                   (uint8_t*)&rec, sizeof(BatteryExceptionRecord_t));
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_EXC_READY, 0xA5);
        exc_records_sent++;

        /* Advance cursor */
        exc_cursor_idx++;
        if (exc_cursor_idx >= exc_page_counts[exc_cursor_page]) {
            exc_cursor_idx = 0;
            for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
                exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
                if (exc_page_counts[exc_cursor_page] > 0) break;
            }
        }
    } else {
        /* Read failed or empty record — skip to next page */
        exc_cursor_idx = 0;
        exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
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

            /* Read engineering current date from 0x60-0x66 and apply to RTC */
            apply_eng_datetime_to_rtc();

            /* Read engineering production date from 0x70-0x73 */
            hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                      REG_ENG_PRODUCTION_DATE,
                                      date_buf, 4);
            {
                /* Sentinel check: prod_year==0 means "don't override production date" */
                uint16_t prod_year  = (uint16_t)date_buf[0] | ((uint16_t)date_buf[1] << 8);
                if (prod_year != 0) {
                    /* Write engineering production date to I2C 0xE2 area (20B ASCII) */
                    /* Format: "YYYY-MM-DD" padded with 0x00 to 20 bytes */
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
                }
                /* else: sentinel, skip prod date override */
            }

            /* Read 0x80-0x87 in one 8-byte I2C read: cycle(2B) + cell1(2B) + cell2(2B) + temp(2B) */
            {
                uint8_t rbuf[8];
                hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                          REG_ENG_CYCLE_CHG_COUNT, rbuf, 8);

                uint16_t raw_cycle = (uint16_t)rbuf[0] | ((uint16_t)rbuf[1] << 8);
                if (raw_cycle != ENG_SENTINEL_CYCLE) {
                    gd->Battery_cycle_count = (uint8_t)raw_cycle;
                    eng_entry_virtual_cycle = (uint8_t)raw_cycle;
                } else {
                    /* Sentinel: keep real cycle count as baseline */
                    eng_entry_virtual_cycle = gd->Battery_cycle_count;
                }

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

            /* Immediately write restored real cycle count to WB7720,
             * so telemetry updates without waiting for next cnt=5 round-robin
             * (exit runs at cnt=14, cnt=5 is ~9 steps later = ~420ms delay) */
            {
                uint16_t write_buf = (uint16_t)gd->Battery_cycle_count;
                hal_i2cm_write_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                           REG_CYCLE_COUNT,
                                           (uint8_t *)&write_buf, 2);
            }

            /* Clear virtual override values (reset to sentinel = no override) */
            eng_virtual_cell1 = ENG_SENTINEL_CELL;
            eng_virtual_cell2 = ENG_SENTINEL_CELL;
            eng_virtual_temp  = (int16_t)ENG_SENTINEL_TEMP;

            eng_mode_active = false;
        }
    }
#endif
}

/**
 * @brief Check and handle RTC time sync trigger from PC.
 *
 * When eng mode active, PC writes datetime to 0x60-0x66, then writes
 * REG_TIME_SYNC(0x51) = 0xCA to trigger sync. NU17112 reads datetime
 * and updates RTC, then clears the trigger register.
 *
 * Called from round-robin step 14 (after check_engineering_mode).
 */
void usb_bridge_check_time_sync(void)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    if (!eng_mode_active) return;

    uint8_t trigger = 0;
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_TIME_SYNC, &trigger);

    if (trigger == TIME_SYNC_MAGIC) {
        apply_eng_datetime_to_rtc();
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_TIME_SYNC, 0x00);
        printk("time sync ok\n");
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
 * @brief Trigger burst sync of N exception records to WB7720.
 *
 * Resets cursor to page 0 / index 0 for sequential re-push.
 * Legacy 'count' param unused in sequential mode.
 */
void usb_bridge_exc_burst(uint8_t count)
{
#if CONFIG_NEW_CCC_LOG_ENABLE
    (void)count;
    exc_cursor_page = 0;
    exc_cursor_idx = 0;
    exc_records_sent = 0;
#endif
}

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
    } else if (erase_cmd == 0xAA) {
        /* Refresh: apply new params IN-PLACE (no exit/re-enter gap) */

        /* 1. Settle RTC and cycle deltas from current session */
        uint32_t elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds;
        gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed;
        uint8_t cycles_added = gd->Battery_cycle_count - eng_entry_virtual_cycle;
        gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added;

        /* 2. Update baseline for next session */
        eng_saved_rtc_seconds = gd->Bat_RTC_Seconds;
        eng_saved_cycle_count = gd->Battery_cycle_count;

        /* 3. Read new virtual params from WB7720 (same sentinel checks as entry) */
        {
            uint8_t rbuf[8];
            hal_i2cm_read_multi_bytes(USB_BRIDGE_WB7720_ADDR,
                                      REG_ENG_CYCLE_CHG_COUNT, rbuf, 8);

            uint16_t raw_cycle = (uint16_t)rbuf[0] | ((uint16_t)rbuf[1] << 8);
            if (raw_cycle != ENG_SENTINEL_CYCLE) {
                gd->Battery_cycle_count = (uint8_t)raw_cycle;
                eng_entry_virtual_cycle = (uint8_t)raw_cycle;
            } else {
                /* Sentinel: keep real cycle count as baseline */
                eng_entry_virtual_cycle = gd->Battery_cycle_count;
            }

            eng_virtual_cell1 = (uint16_t)rbuf[2] | ((uint16_t)rbuf[3] << 8);
            eng_virtual_cell2 = (uint16_t)rbuf[4] | ((uint16_t)rbuf[5] << 8);
            eng_virtual_temp  = (int16_t)((uint16_t)rbuf[6] | ((uint16_t)rbuf[7] << 8));
        }

        /* 4. Re-read current date (0x60-0x66) and apply to RTC */
        apply_eng_datetime_to_rtc();

        /* eng_mode_active stays true — no gap where real ADC triggers stale tracking */

        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_CMD_STATUS, 0x02);
        hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_ERASE_ALL_CMD, 0x00);
    }
#endif
}

#endif /* CONFIG_USB_BRIDGE_ENABLE */
