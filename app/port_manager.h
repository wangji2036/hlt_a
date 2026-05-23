#ifndef PORT_MANAGER_H_
#define PORT_MANAGER_H_

#include "typdef.h"
#include "osal.h"
#include "config.h"

#if SUPPORT_PORTMGR_LOG
#define pm_printk printk
#else
#define pm_printk(...)
#endif

#define PORT_ENUM_EVT_PORT0_CONNECT_START osal_event_declare(0)
#define PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS osal_event_declare(1)
#define PORT_ENUM_EVT_PORT0_CONNECT_CLOSED osal_event_declare(2)
#define PORT_ENUM_EVT_PORT0_SINK_SETVOLT osal_event_declare(3)
#define PORT_ENUM_EVT_PORT0_SINK_SETCHARGE osal_event_declare(4)
#define PORT_ENUM_EVT_PORT0_ENUM_DONE osal_event_declare(5)

#define PORT_ENUM_EVT_PORT1_CONNECT_START osal_event_declare(6)
#define PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS osal_event_declare(7)
#define PORT_ENUM_EVT_PORT1_CONNECT_CLOSED osal_event_declare(8)
#define PORT_ENUM_EVT_PORT1_SINK_SETVOLT osal_event_declare(9)
#define PORT_ENUM_EVT_PORT1_SINK_SETCHARGE osal_event_declare(10)
#define PORT_ENUM_EVT_PORT1_ENUM_DONE osal_event_declare(11)

#define PORT_ENUM_EVT_PORT2_CONNECT_START osal_event_declare(12)
#define PORT_ENUM_EVT_PORT2_CONNECT_SUCCESS osal_event_declare(13)
#define PORT_ENUM_EVT_PORT2_CONNECT_CLOSED osal_event_declare(14)
#define PORT_ENUM_EVT_PORT2_ENUM_DONE osal_event_declare(17)

#define PORT_ENUM_EVT_PORT3_CONNECT_START osal_event_declare(18)
#define PORT_ENUM_EVT_PORT3_CONNECT_SUCCESS osal_event_declare(19)
#define PORT_ENUM_EVT_PORT3_CONNECT_CLOSED osal_event_declare(20)
#define PORT_ENUM_EVT_PORT3_ENUM_DONE osal_event_declare(23)
#define PORT_ENUM_EVT_USB_BRIDGE_CLOSED osal_event_declare(24)

#define PORT_ENUM_EVT_PORT_SCAN osal_event_declare(31)

enum port_state_e
{
	PORT_IDLE_OR_READY = 0,
	PORT_INHANDLING,
};

#define PORT_ENUM_PERIOD 1

#define CHG_IBUS_MIN 500
#define CHG_IBAT_MIN 200

#define PORT0_INDEX 0x00
#define PORT1_INDEX 0x01
#define USBA_INDEX 0x02
#define WPC_INDEX 0x03

#define PORT2_INDEX USBA_INDEX
#define PORT3_INDEX WPC_INDEX

#define DPDM_PHY_OFF 0xFF
#define PORT_STATE_NONE 0x00
#define PORT_STATE_SOURCE 0x01
#define PORT_STATE_SINK 0x02

struct port_infos
{
	uint8_t port_state[4]; //typec0
	uint8_t incharge_port;
	uint8_t inhandle_port;

	uint8_t snk_5v_only;
	bool is_mini_current_mode;
	uint16_t light0_cnt;
	uint8_t light1_cnt;
	uint32_t port_event;
	uint16_t ibat_limit;
	uint16_t ibus_limit;
	uint16_t prot_ibus;
	uint16_t snk_set_volt;
	uint32_t adpater_power;
	enum port_state_e state;
};

#define BIT(n) (0x01ul << n)

#define PORT0_EVENT_TRY_CONNECT BIT(0)
#define PORT1_EVENT_TRY_CONNECT BIT(1)
#define PORT2_EVENT_TRY_CONNECT BIT(2)
#define PORT3_EVENT_TRY_CONNECT BIT(3)

#define PORT0_EVENT_UNCONNECT BIT(4)
#define PORT1_EVENT_UNCONNECT BIT(5)
#define PORT2_EVENT_UNCONNECT BIT(6)
#define PORT3_EVENT_UNCONNECT BIT(7)

#define PORT_EVENT_RESET_CHARGE BIT(8)
struct port_infos g_port;

void port_manager_task_init(void);
void port_manager_set_event(uint32_t event);
void port_manager_event_handle(uint32_t event);

void port_enum_scan_handle(void);
void port_enum_port0_connect_start(void);
void port_enum_port1_connect_start(void);
void port_enum_port2_connect_start(void);
void port_enum_port0_connect_success(void);
void port_enum_port1_connect_success(void);
void port_enum_port2_connect_success(void);
void port_enum_port_snk_setvolt(void);
void port_enum_port_snk_setcharge(void);
void port_enum_port_enum_done(void);
void port_enum_port0_connect_closed(void);
void port_enum_port1_connect_closed(void);
void port_enum_port2_connect_closed(void);

#endif /* FML_H_ */
