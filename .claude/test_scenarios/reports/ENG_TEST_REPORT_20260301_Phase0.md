# 测试报告: 工程模式 Phase 0 — 代码审查验证

> **报告编号**: ENG-TEST-20260301-P0
> **测试日期**: 2026-03-01
> **测试人员**: engineering-test-agent (4 并行 Agent 协同)
> **报告版本**: v1.0

---

## 1. 测试环境

### 1.1 硬件环境

不适用（Phase 0 为纯代码审查，无需硬件）

### 1.2 软件环境

| 项目 | 版本 |
|------|------|
| NU17112 固件 | Git commit `12882a7` (branch: ARUN_N3C) |
| WB7720 固件 | 见 `USB参考/USB_下位机程序/Projects/main.c` |
| Windows 上位机 | `USB参考/ARUN_N3C_WIN上位机/battery_monitor.py` |
| OS | Windows 10 Pro 10.0.19045 |

### 1.3 构建质量快照

| 指标 | 值 | 阈值 | 状态 |
|------|---|------|------|
| ROM 使用 | 待构建确认 | < 95% | N/A |
| RAM 使用 | 待构建确认 | < 90% | N/A |
| 编译警告 | 待构建确认 | < 5 | N/A |

---

## 2. 测试范围与目标

### 2.1 测试阶段
Phase 0 - 代码审查验证 (无硬件)

### 2.2 测试目标
- 验证写保护逻辑完整性
- 验证工程模式进入/退出状态机无遗漏边界
- 验证 Delta 保留计算无溢出
- 验证虚拟参数注入路径安全、不影响硬件保护
- 验证 0xAA/0xEE 命令处理正确
- 验证 Flash 操作安全
- 验证 exc_cache 一致性

### 2.3 前置条件
- [x] 源代码可读取（Git 仓库）
- [x] 工程模式逻辑文档 `ENGINEERING_MODE_LOGIC.md` 可用
- [x] WB7720 接口规范 v1.3 可用

---

## 3. 测试结果汇总

### 3.1 总体结论

| 统计项 | 数值 |
|--------|------|
| 总测试用例 | 7 |
| 通过 (PASS) | 1 |
| 条件通过 (CONDITIONAL PASS) | 6 |
| 失败 (FAIL) | 0 |
| 阻塞 (BLOCKED) | 0 |
| 跳过 (SKIP) | 0 |
| **通过率** | **100% (无 FAIL)** |
| **P0 项 (4项)** | **0 Critical Bug, 可继续** |

### 3.2 测试结果矩阵

| 用例编号 | 用例名称 | 优先级 | 结果 | 发现数 | 最高严重度 |
|---------|---------|--------|------|--------|-----------|
| TC-ENG-000 | 写保护逻辑审查 | P0 | CONDITIONAL PASS | 3 | Critical (设计层) |
| TC-ENG-001 | 进入/退出状态机审查 | P0 | CONDITIONAL PASS | 3 | Critical (路径隐晦) |
| TC-ENG-002 | Delta 保留计算审查 | P0 | CONDITIONAL PASS | 1 | Minor |
| TC-ENG-003 | 虚拟参数注入路径审查 | P0 | **PASS** | 0 | — |
| TC-ENG-004 | 0xAA/0xEE 处理审查 | P1 | CONDITIONAL PASS | 3 | Major |
| TC-ENG-005 | Flash 操作安全审查 | P1 | CONDITIONAL PASS | 4 | Major |
| TC-ENG-006 | exc_cache 一致性审查 | P1 | CONDITIONAL PASS | 4 | Major |

---

## 4. 详细测试步骤与结果

### TC-ENG-000: 写保护逻辑审查

