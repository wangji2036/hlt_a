# WPC Hardware Platform - Technical Knowledge

**Knowledge Depth**: Level 3-4 (Deep System)
**Last Updated**: 2026-02-15
**Managed Files**: 21 files across `fml/`, `app/`, `lib/` directories

---

## 1. Module Overview

### WPC Hardware Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      WPC Hardware Platform                       │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐       │
│  │  ASK Demod    │  │  FSK Modem    │  │  NU103X AFE   │       │
│  │  (RX)         │  │  (TX)         │  │  (Analog FE)  │       │
│  │  lib/ask.c    │  │  fml/fsk.c    │  │  fml/nu103x.c │       │
│  │  fml/ask.h    │  │  fml/fsk.h    │  │  fml/nu103x.h │       │
│  └───────┬───────┘  └───────┬───────┘  └───────┬───────┘       │
│          │                  │                  │                 │
│          └──────────────────┼──────────────────┘                 │
│                             │                                    │
│  ┌──────────────────────────┴─────────────────────────┐         │
│  │         Power Control & FOD Subsystems              │         │
│  ├─────────────────────────────────────────────────────┤         │
│  │  PID:  app/pid.c, pid.h    (Freq/Duty/Volt/Phase)  │         │
│  │  FOD:  app/fod.c           (Legacy FOD)             │         │
│  │  PFOD: lib/pfod.c, app/pfod.h (Power Loss FOD)     │         │
│  │  QFOD: app/qfod.c, qfod.h  (Q-factor FOD)          │         │
│  │  QDT:  fml/qdt.c, qdt.h    (Q Detection)            │         │
│  └─────────────────────────────────────────────────────┘         │
│                                                                   │
│  ┌─────────────────────────────────────────────────────┐         │
│  │         Secure Element (Qi Auth)                    │         │
│  ├─────────────────────────────────────────────────────┤         │
│  │  FM1210:  fml/fm1210.c, fm1210.h  (FocalMCU)       │         │
│  │  T91206:  fml/t91206.c, t91206.h  (Toppan)         │         │
│  └─────────────────────────────────────────────────────┘         │
│                                                                   │
│  ┌─────────────────────────────────────────────────────┐         │
│  │         Protection Systems                          │         │
│  ├─────────────────────────────────────────────────────┤         │
│  │  PROT: app/prot.c, prot.h                           │         │
│  │  - OTP/UTP (NTC, Die Temp)                          │         │
│  │  - OCP/OVP/UVP (Current, Voltage)                   │         │
│  │  - OPP (Power)                                      │         │
│  └─────────────────────────────────────────────────────┘         │
└─────────────────────────────────────────────────────────────────┘
```

**Core Responsibilities**:
- Physical layer communication (ASK/FSK @ 128kHz/360kHz)
- Analog front-end control (NU103X multi-mode demodulation)
- Closed-loop power control (PID with 4 control variables)
- Multi-layer FOD (Power Loss + Q-factor + Legacy)
- Qi 1.3 authentication via secure elements
- Multi-threshold protection logic

---

## 2. Subsystem Index

### 2.1 Communication Layer
- **[ASK Demodulation](#3-ask-demodulation)**: RX path, bit/byte/packet decode, 3-channel DDM
- **[FSK Modulation](#4-fsk-modulation)**: TX path, BMC encoding, depth/polarity control
- **[NU103X AFE](#5-nu103x-analog-front-end)**: Dual DMO, gain/source/BPF switching

### 2.2 Power Control
- **[PID Controller](#6-pid-power-controller)**: 4-variable (V/F/D/P) cascaded control
- **[Capacitor Switching](#61-capacitor-switching)**: Ctx tuning (82nF/482nF)

### 2.3 FOD Systems
- **[Power Loss FOD](#7-fod-foreign-object-detection)**: PFOD (ploss-based), Legacy FOD
- **[Q-Factor FOD](#72-q-factor-fod-qfod)**: QFOD (quality factor detection)
- **[Q Detection](#73-q-detection-qdt)**: LC tank characterization

### 2.4 Authentication
- **[FM1210 SE](#8-secure-elements)**: I2C, SHA-256, ECC-P256
- **[T91206 SE](#82-t91206-toppan)**: APDU, digest/cert chain reading

### 2.5 Protection
- **[Protection System](#9-protection-system)**: 9 protection types with hysteresis

---

## 3. ASK Demodulation

### 3.1 Physical Layer

**Files**: `lib/ask.c`, `fml/ask.h`

**Hardware Chain**:
```
NU103X DMO → ECAP (Edge Capture) → MCU ISR → Bit Decode → Byte Decode → Pkt Decode
```

**Bit Timing (1.125MHz clock)**:
```c
// lib/ask.c:94-106
#define FULL_BIT_0_MAX_THD1  (880 * 1125/1000)  // 990 counts → 800us
#define FULL_BIT_0_MIN_THD1  (390 * 1125/1000)  // 439 counts → 400us
#define STAR_BIT_0_MIN_THD1  (370 * 1125/1000)  // 416 counts → 360us (start bit min)
#define HALF_BIT_1_MAX_THD1  (300 * 1125/1000)  // 338 counts → 300us
#define HALF_BIT_1_MIN_THD1  (135 * 1125/1000)  // 152 counts → 120us (preamble min)
#define DATA_BIT_1_MIN_THD1  (60  * 1125/1000)  // 68  counts → 53us
```

**Dual Decoder**:
- `MAX_SUB_DCODE = 2` (`lib/ask.c:83`) → Two parallel decoders with different thresholds for robustness
- Decoder 0: Stricter timing (THD1)
- Decoder 1: Relaxed timing (THD2, `HALF_BIT_1_MIN_THD2 = 150*1125/1000`)

### 3.2 Decode State Machine

**Packet Structure** (`fml/ask.h:4-10`):
```c
struct ask_packet_t {
    uint8_t src;           // DMO channel (1/2/3)
    uint8_t mark;          // dmox_src & decode_ch
    uint8_t hdr;           // Header byte
    uint8_t len;           // Packet length
    uint8_t data[29];      // Header + Message + Checksum
};
```

**3-Layer Decode** (`lib/ask.c:305-755`):

1. **Bit Decode** (`bit_decode()` @ line 566):
   - Discriminates 0/1 from pulse width
   - Fuzzy logic for edge cases (`fuzzy_flg`, `fuzzy_cnt`)
   - Half-bit detection (`haf_1_flg`)

2. **Byte Decode** (`byt_decode()` @ line 474):
   - 11-bit frame: Start(0) + Data(8) + Parity(1) + Stop(1)
   - Even parity check
   - Error types: `BYT_ERR_STR/PTY/STP`

3. **Packet Decode** (`pkt_decode()` @ line 314):
   - Preamble: 8-30 consecutive '1' bits (`PRMBL_CNT_MIN/MAX`)
   - Header length lookup (`pkt_len_get()` @ line 234)
   - XOR checksum verification
   - Events: `WPC_EVT_HDR_START`, `WPC_EVT_HDR_RECVD`, `WPC_EVT_PKT_RECVD`

### 3.3 Multi-Channel Demodulation

**3 DMO Channels** (`lib/ask.c:82`):
```c
#define ASK_DM_CHAN_MAX 3  // DMO1, DMO2, DMO3 (digital)
static struct ask_dm_t ask_dm[ASK_DM_CHAN_MAX];
```

**Per-Channel Configuration** (`lib/ask.c:1171-1459`):
- **DMO1** (`fml_ask_dmo1_xfer_cfg()`):
  - Sources: IAVG (avg current) / EVDM (external VDM pin)
  - Gain: AUTO / FIX_X36 / FIX_X60
- **DMO2** (`fml_ask_dmo2_xfer_cfg()`):
  - Sources: VCAP (tank voltage) / PHASE / DIGITAL (SW-based)
  - Load-adaptive config (low/mid/high power)
  - VCAP K-ratio: K1/K2/K3 (voltage divider)

**Digital DDM** (`fml_ask_dig_ddm_enable()` @ line 1461):
- Bypasses analog DMO2, uses ECAP4 + EADC for software demodulation
- Enables with `_1030_CFG_DMO2_OUT_MODE_CAP`

### 3.4 Critical Check

**DDM Watchdog** (`fml_ask_decode_check()` @ line 898):
```c
// lib/ask.c:902
if ((sys_ticks - pkt_time_stamp) > (ddm_check_interval_long ? 650 : 350)) {
    ddm_param_chose();  // Reconfigure DMO params
    dm_bad_cnt++;
}

