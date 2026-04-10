# I2C 通信驱动

**最后更新**: 2026-04-08
**来源文件**: hal/i2cm.c, hal/i2cm.h, hal/i2cs.c, hal/i2cs.h

---

## I2C Master (软件实现)

**引脚**: SCL = PA6, SDA = PA7 (开漏 + 上拉)
**时钟源**: LIRC @ 64 kHz
**支持速率**: 100 kHz / 400 kHz

### 公共 API

| 函数 | 参数 | 返回 | 说明 |
|------|------|------|------|
| hal_i2cm_init() | void | void | 初始化 GPIO 引脚 |
| hal_i2cm_read_one_byte() | slave_addr, reg_addr | u8 | 读 1 字节 (8-bit 寄存器地址) |
| hal_i2cm_wirte_one_byte() | slave_addr, reg_addr, data | void | 写 1 字节 |
| hal_i2cm_read_multi_bytes() | slave_addr, reg_addr, buf, len | void | 连续读 N 字节 |
| hal_i2cm_write_multi_bytes() | slave_addr, reg_addr, buf, len | void | 连续写 N 字节 |

> 注意: `wirte` 是代码中的拼写，非笔误文档。

### 使用场景

| 调用者 | 目标设备 | 地址 | 用途 |
|--------|---------|------|------|
| fml/_fml.c (ubsd_wb7720_report_update) | WB7720 | 0x21 | 写入遥测数据到 i2c_buff |
| power/buckboost.c | NU6805 | 0x3C | 充放电控制、ADC 读取 |
| fml/fm1210.c | FM1210 | 0x02 | Qi SE 认证 |
| fml/nu103x.c | NU103x | 脉冲通信 | WPC AFE 配置 (非标准 I2C) |

## I2C Slave (硬件)

**基地址**: 0x40009000
**寄存器组**: GR00 (只读), GR10 (读写), GR03 (中断源)
**中断**: GR03_INT_EN → I2CS_IRQHandler

> I2C Slave 用于 NU17112 作为从设备时的通信（当前系统中 NU17112 主要作为 Master）。

## 交叉引用

- WB7720 寄存器映射: [../chips/wb7720.md](../chips/wb7720.md)
- NU6805 寄存器: [../chips/nu6805.md](../chips/nu6805.md)
- HAL 概览: [hal-overview.md](hal-overview.md)
