# PowerBank Leader Knowledge Base

## 1. Project Overview

### 1.1 Platform Fundamentals

| Dimension | Detail |
|-----------|--------|
| **Project Name** | NU17112 PowerBank |
| **Platform Code** | NU17112 + SW7201 |
| **MCU** | Nuvolta NU171xx, CK802 RISC core @ 36 MHz |
| **Memory** | 120KB Flash (0x00002000), 8KB SRAM (1KB global + 7KB heap) |
| **Operating Range** | 3V-21V input, 2.8V core voltage |
| **Build System** | Eclipse CDT + C-SKY GCC v5.2.14 |

### 1.2 Key Features

- **Wireless Charging**: Qi 2.x TX (BPP/EPP/MPP support)
- **Wired Charging**: DRP PD/QC3.0/AFC/SCP/UFCS multi-protocol support
- **Dual TypeC Ports**: Port-A (sink/source), Port-B (sink only)
- **USB-A Output**: 5V/2A (configurable, disabled on EVK_V02)
- **Battery Management**: BMS/SOC algorithm (MATLAB Simulink generated), CCC compliance logging
- **Advanced Protection**: FOD (Foreign Object Detection), thermal management (NTC), OCP/OVP/UVP

### 1.3 System Clock & Task Structure

| Task | Task_ID | Period | Core Function | Agent Owner |
|------|---------|--------|----------------|------------|
| HAL_TASK | 0 | ISR-driven | Hardware abstraction (reserved/unused handler) | platform-hal-agent |
| FML_TASK | 1 | 100ms (Gauge), 47ms (WB7720) | FML core, Gauge bridge, HID report | platform-fml-agent |
| USB_TASK | 2 | 1ms (PD timer) | TypeC/PD state machines | platform-usb-agent |
| WPC_TASK | 3 | Event-driven | Qi protocol & hardware control | platform-wpc-protocol-agent<br/>platform-wpc-hw-agent |
| APL_TASK | 4 | 5/10/100/250ms | LED/GUI/sleep/battery record | platform-apl-agent |
| BUCKBOOST_TASK | 5 | 17ms/20ms/500ms | Power conversion & battery mgmt | platform-buckboost-agent |
| USB_DPDM_TASK | 6 | 100ms | DPDM fast-charge protocols | platform-dpdm-agent |
| PORT_MANAGER_TASK | 7 | 1ms | Port enumeration & arbitration | platform-port-manager-agent |

### 1.4 Companion ICs

| IC | Function | Interface | Agent Interface |
|----|----------|-----------|-----------------|
| NU6801/NU6805 | Buck-Boost charger | I2C | platform-buckboost-agent |
| NU103x | WPC demod/mod controller | I2C | platform-wpc-hw-agent |
| FM1210 / T91206 | Qi authentication (SE IC) | I2C | platform-wpc-hw-agent |
| WB7720 | Battery gauge display | I2C (HID) | platform-fml-agent |

---

## 2. Agent Team Structure

### 2.1 11-Agent Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    powerbank-leader                         │
│   (Project coordination, code review, integration testing)  │
└─────────────────────────────────────────────────────────────┘
         ↓ coordinates and supervises ↓
┌──────────────────────────────────────────────────────────────────┐
│  Infrastructure (platform-hal-agent) - 53 files                 │
│  - 20 HAL drivers (GPIO/UART/Timer/ADC/ECAP/EPWM/I2C/etc)     │
│  - OSAL scheduler (task/timer/event dispatch)                 │
│  - Utilities (delay/printk/algo)                              │
│  - Startup code (CK802 crt0.S, linker script)                │
└──────────────────────────────────────────────────────────────────┘
         ↓ provides base services to all agents ↓
┌─────────────────────────────────────────────────────────────────────────────────┐
│ Functional Agents (10 domain specialists)                                       │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  FML Core             USB/PD              DPDM Protocols      WPC Qi Protocol  │
│  (platform-fml)       (platform-usb)      (platform-dpdm)     (platform-wpc-p) │
│  - Task dispatch      - TypeC SM          - BC1.2/QC          - 6-stage FSM    │
│  - BSP init           - PD negotiation    - AFC/SCP/UFCS       - EPP/MPP nego   │
│  - Global data        - TCPM layer        - HVDCP              - Auth (SE IC)   │
│  - Gauge bridge       - pdlib API         - 5 protocols        - 4 Xfer modes   │
│                                                                                 │
│  WPC Hardware         Buck-Boost          Gauge Algorithm      APL Layer       │
│  (platform-wpc-hw)    (platform-bb)       (platform-gauge)     (platform-apl)  │
│  - ASK/FSK codecs     - Power conversion  - BMS/SOC (MATLAB)   - LED/GUI        │
│  - NU103x control     - Battery mgmt      - Algorithm          - Sleep/Wake     │
│  - PID controller     - NTC temp mgmt     - Cyclic() 100ms     - Battery record │
│  - FOD detection      - ADC sampling      - No manual mods     - Config macros  │
│  - SE IC (FM/T91)     - Ops table         - Fixed-point        - Debug support  │
│                                                                                 │
│  Port Manager                                                                  │
│  (platform-port-manager)                                                       │
│  - 4-port arbitration (TypeC-A/B, USB-A, WPC)                                 │
│  - Charge/discharge strategy                                                  │
│  - Port enumeration FSM                                                        │
│                                                                                 │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### 2.2 Agent Ownership Matrix

