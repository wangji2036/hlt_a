# WB7720 USB Bridge 接口规范

**版本**: v1.4
**日期**: 2026-02-28
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
| +0 | OffsetPage | u8 | 本包页码 (0=第1组, 1=第2组, 2=第3组) |
| +1 | ReturnCount | u8 | 本包实际返回条数 (1 或 2) |
| +2 起 | ExceptionLog[] | **20B × N** | 异常条目数组，最多 2 条 |

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
| +16~+19 | record_id | u32 LE | NU17112 内部序号，PC 用于去重和排序 |

> **格式对齐说明**: 本格式与 NU17112 固件 `BatteryExceptionRecord_t` (g_data.h) **完全一致（20 字节）**。
> WB7720 直接 memcpy i2c_buff 中的记录数据，无需字段转换。
>
> **原 12B 格式废弃说明**: 旧格式 ErrType 映射 (0=温度/1=电流/2=电压) 与 NU17112 不一致，缺少
> sub_type（无法区分电芯号/充放状态），已废弃并升级为本 20B 格式。

**多条日志全量传输机制 (v1.3 新增)**:

NU17112 最多存 5 条日志，64B HID 报告每包只能携带 2 条，需要 3 包才能传完。
采用「NU17112 滚动写入 + WB7720 本地缓存 + 分页输出」机制：

```
NU17112 每 ~500ms 将当前 exc_current_idx 对应的 1 条记录写入 i2c_buff[0x3A-0x4D]
        （同时更新 0x37=总条数, 0x38=当前序号）

WB7720 在每次 update_report_buffer_0() 末尾检查 i2c_buff[0x3A] 的 record_id：
        若 record_id 不在本地缓存中，追加到 exc_cache[5]（最多保留 5 条）

WB7720 在 update_report_buffer_1() 中从 exc_cache 分页输出：
        page 0: exc_cache[0] + exc_cache[1]   (OffsetPage=0, ReturnCount=2)
        page 1: exc_cache[2] + exc_cache[3]   (OffsetPage=1, ReturnCount=2)
        page 2: exc_cache[4]                  (OffsetPage=2, ReturnCount=1)
        每次 Type 0x02 输出后 page_idx 自动推进，循环。
```

**PC 端汇总策略**:
- 维护 `seen_record_ids = set()` 集合，按 record_id 去重
- 收到 Type 0x02 后，将未见过的记录追加到日志列表
- 按时间戳升序排列展示
- 约 15 秒内（3 包 × 5 秒 round-robin）可收齐全部 5 条记录
- 断开 USB 重连后，PC 端应清空 `seen_record_ids` 重新收集

**包大小计算**:
- 最大包: 9B(头) + 2B(分页) + 40B(2条×20B) + 2B(CRC) = **53B**（在 64B 限制内）
- 最小包(最后页): 9B + 2B + 20B(1条) + 2B = **33B**

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

### 3.2b 异常日志滚动写入区 (NU17112 写入, WB7720 读取缓存)

**地址范围**: `0x37~0x4D` (23 字节，位于遥测区末尾与工程模式区之间，无冲突)

| 地址 | 宏名 | 类型 | 说明 |
|------|------|------|------|
| 0x37 | REG_EXC_TOTAL_COUNT | u8 | NU17112 当前有效异常记录总数 (0~5) |
| 0x38 | REG_EXC_CURRENT_IDX | u8 | 本次写入的是第几条记录 (0~4，循环) |
| 0x39 | REG_EXC_RESERVED | u8 | 保留，写 0x00 |
| 0x3A~0x4D | REG_EXC_RECORD | u8[20] | 当前 `BatteryExceptionRecord_t` (20B) |

**NU17112 写入行为**:
- NU17112 每约 500ms（`ubsd_wb7720_report_update()` 每轮完成后）写一次
- 每次写入 `REG_EXC_TOTAL_COUNT`、`REG_EXC_CURRENT_IDX` 和 20B 记录数据
- `REG_EXC_CURRENT_IDX` 从 0 滚动到 (total_count-1)，然后回 0
- 若无异常记录（`total_count = 0`），仅写 0x37=0，不写记录数据

**WB7720 读取行为**:
- 在每次组装 Type 0x01 遥测报告时，顺带检查 `i2c_buff[0x3A]` 起的 record_id（偏移 +16）
- 若该 record_id 未在本地 `exc_cache[5]` 中，则将整条 20B 追加入缓存
- 缓存满 5 条后不再追加（按 record_id 去重，最旧条目自动被新条目替代）

> **设计约束**: 0x37~0x4D 之所以选在此区间，是因为 0x36 是遥测区末字节（REG_PCB_TEMP），0x50 是工程模式 REG_WORK_MODE，之间有 24B 可用空间，恰好容纳本区定义的 23B。

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

### 3.3b 工程测试寄存器 (0x82-0x89, 虚拟覆写 + 记录清除)

**地址范围**: `0x82~0x89` (8 字节，紧接工程模式日期/循环寄存器之后，位于生产模式区之前)

| 地址 | 名称 | 类型 | 方向 | 说明 |
|------|------|------|------|------|
| 0x82~0x83 | REG_ENG_VIRTUAL_CELL1 | u16 LE | PC→NU17112 | 虚拟电芯1电压 (mV), 0=用真实值 |
| 0x84~0x85 | REG_ENG_VIRTUAL_CELL2 | u16 LE | PC→NU17112 | 虚拟电芯2电压 (mV), 0=用真实值 |
| 0x86~0x87 | REG_ENG_VIRTUAL_TEMP | s16 LE | PC→NU17112 | 虚拟温度 (0.1°C), 0=用真实值 |
| 0x88 | REG_ENG_ERASE_ALL_CMD | u8 | PC→NU17112 | 写 0xEE 触发清除全部异常记录 |
| 0x89 | REG_ENG_CMD_STATUS | u8 | NU17112→PC | 0x00=空闲, 0x01=进行中, 0x02=成功, 0xFF=失败 |

