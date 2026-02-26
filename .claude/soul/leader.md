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
- C_INCLUDE_PATH 必须用 Windows 风格路径（分号分隔，反斜杠）
- makefile 首次需要 patch（修复中文乱码路径），patch 后 makefile.orig 存在则跳过
- 增量构建只编译修改的文件，全量 rebuild 需 make clean 先行

## 知识回流记录

- [回流-001] USB-A gate #if 保护遗漏 -> 标记 [POTENTIAL_COMMON]，待其他项目验证
- [回流-002] port0/port1 connect_start 可安全提取公共函数 -> 标记 [POTENTIAL_COMMON]

## 待验证假设

- [H-001] port1 SNK 路径使用 PORT0 SETVOLT 事件可能是 bug（当前标记 R4 保留行为）
