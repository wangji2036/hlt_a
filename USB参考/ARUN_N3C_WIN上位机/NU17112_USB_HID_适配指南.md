# NU17112 USB HID 适配指南

**目的**: 将 ARUN IP162N PowerBank (NU17112 + NU6805) 的固件数据映射到 USB HID 上位机协议
**日期**: 2026-02-26
**前置文档**: `ARUN_USB_WINDOWS_PRD_v3.0.md`

---

## 1. 系统架构

```
┌─────────────────────────────────────────────────┐
│               Windows PC                         │
│   ARUN 上位机 (battery_monitor.py)           │
│   ← 64B HID Report (1Hz) →                     │
└───────────────────┬─────────────────────────────┘
                    │ USB HID
┌───────────────────▼─────────────────────────────┐
│   WB7720 USB MCU (USB Bridge)                    │
│   固件: NANFU_USB_1052_20260121A                │
│   功能: I2C 寄存器 → HID Report 组装            │
│   ← I2C Slave (0x42) →                         │
└───────────────────┬─────────────────────────────┘
                    │ I2C Bus
┌───────────────────▼─────────────────────────────┐
│   NU17112 主控 (I2C Master)                      │
│   + NU6805 BuckBoost IC                          │
│   + Gauge BMS 算法                               │
│   + 2S Li-ion Battery (4400mV CV)               │
│   功能: 采集电池数据 → 写入 I2C 寄存器          │
└─────────────────────────────────────────────────┘
```

**关键接口**: NU17112 作为 I2C Master，定期将电池遥测数据写入 WB7720 的 I2C Slave 寄存器映射区 (i2c_buff[256])。WB7720 收到上位机 HID 请求后，从 i2c_buff 组装 64B 报告返回。

---

## 2. 数据映射表: NU17112 固件 → WB7720 I2C 寄存器

### 2.1 遥测数据映射 (Type 0x01)

| I2C 地址 | HID 字段 | NU17112 数据源 | 类型 | 原始单位 | 写入处理 |
|----------|---------|---------------|------|---------|---------|
| **0x00** | SOC | `gd->real_soc_show` | u8 | % | 直写 |
| **0x01-0x04** | Capacity | 配置常量 (mAh) | u32 LE | mAh | 直写 (如 10000mAh) |
| **0x05-0x06** | VBAT_mV | `g_buckboost.adc_vbat` | u16 LE | mV | 直写 (WB7720 做 /10) |
| **0x07-0x08** | IBAT_mA | `g_buckboost.adc_ibat` | s16 LE | mA | 直写 (WB7720 做 /10) |
| **0x09-0x0A** | TEMP_DC | `gd->sys_infos.ntc_temp_wpc` | s16 LE | 0.1C | 直写 |
| **0x0B-0x0C** | CycleCount | `gd->Battery_cycle_count` | u16 LE | 次 | u8 扩展为 u16 |
| **0x0D-0x0E** | R_internal | `gd->Bat_Rdc` | u16 LE | mohm | u8 扩展为 u16 |
| **0x0F-0x10** | SOH | `gd->Bat_SoH` | u16 LE | 0.01% | int8 转 u16, x100 |
| **0x11-0x12** | ErrOverTempCnt | 从 `record_storage` 统计 | u16 LE | 次 | 遍历异常日志计数 |
| **0x13-0x14** | ErrOverVoltCnt | 从 `record_storage` 统计 | u16 LE | 次 | 遍历异常日志计数 |
| **0x15-0x16** | ErrOverCurrCnt | (保留, 当前无 OCP 日志) | u16 LE | 次 | 写 0 |
| **0x17** | ChargeState | `g_buckboost.woke_mode` | u8 | enum | 0=待机,1=充,2=放 |
| **0x2D-0x2E** | Cell1_mV | `g_buckboost.adc_vbat / 2` | u16 LE | mV | 2S 串联均分* |
| **0x2F-0x30** | Cell2_mV | `g_buckboost.adc_vbat / 2` | u16 LE | mV | 2S 串联均分* |
| **0x35-0x36** | BoardTemp | `gd->sys_infos.ntc_temp_typec` | s16 LE | 0.1C | 直写 |

> \* **电芯电压说明**: NU6805 仅提供总电池电压 (2S 串联)，无法获取单节电压。当前方案为均分 (vbat/2)。如需精确单节电压，需外加 BMS IC 或 gauge 输出。

### 2.2 工程模式寄存器映射

