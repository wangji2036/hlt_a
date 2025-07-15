#ifndef _WPC_H_
#define _WPC_H_

typedef enum {
	ESYS_ERR_CODE_NONE                              = 0x00,
	ESYS_ERR_CODE_PING_PHASE_1ST_PKT_TYPE_ERR       = 0x01,
	ESYS_ERR_CODE_IDCFG_PHASE_WAIT_NEXT_PKT_ERR     = 0x02,
	ESYS_ERR_CODE_IDCFG_PHASE_WAIT_NEXT_PKT_TIMEOUT = 0x03,
	ESYS_ERR_CODE_IDCFG_PHASE_PKT_SEQUENCE_ERR      = 0x04,
	ESYS_ERR_CODE_IDCFG_PHASE_NO_THIS_PKT           = 0x05,
	ESYS_ERR_CODE_IDCFG_PHASE_ID_PKT_MAJOR_VER_ERR  = 0x06,
	ESYS_ERR_CODE_IDCFG_PHASE_CFG_PKT_CNT_ERR       = 0x07,
	ESYS_ERR_CODE_IDCFG_PHASE_PCH_PKT_TIME_ERR      = 0x08,
	ESYS_ERR_CODE_IDCFG_PHASE_BPP_REF_QVALUE_ERR    = 0x09,
	ESYS_ERR_CODE_IDCFG_PHASE_MPP_RESTRICTED_REP    = 0x0A,
	ESYS_ERR_CODE_XFER_PHASE_CEP_TIMEOUT            = 0x0B,
	ESYS_ERR_CODE_XFER_PHASE_RPP_TIMEOUT            = 0x0C,
	ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT            = 0x0D,
	ESYS_ERR_CODE_XFER_PHASE_EPP_RECVD_BIT8RPP      = 0x0E,
	ESYS_ERR_CODE_XFER_PHASE_RECVD_SSP              = 0x0F,
	ESYS_ERR_CODE_XFER_PHASE_POWER_LOSS_FOD         = 0x10,
	ESYS_ERR_CODE_NEGO_PHASE_WAIT_NEXT_PKT_TIMEOUT  = 0x11,
	ESYS_ERR_CODE_NEGO_PHASE_NO_THIS_PKT            = 0x12,
	ESYS_ERR_CODE_NEGO_PHASE_FODS_REF_QVALUE_ERR    = 0x13,
	ESYS_ERR_CODE_NEGO_PHASE_RPP_TYPE_NOT_UPDATE    = 0x14,
	ESYS_ERR_CODE_RECVD_EPT_PKT                     = 0x15,
	ESYS_ERR_CODE_PING_PHASE_WAIT_1ST_PKT_TIMEOUT   = 0x16,
	ESYS_ERR_CODE_CALI_PHASE_CALIBRATION_TIMEOUT    = 0x17,
	ESYS_ERR_CODE_CALI_PHASE_BIT24RPP_MODE_ERR      = 0x18,
	ESYS_ERR_CODE_XFER_PHASE_FODS_PP_REF_QVALUE_ERR = 0x19,
	ESYS_ERR_CODE_RENOGO_PHASE_TIMEOUT_ERR          = 0x1A,
	ESYS_ERR_CODE_PING_PHASE_REPING                 = 0x1E,
	ESYS_ERR_CODE_PING_PHASE_OCP                    = 0x1F,
	ESYS_ERR_CODE_NTC_OTP                           = 0x20,
	ESYS_ERR_CODE_NTC_UTP                           = 0x21,
	ESYS_ERR_CODE_FMW_OVP                           = 0x22,
	ESYS_ERR_CODE_FMW_UVP                           = 0x23,
	ESYS_ERR_CODE_FMW_OCP                           = 0x24,
	ESYS_ERR_CODE_FMW_OPP                           = 0x26,
	ESYS_ERR_CODE_DIE_OTP                           = 0x28,
	ESYS_ERR_CODE_DIE_OTP_H                         = 0x29,
	ESYS_ERR_CODE_OVP_H                             = 0x2A,
	ESYS_ERR_CODE_UVP_H                             = 0x2B,
	ESYS_ERR_CODE_OCP_H                             = 0x2C,
	ESYS_ERR_CODE_DIE_UTP                           = 0x2D,
	ESYS_ERR_CODE_ADP_TYPE_CHANGED                  = 0x30,
	ESYS_ERR_CODE_APL_STD_FC_REPING                 = 0x31,
	ESYS_ERR_CODE_APL_MAG_FC_REPING                 = 0x32,
	ESYS_ERR_CODE_APL_MAG_NEGO_TIMEOUT              = 0x33,
	ESYS_ERR_CODE_MST_NOT_READY                     = 0x40,

	ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT             = 0x41,
	ESYS_ERR_CODE_CLOAK_SWITCH			            = 0x42,
	ESYS_ERR_CODE_CLOAK_PHASE_NO_THIS_PKT           = 0x43,
	ESYS_ERR_CODE_MPP_ILLEGAL_PKT                   = 0x55,
	ESYS_ERR_CODE_NEED_QDT_CALIBRATION              = 0x60,
	ESYS_ERR_CODE_DIGITAL_REPING                    = 0x61,
	ESYS_ERR_CODE_TYPEC_CHANGE                   	= 0x70,
} TE_SYS_ERR_CODE;

