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
#include "../hal/badc.h"
#include "_fml.h"
#if CONFIG_USB_BRIDGE_ENABLE
#include "usb_bridge.h"
#endif

#if CONFIG_NEW_CCC_LOG_ENABLE

/********************* Global Variables **********************/
/* Multi-page storage state */
static uint8_t g_active_page = 0;          /* Current active page (0 or 1) */
static uint8_t g_page_sequence = 0;        /* Page sequence for determining newest */
static uint8_t g_write_ptr = 0;            /* Write pointer in current page */
static uint32_t g_exception_counter = 0;   /* Total exception count */

/* 1h Window mechanism (GB31241) */
static OvWindowTracker_t g_ov_window[2];     /* [0]=Cell1, [1]=Cell2 */
static TempWindowTracker_t g_temp_window[2]; /* [0]=charging, [1]=discharging */
static uint32_t g_window_start_seconds = 0;
static bool g_window_initialized = false;

/* Monotonic record ID */
static uint32_t g_next_record_id = 1;

/* Static page buffer (avoid stack overflow with 496B struct) */
static FlashPageLayout_t g_flash_page_buffer;

/* Print state (reserved for debug print functions) */
static struct {
    uint8_t current_page;
    uint8_t record_index;
    uint8_t total_records;
    uint8_t records_printed;
    uint8_t initialized;
} print_state __attribute__((unused)) = {0};

/********************* Flash Operation Functions *********************/

static void flash_write_u32(uint32_t addr, uint32_t data) {
    uint32_t tmp = switch_big_little_endian(data);
    hal_fmc_write_word(addr, tmp);
}

static void flash_write_record(uint32_t addr, uint8_t *data, uint16_t len) {
    uint32_t word;
    for (uint16_t i = 0; i < len; i += 4) {
        if (i + 3 < len) {
            word = (data[i+3] << 24) | (data[i+2] << 16) | (data[i+1] << 8) | data[i];
        } else {
            word = 0;
            for (uint16_t j = 0; j < (len - i); j++) {
                word |= (data[i+j] << (j*8));
            }
        }
        flash_write_u32(addr + i, word);
    }
}

static void flash_read_record(uint32_t addr, uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        data[i] = *(uint8_t*)(addr + i);
    }
}

/********************* Time Management Functions *********************/

