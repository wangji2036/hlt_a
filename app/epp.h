#ifndef EPP_H_
#define EPP_H_

//The structure of this code is not easy to read. 
//The content is also not easy to read in places, it is a read-unfriendly code, shame on you!
//If you have any questions, please contact: 15577577568 or i@iotang.cn

#include "typdef.h"
#include "printk.h"
#include "pkt_type.h"
#include "epp_config.h"
#include "epp_port.h"

#define EPP_LOG_OUTPUT_ENABLE

#ifdef EPP_LOG_OUTPUT_ENABLE
#define EPP_Debug(fmt, args...) EPP_Debug_Print(fmt, ##args)
#else
#define EPP_Debug(fmt, args...)
#endif // EPP_LOG_OUTPUT_ENABLE

#define BIT_SET(bit, pos) (*(bit) |= (1 << (pos)))
#define BIT_CLEAR(bit, pos) (*(bit) &= ~(1 << (pos)))
#define IS_BIT_SET(num, pos) ((num) & (1 << (pos)))

#define PU_CERT_LEN_OFS (2 + 32 + 328)
#define PU_CERT_LEN_MAX (400)
#define CERT_CHAIN_LEN (PU_CERT_LEN_OFS + PU_CERT_LEN_MAX)
#define DIGEST_LENGTH (32)
#define CHALL_LENGTH (64)
#define CRC_INITIAL_VALUE (0xFFFF)

#define CERT_LENGHT 328U

#define AUTH_SUCCESS                   0x00U // Successful return value
#define AUTH_FAIL                      0xFFU // Fail return value

#define EPP_AUTH_ADT_HDR 0x76U

/*
 * Qi_v1.xx or 2.xx_epp_comms_protocol, power receiver data packets
 */
enum epp_prx_ask_pkt_type_t
{
	// CFG Phase
	EPP_PRx_PKT_TYP_SIG = 0x01,
	EPP_PRx_PKT_TYP_ID = 0x71,
	EPP_PRx_PKT_TYP_CFG = 0x51,
	EPP_PRx_PKT_TYP_PCH = 0x06,

	// Negotiation Phase
	EPP_PRx_PKT_TYP_NEGO = 0x09,
	EPP_PRx_PKT_TYP_GRQ = 0x07,
	EPP_PRx_PKT_TYP_SRQ = 0x20,
	EPP_PRx_PKT_TYP_FOD = 0x22,
	EPP_PRx_PKT_TYPE_WPID_msb = 0x54,
	EPP_PRx_PKT_TYPE_WPID_lsb = 0x55,

	// Power Transfer Phase
	EPP_PRx_PKT_TYP_CE = 0x03,
	EPP_PRx_PKT_TYP_CHS = 0x05,
	EPP_PRx_PKT_TYPE_RPP_8bit = 0x04,
	EPP_PRx_PKT_TYPE_RPP_24bit = 0x31,

	// Authentication
	EPP_PRx_PKT_TYP_DSR_15 = 0x15,
	EPP_PRx_PKT_TYP_ADC_25 = 0x25,	

	EPP_PRx_PKT_TYP_ADT_1e_16 = 0x16,
	EPP_PRx_PKT_TYP_ADT_1o_17 = 0x17,
	EPP_PRx_PKT_TYP_ADT_2e_26 = 0x26,
	EPP_PRx_PKT_TYP_ADT_2o_27 = 0x27,
	EPP_PRx_PKT_TYP_ADT_3e_36 = 0x36,
	EPP_PRx_PKT_TYP_ADT_3o_37 = 0x37,
	EPP_PRx_PKT_TYP_ADT_4e_46 = 0x46,
	EPP_PRx_PKT_TYP_ADT_4o_47 = 0x47,
	EPP_PRx_PKT_TYP_ADT_5e_56 = 0x56,
	EPP_PRx_PKT_TYP_ADT_5o_57 = 0x57,
	EPP_PRx_PKT_TYP_ADT_6e_66 = 0x66,
	EPP_PRx_PKT_TYP_ADT_6o_67 = 0x67,
	EPP_PRx_PKT_TYP_ADT_7e_76 = 0x76,
	EPP_PRx_PKT_TYP_ADT_7o_77 = 0x77,

