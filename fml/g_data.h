#ifndef G_DATA_H_
#define G_DATA_H_

#include "typdef.h"
#include "nu103x.h"
#include "adp.h"
#include "ask.h"
#include "fsk.h"
#include "bat.h"
#include "config.h"

#if SUPPORT_GDATA_LOG
#define gdata_printk printk
#else
#define gdata_printk(...)
#endif

#define BAT_ADDR_BASE (0x00001900) /* legacy battery energy address */
//0~15ff  LDROM
//1400~15ff log
//1600~17FF Q/freq calibration value, gauge.etc.
// 1800~19ff Product information
#define AP_CFG_ROM_ADDR_LOG3 (0x00001000) // Optional third page
#define AP_CFG_ROM_ADDR_LOG2 (0x00001200)
#define AP_CFG_ROM_ADDR_LOG1 (0x00001400)
#define AP_CFG_ROM_ADDR_PRO_INFO (0x00001800)
#define AP_CFG_ROM_ADDR_BASE (0x00001600)
#define BAT_ENERGY_TOTAL_FLASH_OFFSET 8        /* AP_CFG_ROM_ADDR_BASE + 8 */
#define BAT_ENERGY_TOTAL_CHECK_FLASH_OFFSET 12 /* AP_CFG_ROM_ADDR_BASE + 12 */
#define AP_CFG_RAM_ADDR_BASE (0x20000000)
#define G_DATA_RAM_ADDR_BASE (0x20000200)
#if CONFIG_NEW_CCC_LOG_ENABLE
// Product information Flash address definitions
#define PRODUCT_INFO_FIELD_SIZE 20
#define SERIAL_FIELD_SIZE 20
#define PRODUCT_SERIAL_FIELD_SIZE 32
#define PRODUCT_CHECKSUM_FIELD_SIZE 4
#define PRODUCT_INFO_TOTAL_SIZE 200
#define PRODUCT_INFO_VERSION 0x0005
#define ADDR_MANUFACTURER_NAME1 (AP_CFG_ROM_ADDR_PRO_INFO + 0)
#define ADDR_MANUFACTURER_NAME2 (AP_CFG_ROM_ADDR_PRO_INFO + 20)
#define ADDR_MODEL_NAME (AP_CFG_ROM_ADDR_PRO_INFO + 40)
#define ADDR_PRODUCT_SERIAL (AP_CFG_ROM_ADDR_PRO_INFO + 60)
#define ADDR_BATTERY_MFR (AP_CFG_ROM_ADDR_PRO_INFO + 92)
#define ADDR_BATTERY_MODEL (AP_CFG_ROM_ADDR_PRO_INFO + 112)
#define ADDR_BATTERY_PROD_DATE (AP_CFG_ROM_ADDR_PRO_INFO + 132)
#define ADDR_BATTERY_SERIAL1 (AP_CFG_ROM_ADDR_PRO_INFO + 152)
#define ADDR_BATTERY_SERIAL2 (AP_CFG_ROM_ADDR_PRO_INFO + 172)
#define ADDR_NU171X_CHECKSUM (AP_CFG_ROM_ADDR_PRO_INFO + 192)
#define ADDR_WB7720_CHECKSUM (AP_CFG_ROM_ADDR_PRO_INFO + 196)
#define ADDR_PRODUCT_INFO_VERSION (AP_CFG_ROM_ADDR_PRO_INFO + PRODUCT_INFO_TOTAL_SIZE)

// Product information structure: 200 bytes, stored in Flash as raw struct bytes.
typedef struct
{
	char manufacturer_name1[PRODUCT_INFO_FIELD_SIZE];
	char manufacturer_name2[PRODUCT_INFO_FIELD_SIZE];
	char model_name[PRODUCT_INFO_FIELD_SIZE];
	char product_serial[PRODUCT_SERIAL_FIELD_SIZE];
	char battery_mfr[PRODUCT_INFO_FIELD_SIZE];
	char battery_model[PRODUCT_INFO_FIELD_SIZE];
	char battery_prod_date[PRODUCT_INFO_FIELD_SIZE];
	char serial1[SERIAL_FIELD_SIZE];
	char serial2[SERIAL_FIELD_SIZE];
	char NU171X_checksum[PRODUCT_CHECKSUM_FIELD_SIZE];
	char WB7720_checksum[PRODUCT_CHECKSUM_FIELD_SIZE];
} ProductInfo_t;