// lib/ask.c:917 - Critical threshold
if (dm_bad_cnt[0] >= 2 && dm_bad_cnt[1] >= 2 && dm_bad_cnt[2] >= 2) {
    osal_set_event(WPC_TASK, WPC_EVT_DM_CRITICAL);  // Trigger restart
}
```

---

## 4. FSK Modulation

### 4.1 Physical Layer

**Files**: `fml/fsk.c`, `fml/fsk.h`

**Modulation Method**:
```
Base Freq (f_op) ± Depth → BMC Encoding → EPWM Hardware FSK
```

**Depth Configuration** (`fml/fsk.c:114-120`):
```c
// Maps user depth [0-3] to hardware register values
switch (depth) {
    case 0: depth =  7; break;  //  7 * period_step
    case 1: depth = 11; break;  // 11 * period_step
    case 2: depth = 21; break;  // 21 * period_step
    case 3: depth = 38; break;  // 38 * period_step (max deviation)
}
```

**Bit Cycles** (`fml/fsk.h:6-12`):
```c
enum fsk_cyc_t {
    _FSK_BIT_CYCLES_512 = 0,  // 512 EPWM cycles per bit
    _FSK_BIT_CYCLES_256 = 1,
    _FSK_BIT_CYCLES_128 = 2,  // Standard for MPP/EPP
    _FSK_BIT_CYCLES_064 = 3,
};
```

### 4.2 BMC Encoding

**Encoding Logic** (`fml_fsk_data_encoding()` @ line 129):

**Pattern (1-byte)**:
```c
// fml/fsk.c:152
fsk_encoding_data_buff[idx][0] = (8 << 26) | (pattern << 0);
// [31:26] = bit count, [7:0] = raw data
```

**Packet (multi-byte)**:
```c
// fml/fsk.c:165-173 - Per-byte encoding
uint8_t tmp = data[i];
tmp ^= tmp >> 1;  // Cascade XOR for parity
tmp ^= tmp >> 2;
tmp ^= tmp >> 4;
tmp &= 1;  // Parity bit

fsk_encoding_data_buff[i] =
    (11 << 26) |         // 11 bits total
    (1 << 10) |          // Start bit (1)
    (tmp << 9) |         // Parity
    (data[i] << 1) |     // 8 data bits
    (0 << 0);            // Stop bit (0)
```

**Preamble Addition** (`fml/fsk.c:179-186`):
```c
// For 128-cycle mode, add 4-bit preamble (0xF)
if (cycles == _FSK_BIT_CYCLES_128 && preamble) {
    uint32_t tmp_data = fsk_encoding_data_buff[0];
    fsk_encoding_data_buff[0] = ((tmp_data & 0x3FFFFFF) << 4) | 0xF;
    tmp_data += 4;  // Update bit count
}
```

### 4.3 Hardware Interaction

**EPWM FSK Registers** (`fml/fsk.c:123-124`):
```c
EPWM->FSK_CTRL.WORD |=
    (polarity << EPWM_FSK_CTRL_POLAR_SEL_Pos) |  // Freq deviation sign
    (depth    << EPWM_FSK_CTRL_DEPTH_SEL_Pos) |  // Deviation magnitude
    (cycles   << EPWM_FSK_CTRL_BIT_CYCLE_Pos);   // Cycles per bit
