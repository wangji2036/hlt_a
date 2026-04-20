#ifndef USB_UFCS_H_
#define USB_UFCS_H_

typedef union {
	struct {
		uint32_t SOURCE_SINK_HARDRESET   					: 1;
		uint32_t CABLE_HARDRESET        					: 1;
		uint32_t SND_CMD       								: 1;
		uint32_t BAUD_RATE0      							: 1;
		uint32_t BAUD_RATE1      							: 1;
		uint32_t UFCS_HANDSHAKE_EN     						: 1;
		uint32_t UFCS_SOFT_RESET							: 1;
		uint32_t UFCS_SINK_EN      							: 1;
		uint32_t RELEASE_TX  								: 1;
		uint32_t ACK_DEV_ADDR   							: 2;
		uint32_t RX_CLR      								: 1;
		uint32_t TX_CLR       								: 1;
		uint32_t END_Check_Config							: 1;
		uint32_t Baudrate_Check_Config       				: 1;
		uint32_t tDpDet_Config   							: 1;
		uint32_t tACKtransmit_Config   						: 1;
		uint32_t Buffer_Clear_Config      					: 1;
		uint32_t Buffer_Busy_Config       					: 1;
		uint32_t Confict_Detection_Config      				: 1;
		uint32_t REG_SOFT_RST       						: 1;
		uint32_t tDataRoleSwitch_Disable					: 1;
		uint32_t tx_trans_diable       						: 1;
		uint32_t hdsk_src_1p2   							: 1;
		uint32_t baudrate_mult_err_disable   				: 1;
		uint32_t wait_ack_switch      						: 1;
		uint32_t release_tx_optb       						: 1;
		uint32_t TX_WAIT_FINISH								: 1;
		uint32_t TX_WAIT_1MS_CTRL       					: 1;
		uint32_t        									: 3;
	} BITS;
	uint32_t WORD;
} TS_UFCS_SOURCE_CTRL;

typedef union {
	struct {
		uint32_t VERSION   									: 6;
		uint32_t ACK_RECEIVE_TIMEOUT_MASK        			: 1;
		uint32_t MSG_TRANS_FAIL_MASK       					: 1;
		uint32_t RX_BUFF_BUSY_MASK      					: 1;
		uint32_t RX_OVERFLOW_MASK      						: 1;
		uint32_t DATA_READY_MASK     						: 1;
		uint32_t SENT_PACKET_COMPLETE_MASK					: 1;
		uint32_t UFCS_HANDSHAKE_SUCC_MASK      				: 1;
		uint32_t UFCS_HANDSHAKE_FAIL_MASK  					: 1;
		uint32_t HARD_RESET_MASK   							: 1;
		uint32_t CRC_ERROR_MASK        						: 1;
		uint32_t BAUD_RATE_CHANGE_MASK      				: 1;
		uint32_t START_FAIL_MASK       						: 1;
		uint32_t LENGTH_ERROR_MASK							: 1;
		uint32_t DATA_BYTE_TMOUT_MASK       				: 1;
		uint32_t TRAINING_BYTES_ERROR_MASK   				: 1;
		uint32_t BAUD_RATE_ERROR_MASK   					: 1;
		uint32_t Transeive_Confict_FLAG_MASK      			: 1;
		uint32_t NACK_RECEIVED_FLAG_MASK       				: 1;
		uint32_t TX_EMPETY_FLAG_MASK      					: 1;
		uint32_t RX_FULL_FLAG_MASK       					: 1;
		uint32_t        									: 6;
	} BITS;
	uint32_t WORD;
} TS_UFCS_VERSION_MASK;

typedef union {
	struct {
		uint32_t ACK_RECEIVE_TIMEOUT_FLAG   				: 6;
		uint32_t MSG_TRANS_FAIL_FLAG        				: 1;
		uint32_t RX_BUFF_BUSY_FLAG       					: 1;
		uint32_t RX_OVERFLOW_FLAG      						: 1;
		uint32_t DATA_READY_FLAG      						: 1;
		uint32_t SENT_PACKET_COMPLETE_FLAG     				: 1;
		uint32_t UFCS_HANDSHAKE_FAIL_FLAG					: 1;
		uint32_t UFCS_HANDSHAKE_SUCC_MASK      				: 1;
		uint32_t HARD_RESET_FLAG   							: 1;
		uint32_t CRC_ERROR_FLAG        						: 1;
		uint32_t BAUD_RATE_CHANGE_FLAG      				: 1;
		uint32_t START_FAIL_FLAG       						: 1;
		uint32_t LENGTH_ERROR_FLAG							: 1;
		uint32_t DATA_BYTE_TMOUT_FLAG       				: 1;
		uint32_t TRAINING_BYTES_ERROR_FLAG   				: 1;
		uint32_t BAUD_RATE_ERROR_FLAG   					: 1;
		uint32_t Transeive_Confict_FLAG      				: 1;
		uint32_t RETRY_FLAG       							: 1;
		uint32_t TX_EMPETY_FLAG      						: 1;
		uint32_t RX_FULL_FLAG       						: 1;
		uint32_t        									: 6;
	} BITS;
	uint32_t WORD;
} TS_UFCS_INT_FLAG;

