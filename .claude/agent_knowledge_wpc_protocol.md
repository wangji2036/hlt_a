# Platform-WPC-Protocol Agent Knowledge

**Domain**: Wireless Power Consortium (WPC) Qi 2.x Protocol State Machine
**Jurisdiction**: 31 files in `app/` implementing TX (Power Transmitter) protocol stack
**Depth**: Level 4 - Expert protocol implementation knowledge

---

## 1. Module Overview

### 1.1 Purpose
Implements **Qi 2.x Wireless Power Transmitter (PTx)** protocol stack for powerbank wireless charging. Supports three power profiles: **BPP (5W), EPP (15W), MPP (25W)** with full state machine, packet processing, FOD (Foreign Object Detection), and authentication.

### 1.2 Architecture Hierarchy
```
WPC Protocol Stack (TX Side)
│
├── [Main Loop] _wpc.c/h
│   ├── wpc_task_event_handler()  - Central event dispatcher
│   ├── wpc_protocol_sm()         - Phase router
│   └── wpc_stop_power()          - Safe shutdown handler
│
├── [6 Protocol Phases]
│   ├── Phase 0: IDLE    → wpc_idle.c/h       (QDT-FOD, Ping Decision)
│   ├── Phase 1: PING    → wpc_ping.c/h       (Signal Strength Detection)
│   ├── Phase 2: CNFG    → wpc_cnfg.c/h       (ID/Config Exchange)
│   ├── Phase 3: NEGO    → wpc_nego.c/h       (Power Negotiation)
│   ├── Phase 4: XFER    → wpc_xfer.c/h       (Power Transfer)
│   └── Phase 5: CLOAK   → wpc_xfer.c         (Cloaking Mode)
│
├── [Power Profiles]
│   ├── BPP → wpc_5_xfer_1_bpp.c/h  (5W, no nego)
│   ├── EPP → wpc_5_xfer_2_epp.c/h + epp.c/h (15W, auth optional)
│   └── MPP → wpc_5_xfer_3_mpp.c/h + mpp.c/h (25W, auth + calibration)
│
├── [Sub-Protocols]
│   ├── Data Stream      → wpc_5_xfer_4_dstrm.c/h
│   ├── IOC Test Support → wpc_6_test_1_ioc.c/h
│   └── IOP Test Support → wpc_6_test_2_iop.c/h
│
└── [Dependencies]
    ├── FSK Modulation   → fsk.c/h (Frequency Shift Keying)
    ├── PID Control      → pid.c/h (Voltage/Frequency regulation)
    ├── FOD Detection    → pfod.c/h, qfod.c/h (QDT + Power Loss)
    └── Hardware Driver  → nu103x (DDM), ask.c (ASK demod)
```

### 1.3 File Organization
| File Pattern | Count | Purpose |
|-------------|-------|---------|
| `_wpc.c/h` | 2 | Main coordinator, error codes, timeout constants |
| `wpc_idle.c/h` | 2 | IDLE phase: QDT-FOD, ping scheduling |
| `wpc_ping.c/h` | 2 | PING phase: signal detection |
| `wpc_cnfg.c/h` | 2 | CONFIG phase: ID/CFG packet exchange |
| `wpc_nego.c/h` | 2 | NEGOTIATION phase: power contract |
| `wpc_xfer.c/h` | 2 | XFER phase: power transfer + cloak |
| `wpc_5_xfer_*.c/h` | 8 | BPP/EPP/MPP/DataStream sub-handlers |
| `wpc_6_test_*.c/h` | 4 | IOC/IOP compliance test support |
| `epp.c/h + config` | 4 | EPP authentication, negotiation |
| `mpp.c/h` | 2 | MPP power limits, mode transitions |
| **Total** | **31** | **Complete Qi 2.x TX implementation** |

---

## 2. State Machine Architecture

### 2.1 Top-Level Phase Transitions
```
Qi 2.x TX State Machine (gd->ptx_protocol_phase)

 ┌─────────────┐  dig_ping   ┌─────────────┐  SS>0   ┌─────────────┐
 │ 0: IDLE     │─────────────→│ 1: PING     │────────→│ 2: CNFG     │
 │ (QDT-FOD)   │←─────────────│ (70ms)      │         │ (ID/CFG)    │
 └─────────────┘  no signal   └─────────────┘         └─────────────┘
       ↑                                                      │
       │ error/timeout                                       │ CFG received
       │                                                     ↓
 ┌─────────────┐             ┌─────────────┐         ┌─────────────┐
 │ 5: CLOAK    │←───ATN──────│ 4: XFER     │←────ACK─│ 3: NEGO     │
 │ (suspend)   │             │ (CEP/RPP)   │         │ (PTC/SRQ)   │
 └─────────────┘             └─────────────┘         └─────────────┘
       │                            ↑ │                      │
       └────────restore──────────────┘ └─────renegotiate─────┘
                GET_28                        NEGO_09
```

**Critical Path**: `IDLE → PING → CNFG → [NEGO] → XFER`
**Optional Loops**: XFER ↔ NEGO (renegotiation), XFER ↔ CLOAK (suspend/resume)

### 2.2 IDLE Phase Sub-States (`gd->ptx_idle_phase_status`)
```c
enum ptx_idle_phase_state_t {
    WPC_IDLE_STAT_STANDBY  = 0,  // Normal: waiting for object
    WPC_IDLE_STAT_XER_COM  = 1,  // Post-transfer: Charge Complete
    WPC_IDLE_STAT_XER_FOD  = 2,  // Post-transfer: FOD detected
    WPC_IDLE_STAT_QDT_FOD  = 3,  // FOD during QDT (foreign object)
    WPC_IDLE_STAT_LAR_MET  = 4,  // Large metal detected
    WPC_IDLE_STAT_EPT_ERR  = 5,  // EPT error received
    WPC_IDLE_STAT_EPT_RES  = 6,  // EPT restart request
    WPC_IDLE_STAT_EPT_REP  = 7,  // EPT re-ping request
    WPC_IDLE_STAT_CLOAKING = 8,  // Cloaking mode active
    WPC_IDLE_STAT_QDT_CALI = 9,  // QDT calibration mode
};
```

**Key Logic** (`wpc_idle.c:qfod_detect()`):
- **QDT Detection**: Every `t_next_ping` (200ms default), measure Q-factor/frequency
- **Object Decision**:
  - `q_fact < base - obj_value` OR `f_self < base - obj_value` → Object present
  - After `pin_fod_cnt` (3) consecutive detections → Digital Ping
  - If no object after `pin_max_cnt` (30) pings → Force dig_ping anyway
- **State Persistence**:
  - `XER_COM/EPT_ERR` persist for 11 minutes before reset
  - `EPT_REP` triggers immediate re-ping after `reping_cnt * 100ms`

