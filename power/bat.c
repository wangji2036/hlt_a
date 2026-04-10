#include "buckboost.h"
#include "bat.h"
#include "printk.h"
#include "g_data.h"
#include "app.h"

#define BAT_STS_DISG 0x00
#define BAT_STS_CHNG 0x01
#define BAT_STS_FULL 0x02

#define BAT_BATTERY_DEFAULT 6676543 // 18wh ~~ 5000mAh
#define BAT_ENERGY_CALI_VOLT 3250
#define BAT_BATTERY_EMPTY_VOLTAGE 3150
#define BAT_ENERGY_FULL_LEVEL 90
#define BAT_BAT_rDC 50
#define BAT_DISG_RATE 100 / 94
uint8_t temp_bat_ui;
static uint8_t bat_level_end = 0;

struct bat_info g_bat;

void nano_battery_ocv_handle(void);
void nano_battery_soe_handle(void);
void nano_battery_ui_handle(void);

// OCV表：对应0%-100% SOC（每10%一个点），适用于3.85V标称电压、5000mAh电池
const uint16_t level_ocv_table[] = {2750, 3250, 3450, 3550, 3650, 3750, 3850, 3950, 4050, 4150, 4300};

uint8_t nano_battery_ocv_level_find(int16_t bat_volt)
{
    uint8_t res = 0;
    if (bat_volt <= level_ocv_table[0])
    {
        res = 0;
        return res;
    }
    if (bat_volt >= level_ocv_table[10])
    {
        res = 100;
        return res;
    }
    for (uint8_t i = 1; i < 11; i++)
    {
        if (bat_volt <= level_ocv_table[i])
        {
            res = (i - 1) * 10 + 10 * (bat_volt - level_ocv_table[i - 1]) / (level_ocv_table[i] - level_ocv_table[i - 1]);
            break;
        }
    }
    return res;
}

