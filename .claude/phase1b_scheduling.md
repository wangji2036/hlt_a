# Phase 1B: Scheduling System Discovery Report

## 1. Scheduling Architecture Type

**Type: Bare-metal cooperative OSAL (Operating System Abstraction Layer) with interrupt-driven event delivery**

The system uses a **custom lightweight OSAL** running on a bare-metal MCU (Novltatech NU17112). There is **no RTOS** (no FreeRTOS, no RT-Thread). The architecture consists of:

- A **super-loop** (`for(;;)`) in `osal_start_system()` (`osal/osal.c:249`)
- A **software timer** subsystem that generates events on timeout
- A **cooperative task dispatcher** that iterates through registered tasks and dispatches pending events
- **Hardware interrupts** that set OSAL events to be processed in the main loop context

Key evidence:
- `osal_start_system()` at `osal/osal.c:247-255`: `for(;;) { hal_wdt_feed(); osal_timer_update(); osal_event_handle(); }`
- `main()` at `app/main.c:134`: calls `osal_start_system()` which never returns
- No `xTaskCreate`, `osThreadNew`, or any RTOS API found

---

## 2. Task/Thread Complete List

All tasks are registered via `osal_task_handler_reg()`. Maximum tasks: `MAX_TASK = 8` (`osal/osal.h:61`).

| Task ID | Task Name | Enum Value | Entry/Handler Function | Init Function | File:Line (reg) |
|---------|-----------|------------|----------------------|---------------|-----------------|
| 0 | HAL_TASK | 0 | (not registered, reserved) | N/A | `osal/osal.h:51` |
| 1 | FML_TASK | 1 | `fml_task_event_handler()` | `fml_task_init()` | `fml/_fml.c:20` |
| 2 | USB_TASK | 2 | `tcpm_task_event_handler()` | `tcpm_task_init()` | `fml/tcpm.c:111` |
| 3 | WPC_TASK | 3 | `wpc_task_event_handler()` | `wpc_task_init()` | `app/_wpc.c:364` |
| 4 | APL_TASK | 4 | `apl_task_event_handler()` | `apl_task_init()` | `app/app.c:45` |
| 5 | BUCKBOOST_TASK | 5 | `buckboost_task_event_handler()` | `buckboost_task_init()` | `power/buckboost.c:178` |
| 6 | USB_DPDM_TASK | 6 | `usb_dpdm_task_event_handler()` | `usb_dpdm_task_init()` | `fml/dpdm.c:29` |
| 7 | PORT_MANAGER_TASK | 7 | `port_manager_event_handle()` | `port_manager_task_init()` | `app/port_manager.c:39` |

Task init order in `main()` (`app/main.c:120-131`):
1. `osal_init()` -> `apl_task_init()` -> `buckboost_task_init()` -> `tcpm_task_init()` -> `usb_dpdm_task_init()` -> `fml_task_init()` -> `wpc_task_init()` (conditional) -> `port_manager_task_init()`

---

## 3. Timer Complete List

Maximum timers: `MAX_TIMER = 31` (APP_005ms_TIMER=30, then MAX_TIMER). Defined in `osal/osal.h:5-48`.