### 2.3 XFER Phase Power Profile Dispatch
```c
// wpc_xfer.c:689
void wpc_xfer_phase_process(struct com_prx_ask_pkt_t *com_pkt) {
    switch (gd->rx_infos.power_profile_mode) {
        case BPP: wpc_bpp_xfer_phase_protocol_process(com_pkt); break;
        case EPP: wpc_epp_xfer_phase_protocol_process(com_pkt); break;
        case MPP: wpc_mpp_xfer_phase_protocol_process(com_pkt); break;
    }
}
```

**Profile Selection**:
- **BPP**: Default if no negotiation, `qi_version < 0x12`, or `adp.pwr_high < 20`
- **EPP**: Triggered by `CFG.neg=1` AND `qi_version >= 0x12` AND `pwr_high >= 20`
- **MPP**: Triggered by XID packet `selector=0xFE` AND `pwr_high >= 30`

---

## 3. Phase-by-Phase Protocol Logic

### 3.1 Phase 0: IDLE (wpc_idle.c)
**Entry**: After `wpc_stop_power()` or system boot
**Exit**: Object detected → `WPC_PHASE_PING`

**Critical Functions**:
```c
void wpc_idle_phase_process(void) {
    // 1. QDT Detection
    fml_qdt_detect(&q_fact, &f_self);

    // 2. FOD Decision
    if (qfod_detect()) return;  // No object, stay in IDLE

    // 3. Ping Preparation
    if (dig_ping_type == _128K_HB) {
        wpc_idle_dig_ping_init_128K();  // 128kHz half-bridge
    } else {
        wpc_idle_dig_ping_init_360K();  // 360kHz full-bridge (MPP restricted)
    }

    // 4. Phase Transition
    gd->ptx_protocol_phase = WPC_PHASE_PING;
    osal_start_timerEx(WPC_NEXT_TIMER, T_PING=70ms, WPC_EVT_PIN_NO_PKT);
}
```

**Ping Type Selection**:
- **128kHz HB**: Default, better compatibility, duty ramp-up for FOD
- **360kHz FB**: MPP restricted mode (`mpp_restricted_mode=1`), requires re-ping

**QDT-FOD Parameters** (from `idle_qfod_init()`):
- `q_factor_obj_value`: -120 threshold for object detection
- `fs_obj_value`: -360 Hz threshold
- `pin_fod_cnt`: 3 consecutive detections to confirm
- `pin_max_cnt`: 30 QDT cycles before forcing dig_ping

### 3.2 Phase 1: PING (wpc_ping.c)
**Entry**: After digital ping starts
**Exit**: Signal Strength packet (0x01) received → `WPC_PHASE_CNFG`
**Timeout**: `T_PING=70ms` → back to IDLE

**Packet Handler** (`wpc_ping_phase_process()`):
```c
void wpc_ping_phase_process(struct com_prx_ask_pkt_t *com_ask) {
    if (com_ask->hdr == WPC_PRx_PKT_TYP_SIG_01) {
        gd->rx_infos.ssp_value = com_ask->msg.sig.ss_value;
        gd->ptx_protocol_phase = WPC_PHASE_CNFG;
        osal_start_timerEx(WPC_NEXT_TIMER, T_NEXT=23ms, WPC_EVT_CNFG_NEXT_1ST_TO);

        // Initialize for new session
        auth_init();
        pfod_init();
        idle_qfod_init();
    } else {
        wpc_stop_to_idle(ESYS_ERR_CODE_PING_PHASE_1ST_PKT_TYPE_ERR);
    }
}
```

**Key Actions**:
- Store signal strength (`ssp_value`)
- Initialize authentication, FOD, and idle state
- Start `T_NEXT` timer for ID packet

### 3.3 Phase 2: CNFG/I&C (wpc_cnfg.c)
**Entry**: After Signal Strength packet
**Exit**: CFG (0x51) received → `WPC_PHASE_XFER` or `WPC_PHASE_NEGO`
**Timeout**: `T_NEXT=23ms` between packets

**Expected Sequence**:
1. **ID (0x71)**: Mandatory first packet
   ```c
   qi_version = major_ver << 4 | minor_ver;
   prmc = prmc_msb << 8 | prmc_lsb;  // Manufacturer code
   device_id = bdid0/1 (4 bytes);
   ```
   **RX Type Detection** (31 types supported):
   - `0x005A` + `qi=0x20` → `EPRX_TYPE_APPLE_STD`
   - `0x005C` + `qi=0x20` → `EPRX_TYPE_NUVOLTA`
   - `0x0042` → `EPRX_TYPE_SAMSUNG`
   - See `get_prx_type()` for full mapping

2. **XID (0x81)**: Optional extended ID (MPP only)
   ```c
   if (device_id & 0x80000000) {  // Extended bit set
       if (selector == 0xFE) {
           power_profile_mode = MPP;
           // Calculate k_est from alpha0/alpha1/vrect
           k_est = (vrect * alpha0 / (vctx_pp + vpwr)) * 15926/100 + ...
       }
   }
   ```

3. **PCH (0x06)**: Optional power control hold-off
   ```c
   pch_t_delay = pch_time;  // Range: T_PCH_TIME_MIN=5 ~ MAX=100ms
   ```

4. **CFG (0x51)**: Final packet, triggers mode selection
   ```c
   if (power_profile_mode == MPP && !mpp_restricted_mode) {
       // Send FSK_MPP, enter NEGO
       ptx_protocol_phase = WPC_PHASE_NEGO;
   } else if (qi_version >= 0x12 && neg == 1 && pwr_high >= 20) {
       // Send FSK_ACK, enter EPP NEGO
       power_profile_mode = EPP;
       ptx_protocol_phase = WPC_PHASE_NEGO;
   } else {
       // BPP mode, go directly to XFER
       ptx_protocol_phase = WPC_PHASE_XFER;
   }
   ```

**MPP Restricted Mode Handling**:
- If `mpp_restricted_mode=1` received in XID
- AND current ping is 128kHz → **Re-ping with 360kHz**
- Error code: `ESYS_ERR_CODE_IDCFG_PHASE_MPP_RESTRICTED_REP`

### 3.4 Phase 3: NEGO (wpc_nego.c + epp.c)
**Entry**: After CFG packet in EPP/MPP modes
**Exit**: SRQ/end (0x20,param=0x00) → `WPC_PHASE_XFER`
**Timeout**: `T_NEGOTIATE=400ms` between packets

#### 3.4.1 EPP Negotiation (epp.c)
**Packet Flow**:
```
RX→TX: GRQ/0x07(0x30) → TX→RX: ID (0x30, qi_ver, ptmc)
RX→TX: GRQ/0x07(0x31) → TX→RX: CAP (0x31, neg_power=30, auth=1)
RX→TX: SRQ/0x20(gp)   → TX→RX: ACK (guaranteed power accepted)
RX→TX: FOD/qf (0x22)  → TX→RX: ACK (Q-factor FOD reference stored)
RX→TX: FOD/rf (0x22)  → TX→RX: ACK (Frequency FOD reference stored)
RX→TX: SRQ/0x20(0x00) → TX→RX: ACK → XFER phase
```