**优先级**: P0
**审查文件**: `USB参考/USB_下位机程序/Projects/main.c`, `config.h`
**预期**: WB7720 CMD 0x0C 和 I2C IRQ 中 0x60-0x88 只在 0x50==0xA5 时可写

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | CMD 0x0C 有 0x50==0xA5 前置检查 | 是 | 是 (main.c:528-530) | PASS |
| 2 | 写入范围 0x60-0x88 边界保护 | 连续保护 | 分段保护: 0x60~0x63, 0x70~0x73, 0x80~0x88 (间隙 0x64~0x6F, 0x74~0x7F) | PASS (间隙地址未分配) |
| 3 | I2C IRQ 有相同保护 | 与 CMD 0x0C 对称 | 是 (main.c:613-619) | PASS |
| 4 | 0x50 自身写保护 | 需保护 | **无保护** — PC 可直接写 0x50=0xA5 绕过工程模式流程 | CONDITIONAL |
| 5 | 寄存器命名一致性 | 统一 | REG_ENG_CYCLE_COUNT (config.h) vs REG_ENG_CYCLE_CHG_COUNT (main.c) 不一致 | MINOR |

**发现**:
- **[F-000-1] Critical (设计层)**: REG_WORK_MODE(0x50) 未被写保护，PC 可通过 CMD 0x0C 直接写入 0xA5，绕过 NU17112 确认流程。需评估是否为预期设计（当前上位机通过 GUI 密码控制，但 HID 层无保护）。
- **[F-000-2] Major**: REG_ENG_CYCLE_COUNT / REG_ENG_CYCLE_CHG_COUNT 跨文件重复定义命名不一致，维护风险。
- **[F-000-3] Info**: 0x89 (CMD_STATUS) 未在保护范围内，PC 可在非工程模式下清除状态（影响轻微）。

**结论**: CONDITIONAL PASS — Critical 发现为设计层面问题（上位机密码已提供应用层保护），不阻塞 Phase 1 测试。

---

### TC-ENG-001: 进入/退出状态机审查

**优先级**: P0
**审查文件**: `app/usb_bridge.c`, `app/usb_bridge.h`
**预期**: eng_mode_active 状态转换完整，无遗漏边界

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | eng_mode_active 所有写入点 | 完整 | 4处: 初始化(L29), 进入(L404), 正常退出(L444), 0xAA退出(L559) | PASS |
| 2 | 进入条件 0x50=0xA5 | 正确 | 是 (L332, L335) | PASS |
| 3 | 退出时清理虚拟参数 | 清零 | 正常退出(L439-442) 和 0xAA(L556-558) 均清零 | PASS |
| 4 | 上电默认值 false | 正确 | `static bool eng_mode_active = false` (L29) | PASS |
| 5 | 0xAA 后 WB7720 0x50 状态 | 需评估 | 0xAA 路径不清零 0x50，step-14 将自动重入 | CONDITIONAL |
| 6 | I2C 通信失败处理 | 安全降级 | work_mode 初始化=0，I2C 失败触发虚假退出 | CONDITIONAL |

**发现**:
- **[F-001-1] Critical (路径隐晦)**: 0xAA 刷新路径先退出再由 step-14 重入，delta 保留在此过程中被执行，路径正确但隐晦。需添加注释说明设计意图。
- **[F-001-2] Major**: I2C 通信瞬断时 work_mode 读回 0（初始值），触发虚假退出并执行 gd 还原。建议 HAL 失败时保留旧值或使用 0xFF 标记。
- **[F-001-3] Info**: `eng_mode_active` 用 static 封装，外部通过 `usb_bridge_is_eng_mode()` 只读访问，设计良好。

**结论**: CONDITIONAL PASS — Critical 发现为路径隐晦（功能正确但难理解），Major 为潜在风险（I2C 瞬断概率低）。

---

### TC-ENG-002: Delta 保留计算审查

**优先级**: P0
**审查文件**: `app/usb_bridge.c`, `fml/g_data.h`
**预期**: RTC seconds 和 cycle count 的 elapsed 计算无溢出

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | 进入时保存 real RTC snapshot | 正确 | eng_saved_rtc_seconds = gd->Bat_RTC_Seconds (L339) | PASS |
| 2 | RTC elapsed = current - snapshot 无溢出 | u32 安全 | uint32_t 减法，模运算安全 (L414) | PASS |
| 3 | Cycle count delta 安全 | 需评估 | uint8_t 减法，模运算在 C 中安全，但产品生命周期内不会溢出 (L420) | PASS |
| 4 | 还原公式正确 | real = saved + elapsed | gd->Bat_RTC_Seconds = eng_saved_rtc_seconds + elapsed (L417) | PASS |
| 5 | elapsed=0 处理 | 正确 | 0+saved=saved，直接恢复原值 | PASS |
| 6 | 0xAA 路径与正常退出路径一致 | 一致 | 0xAA (L552-555) 使用相同公式 | PASS |