/********************* Battery Record Structures *********************/
// ========== CONFIGURABLE: Change this to use 1, 2, or 3 pages ==========
#define LOG_PAGE_COUNT          3           // Number of log pages (1, 2, or 3)
// ========================================================================

// Validation
#if (LOG_PAGE_COUNT < 1) || (LOG_PAGE_COUNT > 3)
#error "LOG_PAGE_COUNT must be 1, 2, or 3"
#endif

// Derived configuration (auto-calculated based on LOG_PAGE_COUNT)
#define MAX_RECORDS_RAM 0                                         // No records in RAM (save 100 bytes!)
#define MAX_RECORDS_PER_PAGE 24                                   // Maximum records per flash page
#define MAX_TOTAL_RECORDS (LOG_PAGE_COUNT * MAX_RECORDS_PER_PAGE) // Auto-calculated

// Page identifiers (support up to 3 pages)
#define FLASH_PAGE_LOG1 0 // Page 0: LOG1 (0x1400)
#define FLASH_PAGE_LOG2 1 // Page 1: LOG2 (0x1200)
#define FLASH_PAGE_LOG3 2 // Page 2: LOG3 (0x1000)

// Legacy compatibility
#define MAX_RECORDS MAX_RECORDS_RAM // Backward compatibility

// Timestamp structure (8 bytes)
typedef struct
{
	uint16_t year;    // Year 2026-2099
	uint8_t month;    // Month 1-12
	uint8_t day;      // Day 1-31
	uint8_t hour;     // Hour 0-23
	uint8_t minute;   // Minute 0-59
	uint8_t second;   // Second 0-59
	uint8_t reserved; // Alignment byte
} TimeStamp_t;

// Unified exception record structure (20 bytes)
typedef struct
{
	TimeStamp_t timestamp; // 8 bytes: Record timestamp
	uint8_t error_type;    // 1 byte: 0x01=overvoltage, 0x02=overtemp, 0x03=undertemp
	uint8_t sub_type;      // 1 byte: OV=cell_num, TEMP=charge_state
	union
	{
		struct
		{
			uint16_t max_voltage;   // Cell max voltage (mV)
			uint16_t total_voltage; // Total voltage (mV)
		} ov_data;
		struct
		{
			int16_t max_temperature; // Max temperature (deg C)
			uint16_t reserved;
		} temp_data;
		uint8_t raw_data[6];
	} data;                 // 6 bytes: Union data area
	uint32_t record_id;     // 4 bytes: Record sequence number
} BatteryExceptionRecord_t; // Total: 20 bytes

// Exception tracking cache (RAM) - unified window version
// Shared between sleep and non-sleep modes; persists in RAM across sleep cycles
typedef struct
{
	uint32_t window_start_seconds; // Unified window start (0 = not initialized)

	uint8_t ov1_triggered;
	uint16_t ov1_max_voltage;   // mV
	uint16_t ov1_total_voltage; // mV
	TimeStamp_t ov1_timestamp;

	uint8_t ov2_triggered;
	uint16_t ov2_max_voltage;
	uint16_t ov2_total_voltage;
	TimeStamp_t ov2_timestamp;

	uint8_t temp_chg_triggered;
	int16_t temp_chg_max; // degC
	uint8_t temp_chg_event_type;
	TimeStamp_t temp_chg_timestamp;

	uint8_t temp_dchg_triggered;
	int16_t temp_dchg_max; // degC
	uint8_t temp_dchg_event_type;
	TimeStamp_t temp_dchg_timestamp;
} ExceptionCache_t;

// Virtual parameter sentinel values (shared by usb_bridge and bat_record)
#define VIRTUAL_CELL_SENTINEL 0xFFFF
#define VIRTUAL_TEMP_SENTINEL 0x7FFF