**FOD Reference Storage** (`epp.c:wpc_epp_FOD_pkt_process()`):
```c
if (fod_type == FOD_TYPE_qf) {
    gd->rx_infos.ref_q = ref_qf_msb << 8 | ref_qf_lsb;
} else {  // FOD_TYPE_rf
    gd->rx_infos.ref_f = ref_rf_msb << 8 | ref_rf_lsb;
}
contract.nego_fod_mask |= (1 << fod_type);  // Track FOD packets received
```

**Critical Check** (`epp.c:290`):
```c
if (nego_flag == 1 && contract.nego_fod_mask != 0x03) {
    // Qi > 1.3 MUST send both FOD/qf and FOD/rf
    fml_fsk_patt_send(_FSK_NAK);
    power_profile_mode = BPP;  // Fallback to BPP
}
```

#### 3.4.2 MPP Negotiation (wpc_nego.c)
**Packet Types**:
- **GRQ (0x07)**: General requests for ID/CAP
- **SRQ (0x20)**: Specific requests (9 parameters)
- **GET (0x28)**: Get PTx capabilities (9 selectors)

**SRQ Parameters**:
```c
enum mpp_prx_srq_request_type_t {
    SRQ_END_00 = 0x00,                 // End negotiation
    SRQ_REP_05 = 0x05,                 // Re-ping delay
    SRQ_PCH_07 = 0x07,                 // Power control hold-off
    SRQ_FREQ_SEL_F0 = 0xF0,            // Frequency selection (360kHz)
    SRQ_EPGL_F3 = 0xF3,                // Extended nego power limit
    SRQ_CLOAK_DIG_PING_LSB_F5 = 0xF5,  // Cloaking dig_ping delay LSB
    SRQ_PCP_F6 = 0xF6,                 // Power control profile
    SRQ_CLOAK_DIG_PING_MSB_F7 = 0xF7,  // Cloaking dig_ping delay MSB
    SRQ_CLOAK_DET_PING_F8 = 0xF8,      // Cloaking det_ping delay
};
```

**GET Responses** (`wpc_nego.c:mpp_get_pkt_process()`):
```c
switch (param) {
    case GET_PTx_XID:  // 0x00
        // Send PTx Basic Device ID
        break;
    case GET_PTx_INV:  // 0x02
        vinv = (gd->vpwr >> 1) & 0x1FF;  // PTx input voltage
        break;
    case GET_PTx_ECAP: // 0x04
        ptx_potential_power = gd->tx_infos.max_cap;
        prx_negotiable_power = gd->tx_infos.nego_cap;
        power_limit_reason = gd->tx_infos.power_limit_reason;
        break;
    case GET_PTx_KEST: // 0x07
        kest_value = (gd->k_est * 4095 / 10000);  // Coupling coefficient
        break;
}
```

**Power Contract Changes** (`power_contract_change_cnt()`):
- Count differences between `prx_power_contract` and `ptx_power_contract`
- On `SRQ_END_00`, verify count matches parameter
- If match → Copy to `ptx_power_contract`, send ACK, enter XFER

### 3.5 Phase 4: XFER (wpc_xfer.c + profile-specific files)
**Entry**: After negotiation complete or BPP CFG
**Exit**: EPT (0x02), timeout, or FOD → `WPC_PHASE_IDLE`

#### 3.5.1 Common XFER Packets (All Profiles)
| Packet | Header | Purpose | Handler |
|--------|--------|---------|---------|
| **CEP** | 0x03 | Control Error Packet | Restart `CEP_TIMER` (BPP:1600ms, MPP:2050ms) |
| **RP8** | 0x04 | 8-bit Received Power | Update `rx_power`, restart `RPP_TIMER` |
| **CHS** | 0x05 | Charge Status | Store `chr_status` (0-100%) |
| **EPT** | 0x02 | End Power Transfer | Call `wpc_ept_pkt_process()` |

**CEP Processing** (`wpc_xfer.c:168`):
```c
gd->rx_infos.cep_val = com_ask->msg.cep.ce_value;  // Range: -127 to +127

// BPP: Check if power limiting needed
if (rx_power > 6800) {
    gd->rx_infos.mpp_restricted_power_limit = 1;
    if (cep_val > 0) cep_val = 0;  // Clamp positive CEP
}

osal_start_timerEx(WPC_NEXT_TIMER, pch_t_delay, WPC_EVT_PCH_TO);
```

**Power Calculation** (`_wpc.c:475-479`):
```c
// Every 4 windows (4x4ms = 16ms), sample and average
gd->isns_avg = (isns[0] + isns[1] + isns[2] + isns[3]) >> 2;
gd->vpwr_avg = (vpwr[0] + vpwr[1] + vpwr[2] + vpwr[3]) >> 2;
gd->tx_power = gd->isns_avg * gd->vpwr_avg / 1000;  // mW
```

#### 3.5.2 BPP XFER (wpc_5_xfer_1_bpp.c)
**Packet Set**: `CEP(0x03), RP8(0x04), CHS(0x05)`
**FOD**: IOC BPP FOD (`ioc_bpp_fod_handler()`)
- Triggered every `RP8` packet
- Checks `rpp_value` delta against threshold
- No Q-factor FOD in BPP mode

**Special Handling**:
- **Samsung PPDE** (Proprietary 0x18/0x28 packets):
  ```c
  if (prmc == 0x0042 && prop[0] == 0x06 && prop[1] == 0x2C) {
      samsungPrivateFastChargeFlag = 1;
      gd->pid_duty += 200;  // Boost duty cycle
  }
  ```

#### 3.5.3 EPP XFER (wpc_5_xfer_2_epp.c + epp.c)
**Additional Packets**:
- **RP24 (0x31)**: 24-bit received power (higher precision)
- **NEGO (0x09)**: Re-negotiation request
- **DSR (0x15)**: Data Stream Request (authentication)
- **ADC (0x25)**: Authentication Data Control
- **ADT (0x16-0x77)**: Authentication Data Transport (even/odd chunks)

**Authentication Flow** (`epp.c:EPP_auth_t`):
```
State: EPP_Auth_IDLE → EPP_Auth_GET_DIGEST
RX→TX: ADC(0x25, auth=0x02) → Start auth
RX→TX: DSR(0x15, open) → Open data stream
TX→RX: SDSR(0x00, ACK) → Stream opened

State: EPP_Auth_GET_DIGEST
RX→TX: ADC(0x25, request=0x09) → Request digest
TX→RX: ADT(0x16-0x77, digest[32]) → Send digest chunks

State: EPP_Auth_GET_CERTIFICATE
RX→TX: ADC(0x25, request=0x0A) → Request cert
TX→RX: ADT(0x16-0x77, cert[328]) → Send certificate

State: EPP_Auth_GET_CHALLENGE
RX→TX: ADC(0x25, request=0x0B) → Request challenge response
RX→TX: ADT(0x16-0x77, challenge[64]) → RX sends challenge
TX→RX: ADT(0x16-0x77, signature[64]) → TX responds
```

