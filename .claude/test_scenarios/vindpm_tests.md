# VINDPM Test Protocol

**Project**: TLS_PB26T_25W Powerbank
**Feature**: VINDPM (VIN Dynamic Power Management)
**Version**: 1.0
**Date**: 2026-02-15
**Commit**: 43335f3 + subsequent optimization commits

---

## 1. Build Quality Report

### 1.1 ROM/RAM Usage Summary

| Metric | out/ (Optimized) | Debug/ (Unoptimized) | Delta |
|--------|------------------|----------------------|-------|
| RO Size (Code + RO Data) | 118,876 bytes (116.09 KB) | 122,048 bytes (119.19 KB) | -3,172 bytes |
| RW Size (RW Data + ZI Data) | 6,056 bytes (5.91 KB) | 6,084 bytes (5.94 KB) | -28 bytes |
| ROM Total (Code + RO + RW) | 119,428 bytes (116.63 KB) | 122,600 bytes (119.73 KB) | -3,172 bytes |

### 1.2 Resource Budget Analysis

| Resource | Used | Total | Remaining | Margin |
|----------|------|-------|-----------|--------|
| Flash (ROM) | 119,428 bytes | 122,880 bytes (120 KB) | 3,452 bytes | 2.8% |
| SRAM (RW+ZI) | 6,056 bytes | 8,192 bytes (8 KB) | 2,136 bytes | 26.1% |

### 1.3 VINDPM Code Footprint

| Component | Estimated Size | Location |
|-----------|---------------|----------|
| `fml_vindpm_init()` | ~20 bytes | prot.c:741-746 |
| `fml_vindpm_check()` | ~80 bytes | prot.c:748-783 |
| Static variables (`vd_over`, `vd_reco`) | 2 bytes SRAM | prot.c:739 |
| Bitfield `vindpm_flag:2` | 0 bytes (reuses padding) | g_data.h:357 |
| CEP control in `_wpc.c` | ~40 bytes | _wpc.c:602-609 |
| PID block in `pid.c` | ~20 bytes | pid.c:264 |
| XCE NAK in `wpc_xfer.c` | ~30 bytes | wpc_xfer.c:639-642 |
| `tar_cap` update in `prot.c` | ~40 bytes | prot.c:1166-1170 |
| Config defines in `config.h` | 0 bytes (compile-time) | config.h:64-67 |
| **Total VINDPM** | **~230 bytes ROM, 2 bytes SRAM** | |

### 1.4 Optimization Impact

7 `printk` calls were disabled via `#if 0` in prot.c (`power_capability_init`, `power_limit_sync`) to save ROM space. The optimized VINDPM state machine uses a compact `cur/tgt` comparison pattern instead of the 3-way switch-case from the design document, saving approximately 60-80 bytes of code.

### 1.5 Build Verification Checklist

- [ ] `out/PowerBankEvk_25W.elf` exists and links successfully
- [ ] `Debug/PowerBankEvk_25W.elf` exists for debug builds
- [ ] ROM total < 120 KB (current: 116.63 KB) -- PASS
- [ ] SRAM total < 7 KB operational (current: 5.91 KB) -- PASS
- [ ] No new linker warnings

---

## 2. Functional Verification Tests

### 2.1 State Machine Transitions

#### TC-VINDPM-F01: NORMAL to WARNING Transition

**Preconditions**:
- WPC TX in XFER phase, actively charging a PRx
- `vindpm_flag == 0` (NORMAL)
- `vpwr` stable above 4800 mV

**Steps**:
1. Gradually reduce vpwr below 4700 mV (simulate battery sag via load increase)
2. Hold vpwr at 4600 mV for at least 200 ms

**Expected Results**:
- `vindpm_flag` remains 0 for at least 100 ms (debounce: 10 samples * 10 ms)
- After 11+ consecutive samples below 4700 mV, `vindpm_flag` transitions to 1
- `vd_over` counter resets to 0 after transition
- `vd_reco` counter resets to 0 after transition

**Verification Method**: UART log -- no explicit printk in optimized code, verify via `gd->power_limit_sts.vindpm_flag` readback or adding temporary debug print

---

