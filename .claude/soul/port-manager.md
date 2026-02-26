# Port Manager 模块经验

## 已确认的模式 (Confirmed Patterns)

### [P-001] port0 和 port1 的硬件时序差异
- port0 closed: 先 tcpm_disable_usba_detect() 后 hal_tcpc_set_gate_en()
- port1 closed: 先 hal_tcpc_set_gate_en() 后 tcpm_disable_usba_detect()
- 原因: 硬件布线导致的上电时序要求，不可调换
- 来源: R3 重构时发现，R4 重构时再次确认

### [P-002] connect_start 提取公共函数的安全边界
- port0/port1 可以提取公共函数（结构相似度 >90%）
- port2/port3 不可以（与 port0/1 结构差异太大）
- port2: 禁用两个 TypeC，有 500ms 定时器
- port3: 不停 WPC，仅在端口空闲时清除 support flag，关闭所有有线端口
- 来源: R4 重构分析

### [P-003] scan_handle 两阶段逻辑差异
- IDLE 阶段: 4 端口结构完全相同，适合表驱动
- INHANDLING 阶段: 仅 PORT0/1，且 UNCONNECT 有 pdlib_disable_typec() 调用，不适合与 IDLE 合并
- 来源: R4 重构分析

### [P-004] typec_closed 公共函数的提取模式
- charger_mode 和 discharge_mode 可以按 (this_port, peer_port) 参数化
- wpc_tail 可以按 (peer_port) 参数化
- 但 port0/port1 closed 的阶段1（停止 WPC/关闭 gate）顺序不同，不可合并
- 来源: R3 重构

### [P-005] connect_success SNK 处理可合并
- typec_success_snk_handler(this_port, peer_port, setvolt_event) 统一处理
- 充电模式和放电模式的 SNK 逻辑完全相同
- 来源: R3 重构

## 已知陷阱 (Known Pitfalls)

### [T-001] port1 充电模式 USB-A gate 缺少 #if 保护
- 现象: port1 connect_start 中 hal_tcpc_set_gate_en(PORT2_INDEX,false) 没有 CONFIG_USBA_SUPPORT 宏保护
- 对比: port0 有 #if 保护
- 根因: 原始代码遗漏，非有意设计
- 修复: R4 统一加上 #if 保护（行为变更: 不支持 USB-A 时不再关闭 PORT2 gate）
- 教训: 对比 port0/port1 时注意宏保护的一致性

### [T-002] port1 SNK 充电模式使用 PORT0 SETVOLT 事件
- 现象: port_enum_port1_connect_success() 充电模式 SNK 路径传入 PORT_ENUM_EVT_PORT0_SINK_SETVOLT
- 可能是 bug，但未修改以避免引入回归
- 标记: R4 保留行为

### [T-003] g_port.snk_set_volt 初始化不对称
- port0 connect_start 入口设置 snk_set_volt = VOLTAGE_5V
- port1 connect_start 不设置
- 在提取公共函数时必须保留此差异

### [T-004] WPC 功耗扣除参数需集中管理
- 5V 档: 扣除 8W, 9V 档: 扣除 11W, 12V 档: 扣除 12W
- R4 提取为 calc_wpc_coexist_current() 集中管理
- 修改功耗参数时只需改一处

## 重构经验 (Refactoring Lessons)

### 可以重构的
- scan_handle IDLE 阶段: 表驱动（4 端口结构完全相同）
- connect_start port0/port1: 提取公共函数
- connect_closed charger/discharge mode: 按 (this_port, peer_port) 参数化
- connect_success SNK handler: 统一处理
- WPC 共存电流计算: 提取查表函数

### 不该动的
- scan_handle INHANDLING 阶段: 仅 PORT0/1，逻辑与 IDLE 不同
- event_handle switch/case: OSAL 标准模式，清晰直观
- snk_setvolt: 已有 BUG#1-8 修复，结构清晰
- enum_done: 各阶段逻辑差异大（PORT0 有 ramp 限流），参数化收益低
- connect_start port2/port3: 与 port0/1 结构差异太大
- snk_setcharge 无 WPC 分支: NU6801 ibus 限制在 #if 中，表驱动反而更复杂