```

**Interrupt-Driven TX** (`fml_fsk_int_callback()` @ line 281):
```c
// FSK1_IRQHandler/FSK2_IRQHandler call this
if (FSK_FLAG.BMC_BUFF_EMPTY) {
    if (++have_send_cnt < need_send_cnt) {
        FSK_BUFF.WORD = fsk_encoding_data_buff[have_send_cnt];  // Load next byte
    }
}
if (FSK_FLAG.LAST_BUFF_DONE) {
    osal_set_event(WPC_TASK, WPC_EVT_FSK_RESP_DONE);
    fsk_is_busy = 0;
    if (dither_status) AFD_CTRL.AFD_EN = 1;  // Re-enable dithering
}
```

**EPWM Workaround** (`fml/fsk.c:197-210`):
```c
// Adjust duty to avoid EPWM design issue
uint16_t duty_min = (perd_min/2) - PHAS - 3;
uint16_t duty_max = (perd_max/2) - PHAS - 1;
if (duty > duty_min && duty < duty_max) {
    EPWM->PWM_DUTY = (polarity) ? duty_min : duty_max;
}
```

---

## 5. NU103X Analog Front-End

### 5.1 Configuration Interface

**Files**: `fml/nu103x.c`, `fml/nu103x.h`

**Control Protocol** (`fml_nu103x_config()` @ line 8):
```c
// Pulse-based configuration via GPC_PIN2
// 1. Set PIN2 as input (read current state)
// 2. Set PIN2 as output LOW
// 3. Send N pulses (N = enum nu103x_cmd_t value)
// 4. Set PIN2 back to input
```

**State Tracking** (`nu103x.h:4-122`):
```c
union nu103x_t {
    struct {
        uint32_t OCP_THD           : 1;  // 8A/10A
        uint32_t LPM_STS           : 1;  // Low power mode
        uint32_t VDD_LDO_V4P8_STS  : 1;  // LDO status
        uint32_t DMO1_OUT_MODE     : 1;  // DDM/QDT
        uint32_t DMO1_DDM_SRC      : 1;  // IAVG/EVDM
        uint32_t DMO1_DDM_GAIN_MOD : 1;  // AUTO/FIXD
        uint32_t DMO1_DDM_GAIN_FIX : 1;  // X36/X60
        uint32_t DMO2_OUT_MODE     : 2;  // DDM/QDT/CAP
        uint32_t DMO2_VCAP_RATIO_K : 2;  // K1/K2/K3
        uint32_t DMO2_DDM_SRC      : 1;  // VCAP/PHAS
        uint32_t DMO2_DDM_GAIN_MOD : 1;  // AUTO/FIXD
        uint32_t DMO2_DDM_GAIN_FIX : 1;  // X36/X60
        uint32_t DMOx_DDM_CMP_HYST : 1;  // 12.5mV/30mV
        // ... (27 bits total)
    } BITS;
    uint32_t WORD;
};
```

### 5.2 DMO Configuration

**DMO1 Sources** (`nu103x.c:177-182`):
- `_1030_CFG_DMO1_DDM_SRC_IAVG`: Internal average current (PVIN)
- `_1030_CFG_DMO1_DDM_SRC_EVDM`: External VDM pin (coil voltage after LPF+HPF)

**DMO2 Sources** (`nu103x.c:184-193`):
- `_1030_CFG_DMO2_DDM_SRC_VCAP`: Resonant capacitor voltage (with K-ratio divider)
  - K1: 180K/52K (low gain)
  - K2: 180K/26K (mid gain)
  - K3: 180K/13K (high gain)
- `_1030_CFG_DMO2_DDM_SRC_PHAS`: Phase detection (LC tank phase shift)

**Gain Modes**:
- `AUTO`: Hardware auto-ranging based on signal strength
- `FIXD`: Fixed gain (X36 = normal, X60 = high)

**Band-Pass Filter**:
- `BPF_1ORD`: 1st-order (faster response, less filtering)
- `BPF_2ORD`: 2nd-order (better noise rejection)

### 5.3 QDT Mode

**Q-Factor Measurement** (`fml_nu103x_qdt_init()` @ line 159):
```c
// nu103x.c:171-174
fml_nu103x_config(_1030_CFG_QDT_PRECHARGE_V1P2);  // Set precharge voltage
fml_nu103x_config(_1030_CFG_DMO1_OUT_MODE_QDT);   // DMO1 → VQM (voltage decay)
fml_nu103x_config(_1030_CFG_DMO2_OUT_MODE_QDT);   // DMO2 → NQM (freq counting)
fml_nu103x_config(_1030_CFG_QDT_EN_);             // Enable Q measurement
```

---

## 6. PID Power Controller

### 6.1 Control Variables

**Files**: `app/pid.c`, `app/pid.h`

**4-Variable Cascade** (`pid_cep_handler()` @ line 158):

```
CEP (Control Error Packet) → Mode Selection → Variable Adjustment

Variables:
1. pid_volt  (adapter voltage, mV)
2. pid_perd  (EPWM period, counts → frequency)
3. pid_duty  (PWM duty, 0.1%)
4. pid_phas  (phase shift, counts)
```

**Mode Selection Logic** (`pid_ctrl_mode_sel()` @ line 485):

**Positive CEP (need more power)**:
```
Priority: Phase → Duty → Freq → Volt → Freq(extended)
if (phas > phas_lim_lo)     → EPID_CTRL_MODE_PHAS
if (duty < duty_lim_hi)     → EPID_CTRL_MODE_DUTY
if (perd < perd_lim_mi)     → EPID_CTRL_MODE_FREQ
if (volt < volt_lim_hi)     → EPID_CTRL_MODE_VOLT
if (perd < perd_lim_hi)     → EPID_CTRL_MODE_FREQ
```

**Negative CEP (reduce power)**:
```
Priority: Freq → Volt (Samsung) / Volt → Freq (others)
if (perd > perd_lim_mi)     → EPID_CTRL_MODE_FREQ
if (volt > volt_lim_lo)     → EPID_CTRL_MODE_VOLT
if (perd > perd_lim_lo)     → EPID_CTRL_MODE_FREQ
if (duty > duty_lim_lo)     → EPID_CTRL_MODE_DUTY
if (phas < phas_lim_hi)     → EPID_CTRL_MODE_PHAS
```

### 6.2 Adjustment Algorithms

**Voltage Control** (`pid.c:169-248`):
```c
// Positive CEP
if (cep > 30) cep = 30;
gd->pid_volt += 200 * (cep/10 + 1);  // QC3.0: 200mV steps
gd->pid_volt += 20 * (cep + 1);      // PD/others: 20mV steps

// Negative CEP
if (cep < -24) cep = -24;
gd->pid_volt -= 20 * (abs(cep) + 1);
```

**Frequency Control** (`pid.c:299-332`):
```c
// Positive CEP (increase freq → reduce period)
if (cep > 30) cep = 30;
gd->pid_perd += cep/3 + 1;

// Negative CEP (decrease freq → increase period)
if (cep < -30) cep = -30;
gd->pid_perd -= abs(cep)/3 + 1;
```

**Duty/Phase Control** (`pid.c:334-400`):
```c
// Duty: ±(cep/2 + 1)
// Phase: ±(cep/4 + 1)  // Note: Positive CEP → REDUCE phase (more power)
```

### 6.3 Limit System

**3-Tier Limits** (`pid.h:16-27`):
```c
#define PID_VOLT_LIM_H  (gd->pid_limit.volt_lim_hi)  // Max voltage
#define PID_VOLT_LIM_M  (gd->pid_limit.volt_lim_mi)  // Mid threshold
#define PID_VOLT_LIM_L  (gd->pid_limit.volt_lim_lo)  // Min voltage
// Similar for PERD/DUTY/PHAS
```

**Adapter-Specific Initialization** (`pid_init()` @ line 103):
```c
case EADP_TYPE_QC3P0_12V:
    // QC3.0 can adjust voltage dynamically
    break;

