#ifndef WPC_PKT_TYPE_H_
#define WPC_PKT_TYPE_H_

#include "typdef.h"

/*
 * Qi_v2.0_comms_protocol, common power receiver data packets
 */
enum com_prx_ask_pkt_type_t
{
	WPC_PRx_PKT_TYP_SIG_01      = 0x01,
	WPC_PRx_PKT_TYP_EPT_02      = 0x02,
	WPC_PRx_PKT_TYP_CE_03       = 0x03,
	WPC_PRx_PKT_TYP_RP8_04      = 0x04,
	WPC_PRx_PKT_TYP_CHS_05      = 0x05,
	WPC_PRx_PKT_TYP_PCH_06      = 0x06,
	WPC_PRx_PKT_TYP_GRQ_07      = 0x07,
	WPC_PRx_PKT_TYP_NEGO_09     = 0x09,
	WPC_PRx_PKT_TYP_DSR_15      = 0x15,
	WPC_PRx_PKT_TYP_SRQ_20      = 0x20,
	WPC_PRx_PKT_TYP_FOD_22      = 0x22,
	WPC_PRx_PKT_TYP_ADC_25      = 0x25,
	WPC_PRx_PKT_TYP_RP_31       = 0x31,
	WPC_PRx_PKT_TYP_CFG_51      = 0x51,
	WPC_PRx_PKT_TYP_WPID_msb_54 = 0x54,
	WPC_PRx_PKT_TYP_WPID_lsb_55 = 0x55,
	WPC_PRx_PKT_TYP_ID_71       = 0x71,
	WPC_PRx_PKT_TYP_XID_81      = 0x81,

	WPC_PRx_PKT_TYP_ADT_1e_16   = 0x16,
	WPC_PRx_PKT_TYP_ADT_1o_17   = 0x17,
	WPC_PRx_PKT_TYP_ADT_2e_26   = 0x26,
	WPC_PRx_PKT_TYP_ADT_2o_27   = 0x27,
	WPC_PRx_PKT_TYP_ADT_3e_36   = 0x36,
	WPC_PRx_PKT_TYP_ADT_3o_37   = 0x37,
	WPC_PRx_PKT_TYP_ADT_4e_46   = 0x46,
	WPC_PRx_PKT_TYP_ADT_4o_47   = 0x47,
	WPC_PRx_PKT_TYP_ADT_5e_56   = 0x56,
	WPC_PRx_PKT_TYP_ADT_5o_57   = 0x57,
	WPC_PRx_PKT_TYP_ADT_6e_66   = 0x66,
	WPC_PRx_PKT_TYP_ADT_6o_67   = 0x67,
	WPC_PRx_PKT_TYP_ADT_7e_76   = 0x76,
	WPC_PRx_PKT_TYP_ADT_7o_77   = 0x77,

	WPC_PRx_PKT_TYP_PROP_1e_18  = 0x18,
	WPC_PRx_PKT_TYP_PROP_1o_19  = 0x19,
	WPC_PRx_PKT_TYP_PROP_2e_28  = 0x28,
	WPC_PRx_PKT_TYP_PROP_2o_29  = 0x29,
	WPC_PRx_PKT_TYP_PROP_38     = 0x38,
	WPC_PRx_PKT_TYP_PROP_48     = 0x48,
	WPC_PRx_PKT_TYP_PROP_58     = 0x58,
	WPC_PRx_PKT_TYP_PROP_68     = 0x68,
	WPC_PRx_PKT_TYP_PROP_78     = 0x78,
	WPC_PRx_PKT_TYP_PROP_84     = 0x84,
	WPC_PRx_PKT_TYP_PROP_A4     = 0xA4,
	WPC_PRx_PKT_TYP_PROP_C4     = 0xC4,
	WPC_PRx_PKT_TYP_PROP_E2     = 0xE2,
};

struct general_t
{
	uint8_t hdr;
	uint8_t code;
} __attribute__ ((packed));

struct com_prx_ask_pkt_sig_t
{
	uint8_t ss_value;
} __attribute__ ((packed));

struct com_prx_ask_pkt_ept_t
{
	uint8_t ept_code;
} __attribute__ ((packed));

struct com_prx_ask_pkt_cep_t
{
	uint8_t ce_value;
} __attribute__ ((packed));

struct com_prx_ask_pkt_rp8_t
{
	uint8_t rp_value;
} __attribute__ ((packed));