void nano_battery_soe_handle(void)
{
    int32_t bat_soe_uint = g_bat.vbat * g_bat.ibat / 1000 / 100; // V * A * s = V*0.1A *0.1s = 0.01wS

    if (g_bat.sbat == BAT_STS_CHNG)
    {
        int16_t vbat = g_bat.vbat > g_bat.ibat * g_bat.rbat / 1000 ? g_bat.vbat - g_bat.ibat * g_bat.rbat / 1000 : 0;
        if (vbat / 2 < BAT_ENERGY_CALI_VOLT)
        {
            g_bat.bat_energy_cali = 0;
            g_bat.bat_soe_in_cali = true;

            if (!g_bat.bat_soe_in_cali)
                g_bat.bat_energy_current = 0;
        }
    }
    else if (g_bat.sbat == BAT_STS_FULL)
    {
        if (g_bat.bat_soe_in_cali)
        {
            g_bat.bat_energy_total = g_bat.bat_energy_cali;
            g_bat.bat_energy_cali = 0;
            g_bat.bat_soe_in_cali = false;
            g_bat.bat_level_soe = 100;
            g_bat.bat_soe_calied = true;

            int32_t flash_bat_energy_total = g_bat.bat_energy_total;
            int32_t flash_bat_energy_total_check = ~flash_bat_energy_total;

            // Read-Modify-Write: preserve entire 512-byte page (0x1800-0x19FF)
            extern uint8_t g_page_1800_buf[512];
            uint16_t pg_i;
            for (pg_i = 0; pg_i < 512; pg_i++) {
                g_page_1800_buf[pg_i] = __read_08bits(AP_CFG_ROM_ADDR_PRO_INFO + pg_i);
            }

            // Modify BAT energy at offset 0x100 (BAT_ADDR_BASE - AP_CFG_ROM_ADDR_PRO_INFO)
            uint16_t bat_off = BAT_ADDR_BASE - AP_CFG_ROM_ADDR_PRO_INFO;  // = 256
            g_page_1800_buf[bat_off + 0] = (uint8_t)(flash_bat_energy_total >> 0);
            g_page_1800_buf[bat_off + 1] = (uint8_t)(flash_bat_energy_total >> 8);
            g_page_1800_buf[bat_off + 2] = (uint8_t)(flash_bat_energy_total >> 16);
            g_page_1800_buf[bat_off + 3] = (uint8_t)(flash_bat_energy_total >> 24);
            g_page_1800_buf[bat_off + 4] = (uint8_t)(flash_bat_energy_total_check >> 0);
            g_page_1800_buf[bat_off + 5] = (uint8_t)(flash_bat_energy_total_check >> 8);
            g_page_1800_buf[bat_off + 6] = (uint8_t)(flash_bat_energy_total_check >> 16);
            g_page_1800_buf[bat_off + 7] = (uint8_t)(flash_bat_energy_total_check >> 24);

            // Erase page (use page-aligned address)
            hal_fmc_erase_page(AP_CFG_ROM_ADDR_PRO_INFO);

            // Write back all 512 bytes in big-endian word format
            for (pg_i = 0; pg_i < 512; pg_i += 4) {
                uint32_t word = ((uint32_t)g_page_1800_buf[pg_i] << 24) |
                                ((uint32_t)g_page_1800_buf[pg_i+1] << 16) |
                                ((uint32_t)g_page_1800_buf[pg_i+2] << 8) |
                                (uint32_t)g_page_1800_buf[pg_i+3];
                hal_fmc_write_word(AP_CFG_ROM_ADDR_PRO_INFO + pg_i, word);
            }

            printk("update bat = %d\n", flash_bat_energy_total);
        }
        else if (g_bat.bat_soe_calied)
        {
            g_bat.bat_energy_current = g_bat.bat_energy_total;
        }
        else
        {

            int32_t flash_bat_energy_total = *(int32_t *)(BAT_ADDR_BASE);
            int32_t flash_bat_energy_total_check = *(int32_t *)(BAT_ADDR_BASE + 4);
            if (flash_bat_energy_total == ~flash_bat_energy_total_check && flash_bat_energy_total != 0xFFFFFFFF)
            {
                g_bat.bat_energy_total = flash_bat_energy_total;
                g_bat.bat_energy_current = g_bat.bat_energy_total;
                printk("load bat = %d\n", flash_bat_energy_total);
                g_bat.bat_soe_calied = true;
            }
            else
            {
                g_bat.bat_energy_total = BAT_BATTERY_DEFAULT;
                g_bat.bat_energy_current = g_bat.bat_energy_total;
                printk("default bat = %d\n", flash_bat_energy_total);
                g_bat.bat_soe_calied = true;
            }
        }
    }

    if (g_bat.sbat == BAT_STS_DISG)
        g_bat.bat_energy_current += bat_soe_uint * BAT_DISG_RATE;
    else
        g_bat.bat_energy_current += bat_soe_uint;

    if (g_bat.bat_soe_in_cali)
    {
        g_bat.bat_energy_cali += bat_soe_uint;
    }

    if (g_bat.bat_energy_total != 0)
    {
        if (g_bat.bat_energy_current > g_bat.bat_energy_total)
            g_bat.bat_level_soe = 100;
        else if (g_bat.bat_energy_current < 0)
            g_bat.bat_level_soe = 0;
        else
        {
            g_bat.bat_level_soe = 100 * g_bat.bat_energy_current / g_bat.bat_energy_total;
        }
    }
}

void nano_battery_ocv_handle(void)
{
    static uint8_t ocv_cnt0 = 0;
    static uint8_t ocv_cnt1 = 0;

    int16_t vbat = g_bat.vbat > g_bat.ibat * g_bat.rbat / 1000 ? g_bat.vbat - g_bat.ibat * g_bat.rbat / 1000 : 0;

    int8_t ocv_level = nano_battery_ocv_level_find(vbat / 2);

    if (g_bat.sbat == BAT_STS_DISG && ocv_level < g_bat.bat_level_ocv)
    {
        ocv_cnt0++;
        if (ocv_cnt0 >= 20)
        {
            if (g_bat.bat_level_ocv > 0)
                g_bat.bat_level_ocv--;
            ocv_cnt0 = 0;
        }
    }
    else if (g_bat.sbat != BAT_STS_DISG && ocv_level > g_bat.bat_level_ocv)
    {
        ocv_cnt1++;
        if (ocv_cnt1 >= 20)
        {
            if (g_bat.bat_level_ocv < 100)
                g_bat.bat_level_ocv++;
            ocv_cnt1 = 0;
        }
    }
    else
    {
        ocv_cnt0 = 0;
        ocv_cnt1 = 0;
    }
}