| Agent | Files | Task(s) | Role | Sharability |
|-------|-------|---------|------|------------|
| powerbank-leader | 0 | N/A | Coordination only | N/A |
| platform-hal-agent | 53 | HAL(0) | Infrastructure | Platform-shared |
| platform-fml-agent | 9 | FML(1) | Core framework | Platform-shared |
| platform-usb-agent | 12 | USB(2) | TypeC/PD stacks | Platform-shared |
| platform-dpdm-agent | 8 | DPDM(6) | Fast-charge protocols | Platform-shared |
| platform-wpc-protocol-agent | 31 | WPC(3) | Qi 2.x protocol FSM | Project-variant |
| platform-wpc-hw-agent | 21 | WPC(3) shared | WPC hw + algorithms | Platform-shared |
| platform-buckboost-agent | 6 | BB(5) | Power conversion | Project-variant |
| platform-gauge-agent | 15 | (none) | BMS/SOC algorithm | Platform-shared |
| platform-apl-agent | 12 | APL(4) | Application layer | Project-variant |
| platform-port-manager-agent | 2 | PM(7) | Port arbitration | Project-variant |
| **TOTAL** | **169** | **7 tasks + ISR** | | |

**Sharability Legend**:
- **Platform-shared**: Same for all NU17112 variants (HAL, USB/PD, DPDM, Gauge, WPC hardware)
- **Project-variant**: Differ by customer/SKU (APL, BuckBoost, WPC protocol, Port Manager)

---

## 3. Cross-Agent Workflows

### 3.1 Charging Flow (Complete Path)

#### Scenario A: Wired USB-C Fast Charging (PD/QC/AFC)

```
Port Attach
    ↓
USB_TASK (TCPM) → Detect port type (DRP/SNK/SRC)
    ↓
    ├─ Case TypeC Source (Adapter):
    │  └→ USB_TASK: Start PD negotiation (SM4: SNK)
    │     ├→ Query PDO (5V/9V/15V/20V)
    │     └→ DPDM_TASK: Run BC1.2/QC/AFC negotiation (if PD fails)
    │
    └─ Case TypeC Sink (Host wants power from us):
       └→ TCPM → pdlib_update_source_pdo() → request peer to supply

DPDM Protocols (USB_DPDM_TASK):
    ├─ BC1.2 detection (100ms timer)
    ├─ QC3.0 negotiation (dynamic timer)
    ├─ AFC/SCP negotiation (I2C + PWM)
    └─ UFCS negotiation (new protocol)

    Once negotiated voltage determined:
    └→ Set BUCKBOOST_TASK work mode (CHARGER)
    └→ buckboost_set_bus_iv(voltage, current)
    └→ Enable gate for TypeC port

BUCKBOOST_TASK (17ms + 20ms + 500ms timers):
    ├─ ADC sample VBUS/IBUS/IBAT
    ├─ Enforce current limit (config limit or negotiated)
    ├─ Monitor thermal/protection
    ├─ Adjust NU6801/6805 ops table
    └→ Report status to PORT_MANAGER_TASK

PORT_MANAGER_TASK (1ms enumeration):
    ├─ Update port0/port1 state (IDLE_OR_READY → INHANDLING)
    ├─ Check multi-port conflicts
    └─ Set incharge_port and charger mode (CHARGER/DISCHG)

APL_TASK (10/100/250ms):
    ├─ Read BB work mode & battery voltage
    ├─ Update LED state (charging indicator)
    ├─ Trigger battery record (CCC compliance)
    └─ Check if can enter sleep (port still connected?)
```

#### Scenario B: Wireless Charging (Qi 2.x TX)