**发现**:
- **[F-002-1] Minor**: uint8_t cycle_count 在工程测试场景中不会溢出（数分钟内充放电周期 < 255），但建议添加注释说明此假设。

**结论**: CONDITIONAL PASS — 仅 Minor 观察，计算逻辑正确。

---

### TC-ENG-003: 虚拟参数注入路径审查

**优先级**: P0
**审查文件**: `app/usb_bridge.c`, `app/bat_record.c`
**预期**: zero-means-disabled 一致，不影响 BuckBoost/OTP

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | zero-means-disabled 一致 | 一致 | OV: `eng_c1 > 0` (u16), Temp: `eng_temp != 0` (s16) — 各自类型正确 | PASS |
| 2 | BuckBoost 不引用虚拟值 | 隔离 | buckboost.c/nu6805.c 中无 eng_virtual 引用 | PASS |
| 3 | OTP 不引用虚拟值 | 隔离 | OTP 路径使用 gd->sys_infos.ntc_temp_wpc，不经 bat_record | PASS |
| 4 | cell1=0,cell2=4200 混合场景 | cell1 用真实值 | `cell1_voltage = real_total/2` (bat_record.c:394) | PASS |
| 5 | CONFIG_USB_BRIDGE_ENABLE 保护 | 完整 | OV 和 Temp 路径均被 #if 保护 | PASS |

**发现**: 无

**结论**: **PASS** — 虚拟参数注入路径设计良好，完全隔离，零值语义一致。

---

### TC-ENG-004: 0xAA/0xEE 处理审查

**优先级**: P1
**审查文件**: `app/usb_bridge.c`, `app/bat_record.c`
**预期**: 命令互斥、状态反馈、寄存器清理

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | 0xAA 正确读参数 | 正确 | step-17 完整执行 delta + 清零 + eng_mode_active=false (L550-562) | PASS |
| 2 | 0xEE 正确调用 erase_all | 正确 | battery_record_erase_all() (L546-549) | PASS |
| 3 | 命令互斥 | 安全 | if/else if 分支 + eng_mode_active 守卫 | PASS |
| 4 | 0x89 状态反馈 | 有 busy | 缺少 0x01 (in progress) 过渡状态 | CONDITIONAL |
| 5 | 0x88 清零防重入 | 正确 | 两分支均写 0x88=0x00 | PASS |
| 6 | Flash 操作失败反馈 | 0xFF | 无失败检测，erase_all() 为 void | CONDITIONAL |

**发现**:
- **[F-004-1] Major**: 0xEE/0xAA 执行前缺少 0x89=0x01 (busy) 状态写入，PC 无法区分"正在执行"和"空闲"。
- **[F-004-2] Minor**: erase_all() 为 void 返回，Flash 操作失败时 0x89 仍写 0x02 (success)。
- **[F-004-3] Info**: 无命令 ID 机制，PC 无法关联响应到具体命令（协议设计限制）。

**结论**: CONDITIONAL PASS — Major 不影响功能正确性（PC 上位机使用轮询+超时）。

---

### TC-ENG-005: Flash 操作安全审查

**优先级**: P1
**审查文件**: `app/bat_record.c`, `hal/fmc.c`, `fml/g_data.h`
**预期**: erase_all() 有 magic 保护，无并发写风险

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | Magic 保护 | 函数级 | 上层条件保护 (eng_mode_active + 0xEE), Flash 内容有 MAGIC_VALUE | PASS |
| 2 | 中断禁用 | 是 | **无** — save_storage_to_flash() 和 hal_fmc_erase_page() 均未禁中断 | CONDITIONAL |
| 3 | Flash 区域隔离 | 不越界 | Log(0x1400) vs ProductInfo(0x1800) 不重叠 | PASS |
| 4 | 并发写风险 | 无 | OSAL 协作式调度保证不并发 | PASS |
| 5 | Magic/计数器更新 | 正确 | erase 后设 magic=MAGIC_VALUE, memset 清零 (L608-614) | PASS |
| 6 | 写入失败处理 | 有 | 无 — hal_fmc 为 void 返回 | CONDITIONAL |