static void seconds_to_timestamp(uint32_t total_seconds, TimeStamp_t *ts) {
    uint32_t seconds_in_day = 86400;
    uint32_t days = total_seconds / seconds_in_day;
    uint32_t seconds_today = total_seconds % seconds_in_day;

    uint32_t year = 2026;
    uint32_t days_in_year = 365;
    while (days >= days_in_year) {
        days -= days_in_year;
        year++;
        days_in_year = (year % 4 == 0) ? 366 : 365;
    }

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

static bool is_new_hour(uint32_t last_seconds, uint32_t current_seconds) {
    uint32_t elapsed = current_seconds - last_seconds;
    return elapsed >= EXCEPTION_WINDOW_SECONDS;
}

/********************* Exception Thresholds *********************/
/* Battery record thresholds (independent from NTC protection thresholds) */
#define BR_CHRG_OT_VALUE          NTC_10K_3435_REAL_RT_60   /* 60C charging */
#define BR_CHRG_OT_RESTORE_VALUE  NTC_10K_3435_REAL_RT_50   /* 50C restore */
#define BR_DISG_OT_VALUE          NTC_10K_3435_REAL_RT_65   /* 65C discharging */
#define BR_DISG_OT_RESTORE_VALUE  NTC_10K_3435_REAL_RT_55   /* 55C restore */
#define BR_OVER_VOLTAGE_THRESHOLD OVER_VOLTAGE_THRESHOLD     /* config.h: 4500mV */

/********************* Page Checksum *********************/

static uint16_t calculate_page_checksum(FlashPageLayout_t *page) {
    uint32_t sum = 0;
    uint8_t *data = (uint8_t*)page;
    size_t data_len = offsetof(FlashPageLayout_t, checksum);
    for (size_t i = 0; i < data_len; i++) {
        sum += data[i];
    }
    return (uint16_t)(sum & 0xFFFF);
}

static bool verify_page_checksum(FlashPageLayout_t *page) {
    return (calculate_page_checksum(page) == page->checksum);
}

/********************* Multi-Page Flash Operations *********************/

static void switch_active_page(void) {
    g_active_page = (g_active_page + 1) % LOG_PAGE_COUNT;
    g_page_sequence++;
    uint32_t new_page_addr = GET_ACTIVE_PAGE_ADDR(g_active_page);
    hal_fmc_erase_page(new_page_addr);
    g_write_ptr = 0;
    printk("\r\n[PAGE SWITCH] -> page %d (seq=%d)", g_active_page, g_page_sequence);
}

/* Sync latest record to ap->record_storage RAM cache (USB Bridge compatibility) */
static void sync_record_to_ram_cache(BatteryExceptionRecord_t *record) {
    uint8_t wptr = ap->record_storage.write_ptr;
    ap->record_storage.records[wptr] = *record;
    ap->record_storage.write_ptr = (wptr + 1) % MAX_RECORDS;
    if (ap->record_storage.exception_counter < 255)
        ap->record_storage.exception_counter++;
    ap->record_storage.magic = MAGIC_VALUE;
}

/* Write a single record to the active flash page (read-modify-write entire page) */
static void write_exception_record(BatteryExceptionRecord_t *record) {
    if (g_write_ptr >= MAX_RECORDS_PER_PAGE) {
        switch_active_page();
    }

    record->record_id = g_next_record_id++;
    g_exception_counter++;

    uint32_t page_addr = GET_ACTIVE_PAGE_ADDR(g_active_page);

    /* Read existing page */
    flash_read_record(page_addr, (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

    bool is_new_page = (g_flash_page_buffer.magic != MAGIC_VALUE);
    if (is_new_page) {
        memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
        g_flash_page_buffer.page_records_count = 0;
    }

    /* Update header */
    g_flash_page_buffer.magic = MAGIC_VALUE;
    g_flash_page_buffer.page_number = g_active_page;
    g_flash_page_buffer.overflow_ptr = 0;
    g_flash_page_buffer.page_seq = g_page_sequence;

    VIC_vModuleDisable();
    g_flash_page_buffer.page_timestamp = gd->Bat_RTC_Seconds;
    VIC_vModuleEnable();

    /* Write record */
    g_flash_page_buffer.records[g_write_ptr] = *record;
    g_flash_page_buffer.page_records_count = g_write_ptr + 1;

    /* Checksum and write */
    g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);
    hal_fmc_erase_page(page_addr);
    flash_write_record(page_addr, (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

    g_write_ptr++;

    /* Sync to RAM cache for USB Bridge reads */
    sync_record_to_ram_cache(record);

    printk("\r\n[BR WRITE] page=%d idx=%d id=%d type=%d",
           g_active_page, g_write_ptr - 1, record->record_id, record->error_type);
}

/* Load storage from flash - scan all pages, find newest */
static void load_storage_from_flash(void) {
    bool page_valid[2] = {false, false};
    uint8_t page_sequences[2] = {0};
    uint8_t page_record_counts[2] = {0};
    const uint32_t page_addrs[2] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2};

    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        flash_read_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

        bool magic_ok = (g_flash_page_buffer.magic == MAGIC_VALUE);
        bool checksum_ok = verify_page_checksum(&g_flash_page_buffer);
        bool count_ok = (g_flash_page_buffer.page_records_count <= MAX_RECORDS_PER_PAGE);

        if (magic_ok && checksum_ok && count_ok) {
            page_valid[i] = true;
            page_sequences[i] = g_flash_page_buffer.page_seq;
            page_record_counts[i] = g_flash_page_buffer.page_records_count;
            printk("\r\n[LOAD] Page %d valid: seq=%d, count=%d",
                      i, page_sequences[i], page_record_counts[i]);
        } else if (magic_ok && !count_ok) {
            /* Corrupted - repair */
            memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
            g_flash_page_buffer.magic = MAGIC_VALUE;
            g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);
            hal_fmc_erase_page(page_addrs[i]);
            flash_write_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
            page_valid[i] = true;
            page_sequences[i] = 0;
            page_record_counts[i] = 0;
            printk("\r\n[LOAD] Page %d repaired", i);
        }
    }

    /* Find newest valid page by sequence number */
    uint8_t newest_page = 0;
    bool found = false;
    uint8_t newest_seq = 0;

    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        if (page_valid[i]) {
            if (!found) {
                newest_seq = page_sequences[i];
                newest_page = i;
                found = true;
            } else {
                int16_t diff = (int16_t)page_sequences[i] - (int16_t)newest_seq;
                if (diff > 0 || diff < -128) {
                    newest_seq = page_sequences[i];
                    newest_page = i;
                }
            }
        }
    }

    if (found) {
        g_active_page = newest_page;
        g_page_sequence = newest_seq;
        g_write_ptr = page_record_counts[newest_page];

        /* Count total records and find max record_id */
        uint32_t max_id = 0;
        for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
            if (page_valid[i]) {
                g_exception_counter += page_record_counts[i];
                /* Read page to find max record_id */
                flash_read_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
                for (uint8_t j = 0; j < page_record_counts[i]; j++) {
                    if (g_flash_page_buffer.records[j].record_id > max_id) {
                        max_id = g_flash_page_buffer.records[j].record_id;
                    }
                }
            }
        }
        g_next_record_id = max_id + 1;
        if (g_next_record_id == 0) g_next_record_id = 1;

        printk("\r\n[LOAD] Active page=%d, seq=%d, wptr=%d, next_id=%d",
               g_active_page, g_page_sequence, g_write_ptr, g_next_record_id);
    }
}

