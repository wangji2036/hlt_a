# WPC Protocol 模块经验

## 已确认的模式 (Confirmed Patterns)

### [W-001] CEP 机制特性
- CEP (Control Error Packet, 0x03) 是单向的: PRx -> PTx
- CEP 值范围: -127 到 +127 (8-bit signed)
- 处理链: wpc_xfer.c 接收 -> _wpc.c 修改 -> pid_cep_handler() 应用
- 来源: Task #1 WPC CEP 探索

## 已知陷阱 (Known Pitfalls)

## 调试经验 (Debugging Lessons)

## 待验证假设 (Unverified Hypotheses)

---

## 设计意图与不变约束 (Design Rationale & Invariants)

> 这里记录的是「为什么」，不是「是什么」。「是什么」应当读代码。
> 参考完整文档: `docs/WPC_PROTOCOL_GUIDE.md`

### [D-001] MPP 设备必须经历 128kHz→360kHz 两次 Ping，不是 bug

**原则**: 第一次 Ping 必须用 128kHz，才能接收 XID 包（MPP 意图确认）；确认是 MPP 设备后再切换 360kHz 重新 Ping，完成 MPP 物理层初始化。

**原因**: MPP 规范要求传输层使用 360kHz 全桥，但设备识别阶段（XID 包）不限制频率。如果直接用 360kHz Ping，无法走通 128kHz 的普通 CNFG 流程，也无法确认 XID 扩展位。两次 Ping 是协议规定的正常路径，不是固件缺陷。

**推论**: 错误码 `IDCFG_PHASE_MPP_RESTRICTED_REP`（0x0A）是 MPP 设备初始化的必经路径，不应当作真正错误处理。若看到代码在收到 0x0A 后做过多的错误恢复逻辑（重试次数限制、长时间锁定），这是错误的。

### [D-002] EPP NEGO 阶段必须同时收到 qf 和 rf 两个 FOD 参考

**原则**: Qi ≥ 1.3 规范要求 EPP 协商中，PRx 必须发送 FOD/qf 和 FOD/rf 两个包。PTx 必须验证 `nego_fod_mask == 0x03`（两个 bit 都置位），否则必须发 NAK 并回退到 BPP。

**原因**: qf（Q 因子参考）和 rf（频率参考）共同建立 FOD 算法的基准线。缺少任何一个，XFER 阶段的功率损失检测无法正确校准，会导致 FOD 误报或漏报——两者都是安全问题。

**推论**: 若 EPP 设备协商成功后随即降回 5W，首先检查 `nego_fod_mask` 是否为 0x03。任何省略 FOD 参考验证步骤的代码改动都会破坏与 Qi ≥ 1.3 设备的互操作性。

### [D-003] CEP 处理必须在 PCH 延迟后进行，不能即时处理

**原则**: 收到 CEP 包后，不立即调用 pid_cep_handler()，而是启动 pch_t_delay 定时器，到期后才处理 CEP。

**原因**: PCH（Power Control Hold-off）延迟将"通信时间"和"功率传输时间"分离。CEP 包在 ASK 通信窗口内到达，此时线圈处于通信状态（调制中）。若立即调节功率，会与 ASK 解调互相干扰，导致后续包误码率上升。PCH 延迟保证功率调节发生在通信窗口结束后的纯功率传输阶段。

**推论**: 任何试图在收到 CEP 后立即触发 PID 的优化都会引入通信干扰。pch_t_delay 的值来自手机（PCH 包），不应由 PTx 单方面覆盖缩短。

### [D-004] VINDPM 修正在 CEP 处理链中优先级最高

**原则**: 当 VIN 电压异常（VINDPM WARNING 或 CRITICAL）时，VINDPM 修正在其他所有功率限制之前应用，且只能让 CEP 更负（降功率），不能逆向。

**原因**: 无线充电功率直接来自 VBUS。若 VIN 欠压，继续提升无线功率会进一步拉低 VIN，触发系统崩溃或保护锁定。VINDPM 修正是功率平衡的最后一道软件防线，必须优先于其他限制策略（FOD 限制、模式限制等）。

**推论**: 在功率限制逻辑中，VINDPM 修正必须是第一个应用的，后续的 power_limit_state 修正在其之后。如果 VINDPM 激活时无线充电功率无法有效降低，首先检查 CEP 修正顺序是否正确。

### [D-005] FOD 检测锁定 11 分钟是 WPC 规范要求，不能缩短

**原则**: PFOD 检测到功率损失后，PTx 进入 `WPC_IDLE_STAT_XER_FOD` 状态，必须等待约 11 分钟才允许下一次 Ping。