| I2C 地址 | 寄存器 | NU17112 处理 |
|----------|--------|-------------|
| **0x50** | WORK_MODE | WB7720 本地处理 (解锁标志) |
| **0x60-0x63** | ENG_CURRENT_DATE | NU17112 读取后存入 Flash (`ProductInfo_t.battery_prod_date` 或自定义区) |
| **0x70-0x73** | ENG_PRODUCTION_DATE | NU17112 读取后存入 Flash |
| **0x80-0x81** | ENG_CYCLE_CHG_COUNT | NU17112 读取后写入 `gd->Battery_cycle_count` |

### 2.3 产品信息映射

| HID Brand 字段 | NU17112 数据源 | 说明 |
|---------------|---------------|------|
| Brand 字符串 | `ProductInfo_t.model_name` | Flash 0x1814, 最多 20 字符 |
| (扩展) Manufacturer | `ProductInfo_t.manufacturer_name` | Flash 0x1800 |
| (扩展) Battery MFR | `ProductInfo_t.battery_mfr` | Flash 0x1828 |

### 2.4 生产模式寄存器映射 (新增)

上位机生产模式通过 CMD_WRITE_REGISTER (0x0C) 将设备基本信息写入 WB7720 I2C 寄存器，NU17112 检测到后读取并写入 Flash。

| I2C 地址 | 寄存器名 | 长度 | 方向 | NU17112 处理 |
|----------|---------|------|------|-------------|
| **0x90** | PROD_MODE_FLAG | 1B | PC→WB7720→NU17112 | 轮询检测: 0xB5=进入生产模式 |
| **0x91** | PROD_WRITE_STATUS | 1B | NU17112→WB7720→PC | 写入状态回传 (0x01=进行中, 0x02=成功, 0xFF=失败) |
| **0x92-0xA5** | PROD_MANUFACTURER | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.manufacturer_name` (Flash 0x1800+0) |
| **0xA6-0xB9** | PROD_MODEL | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.model_name` (Flash 0x1800+20) |
| **0xBA-0xCD** | PROD_BATTERY_MFR | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_mfr` (Flash 0x1800+40) |
| **0xCE-0xE1** | PROD_BATTERY_MODEL | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_model` (Flash 0x1800+60) |
| **0xE2-0xF5** | PROD_PROD_DATE | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_prod_date` (Flash 0x1800+80) |

**NU17112 侧处理伪代码:**

```c
// 在 usb_bridge_update() 中追加生产模式检测 (每 1s 执行)
void usb_bridge_check_production_mode(void)
{
    uint8_t flag = 0;
    hal_i2c_master_read(WB7720_I2C_ADDR, 0x90, &flag, 1);

    if (flag != 0xB5) return;  // 未进入生产模式

    // 1. 设置状态 = 写入中
    uint8_t status = 0x01;
    hal_i2c_master_write(WB7720_I2C_ADDR, 0x91, &status, 1);

    // 2. 读取 100 字节 ProductInfo
    ProductInfo_t info;
    hal_i2c_master_read(WB7720_I2C_ADDR, 0x92, (uint8_t*)&info, sizeof(ProductInfo_t));

    // 3. 校验 (至少一个字段非空)
    if (info.manufacturer_name[0] == 0x00 && info.model_name[0] == 0x00) {
        status = 0xFF;  // 校验失败
        hal_i2c_master_write(WB7720_I2C_ADDR, 0x91, &status, 1);
        flag = 0x00;
        hal_i2c_master_write(WB7720_I2C_ADDR, 0x90, &flag, 1);
        return;
    }

    // 4. 写入 Flash (擦除 0x1800 页 → 写入 100B → 版本标记)
    product_info_write(&info);

    // 5. 回读验证
    ProductInfo_t verify;
    product_info_read(&verify);
    if (memcmp(&info, &verify, sizeof(ProductInfo_t)) == 0) {
        status = 0x02;  // 写入成功
    } else {
        status = 0xFF;  // 验证失败
    }
    hal_i2c_master_write(WB7720_I2C_ADDR, 0x91, &status, 1);

    // 6. 清除生产模式标志
    flag = 0x00;
    hal_i2c_master_write(WB7720_I2C_ADDR, 0x90, &flag, 1);

    printk("ProductInfo write %s\n", status == 0x02 ? "OK" : "FAIL");
}
```

---

## 3. NU17112 固件新增模块: USB Bridge Task

### 3.1 设计概要

在 NU17112 固件中新增一个 **周期性 I2C 写入任务**，将实时数据打包写入 WB7720 的 I2C 寄存器。

```c
// 建议: 在 APL_TASK 的 100ms 轮询中追加 USB Bridge 更新
// app/app.c: APL_EVT_100ms_POLL 处理末尾