// 16-bit cycle count accessor macros (lo=Battery_cycle_count, hi=Battery_cycle_count_hi)
#define GET_CYCLE_COUNT(gd) ((uint16_t)(gd)->Battery_cycle_count | ((uint16_t)(gd)->Battery_cycle_count_hi << 8))
#define SET_CYCLE_COUNT(gd, v)                                          \
	do                                                                  \
	{                                                                   \
		VIC_vModuleDisable();                                           \
		(gd)->Battery_cycle_count = (uint8_t)((v) & 0xFF);              \
		(gd)->Battery_cycle_count_hi = (uint8_t)(((uint16_t)(v)) >> 8); \
		VIC_vModuleEnable();                                            \
	} while (0)

// RAM storage metadata (records live in Flash, not RAM — saves ~98B vs old 5-record cache)
typedef struct
{
	uint32_t magic;            // Magic value
	uint8_t exception_counter; // Exception record total count 0-255
	uint8_t write_ptr;         // Write pointer in active page (0-23)
	uint8_t active_page;       // Current active page index (0 or 1)
	uint8_t page_sequence;     // Page sequence for wear leveling
	uint16_t checksum;         // Simple additive checksum
	uint16_t padding;          // Alignment
} BatteryRecordStorage_t;      // 12 bytes (was ~110 bytes)

// Flash page layout structure - stored in flash (496 bytes per page)
typedef struct
{
	uint32_t magic;                                         // 4 bytes: Magic value for validation
	uint8_t page_records_count;                             // 1 byte: Number of records in this page (0-24)
	uint8_t page_number;                                    // 1 byte: Page identifier (0=LOG1, 1=LOG2)
	uint8_t overflow_ptr;                                   // 1 byte: Points to other page (reserved)
	uint8_t page_seq;                                       // 1 byte: Page sequence number (increments on switch, for determining newest)
	uint32_t page_timestamp;                                // 4 bytes: Last write timestamp (seconds)
	BatteryExceptionRecord_t records[MAX_RECORDS_PER_PAGE]; // 480 bytes: 24 records x 20 bytes
	uint16_t checksum;                                      // 2 bytes: Page checksum
	uint16_t padding;                                       // 2 bytes: Alignment padding
} FlashPageLayout_t;                                        // Total: 496 bytes
#endif

#if CYCLE_COUNT_FLASH_PERSIST
void cycle_count_save_to_flash(void);
#endif

struct ap_t
{
	// uint8_t app_info_0; //0-0x2000
	// uint8_t app_info_1; //1-0x2001
	// uint8_t app_info_2;
	// uint8_t app_info_3;
	// uint8_t app_info_4;
	// uint8_t app_info_5;
	// uint8_t app_info_6;
	// uint8_t app_info_7;

	struct
	{
		uint8_t power_on;
		uint8_t idle;
		uint8_t charging;
		uint8_t charged;
		uint8_t error;
	} led_ctrl;

	uint8_t mpp_dither_en;
	uint8_t auth_seic_type; //0-fm1210, 1-t91206, 2-ciu98

	uint16_t ptmc;
	uint16_t t_next_ping;

	uint8_t tntc_otp_dis;
	uint8_t tntc_utp_dis;
	uint8_t tdie_otp_dis;
	uint8_t tdie_utp_dis;
	uint8_t isns_ocp_dis;
	uint8_t icap_ocp_dis;
	uint8_t vbus_ovp_dis;
	uint8_t vbus_uvp_dis;
	uint8_t vbus_dpl_dis;
	uint8_t vpwr_ovp_dis;
	uint8_t pout_opp_dis;

	uint16_t tdie_otp_thd;
	uint16_t tdie_otp_hys;
	uint16_t tdie_utp_thd;
	uint16_t tdie_utp_hys;
	uint16_t tntc_otp_thd;
	uint16_t tntc_otp_hys;
	uint16_t tntc_utp_thd;
	uint16_t tntc_utp_hys;
	uint16_t isns_ocp_thd;
	uint16_t isns_ocp_hys;
	uint16_t icap_ocp_thd;
	uint16_t icap_ocp_hys;
	uint16_t vbus_ovp_thd;
	uint16_t vbus_ovp_hys;
	uint16_t vbus_uvp_thd;
	uint16_t vbus_uvp_hys;
	uint16_t vbus_dpl_thd;
	uint16_t vbus_dpl_hys;
	uint16_t vpwr_ovp_thd;
	uint16_t vpwr_ovp_hys;
	uint16_t pout_opp_thd;
	uint16_t pout_opp_hys;

