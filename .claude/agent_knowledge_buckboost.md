# Platform BuckBoost Agent Knowledge

**Agent Domain**: Power management and regulation subsystem
**Jurisdiction**: 6 files - `power/buckboost.c/h`, `power/bat.c/h`, `fml/ntc.c/h`
**Last Updated**: 2026-02-15

---

## 1. Module Overview

### 1.1 Core Responsibility
The buckboost subsystem manages bidirectional DC-DC power conversion for the powerbank, controlling:
- **Discharge mode**: Battery → VBUS (boost conversion for device charging)
- **Charge mode**: VBUS → Battery (buck conversion for powerbank charging)
- **Protection**: Multi-level fault detection and safe shutdown
- **Thermal management**: NTC-based temperature protection

### 1.2 Hardware Support
Dual chip support with ops table abstraction:
- **NU6805**: Advanced buck-boost controller (BUCKBOOST_USED_NU6805=1)
- **NU6801**: Integrated buck-boost + ADC controller (BUCKBOOST_USED_NU6801=1)

### 1.3 Architecture
```
BUCKBOOST_TASK (17ms period)
├── ADC Sampling Loop (8-step rotation)
│   ├── VBAT, VBUS, IBUS, IBAT
│   ├── IAC1/IAC2 (Type-C channel currents)
│   ├── RNTC1/RNTC2 (temperature sensors)
│   └── VREF (reference voltage validation)
├── Protection Handler (buckboost_protection_handle)
├── IR Drop Compensation (buckboost_ir_drop_handle)
└── Work Mode State Machine
```

---

## 2. Public Interface

### 2.1 Primary Control APIs
```c
void buckboost_set_bus_iv(uint16_t voltage, uint16_t current, uint16_t wait, uint16_t delay)
```
- Sets VBUS output voltage/current with staged regulation
- **voltage**: Target in mV (e.g., 5000 = 5V, 20000 = 20V)
- **current**: Limit in mA
- **wait**: Delay before voltage application (ms)
- **delay**: Stabilization delay after voltage set (ms)
- Triggers VBUS discharge on voltage change (300ms dummy load)

```c
void buckboost_set_work_mode(enum buckboost_mode mode)
```
- **BUCKBOOST_SHUTDOWM_MODE**: Power off
- **BUCKBOOST_CHAGER_MODE**: Charging battery
- **BUCKBOOST_DISCHG_MODE**: Discharging to output

```c
void buckboost_set_charge_current(uint16_t ibat, uint16_t ibus)
```
- Configures charging current limits
- Uses soft-start: begins at 300mA, ramps by 100mA/500ms to target

### 2.2 Gate Control
```c
void buckboost_set_typeca_gate_en(bool en)
void buckboost_set_typecb_gate_en(bool en)
void buckboost_set_usb_a_gate_en(bool en)
```
- Controls power path MOSFETs for each output port

### 2.3 Status Query
```c
bool buckboost_regulator_done(void)  // Returns g_buckboost.regulator_state
```
- Check if voltage regulation completed (used for PD negotiation sync)

### 2.4 Global State
```c
extern struct buckboost_s g_buckboost;
extern uint8_t buckboost_protection_flag;  // 1 = system locked by fault
```

---

## 3. Internal Logic

### 3.1 BUCKBOOST_TASK Event Handlers

#### **BUCKBOOST_EVT_TIME_PERIOD** (17ms timer)
8-step ADC rotation (`get_info_step` counter):
- **Step 0**: RNTC1 channel select (NU6801 only)
- **Step 1**: Read IBAT → USB-A state detection → dead battery check
- **Step 2**: Read IBUS → Protection handler → Charge CV flag check
- **Step 3**: IR drop compensation → VBAT channel select
- **Step 4**: VREF validation (NU6801 only)
- **Step 5**: RNTC2 channel select (NU6801 only)
- **Step 6**: IAC2 channel select
- **Step 7**: IAC1 channel select

