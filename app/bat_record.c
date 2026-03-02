#include "bat_record.h"
#include "g_data.h"
#include "nu6805.h"
#include "buckboost.h"
#include "ntc.h"
#include "fmc.h"
#include "app.h"
#include "printk.h"
#include <string.h>
#include <stddef.h>
#include "../hal/regdef.h"
#include"_fml.h"
#if CONFIG_USB_BRIDGE_ENABLE
#include "usb_bridge.h"
#endif

#if CONFIG_NEW_CCC_LOG_ENABLE
/********************* Global Variables **********************/
// Variables defined in ap_t structure, accessed via ap->
#define g_exception_cache   (ap->exception_cache)
#define g_record_storage    (ap->record_storage)

static uint32_t g_next_record_id = 1;  /* Monotonic ID, starts from 1, never 0 */

/* Forward declaration */
static void battery_record_dump_flash(void);

/********************* Flash Operation Functions *********************/

// Flash write with endian conversion
static void flash_write_u32(uint32_t addr, uint32_t data) {
    uint32_t tmp = switch_big_little_endian(data);
    hal_fmc_write_word(addr, tmp);
}

// Flash read
static uint32_t flash_read_u32(uint32_t addr) {
    return *(uint32_t*)addr;
}

// Write record structure with 4-byte alignment
static void flash_write_record(uint32_t addr, uint8_t *data, uint16_t len) {
    uint32_t word;
    for (uint16_t i = 0; i < len; i += 4) {
        if (i + 3 < len) {
            word = (data[i+3] << 24) | (data[i+2] << 16) | (data[i+1] << 8) | data[i];
        } else {
            // Handle less than 4 bytes case
            word = 0;
            for (uint16_t j = 0; j < (len - i); j++) {
                word |= (data[i+j] << (j*8));
            }
        }
        flash_write_u32(addr + i, word);
    }
}

// Read record structure
static void flash_read_record(uint32_t addr, uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        data[i] = *(uint8_t*)(addr + i);
    }
}

/********************* Time Management Functions *********************/

// Convert seconds to timestamp - simplified
static void seconds_to_timestamp(uint32_t total_seconds, TimeStamp_t *ts) {
    uint32_t seconds_in_day = 86400;

    // Calculate days and seconds today
    uint32_t days = total_seconds / seconds_in_day;
    uint32_t seconds_today = total_seconds % seconds_in_day;

    // Calculate year/month/day from 2026
    uint32_t year = 2026;
    uint32_t days_in_year = 365;

    while (days >= days_in_year) {
        days -= days_in_year;
        year++;
        days_in_year = (year % 4 == 0) ? 366 : 365;
    }

    // Days in each month
    const uint8_t month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    uint8_t month = 1;

    for (month = 1; month <= 12; month++) {
        uint8_t days_this_month = month_days[month-1];
        if (month == 2 && (year % 4 == 0)) days_this_month = 29;

        if (days < days_this_month) break;
        days -= days_this_month;
    }

    ts->year = year;
    ts->month = month;
    ts->day = days + 1;
    ts->hour = seconds_today / 3600;
    ts->minute = (seconds_today % 3600) / 60;
    ts->second = seconds_today % 60;
    ts->reserved = 0;
}

// Get current timestamp with critical section protection
static void get_current_timestamp(volatile TimeStamp_t *ts) {
    uint32_t seconds;
    VIC_vModuleDisable();
    seconds = gd->Bat_RTC_Seconds;
    VIC_vModuleEnable();
    TimeStamp_t local_ts;
    seconds_to_timestamp(seconds, &local_ts);
    ts->year = local_ts.year;
    ts->month = local_ts.month;
    ts->day = local_ts.day;
    ts->hour = local_ts.hour;
    ts->minute = local_ts.minute;
    ts->second = local_ts.second;
    ts->reserved = local_ts.reserved;
}

