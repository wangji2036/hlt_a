# WB7720 USB Bridge 接口规范

**版本**: v1.2
**日期**: 2026-02-26
**管理者**: usb-device-agent
**固件版本**: NANFU_USB_1052_20260121A

> 本文档是 WB7720 USB Bridge 固件的**权威接口文档**，供以下三方使用：
> - **NU17112 固件开发者**: 了解如何通过 I2C 向 WB7720 写入数据 / 读取工程参数
> - **Windows 上位机开发者**: 了解 USB HID 命令格式和回复结构
> - **下位机固件开发者**: 实现本规范定义的所有接口

---

## 1. 系统概述

WB7720 是一个 USB HID Bridge MCU，提供两个接口：

| 接口 | 方向 | 对端 | 协议 |
|------|------|------|------|
| I2C Slave (地址 0x42) | 双向 | NU17112 (I2C Master) | I2C, 标准模式 |
| USB HID | 双向 | Windows PC | USB HID, 64B 报告 |

**核心机制**: WB7720 内部维护 256 字节缓冲区 `i2c_buff[256]`，作为 I2C 和 HID 之间的共享数据交换区。

```
NU17112 ──I2C WRITE──▶ i2c_buff ──HID IN──▶ Windows PC
NU17112 ◀──I2C READ── i2c_buff ◀──HID OUT── Windows PC
```

---

## 2. USB HID 接口

### 2.1 设备描述符

| 参数 | 值 |
|------|---|
| VID | `0xFFFF` |
| PID | `0xFFFF` |
| Usage Page | `0xFF00` (厂商自定义) |
| Usage ID | `0x01` |
| 报告长度 | 64 字节 |
| Report ID | 无（hidapi 写入需加 `0x00` 前缀，实际 65 字节） |

### 2.2 上位机 → 下位机命令格式

```
写入格式: [0x00 前缀][CMD][payload][0x00 填充至 64 字节]
```

| CMD | 名称 | Payload | 说明 |
|-----|------|---------|------|
| `0x01` | CMD_READ_STATUS | 无 | 请求遥测或异常日志 (round-robin) |
| `0x02` | CMD_READ_DEVICE_INFO | `[sub_idx]` | 请求设备信息子页 (sub_idx=0x00/0x01/0x02) |
| `0x0B` | CMD_REBOOT | 无 | USB 断开并重启 MCU |
| `0x0C` | CMD_WRITE_REGISTER | `[REG_ADDR][DATA_LEN][DATA...]` | 写 i2c_buff 寄存器 |

**CMD_READ_DEVICE_INFO Payload 详解**:
```
Byte 0: sub_idx   子页索引 (0x00=基本信息, 0x01=电池信息, 0x02=日期+年限)
```
每次请求独立，无状态依赖。上位机连接后发送 3 次 CMD 0x02 (sub_idx=0/1/2) 获取完整设备信息。

**CMD_WRITE_REGISTER Payload 详解**:
```
Byte 0: REG_ADDR   寄存器地址 (见第3章)
Byte 1: DATA_LEN   数据字节数
Byte 2~N: DATA     实际数据 (Little-Endian)
```

### 2.3 下位机 → 上位机回复格式

**通用协议头 (9 字节)**:

| 偏移 | 字段 | 类型 | 值 |
|------|------|------|---|
| 0~2 | SOF | u8[3] | `0x05 0xA5 0x5A` (固定帧头) |
| 3 | Ver | u8 | `0x02` |
| 4 | Type | u8 | `0x01`=遥测 / `0x02`=异常日志 / `0x03`=设备信息 |
| 5~6 | Seq | u16 LE | 递增序列号 |
| 7~8 | Len | u16 LE | Payload 字节数 |

**CRC16 校验**: 紧跟 Payload 末尾，对 `report_buffer[3..3+Len+5]` (即 Ver~Payload 末尾) 计算 CRC16-CCITT。

---

**Type 0x01 遥测 Payload (从偏移 9 开始)**:

| 相对偏移 | 字段 | 类型 | 单位 |
|---------|------|------|------|
| +0 | SOC | u8 | % |
| +1~+4 | 额定容量 | u32 LE | mAh |
| +5~+6 | 总电压 | u16 LE | V×100 |
| +7~+8 | 总电流 | s16 LE | A×100 (正=充, 负=放) |
| +9~+10 | 功率 | s16 LE | W×10 |
| +11 | 充电状态 | u8 | 0=待机, 1=充电, 2=放电 |
| +12~+13 | 循环次数 | u16 LE | 次 |
| +14~+15 | 电池温度 | s16 LE | °C×10 |
| +16~+17 | 板温 | s16 LE | °C×10 |
| +18 | 电芯数 | u8 | 固定 2 |
| +19~+20 | Cell1 电压 | u16 LE | V×100 |
| +21~+22 | Cell2 电压 | u16 LE | V×100 |
| +23~+24 | 内阻 | u16 LE | mΩ |
| +25~+26 | SOH | u16 LE | pct×100 |
| +27~+28 | 过温异常次数 | u16 LE | 次 |
| +29~+30 | 过压异常次数 | u16 LE | 次 |
| +31~+32 | 过流异常次数 | u16 LE | 次 |
| +33 | 异常日志条数 | u8 | 条 |
| +34 | 品牌名长度 | u8 | 字节 |
| +35~+41 | 品牌名 | char[7] | ASCII |

---

**Type 0x02 异常日志 Payload (从偏移 9 开始)**:

| 相对偏移 | 字段 | 类型 | 说明 |
|---------|------|------|------|
| +0 | OffsetPage | u8 | 分页偏移 |
| +1 | ReturnCount | u8 | 本次返回条数 (0-2) |
| +2 起 | ExceptionLog[] | **20B × N** | 异常条目数组 |

**每条异常日志 (20 字节, 与 NU17112 `BatteryExceptionRecord_t` 完全对齐)**:

| 相对偏移 | 字段 | 类型 | 说明 |
|---------|------|------|------|
| +0~+1 | Year | u16 LE | 年 |
| +2 | Month | u8 | 月 |
| +3 | Day | u8 | 日 |
| +4 | Hour | u8 | 时 |
| +5 | Minute | u8 | 分 |
| +6 | Second | u8 | 秒 |
| +7 | Reserved | u8 | 对齐字节 = 0x00 |
| +8 | error_type | u8 | **0x01**=过压(OV), **0x02**=过温(OT), **0x03**=欠温(UT) |
| +9 | sub_type | u8 | OV: 电芯号(1-N); TEMP: 0=充电态, 1=放电态 |
| +10~+11 | data_low | u16 LE | OV: 最高单节电压(mV); TEMP: 最高温度(s16, 0.1°C) |
| +12~+13 | data_high | u16 LE | OV: 总电压(mV); TEMP: 0x0000 |
| +14~+15 | Padding | u16 | 0x0000 |
| +16~+19 | record_id | u32 LE | NU17112 内部序号，PC 可用于去重/排序 |

> **格式对齐说明**: 本格式与 NU17112 固件 `BatteryExceptionRecord_t` (g_data.h) **完全一致（20 字节）**。
> WB7720 可直接 memcpy NU17112 写入 i2c_buff 的记录数据，无需字段转换。每包最多携带 2 条记录。
>
> **原 12B 格式废弃说明**: 旧格式 ErrType 映射 (0=温度/1=电流/2=电压) 与 NU17112 不一致，缺少
> sub_type（无法区分电芯号/充放状态），已废弃并升级为本 20B 格式。

---

**Type 0x03 设备信息 Payload (从偏移 9 开始, Len=41)**:

上位机发送 CMD_READ_DEVICE_INFO (0x02) 并在 `Vendor_Request[1]` 中指定 sub_idx，
WB7720 根据 sub_idx 返回对应子页。每次请求独立、无状态、可重试。

| 相对偏移 | 字段 | 类型 | 说明 |
|---------|------|------|------|
| +0 | SubIdx | u8 | 子页索引: 0x00 / 0x01 / 0x02 |
| +1~+20 | Field1 | char[20] | 第一字段 (ASCII, 不足用 0x00 填充) |
| +21~+40 | Field2 | char[20] | 第二字段 (ASCII, 不足用 0x00 填充) |

