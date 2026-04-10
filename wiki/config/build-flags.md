# 编译配置与功能开关

**最后更新**: 2026-04-08
**来源文件**: app/config.h (121 行)

---

## 电池配置

| 宏 | 值 | 说明 |
|---|---|------|
| BATTERY_CV_VALUE | 4350 mV | 充电截止电压 |
| CONFIG_NU6801_BATLOW_VOLT | 2500 mV | 死电池阈值 |
| CONFIG_BATTERY_CAPACITY_MAH | 5000 | 额定容量 |
| CONFIG_BATTERY_CELL_COUNT | 2 | 电芯数 (2S 串联) |
| CONFIG_DEADBATT_VOLTAGE | 2900 mV | 死电池休眠阈值 |

## 电压保护 (GB31241 合规)

| 宏 | 值 | 说明 |
|---|---|------|
| OVER_VOLTAGE_THRESHOLD | 4450 mV | 单节过压记录阈值 |
| OVER_VOLTAGE_HYSTERESIS | 40 mV | 过压恢复迟滞 |
| OVER_VOLTAGE_FORBID_THRESHOLD | 4600 mV | 永久禁止充电阈值 |
| UNDER_VOLTAGE_FORBID_THRESHOLD | 1200 mV | 永久禁止放电阈值 |

## 温度保护

| 宏 | 值 | 说明 |
|---|---|------|
| CHRG_NTC_OT_TEMP_VALUE | 600 | 充电过温 (60.0°C, 单位 0.1°C) |
| DISG_NTC_OT_TEMP_VALUE | 650 | 放电过温 (65.0°C) |

## 循环寿命 CV 递减

| 宏 | 值 | 说明 |
|---|---|------|
| CONFIG_CYCLE_CV_REDUCTION_ENABLE | 1 | 使能 CV 递减 |
| CYCLE_CV_TIER1_COUNT | 68 | 68 次后 CV -100mV |
| CYCLE_CV_TIER1_OFFSET | 100 mV | |
| CYCLE_CV_TIER2_COUNT | 135 | 135 次后 CV -150mV |
| CYCLE_CV_TIER2_OFFSET | 150 mV | |
| CYCLE_CV_TIER3_COUNT | 200 | 200 次后 CV -200mV |
| CYCLE_CV_TIER3_OFFSET | 200 mV | |

## 功能开关

| 宏 | 值 | 说明 |
|---|---|------|
| CONFIG_WPC_SUPPORT | 1 | 无线充电支持 |
| CONFIG_USBA_SUPPORT | 0 | USB-A 放电 (EVK 禁用) |
| BUCKBOOST_USED_NU6805 | 1 | 使用 NU6805 |
| BUCKBOOST_USED_NU6801 | 0 | NU6801 禁用 |
| CONFIG_USE_NTC_FOR_CHAGER | 1 | NTC 温度保护 |
| CONFIG_SUPPORT_PPS_CHAGER | 0 | PPS 充电禁用 |
| CONFIG_USE_TYPEC_DOUBLE_MOS | 1 | 双 MOS TypeC |
| ENABLE_EPP_FUNC | 0 | EPP 禁用 |
| OPTION_SAMSUNG_PPDE | 1 | 三星 PPDE 协议 |
| OPTION_FOD_ENABLE | 1 | FOD 异物检测 |
| CONFIG_DEADBATT_SLEEP_SUPPORT | 1 | 死电池休眠 |

## USB Bridge 配置

| 宏 | 值 | 说明 |
|---|---|------|
| CONFIG_USB_BRIDGE_ENABLE | 1 | USB Bridge 总开关 |
| CONFIG_TRIPLE_CLICK_COMM_ENABLE | 1 | 三击进入 USB 通信模式 |
| CONFIG_USB_COMM_LED5_BLINK | 1 | USB 通信时 LED5 闪烁 |
| CONFIG_USB_COMM_LED4_WB_STATE | 1 | LED4 显示 WB7720 状态 |

## 硬件参数

| 宏 | 值 | 说明 |
|---|---|------|
| CONFIG_DISCHG_IBAT_LIMIT | 0x04 | 放电 IBAT 限流索引 (8A) |
| CONFIG_TYPEC_MOS_R | 8 | TypeC FET Rds(on) 8mΩ |
| CONFIG_TYPEC_LIGHT_CURRENT | 60 | 轻载检测 60mA |

## 时间窗口

| 宏 | 值 | 说明 |
|---|---|------|
| EXCEPTION_WINDOW_SECONDS | 120 | 异常记录窗口 2 分钟 |

## 交叉引用

- 电池保护: [../debug/patterns.md](../debug/patterns.md)
- NU6805 寄存器: [../chips/nu6805.md](../chips/nu6805.md)
- 系统概览: [../architecture/system-overview.md](../architecture/system-overview.md)