**原因**: WPC 规范规定异物检测后的最小冷却时间。若允许立即重试，在异物（金属片）未被移走的情况下反复 Ping，会持续给异物注入能量加热，造成安全风险。11 分钟冷却是规范通过的必要条件，缩短会导致 WPC 认证失败。

**推论**: 用户反馈"移走异物后要等很久才能充电"是正常设计行为。不能为了改善用户体验而缩短这个时间。

### [D-006] QDT 是低功耗感知机制，Ping 是高功耗发现机制，两者不可合并

**原则**: 每次 Ping 之前必须先通过 QDT 确认有物体存在，不能直接周期性发 Ping。

**原因**: 数字 Ping 需要驱动线圈发射电磁脉冲，功耗显著高于 QDT 的被动测量。若没有 QDT 过滤，移动电源在无设备时会持续发 Ping，缩短续航。QDT 使用线圈自身的 Q/F 参数变化感知物体，不额外消耗功率。

**推论**: 若 QDT 阶段误判频繁（大量不必要的 Ping），首先检查 Q 因子测量的稳定性（`is_stable()` 函数的方差阈值），而非禁用 QDT 直接 Ping。

### [D-007] IOC/IOP 测试 Workaround 代码是认证强制要求，不是临时补丁

**原则**: `wpc_6_test_1_ioc.c` 和 `wpc_6_test_2_iop.c` 中的特殊逻辑（每 5 次 Ping 切换 DDM 源、CEP±60 标志位等）必须保留。

**原因**: 这些代码是通过 WPC 官方互操作性测试（IOC Test Center）的必要条件，对应 WPC TPR#1C 和 IEC 测试用例。没有这些 Workaround，固件无法通过 Qi 认证，不能进入市场。

**推论**: 任何"清理代码"的重构操作，都不应删除或修改这两个文件中的逻辑。在 code review 中，看到有人试图删除 `pin_cnt % 5 == 0` 这样的"奇怪条件"，需要阻止。

### [D-008] MPP Cloak 模式是正常的低功耗充电延续，不是错误状态

**原则**: 手机发 CLOAK 包请求进入 Cloak 是正常协议行为，PTx 进入 CLOAK 阶段后应继续周期 Ping 确认手机在位，不能视为充电结束。

**原因**: MPP 设备在高 SOC 或低功率需求时进入 Cloak 节省通信开销，减少 FSK 占用的时间，让线圈有更多时间传输功率（效率优化）。Cloak 下充电仍在继续（但以检测 Ping 确认在位），而非停止。

**推论**: 若系统在收到 CLOAK 包后立即显示"充电结束"或停止线圈，这是实现错误。正确行为是进入 Phase 5 继续周期检测，直到手机真正移走或发 GET 包退出 Cloak。

## 调试直觉 (Debugging Heuristics)

### [H-001] MPP iPhone 总是 5W 或 15W，无法达到 25W

最常见原因有两种：① 适配器功率不足（`adp.wpc_gear < GEAR2`），系统在 CNFG 阶段降级；② MPP 受限模式切换未成功（0x0A 错误后没有正确切换到 360kHz）。

排查：看日志中 Phase 和 Mode 的打印，确认是否进入 MPP；检查 adp.wpc_gear 值；确认第二次 Ping 是否用了 360kHz。

### [H-002] EPP 协商成功但充电电流远低于 15W

最常见原因：`nego_fod_mask != 0x03`，NEGO 阶段缺少 FOD 参考包导致回退 BPP（5W）。

排查：检查 NEGO 阶段日志，确认 FOD/qf 和 FOD/rf 两包都被接收并 ACK；若只有一个，检查 T_NEGOTIATE 超时是否打断了另一个包的接收。

### [H-003] 充电启动后数秒内 CEP 超时停止

最常见原因：PCH 延迟设置异常（pch_t_delay 过大，超过 CEP 超时），或 PCH 包未正确处理导致 pch_t_delay 未初始化（默认 0，PID 立即被触发又立即超时）。

排查：打印 pch_t_delay 的值；确认 PCH(0x06) 包是否在 CNFG 阶段被正确处理；检查 CEP 超时值是否与功率模式（BPP/EPP/MPP）匹配。

### [H-004] 手机在位但移动电源不 Ping（IDLE 卡住）

最常见原因：① FOD 锁定（`WPC_IDLE_STAT_XER_FOD`，需等 11 分钟）；② EPT_RES 或 EPT_REP 状态下的等待延迟；③ PortManager 未调用 `tcpm_update_wpc_work_mode(BOOST)`（可能有线端口占用）。

排查：打印 ptx_idle_phase_status，确认 IDLE 子状态；检查 PortManager 是否允许 WPC 工作（有线充电优先时 WPC 可能被禁用）。
