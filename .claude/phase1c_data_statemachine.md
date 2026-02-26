# Phase 1C: Data Structures & State Machines Analysis

## 1. Key Data Structures

### 1.1 Global Data Centers

The system has 5 primary global data objects that form the runtime state of the entire powerbank:

| Global Variable | Type | Declared | Description |
|---|---|---|---|
| `gd` | `volatile struct gd_t *` | `fml/g_data.h:551` | **Central runtime data** - WPC protocol state, sensor readings, protection flags, RX/TX info |
| `ap` | `volatile struct ap_t *` | `fml/g_data.h:550` | **Configuration parameters** - stored in Flash (0x1600~), loaded to RAM at init. Thresholds, PID limits, Q-factor params |
| `g_buckboost` | `struct buckboost_s` | `power/buckboost.h:149` | **Buck-boost converter state** - ADC values (vbat/ibat/vbus), work mode, gate enables, output targets |
| `g_port` | `struct port_infos` | `app/port_manager.h:94` | **Port manager state** - port_state[4] (NONE/SOURCE/SINK), event flags, charge limits, adapter power |
| `g_tc[2]` | `struct tc_s[TYPEC_PORT_MAX_N]` | `lib/typec.c:16` | **Type-C state** - per-port TC state machine (state/substate), CC status, polarity, debounce timers |

Additional global objects:
| Global Variable | Type | Declared | Description |
|---|---|---|---|
| `g_tcpc` | `struct tcpc_s` | `lib/pd_tc.c:20` | PD Type-C port controller - port map, data_role, pwr_role |
| `g_usb_pd_s` | `struct usb_pd_s` | `lib/usb_pd.c:27` | USB PD protocol engine state - msg IDs, PDOs, negotiation revision |
| `g_pd_packet` | `struct usb_pd_pkt_t` | `lib/usb_pd.c:28` | Current PD packet being processed |
| `g_eLedState` | `TE_LED_STATE` | `app/led.h:42` | LED display state machine current state |
| `lib_para` | `struct lib_para_sts` | `fml/g_data.h:547` | Library feature flags - typec_a/b_support, ufcs/afc/fcp/scp_source_support |
| `g_rbuf[400]` | `uint8_t[]` | `fml/fm1210.c:24` | FM1210 authentication IC receive buffer |
| `buckboost_ops` | `struct buckboost_operations` | `power/buckboost.h:150` | Function pointer table for buck-boost HAL abstraction |
| `app_reg_buff[64]` | `uint8_t[]` | `app/gui.h:35` | I2C GUI register buffer for external host communication |

### 1.2 Core Struct Details

#### 1.2.1 `struct gd_t` (Global Dynamic Data) - `fml/g_data.h:254-538`

The primary runtime data structure. Fields grouped by function:

**Wireless Power (WPC) Sensing:**
- `vbus`, `vpwr`, `isns`, `ipga` - real-time ADC readings (voltage, power, current)
- `vpwr_avg`, `isns_avg`, `isns_pre` - averaged/previous sensor values
- `icol_max`, `icol_rms` - coil current metrics
- `vctx_pp` - VCAP peak-to-peak
- `k_est` - coupling coefficient estimate
- `tx_power`, `rx_power`, `rx_prect` - power calculations

**WPC Protocol Phase:**
- `ptx_protocol_phase` (uint8_t) - **WPC main protocol phase** (IDLE/PING/CNFG/NEGO/XFER/CLOAK)
- `ptx_idle_phase_status` (uint8_t) - **WPC idle sub-phase** (STANDBY/XER_COM/XER_FOD/QDT_FOD/LAR_MET/EPT_ERR/EPT_RES/EPT_REP/CLOAKING/QDT_CALI)
- `ptx_end_nego_event` - end-of-negotiation event flags
- `sys_err_code` - system error code (TE_SYS_ERR_CODE enum)
- `wpc_pkt` - current ASK packet being processed
- `fsk_cfg` / `fsk_silence` - FSK modulation configuration

**WPC TX Info (`tx_infos` sub-struct):**
- `ping_type` - qdt_ping/dig_ping/det_ping
- `dig_ping_type` - 128K_HB / 360K_FB
- `q_fact`, `f_self`, `q_fact_air`, `f_self_air` - Q-factor and self-frequency for FOD
- `flg_mode_cloak`, `state_exit_cloak` - cloak mode flags
- `power_mode_trans_atn/eptr/cloak` - power mode transition flags
- `max_cap`, `nego_cap`, `need_renego_cap` - capacity negotiation
- `rx_status` - 0: RX detached, 1: RX attached
- `master_adaptor_cap` - 1: BPP 5W, 2: MPP 15W
- `ept_attempt_cnt`, `reping_cnt` - retry counters

