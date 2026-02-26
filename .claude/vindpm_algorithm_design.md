# VINDPM Closed-Loop Control Algorithm Design

## 1. Overview

VINDPM (VIN Dynamic Power Management) prevents the powerbank's battery voltage from collapsing during WPC TX operation by monitoring `gd->vpwr` (buck-boost output voltage serving as WPC TX input) and progressively throttling WPC output power when vpwr drops below safe thresholds.

**Key insight**: The powerbank has no external adapter -- `gd->vpwr` is sourced from the battery via NU6805 buck-boost. Under heavy WPC load, the battery internal resistance causes vpwr sag. VINDPM must react before the buck-boost enters under-voltage lockout.

### System Context

```
Battery (2S Li-ion, 6.0~8.9V)
    |
    v
NU6805 Buck-Boost --> vpwr (target 9V, sampled as gd->vpwr)
    |
    v
NU17112 WPC TX coil driver (PID-controlled)
    |
    v
PRx (receiver) sends CEP back to PTx
```

### Signal Path

```
gd->vpwr (sampled every 10ms in APL_EVT_010ms_POLL)
    |
    v
fml_vindpm_check(vpwr)  -- state machine + debounce
    |
    v
gd->power_limit_sts.vindpm_flag (0=NORMAL, 1=WARNING, 2=CRITICAL)
    |
    +---> _wpc.c: WPC_EVT_PCH_TO CEP modification
    +---> pid.c: pid_cep_handler() positive CEP blocking
    +---> prot.c: tar_cap[uvp] reduction
    +---> wpc_xfer.c: MPP XCE NAK
```

## 2. Three-State Machine

### 2.1 State Definitions

| State | Value | Condition | Behavior |
|-------|-------|-----------|----------|
| NORMAL | 0 | vpwr >= RECOVERY (4800mV) | No intervention, PID runs freely |
| WARNING | 1 | vpwr < WARNING_THD (4700mV) | Clamp CEP <= 0, start tar_cap reduction |
| CRITICAL | 2 | vpwr < CRITICAL_THD (4200mV) | Force negative CEP, aggressive tar_cap reduction |

**Note on thresholds**: We use 4700mV (not 4500mV from initial spec) to align with the existing `CONFIG_VBUS_UV_THRESHOLD` in config.h line 60. The 4200mV CRITICAL threshold provides margin above NU6805 UVLO.

### 2.2 State Transition Diagram

```
                    vpwr >= 4800mV
                    reco_cnt > 10
              +-------------------------+
              |                         |
              v                         |
         +---------+    vpwr < 4700mV   +-----------+
         | NORMAL  | -----------------> | WARNING   |
         | flag=0  |    over_cnt > 10   | flag=1    |
         +---------+                    +-----------+
              ^                         |         ^
              |   vpwr >= 4800mV        |         |   vpwr >= 4200mV
              |   reco_cnt > 10         |         |   reco_cnt > 5
              |                         v         |
              |                    +------------+ |
              +--------------------| CRITICAL   |-+
                  vpwr >= 4800mV   | flag=2     |
                  reco_cnt > 30    +------------+
                                   vpwr < 4200mV
                                   over_cnt > 5
```

### 2.3 Threshold Summary (config.h)

```c
/* VINDPM Thresholds -- aligned with existing UV protection */
#define CONFIG_VINDPM_WARNING_THD     4700   /* mV, enter WARNING state */
#define CONFIG_VINDPM_CRITICAL_THD    4200   /* mV, enter CRITICAL state */
#define CONFIG_VINDPM_RECOVERY_THD    4800   /* mV, recovery to NORMAL */
```

These align with the existing `CONFIG_VBUS_UV_THRESHOLD` (4700) and `CONFIG_VBUS_UV_RECOVERY` (4800) in config.h:60-62, which define the hardware UV protection. VINDPM acts as a softer, earlier intervention layer.

## 3. Debounce Logic

### 3.1 Design Rationale