| Timer ID | Timer Name | Period (ms) | Associated Task | Event | Init Location |
|----------|-----------|-------------|-----------------|-------|---------------|
| 0 | APP_010ms_TIMER | 10 | APL_TASK | APL_EVT_010ms_POLL | `app/app.c:48` |
| 1 | APP_100ms_TIMER | 100 | APL_TASK | APL_EVT_100ms_POLL | `app/app.c:47` |
| 2 | APP_250ms_TIMER | 250 | APL_TASK | APL_EVT_250ms_POLL | `app/app.c:46` |
| 3 | USB_TIMER | varies | USB_TASK | varies | dynamic |
| 4 | WPC_PING_TIMER | ap->t_next_ping | WPC_TASK | WPC_EVT_DIG_PING | `app/_wpc.c:365` |
| 5 | WPC_NEXT_TIMER | varies | WPC_TASK | varies (1st pkt TO, etc) | dynamic |
| 6 | WPC_RESP_TIMER | varies | WPC_TASK | varies | dynamic |
| 7 | WPC_NEGO_TIMER / WPC_AUTH_TIMER | varies | WPC_TASK | varies | dynamic |
| 8 | WPC_CEP_TIMER | T_COM_CE_TO | WPC_TASK | WPC_EVT_CEP_TO | dynamic |
| 9 | WPC_RPP_TIMER | T_COM_RP_TO | WPC_TASK | WPC_EVT_RPP_TO | dynamic |
| 10 | WPC_DDM_TIMER | varies | WPC_TASK | WPC_EVT_DDM | dynamic |
| 11 | USB_TC_PD_TIMER | 1 (periodic) | USB_TASK | TCPM_EVT_TIME_PERIOD | `fml/tcpm.c:112` |
| 12 | USB_BC12_TIMER | 100 (periodic) | USB_DPDM_TASK | DPDM_EVT_TIMER_PERIOD | `fml/dpdm.c:30` |
| 13 | USB_QC_TIMER | varies | USB_DPDM_TASK | varies | dynamic |
| 14 | BUCKBOOST_PERIOD_TIMER | 17 (periodic) | BUCKBOOST_TASK | BUCKBOOST_EVT_TIME_PERIOD | `power/buckboost.c:179` |
| 15 | BUCKBOOST_REGULATOR_TIMER | varies | BUCKBOOST_TASK | BUCKBOOST_EVT_REGULATOR_* | dynamic |
| 16 | DPDM_SINK_TIMER | varies | USB_DPDM_TASK | varies | dynamic |
| 17 | TCPM_PORT0_TIMER | varies | USB_TASK | varies | dynamic |
| 18 | TCPM_PORT1_TIMER | varies | USB_TASK | varies | dynamic |
| 19 | TCPM_CHG_TIMER | varies | USB_TASK | varies | dynamic |
| 20 | TCPM_USB_A_TIMER | varies | USB_TASK | TCPM_EVT_USBA_DETEN | dynamic |
| 21 | TCPM_PSREADY_TIMER | varies | USB_TASK | varies | dynamic |
| 22 | GAUGE_TIMER | 100 (periodic) | FML_TASK | APL_EVT_GAUGE | `fml/_fml.c:21` |
| 23 | BUCKBOOST_VBUS_TIMER | 20 (periodic) | BUCKBOOST_TASK | BUCKBOOST_EVT_VBUS_PERIOD | `power/buckboost.c:180` |
| 24 | PORT_ENUM_TIMER | 1 (periodic) | PORT_MANAGER_TASK | PORT_ENUM_EVT_PORT_SCAN | `app/port_manager.c:40` |
| 25 | PORT_CONNECT_TIMER | varies | PORT_MANAGER_TASK | varies | dynamic |
| 26 | BUCKBOOST_ADC_TIMER | varies | BUCKBOOST_TASK | BUCKBOOST_EVT_ADC_PERIOD | dynamic |
| 27 | BUCKBOOST_CHAGER_TIMER | 500 (periodic) | BUCKBOOST_TASK | BUCKBOOST_EVT_CHAG_PERIOD | `power/buckboost.c:181` |
| 28 | BUCKBOOST_VBUS_DISG_TIMER | 300 (one-shot) | BUCKBOOST_TASK | BUCKBOOST_EVT_DUMMYLOADOVER | dynamic |
| 29 | USB_WB7720_TIMER | 47 (periodic) | FML_TASK | APL_HID_REPORT | `fml/_fml.c:22` |
| 30 | APP_005ms_TIMER | 5 | APL_TASK | APL_EVT_005ms_POLL | `app/app.c:49` |

Timer period constants (`power/buckboost.h:108-110`):
- `BUCKBOOST_TIME_PERIOD = 17` ms
- `BUCKBOOST_VBUS_PERIOD = 20` ms
- `BUCKBOOST_CHAG_PERIOD = 500` ms
- `T_GAUGE = 100` ms (`fml/_fml.h:11`)
- `PORT_ENUM_PERIOD = 1` ms (`app/port_manager.h:42`)

---

## 4. Event/Signal Complete List

Events are bitmask values defined via `osal_event_declare(nr)` = `(1 << nr)`. Each task has its own 32-bit event space.

### APL_TASK Events (`app/app.h:7-10`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| APL_EVT_010ms_POLL | bit 0 | APP_010ms_TIMER | `apl_task_event_handler()` |
| APL_EVT_100ms_POLL | bit 1 | APP_100ms_TIMER | `apl_task_event_handler()` |
| APL_EVT_250ms_POLL | bit 2 | APP_250ms_TIMER | `apl_task_event_handler()` |
| APL_EVT_005ms_POLL | bit 3 | APP_005ms_TIMER | `apl_task_event_handler()` |

