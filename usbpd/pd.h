#ifndef PD_H_
#define PD_H_

#include "typdef.h"

enum pd_pdo_type
{
    PDO_TYPE_FIXED = 0,
    PDO_TYPE_BATT = 1,
    PDO_TYPE_VAR = 2,
    PDO_TYPE_APDO = 3,
};

enum pd_apdo_type
{
    APDO_TYPE_PPS = 0,
    APDO_TYPE_AVS,
    APDO_TYPE_SPR_AVS,
};

enum pd_bist_mode
{
	BIST_Carrier_Mode = 5,
	BIST_Test_Data = 8,
	BIST_Shared_Test_Mode_Entry = 0x09,
	BIST_Shared_Test_Mode_Exit = 0x0a,
};



#define BIG_LITTLE_SWAP16(A)        ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))

#define PD_REV10                0x0
#define PD_REV20                0x1
#define PD_REV30                0x2
#define PD_MAX_REV              PD_REV30

#define PD_HEADER_EXT_HDR       (0x01<<15)
#define PD_HEADER_CNT_SHIFT     12
#define PD_HEADER_CNT_MASK      0x7
#define PD_HEADER_ID_SHIFT      9
#define PD_HEADER_ID_MASK       0x7
#define PD_HEADER_PWR_ROLE      (0x01<<8)
#define PD_HEADER_REV_SHIFT     6
#define PD_HEADER_REV_MASK      0x3
#define PD_HEADER_DATA_ROLE     (0x01<<5)
#define PD_HEADER_TYPE_SHIFT    0
#define PD_HEADER_TYPE_MASK     0x1F

#define PD_HEADER(type, pwr, data, rev, id, cnt, ext_hdr)       \
    ((((type) & PD_HEADER_TYPE_MASK) << PD_HEADER_TYPE_SHIFT) | \
     ((pwr) == TYPEC_SOURCE ? PD_HEADER_PWR_ROLE : 0) |     \
     ((data) == TYPEC_HOST ? PD_HEADER_DATA_ROLE : 0) |     \
     (((rev) &  PD_HEADER_REV_MASK) << PD_HEADER_REV_SHIFT) |   \
     (((id) & PD_HEADER_ID_MASK) << PD_HEADER_ID_SHIFT) |       \
     (((cnt) & PD_HEADER_CNT_MASK) << PD_HEADER_CNT_SHIFT) |    \
     ((ext_hdr) ? PD_HEADER_EXT_HDR : 0))

#define PD_HEADER_LE(type, pwr, data, rev, id, cnt) \
    (PD_HEADER((type), (pwr), (data), (rev), (id), (cnt), (0)))

#define PD_HEADER_EXT_LE(type, pwr, data, rev, id, cnt) \
    PD_HEADER((type), (pwr), (data), (rev), (id), (cnt), (1))

#define PD_EXT_HDR(data_size, req_chunk, chunk_num, chunked)                \
    ((((data_size) & PD_EXT_HDR_DATA_SIZE_MASK) << PD_EXT_HDR_DATA_SIZE_SHIFT) |    \
     ((req_chunk) ? PD_EXT_HDR_REQ_CHUNK : 0) |                 \
     (((chunk_num) & PD_EXT_HDR_CHUNK_NUM_MASK) << PD_EXT_HDR_CHUNK_NUM_SHIFT) |    \
     ((chunked) ? PD_EXT_HDR_CHUNKED : 0))


#define PD_EXT_HDR_LE(data_size, req_chunk, chunk_num, chunked) \
    (PD_EXT_HDR((data_size), (req_chunk), (chunk_num), (chunked)))


#define RDO_OBJ_POS_SHIFT           28
#define RDO_OBJ_POS_MASK            0xf
#define RDO_GIVE_BACK               (0x01<<27) /* Supports reduced operating current */
#define RDO_CAP_MISMATCH            (0x01<<26) /* Not satisfied by source caps */
#define RDO_USB_COMM                (0x01<<25) /* USB communications capable */
#define RDO_NO_SUSPEND              (0x01<<24) /* USB Suspend not supported */
#define RDO_UNCHUNK_SUPPORT         (0x01<<23) /* USB Suspend not supported */
#define RDO_EPR_CAPBLE              (0x01<<22) /* USB Suspend not supported */

