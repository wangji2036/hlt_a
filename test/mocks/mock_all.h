/*
 * mock_all.h - Comprehensive mock header for bat_record unit tests
 *
 * MUST be included FIRST, before bat_record.c and bat_record.h.
 *
 * 64-bit compatibility strategy for Flash simulation:
 * bat_record.c uses uint32_t for all Flash addresses and dereferences them
 * as raw pointers: *(uint32_t*)addr and *(uint8_t*)(addr + i).
 * On 64-bit Windows, static arrays have addresses > 0xFFFFFFFF, so casting
 * through uint32_t loses the upper bits and causes SEGFAULT.
 *
 * Solution: Use Windows VirtualAlloc() to allocate flash_mem at a fixed
 * address in the lower 4GB (e.g. 0x00010000). This address fits in uint32_t
 * and the pointer round-trip works correctly.
 *
 * AP_CFG_ROM_ADDR_LOG is then set to this fixed low address, so that
 * flash_read_u32(AP_CFG_ROM_ADDR_LOG) correctly dereferences flash_mem[0..3].
 *
 * Endian note: switch_big_little_endian() is a no-op on x86 (identity fn),
 * so magic value comparisons work correctly on the little-endian test host.
 */

#ifndef MOCK_ALL_H_
#define MOCK_ALL_H_

/* ------------------------------------------------------------------ */
/* Standard C headers - basic types                                     */
/* ------------------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef _WIN32
#include <windows.h>
#endif

/* ------------------------------------------------------------------ */
/* Include guards for MCU headers - these prevent stub headers from     */
/* re-including content after the guards are set in mock_all.h         */
/* ------------------------------------------------------------------ */
#define REGDEF_H_
#define TYPDEF_H_
#define NTC_H_
#define BUCK_BOOST_H_
#define _FML_H_
#define FMC_H_
#define APP_H_
#define PRINTK_H_
#define NU6805_H_
#define OSAL_H_
#define NU103X_H_
#define ADP_H_
#define ASK_H_
#define FSK_H_
#define CONFIG_H_
#define G_DATA_H_

/* ------------------------------------------------------------------ */
/* Config macros (normally from config.h)                               */
/* ------------------------------------------------------------------ */
#define CONFIG_NEW_CCC_LOG_ENABLE           1
#define BUCKBOOST_USED_NU6805               1
#define BUCKBOOST_USED_NU6801               0
#define OVER_VOLTAGE_THRESHOLD              4450
#define CHRG_NTC_OT_TEMP_VALUE             600
#define DISCHG_NTC_OT_TEMP_VALUE           650
#define CHRG_NTC_OT_VALUE                   50
#define BATTERY_CV_VALUE                    4400

/* ------------------------------------------------------------------ */
/* BuckBoost mode enum (normally from buckboost.h)                      */
/* ------------------------------------------------------------------ */
typedef enum {
    BUCKBOOST_SHUTDOWM_MODE = 0,
    BUCKBOOST_CHAGER_MODE   = 1,
    BUCKBOOST_DISCHG_MODE   = 2
} buckboost_mode_t;

/* ------------------------------------------------------------------ */
/* Flash simulation using VirtualAlloc at a fixed low 32-bit address   */
/*                                                                      */
/* We allocate 0x800 bytes at address 0x00010000 (lower 4GB).          */
/* AP_CFG_ROM_ADDR_LOG = 0x00010000 (our VirtualAlloc base).           */
/* All bat_record.c address arithmetic:                                 */
/*   FLASH_LOG_BASE = AP_CFG_ROM_ADDR_LOG = 0x00010000                 */
/*   ADDR_MAGIC = FLASH_LOG_BASE + 0 = 0x00010000                      */
/*   flash_read_u32(0x00010000) -> *(uint32_t*)0x00010000 -> flash_mem[0..3] */
/* ------------------------------------------------------------------ */

/* Fixed low 32-bit address for Flash simulation (first attempt) */
#define FLASH_VIRT_BASE     0x00010000UL
#define FLASH_VIRT_SIZE     0x800

