# Engineering Mode Logic — Full System Documentation

> **Generated**: 2026-02-28
> **Scope**: NU17112 firmware + WB7720 bridge + Windows PC application
> **Interface Spec Version**: v1.3 (exception log v1.3 sliding window) + v1.4 (virtual params / 0xAA refresh)

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Data Pipeline Architecture](#2-data-pipeline-architecture)
3. [I2C Register Map (WB7720 i2c_buff)](#3-i2c-register-map)
4. [Engineering Mode Entry / Exit](#4-engineering-mode-entry--exit)
5. [Virtual Parameter Injection (0xAA Refresh)](#5-virtual-parameter-injection)
6. [Exception Record System](#6-exception-record-system)
7. [Exception Record Erasure (0xEE)](#7-exception-record-erasure)
8. [HID Report Types](#8-hid-report-types)
9. [WB7720 Command Dispatch](#9-wb7720-command-dispatch)
10. [Windows App UI & Modes](#10-windows-app-ui--modes)
11. [Complete Write Sequence](#11-complete-write-sequence)
12. [Test Scenarios](#12-test-scenarios)
13. [Source File Reference](#13-source-file-reference)

---

## 1. System Overview

Engineering mode enables **virtual sensor injection** and **exception log management** for factory testing and CCC compliance verification. It spans three subsystems connected in a linear pipeline:

```
┌─────────────┐   I2C (0x42)   ┌─────────────┐   USB HID (64B)   ┌──────────────┐
│  NU17112     │ ◄────────────► │  WB7720      │ ◄───────────────► │  Windows PC   │
│  (Firmware)  │   47ms cycle   │  (Bridge)    │   1Hz polling     │  (App)        │
│              │                │              │                    │              │
│ • Sensor ADC │                │ • i2c_buff[] │                    │ • tkinter GUI│
│ • Exception  │                │ • exc_cache  │                    │ • HID R/W    │
│   detection  │                │ • Report gen │                    │ • 3 modes    │
│ • Flash log  │                │ • CMD parse  │                    │ • Virtual UI │
└─────────────┘                └─────────────┘                    └──────────────┘
```

**Key principle**: The PC writes commands into `i2c_buff[]` via HID → CMD 0x0C. NU17112 polls `i2c_buff[]` via I2C master reads on a 47ms round-robin cycle and acts on the values it finds.

---

## 2. Data Pipeline Architecture

### 2.1 NU17112 → PC (Telemetry & Logs)

```
NU17112 round-robin timer (47ms, 18 steps)
  │
  ├── Step 0-10:  Write telemetry to i2c_buff[0x00-0x36]
  │                (SOC, voltage, current, temp, cycles, cell voltages, etc.)
  │
  ├── Step 11-12: Write exception counts + charge state
  │                usb_bridge_write_exception_counts()  → 0x11-0x16
  │                usb_bridge_write_charge_state()      → 0x17
  │
  ├── Step 13:    Write one exception record (rate-limited: every 64 cycles ≈ 3s)
  │                usb_bridge_write_exception_record()  → 0x37-0x4D
  │
  └── Step 14-17: Check engineering/production mode, read commands
                   usb_bridge_check_engineering_mode()   ← reads 0x50
                   usb_bridge_check_production_mode()    ← reads 0x90
                   usb_bridge_ensure_product_info()      ← retry 0x92-0xF5
                   usb_bridge_check_eng_test_cmds()      ← reads 0x88
```

**Source**: `fml/_fml.c:122-243` — `ubsd_wb7720_report_update()`

### 2.2 PC → NU17112 (Commands)

```
Windows App
  │
  ├─ _hid_write(CMD_WRITE_REGISTER=0x0C, payload)
  │     ↓
  │  USB HID OUT endpoint → WB7720 Vendor_Request[64B]
  │     ↓
  │  user_loop() CMD 0x0C handler → writes to i2c_buff[]
  │     ↓ (with engineering register write protection)
  │  i2c_buff[] shared memory
  │     ↓
  │  NU17112 I2C master read (next round-robin step 14/17)
  │     ↓
  └─ NU17112 acts on new values
```

### 2.3 Round-Robin Timing

| Parameter | Value |
|-----------|-------|
| Timer interval | 47 ms |
| Steps per cycle | 18 |
| Full cycle period | 47 × 18 = **846 ms** |
| Exception record rotation | Every 64 calls × 47ms ≈ **3 seconds** per record |
| Full exception cache fill | 5 records × 3s ≈ **15 seconds** |

---

## 3. I2C Register Map

### 3.1 Telemetry (0x00-0x36) — NU17112 → WB7720 (write)

| Address | Name | Size | Unit | Description |
|---------|------|------|------|-------------|
| 0x00 | REG_SOC_PCT | 1B | % | Battery state of charge |
| 0x01-0x04 | REG_CAPACITY_MAH | 4B u32 LE | mAh | Rated capacity |
| 0x05-0x06 | REG_VBAT_MV | 2B u16 LE | mV | Total battery voltage |
| 0x07-0x08 | REG_IBAT_MA | 2B s16 LE | mA | Battery current (+charge/-discharge) |
| 0x09-0x0A | REG_TEMP_DC | 2B s16 LE | 0.1°C | Battery NTC temperature |
| 0x0B-0x0C | REG_CYCLE_COUNT | 2B u16 LE | cycles | Charge cycle count |
| 0x0D-0x0E | REG_R_INTERNAL_MOHM | 2B u16 LE | mΩ | Internal resistance |
| 0x0F-0x10 | REG_SOH_PCT_X100 | 2B u16 LE | %×100 | State of health |
| 0x11-0x12 | REG_ERR_OVERTEMP_CNT | 2B u16 LE | count | Over-temperature event count |
| 0x13-0x14 | REG_ERR_OVERVOLT_CNT | 2B u16 LE | count | Over-voltage event count |
| 0x15-0x16 | REG_ERR_OVERCURR_CNT | 2B u16 LE | count | Over-current event count |
| 0x17 | REG_CHARGE_STATE | 1B | enum | 0=idle, 1=charging, 2=discharging |
| 0x2C | REG_CELL_COUNT | 1B | count | Cell count (fixed 2) |
| 0x2D-0x2E | REG_CELL1_VOLTAGE_MV | 2B u16 LE | mV | Cell 1 voltage |
| 0x2F-0x30 | REG_CELL2_VOLTAGE_MV | 2B u16 LE | mV | Cell 2 voltage |
| 0x35-0x36 | REG_PCB_TEMP_DC | 2B s16 LE | 0.1°C | PCB board temperature |

### 3.2 Exception Log Sliding Window (0x37-0x4D) — NU17112 → WB7720

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 0x37 | REG_EXC_TOTAL_COUNT | 1B | Total valid records (0-5) |
| 0x38 | REG_EXC_CURRENT_IDX | 1B | Current rotating index (0-4) |
| 0x39 | REG_EXC_RESERVED | 1B | Reserved = 0x00 |
| 0x3A-0x4D | REG_EXC_RECORD | 20B | One BatteryExceptionRecord_t |

**Record layout** (20 bytes at 0x3A):

```
Offset  Size  Type   Field
------  ----  ----   -----
+0      2B    u16    year (LE)
+2      1B    u8     month
+3      1B    u8     day
+4      1B    u8     hour
+5      1B    u8     minute
+6      1B    u8     second
+7      1B    u8     reserved (0x00)
+8      1B    u8     error_type: 0x01=OV, 0x02=OT, 0x03=UT
+9      1B    u8     sub_type: OV→cell_num(1-N), TEMP→charge_state(0=CHG,1=DCHG)
+10     2B    u16    OV: max_voltage(mV) / TEMP: max_temperature(s16, 0.1°C)
+12     2B    u16    OV: total_voltage(mV) / TEMP: 0x0000
+14     2B    u16    padding = 0x0000
+16     4B    u32    record_id (internal sequence, used for deduplication)
```

### 3.3 Sleep/Wake (0x4E-0x4F)

| Address | Name | Direction | Description |
|---------|------|-----------|-------------|
| 0x4E | REG_SLEEP_CMD | PC→WB7720 | Write non-zero → MCU stop mode |
| 0x4F | REG_WAKE_CMD | PC→WB7720 | Write non-zero → re-enable USB |

### 3.4 Engineering Mode Registers (0x50-0x89) — PC → NU17112

| Address | Name | Size | Direction | Description |
|---------|------|------|-----------|-------------|
| **0x50** | **REG_WORK_MODE** | 1B | PC→WB7720→NU17112 | `0xA5` = enter eng mode, `0x00` = user mode |
| 0x60-0x63 | REG_ENG_CURRENT_DATE | 4B | PC→NU17112 | [Year_lo][Year_hi][Month][Day] |
| 0x70-0x73 | REG_ENG_PRODUCTION_DATE | 4B | PC→NU17112 | [Year_lo][Year_hi][Month][Day] |
| 0x80-0x81 | REG_ENG_CYCLE_CHG_COUNT | 2B u16 LE | PC→NU17112 | Virtual cycle count |
| **0x82-0x83** | **REG_ENG_VIRTUAL_CELL1** | 2B u16 LE | PC→NU17112 | Virtual Cell1 mV (0=use real) |
| **0x84-0x85** | **REG_ENG_VIRTUAL_CELL2** | 2B u16 LE | PC→NU17112 | Virtual Cell2 mV (0=use real) |
| **0x86-0x87** | **REG_ENG_VIRTUAL_TEMP** | 2B s16 LE | PC→NU17112 | Virtual temp ×0.1°C (0=use real) |
| **0x88** | **REG_ENG_ERASE_ALL_CMD** | 1B | PC→NU17112 | `0xEE`=erase, `0xAA`=refresh virtual params |
| 0x89 | REG_ENG_CMD_STATUS | 1B | NU17112→PC | `0x00`=idle, `0x02`=success, `0xFF`=failure |

### 3.5 Production / Device Info (0x90-0xF5)

| Address | Name | Size | Description |
|---------|------|------|-------------|
| 0x90 | REG_PROD_MODE_FLAG | 1B | `0xB5` = enter production mode |
| 0x91 | REG_PROD_WRITE_STATUS | 1B | `0x01`=in progress, `0x02`=success, `0xFF`=fail |
| 0x92-0xA5 | REG_PROD_MANUFACTURER | 20B | Manufacturer name string |
| 0xA6-0xB9 | REG_PROD_MODEL | 20B | Model name string |
| 0xBA-0xCD | REG_PROD_BATTERY_MFR | 20B | Battery manufacturer |
| 0xCE-0xE1 | REG_PROD_BATTERY_MODEL | 20B | Battery model |
| 0xE2-0xF5 | REG_PROD_PROD_DATE | 20B | Production date string |

### 3.6 Write Protection

Both the WB7720 USB CMD handler and I2C slave IRQ enforce identical protection:

```
Registers 0x60-0x63, 0x70-0x73, 0x80-0x81, 0x82-0x88:
  → ONLY writable when i2c_buff[0x50] == 0xA5 (ENGINEERING_MODE_KEY)
  → Otherwise writes are silently dropped

All other registers:
  → Always writable
```

**Enforcement locations**:
- WB7720 `user_loop()` CMD 0x0C handler: `main.c:529-541`
- WB7720 `I2C_IRQHandler()` slave write: `main.c:613-620`

---

## 4. Engineering Mode Entry / Exit

### 4.1 State Variables (NU17112 side)

```c
// app/usb_bridge.c — static variables
static bool     eng_mode_active = false;
static uint32_t eng_saved_rtc_seconds;      // Real RTC at entry
static uint8_t  eng_saved_cycle_count;      // Real cycle count at entry
static uint32_t eng_entry_virtual_seconds;  // Virtual RTC assigned at entry
static uint8_t  eng_entry_virtual_cycle;    // Virtual cycle assigned at entry
static uint16_t eng_virtual_cell1 = 0;      // Virtual Cell1 mV (0=disabled)
static uint16_t eng_virtual_cell2 = 0;      // Virtual Cell2 mV (0=disabled)
static int16_t  eng_virtual_temp  = 0;      // Virtual temp ×0.1°C (0=disabled)
```

### 4.2 Entry Sequence

**Trigger**: NU17112 reads `REG_WORK_MODE (0x50) == 0xA5` and `eng_mode_active == false`

**Function**: `usb_bridge_check_engineering_mode()` — `app/usb_bridge.c:325`

```
1. Save real values:
   eng_saved_rtc_seconds  = gd->Bat_RTC_Seconds
   eng_saved_cycle_count  = gd->Battery_cycle_count

2. Read virtual parameters from I2C:
   ┌──────────────────────────────────────────────────────────┐
   │ i2c_buff[0x60-0x63] → current date → date_to_seconds()  │
   │ i2c_buff[0x70-0x73] → production date                    │
   │ i2c_buff[0x80-0x81] → virtual cycle count                │
   │ i2c_buff[0x82-0x83] → eng_virtual_cell1 (mV)            │
   │ i2c_buff[0x84-0x85] → eng_virtual_cell2 (mV)            │
   │ i2c_buff[0x86-0x87] → eng_virtual_temp (×0.1°C)         │
   └──────────────────────────────────────────────────────────┘

3. Override gd fields:
   gd->Bat_RTC_Seconds   = date_to_seconds(eng_year, eng_month, eng_day)
   gd->Battery_cycle_count = virtual cycle

4. Record entry snapshots:
   eng_entry_virtual_seconds = gd->Bat_RTC_Seconds  (after override)
   eng_entry_virtual_cycle   = gd->Battery_cycle_count

5. Set eng_mode_active = true
```

### 4.3 Exit Sequence

**Trigger**: NU17112 reads `REG_WORK_MODE (0x50) != 0xA5` and `eng_mode_active == true`

**Function**: `usb_bridge_check_engineering_mode()` — `app/usb_bridge.c:409-445`

```
1. Calculate elapsed deltas:
   elapsed_seconds = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds
   cycles_added    = gd->Battery_cycle_count - eng_entry_virtual_cycle

2. Restore real values + deltas (preserves time passage during test):
   gd->Bat_RTC_Seconds    = eng_saved_rtc_seconds + elapsed_seconds
   gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added

3. Reload real ProductInfo from Flash to I2C 0x92-0xF5

4. Clear virtual overrides:
   eng_virtual_cell1 = 0
   eng_virtual_cell2 = 0
   eng_virtual_temp  = 0

5. Set eng_mode_active = false
```

**Delta preservation rationale**: If 10 minutes pass during an engineering test, the real RTC advances by 10 minutes rather than snapping back to the pre-test value.

### 4.4 PC-Side Exit Notification

When the user switches away from engineering mode in the Windows app:

```python
# battery_monitor.py:1241-1244
def _exit_eng_on_device(self):
    payload = bytes([0x50, 1, 0x00])  # REG_WORK_MODE = 0x00
    self._hid_write(CMD_WRITE_REGISTER, payload)
```

Runs in a background daemon thread. Silent failure (non-critical cleanup).

---

## 5. Virtual Parameter Injection

### 5.1 How Virtual Values Override Sensors

Virtual values are applied in `app/bat_record.c` during exception detection. The override logic uses a **zero-means-disabled** convention:

**Overvoltage detection** — `battery_record_update_overvoltage()` at `bat_record.c:387-420`:

```c
uint16_t eng_c1 = usb_bridge_get_eng_cell1();
uint16_t eng_c2 = usb_bridge_get_eng_cell2();

if (eng_c1 > 0 || eng_c2 > 0) {
    // Virtual mode active
    uint16_t real_total = hal_nu6805_buckboost_get_bat_voltage();
    cell1_voltage = (eng_c1 > 0) ? eng_c1 : (real_total / 2);  // Per-cell override
    cell2_voltage = (eng_c2 > 0) ? eng_c2 : (real_total / 2);
    total_voltage = cell1_voltage + cell2_voltage;
} else {
    // Normal mode — real ADC
    total_voltage = hal_nu6805_buckboost_get_bat_voltage();
    cell1_voltage = total_voltage / 2;
    cell2_voltage = total_voltage / 2;
}
```

**Temperature detection** — `battery_record_update_overtemperature()` at `bat_record.c:432-444`:

```c
int16_t eng_temp = usb_bridge_get_eng_temp();

if (eng_temp != 0) {
    ntc_temp = eng_temp;        // Use virtual value
} else {
    ntc_temp = gd->sys_infos.ntc_temp_wpc;  // Use real NTC
}
```

### 5.2 Exception Thresholds

| Type | Threshold | Config Location |
|------|-----------|----------------|
| Over-voltage (OV) | > 4450 mV per cell | `config.h:77` |
| Over-temperature (OT) | > 600 (60.0°C) | `config.h:78` |
| Under-temperature (UT) | < -100 (-10.0°C) | `config.h:79` (if defined) |

### 5.3 The 0xAA Refresh Command

**Purpose**: Trigger an immediate engineering mode session reset so new virtual values take effect without fully exiting engineering mode.

**Register**: `REG_ENG_ERASE_ALL_CMD (0x88)` — dual-purpose register

**Handler**: `usb_bridge_check_eng_test_cmds()` at `app/usb_bridge.c:550-562`

```c
else if (erase_cmd == 0xAA) {
    // Refresh: EXIT current session, next step-14 will re-ENTER with new values
    uint32_t elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds;
    gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed;

    uint8_t cycles_added = gd->Battery_cycle_count - eng_entry_virtual_cycle;
    gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added;

    eng_virtual_cell1 = 0;
    eng_virtual_cell2 = 0;
    eng_virtual_temp  = 0;
    eng_mode_active = false;

    // ACK to bridge
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_CMD_STATUS, 0x02);
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_ERASE_ALL_CMD, 0x00);
}
```

**Sequence**:
1. PC writes new virtual values to 0x82-0x87
2. PC writes `0xAA` to `0x88`
3. NU17112 reads 0x88, finds 0xAA
4. NU17112 exits eng mode (preserving RTC/cycle deltas)
5. NU17112 clears 0x88 to 0x00, writes status 0x02 to 0x89
6. On next round-robin step 14, NU17112 re-reads 0x50 (still 0xA5) and **re-enters** eng mode with the new virtual values

**Key difference from full exit**: `REG_WORK_MODE (0x50)` stays at `0xA5`, so the mode re-activates automatically on the next poll cycle.

---

## 6. Exception Record System

### 6.1 NU17112 Flash Storage

**Structure**: `BatteryRecordStorage_t` in `app/bat_record.c`

```
Flash address: 0x00001400 (AP_CFG_ROM_ADDR_LOG)
Page size:     512 bytes

Layout:
  +0x00  magic (u32)            = 0x42415436 ('BAT6')
  +0x04  exception_counter (u32)
  +0x08  write_ptr (u32)
  +0x0C  reserved (u32)
  +0x10  records[5] (5 × 20B = 100B)   ← BatteryExceptionRecord_t array
  +0x74  checksum (u32)
```

### 6.2 I2C Sliding Window (NU17112 → WB7720)

NU17112 writes one record at a time to `i2c_buff[0x37-0x4D]`, rotating through all stored records:

```
  exc_rotate_idx cycles: 0 → 1 → 2 → 3 → 4 → 0 → ...

  Each write (23 bytes):
    [0x37] = valid_count        (how many total records exist)
    [0x38] = exc_rotate_idx     (which record this is)
    [0x39] = 0x00               (reserved)
    [0x3A-0x4D] = 20-byte record data
```

**Rate control**: Only executes every 64th call (`EXC_WRITE_INTERVAL = 64`), yielding ~3 seconds per record update.

### 6.3 WB7720 Exception Cache

**Variables** (WB7720 `main.c:137`):

```c
static uint8_t exc_cache[5][20];   // 5 × 20B local cache
static uint8_t exc_cache_count;    // Current cached count (0-5)
static uint8_t exc_page_idx;       // HID output page (0-2, cycling)
```

**Deduplication** (`exc_cache_update()` at `main.c:151-170`):

```
On every Type 0x01 report assembly:
  1. Read i2c_buff[0x3A-0x4D] (current 20B record)
  2. Extract record_id from bytes [16-19] (u32 LE)
  3. If record_id == 0 → skip (no valid record)
  4. If exc_cache_count >= 5 → skip (cache full)
  5. Scan existing cache for matching record_id → skip if duplicate
  6. Append to exc_cache[exc_cache_count++]
```

### 6.4 PC-Side Deduplication

```python
# battery_monitor.py
_exception_seen_ids: set = set()   # Tracks record_ids already displayed

# On receiving Type 0x02 report:
for record in parsed_records:
    rid = record['record_id']
    if rid == 0 or rid in _exception_seen_ids:
        continue
    _exception_seen_ids.add(rid)
    _exception_list.append(format_record(record))
```

### 6.5 End-to-End Record Flow

```
NU17112 detects threshold violation
  → Creates BatteryExceptionRecord_t (20B) with timestamp + data
  → Saves to Flash (bat_record.c: save_storage_to_flash)
  → Rotates through i2c_buff[0x37-0x4D] every ~3 seconds
      ↓
WB7720 exc_cache_update()
  → Reads record from i2c_buff[0x3A-0x4D]
  → Deduplicates by record_id
  → Stores in exc_cache[count] (max 5)
      ↓
WB7720 update_report_buffer_1()
  → Paginates: 3 pages × (2,2,1) records
  → Sends as HID Type 0x02 report
      ↓
Windows App _parse_exception_log()
  → Filters by record_id (seen set)
  → Formats and displays in exception text widget
```

---

## 7. Exception Record Erasure

### 7.1 Trigger

PC sends `0xEE` to `REG_ENG_ERASE_ALL_CMD (0x88)` via CMD 0x0C.

### 7.2 NU17112 Handler

**Function**: `usb_bridge_check_eng_test_cmds()` at `app/usb_bridge.c:546-549`

```c
if (erase_cmd == 0xEE) {
    battery_record_erase_all();
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_CMD_STATUS, 0x02);  // ACK
    hal_i2cm_wirte_one_byte(USB_BRIDGE_WB7720_ADDR, REG_ENG_ERASE_ALL_CMD, 0x00); // Clear
}
```

### 7.3 Flash Erase Implementation

**Function**: `battery_record_erase_all()` at `app/bat_record.c:608-614`

```c
void battery_record_erase_all(void) {
    memset((void*)&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
    g_record_storage.magic = MAGIC_VALUE;    // Reset to 0x42415436
    save_storage_to_flash();                 // Erase page + rewrite
    memset((void*)&g_exception_cache, 0, sizeof(g_exception_cache));
}
```

**Flash operations** (`save_storage_to_flash()` at `bat_record.c:175-184`):

```
1. Calculate checksum over g_record_storage
2. hal_fmc_erase_page(0x00001400)     ← Erase 512B Flash page
3. flash_write_record(0x00001400, ...)  ← Write clean structure
```

### 7.4 PC-Side Cleanup

```python
# battery_monitor.py: _erase_worker()
_exception_seen_ids.clear()   # Reset dedup set
_exception_list.clear()       # Clear display list
_refresh_exception_display()  # Show "暂无异常记录"
```

### 7.5 Status Feedback

| 0x89 Value | Meaning |
|------------|---------|
| 0x00 | Idle / no pending command |
| 0x02 | Command completed successfully |
| 0xFF | Command failed |

After processing, NU17112 clears `0x88` to `0x00` to prevent re-triggering.

---

## 8. HID Report Types

### 8.1 Common Frame Header

```
Bytes 0-2:   SOF = [0x05, 0xA5, 0xA5] (Type 0x01) or [0x05, 0xA5, 0x5A] (Type 0x02/0x03)
Byte 3:      Ver = 0x02
Byte 4:      Type (0x01 / 0x02 / 0x03)
Bytes 5-6:   Seq (u16 LE, auto-incrementing)
Bytes 7-8:   Len (u16 LE, payload length)
Bytes 9-N:   Payload
Bytes N+1-2: CRC16-CCITT (over bytes [3..N])
Remaining:   0x00 padding to 64 bytes
```

### 8.2 Type 0x01 — Telemetry

**Function**: `update_report_buffer_0()` — WB7720 `main.c:172-257`

**Payload** (42 bytes from byte 9):

| Offset | Size | Field | Unit | Source |
|--------|------|-------|------|--------|
| +0 | 1B | SOC | % | i2c_buff[0x00] |
| +1 | 4B | Capacity | mAh | i2c_buff[0x01-0x04] |
| +5 | 2B | TotalVoltage | V×100 | VBAT_MV ÷ 10 |
| +7 | 2B | TotalCurrent | A×100 | IBAT_MA ÷ 10 |
| +9 | 2B | Power | W×10 | (VBAT × IBAT) ÷ 100000 |
| +11 | 1B | ChargeState | enum | i2c_buff[0x17] |
| +12 | 2B | CycleCount | cycles | i2c_buff[0x0B-0x0C] |
| +14 | 2B | BatteryTemp | °C×10 | i2c_buff[0x09-0x0A] |
| +16 | 2B | BoardTemp | °C×10 | i2c_buff[0x35-0x36] |
| +18 | 1B | CellCount | count | 2 |
| +19 | 2B | Cell1Voltage | V×100 | i2c_buff[0x2D-0x2E] ÷ 10 |
| +21 | 2B | Cell2Voltage | V×100 | i2c_buff[0x2F-0x30] ÷ 10 |
| +23 | 2B | R_internal | mΩ | i2c_buff[0x0D-0x0E] |
| +25 | 2B | SOH | %×100 | i2c_buff[0x0F-0x10] |
| +27 | 2B | ErrOTCount | count | i2c_buff[0x11-0x12] |
| +29 | 2B | ErrOVCount | count | i2c_buff[0x13-0x14] |
| +31 | 2B | ErrOCCount | count | i2c_buff[0x15-0x16] |
| +33 | 1B | ExcLogCount | count | exc_cache_count |
| +34 | 1B | BrandLen | bytes | strlen("Nu17113") |
| +35 | 7B | BrandName | string | "Nu17113" |

**Side effect**: Calls `exc_cache_update()` after assembly to capture new exception records.

### 8.3 Type 0x02 — Exception Logs

**Function**: `update_report_buffer_1()` — WB7720 `main.c:276-322`

**Pagination**: 3 pages cycling (page 0→1→2→0):

| Page | exc_page_idx | Records | Cache Indices |
|------|--------------|---------|---------------|
| 0 | 0 | up to 2 | exc_cache[0], exc_cache[1] |
| 1 | 1 | up to 2 | exc_cache[2], exc_cache[3] |
| 2 | 2 | up to 1 | exc_cache[4] |

**Payload** (from byte 9):

```
+0:   OffsetPage (u8) — page index
+1:   ReturnCount (u8) — 0, 1, or 2
+2+:  ExceptionRecords (20B × ReturnCount)
```

### 8.4 Type 0x03 — Device Info

**Function**: `update_report_buffer_device_info(sub_idx)` — WB7720 `main.c:338-386`

| SubIdx | Field1 (20B) | Field2 (20B) |
|--------|-------------|-------------|
| 0x00 | Manufacturer | Model |
| 0x01 | Battery MFR | Battery Model |
| 0x02 | Prod Date | Safety Years (byte 0 only) |

---

## 9. WB7720 Command Dispatch

**Function**: `user_loop()` — WB7720 `main.c:486-574`

```
USB OUT event received → Read 64B into Vendor_Request[]
  │
  ├── Vendor_Request[0] == CMD_READ_DEVICE_INFO (0x02)
  │     → update_report_buffer_device_info(Vendor_Request[1])
  │     → Return Type 0x03
  │
  ├── Vendor_Request[0] == CMD_WRITE_REGISTER (0x0C)
  │     → update_report_buffer_0()  (Return Type 0x01 as ACK)
  │     → Write payload to i2c_buff[] (with protection check)
  │
  └── Vendor_Request[0] == CMD_READ_STATUS (0x01)  [default]
        → Round-robin: rr_index 0-3 → Type 0x01, rr_index 4 → Type 0x02
        → rr_index cycles: 0→1→2→3→4→0→...

CMD 0x0B (Reboot):
  → USB disconnect → 1.5s delay → NVIC_SystemReset()
```

**Round-robin ratio**: 4:1 (telemetry : exception log) per 5 polls.

---

## 10. Windows App UI & Modes

### 10.1 Three-Mode System

| Mode | Key | Password | Panel | Capabilities |
|------|-----|----------|-------|-------------|
| User (默认) | — | — | None | Read-only telemetry display |
| Engineering (工程) | 0xA5 | "123456" | `_eng_panel` | Virtual params, erase logs, set dates/cycles |
| Production (生产) | 0xB5 | "123456" | `_prod_panel` | Write ProductInfo to Flash |

Modes are mutually exclusive (radio buttons).

**Mode switching**: `_on_mode_change()` at `battery_monitor.py:1198-1237`

### 10.2 Engineering Panel Layout

```
┌── 工程模式面板 ─────────────────────────────────────────────┐
│ 当前日期 (YYYY/MM/DD): [2026/02/28]                        │
│ 生产日期 (YYYY/MM/DD): [________________]                   │
│ 循环次数 (0-65535):    [0_________]                         │
│ Cell1电压(mV):         [0_________]                         │
│ Cell2电压(mV):         [0_________]                         │
│ 虚拟温度(×0.1°C):      [0_________]                         │
│ [清除全部记录]                                               │
│ [写入设备]  状态: 就绪                                       │
└─────────────────────────────────────────────────────────────┘
```

### 10.3 Validation Rules

| Field | Range | Unit |
|-------|-------|------|
| Cell voltages | 0 – 5000 | mV |
| Temperature | -500 – 1000 | ×0.1°C |
| Cycle count | 0 – 65535 | cycles |
| Year | > 2000 | year |
| Month | 1 – 12 | month |
| Day | 1 – 31 | day |

**Zero = disabled**: A virtual value of 0 means "use real hardware sensor reading."

---

## 11. Complete Write Sequence

When the user clicks **[写入设备]** in engineering mode, the following 8-step HID command sequence is sent (50ms between steps):

```
Step  Register  Payload                              Purpose
----  --------  -----------------------------------  ---------------------------
 1    0x50      [0xA5]                               Unlock engineering mode
 2    0x60      [year_lo, year_hi, month, day]       Set current date
 3    0x70      [year_lo, year_hi, month, day]       Set production date
 4    0x80      [count_lo, count_hi]                 Set cycle count
 5    0x82      [cell1_lo, cell1_hi]                 Virtual Cell1 voltage (mV)
 6    0x84      [cell2_lo, cell2_hi]                 Virtual Cell2 voltage (mV)
 7    0x86      [temp_lo, temp_hi]                   Virtual temperature (×0.1°C)
 8    0x88      [0xAA]                               Refresh: apply virtual params
```

Each step is a `CMD_WRITE_REGISTER (0x0C)` HID command:

```
[0x00 hidapi prefix][0x0C cmd][reg_addr][data_len][data_bytes...][0x00 padding to 64B]
```

**NU17112 processing timeline**:
- Step 1 write arrives → WB7720 sets `i2c_buff[0x50] = 0xA5`
- ~47-846ms later → NU17112 round-robin step 14 reads 0x50, enters eng mode
- Steps 2-7 write virtual values → stored in `i2c_buff[]`
- Step 8 writes 0xAA → NU17112 round-robin step 17 processes refresh
- NU17112 exits briefly, then re-enters with new values on next step 14

---

## 12. Test Scenarios

### 12.1 Virtual Overvoltage Test

1. Enter engineering mode (write 0x50=0xA5)
2. Set Cell1=4500 (> threshold 4450), Cell2=0 (use real)
3. Write 0xAA to refresh
4. Wait ~3 seconds for exception detection cycle
5. Expected: OV exception record created with `error_type=0x01, sub_type=0x01, max_voltage=4500`
6. Verify record appears in Type 0x02 HID report

### 12.2 Virtual Over-Temperature Test

1. Enter engineering mode
2. Set temperature=650 (65.0°C, > threshold 600)
3. Write 0xAA to refresh
4. Expected: OT exception record with `error_type=0x02, max_temperature=650`

### 12.3 Exception Erase Test

1. Enter engineering mode with valid virtual params
2. Trigger exception records (OV/OT)
3. Verify records appear in Type 0x02 reports
4. Write 0xEE to 0x88
5. Expected: Flash erased, `REG_ENG_CMD_STATUS=0x02`, no more records in Type 0x02

### 12.4 Mode Exit with Delta Preservation

1. Enter engineering mode with virtual date 2025/01/01
2. Wait 5 minutes
3. Exit engineering mode (write 0x50=0x00)
4. Expected: Real RTC = original_rtc + 5 minutes (not snapped back)

---

## 13. Source File Reference

### NU17112 Firmware

| File | Key Functions / Data |
|------|---------------------|
| `app/usb_bridge.c` | `usb_bridge_check_engineering_mode()`, `usb_bridge_check_eng_test_cmds()`, `usb_bridge_write_exception_record()`, `usb_bridge_get_eng_cell1/cell2/temp()` |
| `app/usb_bridge.h` | `USB_BRIDGE_WB7720_ADDR (0x21)`, function declarations |
| `app/bat_record.c` | `battery_record_update_overvoltage()`, `battery_record_update_overtemperature()`, `battery_record_erase_all()`, `save_storage_to_flash()` |
| `app/bat_record.h` | `FLASH_PAGE_SIZE (512)`, threshold defines |
| `app/config.h` | `CONFIG_USB_BRIDGE_ENABLE`, OV/OT thresholds |
| `fml/g_data.h` | `BatteryExceptionRecord_t`, `TimeStamp_t`, `ProductInfo_t`, Flash addresses |
| `fml/g_data.c` | `product_info_read/write/print()` |
| `fml/_fml.c` | `ubsd_wb7720_report_update()` — 18-step round-robin scheduler |

### WB7720 Bridge Firmware

| File | Key Functions / Data |
|------|---------------------|
| `USB参考/USB_下位机程序/Projects/main.c` | `user_loop()`, `update_report_buffer_0/1/device_info()`, `exc_cache_update()`, `I2C_IRQHandler()` |
| `USB参考/USB_下位机程序/Projects/config.h` | All `REG_*` register address defines, `CMD_*` command codes, `EXC_CACHE_MAX`, `ENGINEERING_MODE_KEY` |

### Windows PC Application

| File | Key Functions |
|------|-------------|
| `USB参考/ARUN_N3C_WIN上位机/battery_monitor.py` | `_on_mode_change()`, `_write_engineering_data()`, `_write_engineering_worker()`, `_erase_all_records()`, `_erase_worker()`, `_exit_eng_on_device()`, `_hid_write()`, `_hid_read()` |

### Specifications

| File | Content |
|------|---------|
| `.claude/references/specs/WB7720_USB_Bridge_接口规范.md` | I2C register map, command format, report types (v1.3) |
| `.claude/references/specs/USB三方数据格式对照表.md` | Three-party data format cross-reference (v1.3) |
| `USB参考/ARUN_N3C_WIN上位机/ARUN_USB_WINDOWS_PRD_v3.0.md` | Windows app PRD (v4.2) |
