# USB 系统三方数据格式对照表

**版本**: v1.1
**日期**: 2026-02-26
**管理者**: powerbank-leader
**适用项目**: ARUN N3C (NU17112 + NU6805 + WB7720)

> **v1.1 变更**: 新增"PC 显示"列，标注哪些字段在 UI 中展示（基于 PRD v4.0 / UI 设计稿）。
> 新增第五节：设备基本信息 (CMD_READ_DEVICE_INFO / Type 0x04)。

---

## 系统数据流

```
NU17112 固件 (I2C Master)
    │ 每 1 秒写入 i2c_buff
    ▼
WB7720 固件 (I2C Slave @0x42, i2c_buff[256])
    │ 收到 HID CMD 后组装 64B 报告
    ▼
Windows 上位机 (hidapi, 1Hz 轮询)
    │ 解析 → 显示
    ▼
用户界面
```

**说明**:
- **写入方向** (→): NU17112 主动写 → WB7720 i2c_buff → 上位机读取
- **回写方向** (←): 上位机 → WB7720 i2c_buff → NU17112 轮询读取

---

## 一、遥测数据 Type 0x01 (NU17112 → WB7720 → PC)

| # | 字段名 | NU17112 数据源 | NU17112<br>原始单位 | 写入 i2c_buff<br>的处理 | i2c_buff<br>地址 | i2c_buff<br>类型 | WB7720→HID<br>的处理 | HID 报告<br>字节偏移 | HID<br>单位 | PC 解析<br>struct | PC 显示<br>换算 | 显示<br>单位 | **PC 显示** |
|---|--------|---------------|-------------------|----------------------|----------------|----------------|---------------------|-------------------|----------|-----------------|-------------|----------|---------|
| 1 | SOC | `gd->real_soc_show` | % | 直写 | 0x00 | u8 | 直传 | +0 | % | `B` | × 1 | % | ✅ 卡片1 SOC 圆弧 |
| 2 | 额定容量 | 配置常量 (`CONFIG_BATTERY_CAPACITY_MAH`) | mAh | 直写 | 0x01~0x04 | u32 LE | 直传 | +1~+4 | mAh | `<I` | × 1 | mAh | — 不显示 |
| 3 | 总电压 | `g_buckboost.adc_vbat` | mV | 直写 mV | 0x05~0x06 | u16 LE | ÷ 10 | +5~+6 | V×100 | `<H` | ÷ 100 | V | ✅ 卡片3 总电压 |
| 4 | 总电流 | `g_buckboost.adc_ibat` | mA (有符号) | 直写 mA | 0x07~0x08 | s16 LE | ÷ 10 | +7~+8 | A×100 | `<h` | × 10 | mA | — 不显示 |
| 5 | 功率 | — | — | (无专用寄存器) | — | — | (VBAT×IBAT)÷100000 | +9~+10 | W×10 | `<h` | ÷ 10 | W | — 不显示 |
| 6 | 充电状态 | `g_buckboost.woke_mode` | 0=待机<br>1=充电<br>2=放电 | 直写 | 0x17 | u8 | 直传 | +11 | enum | `B` | 文字映射 | 状态文字 | — 不显示 |
| 7 | 循环次数 | `gd->Battery_cycle_count` | 次 (u8) | u8 扩展为 u16 | 0x0B~0x0C | u16 LE | 直传 | +12~+13 | 次 | `<H` | × 1 | 次 | ✅ 卡片1 循环次数 |
| 8 | 电池温度 | `gd->sys_infos.ntc_temp_wpc` | 0.1°C | 直写 | 0x09~0x0A | s16 LE | 直传 | +14~+15 | °C×10 | `<h` | ÷ 10 * | °C | ✅ 卡片2 电芯1/2 温度 (同值) |
| 9 | 板温 | `gd->sys_infos.ntc_temp_typec` | 0.1°C | 直写 | 0x35~0x36 | s16 LE | 直传 | +16~+17 | °C×10 | `<h` | ÷ 10 * | °C | — 不显示 |
| 10 | 电芯数 | 硬编码 = 2 | 个 | 直写 | 0x2C | u8 | 直传 | +18 | 个 | `B` | × 1 | 个 | — 不显示 (固定2) |
| 11 | Cell1 电压 | `g_buckboost.adc_vbat / 2` | mV | 直写 mV | 0x2D~0x2E | u16 LE | ÷ 10 | +19~+20 | V×100 | `<H` | ÷ 100 | V | ✅ 卡片3 电芯1电压 |
| 12 | Cell2 电压 | `g_buckboost.adc_vbat - Cell1` | mV | 直写 mV | 0x2F~0x30 | u16 LE | ÷ 10 | +21~+22 | V×100 | `<H` | ÷ 100 | V | ✅ 卡片3 电芯2电压 |
| 13 | 内阻 | `gd->Bat_Rdc` | mΩ (u8) | u8 扩展为 u16 | 0x0D~0x0E | u16 LE | 直传 | +23~+24 | mΩ | `<H` | × 1 | mΩ | — 不显示 |
| 14 | SOH | `gd->Bat_SoH` | % (int8) | int8 × 100 → u16 | 0x0F~0x10 | u16 LE | 直传 | +25~+26 | pct×100 | `<H` | ÷ 100 | % | — 不显示 |
| 15 | 过温异常次数 | CCC Log 统计 | 次 | 遍历计数 | 0x11~0x12 | u16 LE | 直传 | +27~+28 | 次 | `<H` | × 1 | 次 | — 不显示 (内部用) |
| 16 | 过压异常次数 | CCC Log 统计 | 次 | 遍历计数 | 0x13~0x14 | u16 LE | 直传 | +29~+30 | 次 | `<H` | × 1 | 次 | — 不显示 (内部用) |
| 17 | 过流异常次数 | (暂无 OCP 日志) | 次 | 写 0 | 0x15~0x16 | u16 LE | 直传 | +31~+32 | 次 | `<H` | × 1 | 次 | — 不显示 |
| 18 | 异常日志条数 | `record_storage.exception_counter` | 条 | 直写 | — | — | 硬编码 = 3 ** | +33 | 条 | `B` | × 1 | 条 | — 不显示 (内部用) |
| 19 | 品牌名长度 | `strlen(model_name)` | 字节 | 直写 | — | — | 计算 | +34 | 字节 | `B` | — | — | — 不显示 |
| 20 | 品牌名 | `ProductInfo_t.model_name` | ASCII | 直写 | — | — | 直传 | +35~+41 | ASCII | `7s` | 解码 | 字符串 | — 不显示 (由 Type 0x04 取代) |
| — | **电压差** | — | — | — | — | — | 上位机计算 | — | V | — | \|Cell1-Cell2\| | V | ✅ 卡片3 电压差 |