/********************* Window-based Detection (GB31241) *********************/

/* Process cell overvoltage - RAM only, no Flash write */
static void process_cell_overvoltage(uint8_t cell_num, uint16_t cell_voltage,
                                      uint16_t total_voltage) {
    if (cell_voltage < BR_OVER_VOLTAGE_THRESHOLD) return;

    uint8_t idx = cell_num - 1;  /* 0=Cell1, 1=Cell2 */

    if (!g_ov_window[idx].triggered || cell_voltage > g_ov_window[idx].max_voltage) {
        g_ov_window[idx].triggered = true;
        g_ov_window[idx].max_voltage = cell_voltage;
        g_ov_window[idx].total_voltage = total_voltage;
        g_ov_window[idx].cell_num = cell_num;
        get_current_timestamp(&g_ov_window[idx].max_timestamp);

        printk("[BR] OV Cell%d: %dmV (window max)\n", cell_num, cell_voltage);
    }
}

/* Overvoltage detection - uses X20 MCU ADC sampled cell voltages */
void battery_record_update_overvoltage(void) {
#if(BUCKBOOST_USED_NU6805 == 1)
    /* X20 MCU ADC: cell2_voltage is global (app.c 250ms update, PC7 - VBAT-)
     * cell1 = NU6805 total - cell2 */
    extern uint16_t cell2_voltage;
    uint16_t total_voltage, cell1_voltage;

#if CONFIG_USB_BRIDGE_ENABLE
    {
        uint16_t eng_c1 = usb_bridge_get_eng_cell1();
        uint16_t eng_c2 = usb_bridge_get_eng_cell2();
        if (eng_c1 != ENG_SENTINEL_CELL || eng_c2 != ENG_SENTINEL_CELL) {
            uint16_t real_c2 = cell2_voltage;
            uint16_t real_total = g_buckboost.adc_vbat;
            cell1_voltage = (eng_c1 != ENG_SENTINEL_CELL) ? eng_c1 : (real_total > real_c2 ? real_total - real_c2 : 0);
            uint16_t c2_val = (eng_c2 != ENG_SENTINEL_CELL) ? eng_c2 : real_c2;
            total_voltage = cell1_voltage + c2_val;
            process_cell_overvoltage(1, cell1_voltage, total_voltage);
            process_cell_overvoltage(2, c2_val, total_voltage);
        } else {
            total_voltage = g_buckboost.adc_vbat;
            cell1_voltage = (total_voltage > cell2_voltage) ? (total_voltage - cell2_voltage) : 0;
            process_cell_overvoltage(1, cell1_voltage, total_voltage);
            process_cell_overvoltage(2, cell2_voltage, total_voltage);
        }
    }
#else
    total_voltage = g_buckboost.adc_vbat;
    cell1_voltage = (total_voltage > cell2_voltage) ? (total_voltage - cell2_voltage) : 0;
    process_cell_overvoltage(1, cell1_voltage, total_voltage);
    process_cell_overvoltage(2, cell2_voltage, total_voltage);
#endif
#elif(BUCKBOOST_USED_NU6801 == 1)
    uint16_t cell1_voltage = g_buckboost.adc_vbat;
    uint16_t total_voltage = cell1_voltage;
    process_cell_overvoltage(1, cell1_voltage, total_voltage);
#endif
}

