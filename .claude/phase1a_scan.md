# Phase 1A: Project Scan, Layer Identification & Module Dependency Analysis

## 1. Project Overview

### 1.1 Basic Information

| Item | Detail | Evidence |
|------|--------|----------|
| **Project Name** | NU17112 PowerBank | `README.md:1` |
| **Description** | Nu17112 + SW7201 powerbank firmware with Qi 2.x wireless charging TX, DRP wired charging (PD/QC/AFC/SCP) | `README.md:4-8` |
| **MCU** | NU171xx (Nuvolta) - 32-bit CK802 core @ 36 MHz | `hal/overview.h:13`, `hal/sys.c:58-64` |
| **CPU Architecture** | C-SKY CK802 (T-Head/Alibaba RISC arch) | `startup/crt0.S:18` (ck802_vector_table) |
| **Flash** | 120KB APROM (0x00002000, LENGTH 0x1E000) | `startup/ckcpu.ld:5` |
| **SRAM** | 8KB total: 1KB global data + 7KB general | `startup/ckcpu.ld:6-7` |
| **System Clock** | 36 MHz (PLL from XTAL) | `hal/overview.h:34`, `hal/sys.c:63` |
| **Operating Voltage** | 2.8V core, 3V-21V input | `hal/overview.h:31,35` |
| **Build System** | Eclipse CDT + Make (C-SKY CDS V5.2.14 B20231027) | `Debug/sources.mk`, `hal/overview.h:41` |
| **Toolchain** | C-SKY GCC (ckcpu linker script, .S startup) | `startup/ckcpu.ld`, `startup/crt0.S` |

### 1.2 File Statistics

| Category | Count | Directories |
|----------|-------|-------------|
| **.c files** | 75 | app(25), fml(13), hal(18), gauge(5), lib(7), osal(1), power(2), usbpd(1), util(3) |
| **.h files** | 87 | app(29), fml(16), hal(16), gauge(7), osal(1), power(2), usbpd(6), util(4) |
| **Linker script** | 1 | `startup/ckcpu.ld` |
| **Assembly** | 1 | `startup/crt0.S` |
| **Build files** | 11 | `Debug/` subdirectory makefiles |
| **Firmware binaries** | 3 | `FW/*.bin` (prebuilt release binaries) |
| **Total source** | ~162 .c/.h files | 9 source directories |

### 1.3 Key External ICs / Companion Chips

| IC | Role | Evidence |
|----|------|----------|
| **NU6801** | Buck-Boost charger IC (I2C) | `hal/nu6801.c`, `power/buckboost.c:977` |
| **NU6805** | Alternate Buck-Boost charger IC (I2C) | `hal/nu6805.c`, `power/buckboost.c:948` |
| **NU103x** | Wireless charging demod/mod controller | `fml/nu103x.c`, `fml/nu103x.h` |
| **FM1210** | Qi authentication SE IC (I2C) | `fml/fm1210.c`, `app/main.c:103-108` |
| **T91206** | Qi authentication SE IC (I2C) | `fml/t91206.c`, `app/main.c:91-99` |
| **WB7720** | USB HID battery gauge display IC (I2C) | `fml/_fml.c:14,115-187` |

---

## 2. Directory Structure & Role Identification

```
nu17112_powerbank/
|-- startup/               [Startup/Boot]    CK802 vector table, crt0, linker script
|-- hal/                   [HAL Layer]       Hardware registers, peripheral drivers
|-- fml/                   [Middleware/FML]   Functional Module Layer - BSP init, data, protocols
|-- lib/                   [Library Layer]    Pre-built protocol stacks (PD, TypeC, AFC, UFCS, etc.)
|-- usbpd/                 [Protocol Layer]   USB PD definitions, packet structures, PD library
|-- gauge/                 [Algorithm Layer]  BMS/SOC algorithm (MATLAB Simulink auto-generated)
|-- power/                 [Power Layer]      Buck-Boost control, battery management
|-- app/                   [Application]      WPC Qi state machines, LED/GUI, port manager, sleep
|-- osal/                  [OSAL]             OS Abstraction: task scheduler, timers, events
|-- util/                  [Utility]          Type defs, delay, printf, algo helpers
|-- Debug/                 [Build Output]     Eclipse CDT makefile fragments
|-- FW/                    [Release]          Pre-built firmware binaries (.bin)
|-- .claude/               [Analysis]         This analysis output
```