**WPC RX Info (`rx_infos` sub-struct):**
- `power_profile_mode` - BPP(0)/EPP(1)/MPP(2)
- `cep_val`, `cep_pre`, `cep_cnt` - Control Error Packet values
- `rpp`, `rpp_tick`, `rpp_rsp_type` - Received Power Packet
- `max_power`, `guaranteed_power` - power negotiation results
- `qi_version`, `epp_mode`, `fsk_param` - protocol parameters
- `device_id` - RX device identifier

**PID Control:**
- `pid_volt`, `pid_perd`, `pid_duty`, `pid_phas` - current PID output values
- `dig_ping_volt/perd/duty/phas` - digital ping parameters
- `pid_limit` sub-struct - hi/mi/lo limits for volt/perd/duty/phas

**Protection Status (`prot_sts` bitfield):**
- `tntc_otp_flag`, `tntc_utp_flag` - NTC over/under-temp
- `tdie_otp_flag`, `tdie_utp_flag` - die over/under-temp
- `isns_ocp_flag` - over-current
- `vbus_ovp_flag`, `vbus_uvp_flag`, `vbus_dpl_flag` - VBUS protection
- `vpwr_ovp_flag`, `pout_opp_flag` - power protection
- `q_fod_flag`, `xfer_fod_flag` - FOD detection

**Battery/System State:**
- `soc_flag`, `bat_dead_flag`, `bat_dead_flag_with_snk0/1` - battery status
- `Bat_RTC_Seconds`, `Bat_RTC_Milliseconds` - runtime clock
- `SOC_RawSOC_mpct`, `SOC_SleepTime_s` - SOC tracking
- `Battery_cycle_count`, `Battery_charger_cnt`, `Bat_Rdc`, `Bat_SoH` - battery health
- `tc0_lighting_mode`, `tc1_lighting_mode` - Type-C light load mode
- `ship_mode_cnt` - ship mode counter

**Writer/Reader analysis:**
- **Written by:** app task (app.c), WPC task (_wpc.c, wpc_*.c), buckboost task (buckboost.c), TCPM (tcpm.c), port_manager, sleep.c, ADC ISR (via badc.c), FSK ISR
- **Read by:** all tasks above + pid.c, pfod.c, epp.c, led.c, qdt.c, typec.c, usb_pd.c
- **Protection:** None (no mutex/spinlock). Accessed from multiple tasks and ISRs without protection. OSAL task scheduling is cooperative (non-preemptive), so task-level access is safe. ISR writes to sensor fields (isns, vbus) may race with task reads.

#### 1.2.2 `struct ap_t` (Application Parameters) - `fml/g_data.h:129-252`

Configuration data loaded from Flash at boot. Key parameter groups:

- `app_info_0..7` - 8 bytes of application info at 0x2000
- `mpp_dither_en`, `auth_seic_type` - feature config
- `ptmc`, `t_next_ping` - timing parameters
- Protection thresholds/hysteresis: `tdie_otp_thd/hys`, `tntc_otp_thd/hys`, `isns_ocp_thd/hys`, `vbus_ovp_thd/hys`, etc.
- Protection disable flags: `tntc_otp_dis`, `tdie_otp_dis`, etc.
- PID limits: `pid_volt_lim_hi/mi/lo`, `pid_perd_lim_*`, `pid_duty_lim_*`, `pid_phas_lim_*`
- Digital ping parameters per voltage: `dig_ping_volt_5v/6v/9v/11v`, `dig_ping_perd_*`, etc.
- Q-factor/FOD: `q_factor_base/reco/limH/limL/obj/stable_value`, `fs_base/reco/limH/limL/obj/stable_value`
- FOD config: `pin_max_cnt`, `pin_fod_dis/cnt`, `rpp_fod_dis/cnt`
- When `CONFIG_NEW_CCC_LOG_ENABLE`: `exception_cache` (ExceptionCache_t), `record_storage` (BatteryRecordStorage_t)

#### 1.2.3 `struct buckboost_s` - `power/buckboost.h:22-67`