struct com_prx_ask_pkt_chs_t
{
	uint8_t chs_value;
} __attribute__ ((packed));

struct com_prx_ask_pkt_pch_t
{
	uint8_t pch_time;
} __attribute__ ((packed));

struct com_prx_ask_pkt_grq_t
{
	uint8_t request;
} __attribute__ ((packed));

struct com_prx_ask_pkt_nego_t
{
	uint8_t : 8;
} __attribute__ ((packed));

struct com_prx_ask_pkt_dsr_t
{
	uint8_t type;
} __attribute__ ((packed));

struct com_prx_ask_pkt_srq_t
{
	uint8_t request;
	uint8_t parameter;
} __attribute__ ((packed));

struct com_prx_ask_pkt_fod_t
{
	uint8_t type : 2;
	uint8_t reserved : 6;
	uint8_t data;
} __attribute__ ((packed));

struct com_prx_ask_pkt_adc_t
{
	uint8_t param_msb : 3;
	uint8_t request : 5;
	uint8_t params_lsb;
} __attribute__ ((packed));

struct com_prx_ask_pkt_rpp_t
{
	uint8_t mode : 3;
	uint8_t      : 5;
	uint16_t rp_value;
} __attribute__ ((packed));

struct com_prx_ask_pkt_cfg_t {
	uint8_t max_power : 6;
	uint8_t pow_class : 2;
	uint8_t reserved0;
	uint8_t count : 3;
	uint8_t zero : 1;
	uint8_t ob : 1;
	uint8_t reserved1 : 1;
	uint8_t ai : 1;
	uint8_t prop : 1;
	uint8_t wind_offset : 3;
	uint8_t wind_size : 5;
	uint8_t dup : 1;
	uint8_t buffer_size : 3;
	uint8_t fsk_dep : 2;
	uint8_t fsk_pol : 1;
	uint8_t is_nego : 1;
} __attribute__ ((packed));

struct com_prx_ask_pkt_wpid_t
{
	uint8_t segment[3];
	uint8_t crc[2];
} __attribute__ ((packed));

struct com_prx_ask_pkt_id_t
{
	uint8_t minor_ver : 4;
	uint8_t major_ver : 4;
	uint8_t prmc_msb;
	uint8_t prmc_lsb;
	uint8_t bdid0_msb;
	uint8_t bdid0_lsb;
	uint8_t bdid1_msb;
	uint8_t bdid1_lsb;
} __attribute__ ((packed));

struct com_prx_ask_pkt_xid_t
{
	uint8_t selector;
	uint8_t mfg_rsvd_0 : 7;
	uint8_t resticted  : 1;
	uint8_t vrect_msb  : 4;
	uint8_t mfg_rsvd_1 : 4;
	uint8_t vrect_lsb;
	uint8_t alpha0_rx;
	uint8_t alpha1_rx;
	uint8_t alpha_kth_rx;
	uint8_t mfg_rsvd_2;
} __attribute__ ((packed));

struct com_prx_ask_pkt_adt_t
{
	uint8_t data[7];
} __attribute__ ((packed));

struct com_prx_ask_pkt_prop_t
{
	uint8_t data[20];
} __attribute__ ((packed));

struct com_prx_ask_pkt_t
{
	uint8_t hdr;
	union {
		struct com_prx_ask_pkt_sig_t   sig;
		struct com_prx_ask_pkt_ept_t   ept;
		struct com_prx_ask_pkt_cep_t   cep;
		struct com_prx_ask_pkt_rp8_t   rp8;
		struct com_prx_ask_pkt_chs_t   chs;
		struct com_prx_ask_pkt_pch_t   pch;
		struct com_prx_ask_pkt_grq_t   grq;
		struct com_prx_ask_pkt_nego_t nego;
		struct com_prx_ask_pkt_dsr_t   dsr;
		struct com_prx_ask_pkt_srq_t   srq;
		struct com_prx_ask_pkt_fod_t   fod;
		struct com_prx_ask_pkt_adc_t   adc;
		struct com_prx_ask_pkt_rpp_t   rpp;
		struct com_prx_ask_pkt_cfg_t   cfg;
		struct com_prx_ask_pkt_wpid_t wpid;
		struct com_prx_ask_pkt_id_t     id;
		struct com_prx_ask_pkt_xid_t   xid;
		struct com_prx_ask_pkt_adt_t   adt;
		struct com_prx_ask_pkt_prop_t prop;
	} msg;
} __attribute__ ((packed));

