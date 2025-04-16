#ifndef OSAL_H_
#define OSAL_H_

enum {
	APP_010ms_TIMER = 0,
	APP_100ms_TIMER = 1,
	APP_250ms_TIMER = 2,

	USB_TIMER = 3,

	WPC_PING_TIMER = 4,
	WPC_NEXT_TIMER = 5,
	WPC_RESP_TIMER = 6,
	WPC_NEGO_TIMER = 7, WPC_AUTH_TIMER = 7,
	WPC_CEP_TIMER  = 8,
	WPC_RPP_TIMER  = 9,
	WPC_DDM_TIMER  =10,

	USB_TC_PD_TIMER = 11,
	USB_BC12_TIMER = 12,
	USB_QC_TIMER = 13,

	BUCKBOOST_PERIOD_TIMER = 14,
	BUCKBOOST_REGULATOR_TIMER = 15,

	DPDM_SINK_TIMER = 16,
	TCPM_PORT0_TIMER = 17,
	TCPM_PORT1_TIMER = 18,
	TCPM_CHG_TIMER = 19,
	TCPM_USB_A_TIMER = 20,
	TCPM_PSREADY_TIMER = 21,
	GAUGE_TIMER   = 22,

	BUCKBOOST_VBUS_TIMER = 23,

	PORT_ENUM_TIMER = 24,
	PORT_CONNECT_TIMER = 25,
	BUCKBOOST_ADC_TIMER = 26,

	BUCKBOOST_CHAGER_TIMER = 27,

	/////////////
	MAX_TIMER,
};

enum {
	HAL_TASK = 0,
	FML_TASK = 1,
	USB_TASK = 2,
	WPC_TASK = 3,
	APL_TASK = 4,
	BUCKBOOST_TASK = 5,
	USB_DPDM_TASK = 6,

	PORT_MANAGER_TASK = 7,
	/////////////
	MAX_TASK,
};

#define osal_event_declare(nr)    (1 << nr)

#define WPC_EVT_DIG_PING          osal_event_declare(0)
#define WPC_EVT_PIN_NO_PKT        osal_event_declare(1)
#define WPC_EVT_HDR_START         osal_event_declare(2)
#define WPC_EVT_HDR_RECVD         osal_event_declare(3)
#define WPC_EVT_PKT_RECVD         osal_event_declare(4)
#define WPC_EVT_PING_1st_PKT_TO   osal_event_declare(5)
#define WPC_EVT_STOP_POWER        osal_event_declare(6)
#define WPC_EVT_CNFG_NEXT_1ST_TO  osal_event_declare(7)
#define WPC_EVT_CNFG_NEXT_PKT_TO  osal_event_declare(8)
#define WPC_EVT_CEP_TO            osal_event_declare(9)
#define WPC_EVT_RPP_TO            osal_event_declare(10)
#define WPC_EVT_PCH_TO            osal_event_declare(11)
#define WPC_EVT_1ST_WND           osal_event_declare(12)
#define WPC_EVT_2ND_WND           osal_event_declare(13)
#define WPC_EVT_3RD_WND           osal_event_declare(14)
#define WPC_EVT_4TH_WND           osal_event_declare(15)
#define WPC_EVT_PFOD              osal_event_declare(16)
//#define WPC_EVT_RSP               osal_event_declare(17)
#define WPC_EVT_NEGO_NEXT_PKT_TO  osal_event_declare(18)
#define WPC_EVT_FSK_RESP_DONE     osal_event_declare(19)
#define WPC_EVT_SE_IC_TBS_AUTH    osal_event_declare(20)
#define WPC_EVT_CLOAK_PING        osal_event_declare(21)
#define WPC_EVT_STOP_AFTER_FSK    osal_event_declare(22)
#define WPC_EVT_FOD_REPORTED	  osal_event_declare(23)
#define WPC_EVT_RENEGO_TO         osal_event_declare(24)

void osal_init(void);
void osal_start_system(void);
void osal_set_event(uint8_t task_id, uint32_t event);
void osal_clear_event(uint8_t task_id, uint32_t event);
void osal_start_timerEx(uint8_t timer_id, uint16_t timeout, uint16_t period, uint8_t task_id, uint32_t event);
void osal_stop_timerEx(uint8_t timer_id);
void osal_task_handler_reg(uint8_t task_id, void (*handler)(uint32_t));
void osal_mem_copy(void *dst, const void *src, int len);
void osal_mem_set(void *mem, uint8_t val, int len);
void osal_mem_clear(void *mem, int len);

#endif /* OSAL_H_ */