**ADT Packet Chunking** (`epp.c:is_ADT_valid_packet_type()`):
- Headers: 0x16/0x17, 0x26/0x27, ..., 0x76/0x77 (even/odd pairs)
- Each packet: 1-byte header + max 9-byte payload
- Total 7 pairs = 140 bytes max per exchange
- Even (0x*6) = first chunk of pair, Odd (0x*7) = second chunk

#### 3.5.4 MPP XFER (wpc_5_xfer_3_mpp.c)
**MPP-Specific Packets**:
| Packet | Header | Purpose |
|--------|--------|---------|
| **XCE** | 0x19 | Extended Control Error (-255 to +255) |
| **MSR** | 0x13 | Mode Switch Request (CPM/NPM/LPM/HPM) |
| **REPORT** | 0x58 | Power Loss & Authentication Report |
| **PLA2** | 0x88 | Power Loss Assessment with Vrect/Irect |
| **CAL_CAP** | 0x96 | Calibration Capture (ΔP-loss calibration) |

**Power Mode Transitions** (`wpc_xfer.c:586`):
```c
enum mpp_power_mode_t {
    continuous = 0,  // CPM: Continuous Power Mode (full power)
    nominal    = 1,  // NPM: Nominal Power Mode (default)
    light      = 2,  // LPM: Light Power Mode (low power)
    high       = 3,  // HPM: High Power Mode (max 25W)
};

void process_MSR(main_mode, aux_mode) {
    if (gd->power_mode != main_mode) {
        gd->power_mode = main_mode;
        fsk_pkt.status = 0;  // Success
    }
    fml_fsk_data_send(MSS_23, &status);  // Mode Switch Status
}
```

**MPLA FOD** (`pfod_mpla()` in wpc_xfer.c:282):
```c
uint8_t res = pfod_mpla();  // Power Loss Assessment
if (res == 0) {
    // No FOD detected
    if (ntc_ot_flag && (rx_prect+200)/100 <= nego_cap && tntc_ot_flag_atn==1) {
        fml_fsk_patt_send(_FSK_ATN);  // Request re-negotiation for lower power
        need_renego_cap = 1;
    } else {
        fml_fsk_patt_send(_FSK_ACK);
    }
} else if (res == 1) {
    fml_fsk_patt_send(_FSK_NAK);  // Power adjustment needed
    cep_val = -8;  // Request voltage decrease
} else {
    fml_fsk_patt_send(_FSK_ATN);  // FOD detected
}
```

**ΔP-loss Calibration** (Qi 2.2+):
```
1. CAL_START (0x2C) → Accept/Reject (0x34)
2. CAL_CAPTURE (0x96) × 100 points → ACK each
3. CAL_OP (0x2B, commit) → Calculate alpha/beta coefficients
4. Update ECAP with higher nego_cap (250 vs 150)
```

**Cloaking Mode** (`gd->tx_infos.flg_mode_cloak`):
- Triggered by CLOAK (0x18) packet during XFER
- PTx suspends power, enters `WPC_PHASE_CLOAK`
- Periodic detection pings (`cloak_det_ping_delay` × 100ms)
- Resume on GET (0x28) packet → back to XFER

### 3.6 Phase 5: CLOAK (wpc_xfer.c:784)
**Entry**: CLOAK (0x18) packet received in XFER
**Exit**: GET (0x28) → `WPC_PHASE_XFER`, or error → `WPC_PHASE_IDLE`

**Cloak Ping Types**:
- **Detection Ping**: Short 360kHz burst, check Q/F for RX presence
- **Digital Ping**: Full 360kHz ping with DDM enabled
- **Interval**: `cloak_det_ping_delay` detection pings, then 1 digital ping

**Packet Handler** (`wpc_mpp_cloak_phase_protocol_process()`):
```c
case MPP_PRx_PKT_TYP_CLOAK_18:
    cloak_reason = msg.cloak.reason;  // 0x00-0x06 valid
    fml_fsk_patt_send(_FSK_ACK);
    fsk_done_event |= 4;  // Trigger cloak entry
    break;

case MPP_PRx_PKT_TYP_REPORT_58:
    // Verify RX ID matches original RX (prevent cloak hijack)
    tmp_id = (prx_byteid0 << 16 | ...) >> 3) & 0xFFFFF;
    tmp_base_id = (device_id >> 11) & 0xFFFFF;
    if (tmp_base_id != tmp_id) goto _CLOAK_PHASE_ERR_;
    break;

case MPP_PRx_PKT_TYP_GET_28:
    // Exit cloak, resume XFER
    mpp_get_pkt_process(mpp);
    ptx_protocol_phase = WPC_PHASE_XFER;
    osal_start_timerEx(WPC_CEP_TIMER, T_MPP_CE_TO);
    break;
```

---

## 4. Key Numerical Values

### 4.1 Timeout Constants (`_wpc.h:89-112`)
| Constant | Value (ms) | Usage | Phase |
|----------|-----------|-------|-------|
| `T_PING` | 70 | Wait for Signal Strength packet | PING |
| `T_NEXT` | 23 | Wait for next I&C packet | CNFG |
| `T_FIRST_LIMIT` | 20 | First packet arrival window | PING |
| `T_MAX_LIMIT` | 170 | Maximum packet interval | CNFG |
| `T_NEGOTIATE` | 400 | Wait for negotiation packet | NEGO |
| `T_RENEGOTIATE` | 814 | Re-negotiation packet timeout | XFER |
| `T_RENEGO_TO` | 1900 | Total re-negotiation timeout | XFER |
| `T_TERMINATE` | 10 | Power off after FSK | XFER/CLOAK |
| `T_RESPONSE` | 3 | FSK response delay | All |
| `T_WINDOW` | 4 | Power sampling window | XFER |
| `T_ACTIVE` | 20 | Time before CEP window | XFER |
| `T_XCE_RESP_TO` | 20 | XCE response timeout | XFER (MPP) |
| `T_COM_CE_TO` | 1600 | CEP timeout (BPP/EPP) | XFER |
| `T_COM_DDM_TO` | 600 | DDM sampling timeout | XFER |
| `T_MPP_CE_TO` | 2050 | CEP timeout (MPP) | XFER |
| `T_COM_RP_TO` | 20000 | RP timeout (BPP/EPP) | XFER |
| `T_MPP_RP_TO` | 7900 | RP timeout (MPP) | XFER |
| `T_PCH_TIME_MIN` | 5 | Min power control holdoff | CNFG/XFER |
| `T_PCH_TIME_MAX` | 100 | Max power control holdoff | CNFG/XFER |
| `T_CLOAK_PING` | 100 | Cloak detection ping period | CLOAK |
| `T_CLOAK_TIMEOUT` | 239 | Cloak packet timeout | CLOAK |
| `T_CLOAK_TIMEOUT_EX` | 500 | Extended cloak timeout | CLOAK |

