#include "bat_record.h"
#include "g_data.h"
#include "nu6805.h"
#include "nu6801.h"
#include "buckboost.h"
#include "ntc.h"
#include "fmc.h"
#include "app.h"
#include "printk.h"
#include <string.h>
#include <stddef.h>
#include "../hal/regdef.h"
#include "../hal/badc.h"
#include "../hal/i2cm.h"
#include"_fml.h"
#include "usb_bridge.h"

/********************* Global Variables **********************/
// Variables defined in ap_t structure, accessed via ap->
#define g_exception_cache   (ap->exception_cache)
#define g_record_storage    (ap->record_storage)

// Flag: set when engineering mode virtual value injection triggers an exception
// Used by periodic_check to decide whether to flush window immediately
static bool g_eng_virtual_triggered = false;

// Monotonic record ID counter (initialized in battery_record_init, always >= 1, never 0)
static uint32_t g_next_record_id = 0;  /* 单调 record ID，在 battery_record_init() 中初始化为 >=1 */

// Print state tracker
static struct {
    uint8_t current_page;       // Current page being printed
    uint8_t record_index;       // Record index in current page
    uint8_t total_records;      // Total records across all pages
    uint8_t records_printed;    // Records printed so far
    uint8_t initialized;        // Initialization flag
} print_state = {0};

// Static buffer for reading flash pages (avoid stack overflow)
// Can be disabled if BAT_RECORD_USE_STACK_BUFFER=1 (saves 496 bytes RAM but requires larger stack)
#if (BAT_RECORD_USE_STACK_BUFFER == 0)
    // Safe mode: Use static buffer (496 bytes in .bss, no stack pressure)
    static FlashPageLayout_t g_flash_page_buffer;
    #define PAGE_BUFFER_PTR  (&g_flash_page_buffer)
    #define PAGE_BUFFER      g_flash_page_buffer
    #define DECLARE_PAGE_BUFFER()  /* nothing */
#else
    // Stack mode: Use local variable (saves 496 bytes RAM, requires stack >= 600 bytes)
    #define PAGE_BUFFER_PTR  (&page_buffer_local)
    #define PAGE_BUFFER      page_buffer_local
    #define DECLARE_PAGE_BUFFER()  FlashPageLayout_t page_buffer_local
#endif

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

    return elapsed_seconds >= EXCEPTION_WINDOW_SECONDS;
}

/* Battery record exception thresholds (independent from NTC protection thresholds)
 * Charging:  60°C record exception (protection still at 40°C)
 * Discharging: 65°C record exception (protection still at 70°C)
 * Overvoltage: 4510mV record exception (protection still at 4500mV) */
#define BR_CHRG_OT_VALUE          NTC_10K_3435_REAL_RT_60   // 60°C charging record threshold
#define BR_CHRG_OT_RESTORE_VALUE  NTC_10K_3435_REAL_RT_50   // 50°C charging record restore (hysteresis 10°C)
#define BR_DISG_OT_VALUE          NTC_10K_3435_REAL_RT_65   // 65°C discharging record threshold
#define BR_DISG_OT_RESTORE_VALUE  NTC_10K_3435_REAL_RT_55   // 55°C discharging record restore (hysteresis 10°C)
#define BR_OVER_VOLTAGE_THRESHOLD 4530                         // 4.51V record threshold (mV)

#if 0  /* Removed: unified window model no longer uses recovery-based tracking */
static bool is_temperature_recovered(uint16_t ntc_resistance, uint8_t mode) {
    if (mode == BUCKBOOST_CHAGER_MODE) {
        return (ntc_resistance >= BR_CHRG_OT_RESTORE_VALUE);
    } else if (mode == BUCKBOOST_DISCHG_MODE) {
        return (ntc_resistance >= BR_DISG_OT_RESTORE_VALUE);
    }
    return true;
}
#endif