	uint16_t pid_volt_lim_hi;
	uint16_t pid_volt_lim_mi;
	uint16_t pid_volt_lim_lo;

	uint16_t pid_perd_lim_hi;
	uint16_t pid_perd_lim_mi;
	uint16_t pid_perd_lim_lo;

	uint16_t pid_duty_lim_hi;
	uint16_t pid_duty_lim_mi;
	uint16_t pid_duty_lim_lo;

	uint16_t pid_phas_lim_hi;
	uint16_t pid_phas_lim_mi;
	uint16_t pid_pahs_lim_lo;

	uint16_t dig_ping_volt_5v;
	uint16_t dig_ping_perd_5v;
	uint16_t dig_ping_duty_5v;
	uint16_t dig_ping_phas_5v;

	uint16_t dig_ping_volt_6v;
	uint16_t dig_ping_perd_6v;
	uint16_t dig_ping_duty_6v;
	uint16_t dig_ping_phas_6v;

	uint16_t dig_ping_volt_9v;
	uint32_t dig_ping_perd_9v;
	uint16_t dig_ping_duty_9v;
	uint16_t dig_ping_phas_9v;

	uint16_t dig_ping_volt_11v;
	uint32_t dig_ping_perd_11v;
	uint16_t dig_ping_duty_11v;
	uint16_t dig_ping_phas_11v;

	uint8_t pin_max_cnt;
	uint8_t pin_fod_dis;
	uint8_t pin_fod_cnt;
	uint8_t rpp_fod_dis;
	uint8_t rpp_fod_cnt;

	uint16_t q_factor_base_value;
	uint16_t q_factor_reco_value;
	uint16_t q_factor_limH_value;
	uint16_t q_factor_limL_value;
	uint16_t q_factor_obj_value;
	uint16_t q_factor_stable_value;

	uint32_t fs_base_value;
	uint32_t fs_reco_value;
	uint32_t fs_limH_value;
	uint32_t fs_limL_value;
	uint16_t fs_obj_value;
	uint16_t fs_stable_value;
	uint16_t low_k_val;
	uint8_t ddm_check_interval_long;
#if CONFIG_NEW_CCC_LOG_ENABLE
	// Battery exception record module data
	ExceptionCache_t exception_cache;      // fault tracking module (~16 bytes)
	BatteryRecordStorage_t record_storage; // Flash record ram (~110 bytes)
#endif
};

struct gd_t
{
	// struct
	// {
	// 	uint8_t plat_info_0;
	// 	uint8_t plat_info_1;
	// 	uint8_t plat_info_2;
	// 	uint8_t plat_info_3;
	// } plt_infos;

	struct
	{
		uint32_t pd_pdo[7];
	} usb_infos;

	uint16_t vbus;
	uint16_t vpwr;
	uint16_t isns;
	uint16_t vpwr_avg;
	uint16_t isns_avg;
	uint16_t isns_pre;
	uint16_t icol_max;
	uint16_t icol_rms;
	uint8_t power_mode;
	uint8_t ctx_ind;
	uint16_t ctx;
	uint32_t tx_power;
	uint32_t rx_power;
	uint32_t rx_prect;
	uint32_t p_rect_max_ntc_ot;

	uint16_t vctx_pp;
	uint32_t k_est;

	union nu103x_t nu103x_sts_last;
	union nu103x_t nu103x_sts_curr;

	uint16_t dig_ping_volt;
	uint16_t dig_ping_perd;
	uint8_t force_usb_mode;
	uint16_t dig_ping_duty;
	uint16_t dig_ping_phas;

	uint16_t pid_volt;
	uint16_t pid_perd;
	uint16_t pid_duty;
	uint16_t pid_phas;