**写保护**: 0x82~0x88 写入要求 `REG_WORK_MODE(0x50)=0xA5`（工程模式已解锁），否则写入被忽略。

**虚拟覆写测试流程**:

```
1. 进入工程模式 (REG_WORK_MODE=0xA5)，同时写入虚拟值:
   - 0x82~0x83: 虚拟电芯1电压 (mV), 例如 4500 → [0xA4 0x11]
   - 0x84~0x85: 虚拟电芯2电压 (mV), 例如 4500 → [0xA4 0x11]
   - 0x86~0x87: 虚拟温度 (0.1°C), 例如 650(65.0°C) → [0x8A 0x02]
   写入后，i2c_buff 中对应地址被更新

2. NU17112 在 ENTER 工程模式时读取虚拟值:
   - 读 0x82~0x85 获取虚拟电芯电压
   - 读 0x86~0x87 获取虚拟温度
   - 非零值替代真实硬件采样值用于异常检测

3. 异常检测逻辑使用虚拟值:
   - 单节电压 > 4450mV → 触发 OV (过压) 异常记录
   - 温度 > 600 (60.0°C) → 触发 OT (过温) 异常记录
   - 异常记录写入 Flash 并更新 i2c_buff 异常日志区

4. 退出工程模式 → 恢复真实采样值 → Recovery 记录保存
```

**异常记录清除流程**:

```
1. 确保已在工程模式 (REG_WORK_MODE=0xA5)
2. 写 0xEE 到 REG_ENG_ERASE_ALL_CMD(0x88):
   CMD_WRITE_REGISTER → [0x0C][0x88][0x01][0xEE]
3. NU17112 检测到 0xEE → 擦除 Flash 中全部异常记录
4. NU17112 将执行状态写入 REG_ENG_CMD_STATUS(0x89):
   - 0x01 = 进行中 (擦除中)
   - 0x02 = 成功 (擦除完毕)
   - 0xFF = 失败 (Flash 操作异常)
5. PC 轮询 0x89 直到非 0x01 值
6. NU17112 完成后清除 0x88 = 0x00
```

**HID 写入序列示例** (虚拟覆写 + 触发 OV):

```
Step 1: [0x0C][0x50][0x01][0xA5]                解锁工程模式
Step 2: [0x0C][0x82][0x02][0xA4][0x11]          虚拟 Cell1 = 4500mV
Step 3: [0x0C][0x84][0x02][0xA4][0x11]          虚拟 Cell2 = 4500mV
Step 4: [0x0C][0x86][0x02][0x00][0x00]          温度不覆写 (0=真实值)
        ...NU17112 使用虚拟值检测，4500 > 4450 → OV 记录生成...
Step 5: [0x0C][0x50][0x01][0x00]                退出工程模式
```

**HID 写入序列示例** (清除全部异常记录):

```
Step 1: [0x0C][0x50][0x01][0xA5]                解锁工程模式
Step 2: [0x0C][0x88][0x01][0xEE]                触发擦除
        ...PC 轮询 0x89 状态...
Step 3: [0x0C][0x50][0x01][0x00]                退出工程模式
```

> **设计约束**: 0x82~0x89 位于 0x80~0x81 (ENG_CYCLE_CHG_COUNT) 之后、0x90 (PROD_MODE_FLAG) 之前，
> 地址空间充裕无冲突。REG_ENG_CMD_STATUS(0x89) 为只读方向（NU17112→PC），PC 不应写入。

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
| v1.2 | 2026-02-26 | Type 重新分配: Type 0x04 废弃，设备信息改为 **Type 0x03** (3 子页 SubIdx 方案)；CMD 0x02 Payload 明确为 `[sub_idx]`，每次请求独立无状态；§3.4 生产模式寄存器改为双向复用（生产模式写入 + 正常模式设备信息读回）；新增 CMD 分发规则（§2.4） |
| v1.3 | 2026-02-26 | **异常日志全量传输机制**: 新增 §3.2b 异常日志滚动写入区（i2c_buff 0x37~0x4D, 23B）；Type 0x02 补充多条日志分页传输说明、WB7720 本地 5 条缓存设计、PC 端 record_id 去重汇总策略；明确 OffsetPage 页码含义和包大小计算 |
| **v1.4** | **2026-02-28** | **工程测试寄存器**: 新增 §3.3b 工程测试寄存器（i2c_buff 0x82~0x89, 8B）；支持虚拟电芯电压/温度覆写用于异常触发测试（OV >4450mV, OT >60.0°C）；支持 0xEE 命令清除全部异常记录；CMD_STATUS 反馈机制；写保护要求 WORK_MODE=0xA5 |

---

## 6. 关联文档

| 文档 | 位置 | 说明 |
|------|------|------|
| 三方数据格式对照表 | `.claude/references/specs/USB三方数据格式对照表.md` | 端到端数据链路和单位换算 |
| NU17112 适配指南 | `USB系统/ARUN_N3C_WIN上位机/NU17112_USB_HID_适配指南.md` | NU17112 侧实现细节 |
| 上位机 PRD | `USB系统/ARUN_N3C_WIN上位机/ARUN_USB_WINDOWS_PRD_v3.0.md` | 上位机需求规范 |
