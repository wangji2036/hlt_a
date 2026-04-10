#include "usb_bridge.h"

#if CONFIG_USB_BRIDGE_ENABLE

#include "g_data.h"
#include "buckboost.h"
#include "nu6805.h"
#include "bat_record.h"
#include "i2cm.h"
#include "printk.h"
#include "../hal/badc.h"
#include <string.h>

/*===================== External References =====================*/
extern struct buckboost_s g_buckboost;
/********************* Static Variables *********************/

/* Round-robin counter for telemetry writes */
static uint8_t cnt = 0;

/* USB enable state for force_usb_mode control */
static bool is_usb_enable = false;

/* Exception log sequential push cursor (cnt 12) */
static uint8_t exc_cursor_page = 0;
static uint8_t exc_cursor_idx = 0;
static uint8_t exc_records_sent = 0;
static uint8_t exc_page_counts[LOG_PAGE_COUNT];

/* Saved real cycle count before engineering mode injection */
static uint16_t eng_saved_cycle_count = 0;   /* Real cycle count before eng mode */
static uint16_t eng_entry_virtual_cycle = 0; /* Virtual cycle count at entry (baseline for delta) */

/********************* ProductInfo Sync *********************/

void usb_bridge_reset_product_info(void)
{
    ProductInfo_t info;
    product_info_read(&info);

    /* Write 5 fields (each 20 bytes) + serial (10 bytes) */
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

/********************* Init *********************/

void usb_bridge_init(void)
{
    cnt = 0;
    is_usb_enable = false;
    exc_cursor_page = 0;
    exc_cursor_idx = 0;
    exc_records_sent = 0;

    /* Sync ProductInfo from Flash to WB7720 I2C buffer */
    usb_bridge_reset_product_info();

    /* WB7720 boots with USB enabled by default.
     * Explicitly sleep it so USB only activates via triple-click. */
    usb_bridge_sleep();
}

/********************* Sleep / Wake *********************/

void usb_bridge_sleep(void)
{
    printk("[sleep] eng=%d saved=%d entry=%d cur=%d\n",
           gd->eng_mode_active, eng_saved_cycle_count, eng_entry_virtual_cycle, GET_CYCLE_COUNT(gd));
    /* Exit engineering mode if active */
    if (gd->eng_mode_active) {
        gd->eng_mode_active = 0;
        gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
        gd->eng_virtual_temp  = VIRTUAL_TEMP_SENTINEL;
        /* Restore real cycle count + cycles accumulated during eng mode */
        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                               ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);

        /* Restore CV voltage based on real cycle count */
#if (BUCKBOOST_USED_NU6801 == 1)
        hal_nu6801_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif
#if (BUCKBOOST_USED_NU6805 == 1)
        hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

        printk("[sleep] cycle=%d\n", GET_CYCLE_COUNT(gd));
        /* Immediately sync restored cycle count to WB7720 before sleep */
        uint16_t cycle_wb = GET_CYCLE_COUNT(gd);
        hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CYCLE_COUNT, (uint8_t*)&cycle_wb, 2);
    }

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_SLEEP_CMD, 0x01);
    printk("usb bridge sleep\n");
}

void usb_bridge_wakeup(void)
{
    /* Force USB re-enumeration: disconnect first, then reconnect.
     * WB7720 usb_enable() only does Init+Connect without Disconnect,
     * so Windows may not re-enumerate. Send sleep→delay→wakeup sequence. */
    /* WAKE_CMD 重试: 第1次触发 EXTI 唤醒，读回操作提供自然延时 */
    for (int retry = 0; retry < 5; retry++) {
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_WAKEUP_CMD, 0x01);
        uint8_t readback = 0;
        hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_WAKEUP_CMD, &readback);
        printk("[WB] wake r=%d rb=%02X\n", retry, readback);
    }

    /* Resync ProductInfo after wakeup */
    usb_bridge_reset_product_info();

    /* Resync cycle count: WB7720 resets i2c_buff[0x80] to 0x0000 on sleep/wakeup.
     * Without this write, the next eng-mode entry would read 0x0000 != 0xFFFF and
     * overwrite cycle count with 0, corrupting the stored value. */
    uint16_t cycle_buf = GET_CYCLE_COUNT(gd);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle_buf, 2);
}