#define RDO_PWR_MASK                0x3FF
#define RDO_CURR_MASK               0x3FF
#define RDO_FIXED_OP_CURR_SHIFT     10
#define RDO_FIXED_MAX_CURR_SHIFT    0
#define RDO_PROG_VOLT_MASK          0x7FF
#define RDO_PROG_CURR_MASK          0x7F
#define RDO_PROG_VOLT_SHIFT         9
#define RDO_PROG_CURR_SHIFT         0
#define RDO_PROG_VOLT_MV_STEP       20
#define RDO_PROG_CURR_MA_STEP       50

#define RDO_OBJ(idx) (((idx) & RDO_OBJ_POS_MASK) << RDO_OBJ_POS_SHIFT)
#define RDO_FIXED_OP_CURR(ma) ((((ma) / 10) & RDO_CURR_MASK) << RDO_FIXED_OP_CURR_SHIFT)
#define RDO_FIXED_MAX_CURR(ma) ((((ma) / 10) & RDO_CURR_MASK) << RDO_FIXED_MAX_CURR_SHIFT)

#define RDO_FIXED(idx, op_ma, max_ma, flags) (RDO_OBJ(idx) | (flags) | RDO_FIXED_OP_CURR(op_ma) | RDO_FIXED_MAX_CURR(max_ma) | RDO_NO_SUSPEND)



#define PDO_TYPE_SHIFT              30
#define PDO_TYPE_MASK               0x3

#define PDO_TYPE(t)                 ((t) << PDO_TYPE_SHIFT)

#define PDO_VOLT_MASK               0x3FF
#define PDO_CURR_MASK               0x3FF
#define PDO_PWR_MASK                0x3FF

#define PDO_FIXED_DUAL_ROLE             (0x01 << 29)/* Power role swap supported */
#define PDO_FIXED_SUSPEND               (0x01 << 28) /* USB Suspend supported (Source) */
#define PDO_HIGH_CAPABILITY             (0x01 << 28) /* USB Suspend supported (Source) */
#define PDO_FIXED_UNCONSTRAINED_POWER   (0x01 << 27) /* Unconstrained Power */
#define PDO_FIXED_USB_COMM              (0x01 << 26) /* USB communications capable */
#define PDO_FIXED_DATA_SWAP             (0x01 << 25) /* Data role swap supported */
#define PDO_FIXED_UNCHUNK_EXT           (0x01 << 24) /* Unchunked Extended Message supported (Source) */
#define PDO_FIXED_EPR_MODE              (0x01 << 23)

#define PDO_FIXED_VOLT_SHIFT            10  /* 50mV units */
#define PDO_FIXED_CURR_SHIFT            0   /* 10mA units */


#define PDO_PROG_OUT_VOLT(mv)   \
    ((((mv) / RDO_PROG_VOLT_MV_STEP) & RDO_PROG_VOLT_MASK) << RDO_PROG_VOLT_SHIFT)
#define PDO_PROG_OP_CURR(ma)    \
    ((((ma) / RDO_PROG_CURR_MA_STEP) & RDO_PROG_CURR_MASK) << RDO_PROG_CURR_SHIFT)

#define RDO_PROG(idx, out_mv, op_ma, flags) (RDO_OBJ(idx) | (flags) | PDO_PROG_OUT_VOLT(out_mv) | PDO_PROG_OP_CURR(op_ma))
#define PDO_FIXED_VOLT(mv)          ((((mv) / 50) & PDO_VOLT_MASK) << PDO_FIXED_VOLT_SHIFT)
#define PDO_FIXED_CURR(ma)          ((((ma) / 10) & PDO_CURR_MASK) << PDO_FIXED_CURR_SHIFT)