### USB_TASK (TCPM) Events (`fml/tcpm.h:97-110`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| TCPM_EVT_TIME_PERIOD | bit 0 | USB_TC_PD_TIMER (1ms periodic) | `tcpm_task_event_handler()` -> `pdlib_run()` |
| TCPM_EVT_USBA_SCAN | bit 11 | `osal_set_event()` from BUCKBOOST_TASK | USB-A detection scan |
| TCPM_EVT_USBA_REDETECT | bit 12 | `osal_set_event()` from PORT_MANAGER | USB-A re-detection |
| TCPM_EVT_USBA_WORK | bit 13 | dynamic | USB-A work |
| TCPM_EVT_USBA_DETEN | bit 14 | TCPM_USB_A_TIMER | USB-A detect enable |
| TCPM_EVT_QI_WORK | bit 15 | dynamic | Qi wireless work |
| TCPM_EVT_PD_READY | bit 16 | dynamic | PD ready |
| TCPM_EVT_QI_SET_VOLT | bit 17 | dynamic | Set Qi voltage |
| TCPM_EVT_HVDCP_DONE | bit 18 | dynamic | HVDCP negotiation done |
| TCPM_EVT_DPDM_DONE | bit 19 | `osal_set_event()` from DPDM task | DPDM protocol done |

### WPC_TASK Events (`osal/osal.h:66-92`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| WPC_EVT_DIG_PING | bit 0 | WPC_PING_TIMER | Start digital ping |
| WPC_EVT_PIN_NO_PKT | bit 1 | WPC_NEXT_TIMER | No packet timeout |
| WPC_EVT_HDR_START | bit 2 | ASK decode ISR | Header start detected |
| WPC_EVT_HDR_RECVD | bit 3 | ASK decode | Header received |
| WPC_EVT_PKT_RECVD | bit 4 | ASK decode | Packet received |
| WPC_EVT_PING_1st_PKT_TO | bit 5 | WPC_NEXT_TIMER | First packet timeout |
| WPC_EVT_STOP_POWER | bit 6 | various error paths | Stop power transfer |
| WPC_EVT_CNFG_NEXT_1ST_TO | bit 7 | timer | Config phase next 1st timeout |
| WPC_EVT_CNFG_NEXT_PKT_TO | bit 8 | timer | Config phase next packet timeout |
| WPC_EVT_CEP_TO | bit 9 | WPC_CEP_TIMER | CEP timeout |
| WPC_EVT_RPP_TO | bit 10 | WPC_RPP_TIMER | RPP timeout |
| WPC_EVT_PCH_TO | bit 11 | timer | Power control hold timeout |
| WPC_EVT_1ST_WND | bit 12 | timer | 1st window |
| WPC_EVT_2ND_WND | bit 13 | timer | 2nd window |
| WPC_EVT_3RD_WND | bit 14 | timer | 3rd window |
| WPC_EVT_4TH_WND | bit 15 | timer | 4th window |
| WPC_EVT_PFOD | bit 16 | timer | Power FOD check |
| WPC_EVT_NEGO_NEXT_PKT_TO | bit 18 | timer | Negotiation timeout |
| WPC_EVT_FSK_RESP_DONE | bit 19 | FSK ISR | FSK response complete |
| WPC_EVT_SE_IC_TBS_AUTH | bit 20 | negotiation logic | SE IC authentication |
| WPC_EVT_CLOAK_PING | bit 21 | timer | Cloak ping |
| WPC_EVT_STOP_AFTER_FSK | bit 22 | timer | Stop after FSK |
| WPC_EVT_FOD_REPORTED | bit 23 | WPC task internal | FOD reported |
| WPC_EVT_RENEGO_TO | bit 24 | timer | Re-negotiation timeout |
| WPC_EVT_DDM | bit 25 | WPC_DDM_TIMER | DDM event |
| WPC_EVT_DM_CRITICAL | bit 26 | dynamic | DM critical threshold |

