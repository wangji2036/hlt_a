/**
 * @file usb_bridge.c
 * @brief USB Bridge (WB7720) — unified telemetry, exception log, engineering mode,
 *        production mode. Architecture aligned with Nanfu A1052.
 *
 * Single entry: usb_bridge_periodic_update() called from FML_TASK round-robin.
 * Internal cnt 0-13 dispatch, 14 steps per cycle (~47ms each).
 */

#include "usb_bridge.h"

#if CONFIG_USB_BRIDGE_ENABLE

#include "g_data.h"
#include "i2cm.h"
#include "printk.h"
#include "buckboost.h"
#include "bat_record.h"
#include "nu6805.h"
#include "../hal/badc.h"
#include <string.h>

/* batTemp is pre-computed in app.c 100ms poll (NTC lookup) */

/*===================== External References =====================*/
extern struct buckboost_s g_buckboost;
extern uint16_t cell2_voltage;  /* MCU ADC sampled Cell2 (app.c 250ms update) */

/*===================== Static Variables =====================*/

static uint8_t cnt = 0;
static bool is_usb_enable = false;

/* Cached VBAT from cnt 2 MCU ADC, reused by cnt 10 Cell Info */
static uint16_t vbat_cached = 0;

/* Exception log sequential push cursor */
static uint8_t exc_cursor_page = 0;
static uint8_t exc_cursor_idx = 0;
static uint8_t exc_records_sent = 0;
static uint8_t exc_page_counts[LOG_PAGE_COUNT];

/* Engineering mode cycle tracking (16-bit to match GET_CYCLE_COUNT) */
static uint16_t eng_saved_cycle_count = 0;
static uint16_t eng_entry_virtual_cycle = 0;

/*===================== ProductInfo Sync =====================*/

void usb_bridge_reset_product_info(void)
{
    ProductInfo_t info;
    product_info_read(&info);

    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_MANUFACTURER,
                               (uint8_t*)info.manufacturer_name, 20);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_MODEL,
                               (uint8_t*)info.model_name, 20);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_BATTERY_MFR,
                               (uint8_t*)info.battery_mfr, 20);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_BATTERY_MODEL,
                               (uint8_t*)info.battery_model, 20);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_PROD_DATE,
                               (uint8_t*)info.battery_prod_date, 20);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, PROD_SERIAL,
                               (uint8_t*)info.serial, SERIAL_FIELD_SIZE);
}

/*===================== Init =====================*/

void usb_bridge_init(void)
{
    cnt = 0;
    is_usb_enable = false;
    exc_cursor_page = 0;
    exc_cursor_idx = 0;
    exc_records_sent = 0;

    usb_bridge_reset_product_info();
    usb_bridge_sleep();
}

/*===================== Sleep / Wake =====================*/

void usb_bridge_sleep(void)
{
    if (gd->eng_mode_active) {
        gd->eng_mode_active = 0;
        gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_temp  = (int16_t)VIRTUAL_TEMP_SENTINEL;

        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                              ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);

        uint16_t cycle_wb = GET_CYCLE_COUNT(gd);
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CYCLE_COUNT, (uint8_t*)&cycle_wb, 2);
    }

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_SLEEP_CMD, 0x01);
    printk("usb bridge sleep\n");
}

void usb_bridge_wakeup(void)
{
    for (int retry = 0; retry < 5; retry++) {
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_WAKEUP_CMD, 0x01);
        uint8_t readback = 0;
        hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_WAKEUP_CMD, &readback);
        printk("[WB] wake r=%d rb=%02X\n", retry, readback);
    }

    usb_bridge_reset_product_info();

    uint16_t cycle_buf = GET_CYCLE_COUNT(gd);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle_buf, 2);
}

/*===================== Engineering Mode Helpers =====================*/

static void usb_bridge_read_virtual_params(void)
{
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_VIRTUAL_CELL1,
                              (uint8_t*)&gd->eng_virtual_cell1, 2);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_VIRTUAL_CELL2,
                              (uint8_t*)&gd->eng_virtual_cell2, 2);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_VIRTUAL_TEMP,
                              (uint8_t*)&gd->eng_virtual_temp, 2);
}