Critical checks per step:
```c
// Step 1: Dead battery detection
if (g_buckboost.adc_vbat > BAT_ACTIVE_RBATTER_V && pdlib_get_deadbat()) {
    pdlib_set_deadbat(false);
    port_manager_set_event(PORT_EVENT_RESET_CHARGE);
}

// Step 2: NU6801 charge complete detection
if (flag & 0x02) {
    g_buckboost.bat_full_flag = 1;
    hal_nu6801_open_reallow();  // For CV > 4.4V batteries
}
```

#### **BUCKBOOST_EVT_VBUS_PERIOD** (20ms timer)
- Reads VBUS ADC
- **PPS overvoltage/undervoltage protection**: If in discharge mode and VBUS deviates by >20%/15% from target for 50 cycles (1 second), triggers soft protect

```c
if (adc_vbus < target * 0.80 || adc_vbus > target * 1.15) {
    if (++adc_protect_cnt >= 50) {
        adc_protect_flag = true;  // Triggers shutdown
    }
}
```

#### **BUCKBOOST_EVT_CHAG_PERIOD** (500ms timer)
Charging soft-start ramp:
```c
if (chager_ibus_start && chager_ibus_value < limit) {
    chager_ibus_value += 100;  // 100mA increment
    buckboost_ops.set_chager_ibus_limit(chager_ibus_value);
}
```

### 3.2 Protection Handler (`buckboost_protection_handle`)

#### NU6805 Protection Bits
| Bit | Flag | Threshold | Action |
|-----|------|-----------|--------|
| 1 | VBUS_OCP | HW limit | Lock all ports |
| 2 | VBUS_SCP | HW limit | Lock all ports |
| 3 | VBAT_UVP | < 6.0V (10 cycles) | Lock, set dead battery |
| 4 | VBAT_OVP | HW limit | Lock all ports |
| 5 | VBUS_OVP | > 21.5V | Lock all ports |
| 10 | NTC_PCT | Per NTC thresholds | Disable TypeC only |
| 13 | VBUS_SOFT_PROTECT | ADC protection | Lock all ports |

#### NU6801 Protection Bits (Extended)
| Bit | Flag | Condition | Recovery |
|-----|------|-----------|----------|
| 0 | URB_DET | USB remove detection | Lock ports |
| 1 | BST_UV_FLAG | Boost undervoltage | - |
| 2 | VBAT_OV_FLAG | Battery overvoltage | - |
| 3 | VBAT_LOW_FLAG | < CONFIG_NU6801_BATLOW_VOLT | - |
| 4 | VBUS_OV_FLAG | > 20V | Lock ports |
| 7 | HFET_OCP | High-side FET overcurrent | Lock ports |
| 8 | DIS_VBAT_LOW | < 3V in discharge (20 cycles) | Lock, set `gd->bat_dead_flag` |
| 11 | ADC_ERR | VREF < 1.5V | Reinit buckboost |
| 13 | VBUS_SOFT_PROTECT | Same as NU6805 | Lock ports |
| 14 | GATE_FAULT | `nu6801_gate_err` flag | Lock ports |

#### Protection Lock Procedure
```c
// For critical faults (OVP, OCP, UVP, etc.):
pdlib_disable_typec(PORT0_INDEX);
pdlib_disable_typec(PORT1_INDEX);
hal_tcpc_set_gate_en(all_ports, false);
buckboost_set_bus_iv(5000, 3300, 0, 0);  // Reset to 5V/3.3A
pdlib_disable_usbpd();
tcpm_stop_wpc(WPC_DELAY);
buckboost_protection_flag = 1;  // Global lock
```

#### Recovery Condition
```c
if (buckboost_protection_flag && status == 0 && all_ports_idle) {
    buckboost_protection_flag = 0;
    pdlib_restart_typec(PORT0/1);
    buckboost_set_work_mode(BUCKBOOST_DISCHG_MODE);
    buckboost_set_bus_iv(5000, 3300, 0, 0);
}
```

### 3.3 IR Drop Compensation (`buckboost_ir_drop_handle`)