/* Temperature detection - uses X20 pre-computed ntc_temp_wpc */
void battery_record_update_temperature(void) {
    uint8_t mode = g_buckboost.woke_mode;

#if (BUCKBOOST_USED_NU6805 == 1)
    int16_t ntc_temp;
#if CONFIG_USB_BRIDGE_ENABLE
    {
        int16_t eng_temp = usb_bridge_get_eng_temp();
        if (eng_temp != (int16_t)ENG_SENTINEL_TEMP) {
            ntc_temp = eng_temp;
        } else {
            ntc_temp = gd->sys_infos.ntc_temp_wpc;
        }
    }
#else
    ntc_temp = gd->sys_infos.ntc_temp_wpc;  /* 0.1degC from APL pre-compute */
#endif
    bool is_abnormal = (ntc_temp > CHRG_NTC_OT_TEMP_VALUE);
    uint8_t event_type = EXCEPTION_TYPE_OVERTEMP;
#else
    /* NU6801: NTC resistance based */
    uint16_t ntc_resistance = g_buckboost.adc_tbat1;
    int16_t ntc_temp = ntc_to_temp(ntc_resistance) * 10;
    uint8_t event_type = EXCEPTION_TYPE_OVERTEMP;
    bool is_abnormal;
    if (mode == BUCKBOOST_CHAGER_MODE) {
        is_abnormal = (ntc_resistance < BR_CHRG_OT_VALUE);
    } else if (mode == BUCKBOOST_DISCHG_MODE) {
        is_abnormal = (ntc_resistance < BR_DISG_OT_VALUE);
    } else {
        is_abnormal = false;
    }
#endif

    if (!is_abnormal) return;

    uint8_t idx;
    if (mode == BUCKBOOST_CHAGER_MODE) {
        idx = 0;
    } else if (mode == BUCKBOOST_DISCHG_MODE) {
        idx = 1;
    } else {
        idx = 0;  /* Default to charging slot */
    }

    if (!g_temp_window[idx].triggered || ntc_temp > g_temp_window[idx].max_temperature) {
        g_temp_window[idx].triggered = true;
        g_temp_window[idx].max_temperature = ntc_temp;
        g_temp_window[idx].event_type = event_type;
        g_temp_window[idx].charge_state = mode;
        get_current_timestamp(&g_temp_window[idx].max_timestamp);

        printk("[BR] TEMP %s: %d (window max)\n",
            (mode == BUCKBOOST_CHAGER_MODE) ? "CHG" : "DCHG", ntc_temp);
    }
}

