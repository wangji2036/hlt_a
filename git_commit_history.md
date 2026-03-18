# Git Commit History

## 修改记录索引

| 序号 | 日期 | 类型 | 简述 | 文件 |
|------|------|------|------|------|
| 1 | 2026-03-14 | feat | 有线充放电 PDO 上调至 35W | `fml/tcpm.c` |
| 2 | 2026-03-14 | fix | 恢复 PB4 触摸按键 GPIO 唤醒（去屏蔽） | `app/sleep.c` |
| 3 | 2026-03-14 | fix | 移除 PB3 有线快充指示灯，仅保留无线充电指示 | `app/led.c` |

---

## Commit

### Basic Info
- **DateTime**: 2026-03-17 00:10
- **Scope**: NU6805 bus current 调试
- **Files**:
  - `hal/nu6805.c`
- **Functions / Defines**
  - `hal_nu6805_buckboost_get_bus_current`

### Change Summary
1. 在 NU6805 充电模式读取 bus current 时增加一次性打印（进入充电模式后的首次读取打印一次，避免刷屏）。

### Rationale
用于确认 `mode=1 (BUCKBOOST_CHAGER_MODE)` 场景下 NU6805 的 Ibus 采样值是否合理，并辅助定位过流/过压相关问题。

### Risk / Impact
- 仅增加一次 `printk`（带限频），对时序影响极小。
- 不改变 I2C 读写流程与返回值，仅增加调试输出。

### Verification
- 正常路径：进入充电模式后观察串口，确认出现一次 `nu6805 bus_current(chg): ...` 打印。
- 边界条件：反复在充电/非充电模式间切换，确认每次重新进入充电模式都会打印一次。
- 异常/保护路径：在保护触发导致模式退出/重进时，确认打印节奏仍符合“每次进入充电模式打印一次”。

### Rollback
删除 `hal_nu6805_buckboost_get_bus_current()` 中新增的 `chg_printed` 以及对应 `printk` 分支即可。

### Open Questions
- 无

---

## Commit

### Basic Info
- **DateTime**: 2026-03-17 00:00
- **Scope**: NU6805 buck-boost HAL
- **Files**:
  - `hal/nu6805.c`
- **Functions / Defines**:
  - `hal_nu6805_buckboost_set_mode`

### Change Summary
1. 在 NU6805 模式设置函数中增加对 `REG_Mode_Control` 的回读。
2. 新增串口打印，输出 `woke_mode`、写入值与回读值，用于判断模式配置是否真正生效。

### Rationale
调试 NU6805 充放电模式切换问题，需要确认 I2C 写寄存器后芯片内部模式寄存器是否正确更新；通过写后回读与 log 对比，可以快速区分“写失败 / 被其他路径覆盖 / 状态机调用时机异常”等不同根因方向。

### Risk / Impact
- 仅新增一次 I2C 读与一次 `printk`，对时序和功耗影响可忽略。
- 串口打开时 log 增加，极端情况下可能略微拉长相关路径的执行时间，但在当前 36 MHz + 任务模型下风险低。

### Verification
- 正常路径：在充电和放电两条路径上各触发一次 `hal_nu6805_buckboost_set_mode`，检查串口输出：
  - `write` 与 `read` 是否一致；
  - 不同 `woke_mode` 下回读值是否符合预期（例如 0x01 放电、0x10 充电）。
- 边界条件：在保护/恢复场景（如 OCP 后恢复）触发模式切换，确认模式寄存器读值与预期一致。
- 异常/保护路径：若有 I2C 错误或异常模式值，通过 log 观察是否出现 `read` 与 `write` 不一致，作为后续调试入口。

### Rollback
将 `hal_nu6805_buckboost_set_mode` 函数恢复为仅写模式寄存器、不回读和不打印的旧版本，重新编译烧录即可。

### Open Questions
- 当前模式枚举 `buckboost_mode` 与 0x01 / 0x10 之间的映射是否完全与 NU6805 数据手册一致，后续需要结合文档再做一次核对。

---

## Commit

### Basic Info
- **DateTime**: 2026-03-17 00:05
- **Scope**: NU6805 OVP 保护调试
- **Files**:
  - `power/buckboost.c`
- **Functions / Defines**:
  - `buckboost_protection_handle`

### Change Summary
1. 在 NU6805 充电模式 OVP 判定分支中增加详细日志打印，输出 `adc_vbus` 和计算后的 `ovp_value`。

### Rationale
现场观测到 NU6805 在 20V 充电场景下频繁进入 `status=0x20` 保护锁，需要确认是 VBUS OVP 阈值本身偏紧，还是 VBUS 采样存在尖峰/过冲。通过打印 `NU6805 OVP: vbus=..., ovp_th=...`，可以将实际过压点量化出来，用于后续重新评估 OVP 阈值与充电电流配置。

### Risk / Impact
- 仅新增一行 `printk`，对时序和保护行为无影响。
- 串口开启时 log 量略有增加，极端高频 OVP 触发场景下需要注意串口带宽，但在当前场景可接受。