#### TC-VINDPM-F02: WARNING to CRITICAL Transition

**Preconditions**:
- `vindpm_flag == 1` (WARNING)
- `vpwr` between 4500-4700 mV

**Steps**:
1. Further reduce vpwr below 4500 mV
2. Hold for at least 100 ms

**Expected Results**:
- After 6+ consecutive samples below 4500 mV (debounce: `(tgt==2) ? 5 : 10`), `vindpm_flag` transitions to 2
- Faster transition than NORMAL->WARNING (50 ms vs 100 ms)

---

#### TC-VINDPM-F03: CRITICAL to WARNING Recovery

**Preconditions**:
- `vindpm_flag == 2` (CRITICAL)
- `vpwr` below 4500 mV

**Steps**:
1. Increase vpwr to 4600 mV (above CRITICAL_THD but below RECOVERY_THD)
2. Hold stable for at least 400 ms

**Expected Results**:
- Since `tgt` would be 1 (between CRITICAL and WARNING thresholds): `tgt < cur` path
- Recovery debounce: `(cur==2) ? 30 : 10` = 30 samples = 300 ms
- After 31+ consecutive samples, `vindpm_flag` transitions from 2 to 1 (not directly to 0)

**Note**: The actual implementation always uses `tgt = cur` when vpwr is between thresholds (4500-4700 range maps to tgt=1 when in CRITICAL). Recovery from CRITICAL to WARNING requires vpwr in the 4500-4700 band OR above 4800.

---

#### TC-VINDPM-F04: CRITICAL to NORMAL Direct Recovery

**Preconditions**:
- `vindpm_flag == 2` (CRITICAL)

**Steps**:
1. Increase vpwr above 4800 mV (RECOVERY_THD)
2. Hold stable for at least 400 ms

**Expected Results**:
- `tgt = 0` (since vpwr > RECOVERY_THD)
- `tgt < cur` (0 < 2), recovery path with debounce `(cur==2) ? 30 : 10` = 30
- After 31 samples (310 ms), `vindpm_flag` transitions directly from 2 to 0

---

#### TC-VINDPM-F05: WARNING to NORMAL Recovery

**Preconditions**:
- `vindpm_flag == 1` (WARNING)

**Steps**:
1. Increase vpwr above 4800 mV
2. Hold stable for at least 200 ms

**Expected Results**:
- `tgt = 0`, `cur = 1`, debounce `(cur==2) ? 30 : 10` = 10
- After 11 samples (110 ms), `vindpm_flag` transitions from 1 to 0

---

#### TC-VINDPM-F06: Hysteresis Band -- No State Change

**Preconditions**:
- `vindpm_flag == 1` (WARNING)

**Steps**:
1. Set vpwr to 4750 mV (between WARNING_THD=4700 and RECOVERY_THD=4800)
2. Hold for 500 ms

**Expected Results**:
- `tgt = cur = 1` (vpwr is not below any threshold and not above RECOVERY)
- Both `vd_over` and `vd_reco` reset to 0 each cycle
- State remains WARNING indefinitely -- no oscillation

---

#### TC-VINDPM-F07: Debounce Rejection -- Brief Voltage Dip

**Preconditions**:
- `vindpm_flag == 0` (NORMAL)

**Steps**:
1. Briefly dip vpwr below 4700 mV for 5 samples (50 ms)
2. Return vpwr above 4700 mV

**Expected Results**:
- `vd_over` increments to 5, then resets to 0 when vpwr recovers
- `vindpm_flag` remains 0 -- transition requires >10 samples

---

### 2.2 CEP Control Verification

#### TC-VINDPM-F10: WARNING State CEP Clamping

**Preconditions**:
- `vindpm_flag == 1` (WARNING)
- WPC in XFER phase, PRx sending CEP packets

**Steps**:
1. PRx sends positive CEP (e.g., cep_val = +3, requesting more power)
2. Observe CEP value at WPC_EVT_PCH_TO handler

**Expected Results**:
- `_wpc.c:607`: `gd->rx_infos.cep_val` clamped to 0 (positive CEP blocked)
- Negative CEP values pass through unchanged (PRx can still request power reduction)
- Zero CEP passes through unchanged

---