// Check temperature abnormal status - different thresholds for charge/discharge
static bool is_temperature_abnormal(uint16_t ntc_resistance, uint8_t mode, uint8_t *event_type) {
    if (mode == BUCKBOOST_CHAGER_MODE) {
        // Charging mode - 60°C record threshold
        if (ntc_resistance < BR_CHRG_OT_VALUE) {
            *event_type = EXCEPTION_TYPE_OVERTEMP;  // 0x02 Over temperature
            return true;
        }
    } else if (mode == BUCKBOOST_DISCHG_MODE) {
        // Discharging mode - 65°C record threshold
        if (ntc_resistance < BR_DISG_OT_VALUE) {
            *event_type = EXCEPTION_TYPE_OVERTEMP;  // 0x02 Over temperature
            return true;
        }
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

/********************* Dual-Page Management Functions *********************/

// Page switching function - switches to next page (supports 1-3 pages)
static void switch_active_page(void) {
    // Cycle to next page: 0 �� 1 �� 2 �� 0 (for 3 pages)
    //                 or: 0 �� 1 �� 0 (for 2 pages)
    //                 or: 0 �� 0 (for 1 page, no switching)
    g_record_storage.active_page = (g_record_storage.active_page + 1) % LOG_PAGE_COUNT;

    // Increment page sequence for wear leveling tracking
    g_record_storage.page_sequence++;

    // Get new active page address
    uint32_t new_page_addr = GET_ACTIVE_PAGE_ADDR(g_record_storage.active_page);

    // Erase new active page
    hal_fmc_erase_page(new_page_addr);

    // Reset write pointer for new page
    g_record_storage.write_ptr = 0;

    br_printk_debug("\r\n[PAGE SWITCH] Switched to page %d (sequence: %d, total pages: %d)",
              g_record_storage.active_page, g_record_storage.page_sequence, LOG_PAGE_COUNT);
}

/********************* Flash Record Read/Write Functions *********************/

// Calculate checksum for flash page layout
static uint16_t calculate_page_checksum(FlashPageLayout_t *page) {
    uint32_t sum = 0;
    uint8_t *data = (uint8_t*)page;

    // Calculate all data before checksum field
    size_t data_len = offsetof(FlashPageLayout_t, checksum);

    for (size_t i = 0; i < data_len; i++) {
        sum += data[i];
    }

    return (uint16_t)(sum & 0xFFFF);
}

// Write single record to Flash - optimized version (no RAM cache)
static void save_record_to_flash(BatteryExceptionRecord_t *record, uint8_t record_index) {
    DECLARE_PAGE_BUFFER();  // Declare local buffer if stack mode enabled

    // Validate record index to prevent array overflow
    if (record_index >= MAX_RECORDS_PER_PAGE) {
        br_printk_debug("\r\n[ERROR] Invalid record_index: %d (max: %d)", record_index, MAX_RECORDS_PER_PAGE - 1);
        return;
    }

    // Get active page address
    uint32_t page_addr = GET_ACTIVE_PAGE_ADDR(g_record_storage.active_page);

    // Read existing page from Flash
    flash_read_record(page_addr, (uint8_t*)PAGE_BUFFER_PTR, sizeof(FlashPageLayout_t));

    // Check if this is a new/erased page (magic doesn't match)
    bool is_new_page = (PAGE_BUFFER.magic != MAGIC_VALUE);

    if (is_new_page) {
        // Initialize all header fields for new page
        memset(PAGE_BUFFER_PTR, 0, sizeof(FlashPageLayout_t));
        PAGE_BUFFER.page_records_count = 0;
    }

    // Update page header
    PAGE_BUFFER.magic = MAGIC_VALUE;
    PAGE_BUFFER.page_number = g_record_storage.active_page;
    PAGE_BUFFER.overflow_ptr = 0;  // Reserved
    PAGE_BUFFER.page_seq = g_record_storage.page_sequence;  // Write sequence to flash for determining newest page

    // Update record count (ensure it doesn't exceed max)
    if (record_index >= PAGE_BUFFER.page_records_count) {
        PAGE_BUFFER.page_records_count = record_index + 1;
        if (PAGE_BUFFER.page_records_count > MAX_RECORDS_PER_PAGE) {
            PAGE_BUFFER.page_records_count = MAX_RECORDS_PER_PAGE;
        }
    }

    // Get timestamp
    VIC_vModuleDisable();
    PAGE_BUFFER.page_timestamp = gd->Bat_RTC_Seconds;
    VIC_vModuleEnable();

    // Update the specific record
    PAGE_BUFFER.records[record_index] = *record;

    // Calculate and set checksum
    PAGE_BUFFER.checksum = calculate_page_checksum(PAGE_BUFFER_PTR);

    // Erase and write entire page
    hal_fmc_erase_page(page_addr);
    flash_write_record(page_addr, (uint8_t*)PAGE_BUFFER_PTR, sizeof(FlashPageLayout_t));

    // Update RAM storage checksum
    g_record_storage.checksum = calculate_checksum(&g_record_storage);
}

// Verify flash page checksum
static bool verify_page_checksum(FlashPageLayout_t *page) {
    uint16_t calculated = calculate_page_checksum(page);
    return (calculated == page->checksum);
}

// Load storage structure from Flash to RAM - multi-page version
static void load_storage_from_flash(void) {
    // Use static buffer to avoid stack overflow (avoid 1488 bytes on stack)
    bool page_valid[3] = {false, false, false};
    uint8_t page_sequences[3] = {0};  // Page sequence numbers for determining newest
    uint8_t page_record_counts[3] = {0};

    // Read all configured pages
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        flash_read_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

        // Validate page: check magic, checksum, and record count
        bool magic_ok = (g_flash_page_buffer.magic == MAGIC_VALUE);
        bool checksum_ok = verify_page_checksum(&g_flash_page_buffer);
        bool count_ok = (g_flash_page_buffer.page_records_count <= MAX_RECORDS_PER_PAGE);

        if (magic_ok && checksum_ok && count_ok) {
            page_valid[i] = true;
            page_sequences[i] = g_flash_page_buffer.page_seq;  // Read sequence number
            page_record_counts[i] = g_flash_page_buffer.page_records_count;
            br_printk_debug("\r\n[LOAD] Page %d valid: seq=%d, count=%d",
                      i, page_sequences[i], page_record_counts[i]);
        } else if (magic_ok && !count_ok) {
            // Page has correct magic but corrupted record count - repair it
            br_printk_debug("\r\n[LOAD] Page %d corrupted (count=%d), repairing...",
                      i, g_flash_page_buffer.page_records_count);

            // Reinitialize this page
            memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
            g_flash_page_buffer.magic = MAGIC_VALUE;
            g_flash_page_buffer.page_records_count = 0;
            g_flash_page_buffer.page_number = i;
            g_flash_page_buffer.page_seq = 0;
            g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);

            hal_fmc_erase_page(page_addrs[i]);
            flash_write_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

            // Mark as valid with 0 records
            page_valid[i] = true;
            page_sequences[i] = 0;
            page_record_counts[i] = 0;

            br_printk_debug("\r\n[LOAD] Page %d repaired successfully", i);
        }
    }

    // Find the newest valid page based on sequence number (handles wraparound)
    // Higher sequence = newer page (with 8-bit wraparound handling)
    uint8_t newest_seq = 0;
    uint8_t newest_page_num = 0;
    bool found_valid_page = false;

    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        if (page_valid[i]) {
            if (!found_valid_page) {
                // First valid page
                newest_seq = page_sequences[i];
                newest_page_num = i;
                found_valid_page = true;
            } else {
                // Compare sequences (handle 8-bit wraparound: if diff > 128, assume wraparound)
                int16_t diff = (int16_t)page_sequences[i] - (int16_t)newest_seq;
                if (diff > 0 || diff < -128) {
                    // page_sequences[i] is newer
                    newest_seq = page_sequences[i];
                    newest_page_num = i;
                }
            }
        }
    }

    // If no valid page found, return
    if (!found_valid_page) {
        return;
    }

    br_printk_debug("\r\n[LOAD] Newest page is %d (seq=%d)", newest_page_num, newest_seq);

    // Set active page
    g_record_storage.active_page = newest_page_num;

    // Calculate total records across all pages for global counter
    uint8_t total_records = 0;
    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        if (page_valid[i]) {
            total_records += page_record_counts[i];
        }
    }

    // Load metadata
    g_record_storage.magic = MAGIC_VALUE;
    g_record_storage.write_ptr = page_record_counts[newest_page_num];
    g_record_storage.exception_counter = total_records;  // Global counter across all pages
    g_record_storage.page_sequence = newest_seq;         // Restore sequence from flash

    // No need to load records into RAM (we don't cache them anymore)

    // Update checksum
    g_record_storage.checksum = calculate_checksum(&g_record_storage);

    br_printk_debug("\r\n[LOAD] Page %d loaded, %d records (total: %d)",
              g_record_storage.active_page, g_record_storage.write_ptr, total_records);
}

// Write exception record - directly to Flash (no RAM cache)
static void write_exception_record(BatteryExceptionRecord_t *record) {
    // Check if current page is full
    if (g_record_storage.write_ptr >= MAX_RECORDS_PER_PAGE) {
        // Page full, switch to other page
        switch_active_page();
    }

    // Update counter first (total records count, separate from record_id)
    if (g_record_storage.exception_counter < 255) {
        g_record_storage.exception_counter++;
    }

    // Set record ID from monotonic counter (never 0, never repeats)
    record->record_id = g_next_record_id++;

    // Write record directly to Flash at current position
    save_record_to_flash(record, g_record_storage.write_ptr);

    // Increment write pointer
    g_record_storage.write_ptr++;

    br_printk_debug("\r\n[WRITE] Done. Next write will be at page %d, index %d",
              g_record_storage.active_page, g_record_storage.write_ptr);

    // Check if Flash exception records are full → trigger OV_FORBID
    {
        uint8_t flash_total = 0;
        for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
            flash_total += battery_record_get_page_count(p);
        }
        if (flash_total >= MAX_TOTAL_RECORDS) {
            gd->bat_ov_forbid_flag = 1;
#if OV_FORBID_FLASH_PERSIST
            cycle_count_save_to_flash();
#endif
            buckboost_set_work_mode(BUCKBOOST_SHUTDOWM_MODE);
            printk("[BR] EXCEPTION FULL (%d/%d) -> FORBID!\n",
                   flash_total, MAX_TOTAL_RECORDS);
        }
    }
}

/********************* Public API Function Implementations *********************/

/********************* Migration Functions *********************/

// Migrate from old format (V5/V6) to new dual-page format (V7)
static void migrate_old_format(void) {
    br_printk_debug("\r\n[MIGRATE] Migrating from old format...");

    // Read old format from LOG1 (assume old data was stored there)
    // Old structure: magic(4) + counter(1) + write_ptr(1) + padding(2) + records[5]*20 + checksum(2)
    typedef struct {
        uint32_t magic;
        uint8_t  exception_counter;
        uint8_t  write_ptr;
        uint16_t padding1;
        BatteryExceptionRecord_t records[5];
        uint16_t checksum;
    } OldBatteryRecordStorage_t;

    OldBatteryRecordStorage_t old_storage;
    flash_read_record(FLASH_LOG_PAGE1, (uint8_t*)&old_storage, sizeof(OldBatteryRecordStorage_t));

    // Initialize new RAM storage
    memset((void*)&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
    g_record_storage.magic = MAGIC_VALUE;  // V7
    g_record_storage.active_page = FLASH_PAGE_LOG1;
    g_record_storage.page_sequence = 0;

    // Copy old records to new format
    uint8_t records_to_copy = (old_storage.exception_counter < 5) ?
                               old_storage.exception_counter : 5;
    g_record_storage.write_ptr = records_to_copy;
    g_record_storage.exception_counter = records_to_copy;

    // Write old records to new flash page format (use static buffer to avoid stack overflow)
    uint32_t page_addr = GET_ACTIVE_PAGE_ADDR(FLASH_PAGE_LOG1);
    memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));

    g_flash_page_buffer.magic = MAGIC_VALUE;
    g_flash_page_buffer.page_records_count = records_to_copy;
    g_flash_page_buffer.page_number = FLASH_PAGE_LOG1;
    VIC_vModuleDisable();
    g_flash_page_buffer.page_timestamp = gd->Bat_RTC_Seconds;
    VIC_vModuleEnable();

    // Copy old records
    for (uint8_t i = 0; i < records_to_copy; i++) {
        g_flash_page_buffer.records[i] = old_storage.records[i];
    }

    g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);

    hal_fmc_erase_page(page_addr);
    flash_write_record(page_addr, (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

    // Initialize remaining pages as empty (reuse static buffer)
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    for (uint8_t i = 1; i < LOG_PAGE_COUNT; i++) {  // Start from 1, skip LOG1
        memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
        g_flash_page_buffer.magic = MAGIC_VALUE;
        g_flash_page_buffer.page_records_count = 0;
        g_flash_page_buffer.page_number = i;
        g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);

        hal_fmc_erase_page(page_addrs[i]);
        flash_write_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
    }

    br_printk_debug("\r\n[MIGRATE] Migration complete, %d records preserved, %d pages initialized",
              records_to_copy, LOG_PAGE_COUNT);
}

