#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "config.h"
#include "app.h"
#include "bat_record.h"

volatile struct ap_t *ap = (struct ap_t *)(AP_CFG_RAM_ADDR_BASE);
volatile struct gd_t *gd = (struct gd_t *)(G_DATA_RAM_ADDR_BASE);
uint8_t g_forbid_bypass_flag;

/* Save buffer for hot start (sleep wakeup) recovery */
uint8_t saved_exception_cache[sizeof(ap->exception_cache)];
#if CONFIG_NEW_CCC_LOG_ENABLE
/********************* RTC Time Initialization *********************/
// Helper: Convert date/time to seconds since 2026-01-01
static uint32_t datetime_to_seconds(uint16_t year, uint8_t month, uint8_t day,
                                     uint8_t hour, uint8_t minute, uint8_t second) {
    // If year < 2026, return 0 (start from 2026)
    if (year < 2026) {
        return 0;
    }

    // Calculate days since 2026-01-01
    uint32_t total_days = 0;

    // Add days for complete years from 2026 to (year-1)
    for (uint16_t y = 2026; y < year; y++) {
        total_days += (y % 4 == 0) ? 366 : 365;
    }

    // Add days for complete months in current year
    static const uint8_t month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (uint8_t m = 1; m < month; m++) {
        total_days += month_days[m - 1];
        if (m == 2 && (year % 4 == 0)) {
            total_days += 1;  // Leap year February
        }
    }

    // Add days in current month (day - 1 because we count from day 1)
    total_days += (day - 1);

    // Convert to seconds and add time
    uint32_t total_seconds = total_days * 86400UL +
                             (uint32_t)hour * 3600 +
                             (uint32_t)minute * 60 +
                             second;

    return total_seconds;
}

// Get default RTC seconds based on config
static uint32_t get_default_rtc_seconds(void) {
#if (CONFIG_RTC_USE_CUSTOM_TIME == 1)
    // Use custom fixed time from config.h
    return datetime_to_seconds(CONFIG_RTC_DEFAULT_YEAR,
                               CONFIG_RTC_DEFAULT_MONTH,
                               CONFIG_RTC_DEFAULT_DAY,
                               CONFIG_RTC_DEFAULT_HOUR,
                               CONFIG_RTC_DEFAULT_MINUTE,
                               CONFIG_RTC_DEFAULT_SECOND);
#endif
}
#endif

#if CYCLE_COUNT_FLASH_PERSIST
#include "fmc.h"
void cycle_count_save_to_flash(void)
{
    /* Preserve Q/F and battery energy while updating OV forbid, cycle count, and Vref. */
    uint32_t cfg[7];
    for (uint8_t i = 0; i < 7; i++)
        cfg[i] = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + i * 4);
#if OV_FORBID_FLASH_PERSIST
    cfg[4] = gd->bat_ov_forbid_flag ? (uint32_t)1 : (uint32_t)0xFFFFFFFF;  /* offset+16 = OV_FORBID */
#endif
    cfg[5] = (uint32_t)GET_CYCLE_COUNT(gd);            /* offset+20 = cycle count (16-bit) */
    extern uint16_t g_vref_mv;
    cfg[6] = (uint32_t)g_vref_mv;                      /* offset+24 = Vref (mV) */
    hal_fmc_erase_page(AP_CFG_ROM_ADDR_BASE);
    for (uint8_t i = 0; i < 7; i++)
        hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE + i * 4, switch_big_little_endian(cfg[i]));
}
#endif