### 4.2 Power Profile Capabilities
| Profile | Max Power | Nego Required | Auth | FSK Cycles | Voltage Range |
|---------|-----------|---------------|------|----------|---------------|
| **BPP** | 5W (10 × 0.5W) | No | No | 512 | 5V - 10V |
| **EPP** | 15W (30 × 0.5W) | Yes | Optional | 512 | 5V - 20V |
| **MPP** | 25W (250 × 0.1W) | Yes | Required | 128 | 5V - 20V |

**EPP Configuration** (`epp_config.h`):
```c
#define EPP_CAP_NEGOTIABLE_POWER  0x1E  // 15W (30 × 0.5W)
#define EPP_CAP_POTENTIAL_POWER   0x1E  // 15W
#define EPP_CAP_AR                1     // Authentication Required
#define EPP_RP1_TIMEOUT          5050   // First RP timeout (ms)
#define EPP_RP2_TIMEOUT          21000  // Subsequent RP timeout (ms)
#define EPP_CEP_TIMEOUT          2050   // CEP timeout (ms)
```

**MPP Power Limits** (`mpp.h`):
```c
#define MPP_25W_HPM_PING_ENABLE   1     // High Power Mode ping
#define MPP_25W_LOW_K_VALUE       8100  // Coupling threshold
#define MPP_25W_360K_DIG_PING_PHASE 50  // Phase shift (degrees)
#define MPP_25W_FOD_ENABLE        1     // Enable MPLA FOD
```

### 4.3 Frequency/Duty Parameters
**128kHz Half-Bridge** (default):
```c
gd->dig_ping_perd = 144000000 / 127772;  // ~1127 (127.772kHz)
gd->dig_ping_duty = 125;  // 50% duty (250/1127)
gd->dig_ping_phas = 0;    // No phase shift
```

**360kHz Full-Bridge** (MPP restricted/cloak):
```c
gd->dig_ping_perd = 144000000 / 360000;  // 400 (360kHz)
gd->dig_ping_duty = 500;  // 50% duty (500/400 for FB)
gd->dig_ping_phas = 40;   // Phase shift for FB mode
```

**XFER Frequency Limits**:
```c
// BPP: 112kHz - 147kHz
pid_set_freq_limit(144000000/112000, 144000000/147000, 144000000/147000);

// EPP: 112kHz - 127.772kHz
pid_set_freq_limit(144000000/112000, 144000000/127772, 144000000/147000);

// MPP: Fixed 360kHz (after negotiation)
pid_set_freq_limit(144000000/360000, 144000000/360000, 144000000/360000);
```

### 4.4 QDT-FOD Thresholds
**Default Air Values** (from `ap->` application parameters):
```c
ap->q_factor_base_value = 8500;   // Baseline Q-factor (0.1 resolution)
ap->fs_base_value = 127770;       // Baseline frequency (Hz)
ap->q_factor_obj_value = -120;    // Object detection threshold
ap->fs_obj_value = -360;          // Frequency drop threshold
ap->q_factor_limL_value = 6000;   // Large metal lower limit
ap->q_factor_limH_value = 12000;  // Large metal upper limit
ap->fs_limL_value = 124000;       // Frequency lower limit
ap->fs_limH_value = 132000;       // Frequency upper limit
```

**Detection Logic** (`wpc_idle.c:198-209`):
```c
if ((q_fact + 0 + rx_may_still_be*120 + q_factor_obj_value < q_factor_base_value) ||
    (f_self > fs_limL_value && f_self + fs_obj_value + rx_may_still_be*360 < fs_base_value)) {
    qdt_have_obj_count++;  // Object detected
} else {
    qdt_try_ping_count++;  // No object, increment ping counter
}
```

---

## 5. Cross-Module Interaction Map

### 5.1 Hardware Layer Dependencies
```
WPC Protocol (app/)
    │
    ├─→ NU103x DDM Driver (fml_nu103x_*)
    │   ├── DDM1: IAVG/EVDM (current/voltage monitoring)
    │   ├── DDM2: VCAP/PHAS (capacitor voltage/phase)
    │   └── Modes: QDT (capacitance), DDM (demodulation), CAP (direct)
    │
    ├─→ ASK Demodulator (fml_ask_*)
    │   ├── fml_ask_enable() - Enable ASK reception
    │   ├── fml_ask_128_ping_cfg() - Configure 128kHz DDM
    │   └── fml_ask_360_ping_cfg() - Configure 360kHz DDM
    │
    ├─→ FSK Modulator (fml_fsk_*)
    │   ├── fml_fsk_param_set() - Set polarity/depth/cycles
    │   ├── fml_fsk_patt_send() - Send ACK/NAK/ND/ATN
    │   └── fml_fsk_data_send() - Send arbitrary data
    │
    ├─→ EPWM (hal_epwm_*)
    │   ├── hal_epwm_pwm_start(perd, duty, phas)
    │   ├── hal_epwm_afd_start() - Auto Frequency Dither (MPP)
    │   └── hal_epwm_pwm_stop()
    │
    ├─→ PID Controller (pid_*)
    │   ├── pid_init() - Reset PID state
    │   ├── pid_cep_handler(cep_val) - Adjust Vout based on CEP
    │   ├── pid_set_volt_limit(hi, mid, lo)
    │   ├── pid_set_freq_limit(hi, mid, lo)
    │   └── pid_set_duty_limit(hi, mid, lo)
    │
    └─→ FOD Modules (pfod_*, qfod_*)
        ├── qfod_nego(ref_q, ref_f) - Negotiate FOD thresholds
        ├── qfod_qdt_cali_process() - QDT calibration mode
        ├── pfod_mpla() - MPP Power Loss Assessment
        └── pfod_dploss_cal() - ΔP-loss calibration
```

### 5.2 External Module Interfaces
```
WPC Protocol ←→ USB-PD / Port Manager
    │
    ├─→ Adapter Detection (gd->adp.adp_type)
    │   ├── EADP_TYPE_PD3P0_50W → 15V/20V capability
    │   ├── EADP_TYPE_POWERBANK_09V → 9V powerbank mode
    │   └── EADP_TYPE_POWERBANK_WIRELESS_ONLY → Sleep after idle
    │
    ├─→ Voltage Control (fml_adp_volt_set())
    │   ├── Called during phase transitions
    │   ├── Ranges: 5V (BPP) → 11V (EPP) → 20V (MPP)
    │   └── Coordinated with PID limits
    │
    └─→ Power Budget (gd->adp.pwr_high)
        ├── pwr_high >= 30 → MPP enabled
        ├── pwr_high >= 20 → EPP enabled
        └── pwr_high < 20 → BPP only
```

