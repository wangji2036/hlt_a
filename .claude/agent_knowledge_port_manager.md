# PORT MANAGER Module Knowledge Base

**Jurisdiction**: `app/port_manager.c`, `app/port_manager.h`

## 1. Module Overview

The **Port Manager** is the central arbitration system for the 4-port powerbank, managing concurrent connections across:
- **PORT0/PORT1**: Type-C ports (dual-role: SINK for charging, SOURCE for discharging)
- **PORT2**: USB-A port (SOURCE only)
- **PORT3**: WPC wireless charging pad (SOURCE only)

**Core Responsibilities**:
1. **Port Arbitration**: Decides which port can operate when multiple connections occur
2. **Mode Switching**: Coordinates BUCKBOOST transitions between CHARGE_MODE and DISCHARGE_MODE
3. **Power Negotiation**: Manages PD/QC voltage requests and current limits
4. **VBUS Control**: Controls MOS gate enable/disable to prevent backflow
5. **Event Dispatching**: Routes connection/disconnection events to appropriate handlers

**Critical State Machine**: Operates in 2 states:
- `PORT_IDLE_OR_READY`: Can accept new connection events
- `PORT_INHANDLING`: Busy processing a port event (blocks new events)

## 2. Public Interface List

### Task Management
```c
void port_manager_task_init(void)
void port_manager_event_handle(uint32_t event)
void port_manager_set_event(uint32_t event)
```

### Event Handlers (Connected/Disconnected)
```c
void port_enum_scan_handle(void)          // Main event dispatcher
void port_enum_port0_connect_start(void)
void port_enum_port0_connect_success(void)
void port_enum_port0_connect_closed(void)
void port_enum_port1_connect_start(void)
void port_enum_port1_connect_success(void)
void port_enum_port1_connect_closed(void)
void port_enum_port2_connect_start(void)
void port_enum_port2_connect_success(void)
void port_enum_port2_connect_closed(void)
void port_enum_port3_connect_start(void)
void port_enum_port3_connect_success(void)
void port_enum_port3_connect_closed(void)
```

### Power Configuration
```c
void port_enum_port_snk_setvolt(void)     // Negotiate voltage (5V/9V/12V/PPS)
void port_enum_port_snk_setcharge(void)   // Set IBAT/IBUS limits
void port_enum_port_enum_done(void)       // Finalize port setup
```

## 3. Internal Logic

### 3.1 Event Scan State Machine (`port_enum_scan_handle`)

**Priority Order** (highest to lowest):
1. **PORT_IDLE_OR_READY** state required to process most events
2. **Disconnect Events** (PORT0/1/2/3_EVENT_UNCONNECT) - highest priority
3. **Connect Events** (PORT0/1/2/3_EVENT_TRY_CONNECT)
4. **Reset Charge Event** (PORT_EVENT_RESET_CHARGE) - reconfigure existing charge port

**Blocking Logic**:
```c
if(g_port.state != PORT_IDLE_OR_READY) {
    // Only allow event clearing, no new processing
    // Exception: If inhandle_port matches unconnect event, transition to IDLE
    return;
}
```

### 3.2 Port Arbitration Rules

#### CHARGE_MODE (SINK on PORT0/1):
1. **New SINK detected**: Disable all SOURCE ports, request 5V first
2. **Multi-SINK conflict**: Last-connected wins (other port restarts TypeC)
3. **SINK + SOURCE**: SINK takes priority, SOURCE limited to 5V/1.5A

#### DISCHARGE_MODE (SOURCE on any port):
1. **TYPE-C SOURCE**: Full PD negotiation, DPDM arbitration
2. **USB-A SOURCE**: Simple BC1.2 detection
3. **WPC SOURCE**: Requires other ports in specific states

#### Concurrent Scenarios:
- **PORT0 SINK + WPC SOURCE**: Wireless limited to (adapter_power - wireless_load)
- **Dual TYPE-C SOURCE**: Both downgrade to RP_1_5 (1.5A each)
- **TYPE-C SOURCE + WPC SOURCE**: TYPE-C downgrade to RP_1_5

### 3.3 VBUS Gate Control

**Enable Sequence**:
```c
// PORT0 SOURCE example
buckboost_ops.set_out(vbus, 6500);    // Pre-load to 6.5A
hal_tcpc_set_gate_en(PORT0_INDEX, true);
buckboost_ops.set_out(vbus, 3500);    // Restore to 3.5A
```