	EPP_PRx_Prop_pkt = 0x18,
};

// 0x00 SRQ/en End negotiation
// 0x01 SRQ/gp Guaranteed Load Power
// 0x02 SRQ/rpr Received Power reporting
// 0x03 SRQ/fsk FSK configuration
// 0x04 SRQ/rp Reference Power
// 0x05 SRQ/rep Re-ping delay
// 0x06 SRQ/rcs Recalibration support
// 0xF0...0xFF SRQ/prop Proprietary
enum epp_prx_SRQ_request_type_t
{
	EPP_SRQ_en_00 = 0x00,
	EPP_SRQ_gp_01 = 0x01,
	EPP_SRQ_rpr_02 = 0x02,
	EPP_SRQ_fsk_03 = 0x03,
	EPP_SRQ_rp_04 = 0x04,
	EPP_SRQ_rep_05 = 0x05,
	EPP_SRQ_rcs_06 = 0x06,
	EPP_SRQ_prop_F0 = 0xF0,
	EPP_SRQ_prop_F1 = 0xF1,
	EPP_SRQ_prop_F2 = 0xF2,
	EPP_SRQ_prop_F3 = 0xF3,
	EPP_SRQ_prop_F4 = 0xF4,
	EPP_SRQ_prop_F5 = 0xF5,
	EPP_SRQ_prop_F6 = 0xF6,
	EPP_SRQ_prop_F7 = 0xF7,
	EPP_SRQ_prop_F8 = 0xF8,
	EPP_SRQ_prop_F9 = 0xF9,
	EPP_SRQ_prop_FA = 0xFA,
	EPP_SRQ_prop_FB = 0xFB,
	EPP_SRQ_prop_FC = 0xFC,
	EPP_SRQ_prop_FD = 0xFD,
	EPP_SRQ_prop_FE = 0xFE,
	EPP_SRQ_prop_FF = 0xFF,
};

enum epp_prx_FOD_type_t
{
	FOD_TYPE_qf = 0,
	FOD_TYPE_rf = 1,
};

struct wpc_fsk_cfg
{
	uint8_t pola;	//The requested FSK polarity is positive (ZERO) or negative (ONE).
	uint8_t depth;	//
	uint8_t Ncycles; //qi2 new content 0 : 512 cycles, 1 : 256 cycles, 2 : 128 cycles, 3 : 64 cycles
};

struct power_contract
{ // PTC
	uint8_t nego_fod_mask;
	uint8_t nego_mask;
	uint8_t ref_power;
	uint8_t rcv_pwr_type;
	uint8_t guaranteed_power;
	uint8_t wait_update;
	uint8_t re_ping_delay;		
	struct wpc_fsk_cfg fsk_params;		//This is the parameter in the CFG stage
	
} __attribute__((packed));

enum epp_fod_mode_type_t
{
	EPP_RPP_MODE_RP = 0x00,	  // normal value.
	EPP_CALIB_TYPE_01 = 0x01, // first calibration data point.
	EPP_CALIB_TYPE_02 = 0x02, // additional calibration data point.

	EPP_CALIB_TYPE_04 = 0x04, // normal value but suppress the Response Pattern.
};

struct epp_fod_t
{
	uint8_t epp_fod_mode;
	uint8_t epp_fod_cali_fail_time;
	uint16_t epp_fod_vol;
	uint16_t epp_fod_current;
	uint16_t epp_fod_power;
	
} __attribute__((packed));


struct epp_auth_t
{
	uint8_t EPP_auth_status;
	uint8_t fsk_adt_is_odd;
	uint8_t send_auth_data_header;

	uint8_t EPP_DataStream_Tx_mode;
	uint8_t EPP_DataStream_Rx_mode;

	uint16_t EPP_Datastream_RX_len;
	uint16_t EPP_Datastream_TX_len;
	uint8_t  EPP_ADC_status;

	uint16_t EPP_ADC_offset;
	uint16_t EPP_ADC_len;

	uint8_t  send_digest_slot;
	uint16_t send_digest_len;

	uint8_t  semd_cert_slot;
	uint16_t send_cert_offset;
	uint16_t send_cert_total_len;