/* AP_CFG_ROM_ADDR_LOG overrides the real MCU address 0x1400.
 * After VirtualAlloc, flash_mem_ptr points to the allocated region.
 * This macro returns the uint32_t-truncated address of flash_mem_ptr.
 * Since flash_mem_ptr is in the lower 4GB (VirtualAlloc guarantee),
 * the truncation is lossless and pointer dereferences in bat_record.c
 * (*(uint32_t*)addr) work correctly on 64-bit Windows.
 */
#undef  AP_CFG_ROM_ADDR_LOG
#define AP_CFG_ROM_ADDR_LOG     ((uint32_t)(uintptr_t)flash_mem_ptr)

/* Other platform addresses (not used by bat_record but must be defined) */
#define AP_CFG_ROM_ADDR_BASE     0x1600UL
#define AP_CFG_ROM_ADDR_PRO_INFO 0x1800UL
#define AP_CFG_RAM_ADDR_BASE     0x20000000UL
#define G_DATA_RAM_ADDR_BASE     0x20000200UL

/* Pointer to the VirtualAlloc'd flash memory region */
static uint8_t *flash_mem_ptr = NULL;

/* flash_mem: convenience alias - points to flash_mem_ptr */
/* We use a macro so reset_test_state() can use flash_mem directly */
#define flash_mem flash_mem_ptr

/* Initialize the Flash VirtualAlloc region - call once at program start */
static void flash_sim_init(void) {
    if (flash_mem_ptr != NULL) return;  /* Already initialized */
#ifdef _WIN32
    flash_mem_ptr = (uint8_t*)VirtualAlloc(
        (LPVOID)(uintptr_t)FLASH_VIRT_BASE,
        FLASH_VIRT_SIZE,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);
    if (flash_mem_ptr == NULL) {
        /* Address may be in use, try without specific address hint */
        /* Fallback: allocate anywhere and use a patch table */
        fprintf(stderr, "ERROR: VirtualAlloc at 0x%08lX failed: %lu\n",
                FLASH_VIRT_BASE, GetLastError());
        /* Try a range of low addresses */
        for (uint32_t attempt = 0x10000; attempt < 0x10000000; attempt += 0x10000) {
            flash_mem_ptr = (uint8_t*)VirtualAlloc(
                (LPVOID)(uintptr_t)attempt,
                FLASH_VIRT_SIZE,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);
            if (flash_mem_ptr != NULL) {
                fprintf(stderr, "Flash simulation at: %p\n", (void*)flash_mem_ptr);
                break;
            }
        }
        if (flash_mem_ptr == NULL) {
            fprintf(stderr, "FATAL: Cannot allocate flash simulation memory\n");
            exit(1);
        }
    }
#else
    /* Non-Windows: use mmap at fixed address */
    #include <sys/mman.h>
    flash_mem_ptr = (uint8_t*)mmap(
        (void*)(uintptr_t)FLASH_VIRT_BASE,
        FLASH_VIRT_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE,
        -1, 0);
    if (flash_mem_ptr == MAP_FAILED) {
        fprintf(stderr, "FATAL: mmap at 0x%lX failed\n", (unsigned long)FLASH_VIRT_BASE);
        exit(1);
    }
#endif
    memset(flash_mem_ptr, 0xFF, FLASH_VIRT_SIZE);
}

/* For tests to use: erase (reinitialize) flash to 0xFF */
static void flash_sim_erase_all(void) {
    if (flash_mem_ptr) memset(flash_mem_ptr, 0xFF, FLASH_VIRT_SIZE);
}