```
WPC_TASK (platform-wpc-protocol-agent + platform-wpc-hw-agent):

  IDLE State (initial, waiting for RX device):
    ├─ ASK RX absent (no signal on ECAP input)
    ├─ WPC_TASK: Idle sub-FSM (SM2)
    │  ├─ Digital ping (test ASK capability)
    │  └─ Q-factor detection (fml_qdt_detect via WPC-HW)
    └─ Periodic re-ping (ap->t_next_ping interval)

  RX Device Detected (ASK signal received):
    ├─ ISR: ECAP1_IRQHandler → HAL-agent
    │  └→ osal_set_event(WPC_TASK, WPC_EVT_PKT_RECVD) [FML-agent bridges]
    │
    └→ WPC_TASK enters PING state:
       ├─ Wait for Signal Strength Packet (SS)
       ├─ ASK decode: fml_ask_decode() → WPC-HW parses
       ├─ Signal strength check
       └→ Transition to NEGOTIATION

  NEGOTIATION state (power level agreement):
    ├─ Send configuration packets (FSK modulation)
    │  └→ fml_fsk_send_cfg() [WPC-HW provides]
    ├─ Receive RX config (ASK decode)
    ├─ EPP negotiation (if supported):
    │  ├─ epp_config_t structure
    │  └─ SE IC authentication (FM1210/T91206)
    └→ Approve → TRANSFER state

  TRANSFER state (power delivery):
    ├─ Sub-modes: BPP (5W) / EPP (10W+) / MPP (20W+) / DataStream
    ├─ PID control (WPC-HW):
    │  ├─ pid_control() every WPC cycle
    │  ├─ Read Q-factor & frequency
    │  ├─ Adjust TX coil voltage/duty
    │  └→ Optimize efficiency
    ├─ FOD detection (WPC-HW):
    │  ├─ fod_check() - baseline FOD
    │  ├─ qfod_check() - Q-factor FOD
    │  ├─ pfod_check() - power FOD
    │  └→ Trigger protection if detected
    ├─ NU103x coil control (WPC-HW):
    │  ├─ fml_nu103x_dmo1/dmo2_param_set()
    │  └─ Adjust demod/mod params
    └─ CEP (Constraint Energy Transfer):
       ├─ Monitor RX power request
       ├─ Regulate TX output power
       └→ Report to PORT_MANAGER_TASK

  Multi-port arbitration:
    ├─ WPC connected: port3 INHANDLING
    ├─ Also USB-C connected: port_manager decides priority
    │  └─ Typically: USB-C faster → reduce WPC power or suspend
    └→ PORT_MANAGER_TASK sets incharge_port
```

#### Scenario C: Discharging (Output Power)

```
PORT_MANAGER_TASK detects:
    - USB-A attached (Vbus > 4V) or
    - TypeC Source request (peer sends SNK capability)

Multi-port logic:
    ├─ If only USB-A output: Enable DISCHG mode
    ├─ If both input (USB-C Src) + USB-A output:
    │  └─ Prioritize by config/capacity
    └─ If WPC + USB-A: Reduce WPC power / suspend

BUCKBOOST_TASK (DISCHG mode):
    ├─ Set work_mode = DISCHG
    ├─ Read battery voltage (BAT_ADC)
    ├─ Enforce discharge current limit
    │  └─ ap->config->dischg_ibat_limit (default 8A)
    ├─ Monitor VBUS output (5V nominal, ±5%)
    ├─ Protection: OVP/UVP/OCP
    └→ Gate enable for USB-A output

APL_TASK:
    ├─ LED pattern: Discharging indicator (blink rate = remaining capacity %)
    └─ No sleep (output power = device active)
```

### 3.2 System Initialization Flow (main() → System Ready)

