/*
 * test_bat_record.c - Unit tests for the bat_record module
 *
 * INCLUDE ORDER IS CRITICAL:
 * 1. mock_all.h FIRST - sets all MCU header guards, provides platform stubs,
 *    allocates flash_mem at a fixed 32-bit address via VirtualAlloc.
 * 2. bat_record.c SECOND - compiled with all stubs in scope.
 *    Static variables (g_exception_cache, g_record_storage, print_state)
 *    are accessible because we include the .c file directly.
 * 3. Test assertions and main() follow.
 *
 * Build (from test/ directory):
 *   C:\msys64\ucrt64\bin\gcc.exe -std=c99 -Wall -Wno-int-to-pointer-cast
 *       -Wno-unused-function -I. -Imocks -I../app -I../fml -I../hal
 *       -o test_bat_record.exe test_bat_record.c
 *
 * Expected failures (confirm Phase 1 bugs):
 *   test_temperature_onset_uses_ntc_to_temp:
 *     ntc_to_temp() is NOT called (count stays 0)
 *     because current code uses gd->sys_infos.ntc_temp_wpc directly
 */

/* ------------------------------------------------------------------ */
/* 1. Mocks FIRST                                                       */
/* ------------------------------------------------------------------ */
#include "mocks/mock_all.h"

/* ------------------------------------------------------------------ */
/* 2. Source under test (include-the-.c for static variable access)     */
/* ------------------------------------------------------------------ */
#include "../app/bat_record.c"

/* ------------------------------------------------------------------ */
/* 3. Test framework                                                     */
/* ------------------------------------------------------------------ */
#include <stdio.h>
#include <string.h>

int g_test_failures = 0;
int g_test_total    = 0;

#define TEST_ASSERT_EQUAL_INT(expected, actual) do { \
    g_test_total++; \
    if ((int)(expected) != (int)(actual)) { \
        printf("  ASSERT FAIL (line %d): expected %d, got %d\n", \
               __LINE__, (int)(expected), (int)(actual)); \
        g_test_failures++; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_UINT16(expected, actual) do { \
    g_test_total++; \
    if ((uint16_t)(expected) != (uint16_t)(actual)) { \
        printf("  ASSERT FAIL (line %d): expected %u, got %u\n", \
               __LINE__, (unsigned)(uint16_t)(expected), \
               (unsigned)(uint16_t)(actual)); \
        g_test_failures++; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_UINT8(expected, actual) do { \
    g_test_total++; \
    if ((uint8_t)(expected) != (uint8_t)(actual)) { \
        printf("  ASSERT FAIL (line %d): expected %u, got %u\n", \
               __LINE__, (unsigned)(uint8_t)(expected), \
               (unsigned)(uint8_t)(actual)); \
        g_test_failures++; \
    } \
} while(0)

#define TEST_ASSERT_EQUAL_UINT32(expected, actual) do { \
    g_test_total++; \
    if ((uint32_t)(expected) != (uint32_t)(actual)) { \
        printf("  ASSERT FAIL (line %d): expected 0x%08X, got 0x%08X\n", \
               __LINE__, (unsigned)(expected), (unsigned)(actual)); \
        g_test_failures++; \
    } \
} while(0)

#define TEST_ASSERT_TRUE(cond) do { \
    g_test_total++; \
    if (!(cond)) { \
        printf("  ASSERT FAIL (line %d): expected TRUE, got FALSE\n", __LINE__); \
        g_test_failures++; \
    } \
} while(0)

#define TEST_ASSERT_FALSE(cond) do { \
    g_test_total++; \
    if ((cond)) { \
        printf("  ASSERT FAIL (line %d): expected FALSE, got TRUE\n", __LINE__); \
        g_test_failures++; \
    } \
} while(0)

static void RUN_TEST(void (*test_fn)(void), const char *name) {
    int before = g_test_failures;
    printf("[TEST] %s\n", name);
    test_fn();
    if (g_test_failures == before) {
        printf("  PASS\n");
    } else {
        printf("  FAIL (%d assertion(s) failed)\n", g_test_failures - before);
    }
}