#define PDO_FIXED(mv, ma, flags)            \
    (PDO_TYPE(PDO_TYPE_FIXED) | (flags) |       \
     PDO_FIXED_VOLT(mv) | PDO_FIXED_CURR(ma))

#define PDO_VAR(min_mv, max_mv, max_ma)             \
    (PDO_TYPE(PDO_TYPE_VAR) | PDO_VAR_MIN_VOLT(min_mv) |    \
     PDO_VAR_MAX_VOLT(max_mv) | PDO_VAR_MAX_CURR(max_ma))

#define PDO_APDO_TYPE_SHIFT         28  /* Only valid value currently is 0x0 - PPS */
#define PDO_APDO_TYPE_MASK          0x3

#define PDO_APDO_TYPE(t)            ((t) << PDO_APDO_TYPE_SHIFT)

#define PDO_PPS_APOD_OUTPUT_VOLT_SHIFT  9       // 20mV/bit
#define PDO_PPS_APDO_OUTPUT_VOLT_MASK   0xFFF   //bit[20:9]


#define PDO_PPS_APDO_MAX_VOLT_SHIFT 17  /* 100mV units */
#define PDO_PPS_APDO_MIN_VOLT_SHIFT 8   /* 100mV units */
#define PDO_PPS_APDO_MAX_CURR_SHIFT 0   /* 50mA units */
#define PDO_SPR_AVS_9_15_SHIFT		10
#define PDO_SPR_AVS_15_20_SHIFT		0

#define PDO_PPS_APDO_VOLT_MASK      0xFF
#define PDO_PPS_APDO_CURR_MASK      0x7F

#define PDO_PPS_APDO_MIN_VOLT(mv)   \
    ((((mv) / 100) & PDO_PPS_APDO_VOLT_MASK) << PDO_PPS_APDO_MIN_VOLT_SHIFT)
#define PDO_PPS_APDO_MAX_VOLT(mv)   \
    ((((mv) / 100) & PDO_PPS_APDO_VOLT_MASK) << PDO_PPS_APDO_MAX_VOLT_SHIFT)
#define PDO_PPS_APDO_MAX_CURR(ma)   \
    ((((ma) / 50) & PDO_PPS_APDO_CURR_MASK) << PDO_PPS_APDO_MAX_CURR_SHIFT)

#define PDO_PPS_APDO_9_15_CURR(ma)   \
    ((((ma) / 10) & 0x3FF) << PDO_SPR_AVS_9_15_SHIFT)

#define PDO_PPS_APDO_15_20_CURR(ma)   \
    ((((ma) / 10) & 0x3FF) << PDO_SPR_AVS_15_20_SHIFT)

#define PDO_PPS_APDO(min_mv, max_mv, max_ma)                \
    (PDO_TYPE(PDO_TYPE_APDO) | PDO_APDO_TYPE(APDO_TYPE_PPS) |   \
    PDO_PPS_APDO_MIN_VOLT(min_mv) | PDO_PPS_APDO_MAX_VOLT(max_mv) | \
    PDO_PPS_APDO_MAX_CURR(max_ma))

#define PDO_SPR_AVS(ma_9_15,ma_15_20)                \
    (PDO_TYPE(PDO_TYPE_APDO) | PDO_APDO_TYPE(APDO_TYPE_SPR_AVS) |   \
    PDO_PPS_APDO_9_15_CURR(ma_9_15) | \
    PDO_PPS_APDO_15_20_CURR(ma_15_20))

#define PD_PPS_FLAGS_OMF                      (1 << 3)
#define PD_PPS_FLGAS_PTF(raw)                 ((raw & 0x06) >> 1)
#define PD_PPS_FLAGS_SET_PTF(val)             ((val & 0x03) << 1)
#define PD_PPS_SET_OUTPUT_MV(mv)              (((mv) / 20) & 0xFFFF)
#define PD_PPS_SET_OUTPUT_MA(ma)              (((ma) / 50) & 0xFF)


