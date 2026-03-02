# Leader 决策经验

## 团队协调模式

### [L-001] 重构任务拆分策略
- 重构按"注释先行 -> 代码重构"两步走，避免一步到位引入错误
- 每轮重构控制在 3 项以内，超过则拆分为多轮
- 高风险低收益的重构明确列入"不做"清单并说明原因
- 来源: Port Manager R3/R4 重构实践

### [L-002] Plan Mode 使用模式
- 复杂重构必须先进 Plan Mode，列出每项重构的差异点和风险
- Plan 中明确写出"不做的事情"和原因，与用户对齐预期
- 来源: Port Manager R4 重构

## 代码审查经验

### [L-003] port0/port1 对称性审查
- 对比 port0/port1 时必须逐行 diff，宏保护差异容易遗漏
- 发现: port1 充电模式 USB-A gate 缺少 #if(CONFIG_USBA_SUPPORT) 保护（R4 修复）
- 教训: 对称函数的差异可能是 bug 也可能是有意设计，需逐项判断

### [L-004] 表驱动重构验证要点
- 必须验证优先级顺序不变（UNCONNECT > TRY_CONNECT > RESET_CHARGE）
- 必须验证端口优先级不变（PORT0 > PORT1 > PORT2 > PORT3）
- INHANDLING 阶段和 IDLE 阶段逻辑不同，不可混合重构

## 构建验证经验

### [L-005] CDS 构建环境
- C_INCLUDE_PATH 必须用 Windows 风格路径（分号分隔，反斜杠），通过 `cygpath -w` 转换
- makefile 首次需要 patch（修复中文乱码路径），patch 后 makefile.orig 存在则跳过
- 增量构建只编译修改的文件，全量 rebuild 需 make clean 先行
- CDS 路径含空格（"Program Files (x86)"），不能在 Bash 单行 inline 设置 PATH，必须写入脚本文件再 `bash script.sh`

### [L-016] 历史 commit 编译流程（已验证）

完整流程（避免踩坑）：

1. `git checkout <hash>` → detached HEAD
2. 写 `Debug/build_now.sh` 脚本（含 C_INCLUDE_PATH 生成 + make clean && make all）
3. `bash Debug/build_now.sh` 执行编译
4. **先** `cp Debug/PowerBankEvk_relsease.bin Debug/PowerBankEvk_<short_hash>.bin`（保存产物）
5. `make clean`（清理 .o/.d，防止切分支冲突）
6. `git checkout ARUN_N3C`
7. 确认保存的 bin/ihex 仍在，删除临时脚本

**关键 Bug（已修复）**: `ls "$d"*.h "$d"*.c "$d"*.S` 三者 AND 逻辑，无 .S 文件的目录返回非零 → C_INCLUDE_PATH 全空 → `regdef.h: No such file or directory`
**正确写法**: `ls "$d"*.h >/dev/null 2>&1 || ls "$d"*.c >/dev/null 2>&1 || ls "$d"*.S >/dev/null 2>&1`

## 知识回流记录

- [回流-001] USB-A gate #if 保护遗漏 -> 标记 [POTENTIAL_COMMON]，待其他项目验证
- [回流-002] port0/port1 connect_start 可安全提取公共函数 -> 标记 [POTENTIAL_COMMON]

## 工程模式联机测试架构决策

### [L-006] 工程模式测试三层验证体系
- **架构决策**: 工程模式联机测试分三层——单元测试(host PC 编译)、联机集成测试(通过 USB 上位机)、硬件回归测试(人工验证)
- **原因**: NU17112 是嵌入式 MCU 无法运行传统测试框架，必须通过 USB 桥接链路（NU17112→WB7720→PC）进行端到端测试
- **关键约束**: 每个测试步骤需考虑 846ms round-robin 延迟（47ms × 18 步），命令响应非实时
- **来源**: 工程模式联机测试体系设计 (2026-03)

### [L-007] 工程模式测试 Agent 拆分策略
- **决策**: 新建 `engineering-test-agent`，独立于 `platform-test-agent`
- **原因**: `platform-test-agent` 负责编译质量+回归测试（全平台级），工程模式测试需要跨三方系统（NU17112 + WB7720 + Windows App）的端到端验证能力，职责和工具链完全不同
- **边界**: engineering-test-agent 只管辖 `.claude/test_scenarios/engineering_mode_*` 文件和测试报告模板，不写产品代码
- **来源**: 工程模式测试体系设计 (2026-03)

