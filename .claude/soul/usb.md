# USB 模块经验

## 已确认的模式 (Confirmed Patterns)

### [P-001] PE Sink 侧始终先选 PDO1（5V），应用层主动升压
- PE_SNK_Evaluate_Capability 中初始 RDO 永远是 PDO1，PE 不做智能电压选择
- 更高电压由 PortManager/WPC 通过 pdlib_snk_requsrt_voltage() 主动请求
- 来源: lib/usb_pd.c PE_SNK_Evaluate_Capability 完整阅读 + USB_GUIDE.md 撰写

### [P-002] TCPM_EVT_PD_READY 有 500ms 延迟，在 PE_Ready_Entry 中启动定时器
- PE 进入 SNK/SRC Ready 状态后不立刻通知 WPC，而是启动 500ms 定时器
- WPC 工作模式联动必须等定时器触发，确保 VBUS 已稳定
- 来源: lib/usb_pd.c PE_SNK_Ready_Entry + fml/tcpm.c 阅读确认

### [P-003] DRP Toggle 使用 35ms(Rd) + 45ms(Rp) 非对称窗口，双端口计数器独立
- Rd 阶段监听对面 Source，Rp 阶段广播自己是 Source
- 两端口的 Toggle 计数器使用 a_/b_ 前缀分开，互不干扰
- 来源: lib/typec.c 完整阅读

## 已知陷阱 (Known Pitfalls)

## 调试经验 (Debugging Lessons)

## 待验证假设 (Unverified Hypotheses)

---

## 设计意图与不变约束 (Design Rationale & Invariants)

> 这里记录的是「为什么」，不是「是什么」。「是什么」应当读代码。
> 参考完整文档: `docs/USB_GUIDE.md`

### [D-001] PE Sink 侧永远先选 PDO1（5V），关注分离的核心设计

**原则**: PE_SNK_Evaluate_Capability 初始 RDO 始终选 PDO1（5V），不管对面 Source 提供了更高电压的 PDO。更高电压的请求由应用层（PortManager 或 WPC）在 PD 合同建立后主动发起。

**原因**: 关注分离——PE 层只负责确保 PD 合同能建立（5V 是所有 Source 必须支持的基础），功率策略由业务层决定。如果 PE 自行选择最高 PDO，但 VBUS 切换期间其他模块还未准备好，会导致调压过程中的功率震荡或保护触发。先以 5V 建立稳定合同，再按需升压，是最稳健的方案。

**推论**: 插入 20V/5A 适配器后系统以 5V 充电是正常行为，不是 bug。若 WPC 功率始终只有 5W，或充电一直在 5V 而不升压，首先检查 PortManager/WPC 是否在 PD Ready 后调用了 pdlib_snk_requsrt_voltage()，其次检查 TCPM_EVT_PD_READY 是否触发。

### [D-002] PPS 模式下 Sink 必须每 1500ms 刷新请求，规范强制约束

**原则**: 进入 PPS 合同后，Sink 必须在 SinkPPSPeriodicTimer（1500ms）超时前重新发送 Request。Source 的 SourcePPSCommTimer（3000ms）超时后，Source 会发 Hard Reset 强制断开。

**原因**: PD 规范设计——防止 Sink 死机后 Source 持续在动态高压上等待（高压输出比 5V 有更高的功率损耗和热量）。1500ms 刷新（规范允许最长 10s 窗口，实现使用 1.5s 提供更快响应）比 Source 的 3000ms 超时有足够安全余量。

**推论**: PE_SNK_Ready 阶段绝对不能有超过 1000ms 的阻塞操作（否则 1500ms 刷新无法按时触发）。如果 PPS 充电器反复断开重连，首先检查 PPS 刷新是否按时（打印进入 PE_SNK_Select_Capability 的时间戳），再检查 WPC 触发 TCPM_EVT_QI_SET_VOLT 的频率是否过高（每次触发都相当于一次 Re-Request）。

### [D-003] PD Ready 500ms 延迟是 VBUS 稳定缓冲，不是任意等待

**原则**: PE 进入 Ready 状态后，必须等 500ms 才通知 WPC 更新工作模式（`TCPM_EVT_PD_READY` 由 500ms 定时器触发）。WPC 工作模式更新的唯一合法触发点是这个定时器事件，不能提前触发。

