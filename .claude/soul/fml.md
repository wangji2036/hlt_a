# FML 模块经验

## 已确认的模式 (Confirmed Patterns)

## 已知陷阱 (Known Pitfalls)

## 调试经验 (Debugging Lessons)

## 待验证假设 (Unverified Hypotheses)

---

## 设计意图与不变约束 (Design Rationale & Invariants)

> 这里记录的是「为什么」，不是「是什么」。「是什么」应当读代码。
> 参考完整文档: `docs/FML_GUIDE.md`

### [D-001] 数据结构初始化必须先于 BSP，BSP 必须先于任何 Task

**原则**: `ap_data_init()` → `gd_data_init()` → `lib_para_init()` 必须在 `fml_bsp_init()` 之前；`fml_bsp_init()` 必须在所有 `xxx_task_init()` 之前；全部完成后才能调用 `osal_start_system()`。

**原因**: 所有 Task 通过 `ap` 和 `gd` 全局变量共享状态。如果 Task 在数据结构初始化之前注册并开始运行，它们会读到未初始化的 RAM（随机值），导致保护阈值异常、协议参数错误等不可预测行为。HAL 外设同理，I2C/ADC 未初始化时 Task 调用会直接挂起。

**推论**: 在 `main.c` 中增加新的初始化步骤时，必须判断它属于哪个阶段：数据结构类（Phase 2）、硬件类（Phase 3-4）、还是 Task 类（Phase 8）。不能为了方便把数据结构初始化放到 Task 内部（例如在 `fml_task_init()` 里调用 `ap_data_init()`）——此时其他 Task 可能已经在用 `ap` 了。

### [D-002] `power_on_magic` 是应用层冷/热启动协议，不依赖 MCU 复位源寄存器

**原则**: 冷/热启动的判断必须用 `gd->power_on_magic == 0xaaaa`，而不是读 MCU 的复位源寄存器（SYS->RESET_SRC 等）。

**原因**: MCU 复位源寄存器无法区分"应用层主动软复位（睡眠）"和"看门狗超时复位"——两者硬件上都是软复位。`power_on_magic` 是应用层约定的握手协议：只有通过正常启动序列设置了魔数，热启动才会被识别。意外复位（看门狗、异常复位）后 RAM 内容可能保留，但 `RST_vCheck()` 在 `ap_data_init()` 之前检查复位源，可用于区分主动睡眠复位和异常复位。

**推论**: 任何修改 `power_on_magic` 字段在 `gd_t` 中位置的操作，都会导致旧版本遗留的魔数出现在新地址的随机数据中，热启动误判。固件升级后若怀疑冷/热启动行为异常，首先检查 `gd_t` 结构布局是否改变。

### [D-003] `gd_t` 的 `resverd_reset` 是睡眠边界，其后的字段是跨复位持久数据区

**原则**: `gd_t` 中，`resverd_reset` 字段之前的所有字段每次启动都被清零（包括热启动）；之后的字段在热启动时保留（包括 `g_bat` 电量积分状态）。

**原因**: WPC 协议状态、PID 参数等瞬态数据在每次复位后必须归零，否则残留状态会导致错误的协议行为（例如上次充电的 CEP 值被用于新的握手）。而 SOC、lighting mode、电量积分等数据如果每次复位都清零，睡眠唤醒后 LED 会跳变、Gauge 会重新初始化，用户体验极差。两类数据用 `resverd_reset` 分隔，一次 `memset` 只清前半段。

**推论**: 在 `gd_t` 中添加新字段时，必须明确它属于哪一类。把需要跨睡眠保留的字段放到 `resverd_reset` 之前，热启动时会被清零——这是常见的调试陷阱。同理，把临时状态字段放到边界之后会引入 bug（残留值污染新会话）。

### [D-004] FML_TASK 的 Gauge 执行者角色：算法不在 fml/ 目录

**原则**: 虽然 FML_TASK 负责每 100ms 触发电量计算，但算法实现（`battery_task_handle()`）位于 `power/bat.c`，而不是 `fml/` 目录。FML_TASK 只是调度者，不实现算法。

**原因**: 电量计算所需的输入数据（电池电压、电流、充电状态）全部来自 BuckBoost 模块（`g_buckboost.*`），算法与 BuckBoost 的耦合程度高于与 FML 的耦合。从依赖关系上 bat.c 属于 BuckBoost 域，只是执行时机由 FML_TASK 的定时器触发。

**推论**: 查找 Gauge/SOC 相关 bug 时，不要局限在 `fml/` 目录。修改 Gauge 算法参数（OCV 查表、能量单位、校准阈值）需要修改 `power/bat.c`。`bat_info` 结构体的任何改动都会影响 `gd_t` 的大小和 `resverd_reset` 之后的内存布局。