/*
 * Qi_v2.0_comms_protocol, common power transmitter data packets
 */
enum com_ptx_fsk_pkt_type_t
{
	WPC_PTx_PKT_TYP_NULL_00    = 0x00,
	WPC_PTx_PKT_TYP_ADC_25     = 0x25,
	WPC_PTx_PKT_TYP_ID_30      = 0x30,
	WPC_PTx_PKT_TYP_CAP_31     = 0x31,
	WPC_PTx_PKT_TYP_XCAP_32    = 0x32,

	WPC_PTx_PKT_TYP_ADT_1e_16  = 0x16,
	WPC_PTx_PKT_TYP_ADT_1o_17  = 0x17,
	WPC_PTx_PKT_TYP_ADT_2e_26  = 0x26,
	WPC_PTx_PKT_TYP_ADT_2o_27  = 0x27,
	WPC_PTx_PKT_TYP_ADT_3e_36  = 0x36,
	WPC_PTx_PKT_TYP_ADT_3o_37  = 0x37,
	WPC_PTx_PKT_TYP_ADT_4e_46  = 0x46,
	WPC_PTx_PKT_TYP_ADT_4o_47  = 0x47,
	WPC_PTx_PKT_TYP_ADT_5e_56  = 0x56,
	WPC_PTx_PKT_TYP_ADT_5o_57  = 0x57,
	WPC_PTx_PKT_TYP_ADT_6e_66  = 0x66,
	WPC_PTx_PKT_TYP_ADT_6o_67  = 0x67,
	WPC_PTx_PKT_TYP_ADT_7e_76  = 0x76,
	WPC_PTx_PKT_TYP_ADT_7o_77  = 0x77,
	WPC_PTx_PKT_TYP_ADT_11e_98 = 0x98,
	WPC_PTx_PKT_TYP_ADT_11o_99 = 0x99,
	WPC_PTx_PKT_TYP_ADT_15e_B8 = 0xB8,
	WPC_PTx_PKT_TYP_ADT_15o_B9 = 0xB9,
	WPC_PTx_PKT_TYP_ADT_19e_E0 = 0xE0,
	WPC_PTx_PKT_TYP_ADT_19o_E1 = 0xE1,
	WPC_PTx_PKT_TYP_ADT_23e_EC = 0xEC,
	WPC_PTx_PKT_TYP_ADT_23o_ED = 0xED,
	WPC_PTx_PKT_TYP_ADT_27e_FC = 0xFC,
	WPC_PTx_PKT_TYP_ADT_27o_FD = 0xFD,

	WPC_PTx_PKT_TYP_PROP_1e_1E = 0x1E,
	WPC_PTx_PKT_TYP_PROP_1o_1F = 0x1F,
	WPC_PTx_PKT_TYP_PROP_2e_2E = 0x2E,
	WPC_PTx_PKT_TYP_PROP_2o_2F = 0x2F,
	WPC_PTx_PKT_TYP_PROP_3F    = 0x3F,
	WPC_PTx_PKT_TYP_PROP_4F    = 0x4F,
	WPC_PTx_PKT_TYP_PROP_5F    = 0x5F,
	WPC_PTx_PKT_TYP_PROP_6F    = 0x6F,
	WPC_PTx_PKT_TYP_PROP_7F    = 0x7F,
	WPC_PTx_PKT_TYP_PROP_8F    = 0x8F,
};