### [L-008] 测试报告标准化
- **决策**: 所有测试报告统一使用 `.claude/test_scenarios/TEST_REPORT_TEMPLATE.md` 模板
- **模板结构**: 环境信息 → 测试矩阵 → 详细步骤 → 异常记录 → 结论与建议
- **来源**: 工程模式测试体系设计 (2026-03)

### [L-009] 虚拟参数注入只影响 bat_record.c
- **确认**: 虚拟电压/温度仅覆盖 `battery_record_update_overvoltage()` 和 `battery_record_update_overtemperature()` 中的检测逻辑
- **不影响**: BuckBoost 充放电 (用真实 ADC)、温度保护 (用真实 NTC)、LED 控制、端口管理
- **测试意义**: 工程模式测试可安全注入极端值而不影响硬件保护
- **来源**: ENGINEERING_MODE_LOGIC.md §5 + 代码审查确认

## Phase 0 代码审查关键发现 (2026-03-01)

### [L-010] 0x50 写保护缺失 — 设计层面问题
- **发现**: REG_WORK_MODE(0x50) 不在 WB7720 CMD 0x0C 写保护条件中，PC 可直接写入 0xA5 绕过上位机密码
- **缓解**: 上位机密码 + 物理接触控制提供应用层保护
- **决策**: 暂不修复，后续产品化时评估是否需要协议层保护
- **来源**: TC-ENG-000 审查

### [L-011] Flash 操作缺少中断保护
- **发现**: `save_storage_to_flash()` 和 `hal_fmc_erase_page()` 均未包裹 `VIC_vModuleDisable()/Enable()` 临界区
- **风险**: OSAL 协作式调度降低风险，但 ISR 仍可在 Flash 操作期间抢占
- **建议**: 在 save_storage_to_flash() 中添加临界区保护 — 分配给 platform-apl-agent
- **来源**: TC-ENG-005 审查

### [L-012] 规范文档与代码不一致 (exc_cache 满策略)
- **发现**: WB7720 接口规范 §3.2b 写"最旧条目自动被新条目替代"，代码实现为"丢弃新记录"
- **影响**: 正常场景不超 5 条，不会触发；但文档误导
- **修复**: 修正规范文档描述 — Leader 直接修正
- **来源**: TC-ENG-006 审查

### [L-013] 并行 Agent 审查效率经验
- **模式**: 将 7 个审查用例分为 4 组，每组 1-2 个 TC，由独立 Agent 并行执行
- **效果**: 4 个 Agent 并行 ~90s 完成，比串行估算 ~360s 节省 75%
- **适用**: 代码审查、独立模块测试等无依赖任务
- **来源**: Phase 0 执行实践

## 自动测试 Runner 里程碑 (2026-03-01)

### [L-014] 自动测试 Runner 架构落地
- **架构**: 单文件 headless runner (`phase1_test.py` v1.1)，三层结构：HID Transport → Protocol Parsing → Test Cases
- **决策**: 不拆分多文件（嵌入式测试场景，6 个用例无需框架过度设计），保留单文件可直接 `python phase1_test.py` 执行
- **能力**: CLI 用例选择 (`--cases`)、JSON + Markdown 双格式报告 (`--report-md`)
- **验证**: TC-ENG-101/102 真实硬件 PASS (2026-03-01 13:31)
- **来源**: 自动测试架构设计 v1.0

### [L-015] Cell Voltage 报告异常 (观察中)
- **发现**: TC-ENG-102 实测 Cell1=8.54V (=总电压), Cell2=0.01V (≈0)
- **根因**: NU17112 `usb_bridge.c` 写入 `i2c_buff[0x2D-0x30]` 时未正确分离单节电压
- **影响**: TC-ENG-102 Cell Sum 断言因 8.54+0.01≈8.56 偶然通过，物理含义错误
- **建议**: platform-buckboost-agent 确认 NU6805 Cell Voltage ADC 接口, platform-apl-agent 修正 usb_bridge.c
- **优先级**: P1 (不影响核心功能，但测试断言需增强)

## 待验证假设

- [H-001] port1 SNK 路径使用 PORT0 SETVOLT 事件可能是 bug（当前标记 R4 保留行为）
- [H-002] WB7720 exc_cache 在 0xEE 擦除后不清理 → PC 可能看到过期记录直到 WB7720 重启（Phase 0 代码审查已确认，需硬件验证）
- [H-003] I2C 通信瞬断是否触发 eng_mode_active 虚假退出（Phase 0 发现，需硬件验证）
- [H-004] 0xAA 刷新后 delta 保留是否被重入路径叠加计算（Phase 0 分析认为逻辑自洽，需硬件验证 TC-ENG-406）