enum pd_ctrl_msg_type {
/* Control Message type */
	/* 0 Reserved */
	PD_CTRL_GOOD_CRC = 1,
	PD_CTRL_GOTO_MIN = 2,
	PD_CTRL_ACCEPT = 3,
	PD_CTRL_REJECT = 4,
	PD_CTRL_PING = 5,
	PD_CTRL_PS_RDY = 6,
	PD_CTRL_GET_SOURCE_CAP = 7,
	PD_CTRL_GET_SINK_CAP = 8,
	PD_CTRL_DR_SWAP = 9,
	PD_CTRL_PR_SWAP = 10,
	PD_CTRL_VCONN_SWAP = 11,
	PD_CTRL_WAIT = 12,
	PD_CTRL_SOFT_RESET = 13,
	/* 14-15 Reserved */
    PD_CTRL_DATA_RESET = 14,
    PD_CTRL_DATA_RET_COMPLETE = 15,
    // CONFIG_USB_PD_REV30
	PD_CTRL_NOT_SUPP = 16,
	PD_CTRL_GET_SOURCE_CAP_EXT = 17,
	PD_CTRL_GET_STATUS = 18,
	PD_CTRL_FR_SWAP = 19,
	PD_CTRL_GET_PPS_STATUS = 20,
	PD_CTRL_GET_COUNTRY_CODES = 21,

	PD_CTRL_GET_SINK_CAP_EXT = 22,
    PD_CTRL_GET_SOURCE_INFO = 23,
    PD_CTRL_GET_REVISION = 24,
	/* 22-31 Reserved */
	PD_CTRL_MSG_NR,
};

enum pd_data_msg_type {
/* Data message type */
	/* 0 Reserved */
	PD_DATA_SOURCE_CAP = 1,
	PD_DATA_REQUEST = 2,
	PD_DATA_BIST = 3,
	PD_DATA_SINK_CAP = 4,

	PD_DATA_BAT_STATUS = 5,
	PD_DATA_ALERT = 6,
	PD_DATA_GET_COUNTRY_INFO = 7,
    PD_DATA_ENTER_USB = 8,
    PD_DATA_EPR_REQUEST = 9,
    PD_DATA_EPR_MODE = 10,
    PD_DATA_SOURCE_INFO = 11,
    PD_DATA_REVISION = 12,
	/* 7-14 Reserved */
	PD_DATA_VENDOR_DEF = 15,
	PD_DATA_MSG_NR,
};


enum pd_ext_msg_type
{
    /* 0 Reserved */
    PD_EXT_SOURCE_CAP_EXT = 1,
    PD_EXT_STATUS = 2,
    PD_EXT_GET_BATT_CAP = 3,
    PD_EXT_GET_BATT_STATUS = 4,
    PD_EXT_BATT_CAP = 5,
    PD_EXT_GET_MANUFACTURER_INFO = 6,
    PD_EXT_MANUFACTURER_INFO = 7,
    PD_EXT_SECURITY_REQUEST = 8,
    PD_EXT_SECURITY_RESPONSE = 9,
    PD_EXT_FW_UPDATE_REQUEST = 10,
    PD_EXT_FW_UPDATE_RESPONSE = 11,
    PD_EXT_PPS_STATUS = 12,
    PD_EXT_COUNTRY_INFO = 13,
    PD_EXT_COUNTRY_CODES = 14,
    PD_EXT_SNK_CAPABILITIES_EXTENDED=15,
    PD_EXT_EXTENDED_CTRL=16,
    PD_EXT_EPR_SOURCE_CAPABILITIES=17,
    PD_EXT_EPR_SINK_CAPABILITIES=18,
};