void nano_battery_ui_handle(void)
{

    static uint16_t level_ui_cnt = 0;
    static uint8_t empty_cnt = 0;
    static uint8_t empty_flg = 0;
    static uint16_t ui_100_cnt = 0;

    if (g_bat.bat_soe_calied)
    {
        if (g_bat.sbat == BAT_STS_CHNG)
        {
            if (g_bat.bat_level_soe >= BAT_ENERGY_FULL_LEVEL)
                temp_bat_ui = 100;
            else
            {
                if (BAT_ENERGY_FULL_LEVEL != g_bat.bat_level_chng_ey_s)
                {
                    temp_bat_ui = g_bat.bat_level_chng_ui_s +
                                 (100 - g_bat.bat_level_chng_ui_s) * (g_bat.bat_level_soe - g_bat.bat_level_chng_ey_s) / (BAT_ENERGY_FULL_LEVEL - g_bat.bat_level_chng_ey_s);

                    if (temp_bat_ui > 100)
                        temp_bat_ui = 100;
                }
                else
                    temp_bat_ui = 100;
            }

            empty_cnt = 0;
            empty_flg = 0;
            ui_100_cnt = 0;
            g_bat.bat_level_disg_ey_s = g_bat.bat_level_soe;
            g_bat.bat_level_disg_ui_s = g_bat.bat_level_ui;
            g_bat.bat_energy_disgstart = g_bat.bat_energy_current;
        }
        else if (g_bat.sbat == BAT_STS_DISG)
        {
            int16_t end_ibat = (g_bat.vbat / 2) * g_bat.ibat / BAT_BATTERY_EMPTY_VOLTAGE;
            int vbat_end = BAT_BATTERY_EMPTY_VOLTAGE - end_ibat * BAT_BAT_rDC / 1000;
            bat_level_end = nano_battery_ocv_level_find(vbat_end);

            if (g_bat.bat_level_disg_ey_s != bat_level_end)
            {
                temp_bat_ui = g_bat.bat_level_disg_ui_s * (g_bat.bat_level_soe - bat_level_end) / (g_bat.bat_level_disg_ey_s - bat_level_end);
                if (temp_bat_ui < 0)
                    temp_bat_ui = 100;
            }
            else
                temp_bat_ui = 0;

            if (g_bat.bat_level_ui == 100)
            {
                int32_t d = g_bat.bat_energy_disgstart > g_bat.bat_energy_current ? (g_bat.bat_energy_disgstart - g_bat.bat_energy_current) : 0;
                if (d > g_bat.bat_energy_disgstart / 120)
                    temp_bat_ui = 99;
                else
                    temp_bat_ui = 100;
            }

            if (g_bat.vbat / 2 < BAT_BATTERY_EMPTY_VOLTAGE)
            {
                empty_cnt++;
                if (empty_cnt >= 10)
                {
                    printk("\n bat empty!");
                    empty_cnt = 0;
                    empty_flg = 1;
                }
            }
            else
            {
                empty_cnt = 0;
            }

            if (empty_flg == 1)
            {
                printk("\n bat ui to 0!");
                temp_bat_ui = 0;
            }

            g_bat.bat_level_chng_ey_s = g_bat.bat_level_soe;
            g_bat.bat_level_chng_ui_s = g_bat.bat_level_ui;
        }
        else
        {
            temp_bat_ui = 100;
            empty_flg = 0;
            g_bat.bat_energy_disgstart = g_bat.bat_energy_current;
            g_bat.bat_level_disg_ey_s = g_bat.bat_level_soe;
            g_bat.bat_level_disg_ui_s = g_bat.bat_level_ui;
        }
    }
    else
    {
        temp_bat_ui = g_bat.bat_level_ocv;
        if (g_bat.sbat == BAT_STS_CHNG)
        {
            ui_100_cnt = 0;
        }
        else if (g_bat.sbat == BAT_STS_DISG)
        {
            if (g_bat.vbat / 2 < (BAT_BATTERY_EMPTY_VOLTAGE + 150) && g_bat.ibat < -100)
            {
                empty_cnt++;
                if (empty_cnt >= 10)
                {
                    empty_cnt = 0;
                    empty_flg = 1;
                }
            }
            else
            {
                empty_cnt = 0;
            }

            if (empty_flg == 1)
            {
                temp_bat_ui = 0;
            }
        }
        else
        {
            temp_bat_ui = 100;
            empty_flg = 0;
        }
    }

    if (temp_bat_ui != g_bat.bat_level_ui)
    {
        level_ui_cnt++;
        if (temp_bat_ui > g_bat.bat_level_ui && (g_bat.sbat == BAT_STS_CHNG || g_bat.sbat == BAT_STS_FULL))
        {
            if (level_ui_cnt >= 100)
            {
                if (g_bat.bat_level_ui < 100)
                    g_bat.bat_level_ui++;
                    if(g_bat.bat_level_ui == 100){
                        gd->Battery_cycle_count ++;
                    }
                level_ui_cnt = 0;
            }
        }
        else if (temp_bat_ui < g_bat.bat_level_ui && g_bat.sbat == BAT_STS_DISG)
        {
            if (g_bat.bat_level_ui > 85)
            {
                if (level_ui_cnt >= 300)
                {
                    if (g_bat.bat_level_ui > 0)
                        g_bat.bat_level_ui--;
                    level_ui_cnt = 0;
                }
            }
            else
            {
                if (level_ui_cnt >= 10)
                {
                    if (g_bat.bat_level_ui > 0)
                        g_bat.bat_level_ui--;
                    level_ui_cnt = 0;
                }
            }
        }
    }
    else
        level_ui_cnt = 0;

    if (g_bat.bat_level_ui >= 100)
        g_bat.bat_level_ui = 100;
    if (g_bat.bat_level_ui <= 0)
        g_bat.bat_level_ui = 0;
}

