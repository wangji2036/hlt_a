# VINDPM Hardware Test Checklist

**Project**: TLS_PB26T_25W Powerbank
**Feature**: VINDPM (VIN Dynamic Power Management)
**Version**: 1.0
**Date**: 2026-02-15

---

## Equipment Required

| Item | Specification | Purpose |
|------|---------------|---------|
| TLS_PB26T_25W EVK board | PowerBankEvk_25W firmware loaded | DUT (Device Under Test) |
| Qi PRx receiver | BPP 5W + EPP 15W + MPP 25W (3 devices preferred) | WPC load |
| Electronic load | 0-10A, 0-20V, CV/CC/CR modes | Battery simulation / load injection |
| DC power supply | 6-10V, 5A min | Battery voltage simulation (optional) |
| Oscilloscope | >= 100 MHz, 4ch | vpwr/isns waveform capture |
| Multimeter | 4.5+ digit | Voltage/current DC measurement |
| UART-USB adapter | 115200 baud, 3.3V TTL | Firmware debug log capture |
| Serial terminal software | PuTTY / TeraTerm / Realterm | Log recording |
| Power analyzer (optional) | Input + output measurement | Efficiency measurement |
| Thermal camera (optional) | -20 to +150 C | Board thermal monitoring |
| Resistive load for PRx | 5/10/15/25 ohm set | PRx output load control |

---

## Pre-Test Setup

### Step 1: Firmware Verification

- [ ] Flash `out/PowerBankEvk_25W.elf` to DUT
- [ ] Verify firmware version via UART boot log: `TX_FW_VER = 0x16`
- [ ] Confirm VINDPM thresholds compiled correctly:
  - `CONFIG_VINDPM_WARNING_THD = 4700` (mV)
  - `CONFIG_VINDPM_CRITICAL_THD = 4500` (mV)
  - `CONFIG_VINDPM_RECOVERY_THD = 4800` (mV)
- [ ] UART connected and logging at 115200 baud
- [ ] Start serial log recording to file with timestamp

### Step 2: Battery Preparation

- [ ] 2S Li-ion battery pack connected (or DC supply set to 7.4-8.4V)
- [ ] Battery SOC verified (recommend 30-70% for VINDPM testing)
- [ ] Battery voltage measured: _____ V
- [ ] Buck-boost output (vpwr) measured at steady state: _____ mV (expect ~9000 mV)

### Step 3: Measurement Points

Connect oscilloscope probes:
- **CH1**: vpwr (buck-boost output / WPC TX input voltage)
  - Test point: NU6805 VBUS output
- **CH2**: ISNS (coil current sense, optional)
  - Test point: Current sense resistor
- **CH3**: UART TX (optional, for timing correlation)
- **CH4**: GPIO debug pin (if available)

### Step 4: PRx Preparation

- [ ] PRx device powered off, ready for placement
- [ ] PRx output load configured (resistor or electronic load)
- [ ] PRx alignment jig ready (for consistent coil coupling)

---

## Test Execution

### Test A: Baseline (No VINDPM Trigger)

**Objective**: Confirm normal operation with VINDPM inactive

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| A1 | Place PRx on TX, establish 5W BPP charging | vpwr, coil current | Stable charging, vpwr > 5.0V |
| A2 | Monitor for 2 minutes | UART log | No "DPL" messages, no anomalies |
| A3 | Record vpwr range | Oscilloscope | vpwr stays above 4800 mV |
| A4 | Remove PRx, verify idle | UART log | Clean return to idle |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

### Test B: WARNING State Trigger and CEP Clamping

**Objective**: Verify VINDPM WARNING activation and CEP behavior

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| B1 | Place PRx, establish 15W+ charging | vpwr | Stable power transfer |
| B2 | Add external load to battery to sag vpwr to ~4650 mV | vpwr on scope | vpwr drops below 4700 mV |
| B3 | Wait 200 ms (debounce period) | UART log | "DPL" appears when positive CEP arrives |
| B4 | Observe power reduction | Scope + PRx load | Coil power stops increasing |
| B5 | Verify tar_cap reduction | UART log, debugger | nego_cap decreasing over time |
| B6 | Remove extra load, let vpwr recover > 4800 mV | vpwr on scope | vpwr rises above recovery threshold |
| B7 | Wait 200 ms for recovery debounce | UART log | "DPL" messages stop |
| B8 | Verify normal CEP processing resumes | PRx charging power | Power gradually increases |