**三个子页定义**:

| SubIdx | Field1 (20B) | Field2 (20B) | i2c_buff 来源 |
|--------|-------------|-------------|--------------|
| 0x00 | manufacturer_name | model_name | 0x92~0xA5, 0xA6~0xB9 |
| 0x01 | battery_mfr | battery_model | 0xBA~0xCD, 0xCE~0xE1 |
| 0x02 | battery_prod_date | safety_years(1B) + reserved(19B) | 0xE2~0xF5, 硬编码 `SAFETY_SERVICE_YEARS=5` |

> **数据来源**: 设备信息复用生产模式寄存器区 (0x92~0xF5)。
> 正常运行时 NU17112 将 Flash 中的 ProductInfo 写入该区域，供 WB7720 读取。
> 若该区域全为 0x00/0xFF (未写入生产信息)，上位机收到空字符串。

**Type 0x03 总包长**: 9B(头) + 41B(Payload) + 2B(CRC) = **52B**，剩余 12B 填充 0x00。

**上位机获取设备信息的完整流程**:
```
PC → [0x00][0x02][0x00][0x00×62]    CMD_READ_DEVICE_INFO, sub_idx=0
PC ← Type 0x03, SubIdx=0x00        manufacturer + model

PC → [0x00][0x02][0x01][0x00×62]    CMD_READ_DEVICE_INFO, sub_idx=1
PC ← Type 0x03, SubIdx=0x01        battery_mfr + battery_model

PC → [0x00][0x02][0x02][0x00×62]    CMD_READ_DEVICE_INFO, sub_idx=2
PC ← Type 0x03, SubIdx=0x02        prod_date + safety_years
```

### 2.4 通信节奏

**CMD_READ_STATUS (0x01) — 周期性遥测轮询**:

- 上位机以约 **1 秒**周期发送 `CMD_READ_STATUS (0x01)`
- WB7720 按以下模式交替回复（检查 `data[4]` 判断类型）：
  - 请求 1~4: 回复 **Type 0x01**（遥测）
  - 请求 5: 回复 **Type 0x02**（异常日志）
  - 循环
- HID read 超时: **1000ms**

**CMD_READ_DEVICE_INFO (0x02) — 连接时一次性获取**:

- 上位机连接后发送 3 次 CMD 0x02 (sub_idx=0/1/2)，每次间隔 ≥ 50ms
- 每次收到 Type 0x03 回复后缓存对应字段
- 3 包到齐后更新 UI 设备信息卡片
- 若 5 秒内未收齐，显示超时提示

**CMD_WRITE_REGISTER (0x0C) — 工程/生产模式写入**:

- 写入后 WB7720 回复一包 Type 0x01 遥测数据（确认通信正常）
- 不影响 round-robin 计数器

---

## 3. I2C Slave 寄存器映射

I2C Slave 地址: **0x42**
缓冲区: `i2c_buff[256]`，所有寄存器均为字节索引

### 3.1 遥测数据区 (NU17112 写入)