static uint32_t datetime_to_seconds_inline(uint8_t *dt)
{
    static const uint8_t days_in_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    uint16_t year = dt[0] | ((uint16_t)dt[1] << 8);
    if (year == 0) return 0;  /* sentinel */

    uint16_t y = year - 2026U;
    uint32_t total_days = (uint32_t)y * 365UL + ((uint32_t)(y + 1) / 4);
    for (uint8_t m = 1; m < dt[2]; m++)
        total_days += days_in_month[m];
    if (dt[2] > 2 && (year % 4 == 0)) total_days++;
    total_days += (uint32_t)(dt[3] - 1);

    return total_days * 86400UL + (uint32_t)dt[4] * 3600UL
           + (uint32_t)dt[5] * 60UL + (uint32_t)dt[6];
}

static void usb_bridge_apply_eng_datetime(void)
{
    uint8_t dt[7];
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CURRENT_DATE, dt, 7);
    uint32_t secs = datetime_to_seconds_inline(dt);
    if (secs > 0) {
        VIC_vModuleDisable();
        gd->Bat_RTC_Seconds = secs;
        gd->Bat_RTC_Milliseconds = 0;
        VIC_vModuleEnable();
    }
}

static void usb_bridge_exit_eng_mode(void)
{
    if (eng_entry_virtual_cycle != eng_saved_cycle_count) {
        /* Virtual cycle was injected -- keep current value */
        printk("[eng] exit: cycle=%d (injected, kept)\n", GET_CYCLE_COUNT(gd));
    } else {
        /* No injection (sentinel) -- restore real + increments */
        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                               ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);
        printk("[eng] exit: cycle=%d (restored)\n", GET_CYCLE_COUNT(gd));
    }

    /* Update CV voltage based on final cycle count */
#if (BUCKBOOST_USED_NU6805 == 1 && CONFIG_CYCLE_CV_REDUCTION_ENABLE == 1)
    hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

    gd->eng_mode_active = 0;
    gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
    gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
    gd->eng_virtual_temp  = (int16_t)VIRTUAL_TEMP_SENTINEL;

    uint16_t cycle_wb = GET_CYCLE_COUNT(gd);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CYCLE_COUNT, (uint8_t*)&cycle_wb, 2);

#if CYCLE_COUNT_FLASH_PERSIST
    cycle_count_save_to_flash();
#endif
}

/*===================== Engineering Mode Check =====================*/

static void usb_bridge_check_eng_mode(void)
{
    uint8_t work_mode = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_WORK_MODE, &work_mode);

    if (work_mode == ENG_MODE_MAGIC && !gd->eng_mode_active) {
        /* ENTER engineering mode */
        gd->eng_mode_active = 1;

        usb_bridge_apply_eng_datetime();

        uint16_t cycle = 0;
        hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle, 2);
        if (cycle != 0xFFFF) {
            eng_saved_cycle_count = GET_CYCLE_COUNT(gd);
            SET_CYCLE_COUNT(gd, cycle);
            eng_entry_virtual_cycle = cycle;
        } else {
            eng_saved_cycle_count = GET_CYCLE_COUNT(gd);
            eng_entry_virtual_cycle = GET_CYCLE_COUNT(gd);
        }

        /* Update CV voltage based on (possibly injected) cycle count */
#if (BUCKBOOST_USED_NU6805 == 1 && CONFIG_CYCLE_CV_REDUCTION_ENABLE == 1)
        hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

        /* Virtual params NOT read here — only on Refresh (0xAA) trigger.
         * Prevents CC flap re-entry from re-loading stale virtual values. */

        printk("eng enter: saved=%d virt=%d\n", eng_saved_cycle_count, eng_entry_virtual_cycle);
    }
    else if (gd->eng_mode_active && work_mode == 0x00) {
        /* EXIT engineering mode */
        usb_bridge_exit_eng_mode();
    }
}

