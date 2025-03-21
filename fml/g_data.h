#ifndef G_DATA_H_
#define G_DATA_H_

#include "typdef.h"
#include "nu103x.h"
#include "adp.h"
#include "ask.h"
#include "fsk.h"

#define AP_CFG_ROM_ADDR_BASE    (0x00001600)
#define AP_CFG_RAM_ADDR_BASE    (0x20000000)
#define G_DATA_RAM_ADDR_BASE    (0x20000200)

struct ap_t
{
	uint8_t app_info_0; //0-0x2000
	uint8_t app_info_1; //1-0x2001
	uint8_t app_info_2;
	uint8_t app_info_3;
	uint8_t app_info_4;
	uint8_t app_info_5;
	uint8_t app_info_6;
	uint8_t app_info_7;

	struct {
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
};

struct gd_t
{
	struct {
		uint8_t plat_info_0;
		uint8_t plat_info_1;
		uint8_t plat_info_2;
		uint8_t plat_info_3;
	} plt_infos;

	struct {
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
	uint16_t dig_ping_duty;
	uint16_t dig_ping_phas;

	uint16_t pid_volt;
	uint16_t pid_perd;
	uint16_t pid_duty;
	uint16_t pid_phas;

	struct {
		uint8_t tim3_evnt;
		 uint8_t led_status;
		 int16_t die_temp;
		 int16_t ntc_temp;
	} sys_infos;

	struct {
		struct {
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
		 uint8_t  cloak_det_ping_delay;
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
		uint8_t rx_status;//1: Rx attached, 0: Rx detached.
		uint8_t master_adaptor_cap;//1: BPP 5W, 2: MPP 15W
	} tx_infos;

	uint8_t pla_id;

	struct {
		uint8_t cmt;
		uint8_t success;
		uint8_t index;
		uint8_t index_cnt;
		uint16_t preceived;
		uint16_t prect;
		uint16_t vrect;
		uint16_t irect;
	} dploss_cal;

	struct {
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
		 uint32_t               :20;
	 } prot_sts; //set pro_evnt

	 struct {
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

	struct {
		uint16_t fop_flag : 1;//fod limit power flag
		uint16_t vbus_uv_flag : 1;
		uint16_t tntc_ot_flag : 1;//NTC temperature limit power flag,1: will let CEP=-5 to reduce power, 0:
		uint16_t vbus_ov_flag : 1;
		uint16_t isns_oc_flag : 1;
		uint16_t pout_op_flag : 1;
	 } power_limit_sts;
	 uint8_t tntc_ot_flag_atn;//0: initial value, 1:need send ATN, 2: have sent ATN

	 struct ask_packet_t wpc_pkt;
	 struct adp_t adp;
	 uint8_t adp_type_upd; //set adp_evnt

	 uint8_t wpc_idle_state;

	 uint8_t ptx_idle_phase_status;
	 uint8_t ptx_protocol_phase;
	 uint8_t ptx_end_nego_event;
	 uint8_t sys_err_code;

	 struct {
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

	 uint8_t dig_ping_continuous_cnt;
	 uint8_t atl_test_tpr1c_coil_flag;
	 uint8_t atl_test_ldstp_epp_N60;
	 uint8_t atl_test_ldstp_bpp_N60;
	 uint8_t atl_test_ldstp_bpp_P60;

	 uint8_t alt_test_resv_rp8_cnt;
	 uint8_t alt_test_continous_cnt;
	 uint8_t alt_test_last_rp8_value;
	 uint8_t alt_test_1st_rp8_value;



	 uint8_t reset_magicode;
	 uint8_t idle_to_sleep_cnt;
	 uint8_t sleep_qdt_complete_charg_count;
	 uint8_t sleep_qdt_fod_rec_count;
	 uint8_t soc_show;

	 uint8_t rd0_cnt;
	 uint8_t rd1_cnt;

	 uint8_t bat_dead_flag;
};

extern volatile struct ap_t *ap;
extern volatile struct gd_t *gd;

void ap_data_init(void);
void gd_data_init(void);

#endif /* G_DATA_H_ */