case APL_EVT_100ms_POLL:
    // ... 现有温度采集 ...
    gd->sys_infos.ntc_temp_typec = fml_ntc_temp_get_typec();
    gd->sys_infos.ntc_temp_wpc   = fml_ntc_temp_get_wpc();
    fml_tntc_otp_limit_power(gd->sys_infos.ntc_temp_wpc);
    battery_record_periodic_check();

    // [新增] USB Bridge 数据更新 (每 1s 一次即可)
    usb_bridge_update();   // ← 新增函数
    break;
```

### 3.2 usb_bridge_update() 伪代码

```c
#include "hal/i2c.h"  // I2C Master API

#define WB7720_I2C_ADDR  0x42   // WB7720 I2C Slave 地址
#define USB_BRIDGE_UPDATE_INTERVAL_MS  1000  // 1 秒更新一次

static uint32_t last_update_tick = 0;

void usb_bridge_update(void)
{
    uint32_t now = osal_get_system_tick();
    if (now - last_update_tick < USB_BRIDGE_UPDATE_INTERVAL_MS) return;
    last_update_tick = now;

    uint8_t buf[64] = {0};

    // === 基础遥测 ===
    buf[0x00] = gd->real_soc_show;                              // SOC %

    uint32_t capacity = 10000;  // ARUN IP162N 额定容量 mAh (根据产品规格)
    memcpy(&buf[0x01], &capacity, 4);                           // Capacity u32 LE

    uint16_t vbat = g_buckboost.adc_vbat;
    memcpy(&buf[0x05], &vbat, 2);                               // VBAT mV

    int16_t ibat = g_buckboost.adc_ibat;
    memcpy(&buf[0x07], &ibat, 2);                               // IBAT mA (signed)

    int16_t temp_wpc = gd->sys_infos.ntc_temp_wpc;
    memcpy(&buf[0x09], &temp_wpc, 2);                           // Battery Temp (0.1C)

    uint16_t cycle = (uint16_t)gd->Battery_cycle_count;
    memcpy(&buf[0x0B], &cycle, 2);                              // Cycle Count

    uint16_t rdc = (uint16_t)gd->Bat_Rdc;
    memcpy(&buf[0x0D], &rdc, 2);                                // Internal Resistance mohm

    uint16_t soh = (uint16_t)(gd->Bat_SoH > 0 ? gd->Bat_SoH * 100 : 0);
    memcpy(&buf[0x0F], &soh, 2);                                // SOH (0.01% unit)

    // === 异常计数 (从 CCC 日志统计) ===
    uint16_t ot_cnt = 0, ov_cnt = 0, oc_cnt = 0;
#if CONFIG_NEW_CCC_LOG_ENABLE
    usb_bridge_count_exceptions(&ot_cnt, &ov_cnt, &oc_cnt);
#endif
    memcpy(&buf[0x11], &ot_cnt, 2);                             // Over-Temp Count
    memcpy(&buf[0x13], &ov_cnt, 2);                             // Over-Volt Count
    memcpy(&buf[0x15], &oc_cnt, 2);                             // Over-Curr Count

    buf[0x17] = g_buckboost.woke_mode;                          // Charge State

    // === 电芯电压 (2S 均分) ===
    uint16_t cell_v = vbat / 2;
    memcpy(&buf[0x2D], &cell_v, 2);                             // Cell1
    cell_v = vbat - cell_v;  // 余数给 cell2
    memcpy(&buf[0x2F], &cell_v, 2);                             // Cell2

    // === 板温 ===
    int16_t temp_tc = gd->sys_infos.ntc_temp_typec;
    memcpy(&buf[0x35], &temp_tc, 2);                            // Board Temp (0.1C)

    // === I2C 批量写入 WB7720 ===
    hal_i2c_master_write(WB7720_I2C_ADDR, 0x00, buf, 0x37);
}
```

### 3.3 异常计数统计函数

```c
#if CONFIG_NEW_CCC_LOG_ENABLE
void usb_bridge_count_exceptions(uint16_t *ot, uint16_t *ov, uint16_t *oc)
{
    *ot = 0; *ov = 0; *oc = 0;
    BatteryRecordStorage_t *rs = &ap->record_storage;

    for (uint8_t i = 0; i < rs->write_ptr && i < MAX_RECORDS; i++) {
        switch (rs->records[i].error_type) {
            case 0x02: (*ot)++; break;  // Overtemp
            case 0x01: (*ov)++; break;  // Overvoltage
            case 0x03: (*oc)++; break;  // (Reserved for future OCP)
        }
    }
}
#endif
```

---

## 4. Gap 分析: 当前状态 vs PRD 要求

### 4.1 数据可用性

| PRD 要求字段 | NU17112 当前状态 | Gap | 优先级 |
|-------------|-----------------|-----|--------|
| SOC (%) | `gd->real_soc_show` | 可用 | - |
| Capacity (mAh) | 无实时字段, 需配置常量 | **需定义** | P1 |
| Total Voltage (mV) | `g_buckboost.adc_vbat` | 可用 | - |
| Total Current (mA) | `g_buckboost.adc_ibat` | 可用 (signed) | - |
| Power (W) | 无直接字段 | **WB7720 计算** (V x I) | P2 |
| Charge State | `g_buckboost.woke_mode` | 可用 (0/1/2 匹配) | - |
| Cycle Count | `gd->Battery_cycle_count` | 可用 (u8, 范围 0-255) | P3* |
| Battery Temp | `gd->sys_infos.ntc_temp_wpc` | 可用 (0.1C) | - |
| Board Temp | `gd->sys_infos.ntc_temp_typec` | 可用 (0.1C) | - |
| Cell Count | 固定 2 | **硬编码** | - |
| Cell1/2 Voltage | 无单节电压, 仅总电压 | **需均分估算** | P2 |
| Internal Resistance | `gd->Bat_Rdc` | 可用 (u8 mohm) | - |
| SOH (%) | `gd->Bat_SoH` | 可用 (int8) | - |
| Over-Temp Count | 从 CCC Log 统计 | **需计数函数** | P1 |
| Over-Volt Count | 从 CCC Log 统计 | **需计数函数** | P1 |
| Over-Curr Count | CCC Log 暂无 OCP 类型 | **预留为 0** | P3 |
| Exception Log Count | `record_storage.exception_counter` | 可用 | - |
| Brand String | `ProductInfo_t.model_name` | 可用 (Flash 0x1814) | - |
| Exception Logs (Type 0x02) | `record_storage.records[]` | **需序列化** | P1 |

> \* **Cycle Count 限制**: `gd->Battery_cycle_count` 是 u8 (0-255)，而 HID 协议定义为 u16 (0-65535)。当前硬件规格下 u8 足够 (充电宝寿命 ~500 次)，但工程模式写入可能需要 u16。

### 4.2 通信接口 Gap

| 项目 | 当前状态 | 需要 | Gap |
|------|---------|------|-----|
| I2C Master → WB7720 | NU17112 有 I2C Master HAL | 定期写入遥测数据 | **需新增 usb_bridge 模块** |
| I2C Slave (WB7720 读取) | WB7720 有 I2C Slave (0x42) | 被动接收数据 | 已实现 |
| 工程模式写回 | WB7720 → NU17112 | 日期/循环次数写入 Flash | **需新增回写通道** |

### 4.3 异常日志序列化 Gap

WB7720 的 Type 0x02 异常日志格式 (12B/条):

```
Year(u16) Month(u8) Day(u8) Hour(u8) Min(u8) Sec(u8) ErrType(u8) Value(s32)
```

NU17112 的 CCC 异常日志格式 (20B/条, `BatteryExceptionRecord_t`):

```
TimeStamp(8B) error_type(u8) sub_type(u8) data(6B) record_id(u32)
```

**转换需求**:
- TimeStamp_t → Year/Month/Day/Hour/Min/Sec (直接映射)
- error_type: 0x01(OV) → ErrType 2(过压), 0x02(OT) → ErrType 0(过温)
- data.ov_data.max_voltage → Value x 100 (mV → V x100)
- data.temp_data.max_temperature → Value x 10 (0.1C → C x100)

---

## 5. 实施任务分解

### Phase 1: 基础遥测 (P1 — 优先实现)

| 任务 | 负责 Agent | 文件 | 工作量 |
|------|-----------|------|--------|
| T1.1 新建 `app/usb_bridge.c` + `.h` | **platform-apl-agent** | 新文件 | 中 |
| T1.2 实现 `usb_bridge_update()` 遥测数据打包 | **platform-apl-agent** | usb_bridge.c | 中 |
| T1.3 在 APL_EVT_100ms_POLL 中调用 | **platform-apl-agent** | app/app.c | 小 |
| T1.4 添加到构建系统 | **platform-apl-agent** | Debug/app/subdir.mk | 小 |
| T1.5 定义 Capacity 配置常量 | **platform-apl-agent** | app/config.h | 小 |
| T1.6 实现异常计数统计函数 | **platform-apl-agent** | usb_bridge.c | 小 |

### Phase 2: 异常日志序列化 (P1)

| 任务 | 负责 Agent | 文件 | 工作量 |
|------|-----------|------|--------|
| T2.1 实现 CCC Log → HID 12B 格式转换 | **platform-apl-agent** | usb_bridge.c | 中 |
| T2.2 实现 I2C 批量写入日志数据 | **platform-apl-agent** | usb_bridge.c | 小 |
| T2.3 WB7720 侧: 从 I2C 读取日志数据 | **下位机固件** (WB7720) | usbd_user_hid.c | 中 |

### Phase 3: 工程模式回写 (P2)

| 任务 | 负责 Agent | 文件 | 工作量 |
|------|-----------|------|--------|
| T3.1 WB7720 → NU17112 工程数据中转 | **platform-apl-agent** | usb_bridge.c | 中 |
| T3.2 日期写入 Flash (ProductInfo 扩展) | **platform-fml-agent** | fml/g_data.c | 小 |
| T3.3 循环次数覆写 | **platform-buckboost-agent** | power/buckboost.c | 小 |

### Phase 3b: 生产模式 — ProductInfo 写入 (P2, 新增)

| 任务 | 负责 Agent | 文件 | 工作量 |
|------|-----------|------|--------|
| T3b.1 实现 `usb_bridge_check_production_mode()` | **platform-apl-agent** | usb_bridge.c | 中 |
| T3b.2 在 `usb_bridge_update()` 中调用生产模式检测 | **platform-apl-agent** | usb_bridge.c | 小 |
| T3b.3 WB7720 侧: 0x90-0xF5 寄存器透传 | **下位机固件** (WB7720) | usbd_user_hid.c | 小 |
| T3b.4 生产模式 Flash 写入端到端调试 | **platform-apl-agent** | usb_bridge.c | 中 |

### Phase 4: 上位机适配 (P2)

| 任务 | 负责 | 文件 | 工作量 |
|------|------|------|--------|
| T4.1 按 PRD v3.2 UI 重构 | 上位机开发 | battery_monitor.py | 大 |
| T4.2 异常日志 (Type 0x02) 解析实现 | 上位机开发 | battery_monitor.py | 中 |
| T4.3 工程模式 UI 完善 | 上位机开发 | battery_monitor.py | 小 |
| T4.4 生产模式 UI + 写入序列实现 | 上位机开发 | battery_monitor.py | 中 |
| T4.5 三态模式切换 (用户/工程/生产) | 上位机开发 | battery_monitor.py | 小 |

### Phase 5: 集成测试 (P1)

| 任务 | 负责 | 说明 |
|------|------|------|
| T5.1 I2C 通信验证 | 硬件测试 | NU17112 ↔ WB7720 数据一致性 |
| T5.2 HID 数据端到端验证 | 硬件测试 | 上位机显示 vs 万用表实测 |
| T5.3 异常日志回溯验证 | 硬件测试 | 触发 OV/OT → 上位机显示 |
| T5.4 工程模式写入验证 | 硬件测试 | 写入日期/循环 → 断电重启 → 验证持久化 |
| T5.5 生产模式写入验证 | 硬件测试 | 写入 5 字段 ProductInfo → 断电重启 → 验证 Flash 0x1800 持久化 |
| T5.6 生产模式状态回读验证 | 硬件测试 | STATUS=0x02 成功, 0xFF 失败 → 上位机显示正确 |

---

## 6. I2C 通信时序

### 6.1 遥测数据更新 (NU17112 → WB7720, 每 1 秒)

```
NU17112 (Master)                    WB7720 (Slave @ 0x42)
    │                                    │
    │── START ──────────────────────────→│
    │── ADDR(0x42) + W ─────────────────→│ ACK
    │── REG_ADDR(0x00) ─────────────────→│ ACK
    │── DATA[0] (SOC) ──────────────────→│ ACK
    │── DATA[1..4] (Capacity) ──────────→│ ACK x4
    │── DATA[5..6] (VBAT) ─────────────→│ ACK x2
    │── ... (连续写入 0x37 字节) ────────→│
    │── STOP ───────────────────────────→│
    │                                    │
    │  (WB7720 收到上位机 CMD 0x01 后)    │
    │                                    │── 从 i2c_buff 组装 HID Report
    │                                    │── USB IN: 64B Report → PC