**Disable Rules**:
- Before mode switch (CHARGE ↔ DISCHARGE)
- During port conflict resolution
- On disconnect event

### 3.4 Power Negotiation Flow (SINK)

**Step 1: `port_enum_port_snk_setvolt`**
1. Enable gate on `incharge_port`
2. Request highest compatible PDO (≤12V) or PPS (16V/2.5A+)
3. Set preliminary limits: IBAT=5500mA, IBUS=PDO_current
4. Wait 500ms → trigger SETCHARGE event

**Step 2: `port_enum_port_snk_setcharge`**
1. Calculate power budget:
   - **WPC present**: `ibat = (adapter_power - wireless_power) / voltage`
   - **No WPC**: `ibat = 5000mA`, `ibus = adapter_power / voltage`
2. Apply NU6801 current limits (3A@5V, 2A@9V, 1.5A@12V)
3. Apply NTC derating (50% if OT/UT)
4. Call `buckboost_set_charge_current(ibat, ibus)`
5. Configure OVP (20V for PPS, else snk_volt)

## 4. Key Values Table

### Port Indices
| Name            | Value | Description                |
|-----------------|-------|----------------------------|
| PORT0_INDEX     | 0x00  | Type-C Port A              |
| PORT1_INDEX     | 0x01  | Type-C Port B              |
| USBA_INDEX      | 0x02  | USB-A Port (PORT2)         |
| WPC_INDEX       | 0x03  | Wireless Charging (PORT3)  |

### Port States
| State               | Value | Meaning                 |
|---------------------|-------|-------------------------|
| PORT_STATE_NONE     | 0x00  | Disconnected            |
| PORT_STATE_SOURCE   | 0x01  | Providing power (VBUS)  |
| PORT_STATE_SINK     | 0x02  | Receiving power (VBUS)  |

### Manager States
| State              | Meaning                         |
|--------------------|---------------------------------|
| PORT_IDLE_OR_READY | Can process new events          |
| PORT_INHANDLING    | Busy, blocks new event handling |

### Current Limits
| Parameter     | Value  | Context                     |
|---------------|--------|-----------------------------|
| CHG_IBUS_MIN  | 500mA  | Minimum charge current      |
| CHG_IBAT_MIN  | 200mA  | Minimum battery current     |
| Default IBAT  | 5000mA | Normal charge limit         |
| NU6801 @5V    | 3000mA | IBUS limit at 5V            |
| NU6801 @9V    | 2000mA | IBUS limit at 9V            |
| NU6801 @12V   | 1500mA | IBUS limit at 12V           |

### Event Bits
| Event Bit                  | Value      | Trigger Condition         |
|----------------------------|------------|---------------------------|
| PORT0_EVENT_TRY_CONNECT    | BIT(0)     | PORT0 attached            |
| PORT1_EVENT_TRY_CONNECT    | BIT(1)     | PORT1 attached            |
| PORT2_EVENT_TRY_CONNECT    | BIT(2)     | USB-A attached            |
| PORT3_EVENT_TRY_CONNECT    | BIT(3)     | WPC device detected       |
| PORT0_EVENT_UNCONNECT      | BIT(4)     | PORT0 detached            |
| PORT1_EVENT_UNCONNECT      | BIT(5)     | PORT1 detached            |
| PORT2_EVENT_UNCONNECT      | BIT(6)     | USB-A detached            |
| PORT3_EVENT_UNCONNECT      | BIT(7)     | WPC device removed        |
| PORT_EVENT_RESET_CHARGE    | BIT(8)     | Reconfigure charge params |

## 5. Interaction Map

### Downstream Dependencies
```
port_manager.c
├── BUCKBOOST (hal_tcpc_set_source_mode)
│   ├── BUCKBOOST_CHAGER_MODE
│   ├── BUCKBOOST_DISCHG_MODE
│   └── BUCKBOOST_SHUTDOWM_MODE
├── PD Library (pdlib_*)
│   ├── pdlib_snk_requsrt_voltage() - Request PDO
│   ├── pdlib_is_pps_sink() - Check PPS capability
│   ├── pdlib_get_tc_state() - Query TypeC state
│   └── pdlib_disable_typec() - Disable port
├── USB/DPDM
│   ├── usb_dpdm_select(port) - Switch DPDM MUX
│   └── bc12_type - BC1.2 detection result
├── WPC (_wpc.h)
│   ├── tcpm_stop_wpc() - Disable wireless charging
│   └── tcpm_update_wpc_work_mode() - Set WPC mode
└── QC (usb_qc.h)
    └── qc2_set_volt() - Request QC voltage
```