> \* **温度智能检测**: 上位机对温度值 < 100 视为直接 °C（旧固件兼容），≥ 100 则 ÷ 10 转换。
>
> \*\* **异常日志条数**: 当前固件硬编码为 3，来自 `MAX_RECORDS` 限制。

---

## 二、异常日志 Type 0x02 (NU17112 → WB7720 → PC)

### 2.1 I2C 日志区 (暂定, 待实现)

| i2c_buff 地址 | 字段 | 类型 | NU17112 数据源 |
|--------------|------|------|--------------|
| TBD (2B) | 日志元数据 | exception_counter(u8) + write_ptr(u8) | `record_storage.exception_counter/write_ptr` |
| TBD (100B) | 日志条目数组 | `BatteryExceptionRecord_t[5]` (20B×5) | `record_storage.records[]` |

> WB7720 尚未实现异常日志的 I2C 桥接。i2c_buff 地址范围待固件团队确认（Phase 2 任务）。

### 2.2 HID 报告格式 (每条 20 字节, 与 NU17112 `BatteryExceptionRecord_t` 完全对齐)

| HID 偏移 (相对于条目起始) | 字段 | 类型 | 说明 | PC 解析 | PC 显示 |
|--------------------------|------|------|------|---------|---------|
| +0~+1 | Year | u16 LE | 年 | `<H` | 直接显示 |
| +2 | Month | u8 | 月 | `B` | 直接显示 |
| +3 | Day | u8 | 日 | `B` | 直接显示 |
| +4 | Hour | u8 | 时 | `B` | 直接显示 |
| +5 | Minute | u8 | 分 | `B` | 直接显示 |
| +6 | Second | u8 | 秒 | `B` | 直接显示 |
| +7 | Reserved | u8 | 对齐字节, 忽略 | `B` | — |
| +8 | error_type | u8 | 0x01=OV, 0x02=OT, 0x03=UT | `B` | 文字映射 |
| +9 | sub_type | u8 | OV: 电芯号(1-N); TEMP: 0=充电, 1=放电 | `B` | 文字前缀 |
| +10~+11 | data_low | u16 LE | OV: 最高单节电压(mV); TEMP: 最高温度(s16, 0.1°C) | `<H`/`<h` | 换算后显示 |
| +12~+13 | data_high | u16 LE | OV: 总电压(mV); TEMP: 0x0000 | `<H` | OV 时显示 |
| +14~+15 | Padding | u16 | 0x0000, 忽略 | `<H` | — |
| +16~+19 | record_id | u32 LE | NU17112 内部序号 | `<I` | 去重/排序 |

**PC 异常文字格式**:

| error_type | 类型描述 | 显示格式 | 示例 |
|-----------|---------|---------|------|
| 0x01 (OV) | 电压过高 | `电芯{sub_type} 电压过高 {data_low/1000:.2f}V (总{data_high/1000:.2f}V)[ YYYY-MM-DD HH:MM:SS ]` | `电芯1 电压过高 4.50V (总8.90V)[ 2026-01-08 18:43:23 ]` |
| 0x02 (OT) | 过温 | `{'充电' if sub_type==0 else '放电'} 过温 {data_low/10:.1f}℃[ YYYY-MM-DD HH:MM:SS ]` | `充电 过温 55.2℃[ 2026-01-08 18:47:12 ]` |
| 0x03 (UT) | 欠温 | `{'充电' if sub_type==0 else '放电'} 欠温 {data_low/10:.1f}℃[ YYYY-MM-DD HH:MM:SS ]` | `放电 欠温 -5.3℃[ 2026-01-09 11:36:12 ]` |

> **v1.1 变更说明**: Type 0x02 从 12B 升级为 20B，与 NU17112 `BatteryExceptionRecord_t` 原生格式对齐。
> 旧 12B 格式 ErrType 映射 (0=温度/1=电流/2=电压) 与 NU17112 不一致，且缺少 sub_type，已废弃。
> WB7720 桥接时直接 memcpy `BatteryExceptionRecord_t[]` 数据，无需字段转换。

---

## 三、工程模式寄存器 (PC → WB7720 → NU17112)

| 寄存器地址 | 名称 | 类型 | 方向 | HID CMD | NU17112 处理 | Flash 存储 |
|-----------|------|------|------|---------|-------------|----------|
| 0x50 | WORK_MODE | u8 | PC→WB7720 (本地) | CMD_WRITE_REGISTER (0x0C) | 检测 0xA5 → 进入工程模式 | 无 |
| 0x60~0x63 | ENG_CURRENT_DATE | u16+u8+u8 LE | PC→WB7720→NU17112 | 同上 | 写入 Flash (ProductInfo 扩展) | TBD |
| 0x70~0x73 | ENG_PRODUCTION_DATE | u16+u8+u8 LE | PC→WB7720→NU17112 | 同上 | 写入 Flash | TBD |
| 0x80~0x81 | ENG_CYCLE_CHG_COUNT | u16 LE | PC→WB7720→NU17112 | 同上 | 写入 `gd->Battery_cycle_count` | 无 (RAM) |

**工程模式写入序列** (每步间隔 ≥ 50ms):
```
Step 1: [0x0C][0x50][0x01][0xA5]          解锁工程模式
Step 2: [0x0C][0x60][0x04][年Lo][年Hi][月][日]  写当前日期
Step 3: [0x0C][0x70][0x04][年Lo][年Hi][月][日]  写生产日期
Step 4: [0x0C][0x80][0x02][次Lo][次Hi]        写循环次数
```

---

## 四、生产模式寄存器 (PC → WB7720 → NU17112 → Flash)

| 地址 | 名称 | 长度 | 方向 | NU17112 处理 | Flash 地址 |
|------|------|------|------|-------------|----------|
| 0x90 | PROD_MODE_FLAG | 1B | PC→WB7720; NU17112 轮询 | 检测 0xB5 → 触发读取 | 无 |
| 0x91 | PROD_WRITE_STATUS | 1B | NU17112→WB7720→PC | 写 0x01(进行中)/0x02(成功)/0xFF(失败) | 无 |
| 0x92~0xA5 | PROD_MANUFACTURER | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.manufacturer_name` | Flash 0x1800+0 |
| 0xA6~0xB9 | PROD_MODEL | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.model_name` | Flash 0x1800+20 |
| 0xBA~0xCD | PROD_BATTERY_MFR | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_mfr` | Flash 0x1800+40 |
| 0xCE~0xE1 | PROD_BATTERY_MODEL | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_model` | Flash 0x1800+60 |
| 0xE2~0xF5 | PROD_PROD_DATE | 20B | PC→WB7720→NU17112 | → `ProductInfo_t.battery_prod_date` | Flash 0x1800+80 |

---

## 五、快速换算参考

| 参数 | NU17112 原始 | HID 报告值 | 显示值 | 换算链 |
|------|------------|-----------|--------|--------|
| 电压 8800 mV | 8800 | 880 | 8.80 V | mV ÷10 = V×100; ÷100 = V |
| 电流 2000 mA | 2000 | 200 | 2000 mA | mA ÷10 = A×100; ×10 = mA |
| 功率 (8800×2000) | — | 176 | 17.6 W | (mV×mA)÷100000 = W×10; ÷10 = W |
| 温度 25.0°C (250) | 250 | 250 | 25.0°C | 0.1°C = °C×10; ÷10 = °C |
| SOH 98% | 98 (int8) | 9800 | 98.00% | ×100 = pct×100; ÷100 = % |
| 内阻 50 mΩ | 50 (u8) | 50 | 50 mΩ | u8→u16 直传 |
| Cell 电压 4400 mV | 4400 | 440 | 4.40 V | mV ÷10 = V×100; ÷100 = V |