#### TC-VINDPM-F11: CRITICAL State Forced Negative CEP

**Preconditions**:
- `vindpm_flag == 2` (CRITICAL)

**Steps**:
1. PRx sends any CEP (positive, zero, or mildly negative like -2)
2. Observe CEP at WPC_EVT_PCH_TO handler

**Expected Results**:
- `_wpc.c:605-606`: `gd->rx_infos.cep_val` forced to -5 regardless of original value
- Only CEP values already <= -5 (e.g., -10) pass through unchanged (CEP=-5 is the override floor)

**Note**: In the actual code, the check is `vindpm_flag == 2` sets cep_val = -5 unconditionally. If original CEP was -10, it gets overwritten to -5 (less aggressive). This is a potential design consideration.

---

#### TC-VINDPM-F12: PID Positive CEP Blocking

**Preconditions**:
- `vindpm_flag` != 0 (any VINDPM active state)

**Steps**:
1. Force a positive CEP to reach `pid_cep_handler()` (e.g., via load jump event path)

**Expected Results**:
- `pid.c:264`: `cep > 0 && gd->power_limit_sts.vindpm_flag` evaluates true
- PID handler returns early, no power increase applied
- Log output: " DPL"

---

#### TC-VINDPM-F13: NORMAL State CEP Passthrough

**Preconditions**:
- `vindpm_flag == 0` (NORMAL)

**Steps**:
1. PRx sends various CEP values (+5, 0, -5)

**Expected Results**:
- All CEP values pass through VINDPM checks unmodified
- PID processes CEP normally

---

### 2.3 tar_cap Adjustment Verification

#### TC-VINDPM-F20: WARNING tar_cap Reduction

**Preconditions**:
- `vindpm_flag == 1` (WARNING)
- `vbus_uv_flag == 0` (no UV protection active)
- `pwr_lim.tar_cap[uvp]` at initial value (e.g., 250 = 25W)

**Steps**:
1. Wait for `power_limit_tar_cap_update()` call (triggered by WPC_EVT_FOD_REPORTED)
2. Observe tar_cap[uvp] value

**Expected Results**:
- `prot.c:1168-1169`: tar_cap[uvp] decremented by 2
- Floor check: tar_cap[uvp] must remain >= 50 (5W)
- After `power_limit_sync()`, `gd->tx_infos.nego_cap` reflects new minimum

---

#### TC-VINDPM-F21: CRITICAL tar_cap Aggressive Reduction

**Preconditions**:
- `vindpm_flag == 2` (CRITICAL)
- `vbus_uv_flag == 0`

**Steps**:
1. Wait for `power_limit_tar_cap_update()` call

**Expected Results**:
- tar_cap[uvp] decremented by 4 per cycle
- Floor: tar_cap[uvp] >= 32 (3.2W)
- Faster reduction than WARNING state

---

#### TC-VINDPM-F22: tar_cap Recovery When VINDPM Clears

**Preconditions**:
- `vindpm_flag == 0` (NORMAL, after recovery from WARNING/CRITICAL)
- `tar_cap[uvp]` was reduced during VINDPM event
- `vbus_uv_flag == 0`

**Steps**:
1. Verify `gd->vbus > ap->vbus_dpl_thd + 500`
2. Wait for `power_limit_tar_cap_update()` calls

**Expected Results**:
- `prot.c:1171-1177`: Recovery branch activates
- tar_cap[uvp] increments by +5 per cycle
- Only increments when `tar_cap[uvp] == nego_cap` AND `tar_cap[uvp] < max_cap`
- Recovery continues until tar_cap[uvp] reaches max_cap

---

#### TC-VINDPM-F23: VINDPM Priority vs vbus_uv_flag

**Preconditions**:
- Both `vbus_uv_flag == 1` AND `vindpm_flag == 1`

**Steps**:
1. Trigger both conditions simultaneously
2. Observe `power_limit_tar_cap_update()` behavior

**Expected Results**:
- `prot.c:1152-1170`: `vbus_uv_flag` check comes first (if-else chain)
- When `vbus_uv_flag` is set, its branch executes, VINDPM branch skipped
- VINDPM tar_cap adjustment only active when `vbus_uv_flag == 0`