	struct
	{
		uint8_t tim3_evnt;
		uint8_t led_status;
		int16_t die_temp;
		int16_t ntc_temp_wpc;
		int16_t ntc_temp_typec;
	} sys_infos;
	struct
	{
		uint8_t is_dither_en;
		uint8_t is_dig_ddm_en;
	} sys_status;
	struct
	{
		struct
		{
			uint8_t dpl : 1;
		} cep_event;
		uint8_t ping_type;
		uint8_t dig_ping_type;
		uint8_t fsk_done_event; //1->xce_pid 2->update_fsk_param 3->stop_power
		uint16_t t_next_ping;
		uint16_t t_re_ping;
		uint16_t reping_cnt;
		uint32_t q_fact;
		uint32_t f_self;
		uint32_t q_fact_air;
		uint32_t f_self_air;
		uint8_t need_full_brg; //cep_event
		uint8_t fo_exist;
		uint8_t pfod_event; //ioc_event
		uint8_t pfod_trig_cnt;
		int32_t pfod_margin;
		uint16_t cloak_dig_ping_delay;
		uint8_t cloak_det_ping_delay;
		uint8_t cloak_reason;
		uint8_t flg_mode_cloak;
		uint8_t state_exit_cloak;
		uint8_t flg_cloak_tx_init;
		uint8_t flg_cloak_tx_enter;
		uint8_t flg_cloak_tx_exit;
		uint8_t power_mode_trans_atn;
		uint8_t power_mode_trans_eptr;
		uint8_t power_mode_trans_cloak;
		uint8_t ept_reping_type;
		uint8_t _128_nego_gd;
		uint8_t max_cap;
		uint8_t nego_cap; //100mW unit
		uint8_t need_renego_cap;
		uint8_t power_limit_reason;
		uint8_t tar_cap_fod;
		uint8_t tar_cap_cali;
		uint8_t tar_cap_otp;
		//  uint8_t tar_cap_uvp;
		//  uint8_t tar_cap_ocp;
		//  uint8_t tar_cap_opp;
		int16_t mate_q_g0;
		int16_t mate_q_g1;
		int16_t mate_q_g2;
		int16_t mate_q_a0;
		int16_t mate_q_a1;
		int16_t mate_q_a2;
		uint16_t dp_alpha;
		uint16_t dp_beta;
		uint8_t ept_attempt_cnt;
		uint8_t rx_status;          //1: Rx attached, 0: Rx detached.
		uint8_t master_adaptor_cap; //1: BPP 5W, 2: MPP 15W
	} tx_infos;

	uint8_t pla_id;

	struct
	{
		uint8_t cmt;
		uint8_t success;
		uint8_t index;
		uint8_t index_cnt;
		uint16_t preceived;
		uint16_t prect;
		uint16_t vrect;
		uint16_t irect;
	} dploss_cal;

	struct
	{
		uint32_t tntc_otp_flag : 1;
		uint32_t tntc_utp_flag : 1;
		uint32_t tdie_otp_flag : 1;
		uint32_t tdie_utp_flag : 1;
		uint32_t isns_ocp_flag : 1;
		uint32_t vbus_ovp_flag : 1;
		uint32_t vbus_uvp_flag : 1;
		uint32_t vbus_dpl_flag : 1;
		uint32_t vpwr_ovp_flag : 1;
		uint32_t pout_opp_flag : 1;
		uint32_t q_fod_flag : 1;
		uint32_t xfer_fod_flag : 1;
		uint32_t coil_ntc_source_fault : 1;
		uint32_t : 19;
	} prot_sts; //set pro_evnt

	struct
	{
		uint16_t volt_lim_hi;
		uint16_t volt_lim_mi;
		uint16_t volt_lim_lo;
		uint16_t perd_lim_hi;
		uint16_t perd_lim_mi;
		uint16_t perd_lim_lo;
		uint16_t duty_lim_hi;
		uint16_t duty_lim_mi;
		uint16_t duty_lim_lo;
		uint16_t phas_lim_hi;
		uint16_t phas_lim_mi;
		uint16_t phas_lim_lo;
	} pid_limit;

	struct
	{
		uint16_t fop_flag : 1; //fod limit power flag
		uint16_t vbus_uv_flag : 1;
		uint16_t tntc_ot_flag : 1; //NTC temperature limit power flag,1: will let CEP=-5 to reduce power, 0:
		uint16_t vbus_ov_flag : 1;
		uint16_t isns_oc_flag : 1;
		uint16_t pout_op_flag : 1;
	} power_limit_sts;
	uint8_t tntc_ot_flag_atn; //0: initial value, 1:need send ATN, 2: have sent ATN