| 地址 | 宏名 | 类型 | 单位 | 说明 |
|------|------|------|------|------|
| 0x00 | REG_SOC_PCT | u8 | % | 电量百分比 |
| 0x01~0x04 | REG_CAPACITY_MAH | u32 LE | mAh | 额定容量 |
| 0x05~0x06 | REG_VBAT_MV | u16 LE | mV | 总电压 (WB7720 发 HID 时 ÷10) |
| 0x07~0x08 | REG_IBAT_MA | s16 LE | mA | 总电流，正=充负=放 (÷10) |
| 0x09~0x0A | REG_TEMP_DC | s16 LE | 0.1°C | 电池温度 |
| 0x0B~0x0C | REG_CYCLE_COUNT | u16 LE | 次 | 循环充电次数 |
| 0x0D~0x0E | REG_R_INTERNAL_MOHM | u16 LE | mΩ | 内阻 |
| 0x0F~0x10 | REG_SOH_PCT_X100 | u16 LE | pct×100 | 健康度 |
| 0x11~0x12 | REG_ERR_OVERTEMP_CNT | u16 LE | 次 | 过温异常次数 |
| 0x13~0x14 | REG_ERR_OVERVOLT_CNT | u16 LE | 次 | 过压异常次数 |
| 0x15~0x16 | REG_ERR_OVERCURR_CNT | u16 LE | 次 | 过流异常次数 (当前预留为 0) |
| 0x17 | REG_CHARGE_STATE | u8 | enum | 0=待机, 1=充电, 2=放电 |
| 0x2C | REG_CELL_COUNT | u8 | 个 | 电芯数量 (固定 2) |
| 0x2D~0x2E | REG_CELL1_VOLTAGE_MV | u16 LE | mV | Cell1 电压 (÷10) |
| 0x2F~0x30 | REG_CELL2_VOLTAGE_MV | u16 LE | mV | Cell2 电压 (÷10) |
| 0x35~0x36 | REG_PCB_TEMP_DC | s16 LE | 0.1°C | 板温 |

> **WB7720 单位转换**: 地址 0x05~0x08 和 0x2D~0x30 存储 mV/mA，WB7720 在组装 HID 报告时做 ÷10 转换。其他地址直传。功率由 WB7720 实时计算 `(VBAT × IBAT) ÷ 100000`。

### 3.2 扩展遥测区 (NU17112 可写，上位机可读，当前未用)

| 地址 | 宏名 | 类型 | 说明 |
|------|------|------|------|
| 0x19~0x1A | REG_INPUT_VOLTAGE_MV | u16 LE | 输入电压 |
| 0x1B~0x1C | REG_INPUT_CURRENT_MA | u16 LE | 输入电流 |
| 0x1D~0x1E | REG_OUTPUT_VOLTAGE_MV | u16 LE | 输出电压 |
| 0x1F~0x20 | REG_OUTPUT_CURRENT_MA | u16 LE | 输出电流 |
| 0x21~0x22 | REG_REMAINING_TIME_CHARGE | u16 LE | 剩余充电时间 (分钟) |
| 0x23~0x24 | REG_REMAINING_TIME_DISCHARGE | u16 LE | 剩余放电时间 (分钟) |
| 0x29 | REG_FW_VERSION_MAJOR | u8 | 固件主版本 |
| 0x2A | REG_FW_VERSION_MINOR | u8 | 固件次版本 |

### 3.3 工程模式寄存器 (上位机写入, NU17112 轮询读取)

| 地址 | 宏名 | 类型 | 方向 | 说明 |
|------|------|------|------|------|
| 0x50 | REG_WORK_MODE | u8 | PC→WB7720 | 0x00=用户, 0xA5=工程模式 |
| 0x60~0x63 | REG_ENG_CURRENT_DATE | u16+u8+u8 LE | PC→WB7720→NU17112 | 当前日期 |
| 0x70~0x73 | REG_ENG_PRODUCTION_DATE | u16+u8+u8 LE | PC→WB7720→NU17112 | 生产日期 |
| 0x80~0x81 | REG_ENG_CYCLE_CHG_COUNT | u16 LE | PC→WB7720→NU17112 | 循环次数 |

**工程模式写入序列** (每步间隔 ≥ 50ms):
```
Step 1: CMD_WRITE_REGISTER → REG_WORK_MODE(0x50) = 0xA5   (解锁)
Step 2: CMD_WRITE_REGISTER → REG_ENG_CURRENT_DATE(0x60)   (当前日期)
Step 3: CMD_WRITE_REGISTER → REG_ENG_PRODUCTION_DATE(0x70) (生产日期)
Step 4: CMD_WRITE_REGISTER → REG_ENG_CYCLE_CHG_COUNT(0x80) (循环次数)
```
NU17112 检测到 REG_WORK_MODE = 0xA5 后读取数据，处理完毕后将 REG_WORK_MODE 清零。

### 3.4 生产模式寄存器 / 设备信息读回区 (双向复用)

