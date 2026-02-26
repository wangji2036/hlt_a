# DPDM Protocol Module Knowledge (dpdm + usb_qc + afc_scp + ufcs)

## 1. Module Overview

The DPDM module manages **USB D+/D- line protocols** for the NU17112 powerbank platform. It handles both **source-side** (powerbank outputting) and **sink-side** (powerbank charging) protocol detection and communication, including BC1.2, QC2.0/3.0, AFC (Samsung), SCP/FCP (Huawei), and UFCS (China unified fast charging).

### File Map
| File | Role | Lines |
|------|------|-------|
| `fml/dpdm.c` | Main DPDM task, source/sink event handler, IRQ handlers | ~510 |
| `fml/dpdm.h` | DPDM register definitions (memory-mapped), event defines, enums | ~306 |
| `fml/usb_qc.c` | Sink-side BC1.2 and QC detection, QC2 voltage setting | ~104 |
| `fml/usb_qc.h` | Sink-side register definitions (BC1.2 + QC) | ~165 |
| `lib/afc_scp.c` | AFC/SCP/FCP source-side protocol handler, register table | ~384 |
| `fml/afc_scp.h` | SCP packet structure, register map, protocol constants | ~171 |
| `lib/ufcs.c` | UFCS source-side protocol handler | ~366 |
| `fml/ufcs.h` | UFCS register definitions, message types, header macros | ~285 |

### Dependencies
- **Upstream (called by)**: `port_manager.c` (attach/detach events), IRQ handlers (hardware interrupts)
- **Downstream (calls)**: `buckboost` (set_bus_iv, set_ovp), `pdlib` (disable_usbpd, is_connect), `tcpm` (DPDM_DONE event), `port_manager` (via events)
- **Hardware**: DPDM peripheral at APB+0xC080 (source), APB+0xC000 (sink/QC), APB+0x7000 (UFCS)

---

## 2. Public Interface List

### DPDM Core (fml/dpdm.c)
| Function | Signature | Description |
|----------|-----------|-------------|
| `usb_dpdm_task_init` | `void (void)` | Register task handler, start BC12 timer (100ms), init source hardware |
| `usb_dpdm_task_event_handler` | `void (uint32_t event)` | Main DPDM event dispatcher |
| `usb_dpdm_select` | `void (uint8_t tc_index)` | Switch DPDM MUX to port 0/1/2/off. Deinits sink, enables port control |
| `usb_dpdm_port0_switch` | `void (bool en)` | Switch PA0/PA1 between I2C and DP/DM for port0 |
| `usb_dpdm_autodcp_en` | `void (void)` | Enable all source-side protocol detection: DCP, HVDCP, QC, AFC, SCP, UFCS |

### Sink-side (fml/usb_qc.c)
| Function | Signature | Description |
|----------|-----------|-------------|
| `dpdm_sink_init` | `void (void)` | Initialize BC1.2 sink detection hardware |
| `dpdm_sink_deinit` | `void (void)` | Disable BC1.2 sink detection |
| `qc2_set_volt` | `void (uint16_t qc_volt)` | Set QC2.0 sink voltage request (5000/9000/12000) |

### AFC/SCP Source (lib/afc_scp.c)
| Function | Signature | Description |
|----------|-----------|-------------|
| `dpdm_src_afc_handle` | `void (void)` | Handle AFC RX data: parse command, set voltage/current, trigger output |
| `dpdm_src_scp_handle` | `void (void)` | Handle SCP RX data: dispatch single/multi read/write |
| `fcp_single_read_handle` | `void (void)` | FCP/SCP single byte register read |
| `fcp_single_write_handle` | `void (void)` | FCP/SCP single byte register write with side effects |
| `fcp_multi_read_handle` | `void (void)` | FCP/SCP multi-byte register read |
| `fcp_multi_write_handle` | `void (void)` | FCP/SCP multi-byte register write with side effects |
| `update_scp_reg` | `void (void)` | Update SCP register table with live VBUS/IBUS readings |

### UFCS Source (lib/ufcs.c)
| Function | Signature | Description |
|----------|-----------|-------------|
| `dpdm_ufcs_init` | `void (void)` | Enable UFCS interrupts, set ACK device address |
| `dpdm_ufcs_deinit` | `void (void)` | Disable all UFCS interrupts |
| `ufcs_rx_packet_handle` | `void (void)` | Parse received UFCS packet, dispatch to ctrl/data handler |
| `ufcs_exit_handle` | `void (void)` | Exit UFCS mode: set 5V/3A, soft reset DPDM |
| `ufcs_psread_handle` | `void (void)` | Send UFCS POWER_READY message |