- `woke_mode` (enum buckboost_mode) - SHUTDOWN(0) / CHARGER(1) / DISCHG(2)
- Gate enables: `set_typeca_gate_en`, `set_typecb_gate_en`, `set_usb_a_gate_en`
- Output targets: `buckboost_out_voltage`, `buckboost_out_current`, `buckboost_out_current_actual`
- Charger params: `buckboost_chager_current`, `chager_ibus_limit/value`, `chager_ibat_limit`
- ADC readings: `adc_ibus`, `adc_ibat`, `adc_vbat`, `adc_tbat1/2`, `adc_vbus`
- Status: `regulator_state`, `ibus_cc_flag`, `protect_status`, `bat_full_flag`
- NU6801-specific: `adc_iac1/2`, `charging_stat`, `vsnkdisconnect_flag`

#### 1.2.4 `struct port_infos` - `app/port_manager.h:62-79`

- `port_state[4]` - PORT_STATE_NONE(0) / PORT_STATE_SOURCE(1) / PORT_STATE_SINK(2) for Port0(TypeC-A), Port1(TypeC-B), Port2(USB-A), Port3(WPC)
- `incharge_port` - which port is the charging input
- `inhandle_port` - which port is currently being handled by port manager
- `port_event` - bitmask event flags (PORT0_EVENT_TRY_CONNECT, PORT0_EVENT_UNCONNECT, etc.)
- `state` (enum port_state_e) - PORT_IDLE_OR_READY(0) / PORT_INHANDLING(1)
- `ibat_limit`, `ibus_limit`, `prot_ibus` - current limits
- `snk_set_volt` - requested sink voltage
- `adpater_power` - adapter power in mW

#### 1.2.5 `struct tc_s` (Type-C State) - `usbpd/typec.h:24-39`

- `usb_tc_state` (enum usb_tc_state_e) - current TC state
- `usb_tc_substate` (enum usb_pd_substate_e) - enter_state or exit_state
- `polarity` - CC1 or CC2
- `tc_index` - port A or B
- `is_deadbattery`, `is_in_prswap` - flags
- `cc1`, `cc2` (enum tc_cc_status) - CC line status
- `tc_timer_cnt` - debounce timer
- `try_snk_cnt`, `try_src_cnt` - DRP try counters

#### 1.2.6 `struct usb_pd_s` - `usbpd/usb_pd.h:160-190`

- `rx_sop_msgid`, `tx_sop_msgid` - PD message IDs
- `communitcate_capable`, `explicit_contract` - PD contract state
- `hardreset_counter`, `caps_counter` - retry counters
- `nego_revision` - PD revision (2.0/3.0)
- `is_in_pps`, `pe_prl_busy`, `in_bist_mode` - mode flags
- `src_source_pdo[7]`, `snk_rx_source_cap[7]`, `snk_sink_pdo[7]` - PDO arrays
- `supply_voltage`, `supply_current` - negotiated supply
- `pe_tran_cb_type` - transmit type (hard reset, bist, etc.)

### 1.3 Key Enumerations

| Enum | File:Line | Values | Purpose |
|---|---|---|---|
| `enum buckboost_mode` | `power/buckboost.h:15-20` | SHUTDOWN(0), CHARGER(1), DISCHG(2) | Buck-boost operating mode |
| `enum ptx_protocol_phase_t` | `app/_wpc.h:150-157` | IDLE(0), PING(1), CNFG(2), NEGO(3), XFER(4), CLOAK(5) | WPC Qi protocol phase |
| `enum ptx_idle_phase_state_t` | `app/_wpc.h:159-170` | STANDBY(0)..QDT_CALI(9) | WPC idle sub-state |
| `enum power_profile_mode_t` | `app/_wpc.h:116-121` | BPP(0), EPP(1), MPP(2) | Qi power profile |
| `TE_SYS_ERR_CODE` | `app/_wpc.h:4-60` | 0x00-0x70 (60+ codes) | System error codes |
| `enum EPT_CODE` | `app/_wpc.h:73-87` | Unknown..RePing (13 codes) | End Power Transfer codes |
| `enum usb_tc_state_e` | `fml/tcpm.h:9-31` | TC_Disable(0)..TC_STATE_MAX(15) | Type-C state machine states |
| `enum tc_cc_status` | `fml/tcpm.h:33-42` | OPEN, RA, RD, RP_DEF, RP_1_5, RP_3_0 | CC line status |
| `TE_LED_STATE` | `app/led.h:34-41` | NULL(0), POWERON(1), STANDBY(2), CHARGING(3), CHARGED(4), ERROR(5) | LED display state |
| `enum port_state_e` | `app/port_manager.h:36-40` | IDLE_OR_READY(0), INHANDLING(1) | Port manager state |
| `enum dpdm_state_e` | `fml/dpdm.h:5-14` | OFF, DCP, HVDCP_IDLE, QC, AFC, SCP, UFCS | DP/DM protocol state |
| `enum qc_state_e` | `fml/dpdm.h:16-24` | NOT_MODE(0), 5V, 9V, 12V, 20V, CONTINUOUS | QC voltage mode |
| `enum wpc_work_mode` | `fml/tcpm.h:117-124` | FIX5V(0), BOOST(1), ADP_FIX(2), PD_PPS(3), DISABLE(4) | WPC work mode when co-existing with wired |
| `EPP_AuthState` | `app/epp.h:298-306` | IDLE(0), GET_DIGEST, GET_CERTIFICATE, GET_CHALLENGE, DONE, ERROR, ERROR_VERSION | Qi authentication state |
| `EPP_DatastreamMode` | `app/epp.h:275-280` | IDLE(0), RX, TX, CLOSED | Data stream mode |
| `enum usb_pd_state_e` | `usbpd/usb_pd.h:44-86` | PE_SNK_* (0-15), PE_SRC_* (16-35+) | USB PD Policy Engine states |
| `enum ping_type_t` | `app/_wpc.h:129-134` | qdt_ping(0), dig_ping(1), det_ping(2) | Ping type |