**Purpose**: Compensate for cable/connector resistance during high-current discharge

```c
if (woke_mode == BUCKBOOST_DISCHG_MODE && !pps_mode) {
    ir_drop = (-adc_ibus) * 100 / 1000;  // 100mV per 1A
    ir_drop = (ir_drop / 20) * 20;       // Round to 20mV steps
    if (ir_drop > 300) ir_drop = 300;    // Cap at 300mV

    // Apply after 5 consecutive stable readings
    if (ir_drop == g_buckboost.ir_drop) {
        if (++cnt_delay >= 5) {
            buckboost_ops.set_out(voltage + ir_drop, current);
        }
    }
}
```
**Example**: 3A output → +300mV compensation (5.0V becomes 5.3V internally)

### 3.4 NTC Temperature Protection (`buckboost_ntc_handle`)

**Dual-threshold design**:
1. **UT/OT flags**: Soft limits (modify current, stay operational)
2. **Lock flags**: Hard limits (trigger protection shutdown)

#### Charging Mode Thresholds (CHRG_NTC_*)
| Condition | Rntc | Temperature | Action |
|-----------|------|-------------|--------|
| UT Trigger | > 159 (15.9kΩ) | < ~15°C | Reset charge contract |
| UT Restore | < 141 (14.1kΩ) | > ~18°C | Resume normal |
| OT Trigger | < 52 (5.2kΩ) | > ~43°C | Reset charge contract |
| OT Restore | > 60 (6.0kΩ) | < ~39°C | Resume normal |
| UT Lock | > 252 (25.2kΩ) | < ~5°C | Stop charging (`ntc_stop_chrg_flag`) |
| UT Lock Restore | < 224 (22.4kΩ) | > ~8°C | Resume charging |
| OT Lock | < 44 (4.4kΩ) | > ~48°C | Stop charging |
| OT Lock Restore | > 52 (5.2kΩ) | < ~43°C | Resume charging |

#### Discharge Mode Thresholds (DISG_NTC_*)
| Condition | Rntc | Temperature | Action |
|-----------|------|-------------|--------|
| UT Trigger | > 252 (25.2kΩ) | < ~5°C | Reduce current (via `buckboost_set_bus_iv`) |
| OT Trigger | < 49 (4.9kΩ) | > ~45°C | Reduce current |
| UT Lock | > 631 (63.1kΩ) | < -15°C | Trigger `NTC_PCT` protection |
| OT Lock | < 32 (3.2kΩ) | > ~57°C | Trigger `NTC_PCT` protection |

**Debounce**: 10 cycles for UT/OT flags, 20 cycles for lock flags (NU6801 17ms period → 170ms/340ms)

**Current derating**:
```c
// In BUCKBOOST_EVT_REGULATOR_WAITDONE
if (ntc_ut_flag || ntc_ot_flag) {
    out_ibus = min(buckboost_out_current, 2500 * 4000 / voltage);
}
```
Limits to ~10W output power when temperature out of range.

### 3.5 ADC Conversion (NU6801 Specifics)

**Channel multiplexing** (`BUCKBOOST_EVT_ADC_PERIOD`):
```c
switch (nu6801_adc_chennel) {
    case NU6801_ADC_VBAT:
        vbat = row * 120 * 25 / nu6801_vref;  // 25:1 divider
        break;
    case NU6801_ADC_VBUS:
        vbus = row * 120 * 100 / nu6801_vref; // 100:1 divider
        break;
    case NU6801_ADC_IBUS:
        ibus = row * 120 * 25 / nu6801_vref;  // Sign inverted in charge mode
        break;
    case NU6801_ADC_IAC1/2:
        iac = row * 1200 * 4 / CONFIG_TYPEC_MOS_R / nu6801_vref;
        break;
    case NU6801_ADC_RNTC1/2:
        rntc = (row * level_factor) * 1200 / nu6801_vref;
        if (is_220uA) rntc /= 22;  // 220µA current source
        else rntc /= 2;             // 2mA current source
        break;
}
```