uint8_t power_on_cnt = 0;
void ap_data_init(void)
{
	uint32_t i;
	uint32_t *pdest0 = (uint32_t *)(AP_CFG_ROM_ADDR_BASE);
	uint32_t *pdest1 = (uint32_t *)(AP_CFG_ROM_ADDR_BASE + 4);

	/* Save exception_cache before Flash→RAM copy (would be overwritten if offset < 256) */
	osal_mem_copy(saved_exception_cache, &(ap->exception_cache), sizeof(ap->exception_cache));

	for (i=0; i<256; i++)
	{
		__write_08bits(AP_CFG_RAM_ADDR_BASE + i, __read_08bits(AP_CFG_ROM_ADDR_BASE + i));
	}

	/* Restore exception_cache (overwritten by flash->RAM copy above) */
	osal_mem_copy(&(ap->exception_cache), saved_exception_cache, sizeof(ap->exception_cache));

	ap->tntc_otp_dis = 0;//0
	ap->tntc_otp_thd = 85;//80
	ap->tntc_otp_hys = 30;

	ap->tntc_utp_dis = 0;
	ap->tntc_utp_thd = -5;
	ap->tntc_utp_hys = 5;

	ap->tdie_otp_dis = 0;
	ap->tdie_otp_thd = 100;
	ap->tdie_otp_hys = 30;

	ap->tdie_utp_dis = 0;
	ap->tdie_utp_thd = -30;
	ap->tdie_utp_hys = 30;

	ap->isns_ocp_dis = 0;
	ap->isns_ocp_thd = 2500;
	ap->isns_ocp_hys = 1000;

	ap->vbus_ovp_dis = 0;
	ap->vbus_ovp_thd = 16000;//10500
	ap->vbus_ovp_hys = 1000;

	ap->vbus_uvp_dis = 0;
	ap->vbus_uvp_thd = 7800;//8000;
	ap->vbus_uvp_hys = 500;

	ap->vbus_dpl_dis = 0;
	ap->vbus_dpl_thd = 4500;
	ap->vbus_dpl_hys = 200;

	ap->vpwr_ovp_dis = 0;
	ap->vpwr_ovp_thd = 20500;
	ap->vpwr_ovp_hys = 1000;

	ap->pout_opp_dis = 0;
	ap->pout_opp_thd = 35000;//25000
	ap->pout_opp_hys = 5000;


	ap->ptmc = 0x01D1;
	ap->mpp_dither_en = 1;
	ap->auth_seic_type = 0; //0-fm1210, 1-t91206, 2-ciu98

	ap->pin_max_cnt = 10;
	ap->pin_fod_cnt = 30;//250;//10;//0xff;
	ap->pin_fod_dis = 0;

	ap->rpp_fod_cnt = 5;
	ap->rpp_fod_dis = 0;

	ap->dig_ping_volt_5v = 5000;
	ap->dig_ping_perd_5v = 1127;
	ap->dig_ping_duty_5v = 500;
	ap->dig_ping_phas_5v = 0;

	ap->dig_ping_volt_6v = 5600;
	ap->dig_ping_perd_6v = 1127;;//144000000 / 127772;
	ap->dig_ping_duty_6v = 450;
	ap->dig_ping_phas_6v = 0;

	ap->dig_ping_volt_9v = 9000;
	ap->dig_ping_perd_9v = 1127;
	ap->dig_ping_duty_9v = 250;
	ap->dig_ping_phas_9v = 0;

	ap->dig_ping_volt_11v = 11000;//12000;
	ap->dig_ping_perd_11v = 1127;//144000000 / 127772;
	ap->dig_ping_duty_11v = 250;//200;
	ap->dig_ping_phas_11v = 0;

	if ((*pdest0 < 0) || (*pdest0 > 500))
	{
		ap->q_factor_base_value = 226;
	}
	else
	{
		ap->q_factor_base_value = *pdest0;
	}
	ap->q_factor_reco_value =  20;
	ap->q_factor_limH_value = 500;
	ap->q_factor_limL_value =  	0;

	if ((*pdest0 < 0) || (*pdest0 > 1500))
	{
		ap->fs_base_value = 953;
	}
	else
	{
		ap->fs_base_value = *pdest1;
	}
	ap->fs_reco_value = 30;
	ap->fs_limH_value = 3000;
	ap->fs_limL_value = 10;

	ap->q_factor_obj_value = 25;//50
	ap->q_factor_stable_value = 30;
	ap->fs_obj_value = 35;//30;//50
	ap->fs_stable_value = 30;

	ap->t_next_ping = 100;
	gdata_printk("\r\n base_q [%d]", ap->q_factor_base_value);
	gdata_printk("\r\n base_fre [%d]", ap->fs_base_value);

#if CONFIG_NEW_CCC_LOG_ENABLE
	// Print product information - read directly from Flash, no global RAM usage
	product_info_print();
#endif
}