### 1.4 Configuration Macros (from `app/config.h`)

| Macro | Value | Purpose |
|---|---|---|
| `BATTERY_CV_VALUE` | 4200 | Battery charge voltage (mV) |
| `CONFIG_NU6801_BATLOW_VOLT` | 2800 | Battery low/dead threshold |
| `CONFIG_WPC_SUPPORT` | 1 | WPC wireless charging enabled |
| `CONFIG_USBA_SUPPORT` | 0 (EVK_V02) | USB-A port disabled on EVK V02 |
| `BUCKBOOST_USED_NU6801` | 1 | Using NU6801 buck-boost IC |
| `CONFIG_TYPECA_SUPPORT` | 1 | Type-C port A enabled |
| `CONFIG_TYPECB_SUPPORT` | 1 | Type-C port B enabled |
| `CONFIG_AFC_SOURCE_SUPPORT` | 1 | AFC source protocol |
| `CONFIG_FCP_SOURCE_SUPPORT` | 1 | FCP source protocol |
| `CONFIG_SCP_SOURCE_SUPPORT` | 1 | SCP source protocol |
| `CONFIG_SUPPORT_IPGA` | 1 | IPGA current sensing |
| `CONFIG_DISCHG_IBAT_LIMIT` | 0x04 (8A) | Discharge battery current limit |
| `CONFIG_DEADBATT_VOLTAGE` | 2900 | Dead battery threshold (mV) |
| `ENABLE_EPP_FUNC` | 1 | EPP (Extended Power Profile) enabled |
| `OPTION_FOD_ENABLE` | 1 | Foreign Object Detection enabled |
| `SLEEPQ_WAKEUP_ENABLE` | 1 | Sleep Q wake up function |
| `CONFIG_NEW_CCC_LOG_ENABLE` | 1 | New CCC log/exception tracking |
| `OVER_VOLTAGE_THRESHOLD` | 4200 | OV threshold for exception tracking |
| `Rsns` | 200 | Sense resistor (0.01 mohm) |

Flash memory layout (from `fml/g_data.h:9-18`):
- `0x0000-0x15FF` - LDROM
- `0x1400-0x15FF` - Log storage
- `0x1600-0x17FF` - Q-freq calibration, gauge, data flash
- `0x1800-0x19FF` - Product information

---

## 2. State Machines

### 2.1 State Machine Inventory

| # | Name | State Variable | File | Type |
|---|---|---|---|---|
| SM1 | WPC Protocol Phase | `gd->ptx_protocol_phase` | `app/_wpc.c:299` | Hierarchical top-level |
| SM2 | WPC Idle Phase | `gd->ptx_idle_phase_status` | `app/wpc_idle.c:164`, `app/sleep.c:564` | Sub-state of IDLE |
| SM3 | Type-C (per port) | `g_tc[n].usb_tc_state` + `usb_tc_substate` | `lib/typec.c:909-934` | Table-driven (enter/exit callbacks) |
| SM4 | USB PD Policy Engine | `usb_pd_state` + `usb_pd_substate` | `lib/usb_pd.c:20-21` | Table-driven |
| SM5 | Port Manager | `g_port.state` + `g_port.port_event` | `app/port_manager.c:1462` | Event-driven |
| SM6 | Buck-Boost | `g_buckboost.woke_mode` | `power/buckboost.c:538` | Event-driven |
| SM7 | LED Display | `g_eLedState` | `app/led.c` | Polling |
| SM8 | DPDM Protocol | implicit (event-driven) | `fml/dpdm.c:155` | Event-driven |
| SM9 | EPP Authentication | `epp_auth.EPP_auth_status` | `app/epp.c:1316` | Sequential |
| SM10 | ASK Packet Decode | `decode->pkt.pkt_phase` + `decode->byt.byt_phase` | `lib/ask.c:316,509` | ISR-driven nested |