---

## 五、设备基本信息 Type 0x04 (NU17112 → WB7720 → PC)

**触发**: 上位机连接后发一次 CMD_READ_DEVICE_INFO (0x02)，WB7720 回复 3 包。

### 5.1 NU17112 → WB7720 i2c_buff (设备信息读回区)

> NU17112 在开机后将 Flash 中的 ProductInfo 写入 i2c_buff 设备信息读回区，供 WB7720 响应上位机查询。
> i2c_buff 地址范围待固件团队确认（建议使用 0x18~0x6B，与遥测区/工程区不冲突）。

| i2c_buff 地址 | 字段 | 长度 | NU17112 数据源 |
|--------------|------|------|--------------|
| TBD (20B) | manufacturer_name | 20B | `ProductInfo_t.manufacturer_name` (Flash 0x1800+0) |
| TBD (20B) | model_name | 20B | `ProductInfo_t.model_name` (Flash 0x1800+20) |
| TBD (20B) | battery_mfr | 20B | `ProductInfo_t.battery_mfr` (Flash 0x1800+40) |
| TBD (20B) | battery_model | 20B | `ProductInfo_t.battery_model` (Flash 0x1800+60) |
| TBD (20B) | battery_prod_date | 20B | `ProductInfo_t.battery_prod_date` (Flash 0x1800+80) |

### 5.2 WB7720 → HID 报告 (Type 0x04, 3包)

| 包序 | SubIdx (Payload +0) | 字段1 (Payload +1~+20) | 字段2 (Payload +21~+40) |
|------|--------------------|-----------------------|------------------------|
| 包1 | 0x00 | manufacturer_name (20B) | model_name (20B) |
| 包2 | 0x01 | battery_mfr (20B) | battery_model (20B) |
| 包3 | 0x02 | battery_prod_date (20B) | 填充 0x00 (20B) |

### 5.3 PC 显示 (卡片4 设备基本信息)

| UI 标签 | 数据来源 | PC 显示 |
|--------|---------|---------|
| 生产厂家 | Type 0x04 包1 字段1 | ✅ 卡片4 |
| 产品型号 | Type 0x04 包1 字段2 | ✅ 卡片4 |
| 电池生产厂商 | Type 0x04 包2 字段1 | ✅ 卡片4 |
| 电池型号 | Type 0x04 包2 字段2 | ✅ 卡片4 |
| 代码/生产日期 | Type 0x04 包3 字段1 | ✅ 卡片4 |
| 安全使用年限 | **硬编码 "5年"** | ✅ 卡片4 |

---

## 六、版本历史

| 版本 | 日期 | 变更说明 |
|------|------|---------|
| v1.0 | 2026-02-26 | 初版，整合 NU17112_USB_HID_适配指南 + HID通信协议文档 |
| v1.1 | 2026-02-26 | 新增"PC 显示"列；新增第五节设备基本信息 (Type 0x04)；对齐 PRD v4.0 |
| v1.2 | 2026-02-26 | 第二节 Type 0x02: 从 12B 升级为 20B，与 NU17112 BatteryExceptionRecord_t 完全对齐；新增 sub_type/record_id/OV 双电压字段；ErrType 重新定义为 0x01/0x02/0x03 |

---

## 七、待确认 / Gap 记录

| # | 项目 | 状态 | 说明 |
|---|------|------|------|
| 1 | 异常日志 Type 0x02 的 I2C 桥接 | ❌ 未实现 | Phase 2 任务，需设计 i2c_buff 日志区地址 |
| 2 | Cell 单节电压精度 | ⚠️ 估算 | NU6805 仅有总压，当前均分 (vbat/2) |
| 3 | 循环次数 u8 → u16 | ⚠️ 已处理 | NU17112 u8 扩展为 u16 写入 i2c_buff |
| 4 | 过流异常计数 | ⚠️ 预留 | 当前写 0，待 OCP 日志实现后更新 |
| 5 | 品牌名 Type 0x01 Brand 字段 | ⚠️ 废弃 | 由 Type 0x04 设备信息取代，Type 0x01 中 +35~+41 可保留兼容 |
| 6 | 设备信息 i2c_buff 地址 | ❌ 未定义 | NU17112 写 ProductInfo 到 i2c_buff 的具体地址待固件团队确认 |