	uint8_t rec_challenge_data_offset;
	uint8_t rec_challenge_data_len;

	uint8_t send_challenge_data_index;
	uint8_t send_challenge_data_len;

	uint8_t need_poll_rx_data;
};

//enum auth_header_t
//{
//	RSP_DIGESTS     = 0x11,
//	RSP_CERTIFICATE = 0x12,
//	RSP_CHALLENGE   = 0x13,
//};

enum EPP_auth_massage_header_request_type
{
	msg_get_digest = 0x9, 
	msg_get_certtificate = 0xA,
	msg_get_challenge = 0xB,
	msg_get_ic_data = 0xC,
};




/// @note: EPP Authentication:
//
//			Start Authentication
//
//			Request Get Digest    Tx -> Rx Digest ok
//			Digest Response		  TX -> Digest	
//			Request Get Certificate	
//			Certificate Response
//			Request Challenge
//			Challenge Response
//			End Authentication
//

enum
{
	SDSR_ACK = 0,
	SDSR_UNEXPECTED = 1,
	SDSR_ERR_BUSY = 2,
	SDSR_ERR_CRC = 3,
};

enum EPP_ADC_request_t
{
	ADC_end = 0x00,
	ADC_auth = 0x02,
	ADC_rst = 0x05,
	ADC_prop0 = 0x10,
	ADC_prop1 = 0x11,
	ADC_prop2 = 0x12,
	ADC_prop3 = 0x13,
	ADC_prop4 = 0x14,
	ADC_prop5 = 0x15,
	ADC_prop6 = 0x16,
	ADC_prop7 = 0x17,
	ADC_prop8 = 0x18,
	ADC_prop9 = 0x19,
	ADC_propA = 0x1A,
	ADC_propB = 0x1B,
	ADC_propC = 0x1C,
	ADC_propD = 0x1D,
	ADC_propE = 0x1E,
	ADC_propF = 0x1F,
};

enum EPP_DataStream_request_t
{
	DSR_end = 0x00,
	DSR_open = 0x01,
	DSR_close = 0x02,
	DSR_ack = 0x03,
	DSR_nak = 0x04,
	DSR_poll = 0x05,
	DSR_unexpected = 0x06,
	DSR_err_busy = 0x07,
	DSR_err_crc = 0x08,
};

typedef enum {
    EPP_Datastream_IDLE_mode = 0,
    EPP_Datastream_RX_mode,
	EPP_Datastream_TX_mode,
	EPP_Datastream_CLOSED_mode,
} EPP_DatastreamMode;

// RX data stream process state
typedef enum {
    RX_DataStream_IDLE = 0,
    RX_DataStream_OPEN_HDR,
    RX_DataStream_DATA,	//Start Send Header first
    RX_DataStream_CLOSE
} RxDataStreamState;

// Tx data stream process state
typedef enum {
    TX_DataStream_IDLE = 0,
	TX_DataStream_ATN,
    TX_DataStream_OPEN,
    TX_DataStream_CLOSE
} TxDataStreamState;

typedef enum {
    EPP_Auth_IDLE = 0,
    EPP_Auth_GET_DIGEST,
	EPP_Auth_GET_CERTIFICATE,
	EPP_Auth_GET_CHALLENGE,
	EPP_Auth_DONE,
	EPP_Auth_ERROR,
	EPP_Auth_ERROR_VERSION,
} EPP_AuthState;


//************************FSK packet********************************************* */
struct epp_ptx_fsk_pkt_t
{
	union
	{
		uint8_t data[10]; // 1-header + max-9-message, not include checksum

		// Negotiation Phase
		struct com_ptx_fsk_pkt_id_t ID_pkt; // ID 0x30
		struct com_ptx_fsk_pkt_cap_t cap;	// CAP 0x31

		// Authentication
		struct com_ptx_fsk_pkt_adc_t ADC_pkt; // ADC 0x25

		struct com_ptx_fsk_pkt_adt_t ADT_pkt;

	} epp_fsk;
} __attribute__((packed));

void wpc_epp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask);
void wpc_epp_nego_phase_process(struct com_prx_ask_pkt_t *com_ask);

extern void initializePTC(void);
#endif // !EPP_H_