The debounce parameters are chosen based on:
- **Sampling period**: 10ms (APL_EVT_010ms_POLL)
- **ADC noise**: ~7.5mV/LSB, expect ~15-30mV noise band
- **Control loop latency**: CEP arrives every ~250ms (WPC spec t_control), so state changes faster than 100ms are pointless

### 3.2 Debounce Counters

```c
static struct vindpm_ctrl_t {
    uint8_t over_cnt;     /* consecutive samples below threshold */
    uint8_t reco_cnt;     /* consecutive samples above recovery */
} vindpm_ctrl;
```

### 3.3 State Transition Debounce Counts

| Transition | Debounce Count | Time | Rationale |
|-----------|----------------|------|-----------|
| NORMAL -> WARNING | over_cnt > 10 | 100ms | Standard platform pattern (see prot.c) |
| WARNING -> CRITICAL | over_cnt > 5 | 50ms | Faster entry for severe condition |
| CRITICAL -> WARNING | reco_cnt > 5 | 50ms | Moderate recovery, still cautious |
| WARNING -> NORMAL | reco_cnt > 10 | 100ms | Standard recovery |
| CRITICAL -> NORMAL (direct) | reco_cnt > 30 | 300ms | Conservative, avoid bounce-back |

### 3.4 Pseudocode

```c
void fml_vindpm_check(uint16_t vpwr)
{
    switch (gd->power_limit_sts.vindpm_flag)
    {
    case 0: /* NORMAL */
        if (vpwr < CONFIG_VINDPM_WARNING_THD)
        {
            if (++vindpm_ctrl.over_cnt > 10)
            {
                vindpm_ctrl.reco_cnt = 0;
                gd->power_limit_sts.vindpm_flag = 1; /* -> WARNING */
                printk(" VINDPM:W %d", vpwr);
            }
        }
        else
        {
            vindpm_ctrl.over_cnt = 0;
        }
        break;

    case 1: /* WARNING */
        if (vpwr < CONFIG_VINDPM_CRITICAL_THD)
        {
            if (++vindpm_ctrl.over_cnt > 5)
            {
                vindpm_ctrl.reco_cnt = 0;
                gd->power_limit_sts.vindpm_flag = 2; /* -> CRITICAL */
                printk(" VINDPM:C %d", vpwr);
            }
        }
        else if (vpwr > CONFIG_VINDPM_RECOVERY_THD)
        {
            if (++vindpm_ctrl.reco_cnt > 10)
            {
                vindpm_ctrl.over_cnt = 0;
                gd->power_limit_sts.vindpm_flag = 0; /* -> NORMAL */
                printk(" VINDPM:N %d", vpwr);
            }
        }
        else
        {
            /* In WARNING band, reset both counters */
            vindpm_ctrl.over_cnt = 0;
            vindpm_ctrl.reco_cnt = 0;
        }
        break;

    case 2: /* CRITICAL */
        if (vpwr > CONFIG_VINDPM_RECOVERY_THD)
        {
            if (++vindpm_ctrl.reco_cnt > 30)
            {
                vindpm_ctrl.over_cnt = 0;
                gd->power_limit_sts.vindpm_flag = 0; /* -> NORMAL */
                printk(" VINDPM:N %d", vpwr);
            }
        }
        else if (vpwr > CONFIG_VINDPM_CRITICAL_THD)
        {
            if (++vindpm_ctrl.reco_cnt > 5)
            {
                vindpm_ctrl.over_cnt = 0;
                gd->power_limit_sts.vindpm_flag = 1; /* -> WARNING */
                printk(" VINDPM:W %d", vpwr);
            }
        }
        else
        {
            vindpm_ctrl.reco_cnt = 0;
        }
        break;
    }
}
```

## 4. CEP Control Strategy

### 4.1 Integration Points

CEP (Control Error Packet) flows through the system as follows:

```
PRx sends CEP -> wpc_xfer.c decodes -> gd->rx_infos.cep_val
    -> _wpc.c WPC_EVT_PCH_TO modifies cep_val (power_limit_state check)
    -> pid_cep_handler(cep_val) in pid.c applies PID adjustments
```