```
1. startup/crt0.S (CK802 reset handler)
   ├─ BSS clear, data init
   ├─ Stack setup (8KB SRAM)
   └─ Jump to main()

2. app/main.c (platform-fml-agent):
   ├─ Call osal_init()
   │  └─ Initialize OSAL task/timer/event tables
   ├─ Call fml_bsp_init() (platform-fml-agent)
   │  ├─ hal_sys_init() - clock setup (36 MHz)
   │  ├─ hal_gpio_init() - port setup
   │  ├─ hal_timer_init() - Timer0/1/2/3 (OSAL sys tick is TMR1 1ms)
   │  ├─ hal_uart_init(UART1) - debug console
   │  ├─ hal_i2c_init() - I2CM for companion ICs
   │  ├─ hal_tcpc_init() - USB PD physical layer
   │  ├─ hal_adc_init() - BADC/EADC for current/voltage sensing
   │  ├─ hal_wdt_init() - Watchdog timer
   │  └─ hal_vic_init() - Interrupt controller
   │
   ├─ Register all agent task handlers:
   │  ├─ osal_task_handler_reg(FML_TASK, fml_task_event_handler)
   │  ├─ osal_task_handler_reg(USB_TASK, usb_task_event_handler)
   │  ├─ osal_task_handler_reg(WPC_TASK, wpc_task_event_handler)
   │  ├─ osal_task_handler_reg(DPDM_TASK, dpdm_task_event_handler)
   │  ├─ osal_task_handler_reg(BUCKBOOST_TASK, buckboost_task_event_handler)
   │  ├─ osal_task_handler_reg(APL_TASK, apl_task_event_handler)
   │  ├─ osal_task_handler_reg(PORT_MANAGER_TASK, port_manager_task_event_handler)
   │  └─ [No registered handler for HAL_TASK(0)]
   │
   ├─ Initialize companion IC drivers:
   │  ├─ pdlib_init() - USB PD library (TCPM init)
   │  ├─ bms_init() - Gauge algorithm init (if not MATLAB)
   │  ├─ gauge_init() - Read SOC/OCV from gauge
   │  ├─ fml_nu103x_init() - WPC demod IC
   │  ├─ fm1210_init() or t91206_init() - SE IC (config dependent)
   │  └─ buckboost_ops_init(NU6801 or NU6805) - Charger IC ops table
   │
   ├─ Load persistent data:
   │  ├─ fml_load_calibration() - Q-freq, current offset
   │  ├─ battery_record_load() - CCC compliance data
   │  └─ app_config_load() - Customer parameters from Flash
   │
   ├─ Initialize app-level modules (platform-apl-agent):
   │  ├─ apl_gui_init() - I2C slave interface (WB7720)
   │  ├─ apl_led_init() - LED GPIO setup
   │  └─ battery_record_init() - CCC log buffer
   │
   ├─ Start periodic timers:
   │  ├─ osal_start_timerEx(GAUGE_TIMER, 100) - FML Gauge bridge
   │  ├─ osal_start_timerEx(USB_BC12_TIMER, 100) - DPDM BC1.2 detection
   │  ├─ osal_start_timerEx(USB_TC_PD_TIMER, 1) - USB PD 1ms tick
   │  ├─ osal_start_timerEx(BUCKBOOST_PERIOD_TIMER, 17) - Power conversion
   │  ├─ osal_start_timerEx(PORT_ENUM_TIMER, 1) - Port enumeration
   │  ├─ osal_start_timerEx(APP_5ms_TIMER, 5) - APL polling
   │  ├─ osal_start_timerEx(APP_10ms_TIMER, 10) - APL polling
   │  └─ ... (more timers per agent)
   │
   └─ osal_start_system() - Enter main event loop
      └─ Forever: osal_dispatch_event() → per-task event handlers

3. Event-driven execution:
   ├─ ISR occurs (HAL-agent handles) → osal_set_event(task, event_bit)
   ├─ Timer expires → osal_set_event(task, TIMER_EVENT)
   ├─ osal_dispatch_event() → Call task's event handler
   └─ Task agent processes events, updates global state, possibly sets next timer
```

### 3.3 Agent Dependency Graph (Simplified)

```
┌──────────────────────────────────────┐
│  platform-hal-agent                  │
│  (Foundation - all use this)          │
└──────────────────────────────────────┘
            ↑ (used by)
            │
    ┌───────┼───────────────────────────────────────┐
    │       │       │       │       │       │       │
    ↓       ↓       ↓       ↓       ↓       ↓       ↓
┌─────┐ ┌─────┐ ┌──────┐ ┌────┐ ┌──────┐ ┌──────┐ ┌────┐
│FML  │ │USB  │ │DPDM  │ │WPC-│ │WPC-  │ │Buck │ │Gau │
│     │→│     │→│      │ │Proto│ │HW    │ │Boost│ │ge  │
└─────┘ └─────┘ └──────┘ │     │ └──────┘ └──────┘ └────┘
    ↓       ↓       ↓     ├─────┘    ↑       ↑       ↑
    ├───────┼───────┼──→  └─────┬────┘   ┌───┴──────┘
    │       │       │           │        │
    ↓       ↓       ↓           ↓        ↓
        ┌─────────────────────────────┐
        │ PORT_MANAGER (arbitration)   │
        └─────────────────────────────┘
                    ↑
                    │ (uses status of all)
                    │
            ┌───────┴────────┐
            ↓                ↓
        ┌────────┐      ┌────────┐
        │ APL    │      │ Gauge  │
        │(UI)    │      │(BMS)   │
        └────────┘      └────────┘
```

---

## 4. Project-Level Decisions

### 4.1 Configuration Decisioning (config.h)

These macro decisions control project variant creation:

#### Hardware Port Configuration
```c
CONFIG_TYPECA_SUPPORT        1  // TypeC port A (DRP, dual-mode charger + source)
CONFIG_TYPECB_SUPPORT        1  // TypeC port B (SNK only, receive power)
CONFIG_USBA_SUPPORT          0  // USB-A port (0=disabled on EVK, 1=enabled on products)
CONFIG_WPC_SUPPORT           1  // Qi 2.x wireless TX mode
```
**Decision**: Each project SKU may enable/disable ports → impacts port_manager.c logic + component BOM.

#### Power IC Selection
```c
BUCKBOOST_USED_NU6801        1  // Use NU6801 charger (=1) or NU6805 (=0)
CONFIG_NU6801_BATLOW_VOLT    2800  // Battery low threshold (per IC specs)
CONFIG_DEADBATT_VOLTAGE      2900  // Firmware cutoff for deep discharge protection
```
**Decision**: NU6801 vs NU6805 impact efficiency & thermal handling. Must match app/buckboost.c ops table selection.