**VREF validation**:
```c
if (vref < 1500) {  // Internal reference error
    adc_err_flag = 1;  // Triggers ADC_ERR protection
}
```

---

## 4. Key Values & Constants

### 4.1 Voltage Thresholds
```c
// Dead battery detection
#define BAT_DEAD_BATTER_V        6000   // NU6805: 6.0V (2S)
#define BAT_ACTIVE_RBATTER_V     6500   // NU6805: 6.5V
#define BAT_ACTIVE_RBATTER_V     3000   // NU6801: 3.0V (1S assumed)

// OVP thresholds
#define NU6805_VBUS_OVP_TH       21500  // 21.5V
#define NU6801_VBUS_OVP_TH       20000  // 20.0V
#define CONFIG_NU6801_BATLOW_VOLT (config.h) // Typically 3000-6000mV

// ADC protection window (in discharge mode)
if (vbus < target * 0.80 || vbus > target * 1.15)  // 80-115% tolerance
```

### 4.2 Current Limits
```c
// Soft-start parameters
#define CHAGER_IBUS_START_VALUE  300   // Initial 300mA
#define CHAGER_IBUS_RAMP_STEP    100   // +100mA per 500ms

// IR drop compensation
#define IR_DROP_PER_AMP          100   // 100mV/A
#define IR_DROP_MAX              300   // Cap at 300mV
#define IR_DROP_STEP             20    // 20mV quantization
```

### 4.3 Timers
```c
#define BUCKBOOST_TIME_PERIOD    17    // Main task period (ms)
#define BUCKBOOST_VBUS_PERIOD    20    // VBUS ADC period (ms)
#define BUCKBOOST_CHAG_PERIOD    500   // Charge ramp period (ms)

// Protection debounce
#define VBAT_UVP_DEBOUNCE        10    // NU6805: 10 cycles (170ms)
#define VBAT_LOW_DEBOUNCE        20    // NU6801: 20 cycles (340ms)
#define ADC_PROTECT_DEBOUNCE     50    // VBUS: 50 cycles (1000ms)
```

### 4.4 Efficiency Model (`ibus_to_ibat`)
Linear efficiency degradation with voltage:
```c
// Buck mode (ibus < 0) or Boost mode (ibus >= 0)
if (vbus > 15000)       { k = -375; b = 981; }  // 98.1% - 1.88% @ 20V
else if (vbus > 12000)  { k = -300; b = 1000; } // 100% - 3.6% @ 20V
else if (vbus > 9000)   { k = -333; b = 980; }  // 98.0% - 2.67% @ 17V
else                    { k = -500; b = 995; }  // 99.5% - 4.5% @ 14V

efficiency = k * (vbus / 100) / 1000 + b;  // Per-mille (0.1% unit)

// Boost calculation
temp_ibat = (efficiency * (vbus * ibus / 1000)) / vbat;
```

---

## 5. Interaction Map

### 5.1 Upstream Dependencies
```
port_manager.c
├── PORT_EVENT_RESET_CHARGE → Triggers charge renegotiation
└── Uses g_port.port_state[] for multi-port status

pdlib (PD library)
├── pdlib_set_deadbat(bool) → Dead battery flag management
├── pdlib_get_deadbat() → Query dead battery state
├── pdlib_is_pps_source() → Check if PPS contract active
└── pdlib_disable_usbpd() → Emergency PD shutdown

tcpm (Type-C port manager)
├── tcpm_stop_wpc(WPC_DELAY) → Disable wireless charging
├── tcpm_update_wpc_work_mode(mode) → Set WPC mode
└── tcpm_disable_usba_detect() → Disable USB-A detection

hal_tcpc_set_gate_en(port, bool) → Direct gate control
```

### 5.2 Hardware Layer (buckboost_ops table)
All chip-specific functions abstracted via function pointers:
```c
// NU6805 implementation               // NU6801 implementation
hal_nu6805_buckboost_init()            hal_nu6801_buckboost_init()
hal_nu6805_buckboost_set_mode()        hal_nu6801_buckboost_set_mode()
hal_nu6805_buckboost_set_busiv()       hal_nu6801_buckboost_set_busiv()
hal_nu6805_buckboost_get_bat_voltage() hal_nu6801_buckboost_get_bat_voltage()
// ... (25+ function pointers)
```