### Verification
- 正常路径：在 5V/9V 充电场景下运行，确认未触发 `NU6805 OVP` 打印。
- 边界条件：在 20V 充电场景下复现保护锁，记录触发前的 `vbus` 与 `ovp_th` 数值，判断是否存在明显过冲。
- 异常/保护路径：对比 `Flaut State = 0x20` 与 `protect lock =0x20` 前后的 OVP 打印，验证 `0x20` 的确由 VBUS OVP 触发。

### Rollback
删除 `NU6805 OVP: vbus=..., ovp_th=...` 这行 `printk`，恢复为原有仅置位 `VBUS_FUALT_VBUS_OVP` 的代码即可。

### Open Questions
- 针对 20V 档位，是否需要根据线损与适配器过冲特性单独提高 OVP 阈值，仍待进一步实测与规范核对。

---

## Commit 1: feat(pdo): 有线充放电 PDO 上调至 35W

## Commit

### Basic Info
- **DateTime**: 2026-03-17 00:15
- **Scope**: PD Sink 20V/15V 电流上限调整
- **Files**:
  - `app/port_manager.c`
- **Functions / Defines**:
  - `port_enum_port_snk_setvolt`

### Change Summary
1. 将原先基于 `>=18000` / `>=14000` 的电流钳位改为显式 20V/15V 分支：
   - 20V 档：`ibus_limit` 最大 1750mA（约 35W）。
   - 15V 档：`ibus_limit` 最大 2330mA（约 35W）。

### Rationale
在 PD Sink 充电场景下，希望 20V/15V 档稳定做到约 35W，同时避免高压大电流导致线缆、接口或 NU6805 过载；按功率等价关系 \(P = V \times I\) 明确限制 20V/15V 的最大电流更直观、可控。

### Risk / Impact
- 20V 档最大电流被限制在 1.75A，适配器若宣称更高电流（例如 2A），实际只会取不超过 1.75A，可能在个别大功率适配器上略微保守。
- 15V 档最大电流为 2.33A，与之前逻辑基本一致，对用户体验影响有限。

### Verification
- 正常路径：使用支持 20V/15V 的 PD 适配器，确认 Sink 协商后测得输入功率约为 35W，`vbus` 与 `ibus` 波动在合理范围内。
- 边界条件：在电池高/低电压、不同线缆阻抗下重复测试，确认 20V/15V 档不会超过 35W 左右，也不会出现过早降档。
- 异常/保护路径：配合 NU6805 保护 log，确认在极端负载下若触发保护，行为与预期一致（安全优先，能正确掉回 5V）。

### Rollback
将 `port_enum_port_snk_setvolt` 中 20V/15V 钳位逻辑恢复为原来的 `>=18000` / `>=14000` 条件与 1850/2330 比较，并重新编译烧录。

### Open Questions
- 是否需要针对个别高功率适配器放宽 20V 档电流限制（例如 2.0A），仍待后续温升与线损实测评估。

---

## Commit

### Basic Info
- **DateTime**: 2026-03-17 00:20
- **Scope**: PD Sink 固定电压档位电流统一到 35W
- **Files**:
  - `app/port_manager.c`
- **Functions / Defines**:
  - `port_enum_port_snk_setvolt`

### Change Summary
1. 在 PD Sink 固定 PDO 分支中，对 5V/9V/12V/15V/20V 的 `ibus_limit` 统一按约 35W 功率上限进行钳位：
   - 5V：最大 3.0A
   - 9V：最大 3.0A
   - 12V：最大 2.91A
   - 15V：最大 2.33A
   - 20V：最大 1.75A

### Rationale
希望 PD Sink 在所有常见固定电压档位下都遵守统一的 35W 功率曲线，避免部分适配器宣称更大电流导致线缆、接口或内部 DC-DC 超出设计功率，同时保证 5V/9V/12V 档与 15V/20V 一致的功率体验。

### Risk / Impact
- 如果适配器某些电压档位提供的电流上限低于上述值，将按适配器上限为准（本改动只做上限钳位，不会强行提升电流）。
- 对高功率适配器而言，本机在低压档（如 5V）最多仍然只取 3A，可能略低于适配器能力，但能保证统一的 35W 散热/线损设计。

### Verification
- 正常路径：在 5V/9V/12V/15V/20V 各档位下，用 PD 负载测量 VBUS 与 Ibus，确认功率约束在对应 35W 上限附近，不出现明显超功率。
- 边界条件：切换不同适配器和线缆，验证最大电流不会超过上述阈值，且对 5V/9V 档仍能满足常规充电功率需求。
- 异常/保护路径：配合 NU6805 保护与 OVP 日志，确认在极端场景下不会发生因电流过高引起的保护失效。

### Rollback
删除或恢复 `port_enum_port_snk_setvolt` 中对 5V/9V/12V/15V/20V 的 `ibus_limit` 钳位分支，使其仅依赖适配器声明的 `pdo_max_current` 即可。

### Open Questions
- 是否需要对某些档位（例如 5V）单独采用更保守的电流（如 2.4A）以兼顾 USB-A 线缆兼容性，后续可根据实测再微调。

---

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