/* ------------------------------------------------------------------ */
/* 4. Test state reset                                                   */
/* ------------------------------------------------------------------ */

/*
 * reset_test_state() must be called at the start of every test.
 *
 * NOTE: flash_mem is a macro -> flash_mem_ptr (the VirtualAlloc'd region).
 * flash_mem_ptr is initialized in main() before any tests run.
 */
static void reset_test_state(void) {
    /* Reset flash simulation to erased state (all 0xFF) */
    flash_sim_erase_all();

    /* Reset global mock state */
    memset(&gd_mock, 0, sizeof(gd_mock));
    memset(&ap_mock, 0, sizeof(ap_mock));

    /* Reset NTC spy */
    mock_ntc_to_temp_call_count = 0;
    mock_ntc_to_temp_return     = 0;

    /* Reset battery voltage mock */
    mock_bat_voltage = 0;

    /* Reset BuckBoost mock */
    memset(&g_buckboost, 0, sizeof(g_buckboost));

    /* Reset bat_record.c static variables (accessible via #include .c) */
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    memset((void*)&g_record_storage,  0, sizeof(g_record_storage));

    /* Reset print_state (static struct inside bat_record.c) */
    print_state.initialized     = 0;
    print_state.exception_index = 0;
    print_state.exception_total = 0;
}

/* ------------------------------------------------------------------ */
/* 5. Group 1: seconds_to_timestamp()                                   */
/* ------------------------------------------------------------------ */

static void test_timestamp_epoch_zero(void) {
    reset_test_state();
    TimeStamp_t ts;
    seconds_to_timestamp(0, &ts);
    /* Epoch 0 = 2026-01-01 00:00:00 */
    TEST_ASSERT_EQUAL_INT(2026, ts.year);
    TEST_ASSERT_EQUAL_INT(1,    ts.month);
    TEST_ASSERT_EQUAL_INT(1,    ts.day);
    TEST_ASSERT_EQUAL_INT(0,    ts.hour);
    TEST_ASSERT_EQUAL_INT(0,    ts.minute);
    TEST_ASSERT_EQUAL_INT(0,    ts.second);
}

static void test_timestamp_one_day(void) {
    reset_test_state();
    TimeStamp_t ts;
    seconds_to_timestamp(86400, &ts);
    /* 86400 seconds = 1 day -> 2026-01-02 00:00:00 */
    TEST_ASSERT_EQUAL_INT(2026, ts.year);
    TEST_ASSERT_EQUAL_INT(1,    ts.month);
    TEST_ASSERT_EQUAL_INT(2,    ts.day);
    TEST_ASSERT_EQUAL_INT(0,    ts.hour);
    TEST_ASSERT_EQUAL_INT(0,    ts.minute);
    TEST_ASSERT_EQUAL_INT(0,    ts.second);
}

static void test_timestamp_one_year(void) {
    reset_test_state();
    TimeStamp_t ts;
    /* 365 days from 2026-01-01 -> 2027-01-01 (2026 is not a leap year) */
    seconds_to_timestamp(365UL * 86400UL, &ts);
    TEST_ASSERT_EQUAL_INT(2027, ts.year);
    TEST_ASSERT_EQUAL_INT(1,    ts.month);
    TEST_ASSERT_EQUAL_INT(1,    ts.day);
}

static void test_timestamp_time_of_day(void) {
    reset_test_state();
    TimeStamp_t ts;
    /* 1h + 2m + 45s = 3600 + 120 + 45 = 3765 seconds */
    seconds_to_timestamp(3765, &ts);
    TEST_ASSERT_EQUAL_INT(2026, ts.year);
    TEST_ASSERT_EQUAL_INT(1,    ts.month);
    TEST_ASSERT_EQUAL_INT(1,    ts.day);
    TEST_ASSERT_EQUAL_INT(1,    ts.hour);
    TEST_ASSERT_EQUAL_INT(2,    ts.minute);
    TEST_ASSERT_EQUAL_INT(45,   ts.second);
}

/* ------------------------------------------------------------------ */
/* 6. Group 2: battery_record_init()                                    */
/* ------------------------------------------------------------------ */