/* Process window end: flush triggered exceptions to Flash */
static void process_window_end(void) {
    /* Overvoltage: Cell1 and Cell2 */
    for (int i = 0; i < 2; i++) {
        if (g_ov_window[i].triggered) {
            BatteryExceptionRecord_t record;
            record.timestamp = g_ov_window[i].max_timestamp;
            record.error_type = EXCEPTION_TYPE_OVERVOLTAGE;
            record.sub_type = g_ov_window[i].cell_num;
            record.data.ov_data.max_voltage = g_ov_window[i].max_voltage;
            record.data.ov_data.total_voltage = g_ov_window[i].total_voltage;
            record.record_id = 0;  /* write_exception_record assigns ID */

            write_exception_record(&record);
            printk("[BR] WINDOW OV Cell%d: %dmV -> Flash\n",
                g_ov_window[i].cell_num, g_ov_window[i].max_voltage);
        }
    }

    /* Temperature: charging and discharging */
    for (int i = 0; i < 2; i++) {
        if (g_temp_window[i].triggered) {
            BatteryExceptionRecord_t record;
            record.timestamp = g_temp_window[i].max_timestamp;
            record.error_type = g_temp_window[i].event_type;
            record.sub_type = g_temp_window[i].charge_state;
            record.data.temp_data.max_temperature = g_temp_window[i].max_temperature;
            record.data.temp_data.reserved = 0;
            record.record_id = 0;

            write_exception_record(&record);
            printk("[BR] WINDOW TEMP %s: %d -> Flash\n",
                (g_temp_window[i].charge_state == BUCKBOOST_CHAGER_MODE) ? "CHG" : "DCHG",
                g_temp_window[i].max_temperature);
        }
    }

    /* Clear all window trackers */
    memset(g_ov_window, 0, sizeof(g_ov_window));
    memset(g_temp_window, 0, sizeof(g_temp_window));
}

/********************* Public API *********************/

void battery_record_init(void) {
    g_active_page = 0;
    g_page_sequence = 0;
    g_write_ptr = 0;
    g_exception_counter = 0;
    g_next_record_id = 1;
    g_window_initialized = false;

    memset(g_ov_window, 0, sizeof(g_ov_window));
    memset(g_temp_window, 0, sizeof(g_temp_window));

    load_storage_from_flash();

    printk("\r\n[BR INIT] pages=%d, active=%d, records=%d",
           LOG_PAGE_COUNT, g_active_page, g_exception_counter);
}

