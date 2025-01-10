#ifndef USB_QC_H_
#define USB_QC_H_

typedef union {
	struct {
		uint32_t      								: 3;
		uint32_t DCD_TIMEOUT_INT_STAT        		: 1;
		uint32_t        							: 3;
		uint32_t BC1P2_DET_DONE_INT_STAT      		: 1;
		uint32_t UNSTANDARD_TYPE        			: 3;
		uint32_t        							: 1;
		uint32_t BC1P2_TYPE       					: 3;
		uint32_t        							: 17;
	} BITS;
	uint32_t WORD;
} TS_DPDM_BC1P2_STAT;

typedef union {
	struct {
		uint32_t      								: 3;
		uint32_t DCD_TIMEOUT_INT_FLAG        		: 1;
		uint32_t        							: 3;
		uint32_t BC1P2_DET_DONE_INT_FLAG      		: 1;
		uint32_t        							: 24;
	} BITS;
	uint32_t WORD;
} TS_DPDM_BC1P2_INT_FLAG;

typedef union {
	struct {
		uint32_t      								: 1;
		uint32_t BC1P2_EN        					: 1;
		uint32_t        							: 1;
		uint32_t NSD_SKIP      						: 1;
		uint32_t DPDM_PRI_THR     					: 1;
		uint32_t DPDM_EN							: 1;
		uint32_t         							: 1;
		uint32_t DCD_TIMEOUT     					: 1;
		uint32_t       								: 3;
		uint32_t DCD_TIMEOUT_INT_MASK     			: 1;
		uint32_t       								: 3;
		uint32_t BC1P2_DET_DONE_INT_MASK     		: 1;
		uint32_t      								: 16;
	} BITS;
	uint32_t WORD;
} TS_DPDM_BC1P2_INTMSK_CTRL;

typedef union {
	struct {
		uint32_t DM_COT_PULSE_DONE_INT_STAT   		: 1;
		uint32_t DP_COT_PULSE_DONE_INT_STAT        	: 1;
		uint32_t DPDM_2PULSE_DONE_INT_STAT       	: 1;
		uint32_t DPDM_3PULSE_DONE_INT_STAT      	: 1;
		uint32_t DM_16PULSE_DONE_INT_STAT     		: 1;
		uint32_t DP_16PULSE_DONE_INT_STAT			: 1;
		uint32_t HVDCP_DET_FAIL_INT_STAT       		: 1;
		uint32_t HVDCP_DET_OK_INT_STAT       		: 1;
		uint32_t      								: 24;
	} BITS;
	uint32_t WORD;
} TS_DPDM_QC_INT_STAT;

typedef union {
	struct {
		uint32_t DM_COT_PULSE_DONE_INT_FLAG   		: 1;
		uint32_t DP_COT_PULSE_DONE_INT_FLAG        	: 1;
		uint32_t DPDM_2PULSE_DONE_INT_FLAG       	: 1;
		uint32_t DPDM_3PULSE_DONE_INT_FLAG      	: 1;
		uint32_t DM_16PULSE_DONE_INT_FLAG     		: 1;
		uint32_t DP_16PULSE_DONE_INT_FLAG			: 1;
		uint32_t HVDCP_DET_FAIL_INT_FLAG       		: 1;
		uint32_t HVDCP_DET_OK_INT_FLAG       		: 1;
		uint32_t      								: 24;
	} BITS;
	uint32_t WORD;
} TS_DPDM_QC_INT_FLAG;


typedef union {
	struct {
		uint32_t QC_FALLING_DET_TIME   				: 1;
		uint32_t QC3P5_PULSE        				: 3;
		uint32_t QC_MODE       						: 2;
		uint32_t QC_COMMAND      					: 1;
		uint32_t QC_EN     							: 1;
		uint32_t DM_COT_PULSE_DONE_INT_MASK			: 1;
		uint32_t DP_COT_PULSE_DONE_INT_MASK       	: 1;
		uint32_t DPDM_2PULSE_DONE_INT_MASK   		: 1;
		uint32_t DPDM_3PULSE_DONE_INT_MASK        	: 1;
		uint32_t DM_16PULSE_DONE_INT_MASK       	: 1;
		uint32_t DP_16PULSE_DONE_INT_MASK      		: 1;
		uint32_t HVDCP_DET_FAIL_INT_MASK     		: 1;
		uint32_t HVDCP_DET_OK_INT_MASK				: 1;
		uint32_t        							: 16;
	} BITS;
	uint32_t WORD;
} TS_DPDM_QC_INTMSK_CTRL;

typedef union {
	struct {
		uint32_t DPDM_COT_PULSE   					: 7;
		uint32_t PULSE_INACTIVE_TIME        		: 1;
		uint32_t      								: 24;
	} BITS;
	uint32_t WORD;
} TS_DPDM_DPDM_COT_PULSE;


typedef union {
	struct {
		uint32_t DPDM_Manual_EN   					: 1;
		uint32_t         							: 2;
		uint32_t DP_SRC_10UA       					: 1;
		uint32_t DP_SINK_EN      					: 1;
		uint32_t DM_SINK_EN      					: 1;
		uint32_t DM_20K_PD_EN     					: 1;
		uint32_t DPDM_500K_PD_EN					: 1;
		uint32_t        							: 1;
		uint32_t DP_BUFF_EN   						: 1;
		uint32_t DM_BUFF_EN   						: 1;
		uint32_t DP_BUF        						: 2;
		uint32_t DM_BUF      						: 2;
		uint32_t         							: 1;
		uint32_t VDP_RD								: 3;
		uint32_t VDM_RD       						: 3;
		uint32_t DP_RD_EN   						: 1;
		uint32_t DM_RD_EN   						: 1;
		uint32_t        							: 8;
	} BITS;
	uint32_t WORD;
} TS_DPDM_DPDM_MANUAL;

typedef union {
	struct {
		uint32_t BCQC_DGB_SEL   					: 8;
		uint32_t      								: 24;
	} BITS;
	uint32_t WORD;
} TS_DPDM_BCQC_DBG_SEL;

typedef struct {
	__IO TS_DPDM_BC1P2_STAT 					BC1P2_STAT; 		//4000_C000
	__IO TS_DPDM_BC1P2_INT_FLAG 				BC1P2_INT_FLAG; 	//4000_C004
	__IO TS_DPDM_BC1P2_INTMSK_CTRL 				BC1P2_INTMSK_CTRL; 	//4000_C008
	__IO TS_DPDM_QC_INT_STAT 					QC_INT_STAT; 		//4000_C00C
	__IO TS_DPDM_QC_INT_FLAG 					QC_INT_FLAG; 		//4000_C010
	__IO TS_DPDM_QC_INTMSK_CTRL 				QC_INTMSK_CTRL; 	//4000_C014
	__IO TS_DPDM_DPDM_COT_PULSE 				DPDM_COT_PULSE; 	//4000_C018
	__IO TS_DPDM_DPDM_MANUAL 					DPDM_MANUAL; 		//4000_C01C
	__IO TS_DPDM_BCQC_DBG_SEL 					BCQC_DBG_SEL; 		//4000_C020
} TS_DPDM_QC_SINK;


#define  DPDM_QC_SINK_OFFSET                	(0xC000)
#define  DPDM_QC_SINK_BASE                  	(APB_BASE +  DPDM_QC_SINK_OFFSET)
#define  DPDM_QC_SINK                       	((TS_DPDM_QC_SINK *) DPDM_QC_SINK_BASE)


void dpdm_sink_init(void);
void dpdm_sink_deinit(void);
void qc2_set_volt(uint16_t qc_volt);


#endif /* USB_QC_H_ */