/********************* Public API Function Implementations *********************/

#if 0  /* Removed: unified window model no longer needs Flash-based tracking restore */
// Restore exception tracking state from the last Flash record (used by sleep_check).
static void restore_exception_tracking_from_flash(void) { /* ... */ }
#endif

// Initialization function - with multi-page support and migration
void battery_record_init(void) {
    // Read magic values from all configured pages
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};
    uint32_t page_magics[3];
    bool has_new_format = false;

    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        page_magics[i] = flash_read_u32(page_addrs[i]);
        if (page_magics[i] == MAGIC_VALUE) {
            has_new_format = true;
        }
    }

    /* T1 - verified, disabled */

    bool need_init = false;
    bool need_migration = false;
    printk("[BR-INIT] magic=%d power=%x magics=[%08X,%08X]\n",
           has_new_format, gd->power_on_magic, page_magics[0], page_magics[1]);

    // Check for old format (V5 or V6) in LOG1
    if ((page_magics[0] == MAGIC_VALUE_V5 || page_magics[0] == MAGIC_VALUE_V6) &&
        !has_new_format) {
        // Old format detected, need migration
        need_migration = true;
        br_printk_debug("\r\n[INIT] Old format detected, migration required");
    }
    // Check for new format (V7) in any page
    else if (has_new_format) {
        // New format exists, load it
        load_storage_from_flash();

        // Verify checksum
        if (!verify_storage_checksum(&g_record_storage)) {
            // Checksum error, reinitialize
            need_init = true;
            printk("[BR-INIT] CHECKSUM FAIL! page=%d wptr=%d cnt=%d\n",
                   g_record_storage.active_page, g_record_storage.write_ptr,
                   g_record_storage.exception_counter);
        } else {
            printk("[BR-INIT] OK page=%d wptr=%d cnt=%d\n",
                   g_record_storage.active_page, g_record_storage.write_ptr,
                   g_record_storage.exception_counter);
            /* 从 Flash 各页扫描最大 record_id，恢复单调计数器 */
            {
                uint32_t max_rid = 0;
                const uint32_t scan_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};
                for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
                    flash_read_record(scan_addrs[p], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
                    if (g_flash_page_buffer.magic == MAGIC_VALUE) {
                        for (uint8_t i = 0; i < g_flash_page_buffer.page_records_count && i < MAX_RECORDS_PER_PAGE; i++) {
                            if (g_flash_page_buffer.records[i].record_id > max_rid)
                                max_rid = g_flash_page_buffer.records[i].record_id;
                        }
                    } else {
                        /* invalid magic, skip */
                    }
                }
                g_next_record_id = (max_rid > 0) ? max_rid + 1 : 1;
                /* T1 verified: record_id recovery correct */
            }
        }
    }
    else {
        // No valid data found, initialize fresh
        need_init = true;
        printk("[BR-INIT] NO VALID DATA! magics=[%08X,%08X]\n", page_magics[0], page_magics[1]);
    }

    // Perform migration if needed
    if (need_migration) {
        migrate_old_format();
        g_next_record_id = 1;  /* Migration: old records may have id=0, start fresh from 1 */
        br_printk_debug("\r\n[INIT] migrated, nxt=1");
    }

    // Initialize fresh if needed
    if (need_init) {
        // Initialize RAM storage
        memset((void*)&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
        g_record_storage.magic = MAGIC_VALUE;
        g_record_storage.exception_counter = 0;
        g_next_record_id = 1;  /* Fresh start: monotonic ID begins at 1 */
        g_record_storage.write_ptr = 0;
        g_record_storage.active_page = FLASH_PAGE_LOG1;  // Start with page 1
        g_record_storage.page_sequence = 0;

        // Initialize all configured pages (use static buffer to avoid stack overflow)
        const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

        for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
            memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
            g_flash_page_buffer.magic = MAGIC_VALUE;
            g_flash_page_buffer.page_records_count = 0;
            g_flash_page_buffer.page_number = i;
            g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);

            hal_fmc_erase_page(page_addrs[i]);
            flash_write_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
        }

        br_printk_debug("\r\n[INIT] All %d pages initialized", LOG_PAGE_COUNT);
    }

    // Reset print state before printing (ensure clean start)
    print_state.initialized = 0;
    print_state.current_page = 0;
    print_state.record_index = 0;
    print_state.records_printed = 0;
    print_state.total_records = 0;

    // Always print existing logs after initialization
    battery_record_print_next_log();

    // Initialize exception tracking cache (unified window)
    // Cannot use power_on_magic (gd_data_init already set it to 0xaaaa before we run)
    // Instead, validate exception_cache content: if window_start_seconds is invalid, clear all
    if (g_exception_cache.window_start_seconds == 0 ||
        g_exception_cache.window_start_seconds == 0xFFFFFFFF ||
        g_exception_cache.ov1_triggered > 1 ||
        g_exception_cache.ov2_triggered > 1 ||
        g_exception_cache.temp_chg_triggered > 1 ||
        g_exception_cache.temp_dchg_triggered > 1) {
        memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
        printk("[BR-INIT] cache cleared (invalid data)\n");
    } else {
        printk("[BR-INIT] cache preserved (window continues)\n");
    }
}

// Cell overvoltage processing function - unified window, operates on g_exception_cache
static void process_cell_overvoltage(uint8_t cell_num, uint16_t cell_voltage,
                                      uint16_t total_voltage) {
    if (cell_voltage < BR_OVER_VOLTAGE_THRESHOLD) return;

    if (cell_num == 1) {
        if (!g_exception_cache.ov1_triggered || cell_voltage > g_exception_cache.ov1_max_voltage) {
            g_exception_cache.ov1_triggered = 1;
            g_exception_cache.ov1_max_voltage = cell_voltage;
            g_exception_cache.ov1_total_voltage = total_voltage;
            get_current_timestamp(&g_exception_cache.ov1_timestamp);
            br_printk("[BR] OV Cell1: %dmV (window max updated)\n", cell_voltage);
        }
    } else {
        if (!g_exception_cache.ov2_triggered || cell_voltage > g_exception_cache.ov2_max_voltage) {
            g_exception_cache.ov2_triggered = 1;
            g_exception_cache.ov2_max_voltage = cell_voltage;
            g_exception_cache.ov2_total_voltage = total_voltage;
            get_current_timestamp(&g_exception_cache.ov2_timestamp);
            br_printk("[BR] OV Cell2: %dmV (window max updated)\n", cell_voltage);
        }
    }
}