/* Verify our VirtualAlloc address is usable via uint32_t cast */
static void flash_sim_verify(void) {
    uint32_t addr32 = (uint32_t)(uintptr_t)flash_mem_ptr;
    uint8_t *back_ptr = (uint8_t*)(uintptr_t)addr32;
    if (back_ptr != flash_mem_ptr) {
        fprintf(stderr, "FATAL: flash_mem address 0x%p does not fit in uint32_t\n",
                (void*)flash_mem_ptr);
        fprintf(stderr, "  uint32_t truncated to: 0x%08X\n", addr32);
        exit(1);
    }
    /* Update AP_CFG_ROM_ADDR_LOG to the actual allocated address */
    /* (This is done via the mock override of AP_CFG_ROM_ADDR_LOG) */
}

/* ------------------------------------------------------------------ */
/* Flash HAL mocks: use offset from flash_mem_ptr                       */
/* ------------------------------------------------------------------ */

/* Erase 512 bytes at MCU flash address - fills with 0xFF */
static inline void hal_fmc_erase_page(uint32_t addr) {
    if (flash_mem_ptr == NULL) return;
    uint32_t offset = addr - (uint32_t)(uintptr_t)flash_mem_ptr;
    if (offset + 512 <= FLASH_VIRT_SIZE) {
        memset(flash_mem_ptr + offset, 0xFF, 512);
    }
}

/* Write 4-byte word at MCU flash address - little-endian on x86 */
static inline void hal_fmc_write_word(uint32_t addr, uint32_t data) {
    if (flash_mem_ptr == NULL) return;
    uint32_t offset = addr - (uint32_t)(uintptr_t)flash_mem_ptr;
    if (offset + 4 <= FLASH_VIRT_SIZE) {
        flash_mem_ptr[offset + 0] = (uint8_t)(data & 0xFF);
        flash_mem_ptr[offset + 1] = (uint8_t)((data >> 8) & 0xFF);
        flash_mem_ptr[offset + 2] = (uint8_t)((data >> 16) & 0xFF);
        flash_mem_ptr[offset + 3] = (uint8_t)((data >> 24) & 0xFF);
    }
}

/* ------------------------------------------------------------------ */
/* Endian swap - IDENTITY on x86 test host                             */
/* CK802 is big-endian. The real MCU code calls switch_big_little_endian
 * before writing to Flash. On x86 (little-endian), making this a no-op
 * means data is stored and read back in the same byte order, so all
 * comparisons (e.g. magic == MAGIC_VALUE) work correctly on the host.
 * ------------------------------------------------------------------ */
static inline uint32_t switch_big_little_endian(uint32_t x) {
    return x;  /* Identity no-op: preserves endianness for x86 tests */
}

/* ------------------------------------------------------------------ */
/* Interrupt control stubs (no-op on host)                              */
/* ------------------------------------------------------------------ */
static inline void VIC_vModuleDisable(void) {}
static inline void VIC_vModuleEnable(void)  {}

/* ------------------------------------------------------------------ */
/* printk stub - suppress all firmware debug output during tests        */
/* ------------------------------------------------------------------ */
#define printk(fmt, ...) do { (void)(fmt); } while(0)

/* ------------------------------------------------------------------ */
/* NTC temperature conversion spy                                        */
/* The BUG to detect: current bat_record.c uses gd->sys_infos.ntc_temp_wpc
 * instead of calling ntc_to_temp(ntc_resistance).
 * We spy on ntc_to_temp() to detect whether it was called.
 * ------------------------------------------------------------------ */
static int16_t  mock_ntc_to_temp_return     = 0;
static uint32_t mock_ntc_to_temp_call_count = 0;

static inline int16_t ntc_to_temp(uint16_t resistance) {
    (void)resistance;
    mock_ntc_to_temp_call_count++;
    return mock_ntc_to_temp_return;
}

/* ------------------------------------------------------------------ */
/* BuckBoost battery voltage mock (NU6805 path)                         */
/* ------------------------------------------------------------------ */
static uint16_t mock_bat_voltage = 0;

static inline uint16_t hal_nu6805_buckboost_get_bat_voltage(void) {
    return mock_bat_voltage;
}