### 2.2 SM1: WPC Protocol Phase State Machine

**State variable:** `gd->ptx_protocol_phase` (uint8_t)
**Enum:** `enum ptx_protocol_phase_t` (`app/_wpc.h:150-157`)
**Main dispatcher:** `wpc_protocol_sm()` (`app/_wpc.c:287-321`)
**Initial state:** `WPC_PHASE_IDLE` (set during init)

**States:**
| State | Value | Description |
|---|---|---|
| `WPC_PHASE_IDLE` | 0 | No power transfer, idle/sleep |
| `WPC_PHASE_PING` | 1 | Digital ping - detecting receiver |
| `WPC_PHASE_CNFG` | 2 | Identification & Configuration |
| `WPC_PHASE_NEGO` | 3 | Negotiation (EPP/MPP power contract) |
| `WPC_PHASE_XFER` | 4 | Power Transfer (main operating phase) |
| `WPC_PHASE_CLOAK` | 5 | Cloak mode (low-power standby with RX) |

**Transition Table:**

| From | To | Condition | File:Line |
|---|---|---|---|
| IDLE | PING | `WPC_EVT_DIG_PING` timer fires | `app/wpc_idle.c` (ping start) |
| PING | CNFG | First valid ASK packet (SIG) received | `app/wpc_ping.c` |
| CNFG | NEGO | End of configuration phase (EPP/MPP detection) | `app/wpc_cnfg.c` |
| CNFG | XFER | BPP: config complete, skip nego | `app/wpc_cnfg.c` |
| NEGO | XFER | End negotiation SRQ(0x00) received | `app/_wpc.c:265` |
| XFER | IDLE | EPT received / timeout / error | `app/_wpc.c:177,217` |
| XFER | CLOAK | cloak mode trigger (`flg_mode_cloak == TRUE`) | `app/_wpc.c:137` |
| CLOAK | IDLE | Cloak ping timeout / error | `app/_wpc.c:547` |
| ANY > PING | IDLE | Error/timeout/protection event | `wpc_stop_to_idle()` (`app/_wpc.c:101`) |

**Entry/Exit Actions:**
- Enter IDLE: `wpc_stop_power()` - stops EPWM, disables ASK, resets NU103x, starts re-ping timer
- Enter PING: Starts digital ping via EPWM, enables ASK decode
- Enter XFER: Starts CEP/RPP timers, power measurement windows (1st-4th WND)

### 2.3 SM2: WPC Idle Phase State Machine

**State variable:** `gd->ptx_idle_phase_status` (uint8_t)
**Enum:** `enum ptx_idle_phase_state_t` (`app/_wpc.h:159-170`)
**Dispatcher:** `switch (gd->ptx_idle_phase_status)` in `wpc_idle.c:164` and `sleep.c:564`

**States:**
| State | Value | Description |
|---|---|---|
| `WPC_IDLE_STAT_STANDBY` | 0 | Normal standby - QDT scanning for object |
| `WPC_IDLE_STAT_XER_COM` | 1 | Transfer complete (charge complete EPT) |
| `WPC_IDLE_STAT_XER_FOD` | 2 | Transfer ended due to FOD |
| `WPC_IDLE_STAT_QDT_FOD` | 3 | QDT detected foreign object |
| `WPC_IDLE_STAT_LAR_MET` | 4 | Large metal object detected |
| `WPC_IDLE_STAT_EPT_ERR` | 5 | EPT error (battery failure, etc.) |
| `WPC_IDLE_STAT_EPT_RES` | 6 | EPT restart (no response, negotiation failure) |
| `WPC_IDLE_STAT_EPT_REP` | 7 | EPT re-ping requested by RX |
| `WPC_IDLE_STAT_CLOAKING` | 8 | Cloaking active |
| `WPC_IDLE_STAT_QDT_CALI` | 9 | QDT calibration in progress |