/*===================== Erase / Refresh Commands =====================*/

static void usb_bridge_check_eng_cmd(void)
{
    if (!gd->eng_mode_active) return;

    uint8_t cmd = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_ENG_ERASE_CMD, &cmd);

    if (cmd == ENG_CMD_ERASE_ALL) {
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_CMD_STATUS, ENG_STATUS_BUSY);

        bool ok = battery_record_erase_all();

        /* Notify WB7720 to clear exc_cache */
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_TOTAL_COUNT, 0);
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_READY, 0x00);

        exc_cursor_page = 0;
        exc_cursor_idx = 0;
        exc_records_sent = 0;

        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_CMD_STATUS,
                                ok ? ENG_STATUS_OK : ENG_STATUS_FAIL);
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_ERASE_CMD, 0x00);

        printk("eng erase %s\n", ok ? "ok" : "fail");
    }
    else if (cmd == ENG_CMD_REFRESH) {
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_CMD_STATUS, ENG_STATUS_BUSY);

        /* Settle delta */
        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                              ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);
        eng_saved_cycle_count = GET_CYCLE_COUNT(gd);

        /* Re-read params */
        usb_bridge_read_virtual_params();
        usb_bridge_apply_eng_datetime();

        /* Re-read cycle */
        uint16_t cycle = 0;
        hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle, 2);
        if (cycle != 0xFFFF) {
            SET_CYCLE_COUNT(gd, cycle);
            eng_entry_virtual_cycle = cycle;
        } else {
            eng_entry_virtual_cycle = GET_CYCLE_COUNT(gd);
        }

        /* Update CV voltage based on refreshed cycle count */
#if (BUCKBOOST_USED_NU6805 == 1 && CONFIG_CYCLE_CV_REDUCTION_ENABLE == 1)
        hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_CMD_STATUS, ENG_STATUS_OK);
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_ERASE_CMD, 0x00);

        printk("eng refresh\n");
    }
}

/*===================== Time Sync =====================*/

static void usb_bridge_check_time_sync(void)
{
    uint8_t trigger = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_TIME_SYNC, &trigger);

    if (trigger == TIME_SYNC_MAGIC) {
        usb_bridge_apply_eng_datetime();
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_TIME_SYNC, 0x00);
        printk("time sync ok\n");
    }
}

/*===================== Production Mode =====================*/

static void usb_bridge_check_prod_mode(void)
{
    uint8_t flag = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, PROD_MODE_FLAG, &flag);
    if (flag != PROD_MODE_MAGIC) return;

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_WRITE_STATUS, ENG_STATUS_BUSY);

    ProductInfo_t info;
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_MANUFACTURER,
                              (uint8_t*)info.manufacturer_name, 20);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_MODEL,
                              (uint8_t*)info.model_name, 20);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_BATTERY_MFR,
                              (uint8_t*)info.battery_mfr, 20);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_BATTERY_MODEL,
                              (uint8_t*)info.battery_model, 20);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_PROD_DATE,
                              (uint8_t*)info.battery_prod_date, 20);
    hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, PROD_SERIAL,
                              (uint8_t*)info.serial, SERIAL_FIELD_SIZE);

    product_info_write(&info);

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_WRITE_STATUS, ENG_STATUS_OK);
    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_MODE_FLAG, 0x00);

    usb_bridge_reset_product_info();
    printk("prod info written\n");
}

/*===================== Periodic Update (cnt 0-13) =====================*/