### 5.3 OSAL Event System
**Task Registration** (`wpc_task_init()`):
```c
osal_task_handler_reg(WPC_TASK, wpc_task_event_handler);
osal_start_timerEx(WPC_PING_TIMER, t_next_ping, WPC_TASK, WPC_EVT_DIG_PING);
```

**Event Map** (`_wpc.c:372-639`):
| Event | Trigger | Handler | Phase |
|-------|---------|---------|-------|
| `WPC_EVT_DIG_PING` | Periodic timer | `wpc_idle_phase_process()` | IDLE |
| `WPC_EVT_CLOAK_PING` | Cloak timer | `wpc_idle_cloak_phase_process()` | CLOAK |
| `WPC_EVT_HDR_RECVD` | ASK header RX | `wpc_pkt_hdr_handler()` | All |
| `WPC_EVT_PKT_RECVD` | ASK packet RX | `wpc_protocol_sm()` | All |
| `WPC_EVT_STOP_POWER` | Error/EPT | `wpc_stop_power()` | All |
| `WPC_EVT_CEP_TO` | CEP timeout | `wpc_stop_to_idle(CEP_TIMEOUT)` | XFER |
| `WPC_EVT_RPP_TO` | RPP timeout | `wpc_stop_to_idle(RPP_TIMEOUT)` | XFER |
| `WPC_EVT_PCH_TO` | PCH delay end | `pid_cep_handler() + window sampling` | XFER |
| `WPC_EVT_DDM` | DDM sample | `PID_vDDMEventHandler()` | XFER |
| `WPC_EVT_PFOD` | FOD check | `pfod_common()` | XFER |

---

## 6. Expert Implementation Insights

### 6.1 Qi Compliance Critical Paths

#### IOC Test Workarounds
**TPR#1C 6.2.09 Test#23** (`wpc_idle.c:454`):
- Every 5th digital ping: Use IAVG DDM instead of EVDM
- Reason: Better compatibility with large-coil test fixtures
- Comment: `//for IOC test, TPR#1C, 6.2.09 Test#23`

**BPP Load Step Test** (`wpc_xfer.c:170-182`):
- Detect CEP -60/+60 during `pch_t_delay = 0x32`
- Set flags `atl_test_ldstp_bpp_N60/P60` for verification
- Used in IEC 8.4.21/22/23 compliance

**MPP Restricted Mode Re-ping** (`wpc_cnfg.c:236-244`):
- If 128kHz ping receives `mpp_restricted_mode=1` in XID
- Must re-ping with 360kHz before negotiation
- Error: `ESYS_ERR_CODE_IDCFG_PHASE_MPP_RESTRICTED_REP`

#### EPP FOD Requirement (`epp.c:244-252`):
```c
if (qi_version >= 0x13 && nego_flag == 1) {
    if (contract.nego_fod_mask != 0x03) {  // Both FOD/qf and FOD/rf required
        fml_fsk_patt_send(_FSK_NAK);
        power_profile_mode = BPP;  // Fallback to BPP on FOD failure
        // IEC 8.3.48: PTx must reject if RX doesn't send FOD references
    }
}
```

### 6.2 FOD Integration Points

**Three-Layer FOD Strategy**:
1. **QDT-FOD** (IDLE phase): Q-factor/frequency shift detection
2. **IOC BPP FOD** (BPP XFER): RP8 delta monitoring
3. **MPLA** (MPP XFER): Power loss assessment with Vrect/Irect

**QDT-FOD to Protocol Linkage** (`wpc_idle.c:255-261`):
```c
// EPP negotiation FOD check
if (qfod_nego(ref_q, ref_f) > 0) {
    ptx_idle_phase_status = WPC_IDLE_STAT_XER_FOD;
    wpc_stop_to_idle(ESYS_ERR_CODE_NEGO_PHASE_FODS_REF_QVALUE_ERR);
    return;  // Prevent XFER entry
}
```

**MPLA FOD Response** (`wpc_xfer.c:282-313`):
- Return 0 → ACK (no FOD)
- Return 1 → NAK + CEP=-8 (adjust power down)
- Return 2 → ATN (FOD detected, stop transfer)

**NTC Overtemperature Integration** (`wpc_xfer.c:286-294`):
```c
if (tntc_ot_flag == 1 && rx_prect+200)/100 <= nego_cap && tntc_ot_flag_atn == 1) {
    fml_fsk_patt_send(_FSK_ATN);  // Request re-negotiation
    need_renego_cap = 1;
    tntc_ot_flag_atn = 2;  // Mark as re-negotiating
}
```

### 6.3 Authentication Flow Details

**EPP Authentication State Machine** (`epp.c:epp_auth_t`):
```c
typedef enum {
    EPP_Auth_IDLE = 0,
    EPP_Auth_GET_DIGEST,        // Step 1: Send digest (32 bytes)
    EPP_Auth_GET_CERTIFICATE,   // Step 2: Send cert (328 bytes)
    EPP_Auth_GET_CHALLENGE,     // Step 3: Respond to challenge (64 bytes)
    EPP_Auth_DONE,              // Success
    EPP_Auth_ERROR,             // Failure
} EPP_AuthState;
```

**Data Chunking** (`epp.c:is_ADT_valid_packet_type()`):
- ADT headers: 0x16/0x17 (pair 1), 0x26/0x27 (pair 2), ..., 0x76/0x77 (pair 7)
- Each header: 1 byte, payload: max 9 bytes → 10 bytes/packet
- 7 pairs × 2 packets × 9 bytes = 126 bytes per full exchange
- Digest (32B): 4 packets, Cert (328B): 37 packets, Challenge (64B): 8 packets

**Security IC Integration** (`_wpc.c:583-598`):
```c
case WPC_EVT_SE_IC_TBS_AUTH:
    need_atn_cnt = 100;  // Keep ATN active for 100 CEP cycles
    if (ap->auth_seic_type == 1) {
        t91206_get_tbs_auth(array_chall, adt_data_recv_buf + 2);
    } else {
        fm1210_get_tbs_auth(array_chall);  // Default auth IC
    }
    // Signature stored in array_chall[64]
    break;
```

**MPP Data Stream** (`wpc_5_xfer_4_dstrm.c`):
- Different from EPP: uses SDSR (0x38) and SADT (0x26-0x77) packets
- Stream ID: bit 0 = authentication, bit 1 = proprietary
- Flow: `DSR_open → SDSR_ACK → SADT_chunks → DSR_close`

### 6.4 Power Mode Transitions (MPP)