#### Battery & Charging Profile
```c
BATTERY_CV_VALUE             4200  // Charge voltage CV target (mV)
CONFIG_DISCHG_IBAT_LIMIT     0x04  // Discharge current limit (default 8A)
Rsns                         200   // Sense resistor value (milliohm × 100)
```
**Decision**: Different battery packs (2S2P / 3S1P configurations) require adjusted CV/CC parameters.

#### Protocol & Feature Flags
```c
CONFIG_AFC_SOURCE_SUPPORT    1  // AFC fast-charge (SRC mode)
CONFIG_FCP_SOURCE_SUPPORT    1  // FCP fast-charge (SRC mode)
CONFIG_SCP_SOURCE_SUPPORT    1  // SCP fast-charge (SRC mode)
CONFIG_SUPPORT_IPGA          1  // Use IPGA current sensing (vs DAVIS)
ENABLE_EPP_FUNC              1  // Qi EPP (Extended Power Profile) support
OPTION_FOD_ENABLE            1  // Foreign Object Detection (WPC safety)
SLEEPQ_WAKEUP_ENABLE         1  // Sleep Q-factor wake-up (low-power idle)
CONFIG_NEW_CCC_LOG_ENABLE    1  // New CCC exception tracking & logging
```
**Decision**: Enable/disable specific protocols per regional compliance & customer requirements.

#### SE IC Selection (Qi Authentication)
```c
// In app/main.c:
#if CONFIG_SOME_FLAG
    fm1210_init();  // FM1210 SE IC
#else
    t91206_init();  // T91206 SE IC
#endif
```
**Decision**: Different Qi certified SE ICs provide authentication. Choose per product variant.

### 4.2 Scalability Decisions

| Decision Point | Options | Impact |
|---|---|---|
| Port count | 2/3/4 ports (TypeC-A/B, USB-A, WPC) | platform-port-manager-agent redesign |
| Power classes | 5W (BPP) / 10W (EPP) / 20W+ (MPP) | app/config.h Qi power limits, charger IC ops |
| Battery chemistry | Li-Ion / Li-Po, series config | Battery CV/CC, protection thresholds |
| Charger IC | NU6801 / NU6805 / future ICs | Ops table, driver implementation |
| SE IC | FM1210 / T91206 | I2C communication protocol, authentication flow |
| Protocol stack | Add UFCS/new standards | DPDM_TASK new state handling |

### 4.3 Cross-Cutting Concerns

| Concern | Owner | Key Files | Notes |
|---------|-------|-----------|-------|
| **Exception Logging** | platform-apl-agent | app/bat_record.c | CCC compliance, uses Flash 0x1400-0x15FF |
| **Low-Power Sleep** | platform-apl-agent | app/sleep.c | Monitors ptx_idle_phase_status (WPC idle) |
| **Q-factor Calibration** | platform-wpc-hw-agent | fml/qdt.c + Flash 0x1600 | Persistent calibration per product |
| **Thermal Protection** | platform-wpc-hw-agent | fml/ntc.c | NTC curve lookup, disable TX if T > limit |
| **Watchdog Safety** | platform-hal-agent | hal/wdt.c + TMR2_IRQHandler | Software watchdog trigger |

---

## 5. Integration Test Scenarios

### 5.1 Baseline System Tests

#### Test 5.1.1: Bootup & Initialization
- **Scenario**: Power-on from cold state
- **Agents Involved**: HAL, FML, all
- **Validation**:
  - UART debug output shows clock/peripheral initialization
  - OSAL timers running (1ms tick visible)
  - All agent task handlers registered
  - No HardFault or watchdog reset

#### Test 5.1.2: Port Enumeration
- **Scenario**: Sequentially attach/detach USB-C adapter, USB-A device, Qi pad
- **Agents Involved**: USB, DPDM, WPC-Protocol, Port Manager, Buckboost
- **Validation**:
  - PORT_MANAGER_TASK: port_state[] transitions (IDLE → INHANDLING → IDLE)
  - LED indicator changes per port state
  - No port conflict errors (multiple INHANDLING simultaneously)
  - Proper gate enable/disable per port rules

#### Test 5.1.3: Single Port Charge (TypeC USB-C)
- **Scenario**: Connect 9V/2A PD adapter to TypeC-A, monitor charge
- **Agents Involved**: USB, DPDM, Buckboost, APL, Gauge
- **Validation**:
  - TCPM negotiates 9V/2A (PD SNK)
  - Buckboost ADC: VBUS reads ~9V, IBUS reads ~2A (within 100mA tolerance)
  - LED: Charging indicator on
  - Battery record: SOC increases at expected rate
  - Gauge: Cyclic() called every 100ms, SOC updated