VINDPM hooks into two points:

1. **WPC_EVT_PCH_TO** (`_wpc.c:561-623`): Modify `gd->rx_infos.cep_val` before PID
2. **pid_cep_handler()** (`pid.c:259-267`): Block positive CEP execution

### 4.2 CEP Modification Rules

#### NORMAL state (vindpm_flag == 0)
- No modification. PID runs with original CEP from PRx.

#### WARNING state (vindpm_flag == 1)

Two-part strategy:

**Part A -- CEP clamping in WPC_EVT_PCH_TO:**
```c
if (gd->power_limit_sts.vindpm_flag >= 1)
{
    if (gd->rx_infos.cep_val > 0)
    {
        gd->rx_infos.cep_val = 0;
        printk(" VINDPM:clamp0");
    }
}
```

**Part B -- Proportional negative CEP (optional enhancement):**
```c
if (gd->power_limit_sts.vindpm_flag == 1)
{
    int16_t delta_v = CONFIG_VINDPM_WARNING_THD - vpwr;
    if (delta_v > 0)
    {
        int8_t cep_adjust = -(delta_v / 100);  /* -1 per 100mV below threshold */
        if (cep_adjust < -3) cep_adjust = -3;  /* clamp to avoid overshoot */
        if (gd->rx_infos.cep_val > cep_adjust)
        {
            gd->rx_infos.cep_val = cep_adjust;
        }
    }
}
```

Proportional response examples (WARNING state):
| vpwr (mV) | delta_v | cep_adjust | Effect |
|-----------|---------|------------|--------|
| 4650 | 50 | 0 | Clamp to 0 only |
| 4550 | 150 | -1 | Gentle reduction |
| 4450 | 250 | -2 | Moderate reduction |
| 4350 | 350 | -3 | Max WARNING reduction |

#### CRITICAL state (vindpm_flag == 2)

Forced maximum negative CEP:
```c
if (gd->power_limit_sts.vindpm_flag == 2)
{
    if (gd->rx_infos.cep_val > -5)
    {
        gd->rx_infos.cep_val = -5;
        printk(" VINDPM:force-5");
    }
}
```

**Why -5?** This matches the existing `tntc_ot_flag` thermal throttle behavior (prot.c references CEP=-5 for temperature limiting). A CEP of -5 causes a ~80-125mV voltage reduction per cycle in EPID_CTRL_MODE_VOLT (`pid.c:387-434`: `tmp = 16 * 5 = 80` for small CEP path).

### 4.3 PID Integration (pid.c)

At `pid.c:259`, the existing vbus_uv_flag check blocks positive CEP:

```c
void pid_cep_handler(int8_t cep)
{
    pid_ctrl_mode_sel(cep);

    if (cep == 0) return;
    if (cep > 0 && gd->power_limit_sts.vbus_uv_flag)
    {
        printk(" DPL");
        return;
    }
    // ... PID control modes
}
```

**VINDPM integration** adds a parallel check:

```c
    if (cep > 0 && gd->power_limit_sts.vindpm_flag)
    {
        printk(" VINDPM_BLK");
        return;
    }
```

This provides a safety net: even if the CEP clamping in WPC_EVT_PCH_TO is bypassed (e.g., by direct PID calls from `pid_load_jump()`), positive CEP will not increase power during VINDPM.

### 4.4 MPP XCE NAK (wpc_xfer.c)

During MPP negotiation, PRx may request power increase via XCE (Extended Control Error). When VINDPM is active, NAK all XCE power-up requests:

```c
/* In wpc_xfer.c XCE handler */
if (gd->power_limit_sts.vindpm_flag)
{
    /* NAK - do not allow power increase */
    fml_fsk_response(FSK_NAK);
    printk(" VINDPM:XCE_NAK");
    return;
}
```

## 5. tar_cap Adjustment Mechanism

### 5.1 Integration with Existing Power Limit Framework

The existing `pwr_lim.tar_cap[]` array (prot.h:34) stores per-reason power capability limits. The minimum across all reasons becomes the effective `nego_cap` via `find_min_cap()` (prot.c:824).