## 代码结构演化记录

| 轮次 | 内容 | Commit |
|------|------|--------|
| R1 | 提取公共函数 (any_port_sourcing 等)，清理乱码注释 | 9460644 |
| R2 | 参数化 TypeC closed 处理 (charger/discharge/wpc_tail) | 4f69ba6 |
| R3 | 提取 connect_success SNK 公共逻辑 | 7fa1775 |
| R4 | 表驱动 scan_handle, connect_start 公共函数, WPC 电流计算 | 74eab19 |

---

## 设计意图与不变约束 (Design Rationale & Invariants)

> 这里记录的是「为什么」，不是「是什么」。「是什么」应当读代码。
> 参考完整文档: `docs/PORT_MANAGER_GUIDE.md`

### [D-001] 断开永远优先于连接

**原则**: 事件仲裁中，UNCONNECT 优先级高于 TRY_CONNECT。

**原因**: 用户拔出设备必须立即停止 VBUS 输出，不能因为其他端口正在连接而延迟。延迟断开会导致 VBUS 持续向已拔出的悬空线缆输出，存在安全风险。

**推论**: 如果看到 UNCONNECT 与 TRY_CONNECT 并列处理或优先级被调换，这是安全性错误。

### [D-002] INHANDLING 锁保护的是不可中断的硬件操作序列

**原则**: `state == PORT_INHANDLING` 期间，新的 TRY_CONNECT 被挂起不处理。

**原因**: 一次端口连接涉及多个有顺序依赖的硬件操作（关 gate → 切换充放电模式 → 配置 BuckBoost → 开 gate），若被另一个端口的连接操作插入，硬件会处于不一致状态（例如 gate 已开但 BuckBoost 尚未切换模式，会触发过流保护）。

**推论**: INHANDLING 期间事件挂起是正确行为。但若 INHANDLING 持续超过几秒，通常意味着定时器未触发或 OSAL 事件丢失，需要排查。

### [D-003] 双层事件系统：位图负责汇聚，OSAL 负责执行

**原则**: 外部信号写入 `port_event` 位图（OR 操作，非阻塞）；scan_handle 仲裁后转换为 OSAL 事件（顺序执行）。

**原因**: 外部模块（BuckBoost ISR、NTC 定时器、TypeC 库回调）可能在任意时刻调用 `port_manager_set_event()`，不能要求它们等待 PortManager 空闲。位图 OR 是原子无锁的。实际处理需要串行，用 OSAL 事件队列保证。

**推论**: `port_manager_set_event()` 永远不能阻塞调用方。若在此加锁或等待，违背设计意图。

### [D-004] TypeC 端口检查空闲，USB-A/WPC 不检查

**原则**: TRY_CONNECT 分发时，TypeC 端口需确认 `port_state == NONE`；USB-A 和 WPC 无条件接受。

**原因**: TypeC 有硬件状态机（SRC/SNK_Attached），重复接受同一已连接的 TypeC 端口会导致重复初始化。USB-A/WPC 没有硬件状态机，检测信号本身就代表新连接，不存在重复问题。

### [D-005] SNK 流水线的 2 秒延迟是 PD 协议时序要求

**原则**: TypeC SNK_Attached 到 SINK_SETVOLT 之间约有 2 秒延迟。

**原因**: PD 协议在 CC 握手后需要时间完成 Source_Capabilities 消息交换，Sink 收到 PDO 列表后才能发 Request。过早发起请求会导致 PD 协商失败并回退到 5V。2 秒是保守安全值，确保 PDO 列表已就绪。

**推论**: 若 PD 充电器总是只协商到 5V，首先检查这 2 秒延迟是否被缩短，以及 `pdlib_is_connect()` 在此时刻是否返回 true。

### [D-006] WPC connect_start 关闭所有有线 gate 是硬件防护要求

