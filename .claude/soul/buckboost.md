# BuckBoost 模块经验

## 已确认的模式 (Confirmed Patterns)

### [B-001] VIN 采样来源
- gd->vpwr = g_buckboost.adc_vbus (NU6805 buck-boost ADC, ~100ms 周期)
- gd->vbus 是硬编码 9000mV，不是真实采样
- 来源: VINDPM 实现时确认

## 已知陷阱 (Known Pitfalls)

## 调试经验 (Debugging Lessons)

## 待验证假设 (Unverified Hypotheses)

---

## 设计意图与不变约束 (Design Rationale & Invariants)

> 这里记录的是「为什么」，不是「是什么」。「是什么」应当读代码。
> 参考完整文档: `docs/BUCKBOOST_GUIDE.md`

### [D-001] 充电软启动必须从低电流开始，不能跳过

**原则**: 每次切换到充电模式时，IBUS 从 300mA 起步，每 500ms 步进 100mA 至目标值。

**原因**: 充电器与 BuckBoost 的协商刚完成时，两端的电容尚未稳定。若立即以满电流启动，瞬态冲击电流会触发充电器侧的过流保护，导致 VBUS 塌陷或充电器重置。慢速爬升给两端留出足够的 RC 建立时间。

**推论**: 任何绕过软启动（直接写 IC 寄存器到满电流）的优化都会带来充电器兼容性问题。若充电器在接入后数秒内反复断开重连，首先检查是否有代码跳过了软启动流程。

### [D-002] VBUS 电压切换前必须先放电（dummy load）

**原则**: `buckboost_set_bus_iv()` 改变目标电压时，必须先激活 dummy load 对 VBUS 放电约 300ms，才能设置新电压。

**原因**: VBUS 侧有大容量滤波电容。若直接切换目标电压（如从 9V 降到 5V），BuckBoost IC 会看到输出端电压高于目标值，进入不确定状态，可能触发 OVP 保护或输出电压振荡。Dummy load 强制快速释放电容存储的能量，使 VBUS 降到安全范围后再建立新目标。

**推论**: 修改电压切换流程时，不能省略 dummy load 步骤。若看到电压切换后系统保护锁定，先检查 dummy load 时序是否完整。

### [D-003] 保护锁定对 VBUS 类故障和 NTC 故障处理策略不同

**原则**: VBUS/过流故障仅锁定当前**空闲**的 TypeC 端口；NTC 过温锁定**强制所有端口 state 为 NONE** 再执行锁定。

**原因**: VBUS 类故障的成因可能是刚插入的新端口短路，锁定空闲端口可以防止扩大，但不应强制断开已在充电的适配器（断开适配器会让用户无法继续给空电池充电）。NTC 过温是电池物理安全问题，所有功率输出必须立即停止，不论当前连接状态。

**推论**: 如果看到保护逻辑同时对 VBUS 故障和 NTC 用相同的策略，这是错误的。NTC 故障必须优先于连接状态，强制清零所有端口状态。

### [D-004] ADC 8 步轮转是负载均摊的设计，不是简单延迟

**原则**: BUCKBOOST_TASK 每 17ms 只处理 8 步中的一步，完整一轮约 136ms。

**原因**: BuckBoost IC 通过 I2C 通信，每次读写占用总线时间。如果每 17ms 读取全部 ADC 通道（VBAT/VBUS/IBUS/IBAT/NTC1/NTC2/IAC1/IAC2），I2C 总线会在这 17ms 内持续占用，阻塞其他 I2C 设备（如 NTC、TCPC）。轮转分散了 I2C 负载，保证总线响应性。

**推论**: 新增 ADC 采样需求时，不能简单在某一步中追加读取，而应插入新步骤或使用空闲步骤，保持轮转均衡。读取 `g_buckboost.adc_*` 的代码必须容忍最多 136ms（NU6805）或 272ms（NU6801）的数据延迟。

### [D-005] NU6801 的 ibat 来自效率模型而非直接测量，精度有限

**原则**: NU6801 上 `adc_ibat` 由 `ibus_to_ibat(ibus, vbus, vbat)` 效率模型计算，不是 IC 直接测量。

**原因**: NU6801 只有 VBUS 侧的电流传感器，没有独立的电池电流传感器。系统需要 IBAT 主要用于 SOC 估算和日志。效率模型用分段线性拟合，在正常负载区间误差约 5%，可接受。