void usb_bridge_periodic_update(void)
{
    uint16_t write_buf;

    /* cnt 13: unconditional — always poll eng/prod/time regardless of force_usb_mode */
    if (cnt == 13)
    {
        usb_bridge_check_eng_mode();
        usb_bridge_check_time_sync();
        usb_bridge_check_eng_cmd();
        usb_bridge_check_prod_mode();
        cnt = 0;

        if (!gd->force_usb_mode && is_usb_enable) {
            usb_bridge_sleep();
            is_usb_enable = false;
        }
        return;
    }

    /* cnt 0-12: gated by force_usb_mode */
    if (!gd->force_usb_mode) {
        if (is_usb_enable) {
            usb_bridge_sleep();
            is_usb_enable = false;
        }
        cnt++;
        if (cnt >= 14) cnt = 0;
        return;
    }

    if (!is_usb_enable) {
        usb_bridge_wakeup();
        is_usb_enable = true;
    }

    /* ---- cnt 0: SOC ---- */
    if (cnt == 0)
    {
        write_buf = gd->real_soc_show;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_SOC_PCT, (uint8_t*)&write_buf, 1);
    }
    /* ---- cnt 1: Capacity ---- */
    else if (cnt == 1)
    {
        write_buf = CONFIG_BATTERY_CAPACITY_MAH;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CAPACITY_MAH, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 2: VBAT + Cell1 + Cell2 (MCU ADC triple-sample, one shared PD3 EMA) ---- */
    else if (cnt == 2)
    {
        /* PD3 = VBAT- negative rail (shared EMA filter for all channels) */
        static int16_t vbat_minus = 0;
        uint16_t pd3_adc = hal_badc_meas(_BADC_CH_PD3_ADC9);
        if (pd3_adc >= BADC_PD3_VBATN_MIN_MV) {
            uint16_t vdd_mv = hal_badc_get_vdd_mv();
            int16_t raw = (int16_t)(pd3_adc * 3) - (int16_t)(vdd_mv * 2);
            vbat_minus += (raw - vbat_minus) >> BADC_PD3_VBATN_EMA_SHIFT;
        }

        /* PB6 = BAT2+ total voltage */
        uint16_t pb6_bat2p = (uint16_t)(hal_badc_meas(_BADC_CH_PB6_ADC7) * BADC_PB6_BAT2P_DIV_RATIO);
        vbat_cached = (uint16_t)((int16_t)pb6_bat2p - vbat_minus);

        /* PC7 = Cell2 midpoint */
        uint16_t pc7_mv = (uint16_t)(hal_badc_meas(_BADC_CH_PC7_ADC4) * BADC_PC7_CELL2_DIV_RATIO);
        cell2_voltage = (uint16_t)((int16_t)pc7_mv - vbat_minus);

        /* Write VBAT total to 0x05 */
        write_buf = vbat_cached;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_VBAT_MV, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 3: IBAT (0 when idle/shutdown) ---- */
    else if (cnt == 3)
    {
        int16_t ibat = g_buckboost.adc_ibat;
        /* SHUTDOWN mode: force 0 (NU6805 returns stale discharge value in idle) */
        if (g_buckboost.woke_mode == BUCKBOOST_SHUTDOWM_MODE)
            ibat = 0;
        write_buf = (uint16_t)ibat;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_IBAT_MA, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 4: Temperature (0.1°C units for HID report) ---- */
    else if (cnt == 4)
    {
        if (gd->eng_mode_active && gd->eng_virtual_temp != (int16_t)VIRTUAL_TEMP_SENTINEL) {
            write_buf = (uint16_t)gd->eng_virtual_temp;           /* already 0.1°C */
        } else {
            /* TypeC NTC: °C → ×10 for 0.1°C units (matches WB7720 HID format) */
            int16_t temp_c = gd->sys_infos.ntc_temp_typec;
            write_buf = (uint16_t)(temp_c * 10);
        }
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_TEMP_DC, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 5: Cycle Count ---- */
    else if (cnt == 5)
    {
        write_buf = GET_CYCLE_COUNT(gd);
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CYCLE_COUNT, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 6: Internal Resistance ---- */
    else if (cnt == 6)
    {
        write_buf = gd->Bat_Rdc;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_R_INTERNAL_MOHM, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 7: SOH ---- */
    else if (cnt == 7)
    {
        write_buf = gd->Bat_SoH;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_SOH_PCT_X100, (uint8_t*)&write_buf, 2);
    }
    /* ---- cnt 8: Error Counts (Flash scan) ---- */
    else if (cnt == 8)
    {
        uint16_t err_buf[3];
        err_buf[0] = battery_record_get_overtemp_count();
        err_buf[1] = battery_record_get_overvolt_count();
        err_buf[2] = 0;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_ERR_OVERTEMP_CNT, (uint8_t*)err_buf, 6);
    }
    /* ---- cnt 9: Charge State ---- */
    else if (cnt == 9)
    {
        uint8_t state = 0;
        if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
            state = 1;
        else if (g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
            state = 2;
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_CHARGE_STATE, state);
    }
    /* ---- cnt 10: Cell Info (X20: 2-cell, uses MCU ADC cached from cnt 2) ---- */
    else if (cnt == 10)
    {
        uint8_t cell_buf[5];
        cell_buf[0] = CONFIG_BATTERY_CELL_COUNT;
        uint16_t c2 = cell2_voltage;  /* MCU ADC PC7, updated at cnt 2 */
        uint16_t c1 = (vbat_cached > c2) ? (vbat_cached - c2) : 0;  /* MCU ADC PB6 - Cell2 */

        /* Engineering mode virtual override */
        if (gd->eng_mode_active) {
            if (gd->eng_virtual_cell1 != VIRTUAL_CELL_SENTINEL) c1 = gd->eng_virtual_cell1;
            if (gd->eng_virtual_cell2 != VIRTUAL_CELL_SENTINEL) c2 = gd->eng_virtual_cell2;
        }

        memcpy(&cell_buf[1], &c1, 2);
        memcpy(&cell_buf[3], &c2, 2);
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CELL_COUNT, cell_buf, 5);
    }
    /* ---- cnt 11: PCB Temperature ---- */
    else if (cnt == 11)
    {
        int16_t pcb_temp = gd->sys_infos.ntc_temp_typec;
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_PCB_TEMP_DC, (uint8_t*)&pcb_temp, 2);
    }
    /* ---- cnt 12: Exception Log Sequential Push ---- */
    else if (cnt == 12)
    {
        uint8_t total = 0;
        for (uint8_t p = 0; p < LOG_PAGE_COUNT; p++) {
            exc_page_counts[p] = battery_record_get_page_count(p);
            total += exc_page_counts[p];
        }
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_TOTAL_COUNT, total);

        if (total == 0) goto exc_done;

        if (exc_cursor_page >= LOG_PAGE_COUNT) exc_cursor_page = 0;
        if (exc_cursor_idx >= exc_page_counts[exc_cursor_page])
            exc_cursor_idx = 0;

        for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
            if (exc_page_counts[exc_cursor_page] > 0) break;
            exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
            exc_cursor_idx = 0;
        }

        BatteryExceptionRecord_t rec;
        if (battery_record_read_by_page_index(exc_cursor_page, exc_cursor_idx, &rec)
            && rec.record_id != 0) {
            /* NU17112 主导握手: 先清 READY → 写记录 → 设 READY
             * (WB7720 不清 READY，由 NU17112 每轮自行重置) */
            hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_READY, 0x00);
            hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_CURRENT_IDX, exc_records_sent);
            hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_EXC_RECORD, (uint8_t*)&rec, 20);
            hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_EXC_READY, 0xA5);
            exc_records_sent++;

            exc_cursor_idx++;
            if (exc_cursor_idx >= exc_page_counts[exc_cursor_page]) {
                exc_cursor_idx = 0;
                for (uint8_t i = 0; i < LOG_PAGE_COUNT; i++) {
                    exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
                    if (exc_page_counts[exc_cursor_page] > 0) break;
                }
            }
        } else {
            exc_cursor_idx = 0;
            exc_cursor_page = (exc_cursor_page + 1) % LOG_PAGE_COUNT;
        }
exc_done: ;
    }

    cnt++;
    if (cnt >= 14) cnt = 0;
}

#endif /* CONFIG_USB_BRIDGE_ENABLE */