//typedef enum {
//	EWPC_IDLE_STAT_STANDBY = 0,
//	EWPC_IDLE_STAT_XER_COM = 1,
//	EWPC_IDLE_STAT_XER_FOD = 2,
//	EWPC_IDLE_STAT_QDT_FOD = 3,
//	EWPC_IDLE_STAT_LAR_MET = 4,
//	EWPC_IDLE_STAT_EPT_ERR = 5,
//	EWPC_IDLE_STAT_EPT_RES = 6,
//	EWPC_IDEL_STAT_EPT_REP = 7,
//} TE_WPC_IDLE_STAT;

enum EPT_CODE {
	EPT_CODE_00_Unknown              = 0x00,
	EPT_CODE_01_ChargeComplete       = 0x01,
	EPT_CODE_02_InternalFault        = 0x02,
	EPT_CODE_03_OverTemperature      = 0x03,
	EPT_CODE_04_OverVoltage          = 0x04,
	EPT_CODE_05_OverCurrent          = 0x05,
	EPT_CODE_06_BatteryFailure       = 0x06,
	EPT_CODE_07_NA                   = 0x07,
	EPT_CODE_08_NoResponse           = 0x08,
	EPT_CODE_09_NA                   = 0x09,
	EPT_CODE_0A_NegotiationFailure   = 0x0A,
	EPT_CODE_0B_RestartPowerTransfer = 0x0B,
	EPT_CODE_0C_RePing               = 0x0C,
};

#define T_PING             70
#define T_NEXT             23
#define T_FIRST_LIMIT                 (   20)
#define T_MAX_LIMIT                   (  170)
#define T_NEGOTIATE       400
#define T_RENEGOTIATE       814
#define T_RENEGO_TO        1900
#define T_TERMINATE        10
#define T_RESPONSE          3
#define T_WINDOW            4
#define T_ACTIVE           20
#define T_XCE_RESP_TO      20
#define T_COM_CE_TO    1600
#define T_COM_DDM_TO    600
#define T_MPP_CE_TO    2050
#define T_COM_RP_TO   20000
#define T_MPP_RP_TO    7900

#define T_PCH_TIME_MIN      5
#define T_PCH_TIME_MAX    100

#define T_CLOAK_PING        100
#define T_CLOAK_TIMEOUT     239
#define T_CLOAK_TIMEOUT_EX  500

#define PRX_ID71_PKT_EXT_BIT_MSK    (0x80000000)

enum power_profile_mode_t
{
	BPP = 0,
	EPP = 1,
	MPP = 2,
};

enum fsk_prmbl_t
{
	FSK_PRMBL_NONE = 0,
	FSK_PRMBL_NEED = 1,
};

enum ping_type_t
{
	qdt_ping = 0,
	dig_ping = 1,
	det_ping = 2,
};

enum dig_ping_type_t//need add _128K_HB_HIGH/LOW, _128K_FB, _360K_FB_HIGH/NOMINAL
{
	_128K_HB = 0,
	_360K_FB = 1,
};

enum mpp_power_mode_t
{
	continuous 	= 0,//CPM
	nominal 	= 1,//NPM
	light		= 2,//LPM
	high 		= 3,//HPM
};

enum ptx_protocol_phase_t {
	WPC_PHASE_IDLE = 0,
	WPC_PHASE_PING = 1,
	WPC_PHASE_CNFG = 2,
	WPC_PHASE_NEGO = 3,
	WPC_PHASE_XFER = 4,
	WPC_PHASE_CLOAK= 5,
};

enum ptx_idle_phase_state_t {
	WPC_IDLE_STAT_STANDBY = 0,
	WPC_IDLE_STAT_XER_COM = 1,
	WPC_IDLE_STAT_XER_FOD = 2,
	WPC_IDLE_STAT_QDT_FOD = 3,
	WPC_IDLE_STAT_LAR_MET = 4,
	WPC_IDLE_STAT_EPT_ERR = 5,
	WPC_IDLE_STAT_EPT_RES = 6,
	WPC_IDLE_STAT_EPT_REP = 7,
	WPC_IDLE_STAT_CLOAKING = 8,
	WPC_IDLE_STAT_QDT_CALI = 9,
};

enum
{
	_NU103x_DM_PHASE_DIG_PING = 0,
	_NU103x_DM_PHASE_LO_POWER = 1,
	_NU103x_DM_PHASE_HI_POWER = 2,
};

enum
{
	power_limit_reason_no = 0,
	power_limit_reason_rsv1 = 1,
	power_limit_reason_fop = 2,
	power_limit_reason_bop = 3,
	power_limit_reason_ot = 4,
	power_limit_reason_rsv5 = 5,
	power_limit_reason_oc = 6,
	power_limit_reason_map = 7,
};

#define EPP_END_NEGO_FLAG    0x01
#define MPP_END_NEGO_FLAG    0x02

uint8_t wpc_msg_size_get(uint8_t hdr);
void wpc_task_init(void);
void wpc_task_event_handler(uint32_t event);
void wpc_stop_to_idle(uint8_t err_code);

#endif /* _WPC_H_ */