---

### 2.4 MPP XCE NAK Verification

#### TC-VINDPM-F30: XCE NAK During VINDPM Active

**Preconditions**:
- `vindpm_flag` != 0
- MPP connection established (Qi 2.x)

**Steps**:
1. PRx sends XCE (Extended Control Error) power increase request
2. Observe TX response in wpc_xfer.c

**Expected Results**:
- `wpc_xfer.c:639-642`: VINDPM check takes priority (first in else-if chain)
- TX responds with FSK NAK
- PRx power request denied

---

#### TC-VINDPM-F31: XCE ACK When VINDPM Cleared

**Preconditions**:
- `vindpm_flag == 0`
- No other power limit conditions active

**Steps**:
1. PRx sends XCE power increase request

**Expected Results**:
- VINDPM check skipped (vindpm_flag == 0)
- Normal XCE processing continues
- XCE ACK sent if within power capability

---

## 3. Performance Tests

### 3.1 Power Reduction Effectiveness

#### TC-VINDPM-P01: Verify Power Reduction Under Load

**Setup**:
- PRx: Standard Qi receiver with known load (e.g., 15W)
- Battery: 2S Li-ion at moderate SOC (30-50%)
- Measurement: Power analyzer on TX coil output

**Steps**:
1. Establish stable 15W WPC charging
2. Add parallel load to battery to force vpwr sag
3. Monitor TX output power over 10 seconds

**Expected Results**:
- Power begins reducing within 200 ms of VINDPM WARNING trigger
- At CRITICAL, power reduction rate increases (CEP=-5 forces ~80mV/cycle PID adjustment)
- Coil power drops to match tar_cap floor (WARNING: 5W min, CRITICAL: 3.2W min)
- No system shutdown or buck-boost UVLO during controlled reduction

**Pass Criteria**: TX output power decreases monotonically while VINDPM is active, vpwr stabilizes above CRITICAL threshold

---

#### TC-VINDPM-P02: Voltage Stability During VINDPM Operation

**Setup**: Same as TC-VINDPM-P01

**Steps**:
1. Trigger VINDPM WARNING condition
2. Record vpwr waveform for 30 seconds

**Expected Results**:
- vpwr oscillation amplitude < 200 mV peak-to-peak after VINDPM stabilizes
- No sustained oscillation between WARNING and NORMAL states
- Hysteresis band (4700-4800 mV) prevents chattering

**Pass Criteria**: vpwr variance < 200 mV after initial 2-second transient period

---

#### TC-VINDPM-P03: Response Time Measurement

**Setup**: Oscilloscope on vpwr, UART log with timestamps

**Steps**:
1. Apply sudden load step causing vpwr to drop from 5.0V to 4.5V
2. Measure time from vpwr crossing 4700 mV to first CEP modification

**Expected Results**:
| Parameter | Expected | Acceptable Range |
|-----------|----------|-----------------|
| Detection latency (to WARNING) | 100 ms | 80-150 ms |
| Detection latency (WARNING to CRITICAL) | 50 ms | 30-80 ms |
| First CEP intervention | Next CEP cycle (~250 ms from WARNING) | 100-500 ms |
| tar_cap first reduction | Next RPP cycle (~1-2s) | 0.5-3s |
| Total stabilization | ~3-5s | 2-10s |

---

### 3.2 Recovery Performance

#### TC-VINDPM-P10: Recovery Speed After Load Removal

**Steps**:
1. Trigger CRITICAL state (vindpm_flag == 2)
2. Remove extra battery load
3. Measure time from vpwr crossing 4800 mV to vindpm_flag == 0

**Expected Results**:
- CRITICAL->NORMAL recovery debounce: 300 ms (30 samples)
- tar_cap[uvp] recovery: +5 per RPP cycle, dependent on how far tar_cap was reduced
- Full power restoration: depends on tar_cap reduction depth

---

## 4. Boundary Condition Tests

### 4.1 Voltage Jitter at Thresholds

#### TC-VINDPM-B01: vpwr Oscillating Around WARNING Threshold

**Setup**: Use variable power supply to inject controlled noise

