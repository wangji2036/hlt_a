# OSAL Task 注册表

**最后更新**: 2026-04-08
**来源文件**: osal/osal.h, osal/osal.c, app/main.c, 各 task 源文件

> 这是 wiki 的**锚定页面**。理解系统先从这里开始。

---

## Task 总览

| Task 名称 | Task ID | 优先级 | 源文件 | 注册函数 | Handler | 职责 |
|-----------|---------|--------|--------|---------|---------|------|
| HAL_TASK | 0 | 按 ID 顺序 | — | — | — | 预留 (未使用) |
| FML_TASK | 1 | 按 ID 顺序 | fml/_fml.c | fml_task_init() | fml_task_event_handler() | Gauge 数据采集、WB7720 HID 报告 |
| USB_TASK | 2 | 按 ID 顺序 | fml/tcpm.c | tcpm_task_init() | tcpm_task_event_handler() | USB PD/TypeC 协议栈 |
| WPC_TASK | 3 | 按 ID 顺序 | app/_wpc.c | wpc_task_init() | wpc_task_event_handler() | Qi 2.x TX 无线充电协议 |
| APL_TASK | 4 | 按 ID 顺序 | app/app.c | apl_task_init() | apl_task_event_handler() | 应用层: LED、睡眠、按键、保护监控 |
| BUCKBOOST_TASK | 5 | 按 ID 顺序 | power/buckboost.c | buckboost_task_init() | buckboost_task_event_handler() | 充放电控制、电压电流管理 |
| USB_DPDM_TASK | 6 | 按 ID 顺序 | fml/dpdm.c | usb_dpdm_task_init() | usb_dpdm_task_event_handler() | D+/D- 快充协议: BC1.2/QC/AFC/SCP/UFCS |
| PORT_MANAGER_TASK | 7 | 按 ID 顺序 | app/port_manager.c | port_manager_task_init() | port_manager_event_handle() | 4 端口仲裁、模式切换、功率分配 |

> **调度模型**: 协作式，无优先级抢占。OSAL 主循环按 Task ID 0→7 顺序扫描，有待处理事件的 Task 依次执行。

## Task 启动顺序 (main.c L119-130)

```
L118: osal_init()
L119: apl_task_init()          → APL_TASK (ID=4)
L121: buckboost_task_init()    → BUCKBOOST_TASK (ID=5)
L122: tcpm_task_init()         → USB_TASK (ID=2)
L123: usb_dpdm_task_init()     → USB_DPDM_TASK (ID=6)
L126: fml_task_init()          → FML_TASK (ID=1)
L128: wpc_task_init()          → WPC_TASK (ID=3)
L130: port_manager_task_init() → PORT_MANAGER_TASK (ID=7)
L133: osal_start_system()      → 进入无限循环
```

> 注意: 注册顺序 ≠ ID 顺序。执行时按 ID 顺序扫描。

---

## 定时器分配

### Timer ID 定义 (osal.h L4-46)