### [D-005] 适配器类型变更必须通过 `fml_adp_type_set()` 进行，不能直接写 `gd->adp`

**原则**: 所有修改适配器类型的操作必须调用 `fml_adp_type_set()`，不能直接修改 `gd->adp` 字段。

**原因**: `fml_adp_type_set()` 除了更新 `gd->adp`，还会：① 设置 `gd->adp_type_upd = 1` 通知 WPC_TASK 重新协商功率；② 调用 `power_capability_init()` / `power_limit_reset()` / `power_limit_init()` 重建功率限制表。直接写 `gd->adp` 会绕过这三个副作用，导致 WPC 功率限制状态与适配器能力不一致（例如适配器升级到 25W 但功率限制仍按 7.5W 计算）。

**推论**: 如果发现 WPC 功率上不去但适配器识别正确，优先检查适配器更新是否通过 `fml_adp_type_set()` 进行（而非直接赋值），以及 `adp_type_upd` 标志是否被 WPC_TASK 正确消费。

### [D-006] SE IC 初始化必须在 `osal_start_system()` 之前完成，中间需喂看门狗

**原则**: T91206（Qi 认证 SE IC）的 I2C 初始化序列（`init` + `read_cert_hash` + `read_se_cert` + `get_qi_id`）必须在调度器启动前完成，中间需要多次调用 `hal_wdt_feed()`。

**原因**: SE IC 的 I2C 操作是阻塞式的，单次操作可能耗时数十至数百毫秒（证书链读取 363 字节尤其慢）。调度器启动后无法做长时阻塞操作。看门狗超时时间有限，SE IC 初始化过程中如果不喂狗会触发复位重启，导致系统无法启动。

**推论**: 新增 SE IC 类型或扩展认证流程时，每个耗时超过 ~50ms 的操作后都要加 `hal_wdt_feed()`。证书链读取（`t91206_read_se_cert()`）必须在 `t91206_get_qi_id()` 之前执行，顺序不可颠倒（SE IC 内部状态机要求）。

## 调试直觉 (Debugging Heuristics)

### [H-001] 开机 SOC 跳变或 LED 格数不对

最常见原因：① 冷启动时 Gauge 的 30 次延迟（3 秒）还没结束，`bat_level_ui` 还是 0；② 热启动时 `gd->g_bat` → `g_bat` 的复制未能保留有效状态（`bat_is_inited` 不等于魔数）；③ OCV 查表的输入 `vbat` 包含了充电电流造成的压降（需减去 IR drop）。

排查：看启动日志中 `"inited [magic vbat ocv ui]"` 是否打印（说明 30 次延迟结束）；检查是冷启动还是热启动（看 `"----powereon reset"` 是否出现）；打印 `g_bat.vbat` 和 `g_bat.ibat` 确认 ADC 数据是否有效。

### [H-002] 适配器已接入但 WPC 功率未提升

最常见原因：① `fml_adp_type_set()` 未被调用，`gd->adp` 停留在 `POWERBANK_WIRELESS_ONLY` 默认值；② `adp_type_upd` 标志被设置但 WPC_TASK 在处理其他事件时未及时检测；③ `wpc_gear` 字段（在 `fml_adp_type_set()` 的第 6 个参数）设置不正确（GEAR2 才允许 15W）。

排查：看日志 `"ADP->"` 打印，确认 `adp_type` 和 `wpc_gear` 值；在 WPC_TASK 中搜索 `adp_type_upd` 的读取位置，确认是否被消费并清零。

### [H-003] 系统冷启动后 3 秒内 LED 不显示电量

这是正常行为，不是 bug。`battery_task_handle()` 有内置的 30 次（3 秒）启动延迟，等待 BuckBoost ADC 稳定后才初始化 OCV。在此期间 `g_bat.bat_level_ui = 0`，APL 可能显示 0% 或不亮灯。

排查：如果 3 秒后仍不显示，检查 BuckBoost ADC（`g_buckboost.adc_vbat`）是否有有效值，以及 `GAUGE_TIMER` 是否在正常触发（每 100ms 应有 `"i= [vbat ibat rbat]"` 日志）。

### [H-004] 每次睡眠唤醒后 SOC 不连续（跳到 0 或某个固定值）

最常见原因：`gd->g_bat` 没有在睡眠前被正确保存（APL sleep 路径中写入 `gd->g_bat = g_bat` 的操作被跳过），或 `resverd_reset` 之后的区域因 `gd_t` 结构变化而被意外覆盖。

排查：在 APL 的睡眠入口函数处加日志，确认 `gd->g_bat` 写入步骤执行了；检查 `gd_t` 结构体中 `g_bat` 字段相对于 `resverd_reset` 的位置是否正确（必须在其后）。