VINDPM reuses the existing `uvp` reason slot (`power_limit_reason_t::uvp = 3`), which is the natural fit since VINDPM is fundamentally an undervoltage protection.

### 5.2 tar_cap Update Logic

Called from `power_limit_tar_cap_update()` (prot.c:1094), which is invoked by `WPC_EVT_FOD_REPORTED` (_wpc.c:841) approximately every RPP cycle (~1-2 seconds).

**Current implementation** (prot.c:1096-1116):
```c
void power_limit_tar_cap_update(void)
{
    if (gd->power_limit_sts.vbus_uv_flag)  /* trigger uv */
    {
        if (pwr_lim.tar_cap[uvp] > 32)
        {
            if (pwr_lim.tar_cap[uvp] > gd->rx_prect/100 && gd->rx_prect/100 > 32)
            {
                pwr_lim.tar_cap[uvp] = gd->rx_prect / 100;
            }
            pwr_lim.tar_cap[uvp] -= 2;
        }
    }
    else if (gd->vbus > ap->vbus_dpl_thd + 500)  /* uvw recover */
    {
        if (pwr_lim.tar_cap[uvp] == gd->tx_infos.nego_cap &&
            pwr_lim.tar_cap[uvp] < gd->tx_infos.max_cap)
        {
            pwr_lim.tar_cap[uvp] += 5;
        }
    }
}
```

**VINDPM enhancement** -- Add VINDPM-aware logic:

```c
void power_limit_tar_cap_update(void)
{
    /* Existing vbus_uv_flag logic remains unchanged */
    if (gd->power_limit_sts.vbus_uv_flag)
    {
        /* ... existing code ... */
    }
    /* VINDPM tar_cap adjustment */
    else if (gd->power_limit_sts.vindpm_flag == 2) /* CRITICAL */
    {
        if (pwr_lim.tar_cap[uvp] > 32)
        {
            pwr_lim.tar_cap[uvp] -= 4;  /* Aggressive: -4 per cycle */
            printk(" VINDPM:cap-%d", pwr_lim.tar_cap[uvp]);
        }
    }
    else if (gd->power_limit_sts.vindpm_flag == 1) /* WARNING */
    {
        if (pwr_lim.tar_cap[uvp] > 50)
        {
            pwr_lim.tar_cap[uvp] -= 2;  /* Gentle: -2 per cycle */
            printk(" VINDPM:cap-%d", pwr_lim.tar_cap[uvp]);
        }
    }
    else if (gd->power_limit_sts.vindpm_flag == 0) /* NORMAL -- recovery */
    {
        if (gd->vpwr > CONFIG_VINDPM_RECOVERY_THD + 200) /* 5.0V hysteresis */
        {
            if (pwr_lim.tar_cap[uvp] == gd->tx_infos.nego_cap &&
                pwr_lim.tar_cap[uvp] < gd->tx_infos.max_cap)
            {
                pwr_lim.tar_cap[uvp] += 5;
            }
        }
    }
}
```

### 5.3 tar_cap Adjustment Rates

| State | Direction | Rate | Effective Speed |
|-------|-----------|------|-----------------|
| WARNING | Reduce | -2 / RPP cycle | ~200mW/s reduction |
| CRITICAL | Reduce | -4 / RPP cycle | ~400mW/s reduction |
| NORMAL (recovery) | Increase | +5 / RPP cycle | ~500mW/s recovery |
| WARNING (floor) | N/A | min = 50 (5W) | Never go below 5W |
| CRITICAL (floor) | N/A | min = 32 (3.2W) | Hard floor at 3.2W |

**Unit**: tar_cap is in 100mW units, so tar_cap=250 means 25W, tar_cap=50 means 5W.

## 6. g_data.h Bitfield Addition

### 6.1 vindpm_flag Location

Add `vindpm_flag` to the existing `power_limit_sts` bitfield in `g_data.h:348-357`:

```c
struct {
    uint16_t fop_flag : 1;
    uint16_t tntc_ot_flag : 1;
    uint16_t isns_oc_flag : 1;
    uint16_t vpwr_ov_flag : 1;
    uint16_t vpwr_uv_flag : 1;
    uint16_t vbus_ov_flag : 1;
    uint16_t vbus_uv_flag : 1;
    uint16_t pout_op_flag : 1;
    uint16_t vindpm_flag : 2;  /* NEW: 0=NORMAL, 1=WARNING, 2=CRITICAL */
} power_limit_sts;
```

**Why 2 bits?** Three states (0, 1, 2) require 2 bits minimum. Fits within the existing 16-bit bitfield which had 8 unused bits.

## 7. Control Response Time Analysis

### 7.1 Detection Latency

| Phase | Latency | Source |
|-------|---------|--------|
| ADC sampling | 10ms | APL_EVT_010ms_POLL period |
| Debounce (NORMAL->WARNING) | 100ms | over_cnt > 10 * 10ms |
| Debounce (WARNING->CRITICAL) | 50ms | over_cnt > 5 * 10ms |
| **Total detection** | **60-110ms** | |

### 7.2 CEP Response Latency

| Phase | Latency | Source |
|-------|---------|--------|
| CEP arrival period | ~250ms | WPC spec t_control (BPP), ~1s (MPP RPP) |
| CEP modification | <1ms | In WPC_EVT_PCH_TO handler |
| PID response | <1ms | In pid_cep_handler |
| Adapter voltage change | 2-5ms | Buck-boost response |
| **Total CEP response** | **~250ms** | Dominated by CEP period |

### 7.3 tar_cap Response Latency

| Phase | Latency | Source |
|-------|---------|--------|
| RPP cycle | ~1-2s | WPC MPP spec |
| tar_cap update | <1ms | In power_limit_tar_cap_update |
| power_limit_sync | <1ms | Called after tar_cap update |
| PID adjustment | Next CEP cycle | ~250ms |
| **Total tar_cap response** | **~1-2s** | Dominated by RPP cycle |

### 7.4 End-to-End Scenario

**Scenario**: Battery under heavy load, vpwr drops from 5.0V to 4.5V in 200ms

```
T=0ms:    vpwr=5000mV, NORMAL, no intervention
T=50ms:   vpwr=4850mV, NORMAL, over_cnt=0
T=100ms:  vpwr=4650mV, NORMAL, over_cnt starts (below 4700)
T=200ms:  vpwr=4500mV, over_cnt=10 -> WARNING triggered
T=250ms:  Next CEP arrives, cep_val clamped to 0 (no power increase)
T=450ms:  vpwr=4300mV (still dropping), over_cnt > 5 -> CRITICAL
T=500ms:  Next CEP, forced to -5, PID reduces voltage by ~80mV
T=750ms:  CEP=-5 again, cumulative reduction ~160mV
T=1000ms: vpwr starts recovering as coil current decreases
T=1200ms: RPP arrives, tar_cap[uvp] -= 4
T=2000ms: tar_cap effect propagates, PRx adjusts requested power
T=3000ms: vpwr > 4800mV, recovery debounce starts
T=3300ms: reco_cnt > 30 -> NORMAL state restored
```

**Total protection response: ~200ms to first intervention, ~1s to significant power reduction**

## 8. Complete File Modification Summary

### 8.1 Files to Modify

| File | Change | Lines |
|------|--------|-------|
| `app/config.h` | Add VINDPM threshold defines | 3 new lines |
| `fml/g_data.h` | Add vindpm_flag:2 to power_limit_sts | 1 line modification |
| `app/prot.h` | Add fml_vindpm_init/check declarations | 2 new lines |
| `app/prot.c` | Add vindpm state machine + debounce | ~60 new lines |
| `app/prot.c` | Modify power_limit_tar_cap_update | ~20 lines modified |
| `app/app.c` | Add fml_vindpm_init() call in apl_task_init | 1 new line |
| `app/app.c` | Add fml_vindpm_check(vpwr) call in 010ms handler | 1 new line |
| `app/_wpc.c` | Add VINDPM CEP clamp in WPC_EVT_PCH_TO | ~10 lines |
| `app/pid.c` | Add vindpm_flag check in pid_cep_handler | 5 lines |
| `app/wpc_xfer.c` | Add XCE NAK when vindpm active | ~5 lines |