**Steps**:
1. Set vpwr to oscillate between 4680 mV and 4720 mV (crosses 4700 mV threshold)
2. Oscillation period: 20 ms (crossing threshold every 10 ms = 1 sample)

**Expected Results**:
- `vd_over` increments on below-threshold samples, resets on above-threshold samples
- `vindpm_flag` never transitions (counter never reaches 11 consecutive)
- System remains stable in NORMAL state

---

#### TC-VINDPM-B02: vpwr Exactly at Threshold Values

**Steps**:
1. Set vpwr = 4700 mV exactly (WARNING threshold)
2. Verify: `vpwr < CONFIG_VINDPM_WARNING_THD` -> 4700 < 4700 = false
3. Set vpwr = 4699 mV
4. Verify: 4699 < 4700 = true

**Expected Results**:
- At exactly threshold value: condition is `<` (strict less-than), so 4700 mV does NOT trigger WARNING
- At 4699 mV: triggers WARNING debounce counter
- Same logic applies for CRITICAL (4500) and RECOVERY (4800)

---

#### TC-VINDPM-B03: vpwr at Recovery Threshold Boundary

**Steps**:
1. In WARNING state, set vpwr = 4800 mV
2. Verify: `vpwr > CONFIG_VINDPM_RECOVERY_THD` -> 4800 > 4800 = false
3. Set vpwr = 4801 mV
4. Verify: 4801 > 4800 = true

**Expected Results**:
- At exactly 4800 mV: recovery NOT triggered (strict greater-than)
- At 4801 mV: recovery debounce begins

---

### 4.2 Extreme Load Conditions

#### TC-VINDPM-B10: Rapid vpwr Collapse (Worst Case)

**Steps**:
1. Force vpwr from 5000 mV to 3800 mV in < 50 ms (simulate dead short on battery)
2. Observe system behavior

**Expected Results**:
- vpwr crosses both WARNING and CRITICAL thresholds rapidly
- CRITICAL debounce (5 samples = 50 ms) may trigger if collapse is slow enough
- If collapse is faster than 50 ms, hardware UV protection (CONFIG_VBUS_UV_THRESHOLD) should intervene
- VINDPM is a software protection layer -- hardware UV is the last resort

---

#### TC-VINDPM-B11: vpwr Already Below CRITICAL at Startup

**Steps**:
1. Start WPC TX when battery is severely depleted (vpwr < 4500 mV)
2. Observe VINDPM initialization and behavior

**Expected Results**:
- `fml_vindpm_init()` sets `vindpm_flag = 0`
- First `fml_vindpm_check()` call will start debounce toward CRITICAL
- After 6 samples, transitions NORMAL->WARNING (or NORMAL->CRITICAL via tgt=2)
- Actually: with vpwr < 4500, `tgt = 2`, `cur = 0`, `tgt > cur`, debounce `(tgt==2) ? 5 : 10` = 5, so 6 samples (60 ms) to CRITICAL directly

---

#### TC-VINDPM-B12: Maximum Duration VINDPM Active

**Steps**:
1. Maintain vpwr at 4600 mV (WARNING state) for 10 minutes
2. Observe tar_cap[uvp] and system stability

**Expected Results**:
- tar_cap[uvp] decreases by 2 per RPP cycle (~1-2s per cycle)
- After reaching floor of 50, no further reduction
- System continues operating at minimum 5W output
- No memory leaks or counter overflow (uint8_t vd_over/vd_reco)

---

### 4.3 Multi-Port Conflict Scenarios

#### TC-VINDPM-B20: VINDPM + TypeC Simultaneous Load

**Steps**:
1. WPC TX charging PRx at 15W
2. TypeC port simultaneously sourcing power (PD 5V/3A)
3. Total load exceeds battery capability, vpwr drops

**Expected Results**:
- VINDPM activates on WPC channel
- TypeC port managed by `port_manager.c` independently
- No interference between VINDPM and TypeC power negotiation
- Both ports may experience reduced power

---

#### TC-VINDPM-B21: VINDPM + vbus_uv_flag Coexistence

**Steps**:
1. Trigger both vbus_uv_flag (from buckboost UV detection) and vindpm_flag simultaneously
2. Observe `power_limit_tar_cap_update()` behavior

