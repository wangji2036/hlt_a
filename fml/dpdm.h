#ifndef DPDM_H_
#define DPDM_H_


enum dpdm_state_e
{
	DPDM_OFF_STATE,
	DPDM_DCP_MODE,
	DPDM_HVDCP_IDLE_MODE,
	DPDM_HVDCP_QC_MODE,
	DPDM_HVDCP_AFC_MODE,
	DPDM_HVDCP_SCP_MODE,
	DPDM_UFCS_MODE,
};

enum qc_state_e
{
	QC_NOT_MODE = 0,
	QC_FIX_5V_MODE,
	QC_FIX_9V_MODE,
	QC_FIX_12V_MODE,
	QC_FIX_20V_MODE,
	QC_CONTINUOUS_MODE,
};


typedef union {
	struct {
		uint32_t EN_SRC_PROTOCOL     	: 1;
		uint32_t SOFT_RESET        		: 1;
		uint32_t EN_AUTO_DCP       		: 1;
		uint32_t EN_HVDCP_DET      		: 1;
		uint32_t EN_QC_SRC_DET        	: 2;
		uint32_t EN_SCP_SRC_DET       	: 1;
		uint32_t EN_AFC_SRC_DET       	: 1;
		uint32_t EN_UFCS_SRC_DET       	: 1;
		uint32_t EN_900K_PD       		: 1;
		uint32_t MUX_PORT_NUM      	 	: 3;
		uint32_t PORT1_CTRL       		: 1;
		uint32_t PORT2_CTRL       		: 1;
		uint32_t PORT3_CTRL       		: 1;
		uint32_t         				:16;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_CTRL;

typedef union {
	struct {
		uint32_t SOURCE_STAT      	: 3;
		uint32_t         			:29;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_STAT;

typedef union {
	struct {
		uint32_t DM_OV_MASK     			: 1;
		uint32_t DP_OV_MASK        			: 1;
		uint32_t APLLE_DEGLITCH       		: 1;
		uint32_t SAMSUNG_DURATION      		: 1;
		uint32_t DP_FAIL_DEG        		: 3;
		uint32_t DP_FAIL_DET_DIS       		: 1;
		uint32_t ENTER_DCP_INT_MASK       	: 1;
		uint32_t ENTER_HVDCP_INT_MASK       : 1;
		uint32_t         					: 22;
	} BITS;
	uint32_t WORD;
} TS_DPDM_HVDCP_CTRL;

typedef union {
	struct {
		uint32_t DM_OV_FLAG     			: 1;
		uint32_t DP_OV_FLAG        			: 1;
		uint32_t ENTER_DCP       			: 1;
		uint32_t ENTER_HVDCP      			: 1;
		uint32_t         					: 28;
	} BITS;
	uint32_t WORD;
} TS_DPDM_HVDCP_FLAG;

typedef union {
	struct {
		uint32_t TGLITCH_MODE_CHANGE     			: 1;
		uint32_t TACTIVE        					: 1;
		uint32_t FIXED_5V_REQ_INT_MASK       		: 1;
		uint32_t FIXED_9V_REQ_INT_MASK      		: 1;
		uint32_t FIXED_12V_REQ_INT_MASK        		: 1;
		uint32_t FIXED_20V_REQ_INT_MASK       		: 1;
		uint32_t CONTINUOUS_MODE_INT_MASK       	: 1;
		uint32_t QC_PULSE_INC_INT_MASK       		: 1;
		uint32_t QC_PULSE_DEC_INT_MASK    			: 1;
		uint32_t INVALID_PLUSS_INT_MASK     		: 1;
		uint32_t         							: 22;
	} BITS;
	uint32_t WORD;
} TS_DPDM_QC_SRC_CTRL;

typedef union {
	struct {
		uint32_t QC_SRC_STAT     					: 3;
		uint32_t QC_STATE        					: 4;
		uint32_t FIXED_5V_REQ_INT       			: 1;
		uint32_t FIXED_9V_REQ_INT      				: 1;
		uint32_t FIXED_12V_REQ_INT        			: 1;
		uint32_t FIXED_20V_REQ_INT		       		: 1;
		uint32_t CONTINUOUS_MODE_INT		       	: 1;
		uint32_t QC_PULSE_INC_INT       			: 1;
		uint32_t QC_PULSE_DEC_INT	    			: 1;
		uint32_t INVALID_PLUSS_INT		     		: 1;
		uint32_t         							: 17;
	} BITS;
	uint32_t WORD;
} TS_DPDM_QC_SRC_STAT;

typedef union {
	struct {
		uint32_t AFC_SOFT_RESET     				: 1;
		uint32_t PING_INT_MASK        				: 1;
		uint32_t AFC_RX_DATA_MASK       			: 1;
		uint32_t SCP_RX_DATA_MASK      				: 1;
		uint32_t CMPL_RTX_INT_MASK        			: 1;
		uint32_t RESET_RCVD_INT_MASK		       	: 1;
		uint32_t PARITY_ERR_INT_MASK		       	: 1;
		uint32_t CRC_ERR_INT_MASK       			: 1;
		uint32_t ABNORMOL_RTX_INT_MASK	    		: 1;
		uint32_t RX_LEN_ERR_INT_MASK		     	: 1;
		uint32_t PGTMOUT_INT_FLAG_MASK		       	: 1;
		uint32_t PING_SYNC_DISABLE       			: 1;
		uint32_t UI_CHECK_CONFIG	    			: 1;
		uint32_t UI98_CHECK_DISABLE		     		: 1;
		uint32_t         							: 18;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_CTRL;

typedef union {
	struct {
		uint32_t FAIL_CODE_NOTIF     				: 1;
		uint32_t VI_NOTSUPPORT_NOTIF        		: 1;
		uint32_t DATA_INCONSISTENT_NOTIF       		: 1;
		uint32_t         							: 29;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_NOTIF;


typedef union {
	struct {
		uint32_t IN_AFC_MODE_STAT     				: 1;
		uint32_t IN_SCP_MODE_STAT        			: 1;
		uint32_t AFC_MSG_RCV_CNT       				: 2;
		uint32_t PING_RCVD_INT_FLAG      			: 1;
		uint32_t AFC_RX_DATA_READY_FLAG        		: 1;
		uint32_t SCP_RX_DATA_READY_FLAG		       	: 1;
		uint32_t CMPL_RTX_INT_FLAG		       		: 1;
		uint32_t RESET_RCVD_INT_FLAG       			: 1;
		uint32_t PARITY_ERR_INT_FLAG	    		: 1;
		uint32_t CRC_ERR_INT_FLAG		     		: 1;
		uint32_t ABNORMOL_RTX_INT_FLAG       		: 1;
		uint32_t RX_LEN_ERR_INT_FLAG	    		: 1;
		uint32_t PGTMOUT_INT_FLAG_FLAG		     	: 1;
		uint32_t         							: 18;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_INT_FLAG;

typedef union {
	struct {
		uint32_t RX_LEN     						: 8;
		uint32_t RX_BUFFER_0        				: 8;
		uint32_t RX_BUFFER_1       					: 8;
		uint32_t RX_BUFFER_2      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_RX_0;

typedef union {
	struct {
		uint32_t RX_BUFFER_3     					: 8;
		uint32_t RX_BUFFER_4        				: 8;
		uint32_t RX_BUFFER_5       					: 8;
		uint32_t RX_BUFFER_6      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_RX_1;

typedef union {
	struct {
		uint32_t RX_BUFFER_7     					: 8;
		uint32_t RX_BUFFER_8        				: 8;
		uint32_t RX_BUFFER_9       					: 8;
		uint32_t RX_BUFFER_10      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_RX_2;

typedef union {
	struct {
		uint32_t TX_LEN     						: 8;
		uint32_t TX_BUFFER_0        				: 8;
		uint32_t TX_BUFFER_1       					: 8;
		uint32_t TX_BUFFER_2      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_TX_0;

typedef union {
	struct {
		uint32_t TX_BUFFER_3     					: 8;
		uint32_t TX_BUFFER_4        				: 8;
		uint32_t TX_BUFFER_5       					: 8;
		uint32_t TX_BUFFER_6      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_TX_1;

typedef union {
	struct {
		uint32_t TX_BUFFER_7     					: 8;
		uint32_t TX_BUFFER_8        				: 8;
		uint32_t TX_BUFFER_9       					: 8;
		uint32_t TX_BUFFER_10      					: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SOURCE_AFC_TX_2;

typedef struct {
	__IO TS_DPDM_SOURCE_CTRL 				SOURCE_CTRL; 	//4000_C080
	__IO TS_DPDM_SOURCE_STAT 				SOURCE_STAT; 	//4000_C084
	__IO TS_DPDM_HVDCP_CTRL 				HVDCP_CTRL; 	//4000_C088
	__IO TS_DPDM_HVDCP_FLAG 				HVDCP_FLAG; 	//4000_C08c
	__IO TS_DPDM_QC_SRC_CTRL 				QC_SRC_CTRL; 	//4000_C090
	__IO TS_DPDM_QC_SRC_STAT 				QC_SRC_FLAG; 	//4000_C094
	__IO TS_DPDM_SOURCE_AFC_CTRL 			AFC_CTRL; 		//4000_C098
	__IO TS_DPDM_SOURCE_AFC_NOTIF 			AFC_NOTIF; 		//4000_C09c
	__IO TS_DPDM_SOURCE_AFC_INT_FLAG 		AFC_INT_FLAG; 	//4000_C0a0
	__IO TS_DPDM_SOURCE_AFC_RX_0	 		AFC_RX_0; 		//4000_C0a4
	__IO TS_DPDM_SOURCE_AFC_RX_1 			AFC_RX_1; 		//4000_C0a8
	__IO TS_DPDM_SOURCE_AFC_RX_2 			AFC_RX_2; 		//4000_C0ac
	__IO TS_DPDM_SOURCE_AFC_TX_0 			AFC_TX_0; 		//4000_C0b0
	__IO TS_DPDM_SOURCE_AFC_TX_1 			AFC_TX_1; 		//4000_C0b4
	__IO TS_DPDM_SOURCE_AFC_TX_2 			AFC_TX_2; 		//4000_C0b8
} TS_DPDM;

#define  DPDM_APB_ADDR_OFFSET                (                         0xC080)
#define  DPDM_BASE                           (APB_BASE +  DPDM_APB_ADDR_OFFSET)
#define  DPDM                                (          (TS_DPDM *) DPDM_BASE)

void usb_dpdm_task_init(void);
void usb_dpdm_task_event_handler(uint32_t event);
void usb_dpdm_select(uint8_t tc_index);

#define DPDM_EVT_SRC_ATTACHED    		osal_event_declare(0)
#define DPDM_EVT_SRC_UNATTCHED    		osal_event_declare(1)
#define DPDM_EVT_ENTER_DCP    			osal_event_declare(2)
#define DPDM_EVT_ENTER_HVDCP    		osal_event_declare(3)

#define DPDM_EVT_QC_FIXED_5V    		osal_event_declare(4)
#define DPDM_EVT_QC_FIXED_9V    		osal_event_declare(5)
#define DPDM_EVT_QC_FIXED_12V    		osal_event_declare(6)
#define DPDM_EVT_QC_FIXED_20V    		osal_event_declare(7)

#define DPDM_EVT_QC_CONTINUES    		osal_event_declare(8)
#define DPDM_EVT_QC_PLUSE_INC    		osal_event_declare(9)
#define DPDM_EVT_QC_PLUSE_DEC    		osal_event_declare(10)

#define DPDM_EVT_AFC_RX_DATA    		osal_event_declare(11)
#define DPDM_EVT_SCP_RX_DATA    		osal_event_declare(12)
#define DPDM_EVT_SCP_TX_DATA    		osal_event_declare(13)

#define DPDM_EVT_SNK_ATTACHED    		osal_event_declare(14)
#define DPDM_EVT_SNK_UNATTCHED    		osal_event_declare(15)
#define DPDM_EVT_SNK_BC12DONE    		osal_event_declare(16)

#define DPDM_EVT_SNK_HVDCP_START    	osal_event_declare(17)
#define DPDM_EVT_SNK_HVDCP_DONE    		osal_event_declare(18)
#define DPDM_EVT_SNK_QC_START    		osal_event_declare(19)
#define DPDM_EVT_AFC_SCP_OUT    		osal_event_declare(20)



#define DPDM_EVT_TIMER_PERIOD    	osal_event_declare(31)




#endif /* DPDM_H_ */