### Key Globals
| Variable | Type | Description |
|----------|------|-------------|
| `bc12_type` | `uint8_t` | Detected sink-side charger type (BC1P2_SDP..BC1P2_QC12V) |
| `dpdm_map` | `uint8_t` | Current DPDM MUX mapping (port index or 0xFF=off) |
| `qc_volt` | `uint16_t` | Current QC voltage (5000-12000 mV) |
| `scp_vout` | `uint16_t` | SCP/AFC requested output voltage (mV) |
| `scp_iout` | `uint16_t` | SCP/AFC requested output current (mA) |
| `scp_packet` | `union scp_packet_t` | Received SCP packet buffer |
| `scp_tx` | `union scp_packet_t` | Transmitted SCP packet buffer |
| `SCP_REG[256]` | `uint8_t[]` | SCP virtual register table |
| `is_enter_dpdm_prot` | `bool` | Flag: DPDM protocol is active (QC/AFC/SCP entered) |
| `dpdm_snk_support` | `uint8_t` | Sink-side protocol support flags |
| `ufcs_msg_id` | `uint8_t` | UFCS message ID counter |

---

## 3. Internal Logic Details

### 3.1 DPDM MUX Architecture
The hardware has a single DPDM engine with a MUX to select which physical port connects:
```
MUX_PORT_NUM mapping:
  0 -> OFF
  1 -> Type-C Port 0 (tc_index=0)
  2 -> USB-A (tc_index=2)
  3 -> Type-C Port 1 (tc_index=1)
```
Only ONE port can use DPDM protocols at a time. Port selection via `usb_dpdm_select()`.

### 3.2 Source-Side Protocol Detection Flow
```
SRC_ATTACHED event
  -> usb_dpdm_autodcp_en()
     Enable: AUTO_DCP, HVDCP_DET, QC_SRC_DET(mode=2), AFC_SRC_DET, SCP_SRC_DET, UFCS_SRC_DET
     Enable: 900K pull-down
     Unmask: DCP, HVDCP, QC fixed/continuous/pulse, AFC RX, SCP RX interrupts

  Sink device plugs charger D+/D-:
    -> DCP detected (IRQ) -> ENTER_DCP event -> log "enter dcp"
    -> HVDCP handshake (IRQ) -> ENTER_HVDCP event -> log "hvdcp"
    -> QC2 fixed voltage request (IRQ) -> QC_FIXED_5V/9V/12V event
    -> QC3 pulse (IRQ) -> QC_PULSE_INC/DEC event (200mV steps)
    -> AFC data (IRQ) -> AFC_RX_DATA -> dpdm_src_afc_handle()
    -> SCP data (IRQ) -> SCP_RX_DATA -> dpdm_src_scp_handle()
    -> UFCS packet (IRQ) -> UFCS_RX_PACKET -> ufcs_rx_packet_handle()
```

### 3.3 Source-Side QC2/QC3 Handling
**QC2 Fixed Voltage**:
```
QC_FIXED_5V/9V/12V/20V event:
  1. Check pdlib_is_connect() -- if PD active, reject QC ("pd has work, qc should not work")
  2. Read QC_SRC_STAT to get actual mode
  3. Set qc_volt (5000/9000/12000, 20V not supported)
  4. Calculate current: qc_current = 18W / qc_volt, cap at 3000mA
  5. Set VBUS via hal_tcpc_pd_set_bus_iv(0, qc_volt, qc_current+300, 0, 10)
```

**QC3 Pulse** (in IRQ): qc_volt +/- 200mV, clamped 5000-12000mV

### 3.4 Source-Side AFC Handling
```
AFC RX commands:
  0x01 (RESET) -> 5V
  0x08 (5V)    -> TX echo 0x08, set 5V, 3.3A
  0x46 (9V)    -> TX echo 0x46, set 9V, 2.4A
  0x79 (12V)   -> TX echo 0x79, set 12V, 1.8A
```
All AFC changes trigger `DPDM_EVT_AFC_SCP_OUT` -> `hal_tcpc_pd_set_bus_iv(0, scp_vout, scp_iout, 0, 10)`.