**原则**: WPC 新连接时，connect_start 先关闭所有正在输出的有线端口 gate，500ms 后 connect_success 再重开。

**原因**: WPC 线圈功率调制初始化阶段有较大电流浪涌。若此时有线端口 BuckBoost 全功率输出，总电流可能触发过流保护导致系统复位。先降低有线输出，等 WPC 稳定后再恢复。

**推论**: 不能把 WPC connect_start 改成"不关有线 gate"以提升切换速度，这会带来硬件风险。

### [D-007] RESET_CHARGE 优先级最低，因为它是软性重协商

**原则**: `PORT_EVENT_RESET_CHARGE` 排在所有连接/断开事件之后处理。

**原因**: RESET_CHARGE 由温度变化、死电池恢复等"软性"条件触发，不改变物理连接状态，延迟几十毫秒无影响。而连接/断开是物理事件，必须先处理。

### [D-008] ibus_limit 与 PD Request 电流必须分离管理

**原则**: 充电 IC 的实际限流（ibus_limit）和发送给适配器的 PD Request 电流是两个不同的值，后者最多是前者的 1.2 倍。

**原因**: USB PD 规范允许 Sink 向 Source 请求比实际使用量多的电流（最多 1.2 倍），让 Source 工作在更佳效率点。充电 IC 的 ibus_limit 是真正流进系统的上限，两者必须分开，否则会导致充电 IC 过流。

**推论**: 修改充电参数时，检查是在改 ibus_limit（充电 IC 限流，正确目标）还是 request_current（PD 协商请求，不能超出 IC 能力）。

### [D-009] 孤立端口回收是主动的状态整洁机制，不是 bug

**原则**: 某端口断开后，若剩余输出端口完全孤立（其他端口均空闲），将孤立端口关闭并重新枚举。

**原因**: 让孤立端口重走完整连接流水线（CC 通告 → PD 协商），使系统彻底回到 IDLE 态，WPC 才能重进高效的 BOOST 模式。这是以短暂断电换取系统状态整洁。

**推论**: 用户反映"拔掉一个设备后另一个设备短暂断电重连"，这是正常设计行为。

### [D-010] adpater_power 是协议估算值，不是实时采样

**原则**: `adpater_power = ibus_limit × snk_set_volt / 1000`，是用协议协商值推算，不是实时功率测量。

**原因**: USB PD 协议没有实时功率查询接口，只有 PDO 声明的能力上限。此估算值用于 WPC 共存时的功率预算分配，精度有限。

**推论**: WPC 共存的功率扣除是基于固定经验值（按电压档位），不会动态跟踪适配器实际输出，这是系统约束。

## 调试直觉 (Debugging Heuristics)

### [H-001] 充电电流异常低（停在 500mA）

最常见原因：RESET_CHARGE 触发时 `any_port_sourcing()` 返回 true，走了"有端口在输出"的保守分支，将充电限制到 5V/500mA。

排查：检查触发 RESET_CHARGE 时各端口的 port_state，以及 `g_buckboost.woke_mode` 是否确实是充电模式。

### [H-002] 新设备插入后系统长时间无响应

最常见原因：`g_port.state` 卡在 PORT_INHANDLING，前一个流水线中某个定时器未触发或 OSAL 事件丢失。

排查：查看最后一条 printk 日志，判断卡在哪个阶段（CONNECT_START / CONNECT_SUCCESS / SETVOLT / SETCHARGE）。

### [H-003] PD 充电器总是只协商到 5V

排查顺序：① `pdlib_is_connect()` 在 SETVOLT 时是否返回 true → ② SETVOLT 的延迟是否被缩短 → ③ `sink_pdo[]` 中是否定义了对应电压 → ④ Source PDO 列表中是否有电压匹配项。

### [H-004] WPC 始终 FIX5V 而非 BOOST 模式

原因：有线端口还有 SOURCE 状态，`all_wired_ports_idle()` 返回 false。

排查：检查 `g_port.port_state[0/1/2]` 是否都是 NONE。