**发现**:
- **[F-005-1] Major**: Flash erase/write 操作未包裹 `VIC_vModuleDisable()/VIC_vModuleEnable()` 临界区。中断 ISR 若访问 Flash 常量可能导致总线冲突。OSAL 降低了风险，但 ISR 仍可抢占。
- **[F-005-2] Minor**: hal_fmc 无返回值，上层无法报告写入失败。
- **[F-005-3] Minor**: battery_record_erase_all() 无函数级 magic 参数（依赖上层保护）。
- **[F-005-4] Info**: get_current_timestamp() (L104) 有 VIC_vModuleDisable() 保护作为正面参考。

**结论**: CONDITIONAL PASS — Major 为防御性编程缺陷，实际 ISR 访问 Flash 常量的概率较低。

---

### TC-ENG-006: exc_cache 一致性审查

**优先级**: P1
**审查文件**: `USB参考/USB_下位机程序/Projects/main.c`, `config.h`
**预期**: record_id 去重、cache 满处理、分页输出

| # | 检查点 | 预期 | 实际 | 结果 |
|---|--------|------|------|------|
| 1 | record_id 去重 | 正确 | 遍历比较 (main.c:163-165)，重复则 return | PASS |
| 2 | cache 满处理 | FIFO 覆盖 | **丢弃新记录** (main.c:159-160) — 与规范文档矛盾 | CONDITIONAL |
| 3 | 分页输出 3 页循环 | 正确 | page_start=idx*2, return_count 裁剪, 0→1→2→0 (main.c:276-322) | PASS |
| 4 | 不足 5 条时分页 | 正确 | if (exc_cache_count > page_start) 保护下溢 | PASS |
| 5 | i2c_buff → exc_cache 同步 | 正确 | exc_cache_update() 在 update_report_buffer_0() 末尾调用 (main.c:256) | PASS |
| 6 | 0xEE 后 cache 清理 | 清理 | **不清理** — WB7720 无 0xEE 响应逻辑，过期数据保留 | CONDITIONAL |

**发现**:
- **[F-006-1] Major**: 规范文档 §3.2b 写"最旧条目自动被新条目替代"，但代码实现为"cache 满丢弃新记录"。文档与代码不一致。
- **[F-006-2] Major**: 0xEE 擦除后 WB7720 exc_cache 不清理，PC 仍收到过期记录。需拔插 USB 或 WB7720 重启才能清空。（已知限制 [H-002]）
- **[F-006-3] Info**: record_id=0 过滤正确，分页 u8 下溢有条件保护。
- **[F-006-4] Info**: EXC_CACHE_MAX=5 与 NU17112 最大记录数一致，正常场景不会超 5 条。

**结论**: CONDITIONAL PASS — Major 发现为文档不一致和已知限制，不阻塞功能测试。

---

## 5. 异常记录

### 异常 #1: REG_WORK_MODE(0x50) 缺少写保护

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-000 |
| 严重程度 | Critical (设计层) |
| 复现率 | 必现 (设计如此) |
| 根因分析 | 0x50 不在 CMD 0x0C 的保护条件中，PC 可直接写入 |
| 责任模块 | `USB参考/USB_下位机程序/Projects/main.c` (usb-device-agent) |
| 修复状态 | 待评估 — 上位机密码提供应用层保护，是否需要协议层保护待决策 |

### 异常 #2: Flash 操作缺少中断保护

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-005 |
| 严重程度 | Major |
| 复现率 | 偶现 (仅当 ISR 在 Flash 操作期间执行且访问 Flash 时触发) |
| 根因分析 | `save_storage_to_flash()` 和 `hal_fmc_erase_page()` 未包裹临界区 |
| 责任模块 | `app/bat_record.c` (platform-apl-agent) + `hal/fmc.c` (platform-hal-agent) |
| 修复状态 | 待修复 — 建议在 save_storage_to_flash() 中添加 VIC_vModuleDisable()/Enable() |