### Upstream Callers
- **TypeC Task**: Sends PORT0/1_EVENT_TRY_CONNECT on cable attach
- **USB Task**: Sends PORT2_EVENT_TRY_CONNECT on USB-A detect
- **WPC Task**: Sends PORT3_EVENT_TRY_CONNECT on RX coil detection

### Critical Globals
```c
struct port_infos g_port;  // Global port state
g_port.port_state[4]       // Each port's state (NONE/SOURCE/SINK)
g_port.inhandle_port       // Currently processing port (0-3)
g_port.incharge_port       // Port providing charge (0-1)
g_port.port_event          // Pending event bitmap
g_port.ibat_limit          // Battery current limit
g_port.ibus_limit          // Bus current limit
```

## 6. Expert Insights

### Concurrent Port Risks
1. **VBUS Backflow**: Always disable gates before mode switch to prevent reverse current
2. **DPDM Contention**: Only one port can own DPDM PHY (use `usb_dpdm_select(DPDM_PHY_OFF)`)
3. **PD Library Confusion**: pdlib tracks single active PD port via `pdlib_set_pd_port()`

### Arbitration Edge Cases
- **Fast Swap Scenario**: USER plugs/unplugs cables rapidly
  - **Risk**: Event queue overflow, state machine deadlock
  - **Mitigation**: `PORT_INHANDLING` blocks new events until current flow completes

- **WPC + SINK Together**: Wireless provides supplemental discharge power
  - **Calculation**: `ibat_limit = (adapter_power - wireless_load) / voltage`
  - **Cap**: Max 2A IBUS when WPC active

### Power Budget Calculation
```c
// Example: 45W PD adapter (9V/5A) + 10W wireless
// Scenario: PORT0 SINK (9V/5A), PORT3 SOURCE (wireless)
adapter_power = 45000mW;
wireless_load = 11000mW;  // 10W + overhead
voltage = 9000mV;
ibat = (45000 - 11000) / 9 = 3777mA → capped at 2000mA (WPC limit)
ibus = 2000mA * 0.95 = 1900mA
```

### OSAL Timer Delays
- `PORT_ENUM_PERIOD = 1ms` - Scan interval
- **SINK_SETVOLT → SINK_SETCHARGE**: 500ms delay for voltage stabilization
- **CONNECT_SUCCESS → ENUM_DONE**: 100-200ms delay for gate enable

## 7. Quick Reference

### Debugging Checklist
```bash
# Check port states
(gdb) p g_port.port_state  # [0]=PORT0, [1]=PORT1, [2]=USBA, [3]=WPC

# Check event queue
(gdb) p/x g_port.port_event

# Check handling state
(gdb) p g_port.state  # 0=IDLE, 1=INHANDLING

# Check current charge port
(gdb) p g_port.incharge_port
```

### Common Modification Points
- **Add Port 4**: Extend `port_state[5]`, add PORT4_EVENT bits, clone handler functions
- **Change Priority**: Reorder `if-else` chain in `port_enum_scan_handle()`
- **Custom Power Limits**: Modify `port_enum_port_snk_setcharge()` calculation

## 8. Reference Material Needs

### Missing Documentation
1. **VBUS Sequencing Timing**: Why 6.5A pre-load before gate enable?
2. **TypeC Library Restart**: Why `pdlib_delayms_restart_typec(200ms)` vs `pdlib_restart_typec()`?
3. **WPC Work Mode Matrix**: Full truth table for `TCPM_WPC_WORK_*` modes
4. **Dead Battery Handling**: Logic around `gd->bat_dead_flag` and `nu6801_dead_bat`

### Related Files to Consult
- `app/buckboost.c` - MODE_SWITCH implementation
- `pdlib/pdlib.c` - PD voltage request internals
- `app/_wpc.c` - WPC mode transitions
- `app/usb_qc.c` - QC2.0/3.0 voltage stepping