### USB_DPDM_TASK Events (`fml/dpdm.h:253-287`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| DPDM_EVT_SRC_ATTACHED | bit 0 | port_manager | Source attached |
| DPDM_EVT_SRC_UNATTCHED | bit 1 | port_manager | Source unattached |
| DPDM_EVT_ENTER_DCP | bit 2 | DCP_HVDCP_IRQHandler ISR | Enter DCP mode |
| DPDM_EVT_ENTER_HVDCP | bit 3 | DCP_HVDCP_IRQHandler ISR | Enter HVDCP mode |
| DPDM_EVT_QC_FIXED_5V | bit 4 | QC_SRC_IRQHandler ISR | QC 5V request |
| DPDM_EVT_QC_FIXED_9V | bit 5 | QC_SRC_IRQHandler ISR | QC 9V request |
| DPDM_EVT_QC_FIXED_12V | bit 6 | QC_SRC_IRQHandler ISR | QC 12V request |
| DPDM_EVT_QC_FIXED_20V | bit 7 | QC_SRC_IRQHandler ISR | QC 20V request |
| DPDM_EVT_QC_CONTINUES | bit 8 | QC_SRC_IRQHandler ISR | QC continuous mode |
| DPDM_EVT_QC_PLUSE_INC | bit 9 | QC_SRC_IRQHandler ISR | QC3.0 pulse inc |
| DPDM_EVT_QC_PLUSE_DEC | bit 10 | QC_SRC_IRQHandler ISR | QC3.0 pulse dec |
| DPDM_EVT_AFC_RX_DATA | bit 11 | AFC_SCP_SRC_IRQHandler ISR | AFC data received |
| DPDM_EVT_SCP_RX_DATA | bit 12 | AFC_SCP_SRC_IRQHandler ISR | SCP data received |
| DPDM_EVT_SCP_TX_DATA | bit 13 | dynamic | SCP transmit data |
| DPDM_EVT_SNK_ATTACHED | bit 14 | port_manager | Sink attached |
| DPDM_EVT_SNK_UNATTCHED | bit 15 | port_manager | Sink unattached |
| DPDM_EVT_SNK_BC12DONE | bit 16 | DPDM_SINK_IRQHandler ISR | BC1.2 detection done |
| DPDM_EVT_SNK_HVDCP_START | bit 17 | DPDM_SINK_TIMER | HVDCP start |
| DPDM_EVT_SNK_HVDCP_DONE | bit 18 | DPDM_SINK_IRQHandler ISR | HVDCP done |
| DPDM_EVT_SNK_QC_START | bit 19 | timer | QC detection start |
| DPDM_EVT_SNK_QC_DONE | bit 20 | timer | QC detection done |
| DPDM_EVT_AFC_SCP_OUT | bit 21 | AFC/SCP handler | AFC/SCP output |
| DPDM_EVT_SNK_HVDCP_FAIL | bit 22 | ISR | HVDCP failed |
| DPDM_EVT_SNK_QC12V_DONE | bit 23 | timer | QC 12V test done |
| DPDM_EVT_UFCS_INT | bit 24 | UFCS_IRQHandler ISR | UFCS interrupt |
| DPDM_EVT_UFCS_RX_PACKET | bit 25 | UFCS handler | UFCS packet received |
| DPDM_EVT_UFCS_PSREADY | bit 26 | UFCS handler | UFCS PS ready |
| DPDM_EVT_UFCS_RX_RESET | bit 27 | UFCS handler | UFCS hard reset |
| DPDM_EVT_TIMER_PERIOD | bit 31 | USB_BC12_TIMER (100ms) | Periodic polling |

### BUCKBOOST_TASK Events (`power/buckboost.h:112-134`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| BUCKBOOST_EVT_SWITCH_WORK_MODE | bit 0 | dynamic | Switch work mode |
| BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT | bit 1 | `buckboost_set_bus_iv()` | Set discharge VBUS voltage |
| BUCKBOOST_EVT_SET_CHARGER_CURRENT | bit 2 | dynamic | Set charger current |
| BUCKBOOST_EVT_SET_DISCHG_IBUS_LIMIT | bit 3 | dynamic | Set discharge IBUS limit |
| BUCKBOOST_EVT_SET_CHAGER_IBUS_LIMIT | bit 4 | dynamic | Set charger IBUS limit |
| BUCKBOOST_EVT_SET_CHAGER_IBAT_LIMIT | bit 5 | dynamic | Set charger IBAT limit |
| BUCKBOOST_EVT_SET_VBUS_DUMMYLOAD_DISCHG | bit 6 | dynamic | VBUS dummy load discharge |
| BUCKBOOST_EVT_SET_TYPECA_GATE_EN | bit 7 | dynamic | TypeC-A gate enable |
| BUCKBOOST_EVT_REGULATOR_WAITDONE | bit 8 | BUCKBOOST_REGULATOR_TIMER | Regulator wait done |
| BUCKBOOST_EVT_REGULATOR_DELAYDONE | bit 9 | BUCKBOOST_REGULATOR_TIMER | Regulator delay done |
| BUCKBOOST_EVT_SET_TYPECB_GATE_EN | bit 10 | dynamic | TypeC-B gate enable |
| BUCKBOOST_EVT_SET_USB_A_GATE_EN | bit 11 | dynamic | USB-A gate enable |
| BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_EN | bit 12 | dynamic | TypeC-A dummy load on |
| BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_EN | bit 13 | dynamic | TypeC-B dummy load on |
| BUCKBOOST_EVT_SET_TYPECA_DUMMYLOAD_DIS | bit 14 | dynamic | TypeC-A dummy load off |
| BUCKBOOST_EVT_SET_TYPECB_DUMMYLOAD_DIS | bit 15 | dynamic | TypeC-B dummy load off |
| BUCKBOOST_EVT_DUMMYLOADOVER | bit 27 | BUCKBOOST_VBUS_DISG_TIMER | VBUS discharge done |
| BUCKBOOST_EVT_CHAG_PERIOD | bit 28 | BUCKBOOST_CHAGER_TIMER (500ms) | Charger periodic |
| BUCKBOOST_EVT_ADC_PERIOD | bit 29 | BUCKBOOST_ADC_TIMER | ADC read period |
| BUCKBOOST_EVT_VBUS_PERIOD | bit 30 | BUCKBOOST_VBUS_TIMER (20ms) | VBUS periodic |
| BUCKBOOST_EVT_TIME_PERIOD | bit 31 | BUCKBOOST_PERIOD_TIMER (17ms) | Main periodic |