struct usb_pd_source_cap_packet_t
{
	union
	{
		union
		{
			struct
			{
				uint32_t max_current           	: 10;
				uint32_t voltage              	: 10;
				uint32_t peak_current 			: 2;
				uint32_t                		: 1;
				uint32_t epr_mode_capable       : 1;
				uint32_t unchunked_support 		: 1;
				uint32_t data_role 				: 1;
				uint32_t usb_comm_capable 		: 1;
				uint32_t unconstrained_power 	: 1;
				uint32_t usb_supend_support 	: 1;
				uint32_t dual_role_power 		: 1;
				uint32_t fixed 					: 2;
			} FIX_BITS;
			struct
			{
				uint32_t max_current           	: 7;
				uint32_t               			: 1;
				uint32_t min_voltage 			: 8;
				uint32_t                		: 1;
				uint32_t max_voltage       		: 8;
				uint32_t  						: 2;
				uint32_t pps_limited 			: 1;
				uint32_t apdo_type 				: 2;
				uint32_t fixed 					: 2;
			} PPS_BITS;
			struct
			{
				uint32_t i_15_20           		: 10;
				uint32_t i_9_15             	: 10;
				uint32_t  						: 6;
				uint32_t i_peak              	: 2;
				uint32_t apdo_type 				: 2;
				uint32_t fixed 					: 2;
			} SPR_AVS_BITS;
		} BITS;
		uint32_t WORD;
	} source_pdo[7];
}  __attribute__ ((packed));

struct usb_pd_sink_cap_packet_t
{
	union
	{
		union
		{
			struct
			{
				uint32_t op_current           	: 10;
				uint32_t voltage              	: 10;
				uint32_t 			 			: 3;
				uint32_t fast_swap_required     : 2;
				uint32_t dual_data_role 		: 1;
				uint32_t usb_comm_capable 		: 1;
				uint32_t unconstrained_power 	: 1;
				uint32_t high_capability	 	: 1;
				uint32_t dual_role_power 		: 1;
				uint32_t fixed 					: 2;
			} FIX_BITS;
			struct
			{
				uint32_t max_current           	: 7;
				uint32_t               			: 1;
				uint32_t min_voltage 			: 8;
				uint32_t                		: 1;
				uint32_t max_voltage       		: 8;
				uint32_t  						: 5;
				uint32_t fixed 					: 2;
			} PPS_BITS;
		} BITS;
		uint32_t WORD;
	} sink_pdo[7];
}  __attribute__ ((packed));

struct usb_pd_request_packet_t
{
	union
	{
		struct
		{
			uint32_t max_current           		: 10;
			uint32_t op_current             	: 10;
			uint32_t 			 				: 2;
			uint32_t epr_mode_capable           : 1;
			uint32_t unchunked_support       	: 1;
			uint32_t no_usb_suspend 			: 1;
			uint32_t usb_comm_capable 			: 1;
			uint32_t capability_mismatch 		: 1;
			uint32_t giveback_flag 				: 1;
			uint32_t object_posiotion 			: 4;
		} FIX_BITS;
		struct
		{
			uint32_t op_current           		: 7;
			uint32_t              				: 2;
			uint32_t output_voltage 			: 12;
			uint32_t            				: 1;
			uint32_t epr_mode_capable       	: 1;
			uint32_t unchunked_support 			: 1;
			uint32_t no_usb_suspend 			: 1;
			uint32_t usb_comm_capable 			: 1;
			uint32_t capability_mismatch 		: 1;
			uint32_t  							: 1;
			uint32_t object_posiotion 			: 4;
		} PPS_BITS;
		uint32_t WORD;
	} request;
}  __attribute__ ((packed));

struct usb_pd_bist_packet_t
{
	union
	{
		struct
		{
			uint32_t            				: 28;
			uint32_t bist_mode 					: 4;
		} BITS;
		uint32_t WORD;
	} bist;
}  __attribute__ ((packed));

struct usb_pd_chunked_ext_packet_t
{
	union
	{
	    struct
	    {
	        uint16_t data_size					:9;
	        uint16_t reserved					:1;
	        uint16_t request_chunk				:1;
	        uint16_t chunk_num					:4;
	        uint16_t chunked					:1;
	    } BITS;
		uint16_t WORD;
	} ext_hrd;