#### Test 5.1.4: Multi-Port Charge Conflict
- **Scenario**: Simultaneously connect TypeC (9V/2A) + USB-A input + WPC pad
- **Agents Involved**: All 10 functional agents
- **Validation**:
  - PORT_MANAGER: Only one incharge_port at a time
  - Buckboost: Total current ≤ battery limit
  - LED: Shows priority port being charged
  - No deadlock or event queue overflow
  - Proper port release when adapter removed

### 5.2 Protocol-Specific Tests

#### Test 5.2.1: Qi 2.x Wireless TX
- **Scenario**: Bring compatible Qi 2.x RX device into coil range
- **Agents Involved**: WPC-Protocol, WPC-HW, Buckboost, APL
- **Validation**:
  - WPC_TASK: Ping → Negotiation → Transfer state transitions
  - ASK decode: Signal strength packet parsed correctly
  - PID control active (Q-factor updated every ~10ms)
  - FOD detection: No false positives on clean receive
  - Power transfer: Device charges at expected rate
  - LED: Qi charging indicator active

#### Test 5.2.2: QC3.0 Fast Charge
- **Scenario**: Connect QC3.0 charger (to USB-C or DPDM D+/D-)
- **Agents Involved**: DPDM, USB, Buckboost
- **Validation**:
  - DPDM_TASK: QC protocol handshake (HVDCP detect, then QC voltage requests)
  - Voltage steps through 5V → 9V → 12V (per device capability)
  - Current increases as voltage rises
  - No thermal throttling during 20+ second charge ramp

#### Test 5.2.3: AFC/SCP Fast Charge
- **Scenario**: Connect AFC (Samsung) or SCP (Huawei) adapter
- **Agents Involved**: DPDM, Buckboost
- **Validation**:
  - DPDM_TASK: I2C handshake with adapter (DM/DP pin toggle)
  - Voltage negotiated (9V or 12V per protocol)
  - No I2C timeout or communication errors
  - Charge current ramps within 5 seconds

### 5.3 Edge Cases & Reliability

#### Test 5.3.1: Battery Low/Dead State
- **Scenario**: Discharge battery to 2.9V (DEADBATT_VOLTAGE)
- **Agents Involved**: Buckboost, Gauge, APL
- **Validation**:
  - Buckboost ADC reads VBAT < 2.9V
  - Discharge gates disabled
  - WPC/USB output gates disabled
  - LED: Critical low indicator
  - Charging still allowed (to recover battery)
  - System does not shut down (can accept external power)

#### Test 5.3.2: Thermal Runaway Protection
- **Scenario**: Simulate NTC < 0°C or > 50°C via ADC calibration
- **Agents Involved**: WPC-HW (FOD/protection), Buckboost, APL
- **Validation**:
  - WPC transmission stops (prot_check() sets flag)
  - Charge current reduced (Buckboost reduces CC setpoint)
  - LED: Thermal warning indicator
  - Recovery when temperature normalizes

#### Test 5.3.3: Foreign Object Detection (Qi TX)
- **Scenario**: Place metallic object on TX coil (safely isolated)
- **Agents Involved**: WPC-Protocol, WPC-HW
- **Validation**:
  - WPC_TASK: FOD algorithm detects anomaly (Q-factor drop, voltage spike)
  - Transmission disabled (EVT_PFOD or EVT_FOD_REPORTED)
  - WPC state → CLOAK (suspend, retry later)
  - No thermal runaway
  - LED: FOD warning indicator
  - Manual restart required (re-ping cycle)

#### Test 5.3.4: Port Hot-Swap (Rapid Connect/Disconnect)
- **Scenario**: Rapidly insert/remove adapter 10 times @ 1s intervals
- **Agents Involved**: USB, Port Manager, Buckboost, APL
- **Validation**:
  - No event queue overflow
  - No memory corruption (stack/heap integrity)
  - Port state machine stable (no stuck states)
  - Watchdog not triggered
  - LED updates reflect current port state

### 5.4 Sleep & Wake Tests

#### Test 5.4.1: Idle to Sleep Transition
- **Scenario**: No external activity for 30+ seconds
- **Agents Involved**: APL, WPC-Protocol, Gauge
- **Validation**:
  - APL detects: ptx_idle_phase_status == IDLE (WPC), no active charging
  - sleep_check() returns true
  - MCU enters CK802 sleep mode (reduced clock)
  - Current draw < 5mA
  - LED dim/off

#### Test 5.4.2: Wake on Port Attach
- **Scenario**: Attach adapter while in sleep
- **Agents Involved**: GPIO (HAL), Port Manager, APL
- **Validation**:
  - GPIO_IRQHandler fires (D+/D- or USB-VBUS detect)
  - osal_set_event(PORT_ENUM_TASK, PORT_ENUM_EVT_PORT_SCAN)
  - MCU wakes, re-enters main event loop
  - Port enumeration begins within 10ms
  - LED turns on