/********************* Engineering Mode Helpers *********************/

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
    /* If eng mode injected a virtual cycle count (not sentinel),
     * keep it as the new real value and persist to Flash.
     * Otherwise restore saved real value + any increments. */
    if (eng_entry_virtual_cycle != eng_saved_cycle_count) {
        /* Virtual cycle was injected — keep current value (includes any increments) */
        printk("[eng] exit: cycle=%d (injected, kept)\n", GET_CYCLE_COUNT(gd));
    } else {
        /* No injection (sentinel) — restore real + increments */
        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                               ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);
        printk("[eng] exit: cycle=%d (restored)\n", GET_CYCLE_COUNT(gd));
    }

    /* Update CV voltage based on final cycle count */
#if (BUCKBOOST_USED_NU6801 == 1)
    hal_nu6801_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif
#if (BUCKBOOST_USED_NU6805 == 1)
    hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

    gd->eng_mode_active = 0;
    gd->eng_virtual_cell1 = VIRTUAL_CELL_SENTINEL;
    gd->eng_virtual_cell2 = VIRTUAL_CELL_SENTINEL;
    gd->eng_virtual_temp  = VIRTUAL_TEMP_SENTINEL;
    uint16_t cycle_wb = GET_CYCLE_COUNT(gd);
    hal_i2cm_write_multi_bytes(USBD_WB7720_ADDR, REG_CYCLE_COUNT, (uint8_t*)&cycle_wb, 2);
#if CYCLE_COUNT_FLASH_PERSIST
    cycle_count_save_to_flash();
#endif
}

static void usb_bridge_check_eng_mode(void)
{
    uint8_t work_mode = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_WORK_MODE, &work_mode);

    if (work_mode == ENG_MODE_MAGIC && !gd->eng_mode_active) {
        gd->eng_mode_active = 1;

        /* Read datetime (7B: 0x60-0x66) */
        usb_bridge_apply_eng_datetime();

        /* Read cycle count (2B: 0x80) */
        uint16_t cycle = 0;
        hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle, 2);
        if (cycle != 0xFFFF) {
            /* Save real cycle count before injecting virtual value */
            eng_saved_cycle_count = GET_CYCLE_COUNT(gd);
            SET_CYCLE_COUNT(gd, (uint16_t)cycle);
            eng_entry_virtual_cycle = (uint16_t)cycle;
        } else {
            /* Sentinel: keep real value, record it as baseline */
            eng_entry_virtual_cycle = GET_CYCLE_COUNT(gd);
        }

        /* Update CV voltage based on injected cycle count */
#if (BUCKBOOST_USED_NU6801 == 1)
        hal_nu6801_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif
#if (BUCKBOOST_USED_NU6805 == 1)
        hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

        /* Read virtual parameters (6B: 0x82-0x87) */
        usb_bridge_read_virtual_params();

        printk("eng mode enter: saved=%d virt=%d\n", eng_saved_cycle_count, eng_entry_virtual_cycle);
    }
    else if (gd->eng_mode_active && work_mode == 0x00) {
        /* PC wrote 0x00 to REG_WORK_MODE → exit engineering mode */
        usb_bridge_exit_eng_mode();
        printk("eng mode exit\n");
    }
}

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

        /* Reset push cursor so next cnt==12 starts from page 0 */
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

        /* 1. Settle delta from current session: restore real + cycles accumulated */
        uint16_t cycles_added = (GET_CYCLE_COUNT(gd) >= eng_entry_virtual_cycle)
                               ? (GET_CYCLE_COUNT(gd) - eng_entry_virtual_cycle) : 0;
        SET_CYCLE_COUNT(gd, eng_saved_cycle_count + cycles_added);

        /* 2. Update baseline for next session */
        eng_saved_cycle_count = GET_CYCLE_COUNT(gd);

        /* 3. Read new virtual params + arm single-shot */
        usb_bridge_read_virtual_params();
        usb_bridge_apply_eng_datetime();

        /* 4. Re-read cycle count and re-inject */
        uint16_t cycle = 0;
        hal_i2cm_read_multi_bytes(USBD_WB7720_ADDR, REG_ENG_CYCLE_COUNT, (uint8_t*)&cycle, 2);
        if (cycle != 0xFFFF) {
            SET_CYCLE_COUNT(gd, (uint16_t)cycle);
            eng_entry_virtual_cycle = (uint16_t)cycle;
        } else {
            eng_entry_virtual_cycle = GET_CYCLE_COUNT(gd);
        }

        /* Update CV voltage based on refreshed cycle count */
#if (BUCKBOOST_USED_NU6801 == 1)
        hal_nu6801_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif
#if (BUCKBOOST_USED_NU6805 == 1)
        hal_nu6805_update_cv_by_cycle(GET_CYCLE_COUNT(gd));
#endif

        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_CMD_STATUS, ENG_STATUS_OK);
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_ENG_ERASE_CMD, 0x00);

        printk("eng refresh\n");
    }
}