**Key Transitions (in wpc_idle.c:164):**
- STANDBY: Q/F outside limL/limH for 3 cycles -> LAR_MET; Q/F delta detected -> QDT_FOD; Object detected -> try digital ping
- XER_COM/XER_FOD/QDT_FOD/LAR_MET: Object removed (Q/F back to normal) -> STANDBY
- EPT_RES/EPT_REP: Re-ping countdown -> STANDBY

### 2.4 SM3: Type-C State Machine (per port)

**State variable:** `g_tc[port].usb_tc_state` (enum usb_tc_state_e)
**Sub-state:** `g_tc[port].usb_tc_substate` (enter_state / exit_state)
**Transition table:** `usb_tc_table[]` (`lib/typec.c:909-934`)
**Runner:** `usb_tc_run()` (`lib/typec.c:938-968`) - called from OSAL timer

This is a **table-driven state machine** with enter/exit callback pairs. Each state has an `enter_cb` that performs entry actions and sets substate to `exit_state`, and an `exit_cb` that polls for transitions.

**States (enum usb_tc_state_e, fml/tcpm.h:9-31):**

| State | Value | Description |
|---|---|---|
| `TC_Disable` | 0 | Port disabled (CC open) |
| `TC_SNK_Unattached` | 1 | Sink unattached - CC=RD, looking for source |
| `TC_SNK_AttachWait` | 2 | Sink detected source, debouncing |
| `TC_SNK_Attached` | 3 | Sink attached - VBUS present |
| `TC_SRC_Unattached` | 4 | Source unattached - CC=RP, looking for sink |
| `TC_SRC_AttachWait` | 5 | Source detected sink, debouncing |
| `TC_SRC_Attached` | 6 | Source attached - providing VBUS |
| `TC_DEBUG_Attached` | 7 | Debug accessory (both CC = Rd) |
| `TC_DRP_TOGGLE` | 8 | DRP toggle - alternates Rp/Rd |
| `TC_Try_SNK` | 9 | Try.SNK state |
| `TC_TryWAIT_SRC` | 10 | TryWait.SRC state |
| `TC_Try_SRC` | 11 | Try.SRC state |
| `TC_TryWAIT_SNK` | 12 | TryWait.SNK state |
| `TC_ACCESSORY_Attached` | 13 | Audio accessory (both CC = Ra) |
| `TC_ErrorRecovery` | 14 | Error recovery (CC open, wait) |

**Key Transitions:**

| From | To | Condition |
|---|---|---|
| DRP_TOGGLE | SNK_Unattached | Rd set, SNK connected for 10 cycles |
| DRP_TOGGLE | SRC_Unattached | Rp set, SRC connected for 10 cycles |
| SNK_Unattached | SNK_AttachWait | CC connected |
| SNK_AttachWait | SNK_Attached | VBUS present + debounce (TC_T_PD_DEBOUNCE) |
| SNK_Attached | SNK_Unattached | CC disconnect + VBUS removed |
| SRC_Unattached | SRC_AttachWait | Rd detected on CC |
| SRC_AttachWait | SRC_Attached | CC stable + debounce (TC_T_CC_DEBOUNCE) |
| SRC_Attached | SRC_Unattached | Rd removed + debounce |
| SNK_AttachWait | Try_SRC | (try_src_cnt < 3) |
| SRC_AttachWait | Try_SNK | (try_snk_cnt < 5) |
| ANY | ErrorRecovery | Error condition |
| ErrorRecovery | SNK_Unattached/SRC_Unattached | TC_T_ERROR_RECOVERY timeout |

**Interaction with Port Manager:**
- SNK_Attached entry: sends `PORT_ENUM_EVT_PORTn_CONNECT_SUCCESS` to PORT_MANAGER_TASK
- SNK_Attached exit (disconnect): sends `PORTn_EVENT_UNCONNECT` event
- SRC_Attached entry: same pattern

### 2.5 SM4: USB PD Policy Engine

**State variable:** `usb_pd_state` (static uint8_t, `lib/usb_pd.c:20`)
**Sub-state:** `usb_pd_substate` (static uint8_t, `lib/usb_pd.c:21`)
**States (enum usb_pd_state_e, usbpd/usb_pd.h:44-86):**

SNK states (0-15):
- PE_SNK_RSC_Disable(0), PE_SNK_Startup(3), PE_SNK_Discovery, PE_SNK_Wait_for_Capabilities, PE_SNK_Evaluate_Capability, PE_SNK_Select_Capability, PE_SNK_Transition_Sink, PE_SNK_Ready(7), PE_SNK_Hard_Reset(8), PE_SNK_Transition_to_default, PE_SNK_Give_Sink_Cap, PE_SNK_Send_Soft_Reset, PE_SNK_Soft_Reset(12), PE_SNK_Not_Supported_Received, PE_SNK_Send_Not_Supported(14), PE_SNK_Give_Sink_Cap_Ext(15)