// Check if one hour has passed
static bool is_new_hour(uint32_t last_seconds, uint32_t current_seconds) {
    // Calculate elapsed seconds - handles uint32_t overflow
    uint32_t elapsed_seconds = current_seconds - last_seconds;

    // 1 hour = 3600 seconds
    return elapsed_seconds >= 3600;
}

// Check temperature abnormal status - Only check over-temperature, charge/discharge are the same
static bool is_temperature_abnormal(uint16_t ntc_resistance, uint8_t mode, uint8_t *event_type) {
	/*
    if (mode == BUCKBOOST_CHAGER_MODE) {
        if (ntc_resistance < CHRG_NTC_OT_VALUE) {
            *event_type = EXCEPTION_TYPE_OVERTEMP;  // 0x02 Over temperature
            return true;
        }
    } else if (mode == BUCKBOOST_DISCHG_MODE) {
        if (ntc_resistance < DISG_NTC_OT_VALUE) {
            *event_type = EXCEPTION_TYPE_OVERTEMP;  // 0x02 Over temperature
            return true;
        }
    }*/
    if (ntc_resistance < CHRG_NTC_OT_VALUE) {
        *event_type = EXCEPTION_TYPE_OVERTEMP;  // 0x02 Over temperature
        return true;
    }
    return false;
}

/********************* Checksum Functions *********************/

// Calculate simple checksum for storage structure - excluding checksum field itself
static uint16_t calculate_checksum(volatile BatteryRecordStorage_t *storage) {
    uint32_t sum = 0;
    volatile uint8_t *data = (volatile uint8_t*)storage;

    // Calculate all data before checksum field
    size_t data_len = offsetof(BatteryRecordStorage_t, checksum);

    for (size_t i = 0; i < data_len; i++) {
        sum += data[i];
    }

    // Return 16-bit sum
    return (uint16_t)(sum & 0xFFFF);
}

// Verify data integrity of storage structure
static bool verify_storage_checksum(volatile BatteryRecordStorage_t *storage) {
    uint16_t calculated = calculate_checksum(storage);
    return (calculated == storage->checksum);
}

/********************* Flash Record Read/Write Functions *********************/

// Write entire RAM storage structure to Flash - single page
static void save_storage_to_flash(void) {
    // Calculate and update checksum
    g_record_storage.checksum = calculate_checksum(&g_record_storage);

    // Erase the entire page
    hal_fmc_erase_page(FLASH_LOG_BASE);

    // Write entire storage structure to Flash
    flash_write_record(FLASH_LOG_BASE, (uint8_t*)&g_record_storage, sizeof(BatteryRecordStorage_t));
}

// Load storage structure from Flash to RAM
static void load_storage_from_flash(void) {
    flash_read_record(FLASH_LOG_BASE, (uint8_t*)&g_record_storage, sizeof(BatteryRecordStorage_t));
}

// Write exception record - operate in RAM, then write to Flash
static void write_exception_record(BatteryExceptionRecord_t *record) {
    // Write record to RAM buffer - circular
    if (g_record_storage.write_ptr >= MAX_RECORDS) {
        g_record_storage.write_ptr = 0;
    }

    // Set record ID to monotonic counter (never 0, never repeats within a power cycle)
    record->record_id = g_next_record_id++;

    g_record_storage.records[g_record_storage.write_ptr] = *record;
    g_record_storage.write_ptr = (g_record_storage.write_ptr + 1) % MAX_RECORDS;

    // Update counter, limit to MAX_RECORDS
    if (g_record_storage.exception_counter < MAX_RECORDS) {
        g_record_storage.exception_counter++;
    }

    // Write entire RAM storage to Flash
    save_storage_to_flash();
    printk("[EXC]id%d ty%d\r\n", (int)record->record_id, record->error_type);
#if CONFIG_USB_BRIDGE_ENABLE
    {
        uint8_t vcnt = g_record_storage.exception_counter;
        if (vcnt > MAX_RECORDS) vcnt = MAX_RECORDS;
        usb_bridge_exc_burst(vcnt);
    }
#endif
}

