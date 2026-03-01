# 跨模块共享经验

## 已确认的模式

### [S-001] 保护机制的标准模式
- 使用 over_cnt/reco_cnt 去抖计数器
- check 函数从周期性 ISR 调用
- 功率限制通过 pwr_lim.tar_cap[reason] 数组，min 选择
- 来源: VINDPM 实现 + prot.c 分析

### [S-002] OSAL 事件驱动模式
- 每个 Task 一个 event_handle(uint32_t event) 入口
- switch/case 分发，不要重构为表驱动（这是平台标准模式）
- 定时器触发异步事件: osal_start_timerEx(timer_id, delay, period, task, event)
- 来源: 全平台通用

### [S-003] Handler 必须快速返回，长耗时操作用定时器拆分
- **原则**: 任何 OSAL event handler 执行时间必须远低于 WDT 超时（约 2 秒）；有延时需求的操作必须拆分为"启动定时器 → 等定时器事件 → 继续"的两步模式
- **原因**: 调度是协作式的，handler 不返回则所有其他 Task 和主循环都被阻塞；主循环开头喂看门狗，handler 中不喂——执行超时直接触发 WDT 复位
- **推论（Implication）**: 代码中出现 `delay_1ms(N)` 且 N > 10ms 的 handler，说明存在调度阻塞风险，需要重构为定时器驱动

### [S-004] 每个 Timer ID 全局唯一，不可跨模块共用
- **原则**: `osal.h` 中的 Timer 枚举是全局资源，每个 Timer 槽位只能被一个模块使用
- **原因**: `osal_start_timerEx(timer_id, ...)` 会覆盖该槽位的所有配置；两个模块用同一 timer_id，后者会静默覆盖前者，前者的超时事件永不触发
- **推论**: 添加新功能需要新 Timer 时，必须在 `osal.h` 的枚举中分配新 ID，并确认不超过 `MAX_TIMER = 32`

### [S-005] ISR 仅发事件，逻辑在 Task 处理（AFC/SCP 例外）
- **原则**: 硬件中断 ISR 应该只读取中断状态、清除中断标志、调用 `osal_set_event()`，不在 ISR 中做协议逻辑
- **例外**: AFC/SCP 协议响应时间要求 ≤20ms，必须在 ISR 内同步完成，不走 OSAL 事件队列
- **推论**: 如果新增一个 ISR 中含有复杂逻辑或延时，检查该协议是否有严格的响应时间要求；若有，参照 AFC/SCP 模式；若无，必须移到 Task 处理

### [S-006] ap 是只读配置，gd 是运行时公告板
- **原则**: `ap`（配置参数）在启动时从 Flash 加载后运行时不修改；`gd`（运行时状态）在各 Task 间共享读写，每个字段有明确的"唯一写入者"
- **原因**: 多个 Task 并发写同一字段会导致状态竞争；ap 参数若运行时被意外修改会导致保护阈值错误
- **推论**: 如果发现两个 Task 都在写同一个 gd 字段，说明设计有问题——需要确认写入权限归属，其他 Task 应通过发事件请求写入者修改

## 已知陷阱

### [ST-001] CDS 构建路径中文乱码
- CDS Eclipse 生成的 makefile 含中文 Windows 路径（如"图拉斯"）
- 解决: makefile patch 脚本使用 LC_ALL=C sed + [^$] 匹配乱码字节
- 来源: 构建环境搭建

## 工程模式联机测试要点

### [S-007] 工程模式虚拟参数只影响异常检测，不影响硬件保护
- **原则**: `eng_virtual_cell1/cell2/temp` 仅在 `bat_record.c` 的 `battery_record_update_overvoltage()` 和 `battery_record_update_overtemperature()` 中使用
- **安全保证**: BuckBoost 充放电控制、NTC 温度保护、端口管理等子系统始终使用真实 ADC/NTC 值
- **推论**: 工程模式注入极端虚拟值不会导致硬件损坏或危险行为

### [S-008] USB 桥接链路的时序约束
- **端到端延迟**: PC 写 → WB7720 i2c_buff → NU17112 I2C 读 ≈ 50ms(HID) + 47~846ms(round-robin) ≈ 100ms~1s
- **异常记录回传**: NU17112 检测 → Flash 写入 → i2c_buff 轮转 → WB7720 exc_cache → HID Type 0x02 ≈ 3-15s
- **测试影响**: 所有联机测试步骤之间需插入足够等待时间，不能假设即时响应

## 平台约束备忘

- MCU: CK802 @ 36 MHz, 120KB Flash, 8KB SRAM (7KB 可用)
- 调度: 协作式，无抢占，任务需主动返回
- 全局数据: ap (配置), gd (运行时), 通过 g_data.h 共享
- 适配器: EADP_TYPE_POWERBANK_WIRELESS_ONLY, volt_max=19500, curr_max=3000