SRC states (16+):
- PE_SRC_Startup(16), PE_SRC_Discovery(17), PE_SRC_Send_Capabilities(18), PE_SRC_Negotiate_Capability, PE_SRC_Transition_Supply, PE_SRC_Ready(21), PE_SRC_Disabled, PE_SRC_Capability_Response, PE_SRC_Hard_Reset, PE_SRC_Hard_Reset_Received, PE_SRC_Transition_to_default, PE_SRC_Give_Source_Cap, PE_SRC_Wait_New_Capabilities, PE_SRC_Send_Soft_Reset(29), PE_SRC_Soft_Reset, PE_SRC_Not_Supported_Received, PE_SRC_Send_Not_Supported, PE_SRC_Give_PPS_Status(33), PE_SRC_SNK_Chunk_Received(35)

**Key Transitions:**
- SRC_Startup -> SRC_Discovery -> SRC_Send_Capabilities -> SRC_Negotiate_Capability -> SRC_Transition_Supply -> SRC_Ready
- SNK_Startup -> SNK_Discovery -> SNK_Wait_for_Capabilities -> SNK_Evaluate_Capability -> SNK_Select_Capability -> SNK_Transition_Sink -> SNK_Ready
- ANY -> Hard_Reset (on hard reset event)
- Ready -> various (on VDM, DR_Swap, PR_Swap, etc.)

### 2.6 SM5: Port Manager

**State variable:** `g_port.state` (enum port_state_e)
**Event flags:** `g_port.port_event` (bitmask)
**Dispatcher:** `port_manager_event_handle()` (`app/port_manager.c:1460`)
**Scanner:** `port_enum_scan_handle()` (`app/port_manager.c:1320`)

**States:**
| State | Value | Description |
|---|---|---|
| `PORT_IDLE_OR_READY` | 0 | Ready to handle new port events |
| `PORT_INHANDLING` | 1 | Currently handling a port connect/disconnect |

**Event Processing (scan_handle, port_manager.c:1320):**
1. If `PORT_INHANDLING`: only process UNCONNECT for non-inhandle ports (clear event, set state to NONE)
2. If `PORT_IDLE_OR_READY`: process events in priority order:
   - UNCONNECT events (Port0 > Port1 > Port2 > Port3) -> enter INHANDLING
   - TRY_CONNECT events (Port0 > Port1 > Port2 > Port3) -> enter INHANDLING
   - RESET_CHARGE event -> re-negotiate charging

**Port lifecycle:**
1. TC attach detected -> `PORTn_EVENT_TRY_CONNECT` -> `PORT_ENUM_EVT_PORTn_CONNECT_START`
2. Start handler: disable other ports, prepare buck-boost -> wait for TC attach confirmation
3. `PORT_ENUM_EVT_PORTn_CONNECT_SUCCESS` -> determine SNK/SRC, configure PD/DPDM
4. If SNK: `SETVOLT` -> `SETCHARGE` -> `ENUM_DONE`
5. If SRC: `ENUM_DONE` -> enable gate, start WPC
6. Disconnect: `PORTn_EVENT_UNCONNECT` -> `CONNECT_CLOSED` -> restore other ports

### 2.7 SM6: Buck-Boost State Machine

**State variable:** `g_buckboost.woke_mode` (enum buckboost_mode)
**Dispatcher:** `buckboost_task_event_handler()` (`power/buckboost.c:538`)

**States:**
| State | Value | Description |
|---|---|---|
| `BUCKBOOST_SHUTDOWM_MODE` | 0 | Power converter off |
| `BUCKBOOST_CHAGER_MODE` | 1 | Charging battery from external source |
| `BUCKBOOST_DISCHG_MODE` | 2 | Discharging battery to output ports |

**Transitions:**
- DISCHG -> SHUTDOWN -> CHARGER: when SNK attached (port_manager.c)
- CHARGER -> SHUTDOWN -> DISCHG: when all SNK disconnected (port_manager.c)
- Set via `buckboost_set_work_mode()` / `hal_tcpc_set_source_mode()` from port_manager

### 2.8 SM7: LED Display State Machine

**State variable:** `g_eLedState` (TE_LED_STATE, `app/led.h:42`)
**States:** NULL(0), POWERON(1), STANDBY(2), CHARGING(3), CHARGED(4), ERROR(5)
**Updated by:** `ui_update()` in `app/led.c`
**Displayed by:** `led_display()` using charlieplexed 5-LED array