/* ------------------------------------------------------------------ */
/* g_buckboost mock struct                                              */
/* bat_record.c accesses:
 *   g_buckboost.adc_vbat    (NU6801 path)
 *   g_buckboost.adc_tbat1   (NTC ADC resistance value)
 *   g_buckboost.woke_mode   (charge/discharge mode)
 * ------------------------------------------------------------------ */
static struct {
    uint16_t adc_vbat;
    uint16_t adc_tbat1;
    uint8_t  woke_mode;
    uint8_t  _pad[3];
} g_buckboost;

/* ------------------------------------------------------------------ */
/* CCC data structures (normally from g_data.h)                         */
/* ------------------------------------------------------------------ */
#define MAX_RECORDS     5

/* ExceptionCache_t.status_flags bit layout:
 * bit[0]: cell1_tracking, bit[1]: cell2_tracking, bit[2]: temp_tracking
 * bit[3-5]: temp_event_type, bit[6-7]: charge_state
 */
#define CACHE_GET_CELL1_TRACKING(cache)     ((cache)->status_flags & 0x01)
#define CACHE_SET_CELL1_TRACKING(cache, v)  do { \
    if (v) (cache)->status_flags |= 0x01; \
    else   (cache)->status_flags &= ~0x01; \
} while(0)

#define CACHE_GET_CELL2_TRACKING(cache)     (((cache)->status_flags >> 1) & 0x01)
#define CACHE_SET_CELL2_TRACKING(cache, v)  do { \
    if (v) (cache)->status_flags |= 0x02; \
    else   (cache)->status_flags &= ~0x02; \
} while(0)

#define CACHE_GET_TEMP_TRACKING(cache)      (((cache)->status_flags >> 2) & 0x01)
#define CACHE_SET_TEMP_TRACKING(cache, v)   do { \
    if (v) (cache)->status_flags |= 0x04; \
    else   (cache)->status_flags &= ~0x04; \
} while(0)

#define CACHE_GET_TEMP_EVENT_TYPE(cache)    (((cache)->status_flags >> 3) & 0x07)
#define CACHE_SET_TEMP_EVENT_TYPE(cache, v) do { \
    (cache)->status_flags = ((cache)->status_flags & 0xC7) | (((v) & 0x07) << 3); \
} while(0)

#define CACHE_GET_CHARGE_STATE(cache)       (((cache)->status_flags >> 6) & 0x03)
#define CACHE_SET_CHARGE_STATE(cache, v)    do { \
    (cache)->status_flags = ((cache)->status_flags & 0x3F) | (((v) & 0x03) << 6); \
} while(0)

/* TimeStamp_t - 8 bytes */
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  reserved;
} TimeStamp_t;

/* BatteryExceptionRecord_t - 20 bytes */
typedef struct {
    TimeStamp_t timestamp;      /* 8B */
    uint8_t  error_type;        /* 1B */
    uint8_t  sub_type;          /* 1B */
    union {
        struct {
            uint16_t max_voltage;
            uint16_t total_voltage;
        } ov_data;
        struct {
            int16_t  max_temperature;
            uint16_t reserved;
        } temp_data;
        uint8_t raw_data[6];
    } data;                     /* 6B */
    uint32_t record_id;         /* 4B */
} BatteryExceptionRecord_t;     /* total: 20B */

/* ExceptionCache_t - ~16 bytes */
typedef struct {
    uint8_t  status_flags;
    uint8_t  padding1;
    uint16_t cell1_max_voltage;
    uint16_t cell2_max_voltage;
    int16_t  max_temperature;
    uint32_t cell1_hour_start_seconds;   /* OV 1-hour window start for Cell1 */
    uint32_t cell2_hour_start_seconds;   /* OV 1-hour window start for Cell2 */
    uint32_t temp_hour_start_seconds;
} ExceptionCache_t;

/* BatteryRecordStorage_t - ~110 bytes */
typedef struct {
    uint32_t magic;
    uint8_t  exception_counter;
    uint8_t  write_ptr;
    uint16_t padding1;
    BatteryExceptionRecord_t records[MAX_RECORDS];
    uint16_t checksum;
} BatteryRecordStorage_t;

