# Git Commit History

## 修改记录索引

| 序号 | 日期 | 类型 | 简述 | 文件 |
|------|------|------|------|------|
| 1 | 2026-03-14 | feat | 有线充放电 PDO 上调至 35W | `fml/tcpm.c` |
| 2 | 2026-03-14 | fix | 恢复 PB4 触摸按键 GPIO 唤醒（去屏蔽） | `app/sleep.c` |
| 3 | 2026-03-14 | fix | 移除 PB3 有线快充指示灯，仅保留无线充电指示 | `app/led.c` |

---

## Commit 1: feat(pdo): 有线充放电 PDO 上调至 35W

| 字段 | 内容 |
|------|------|
| **DateTime** | 2026-03-14 |
| **Scope** | USB PD / PDO 配置 |
| **Files** | `fml/tcpm.c` |
| **Functions / Defines** | `source_pdo[]`（NU6805 分支）、`source_pdo[]`（NU6801 分支）、`sink_pdo[]` |

### 变更统计
- 修改文件数：1
- 修改 PDO 数组数：3（source × 2 分支 + sink × 1）
- 每数组修改行数：3（12V / 15V / 20V 档位）

### 详细变更分析

#### source_pdo[]（NU6805 & NU6801 双分支）

| 电压 | 原电流 | 新电流 | 原功率 | 新功率 | 原→新（行） |
|------|--------|--------|--------|--------|------------|
| 5V   | 3000 mA | 3000 mA | 15W | 15W | 不变 |
| 9V   | 3000 mA | 3000 mA | 27W | 27W | 不变 |
| 12V  | 2500 mA | **2910 mA** | 30W | **34.92W** | `PDO_FIXED(12000, 2500, 0)` → `PDO_FIXED(12000, 2910, 0)` |
| 15V  | 2000 mA | **2330 mA** | 30W | **34.95W** | `PDO_FIXED(15000, 2000, 0)` → `PDO_FIXED(15000, 2330, 0)` |
| 20V  | 1500 mA | **1750 mA** | 30W | **35.00W** | `PDO_FIXED(20000, 1500, 0)` → `PDO_FIXED(20000, 1750, 0)` |

#### sink_pdo[]

| 电压 | 原电流 | 新电流 | 变更 |
|------|--------|--------|------|
| 12V  | 2500 mA | **2910 mA** | `PDO_FIXED(12000, 2500, 0)` → `PDO_FIXED(12000, 2910, 0)` |
| 15V  | 2000 mA | **2330 mA** | `PDO_FIXED(15000, 2000, 0)` → `PDO_FIXED(15000, 2330, 0)` |
| 20V  | 1500 mA | **1750 mA** | `PDO_FIXED(20000, 1500, 0)` → `PDO_FIXED(20000, 1750, 0)` |

#### 未修改（保护性保留）

| 数组 | 功能 | 理由 |
|------|------|------|
| `source_pdo1[]` | 双口限功率模式（NU6805） | 功率分配策略由 port-manager 另行定义，不在本次范围 |
| `source_pdo_ntc[]` | NTC 过温保护降级 | 安全保护策略，强制保持 5V/2A |

### Change Summary
1. 将 `source_pdo[]`（NU6805/NU6801 两分支）中 12V/15V/20V 档位电流上调至 35W 等效值（2910/2330/1750 mA）
2. 将 `sink_pdo[]` 同步更新为相同 35W 配置，充放电能力对齐
3. `source_pdo1[]`（限功率模式）与 `source_pdo_ntc[]`（NTC保护）保持原值不变

### Rationale
产品功率规格从 30W 上调至 35W，需同步更新 USB PD Source/Sink PDO 广播能力值，使充放电双向均支持 35W。

### Risk / Impact