**原因**: PD 协商 Accept+PS_RDY 完成时，VBUS 电压仍可能在从 5V 向目标高压（如 9V/15V）切换（tPSTransition 最长 500ms）。若在 VBUS 切换期间就告知 WPC PID 可用的最高电压，WPC 会立刻开始拉电流，可能在 VBUS 未稳定时引起电压震荡或过冲保护。500ms 延迟确保 VBUS 完全稳定后再联动。

**推论**: WPC 功率异常的排查顺序：① 确认 TCPM_EVT_PD_READY 是否触发（加日志）→ ② 确认 tcpm_update_wpc_work_mode() 传入正确的 mode → ③ 确认 pid_set_volt_limit() 已更新。若在 TypeC_CONNECT_SUCCESS 事件后就更新 WPC 工作模式，是架构错误。

### [D-004] DRP Toggle 非对称（35ms Rd / 45ms Rp）加快双 DRP 互连仲裁

**原则**: DRP Toggle 的 Rd 等待窗口（35ms）和 Rp 广播窗口（45ms）刻意不对称。不能修改为相等值。

**原因**: 两台 DRP 设备互连时，若 Toggle 周期完全相同，两台可能长时间保持同步（总是处于相同阶段），导致角色无法快速确定。非对称周期使相位自然错开，从而更快完成角色仲裁。规范要求 tDRP = 50-100ms，35+45=80ms 满足约束。

**推论**: 若双 DRP 互连后角色仲裁极慢（超过 5 秒未稳定），检查 Toggle 定时是否被意外修改为对称值，或 Try.SRC/SNK 的计数器是否配置过小（导致过早放弃）。

### [D-005] Try.SRC/SNK 计数器限制保证 DRP 仲裁在有限次内必然收敛

**原则**: `try_src_cnt < 3` 和 `try_snk_cnt < 5` 是硬性边界，必须保持。达到上限后，当前 DRP 必须接受对方角色，不再竞争。

**原因**: 没有计数器限制，两台 DRP 设备会无限交替 Try.SRC → 失败 → Try.SRC，永远无法稳定连接（协议上的活锁）。计数器保证至少有一台会在有限次后"认输"，接受 Sink 角色，让另一台稳定作为 Source。

**推论**: 减小计数器阈值会降低仲裁可靠性（某些较慢的设备可能需要更多次 Try 才能响应）。增大阈值会延长仲裁时间。3/5 是 TypeC 规范建议的经验值，不要随意修改。若两台移动电源互连后始终卡住，检查计数器是否正确递增和判断。

### [D-006] 死电池设备跳过 Try.SRC，避免反向竞争 Source 角色

**原则**: `pdlib_get_deadbat()` 返回 true（电池电压 < 死电池阈值，通常 2.8V）时，TypeC 连接流程跳过 Try.SRC 检查，直接接受 Sink 角色。

**原因**: 死电池移动电源若执行 Try.SRC，会与满电移动电源争抢 Source 角色，若 Try.SRC 意外成功，会导致死电池移动电源向满电移动电源反向输出（死电池先放电给满电，违背期望行为），且两台均无法正常充电（死电池不能充自己）。

**推论**: 修改 Try.SRC 触发条件时，必须保留死电池检查。若两台移动电源互连后死电池设备变成了 Source，检查死电池判断逻辑是否生效（电压阈值和 pdlib_get_deadbat() 实现）。

### [D-007] PD 定时器在 TMR1 ISR 更新，状态机在 OSAL 主循环推进，有意分离

**原则**: `usb_pdlib_timer_update()` 在 TMR1 ISR（1ms 周期）中调用，仅递减计数器和设置超时标志，不做任何状态机处理。`pdlib_run()` 在 OSAL 主循环 TCPM 事件处理中调用，推进状态机。两者不能合并。

**原因**: OSAL 协作式调度可能存在最高 850ms 的任务延迟（软 WDT 上限），但 PD 定时器精度要求毫秒级。将定时器递减放在 ISR 中，保证其不受任务调度延迟影响，定时器总是按时"超时"。状态机在主线程中运行，避免了 ISR 中处理状态机带来的并发安全问题（临界区保护复杂度高）。

**推论**: 如果某个 Task 占用 CPU 超过 10ms，PD 定时器仍然正确计时（因为 ISR 不受影响），但状态机响应会相应延迟。调试 PD 协议时序问题时，分清"定时器超时时间"（ISR 层面，精确）和"状态机响应时间"（OSAL 层面，受调度影响）两个概念。