/* ProductInfo_t */
#define PRODUCT_INFO_FIELD_SIZE  20
typedef struct {
    char manufacturer_name[PRODUCT_INFO_FIELD_SIZE];
    char model_name[PRODUCT_INFO_FIELD_SIZE];
    char battery_mfr[PRODUCT_INFO_FIELD_SIZE];
    char battery_model[PRODUCT_INFO_FIELD_SIZE];
    char battery_prod_date[PRODUCT_INFO_FIELD_SIZE];
} ProductInfo_t;

/* ------------------------------------------------------------------ */
/* Minimal stub types required by g_data.h transitive includes          */
/* ------------------------------------------------------------------ */
union nu103x_t { uint32_t raw; };
struct adp_t   { uint8_t _pad; };
struct fsk_cfg_t { uint8_t _pad; };
struct ask_packet_t { uint8_t _pad; };

/* ------------------------------------------------------------------ */
/* gd_t struct - matches real g_data.h                                  */
/* bat_record.c uses:
 *   gd->Bat_RTC_Timer          (uint64_t milliseconds)
 *   gd->sys_infos.ntc_temp_wpc (int16_t, sentinel for BUG test)
 * ------------------------------------------------------------------ */
struct gd_t {
    struct {
        uint8_t plat_info_0, plat_info_1, plat_info_2, plat_info_3;
    } plt_infos;

    struct { uint32_t pd_pdo[7]; } usb_infos;

    uint16_t vbus, vpwr, isns, vpwr_avg, isns_avg, isns_pre, icol_max, icol_rms;
    uint8_t  power_mode, ctx_ind;
    uint16_t ctx;
    uint32_t tx_power, rx_power, rx_prect, p_rect_max_ntc_ot;
    uint16_t vctx_pp;
    uint32_t k_est;

    union nu103x_t nu103x_sts_last, nu103x_sts_curr;

    uint16_t dig_ping_volt, dig_ping_perd;
    uint8_t  force_usb_mode;
    uint16_t dig_ping_duty, dig_ping_phas;
    uint16_t pid_volt, pid_perd, pid_duty, pid_phas;

    struct {
        uint8_t  tim3_evnt;
        uint8_t  led_status;
        int16_t  die_temp;
        int16_t  ntc_temp_wpc;    /* <-- bat_record.c BUG reads this */
        int16_t  ntc_temp_typec;
    } sys_infos;

    struct { uint8_t is_dither_en, is_dig_ddm_en; } sys_status;

    struct { uint8_t _pad[128]; } tx_infos;  /* Abbreviated */

    uint8_t pla_id;

    struct {
        uint8_t  cmt, success, index, index_cnt;
        uint16_t preceived, prect, vrect, irect;
    } dploss_cal;

    struct {
        uint32_t tntc_otp_flag : 1, tntc_utp_flag : 1, tdie_otp_flag : 1,
                 tdie_utp_flag : 1, isns_ocp_flag : 1, vbus_ovp_flag : 1,
                 vbus_uvp_flag : 1, vbus_dpl_flag : 1, vpwr_ovp_flag : 1,
                 pout_opp_flag : 1, q_fod_flag : 1, xfer_fod_flag : 1,
                 _unused : 20;
    } prot_sts;

    struct {
        uint16_t volt_lim_hi, volt_lim_mi, volt_lim_lo;
        uint16_t perd_lim_hi, perd_lim_mi, perd_lim_lo;
        uint16_t duty_lim_hi, duty_lim_mi, duty_lim_lo;
        uint16_t phas_lim_hi, phas_lim_mi, phas_lim_lo;
    } pid_limit;

    struct {
        uint16_t fop_flag : 1, vbus_uv_flag : 1, tntc_ot_flag : 1,
                 vbus_ov_flag : 1, isns_oc_flag : 1, pout_op_flag : 1;
    } power_limit_sts;
    uint8_t tntc_ot_flag_atn;

    struct ask_packet_t wpc_pkt;
    struct adp_t        adp;
    uint8_t adp_type_upd;