**合规性（⚠️ 需关注）**:
- USB PD 规范：PDO 电流以 10mA 为单位，2910/2330/1750 均为整数倍，编码合法 ✓
- WPC 合规：仅 TypeC 有线端修改，不影响 Qi PDO 广播
- JEITA/NTC：过温保护 `source_pdo_ntc[]` 未修改，安全路径不受影响 ✓

**互操作性（⚠️ 需测试）**:
- 接收端请求 35W 时，BuckBoost（NU6805/NU6801）需确认实际可稳定输出 35W
- 部分老旧设备仅识别标准档位（如 12V/3A），PDO 缩减非整档位（2.91A）可能触发 Capability Mismatch

**功率影响**:
- Source PDO 最大功率：30W → 35W（+17%）
- Sink PDO 最大功率：30W → 35W（+17%）
- 热设计余量需重新评估（尤其 12V 档从 2.5A → 2.91A，+400mA）

### Verification

#### 正常路径
- 连接支持 35W 的 PD 充电头（Sink 模式），协商至 20V/1.75A，稳定充电 ≥ 30min
- 连接支持 35W 的受电设备（Source 模式），协商至 20V/1.75A，观测 VBUS = 20V ±5%

#### 边界条件
- 连接仅支持 30W 设备，确认 PD fallback 正常（如协商至 15V/2A = 30W）
- 连接 5V-only 设备，确认 5V/3A = 15W 正常工作
- 双口同时使用：确认 port-manager 功率分配策略触发 `source_pdo1[]`（限功率模式）

#### 异常/保护路径
- 触发 NTC 过温保护，确认切换至 `source_pdo_ntc[]`（5V/2A），与本次修改无关 ✓
- 触发 OCP，确认 BuckBoost 限流保护在 35W 场景下正常工作
- 观测指标：VBUS 电压/电流、NTC 温度、UART log 中 PD 协商报文

### Rollback
将 `fml/tcpm.c` 中 `source_pdo[]`（两分支）和 `sink_pdo[]` 的 12V/15V/20V 电流值恢复：
- `PDO_FIXED(12000, 2910, 0)` → `PDO_FIXED(12000, 2500, 0)`
- `PDO_FIXED(15000, 2330, 0)` → `PDO_FIXED(15000, 2000, 0)`
- `PDO_FIXED(20000, 1750, 0)` → `PDO_FIXED(20000, 1500, 0)`
重新编译烧录即可回退。

### Open Questions
1. NU6805/NU6801 在 35W（20V/1.75A）场景下的实际输出效率和热表现是否满足要求？
2. 12V/2.91A 非整档位是否与目标市场主流受电设备兼容（需重点测试 Apple、Samsung、OPPO 设备）？
3. `source_pdo1[]`（双口限功率模式）是否需要同步上调（当前 ≈20W）？

---

## Commit 2: fix(sleep): 恢复 PB4 触摸按键 GPIO 唤醒功能

| 字段 | 内容 |
|------|------|
| **DateTime** | 2026-03-14 |
| **Scope** | 低功耗睡眠 / GPIO 唤醒 |
| **Files** | `app/sleep.c` |
| **Functions / Defines** | `SLP_vNormalToSleep()`、`SLP_vSleepToSleep()` |

### 变更统计
- 修改文件数：1
- 修改函数数：2
- 修改代码路径数：4（每函数 2 个 if/else 分支）

### 详细变更分析

| 行号 | 函数 | 分支 | 原 ITEN | 新 ITEN | 原 ITTP | 新 ITTP |
|------|------|------|---------|---------|---------|---------|
| ~325 | SLP_vNormalToSleep | ship_mode | 0（禁用） | **1（使能）** | 未设 | **0（下降沿）** |
| ~334 | SLP_vNormalToSleep | else | 0（禁用） | **1（使能）** | 2（双边沿） | **0（下降沿）** |
| ~449 | SLP_vSleepToSleep | ship_mode | 0（禁用） | **1（使能）** | 未设 | **0（下降沿）** |
| ~458 | SLP_vSleepToSleep | else | 0（禁用） | **1（使能）** | 2（双边沿） | **0（下降沿）** |