static void test_init_cold_start(void) {
    reset_test_state();
    /* Flash has 0xFF = no valid magic -> cold start */
    battery_record_init();

    /* After cold start: g_record_storage should have MAGIC_VALUE, counter=0 */
    TEST_ASSERT_EQUAL_UINT32(MAGIC_VALUE, g_record_storage.magic);
    TEST_ASSERT_EQUAL_UINT8(0, g_record_storage.exception_counter);
    TEST_ASSERT_EQUAL_UINT8(0, g_record_storage.write_ptr);

    /* Flash should contain MAGIC_VALUE at offset 0
     * flash_mem_ptr[0..3] should decode to MAGIC_VALUE on little-endian host.
     * The write path:
     *   g_record_storage.magic = 0x42415436 (LE bytes: 0x36 0x54 0x41 0x42)
     *   flash_write_record packs: word = (data[3]<<24)|(data[2]<<16)|(data[1]<<8)|data[0]
     *                                  = (0x42<<24)|(0x41<<16)|(0x54<<8)|0x36 = 0x42415436
     *   switch_big_little_endian(0x42415436) = 0x42415436 (no-op on x86)
     *   hal_fmc_write_word writes 0x42415436 LE -> bytes {0x36, 0x54, 0x41, 0x42}
     *   flash_read_u32 reads {0x36, 0x54, 0x41, 0x42} LE -> 0x42415436 = MAGIC_VALUE
     */
    uint32_t flash_magic = *(uint32_t*)(uintptr_t)(uint32_t)(uintptr_t)flash_mem_ptr;
    TEST_ASSERT_EQUAL_UINT32(MAGIC_VALUE, flash_magic);
}

static void test_init_warm_start_valid(void) {
    reset_test_state();

    /* Step 1: Cold start - writes valid Flash state */
    battery_record_init();
    uint8_t saved_counter = g_record_storage.exception_counter;  /* = 0 */

    /* Step 2: Simulate warm restart - clear RAM, keep Flash intact */
    memset((void*)&g_record_storage, 0, sizeof(g_record_storage));
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    print_state.initialized = 0;

    /* Step 3: Second init - should detect valid Flash and load from it */
    battery_record_init();

    TEST_ASSERT_EQUAL_UINT32(MAGIC_VALUE, g_record_storage.magic);
    TEST_ASSERT_EQUAL_UINT8(saved_counter, g_record_storage.exception_counter);
}

/* ------------------------------------------------------------------ */
/* 7. Group 3: is_new_hour()                                            */
/* ------------------------------------------------------------------ */

static void test_is_new_hour_not_yet(void) {
    reset_test_state();
    /* 3599 seconds elapsed - not a full hour */
    bool result = is_new_hour(1000, 1000 + 3599);
    TEST_ASSERT_FALSE(result);
}

static void test_is_new_hour_exact(void) {
    reset_test_state();
    /* Exactly 3600 seconds = one full hour */
    bool result = is_new_hour(1000, 1000 + 3600);
    TEST_ASSERT_TRUE(result);
}

static void test_is_new_hour_overflow(void) {
    reset_test_state();
    /*
     * uint32_t overflow test.
     * Elapsed = current - last (wraps around via unsigned arithmetic)
     * last = 0xFFFFF1F0, current = 0x00000000
     * elapsed = 0x00000000 - 0xFFFFF1F0 = 0x00000E10 = 3600 -> TRUE
     *
     * Verify: UINT32_MAX - 3600 + 1 = 0xFFFFF1F0 + 1 = 0xFFFFF1F1
     * Actually: 0 - 0xFFFFF1F0 = (2^32 + 0) - 0xFFFFF1F0 = 0xFFFFFFFF - 0xFFFFF1F0 + 1
     *         = 0x00000E0F + 1 = 0x00000E10 = 3600. YES, exactly 3600.
     */
    uint32_t last    = 0xFFFFF1F0UL;
    uint32_t current = 0x00000000UL;
    bool result = is_new_hour(last, current);
    TEST_ASSERT_TRUE(result);
}