### [D-008] Source Hard Reset 后 480ms+660ms 是 VBUS 安全放电的规范要求

**原则**: Source 收到 Hard Reset 后必须等 480ms（tPSHardResetTime）再关闭 VBUS，关闭后等 660ms（SourceHardResetRecoverTimer）才重新上电。不能缩短这两个时间。

**原因**: Sink 端（通常是手机）有大容量 VBUS 滤波电容（可达数百 µF）。若 Source 立刻重新上电，Sink 电容中的残压与 Source 新上电电压叠加，会导致 Sink 误判 VBUS 已就绪（跳过 VSafe0V 检测），PE 立刻启动 Wait_for_Capabilities，但 Source 可能还未准备好发 Source_Cap。660ms 确保 VBUS 通过 dummy load 放电至 VSafe0V（< 0.8V）。

**推论**: TypeC Source 侧 Hard Reset 后 Sink 端屏幕短暂黑屏（约 1 秒）是正常行为，不是 bug。若 Hard Reset 恢复后协商仍然失败，检查 dummy load 是否有效工作（`hal_tcpc_port_dummyload_en()` 实际效果），或适当延长 660ms 时间（大电容场景）。

## 调试直觉 (Debugging Heuristics)

### [H-001] TypeC 连接后 PortManager 始终未收到 CONNECT_SUCCESS 事件

最常见原因: ① CC 去抖（100ms tCCDebounce）因接触不良或噪声持续失败，TypeC SM 一直在 AttachWait；② Try.SRC/SNK 计数器达到上限后未能进入 Attached 状态（罕见，极端信号条件）；③ `pdlib_disable_typec()` 被意外调用，端口进入 TC_Disable 状态。

排查: 打印 `pdlib_get_tc_state(PORT0_INDEX)` 确认当前 TypeC 状态；读取 CC 状态确认 cc1/cc2 电平合理；检查 `try_src_cnt` 和 `try_snk_cnt` 是否已到达上限；搜索代码中 `pdlib_disable_typec()` 的所有调用点确认未误触发。

### [H-002] PD 协商反复 Hard Reset，合同始终无法建立

最常见原因: ① `PE_SNK_Wait_for_Capabilities` 365ms 超时（低质量或慢速适配器发 Source_Cap 较慢）；② `PE_SRC_Transition_Supply` 等待 VBUS 就绪超时（BuckBoost 调压响应慢于预期）；③ 对面 Source 发送了格式异常的 PDO，PE 解析失败后放弃。

排查: 打印当前 PE 状态和 `hardreset_counter` 值（接近 2 说明 3 次协商均失败）；若是 Source 端，用逻辑分析仪捕获 CC 线 BMC 波形确认消息格式；尝试将 `SinkWaitCapTimer` 从 365ms 增加到 500ms 验证是否为超时问题。

### [H-003] PPS 模式下充电器反复断开重连，之后从 5V 重新开始

最常见原因: ① PPS 1500ms 刷新定时器触发时 PE 未能成功重新 Request（OSAL 调度延迟超过 1500ms，或 TCPC 发送失败）；② WPC 触发 `TCPM_EVT_QI_SET_VOLT` 过于频繁，Source 来不及处理每次新 Request。

排查: 确认 `pdlib_is_pps_sink()` 返回 1（真正进入了 PPS 合同）；打印每次进入 `PE_SNK_Select_Capability` 的 `sys_ticks`，确认刷新间隔是否稳定在 1500ms 以内；检查 WPC 侧 `TCPM_EVT_QI_SET_VOLT` 的触发频率，若 < 500ms 一次，增加 WPC 侧防抖。

### [H-004] WPC 无线充电功率无法超过 5W（高压 PD 适配器已连接）

最常见原因: ① `TCPM_EVT_PD_READY` 未触发（PD 协商失败，或 500ms 定时器未启动）；② `tcpm_update_wpc_work_mode()` 判断条件未匹配，未进入 BOOST 或 PD_PPS 模式；③ `pid_set_volt_limit()` 传入的 `volt_max` 仍为 5000mV（未被更新）。

排查: 确认 PD 协商成功（`pdlib_is_connect()` 应返回 1）；打印 `gd->adp.type` 和 `gd->adp.volt_max`（确认适配器信息已更新）；在 `tcpm_update_wpc_work_mode()` 入口加日志打印传入的 mode 参数；检查 WPC PID 的 `pid_volt_lim_hi` 是否已超过 5000mV。