case EADP_TYPE_DCSRC_05V:
    pid_set_volt_limit(volt_max, volt_min, volt_min);  // Fixed voltage
    pid_set_freq_limit(127772, 127772, 127772);        // Fixed freq
    pid_set_duty_limit(500, 350, 150);                 // Wide duty range
    break;

case EADP_TYPE_POWERBANK_09V:
    pid_set_volt_limit(volt_max, volt_min, volt_min);
    pid_set_freq_limit(127772, 127772, 127772);
    pid_set_duty_limit(500, 350, 100);                 // Tighter low limit
    break;
```

### 6.4 Capacitor Switching

**Ctx Tuning** (`ctx_switch()` in `pid.c:54-101`):

```c
// Actual ctx_ind:  0: 82nF,  1: 82nF,  2: 82nF,  3: 482nF,  4: 482nF
// Spec  ctx_ind:  0: 68nF,  1: 101nF, 2: 134nF, 3: 468nF, 4: 501nF

#define cap_s3_enable()   fml_nu103x_config(_1030_CFG_DRVH1_TURN_ON_)  // 400nF
#define cap_s2_enable()   fml_nu103x_config(_1030_CFG_DRVH2_TURN_ON_)  // 47nF

switch (ctx_ind) {
    case 0: cap_s2_disable(); gd->ctx = 82;  break;   // Base 82nF
    case 3: cap_s2_enable();  gd->ctx = 482; break;   // 82 + 400 = 482nF
    case 4: cap_s2_enable();  gd->ctx = 482; break;   // Same as 3
}
```

**Note**: Indices 1/2 also map to 82nF (commented code shows disabled S1/S3 switches).

---

## 7. FOD (Foreign Object Detection)

### 7.1 Power Loss FOD (PFOD)

**Files**: `lib/pfod.c`, `app/pfod.h`

**Core Algorithm** (`pfod_mpla()` @ line 40):

```c
// 1. Calculate TX-side power loss
ploss = ploss_calc(gd->rx_infos.pla_type);

// 2. Cap ploss based on RX power
if (rx_power < 4000  && ploss > 1600) ploss = 1600;
if (rx_power < 8000  && ploss > 2000) ploss = 2000;
if (rx_power < 12000 && ploss > 2800) ploss = 2800;
if (ploss > 4000) ploss = 4000;

// 3. Calculate power loss in free space (pfo)
pfo = tx_power - ploss - rx_power - 300;  // 300mW offset

// 4. Check thresholds
if (pfo > pfo_thd || (pfo >= pfo_thd_reco && fod_count > 0)) {
    fod_count++;
    if (fod_count >= FOD_MAX_CNT) {
        wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_POWER_LOSS_FOD);
    }
}
```

**Power Loss Calculation** (`ploss_calc()` @ line 60):

```c
// ploss = Pcoil + Pfm + Pmos + Psnuber

// Pcoil = pcoil_factor * Icoil^2 / 1e6
Pcoil = 160 * (icol_rms * icol_rms / 1000) / 1000;  // ACR loss

// Pfm (ferrite loss, MPP mode only)
if (mode) {
    alpha_fm = rx_infos.alpha_fm;  // From RX PLA packet
    alpha_fm_dc = rx_infos.alpha_fm_dc;
    Pfm = (alpha_fm^2 / 267) * Icoil^2 + (alpha_fm_dc^2 / 492);
}

// Pmos = 2*Rds*Icoil^2 + 4*Vin*Icoil*t*freq/6
Pmos = (80 * Icoil^2 + 792 * vpwr * icoil / 100) / 1000;

// Psnuber = 1188 * vpwr^2 / 1e6
Psnuber = 1188 * vpwr * vpwr / 1e12;
```

**Thresholds** (`pfod.h:6-12`):
```c
#define PFO_10W_THD      265   // 10W mode (with FO detected)
#define PFO_10W_RECO     251   // 10W recovery (95% of THD)
#define PFO_15W_THD      385   // 15W mode (no FO)
#define PFO_15W_RECO     365   // 15W recovery
#define PFO_THD_APL_MPP  385   // Apple MPP mode
#define FOD_MAX_CNT      30    // Max consecutive violations
```

**Power Limit Feedback** (`pfod_action()` @ line 166):
```c
// If FOD detected but not critical, throttle power
if (pfo > pfo_thd) {
    fod_count_filter++;
    if (fod_count_filter > 3) {
        fod_count++;
        if (fod_count < FOD_MAX_CNT) {
            gd->power_limit_sts.fop_flag = 1;  // Throttle, don't ACK CEP
            return 1;
        }
    }
}

// If safe, allow power increase
if (pfo_avg < pfo_thd_reco && fod_count == 0) {
    if (p_rect_max + 200 >= nego_cap) {
        tar_cap_fod += 2;  // Increase target by 2W
        need_renego_cap = 1;
        return 2;  // Send ATN to renegotiate
    }
}
```

### 7.2 Q-Factor FOD (QFOD)

**Files**: `app/qfod.c`, `app/qfod.h`

**Calibration Flow** (`qfod_qdt_cali_process()` @ line 27):

```c
// Phase 1: Wait for RX removal
if (!cali_ready) {
    if (q_fact outside [q_cali_start ± 10]) {
        ctx_switch(2);  // Switch to calibration capacitance
        tool_remove_cnt++;
        if (tool_remove_cnt >= 20) {
            cali_ready = 1;
        }
    }
}

// Phase 2: Measure baseline Q/F
if (cali_ready) {
    q_sum += q_fact;
    f_sum += f_self;
    if (++tool_remove_cnt >= 8) {
        q_avg = q_sum >> 3;
        f_avg = f_sum >> 3;
        // Save to flash at AP_CFG_ROM_ADDR_BASE
        hal_fmc_write_word(addr, q_avg);
        hal_fmc_write_word(addr+4, f_avg);
    }
}
```

**Negotiation-Time Detection** (`qfod_nego()` @ line 83):

```c
// Nokia MP.TPR#MP3
if (ref_q in [0x7C, 0x8F] && ref_f in [0x79, 0x85]) {
    if (q_fact + 110 < q_factor_base_value &&
        f_self + 50 > fs_base_value) {
        return 1;  // FOD detected
    }
}

// Nokia MP.TPR#MP4
if (ref_q in [0x25, 0x28] && ref_f in [0x73, 0x75]) {
    if (q_fact + 110 < q_factor_base_value &&
        f_self + 50 > fs_base_value) {
        return 1;
    }
}