void battery_task_handle(void) // 100mS
{
    static uint8_t delay_cnt = 30;

    g_bat.vbat = g_buckboost.adc_vbat;
    g_bat.rbat = BAT_BAT_rDC;
    g_bat.ibat = g_buckboost.adc_ibat;
    if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
    {
        if (g_buckboost.bat_full_flag)
            g_bat.sbat = BAT_STS_FULL;
        else
            g_bat.sbat = BAT_STS_CHNG;
    }
    else
        g_bat.sbat = BAT_STS_DISG;

    if (g_buckboost.adc_ibus > 0)
        g_bat.ibat = g_buckboost.adc_ibus * g_buckboost.adc_vbus * 90 / g_buckboost.adc_vbat / 100;
    else
        g_bat.ibat = g_buckboost.adc_ibus * g_buckboost.adc_vbus * 100 / g_buckboost.adc_vbat / 90;

    if (g_bat.ibat == 0)
        return;

#define INIT_VALUE 0xff89
    if (g_bat.bat_is_inited != INIT_VALUE)
    {
        if (delay_cnt)
            delay_cnt--;
        if (delay_cnt == 0)
        {
            osal_mem_set((&g_bat), 0, sizeof(struct bat_info));
            g_bat.bat_is_inited = INIT_VALUE;
            g_bat.vbat = g_buckboost.adc_vbat;
            g_bat.rbat = BAT_BAT_rDC;
            g_bat.ibat = g_buckboost.adc_ibat;
            if (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
            {
                if (g_buckboost.bat_full_flag)
                    g_bat.sbat = BAT_STS_FULL;
                else
                    g_bat.sbat = BAT_STS_CHNG;
            }
            else
                g_bat.sbat = BAT_STS_DISG;

            int16_t vbat = (int16_t)g_bat.vbat > (int16_t)(g_bat.ibat * g_bat.rbat / 1000) ? (g_bat.vbat - g_bat.ibat * g_bat.rbat / 1000) : 0;

            g_bat.bat_level_ocv = nano_battery_ocv_level_find(vbat / 2);
            g_bat.bat_level_ui = g_bat.bat_level_ocv;

            g_bat.bat_energy_total = BAT_BATTERY_DEFAULT;
            g_bat.bat_energy_current = BAT_BATTERY_DEFAULT / 100 * g_bat.bat_level_ui;
            g_bat.bat_level_chng_ey_s = g_bat.bat_level_ui;
            g_bat.bat_level_chng_ui_s = g_bat.bat_level_ui;
            g_bat.bat_level_disg_ey_s = g_bat.bat_level_ui;
            g_bat.bat_level_disg_ui_s = g_bat.bat_level_ui;
            g_bat.bat_soe_calied = true;
        }
        return;
    }

    nano_battery_ocv_handle();
    nano_battery_soe_handle();
    nano_battery_ui_handle();

    // 定期保存g_bat到gd->g_bat，用于睡眠唤醒后恢复
    osal_mem_copy((void *)&(gd->g_bat), &(g_bat), sizeof(struct bat_info));
}