### PORT_MANAGER_TASK Events (`app/port_manager.h:10-34`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| PORT_ENUM_EVT_PORT0_CONNECT_START | bit 0 | port_enum_scan_handle | Port0 connect start |
| PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS | bit 1 | timer/TypeC callback | Port0 connect success |
| PORT_ENUM_EVT_PORT0_CONNECT_CLOSED | bit 2 | port_enum_scan_handle | Port0 disconnected |
| PORT_ENUM_EVT_PORT0_SINK_SETVOLT | bit 3 | timer/port_manager | Port0 sink set voltage |
| PORT_ENUM_EVT_PORT0_SINK_SETCHARGE | bit 4 | timer | Port0 sink set charge |
| PORT_ENUM_EVT_PORT0_ENUM_DONE | bit 5 | timer | Port0 enumeration done |
| PORT_ENUM_EVT_PORT1_CONNECT_START | bit 6 | port_enum_scan_handle | Port1 connect start |
| PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS | bit 7 | timer/TypeC callback | Port1 connect success |
| PORT_ENUM_EVT_PORT1_CONNECT_CLOSED | bit 8 | port_enum_scan_handle | Port1 disconnected |
| PORT_ENUM_EVT_PORT1_SINK_SETVOLT | bit 9 | timer/port_manager | Port1 sink set voltage |
| PORT_ENUM_EVT_PORT1_SINK_SETCHARGE | bit 10 | timer | Port1 sink set charge |
| PORT_ENUM_EVT_PORT1_ENUM_DONE | bit 11 | timer | Port1 enumeration done |
| PORT_ENUM_EVT_PORT2_CONNECT_START | bit 12 | port_enum_scan_handle | Port2 (USB-A) connect start |
| PORT_ENUM_EVT_PORT2_CONNECT_SUCCESS | bit 13 | timer | Port2 connect success |
| PORT_ENUM_EVT_PORT2_CONNECT_CLOSED | bit 14 | port_enum_scan_handle | Port2 disconnected |
| PORT_ENUM_EVT_PORT2_ENUM_DONE | bit 17 | timer | Port2 enumeration done |
| PORT_ENUM_EVT_PORT3_CONNECT_START | bit 18 | port_enum_scan_handle | Port3 (WPC) connect start |
| PORT_ENUM_EVT_PORT3_CONNECT_SUCCESS | bit 19 | timer | Port3 connect success |
| PORT_ENUM_EVT_PORT3_CONNECT_CLOSED | bit 20 | port_enum_scan_handle | Port3 disconnected |
| PORT_ENUM_EVT_PORT3_ENUM_DONE | bit 23 | timer | Port3 enumeration done |
| PORT_ENUM_EVT_PORT_SCAN | bit 31 | PORT_ENUM_TIMER (1ms periodic) | Scan all port events |

### FML_TASK Events (`fml/_fml.h:6-8`)
| Event | Bit | Trigger Source | Handler |
|-------|-----|----------------|---------|
| FML_EVT_ASK_INT_RECVD | bit 0 | ASK decode interrupt (ECAP) | `fml_ask_decode()` |
| APL_EVT_GAUGE | bit 1 | GAUGE_TIMER (100ms) | Battery gauge `Cyclic()` |
| APL_HID_REPORT | bit 2 | USB_WB7720_TIMER (47ms) | WB7720 HID report update |