// Overvoltage detection and record function
void battery_record_update_overvoltage(void) {
#if(BUCKBOOST_USED_NU6805 == 1)
    /* Engineering mode: inject virtual Cell1/Cell2 voltage if set (not sentinel) */
    bool ov_using_virtual1 = (gd->eng_mode_active && gd->eng_virtual_cell1 != VIRTUAL_CELL_SENTINEL);
    bool ov_using_virtual2 = (gd->eng_mode_active && gd->eng_virtual_cell2 != VIRTUAL_CELL_SENTINEL);
    uint16_t cell1_voltage = ov_using_virtual1 ? gd->eng_virtual_cell1 : g_buckboost.adc_vcell1;
    uint16_t cell2_voltage = ov_using_virtual2 ? gd->eng_virtual_cell2 : g_buckboost.adc_vcell2;

    /* Suspect reading filter: > 5500mV likely ADC glitch.
     * Must see 50 consecutive readings (5 × 1s = 5s) before trusting it.
     * Engineering mode virtual values bypass this filter. */
    static uint8_t suspect_cell1_cnt = 0;
    static uint8_t suspect_cell2_cnt = 0;
    if (!ov_using_virtual1 && cell1_voltage > 5500) {
        if (suspect_cell1_cnt < 5) {
            suspect_cell1_cnt++;
            cell1_voltage = 0;  /* 不够 5 次，暂不采信 */
        }
        /* >= 5 次：放行原值，让 process_cell_overvoltage 处理 */
    } else {
        suspect_cell1_cnt = 0;
    }
    if (!ov_using_virtual2 && cell2_voltage > 5500) {
        if (suspect_cell2_cnt < 5) {
            suspect_cell2_cnt++;
            cell2_voltage = 0;
        }
    } else {
        suspect_cell2_cnt = 0;
    }
    uint16_t total_voltage = cell1_voltage + cell2_voltage;

    br_printk("[BR] OV: c1=%d c2=%d thr=%d eng=%d vc1=%d vc2=%d\n",
              cell1_voltage, cell2_voltage, BR_OVER_VOLTAGE_THRESHOLD,
              gd->eng_mode_active, gd->eng_virtual_cell1, gd->eng_virtual_cell2);

    process_cell_overvoltage(1, cell1_voltage, total_voltage);
    process_cell_overvoltage(2, cell2_voltage, total_voltage);

    /* Engineering mode single-shot: consume virtual values and flag for immediate flush */
    if (ov_using_virtual1 || ov_using_virtual2) {
        gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
        g_eng_virtual_triggered = true;
    }
#elif(BUCKBOOST_USED_NU6801 == 1)
    /* Engineering mode: inject virtual Cell1 voltage if set (not sentinel) */
    bool ov_using_virtual = (gd->eng_mode_active && gd->eng_virtual_cell1 != VIRTUAL_CELL_SENTINEL);
    uint16_t cell1_voltage = ov_using_virtual ? gd->eng_virtual_cell1 : g_buckboost.real_adc_vbat;
    uint16_t total_voltage = cell1_voltage;  /* single cell: total = Cell1 */
    br_printk("[BR] OV: cell1=%dmV thr=%d eng=%d virt_cell1=%d\n",
              cell1_voltage, BR_OVER_VOLTAGE_THRESHOLD,
              gd->eng_mode_active, gd->eng_virtual_cell1);
    process_cell_overvoltage(1, cell1_voltage, total_voltage);

    /* Engineering mode single-shot: consume virtual value and flag for immediate flush */
    if (ov_using_virtual) {
        gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
        g_eng_virtual_triggered = true;
    }
#endif
}

/* Engineering mode temperature thresholds (0.1°C units, matching eng_virtual_temp from PC):
 *   BR_CHRG_OT = 60°C → 600 (0.1°C)
 *   BR_DISG_OT = 65°C → 650 (0.1°C)
 * Note: All temperature storage uses 0.1°C units for PC compatibility. */
#define ENG_CHRG_OT_TEMP_DEGC   600   /* 60.0°C charging record threshold (0.1°C) */
#define ENG_DISG_OT_TEMP_DEGC   650   /* 65.0°C discharging record threshold (0.1°C) */

// Temperature abnormal detection and record function - New GB standard: fixed 1h window
void battery_record_update_temperature(void) {
    uint8_t mode = g_buckboost.woke_mode;
    uint8_t event_type;
    int16_t ntc_temp;
    bool is_temp_abnormal;

    bool temp_using_virtual = (gd->eng_mode_active && gd->eng_virtual_temp != (int16_t)VIRTUAL_TEMP_SENTINEL);
    if (temp_using_virtual) {
        /* Engineering mode: use virtual temperature (0.1 degC units) */
        ntc_temp = gd->eng_virtual_temp;
        if (mode == BUCKBOOST_CHAGER_MODE) {
            is_temp_abnormal = (ntc_temp >= ENG_CHRG_OT_TEMP_DEGC);
        } else if (mode == BUCKBOOST_DISCHG_MODE) {
            is_temp_abnormal = (ntc_temp >= ENG_DISG_OT_TEMP_DEGC);
        } else {
            is_temp_abnormal = (ntc_temp >= ENG_CHRG_OT_TEMP_DEGC);
        }
        event_type = EXCEPTION_TYPE_OVERTEMP;
        br_printk("[BR] TEMP(eng): mode=%d virt=%d abnormal=%d\n",
                  mode, ntc_temp, is_temp_abnormal);
    } else {
        /* Normal mode */
        uint16_t ntc_resistance = g_buckboost.adc_tbat1;
        ntc_temp = ntc_to_temp(ntc_resistance) * 10;  // Convert to 0.1 degC
        is_temp_abnormal = is_temperature_abnormal(ntc_resistance, mode, &event_type);
        br_printk("[BR] TEMP(real): mode=%d ntc=%d temp=%d abnormal=%d\n",
                  mode, g_buckboost.adc_tbat1, ntc_temp, is_temp_abnormal);
    }

    if (!is_temp_abnormal) {
        return;
    }

    /* Write to unified exception_cache: charging → temp_chg, discharging → temp_dchg */
    if (mode == BUCKBOOST_CHAGER_MODE || (mode != BUCKBOOST_DISCHG_MODE)) {
        /* Charging (or idle/shutdown for engineering mode convenience) */
        if (!g_exception_cache.temp_chg_triggered || ntc_temp > g_exception_cache.temp_chg_max) {
            g_exception_cache.temp_chg_triggered = 1;
            g_exception_cache.temp_chg_max = ntc_temp;
            g_exception_cache.temp_chg_event_type = event_type;
            get_current_timestamp(&g_exception_cache.temp_chg_timestamp);
            br_printk("[BR] TEMP CHG: %d (window max updated)\n", ntc_temp);
        }
    } else {
        /* Discharging */
        if (!g_exception_cache.temp_dchg_triggered || ntc_temp > g_exception_cache.temp_dchg_max) {
            g_exception_cache.temp_dchg_triggered = 1;
            g_exception_cache.temp_dchg_max = ntc_temp;
            g_exception_cache.temp_dchg_event_type = event_type;
            get_current_timestamp(&g_exception_cache.temp_dchg_timestamp);
            br_printk("[BR] TEMP DCHG: %d (window max updated)\n", ntc_temp);
        }
    }

    /* Engineering mode single-shot: consume virtual value and flag for immediate flush */
    if (temp_using_virtual) {
        gd->eng_virtual_temp = VIRTUAL_TEMP_SENTINEL;
        g_eng_virtual_triggered = true;
    }
}