### 5.5 System Stability

#### Test 5.5.1: 48-Hour Soak Test
- **Scenario**: Continuous charge/discharge cycles (8-hour loops × 6)
- **Agents Involved**: All
- **Validation**:
  - No memory leaks (SRAM allocation/deallocation stable)
  - No event queue corruption (overflow)
  - Battery record log size remains bounded
  - Flash write/erase operations stable
  - Watchdog never triggered
  - Average current draw matches expected profile
  - Final SOC estimate matches coulomb counter (< 5% error)

#### Test 5.5.2: Extreme Input Voltage Swing
- **Scenario**: Adapter voltage 5V → 20V → 5V (simulate ripple + PD renegotiation)
- **Agents Involved**: Buckboost, WPC, Protection
- **Validation**:
  - No hardware glitch (OVP not falsely triggered)
  - prot_check() detects actual overvoltage condition (V > 24V)
  - Gates disabled if needed
  - No damage to downstream circuitry

---

## 6. Code Review Checklist

### 6.1 Pre-Merge Review (Every PR)

#### Architecture & Dependencies
- [ ] **Single Agent Ownership**: All modified files assigned to one agent. If multiple agents → split PR.
- [ ] **Agent Boundary Respected**: No direct include from non-adjacent agent (must route via FML/Port Manager).
- [ ] **Task Boundary Respected**: Only task-assigned agent handles that task's event loop (no cross-task polling).
- [ ] **No Circular Dependencies**: Build dependency graph; ensure no cycles.

#### Functional Correctness
- [ ] **Event Handling**: If new event type, verify osal_set_event() called from correct location (ISR or task).
- [ ] **Timer Lifecycle**: osal_start_timerEx() called at init; osal_stop_timer() on shutdown/mode change.
- [ ] **Critical Section**: Check interrupt disables (osal_disable_irq) for shared data access (e.g., global state).
- [ ] **State Machine**: If modifying FSM, verify all transitions valid + no missing edge cases.
- [ ] **SRAM Pressure**: Estimate stack depth; ensure 7KB heap sufficient (typical ~3-4KB used).

#### Testing
- [ ] **Unit Test Coverage**: New function added → corresponding test in `test/` (if applicable).
- [ ] **Integration Test**: If cross-agent API modified, confirm integration test scenario updated (section 5.0).
- [ ] **Edge Case Tested**: Boundary conditions (battery low, thermal limit, port detach during charge) verified.

#### Code Quality
- [ ] **No Magic Numbers**: Numeric constants → named `#define` or enum.
- [ ] **Error Handling**: HAL returns status; checked and handled gracefully (not ignored).
- [ ] **Resource Leak**: No forgotten free() / stop_timer() / close_file().
- [ ] **Watchdog**: If loop takes >500ms, add watchdog kick.

#### Documentation
- [ ] **Comment Added**: Complex logic (PID tuning, FOD thresholds, SE IC handshake) has explanatory comment.
- [ ] **API Contract**: If public function, header comment includes precondition, postcondition, side effects.
- [ ] **Config Impact**: If config.h macro affects behavior, note in commit message.

### 6.2 Release Review (Before Tag)

#### Regression Verification
- [ ] **Baseline Tests Pass**: Section 5.1 (bootup, port enum, single charge) all green.
- [ ] **Protocol Tests Pass**: Section 5.2 (Qi, QC, AFC) validated on hardware.
- [ ] **Edge Cases Pass**: Section 5.3 (low battery, thermal, FOD, hot-swap) no regressions.
- [ ] **Sleep/Wake Pass**: Section 5.4 current draw / wake latency within spec.

#### Performance
- [ ] **Timing Budget**: USB_TASK (1ms), WPC_TASK (event-driven, <5ms latency), Buckboost (17ms), APL (10/100ms) all met.
- [ ] **Power Budget**: Idle <5mA, charging <50mA (core + peripherals), TX Qi <100mA (coil off).
- [ ] **Flash Utilization**: Code + data < 120KB (headroom for calibration/log).

#### Security & Safety
- [ ] **No Stack Overflow**: Static analysis or runtime monitoring shows no corruption.
- [ ] **No Integer Overflow**: Voltage/current calculations checked for wraparound (use 32-bit intermediates).
- [ ] **No Uninitialized Variables**: All globals in g_data.h initialized at startup.
- [ ] **No Hardcoded Secrets**: No credentials/keys in firmware (use SE IC for Qi auth).

#### Configuration Validation
- [ ] **config.h Consistency**: All macros align with hardware BOM (port count, charger IC, battery voltage).
- [ ] **Flash Layout**: Log, calibration, product info regions defined and tested.
- [ ] **Default Parameters**: ap_t / lib_para default values safe for all SKUs.