// Similar for MP1B, 1F variants
```

### 7.3 Q Detection (QDT)

**Files**: `fml/qdt.c`, `fml/qdt.h`

**Measurement Sequence** (`fml_qdt_detect()` @ line 69):

```c
// Step 1: Set PWM pins LOW
qdt_pin_ctrl(ch0, {O_EN=1, DOUT=0});
qdt_pin_ctrl(ch1, {O_EN=1, DOUT=0});

// Step 2: Set PWM1 to High-Z, enable NU103X QDT
qdt_pin_ctrl(ch0, {I_EN=1, O_EN=0});
fml_nu103x_qdt_init();  // DMO1→VQM, DMO2→NQM

// Step 3: Wait 500us for tank pre-charge
delay_1us(500);

// Step 4: Configure ECAP for QDT
hal_ecap_init(ECAP1, _ECAP_FUNC_MODE_QDT);  // VQM decay time
hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_QDT);  // NQM resonant freq

// Step 5: Discharge tank (set PWM1 LOW)
qdt_pin_ctrl(ch0, {I_EN=0, O_EN=1, DOUT=0});

// Step 6: Wait for ECAP capture (timeout 500 * 4us = 2ms)
while (!(ECAP1_QDT_DONE && ECAP2_QDT_DONE) && timeout++ < 500) {
    delay_1us(4);
}

// Step 7: Read measurements
vqm_decay_time_cnt = ECAP1->QDT_MEAS.VQM.DECAY_TIME_CNT;
vqm_width_last_cnt = ECAP1->QDT_MEAS.VQM.WIDTH_LAST_CNT;
nqm_reson_freq_cnt = ECAP2->QDT_MEAS.NQM.RESON_FREQ_CNT;
```

**Q-Factor Calculation** (`qdt.c:149-167`):

```c
// Voltage peak threshold correction
tmp = 10000 * vqm_width_last_cnt / nqm_reson_freq_cnt;
tmp = tmp * tmp;
tmp = 1232 * tmp / 100000;
tmp = (tmp < 10000) ? (10000 - tmp) : 10000;
vpeak_th2 = QDT_VPEAK_STD_THD2 * 10000 / tmp;  // 300mV standard

// Q = 157000 * (T_decay - T_dly - T_width/2 + T_period/20) / (T_period * ln(Vpeak_th2))
ln_index = (vpeak_th2 - 300) clamp [0, 99];
q_fact = 157000 * (vqm_decay_time_cnt - 7200 - vqm_width_last_cnt/2 + nqm_reson_freq_cnt/20)
         / (nqm_reson_freq_cnt * _qdt_ln_tbl[ln_index]);

// Frequency = 360000 * (meas_times + 1) / nqm_reson_freq_cnt
f_self = 360000 * (ECAP2->QDT_CTRL.NQM.MEAS_TIMES_SET + 1) / nqm_reson_freq_cnt;
```

**LN Lookup Table** (`qdt.c:9-21`):
```c
// _qdt_ln_tbl[100]: Natural log values * 1000, scaled for integer math
// Index 0 (Vpeak=300mV): ln(300) * 1000 = 1789
// Index 99 (Vpeak=399mV): ln(399) * 1000 = 1505
```

---

## 8. Secure Elements

### 8.1 FM1210 (FocalMCU)

**Files**: `fml/fm1210.c`, `fml/fm1210.h`

**I2C Protocol** (`fm1210_i2c_send_frame()` @ line 92):

```
[SEIC_DEV_ADDR] [LEN] [DATA...] [CRC16_MSB] [CRC16_LSB]

SEIC_DEV_ADDR = 0x04 (7-bit address)
LEN = length of DATA (not including CRC)
CRC16 = CCITT with init 0xC6C6
```

**Wake-up Sequence** (`fm1210_wakeup()` @ line 33):
```c
// Toggle VCC pin: LOW → HIGH → LOW → HIGH
GPA->DOUT.PIN7 = 0;  delay_1us(1000);
GPA->DOUT.PIN7 = 1;  delay_1us(1000);
GPA->DOUT.PIN7 = 0;  delay_1us(1000);
GPA->DOUT.PIN7 = 1;  delay_1us(1000);
GPA->ODEN.PIN6 = 1;  // Enable open-drain on I2C pins
GPA->ODEN.PIN7 = 1;
```

**Key APIs**:

```c
// Get Qi ID (6 bytes)
fm1210_get_qi_id(uint8_t *rbuf);
  → CMD: 0x30, DATA: 0x00
  → Extract rbuf[11:13] as Qi ID

// Read Certificate Chain Hash (32 bytes)
fm1210_read_cert_hash(uint8_t *rbuf);
  → CMD: 0x30, DATA: 0x30  // First 16 bytes
  → CMD: 0x30, DATA: 0x33  // Second 16 bytes

// Read SE Certificate (variable length)
fm1210_read_se_cert(uint8_t *rbuf, uint32_t *rlen);
  → CMD: 0x30, DATA: 0x01, 0x02, ..., 0x27 (multi-block read)
  → Length extracted from first block: rbuf[2:3]

// Sign TBS (To-Be-Signed) Auth Structure
fm1210_get_tbs_auth(uint8_t *rbuf);
  1. Build TBS: 0x41 | CertHash[32] | 0x1B | 0x00 | Challenge[16] | 0x13 | 0x11 | CertHashLSB
  2. SHA-256 compress: CMD: 0x45, DATA: TBS (padded to 64 bytes)
  3. ECC sign:         CMD: 0x41, DATA: SHA-256 hash
  → Returns signature R||S (64 bytes)
```

**Checksum Calculation** (`algo.h: crc16_ccitt()`):
```c
// Qi uses CCITT polynomial with 0xC6C6 init
uint16_t crc16_ccitt(uint8_t *data, uint16_t len, uint16_t init);
```

### 8.2 T91206 (Toppan)

**Files**: `fml/t91206.c`, `fml/t91206.h`

**I2C Protocol** (`transmit_apdu()` @ line 911):

```
TX: [0xAA] [LEN_H] [LEN_L] [APDU...] [CRC8]
RX: [0xAA] [LEN_H] [LEN_L] [DATA...] [SW1] [SW2] [CRC8]

CRC8 = Custom table-based CRC (tmc_i2c_crc)
SW1/SW2: 0x9000 = success, else error code
```

**APDU Structure**:
```c
[CLA] [INS] [P1] [P2] [Lc] [DATA...]

INS_READ = 0xB0       // Read data/digest/certificate
INS_ECC  = 0xE4       // ECC signature
INS_POWERDOWN = 0xAD  // Power down SE
```

**Slot ID Encoding** (`tmc_read_data()` @ line 1028):
```c
// P1/P2 formed from: (offset | slotID)

// Read populated mask:
slotID = 0x80 0X  // X = slot mask (bit0-3)
  → Returns: (max_protocol_ver | populated_mask) + CC_Hash_LSBs