// Process 1-hour window end: write Flash records from g_exception_cache
static void process_window_end(void) {
    if (g_exception_cache.ov1_triggered) {
        BatteryExceptionRecord_t record;
        record.timestamp = g_exception_cache.ov1_timestamp;
        record.error_type = EXCEPTION_TYPE_OVERVOLTAGE;
        record.sub_type = 1;
        record.data.ov_data.max_voltage = g_exception_cache.ov1_max_voltage;
        record.data.ov_data.total_voltage = g_exception_cache.ov1_total_voltage;
        record.record_id = 0;
        write_exception_record(&record);
        printk("[BR] WINDOW OV Cell1: %dmV -> Flash\n", g_exception_cache.ov1_max_voltage);
    }

    if (g_exception_cache.ov2_triggered) {
        BatteryExceptionRecord_t record;
        record.timestamp = g_exception_cache.ov2_timestamp;
        record.error_type = EXCEPTION_TYPE_OVERVOLTAGE;
        record.sub_type = 2;
        record.data.ov_data.max_voltage = g_exception_cache.ov2_max_voltage;
        record.data.ov_data.total_voltage = g_exception_cache.ov2_total_voltage;
        record.record_id = 0;
        write_exception_record(&record);
        printk("[BR] WINDOW OV Cell2: %dmV -> Flash\n", g_exception_cache.ov2_max_voltage);
    }

    if (g_exception_cache.temp_chg_triggered) {
        BatteryExceptionRecord_t record;
        record.timestamp = g_exception_cache.temp_chg_timestamp;
        record.error_type = g_exception_cache.temp_chg_event_type;
        record.sub_type = BUCKBOOST_CHAGER_MODE;
        record.data.temp_data.max_temperature = g_exception_cache.temp_chg_max;
        record.data.temp_data.reserved = 0;
        record.record_id = 0;
        write_exception_record(&record);
        printk("[BR] WINDOW TEMP CHG: %d -> Flash\n", g_exception_cache.temp_chg_max);
    }

    if (g_exception_cache.temp_dchg_triggered) {
        BatteryExceptionRecord_t record;
        record.timestamp = g_exception_cache.temp_dchg_timestamp;
        record.error_type = g_exception_cache.temp_dchg_event_type;
        record.sub_type = BUCKBOOST_DISCHG_MODE;
        record.data.temp_data.max_temperature = g_exception_cache.temp_dchg_max;
        record.data.temp_data.reserved = 0;
        record.record_id = 0;
        write_exception_record(&record);
        printk("[BR] WINDOW TEMP DCHG: %d -> Flash\n", g_exception_cache.temp_dchg_max);
    }

    // Clear all trackers for next window
    g_exception_cache.ov1_triggered = 0; g_exception_cache.ov1_max_voltage = 0;
    g_exception_cache.ov2_triggered = 0; g_exception_cache.ov2_max_voltage = 0;
    g_exception_cache.temp_chg_triggered = 0; g_exception_cache.temp_chg_max = 0;
    g_exception_cache.temp_dchg_triggered = 0; g_exception_cache.temp_dchg_max = 0;
}

// Periodic check function - unified window using g_exception_cache
void battery_record_periodic_check(void) {
    static uint16_t check_counter = 0;

    if (++check_counter >= 10) {
        check_counter = 0;

        // Window init (cold boot, erase, or uninitialized RAM)
        if (g_exception_cache.window_start_seconds == 0 ||
            g_exception_cache.window_start_seconds == 0xFFFFFFFF) {
            memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
            VIC_vModuleDisable();
            g_exception_cache.window_start_seconds = gd->Bat_RTC_Seconds;
            VIC_vModuleEnable();
            br_printk("[BR] Window initialized\n");
        }

        br_printk("[BR] periodic: eng=%d mode=%d cell1=%d temp=%d\n",
                  gd->eng_mode_active, g_buckboost.woke_mode,
                  gd->eng_virtual_cell1, gd->eng_virtual_temp);

        battery_record_update_overvoltage();
        battery_record_update_temperature();

        // Engineering mode: immediate flush
        if (gd->eng_mode_active && g_eng_virtual_triggered) {
            g_eng_virtual_triggered = false;
            if (g_exception_cache.ov1_triggered || g_exception_cache.ov2_triggered ||
                g_exception_cache.temp_chg_triggered || g_exception_cache.temp_dchg_triggered) {
                process_window_end();
                br_printk("[BR] Eng mode: immediate flush\n");
            }
        }

        // Window expiry check
        uint32_t current_seconds;
        VIC_vModuleDisable();
        current_seconds = gd->Bat_RTC_Seconds;
        VIC_vModuleEnable();

        if (is_new_hour(g_exception_cache.window_start_seconds, current_seconds)) {
            process_window_end();
            g_exception_cache.window_start_seconds = current_seconds;
            br_printk("[BR] New window started\n");
        }

        TimeStamp_t ts;
        get_current_timestamp(&ts);
        uint32_t elapsed = current_seconds - g_exception_cache.window_start_seconds;
        printk("RTC: %04d-%02d-%02d %02d:%02d:%02d W:%u/%ds\n",
            ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second,
            elapsed, EXCEPTION_WINDOW_SECONDS);
    }
}