---

## 5. Interrupt Vector List

All ISRs are declared `weak` in `hal/isr.c` and overridden in actual implementation files. The VIC priority is configured in `hal/vic.c:64-101`.

| ISR Function | Interrupt Source | Priority | Implementation File:Line | Processing Logic |
|---|---|---|---|---|
| `default_IRQHandler` | Unhandled exceptions (NMI, HardFault) | N/A | `hal/isr.c:58` | MCU reset |
| `PROT_IRQHandler` | Hardware protection | 0 (highest, commented) | `hal/sys.c:78` | Protection handling |
| `TMR0_IRQHandler` | Timer0 (250ms, one-shot for sleep wakeup) | 2 | `hal/timer.c:107` | ECAP edge detect callback dispatch |
| `TMR1_IRQHandler` | Timer1 (1ms periodic) | 1 | `hal/timer.c:173` | **System tick**: `sys_ticks++`, `tc_sys_ticks++`, `usb_pdlib_timer_update()`, `ui_display()`, `key_handle_10ms()`, RTC tick |
| `TMR2_IRQHandler` | Timer2 (850ms periodic) | 3 | `hal/timer.c:237` | Software watchdog reset (`soft_wdt_reset()`) |
| `TMR3_IRQHandler` | Timer3 (1ms, dynamic) | 3 | `hal/timer.c:251` | PWM duty ramp-up, FSK silence countdown |
| `ECAP1_IRQHandler` | Edge capture 1 | 2 | `hal/ecap.c:245` | ASK demodulation edge capture |
| `ECAP2_IRQHandler` | Edge capture 2 | 2 | `hal/ecap.c:254` | ASK demodulation edge capture |
| `ECAP3_IRQHandler` | Edge capture 3 | 2 | `hal/ecap.c:272` | ASK demodulation edge capture |
| `ECAP4_IRQHandler` | Edge capture 4 | 2 | `hal/ecap.c:263` | ASK demodulation edge capture |
| `ECAP5_IRQHandler` | Edge capture 5 | 2 | `hal/ecap.c:281` | ASK demodulation edge capture |
| `UART1_IRQHandler` | UART1 RX | 3 | `hal/uart.c:123` | UART RX callback dispatch -> `APP_vUART1_RxIntHandler()` |
| `I2CS_IRQHandler` | I2C Slave | 3 | `hal/i2cs.c:74` | I2C slave data handling |
| `USBPD_IRQHandler` | USB PD hardware | 0 (highest) | `lib/usb_pd.c:2630` | USB PD message TX/RX, GoodCRC, hard reset |
| `UFCS_IRQHandler` | UFCS protocol | 1 (conditional) | `lib/ufcs.c:309` | UFCS protocol packet handling |
| `DPDM_SINK_IRQHandler` | D+/D- sink detection | 1 | `fml/usb_qc.c:64` | BC1.2/QC sink detection -> sets DPDM events |
| `DCP_HVDCP_IRQHandler` | DCP/HVDCP detection | 1 | `fml/dpdm.c:395` | Sets `DPDM_EVT_ENTER_DCP`, `DPDM_EVT_ENTER_HVDCP` via `osal_set_event()` |
| `QC_SRC_IRQHandler` | QC source request | 1 | `fml/dpdm.c:419` | Sets QC voltage events (5V/9V/12V/20V/continuous/pulse) via `osal_set_event()` |
| `AFC_SCP_SRC_IRQHandler` | AFC/SCP source | 1 | `fml/dpdm.c:480` | Sets `DPDM_EVT_AFC_RX_DATA`, `DPDM_EVT_SCP_RX_DATA` via `osal_set_event()` |
| `FSK1_IRQHandler` | FSK modulator 1 | 1 | `fml/fsk.c:315` | FSK modulation complete |
| `FSK2_IRQHandler` | FSK modulator 2 | 1 | `fml/fsk.c:320` | FSK modulation complete |
| `GPIO_IRQHandler` | GPIO edge | (disabled) | `hal/gpio.c:773` | GPIO callback dispatch |
| `WDT_IRQHandler` | Watchdog timer | (disabled) | `hal/wdt.c:118` | WDT handling |
| `DMA_IRQHandler` | DMA transfer | (disabled) | `hal/fmc.c:67` | Flash DMA complete |
| `I2CM_IRQHandler` | I2C Master | (disabled) | `hal/i2cm.c:1067` | I2C master transfer |
| `BADC_IRQHandler` | Battery ADC | (disabled) | `hal/badc.c:541` | ADC conversion complete |

