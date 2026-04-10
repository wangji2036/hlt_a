# System Overview

**最后更新**: 2026-04-08
**来源文件**: app/main.c, osal/osal.c, osal/osal.h, startup/ckcpu.ld, hal/_hal.c

---

## 架构框图

```
+-----------------------------------------------------+
|                  Application Layer                   |
|  app/app.c   app/_wpc.c   app/port_manager.c        |
|  app/sleep.c app/gui.c    app/bat_record.c           |
+-----------------------------------------------------+
        |               |               |
+-------v-------+  +----v----+  +-------v-----------+
| FML 中间件层   |  | Power   |  | Gauge 电量算法    |
| fml/_fml.c    |  | power/  |  | gauge/            |
| fml/tcpm.c    |  | buckboost|  | (Simulink 自动    |
| fml/dpdm.c    |  |  .c     |  |  生成，只读)      |
| fml/ntc.c     |  +---------+  +-------------------+
| fml/bsp.c     |
+-------+-------+
        |
+-------v-------------------------------------------+
|              OSAL 调度器 (协作式)                    |
|  osal/osal.c — 8 Task 槽位, 30 Timer, 事件驱动     |
+-------+-------------------------------------------+
        |
+-------v-------------------------------------------+
|              HAL 硬件抽象层                          |
|  hal/gpio.c  hal/timer.c  hal/i2cm.c  hal/eadc.c  |
|  hal/epwm.c  hal/ecap.c   hal/fmc.c   hal/wdt.c   |
|  hal/nu6805.c hal/tcpc.c  hal/uart.c  hal/badc.c  |
+-------+-------------------------------------------+
        |
+-------v-------------------------------------------+
|              硬件                                    |
|  NU17112 MCU (CK802 @36MHz, 120KB Flash, 7KB SRAM)|
|  NU6805/SW7201 Buck-Boost | NU103x WPC AFE         |
|  WB7720 USB Bridge | FM1210/T91206 Qi SE           |
+---------------------------------------------------+
```

## OSAL 平台

| 项目 | 值 |
|------|---|
| OSAL 类型 | 自研轻量级 OSAL (非 TI/FreeRTOS) |
| 调度模型 | **协作式** (非抢占)，事件驱动 |
| Tick 来源 | TMR1_IRQHandler (hal/timer.c:175)，1ms 中断 |
| 主循环 | osal_start_system() (osal.c:247) — 喂狗 → 更新定时器 → 处理事件 |
| Task 槽位 | MAX_TASK = 8 (全部占满) |
| Timer 槽位 | MAX_TIMER = 30 (当前约 15 在用) |
| 事件位宽 | 32-bit (每 Task 最多 32 个事件) |
| 工具链 | C-Sky Development Suite (CDS) V5.2.14, csky-abiv2-elf-gcc |

## 内存布局

| 区域 | 起始地址 | 大小 | 用途 |
|------|---------|------|------|
| APROM (Flash) | 0x00002000 | 120 KB | 固件代码 + 常量 |
| CFG | 0x20000000 | 1 KB | 配置区 |
| SRAM | 0x20000400 | 7 KB | 全局变量 + 栈 |
| Stack | 0x20001FF8 | 向下增长 | 调用栈 |

## 代码库统计

| 目录 | 文件数 (约) | 职责 |
|------|-----------|------|
| app/ | 41 | 应用层: WPC协议、GUI、LED、睡眠、端口管理、电池记录 |
| fml/ | 29 | 中间件: USB/TCPM、DPDM快充、适配器检测、gauge接口、NTC |
| hal/ | 34 | 硬件抽象: GPIO、UART、Timer、ADC、I2C、TCPC、PWM、Flash |
| power/ | 4 | 电源管理: Buck-Boost充放电、电池管理 |
| gauge/ | 6 | BMS算法: SOC/SOH计算 (Simulink自动生成，只读) |
| osal/ | 2 | OSAL调度器核心 |
| util/ | 6 | 工具函数: delay、printk、算法辅助 |
| startup/ | 2 | 启动代码: crt0.S、链接脚本 ckcpu.ld |
| lib/ | ~10 | 外部库: typec、pd_tc、usb_pd、ask、pfod、wpc |
| usbpd/ | ~5 | USB PD 协议栈 |

## 关键入口点

| 入口 | 文件 | 行号 |
|------|------|------|
| main() | app/main.c | L40 |
| OSAL 初始化 | osal/osal.c:osal_init() | L51 |
| Task 注册 | app/main.c | L119-130 |
| 调度器启动 | osal/osal.c:osal_start_system() | L247 |
| BSP 初始化 | fml/bsp.c:fml_bsp_init() | (由 main.c L54 调用) |

## 启动序列 (main.c)

```
1. TCPC CC 使能 (L42-43)
2. VIC 中断控制器禁用 (L45)
3. 看门狗初始化 (L47)
4. RST 检查 (L49)
5. 数据结构初始化: ap, gd, lib 参数 (L51-53)
6. FML BSP 初始化 (L54)
7. GUI 初始化 (L56)
8. Nu103x POR 初始化 — 无线充电上电检测 (L60-75)
9. 喂狗 (L77-78)
10. SE IC 认证 — T91206 或 FM1210 (L93-112)
11. 适配器检测初始化 (L113)
12. 电池记录初始化 — CONFIG_NEW_CCC_LOG_ENABLE (L116)
13. OSAL 系统初始化 (L118)
14. 8 个 Task 依次注册 (L119-130)
15. osal_start_system() — 进入无限循环，不返回 (L133)
```

## 层间边界

| 边界 | 规则 |
|------|------|
| App → OSAL | 通过 `osal_set_event()` / `osal_start_timerEx()` 通信 |
| App → FML | 直接函数调用 (如 `fml_bsp_init()`, `tcpm_xxx()`) |
| App → HAL | **禁止直接调用**，必须通过 FML 层 |
| FML → HAL | 直接调用 HAL API (如 `hal_i2cm_read()`, `hal_epwm_start()`) |
| 任何层 → 全局数据 | 通过 `ap` (配置) 和 `gd` (运行时) 结构体共享 |

## 交叉引用

- 任务注册表: [osal-task-registry.md](osal-task-registry.md)
- 消息流: [message-flow.md](message-flow.md)
- 状态机: [state-machines.md](state-machines.md)