// Read single record from flash by page number and record index
static bool read_record_from_flash(uint8_t page_num, uint8_t record_index,
                                    BatteryExceptionRecord_t *record) {
    if (record == NULL || record_index >= MAX_RECORDS_PER_PAGE) {
        return false;
    }

    // Get page address
    uint32_t page_addr = GET_ACTIVE_PAGE_ADDR(page_num);

    // Calculate record address in flash
    uint32_t record_offset = offsetof(FlashPageLayout_t, records) +
                             (record_index * sizeof(BatteryExceptionRecord_t));
    uint32_t record_addr = page_addr + record_offset;

    // Read record from flash
    flash_read_record(record_addr, (uint8_t*)record, sizeof(BatteryExceptionRecord_t));

    return true;
}
/*
// Get total number of records across all configured pages
static uint8_t get_total_record_count(void) {
    // Only read page header (12 bytes) instead of entire page (496 bytes) to avoid stack overflow
    struct {
        uint32_t magic;
        uint8_t  page_records_count;
        uint8_t  page_number;
        uint8_t  overflow_ptr;
        uint8_t  reserved;
        uint32_t page_timestamp;
    } page_header;  // Only 12 bytes on stack

    uint8_t total = 0;
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    br_printk_debug("\r\n[DEBUG] get_total_record_count: MAGIC_VALUE=0x%08X", MAGIC_VALUE);

    // Read all configured pages (only headers)
    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        flash_read_record(page_addrs[i], (uint8_t*)&page_header, sizeof(page_header));

        br_printk_debug("\r\n[DEBUG] Page %d: addr=0x%04X, magic=0x%08X, count=%d",
                  i, page_addrs[i], page_header.magic, page_header.page_records_count);

        if (page_header.magic == MAGIC_VALUE) {
            uint8_t count = (page_header.page_records_count <= MAX_RECORDS_PER_PAGE) ?
                            page_header.page_records_count : MAX_RECORDS_PER_PAGE;
            total += count;
            br_printk_debug("\r\n[DEBUG] Page %d valid, adding %d records (total now: %d)",
                      i, count, total);
        } else {
            br_printk_debug("\r\n[DEBUG] Page %d invalid (magic mismatch)", i);
        }
    }

    br_printk_debug("\r\n[DEBUG] Final total: %d (max: %d)", total, MAX_TOTAL_RECORDS);
    return (total > MAX_TOTAL_RECORDS) ? MAX_TOTAL_RECORDS : total;
}
*/
/* Read single record by page/index */
uint8_t battery_record_read_by_page_index(uint8_t page, uint8_t index, BatteryExceptionRecord_t *record) {
    if (!read_record_from_flash(page, index, record)) return false;
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


/********************* Log Print Functions *********************/

// Print all logs from all pages (NEWEST to OLDEST)
// Optimized version: Only read headers (12 bytes �� 2 = 24 bytes) instead of full pages (496 bytes �� 2 = 992 bytes)
void battery_record_print_next_log(void) {
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    // Page header structure (only 12 bytes instead of 496 bytes)
    struct {
        uint32_t magic;
        uint8_t  page_records_count;
        uint8_t  page_number;
        uint8_t  overflow_ptr;
        uint8_t  reserved;
        uint32_t page_timestamp;
    } page_headers[LOG_PAGE_COUNT];  // Only 12 �� 2 = 24 bytes!

    uint8_t page_counts[LOG_PAGE_COUNT] = {0};
    uint8_t total_count = 0;

    // Read only headers from all pages (12 bytes each)
    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        flash_read_record(page_addrs[i], (uint8_t*)&page_headers[i], 12);
        if (page_headers[i].magic == MAGIC_VALUE) {
            page_counts[i] = (page_headers[i].page_records_count <= MAX_RECORDS_PER_PAGE) ?
                            page_headers[i].page_records_count : MAX_RECORDS_PER_PAGE;
            total_count += page_counts[i];
        }
    }

    br_printk("\r\n=== Battery Exception Records (Newest First) ===");
    if (total_count == 0) {
        br_printk("\r\n=== No Exception Records ===");
        return;
    }
    br_printk("\r\nTotal Records: %d across %d page(s)", total_count, LOG_PAGE_COUNT);

    uint16_t printed_count = 0;

    // Print from active page first (newest records)
    uint8_t active = g_record_storage.active_page;
    if (page_counts[active] > 0) {
        br_printk("\r\n--- Page %d (LOG%d at 0x%04X): %d records (NEWEST) ---",
                  active, active + 1, page_addrs[active], page_counts[active]);

        // Print in reverse order (newest to oldest)
        // Read records one by one from flash (only 20 bytes each)
        for (int16_t i = page_counts[active] - 1; i >= 0; i--) {
            BatteryExceptionRecord_t record;  // Only 20 bytes on stack

            // Read single record from flash
            if (!read_record_from_flash(active, (uint8_t)i, &record)) {
                continue;  // Skip if read failed
            }

            // Skip invalid records (error_type = 0 means uninitialized/corrupted)
            if (record.error_type == 0x00 || record.error_type == 0xFF) {
                continue;
            }

            // Print common timestamp format
            br_printk("\r\n  [%d] %04d-%02d-%02d %02d:%02d:%02d | ",
                record.record_id,
                record.timestamp.year,
                record.timestamp.month,
                record.timestamp.day,
                record.timestamp.hour,
                record.timestamp.minute,
                record.timestamp.second);

            // Print details based on type
            switch (record.error_type) {
                case EXCEPTION_TYPE_OVERVOLTAGE:
                    br_printk("OV Cell%d: %dmV (Total:%dmV)",
                        record.sub_type,
                        record.data.ov_data.max_voltage,
                        record.data.ov_data.total_voltage);
                    break;

                case EXCEPTION_TYPE_OVERTEMP:
                case EXCEPTION_TYPE_UNDERTEMP: {
                    int16_t temp_int = record.data.temp_data.max_temperature / 10;
                    int16_t temp_dec = record.data.temp_data.max_temperature % 10;
                    if (temp_dec < 0) temp_dec = -temp_dec;

                    const char *state = (record.sub_type == BUCKBOOST_CHAGER_MODE) ? "CHG" : "DCHG";
                    const char *type = (record.error_type == EXCEPTION_TYPE_OVERTEMP) ? "OT" : "UT";

                    br_printk("%s %s: %d.%ddegC", state, type, temp_int, temp_dec);
                    break;
                }

                default:
                    br_printk("Unknown type: 0x%02X", record.error_type);
                    break;
            }

            printed_count++;
        }
    }

    // Print other pages (older records)
    for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
        if (p == active) continue;  // Skip active page (already printed)
        if (page_counts[p] == 0) continue;  // Skip empty pages

        br_printk_debug("\r\n--- Page %d (LOG%d at 0x%04X): %d records (OLDER) ---",
                  p, p + 1, page_addrs[p], page_counts[p]);

        // Print in reverse order (newest to oldest within this page)
        // Read records one by one from flash (only 20 bytes each)
        for (int16_t i = page_counts[p] - 1; i >= 0; i--) {
            BatteryExceptionRecord_t record;  // Only 20 bytes on stack

            // Read single record from flash
            if (!read_record_from_flash(p, (uint8_t)i, &record)) {
                continue;  // Skip if read failed
            }

            // Skip invalid records
            if (record.error_type == 0x00 || record.error_type == 0xFF) {
                continue;
            }

            // Print record
            br_printk("\r\n  [%d] %04d-%02d-%02d %02d:%02d:%02d | ",
                record.record_id,
                record.timestamp.year,
                record.timestamp.month,
                record.timestamp.day,
                record.timestamp.hour,
                record.timestamp.minute,
                record.timestamp.second);

            switch (record.error_type) {
                case EXCEPTION_TYPE_OVERVOLTAGE:
                    br_printk("OV Cell%d: %dmV (Total:%dmV)",
                        record.sub_type,
                        record.data.ov_data.max_voltage,
                        record.data.ov_data.total_voltage);
                    break;

                case EXCEPTION_TYPE_OVERTEMP:
                case EXCEPTION_TYPE_UNDERTEMP: {
                    int16_t temp_int = record.data.temp_data.max_temperature / 10;
                    int16_t temp_dec = record.data.temp_data.max_temperature % 10;
                    if (temp_dec < 0) temp_dec = -temp_dec;

                    const char *state = (record.sub_type == BUCKBOOST_CHAGER_MODE) ? "CHG" : "DCHG";
                    const char *type = (record.error_type == EXCEPTION_TYPE_OVERTEMP) ? "OT" : "UT";

                    br_printk("%s %s: %d.%ddegC", state, type, temp_int, temp_dec);
                    break;
                }

                default:
                    br_printk("Unknown type: 0x%02X", record.error_type);
                    break;
            }

            printed_count++;
        }
    }

    // End of stored records
    if (printed_count > 0) {
        br_printk("\r\n=== End of Stored Records (%d total) ===", printed_count);
    }

        // Print current window tracking status (unified exception_cache)
        br_printk_debug("\r\n\r\n=== Current Window Tracking ===");

        if (g_exception_cache.ov1_triggered) {
            br_printk_debug("\r\n[WINDOW] OV Cell1: %dmV (max in current window)",
                g_exception_cache.ov1_max_voltage);
        }
        if (g_exception_cache.ov2_triggered) {
            br_printk_debug("\r\n[WINDOW] OV Cell2: %dmV (max in current window)",
                g_exception_cache.ov2_max_voltage);
        }
        if (g_exception_cache.temp_chg_triggered) {
            br_printk_debug("\r\n[WINDOW] TEMP CHG: %d.%d degC (max in current window)",
                g_exception_cache.temp_chg_max / 10,
                abs(g_exception_cache.temp_chg_max % 10));
        }
        if (g_exception_cache.temp_dchg_triggered) {
            br_printk_debug("\r\n[WINDOW] TEMP DCHG: %d.%d degC (max in current window)",
                g_exception_cache.temp_dchg_max / 10,
                abs(g_exception_cache.temp_dchg_max % 10));
        }

        // If no active window tracking
        if (!g_exception_cache.ov1_triggered && !g_exception_cache.ov2_triggered &&
            !g_exception_cache.temp_chg_triggered && !g_exception_cache.temp_dchg_triggered) {
            br_printk_debug("\r\n  No exceptions in current window");
        }

        br_printk("\r\n=== End of Report ===\r\n");

        // Reset print state for next call
        print_state.initialized = 0;
        print_state.current_page = 0;
        print_state.record_index = 0;
        print_state.records_printed = 0;
        print_state.total_records = 0;
}

/********************* Sleep Mode Functions *********************/

// Disable ADC after sleep wakeup reading
static void sleep_adc_deinit(void) {
    BADC->CTRL.WORD = 0;  // Disable BADC to save power

#if(BUCKBOOST_USED_NU6801 == 1)
    // enable all 6801 INT
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x80);
	// 6801 sleep function and firmware work-round start
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,0x00);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x09);//09
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H,0x1C);//0C
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,0x50);//0D
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x0D);//09

	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xff);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2D);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xF9);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x29);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xCB);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xE2);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x6A);

	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x63,0x01);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xff);
	hal_wdt_feed();

	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,0x01);//09
	//delay_1ms(500);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,0x41);//10