#### Documentation
- [ ] **Changelog Updated**: Release notes document new features, fixes, compatibility notes.
- [ ] **Agent Knowledge Updated**: If major change to agent responsibility, update `agent_knowledge_*.md`.
- [ ] **README Updated**: Build instructions, configuration steps, known issues noted.

---

## 7. Release Management

### 7.1 Version Control & Branching

```
main (stable releases)
  ├─ tag: v1.0.0_NU17112   (baseline product release)
  ├─ tag: v1.1.0_NU17112   (hotfix + feature)
  └─ tag: v1.2.0_NU17112   (major feature: UFCS support added)

development (integration point)
  ├─ feature/qi-epp-auth       (platform-wpc-protocol-agent)
  ├─ bugfix/thermal-protection (platform-wpc-hw-agent + platform-buckboost-agent)
  └─ feature/ufcs-protocol     (platform-dpdm-agent)

project/* (variant branches per SKU)
  ├─ project/nu17113_v1        (new platform variant)
  └─ project/nu17112_customerA_v2 (customer-specific config)
```

### 7.2 Release Checklist

#### Pre-Release (1 week before)
- [ ] Create release branch from `development`
- [ ] Merge all feature PRs into release branch
- [ ] Run full integration test suite (Section 5)
- [ ] Review code (Section 6.2)
- [ ] Update CHANGELOG.md
- [ ] Bump version in firmware header (e.g., `hal/overview.h` or dedicated version.h)

#### Release Day
- [ ] Create tag on main branch (`vX.Y.Z_NU17112`)
- [ ] Generate firmware binary (`build/ → *.bin`)
- [ ] Sign binary with release key (if required)
- [ ] Publish to FW/ directory
- [ ] Create GitHub Release with:
  - Changelog summary
  - Binary download link
  - Known issues / workarounds
  - Upgrade instructions

#### Post-Release
- [ ] Monitor field for issues (create tickets, assign to agents)
- [ ] Backport critical hotfixes to main (tag vX.Y.Z-hotfix-1)
- [ ] Merge stable changes back to development
- [ ] Archive old firmware versions per retention policy

### 7.3 Cloning for New Projects

When creating a new project variant (e.g., NU17113 or NU17112_CustomerB):

#### Step 1: Create Project Branch
```bash
git checkout -b project/nu17113_v1
```

#### Step 2: Clone Variant-Specific Agents
Copy & modify these agents' files:
- **platform-apl-agent**: config.h, led.c, sleep.c (port count, LED GPIO, Qi power)
- **platform-buckboost-agent**: buckboost.c, bat.c (charger IC type, battery CV/CC)
- **platform-wpc-protocol-agent**: wpc*.c, epp.c (Qi power class, EPP support)
- **platform-port-manager-agent**: port_manager.c (port topology)

#### Step 3: Keep Platform-Shared Agents Identical
Do **not** modify:
- platform-hal-agent (HAL layer unchanged)
- platform-fml-agent (framework unchanged)
- platform-usb-agent (USB PD stack unchanged)
- platform-dpdm-agent (fast-charge stack unchanged)
- platform-gauge-agent (BMS algorithm unchanged)
- platform-wpc-hw-agent (WPC hardware/algorithms unchanged)

#### Step 4: Update Knowledge Files
Create project-specific knowledge:
- `agent_knowledge_apl_nu17113.md` (inherits from agent_knowledge_apl.md, note diffs)
- `agent_knowledge_buckboost_nu17113.md`
- Commit alongside code changes

#### Step 5: Validate & Test
- [ ] All tests pass (Section 5 scenarios)
- [ ] No new regressions
- [ ] Cross-agent workflow verified
- [ ] Create pull request for code review

---

## Appendix: Key File Locations

| Aspect | Key Files |
|--------|-----------|
| **Global State** | fml/g_data.h, fml/g_data.c |
| **Configuration** | app/config.h |
| **OSAL Framework** | osal/osal.h, osal/osal.c |
| **Task Dispatch** | fml/_fml.c (FML), platform-usb-agent/*.c (USB), ... |
| **Protocol Stacks** | lib/typec.c, lib/usb_pd.c, lib/ask.c, lib/afc_scp.c, lib/ufcs.c |
| **Qi State Machines** | app/_wpc.c, app/wpc_*.c (6 phases) |
| **Power Conversion** | power/buckboost.c, power/bat.c |
| **Algorithm** | gauge/BMS_FixPoint.c, gauge/SOC.c |
| **UI/System** | app/led.c, app/gui.c, app/sleep.c, app/bat_record.c |
| **Port Arbitration** | app/port_manager.c |
| **Build System** | Debug/sources.mk, startup/ckcpu.ld |
| **Startup Code** | startup/crt0.S, app/main.c |

---

**Document Version**: 1.0 (Generated 2026-02-15)
**Target Audience**: Team Lead (powerbank-leader), Project Managers, QA Integration Testers, Code Reviewers
**Maintenance**: Update whenever major architectural change occurs or new agent added