**推论**: 在轻载（< 200mA）或过渡态（刚切换模式）时，NU6801 的 `adc_ibat` 精度显著下降。BMS/SOC 算法应优先使用专用 Gauge IC 的数据，不应直接依赖 BuckBoost 的推算 ibat。NU6805 设计上有独立 IBAT 测量，此限制不适用。

### [D-006] 保护解锁要求端口处于完全空闲状态

**原则**: `buckboost_protection_flag` 从 1 清除为 0 需要 `g_port.port_state[PORT0]` 和 `[PORT1]` 均为 `PORT_STATE_NONE`。

**原因**: 解锁时会调用 `pdlib_restart_typec()` 重启 TypeC CC 状态机。如果端口仍处于 SINK 或 SOURCE 状态，重启会打乱现有的 PD 会话，导致已连接设备被强制断开重连。只有确认端口完全空闲，重启才是安全的。

**推论**: 若系统保护后长时间无法解锁（protection_flag 卡在 1），首先检查 PortManager 是否已将端口状态清零（NTC 类保护触发时会强制清零；VBUS 类保护不会强制清零已连接端口）。

### [D-007] IR drop 补偿仅在放电且非 PPS/WPC 时激活

**原则**: IR drop 补偿在充电模式、PPS 模式、WPC 模式下均不激活。

**原因**: 充电模式下 VBUS 由适配器提供，BuckBoost 是降压端，不控制适配器输出电压，IR drop 补偿无意义。PPS 和 WPC 各自有闭环功率控制（CEP 反馈、PDO 请求），它们会自行协商合适的电压，BuckBoost 再额外补偿反而会干扰闭环控制。

**推论**: 若在 PPS 充电时看到 ir_drop 不为零，这是设计错误（当前代码已正确排除）。禁止在 `wpc_mode == TCPM_WPC_WORK_BOOST` 或 `pdlib_is_pps_source()` 时激活 IR drop 补偿。

## 调试直觉 (Debugging Heuristics)

### [H-001] 充电电流比预期低，设备充电缓慢

最常见原因有三：① 软启动仍在进行（刚接入充电器后的前 8-10 秒）；② NTC 软限制触发，限制了最大允许功率；③ 保护锁定（`buckboost_protection_flag == 1`）导致充电 IC 处于安全状态。

排查：先看 log 中 "Charging = [%d %d]" 确认目标电流；再看 "R_ntc=%d" 确认 NTC 无异常；若无 log 则检查 `buckboost_protection_flag`。

### [H-002] 放电输出电压偏低或波动

最常见原因：IR drop 补偿过大（电流过大或传感器异常），或 VBUS dummy load 放电后电压未稳定就切换（wait/delay 参数过短）。

排查：看 log "ir drop = %d" 确认补偿量是否合理（>300mV 说明电流测量异常）；看 "OUT = %d" 确认目标电压（记住 +150mV 补偿）；检查 `regulator_state` 是否为 1 再开 gate。

### [H-003] 系统保护频繁触发锁定

最常见原因：VBUS 软件保护（VBUS_SOFT_PROTECT），即 20ms 周期检测 VBUS 持续偏离目标 ±20% 超过 1 秒。这通常是输出端接了超出 BuckBoost 能力的负载，或者 ADC 采样错误（NU6801 的 VREF 异常）。

排查：看 "protect lock =0x%x" 的十六进制值，对照保护位图定位具体原因；检查 VBUS 实际值（`adc_vbus`）与目标值的差；若 `ADC_ERR` bit 置位，检查 NU6801 VREF 采样是否正常。

### [H-004] 插入充电器后死电池设备识别失败

原因：保护锁定时调用了 `pdlib_disable_typec()`，但解锁时 PortManager 的 `port_state` 没有归零（VBUS 类保护不强制清零已连接端口 state），导致解锁条件 `port_state == NONE` 不满足，`pdlib_restart_typec()` 永远不被调用。

排查：确认 `g_port.port_state[0]` 和 `[1]` 的值；若 NTC 类保护触发（会强制清零），检查 NTC 是否已经恢复正常范围（否则 `ntc_lock_flag` 仍会持续触发锁定）。