static void usb_bridge_check_time_sync(void)
{
    /* RTC校准不再限制工程模式，上位机连接时自动同步 */

    uint8_t trigger = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, REG_TIME_SYNC, &trigger);

    if (trigger == TIME_SYNC_MAGIC) {
        usb_bridge_apply_eng_datetime();
        hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, REG_TIME_SYNC, 0x00);
        printk("time sync ok\n");
    }
}

static void usb_bridge_check_prod_mode(void)
{
    uint8_t flag = 0;
    hal_i2cm_read_one_byte(USBD_WB7720_ADDR, PROD_MODE_FLAG, &flag);

    if (flag != PROD_MODE_MAGIC) return;

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_WRITE_STATUS, ENG_STATUS_BUSY);

    /* Read 110B ProductInfo from WB7720 */
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

    /* Write to Flash */
    product_info_write(&info);

    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_WRITE_STATUS, ENG_STATUS_OK);
    hal_i2cm_wirte_one_byte(USBD_WB7720_ADDR, PROD_MODE_FLAG, 0x00);

    /* Refresh i2c_buff so WB7720 Type 0x03 reflects new data */
    usb_bridge_reset_product_info();

    printk("prod info written\n");
}

/********************* Periodic Update (47ms) *********************/

void usb_bridge_periodic_update(void)
{
    uint16_t write_buf;

    /* cnt 13: 无论 force_usb_mode 状态如何，始终轮询工程/生产模式。
     * 工厂生产时 force_usb_mode=false（未三击），但 WB7720 通过 PORT1 VBUS 自动上电
     * 并启用 USB，PC 可直接写入 PROD_MODE_FLAG，必须无条件轮询。
     * 工程模式同理，进入条件是 PC 写 REG_WORK_MODE，不依赖三击。
     *
     * cnt 在 cnt 0-12 阶段由底部 cnt++ 推进（不论 force_usb_mode），确保每 14 轮
     * 必然触发一次 cnt==13 检查。 */
    if (cnt == 13)
    {
        usb_bridge_check_eng_mode();
        usb_bridge_check_time_sync();
        usb_bridge_check_eng_cmd();
        usb_bridge_check_prod_mode();
        cnt = 0;
        /* 管理 WB7720 睡眠状态：非 force_usb_mode 时让 WB7720 回到睡眠 */
        if (!gd->force_usb_mode && is_usb_enable)
        {
            printk("[USB] mode->sleep eng=%d cycle=%d\n", gd->eng_mode_active, GET_CYCLE_COUNT(gd));
            usb_bridge_sleep();
            is_usb_enable = false;
        }
        return;
    }

    /* === USB 通信总开关 (cnt 0-12) === */
    if (!gd->force_usb_mode)
    {
        /* 非通信模式：确保 WB7720 睡眠，停止所有 I2C 写入（避免唤醒 WB7720）。
         * 注意：仍需执行底部 cnt++ 推进计数器，以便 cnt 最终到达 13 触发
         * 工程/生产模式轮询。 */
        if (is_usb_enable)
        {
            printk("[USB] mode->sleep eng=%d cycle=%d\n", gd->eng_mode_active, GET_CYCLE_COUNT(gd));
            usb_bridge_sleep();
            is_usb_enable = false;
        }
        cnt++;
        if (cnt >= 14) cnt = 0;
        return;
    }

    /* force_usb_mode 激活：按需唤醒 WB7720 */
    if (!is_usb_enable)
    {
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
    /* ---- cnt 2: VBAT total (from buckboost ADC) ---- */
    else if (cnt == 2)
    {
        write_buf = g_buckboost.adc_vcell1 + g_buckboost.adc_vcell2;
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
        printk("[cnt5] cycle=%d eng=%d\n", write_buf, gd->eng_mode_active);
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
    /* ---- cnt 8: Error Counts (overtemp + overvolt + overcurrent) ---- */
    else if (cnt == 8)
    {
        uint16_t err_buf[3];
        err_buf[0] = battery_record_get_overtemp_count();
        err_buf[1] = battery_record_get_overvolt_count();
        err_buf[2] = 0;  /* overcurrent reserved */
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
    /* ---- cnt 10: Cell Info (X20: 2-cell, from buckboost ADC) ---- */
    else if (cnt == 10)
    {
        uint8_t cell_buf[5];
        cell_buf[0] = CONFIG_BATTERY_CELL_COUNT;
        uint16_t c1 = g_buckboost.adc_vcell1;
        uint16_t c2 = g_buckboost.adc_vcell2;

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
            /* NU17112 主导握手: 先清 READY → 写记录 → 设 READY */
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
