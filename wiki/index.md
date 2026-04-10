# Wiki 索引 — NU17112 PowerBank (ARUN X20)

**最后更新**: 2026-04-08
**固件分支**: main (commit abe81b0)
**硬件**: NU17112 + SW7201 + WB7720

---

## 架构

| 页面 | 摘要 |
|------|------|
| [architecture/system-overview.md](architecture/system-overview.md) | 系统大图、OSAL 平台、内存布局、启动序列、层间边界 |
| [architecture/osal-task-registry.md](architecture/osal-task-registry.md) | **最重要** — 8 个 Task 注册表、30 个 Timer、事件定义、时序 |
| [architecture/message-flow.md](architecture/message-flow.md) | 87 处 osal_set_event 调用图、关键数据路径、时序约束 |
| [architecture/state-machines.md](architecture/state-machines.md) | 7 个状态机: WPC/TypeC/PD/DPDM/BuckBoost/PortMgr/Sleep |

## HAL / 驱动

| 页面 | 摘要 |
|------|------|
| [hal/hal-overview.md](hal/hal-overview.md) | HAL 目录总览、I2C 总线设备、27 个 ISR 列表 |
| [hal/i2c.md](hal/i2c.md) | I2C Master (软件 PA6/PA7)、Slave 地址: 0x21/0x3C/0x66/0x02 |
| [hal/timer-adc.md](hal/timer-adc.md) | Timer0~3 (1ms tick)、看门狗、EADC/BADC 通道 |
| [hal/pwm-capture.md](hal/pwm-capture.md) | EPWM (Qi TX 全桥 360kHz)、ECAP (谐振频率捕获) |

## 芯片

| 页面 | 摘要 |
|------|------|
| [chips/nu17112.md](chips/nu17112.md) | MCU: CK802 @36MHz, 120KB Flash, 7KB SRAM, 引脚分配 |
| [chips/nu6805.md](chips/nu6805.md) | Buck-Boost 控制器: I2C @0x3C, 49 寄存器, 充/放/关断模式 |
| [chips/wb7720.md](chips/wb7720.md) | USB HID Bridge: I2C @0x21, 256B 缓冲, 遥测/工程/生产模式 |
| [chips/nu103x.md](chips/nu103x.md) | WPC AFE: ASK 解调, Q-factor, DDM, OCP 保护 |

## 配置

| 页面 | 摘要 |
|------|------|
| [config/build-flags.md](config/build-flags.md) | config.h: 电池参数、保护阈值、功能开关、CV 递减策略 |

## 调试

| 页面 | 摘要 |
|------|------|
| [debug/patterns.md](debug/patterns.md) | 6 条铁律、已修复问题、常见故障排查表 |
| [debug/tools.md](debug/tools.md) | 串口 COM25@250000、构建命令、工程模式、Debug 工作流 |

---

## 快速参考

| 项目 | 值 |
|------|---|
| OSAL 类型 | 自研协作式, 事件驱动 |
| Task 数量 | 8 (全部占满) |
| Timer 数量 | 30 槽位 (约 20 在用) |
| 芯片 | NU17112 + NU6805 + NU103x + WB7720 + FM1210/T91206 |
| SRAM | 7 KB (栈 + 全局变量) |
| Flash | 120 KB (代码 + 配置 + 日志) |
| 最快 Task 周期 | 1ms (USB_TASK, PORT_MANAGER_TASK) |
| 串口调试 | COM25, 250000 baud |