    uint8_t wpc_idle_state, ptx_idle_phase_status, ptx_protocol_phase;
    uint8_t ptx_end_nego_event, sys_err_code;

    struct {
        uint8_t  power_profile_mode, mpp_restricted_mode, mpp_restricted_power_limit;
        uint8_t  ssp_value, qi_version, ref_q, ref_f, opt_cnt, neg, phase_state;
        uint8_t  epp_mode, max_power, max_power_temp, gant_power_temp, fsk_param;
        uint8_t  wnd_size, pch_t_delay, guaranteed_power, private_charge;
        int8_t   cep_val, cep_pre;
        uint8_t  cep_cnt, rx_type, chr_status, pla_type;
        uint16_t prmc;
        int16_t  gcoil_tx, alpha_fm, alpha_fm_dc, gcoil_tx2;
        int16_t  alpha_fm_itx, alpha_fm_irect, alpha_fm_vrect;
        uint16_t pla_prect, pla_vrect, pla_irect;
        uint32_t device_id;
        uint8_t  rsp_type, gant_power, ref_power;
        uint32_t stand_power;
        uint8_t  rpp_tick, rpp_rsp_type;
        uint16_t rpp, cali_light, cali_connect;
    } rx_infos;

    struct fsk_cfg_t fsk_cfg;
    uint8_t fsk_silence, dmo1_phase, dmo2_phase, nego_flag, ios_nego_cnt;
    uint8_t dig_ping_continuous_cnt, atl_test_tpr1c_coil_flag;
    uint8_t atl_test_ldstp_epp_N60, atl_test_ldstp_bpp_N60, atl_test_ldstp_bpp_P60;

    uint16_t recv_rpp_count, last_rpp_value;

    uint8_t  reset_magicode, idle_to_sleep_cnt;
    uint8_t  sleep_qdt_complete_charg_count, sleep_qdt_fod_rec_count;
    uint8_t  sleep_q_times, rd0_cnt, rd1_cnt, light0_cnt, light1_cnt;
    uint8_t  charger_is_6801_flag, renego_flag, soc_flag, q_standby_flag;
    uint8_t  resverd_reset;
    uint16_t power_on_magic;
    uint8_t  tc0_lighting_mode, tc1_lighting_mode, wpc_disable;
    uint8_t  real_soc_show, real_soc_obtained, dp_result;
    uint8_t  bat_dead_flag, bat_dead_flag_with_snk0, bat_dead_flag_with_snk1;
    uint8_t  Battery_cycle_count, Battery_charger_cnt, Bat_Rdc;
    int8_t   Bat_SoH;
    uint64_t Bat_RTC_Timer;    /* <-- bat_record.c reads this for RTC time */
    int32_t  SOC_RawSOC_mpct;
    uint32_t SOC_SleepTime_s;
    uint8_t  ship_mode_cnt, sigle_clicked;
    uint8_t  led_fault, led_fault1, ntc_led_off, recharge_flag;
    uint8_t  wirless_ntc_lock, bat_ntc_lock_flag, typec_ntc_lock;
    uint8_t  bat_ntc_dischg_reduce_flag, typec_charge_ntc_lock;
    uint8_t  flash_times, typec_scp, vbus_ovp, touch_to_weakup, flag11;
    uint32_t timer_cnt;
    uint8_t  fault_status;
};

/* ap_t struct - with Phase 3 fields appended at end */
struct ap_t {
    uint8_t  app_info_0, app_info_1, app_info_2, app_info_3;
    uint8_t  app_info_4, app_info_5, app_info_6, app_info_7;

    struct { uint8_t power_on, idle, charging, charged, error; } led_ctrl;

    uint8_t  mpp_dither_en, auth_seic_type;
    uint16_t ptmc, t_next_ping;