**Expected Results**:
- `vbus_uv_flag` branch takes priority in if-else chain (prot.c:1152)
- VINDPM tar_cap adjustment deferred until vbus_uv_flag clears
- Both flags can be set simultaneously in `power_limit_sts` bitfield without conflict
- CEP control in `_wpc.c` checks both: `vbus_uv_flag` path and `vindpm_flag` path are separate
- PID blocking checks both with OR: `cep > 0 && (vbus_uv_flag || vindpm_flag)`

---

#### TC-VINDPM-B22: VINDPM + Thermal Throttle (tntc_ot_flag)

**Steps**:
1. Trigger VINDPM WARNING while thermal throttle (tntc_ot_flag) is also active
2. Observe tar_cap behavior

**Expected Results**:
- VINDPM uses `tar_cap[uvp]` slot (reason index 3)
- Thermal uses `tar_cap[otp]` slot (reason index 4)
- `find_min_cap()` selects the minimum across all reasons
- Both protections apply simultaneously, most restrictive wins

---

## 5. Debug Capability Verification

### TC-VINDPM-D01: UART Log Verification

**Note**: The optimized VINDPM implementation removed explicit `printk` calls for ROM savings. Debug verification relies on:

1. **PID DPL print**: `pid.c:266` -- prints " DPL" when positive CEP is blocked (both vbus_uv and vindpm)
2. **Power limit prints**: `_wpc.c:583,591` -- prints " ->lim-4" and " ->lim-0" for power_limit_state changes
3. **tar_cap print**: `prot.c:1162` -- prints " --->tar_uvp %d" when vbus_uv_flag triggers tar_cap reduction
4. **XCE limit print**: `wpc_xfer.c:645` -- prints "limit %d %d %d %d" for power limit NAK

**Steps**:
1. Connect UART at 115200 baud
2. Trigger VINDPM WARNING/CRITICAL states
3. Search logs for "DPL", "->lim", "tar_uvp", "limit" keywords

**Expected Results**:
- "DPL" appears when positive CEP is blocked during VINDPM active
- Power limit state changes visible through existing printk infrastructure
- No dedicated "VINDPM:" prefix messages in optimized build (removed for ROM)

---

### TC-VINDPM-D02: Runtime State Readback

**Steps**:
1. Read `gd->power_limit_sts.vindpm_flag` via debugger or memory dump
2. Read `pwr_lim.tar_cap[3]` (uvp slot) for current power cap
3. Read `gd->tx_infos.nego_cap` for effective negotiated capability
4. Read `gd->tx_infos.power_limit_reason` for active limit reason

**Expected Results**:
- vindpm_flag: 0/1/2 matches expected state
- tar_cap[3]: decreasing when VINDPM active, increasing during recovery
- nego_cap: reflects minimum of all tar_cap[] entries
- power_limit_reason: 3 (uvp) when VINDPM is the most restrictive limit

---

## 6. Regression Tests

### TC-VINDPM-R01: Normal WPC Charging (No VINDPM Trigger)

**Steps**:
1. Full battery (vpwr > 5V stable), charge standard PRx at 5W/10W/15W/25W
2. Run for 30 minutes

**Expected Results**:
- vindpm_flag remains 0 throughout
- CEP processing unchanged from pre-VINDPM behavior
- No impact on charging efficiency, FOD, or MPP negotiation
- tar_cap[uvp] remains at max_cap

---

### TC-VINDPM-R02: Existing UV Protection Unchanged

**Steps**:
1. Verify `fml_vbus_uvp_check()` still operates on `gd->vbus` (not vpwr)
2. Verify `fml_vbus_dpl_check()` still operates on `gd->vbus`
3. Trigger vbus_uv_flag via buckboost UV detection path

**Expected Results**:
- Existing UV protections unaffected by VINDPM addition
- `gd->vbus` is a separate signal from `gd->vpwr`
- UV protection still triggers `wpc_stop_to_idle()` when vbus_uvp_flag set

---

### TC-VINDPM-R03: Thermal Protection Unchanged

**Steps**:
1. Trigger thermal protection (NTC OTP/OTW) with VINDPM inactive
2. Verify thermal protection behavior matches pre-VINDPM baseline