struct com_ptx_fsk_pkt_null_t
{
	uint8_t hdr_00;
	uint8_t invalid_data;
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_adc_t
{
	uint8_t hdr_25;
	uint8_t param_msb : 3;
	uint8_t request   : 5;
	uint8_t params_lsb;
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_id_t
{
	uint8_t hdr_30;
	uint8_t qi_version;
	uint8_t ptmc_msb;
	uint8_t ptmc_lsb;
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_cap_t
{
	uint8_t hdr_31;
	uint8_t neg_power : 6;
	uint8_t           : 2;
	uint8_t pot_power : 6;
	uint8_t           : 2;
	uint8_t nrs       : 1;
	uint8_t wpid      : 1;
	uint8_t buff_size : 3;
	uint8_t ob        : 1;
	uint8_t ar        : 1;
	uint8_t dup       : 1;
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_xcap_t
{
	uint8_t hdr_32;
	uint8_t     : 8;
	uint8_t tds : 1;
	uint8_t tde : 1;
	uint8_t tps : 1;
	uint8_t     : 5;
	uint8_t     : 8;
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_adt_t
{
	uint8_t hdr;
	uint8_t data[27];
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_prop_t
{
	uint8_t hdr;
	uint8_t data[9];
} __attribute__ ((packed));

struct com_ptx_fsk_pkt_t
{
	union {
		uint8_t data[28];
		struct com_ptx_fsk_pkt_null_t null;
		struct com_ptx_fsk_pkt_adc_t   adc;
		struct com_ptx_fsk_pkt_id_t     id;
		struct com_ptx_fsk_pkt_cap_t   cap;
		struct com_ptx_fsk_pkt_xcap_t xcap;
		struct com_ptx_fsk_pkt_adt_t   adt;
		struct com_ptx_fsk_pkt_prop_t prop;
	} com_fsk;
} __attribute__ ((packed));


/*
 * Qi_v2.0_mpp_comms_protocol, power receiver data packets
 */
enum mpp_prx_ask_pkt_type_t
{
	MPP_PRx_PKT_TYP_MSR_13	   = 0X13,
	MPP_PRx_PKT_TYP_CLOAK_18   = 0x18,
	MPP_PRx_PKT_TYP_XCE_19     = 0x19,
	MPP_PRx_PKT_TYP_SRQ_20     = 0x20,
	MPP_PRx_PKT_TYP_GET_28     = 0x28,
	MPP_PRx_PKT_TYP_EDS_29     = 0x29,
	MPP_PRx_PKT_TYP_SDSR_38    = 0x38,
	MPP_PRx_PKT_TYP_SADC_48    = 0x48,
	MPP_PRx_PKT_TYP_REPORT_58  = 0x58,
	MPP_PRx_PKT_TYP_PLAP_78    = 0x78,
	MPP_PRx_PKT_TYP_MPP_XID_81 = 0x81,
	MPP_PRx_PKT_TYP_ECAP_84    = 0x84,
	MPP_PRx_PKT_TYP_PLA2_88    = 0x88,
	MPP_PRx_PKT_TYP_PLAP2_90   = 0x90,
	MPP_PRx_PKT_TYP_CAL_CAP_96 = 0x96,
	MPP_PRx_PKT_TYP_MATE_Q_A8  = 0xA8,

	MPP_PRx_PKT_TYP_SADT_1e_26 = 0x26,
	MPP_PRx_PKT_TYP_SADT_1o_27 = 0x27,
	MPP_PRx_PKT_TYP_SADT_2e_36 = 0x36,
	MPP_PRx_PKT_TYP_SADT_2o_37 = 0x37,
	MPP_PRx_PKT_TYP_SADT_3e_46 = 0x46,
	MPP_PRx_PKT_TYP_SADT_3o_47 = 0x47,
	MPP_PRx_PKT_TYP_SADT_4e_56 = 0x56,
	MPP_PRx_PKT_TYP_SADT_4o_57 = 0x57,
	MPP_PRx_PKT_TYP_SADT_5e_66 = 0x66,
	MPP_PRx_PKT_TYP_SADT_5o_67 = 0x67,
	MPP_PRx_PKT_TYP_SADT_6e_76 = 0x76,
	MPP_PRx_PKT_TYP_SADT_6o_77 = 0x77,

	MPP_PRx_PKT_TYP_PROP_1A    = 0x1A,
	MPP_PRx_PKT_TYP_PROP_1B    = 0x1B,
	MPP_PRx_PKT_TYP_PROP_2A    = 0x2A,
	MPP_PRx_PKT_TYP_PROP_2B    = 0x2B,
	MPP_PRx_PKT_TYP_PROP_39    = 0x39,
	MPP_PRx_PKT_TYP_PROP_49    = 0x49,
	MPP_PRx_PKT_TYP_PROP_59    = 0x59,
	MPP_PRx_PKT_TYP_PROP_79    = 0x79,
	MPP_PRx_PKT_TYP_PROP_85    = 0x85,
};

struct mpp_prx_ask_pkt_cloak_t
{
	uint8_t reason : 4;
	uint8_t        : 4;
}  __attribute__ ((packed));

struct mpp_prx_ask_pkt_xce_t
{
	uint8_t xce_value;
}  __attribute__ ((packed));

struct mpp_prx_ask_pkt_srq_t
{
	uint8_t request;
	uint8_t parameter;
}  __attribute__ ((packed));

struct mpp_prx_ask_pkt_get_t
{
	uint8_t zero;
	uint8_t param : 5;
	uint8_t       : 3;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_eds_t
{
	uint8_t streams_bitmask;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_sdsr_t
{
	uint8_t            : 8;
	uint8_t stream_num : 4;
	uint8_t            : 4;
	uint8_t type;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_sadc_t
{
	uint8_t request    : 3;
	uint8_t resverved0 : 5;
	uint8_t stream_num : 4;
	uint8_t resverved1 : 4;
	uint8_t param_msb;
	uint8_t param_lsb;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_report_xid_t
{
	uint8_t report_id : 2;
	uint8_t reserved0 : 3;
	uint8_t select    : 3;
//	uint8_t prx_bdid0 : 7;
//	uint8_t reserved1 : 1;
//	uint8_t prx_bdid1;
//	uint8_t mfg_rsvd0 : 3;
//	uint8_t prx_bdid2 : 5;
	uint8_t prx_byteid0;
	uint8_t prx_byteid1;
	uint8_t prx_byteid2;
	uint8_t mfg_rsvd1;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_report_pla_t
{
	uint8_t reserved0 : 5;
	uint8_t select    : 3;
	uint8_t rcvd_power_msb;
	uint8_t rcvd_power_lsb;
	uint8_t rect_power_msb;
	uint8_t rect_power_lsb;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_plap_t
{
	uint8_t reserved;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} alpha_fm;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} alpha_fm_dc;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} g_coil_tx;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_mpp_xid_t
{
	uint8_t selector;
	uint8_t mfg_rsvd_h : 7;
	uint8_t restricted : 1;
	uint8_t vrecth     : 4;
	uint8_t mfg_rsvd_l : 4;
	uint8_t vrectl;
	uint8_t alpha0_rx;
	uint8_t alpha1_rx;
	uint8_t alpha_kth_rx;
	uint8_t mfg_rsvd;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_ecap_t
{
	uint8_t zero;
	uint8_t family_collection : 4;
	uint8_t : 4;
	uint8_t reserved;
	uint8_t mini_chg_power_level : 4;
	uint8_t : 2;
	uint8_t tethered : 1;
	uint8_t battery : 1;
	uint8_t freq_mask : 2;
	uint8_t : 1;
	uint8_t prefer_freq : 2;
	uint8_t : 1;
	uint8_t mpp_ref_design : 2;
	uint8_t : 2;
	uint8_t data_stream_buffer_size : 3;
	uint8_t concurrent_data_strteams : 3;
	uint8_t : 6;
	uint8_t dual_role : 1;
	uint8_t nfc_tag : 1;
	uint8_t mfg_rsvd;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_sadt_t
{
	uint8_t stream_num;
	uint8_t data[6];
} __attribute__ ((packed));

/*
 * Qi_v2.2_comms_protocol, common power receiver data packets
 */
struct mpp_prx_ask_pkt_msr_t {//0x13 mode_select_request
	uint8_t aux 	  : 1;
	uint8_t  		  : 2;
	uint8_t main_mode : 2;
	uint8_t 		  : 3;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_pla2_t {//0x88 power_loss_account2
	uint8_t reserved;
	uint8_t rcvd_power_msb;
	uint8_t rcvd_power_lsb;
	uint8_t prect_msb;
	uint8_t prect_lsb;
	uint8_t vrect_msb;
	uint8_t vrect_lsb;
	uint8_t irect_h : 4;
	uint8_t 		: 4;
	uint8_t irect_l;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_plap2_t {//0x90 power_loss_account_param
	uint8_t reserved;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} g_coil_tx2;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} alpha_fm_itx;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} alpha_fm_irect;

	struct {
		uint8_t msb_h : 3;
		uint8_t 	  : 5;
		uint8_t msb_l;
		uint8_t lsb;
	} alpha_fm_vrect;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_cal_capture_t {//0x96 ploss_cal_capture
	uint8_t index;
	uint8_t operation;
	uint8_t rcvd_power_msb;
	uint8_t rcvd_power_lsb;
	uint8_t prect_msb;
	uint8_t prect_lsb;
	uint8_t vrect_msb;
	uint8_t vrect_lsb;
	uint8_t irect_h : 4;
	uint8_t 		: 4;
	uint8_t irect_l;
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_mated_q_coeff_t {//0xA8 mated_q_coeff
	uint8_t reserved0;
	struct {
		uint8_t msb;
		uint8_t lsb;
	} g0;
	struct {
		uint8_t msb : 3;
		uint8_t 	: 5;
		uint8_t lsb;
	} g1;
	struct {
		uint8_t msb : 3;
		uint8_t 	: 5;
		uint8_t lsb;
	} g2;
	uint8_t reserved[6];
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_prop_t
{
	uint8_t data[20];
} __attribute__ ((packed));

struct mpp_prx_ask_pkt_t
{
	uint8_t hdr;
	union {
		uint8_t data[10];
		struct mpp_prx_ask_pkt_cloak_t cloak;
		struct mpp_prx_ask_pkt_msr_t msr;
		struct mpp_prx_ask_pkt_xce_t xce;
		struct mpp_prx_ask_pkt_srq_t srq;
		struct mpp_prx_ask_pkt_get_t get;
		struct mpp_prx_ask_pkt_eds_t eds;
		struct mpp_prx_ask_pkt_sadt_t sadt;
		struct mpp_prx_ask_pkt_sdsr_t sdsr;
		struct mpp_prx_ask_pkt_sadc_t sadc;
		struct mpp_prx_ask_pkt_report_xid_t report_xid;
		struct mpp_prx_ask_pkt_report_pla_t report_pla;
		struct mpp_prx_ask_pkt_pla2_t pla2;
		struct mpp_prx_ask_pkt_plap_t plap;
		struct mpp_prx_ask_pkt_plap2_t plap2;
		struct mpp_prx_ask_pkt_mpp_xid_t mpp_xid;
		struct mpp_prx_ask_pkt_ecap_t ecap;
		struct mpp_prx_ask_pkt_cal_capture_t cal_capture;
		struct mpp_prx_ask_pkt_mated_q_coeff_t mate_q;
		struct mpp_prx_ask_pkt_prop_t prop;
	} msg;
} __attribute__ ((packed));

/*
 * Qi_v2.0_mpp_comms_protocol, power transmitter data packets
 */
enum mpp_ptx_fsk_pkt_type_t
{
	MPP_PTx_PKT_TYP_ERR_01   = 0x01,
	MPP_PTx_PKT_TYP_CLOAK_1E = 0x1E,
	MPP_PTx_PKT_TYP_RCS_1E   = 0x1E,
	MPP_PTx_PKT_TYP_CHS_1F   = 0x1F,
	MPP_PTx_PKT_TYP_MSS_23	 = 0x23,
	MPP_PTx_PKT_TYP_GET_2E   = 0x2E,
	MPP_PTx_PKT_TYP_EDS_2F   = 0x2F,
	MPP_PTx_PKT_TYP_INV_3F   = 0x3F,
	MPP_PTx_PKT_TYP_SDSR_3F  = 0x3F,
	MPP_PTx_PKT_TYP_KEST_3F  = 0x3F,
	MPP_PTx_PKT_TYP_SADC_4F  = 0x4F,
	MPP_PTx_PKT_TYP_MI_5A 	 = 0x5A,
	MPP_PTx_PKT_TYP_PLAP_5F  = 0x5F,
	MPP_PTx_PKT_TYP_PLAP2_88 = 0x88,
	MPP_PTx_PKT_TYP_XID_8F   = 0x8F,
	MPP_PTx_PKT_TYP_ECAP_8F  = 0x8F,

	MPP_PTx_PKT_TYP_SADT_1e_26 = 0x26,
	MPP_PTx_PKT_TYP_SADT_1o_27 = 0x27,
	MPP_PTx_PKT_TYP_SADT_2e_36 = 0x36,
	MPP_PTx_PKT_TYP_SADT_2o_37 = 0x37,
	MPP_PTx_PKT_TYP_SADT_3e_46 = 0x46,
	MPP_PTx_PKT_TYP_SADT_3o_47 = 0x47,
	MPP_PTx_PKT_TYP_SADT_4e_56 = 0x56,
	MPP_PTx_PKT_TYP_SADT_4o_57 = 0x57,
	MPP_PTx_PKT_TYP_SADT_5e_66 = 0x66,
	MPP_PTx_PKT_TYP_SADT_5o_67 = 0x67,
	MPP_PTx_PKT_TYP_SADT_6e_76 = 0x76,
	MPP_PTx_PKT_TYP_SADT_6o_77 = 0x77,

	MPP_PTx_PKT_TYP_PROP_1C    = 0x1C,
	MPP_PTx_PKT_TYP_PROP_1D    = 0x1D,
	MPP_PTx_PKT_TYP_PROP_2C    = 0x2C,
	MPP_PTx_PKT_TYP_PROP_2D    = 0x2D,
	MPP_PTx_PKT_TYP_PROP_3E    = 0x3E,
	MPP_PTx_PKT_TYP_PROP_4E    = 0x4E,
	MPP_PTx_PKT_TYP_PROP_5E    = 0x5E,
	MPP_PTx_PKT_TYP_PROP_8E    = 0x8E,
};

struct mpp_ptx_fsk_pkt_err_t
{
	uint8_t hdr_01;
	uint8_t error : 2;
	uint8_t       : 3;
	uint8_t info  : 3; //support after qi22
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_cloak_t
{
	uint8_t hdr_1E;
	uint8_t reason   : 4;
	uint8_t selector : 4;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_rcs_t
{
	uint8_t hdr_1E;
	uint8_t status   : 4;
	uint8_t selector : 4;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_chs_t
{
	uint8_t hdr_1F;
	uint8_t chs_value;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_get_t
{
	uint8_t hdr_2E;
	uint8_t       : 8;
	uint8_t param : 5;
	uint8_t       : 3;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_eds_t
{
	uint8_t hdr_2F;
	uint8_t streams_bitmask_msb;
	uint8_t streams_bitmask_lsb;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_inv_t
{
	uint8_t hdr_3F;
	uint8_t selector;
	uint8_t vinv_msb : 6;
	uint8_t          : 2;
	uint8_t vinv_lsb;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_sdsr_t
{
	uint8_t hdr_3F;
	uint8_t selector;
	uint8_t stream_num : 4;
	uint8_t            : 4;
	uint8_t type       : 2;
	uint8_t            : 6;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_kest_t
{
	uint8_t hdr_3F;
	uint8_t selector;
	uint8_t kest_msb : 4;
	uint8_t          : 4;
	uint8_t kest_lsb;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_sadc_t
{
	uint8_t hdr_4F;
	uint8_t request : 3;
	uint8_t         : 5;
	uint8_t stream_num : 4;
	uint8_t            : 4;
	uint8_t param_msb;
	uint8_t param_lsb;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_plap_t
{
	uint8_t hdr_5F;
	uint8_t : 8;
	uint8_t g_coil_rx_msb;
	uint8_t g_coil_rx_lsb;
	uint8_t : 8;
	uint8_t : 8;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_xid_t
{
	uint8_t hdr_8F;
	uint8_t uid      : 1;
	uint8_t app      : 1;
	uint8_t          : 2;
	uint8_t selector : 4;
	uint8_t          : 8;
	uint8_t          : 8;
	uint8_t          : 8;
	uint8_t ptx_bdid_msb : 7;
	uint8_t              : 1;
	uint8_t ptx_bdid_mid;
	uint8_t mfg_rsvd_msb : 3;
	uint8_t ptx_bdid_lsb : 5;
	uint8_t mfg_rsvd_mid;
	uint8_t mfg_rsvd_lsb;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_ecap_t
{
	uint8_t hdr_8F;
	uint8_t          : 4;
	uint8_t selector : 4;
	uint8_t          : 8;
	uint8_t ptx_potential_power;
	uint8_t 		 : 8;
	uint8_t prx_negotiable_power;
	uint8_t power_limit_reason : 4;
	uint8_t 				   : 2;
	uint8_t cal_support 	   : 1; //support after qi22
	uint8_t 				   : 1;
	uint8_t concurrent_data_stream  : 3;
	uint8_t data_stream_buffer_size : 3;
	uint8_t                         : 1;
	uint8_t power_src               : 1; //support after qi22
	uint8_t          : 8;
	uint8_t          : 8;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_sadt_t
{
	uint8_t hdr;
	uint8_t stream_num : 4;
	uint8_t            : 4;
	uint8_t data[6];
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_prop_t
{
	uint8_t hdr;
	uint8_t data[9];
} __attribute__ ((packed));

/*
 * Qi_v2.2_comms_protocol, common power transmitter data packets
 */
struct mpp_ptx_fsk_pkt_mss_t {//Mode Select Status
	uint8_t hdr_0x23;
	uint8_t status : 2;//0: success, 1: pending, 2: fail, 3: busy
	uint8_t : 6;
	uint8_t error_code : 2;//0: no error, 1: not supported, 2: mode switch fail
	uint8_t : 6;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_mode_info_t {
	uint8_t hdr_0x5A;
	uint8_t active_aux: 1;
	uint8_t : 2;
	uint8_t active_main_mode : 2;
	uint8_t : 3;
	uint8_t cpm : 1;
	uint8_t cpm_aux : 1;
	uint8_t : 2;
	uint8_t npm : 1;
	uint8_t npm_aux : 1;
	uint8_t : 2;
	uint8_t llpm : 1;
	uint8_t : 3;
	uint8_t hpm : 1;
	uint8_t hpm_aux : 1;
	uint8_t : 2;
	uint16_t reserved;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_plap2_t {
	uint8_t hdr_0x88;
	uint8_t : 8;
	uint16_t g_coil_rx2;
	uint8_t g_alfa_fm_irect_bit16 : 1;
	uint8_t : 7;
	uint16_t g_alfa_fm_irect_bit0_15;
	uint8_t g_alfa_fm_vrect_bit16 : 1;
	uint8_t : 7;
	uint16_t g_alfa_fm_vrect_bit0_15;
} __attribute__ ((packed));

struct mpp_ptx_fsk_pkt_t
{
	union{
		uint8_t data[10]; //1-header + max-9-message, not include checksum
		struct mpp_ptx_fsk_pkt_err_t err;
		struct mpp_ptx_fsk_pkt_cloak_t cloak;
		struct mpp_ptx_fsk_pkt_rcs_t rcs;
		struct mpp_ptx_fsk_pkt_chs_t chs;
		struct mpp_ptx_fsk_pkt_mss_t mss;
		struct mpp_ptx_fsk_pkt_mode_info_t mode_info;
		struct mpp_ptx_fsk_pkt_get_t get;
		struct mpp_ptx_fsk_pkt_eds_t eds;
		struct mpp_ptx_fsk_pkt_inv_t inv;
		struct mpp_ptx_fsk_pkt_sdsr_t sdsr;
		struct mpp_ptx_fsk_pkt_kest_t kest;
		struct mpp_ptx_fsk_pkt_sadc_t sadc;
		struct mpp_ptx_fsk_pkt_plap_t plap;
		struct mpp_ptx_fsk_pkt_plap2_t plap2;
		struct mpp_ptx_fsk_pkt_xid_t xid;
		struct mpp_ptx_fsk_pkt_ecap_t ecap;
		struct mpp_ptx_fsk_pkt_sadt_t sadt;
		struct mpp_ptx_fsk_pkt_prop_t prop;
	} mpp_fsk;
} __attribute__ ((packed));



////////////////////////////////////////////////////////
union com_ptx_fsk_pkt_typ
{
	struct com_ptx_fsk_pkt_null_t null;
	struct com_ptx_fsk_pkt_adc_t   adc;
	struct com_ptx_fsk_pkt_id_t     id;
	struct com_ptx_fsk_pkt_cap_t   cap;
	struct com_ptx_fsk_pkt_xcap_t xcap;
	struct com_ptx_fsk_pkt_adt_t   adt;
	struct com_ptx_fsk_pkt_prop_t prop;
} __attribute__ ((packed));
union mpp_ptx_fsk_pkt_typ
{
	struct mpp_ptx_fsk_pkt_err_t err;
	struct mpp_ptx_fsk_pkt_cloak_t cloak;
	struct mpp_ptx_fsk_pkt_rcs_t rcs;
	struct mpp_ptx_fsk_pkt_chs_t chs;
	struct mpp_ptx_fsk_pkt_get_t get;
	struct mpp_ptx_fsk_pkt_eds_t eds;
	struct mpp_ptx_fsk_pkt_inv_t inv;
	struct mpp_ptx_fsk_pkt_sdsr_t sdsr;
	struct mpp_ptx_fsk_pkt_kest_t kest;
	struct mpp_ptx_fsk_pkt_sadc_t sadc;
	struct mpp_ptx_fsk_pkt_plap_t plap;
	struct mpp_ptx_fsk_pkt_xid_t xid;
	struct mpp_ptx_fsk_pkt_ecap_t ecap;
	struct mpp_ptx_fsk_pkt_sadt_t sadt;
	struct mpp_ptx_fsk_pkt_prop_t prop;
} __attribute__ ((packed));
struct ptx_fsk_pkt_t
{
	union {
		uint8_t data[28];
		union com_ptx_fsk_pkt_typ com_fsk;
		union mpp_ptx_fsk_pkt_typ mpp_fsk;
	} pkt;
} __attribute__ ((packed));
////////////////////////////////////////////////////////

#endif /* WPC_PKT_TYPE_H_ */