| Timer ID | 宏名 | 所属 Task | 周期 | 触发事件 |
|----------|------|----------|------|---------|
| 0 | APP_010ms_TIMER | APL_TASK | 10ms | APL_EVT_010ms_POLL |
| 1 | APP_100ms_TIMER | APL_TASK | 100ms | APL_EVT_100ms_POLL |
| 2 | APP_250ms_TIMER | APL_TASK | 250ms | APL_EVT_250ms_POLL |
| 3 | USB_TIMER | USB_TASK | — | (通用) |
| 4 | WPC_PING_TIMER | WPC_TASK | 可配 | WPC_EVT_DIG_PING |
| 5 | WPC_ID_TIMER | WPC_TASK | — | WPC 协议 |
| 6 | WPC_NEGO_TIMER | WPC_TASK | — | WPC 协议 |
| 7 | WPC_CE_TIMER | WPC_TASK | — | WPC 协议 |
| 8 | WPC_RPWR_TIMER | WPC_TASK | — | WPC 协议 |
| 9 | WPC_PRMC_TIMER | WPC_TASK | — | WPC 协议 |
| 10 | WPC_DDM_TIMER | WPC_TASK | — | WPC 协议 |
| 11 | USB_TC_PD_TIMER | USB_TASK | 1ms | TCPM_EVT_TIME_PERIOD |
| 12 | USB_BC12_TIMER | USB_DPDM_TASK | 100ms | DPDM_EVT_TIMER_PERIOD |
| 13 | USB_QC_TIMER | USB_DPDM_TASK | — | QC 协议 |
| 14 | BUCKBOOST_PERIOD_TIMER | BUCKBOOST_TASK | 17ms | BUCKBOOST_EVT_TIME_PERIOD |
| 15 | BUCKBOOST_VBUS_TIMER | BUCKBOOST_TASK | 20ms | BUCKBOOST_EVT_VBUS_PERIOD |
| 16 | DPDM_SINK_TIMER | USB_DPDM_TASK | — | DPDM sink |
| 17-21 | TCPM_PORT0~4_TIMER | USB_TASK | — | 端口定时器 |
| 22 | GAUGE_TIMER | FML_TASK | 100ms | APL_EVT_GAUGE |
| 23 | BUCKBOOST_CHAGER_TIMER | BUCKBOOST_TASK | 500ms | BUCKBOOST_EVT_CHAG_PERIOD |
| 26-28 | BUCKBOOST_xxx | BUCKBOOST_TASK | — | 辅助定时器 |
| 29 | USB_WB7720_TIMER | FML_TASK | 47ms | APL_HID_REPORT |

> MAX_TIMER = 30。当前约 20 个在用。

---

## 各 Task 事件定义

### APL_TASK 事件 (app/app.h L7-9)

| 事件位 | 宏名 | 触发源 | 用途 |
|--------|------|--------|------|
| bit 0 | APL_EVT_010ms_POLL | APP_010ms_TIMER | 10ms 周期: 按键扫描、LED 刷新 |
| bit 1 | APL_EVT_100ms_POLL | APP_100ms_TIMER | 100ms 周期: NTC 读取、保护检查 |
| bit 2 | APL_EVT_250ms_POLL | APP_250ms_TIMER | 250ms 周期: 睡眠检测、状态上报 |

### FML_TASK 事件 (_fml.h L6-8)

| 事件位 | 宏名 | 触发源 | 用途 |
|--------|------|--------|------|
| bit 0 | FML_EVT_ASK_INT_RECVD | ISR (ASK 中断) | Qi ASK 数据包接收 |
| bit 1 | APL_EVT_GAUGE | GAUGE_TIMER (100ms) | Gauge SOC/SOH 数据更新 |
| bit 2 | APL_HID_REPORT | USB_WB7720_TIMER (47ms) | 向 WB7720 写入 HID 遥测数据 |

### USB_TASK/TCPM 事件 (tcpm.h L99-112)

| 事件位 | 宏名 | 用途 |
|--------|------|------|
| bit 0 | TCPM_EVT_TIME_PERIOD | 1ms TypeC/PD 状态机驱动 |
| bit 11 | TCPM_EVT_USBA_SCAN | USB-A 插入检测 |
| bit 12 | TCPM_EVT_USBA_REDETECT | USB-A 重检测 |
| bit 17 | TCPM_EVT_QI_SET_VOLT | Qi 请求电压设置 |
| bit 19 | TCPM_EVT_DPDM_DONE | DPDM 协议检测完成 |

### BUCKBOOST_TASK 事件 (buckboost.h L109-131)

| 事件位 | 宏名 | 用途 |
|--------|------|------|
| bit 0 | BUCKBOOST_EVT_SWITCH_WORK_MODE | 充放电模式切换 |
| bit 1 | BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT | 设置放电 VBUS 电压 |
| bit 30 | BUCKBOOST_EVT_VBUS_PERIOD | 20ms VBUS 扫描周期 |
| bit 31 | BUCKBOOST_EVT_TIME_PERIOD | 17ms 主周期 |
| ... | (共 28 个事件) | 充放电控制相关 |

### USB_DPDM_TASK 事件 (dpdm.h L253-287)

| 事件位 | 宏名 | 用途 |
|--------|------|------|
| bit 0 | DPDM_EVT_SRC_ATTACHED | SRC 插入 |
| bit 31 | DPDM_EVT_TIMER_PERIOD | 100ms 周期扫描 |
| ... | (共 28 个事件) | QC/AFC/SCP/UFCS 协议事件 |

