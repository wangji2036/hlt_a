# 工程模式完整技术 Guide
## NU17112 + WB7720 + Windows PC 三方系统

> **项目**: ARUN IP162N 15W PowerBank2 (NU17112 + NU6805 + WB7720)
> **文档版本**: v2.0 | **日期**: 2026-03-02
> **分支**: `ARUN_N3C`
> **生成方式**: powerbank-leader 协调 platform-apl-agent + usb-device-agent + windows-app-agent 联合分析
> **接口规范**: WB7720_USB_Bridge_接口规范 v1.3 | 三方对照表 v1.3

---

## 目录

1. [系统架构概览](#1-系统架构概览)
2. [工程模式进入：完整三层流程](#2-工程模式进入完整三层流程)
3. [虚拟值注入机制](#3-虚拟值注入机制)
4. [虚拟值→异常检测→Flash 写入](#4-虚拟值异常检测flash-写入)
5. [异常记录从 Flash 传递到上位机](#5-异常记录从-flash-传递到上位机)
6. [退出工程模式](#6-退出工程模式)
7. [0xAA 刷新命令（不退出，更新参数）](#7-0xaa-刷新命令不退出更新参数)
8. [0xEE 擦除全部记录](#8-0xee-擦除全部记录)
9. [完整时序图](#9-完整时序图)
10. [关键参数速查表](#10-关键参数速查表)
11. [已知问题与风险](#11-已知问题与风险)
12. [源文件索引](#12-源文件索引)

---

## 1. 系统架构概览

```
┌────────────────────────────────────────────────────────────────────┐
│  Windows PC  (battery_monitor.py)                                  │
│  - 三态模式切换 (用户/工程/生产)    密码: 123456                    │
│  - 工程模式面板: 当前日期/生产日期/循环次数/Cell1/Cell2/温度        │
│  - 异常记录显示: Text 控件，最多 20 条，按 record_id 去重           │
└───────────────────────┬────────────────────────────────────────────┘
                        │ USB HID (64B, 1Hz)
                        │ CMD 0x01 → round-robin (Type 0x01 / Type 0x02)
                        │ CMD 0x0C → 写寄存器
┌───────────────────────▼────────────────────────────────────────────┐
│  WB7720  (USB Bridge, main.c)                                      │
│  i2c_buff[256]: 共享数据区 (I2C Slave @0x42)                       │
│  exc_cache[5][20]: 异常日志本地缓存                                 │
│  user_loop(): CMD 分发，无任何工程模式感知，透传数据                 │
└───────────────────────┬────────────────────────────────────────────┘
                        │ I2C Master @0x42 (NU17112 主动读写)
                        │ 每 47ms 执行一步，18步/周期 = 846ms
┌───────────────────────▼────────────────────────────────────────────┐
│  NU17112  (PowerBank MCU, FML_TASK round-robin)                    │
│  usb_bridge.c: 工程模式逻辑 (进入/维持/退出/刷新/擦除)             │
│  bat_record.c: 异常检测 (OV/OT)，写 Flash，Burst 同步              │
│  Flash 0x1400: BatteryRecordStorage_t (5条 × 20B，~110B)           │
└────────────────────────────────────────────────────────────────────┘
```

### FML_TASK Round-Robin 步骤（846ms/周期，每步 47ms）

| cnt | 函数 | 寄存器 | 职责 |
|-----|------|--------|------|
| 0~10 | 标准遥测写入 | 0x00~0x36 | SOC/电压/电流/温度/循环等 |
| 11 | `usb_bridge_write_exception_counts()` | 0x11~0x16 | OT/OV/OC 计数 |
| 12 | `usb_bridge_write_charge_state()` | 0x17 | 充放电状态 |
| **13** | **`usb_bridge_write_exception_record()`** | **0x37~0x4D** | **轮转写异常记录到 WB7720** |
| **14** | **`usb_bridge_check_engineering_mode()`** | **0x50** | **检测工程模式进入/退出** |
| 15 | `usb_bridge_check_production_mode()` | 0x90 | 生产模式 |
| 16 | `usb_bridge_ensure_product_info()` | 0x92~0xF5 | ProductInfo 补写 |
| **17** | **`usb_bridge_check_eng_test_cmds()`** | **0x88** | **处理 0xAA 刷新 / 0xEE 擦除** |

---

## 2. 工程模式进入：完整三层流程

### 2.1 上位机侧（battery_monitor.py）

**UI 触发路径：**
1. 用户点击连接栏第二行的 **"工程"** Radio Button
2. 弹出密码对话框，输入 `123456`
3. 验证通过，工程模式面板展开
4. 用户填写参数，点击 **"写入设备"**

**后台线程写入序列（`_write_engineering_worker()`）：**

```
Step 1: write_reg(0x50, [0xA5])                       工程模式解锁
Step 2: write_reg(0x60, Y_Lo Y_Hi M D H Mi S)          当前日期时间 (7B)
Step 3: write_reg(0x70, Y_Lo Y_Hi M D)                 生产日期 (4B)
Step 4: write_reg(0x80, cycle_Lo cycle_Hi)             循环次数 (u16 LE)
Step 5: write_reg(0x82, cell1_Lo cell1_Hi)             Cell1电压 (u16 LE, mV)
Step 6: write_reg(0x84, cell2_Lo cell2_Hi)             Cell2电压 (u16 LE, mV)
Step 7: write_reg(0x86, temp_Lo temp_Hi)               虚拟温度 (s16 LE, ×0.1°C)
Step 8: write_reg(0x88, [0xAA])                        刷新触发（最后）
```

每步间隔 `WRITE_STEP_INTERVAL`（约 50ms），总耗时约 400ms。

> ⚠️ **时序要点**：Step 1 写 `0x50=0xA5` 在**最前**，但参数在后续步骤才写入。
> NU17112 首次读到 0xA5 时（cnt=14, 846ms 周期），参数可能还未全部写完（读到旧值或零）。
> **Step 8 的 0xAA 刷新是最终生效触发点**，确保所有参数被重新读取后生效。

**HID 报文格式（CMD_WRITE_REGISTER，65 字节）：**

```
buf[0]  = 0x00      # hidapi 写入前缀（必须）
buf[1]  = 0x0C      # CMD_WRITE_REGISTER
buf[2]  = reg_addr  # 目标寄存器地址
buf[3]  = data_len  # 数据长度
buf[4+] = data...   # 数据字节
buf[N:] = 0x00...   # padding 至 65B
```

---

### 2.2 WB7720 侧（main.c, user_loop）

WB7720 对工程模式**完全透明**，收到 CMD 0x0C 后直接写 i2c_buff：

```c
// main.c — CMD 0x0C 处理
uint8_t reg_addr = Vendor_Request[1];
uint8_t reg_len  = Vendor_Request[2];
if (reg_len > 60) reg_len = 60;           // 防越界
for (uint8_t i = 0; i < reg_len; i++) {
    i2c_buff[reg_addr + i] = Vendor_Request[3 + i];
}
// 响应: update_report_buffer_0() → Type 0x01 遥测（不走 round-robin）
```

> WB7720 不知道"工程模式"，不做任何拦截。写保护由 NU17112 侧 `eng_mode_active` 守卫。

---

### 2.3 NU17112 侧（usb_bridge.c:332）

**`usb_bridge_check_engineering_mode()`，cnt=14，每 846ms 执行一次：**

```c
// usb_bridge.c:332
void usb_bridge_check_engineering_mode(void)
{
    uint8_t work_mode = 0;
    hal_i2cm_read_one_byte(USB_BRIDGE_WB7720_ADDR, REG_WORK_MODE, &work_mode);  // 读 0x50

    if (work_mode == 0xA5) {
        if (!eng_mode_active) {
            /* ===== ENTER 工程模式 ===== */

            // 1. 保存真实值
            eng_saved_rtc_seconds = gd->Bat_RTC_Seconds;
            eng_saved_cycle_count = gd->Battery_cycle_count;

            // 2. 读当前日期时间 (0x60~0x66, 7B) → 转换为秒数 → 更新 gd->Bat_RTC_Seconds
            apply_eng_datetime_to_rtc();
            eng_entry_virtual_seconds = gd->Bat_RTC_Seconds;

            // 3. 读生产日期 (0x70~0x73, 4B) → 格式化 "YYYY-MM-DD" → 写 i2c_buff[0xE2]
            hal_i2cm_read_multi_bytes(..., REG_ENG_PRODUCTION_DATE, date_buf, 4);
            // ... 格式化并写入 REG_PROD_PROD_DATE (0xE2)

            // 4. 一次 8B 读取虚拟参数 (0x80~0x87)
            hal_i2cm_read_multi_bytes(..., REG_ENG_CYCLE_CHG_COUNT, rbuf, 8);
            gd->Battery_cycle_count = rbuf[0];                           // 循环次数 u8
            eng_entry_virtual_cycle = rbuf[0];
            eng_virtual_cell1 = (uint16_t)rbuf[2] | ((uint16_t)rbuf[3] << 8);  // Cell1 mV
            eng_virtual_cell2 = (uint16_t)rbuf[4] | ((uint16_t)rbuf[5] << 8);  // Cell2 mV
            eng_virtual_temp  = (int16_t)((uint16_t)rbuf[6] | ((uint16_t)rbuf[7] << 8)); // 0.1°C

            eng_mode_active = true;
        }
        /* else: Maintain 维持状态，不做任何事，系统用虚拟值运行 */
    } else {
        if (eng_mode_active) {
            /* ===== EXIT 退出工程模式 ===== */
            // 见第6节
        }
    }
}
```

**进入时 static 变量变化：**

| 变量 | 类型 | 进入时 | 用途 |
|------|------|--------|------|
| `eng_mode_active` | `bool` | → `true` | 工程模式激活标志 |
| `eng_saved_rtc_seconds` | `uint32_t` | ← 真实 RTC | 退出时恢复基准 |
| `eng_saved_cycle_count` | `uint8_t` | ← 真实循环数 | 退出时恢复基准 |
| `eng_entry_virtual_seconds` | `uint32_t` | ← 虚拟日期秒数 | 退出时计算增量 |
| `eng_entry_virtual_cycle` | `uint8_t` | ← 虚拟循环数 | 退出时计算增量 |
| `eng_virtual_cell1` | `uint16_t` | ← i2c_buff[0x82~0x83] | OV 检测注入 |
| `eng_virtual_cell2` | `uint16_t` | ← i2c_buff[0x84~0x85] | OV 检测注入 |
| `eng_virtual_temp` | `int16_t` | ← i2c_buff[0x86~0x87] | OT 检测注入 |

---

## 3. 虚拟值注入机制

### 3.1 虚拟值 Getter API（usb_bridge.c:535~537）

```c
uint16_t usb_bridge_get_eng_cell1(void) { return eng_virtual_cell1; }
uint16_t usb_bridge_get_eng_cell2(void) { return eng_virtual_cell2; }
int16_t  usb_bridge_get_eng_temp(void)  { return eng_virtual_temp;  }
bool     usb_bridge_is_eng_mode(void)   { return eng_mode_active;   }
```

**仅 `bat_record.c` 调用这些接口，其他模块不感知工程模式。**

---

### 3.2 OV 过压检测注入（bat_record.c:429~462）

```c
void battery_record_update_overvoltage(void)
{
    uint16_t total_voltage, cell1_voltage, cell2_voltage;

    uint16_t eng_c1 = usb_bridge_get_eng_cell1();
    uint16_t eng_c2 = usb_bridge_get_eng_cell2();

    if (eng_c1 > 0 || eng_c2 > 0) {
        /* 虚拟模式：非零用虚拟值，零用真实总压/2 */
        uint16_t real_total = hal_nu6805_buckboost_get_bat_voltage();
        cell1_voltage = (eng_c1 > 0) ? eng_c1 : (real_total / 2);
        cell2_voltage = (eng_c2 > 0) ? eng_c2 : (real_total / 2);
        total_voltage = cell1_voltage + cell2_voltage;
    } else {
        /* 正常模式：使用真实 ADC */
        total_voltage = hal_nu6805_buckboost_get_bat_voltage();
        cell1_voltage = total_voltage / 2;
        cell2_voltage = total_voltage / 2;
    }

    process_cell_overvoltage(1, cell1_voltage, total_voltage);
    process_cell_overvoltage(2, cell2_voltage, total_voltage);
}
```

**OV 阈值：** `OVER_VOLTAGE_THRESHOLD = 4450 mV`（单节）

**虚拟值矩阵：**

| eng_c1 (mV) | eng_c2 (mV) | cell1 实际用值 | cell2 实际用值 |
|-------------|-------------|--------------|--------------|
| 0 | 0 | 真实/2 | 真实/2 |
| 4500 | 0 | **4500** ← OV触发 | 真实/2 |
| 0 | 4500 | 真实/2 | **4500** ← OV触发 |
| 4500 | 4500 | **4500** | **4500** |

---

### 3.3 OT 过温检测注入（bat_record.c:467~602）

```c
void battery_record_update_temperature(void)
{
    int16_t ntc_temp;

    int16_t eng_temp = usb_bridge_get_eng_temp();
    if (eng_temp != 0) {
        ntc_temp = eng_temp;                        // 虚拟值（非零即用）
    } else {
        ntc_temp = gd->sys_infos.ntc_temp_wpc;     // 真实 NTC（APL_TASK 每100ms更新）
    }

    bool is_abnormal = (ntc_temp > CHRG_NTC_OT_TEMP_VALUE);  // > 600 = 60.0°C
    // ...（后续异常记录状态机）
}
```

**OT 阈值：** `CHRG_NTC_OT_TEMP_VALUE = 600`（单位 0.1°C，即 60.0°C）

> ⚠️ **"零即禁用"约定**：温度用 `!= 0` 判断，意味着无法测试精确 0.0°C，可用 1 (0.1°C) 或 -1 (-0.1°C) 替代。

---

### 3.4 不受影响的路径

以下功能**始终使用真实传感器值**，工程模式对其无影响：

| 功能 | 真实值来源 | 影响 |
|------|-----------|------|
| BuckBoost 充放电 PID | `hal_nu6805_buckboost_get_bat_voltage()` | **无** |
| NTC 温度保护限功 | `gd->sys_infos.ntc_temp_wpc`（APL_TASK 每100ms更新） | **无** |
| 芯片过温保护 | `gd->sys_infos.die_temp` | **无** |
| LED 显示 | 真实 SOC + 充放电状态 | **无** |
| TypeC PD / DPDM 快充 | 各自 ADC | **无** |
| Qi 无线充电 | 各自传感器 | **无** |
| 端口仲裁 | 真实端口状态 | **无** |

---

## 4. 虚拟值→异常检测→Flash 写入

### 4.1 完整调用链

```
APL_TASK (APL_EVT_100ms_POLL, 每 100ms)
  └─ battery_record_periodic_check()              bat_record.c:605
      check_counter++ 至 10 → 每 1s 执行一次
      │
      ├─ battery_record_update_overvoltage()       bat_record.c:429
      │   ├─ 读虚拟值 eng_c1/c2
      │   └─ process_cell_overvoltage(cell_num, cell_v, total_v)
      │       ├─ [首次 OV, cell_v >= 4450]
      │       │   └─ write_exception_record(&record)   bat_record.c:197
      │       │       ├─ record.record_id = g_next_record_id++  (从1开始,单调递增)
      │       │       ├─ g_record_storage.records[write_ptr] = record  (写 RAM)
      │       │       ├─ write_ptr = (write_ptr+1) % MAX_RECORDS  (5条循环)
      │       │       ├─ exception_counter++ (上限 MAX_RECORDS=5)
      │       │       ├─ save_storage_to_flash()          bat_record.c:180
      │       │       │   ├─ checksum = calculate_checksum(...)
      │       │       │   ├─ hal_fmc_erase_page(FLASH_LOG_BASE)   ← 擦除整页512B
      │       │       │   └─ flash_write_record(FLASH_LOG_BASE, ..., ~110B) ← 整体写入
      │       │       └─ usb_bridge_exc_burst(vcnt)        bat_record.c:221
      │       │           → exc_burst_cnt=vcnt, exc_rotate_idx=0, exc_cycle_cnt=0
      │       │
      │       ├─ [持续 OV, 1h 内] 更新 RAM 中 max_voltage，不写 Flash
      │       └─ [持续 OV, 满 1h 且峰值增加] 更新 RAM + save_storage_to_flash()
      │
      └─ battery_record_update_temperature()       bat_record.c:467
          ├─ 读虚拟值 eng_temp
          └─ [首次 OT, ntc_temp > 600] → write_exception_record() → Flash 写入
```

---

### 4.2 Flash 布局

```
Flash 0x1400 (FLASH_LOG_BASE = AP_CFG_ROM_ADDR_LOG):

Offset  Size  字段
0x00    4B    magic          (u32, MAGIC_VALUE)
0x04    1B    exception_counter (u8, 0~MAX_RECORDS)
0x05    1B    write_ptr      (u8, 0~4, 循环写指针)
0x06    2B    padding1
0x08    20B   records[0]     (BatteryExceptionRecord_t)
0x1C    20B   records[1]
0x30    20B   records[2]
0x44    20B   records[3]
0x58    20B   records[4]
0x6C    2B    checksum       (u16, 简单加法校验)
              Total: ~110B，一次整页擦写 (512B)
```

---

### 4.3 BatteryExceptionRecord_t（20 字节，g_data.h:90~106）

```
Offset  Size  字段          说明
0       2B    year          u16 LE, 2026~
2       1B    month         u8
3       1B    day           u8
4       1B    hour          u8
5       1B    minute        u8
6       1B    second        u8
7       1B    reserved      u8
8       1B    error_type    0x01=OV, 0x02=OT, 0x03=UT
9       1B    sub_type      OV=cell_num(1/2), TEMP=charge_state(0=CHG,1=DCHG)
10      2B    data_low      OV=max_voltage(mV,u16), TEMP=max_temperature(0.1°C,s16)
12      2B    data_high     OV=total_voltage(mV,u16), TEMP=reserved
14      2B    pad
16      4B    record_id     u32 LE, 从1开始，上电单调递增
              Total: 20B
```

---

### 4.4 OV 1小时追踪窗口逻辑

```
首次 cell_voltage >= 4450mV:
  → 立即 write_exception_record() → Flash 写入
  → 置 CELL_TRACKING 标志, 记录 hour_start_seconds
  → usb_bridge_exc_burst(vcnt)

1小时内持续 OV:
  → 只更新 RAM 中 g_record_storage.records[last_index].max_voltage
  → 不写 Flash（避免频繁擦写）

1小时到期 且 peak > 已保存值:
  → 更新 RAM + save_storage_to_flash()
  → 重置 hour_start_seconds

电压恢复 (< 4450mV) 且 1小时未到:
  → 保持 is_tracking = true（不立即清除）
  → 等待1小时窗口到期后才清除追踪（防止短暂恢复误清除）

电压恢复 且 1小时已过:
  → 若峰值 > 已保存值，更新 Flash
  → 清除 CELL_TRACKING 标志
```

---

### 4.5 usb_bridge_exc_burst() 机制（usb_bridge.c:548）

**触发时机：**每次调用 `write_exception_record()` 后立即调用，参数为当前有效记录总数。

```c
void usb_bridge_exc_burst(uint8_t count) {
    exc_burst_cnt  = count;   // 设置 burst 计数
    exc_rotate_idx = 0;       // 重置轮转索引
    exc_cycle_cnt  = 0;       // 重置周期计数
}
```

**效果对比：**

| 模式 | 触发条件 | 写入速率 | 适用场景 |
|------|---------|---------|---------|
| 维护轮转 | 正常（无新记录） | `EXC_WRITE_INTERVAL(64)` × 846ms ≈ 54s/条 | 日常保持 |
| Burst 模式 | 新记录写入后 | 846ms/条 | 新异常立即同步 |

---

## 5. 异常记录从 Flash 传递到上位机

### 5.1 NU17112 → i2c_buff（cnt=13，每 846ms）

**`usb_bridge_write_exception_record()`（usb_bridge.c:249）：**

```c
// 检查 burst 模式
if (exc_burst_cnt > 0) {
    exc_burst_cnt--;     // Burst: 跳过 EXC_WRITE_INTERVAL 检查
} else {
    exc_cycle_cnt++;
    if (exc_cycle_cnt < EXC_WRITE_INTERVAL) return;  // 正常模式: 每 64 轮才写一次
    exc_cycle_cnt = 0;
}

// 构建 23 字节负载写入 i2c_buff[0x37~0x4D]
buf[0] = valid_count;       // [0x37] 有效记录总数
buf[1] = exc_rotate_idx;    // [0x38] 当前写入索引
buf[2] = 0x00;              // [0x39] 保留
memcpy(&buf[3], &ap->record_storage.records[exc_rotate_idx], 20);  // [0x3A~0x4D]

hal_i2cm_write_multi_bytes(WB7720_ADDR, REG_EXC_TOTAL_COUNT, buf, 23);

exc_rotate_idx++;
if (exc_rotate_idx >= valid_count) exc_rotate_idx = 0;
```

---

### 5.2 i2c_buff → WB7720 exc_cache（每次 Type 0x01 组装后）

**`exc_cache_update()`（main.c，在 `update_report_buffer_0()` 末尾调用）：**

```c
static void exc_cache_update(void)
{
    // 擦除检测: total_count=0 → 清空缓存
    if (i2c_buff[REG_EXC_TOTAL_COUNT] == 0) {
        if (exc_cache_count > 0) {
            exc_cache_count = 0;
            exc_page_idx = 0;
        }
        return;
    }

    const uint8_t *src = &i2c_buff[REG_EXC_RECORD];  // i2c_buff[0x3A~0x4D]
    uint32_t rid = get_record_id(src);                // src[16~19] u32 LE

    if (rid == 0) return;   // 无效记录兜底

    // 已在缓存中 → 原位更新（处理 max_voltage 实时更新）
    for (uint8_t i = 0; i < exc_cache_count; i++) {
        if (get_record_id(exc_cache[i]) == rid) {
            memcpy(exc_cache[i], src, EXC_RECORD_SIZE);
            return;
        }
    }

    // 不在缓存中
    if (exc_cache_count < EXC_CACHE_MAX) {
        memcpy(exc_cache[exc_cache_count++], src, EXC_RECORD_SIZE);  // 追加
    } else {
        // 缓存已满 (5条): FIFO 覆盖最旧记录
        for (uint8_t i = 0; i < EXC_CACHE_MAX - 1; i++)
            memcpy(exc_cache[i], exc_cache[i+1], EXC_RECORD_SIZE);
        memcpy(exc_cache[EXC_CACHE_MAX - 1], src, EXC_RECORD_SIZE);
    }
}
```

**exc_cache 状态变量：**

| 变量 | 含义 |
|------|------|
| `exc_cache[5][20]` | 最多5条 BatteryExceptionRecord_t 本地缓存 |
| `exc_cache_count` | 当前有效条数 (0~5) |
| `exc_page_idx` | Type 0x02 分页输出游标 (0/1/2，循环) |

---

### 5.3 WB7720 → USB HID（CMD 0x01 round-robin）

**`user_loop()` CMD 0x01 分发规则：**

```c
static uint8_t rr_index = 0;

if (Vendor_Request[0] == CMD_READ_STATUS) {  // 0x01
    if (rr_index >= 4) {
        update_report_buffer_1();   // Type 0x02 异常日志（每5次触发1次）
    } else {
        update_report_buffer_0();   // Type 0x01 遥测
    }
    rr_index++;
    if (rr_index > 4) rr_index = 0;
}
```

**round-robin 节奏（主机 1Hz 发 CMD_READ_STATUS）：**

| 请求次序 | rr_index | 响应 Type | exc_page_idx 变化 |
|---------|----------|----------|-----------------|
| 1 | 0 | 0x01 遥测 | — |
| 2 | 1 | 0x01 遥测 | — |
| 3 | 2 | 0x01 遥测 | — |
| 4 | 3 | 0x01 遥测 | — |
| **5** | **4** | **0x02 异常日志** | **0→1** (Page 0: [0,1]) |
| 6~9 | 0~3 | 0x01 遥测 | — |
| **10** | **4** | **0x02 异常日志** | **1→2** (Page 1: [2,3]) |
| 11~14 | 0~3 | 0x01 遥测 | — |
| **15** | **4** | **0x02 异常日志** | **2→0** (Page 2: [4]) |

---

### 5.4 Type 0x02 异常日志报文格式（`update_report_buffer_1()`）

```
byte[0~2]:   SOF = [0x05, 0xA5, 0x5A]
byte[3]:     Ver = 0x02
byte[4]:     Type = 0x02 (REPORT_TYPE_EXCEPTION_LOG)
byte[5~6]:   Seq (u16 LE) = index++
byte[7~8]:   Len (u16 LE) = ReturnCount × 20 + 2
byte[9]:     OffsetPage (exc_page_idx: 0/1/2)
byte[10]:    ReturnCount (0~2)
byte[11~30]: ExceptionLog[0] (20B, 若 ReturnCount >= 1)
byte[31~50]: ExceptionLog[1] (20B, 若 ReturnCount >= 2)
byte[51~52]: CRC16 (LE)
```

**分页规则：**

| exc_page_idx | page_start | 返回记录 | 最大报文 |
|-------------|-----------|---------|---------|
| 0 | 0 | exc_cache[0], [1] | 9+2+40+2=53B |
| 1 | 2 | exc_cache[2], [3] | 9+2+40+2=53B |
| 2 | 4 | exc_cache[4] | 9+2+20+2=33B |

---

### 5.5 上位机解析和显示

**`_parse_exception_record(raw20)`（battery_monitor.py）：**

```python
year,   = struct.unpack_from('<H', raw20, 0)   # [0:2] 年份
month   = raw20[2]                              # [2] 月
day     = raw20[3]                              # [3] 日
hour, minute, second = raw20[4], raw20[5], raw20[6]
error_type = raw20[8]   # 0x01=OV, 0x02=OT, 0x03=UT
sub_type   = raw20[9]   # OV=cell_num, TEMP=charge_state
data_low,  = struct.unpack_from('<H', raw20, 10)  # OV=max_voltage(mV), TEMP=max_temp(0.1°C)
data_high, = struct.unpack_from('<H', raw20, 12)  # OV=total_voltage(mV)
record_id, = struct.unpack_from('<I', raw20, 16)  # u32 LE
```

**显示格式示例：**
```
[#1] 电芯1 电池充电电压异常 4.51V (总9.01V)[ 2026-03-02 14:30:15 ]
[#2] 电芯1 电池充电温度异常 65.5℃[ 2026-03-02 14:35:00 ]
```

**record_id==0 过滤（battery_monitor.py:1686）：**
```python
for rec in records:
    if rec['record_id'] == 0:   # WB7720 空槽兜底过滤
        continue
```

**去重与峰值更新：**
- `_exception_seen_ids: set` — 已见 record_id 集合，防止重复添加
- `_update_exception_record_in_list()` — 同一 record_id 收到更高 max_voltage 时原位更新显示

---

## 6. 退出工程模式

### 6.1 上位机侧

用户切回 **"用户"** Radio Button：

```python
def _exit_eng_on_device(self):
    payload = bytes([0x50, 1, 0x00])    # reg=0x50, len=1, data=0x00
    self._hid_write(CMD_WRITE_REGISTER, payload)
```

HID 报文：`buf[1]=0x0C, buf[2]=0x50, buf[3]=0x01, buf[4]=0x00`

UI：工程模式面板隐藏（`pack_forget()`），轮询继续，已记录异常不清除。

---

### 6.2 NU17112 侧（usb_bridge.c:403）

**触发条件**：`i2c_buff[0x50] != 0xA5` 且 `eng_mode_active == true`

```c
/* EXIT engineering mode */

// 1. 计算测试期间流逝的增量（虚拟时间内真实流逝的秒数）
uint32_t elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds;

// 2. 恢复真实 RTC + 增量（防止时间倒退）
gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed;

// 3. 计算循环次数增量
uint8_t cycles_added = gd->Battery_cycle_count - eng_entry_virtual_cycle;

// 4. 恢复真实循环次数 + 增量
gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added;

// 5. 从 Flash 重新加载真实 ProductInfo → 写回 i2c_buff[0x92~0xF5]
product_info_read(&info);
if (is_product_info_valid(...)) {
    hal_i2cm_write_multi_bytes(WB7720_ADDR, REG_PROD_MANUFACTURER, &info, 100);
}

// 6. 立即写恢复后的 cycle_count 到 i2c_buff（不等 cnt=5 的下一步）
hal_i2cm_write_multi_bytes(WB7720_ADDR, REG_CYCLE_COUNT, &cycle_val, 2);

// 7. 清除虚拟值
eng_virtual_cell1 = 0;
eng_virtual_cell2 = 0;
eng_virtual_temp  = 0;

// 8. 关闭标志
eng_mode_active = false;
```

**时间增量公式：**
```
退出后 RTC = 保存的真实 RTC + (退出时虚拟 RTC − 进入时虚拟 RTC)
           = eng_saved_rtc_seconds + elapsed
```

---

## 7. 0xAA 刷新命令（不退出，更新参数）

### 7.1 适用场景

工程模式**已激活**时，用户修改虚拟值（如改变电压或温度），再次点击"写入设备"，
末尾的 `write_reg(0x88, [0xAA])` 自动触发刷新，**无需退出再进入**。

### 7.2 NU17112 侧处理（usb_bridge.c:573，cnt=17）

```c
void usb_bridge_check_eng_test_cmds(void)
{
    if (!eng_mode_active) return;   // 写保护：非工程模式直接返回

    uint8_t erase_cmd = 0;
    hal_i2cm_read_one_byte(..., REG_ENG_ERASE_ALL_CMD, &erase_cmd);  // 读 0x88

    if (erase_cmd == 0xAA) {
        /* 1. Delta 结算（同退出逻辑） */
        uint32_t elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds;
        gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed;
        uint8_t cycles_added = gd->Battery_cycle_count - eng_entry_virtual_cycle;
        gd->Battery_cycle_count = eng_saved_cycle_count + cycles_added;

        /* 2. 更新 baseline（为下次结算准备正确的基准） */
        eng_saved_rtc_seconds = gd->Bat_RTC_Seconds;
        eng_saved_cycle_count = gd->Battery_cycle_count;

        /* 3. 重新读虚拟参数（0x80~0x87，8B）*/
        hal_i2cm_read_multi_bytes(..., REG_ENG_CYCLE_CHG_COUNT, rbuf, 8);
        gd->Battery_cycle_count = rbuf[0];
        eng_entry_virtual_cycle = rbuf[0];
        eng_virtual_cell1 = (uint16_t)rbuf[2] | ((uint16_t)rbuf[3] << 8);
        eng_virtual_cell2 = (uint16_t)rbuf[4] | ((uint16_t)rbuf[5] << 8);
        eng_virtual_temp  = (int16_t)((uint16_t)rbuf[6] | ((uint16_t)rbuf[7] << 8));

        /* 4. 重新读当前日期时间 (0x60~0x66) 并更新 RTC */
        apply_eng_datetime_to_rtc();

        /* eng_mode_active 保持 true — 无间隙 */

        hal_i2cm_wirte_one_byte(..., REG_ENG_CMD_STATUS, 0x02);   // 写 0x89=0x02 成功
        hal_i2cm_wirte_one_byte(..., REG_ENG_ERASE_ALL_CMD, 0x00); // 清零 0x88
    }
}
```

### 7.3 0xAA vs EXIT 区别

| 特性 | EXIT（写 0x50=0x00） | 0xAA 刷新（写 0x88=0xAA） |
|------|---------------------|--------------------------|
| 触发函数 | `check_engineering_mode()` cnt=14 | `check_eng_test_cmds()` cnt=17 |
| `eng_mode_active` | → `false` | **保持 `true`** |
| 重新读虚拟参数 | 否（清零） | **是（重读 i2c_buff）** |
| 恢复 ProductInfo | 是（从 Flash 读回） | 否 |
| 用途 | 彻底退出 | **在线刷新参数** |

### 7.4 ⚠️ 已知缺失：未调用 reset_tracking

0xAA 刷新后，`bat_record.c` 的 `g_exception_cache`（tracking 状态）**未被清零**。

**影响**：若旧虚拟温度已触发 OT tracking，刷新为低于阈值的新温度后，
tracking 状态不立即清除（需等 1 小时窗口），导致新参数无法独立评估。

**建议修复**（platform-apl-agent）：在 `apply_eng_datetime_to_rtc()` 后添加：
```c
battery_record_reset_tracking();  // bat_record.c:696
```

---

## 8. 0xEE 擦除全部记录

### 8.1 上位机侧

```python
def _erase_worker(self):
    payload = bytes([0x88, 1, 0xEE])          # reg=0x88, len=1, data=0xEE
    self._hid_write(CMD_WRITE_REGISTER, payload)
    time.sleep(0.5)                            # 等待设备执行
    self._exception_seen_ids.clear()           # 清 PC 端缓存
    self._exception_list.clear()
    self._exception_id_to_index.clear()
    self._refresh_exception_display()          # UI 显示"暂无异常记录"
```

前置条件：已连接 + `_eng_mode_active == True`（写保护）。弹出确认对话框。

### 8.2 NU17112 侧（usb_bridge.c:568，cnt=17）

```c
if (erase_cmd == 0xEE) {
    battery_record_erase_all();   // bat_record.c:677

    hal_i2cm_wirte_one_byte(..., REG_ENG_CMD_STATUS, 0x02);    // 0x89=成功
    hal_i2cm_wirte_one_byte(..., REG_ENG_ERASE_ALL_CMD, 0x00); // 清零 0x88
}
```

**`battery_record_erase_all()`（bat_record.c:677）：**

```c
void battery_record_erase_all(void) {
    memset(&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
    g_record_storage.magic = MAGIC_VALUE;
    save_storage_to_flash();                      // Flash 擦除 + 写空结构
    memset(&g_exception_cache, 0, sizeof(...));   // 清除 tracking 状态
    g_next_record_id = 1;                         // 重置单调 ID
    usb_bridge_exc_burst(1);                      // 推一个空包到 WB7720
}
```

**擦除后各组件状态：**

| 组件 | 状态 |
|------|------|
| NU17112 Flash 0x1400 | magic 有效，counter=0，records 全零 |
| NU17112 RAM g_record_storage | 全零，counter=0 |
| NU17112 RAM g_exception_cache | 全零，tracking 标志清除 |
| i2c_buff[0x37] (total_count) | 下次 cnt=13 写入 0 |
| WB7720 exc_cache | **自动联动**：下次 `exc_cache_update()` 检测 0x37==0 → 清零 |
| PC _exception_list | 已清空 + UI 刷新 |

---

## 9. 完整时序图

```
上位机 (battery_monitor.py)    WB7720 (main.c)         NU17112 (usb_bridge.c + bat_record.c)
─────────────────────────────────────────────────────────────────────────────────────
[T0] 选择"工程"模式
     输入密码 123456
     填写面板参数
     点击"写入设备"
     │
[T1] CMD 0x0C: 0x50=0xA5 ──→  i2c_buff[0x50]=0xA5
[T2] CMD 0x0C: 0x60=日期 ──→  i2c_buff[0x60~0x66]
[T3] CMD 0x0C: 0x70=日期 ──→  i2c_buff[0x70~0x73]
[T4] CMD 0x0C: 0x80=循环 ──→  i2c_buff[0x80~0x81]
[T5] CMD 0x0C: 0x82=C1  ──→   i2c_buff[0x82~0x83]
[T6] CMD 0x0C: 0x84=C2  ──→   i2c_buff[0x84~0x85]
[T7] CMD 0x0C: 0x86=Tmp ──→   i2c_buff[0x86~0x87]
[T8] CMD 0x0C: 0x88=0xAA ──→  i2c_buff[0x88]=0xAA
     │                                                  [等待 ≤846ms，cnt=14]
[T9]                                                    cnt=14: check_engineering_mode()
                                                          读 0x50=0xA5
                                                          ENTER: 保存真实值
                                                          apply_eng_datetime_to_rtc()
                                                          读 0x80~0x87 → eng_virtual_*
                                                          eng_mode_active = true
     │                                                  [等待 ≤846ms，cnt=17]
[T10]                                                   cnt=17: check_eng_test_cmds()
                                                          读 0x88=0xAA
                                                          0xAA REFRESH:
                                                          delta 结算，更新 baseline
                                                          重读 0x80~0x87
                                                          apply_eng_datetime_to_rtc()
                                                          写 0x89=0x02, 清零 0x88
     │
     │ ◄── Type 0x01 (每秒, rr 0~3)                    APL_TASK (100ms × 10 = 每1s):
     │ ◄── Type 0x02 (每5秒, rr 4)                       battery_record_periodic_check()
     │      (若虚拟参数触发异常)                            使用 eng_virtual_* 检测 OV/OT
     │                                                     触发异常 → write_exception_record()
     │                                                              → save_storage_to_flash()
     │                                                              → usb_bridge_exc_burst(n)
     │                                                  cnt=13: write_exception_record()
     │                              exc_cache_update() ← Burst 模式写 0x37~0x4D
     │ ◄── Type 0x02 (Page 0/1/2循环)
     │ 显示异常记录
     │
     [用户切回"用户"模式]
[T-退1] CMD 0x0C: 0x50=0x00 ──→ i2c_buff[0x50]=0x00
     │                                                  [等待 ≤846ms，cnt=14]
[T-退2]                                                 cnt=14: 读 0x50=0x00
                                                          EXIT:
                                                          elapsed 计算 → 恢复真实 gd 字段
                                                          重载真实 ProductInfo
                                                          eng_virtual_* = 0
                                                          eng_mode_active = false
```

---

## 10. 关键参数速查表

### 工程模式寄存器映射（i2c_buff 地址空间）

| 地址 | 名称 | 方向 | 格式 | 写保护 | 说明 |
|------|------|------|------|--------|------|
| 0x50 | REG_WORK_MODE | PC→NU | u8 | 否 | 0xA5=工程, 0x00=用户 |
| 0x60~0x66 | REG_ENG_CURRENT_DATE | PC→NU | Y(LE16)+M+D+H+Mi+S | 是 | 虚拟当前日期时间 |
| 0x70~0x73 | REG_ENG_PRODUCTION_DATE | PC→NU | Y(LE16)+M+D | 是 | 虚拟生产日期 |
| 0x80~0x81 | REG_ENG_CYCLE_COUNT | PC→NU | u16 LE | 是 | 虚拟循环次数 |
| 0x82~0x83 | REG_ENG_VIRTUAL_CELL1 | PC→NU | u16 LE, mV | 是 | 0=禁用 |
| 0x84~0x85 | REG_ENG_VIRTUAL_CELL2 | PC→NU | u16 LE, mV | 是 | 0=禁用 |
| 0x86~0x87 | REG_ENG_VIRTUAL_TEMP | PC→NU | s16 LE, ×0.1°C | 是 | 0=禁用 |
| 0x88 | REG_ENG_ERASE_ALL_CMD | PC→NU | u8 | 是 | 0xEE=擦除, 0xAA=刷新 |
| 0x89 | REG_ENG_CMD_STATUS | NU→PC | u8 | — | 0x02=成功, 0xFF=失败 |
| 0x37 | REG_EXC_TOTAL_COUNT | NU→WB | u8 | — | 有效记录总数 0~5 |
| 0x38 | REG_EXC_CURRENT_IDX | NU→WB | u8 | — | 当前轮转索引 0~4 |
| 0x3A~0x4D | REG_EXC_RECORD | NU→WB | 20B | — | BatteryExceptionRecord_t |

### 关键阈值与常量

| 参数 | 值 | 说明 |
|------|-----|------|
| OV 阈值 | 4450 mV | 单节 OVER_VOLTAGE_THRESHOLD |
| OT 阈值 | 600 | 单位 0.1°C，即 60.0°C |
| MAX_RECORDS | 5 | Flash 最多存 5 条 |
| EXC_CACHE_MAX | 5 | WB7720 最多缓存 5 条 |
| EXC_WRITE_INTERVAL | 64 | 正常轮转间隔（≈54s/条） |
| Burst 写入速率 | 846ms/条 | 新记录后立即同步 |
| Round-robin 周期 | 846ms | 18步 × 47ms |
| Type 0x02 出现频率 | 1/5 次 CMD_READ_STATUS | rr_index==4 时 |
| 端到端延迟 | 5~15s | 虚拟参数设置到 PC 显示异常 |
| 参数刷新生效时间 | ≤1.7s | PC 写 0xAA 到 NU17112 重读 |

---

## 11. 已知问题与风险

| ID | 描述 | 风险 | 状态 |
|----|------|------|------|
| **[NEW-1]** | **0xAA 刷新未调用 `battery_record_reset_tracking()`，新参数无法独立评估** | **MEDIUM** | **建议修复** |
| [L-011] | `save_storage_to_flash()` 缺少 `VIC_vModuleDisable()` 临界区保护 | MEDIUM | 建议修复 |
| [H-002] | 0xEE 擦除后 WB7720 exc_cache 需等下次 `update_report_buffer_0()` 才清零（≤1s 延迟） | LOW | 待验证 |
| [H-003] | I2C 通信瞬断（NU17112 读到 0x50≠0xA5）可能触发虚假退出 | MEDIUM | 待验证 |
| [H-004] | 0xAA 刷新后 delta 叠加在重入路径是否正确 | LOW | 待验证 |
| [L-015] | Cell Voltage 遥测显示异常（Cell1=总电压，Cell2≈0） | P1 | 观察中 |
| [设计限制] | 循环次数为 u8，虚拟值 >255 被截断 | LOW | 已知设计限制 |
| [设计限制] | 温度 0 不可测试（用 1 或 -1 替代） | LOW | 已知设计限制 |

---

## 12. 源文件索引

### NU17112 固件

| 文件 | 关键函数 | 行号 |
|------|---------|------|
| `app/usb_bridge.c` | `usb_bridge_check_engineering_mode()` | :332 |
| | `usb_bridge_check_eng_test_cmds()` | :561 |
| | `usb_bridge_write_exception_record()` | :249 |
| | `usb_bridge_exc_burst()` | :548 |
| | `eng_virtual_cell1/cell2/temp` | :41~43 |
| `app/usb_bridge.h` | `REG_ENG_*`, `EXC_WRITE_INTERVAL` | 寄存器宏定义 |
| `app/bat_record.c` | `battery_record_periodic_check()` | :605 |
| | `battery_record_update_overvoltage()` | :429 |
| | `battery_record_update_temperature()` | :467 |
| | `write_exception_record()` (static) | :197 |
| | `save_storage_to_flash()` (static) | :180 |
| | `battery_record_erase_all()` | :677 |
| | `battery_record_reset_tracking()` | :696 |
| `app/bat_record.h` | `OVER_VOLTAGE_THRESHOLD`, `CHRG_NTC_OT_TEMP_VALUE` | 阈值定义 |
| `fml/g_data.h` | `BatteryExceptionRecord_t` | :90~106 |
| | `gd->Bat_RTC_Seconds`, `gd->Battery_cycle_count` | :536~531 |
| `fml/_fml.c` | `ubsd_wb7720_report_update()` round-robin | cnt=13/14/17 |

### WB7720 下位机固件

| 文件 | 关键函数 | 说明 |
|------|---------|------|
| `USB参考/USB_下位机程序/Projects/config.h` | `REG_ENG_*`, `EXC_CACHE_MAX`, `CMD_*` | 寄存器和命令宏 |
| `USB参考/USB_下位机程序/Projects/main.c` | `user_loop()` | CMD 0x0C 写寄存器 |
| | `exc_cache_update()` | 异常记录去重缓存 |
| | `update_report_buffer_0()` | Type 0x01 + exc_cache_update |
| | `update_report_buffer_1()` | Type 0x02 分页 |
| | `update_report_buffer_device_info()` | Type 0x03 设备信息 |

### Windows 上位机

| 文件 | 关键函数 | 说明 |
|------|---------|------|
| `USB参考/ARUN_N3C_WIN上位机/battery_monitor.py` | `_on_mode_change()` | 三态模式切换+密码 |
| | `_write_engineering_worker()` | 8步写入序列 |
| | `_erase_all_records()` + `_erase_worker()` | 擦除功能 |
| | `_exit_eng_on_device()` | 退出通知 |
| | `_parse_exception_record()` | 20B 记录解析 |
| | `_append_exception_logs()` | 去重追加 + record_id==0 过滤 |

### 参考文档

| 文档 | 路径 |
|------|------|
| WB7720 接口规范 | `.claude/references/specs/WB7720_USB_Bridge_接口规范.md` |
| 三方数据格式对照表 | `.claude/references/specs/USB三方数据格式对照表.md` |
| 原版工程模式说明 | `工程模式工作原理及系统影响说明.md` |

---

*本文档由 **powerbank-leader** 协调以下 Agent 联合分析生成：*
*platform-apl-agent (NU17112 侧) · usb-device-agent (WB7720 侧) · windows-app-agent (PC 侧)*
