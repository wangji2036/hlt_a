# Timer 与 ADC 驱动

**最后更新**: 2026-04-08
**来源文件**: hal/timer.c, hal/timer.h, hal/eadc.c, hal/eadc.h, hal/badc.c, hal/badc.h, hal/wdt.c

---

## Timer (hal/timer.c)

**硬件**: TMR0 ~ TMR3
**时钟源**: LIRC @ 64 kHz
**模式**: 单次 (Oneshot) / 周期 (Periodic)

### 关键用途

| Timer | ISR | 周期 | 用途 |
|-------|-----|------|------|
| TMR1 | TMR1_IRQHandler (timer.c:175) | **1ms** | OSAL 系统 tick 源 (sys_ticks++) |
| TMR0 | TMR0_IRQHandler | 可配 | USB PD 定时 |
| TMR2 | TMR2_IRQHandler | 可配 | 辅助定时 |
| TMR3 | TMR3_IRQHandler | 可配 | 辅助定时 |

> TMR1 是系统心跳，不可禁用。osal_tick_cnt_get() 通过 sys_ticks 差值计算经过时间。

## 看门狗 (hal/wdt.c)

| 函数 | 说明 |
|------|------|
| hal_wdt_init() | 初始化 (1000ms 超时) |
| hal_wdt_init_to_reset() | 初始化并使能复位 |
| hal_wdt_feed() | 喂狗 (OSAL 主循环每轮调用) |
| hal_wdt_stop() | 停止看门狗 |

> 循环 >500ms 的操作必须喂狗，否则系统复位。

## EADC — 增强 ADC (hal/eadc.c)

**用途**: Qi 2.0 模拟解调、电压测量

### 通道

| 通道 | 宏名 | 用途 |
|------|------|------|
| 0 | AVSS | 地参考 |
| 1 | TEST | 测试 |
| 2 | VCAP | 电容电压 |
| 3 | VPGA | PGA 输出 |
| 4 | V1P2 | 1.2V 基准 |
| 5 | V055 | 0.55V 基准 |

### API

| 函数 | 说明 |
|------|------|
| hal_eadc_meas(chan) | 单次测量指定通道 |
| hal_eadc_ddm_init() | DDM 模拟解调模式初始化 |

## BADC — 基础 ADC (hal/badc.c)

**用途**: 电池电压、电流、NTC 温度测量

### 关键通道

| 引脚 | 通道 | 分压比 | 用途 |
|------|------|--------|------|
| PB6 | ADC7 | ÷3 | BAT2+ 电池正极采样 |
| PC7 | ADC4 | ÷3 | Cell2 单节电压 |
| PD3 | ADC9 | — | VBAT- 负极 |
| PC6 | ADC0 | — | NTC 温度 |
| PB2 | ADC1 | — | NTC 温度 |

> ADC 值经分压比校正后得到实际电压。

## 交叉引用

- OSAL tick 机制: [../architecture/osal-task-registry.md](../architecture/osal-task-registry.md)
- BuckBoost ADC 使用: [../chips/nu6805.md](../chips/nu6805.md)
- HAL 概览: [hal-overview.md](hal-overview.md)