### 5.3 Downstream Consumers
```
LED module → Reads buckboost_protection_flag for fault indication
Battery module (bat.c/h) → Minimal/empty (likely deprecated)
Port manager → Polls buckboost_regulator_done() before PD messages
PPS control → Uses IR drop compensation flag to disable compensation
```

---

## 6. Expert Insights

### 6.1 Critical Design Patterns

**Pattern 1: Staged Voltage Regulation**
```c
buckboost_set_bus_iv(voltage, current, wait, delay)
    ↓ (Triggers VBUS discharge if voltage changed)
    Event: BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT
    ↓
    Timer: wait ms → BUCKBOOST_EVT_REGULATOR_WAITDONE
    ↓ (Apply voltage + IR drop)
    buckboost_ops.set_out(voltage + ir_drop, current)
    ↓
    Timer: delay + 10ms → BUCKBOOST_EVT_REGULATOR_DELAYDONE
    ↓
    g_buckboost.regulator_state = 1  // Signal completion
```
This ensures safe voltage transitions with dummy load discharge and stabilization time.

**Pattern 2: Protection Lock/Unlock State Machine**
- **Lock**: Any fault → `buckboost_protection_flag = 1` → Disable all ports → Set 5V output
- **Unlock**: `status == 0` AND all ports idle → Restart TypeC → Re-enable discharge mode
- **Prevents oscillation**: Once locked, requires ALL protections to clear before recovery

**Pattern 3: Dual-Threshold NTC Design**
- **Soft limits** (UT/OT flags): Reduce power but stay operational
- **Hard limits** (Lock flags): Trigger full protection shutdown
- **Hysteresis**: Restore thresholds offset by 3-5°C to prevent chattering

### 6.2 Common Pitfalls

**Pitfall 1: IBAT calculation vs IPGA**
```c
#if(!CONFIG_SUPPORT_IPGA)
    g_buckboost.adc_ibat = ibus_to_ibat(adc_ibus, adc_vbus, adc_vbat);
#endif
```
- When IPGA (precision current sense) enabled, discard NU6801's IBAT ADC
- Use software efficiency model instead of hardware current sense
- **Reason**: NU6801 IBAT ADC has accuracy issues at high currents

**Pitfall 2: ADC protection false triggers**
The 50-cycle debounce is ONLY active when:
```c
if (woke_mode == DISCHG && (port[0] == SOURCE || port[1] == SOURCE))
```
- Prevents nuisance trips during idle/charging states
- But means no protection during charging mode overvoltage

**Pitfall 3: Dead battery flag confusion**
Three separate flags:
- `pdlib_get_deadbat()`: PD library's persistent flag
- `gd->bat_dead_flag`: Global data structure flag
- `nu6801_dead_bat`: NU6801-specific flag

All must be synchronized to avoid inconsistent port state.

### 6.3 NU6801 vs NU6805 Key Differences

| Feature | NU6805 | NU6801 |
|---------|--------|--------|
| ADC integration | External ADC via I2C | Integrated 12-bit ADC |
| VREF validation | Not present | < 1.5V triggers ADC_ERR |
| Dead battery threshold | 6.0V (2S) | 3.0V (1S capable) |
| Charge complete detection | External | Hardware CV flag (0x02) |
| Gate fault detection | Not implemented | `nu6801_gate_err` flag |
| IR drop compensation | Always enabled | Disabled in PPS mode |
| NTC current source | Fixed | Dual (220µA / 2mA switchable) |

---

## 7. Quick Reference