typedef union {
	struct {
		uint32_t ACK_RECEIVE_TIMEOUT_STAT   				: 6;
		uint32_t MSG_TRANS_FAIL_STAT        				: 1;
		uint32_t RX_BUFF_BUSY_STAT       					: 1;
		uint32_t RX_OVERFLOW_STAT      						: 1;
		uint32_t DATA_READY_STAT      						: 1;
		uint32_t SENT_PACKET_COMPLETE_STAT     				: 1;
		uint32_t UFCS_HANDSHAKE_FAIL_STAT					: 1;
		uint32_t UFCS_HANDSHAKE_SUCC_MASK      				: 1;
		uint32_t HARD_RESET_STAT   							: 1;
		uint32_t CRC_ERROR_STAT        						: 1;
		uint32_t BAUD_RATE_CHANGE_STAT      				: 1;
		uint32_t START_FAIL_STAT       						: 1;
		uint32_t LENGTH_ERROR_STAT							: 1;
		uint32_t DATA_BYTE_TMOUT_STAT       				: 1;
		uint32_t TRAINING_BYTES_ERROR_STAT   				: 1;
		uint32_t BAUD_RATE_ERROR_STAT   					: 1;
		uint32_t Transeive_Confict_STAT      				: 1;
		uint32_t RETRY_STAT       							: 1;
		uint32_t TX_EMPETY_STAT      						: 1;
		uint32_t RX_FULL_STAT       						: 1;
		uint32_t        									: 6;
	} BITS;
	uint32_t WORD;
} TS_UFCS_INT_STAT;

typedef union {
	struct {
		uint32_t TX_LENGTH   								: 8;
		uint32_t        									: 24;
	} BITS;
	uint32_t WORD;
} TS_UFCS_TX_LENGTH;

typedef union {
	struct {
		uint32_t RX_LENGTH   								: 8;
		uint32_t        									: 24;
	} BITS;
	uint32_t WORD;
} TS_UFCS_RX_LENGTH;

typedef union {
	struct {
		uint32_t TX_BUFFER0   								: 8;
		uint32_t TX_BUFFER1   								: 8;
		uint32_t TX_BUFFER2   								: 8;
		uint32_t TX_BUFFER3   								: 8;
	} BITS;
	uint32_t WORD;
} TS_UFCS_TX_BUFFER;

typedef union {
	struct {
		uint32_t RX_BUFFER0   								: 8;
		uint32_t RX_BUFFER1   								: 8;
		uint32_t RX_BUFFER2   								: 8;
		uint32_t RX_BUFFER3   								: 8;
	} BITS;
	uint32_t WORD;
} TS_UFCS_RX_BUFFER;

typedef union {
	struct {
		uint32_t In_Receiving_STAT   						: 1;
		uint32_t In_Sending_STAT        					: 1;
		uint32_t Duration_Per_Bit0   						: 1;
		uint32_t Duration_Per_Bit1        					: 1;
		uint32_t Duration_Per_Bit2       					: 1;
		uint32_t Duration_Per_Bit3      					: 1;
		uint32_t Duration_Per_Bit4      					: 1;
		uint32_t Duration_Per_Bit5     						: 1;
		uint32_t Duration_Per_Bit6      					: 1;
		uint32_t Duration_Per_Bit7      					: 1;
		uint32_t 											: 1;
	} BITS;
	uint32_t WORD;
} TS_UFCS_BR_TRANS_STAT;

typedef union {
	struct {
		uint32_t UFCS_DEBUG_SEL0   							: 1;
		uint32_t UFCS_DEBUG_SEL1        					: 1;
		uint32_t UFCS_DEBUG_SEL2       						: 1;
		uint32_t UFCS_DEBUG_SEL3      						: 1;
		uint32_t UFCS_DEBUG_SEL4      						: 1;
		uint32_t UFCS_DEBUG_SEL5     						: 1;
		uint32_t UFCS_DEBUG_SEL6      						: 1;
		uint32_t UFCS_DEBUG_SEL7      						: 1;
		uint32_t 											: 1;
	} BITS;
	uint32_t WORD;
} TS_DPDM_DBG_SEL;