/********************* Public API Function Implementations *********************/

// Initialization function
void battery_record_init(void) {
    uint32_t magic = flash_read_u32(ADDR_MAGIC);
    bool need_init = false;
    if (magic != MAGIC_VALUE) {
        // First use or unknown version
        need_init = true;
        printk("log, initializing...\r\n");
    }
    else {
        // Load storage data from Flash to RAM
        load_storage_from_flash();

        // Verify checksum
        if (!verify_storage_checksum(&g_record_storage)) {
            // Checksum error, discard all data and reinitialize
            need_init = true;
            printk("log: Checksum error...\r\n");
        } else {
            // printk("Battery record: Data loaded successfully (checksum OK)\r\n");
            battery_record_print_next_log();
            /* Restore monotonic ID from existing records: find max record_id + 1 */
            {
                uint32_t max_rid = 0;
                uint8_t valid = g_record_storage.exception_counter;
                if (valid > MAX_RECORDS) valid = MAX_RECORDS;
                for (uint8_t i = 0; i < valid; i++) {
                    if (g_record_storage.records[i].record_id > max_rid) {
                        max_rid = g_record_storage.records[i].record_id;
                    }
                }
                g_next_record_id = (max_rid > 0) ? max_rid + 1 : 1;
#if CONFIG_USB_BRIDGE_ENABLE
                if (valid > 0) usb_bridge_exc_burst(valid);
#endif
            }
        }
    }

    // If initialization needed
    if (need_init) {
        memset((void*)&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
        g_record_storage.magic = MAGIC_VALUE;
        g_record_storage.exception_counter = 0;
        g_record_storage.write_ptr = 0;
        // Removed reserved and padding fields

        g_next_record_id = 1;  /* Fresh start: monotonic ID begins at 1 */

        // Write initialized data to Flash
        save_storage_to_flash();
        printk("log initialized\r\n");
    }

    // Initialize exception tracking cache
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
}

// Cell overvoltage processing function - Optimized version
static void process_cell_overvoltage(uint8_t cell_num, uint16_t cell_voltage,
                                      uint16_t total_voltage) {
    volatile uint16_t *max_voltage;
    uint8_t tracking_mask;

    // Select cache fields based on cell number (use bitfield macros for tracking)
    if (cell_num == 1) {
        tracking_mask = 0x01;  // CELL1_TRACKING bit
        max_voltage = &g_exception_cache.cell1_max_voltage;
    } else {
        tracking_mask = 0x02;  // CELL2_TRACKING bit
        max_voltage = &g_exception_cache.cell2_max_voltage;
    }

    bool cell_over = (cell_voltage >= OVER_VOLTAGE_THRESHOLD);
    bool is_tracking = (g_exception_cache.status_flags & tracking_mask) != 0;

    if (cell_over) {
        if (!is_tracking) {
            // Exception just occurred - save first record immediately
            // Use bit operation to set tracking flag
            g_exception_cache.status_flags |= tracking_mask;
            *max_voltage = cell_voltage;
            // Read last_saved_max from records instead

            // Get time stamp
            VIC_vModuleDisable();
            g_exception_cache.ov_hour_start_seconds = gd->Bat_RTC_Seconds;
            VIC_vModuleEnable();

            // Generate timestamp directly

            // Write first record immediately
            BatteryExceptionRecord_t record;
            get_current_timestamp(&record.timestamp);  // Generate realtime
            record.error_type = EXCEPTION_TYPE_OVERVOLTAGE;
            record.sub_type = cell_num;
            record.data.ov_data.max_voltage = *max_voltage;
            record.data.ov_data.total_voltage = total_voltage;
            write_exception_record(&record);

            printk("\r\n[ACTIVE] %04d-%02d-%02d %02d:%02d:%02d | OV Cell%d: %dmV (Tracking...)",
                record.timestamp.year, record.timestamp.month, record.timestamp.day,
                record.timestamp.hour, record.timestamp.minute, record.timestamp.second,
                cell_num, record.data.ov_data.max_voltage);

        } else {
            // Fault continues - update real-time maximum value
            if (cell_voltage > *max_voltage) {
                *max_voltage = cell_voltage;
            }

            // Check if within 1-hour window
            uint32_t current_seconds;
            VIC_vModuleDisable();
            current_seconds = gd->Bat_RTC_Seconds;
            VIC_vModuleEnable();

            bool is_hour_passed = is_new_hour(g_exception_cache.ov_hour_start_seconds, current_seconds);

            // Read last saved value from records (moved outside if/else)
            uint8_t last_index = (g_record_storage.write_ptr == 0) ?
                                 (MAX_RECORDS - 1) : (g_record_storage.write_ptr - 1);
            uint16_t last_saved_max = g_record_storage.records[last_index].data.ov_data.max_voltage;

            if (!is_hour_passed) {
                // Within 1 hour, update RAM only, no Flash write
                if (*max_voltage > last_saved_max) {
                    // Update last record in RAM only - last_index already calculated above

                    // Update record in RAM
                    get_current_timestamp(&g_record_storage.records[last_index].timestamp);
                    g_record_storage.records[last_index].data.ov_data.max_voltage = *max_voltage;
                    g_record_storage.records[last_index].data.ov_data.total_voltage = total_voltage;

                    // No Flash write, host can read RAM
                    // printk("\r\n[RAM] OV Cell%d: %dmV", cell_num, *max_voltage);
                }
            } else {
                // 1 hour passed, save to Flash if max changed
                if (*max_voltage > last_saved_max) {
                    // Update record in RAM - last_index already calculated above

                    get_current_timestamp(&g_record_storage.records[last_index].timestamp);
                    g_record_storage.records[last_index].data.ov_data.max_voltage = *max_voltage;
                    g_record_storage.records[last_index].data.ov_data.total_voltage = total_voltage;

                    // No longer need to update last_saved_max cache

                    // Save to Flash
                    save_storage_to_flash();

                    printk("\r\n[SAVE] OV Cell%d: %dmV (1h passed, saved to Flash)", cell_num, *max_voltage);

                    // Reset 1-hour window
                    VIC_vModuleDisable();
                    g_exception_cache.ov_hour_start_seconds = gd->Bat_RTC_Seconds;
                    VIC_vModuleEnable();
                }
            }
        }
    } else if (is_tracking) {
        // Voltage recovered - check 1-hour window before clearing tracking
        uint32_t current_seconds;
        VIC_vModuleDisable();
        current_seconds = gd->Bat_RTC_Seconds;
        VIC_vModuleEnable();

        bool is_hour_passed = is_new_hour(g_exception_cache.ov_hour_start_seconds, current_seconds);

        if (is_hour_passed) {
            // 1 hour window has ended - finalize: save to Flash if max changed, then clear tracking
            uint8_t last_index = (g_record_storage.write_ptr == 0) ?
                                 (MAX_RECORDS - 1) : (g_record_storage.write_ptr - 1);
            uint16_t last_saved_max = g_record_storage.records[last_index].data.ov_data.max_voltage;

            if (*max_voltage > last_saved_max) {
                // Update record in RAM with peak voltage captured during OV period
                get_current_timestamp(&g_record_storage.records[last_index].timestamp);
                g_record_storage.records[last_index].data.ov_data.max_voltage = *max_voltage;
                g_record_storage.records[last_index].data.ov_data.total_voltage = total_voltage;

                // Save to Flash
                save_storage_to_flash();

                printk("\r\n[SAVE] OV Cell%d: %dmV (Recovered+1h, saved to Flash)", cell_num, *max_voltage);
            }

            // Clear tracking state - use bit operation
            g_exception_cache.status_flags &= ~tracking_mask;
            *max_voltage = 0;
        }
        // else: 1 hour not yet elapsed - keep tracking state and *max_voltage intact.
        // If voltage rises above threshold again, the (cell_over && is_tracking) branch
        // will capture the new peak. Tracking clears only when the 1-hour window ends.
    }
}

// Overvoltage detection and record function
void battery_record_update_overvoltage(void) {
#if(BUCKBOOST_USED_NU6805 == 1)
    uint16_t total_voltage, cell1_voltage, cell2_voltage;
#if CONFIG_USB_BRIDGE_ENABLE
    {
        uint16_t eng_c1 = usb_bridge_get_eng_cell1();
        uint16_t eng_c2 = usb_bridge_get_eng_cell2();
        if (eng_c1 > 0 || eng_c2 > 0) {
            /* Virtual mode: non-zero uses virtual value, zero uses real_total/2 */
            uint16_t real_total = hal_nu6805_buckboost_get_bat_voltage();
            cell1_voltage = (eng_c1 > 0) ? eng_c1 : (real_total / 2);
            cell2_voltage = (eng_c2 > 0) ? eng_c2 : (real_total / 2);
            total_voltage = cell1_voltage + cell2_voltage;
        } else {
            total_voltage = hal_nu6805_buckboost_get_bat_voltage();
            cell1_voltage = total_voltage / 2;
            cell2_voltage = total_voltage / 2;
        }
    }
#else
    total_voltage = hal_nu6805_buckboost_get_bat_voltage();
    cell1_voltage = total_voltage / 2;  // Cell 1 voltage estimation
    cell2_voltage = total_voltage / 2;  // Cell 2 voltage estimation
#endif

    // Process cell 1 and cell 2
    process_cell_overvoltage(1, cell1_voltage, total_voltage);
    process_cell_overvoltage(2, cell2_voltage, total_voltage);
#elif(BUCKBOOST_USED_NU6801 == 1)
    uint16_t cell1_voltage = g_buckboost.adc_vbat;
    uint16_t total_voltage = g_buckboost.adc_vbat;
    process_cell_overvoltage(1, cell1_voltage, total_voltage);
#endif
}

// Temperature abnormal detection and record function
// Adaptation for NU6805: use gd->sys_infos.ntc_temp_wpc (pre-computed, 0.1degC)
// instead of g_buckboost.adc_tbat1 + ntc_to_temp() (NU6801 only)
void battery_record_update_temperature(void) {
    uint8_t mode = g_buckboost.woke_mode;
    uint8_t event_type;
    bool is_temp_tracking = CACHE_GET_TEMP_TRACKING(&g_exception_cache);

#if (BUCKBOOST_USED_NU6805 == 1)
    // NU6805: WPC NTC temperature is pre-computed in APL_TASK (100ms poll)
    int16_t ntc_temp;
#if CONFIG_USB_BRIDGE_ENABLE
    {
        int16_t eng_temp = usb_bridge_get_eng_temp();
        if (eng_temp != 0) {
            ntc_temp = eng_temp;
        } else {
            ntc_temp = gd->sys_infos.ntc_temp_wpc;
        }
    }
#else
    ntc_temp = gd->sys_infos.ntc_temp_wpc;  // 0.1degC
#endif
    event_type = EXCEPTION_TYPE_OVERTEMP;
    bool is_abnormal = (ntc_temp > CHRG_NTC_OT_TEMP_VALUE);
#else
    // NU6801: read NTC resistance and convert to temperature
    uint16_t ntc_resistance = g_buckboost.adc_tbat1;
    int16_t ntc_temp = ntc_to_temp(ntc_resistance);
    bool is_abnormal = is_temperature_abnormal(ntc_resistance, mode, &event_type);
#endif

    if (is_abnormal) {
        if (!is_temp_tracking) {
            // Exception just occurred - save first record immediately
            // Use bitfield macros to set flags
            CACHE_SET_TEMP_TRACKING(&g_exception_cache, 1);
            CACHE_SET_TEMP_EVENT_TYPE(&g_exception_cache, event_type);
            CACHE_SET_CHARGE_STATE(&g_exception_cache, mode);
            g_exception_cache.max_temperature = ntc_temp;
            // Field removed

            // Atomic read of seconds
            VIC_vModuleDisable();
            g_exception_cache.temp_hour_start_seconds = gd->Bat_RTC_Seconds;
            VIC_vModuleEnable();

            // Generate timestamp directly

            // Write first record immediately
            BatteryExceptionRecord_t record;
            get_current_timestamp(&record.timestamp);  // Generate realtime
            record.error_type = event_type;
            record.sub_type = mode;
            record.data.temp_data.max_temperature = g_exception_cache.max_temperature;
            record.data.temp_data.reserved = 0;
            write_exception_record(&record);
            printk("\r\n[ACTIVE] : %ddegC (Tracking...)", record.data.temp_data.max_temperature);
        } else {
            // Exception continues - update real-time maximum temperature
            int16_t current_temp = ntc_temp;

            // For overtemp, take max
            if (current_temp > g_exception_cache.max_temperature) {
                g_exception_cache.max_temperature = current_temp;
            }

            // Check if within 1-hour window
            uint32_t current_seconds;
            VIC_vModuleDisable();
            current_seconds = gd->Bat_RTC_Seconds;
            VIC_vModuleEnable();

            bool is_hour_passed = is_new_hour(g_exception_cache.temp_hour_start_seconds, current_seconds);

            // Read last saved temperature from records
            uint8_t last_index = (g_record_storage.write_ptr == 0) ?
                                 (MAX_RECORDS - 1) : (g_record_storage.write_ptr - 1);
            int16_t last_saved_temperature = g_record_storage.records[last_index].data.temp_data.max_temperature;

            if (!is_hour_passed) {
                // Within 1 hour, update RAM only, no Flash write
                if (g_exception_cache.max_temperature > last_saved_temperature) {
                    // Update last record in RAM only - last_index already calculated above

                    // get_current_timestamp(&g_record_storage.records[last_index].timestamp);
                    g_record_storage.records[last_index].data.temp_data.max_temperature = g_exception_cache.max_temperature;

                    // No Flash write, host can read RAM
                    // printk("\r\n[RAM] OT: %d.%ddegC", temp_int, temp_dec);
                }
            } else {
                // 1 hour passed, save to Flash if max changed
                if (g_exception_cache.max_temperature > last_saved_temperature) {
                    // Update record in RAM - last_index already calculated above

                    get_current_timestamp(&g_record_storage.records[last_index].timestamp);
                    g_record_storage.records[last_index].data.temp_data.max_temperature = g_exception_cache.max_temperature;

                    // No longer need to update last_saved_temperature cache

                    // Save to Flash
                    save_storage_to_flash();

                    printk("\r\n[SAVE] OT: %ddegC (1h passed, saved to Flash)", g_exception_cache.max_temperature);

                    // Reset 1-hour window
                    VIC_vModuleDisable();
                    g_exception_cache.temp_hour_start_seconds = gd->Bat_RTC_Seconds;
                    VIC_vModuleEnable();
                }
            }
        }
    } else if (is_temp_tracking) {
        // Recovered - save to Flash if max changed
        // Read last saved temperature from records
        uint8_t last_index = (g_record_storage.write_ptr == 0) ?
                             (MAX_RECORDS - 1) : (g_record_storage.write_ptr - 1);
        int16_t last_saved_temperature = g_record_storage.records[last_index].data.temp_data.max_temperature;

        if (g_exception_cache.max_temperature > last_saved_temperature) {
            // Update record in RAM
            get_current_timestamp(&g_record_storage.records[last_index].timestamp);
            g_record_storage.records[last_index].data.temp_data.max_temperature = g_exception_cache.max_temperature;

            // No longer need to update last_saved_temperature cache

            // Save to Flash
            save_storage_to_flash();

            printk("\r\n[SAVE] OT: %ddegC (Recovered, saved to Flash)", g_exception_cache.max_temperature);
        }

        // Clear tracking state - use bitfield macros
        CACHE_SET_TEMP_TRACKING(&g_exception_cache, 0);
        g_exception_cache.max_temperature = 0;
        // Field no longer exists
    }
}

// Periodic check function
void battery_record_periodic_check(void) {
    static uint16_t check_counter = 0;

    // Check every 1 second (10 × 100ms APL_EVT_100ms_POLL)
    if (++check_counter >= 10) {
        check_counter = 0;
        battery_record_update_overvoltage();
        battery_record_update_temperature();

        TimeStamp_t ts;
        get_current_timestamp(&ts);
        printk("\r\n[................time] %04d-%02d-%02d %02d:%02d:%02d ",
            ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
    }
}

// Read all exception records from RAM cache
uint8_t battery_record_read_exceptions(BatteryExceptionRecord_t *buf, uint8_t max_count) {
    if (buf == NULL) return 0;

    // Calculate valid record count
    uint32_t total_count = g_record_storage.exception_counter;
    uint8_t valid_count = (total_count < MAX_RECORDS) ? total_count : MAX_RECORDS;

    // Copy records from RAM cache
    for (uint8_t i = 0; i < valid_count && i < max_count; i++) {
        buf[i] = g_record_storage.records[i];
    }

    return valid_count;
}


/********************* Log Print Functions *********************/

// Print next abnormal log
void battery_record_print_next_log(void) {
    /* Disabled to save ROM (~500B). Re-enable for debug if needed. */
}

/**
 * @brief Dump exception records read directly from Flash (bypasses RAM cache).
 * Format: [FD]magic cnt wptr / [FDi]id type YYYYMMDD sub V1/V2
 * type: 1=OV 2=OT 3=UT; V1=max_voltage(OV) or max_temp(OT/UT); V2=total_voltage
 */
static void battery_record_dump_flash(void)
{
    static BatteryRecordStorage_t snap;
    uint8_t i;

    flash_read_record(FLASH_LOG_BASE, (uint8_t *)&snap, sizeof(snap));
    printk("[FD]%08X c%d w%d\r\n",
           (unsigned int)snap.magic, snap.exception_counter, snap.write_ptr);
    if (snap.magic != MAGIC_VALUE) return;

    for (i = 0; i < MAX_RECORDS; i++) {
        BatteryExceptionRecord_t *r = &snap.records[i];
        if (!r->record_id) continue;
        printk("[FD%d]id%d t%d %04d%02d%02d s%d V%d/%d\r\n",
               i, (int)r->record_id, r->error_type,
               r->timestamp.year, r->timestamp.month, r->timestamp.day,
               r->sub_type,
               r->data.ov_data.max_voltage,
               r->data.ov_data.total_voltage);
    }
}

/**
 * @brief Erase all exception records and reset storage to initial state.
 * Resets g_record_storage (magic, counter, write_ptr), saves to Flash,
 * and clears g_exception_cache tracking state.
 */
void battery_record_erase_all(void) {
    printk("[FD]pre-erase:\r\n");
    battery_record_dump_flash();
    memset((void*)&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
    g_record_storage.magic = MAGIC_VALUE;
    save_storage_to_flash();
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    g_next_record_id = 1;  /* Reset monotonic ID after erase */
#if CONFIG_USB_BRIDGE_ENABLE
    usb_bridge_exc_burst(1);  /* one burst to push zeros → clears WB7720 exc_cache */
#endif
    printk("[FD]post-erase:\r\n");
    battery_record_dump_flash();
}

/**
 * @brief Reset exception tracking state without erasing records.
 * Called from 0xAA refresh to allow new virtual params to trigger fresh detection.
 */
void battery_record_reset_tracking(void) {
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
}
#endif