### 7.1 State Query Checklist
```c
// Before PD voltage change
if (!buckboost_regulator_done()) wait_or_retry();

// Check if system locked
if (buckboost_protection_flag) {
    // All ports disabled, investigate g_buckboost.protect_status
}

// Get current mode
switch (g_buckboost.woke_mode) {
    case BUCKBOOST_SHUTDOWM_MODE: // Powered off
    case BUCKBOOST_CHAGER_MODE:   // Charging battery
    case BUCKBOOST_DISCHG_MODE:   // Powering output
}

// Check temperature status
if (ntc_ut_flag || ntc_ot_flag) {
    // Power derating active (10W limit)
}
if (ntc_lock_flag || ntc_stop_chrg_flag) {
    // Hard temperature limit, system locked
}
```

### 7.2 Modify Charging Current
```c
// Set 3A battery / 1.5A bus limit
buckboost_set_charge_current(3000, 1500);
// → Starts at 300mA, ramps to 1500mA over 6 seconds
// → IBAT immediately set to 3000mA limit
```

### 7.3 Modify Output Voltage/Current
```c
// PD 9V/3A negotiation
buckboost_set_bus_iv(9000, 3000, 100, 200);
// → 300ms VBUS discharge
// → 100ms wait
// → Set 9V + IR_drop (up to 9.3V under load)
// → 210ms delay
// → regulator_state = 1

// PPS 11V/5A (no IR compensation)
buckboost_set_bus_iv(11000, 5000, 0, 50);
```

### 7.4 Force Protection Recovery
```c
buckboost_fault_restore();
// → Clears buckboost_protection_flag
// → Restarts TypeC ports 0/1
// → Reinitializes buckboost chip
// → Sets discharge mode + 5V/3.3A
// USE WITH CAUTION: Only for manual override after fixing root cause
```

### 7.5 Typical Event Flow (Discharge)
```
1. System startup
   → buckboost_task_init()
   → buckboost_ops.init()
   → Set BUCKBOOST_DISCHG_MODE

2. Device connected
   → PD negotiation (port_manager)
   → buckboost_set_bus_iv(9000, 3000, 100, 200)
   → VBUS discharge 300ms
   → Regulation complete

3. High current draw (3A)
   → IR drop handler: +300mV compensation
   → Output = 9300mV internally
   → Device sees ~9000mV

4. Overcurrent fault
   → buckboost_protection_handle()
   → HFET_OCP detected
   → Lock all ports
   → buckboost_protection_flag = 1

5. Fault clears
   → Status = 0 for sustained period
   → Auto-recovery to 5V/3.3A
   → TypeC ports restart
```

---

## 8. Reference Needs

### 8.1 External Knowledge Required
When debugging issues, cross-reference:
- **port_manager.c**: Port state machine (`g_port.port_state[]`)
- **pdlib**: Dead battery management, PPS contract details
- **config.h**: Compile-time thresholds (BATLOW_VOLT, TYPEC_MOS_R, SUPPORT_IPGA)
- **nu6801.c/nu6805.c**: HAL implementation of `buckboost_ops` table
- **g_data.h**: Global flags (`gd->bat_dead_flag`, `gd->bat_dead_flag_with_snk0/1`)

### 8.2 Hardware Register Maps
Consult NU6801/NU6805 datasheets for:
- REG_TEMP_STAT (0x??) bit definitions
- REG_AMUX_CTRL (0x11) ADC multiplexer
- REG_MISC_CTRL (0x10) mode control
- REG_BUBO_FAULT_FLAG (0x06) protection flags

### 8.3 NTC Lookup Table
`ntc.h` defines resistance-to-temperature mapping for 10K 3435 thermistor:
- Use for reverse-calculating actual temperatures from Rntc thresholds
- Example: `NTC_10K_3435_REAL_RT_25 = 100` (100 = 10.0kΩ @ 25°C)

---

## Notes
- **bat.c/bat.h are empty**: Likely legacy files, battery functions moved to buckboost.c
- **Critical race condition**: ADC protection uses `static bool adc_protect_flag` shared across modes → ensure atomic access
- **NU6801 dead battery patch**: `hal_nu6801_deadbat_patch()` called every 17ms in step 3, likely handles startup edge cases