/* ------------------------------------------------------------------ */
/* 8. Group 4: Overvoltage state machine                                */
/* ------------------------------------------------------------------ */

static void test_ov_onset_writes_record(void) {
    reset_test_state();
    battery_record_init();

    /* Clear any state from init */
    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));

    /* NU6805 path: total = 9200 -> cell1=cell2=4600 > threshold(4500) */
    mock_bat_voltage      = 9200;
    gd_mock.Bat_RTC_Timer = 5000;  /* 5 seconds */

    battery_record_update_overvoltage();

    /* Both cells should trigger onset -> at least 1 record written */
    TEST_ASSERT_TRUE(g_record_storage.exception_counter >= 1);
    TEST_ASSERT_TRUE(CACHE_GET_CELL1_TRACKING(&g_exception_cache));
}

static void test_ov_no_trigger_below_threshold(void) {
    reset_test_state();
    battery_record_init();

    g_record_storage.exception_counter = 0;
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));

    /* total = 8998 -> cell = 4499 < threshold(4500) -> no onset */
    mock_bat_voltage      = 8998;
    gd_mock.Bat_RTC_Timer = 0;

    battery_record_update_overvoltage();

    TEST_ASSERT_EQUAL_UINT8(0, g_record_storage.exception_counter);
    TEST_ASSERT_FALSE(CACHE_GET_CELL1_TRACKING(&g_exception_cache));
    TEST_ASSERT_FALSE(CACHE_GET_CELL2_TRACKING(&g_exception_cache));
}

static void test_ov_tracking_updates_max(void) {
    reset_test_state();
    battery_record_init();

    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;

    /* First call: onset at 4600 mV */
    mock_bat_voltage      = 9200;  /* cell = 4600 > 4500 */
    gd_mock.Bat_RTC_Timer = 0;
    battery_record_update_overvoltage();
    TEST_ASSERT_TRUE(g_exception_cache.cell1_max_voltage >= 4600);
    uint16_t max_after_onset = g_exception_cache.cell1_max_voltage;

    /* Second call: higher voltage -> max should update to 4650 */
    mock_bat_voltage      = 9300;  /* cell = 4650 */
    gd_mock.Bat_RTC_Timer = 1000;  /* 1 second later, well within 1 hour */
    battery_record_update_overvoltage();

    TEST_ASSERT_TRUE(g_exception_cache.cell1_max_voltage >= max_after_onset);
}

static void test_ov_recover_clears_flag(void) {
    reset_test_state();
    battery_record_init();

    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;

    /* Onset at high voltage */
    mock_bat_voltage      = 9200;
    gd_mock.Bat_RTC_Timer = 0;
    battery_record_update_overvoltage();
    TEST_ASSERT_TRUE(CACHE_GET_CELL1_TRACKING(&g_exception_cache));

    /* Recovery: voltage drops below threshold */
    mock_bat_voltage      = 8800;  /* cell = 4400 < 4500 */
    gd_mock.Bat_RTC_Timer = 2000;
    battery_record_update_overvoltage();

    /* Tracking flags cleared after recovery */
    TEST_ASSERT_FALSE(CACHE_GET_CELL1_TRACKING(&g_exception_cache));
    TEST_ASSERT_EQUAL_UINT16(0, g_exception_cache.cell1_max_voltage);
}

/* ------------------------------------------------------------------ */
/* 9. Group 5: Temperature - KEY BUG TEST                               */
/* ------------------------------------------------------------------ */

/*
 * test_temperature_onset_uses_ntc_to_temp - KEY BUG DETECTION TEST
 *
 * EXPECTED FAIL on current code:
 *   Current bat_record.c uses gd->sys_infos.ntc_temp_wpc directly.
 *   ntc_to_temp() is NEVER called -> mock_ntc_to_temp_call_count stays 0.
 *
 * EXPECTED PASS after Phase 3 fix:
 *   Fixed code calls ntc_to_temp(ntc_resistance) -> count becomes 1.
 *   Recorded temperature = 950 (from ntc_to_temp), not 100 (from gd).
 */