	struct ask_packet_t wpc_pkt;
	struct adp_t adp;
	uint8_t adp_type_upd; //set adp_evnt

	uint8_t wpc_idle_state;

	uint8_t ptx_idle_phase_status;
	uint8_t ptx_protocol_phase;
	uint8_t ptx_end_nego_event;
	uint8_t sys_err_code;

	struct
	{
		uint8_t power_profile_mode;
		uint8_t mpp_restricted_mode;
		uint8_t mpp_restricted_power_limit;
		uint8_t ssp_value;
		uint8_t qi_version;
		uint8_t ref_q;
		uint8_t ref_f;
		uint8_t opt_cnt;
		uint8_t neg;
		uint8_t phase_state;
		uint8_t epp_mode;
		uint8_t max_power;
		uint8_t max_power_temp;
		uint8_t gant_power_temp;
		uint8_t fsk_param;
		uint8_t wnd_size;
		uint8_t pch_t_delay;
		uint8_t guaranteed_power;
		uint8_t private_charge;
		int8_t cep_val;
		int8_t cep_pre;
		uint8_t cep_cnt;
		uint8_t rx_type;
		uint8_t chr_status;
		uint8_t pla_type;
		uint16_t prmc;
		int16_t gcoil_tx;
		int16_t alpha_fm;
		int16_t alpha_fm_dc;
		int16_t gcoil_tx2;
		int16_t alpha_fm_itx;
		int16_t alpha_fm_irect;
		int16_t alpha_fm_vrect;
		uint16_t pla_prect;
		uint16_t pla_vrect;
		uint16_t pla_irect;
		uint32_t device_id;

		uint8_t rsp_type;
		uint8_t gant_power;
		uint8_t ref_power;

		uint32_t stand_power;
		uint8_t rpp_tick;
		uint8_t rpp_rsp_type;
		uint16_t rpp;
		uint16_t cali_light;
		uint16_t cali_connect;
	} rx_infos;

	struct fsk_cfg_t fsk_cfg;
	uint8_t fsk_silence;

	uint8_t dmo1_phase; //0-dig_ping, 1-lo_power, 2-hi_power
	uint8_t dmo2_phase; //0-dig_ping, 1-lo_power, 2-hi_power

	uint8_t nego_flag;
	uint8_t ios_nego_cnt;

	uint8_t dig_ping_continuous_cnt;
	uint8_t atl_test_tpr1c_coil_flag;
	uint8_t atl_test_ldstp_epp_N60;
	uint8_t atl_test_ldstp_bpp_N60;
	uint8_t atl_test_ldstp_bpp_P60;

	uint16_t recv_rpp_count;
	uint16_t last_rpp_value;

	uint8_t reset_magicode;
	uint16_t idle_to_sleep_cnt;
	uint8_t sleep_qdt_complete_charg_count;
	uint8_t sleep_qdt_fod_rec_count;
	uint8_t sleep_q_times;
	uint8_t rd0_cnt;
	uint8_t rd1_cnt;
	uint8_t light0_cnt;
	uint8_t light1_cnt;

	uint8_t charger_is_6801_flag; // not delete,for gauge
	uint8_t renego_flag;
	uint8_t soc_flag;
	uint8_t q_standby_flag;
	uint8_t flash_times;
	uint8_t resverd_reset;
	uint16_t power_on_magic;
	uint8_t tc0_lighting_mode;
	uint8_t tc1_lighting_mode;
	uint8_t wpc_disable;
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	uint8_t usb_comm_activated; // Triple-click USB bridge gate (survives sleep)
#endif
	uint8_t real_soc_show;
	uint8_t real_soc_obtained;
	// uint8_t dp_result;
	uint8_t bat_dead_flag;
	uint8_t bat_dead_flag_with_snk0;
	uint8_t bat_dead_flag_with_snk1;