// Read digest/certificate:
slotID = 0xN0 00  // N = slot number (0-3: cert, 4-7: digest)
  → offset [0, LEN_OF_CERTIFICATION)

// Read SEID:
slotID = 0xF0 00
  → offset [0, 0x1000)
```

**Key APIs**:

```c
// Get Qi ID (6 bytes from Product Unit Cert common name)
t91206_get_qi_id(uint8_t *rbuf);
  → TMC_ReadCertification(offset=0x600, len=255, slot=0)
  → Parse X.509 TLV to extract CN field

// Read Digests (32 bytes per slot)
TMC_ReadDigests(slotMask, digests, outLen, slotMaskReq);
  → slotID = (slotNum+4) << 12
  → INS_READ, P1/P2 = slotID

// Read Certificate Chain
TMC_ReadCertification(cert, offset, length, slotNum);
  → Auto-reads: CertLen[2] + N_RH[32] + N_MC[333] + N_PUC[442]

// Sign Challenge (16-byte random → 64-byte signature)
TMC_SignChallenge(random, 16, slotNum, signature, signLen);
  → INS_ECC, P1 = slotNum, P2 = 0x00, DATA = random[16]
  → Returns: R[32] || S[32]
```

**CRC8 Table** (`t91206.c:31-42`):
```c
static const uint8_t crc8Table1[16] = {...};  // Nibble lookup
static const uint8_t crc8Table2[16] = {...};

uint8_t tmc_i2c_crc(uint8_t accum, uint8_t *buf, uint16_t len) {
    for (; buf < ptrEnd; buf++) {
        uint8_t data = accum ^ *buf;
        accum = crc8Table1[data & 0x0F] ^ crc8Table2[data >> 4];
    }
    return accum;
}
```

---

## 9. Protection System

**Files**: `app/prot.c`, `app/prot.h`

### 9.1 Temperature Protection

**NTC Thermistor** (`fml_ntc_temp_get()` @ line 61):

```c
// Hardware: PB5_ADC6 (NU17111) or PB2_ADC2 (NU17112)
// 8-sample averaging buffer
// Lookup table: ntc_tbl[150] maps V_NTC → Temperature [-29°C, +120°C]

static const uint16_t ntc_tbl[] = {
    // NU17112: 10K NTC @ 25°C
    3022, 3008, ..., 1650 (30°C), ..., 186 (120°C)
};

v_ntc = avg(vntc_buf[8]);
temp = table_lookup(v_ntc) - 29;  // Index → °C
```

**NTC OTP** (`fml_tntc_otp_limit_power()` @ line 126):

```c
if (tntc >= 72°C) {
    wpc_stop_to_idle(ESYS_ERR_CODE_NTC_OTP);  // Hard shutdown
}
if (tntc > 55°C) {
    DeltaTemp = tntc - 55;
    tar_cap_otp = max_cap - (10W * DeltaTemp);  // Throttle 10W per °C
    if (tar_cap_otp < nego_cap) {
        nego_cap = tar_cap_otp;
        power_limit_reason = power_limit_reason_ot;
        tntc_ot_flag = 1;  // Send ATN to reduce power
    }
}
```

**Die Temperature** (`fml_die_temp_get()` @ line 105):

```c
// Hardware: Internal TJ sensor (_BADC_CH_INR_TJ_L)
// 8-sample averaging buffer
// Returns: °C (directly from ADC)
```

**Die OTP** (`fml_tdie_otp_check()` @ line 325):

```c
// 10-sample debounce counter
if (tdie > tdie_otp_thd) {
    if (++over_cnt > 10) {
        tdie_otp_flag = 1;
        wpc_stop_to_idle(ESYS_ERR_CODE_DIE_OTP);
    }
}
if (tdie + tdie_otp_hys < tdie_otp_thd) {
    if (++reco_cnt > 10) {
        tdie_otp_flag = 0;  // Hysteresis recovery
    }
}
```

### 9.2 Current Protection

**ISNS OCP** (`fml_isns_ocp_check()` @ line 451):

```c
// Input current (VBUS side)
if (isns > isns_ocp_thd) {
    if (++over_cnt > 5) {
        isns_ocp_flag = 1;
        wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OCP);
    }
}
```

### 9.3 Voltage Protection

**VBUS OVP** (`fml_vbus_ovp_check()` @ line 514):

```c
// 10-sample debounce
if (vbus > vbus_ovp_thd) {
    if (++over_cnt > 10) {
        vbus_ovp_flag = 1;
        wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OVP);
    }
}
```

**VBUS DPL (Deep Power Limit)** (`fml_vbus_dpl_check()` @ line 640):

```c
// Soft limit for CEP control, doesn't stop charging
if (vbus < vbus_dpl_thd) {
    if (++over_cnt > 3) {
        cep_event.dpl = 1;  // Signal to PID controller
    }
}
```

### 9.4 Power Protection

**POUT OPP** (`fml_pout_opp_check()` @ line 760):

```c
// Output power = VPWR * ISNS / 1000
if (vpwr * isns / 1000 > pout_opp_thd) {
    if (++over_cnt > 10) {
        pout_opp_flag = 1;
        wpc_stop_to_idle(ESYS_ERR_CODE_FMW_OPP);
    }
}
```

### 9.5 Protection Summary Table

| Protection | Type | Source | Threshold Access | Action |
|------------|------|--------|------------------|--------|
| TNTC_OTP | OTP | NTC thermistor | `ap->tntc_otp_thd` | Throttle @55°C, Stop @72°C |
| TNTC_UTP | UTP | NTC thermistor | `ap->tntc_utp_thd` | Stop charging |
| TDIE_OTP | OTP | Die temp sensor | `ap->tdie_otp_thd` | Stop charging |
| TDIE_UTP | UTP | Die temp sensor | `ap->tdie_utp_thd` | Stop charging |
| ISNS_OCP | OCP | VBUS current | `ap->isns_ocp_thd` | Stop charging |
| VBUS_OVP | OVP | VBUS voltage | `ap->vbus_ovp_thd` | Stop charging |
| VBUS_UVP | UVP | VBUS voltage | `ap->vbus_uvp_thd` | Stop charging |
| VBUS_DPL | Soft | VBUS voltage | `ap->vbus_dpl_thd` | Set CEP flag |
| VPWR_OVP | OVP | VPWR voltage | `ap->vpwr_ovp_thd` | Stop charging |
| POUT_OPP | OPP | VPWR * ISNS | `ap->pout_opp_thd` | Stop charging |

**Hysteresis**: All protections have `*_hys` parameters to prevent oscillation.
**Debouncing**: Over counters require 3-10 consecutive violations before triggering.

---

## 10. Key Values & Parameters

### 10.1 ASK Timing

| Parameter | Value (counts) | Time (µs) | Purpose |
|-----------|----------------|-----------|---------|
| FULL_BIT_0_MAX | 990 | 880 | Max width for '0' bit |
| FULL_BIT_0_MIN | 439 | 390 | Min width for '0' bit |
| STAR_BIT_0_MIN | 416 | 370 | Min width for start bit |
| HALF_BIT_1_MAX | 338 | 300 | Max width for half '1' |
| HALF_BIT_1_MIN | 152 | 135 | Min width for preamble bit |

**Clock**: 1.125 MHz (1 count ≈ 0.889 µs)

### 10.2 PID Default Limits

**Powerbank 5V Mode** (`pid.c:126-130`):
```c
pid_set_volt_limit(volt_max, volt_min, volt_min);  // Fixed voltage
pid_set_freq_limit(110500, 127772, 147000);        // 130kHz - 110kHz - 98kHz
pid_set_duty_limit(500, 350, 150);                 // 50% - 35% - 15%
pid_set_phas_limit(0, 0, 0);                       // No phase shift
```

**Powerbank 9V Mode** (`pid.c:138-142`):
```c
pid_set_freq_limit(127772, 127772, 127772);        // Fixed ~112kHz
pid_set_duty_limit(500, 350, 100);                 // 50% - 35% - 10%
```

### 10.3 FOD Thresholds

| Mode | Threshold | Recovery | Conditions |
|------|-----------|----------|------------|
| 10W (FO present) | 265 mW | 251 mW | `tx_infos.fo_exist == 1` |
| 15W (No FO) | 385 mW | 365 mW | `tx_infos.fo_exist == 0` |
| Apple MPP | 385 mW | 365 mW | `rx_type == EPRX_TYPE_APPLE_MPP` |

**FOD Count Limit**: `FOD_MAX_CNT = 30` consecutive violations

### 10.4 NU103X Register Values

**Gain Settings**:
- `X36`: Gain = 36 (normal sensitivity)
- `X60`: Gain = 60 (high sensitivity)

**VCAP K-Ratio**:
- K1: 180kΩ/52kΩ = 3.46 (low gain)
- K2: 180kΩ/26kΩ = 6.92 (mid gain)
- K3: 180kΩ/13kΩ = 13.85 (high gain)

**Comparator Hysteresis**:
- Low: 12.5 mV (for stable signals)
- High: 30.0 mV (for noisy environments)

### 10.5 Protection Typical Values

**Temperature** (from `app.c` defaults):
```c
tntc_otp_thd = 72;   // °C, hard stop
tntc_otp_hys = 10;   // °C, recovery hysteresis
tdie_otp_thd = 125;  // °C, die temp limit
```

**Voltage/Current** (example from `app.c`):
```c
vbus_ovp_thd = 22000;  // 22V
vbus_uvp_thd = 3500;   // 3.5V
isns_ocp_thd = 6000;   // 6A
pout_opp_thd = 30000;  // 30W
```

---

## 11. Interaction Map

### 11.1 With WPC Protocol Layer

**ASK → Protocol**:
```
fml_ask_decode() → WPC_EVT_PKT_RECVD → wpc_task()
  ├─ gd->wpc_pkt.hdr/len/data
  ├─ gd->wpc_pkt.src (1=DMO1, 2=DMO2, 3=DMO3)
  └─ gd->wpc_pkt.mark (decoder index)