**Mode Transition Matrix**:
```
Current Mode | Request | Result | Notes
-------------|---------|--------|-------
NPM          | HPM     | Switch | Increase to 25W max
HPM          | NPM     | Switch | Decrease to 15W nominal
NPM          | LPM     | Switch | Decrease to <10W light
LPM          | CPM     | Switch | Continuous power (rare)
Any          | Same    | No-op  | MSS status=0 (success)
```

**Transition Handling** (`wpc_xfer.c:586-602`):
```c
case MPP_PRx_PKT_TYP_MSR_13:  // Mode Switch Request
    if (gd->power_mode != msr.main_mode) {
        gd->power_mode = msr.main_mode;
        fsk_pkt.mss.status = 0;  // Success
        printk(" [MSR trans:%d aux:%d]", main_mode, aux);
    } else {
        fsk_pkt.mss.status = 0;  // No change needed
    }
    fml_fsk_data_send(MSS_23, &fsk_pkt);
    break;
```

**Power Adjustment** (`mpp.c:mpp_power_limit_init()`):
- NPM: `tx_infos.max_cap = 150` (15W), `nego_cap = 150`
- HPM: `tx_infos.max_cap = 250` (25W), `nego_cap = 250` (after calibration)
- Power limit reasons: `0=none, 2=FOP, 3=BOP, 4=OT, 6=OC, 7=MAP`

### 6.5 Cloaking Mode Nuances

**Cloak Entry Conditions** (`wpc_xfer.c:497-512`):
```c
// TX-initiated cloaking (rare, for demo)
if (cep_val == 0 && flg_cloak_tx_init == TRUE) {
    if (++cnt_cep0 > 20) {  // 20 consecutive zero CEPs
        flg_cloak_tx_enter = TRUE;
        need_atn_cnt = 100;
        need_cloak_atn = 1;  // Send ATN with cloak reason
    }
}
```

**Cloak Ping Strategy** (`wpc_idle.c:611-713`):
```c
// Detection ping: Short burst to check Q/F
if (cnt_cloak_det_ping >= cloak_det_ping_delay) {
    fml_qdt_detect(&q_fact, &f_self);
    if (idle_qdt_back_to_normal()) {
        // RX removed, exit cloaking
        flg_mode_cloak = FALSE;
        ptx_protocol_phase = WPC_PHASE_IDLE;
    }
    // Otherwise, short 360kHz ping without full startup
}

// Digital ping: Full ping with DDM
if (cnt_cloak_dig_ping >= cloak_dig_ping_delay) {
    wpc_idle_dig_ping_init_360K();
    ptx_protocol_phase = WPC_PHASE_CLOAK;
    osal_start_timerEx(WPC_NEXT_TIMER, T_CLOAK_TIMEOUT);
}
```

**Resume Verification** (`wpc_xfer.c:744-758`):
```c
case MPP_PRx_PKT_TYP_REPORT_58:
    // Anti-hijack: Verify RX ID matches original
    tmp_id = (prx_byteid0 << 16 | prx_byteid1 << 8 | prx_byteid2) >> 3) & 0xFFFFF;
    tmp_base_id = (device_id >> 11) & 0xFFFFF;  // Original RX ID
    if (tmp_base_id != tmp_id) {
        wpc_stop_to_idle(ESYS_ERR_CODE_CLOAK_PHASE_NO_THIS_PKT);
    }
```

### 6.6 Renegotiation Handling

**EPP Renegotiation** (`epp.c:wpc_epp_xfer_phase_protocol_process()`):
```c
case WPC_PRx_PKT_TYP_NEGO_09:
    ptx_protocol_phase = WPC_PHASE_NEGO;
    nego_flag = 2;  // Mark as renegotiation (vs initial nego=1)
    fml_fsk_patt_send(_FSK_ACK);
    osal_start_timerEx(WPC_NEXT_TIMER, T_RENEGOTIATE=814ms);
    break;
```

**MPP Renegotiation** (`wpc_xfer.c:547-553`):
```c
case WPC_PRx_PKT_TYP_NEGO_09:
    osal_stop_timerEx(WPC_CEP_TIMER);  // Suspend power transfer monitoring
    osal_start_timerEx(WPC_NEGO_TIMER, T_RENEGO_TO=1900ms);  // Total timeout
    osal_start_timerEx(WPC_NEXT_TIMER, T_RENEGOTIATE=814ms);  // Packet timeout
    ptx_protocol_phase = WPC_PHASE_NEGO;
    fml_fsk_patt_send(_FSK_ACK);
    break;
```

**Trigger Scenarios**:
1. **NTC Overtemperature**: Request lower `nego_cap` via ECAP packet
2. **FOD False Positive**: Recalibrate FOD thresholds
3. **RX Request**: RX sends NEGO(0x09) packet directly

---

## 7. Quick Reference

### 7.1 Packet Type Quick Lookup
| Header | Name | Phase | Direction | Purpose |
|--------|------|-------|-----------|---------|
| 0x01 | SIG | PING | RX→TX | Signal Strength |
| 0x71 | ID | CNFG | RX→TX | RX Identification |
| 0x81 | XID | CNFG | RX→TX | Extended ID (MPP) |
| 0x06 | PCH | CNFG | RX→TX | Power Control Holdoff |
| 0x51 | CFG | CNFG | RX→TX | Configuration |
| 0x07 | GRQ | NEGO | RX→TX | General Request |
| 0x20 | SRQ | NEGO | RX→TX | Specific Request |
| 0x28 | GET | NEGO/XFER | RX→TX | Get PTx Info |
| 0x22 | FOD | NEGO | RX→TX | FOD Reference (EPP) |
| 0x03 | CEP | XFER | RX→TX | Control Error |
| 0x04 | RP8 | XFER | RX→TX | 8-bit Received Power |
| 0x31 | RP24 | XFER | RX→TX | 24-bit Received Power (EPP) |
| 0x05 | CHS | XFER | RX→TX | Charge Status |
| 0x09 | NEGO | XFER | RX→TX | Renegotiation Request |
| 0x02 | EPT | Any | RX→TX | End Power Transfer |
| 0x19 | XCE | XFER | RX→TX | Extended CEP (MPP) |
| 0x13 | MSR | XFER | RX→TX | Mode Switch Request (MPP) |
| 0x58 | REPORT | XFER | RX→TX | Power Loss Report (MPP) |
| 0x88 | PLA2 | XFER | RX→TX | Power Loss + Vrect/Irect (MPP) |
| 0x18 | CLOAK | XFER/CLOAK | RX→TX | Enter Cloaking |
| 0x15 | DSR | XFER | RX→TX | Data Stream Request (EPP) |
| 0x25 | ADC | XFER | RX→TX | Auth Data Control (EPP) |
| 0x16-0x77 | ADT | XFER | RX↔TX | Auth Data Transport (even/odd) |
| 0x30 | ID | NEGO | TX→RX | PTx ID Response |
| 0x31 | CAP | NEGO | TX→RX | PTx Capabilities |
| 0x8F | ECAP | NEGO | TX→RX | Extended CAP (MPP) |
| 0x3F | INV | NEGO | TX→RX | Input Voltage (MPP) |
| 0x23 | MSS | XFER | TX→RX | Mode Switch Status (MPP) |