bool tc_power_on = false;

void gd_data_init(void)
{
	uint32_t addr;
#if BUCKBOOST_USED_NU6801
	gd->charger_is_6801_flag = 1;
#else
	gd->charger_is_6801_flag = 0;
#endif
	//for (addr=G_DATA_RAM_ADDR_BASE; addr<G_DATA_RAM_ADDR_BASE + sizeof(struct gd_t); addr++)
	for (addr=G_DATA_RAM_ADDR_BASE; addr< (uint32_t)(&(gd->resverd_reset)); addr++)
	{
		__write_08bits(addr, 0);
	}
    gdata_printk(" \r\n magic code %x",gd->power_on_magic);
	if(gd->power_on_magic != 0xaaaa)
	{
		gd->tc0_lighting_mode = 0x00;
		gd->tc1_lighting_mode = 0x00;
		gd->bat_dead_flag_with_snk0 = 0;
		gd->bat_dead_flag_with_snk1 = 0;
		gd->wpc_disable = 0x00;
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
		gd->usb_comm_activated = 0;
#endif
		gd->real_soc_show = 0;
		gd->real_soc_obtained = 0;
		gd->bat_dead_flag = 0;
		gd->SOC_RawSOC_mpct = 0;
		gd->SOC_SleepTime_s = 2000;
		gd->ship_sleep_start_seconds = 0;
		tc_power_on = true;
		gd ->ship_mode_cnt = 0;
		gd->led_fault = 0;
		gd->led_fault1 = 0;
		gd->ntc_led_off = 0;
		gd->recharge_flag = 0;
		gd->force_usb_mode = 0;
		gd->bat_ntc_wpc_dischg_reduce_flag = 0;
		gd->bat_ntc_cport_dischg_reduce_flag = 0;
		gd->ntc_total_lock_flag =0;
		gd->flash_times = 0;
		gd->typec_scp = 0;
		gd->vbus_ovp = 0;
		gd->touch_to_weakup = 0;
		power_on_cnt = 40;
		gd->Battery_charger_cnt = 0;
		gd->Battery_cycle_count = 0;
		gd->Battery_cycle_count_hi = 0;
		gd->exception_sleep_counter = 0;
		gd->eng_mode_active = 0;
		gd->eng_virtual_cell1 = 0xFFFF;
		gd->eng_virtual_cell2 = 0xFFFF;
		gd->eng_virtual_temp = 0x7FFF;
#if CYCLE_COUNT_FLASH_PERSIST
		/* Restore cycle count from Flash */
		{
			uint32_t flash_cycle = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + CYCLE_COUNT_FLASH_OFFSET);
			if (flash_cycle != 0xFFFFFFFF && flash_cycle <= 65535) {
				SET_CYCLE_COUNT(gd, (uint16_t)flash_cycle);
				xgb_printk("\r\n[CYCLE] Restored from Flash: %d", (int)flash_cycle);
			}
		}
#endif
#if OV_FORBID_FLASH_PERSIST
  #if OV_FORBID_FORCE_CLEAR
		/* Debug: erase forbid flag from Flash, preserve other fields */
		{
			uint32_t cfg[7];
			for (uint8_t i = 0; i < 7; i++)
				cfg[i] = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + i * 4);
			cfg[4] = 0xFFFFFFFF;  /* clear OV_FORBID (+16) */
			hal_fmc_erase_page(AP_CFG_ROM_ADDR_BASE);
			for (uint8_t i = 0; i < 7; i++)
				hal_fmc_write_word(AP_CFG_ROM_ADDR_BASE + i * 4, switch_big_little_endian(cfg[i]));
		}
		gd->bat_ov_forbid_flag = 0;
		xgb_printk("\r\n[OV_FORBID] Force cleared");
  #else
		/* Normal: restore forbid flag from Flash */
		{
			uint32_t flash_val = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + 16);
			if (flash_val == 0xFFFFFFFF) {
				gd->bat_ov_forbid_flag = 0;  /* Flash erased = never triggered */
			} else {
				gd->bat_ov_forbid_flag = 1;  /* Has value = previously triggered */
				xgb_printk("\r\n[OV_FORBID] Restored from Flash! Charge/discharge forbidden.");
			}
		}
  #endif