```

**Protocol → FSK**:
```
wpc_task() → fml_fsk_data_send(EPWM1, delay, data, len)
  → WPC_EVT_FSK_RESP_DONE
```

**Protocol → PID**:
```
wpc_xfer_cep_process() → pid_cep_handler(cep_val)
  → Adjusts pid_volt/perd/duty/phas
  → fml_adp_volt_set() / hal_epwm_pwm_update()
```

**Protocol → FOD**:
```
wpc_xfer_rpp_process() → pfod_mpla() / pfod_common()
  → Returns:
      0 = Safe, continue
      1 = FOD, throttle power
      2 = Increase power, send ATN
      3 = Decrease power, send ATN
```

### 11.2 With BSP/HAL Layer

**ASK**:
```
ECAP1/2 ISR → fml_ask_int_handler() → ask_dm[].tim.tim_buff[]
  → FML_EVT_ASK_INT_RECVD → fml_ask_decode()
```

**FSK**:
```
FSK1/2_IRQHandler() → fml_fsk_int_callback()
  → Loads next byte to EPWM->FSK_BUFF
  → WPC_EVT_FSK_RESP_DONE when complete
```

**NU103X**:
```
fml_nu103x_config(cmd) → GPIO pulse sequence on GPC_PIN2
  → Updates gd->nu103x_sts_curr shadow register
```

**QDT**:
```
fml_qdt_detect() → ECAP1/2 in QDT mode
  → Reads ECAP1->QDT_MEAS.VQM / ECAP2->QDT_MEAS.NQM
  → Calculates gd->tx_infos.q_fact / f_self
```

**Protection**:
```
prot_task() (100ms periodic)
  ├─ fml_ntc_temp_get() → BADC_CH_PB2_ADC2
  ├─ fml_die_temp_get() → BADC_CH_INR_TJ_L
  ├─ fml_isns_ocp_check(gd->isns_avg)
  └─ fml_vbus_ovp_check(gd->vbus)
```

---

## 12. Expert Insights

### 12.1 ASK Demodulation Tuning

**Problem**: Packet errors increase with certain phones.
**Root Cause**: Dual-decoder thresholds too strict.

**Solution**:
```c
// lib/ask.c:836-851
// Decoder 0: Tight timing for clean signals
ask_dm[i].decode[0].bit_zero_min = FULL_BIT_0_MIN_THD1;  // 439
ask_dm[i].decode[0].bit_one_min  = HALF_BIT_1_MIN_THD1;  // 152