**Expected Results**:
- tntc_otp triggers `wpc_stop_to_idle()` as before
- tntc_otw throttles via `tar_cap[otp]` as before
- No interaction with VINDPM when vindpm_flag == 0

---

### TC-VINDPM-R04: EPP/MPP Negotiation Unaffected

**Steps**:
1. Connect EPP-capable PRx, negotiate 15W
2. Connect MPP-capable PRx, negotiate 25W
3. Verify full negotiation sequence completes

**Expected Results**:
- VINDPM does not interfere with negotiation phases (only active in XFER)
- `fml_vindpm_check()` is called from 010ms poll regardless of phase, but CEP/XCE controls only apply during XFER
- Negotiated power level correct

---

### TC-VINDPM-R05: Sleep/Wake Cycle

**Steps**:
1. Enter sleep mode
2. Wake on PRx placement
3. Verify VINDPM state after wake

**Expected Results**:
- `fml_vindpm_init()` called during `apl_task_init()` -- resets state on boot
- After wake, vindpm_flag == 0 (clean state)
- No stale VINDPM state from previous session

---

## 7. Test Matrix Summary

| Test ID | Category | Priority | Estimated Time |
|---------|----------|----------|---------------|
| TC-VINDPM-F01 | State Machine | P0 | 10 min |
| TC-VINDPM-F02 | State Machine | P0 | 10 min |
| TC-VINDPM-F03 | State Machine | P0 | 15 min |
| TC-VINDPM-F04 | State Machine | P0 | 10 min |
| TC-VINDPM-F05 | State Machine | P0 | 10 min |
| TC-VINDPM-F06 | Hysteresis | P1 | 10 min |
| TC-VINDPM-F07 | Debounce | P1 | 10 min |
| TC-VINDPM-F10 | CEP Control | P0 | 15 min |
| TC-VINDPM-F11 | CEP Control | P0 | 15 min |
| TC-VINDPM-F12 | PID Block | P0 | 10 min |
| TC-VINDPM-F13 | CEP Passthrough | P1 | 5 min |
| TC-VINDPM-F20 | tar_cap | P0 | 15 min |
| TC-VINDPM-F21 | tar_cap | P0 | 15 min |
| TC-VINDPM-F22 | tar_cap Recovery | P1 | 20 min |
| TC-VINDPM-F23 | Priority | P1 | 15 min |
| TC-VINDPM-F30 | MPP XCE | P1 | 15 min |
| TC-VINDPM-F31 | MPP XCE | P1 | 10 min |
| TC-VINDPM-P01 | Performance | P0 | 30 min |
| TC-VINDPM-P02 | Stability | P0 | 30 min |
| TC-VINDPM-P03 | Response Time | P1 | 20 min |
| TC-VINDPM-P10 | Recovery | P1 | 20 min |
| TC-VINDPM-B01 | Jitter | P1 | 15 min |
| TC-VINDPM-B02 | Boundary | P2 | 10 min |
| TC-VINDPM-B03 | Boundary | P2 | 10 min |
| TC-VINDPM-B10 | Extreme | P1 | 15 min |
| TC-VINDPM-B11 | Startup | P1 | 10 min |
| TC-VINDPM-B12 | Duration | P2 | 15 min |
| TC-VINDPM-B20 | Multi-port | P1 | 20 min |
| TC-VINDPM-B21 | Coexistence | P1 | 15 min |
| TC-VINDPM-B22 | Coexistence | P1 | 15 min |
| TC-VINDPM-D01 | Debug | P2 | 10 min |
| TC-VINDPM-D02 | Debug | P2 | 10 min |
| TC-VINDPM-R01 | Regression | P0 | 30 min |
| TC-VINDPM-R02 | Regression | P0 | 15 min |
| TC-VINDPM-R03 | Regression | P1 | 15 min |
| TC-VINDPM-R04 | Regression | P1 | 20 min |
| TC-VINDPM-R05 | Regression | P1 | 10 min |

**Total estimated test time**: ~8.5 hours

**Priority Legend**:
- P0: Must pass before release (blocking)
- P1: Should pass, investigate if fails
- P2: Nice to have, can be deferred