#endif
#if(CONFIG_USE_USB_XGB == 1)
	usb_bridge_sleep();
#endif
#if(BUCKBOOST_USED_NU6805 == 1)
	hal_wdt_feed();
	uint8_t read;

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_EN_1,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_EN_1,read & (~0x0C));

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x0F));

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x07));
	hal_wdt_feed();

	hal_i2cm_read_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,&read);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Mode_Control,read & (~0x11));

	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event1,0xFF);
	hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_IRQ_Event2,0xFF);
    hal_i2cm_wirte_one_byte(NU6805_I2C_DEV_ADDR,REG_Indt_Control,0x03);
#endif
}

// Exception check for sleep mode - unified window logic (shares g_exception_cache with non-sleep)
uint8_t battery_record_sleep_check(void) {
    uint8_t flash_written = 0;

    printk("[BR-SLEEP] page=%d wptr=%d cnt=%d win=%u ov1=%d\n",
           g_record_storage.active_page, g_record_storage.write_ptr,
           g_record_storage.exception_counter,
           g_exception_cache.window_start_seconds,
           g_exception_cache.ov1_triggered);

    // Initialize ADC
    hal_badc_init();
    _SET_I2CM_SDA_OUTPUT();
    _SET_I2CM_SCL_OUTPUT();
    buckboost_ops.init();

    // Read current values
#if(BUCKBOOST_USED_NU6801 == 1)
    uint16_t raw_voltage = hal_nu6801_buckboost_get_bat_voltage();
    uint16_t pc7_mv = hal_badc_meas(_BADC_CH_PC7_ADC4);
    uint16_t current_voltage = (int32_t)(raw_voltage-(pc7_mv*2-3300))>0 ? raw_voltage-(pc7_mv*2-3300) : 0;
    printk("current_voltage:%d\n",  current_voltage);
    uint16_t ntc_resistance = hal_nu6801_buckboost_get_bat_temperature();
#elif(BUCKBOOST_USED_NU6805 == 1)
    
    /* Sleep ADC: temporarily enable BADC mode + input buffer on PB6/PC7/PD3 */
    GPB->I_EN.BITS.PIN6 = 1;  GPB->MODE.BITS.PIN6 = 1;  /* PB6 → BADC7 (BAT2+) */
    GPC->I_EN.BITS.PIN7 = 1;  GPC->MODE.BITS.PIN7 = 1;  /* PC7 → BADC4 (Cell2) */
    GPD->I_EN.BITS.PIN3 = 1;  GPD->MODE.BITS.PIN3 = 1;  /* PD3 → BADC9 (VBAT-) */

    /* Sample Cell1/Cell2 via BADC — formula aligned with buckboost.c step3.
     * Vref 直接从 Flash 读取，不依赖 SRAM g_vref_mv（POR 后可能未恢复）。*/
    uint16_t vref_mv;
    uint32_t flash_vref = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + VREF_FLASH_OFFSET);
    if (flash_vref != 0xFFFFFFFF && flash_vref >= 2000 && flash_vref <= 4000) {
        vref_mv = (uint16_t)flash_vref;
    } else {
        vref_mv = VREF_DEFAULT_MV;
    }
    uint16_t pd3_adc_mv = hal_badc_meas(_BADC_CH_PD3_ADC9);
    int32_t pack_neg = 3 * (int32_t)pd3_adc_mv - 2 * (int32_t)vref_mv;

    uint16_t pc7_adc_mv = hal_badc_meas(_BADC_CH_PC7_ADC4);
    int32_t vcell1_raw = 3 * (int32_t)pc7_adc_mv - pack_neg;
    if (vcell1_raw < 0) vcell1_raw = 0;
    if (vcell1_raw > 5500) vcell1_raw = 5500;
    uint16_t sleep_vcell1 = (uint16_t)vcell1_raw;

    uint16_t pb6_adc_mv = hal_badc_meas(_BADC_CH_PB6_ADC7);
    int32_t vcell2_raw = 3 * (int32_t)pb6_adc_mv - pack_neg - vcell1_raw;
    if (vcell2_raw < 0) vcell2_raw = 0;
    if (vcell2_raw > 5500) vcell2_raw = 5500;
    uint16_t sleep_vcell2 = (uint16_t)vcell2_raw;
    br_printk("Sleep_Vref=%d,Vcell1=%d,Vcell2=%d,ADC=[%d,%d]\n",vref_mv,sleep_vcell1,sleep_vcell2,pc7_adc_mv,pb6_adc_mv);
    /* Restore GPIO mode for sleep (disable input buffer to save power) */
    GPB->MODE.BITS.PIN6 = 0;  GPB->I_EN.BITS.PIN6 = 0;  /* PB6 → GPIO */
    GPC->MODE.BITS.PIN7 = 0;  GPC->I_EN.BITS.PIN7 = 0;  /* PC7 → GPIO */
    GPD->MODE.BITS.PIN3 = 0;  GPD->I_EN.BITS.PIN3 = 0;  /* PD3 → GPIO */

    /* 触发 NTC 通道切换 → 阻塞 1ms 等 ADC 采样 → 读真实值 */
    (void)hal_nu6805_buckboost_get_bat_temperature();
    delay_1us(300);
    uint16_t ntc_resistance = hal_nu6805_buckboost_get_bat_temperature();  /* Ohm → Ohm/100 */
#endif
    int16_t ntc_temp = ntc_to_temp(ntc_resistance);
    br_printk("ntc:%d t:%d\n",  ntc_resistance, ntc_temp);

    // Window init (cold boot, erase, or uninitialized RAM)
    if (g_exception_cache.window_start_seconds == 0 ||
        g_exception_cache.window_start_seconds == 0xFFFFFFFF) {
        memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
        g_exception_cache.window_start_seconds = gd->Bat_RTC_Seconds;
    }

    // --- OV detection (same as non-sleep) ---
#if(BUCKBOOST_USED_NU6801 == 1)
    {
        uint16_t cell1_voltage = current_voltage;
        if (cell1_voltage >= BR_OVER_VOLTAGE_THRESHOLD) {
            if (!g_exception_cache.ov1_triggered || cell1_voltage > g_exception_cache.ov1_max_voltage) {
                g_exception_cache.ov1_triggered = 1;
                g_exception_cache.ov1_max_voltage = cell1_voltage;
                g_exception_cache.ov1_total_voltage = current_voltage;
                get_current_timestamp(&g_exception_cache.ov1_timestamp);
            }
        }
    }
#elif(BUCKBOOST_USED_NU6805 == 1)
    {
        uint16_t cell1_voltage = sleep_vcell1;
        uint16_t cell2_voltage = sleep_vcell2;

        /* Suspect reading filter (same as non-sleep path):
         * > 5500mV 连续 3 次才采信 (sleep 每 ~30s 调一次, 5 次 ≈ 150s) */
        static uint8_t sleep_suspect_c1 = 0;
        static uint8_t sleep_suspect_c2 = 0;
        if (cell1_voltage > 5500) {
            if (sleep_suspect_c1 < 3) { sleep_suspect_c1++; cell1_voltage = 0; }
        } else {
            sleep_suspect_c1 = 0;
        }
        if (cell2_voltage > 5500) {
            if (sleep_suspect_c2 < 3) { sleep_suspect_c2++; cell2_voltage = 0; }
        } else {
            sleep_suspect_c2 = 0;
        }

        uint16_t total_voltage = cell1_voltage + cell2_voltage;
        if (cell1_voltage >= BR_OVER_VOLTAGE_THRESHOLD) {
            if (!g_exception_cache.ov1_triggered || cell1_voltage > g_exception_cache.ov1_max_voltage) {
                g_exception_cache.ov1_triggered = 1;
                g_exception_cache.ov1_max_voltage = cell1_voltage;
                g_exception_cache.ov1_total_voltage = total_voltage;
                get_current_timestamp(&g_exception_cache.ov1_timestamp);
            }
        }
        if (cell2_voltage >= BR_OVER_VOLTAGE_THRESHOLD) {
            if (!g_exception_cache.ov2_triggered || cell2_voltage > g_exception_cache.ov2_max_voltage) {
                g_exception_cache.ov2_triggered = 1;
                g_exception_cache.ov2_max_voltage = cell2_voltage;
                g_exception_cache.ov2_total_voltage = total_voltage;
                get_current_timestamp(&g_exception_cache.ov2_timestamp);
                // printk("[SLP-OV2] max=%d ADC=[%d,%d,%d] vref=%d\n",
                    //    cell2_voltage, pd3_adc_mv, pc7_adc_mv, pb6_adc_mv, vref_mv);
            }
        }
    }