### 7.2 Error Code Reference
| Code | Name | Trigger | Recovery |
|------|------|---------|----------|
| 0x01 | PING_PHASE_1ST_PKT_TYPE_ERR | Non-0x01 first packet | Re-ping |
| 0x03 | IDCFG_PHASE_WAIT_NEXT_PKT_TIMEOUT | T_NEXT timeout | Re-ping |
| 0x04 | IDCFG_PHASE_PKT_SEQUENCE_ERR | Out-of-order packets | Re-ping |
| 0x07 | IDCFG_PHASE_CFG_PKT_CNT_ERR | CFG count mismatch | Re-ping |
| 0x0A | IDCFG_PHASE_MPP_RESTRICTED_REP | 128kHz during MPP restricted | 360kHz re-ping |
| 0x0B | XFER_PHASE_CEP_TIMEOUT | No CEP for 1600/2050ms | Stop |
| 0x0C | XFER_PHASE_RPP_TIMEOUT | No RP for 20s/7.9s | Stop |
| 0x10 | XFER_PHASE_POWER_LOSS_FOD | FOD detected | Stop, FOD state |
| 0x11 | NEGO_PHASE_WAIT_NEXT_PKT_TIMEOUT | T_NEGOTIATE timeout | Stop |
| 0x13 | NEGO_PHASE_FODS_REF_QVALUE_ERR | FOD ref check failed | Stop, FOD state |
| 0x15 | RECVD_EPT_PKT | EPT packet received | Decode EPT reason |
| 0x16 | PING_PHASE_WAIT_1ST_PKT_TIMEOUT | T_PING timeout | Re-ping |
| 0x42 | CLOAK_SWITCH | Normal cloak entry | Enter cloak mode |
| 0x55 | MPP_ILLEGAL_PKT | Illegal MPP packet | Stop |
| 0x61 | DIGITAL_REPING | Forced re-ping (IOC test) | Re-ping |

### 7.3 FSK Pattern Codes
| Pattern | Value | Meaning | Usage |
|---------|-------|---------|-------|
| `_FSK_ACK` | 0x00/0xFF | Acknowledge | Accept request/data |
| `_FSK_NAK` | 0x55/0xAA | Negative Acknowledge | Reject/retry |
| `_FSK_N_D` | 0x33/0xCC | Not Defined | Unknown request |
| `_FSK_ATN` | 0xF0/0x0F | Attention | Need polling/data ready |
| `_FSK_MPP` | Special | MPP Mode | MPP restricted mode ACK |
| `_FSK_APP` | Special | Apple Mode | Proprietary mode |

### 7.4 Critical Function Call Chains
**Normal Power Transfer Startup**:
```
wpc_idle_phase_process()
 └→ wpc_idle_dig_ping_init_128K()
     └→ [Phase: PING, Timer: 70ms]
         └→ wpc_ping_phase_process(SIG)
             └→ [Phase: CNFG, Timer: 23ms]
                 └→ wpc_cnfg_phase_process(ID/XID/PCH/CFG)
                     └→ [EPP/MPP: Phase NEGO] OR [BPP: Phase XFER]
                         └→ wpc_nego_phase_process(GRQ/SRQ/FOD)
                             └→ [Phase: XFER, Timers: CEP/RPP]
                                 └→ wpc_xfer_phase_process(CEP/RP/...)
```

**Error Path**:
```
wpc_xfer_phase_process() [or any phase]
 └→ wpc_stop_to_idle(err_code)
     └→ gd->sys_err_code = err_code
         └→ osal_set_event(WPC_EVT_STOP_POWER)
             └→ wpc_stop_power()
                 ├→ hal_epwm_pwm_stop()
                 ├→ fml_nu103x_por_rst()
                 ├→ [Phase: IDLE or CLOAK]
                 └→ osal_start_timerEx(WPC_PING_TIMER, t_next_ping)
```

---

## 8. Reference Material Needs

### 8.1 Essential Specifications
1. **Qi v2.0 Specification** (WPC)
   - Section 5: Power Transmitter Requirements
   - Section 6: Communication Protocol
   - Section 7: Foreign Object Detection
   - Section 8: Compliance Testing (IOC/IOP)

2. **Qi v2.1 MPP Specification** (WPC)
   - Section 4: MPP Power Transfer Protocol
   - Section 5: Power Loss Assessment (MPLA)
   - Section 6: ΔP-loss Calibration
   - Section 7: Cloaking Mode

3. **EPP Specification** (WPC)
   - Section 3: Extended Power Profile
   - Section 4: Authentication Protocol
   - Section 5: Data Stream Protocol

### 8.2 Hardware Dependencies
1. **NU103x DDM Controller**: Datasheet for DDM modes (QDT/DDM/CAP)
2. **EPWM Configuration**: PWM frequency/duty/phase relationship
3. **FSK Modulation**: Bit encoding, polarity, depth settings
4. **PID Tuning**: Voltage/frequency regulation parameters

### 8.3 Testing References
1. **IEC 63028**: Wireless Power Transfer System Test Standard
2. **WPC IOC Tests**: Interoperability Center test procedures
3. **ATL TPR#1C**: Authorized Test Lab Test Plan Reports

### 8.4 Key Packet Format References
Missing detailed packet structures should reference:
- `pkt_type.h`: Complete ASK/FSK packet definitions
- Qi spec Appendix A: Packet Format Tables
- MPP spec Section 3.2: Extended Packet Formats

---

## Quick Debug Commands
```c
// Enable detailed logging
#define EPP_LOG_OUTPUT_ENABLE  // In epp.h

// Key global data inspection
gd->ptx_protocol_phase       // Current phase (0-5)
gd->rx_infos.power_profile_mode  // BPP/EPP/MPP (0/1/2)
gd->rx_infos.qi_version      // 0x12=1.2, 0x20=2.0, 0x21=2.1
gd->sys_err_code             // Last error code
gd->tx_power                 // TX power (mW)
gd->rx_power                 // RX reported power (mW)
gd->rx_infos.cep_val         // Last CEP value (-127 to +127)

// Timer states
WPC_CEP_TIMER, WPC_RPP_TIMER, WPC_NEXT_TIMER, WPC_PING_TIMER

// Protocol state flags
gd->nego_flag                // 0=none, 1=initial, 2=re-nego
gd->ptx_end_nego_event       // 0x01=EPP ended, 0x02=MPP ended
gd->tx_infos.flg_mode_cloak  // TRUE if in cloaking mode
```

---

**Document Version**: 2.0
**Last Updated**: 2026-02-15
**Coverage**: All 31 WPC protocol files analyzed
**Validation**: Cross-referenced with Qi 2.x specification requirements