#else
		gd->bat_ov_forbid_flag = 0;
#endif
		gd-> enter_sleep_flag = (uint32_t *)(AP_CFG_ROM_ADDR_BASE + 32);
		gd->Bat_Rdc = 0;
		gd->Bat_SoH = 0;
		gd->Bat_RTC_Timer = 0;
		gd->wpc_sleepship = 0;
		gd->enter_sleep_flag = 0;
		osal_mem_set(&(gd->g_bat), 0, sizeof(struct bat_info));
		g_forbid_bypass_flag = 0;
		osal_mem_set((&g_bat),0,sizeof(struct bat_info));
#if CONFIG_NEW_CCC_LOG_ENABLE
		gd->Bat_RTC_Seconds = get_default_rtc_seconds();  // Initialize with default time
		gd->Bat_RTC_Milliseconds = 0;

		gdata_printk("\r\n ------------------------------------------------------------poweron reset");
#if (CONFIG_RTC_USE_CUSTOM_TIME == 1)
		xgb_printk("\r\n RTC init: %lu seconds (custom: %d-%02d-%02d %02d:%02d:%02d)",
		       gd->Bat_RTC_Seconds,
		       CONFIG_RTC_DEFAULT_YEAR, CONFIG_RTC_DEFAULT_MONTH, CONFIG_RTC_DEFAULT_DAY,
		       CONFIG_RTC_DEFAULT_HOUR, CONFIG_RTC_DEFAULT_MINUTE, CONFIG_RTC_DEFAULT_SECOND);
#endif
#else
		gdata_printk("\r\n ------------------------------------------------------------poweron reset");
#endif
	}

#if Cali_Vref
	{
		extern uint16_t g_vref_mv;
		uint32_t flash_vref = *(uint32_t *)(AP_CFG_ROM_ADDR_BASE + VREF_FLASH_OFFSET);
		if (flash_vref != 0xFFFFFFFF && flash_vref >= 3240 && flash_vref <= 3300) {
			g_vref_mv = (uint16_t)flash_vref;
			xgb_printk("\r\n[VREF] Restored from Flash: %dmV", g_vref_mv);
		} else {
			g_vref_mv = VREF_DEFAULT_MV;
			xgb_printk("\r\n[VREF] No Flash cal, default %dmV", VREF_DEFAULT_MV);
		}
	}
#endif

	osal_mem_copy(&(g_bat),(const void *)&(gd->g_bat),sizeof(struct bat_info));
	g_bat.bat_soe_in_cali = false;
	gd->key_sleep_exit = 0;
	gd->power_on_magic = 0xaaaa;

	gdata_printk("\r\n light [%d %d]", gd->tc0_lighting_mode,gd->tc1_lighting_mode);

	gd->tx_infos.t_next_ping = ap->t_next_ping;
//	gd->tx_infos.fo_exist = 1;
	gd->tx_infos.fo_exist = 0;
}

void lib_para_init(void)
{
	#if(CONFIG_TYPECA_SUPPORT == 1)
		lib_para.typec_a_support = 1;
	#else
		lib_para.typec_a_support = 0;
	#endif

	#if(CONFIG_TYPECB_SUPPORT == 1)
		lib_para.typec_b_support = 1;
	#else
		lib_para.typec_b_support = 0;
	#endif

	#if(CONFIG_UFCS_SOURCE_SUPPORT == 1)
		lib_para.ufcs_source_support = 1;
	#else
		lib_para.ufcs_source_support = 0;
	#endif

	#if(CONFIG_AFC_SOURCE_SUPPORT == 1)
		lib_para.afc_source_support = 1;
	#else
		lib_para.afc_source_support = 0;
	#endif

	#if(CONFIG_FCP_SOURCE_SUPPORT == 1)
		lib_para.fcp_source_support = 1;
	#else
		lib_para.fcp_source_support = 0;
	#endif

	#if(CONFIG_SCP_SOURCE_SUPPORT == 1)
		lib_para.scp_source_support = 1;
	#else
		lib_para.scp_source_support = 0;
	#endif
		dead_battery_voltage = CONFIG_NU6801_BATLOW_VOLT;
}
#if CONFIG_NEW_CCC_LOG_ENABLE
/********************* Product Information Functions *********************/