	uint8_t Battery_cycle_count;
	uint8_t Battery_cycle_count_hi;
	uint8_t Battery_charger_cnt;
	uint8_t Bat_Rdc;
	int8_t Bat_SoH;
	uint8_t bat_ov_forbid_flag;      // OV Forbid: 1=permanent shutdown (GB31241 3C)
	uint8_t exception_sleep_counter; // Sleep cycles counter for exception tracking
	uint8_t eng_mode_active;         // Engineering mode flag (1=active)
	uint16_t eng_virtual_cell1;      // Virtual Cell1 voltage (0xFFFF=no override)
	uint16_t eng_virtual_cell2;      // Virtual Cell2 voltage (0xFFFF=no override)
	int16_t eng_virtual_temp;        // Virtual temperature (0x7FFF=no override)
	uint32_t Bat_RTC_Timer;     // Shrunk uint64->uint32 to free 4B for relocated fields below.
	                            // Compatible: only used as 10ms tick counter (~497 days uint32 range).
	// --- Relocated from offset 0x400-0x403 to avoid RAM overlap with gui.c app_reg_buff[0..3] ---
	uint8_t protect_ntc1;       // Moved here (within CFG region < 0x200) to escape .data overlap
	uint8_t air_protect_ntc1;   // Moved here
	uint8_t led_fault2;         // Moved here (was silently zeroed by iic_read_info_sync every 10ms)
	uint8_t key_led_fault2;     // Moved here
#if CONFIG_NEW_CCC_LOG_ENABLE
	// System runtime (seconds + milliseconds) - 136 years range
	uint32_t Bat_RTC_Seconds;      // Running seconds: 0 ~ 4,294,967,295 (~136 years)
	uint16_t Bat_RTC_Milliseconds; // Sub-second precision: 0 ~ 999 ms
#endif

	int32_t SOC_RawSOC_mpct;
	uint32_t SOC_SleepTime_s;
	/* 普通休眠起始 RTC 秒数，用于连续 7 天休眠后自动进入船运模式。 */
	uint32_t ship_sleep_start_seconds;

	uint8_t ship_mode_cnt;
	uint8_t sigle_clicked;

	uint8_t led_fault;
	uint8_t led_fault1;
	uint8_t ntc_led_off;
	uint8_t recharge_flag;
	uint8_t bat_ntc_wpc_dischg_reduce_flag;
	uint8_t bat_ntc_cport_dischg_reduce_flag;
	uint8_t ntc_total_lock_flag;
	uint8_t typec_scp;
	uint8_t vbus_ovp;
	uint8_t touch_to_weakup;
	uint8_t flag11;
	uint32_t timer_cnt;
	uint8_t fault_status;
	uint8_t wpc_sleepship;
	struct bat_info g_bat;
	uint8_t key_sleep_exit;
	uint8_t enter_sleep_flag;
	uint8_t bat_ntc_dischg_lock;
	uint8_t bat_ntc_stop_chrg_flag;
	// --- Original 0x400-0x403 slots kept as padding to preserve sizeof(gd_t)=516B ---
	// These 4 bytes fall on 0x20000400-0x20000403 which physically overlaps gui.c app_reg_buff[0..3]
	// (tx_fw_version/tx_chip_id/tx_status/reserved). iic_read_info_sync() writes 0 to tx_status
	// every 10ms, silently clobbering whatever lives at 0x402. Do NOT use these for state.
	// Real fields moved up to ~offset 0x228 (Bat_RTC_Timer region).
	uint8_t _conflict_pad_400;  // formerly protect_ntc1
	uint8_t _conflict_pad_401;  // formerly air_protect_ntc1
	uint8_t _conflict_pad_402;  // formerly led_fault2
	uint8_t _conflict_pad_403;  // formerly key_led_fault2
};
uint16_t dead_battery_voltage;
struct lib_para_sts
{
	uint16_t typec_a_support : 1;
	uint16_t typec_b_support : 1;
	uint16_t ufcs_source_support : 1;
	uint16_t afc_source_support : 1;
	uint16_t fcp_source_support : 1;
	uint16_t scp_source_support : 1;
} lib_para;

extern volatile struct ap_t *ap;
extern volatile struct gd_t *gd;
extern uint8_t g_forbid_bypass_flag; // 4-click toggle: 1=OV/UV forbid bypassed

void ap_data_init(void);
void lib_para_init(void);
void gd_data_init(void);
#if CONFIG_NEW_CCC_LOG_ENABLE
// Product information read/write functions
void product_info_read(ProductInfo_t *info);
void product_info_write(const ProductInfo_t *info);
void product_info_print(void);
#endif

#endif /* G_DATA_H_ */