**地址 0x90~0xF5 同时服务两种模式** (时序互斥):

- **生产模式** (产线): PC → i2c_buff → NU17112 读取 → 写 Flash
- **正常运行** (设备信息读回): NU17112 读 Flash → 写 i2c_buff → WB7720 构建 Type 0x03

| 地址 | 宏名 | 长度 | 生产模式方向 | 正常模式方向 | 字段 |
|------|------|------|------------|------------|------|
| 0x90 | PROD_MODE_FLAG | 1B | PC→WB7720 | — | 0xB5=进入生产模式 |
| 0x91 | PROD_WRITE_STATUS | 1B | NU17112→WB7720→PC | — | 0x01=进行中, 0x02=成功, 0xFF=失败 |
| 0x92~0xA5 | PROD_MANUFACTURER | 20B | PC→WB7720→NU17112 | NU17112→WB7720 | manufacturer_name |
| 0xA6~0xB9 | PROD_MODEL | 20B | PC→WB7720→NU17112 | NU17112→WB7720 | model_name |
| 0xBA~0xCD | PROD_BATTERY_MFR | 20B | PC→WB7720→NU17112 | NU17112→WB7720 | battery_mfr |
| 0xCE~0xE1 | PROD_BATTERY_MODEL | 20B | PC→WB7720→NU17112 | NU17112→WB7720 | battery_model |
| 0xE2~0xF5 | PROD_PROD_DATE | 20B | PC→WB7720→NU17112 | NU17112→WB7720 | battery_prod_date |

**生产模式流程**:
1. PC 写入 0x92~0xF5 (100B ProductInfo)
2. PC 写入 0x90 = 0xB5 (触发标志)
3. NU17112 检测到 0xB5 → 读取 0x92~0xF5 → 写 Flash → 写 0x91 状态
4. PC 轮询 0x91 直到 0x02(成功) 或 0xFF(失败)
5. NU17112 写 0x90 = 0x00 清除标志

**正常运行流程** (设备信息读回):
1. NU17112 开机后将 Flash ProductInfo 写入 i2c_buff[0x92~0xF5]
2. 上位机连接后发 CMD_READ_DEVICE_INFO (0x02) × 3 次
3. WB7720 从 i2c_buff[0x92~0xF5] 读取数据组装 Type 0x03 报告

---

## 4. 字节序和数据规范

- 所有多字节整数: **Little-Endian**
- 有符号整数使用补码 (2's complement)
- ASCII 字符串不含 null terminator，不足长度用 0x00 填充
- hidapi 写入必须以 `0x00` 开头 (Report ID 占位)
- 所有报告固定 64 字节，不足部分填充 `0x00`

---

## 5. 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2026-02-26 | 初版，整合 HID通信协议与寄存器说明.md + NU17112_USB_HID_适配指南.md |
| v1.1 | 2026-02-26 | Type 0x02 异常日志从 12B 升级到 20B，与 NU17112 BatteryExceptionRecord_t 完全对齐；新增 Type 0x04 设备信息类型；CMD_READ_DEVICE_INFO 补充 SubPage 参数 |
| **v1.2** | **2026-02-26** | **Type 重新分配**: Type 0x04 废弃，设备信息改为 **Type 0x03** (3 子页 SubIdx 方案)；CMD 0x02 Payload 明确为 `[sub_idx]`，每次请求独立无状态；§3.4 生产模式寄存器改为双向复用（生产模式写入 + 正常模式设备信息读回）；新增 CMD 分发规则（§2.4） |

---

## 6. 关联文档

| 文档 | 位置 | 说明 |
|------|------|------|
| 三方数据格式对照表 | `.claude/references/specs/USB三方数据格式对照表.md` | 端到端数据链路和单位换算 |
| NU17112 适配指南 | `USB系统/ARUN_N3C_WIN上位机/NU17112_USB_HID_适配指南.md` | NU17112 侧实现细节 |
| 上位机 PRD | `USB系统/ARUN_N3C_WIN上位机/ARUN_USB_WINDOWS_PRD_v3.0.md` | 上位机需求规范 |