### 3.5 Source-Side SCP/FCP Register Model
SCP implements a **virtual register table** (`SCP_REG[256]`). The sink reads/writes these registers via D+/D- signaling.

**Key SCP registers**:
| Register | Address | Function |
|----------|---------|----------|
| FCP_REG_OUTPUT_CTRL | 0x2B | Write bit 0 = apply VOUT_CONFIG |
| FCP_REG_VOUT_CONFIG | 0x2C | FCP voltage = value * 100mV |
| SCP_REG_VSET_H/L | 0xB8/0xB9 | SCP voltage (16-bit mV) |
| SCP_REG_CTRL_BYTE0 | 0xA0 | Control: bit 6=0 -> reset to 5V |
| SCP_REG_SPEC_FUN2 | 0xCF | Special function: reset to 5V |
| SCP_REG_READ_VOUT_H/L | 0xA8/0xA9 | Live VBUS voltage (updated on read) |
| SCP_REG_SREAD_IOUT | 0xC9 | Live IBUS / 50 |

**FCP vs SCP discrimination**: If `lib_para.scp_source_support == 0`, registers >= 0x7E return NACK (FCP-only mode).

**Voltage/current calculation on write**:
```
FCP: scp_vout = VOUT_CONFIG * 100
SCP: scp_vout = (VSET_H << 8) | VSET_L
Both: scp_iout = 24W / scp_vout, cap at 2400mA
      If 5V: scp_iout = 3300mA
      If voltage > 10V: clamp to 10V
```

**Side effects on register write**: Both FCP and SCP voltage changes trigger `pdlib_disable_usbpd()` (disables PD negotiation to prevent conflict).

### 3.6 Sink-Side Protocol Detection Flow
```
SNK_ATTACHED -> dpdm_sink_init()
  -> BC1.2 detection runs in hardware
  -> SNK_BC12DONE:
     BC1P2_TYPE: 0x02=CDP, 0x03=DCP, 0x06=Apple, else=SDP
     If DCP -> start HVDCP detection (25ms delay)
  -> SNK_HVDCP_START:
     Enable QC sink mode (QC_MODE=0x03, 900K PD)
  -> SNK_HVDCP_DONE:
     If snk_5v_only==0 && no PD: try QC voltage
     Else: report as HVDCP, notify TCPM
  -> SNK_QC_START:
     Set OVP to 20V, set QC to 12V, wait 200ms
  -> SNK_QC12V_DONE:
     If VBUS >= 10500mV: QC 12V supported -> bc12_type = BC1P2_QC12V
     Else: try 9V, wait 200ms
  -> SNK_QC_DONE:
     If VBUS >= 7500mV: QC 9V supported -> bc12_type = BC1P2_QC9V
     Reset to 5V, notify TCPM
```

### 3.7 UFCS Source-Side Protocol
**UFCS message structure**: Header(2B) + Command(1B) + Length(1B) + Data(variable)

**Header format**: `UFCS_HEADER(type, rev, id, attr)` where:
- type: 0=ctrl, 1=data, 2=user
- rev: version (0x01)
- id: message ID (4-bit, incrementing)
- attr: device attribute (0x02 = charger)

**Supported ctrl messages**:
| Command | Response |
|---------|----------|
| GET_OUTPUT_CAP | OUTPUT_CAPS: min 5V, max 11V, max 2A |
| GET_SOURCEINFO | SOURCEINFO: live VBUS/IBUS readings |
| GET_DEVICEINFO | DEVICE_INFO: all zeros |
| GET_ERRINFO | ERROR_INFO: all zeros |
| DETECT_CABLEINFO | REFUSE (not supported) |
| EXIT_MODE | Set 5V/3A, soft reset DPDM |

**Supported data messages**:
| Command | Response |
|---------|----------|
| RESQT (voltage request) | ACCEPT if within 5-11V range, else REFUSE |
| CONFIG_WATCHDOG | ACCEPT |
| VERIFY_REQUEST | REFUSE (not supported) |

**UFCS voltage range**: 5000-11000mV (CONFIG_UFCS_MIN/MAX_VOLTAGE), max 2000mA.