/**
 * @brief Read product information from Flash to RAM
 * @param info Receive buffer
 */
void product_info_read(ProductInfo_t *info) {
	uint16_t i;
	uint8_t *dst_data;

	if (info == NULL) return;

	dst_data = (uint8_t *)info;
	for (i = 0; i < sizeof(ProductInfo_t); i++) {
		dst_data[i] = __read_08bits(AP_CFG_ROM_ADDR_PRO_INFO + i);
	}
}

void product_info_write(const ProductInfo_t *info) {
	uint16_t i;
	const uint8_t *src_data;
	uint16_t write_size;

	if (info == NULL) return;

	VIC_vModuleDisable();

	hal_fmc_erase_page(AP_CFG_ROM_ADDR_PRO_INFO);

	src_data = (const uint8_t *)info;
	write_size = (sizeof(ProductInfo_t) + 3) & ~3u;

	for (i = 0; i < write_size; i += 4) {
		uint32_t word;
		uint8_t b0 = (i     < sizeof(ProductInfo_t)) ? src_data[i]     : 0;
		uint8_t b1 = (i + 1 < sizeof(ProductInfo_t)) ? src_data[i + 1] : 0;
		uint8_t b2 = (i + 2 < sizeof(ProductInfo_t)) ? src_data[i + 2] : 0;
		uint8_t b3 = (i + 3 < sizeof(ProductInfo_t)) ? src_data[i + 3] : 0;
		word = ((uint32_t)b0 << 24) | ((uint32_t)b1 << 16) |
		       ((uint32_t)b2 << 8) | (uint32_t)b3;
		hal_fmc_write_word(AP_CFG_ROM_ADDR_PRO_INFO + i, word);
	}

	hal_fmc_write_word(ADDR_PRODUCT_INFO_VERSION, PRODUCT_INFO_VERSION);

	VIC_vModuleEnable();
}

void product_info_print(void) {
	ProductInfo_t info;
	char temp_buf[PRODUCT_SERIAL_FIELD_SIZE + 1];
	uint8_t i;

	static const struct {
		uint16_t offset;
		uint8_t len;
		const char *label;
	} fields[] = {
		{ 0,   PRODUCT_INFO_FIELD_SIZE,     "Manufacturer1" },
		{ 20,  PRODUCT_INFO_FIELD_SIZE,     "Manufacturer2" },
		{ 40,  PRODUCT_INFO_FIELD_SIZE,     "Model" },
		{ 60,  PRODUCT_SERIAL_FIELD_SIZE,   "Product SN" },
		{ 92,  PRODUCT_INFO_FIELD_SIZE,     "Battery MFR" },
		{ 112, PRODUCT_INFO_FIELD_SIZE,     "Battery Model" },
		{ 132, PRODUCT_INFO_FIELD_SIZE,     "Battery Date" },
		{ 152, SERIAL_FIELD_SIZE,           "Cell SN1" },
		{ 172, SERIAL_FIELD_SIZE,           "Cell SN2" },
		{ 192, PRODUCT_CHECKSUM_FIELD_SIZE, "NU171X CRC" },
		{ 196, PRODUCT_CHECKSUM_FIELD_SIZE, "WB7720 CRC" }
	};

	product_info_read(&info);
	xgb_printk("\r\n===== Product Information =====");

	for (i = 0; i < sizeof(fields) / sizeof(fields[0]); i++) {
		uint8_t j;
		const uint8_t *data = ((const uint8_t *)&info) + fields[i].offset;
		for (j = 0; j < fields[i].len; j++) {
			temp_buf[j] = data[j];
		}
		temp_buf[fields[i].len] = '\0';
		xgb_printk("\r\n%-13s: %s", fields[i].label, temp_buf);
	}

	xgb_printk("\r\n===============================");
}
#endif