```diff
-		GPB->ITEN.BITS.PIN4 = 0; // disabled
-		GPB->ITTP.BITS.PIN4 = 2;
+		GPB->ITEN.BITS.PIN4 = 1;
+		GPB->ITTP.BITS.PIN4 = 0; //00:Falling Edge
```

### Change Summary
1. 将全部 4 处 `ITEN.PIN4 = 0`（禁用）改为 `ITEN.PIN4 = 1`（使能），恢复 PB4 GPIO 唤醒能力
2. 将所有 `ITTP.PIN4` 统一设为 `0`（下降沿触发），对应 PB4 低电平有效逻辑
3. 更新注释，去除"disabled"描述

### Rationale
原代码在所有 sleep 路径中均设 `ITEN = 0`，导致 PB4 完全无法从休眠唤醒系统，用户要求恢复。

### Risk / Impact
- 恢复后 PB4 低电平（下降沿）可触发 `RST_SRC_GPIO` 唤醒
- 需确认 PB4 引脚在休眠期间无噪声/漂移导致误唤醒

### Verification
- 正常路径：休眠后按下 PB4（低电平），系统从 `RST_SRC_GPIO` 唤醒激活
- 边界条件：同时按 PB4 + PC6，`touch_to_weakup = 0`；仅 PB4，`touch_to_weakup = 1`
- 异常路径：确认 PB4 浮空不会误唤醒

### Rollback
将 4 处 `ITEN.PIN4 = 1` 改回 `0`，重新编译烧录。

### Open Questions
- PB4 引脚在休眠期间是否存在外部上拉/下拉导致误唤醒？（需硬件确认）

---

## Commit 3: fix(led): 移除 PB3 有线快充指示灯功能

| 字段 | 内容 |
|------|------|
| **DateTime** | 2026-03-14 |
| **Scope** | LED 显示 / PB3 用途 |
| **Files** | `app/led.c` |
| **Functions / Defines** | `ui_update_led()` 内 3 处分支 |

### 变更统计
- 修改文件数：1
- 删除代码块数：3（每块 2 行注释 + 1 个 else if 块）

### 详细变更分析

删除内容（3 处相同）：

```c
// if (gd->vpwr>6200 && g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE)
// LED5: 快速充电/放电指示灯 - 设备被充电或放电时都点亮
else if (gd->vpwr > 6200
    && g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE
    && !buckboost_protection_flag
    && (g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE || gd->real_soc_show < 100)
)
{
    soc_show_ram_led |= 0x10;// fast LED is on
}
```

| 位置 | 分支 | 原行号（约） |
|------|------|--------------|
| 1 | 小电流模式 `g_port.is_mini_current_mode` | 216–227 |
| 2 | 正常显示 else 主分支 | 365–376 |
| 3 | `flash_flag == 4` 分支 | 422–433 |

### Change Summary
1. 去掉三处根据 `gd->vpwr > 6200` 与 `port_state[PORT0_INDEX]` 置位 `soc_show_ram_led |= 0x10` 的逻辑
2. PB3（LED5，bit4）不再因有线 QC/PD 快充点亮，仅保留无线充电相关显示（`0x20` 等逻辑未动）

### Rationale
PB3 定义为无线充电指示灯，用户要求去掉有线快充指示灯功能。

### Risk / Impact
- 有线快充/放电时蓝灯（PB3）不再亮，仅无线充电时由既有逻辑控制
- LED1–LED4 电量指示、无线 LED 判断与闪烁逻辑未修改

### Verification
- 有线 C 口 QC/PD 快充/放电：PB3 不亮
- 无线充电进行中：PB3 按原逻辑点亮/闪烁

### Rollback
恢复 3 处 `else if (gd->vpwr > 6200 ...)` 块及前置 2 行注释，重新编译烧录。

### Open Questions
- 无

---