### 3.8 IRQ Handlers
| Handler | Hardware Source | Action |
|---------|---------------|--------|
| `DCP_HVDCP_IRQHandler` | DPDM HVDCP_FLAG | Set DCP/HVDCP events |
| `QC_SRC_IRQHandler` | DPDM QC_SRC_FLAG | Set QC fixed/continuous/pulse events, update qc_volt |
| `AFC_SCP_SRC_IRQHandler` | DPDM AFC_INT_FLAG | Handle AFC/SCP RX in ISR context, set events |
| `DPDM_SINK_IRQHandler` | DPDM_QC_SINK | BC1.2 done, HVDCP done/fail events |
| `UFCS_IRQHandler` | DPDM_UFCS INT_FLAG | RX buffer fill, TX buffer empty, data ready, hard reset |

**Important**: AFC/SCP handlers call `dpdm_src_afc_handle()` and `dpdm_src_scp_handle()` **directly in ISR context** for low-latency response. The protocol response TX is set up in ISR, then event is posted for logging/further processing.

---

## 4. Key Values Table

| Parameter | Value | Unit | Notes |
|-----------|-------|------|-------|
| BC12 timer period | 100 | ms | |
| QC2 max voltage (source) | 12000 | mV | 20V not supported |
| QC3 step size | 200 | mV | Per pulse |
| QC power limit (source) | 18000 | mW | 18W |
| QC max current (source) | 3000 | mA | Cap |
| QC extra current margin | 300 | mA | Added to calculated limit |
| AFC 5V current | 3300 | mA | |
| AFC 9V current | 2400 | mA | |
| AFC 12V current | 1800 | mA | |
| SCP max voltage | 10000 | mV | Capped in code |
| SCP max power | 24000 | mW | 24W for current calculation |
| SCP max current | 2400 | mA | Cap |
| SCP 5V current | 3300 | mA | Special case |
| FCP max power (register) | 36 | W | Advertised |
| FCP discrete voltages | 5V, 9V, 12V | | Advertised |
| UFCS max voltage | 11000 | mV | CONFIG_UFCS_MAX_VOLTAGE |
| UFCS min voltage | 5000 | mV | CONFIG_UFCS_MIN_VOLTAGE |
| UFCS max current | 2000 | mA | CONFIG_UFCS_MAX_CURRENT |
| HVDCP detect delay (sink) | 25 | ms | After DCP detected |
| QC sink test 12V timeout | 200 | ms | |
| QC sink test 9V timeout | 200 | ms | |
| QC 12V validation threshold | 10500 | mV | VBUS >= this means 12V works |
| QC 9V validation threshold | 7500 | mV | VBUS >= this means 9V works |
| DPDM DP_FAIL_DEG | 3 | | Deglitch setting |

### SCP Advertised Capabilities
| Register | Value | Meaning |
|----------|-------|---------|
| MAX_PWR | 0x98 | 40W |
| CNT_PWR | 0x9E | 30W |
| MIN_VOUT | 0xB7 | 5500mV |
| MAX_VOUT | 0xCA | 10000mV |
| MIN_IOUT | 0x5E | 300mA |
| MAX_IOUT | 0x94 | 2000mA |
| VSTEP | 0x14 | 20mV |
| ISTEP | 0x64 | 100mA |

### BC1.2 Type Enum
```c
BC1P2_SDP = 0,    // Standard Downstream Port
BC1P2_CDP = 1,    // Charging Downstream Port
BC1P2_DCP = 2,    // Dedicated Charging Port
BC1P2_APPLE = 3,  // Apple 2.4A/2.1A
BC1P2_HVDCP = 4,  // High Voltage DCP
BC1P2_QC12V = 5,  // QC charger supporting 12V
BC1P2_QC9V = 6,   // QC charger supporting 9V (but not 12V)
```

---

## 5. Interaction Map

### Outputs (dpdm -> other modules)
| Target | Interface | Trigger | Data |
|--------|-----------|---------|------|
| buckboost | `hal_tcpc_pd_set_bus_iv()` | QC/AFC/SCP/UFCS voltage change | Voltage, current, wait, delay |
| buckboost | `buckboost_ops.set_ovp()` | QC sink start (12V test) | 20000mV |
| pdlib | `pdlib_disable_usbpd()` | SCP/FCP voltage change | Disables PD to avoid conflict |
| pdlib | `pdlib_is_connect()` | QC source check | Prevents QC when PD active |
| tcpm | `TCPM_EVT_DPDM_DONE` | Sink-side detection complete | bc12_type set |
| port_manager | (via bc12_type global) | After detection | Charger capability info |