VIC Priority scheme (lower = higher priority):
- **Priority 0**: USBPD (highest - real-time PD protocol)
- **Priority 1**: TMR1 (sys tick), FSK1/FSK2, DPDM_SINK, DCP_HVDCP, QC_SRC, AFC_SCP_SRC, UFCS
- **Priority 2**: TMR0, ECAP1-5 (ASK capture)
- **Priority 3**: TMR2, TMR3, UART1, I2CS

---

## 6. Main Loop Structure Analysis

### `main()` - `app/main.c:41-137`

```
int main(void)
{
    // Phase 1: Early Hardware Init
    VIC_vModuleDisable();               // Disable all interrupts
    hal_wdt_init();                     // Watchdog init
    RST_vCheck();                       // Reset cause check

    // Phase 2: Data & BSP Init
    ap_data_init();                     // App parameter init (from flash)
    gd_data_init();                     // Global data init
    lib_para_init();                    // Library parameter init
    fml_bsp_init();                     // Board support package init
    battery_record_init();              // (conditional) Battery record
    apl_gui_init();                     // GUI init

    // Phase 3: External IC Init (Authentication SE)
    fml_nu103x_por_init();              // NU103x power-on reset
    t91206_init() / fm1210_init();      // Security element init
    // Reads cert chain, cert hash, QI ID

    // Phase 4: Adapter Init
    fml_adp_init();                     // Adapter detection init

    // Phase 5: OSAL & Task Registration
    osal_init();                        // Clear task/timer tables
    apl_task_init();                    // Register APL_TASK + start 4 periodic timers
    buckboost_task_init();              // Register BUCKBOOST_TASK + start 3 periodic timers
    tcpm_task_init();                   // Register USB_TASK + start 1ms PD timer
    usb_dpdm_task_init();              // Register USB_DPDM_TASK + start 100ms timer
    fml_task_init();                    // Register FML_TASK + start gauge/HID timers
    wpc_task_init();                    // (conditional) Register WPC_TASK + ping timer
    port_manager_task_init();           // Register PORT_MANAGER_TASK + start 1ms scan timer

    // Phase 6: Enter Super-Loop (never returns)
    osal_start_system();               // for(;;) { wdt_feed; timer_update; event_handle; }
}
```

### `osal_start_system()` - Super Loop - `osal/osal.c:247-255`

```
for (;;)
{
    hal_wdt_feed();           // Feed hardware watchdog
    osal_timer_update();      // Decrement all running timers, fire events on expiry
    osal_event_handle();      // Iterate tasks 0..7, dispatch pending events one-by-one
}
```

### Timer Tick Source

`sys_ticks` is a `volatile uint16_t` incremented every 1ms in `TMR1_IRQHandler` (`hal/timer.c:176`).
The OSAL timer system reads elapsed ticks via `osal_tick_cnt_get()` which computes `sys_ticks - old_ticks` (`osal/osal.c:174-185`).

### Event Dispatch Mechanism

`osal_event_handle()` (`osal/osal.c:105-146`):
1. Iterates all tasks (0 to MAX_TASK-1)
2. For each task with pending events and a registered callback:
   - Atomically reads and clears the event word (with interrupts disabled)
   - Processes events one bit at a time using a `bit_map[]` lookup for lowest-set-bit finding
   - Calls `task_cb(evt_msk)` for each individual event bit

---

## 7. Scheduling Relationship Diagram (Text)