### 2.1 Directory Role Details

| Directory | Role | Key Files | Description |
|-----------|------|-----------|-------------|
| **startup/** | Boot | `crt0.S`, `ckcpu.ld` | CK802 reset handler, vector table (32 IRQs), BSS/data init, jump to `main()` (`crt0.S:159`) |
| **hal/** | HAL/Driver | `regdef.h`(register defs), `isr.c`, `sys.c`, `gpio.c`, `uart.c`, `timer.c`, `epwm.c`, `badc.c`, `eadc.c`, `ecap.c`, `fmc.c`, `i2cm.c`, `i2cs.c`, `tcpc.c`, `vic.c`, `wdt.c`, `bpwm.c`, `ddm.c`, `nu6801.c`, `nu6805.c` | Direct register manipulation via memory-mapped structs (e.g., `SYS->CLK_CTRL.WORD`, `hal/sys.c:63`). Every HAL function prefixed `hal_xxx_init/xxx`. `regdef.h` defines all peripheral register bitfields with base addresses at 0x40000000+. |
| **fml/** | Middleware | `_fml.c`, `bsp.c`, `g_data.c`, `adp.c`, `fsk.c`, `ask.c`, `dpdm.c`, `nu103x.c`, `ntc.c`, `qdt.c`, `tcpm.c`, `usb_qc.c`, `fm1210.c`, `t91206.c` | "Functional Module Layer" - Calls HAL APIs, provides service-level interfaces. BSP init sequence (`bsp.c:10-40`), global data management (`g_data.c/h`), adapter detection (`adp.c`), NTC temperature conversion, FSK/ASK modulation control, Q-factor detection. |
| **lib/** | Protocol Lib | `pd_tc.c`, `typec.c`, `usb_pd.c`, `afc_scp.c`, `ask.c`, `pfod.c`, `ufcs.c` | Protocol stack implementations for USB PD, TypeC state machine, AFC/SCP/FCP, UFCS. Calls HAL (`TCPC->` registers in `pd_tc.c:26`). Note: config.h comment says "lib folder" changes require lib rebuild for customers (`config.h:64-66`). |
| **usbpd/** | USB PD Defs | `pd.h`, `pd_tc.h`, `typec.h`, `usb_pd.h`, `usbpd_config.h`, `pdlib.h`, `pdlib.c` | USB PD packet structures, PDO/RDO definitions, VDM headers. `pdlib.c/.h` wraps the lib/ PD stack into a clean API. |
| **gauge/** | Algorithm | `BMS_FixPoint.c`, `BMS_FixPoint_data.c`, `BMS_data.c`, `SOC.c`, `SOCPack.c` | MATLAB Simulink auto-generated fixed-point BMS algorithm. SOC estimation via AH integration + OCV lookup. Types from `rtwtypes.h`, `multiword_types.h`. |
| **power/** | Power Mgmt | `buckboost.c`, `buckboost.h`, `bat.c`, `bat.h` | Buck-Boost charger management via ops table pattern (function pointers to NU6801/NU6805 HAL, `buckboost.c:948-1005`). Charge/discharge mode control, protection, IR drop compensation. |
| **app/** | Application | `main.c`, `app.c`, `config.h`, `_wpc.c`, `wpc_*.c`, `led.c`, `gui.c`, `port_manager.c`, `prot.c`, `sleep.c`, `pid.c`, `fod.c`, `qfod.c`, `epp.c`, `mpp.c`, `bat_record.c` | Application entry (`main.c`), Qi WPC state machine (idle/ping/nego/cnfg/xfer phases), LED/key UI, I2C slave GUI interface, multi-port manager, sleep/wake, PID controller for wireless power regulation. |
| **osal/** | OSAL | `osal.c`, `osal.h` | Cooperative task scheduler: event-driven with software timers. 8 tasks max, 31 timers max. Main loop: `wdt_feed -> timer_update -> event_handle` (`osal.c:247-254`). |
| **util/** | Utility | `typdef.h`, `delay.c`, `printk.c`, `algo.c` | Basic type definitions (`__IO volatile`), microsecond delay (ASM in `crt0.S:166-177`), debug printf, algorithm helpers. |

---

## 3. Layered Architecture

### 3.1 Architecture Diagram (Bottom-Up)

```
+============================================================================+
|  LAYER 5: APPLICATION (app/)                                               |
|  - WPC Qi 2.x TX state machine (idle/ping/nego/cnfg/xfer)                 |
|  - Port Manager (multi-port charge/discharge arbitration)                  |
|  - LED/Key UI, I2C Slave GUI, Sleep/Wake                                  |
|  - PID controller, FOD (Foreign Object Detection)                         |
|  - Battery record/logging (CCC compliance)                                |
+============================================================================+
        |                |                |               |
        v                v                v               v
+==================+ +===============+ +==============+ +=================+
| LAYER 4: POWER   | | LAYER 3: FML  | | LAYER 3: LIB | | LAYER 3: USBPD |
| (power/)         | | (fml/)        | | (lib/)        | | (usbpd/)       |
| - BuckBoost ctrl | | - BSP init    | | - PD stack    | | - PD defs      |
| - Bat management | | - Adapter det | | - TypeC SM    | | - pdlib wrapper |
| - Charge protect | | - FSK/ASK     | | - AFC/SCP     | |                |
|                  | | - NTC, QDT    | | - UFCS        | |                |
|                  | | - SE IC comm  | | - ASK decode  | |                |
+==================+ +===============+ +==============+ +=================+
        |                |                |               |
        v                v                v               v
+============================================================================+
|  LAYER 2: OSAL (osal/)                                                     |
|  - Cooperative task scheduler (event-driven, 8 tasks)                      |
|  - Software timer management (31 timers)                                   |
|  - Memory utility (copy, set, clear)                                       |
+============================================================================+
        |
        v
+============================================================================+
|  LAYER 1: HAL (hal/)                                                       |
|  - Register definitions (regdef.h - memory-mapped bitfield structs)        |
|  - Peripheral drivers: GPIO, UART, Timer, EADC, BADC, ECAP, EPWM, BPWM,  |
|    FMC(Flash), I2CM, I2CS, TCPC, WDT, VIC, SYS(PLL), DDM                  |
|  - ISR vector table and weak IRQ handlers                                  |
|  - External IC drivers: NU6801, NU6805 (I2C register access)              |
+============================================================================+
        |
        v
+============================================================================+
|  LAYER 0: STARTUP / UTIL                                                   |
|  startup/: crt0.S (vector table, data/bss init), ckcpu.ld (linker)        |
|  util/: typdef.h, delay (ASM), printk (debug), algo helpers               |
+============================================================================+
        |
        v
+============================================================================+
|  LAYER -1: GAUGE (gauge/) - Standalone Algorithm                           |
|  - MATLAB Simulink auto-generated BMS fixed-point code                     |
|  - SOC estimation (AH integration + OCV lookup)                            |
|  - No hardware dependency, pure computation                                |
+============================================================================+
```

### 3.2 Layer Classification Evidence

#### HAL Layer (hal/) - Direct Register Access
- `hal/sys.c:63`: `SYS->CLK_CTRL.WORD = (_SYS_CPU_CLK_36M << SYS_CLK_CTRL_CPU_CLK_SEL_Pos) | ...`
- `hal/regdef.h:65`: `#define WDT_CTRL_MODU_EN_Pos  (0)` - Register bit position definitions
- `hal/regdef.h:1-67`: Structs like `TS_WDT` with `__IO` volatile register access at 0x40000000+
- `hal/isr.c:58-176`: Weak ISR handler declarations with `__attribute__((isr))`
- `lib/pd_tc.c:26-28`: `TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;` (lib also directly accesses HW - this is a design note)

#### FML Layer (fml/) - Middleware / Service Layer
- `fml/bsp.c:10-40`: Calls HAL init functions: `hal_gpio_init()`, `hal_sys_init()`, `hal_timer_init()`, etc.
- `fml/_fml.c:27-70`: Event-driven task handler registered with OSAL, calls `fml_ask_decode()`, gauge `Cyclic()`
- `fml/adp.c`: Adapter type detection logic calling HAL BADC measurements
- `fml/nu103x.c`: I2C communication with NU103x wireless charging companion IC
- `fml/g_data.c/h`: Global data structures `struct ap_t`, `struct gd_t` shared across layers

#### Library Layer (lib/) - Protocol Stacks
- `lib/pd_tc.c:1-15`: Includes `regdef.h`, `pd.h`, `usb_pd.h`, `buckboost.h` - implements PD/TypeC FSM
- `lib/typec.c:1-14`: TypeC state machine with dual-port support `g_tc[TYPEC_PORT_MAX_N]`
- `lib/afc_scp.c`, `lib/ufcs.c`: Fast charging protocol implementations
- Note: `config.h:64-66` warns that lib changes need rebuild for customer distribution

#### Application Layer (app/) - Business Logic
- `app/main.c:42-137`: System init sequence and `osal_start_system()` call
- `app/app.c:30-50`: Task init with periodic timers (5/10/100/250ms), protection checks
- `app/_wpc.c` + `wpc_*.c`: Qi WPC TX state machine (WPC_PHASE_IDLE/PING/NEGO/CNFG/XFER)
- `app/led.c`: LED display control with GPIO pin definitions (`led.h:3-17`)
- `app/port_manager.c`: Multi-port (TypeC A/B, USB-A, WPC) charge/discharge arbitration
- `app/sleep.c`: Low-power sleep mode entry/exit with Q-factor wake detection

#### Gauge Layer (gauge/) - Pure Algorithm
- `gauge/SOC.h:13-24`: `AhIntegralSOC()`, `Lookup_OCVSOC()`, `SOC_Correction()` - pure math
- `gauge/BMS_FixPoint.h`: MATLAB Simulink generated: `BMS_FixPoint_private.h`, `rtwtypes.h`
- `gauge/SOC.h:26-28`: Inputs are simple variables: `SigPr_CellVolts_mV_s`, `SigPr_PackCurr_mA_s`

---

## 4. Module Boundaries & Dependencies

### 4.1 Module Public Interface Summary

| Module | Key Public Functions | Header |
|--------|---------------------|--------|
| **hal/sys** | `hal_sys_init()` | `sys.h:13` |
| **hal/gpio** | `hal_gpio_init()` | `gpio.h` |
| **hal/timer** | `hal_timer_init(TMRx)` | `timer.h` |
| **hal/uart** | `hal_uart_init()`, `hal_uart_putc()`, `hal_uart_reg_int_cb()` | `uart.h` |
| **hal/badc** | `hal_badc_init()`, `hal_badc_meas(ch)` | `badc.h` |
| **hal/eadc** | `hal_eadc_init()` | `eadc.h` |
| **hal/epwm** | `hal_epwm_pwm_start()`, `hal_epwm_pwm_update()` | `epwm.h` |
| **hal/fmc** | `hal_fmc_erase_page()`, `hal_fmc_write_word()` | `fmc.h` |
| **hal/i2cm** | `hal_i2cm_init()`, `hal_i2cm_write_multi_bytes()`, `hal_i2cm_read_one_byte()` | `i2cm.h` |
| **hal/i2cs** | `hal_i2cs_init()` | `i2cs.h` |
| **hal/tcpc** | `hal_tcpc_init()`, `hal_tcpc_set_gate_en()` | `tcpc.h` |
| **hal/wdt** | `hal_wdt_init()`, `hal_wdt_feed()` | `wdt.h` |
| **hal/vic** | `hal_vic_init()`, `VIC_vModuleEnable()`, `VIC_vModuleDisable()` | `vic.h` |
| **hal/nu6801** | `hal_nu6801_buckboost_init()`, `hal_nu6801_buckboost_set_mode()`, ... | `nu6801.h` |
| **hal/nu6805** | `hal_nu6805_buckboost_init()`, `hal_nu6805_buckboost_set_mode()`, ... | `nu6805.h` |
| **osal** | `osal_init()`, `osal_start_system()`, `osal_set_event()`, `osal_start_timerEx()`, `osal_task_handler_reg()` | `osal.h:93-101` |
| **fml/bsp** | `fml_bsp_init()` | `bsp.h:4` |
| **fml/g_data** | `ap_data_init()`, `gd_data_init()`, `lib_para_init()` | `g_data.h:553-555` |
| **fml/adp** | `fml_adp_init()`, `fml_adp_volt_set()`, `fml_adp_type_set()` | `adp.h:38-40` |
| **fml/ask** | `fml_ask_decode()`, `fml_ask_decode_check()` | `ask.h` |
| **fml/fsk** | FSK modulation control | `fsk.h` |
| **fml/nu103x** | `fml_nu103x_por_init()`, `fml_nu103x_dmo1_param_set()`, `fml_nu103x_dmo2_param_set()` | `nu103x.h` |
| **fml/tcpm** | `tcpm_task_init()`, `tcpm_stop_wpc()`, `tcpm_update_wpc_work_mode()` | `tcpm.h:129-138` |
| **fml/ntc** | `fml_ntc_temp_get()`, NTC temperature lookup | `ntc.h` |
| **fml/qdt** | `fml_qdt_detect()` - Q-factor/frequency detection | `qdt.h` |
| **fml/fm1210** | `fm1210_init()`, `fm1210_get_qi_id()`, `fm1210_read_cert_hash()` | `fm1210.h` |
| **fml/t91206** | `t91206_init()`, `t91206_get_qi_id()`, `t91206_read_cert_hash()` | `t91206.h` |
| **fml/usb_qc** | QC/HVDCP protocol control | `usb_qc.h` |
| **fml/dpdm** | `usb_dpdm_task_init()`, DPDM sink detection | `dpdm.h` |
| **lib/typec** | `usb_tc_init()`, TypeC state machine | `typec.h` |
| **lib/pd_tc** | `hal_tcpc_init()`, PD TypeC physical layer | `pd_tc.h` |
| **lib/usb_pd** | USB PD protocol engine | `usb_pd.h` |
| **lib/afc_scp** | AFC/SCP/FCP source protocol | `afc_scp.h` (in fml/) |
| **lib/ufcs** | UFCS protocol | `ufcs.h` (in fml/) |
| **usbpd/pdlib** | `pdlib_init()`, `pdlib_run()`, `pdlib_is_connect()`, `pdlib_disable_typec()`, `pdlib_restart_typec()`, `pdlib_get_tc_state()`, `pdlib_update_source_pdo()` | `pdlib.h:14-46` |
| **power/buckboost** | `buckboost_task_init()`, `buckboost_set_bus_iv()`, `buckboost_set_charge_current()`, `buckboost_set_work_mode()`, `buckboost_set_gate_en()` | `buckboost.h:137-150` |
| **gauge/SOC** | `SOC_Init()`, `SOC()`, `Cyclic()`, `AhIntegralSOC()` | `SOC.h:13-24` |
| **app/app** | `apl_task_init()`, `apl_task_event_handler()`, `APP_vInit()` | `app.h:12-16` |
| **app/_wpc** | `wpc_task_init()`, WPC Qi state machine | `_wpc.h` |
| **app/port_manager** | `port_manager_task_init()`, `port_manager_set_event()` | `port_manager.h:96-98` |
| **app/led** | `led_init()`, `ui_update()`, `led_display()`, `detectSingleKey()` | `led.h:52-57` |
| **app/gui** | `apl_gui_init()`, `iic_read_info_sync()`, `iic_write_info_sync()` | `gui.h:37-39` |
| **app/sleep** | `SLP_vNormalToSleep()`, `RST_vCheck()` | `sleep.h:3-4` |
| **app/bat_record** | `battery_record_init()`, `battery_record_periodic_check()` | `bat_record.h` |

### 4.2 Module Dependency Matrix

The arrow `A -> B` means module A depends on (includes/calls) module B.

```
                 util  hal   osal  fml   lib   usbpd  gauge  power  app
  util            -     .     .     .     .      .      .      .     .
  hal (regdef)   YES    -     .     .     .      .      .      .     .
  hal (drivers)  YES   YES    .     .     .      .      .      .     .
  osal           YES   YES    -     .     .      .      .      .     .
  fml            YES   YES   YES    -     .      .     YES     .     .
  lib            YES   YES   YES   YES    -     YES     .     YES    .
  usbpd          YES    .     .    YES   YES     -      .      .     .
  gauge           .     .     .     .     .      .      -      .     .
  power          YES   YES   YES   YES   YES    YES     .      -     .
  app            YES   YES   YES   YES   YES    YES    YES    YES    -
```

### 4.3 Detailed Cross-Module Dependencies

#### HAL -> (util only)
- All HAL .c files include `regdef.h` which includes `typdef.h` (from util/)
- `hal/isr.c:51-56`: `#include "typdef.h"`, `#include "regdef.h"`, `#include "printk.h"`
- HAL is self-contained at the register level

#### OSAL -> HAL + util
- `osal/osal.c:1-3`: `#include "regdef.h"`, `#include "typdef.h"`, `#include "printk.h"`
- Uses `VIC_vModuleDisable()/Enable()` for critical sections (`osal.c:117,120`)
- References `hal_wdt_feed()` from main loop (`osal.c:251`)

#### FML -> HAL + OSAL + util + gauge
- `fml/bsp.c:10-24`: Calls all `hal_xxx_init()` functions
- `fml/_fml.c:9-11`: Includes gauge headers `BMS_FixPoint.h`, `SOC.h`
- `fml/_fml.c:44`: Calls `Cyclic()` from gauge for SOC computation
- `fml/g_data.h`: Defines shared global state structs (`ap_t`, `gd_t`) used by all upper layers
- `fml/tcpm.c`: Manages USB task coordination, WPC work mode

#### LIB -> HAL + OSAL + FML + USBPD
- `lib/pd_tc.c:1-14`: Includes `regdef.h`, `pd_tc.h`, `pd.h`, `usb_pd.h`, `buckboost.h`, `osal.h`
- `lib/pd_tc.c:26`: Direct TCPC register access `TCPC->CCA_CTRL.BITS.CC_BLOCK_DIS = 0;`
- `lib/typec.c:1-13`: Includes `tcpm.h`, `tcpc.h`, `pd_tc.h`, `typec.h`, `_wpc.h`, `g_data.h`, `port_manager.h`, `nu6801.h`
- **Note**: lib/ has cross-layer dependencies to app/ (`_wpc.h`, `port_manager.h`) - this is a coupling issue worth noting

#### USBPD -> FML + LIB
- `usbpd/pdlib.h:4`: `#include "tcpm.h"` (from fml/)
- pdlib wraps lib/ PD stack functions into simplified API

#### GAUGE -> (standalone)
- `gauge/SOC.h:11`: Only includes `rtwtypes.h` (own type defs)
- No hardware dependencies - pure algorithmic code
- Interface: Input variables `SigPr_CellVolts_mV_s`, `SigPr_PackCurr_mA_s`, `SigPr_CellTemps_C_s`

#### POWER -> HAL + OSAL + FML + LIB + USBPD
- `power/buckboost.c:1-16`: Includes `regdef.h`, `nu6805.h`, `nu6801.h`, `tcpm.h`, `pdlib.h`, `port_manager.h`, `g_data.h`, `bat.h`, `ntc.h`, `typec.h`
- Uses operations table pattern (`buckboost_ops`) to abstract NU6801 vs NU6805 hardware
- Calls `pdlib_disable_typec()`, `pdlib_restart_typec()` for protection handling

#### APP -> All layers
- `app/main.c:1-30`: Includes headers from every layer
- `app/app.c:1-15`: Includes `regdef.h`, `g_data.h`, `config.h`, `_wpc.h`, `led.h`, `gui.h`, `badc.h`, `bat_record.h`
- `app/port_manager.c`: References `pdlib_*`, `buckboost_*`, `tcpm_*` functions

### 4.4 Global Data Sharing (extern)

Key shared global variables:

| Variable | Defined In | Type | Used By |
|----------|-----------|------|---------|
| `*ap` | `fml/g_data.c` | `volatile struct ap_t*` | app/, fml/ - App configuration (CFG RAM) |
| `*gd` | `fml/g_data.c` | `volatile struct gd_t*` | Everywhere - Runtime state data |
| `g_buckboost` | `power/buckboost.c:24` | `struct buckboost_s` | power/, fml/, app/ |
| `buckboost_ops` | `power/buckboost.c:948/975` | `const struct buckboost_operations` | power/ |
| `g_port` | `app/port_manager.h:94` | `struct port_infos` | app/, power/, lib/ |
| `g_tc[]` | `lib/typec.c:16` | `struct tc_s[MAX]` | lib/, usbpd/ |
| `sys_ticks` | hal (timer ISR) | `volatile uint16_t` | osal/osal.c:8 |
| `lib_para` | `fml/g_data.h:547` | `struct lib_para_sts` | fml/, lib/ |
| `app_reg_buff[]` | `app/gui.h:35` | `uint8_t[64]` | app/ (I2C slave data) |
| `wpc_mode` | `fml/tcpm.c` | `uint8_t` | fml/, app/, power/ |
| `qi_state` | `fml/tcpm.c` | `uint8_t` | fml/, power/ |
| `SigPr_CellVolts_mV_s` | gauge/ | `uint16_T` | fml/_fml.c (bridge to gauge) |
| `SigPr_PackCurr_mA_s` | gauge/ | `int32_T` | fml/_fml.c (bridge to gauge) |

### 4.5 OSAL Task Registration (System Init Order)

From `app/main.c:119-132`, the system initialization and task registration order:

```
1. VIC_vModuleDisable()           -- Disable interrupts
2. hal_wdt_init()                 -- Watchdog
3. RST_vCheck()                   -- Reset reason check
4. ap_data_init()                 -- App config from CFG flash
5. gd_data_init()                 -- Runtime data init
6. lib_para_init()                -- Library parameters
7. fml_bsp_init()                 -- All HAL peripheral init
8. battery_record_init()          -- Battery logging (CCC)
9. apl_gui_init()                 -- I2C slave GUI init
10. fml_nu103x_por_init()         -- NU103x wireless IC init
11. SE IC init (FM1210 or T91206) -- Authentication IC
12. fml_adp_init()                -- Adapter detection init
13. osal_init()                   -- OSAL scheduler init
14. apl_task_init()    -> APL_TASK (task 4) -- App periodic polling
15. buckboost_task_init() -> BUCKBOOST_TASK (task 5) -- Power management
16. tcpm_task_init()   -> USB_TASK (task 2) -- USB/TypeC management
17. usb_dpdm_task_init() -> USB_DPDM_TASK (task 6) -- DPDM sink detection
18. fml_task_init()    -> FML_TASK (task 1) -- Gauge + HID report
19. wpc_task_init()    -> WPC_TASK (task 3) -- WPC Qi state machine
20. port_manager_task_init() -> PORT_MANAGER_TASK (task 7) -- Port arbitration
21. osal_start_system()           -- Enter main loop (never returns)
```

OSAL Task Table (`osal.h:50-62`):
| Task ID | Name | Handler |
|---------|------|---------|
| 0 | HAL_TASK | `hal_event_handler()` (empty, `_hal.c:3`) |
| 1 | FML_TASK | `fml_task_event_handler()` |
| 2 | USB_TASK | `tcpm_task_event_handler()` |
| 3 | WPC_TASK | WPC Qi event handler |
| 4 | APL_TASK | `apl_task_event_handler()` |
| 5 | BUCKBOOST_TASK | `buckboost_task_event_handler()` |
| 6 | USB_DPDM_TASK | DPDM sink handler |
| 7 | PORT_MANAGER_TASK | `port_manager_event_handle()` |

### 4.6 Key Architectural Observations

1. **Cooperative RTOS**: The system uses a custom cooperative scheduler (OSAL) with event-driven tasks and software timers, not a preemptive RTOS. All processing happens in event callbacks.

2. **Ops Table Pattern**: The `power/buckboost.c` uses a C function pointer table (`buckboost_operations`) to abstract different charger ICs (NU6801 vs NU6805), selected at compile time via `#if BUCKBOOST_USED_NU6801`.

3. **Global State Architecture**: Two primary global structs `ap_t` (app configuration, stored in CFG flash at 0x20000000) and `gd_t` (runtime data at 0x20000200) are shared across all layers via `volatile` pointers.

4. **Cross-Layer Coupling** (待确认 - potential issue): `lib/typec.c` includes `app/_wpc.h` and `app/port_manager.h`, creating an upward dependency from library to application layer. This may be intentional for callback/event notification but violates strict layering.

5. **MATLAB Auto-Generated Code**: The `gauge/` directory contains Simulink-generated fixed-point BMS code. It should not be manually modified. The bridge between gauge and the rest of the system is in `fml/_fml.c` where gauge inputs are set from ADC readings.

6. **Dual Authentication IC Support**: The system supports two alternative Qi authentication ICs (FM1210, T91206) selected at runtime via `ap->auth_seic_type` (`main.c:89`).

7. **Memory Layout**: Very constrained - 120KB Flash, 8KB RAM. The `gd_t` struct alone is several hundred bytes, and `ap_t` with battery records is ~200+ bytes.

8. **WPC State Machine**: The Qi wireless charging TX follows the WPC specification phases: IDLE -> PING -> NEGO -> CNFG -> XFER, implemented across multiple source files (`wpc_idle.c`, `wpc_ping.c`, `wpc_nego.c`, `wpc_cnfg.c`, `wpc_xfer.c`, etc.).