### Inputs (other modules -> dpdm)
| Source | Interface | Trigger | Data |
|--------|-----------|---------|------|
| port_manager | `usb_dpdm_select()` | Port connect/disconnect | Port index |
| port_manager | `DPDM_EVT_SRC_ATTACHED` / `_UNATTACHED` | Source port events | Attach/detach |
| port_manager | `DPDM_EVT_SNK_ATTACHED` / `_UNATTACHED` | Sink port events | Attach/detach |
| port_manager | `qc2_set_volt()` | Sink QC voltage negotiation | Voltage |
| IRQ handlers | osal_set_event | Hardware interrupts | Protocol events |

---

## 6. Expert Insights

### Design Intent
- **Single DPDM engine with MUX**: Hardware limitation -- only one port can run D+/D- protocols at a time. The MUX selection is critical for multi-port designs
- **ISR-context protocol handling**: AFC/SCP responses are constructed in ISR for timing compliance (SCP has strict response time requirements, MAX_RSPTIME = 20ms)
- **Virtual register model for SCP**: Emulates a register-based slave device on D+/D-, making the powerbank appear as a Huawei charger
- **Progressive sink detection**: BC1.2 -> HVDCP -> QC 12V -> QC 9V, progressively testing higher voltages
- **CONFIG_USE_USB_XGB**: When enabled, port1 DPDM routing changes (commented out PORT3_CTRL settings)

### Risk Areas / Potential Bugs
1. **QC voltage in ISR**: `qc_volt` is modified in `QC_SRC_IRQHandler` (qc_volt += 200) and read in task context. No atomic protection -- could cause torn reads on 16-bit value
2. **SCP register table buffer overflow**: `fcp_multi_read_handle` copies from `SCP_REG[scp_packet.bytes.msg_1]` with length from `scp_packet.bytes.msg_2`. If msg_1 + msg_2 > 256, this reads beyond the array. The `copy_len = min(msg_2, 10)` partially mitigates but `msg_1` is not bounds-checked
3. **dpdm_source_init direct register access**: Uses hardcoded memory addresses (0x4000c0bc) for override registers -- fragile if chip revision changes
4. **UFCS buffer overflow**: `ufcs_rx_buffer[64]` filled in ISR with 4-byte chunks. If `ufcs_rx_index` exceeds 60 before DATA_READY flag, the next write goes out of bounds
5. **PD/QC conflict**: The check `pdlib_is_connect()` only prevents QC when PD is fully connected. During PD negotiation, QC events could still fire and change VBUS
6. **AFC/SCP in ISR**: `dpdm_src_afc_handle()` and `dpdm_src_scp_handle()` are called from ISR. If these take too long (especially SCP multi-byte operations), they could cause interrupt latency issues
7. **Sink QC detection always tries 12V first**: This causes a brief 12V surge even if the charger doesn't support it. The 200ms timeout allows the voltage to settle, but some chargers may react poorly

### Debugging Guide
- **"dpdm_map=%d"**: Shows which port has DPDM ownership
- **"enter dcp"**: Sink device detected as DCP
- **"hvdcp"**: HVDCP handshake completed (source side)
- **"qc2 v= %d i= %d"**: QC2 voltage request processed
- **"qc3 v= %d i= %d"**: QC3 continuous mode voltage
- **"pd has work, qc should not work"**: PD took priority over QC
- **"SCP RX: 0x.."**: SCP packet received (bytes dumped)
- **"SCP TX: 0x.."**: SCP response sent
- **"BC12 bc12_type=0x%x"**: BC1.2 detection result
- **"hvdcp start"**: Sink starting HVDCP detection
- **"Set Qc 12V=%d" / "Set Qc 9V=%d"**: Sink QC voltage test result
- **"UFSC V=%d I=%d"**: UFCS voltage request received
- **"UFCS HARDRESET"**: UFCS hard reset received
- **"UFCS EXIT"**: UFCS exit mode