**Result**: [ ] PASS / [ ] FAIL

**Measured Values**:
- vpwr at WARNING trigger: _____ mV
- Time from threshold crossing to first DPL log: _____ ms
- Minimum tar_cap during WARNING: _____
- Recovery time to normal operation: _____ s

**Notes**: _______________________________________________

---

### Test C: CRITICAL State and Forced Power Reduction

**Objective**: Verify VINDPM CRITICAL activation and aggressive power reduction

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| C1 | Establish 15W+ WPC charging | vpwr | Stable |
| C2 | Force vpwr below 4500 mV with heavy battery load | vpwr on scope | vpwr < 4500 mV sustained |
| C3 | Observe rapid debounce (50 ms) to CRITICAL | UART log | System enters CRITICAL faster than WARNING |
| C4 | Verify CEP forced to -5 | UART + scope | PID reduces voltage ~80mV per CEP cycle |
| C5 | Verify aggressive tar_cap reduction (-4/cycle) | Debugger | tar_cap[3] drops by 4 per RPP |
| C6 | Verify XCE NAK (MPP only) | UART log | No power increase requests accepted |
| C7 | Hold CRITICAL for 30 seconds | Full monitoring | No system crash, no UVLO, vpwr stabilizes |
| C8 | Remove load, verify recovery | vpwr, UART | Flag returns to 0 after 300+ ms debounce |

**Result**: [ ] PASS / [ ] FAIL

**Measured Values**:
- vpwr at CRITICAL trigger: _____ mV
- Detection latency (WARNING->CRITICAL): _____ ms
- Minimum vpwr during CRITICAL: _____ mV
- Power at CRITICAL floor: _____ W
- Recovery time (CRITICAL->NORMAL): _____ ms

**Notes**: _______________________________________________

---

### Test D: Debounce Rejection (Brief Voltage Dip)

**Objective**: Confirm short voltage dips do not trigger false VINDPM activation

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| D1 | Establish stable WPC charging | vpwr | vpwr > 5.0V |
| D2 | Apply brief load pulse (< 50 ms) causing vpwr dip to 4600 mV | Scope | vpwr dips and recovers |
| D3 | Verify no VINDPM activation | UART log | No "DPL" messages |
| D4 | Verify charging continues normally | PRx power | No power reduction |
| D5 | Repeat 10 times | All | Consistent rejection |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

### Test E: Hysteresis Band Stability

**Objective**: Verify no oscillation when vpwr is in hysteresis band

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| E1 | Trigger WARNING state (vpwr < 4700 mV for > 100 ms) | UART | DPL messages appear |
| E2 | Raise vpwr to 4750 mV (between WARNING and RECOVERY) | Scope | vpwr stable in band |
| E3 | Hold for 60 seconds | UART + scope | State stays at WARNING, no oscillation |
| E4 | No flip-flopping between states | UART log analysis | Consistent DPL behavior |
| E5 | Raise vpwr above 4800 mV to clear | UART | DPL messages stop after debounce |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

### Test F: Multi-Port Interaction

**Objective**: Verify VINDPM works correctly with concurrent TypeC charging

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| F1 | Connect TypeC-A charger (PD 9V/3A input) | vpwr | vpwr stable |
| F2 | Place PRx, establish WPC charging simultaneously | Power measurement | Both ports active |
| F3 | Disconnect TypeC charger suddenly | vpwr on scope | vpwr drops as battery takes over |
| F4 | If vpwr drops below 4700 mV, verify VINDPM activates | UART | DPL messages, power reduction |
| F5 | WPC power reduces without crash | PRx output | Graceful degradation |
| F6 | Reconnect charger, verify recovery | All | Full power restoration |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

### Test G: Regression -- Normal Operation

**Objective**: Verify VINDPM code does not affect normal charging scenarios

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| G1 | BPP 5W charging, 10 min | vpwr, PRx power | Stable, no DPL |
| G2 | EPP 10W charging, 10 min | vpwr, PRx power | Stable, no DPL |
| G3 | EPP 15W charging, 10 min | vpwr, PRx power | Stable, no DPL |
| G4 | MPP 25W charging, 10 min | vpwr, PRx power | Stable, no DPL |
| G5 | PRx placement/removal cycle x10 | UART | Clean transitions, no stale state |
| G6 | Sleep and wake cycle x5 | UART | VINDPM init on each wake |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