	uint8_t data[26];

}  __attribute__ ((packed));

struct usb_pd_pkt_t
{

	uint8_t msg_len;
	uint8_t sop_type;
	union
	{
		struct
		{
			uint16_t message_type           : 5;
			uint16_t data_role              : 1;
			uint16_t spec_revision 			: 2; //USBPD_INT_EN
			uint16_t pwr_rolr               : 1;
			uint16_t message_id             : 3;
			uint16_t n_data_object 			: 3; //USBPD_INT_EN
			uint16_t externed 				: 1; //USBPD_INT_EN
		} BITS;
		uint16_t WORD;
	} hdr;

	union
	{
		struct usb_pd_source_cap_packet_t source_cap;
		struct usb_pd_sink_cap_packet_t sink_cap;
		struct usb_pd_request_packet_t request;
		struct usb_pd_bist_packet_t bist;
		struct usb_pd_chunked_ext_packet_t ext_msg;
		uint32_t WORDS[7];
	} msg;
} __attribute__ ((packed));

#define RDO_CURR_MASK               0x3FF
#define RDO_OBJ_POS_SHIFT           28
#define RDO_OBJ_POS_MASK            0xf
#define RDO_FIXED_OP_CURR_SHIFT     10
#define RDO_FIXED_MAX_CURR_SHIFT    0
#define PDO_VAR_MAX_VOLT_SHIFT      20  /* 50mV units */
#define PDO_VAR_MIN_VOLT_SHIFT      10  /* 50mV units */
#define PDO_VAR_MAX_CURR_SHIFT      0   /* 10mA units */
#define PDO_FIXED_VOLT_SHIFT        10  /* 50mV units */
#define PDO_VOLT_MASK               0x3FF

static inline enum pd_pdo_type pdo_type(uint32_t pdo)
{
        return (pdo >> PDO_TYPE_SHIFT) & PDO_TYPE_MASK;
}

static inline unsigned int rdo_index(uint32_t rdo)
{
    return (rdo >> RDO_OBJ_POS_SHIFT) & RDO_OBJ_POS_MASK;
}

static inline unsigned int rdo_op_current(uint32_t rdo)
{
    return ((rdo >> RDO_FIXED_OP_CURR_SHIFT) & RDO_CURR_MASK) * 10;
}

static inline unsigned int rdo_max_current(uint32_t rdo)
{
    return ((rdo >> RDO_FIXED_MAX_CURR_SHIFT) & RDO_CURR_MASK) * 10;
}

static inline unsigned int pdo_fixed_voltage(uint32_t pdo)
{
	return ((pdo >> PDO_FIXED_VOLT_SHIFT) & PDO_VOLT_MASK) * 50;
}

static inline unsigned int pdo_max_current(uint32_t pdo)
{
	return ((pdo >> PDO_VAR_MAX_CURR_SHIFT) & PDO_CURR_MASK) * 10;
}

static inline unsigned int pdo_pps_apdo_max_current(uint32_t pdo)
{
    return ((pdo >> PDO_PPS_APDO_MAX_CURR_SHIFT) & PDO_PPS_APDO_CURR_MASK) * 50;
}

static inline unsigned int pdo_pps_apdo_min_voltage(uint32_t pdo)
{
    return ((pdo >> PDO_PPS_APDO_MIN_VOLT_SHIFT) &PDO_PPS_APDO_VOLT_MASK) * 100;
}

static inline unsigned int pdo_pps_apdo_max_voltage(uint32_t pdo)
{
	return ((pdo >> PDO_PPS_APDO_MAX_VOLT_SHIFT) &PDO_PPS_APDO_VOLT_MASK) * 100;
}

static inline uint32_t rdo_pps_output_voltage(uint32_t rdo)
{
    return ((rdo >> RDO_PROG_VOLT_SHIFT) & RDO_PROG_VOLT_MASK) * RDO_PROG_VOLT_MV_STEP;
}
#endif