### 2.9 SM10: ASK Packet Decode (3-level nested)

**Packet-level:** `decode->pkt.pkt_phase` (`lib/ask.c:316`)
- `PKT_PHS_PRM` - Preamble detection
- `PKT_PHS_HDR` - Header byte
- `PKT_PHS_MSG` - Message bytes
- `PKT_PHS_CHS` - Checksum byte

**Byte-level:** `decode->byt.byt_phase` (`lib/ask.c:509`)
- `BYT_PHS_STR` - Start bit
- `BYT_PHS_DAT` - Data bits
- `BYT_PHS_PTY` - Parity bit
- `BYT_PHS_STP` - Stop bit

**Bit-level:** `decode->bit.bit_phase` (`lib/ask.c:561`)
- `BIT_PHS_NOR` - Normal bit detection

---

## 3. State Machine Relationships

```
                    +------------------+
                    |   Port Manager   |  (SM5)
                    |  g_port.state    |
                    +--------+---------+
                             |
              +--------------+--------------+
              |              |              |
     +--------v----+  +-----v------+  +----v-------+
     | Type-C A    |  | Type-C B   |  | USB-A/WPC  |
     | g_tc[0]     |  | g_tc[1]    |  | (implicit) |
     | (SM3)       |  | (SM3)      |  |            |
     +------+------+  +-----+------+  +----+-------+
            |                |              |
     +------v------+  +-----v------+       |
     | USB PD PE   |  | (shared)   |       |
     | usb_pd_state|  |            |       |
     | (SM4)       |  +------------+       |
     +------+------+                       |
            |                              |
     +------v---------+            +-------v--------+
     | DPDM/BC1.2     |            | Buck-Boost     |
     | (SM8)          |            | g_buckboost    |
     +----------------+            | woke_mode(SM6) |
                                   +-------+--------+
                                           |
                              +------------+------------+
                              |                         |
                     +--------v--------+       +--------v--------+
                     | WPC Protocol    |       | LED Display     |
                     | ptx_protocol_   |       | g_eLedState     |
                     | phase (SM1)     |       | (SM7)           |
                     +--------+--------+       +-----------------+
                              |
                     +--------v--------+
                     | WPC Idle Phase  |
                     | ptx_idle_phase_ |
                     | status (SM2)    |
                     +--------+--------+
                              |
                     +--------v--------+
                     | ASK Decode      |
                     | pkt/byt/bit     |
                     | phase (SM10)    |
                     +-----------------+
```

**Key interactions:**
1. **Port Manager** orchestrates all port state machines. When a Type-C port attaches/detaches, Port Manager coordinates buck-boost mode, WPC enable/disable, and PDO updates.
2. **Type-C SM** notifies Port Manager via events (`PORTn_EVENT_TRY_CONNECT`, `PORTn_EVENT_UNCONNECT`).
3. **USB PD PE** is driven by Type-C attach events and handles PD negotiation.
4. **Buck-Boost** mode is set by Port Manager: DISCHG when all ports are source, CHARGER when a sink is attached.
5. **WPC Protocol** runs independently on WPC_TASK, but Port Manager can stop/reconfigure WPC via `tcpm_stop_wpc()` and `tcpm_update_wpc_work_mode()`.
6. **LED SM** reads from multiple sources (`g_buckboost.woke_mode`, `gd->ptx_protocol_phase`, port states) to determine display.

---

## 4. OSAL Task/Timer Architecture

**Tasks (osal.h:50-62):**
| ID | Name | Handler | Period |
|---|---|---|---|
| 0 | HAL_TASK | (HAL layer) | - |
| 1 | FML_TASK | `fml_task_event_handler` | - |
| 2 | USB_TASK | `tcpm_task_event_handler` | 1ms timer |
| 3 | WPC_TASK | `wpc_task_event_handler` | ping timer |
| 4 | APL_TASK | `app_task_event_handler` | 10ms/100ms/250ms |
| 5 | BUCKBOOST_TASK | `buckboost_task_event_handler` | 17ms/20ms |
| 6 | USB_DPDM_TASK | `dpdm_task_event_handler` | - |
| 7 | PORT_MANAGER_TASK | `port_manager_event_handle` | 1ms scan |

**Timer count:** 31 timers (MAX_TIMER, `osal.h:47`), allocated to specific tasks/functions.

**Scheduling:** Cooperative round-robin. No preemption between tasks. ISRs can set events that are processed in the next task run.