```

### 6.2 工程数据回写 (WB7720 → NU17112, 按需)

```
方案 A: WB7720 设置标志位, NU17112 轮询读取
─────────────────────────────────────────────
NU17112 (Master)                    WB7720 (Slave)
    │                                    │
    │  (每 1s 轮询一次)                   │
    │── READ REG[0x50] ─────────────────→│  返回 WORK_MODE
    │                                    │
    │  if WORK_MODE == 0xA5:             │
    │── READ REG[0x60..0x63] ───────────→│  返回当前日期
    │── READ REG[0x70..0x73] ───────────→│  返回生产日期
    │── READ REG[0x80..0x81] ───────────→│  返回循环次数
    │                                    │
    │── 写入 Flash ──────────────────────│
    │── WRITE REG[0x50] = 0x00 ─────────→│  清除工程模式标志
```

---

## 7. 配置参数清单

需在 `app/config.h` 中新增:

```c
// === USB Bridge Configuration ===
#define CONFIG_USB_BRIDGE_ENABLE         1       // 总开关
#define CONFIG_USB_BRIDGE_UPDATE_MS      1000    // 更新间隔 (ms)
#define CONFIG_USB_BRIDGE_I2C_ADDR       0x42    // WB7720 I2C 地址
#define CONFIG_BATTERY_CAPACITY_MAH      10000   // 额定容量 (mAh)
#define CONFIG_BATTERY_CELL_COUNT        2       // 电芯串数
#define CONFIG_USB_BRIDGE_BRAND          "IP162N" // 品牌字符串
```

---

## 8. 风险与缓解

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| I2C 总线冲突 | NU6805 和 WB7720 共用 I2C | 使用不同 I2C 外设 或 时分复用 |
| 单节电压不可获取 | 显示值为估算 | UI 标注"估算值", 或外加 BMS IC |
| Cycle Count u8 溢出 | 工程模式写入 >255 | 扩展为 u16 持久化 (Flash) |
| I2C 写入耗时 | 占用 APL_TASK 时间 | 拆分多次写入 或 DMA 传输 |
| 异常日志同步延迟 | 日志更新不及时 | 异常触发时立即追加写入 |
| WB7720 固件差异 | 参考固件可能需定制 | 先验证 I2C 寄存器映射兼容性 |
| 生产模式 Flash 断电丢失 | `product_info_write()` 擦页后断电导致全页丢失 | 产线确保写入期间供电稳定; 未来可加双缓冲 |
| 生产模式并发冲突 | 遥测写入与生产模式写入同时操作 I2C | 在 usb_bridge_update() 中互斥: 检测到 PROD_MODE_FLAG 时跳过遥测写入 |

---

## 9. 验收标准

### 9.1 基础遥测 (Phase 1)

- [ ] 上位机显示 SOC 与固件 `gd->real_soc_show` 一致
- [ ] 电压误差 < 50mV (vs 万用表)
- [ ] 电流方向正确 (充电正, 放电负)
- [ ] 温度误差 < 2C (vs 温度枪)
- [ ] 充电/放电/待机状态切换正确

### 9.2 异常日志 (Phase 2)

- [ ] 触发过压后，上位机异常计数 +1
- [ ] 触发过温后，上位机异常计数 +1
- [ ] 异常日志表显示正确时间戳和数值
- [ ] 断电重启后日志仍可读取

### 9.3 工程模式 (Phase 3)

- [ ] 写入日期后，断电重启保持
- [ ] 写入循环次数后，上位机显示更新
- [ ] 工程模式不干扰正常遥测刷新

### 9.4 生产模式 (Phase 3b)

- [ ] 上位机 5 字段输入校验正确 (空值/超长/非 ASCII 均拒绝)
- [ ] 7 步写入序列 HEX dump 与 PRD F6.6 规格一致
- [ ] NU17112 检测到 PROD_MODE_FLAG=0xB5 后自动读取 100 字节
- [ ] `product_info_write()` 成功写入 Flash 0x1800
- [ ] 回读验证: `product_info_read()` 返回值与写入一致
- [ ] PROD_WRITE_STATUS 正确回传 (0x02 成功 / 0xFF 失败)
- [ ] 断电重启后，`product_info_print()` 输出与写入值一致
- [ ] 生产模式不干扰正常遥测刷新
- [ ] Flash 写入期间断电恢复: 重新进入生产模式可覆写