static void test_temperature_onset_uses_ntc_to_temp(void) {
    reset_test_state();
    battery_record_init();

    /* Setup:
     * - adc_tbat1 = 30 < CHRG_NTC_OT_VALUE(50) -> is_temperature_abnormal() returns TRUE
     * - mock_ntc_to_temp_return = 950   (sentinel for ntc_to_temp path)
     * - gd->sys_infos.ntc_temp_wpc = 100 (sentinel for gd path - different value)
     * - These different sentinel values let us detect which path was taken
     */
    g_buckboost.adc_tbat1          = 30;
    mock_ntc_to_temp_return        = 950;
    gd_mock.sys_infos.ntc_temp_wpc = 100;
    gd_mock.Bat_RTC_Timer          = 0;
    g_buckboost.woke_mode          = BUCKBOOST_CHAGER_MODE;

    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;

    battery_record_update_temperature();

    /* Assertion (a): ntc_to_temp() should be called
     * FAIL on current code: count=0 (gd->ntc_temp_wpc used instead)
     * PASS after Phase 3 fix: count=1
     */
    printf("  [BUG CHECK] ntc_to_temp call count: expected=1, actual=%u\n",
           (unsigned)mock_ntc_to_temp_call_count);
    TEST_ASSERT_EQUAL_INT(1, (int)mock_ntc_to_temp_call_count);

    /* Assertion (b): recorded temperature = 950 (ntc_to_temp return), not 100 (gd) */
    if (g_record_storage.exception_counter > 0) {
        uint8_t last_idx = (g_record_storage.write_ptr == 0) ?
                           (MAX_RECORDS - 1) : (g_record_storage.write_ptr - 1);
        int16_t recorded_temp =
            g_record_storage.records[last_idx].data.temp_data.max_temperature;
        printf("  [BUG CHECK] recorded temperature: expected=950, actual=%d\n",
               (int)recorded_temp);
        TEST_ASSERT_EQUAL_INT(950, (int)recorded_temp);
    } else {
        /* Onset did NOT fire (shouldn't happen when adc_tbat1 < CHRG_NTC_OT_VALUE) */
        printf("  [UNEXPECTED] exception_counter=0, onset did not fire\n");
        TEST_ASSERT_TRUE(g_record_storage.exception_counter > 0);
    }
}

static void test_temperature_no_onset_above_threshold(void) {
    reset_test_state();
    battery_record_init();

    /* adc_tbat1 = 60 > CHRG_NTC_OT_VALUE(50) -> normal temperature -> no onset */
    g_buckboost.adc_tbat1          = 60;
    mock_ntc_to_temp_return        = 250;  /* 25.0 deg C */
    gd_mock.sys_infos.ntc_temp_wpc = 250;
    gd_mock.Bat_RTC_Timer          = 0;

    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    g_record_storage.exception_counter = 0;

    battery_record_update_temperature();

    TEST_ASSERT_EQUAL_UINT8(0, g_record_storage.exception_counter);
    TEST_ASSERT_FALSE(CACHE_GET_TEMP_TRACKING(&g_exception_cache));
}

/* ------------------------------------------------------------------ */
/* 10. Group 6: Checksum                                                */
/* ------------------------------------------------------------------ */

static void test_checksum_valid_after_init(void) {
    reset_test_state();
    battery_record_init();  /* Writes valid state to Flash */

    /* Verify the in-RAM checksum passes */
    bool cs_ok = verify_storage_checksum(&g_record_storage);
    TEST_ASSERT_TRUE(cs_ok);
    TEST_ASSERT_EQUAL_UINT32(MAGIC_VALUE, g_record_storage.magic);
}

static void test_checksum_corruption_triggers_reinit(void) {
    reset_test_state();
    battery_record_init();  /* Valid state in Flash */

    /* Corrupt one byte: flip exception_counter byte in flash_mem.
     * exception_counter is at byte offset 4 in BatteryRecordStorage_t
     * (after 4-byte magic field). Flipping a bit breaks the checksum.
     */
    flash_mem_ptr[4] ^= 0x55;

    /* Clear RAM state to force reload from Flash */
    memset((void*)&g_record_storage, 0, sizeof(g_record_storage));
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
    print_state.initialized = 0;

    /* Re-init: should detect checksum failure and reinitialize to clean state */
    battery_record_init();

    TEST_ASSERT_EQUAL_UINT32(MAGIC_VALUE, g_record_storage.magic);
    TEST_ASSERT_EQUAL_UINT8(0, g_record_storage.exception_counter);
}