### Test H: Long Duration Stress

**Objective**: Verify stability during extended VINDPM operation

| Step | Action | Measurement | Pass Criteria |
|------|--------|-------------|---------------|
| H1 | Establish WPC charging with marginal battery (vpwr ~4650 mV) | All | VINDPM WARNING active |
| H2 | Run for 30 minutes | UART log, scope | No crash, no watchdog reset |
| H3 | Verify tar_cap reaches floor and holds | Debugger | tar_cap[3] stable at 50 |
| H4 | Verify vpwr stabilizes | Scope | vpwr oscillation < 200 mV |
| H5 | Remove PRx, verify clean idle | UART | Normal idle, VINDPM resets on next init |

**Result**: [ ] PASS / [ ] FAIL

**Notes**: _______________________________________________

---

## Data Collection Template

For each test, record the following:

### Environmental Conditions

| Parameter | Value |
|-----------|-------|
| Ambient temperature | _____ C |
| Battery initial voltage | _____ V |
| Battery SOC | _____ % |
| PRx model | _____________ |
| PRx load | _____ ohm / _____ W |

### Key Measurements

| Parameter | Value | Unit |
|-----------|-------|------|
| vpwr at WARNING trigger | | mV |
| vpwr at CRITICAL trigger | | mV |
| vpwr at recovery | | mV |
| WARNING detection latency | | ms |
| CRITICAL detection latency | | ms |
| Recovery latency (WARNING->NORMAL) | | ms |
| Recovery latency (CRITICAL->NORMAL) | | ms |
| Minimum vpwr during protection | | mV |
| Minimum PRx power during protection | | W |
| Maximum board temperature during test | | C |

### UART Log Analysis

| Log Keyword | Count | First Occurrence (timestamp) |
|-------------|-------|------------------------------|
| "DPL" | | |
| "->lim-4" | | |
| "->lim-0" | | |
| "limit" | | |
| "--->tar_uvp" | | |

---

## Pass/Fail Summary

| Test | Name | Result | Notes |
|------|------|--------|-------|
| A | Baseline (No Trigger) | [ ] PASS / [ ] FAIL | |
| B | WARNING Trigger + CEP | [ ] PASS / [ ] FAIL | |
| C | CRITICAL + Forced Reduction | [ ] PASS / [ ] FAIL | |
| D | Debounce Rejection | [ ] PASS / [ ] FAIL | |
| E | Hysteresis Stability | [ ] PASS / [ ] FAIL | |
| F | Multi-Port Interaction | [ ] PASS / [ ] FAIL | |
| G | Regression | [ ] PASS / [ ] FAIL | |
| H | Long Duration Stress | [ ] PASS / [ ] FAIL | |

**Overall Result**: [ ] PASS / [ ] FAIL

**Tested by**: _______________
**Date**: _______________
**Firmware version**: TX_FW_VER 0x16
**Board revision**: _______________

---

## Known Limitations

1. **No dedicated VINDPM printk in optimized build**: Debug messages were removed to save ROM. Diagnosis relies on "DPL" and existing power limit logs. For detailed debugging, re-enable prints by changing `#if 0` to `#if 1` in the relevant code sections.

2. **VINDPM flag readable only via debugger**: No UART command to query vindpm_flag at runtime in the current firmware. Consider adding a debug command if needed for field testing.

3. **CRITICAL CEP override is unconditional**: At CRITICAL state, CEP is forced to -5 even if PRx sent a more negative value (e.g., -10). This means VINDPM may actually reduce the rate of power decrease in some edge cases.

4. **Recovery depends on external conditions**: If battery voltage cannot recover above 4800 mV even at minimum power, VINDPM will remain in WARNING/CRITICAL indefinitely. This is expected behavior -- the system operates at reduced power rather than shutting down.

5. **vpwr vs vbus distinction**: VINDPM monitors `gd->vpwr` (buck-boost output, real ADC sampling), while existing UV protections monitor `gd->vbus` (which is hardcoded to 9000 mV in some paths). Testers should be aware these are different signals.