typedef struct {
	__IO TS_UFCS_SOURCE_CTRL 					SOURCE_CTRL; 		//4000_7000
	__IO TS_UFCS_VERSION_MASK 					VERSION_MASK; 		//4000_7004
	__IO TS_UFCS_INT_FLAG 						INT_FLAG; 			//4000_7008
	__IO TS_UFCS_INT_STAT 						INT_STAT; 			//4000_700C
	__IO TS_UFCS_TX_LENGTH 						TX_LENGTH; 			//4000_7010
	__IO TS_UFCS_TX_BUFFER 						TX_BUFFER; 			//4000_7014
	__IO TS_UFCS_RX_LENGTH 						RX_LENGTH; 			//4000_7018
	__IO TS_UFCS_RX_BUFFER 						RX_BUFFER; 			//4000_701C
	__IO TS_UFCS_BR_TRANS_STAT 					BR_TRANS_STAT; 		//4000_7020
	__IO TS_DPDM_DBG_SEL 						DBG_SEL; 			//4000_7024
} TS_UFCS_SOURCE_SINK;

#define  DPDM_UFCS_OFFSET                		(0x7000)
#define  DPDM_UFCS_BASE                  		(APB_BASE +  DPDM_UFCS_OFFSET)
#define  DPDM_UFCS                       		((TS_UFCS_SOURCE_SINK *) DPDM_UFCS_BASE)

#define bswap_16(x)             ((((x) >> 8) & 0xffu) | (((x) & 0xffu) << 8))

#define UFCS_PING                   0x00
#define UFCS_ACK                    0x01
#define UFCS_NAK                    0x02
#define UFCS_ACCEPT                 0x03
#define UFCS_SOFT_RESET             0x04
#define UFCS_POWER_READY            0x05
#define UFCS_GET_OUTPUT_CAP         0x06
#define UFCS_GET_SOURCEINFO         0x07
#define UFCS_GET_SINKINFO           0x08
#define UFCS_GET_CABLEINFO          0x09
#define UFCS_GET_DEVICEINFO         0x0A
#define UFCS_GET_ERRINFO            0x0B
#define UFCS_DETECT_CABLEINFO       0x0C
#define UFCS_START_CABLEDET         0x0D
#define UFCS_END_CABLEDET           0x0E
#define UFCS_EXIT_MODE              0x0F


#define UFCS_OUTPUT_CAPS            0x01
#define UFCS_RESQT                  0x02
#define UFCS_SOURCEINFO             0x03
#define UFCS_SINKINFO               0x04
#define UFCS_CABLEINFO              0x05
#define UFCS_DEVICE_INFO            0x06
#define UFCS_ERROR_INFO             0x07
#define UFCS_CONFIG_WATCHDOG        0x08
#define UFCS_REFUSE                 0x09
#define UFCS_VERIFY_REQUEST         0x0A
#define UFCS_VERIFY_RESPONSE        0x0B
#define UFCS_POWER_CHANGE           0x0C
#define UFCS_TEST_REQUEST           0xFF



enum dev_attr
{
    UFCS_SUPPLY_DEV = 0x01,
    UFCS_CHG_DEV = 0x02,
    UFCS_CABLE = 0x03,
};

#define UFCS_VSERION 0x01

enum ufcs_mgs_type
{
    UFCS_CTRL_MSG = 0x00,
    UFCS_DATA_MSG = 0x01,
    UFCS_USER_MSG = 0x02,
};

enum REFUSE_REASON
{
    REFUSE_REASON_Unrecognized = 0x01,
    REFUSE_REASON_NotSupport = 0x02,
    REFUSE_REASON_DeviceBusy = 0x03,
    REFUSE_REASON_PowerOutRange = 0x04,
    REFUSE_REASON_Other = 0x05,
};

#define CONFIG_UFCS_MAX_VOLTAGE 	11000
#define CONFIG_UFCS_MIN_VOLTAGE		5000
#define CONFIG_UFCS_MAX_CURRENT 	2000


#define UFCS_HEADER(type, rev, id, attr)       \
    ((((type) & 0x7) << 0U) | \
     (((rev) &  0x3f) << 3U) |   \
     (((id) & 0xf) << 9U) |       \
     (((attr) & 0x7) << 13U))

void dpdm_ufcs_init(void);
void dpdm_ufcs_deinit(void);
void ufcs_psread_handle(void);
void ufcs_rx_packet_handle(void);
void ufcs_exit_handle(void);

#endif /* USB_QC_H_ */