### Customization Hotspots
1. **SCP_REG table** (afc_scp.c): Modify advertised capabilities (power, voltage range, current, vendor ID, etc.)
2. **QC power limit**: Hardcoded 18W in QC handler, change for different designs
3. **AFC voltage/current mapping**: Hardcoded in `dpdm_src_afc_handle()`
4. **UFCS voltage/current range**: CONFIG_UFCS_MAX/MIN_VOLTAGE, CONFIG_UFCS_MAX_CURRENT
5. **Sink QC test sequence**: Currently tests 12V then 9V; could skip 12V test if not needed
6. **Protocol enables**: CONFIG_AFC/FCP/SCP_SOURCE_SUPPORT, CONFIG_UFCS_SOURCE_SUPPORT in config.h
7. **SCP max power formula**: `24000000 / scp_vout` with 2400mA cap -- adjust for different power levels

---

## 7. Quick Reference

### DPDM Hardware Register Map
```
APB + 0xC000: TS_DPDM_QC_SINK (BC1.2 + QC sink)
  0xC000: BC1P2_STAT     - BC1.2 detection result
  0xC004: BC1P2_INT_FLAG - BC1.2 interrupt flags
  0xC008: BC1P2_INTMSK_CTRL - BC1.2 control + masks
  0xC00C: QC_INT_STAT    - QC interrupt status
  0xC010: QC_INT_FLAG    - QC interrupt flags
  0xC014: QC_INTMSK_CTRL - QC mode control + masks
  0xC018: DPDM_COT_PULSE - Continuous pulse config
  0xC01C: DPDM_MANUAL    - Manual D+/D- control

APB + 0xC080: TS_DPDM (Source-side)
  0xC080: SOURCE_CTRL    - Protocol enables + MUX
  0xC084: SOURCE_STAT    - Source status
  0xC088: HVDCP_CTRL     - HVDCP control + masks
  0xC08C: HVDCP_FLAG     - DCP/HVDCP interrupt flags
  0xC090: QC_SRC_CTRL    - QC source control
  0xC094: QC_SRC_FLAG    - QC source status + interrupts
  0xC098: AFC_CTRL       - AFC/SCP control + masks
  0xC09C: AFC_NOTIF      - AFC notification
  0xC0A0: AFC_INT_FLAG   - AFC/SCP interrupt flags
  0xC0A4-AC: AFC_RX_0/1/2 - RX buffer (12 bytes)
  0xC0B0-B8: AFC_TX_0/1/2 - TX buffer (12 bytes)

APB + 0x7000: TS_UFCS_SOURCE_SINK
  0x7000: SOURCE_CTRL    - UFCS control
  0x7004: VERSION_MASK   - Version + interrupt masks
  0x7008: INT_FLAG       - Interrupt flags
  0x700C: INT_STAT       - Interrupt status
  0x7010: TX_LENGTH      - TX packet length
  0x7014: TX_BUFFER      - TX 4-byte FIFO
  0x7018: RX_LENGTH      - RX packet length
  0x701C: RX_BUFFER      - RX 4-byte FIFO
```

### SCP Packet Structure
```c
union scp_packet_t {
    struct {
        uint8_t msg_len;  // Byte count
        uint8_t msg_0;    // Command / ACK
        uint8_t msg_1;    // Register address
        uint8_t msg_2;    // Data / Length
        uint8_t msg_3-10; // Additional data
    } bytes;
    uint32_t words[3];    // 12-byte aligned
};
```

### Protocol Priority
When PD is active, QC is rejected. AFC/SCP/UFCS all call `pdlib_disable_usbpd()` when changing voltage, ensuring no PD conflict. Only ONE fast-charge protocol can be active at a time on the same port.

---

## 8. Reference Material Needs

| Item | Priority | Reason |
|------|----------|--------|
| NU17112 DPDM register detailed docs | HIGH | Understanding bit fields, timing, deglitch settings |
| Qualcomm QC2.0/3.0 specification | MEDIUM | Validating pulse handling, voltage steps |
| Samsung AFC specification | MEDIUM | Understanding AFC command encoding |
| Huawei SCP/FCP specification | HIGH | Validating register table, command format, timing requirements |
| UFCS specification (GB/T) | MEDIUM | Understanding message format, required responses |
| lib_para structure definition | HIGH | Understanding feature enable flags (afc_source_support, scp_source_support, fcp_source_support) |
| pdlib API documentation | MEDIUM | Understanding pdlib_is_connect, pdlib_disable_usbpd interactions |
| tcpm.h full contents | LOW | Understanding TCPM_EVT_DPDM_DONE and other event definitions |