/* Periodic check: 1h window mechanism */
void battery_record_periodic_check(void) {
    static uint16_t check_counter = 0;

    if (++check_counter < 10) return;  /* Every 1s (10 × 100ms) */
    check_counter = 0;

    /* Initialize window on first call */
    if (!g_window_initialized) {
        VIC_vModuleDisable();
        g_window_start_seconds = gd->Bat_RTC_Seconds;
        VIC_vModuleEnable();
        g_window_initialized = true;
        memset(g_ov_window, 0, sizeof(g_ov_window));
        memset(g_temp_window, 0, sizeof(g_temp_window));
        printk("[BR] Window initialized\n");
    }

    /* Detect exceptions (RAM only, no Flash) */
    battery_record_update_overvoltage();
    battery_record_update_temperature();

#if CONFIG_USB_BRIDGE_ENABLE
    /* Engineering mode: immediate flush when virtual injection triggers exception */
    {
        bool eng_triggered = false;
        uint16_t eng_c1 = usb_bridge_get_eng_cell1();
        uint16_t eng_c2 = usb_bridge_get_eng_cell2();
        int16_t eng_temp = usb_bridge_get_eng_temp();
        if (eng_c1 != ENG_SENTINEL_CELL || eng_c2 != ENG_SENTINEL_CELL ||
            eng_temp != (int16_t)ENG_SENTINEL_TEMP) {
            for (int i = 0; i < 2; i++) {
                if (g_ov_window[i].triggered || g_temp_window[i].triggered) {
                    eng_triggered = true;
                    break;
                }
            }
        }
        if (eng_triggered) {
            process_window_end();
            printk("[BR] Eng mode: immediate flush\n");
        }
    }
#endif

    /* Check if window has expired */
    uint32_t current_seconds;
    VIC_vModuleDisable();
    current_seconds = gd->Bat_RTC_Seconds;
    VIC_vModuleEnable();

    if (is_new_hour(g_window_start_seconds, current_seconds)) {
        process_window_end();
        g_window_start_seconds = current_seconds;
        printk("[BR] New window started\n");
    }

    /* Print RTC for debug */
    TimeStamp_t ts;
    get_current_timestamp(&ts);
    printk("RTC: %04d-%02d-%02d %02d:%02d:%02d\n",
        ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
}

/* Read exception records from Flash (for USB Bridge telemetry) */
uint8_t battery_record_read_exceptions(BatteryExceptionRecord_t *buf, uint8_t max_count) {
    if (buf == NULL || max_count == 0) return 0;

    uint8_t read_count = 0;
    const uint32_t page_addrs[2] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2};

    /* Read from all valid pages, newest first */
    for (uint8_t p = 0; p < LOG_PAGE_COUNT && read_count < max_count; p++) {
        /* Start from active page, go backwards */
        uint8_t page_idx = (g_active_page + LOG_PAGE_COUNT - p) % LOG_PAGE_COUNT;
        flash_read_record(page_addrs[page_idx], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

        if (g_flash_page_buffer.magic != MAGIC_VALUE) continue;
        if (!verify_page_checksum(&g_flash_page_buffer)) continue;

        for (uint8_t i = 0; i < g_flash_page_buffer.page_records_count && read_count < max_count; i++) {
            buf[read_count++] = g_flash_page_buffer.records[i];
        }
    }
    return read_count;
}

void battery_record_print_next_log(void) {
    /* Placeholder - re-enable for debug if needed */
}

/* Erase all exception records */
void battery_record_erase_all(void) {
    hal_fmc_erase_page(FLASH_LOG_PAGE1);
    hal_fmc_erase_page(FLASH_LOG_PAGE2);

    g_active_page = 0;
    g_page_sequence = 0;
    g_write_ptr = 0;
    g_exception_counter = 0;
    g_next_record_id = 1;

    memset(g_ov_window, 0, sizeof(g_ov_window));
    memset(g_temp_window, 0, sizeof(g_temp_window));
    g_window_initialized = false;

    printk("\r\n[BR] All records erased");
}

/* Reset tracking state without erasing records */
void battery_record_reset_tracking(void) {
    memset(g_ov_window, 0, sizeof(g_ov_window));
    memset(g_temp_window, 0, sizeof(g_temp_window));
    g_window_initialized = false;
    printk("\r\n[BR] Tracking reset");
}

/* Read single record by page number and record index (for USB Bridge sequential push) */
bool battery_record_read_by_page_index(uint8_t page, uint8_t index, BatteryExceptionRecord_t *record) {
    if (record == NULL || page >= LOG_PAGE_COUNT || index >= MAX_RECORDS_PER_PAGE) {
        return false;
    }
    uint32_t page_addr = GET_ACTIVE_PAGE_ADDR(page);
    uint32_t record_offset = offsetof(FlashPageLayout_t, records) +
                             (index * sizeof(BatteryExceptionRecord_t));
    flash_read_record(page_addr + record_offset, (uint8_t*)record, sizeof(BatteryExceptionRecord_t));
    return true;
}

/* Get record count for a page (header-only read, 5 bytes) */
uint8_t battery_record_get_page_count(uint8_t page) {
    if (page >= LOG_PAGE_COUNT) return 0;
    uint32_t addr = GET_ACTIVE_PAGE_ADDR(page);
    struct { uint32_t magic; uint8_t count; } hdr;
    flash_read_record(addr, (uint8_t*)&hdr, 5);
    if (hdr.magic != MAGIC_VALUE) return 0;
    return (hdr.count <= MAX_RECORDS_PER_PAGE) ? hdr.count : MAX_RECORDS_PER_PAGE;
}

#endif /* CONFIG_NEW_CCC_LOG_ENABLE */
