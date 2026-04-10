# HAL 硬件抽象层概览

**最后更新**: 2026-04-08
**来源文件**: hal/*.c, hal/*.h

---

## 目录结构

```
hal/
├── _hal.c/.h       ← HAL 初始化入口
├── sys.c/.h        ← 系统时钟 PLL (36MHz)
├── regdef.h        ← 寄存器地址定义 (847行)
├── overview.h      ← NU171xx API 文档
├── gpio.c/.h       ← GPIO 端口配置
├── uart.c/.h       ← UART1/UART2 串口
├── timer.c/.h      ← Timer0~3 (1ms tick)
├── wdt.c/.h        ← 看门狗 (1s 超时)
├── i2cm.c/.h       ← I2C Master (软件, PA6/PA7)
├── i2cs.c/.h       ← I2C Slave (硬件)
├── eadc.c/.h       ← 增强 ADC (Qi 解调)
├── badc.c/.h       ← 基础 ADC (电池/电源测量)
├── epwm.c/.h       ← 增强 PWM (Qi TX 全桥 @360kHz)
├── bpwm.c/.h       ← 基础 PWM
├── ecap.c/.h       ← 增强捕获 (Qi 谐振频率)
├── ddm.c/.h        ← 数字解调模块
├── fmc.c/.h        ← Flash 控制器 (512B 页擦除)
├── tcpc.c/.h       ← TypeC 电源控制器
├── nu6805.c/.h     ← NU6805 Buck-Boost IC 驱动 (I2C @0x3C)
├── nu6801.c/.h     ← NU6801 Buck-Boost IC 驱动 (I2C @0x66)
├── vic.c/.h        ← 向量中断控制器
└── isr.c/.h        ← 27 个 ISR 处理函数
```

## I2C 总线设备

| 地址 | 设备 | 接口 | 用途 |
|------|------|------|------|
| 0x21 | WB7720 | I2C Master (软件) | USB HID Bridge — 遥测/工程/生产模式 |
| 0x3C | NU6805/SW7201 | I2C Master (软件) | 双向 Buck-Boost 充放电控制器 |
| 0x66 | NU6801 | I2C Master (软件) | 备用 Buck-Boost (当前禁用) |
| 0x02 | FM1210 | I2C Master (软件) | Qi SE 安全认证芯片 |

> I2C Master 使用 PA6 (SCL) / PA7 (SDA) 软件位操作实现。

## 中断处理器 (isr.c)

| ISR | 来源 | 用途 |
|-----|------|------|
| TMR0~3_IRQHandler | Timer0~3 | 定时中断 (TMR1=1ms tick) |
| ECAP1~5_IRQHandler | ECAP | Qi 谐振频率捕获 |
| UART1/2_IRQHandler | UART | 串口收发 |
| FSK1/2_IRQHandler | FSK | Qi FSK 通信 |
| WDT_IRQHandler | WDT | 看门狗溢出 |
| EADC_IRQHandler | EADC | ADC 转换完成 |
| BADC_IRQHandler | BADC | 电池 ADC 采样完成 |
| I2CM_IRQHandler | I2C Master | I2C 传输完成 |
| I2CS_IRQHandler | I2C Slave | I2C 从机被访问 |
| USBPD_IRQHandler | USB PD | PD 协议事件 |
| UFCS_IRQHandler | UFCS | 融合快充事件 |
| DPDM_SINK_IRQHandler | DPDM | BC1.2 Sink 检测 |
| DCP_HVDCP_IRQHandler | DCP/HVDCP | DCP 快充 |
| QC_SRC_IRQHandler | QC | Quick Charge 源 |
| AFC_SCP_SRC_IRQHandler | AFC/SCP | AFC/SCP 源 |
| TCPC_IRQHandler | TCPC | TypeC 电源控制 |
| DMA_IRQHandler | DMA | DMA 传输完成 |

## 交叉引用

- 各驱动详情: [i2c.md](i2c.md), [timer-adc.md](timer-adc.md), [pwm-capture.md](pwm-capture.md)
- 芯片页面: [../chips/nu17112.md](../chips/nu17112.md)
- 系统概览: [../architecture/system-overview.md](../architecture/system-overview.md)