```
                         +------------------+
                         |    Hardware       |
                         |    Interrupts     |
                         +--------+---------+
                                  |
              +-------------------+-------------------+
              |                   |                   |
     +--------v--------+ +-------v--------+ +--------v--------+
     | TMR1_IRQHandler | | USBPD_IRQ      | | DCP_HVDCP_IRQ   |
     | (1ms sys tick)  | | (PD protocol)  | | QC_SRC_IRQ      |
     | -> sys_ticks++  | | -> PD state    | | AFC_SCP_SRC_IRQ |
     | -> pdlib_timer  | |    machine     | | DPDM_SINK_IRQ   |
     | -> ui_display   | |                | | -> osal_set_event|
     | -> key_handle   | |                | |                  |
     +--------+--------+ +-------+--------+ +--------+---------+
              |                   |                   |
              v                   |                   v
     +--------+--------+         |          +--------+---------+
     | ECAP1-5_IRQ     |         |          | FSK1/FSK2_IRQ    |
     | (ASK decode)    |         |          | (FSK done)       |
     | -> ecap_callback|         |          | -> osal_set_event|
     | -> osal_set_evt |         |          +------------------+
     +--------+--------+         |
              |                   |
              v                   v
    +---------+-------------------+-----------+
    |          OSAL Main Super-Loop           |
    |  osal_start_system() [osal/osal.c:247]  |
    |                                         |
    |  for(;;) {                              |
    |    hal_wdt_feed();                      |
    |    osal_timer_update();  ----+          |
    |    osal_event_handle();  ----+          |
    |  }                           |          |
    +------------------------------+----------+
                                   |
           osal_timer_update():    |
           Decrements timer remain |
           On expiry -> osal_set_event(task_id, event)
                                   |
           osal_event_handle():    |
           For each task 0..7:     |
             Read & clear events   |
             Dispatch one-by-one   |
                                   |
    +------------------------------+----------+
    |          OSAL Task Table (8 tasks)       |
    +------------------------------------------+
    |                                          |
    | [0] HAL_TASK      - (unused/reserved)    |
    | [1] FML_TASK      - fml_task_event_handler  -> Gauge, ASK, HID
    | [2] USB_TASK      - tcpm_task_event_handler -> TypeC/PD/QC/WPC mode
    | [3] WPC_TASK      - wpc_task_event_handler  -> Qi wireless protocol SM
    | [4] APL_TASK      - apl_task_event_handler  -> 5/10/100/250ms polling
    | [5] BUCKBOOST_TASK- buckboost_task_event_handler -> Power conversion ADC
    | [6] USB_DPDM_TASK - usb_dpdm_task_event_handler -> BC1.2/QC/AFC/SCP/UFCS
    | [7] PORT_MANAGER  - port_manager_event_handle   -> Port connect/disconnect SM
    |                                          |
    +------------------------------------------+

    Inter-task communication:
    ========================
    ISR -> osal_set_event(task_id, event)  [interrupt to task]
    Task -> osal_set_event(other_task, event)  [task to task]
    Timer expiry -> osal_set_event(task_id, event)  [timer to task]
    port_manager_set_event(event) -> g_port.port_event |= event [soft event queue]

    Timing relationships:
    ====================
    TMR1 (HW) --1ms--> sys_ticks++ --> osal_timer_update()
                                        |
                                        +--> APP_005ms_TIMER (5ms)   -> APL_TASK
                                        +--> APP_010ms_TIMER (10ms)  -> APL_TASK
                                        +--> BUCKBOOST_PERIOD (17ms) -> BUCKBOOST_TASK
                                        +--> BUCKBOOST_VBUS (20ms)   -> BUCKBOOST_TASK
                                        +--> USB_WB7720 (47ms)       -> FML_TASK
                                        +--> APP_100ms_TIMER (100ms) -> APL_TASK
                                        +--> USB_BC12 (100ms)        -> USB_DPDM_TASK
                                        +--> GAUGE (100ms)           -> FML_TASK
                                        +--> APP_250ms_TIMER (250ms) -> APL_TASK
                                        +--> BUCKBOOST_CHAG (500ms)  -> BUCKBOOST_TASK
                                        +--> PORT_ENUM (1ms)         -> PORT_MANAGER_TASK
                                        +--> USB_TC_PD (1ms)         -> USB_TASK
```

---

## 8. Key Observations

1. **No preemption between tasks**: Tasks run cooperatively in the super-loop. Only ISRs can preempt tasks.
2. **Critical sections**: `VIC_vModuleDisable()` / `VIC_vModuleEnable()` used for atomicity when reading/clearing event flags (`osal/osal.c:117-120`).
3. **Event priority within a task**: Events are dispatched lowest-bit-first using the `bit_map[]` table (`osal/osal.c:11-29`), so lower bit number = higher priority.
4. **Task dispatch order**: Tasks are scanned in order 0-7, so lower task ID gets serviced first in each loop iteration.
5. **TMR1 is the heartbeat**: The 1ms TMR1 interrupt at priority 1 drives `sys_ticks`, TypeC PD timer updates, UI display, and key scanning - making it the most critical periodic interrupt.
6. **USBPD has highest ISR priority (0)**: USB PD protocol timing is the most critical real-time requirement.
7. **Software watchdog**: TMR2 at 850ms calls `soft_wdt_reset()` which resets the MCU - the main loop must not block for more than ~850ms.
8. **Port Manager uses a soft event queue**: `g_port.port_event` is a separate 32-bit bitmask managed outside the OSAL event system, scanned every 1ms by `PORT_ENUM_EVT_PORT_SCAN`.