#endif

    // --- Temp detection (sleep mode: BSS zeroed woke_mode=0, use discharging as default) ---
    {
        uint8_t mode = g_buckboost.woke_mode;
        if (mode == BUCKBOOST_SHUTDOWM_MODE) mode = BUCKBOOST_DISCHG_MODE;
        uint8_t event_type;
        if (is_temperature_abnormal(ntc_resistance, mode, &event_type)) {
            int16_t current_temp_01c = ntc_temp * 10;
            if (mode == BUCKBOOST_CHAGER_MODE) {
                if (!g_exception_cache.temp_chg_triggered || current_temp_01c > g_exception_cache.temp_chg_max) {
                    g_exception_cache.temp_chg_triggered = 1;
                    g_exception_cache.temp_chg_max = current_temp_01c;
                    g_exception_cache.temp_chg_event_type = event_type;
                    get_current_timestamp(&g_exception_cache.temp_chg_timestamp);
                }
            } else if (mode == BUCKBOOST_DISCHG_MODE) {
                if (!g_exception_cache.temp_dchg_triggered || current_temp_01c > g_exception_cache.temp_dchg_max) {
                    g_exception_cache.temp_dchg_triggered = 1;
                    g_exception_cache.temp_dchg_max = current_temp_01c;
                    g_exception_cache.temp_dchg_event_type = event_type;
                    get_current_timestamp(&g_exception_cache.temp_dchg_timestamp);
                }
            }
        }
    }

    // --- Window expiry check ---
    uint32_t current_seconds = gd->Bat_RTC_Seconds;
    uint32_t elapsed = current_seconds - g_exception_cache.window_start_seconds;
    br_printk(" W:%u/%ds", elapsed, EXCEPTION_WINDOW_SECONDS);
    if (is_new_hour(g_exception_cache.window_start_seconds, current_seconds)) {
        if (g_exception_cache.ov1_triggered || g_exception_cache.ov2_triggered ||
            g_exception_cache.temp_chg_triggered || g_exception_cache.temp_dchg_triggered) {
            flash_written = 1;

            // Ensure g_record_storage and g_next_record_id are valid
            // (BSS zeroed on every LDROM boot, battery_record_init hasn't run yet)
            if (g_record_storage.magic != MAGIC_VALUE) {
                load_storage_from_flash();
            }
            if (g_next_record_id == 0) {
                uint32_t max_rid = 0;
                const uint32_t scan_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};
                for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
                    flash_read_record(scan_addrs[p], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
                    if (g_flash_page_buffer.magic == MAGIC_VALUE) {
                        for (uint8_t i = 0; i < g_flash_page_buffer.page_records_count && i < MAX_RECORDS_PER_PAGE; i++) {
                            if (g_flash_page_buffer.records[i].record_id > max_rid)
                                max_rid = g_flash_page_buffer.records[i].record_id;
                        }
                    }
                }
                g_next_record_id = (max_rid > 0) ? max_rid + 1 : 1;
                // printk("[BR-SLEEP] restored rid=%u\n", (unsigned)g_next_record_id);
            }
        }
        process_window_end();
        g_exception_cache.window_start_seconds = current_seconds;
    }

    // Disable ADC
    sleep_adc_deinit();
    usb_bridge_sleep();
    _SET_I2CM_SDA_IN_PUT();
    _SET_I2CM_SCL_IN_PUT();
    return flash_written;
}

/********************* USB Bridge Support Functions *********************/

/**
 * @brief Erase all exception records (engineering mode 0xEE command)
 * @return true if erase succeeded, false otherwise
 */
uint8_t battery_record_erase_all(void)
{
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    /* Erase all configured log pages and re-initialize with empty headers */
    for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
        hal_fmc_erase_page(page_addrs[i]);

        /* Write empty page header */
        memset(&g_flash_page_buffer, 0, sizeof(FlashPageLayout_t));
        g_flash_page_buffer.magic = MAGIC_VALUE;
        g_flash_page_buffer.page_records_count = 0;
        g_flash_page_buffer.page_number = i;
        g_flash_page_buffer.page_seq = 0;
        g_flash_page_buffer.checksum = calculate_page_checksum(&g_flash_page_buffer);

        flash_write_record(page_addrs[i], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));
    }

    /* Reset RAM storage metadata */
    g_record_storage.magic = MAGIC_VALUE;
    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr = 0;
    g_record_storage.active_page = 0;
    g_record_storage.page_sequence = 0;
    g_record_storage.checksum = calculate_checksum(&g_record_storage);

    /* Reset record_id counter */
    g_next_record_id = 1;

    /* Note: Do NOT reset eng_virtual_cell1/cell2/temp here.
     * They are engineering mode inputs, not record state.
     * Resetting them would discard the user's injected values
     * and prevent new records from being generated after erase. */

    /* Clear tracking state (unified window) */
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));

    br_printk("\r\n[ERASE] All %d log pages erased", LOG_PAGE_COUNT);
    return true;
}

/**
 * @brief Get count of overtemperature exception records
 * @return Number of records with error_type == EXCEPTION_TYPE_OVERTEMP
 */
uint16_t battery_record_get_overtemp_count(void)
{
    uint16_t count = 0;
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    for (uint8_t page = 0; page < LOG_PAGE_COUNT; page++) {
        flash_read_record(page_addrs[page], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

        if (g_flash_page_buffer.magic == MAGIC_VALUE) {
            uint8_t records_in_page = (g_flash_page_buffer.page_records_count <= MAX_RECORDS_PER_PAGE) ?
                                       g_flash_page_buffer.page_records_count : MAX_RECORDS_PER_PAGE;

            for (uint8_t i = 0; i < records_in_page; i++) {
                if (g_flash_page_buffer.records[i].error_type == EXCEPTION_TYPE_OVERTEMP) {
                    count++;
                }
            }
        }
    }

    return count;
}

/**
 * @brief Get count of overvoltage exception records
 * @return Number of records with error_type == EXCEPTION_TYPE_OVERVOLTAGE
 */
uint16_t battery_record_get_overvolt_count(void)
{
    uint16_t count = 0;
    const uint32_t page_addrs[3] = {FLASH_LOG_PAGE1, FLASH_LOG_PAGE2, FLASH_LOG_PAGE3};

    for (uint8_t page = 0; page < LOG_PAGE_COUNT; page++) {
        flash_read_record(page_addrs[page], (uint8_t*)&g_flash_page_buffer, sizeof(FlashPageLayout_t));

        if (g_flash_page_buffer.magic == MAGIC_VALUE) {
            uint8_t records_in_page = (g_flash_page_buffer.page_records_count <= MAX_RECORDS_PER_PAGE) ?
                                       g_flash_page_buffer.page_records_count : MAX_RECORDS_PER_PAGE;

            for (uint8_t i = 0; i < records_in_page; i++) {
                if (g_flash_page_buffer.records[i].error_type == EXCEPTION_TYPE_OVERVOLTAGE) {
                    count++;
                }
            }
        }
    }

    return count;
}