// Decoder 1: Relaxed timing for noisy signals
ask_dm[i].decode[1].bit_zero_min = FULL_BIT_0_MIN_THD2;  // 437
ask_dm[i].decode[1].bit_one_min  = HALF_BIT_1_MIN_THD2;  // 169
```

**Tuning Notes**:
- Increase `HALF_BIT_1_MIN` if missing short pulses
- Decrease `FULL_BIT_0_MIN` if false-triggering on long '1' bits
- Check `dm_bad_cnt` in logs to identify problematic channel

### 12.2 Q-Factor Drift Compensation

**Problem**: Q-factor changes with temperature, causing false FOD.

**Mitigation**:
```c
// qfod.c:91 - Use wide tolerance bands
if (q_fact + 110 < q_factor_base_value && f_self + 50 > fs_base_value) {
    // ±110 Q units, ±50 kHz tolerance
}
```

**Best Practice**:
- Calibrate at 25°C after 30min warmup
- Re-calibrate if ambient temp changes >10°C
- Store multiple baseline Q/F sets for different temp ranges

### 12.3 ASK Noise Rejection

**Issue**: EMI from switching power supply couples into DMO channels.

**Hardware Fixes**:
1. Enable BPF_2ORD for better stopband attenuation
2. Use AUTO gain mode to adapt to noise floor
3. Increase comparator hysteresis to 30mV in noisy env

**Software Workaround**:
```c
// lib/ask.c:1326 - 128kHz ping uses high gain + 30mV hysteresis
fml_nu103x_config(_1030_CFG_DMOx_DDM_CMP_HYST_30P0mV);
fml_ask_dmo1_cfg(&dmo1_cfg_128_ping[0]);  // fix_high gain

// lib/ask.c:1367 - 360kHz ping uses auto gain + 12.5mV hysteresis
fml_nu103x_config(_1030_CFG_DMOx_DDM_CMP_HYST_12P5mV);
fml_ask_dmo1_cfg(&dmo1_cfg_360_ping[0]);  // auto_gain
```

### 12.4 PID Oscillation Debugging

**Symptom**: CEP oscillates between -2 and +2.

**Diagnosis**:
1. Check `m_pid_ctrl_mode` in logs → should settle on one mode
2. If toggling VOLT/FREQ → limits too tight
3. If toggling DUTY/PHASE → step size too large

**Fix**:
```c
// pid.c:303 - Reduce frequency step
gd->pid_perd += cep/3 + 1;  // Original
gd->pid_perd += cep/4;      // Gentler (remove +1 offset)

// pid.c:338 - Reduce duty step
gd->pid_duty += cep/2 + 1;  // Original
gd->pid_duty += cep/3;      // Gentler
```

### 12.5 PFOD False Positives

**Problem**: High `ploss` calculation triggers FOD on thick cases.

**Analysis**:
- `Pfm` term dominates for high-permeability materials
- `alpha_fm` from RX may be under-reported

**Workaround**:
```c
// pfod.c:139-158 - Add power-dependent offset
if (rx_power < 3000) {
    pfo += 400;  // Loosen threshold for low power
} else if (rx_power < 4000) {
    pfo += 300;
}
```

**Long-term Solution**: Request WPC Qi consortium for revised `alpha_fm` reporting in PLA packet.

---

## 13. Quick Reference

### 13.1 DMO Configuration Cookbook

**Ping Phase (128kHz)**:
```c
fml_ask_128_ping_cfg();
  → DMO1: IAVG, fix_high gain, BPF_1ord
  → DMO2: VCAP K1, fix_high gain, BPF_1ord
  → Hysteresis: 30mV
```

**Ping Phase (360kHz)**:
```c
fml_ask_360_ping_cfg();
  → DMO1: IAVG, auto_gain, BPF_1ord
  → DMO2: DIGITAL (SW demod), fix_high gain
  → Hysteresis: 12.5mV
```

**Transfer Phase (BPP, <2.5W)**:
```c
fml_ask_dmo1_xfer_cfg();
  → DMO1: Rotates [VDM_fix_high, IAVG_fix_normal, VDM_fix_normal, IAVG_auto]
fml_ask_dmo2_xfer_cfg();
  → DMO2: Rotates [VCAP_K2_fix_normal, PHASE_K2_fix_normal, VCAP_K3_fix_high, PHASE_K3_fix_high]
```

**Transfer Phase (EPP/MPP, >6W)**:
```c
  → DMO2: Rotates [VCAP_K3_fix_normal, VCAP_K3_auto, VCAP_K3_fix_high, PHASE_K2_fix_high]
```

### 13.2 Register Snapshot Commands

**Read NU103X State**:
```c
printk("NU103X: %08X\n", gd->nu103x_sts_curr.WORD);
// Decode bits manually or use nu103x.h BITS union
```

**Read PID State**:
```c
printk("PID: V=%d F=%d(%d) D=%d P=%d\n",
    gd->pid_volt, 144000000/gd->pid_perd, gd->pid_perd, gd->pid_duty, gd->pid_phas);
```

**Read FOD Counters**:
```c
printk("FOD: fod_cnt=%d pfo=%d pfo_avg=%d thd=%d\n",
    fod_count, pfo, pfo_avg, pfo_thd);
```

### 13.3 Common Register Addresses

| Register | File | Address/Macro | Purpose |
|----------|------|---------------|---------|
| ECAP1 | regdef.h | `ECAP1` | ASK DMO1 edge capture |
| ECAP2 | regdef.h | `ECAP2` | ASK DMO2 edge capture |
| ECAP4 | regdef.h | `ECAP4` | Digital DDM |
| EPWM1 | regdef.h | `EPWM1` | TX coil drive + FSK |
| BADC ADC2 | badc.h | `_BADC_CH_PB2_ADC2` | NTC thermistor |
| BADC TJ | badc.h | `_BADC_CH_INR_TJ_L` | Die temperature |

---

## 14. Reference Material Needs

### 14.1 External Specs Required

1. **Qi v1.3.2 Specification** (WPC)
   - ASK/FSK physical layer timing
   - Packet structure (Header/Message/Checksum)
   - Power Loss FOD reference algorithm

2. **NU103X Datasheet** (NuVolta)
   - DMO1/DMO2 gain curves
   - QDT measurement accuracy vs. Q-factor
   - OCP/OVP hardware threshold mapping

3. **FM1210 Integration Guide** (FocalMCU)
   - I2C command set (0x30/0x41/0x45)
   - SHA-256 padding requirements
   - ECC P-256 signature format

4. **T91206 SDK Manual** (Toppan)
   - APDU INS codes (0xB0/0xE4)
   - Slot ID encoding (digest vs. cert)
   - CRC8 polynomial confirmation

### 14.2 Internal Docs to Review

1. **app/wpc_xfer.c** - Integration points for CEP/RPP processing
2. **app/adp.c** - Voltage adjustment commands for QC/PD adapters
3. **hal/ecap.c** - ECAP ISR routing and buffer management
4. **hal/epwm.c** - PWM update timing constraints

---

**End of WPC Hardware Knowledge Document**

*For WPC protocol layer (packet handling, state machines), see `agent_knowledge_wpc_protocol.md`.*
*For power management/BMS, see `agent_knowledge_power_mgmt.md`.*