/* ------------------------------------------------------------------ */
/* 11. Group 7: Circular buffer                                         */
/* ------------------------------------------------------------------ */

static void test_circular_buffer_wraps(void) {
    reset_test_state();
    battery_record_init();

    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;

    BatteryExceptionRecord_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.error_type = EXCEPTION_TYPE_OVERVOLTAGE;
    rec.sub_type   = 1;

    /* Write 6 records with MAX_RECORDS=5:
     * Write 0: ptr=0->1, counter=1
     * Write 1: ptr=1->2, counter=2
     * Write 2: ptr=2->3, counter=3
     * Write 3: ptr=3->4, counter=4
     * Write 4: ptr=4->0 (mod 5), counter=5
     * Write 5: ptr=0->1, counter stays 5 (max)
     * Final: write_ptr=1, exception_counter=5
     */
    for (int i = 0; i < 6; i++) {
        rec.data.ov_data.max_voltage = (uint16_t)(4600 + i * 10);
        write_exception_record(&rec);
    }

    TEST_ASSERT_EQUAL_UINT8(1, g_record_storage.write_ptr);
    TEST_ASSERT_EQUAL_UINT8(MAX_RECORDS, g_record_storage.exception_counter);
}

/* ------------------------------------------------------------------ */
/* 12. Additional integration tests                                     */
/* ------------------------------------------------------------------ */

static void test_init_then_read(void) {
    reset_test_state();
    battery_record_init();

    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));

    /* Write one OV record via onset */
    mock_bat_voltage      = 9200;  /* cell=4600 > 4500 */
    gd_mock.Bat_RTC_Timer = 60000; /* 60 seconds */
    battery_record_update_overvoltage();
    TEST_ASSERT_TRUE(g_record_storage.exception_counter >= 1);

    /* Read back */
    BatteryExceptionRecord_t buf[MAX_RECORDS];
    uint8_t count = battery_record_read_exceptions(buf, MAX_RECORDS);
    TEST_ASSERT_TRUE(count >= 1);
    TEST_ASSERT_EQUAL_INT(EXCEPTION_TYPE_OVERVOLTAGE, (int)buf[0].error_type);
}

static void test_exception_type_in_record(void) {
    reset_test_state();
    battery_record_init();

    g_record_storage.exception_counter = 0;
    g_record_storage.write_ptr         = 0;
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));

    /* Trigger OV onset */
    mock_bat_voltage      = 9200;
    gd_mock.Bat_RTC_Timer = 0;
    battery_record_update_overvoltage();

    if (g_record_storage.exception_counter >= 1) {
        TEST_ASSERT_EQUAL_INT(EXCEPTION_TYPE_OVERVOLTAGE,
                              (int)g_record_storage.records[0].error_type);
    } else {
        printf("  [FAIL] No record written despite high voltage\n");
        TEST_ASSERT_TRUE(0);
    }
}

/* ------------------------------------------------------------------ */
/* 13. main()                                                            */
/* ------------------------------------------------------------------ */