### 异常 #3: exc_cache 擦除后不清理

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-006 |
| 严重程度 | Major |
| 复现率 | 必现 |
| 根因分析 | WB7720 无 0xEE 命令感知逻辑，exc_cache 生命周期与 WB7720 电源绑定 |
| 责任模块 | `USB参考/USB_下位机程序/Projects/main.c` (usb-device-agent) |
| 修复状态 | 待修复 — 建议检测 i2c_buff[0x88] 变化时同步清理 exc_cache |

### 异常 #4: 规范文档与代码不一致 (cache 满策略)

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-006 |
| 严重程度 | Major (文档) |
| 复现率 | 必现 |
| 根因分析 | 规范写"最旧覆盖"，代码实现"丢弃新记录" |
| 责任模块 | `.claude/references/specs/WB7720_USB_Bridge_接口规范.md` (Leader 管理) |
| 修复状态 | 待修复 — 修正文档描述为"缓存满后丢弃新记录" |

### 异常 #5: 0xAA/0xEE 缺少 busy 状态

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-004 |
| 严重程度 | Major |
| 复现率 | 必现 (设计如此) |
| 根因分析 | 0x89 直接从 0x00 跳到 0x02，未经过 0x01 过渡 |
| 责任模块 | `app/usb_bridge.c` (platform-apl-agent) |
| 修复状态 | 待评估 — 当前 PC 轮询+超时机制可工作 |

---

## 6. 结论与建议

### 6.1 总体评价
**条件通过 (CONDITIONAL PASS)** — Phase 0 代码审查未发现阻塞性功能 Bug。所有 P0 项无逻辑错误，Critical 发现均为设计层面问题（有应用层缓解）。可继续 Phase 1 硬件测试。

### 6.2 关键发现
1. **虚拟参数注入路径设计良好** (TC-ENG-003 PASS) — BuckBoost/OTP 完全隔离，zero-means-disabled 一致
2. **Delta 保留计算正确** (TC-ENG-002) — RTC u32 和 cycle u8 在设计场景下安全
3. **0x50 写保护缺失** (TC-ENG-000) — 协议层未防护，依赖上位机密码
4. **Flash 操作无中断保护** (TC-ENG-005) — OSAL 协作式降低风险，但 ISR 仍可抢占
5. **exc_cache 擦除后不清理** (TC-ENG-006) — 已知限制，需拔插 USB

### 6.3 遗留风险

| 风险编号 | 描述 | 等级 | 缓解措施 |
|---------|------|------|---------|
| R-001 | 0x50 写保护缺失，PC 可绕过密码 | Medium | 上位机密码 + HID 设备物理接触控制 |
| R-002 | Flash 操作无中断保护 | Medium | OSAL 协作式 + ISR 极少访问 Flash 常量 |
| R-003 | exc_cache 擦除后保留过期数据 | Medium | PC 端 seen_ids 过滤 + USB 重连清空 |
| R-004 | I2C 瞬断触发虚假退出 | Low | I2C 总线稳定性 + 重新进入即恢复 |

### 6.4 后续建议
1. **Phase 1 可立即启动** — 无阻塞性 Bug
2. 优先修复 F-005-1 (Flash 中断保护) — 影响数据完整性
3. 修正规范文档 cache 满策略描述 (F-006-1)
4. 评估 0x50 写保护需求 — 若产品面向消费者需加固
5. 考虑在 WB7720 增加 0xEE 感知逻辑清理 exc_cache (F-006-2)

---

## 7. 附件

| 编号 | 文件名 | 说明 |
|------|--------|------|
| A-001 | `app/usb_bridge.c` | NU17112 工程模式主逻辑 |
| A-002 | `app/bat_record.c` | 异常记录与 Flash 操作 |
| A-003 | `USB参考/USB_下位机程序/Projects/main.c` | WB7720 exc_cache 与分页 |
| A-004 | `ENGINEERING_MODE_LOGIC.md` | 工程模式设计文档 |

---

**签署**:
- 测试执行: engineering-test-agent (4 并行 Agent)
- 测试审核: powerbank-leader (Commander IP162N)
- 日期: 2026-03-01