    uint8_t  tntc_otp_dis, tntc_utp_dis, tdie_otp_dis, tdie_utp_dis;
    uint8_t  isns_ocp_dis, icap_ocp_dis, vbus_ovp_dis, vbus_uvp_dis;
    uint8_t  vbus_dpl_dis, vpwr_ovp_dis, pout_opp_dis;

    uint16_t tdie_otp_thd, tdie_otp_hys, tdie_utp_thd, tdie_utp_hys;
    uint16_t tntc_otp_thd, tntc_otp_hys, tntc_utp_thd, tntc_utp_hys;
    uint16_t isns_ocp_thd, isns_ocp_hys, icap_ocp_thd, icap_ocp_hys;
    uint16_t vbus_ovp_thd, vbus_ovp_hys, vbus_uvp_thd, vbus_uvp_hys;
    uint16_t vbus_dpl_thd, vbus_dpl_hys, vpwr_ovp_thd, vpwr_ovp_hys;
    uint16_t pout_opp_thd, pout_opp_hys;

    uint16_t pid_volt_lim_hi, pid_volt_lim_mi, pid_volt_lim_lo;
    uint16_t pid_perd_lim_hi, pid_perd_lim_mi, pid_perd_lim_lo;
    uint16_t pid_duty_lim_hi, pid_duty_lim_mi, pid_duty_lim_lo;
    uint16_t pid_phas_lim_hi, pid_phas_lim_mi, pid_pahs_lim_lo;

    uint16_t dig_ping_volt_5v, dig_ping_perd_5v, dig_ping_duty_5v, dig_ping_phas_5v;
    uint16_t dig_ping_volt_6v, dig_ping_perd_6v, dig_ping_duty_6v, dig_ping_phas_6v;
    uint16_t dig_ping_volt_9v;
    uint32_t dig_ping_perd_9v;
    uint16_t dig_ping_duty_9v, dig_ping_phas_9v;
    uint16_t dig_ping_volt_11v;
    uint32_t dig_ping_perd_11v;
    uint16_t dig_ping_duty_11v, dig_ping_phas_11v;

    uint8_t  pin_max_cnt, pin_fod_dis, pin_fod_cnt, rpp_fod_dis, rpp_fod_cnt;

    uint16_t q_factor_base_value, q_factor_reco_value;
    uint16_t q_factor_limH_value, q_factor_limL_value;
    uint16_t q_factor_obj_value, q_factor_stable_value;

    uint32_t fs_base_value, fs_reco_value, fs_limH_value, fs_limL_value;
    uint16_t fs_obj_value, fs_stable_value, low_k_val;
    uint8_t  ddm_check_interval_long;

    /* Phase 3 fields: exception cache and record storage moved to ap_t */
    ExceptionCache_t       exception_cache;
    BatteryRecordStorage_t record_storage;
};

/* ------------------------------------------------------------------ */
/* Global instances and extern declarations                             */
/* ------------------------------------------------------------------ */
static struct gd_t gd_mock;
static struct ap_t ap_mock;

volatile struct gd_t *gd = (volatile struct gd_t *)&gd_mock;
volatile struct ap_t *ap = (volatile struct ap_t *)&ap_mock;

/* g_data.h declares dead_battery_voltage as a global */
uint16_t dead_battery_voltage = 0;

/* lib_para_sts and lib_para (declared in g_data.h) */
struct lib_para_sts {
    uint16_t typec_a_support    : 1;
    uint16_t typec_b_support    : 1;
    uint16_t ufcs_source_support: 1;
    uint16_t afc_source_support : 1;
    uint16_t fcp_source_support : 1;
    uint16_t scp_source_support : 1;
} lib_para;

/* g_data.h API stubs */
static inline void ap_data_init(void) {}
static inline void lib_para_init(void) {}
static inline void gd_data_init(void) {}

#if CONFIG_NEW_CCC_LOG_ENABLE
static inline void product_info_read(ProductInfo_t *info)        { (void)info; }
static inline void product_info_write(const ProductInfo_t *info) { (void)info; }
static inline void product_info_print(void)                      {}
#endif

#endif /* MOCK_ALL_H_ */