### 8.2 Resource Impact

| Resource | Usage | Budget | Remaining |
|----------|-------|--------|-----------|
| SRAM (static) | +3 bytes (vindpm_ctrl) | 7KB | Negligible |
| SRAM (bitfield) | 0 (reuses existing padding) | - | - |
| Flash (code) | ~200 bytes | 120KB | Negligible |
| CPU (10ms ISR) | ~50 cycles | 360K/period | <0.01% |

## 9. Safety Considerations

### 9.1 Interaction with Existing Protections

| Existing Protection | Threshold | Interaction |
|-------------------|-----------|-------------|
| `fml_vbus_uvp_check` | vbus_uvp_thd | Monitors `gd->vbus` (hardcoded 9000mV) -- **no conflict**, different signal |
| `fml_vbus_dpl_check` | vbus_dpl_thd | Also monitors `gd->vbus` -- **no conflict** |
| `fml_vpwr_ovp_check` | vpwr_ovp_thd | Monitors vpwr overvoltage -- orthogonal |
| `vbus_uv_flag` (power_limit) | existing | Both share tar_cap[uvp] -- **coordinate**: VINDPM only adjusts when vbus_uv_flag is clear |
| `tntc_ot_flag` (thermal) | tntc_otw_thd | Independent, uses tar_cap[otp] -- **no conflict** |

### 9.2 Critical Path Protection

VINDPM must NOT interfere with:
1. **EPT processing**: Never modify CEP during EPT response
2. **Negotiation phase**: Only active during XFER phase (`gd->ptx_protocol_phase == WPC_PHASE_XFER`)
3. **Load dump detection**: Existing load_jump_evt mechanism in app.c must take priority

### 9.3 Recovery Guarantee

The algorithm guarantees recovery because:
1. CRITICAL state forces continuous power reduction (CEP=-5 + tar_cap decrease)
2. Reduced power means reduced coil current means reduced battery draw
3. Battery voltage will eventually recover as load decreases
4. Conservative CRITICAL->NORMAL debounce (300ms) prevents oscillation

## 10. Tuning Guidelines

### 10.1 Parameters to Tune on Hardware

| Parameter | Default | Adjust If... |
|-----------|---------|-------------|
| WARNING_THD (4700mV) | First | Lower if false triggers during normal operation |
| CRITICAL_THD (4200mV) | Rarely | Only if NU6805 UVLO occurs before CRITICAL triggers |
| RECOVERY_THD (4800mV) | Rarely | Increase for more hysteresis |
| WARNING debounce (10) | If needed | Increase for noisy supplies |
| CRITICAL debounce (5) | Rarely | Decrease only if brownout is rapid |
| CEP WARNING clamp | Per RX | Some RX respond slowly to small CEP changes |
| CEP CRITICAL value (-5) | Per RX | Some RX need stronger signal (-10) |
| tar_cap reduction rate (2/4) | Per battery | Larger batteries can tolerate slower reduction |

### 10.2 Debug Prints

All state transitions and CEP modifications include `printk` with "VINDPM:" prefix for easy log filtering:
- `VINDPM:W` -- entered WARNING
- `VINDPM:C` -- entered CRITICAL
- `VINDPM:N` -- returned to NORMAL
- `VINDPM:clamp0` -- CEP clamped to 0
- `VINDPM:force-5` -- CEP forced to -5
- `VINDPM:XCE_NAK` -- XCE request rejected
- `VINDPM:cap-XXX` -- tar_cap reduced to XXX
- `VINDPM_BLK` -- positive CEP blocked in PID

---

**Document Version**: 1.0
**Author**: platform-wpc-hw-agent
**Date**: 2026-02-15
**Status**: Design complete, ready for implementation review