### WPC_TASK 事件 (osal.h L64-90)

| 事件位 | 宏名 | 用途 |
|--------|------|------|
| bit 0 | WPC_EVT_DIG_PING | 数字 Ping 检测 |
| bit 1 | WPC_EVT_PIN_NO_PKT | Ping 无响应 |
| ... | (共 27 个事件) | Qi 协议 6 阶段状态机驱动 |

### PORT_MANAGER_TASK 事件 (port_manager.h L10-34)

| 事件位 | 宏名 | 用途 |
|--------|------|------|
| bit 0-3 | PORT_ENUM_EVT_PORT0~3_CONNECT_START | 端口连接开始 |
| bit 4-7 | PORT_ENUM_EVT_PORT0~3_DISCONNECT | 端口断开 |
| ... | (共 16 个事件) | 端口枚举和仲裁 |
| bit 31 | PORT_ENUM_EVT_PORT_SCAN | 1ms 端口扫描周期 |

---

## Task 生命周期

### APL_TASK (app/app.c)
- **init**: apl_task_init() — 启动 3 个周期定时器 (10/100/250ms)
- **主循环**: 由 3 个定时事件驱动，永不终止
- **10ms**: key_scan(), led_refresh()
- **100ms**: ntc_read(), protection_check(), battery_record_update()
- **250ms**: sleep_check(), usb_bridge_status_report()

### FML_TASK (fml/_fml.c)
- **init**: fml_task_init() — 启动 GAUGE_TIMER(100ms) + USB_WB7720_TIMER(47ms)
- **100ms**: gauge_task() — 更新 SOC/SOH
- **47ms**: ubsd_wb7720_report_update() — 写入 WB7720 I2C 遥测数据

### USB_TASK (fml/tcpm.c)
- **init**: tcpm_task_init() — 启动 1ms 定时器
- **1ms**: tcpm_process() — TypeC 状态机 + PD 策略引擎
- 管理 Port0 (TypeC0) 和 Port1 (TypeC1) 两个 TypeC 端口

### WPC_TASK (app/_wpc.c)
- **init**: wpc_task_init() — 启动 WPC_PING_TIMER
- **主循环**: Qi 2.x TX 6 阶段状态机 (Idle→Ping→Cnfg→Nego→Xfer→Cloak)
- 事件驱动: ASK/FSK 中断 → 状态转移

### BUCKBOOST_TASK (power/buckboost.c)
- **init**: buckboost_task_init() — 启动 3 个定时器 (17/20/500ms)
- **17ms**: 主控制循环 — ADC 采样、充放电控制
- **20ms**: VBUS 电压监控
- **500ms**: 充电器状态管理

### USB_DPDM_TASK (fml/dpdm.c)
- **init**: usb_dpdm_task_init() — 启动 100ms 定时器
- **100ms**: D+/D- 协议检测状态机
- 支持 BC1.2, QC2.0/3.0, AFC, SCP/FCP, UFCS

### PORT_MANAGER_TASK (app/port_manager.c)
- **init**: port_manager_task_init() — 启动 1ms 定时器
- **1ms**: 4 端口枚举扫描
- 管理端口优先级、模式切换、功率预算

---

## 时序总览

```
1ms  ─────────────────────────────────────────── USB_TASK, PORT_MANAGER_TASK
     ┃
10ms ─────────────────────────── APL_TASK (按键/LED)
     ┃
17ms ──────────────────── BUCKBOOST_TASK (主控制)
20ms ────────────────── BUCKBOOST_TASK (VBUS)
     ┃
47ms ──────────── FML_TASK (HID 报告)
     ┃
100ms ──────── APL_TASK (保护), FML_TASK (Gauge), USB_DPDM_TASK
     ┃
250ms ──── APL_TASK (睡眠检测)
     ┃
500ms ── BUCKBOOST_TASK (充电器)
```

## 交叉引用

- 系统概览: [system-overview.md](system-overview.md)
- 消息流: [message-flow.md](message-flow.md)
- 状态机: [state-machines.md](state-machines.md)