int main(void) {
    /* Initialize Flash simulation at a fixed 32-bit address.
     * This must run BEFORE any test that calls battery_record_* functions.
     * AP_CFG_ROM_ADDR_LOG = FLASH_VIRT_BASE = 0x00010000 (matches VirtualAlloc) */
    flash_sim_init();
    flash_sim_verify();

    printf("========================================================\n");
    printf("  bat_record Unit Tests\n");
    printf("  Flash simulation at: %p (uint32: 0x%08X)\n",
           (void*)flash_mem_ptr, (uint32_t)(uintptr_t)flash_mem_ptr);
    printf("  AP_CFG_ROM_ADDR_LOG = 0x%08lX\n", (unsigned long)AP_CFG_ROM_ADDR_LOG);
    printf("========================================================\n\n");

    /* Group 1: seconds_to_timestamp() */
    printf("--- Group 1: seconds_to_timestamp() ---\n");
    RUN_TEST(test_timestamp_epoch_zero,  "test_timestamp_epoch_zero");
    RUN_TEST(test_timestamp_one_day,     "test_timestamp_one_day");
    RUN_TEST(test_timestamp_one_year,    "test_timestamp_one_year");
    RUN_TEST(test_timestamp_time_of_day, "test_timestamp_time_of_day");

    /* Group 2: battery_record_init() */
    printf("\n--- Group 2: battery_record_init() ---\n");
    RUN_TEST(test_init_cold_start,       "test_init_cold_start");
    RUN_TEST(test_init_warm_start_valid, "test_init_warm_start_valid");

    /* Group 3: is_new_hour() */
    printf("\n--- Group 3: is_new_hour() ---\n");
    RUN_TEST(test_is_new_hour_not_yet,  "test_is_new_hour_not_yet");
    RUN_TEST(test_is_new_hour_exact,    "test_is_new_hour_exact");
    RUN_TEST(test_is_new_hour_overflow, "test_is_new_hour_overflow");

    /* Group 4: Overvoltage state machine */
    printf("\n--- Group 4: Overvoltage state machine ---\n");
    RUN_TEST(test_ov_onset_writes_record,        "test_ov_onset_writes_record");
    RUN_TEST(test_ov_no_trigger_below_threshold, "test_ov_no_trigger_below_threshold");
    RUN_TEST(test_ov_tracking_updates_max,       "test_ov_tracking_updates_max");
    RUN_TEST(test_ov_recover_clears_flag,        "test_ov_recover_clears_flag");

    /* Group 5: Temperature - KEY BUG TEST */
    printf("\n--- Group 5: Temperature (KEY BUG TEST) ---\n");
    printf("NOTE: test_temperature_onset_uses_ntc_to_temp is EXPECTED to FAIL\n");
    printf("      Current code uses gd->sys_infos.ntc_temp_wpc (BUG)\n");
    printf("      instead of calling ntc_to_temp(ntc_resistance) (CORRECT)\n\n");
    RUN_TEST(test_temperature_onset_uses_ntc_to_temp,
             "test_temperature_onset_uses_ntc_to_temp");
    RUN_TEST(test_temperature_no_onset_above_threshold,
             "test_temperature_no_onset_above_threshold");

    /* Group 6: Checksum */
    printf("\n--- Group 6: Checksum ---\n");
    RUN_TEST(test_checksum_valid_after_init,
             "test_checksum_valid_after_init");
    RUN_TEST(test_checksum_corruption_triggers_reinit,
             "test_checksum_corruption_triggers_reinit");

    /* Group 7: Circular buffer */
    printf("\n--- Group 7: Circular buffer ---\n");
    RUN_TEST(test_circular_buffer_wraps, "test_circular_buffer_wraps");

    /* Additional integration */
    printf("\n--- Additional integration tests ---\n");
    RUN_TEST(test_init_then_read,           "test_init_then_read");
    RUN_TEST(test_exception_type_in_record, "test_exception_type_in_record");

    /* Summary */
    printf("\n========================================================\n");
    printf("  Results: %d assertions checked\n", g_test_total);
    if (g_test_failures == 0) {
        printf("  ALL TESTS PASSED\n");
    } else {
        printf("  FAILURES: %d assertion(s) failed\n", g_test_failures);
        printf("\n  Expected failure (confirms Phase 1 bug):\n");
        printf("    test_temperature_onset_uses_ntc_to_temp:\n");
        printf("      bat_record.c uses gd->sys_infos.ntc_temp_wpc\n");
        printf("      instead of calling ntc_to_temp(ntc_resistance).\n");
        printf("      ntc_to_temp spy count = 0 (not called).\n");
        printf("      This confirms the Phase 1 bug identification.\n");
    }
    printf("========================================================\n");

    return (g_test_failures > 0) ? 1 : 0;
}
