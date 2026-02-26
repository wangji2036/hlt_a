# DPDM 模块经验

> 参考完整文档: `docs/DPDM_GUIDE.md`

## 已确认的模式 (Confirmed Patterns)

### [P-001] AFC/SCP 必须在 ISR 中同步处理
- **原则**: AFC/SCP 数据包解析和 TX 写入必须在中断上下文中直接完成，不能通过 OSAL 事件队列延迟处理
- **原因**: SCP MAX_RSPTIME = 20ms，OSAL 任务调度最坏情况可能超过此限制
- **推论（Implication）**: 如果把 `dpdm_src_afc_handle()` / `dpdm_src_scp_handle()` 移出 ISR，SCP/AFC 握手会超时失败。任何在 ISR 里增加延时的修改都是危险的。

### [P-002] QC3 两步输出：先宽限、后精算
- **原则**: QC3 脉冲到达时，先用宽限电流立即输出，启动 250ms 计时，超时后再用 18W 公式精确限流
- **原因**: QC3 脉冲可能连续快速到达，频繁精确计算+BuckBoost 调用会导致 VBUS 抖动
- **推论**: 如果去掉 250ms 定时器、每个脉冲都精确限流，会导致 QC3 升压过程中频繁触发限流保护，手机协商体验变差

### [P-003] Sink QC 探测：从高到低，测完恢复 5V
- **原则**: Sink 侧先探测 12V，再探测 9V，探测完毕无论结果如何都恢复到 5V
- **原因**: 探测阶段只是"查询能力"，不是"实际充电"；实际充电电压由 Port Manager 根据 bc12_type 另行决定
- **推论**: 不能在探测过程中保持高压状态，否则 TCPM 收到 DPDM_DONE 后的 PD 协商会和高压 QC 产生冲突

### [P-004] 协议优先级：PD > SCP/FCP/QC/AFC
- **原则**: PD 协议（CC 线）和 D+/D- 快充协议不能同时工作于同一端口
- **实现**: QC 事件处理前检查 `pdlib_is_connect()`；SCP/FCP 写电压时主动调用 `pdlib_disable_usbpd()`
- **推论**: 若发现某端口 PD 协商成功后又出现快充异常，先查日志中是否有 "pd has work" 或 SCP 写电压操作触发了 pdlib_disable_usbpd

### [P-005] DPDM MUX 唯一性：切换前必须 Deinit
- **原则**: 调用 `usb_dpdm_select()` 切换 MUX 前，内部自动调用 `dpdm_sink_deinit()` 清除 Sink 检测状态
- **原因**: 防止旧端口的中断在 MUX 切换后仍触发，被误判为新端口的事件
- **推论**: 不要直接写 MUX_PORT_NUM 寄存器，必须通过 `usb_dpdm_select()` 接口，否则残留 Sink 状态会引起误报

## 已知陷阱 (Known Pitfalls)

### [T-001] qc_volt 并发撕裂风险
- **症状**: QC3 连续模式下偶现奇怪电压值（不是 200mV 的倍数）
- **原因**: uint16_t qc_volt 在 ISR（QC_SRC_IRQHandler）中修改，在 Task 中读取，非原子操作
- **规避**: 读取 qc_volt 前使用 osal_disable_irq()（当前代码未做，是已知风险）
- **实际风险**: QC3 脉冲频率 <10Hz，撕裂概率极低

### [T-002] SCP 多字节读写起始地址未检查
- **症状**: 手机发送异常 SCP 多字节读命令时，可能读取 SCP_REG[256] 越界内存
- **原因**: `fcp_multi_read_handle` 限制了读取长度（≤10字节）但未检查起始地址 msg_1
- **规避**: msg_1 + copy_len 应限制在 256 以内

### [T-003] Sink QC 探测的 12V 短暂冲击
- **症状**: 某些劣质充电器在 Sink QC 探测时报 OVP 并断电
- **原因**: 探测总是先请求 12V，即使充电器不支持，充电器可能因此触发保护
- **现状**: 已将等待时间从 200ms 延长到 400ms 改善稳定性

### [T-004] dpdm_source_init 使用硬编码地址
- **地址**: 0x4000c0bc 和 0x4000c0bc+4
- **含义**: NU17112 内部未文档化的 override 寄存器
- **规则**: **不要修改这段代码**，来自原厂调试序列，若升级芯片版本需重新向原厂确认

## 调试经验 (Debugging Lessons)

### [D-001] Source 侧无快充的诊断顺序
1. 看 `dpdm_map` 值 → 是否正确分配端口
2. 看 `SOURCE_CTRL.WORD` → EN_SRC_PROTOCOL bit0 是否为 1
3. 看是否有 "pd has work" → PD 已抢占，属正常
4. 看 `is_enter_dpdm_prot` → 若 false 说明手机未发起任何快充握手

### [D-002] Sink 侧充电器被识别为 SDP 的诊断顺序
1. 确认有 "bc12_init" 日志 → Sink 检测已触发
2. 看 "BC12 bc12_type=0x?" → 0x03(DCP) 才会继续探测 HVDCP
3. 非 DCP 充电器识别为 SDP/CDP 是正确行为

## 待验证假设 (Unverified Hypotheses)

### [H-001] PD 协商中的 QC 竞争窗口
- `pdlib_is_connect()` 只检查 PD "已连接"，不检查"协商中"
- 假设：TypeC Attach 后 PD 协商约 200-500ms 期间，QC 中断仍可能触发
- 需要验证：在 QC+PD 双能力设备上，是否出现 VBUS 短暂抖动
