# usb-device-agent Soul — WB7720 下位机经验积累

## 项目背景

- 项目: ARUN N3C (基于 ARUN IP162N 15W PowerBank2)
- MCU: WB7720 (WestBerry, ARM Cortex-M), Keil MDK
- 固件版本: NANFU_USB_1052_20260121A
- 作用: NU17112 ↔ Windows PC 的 USB HID 桥接

## 已知陷阱

### I2C 并发安全
- `i2c_buff[256]` 由 I2C 中断(NU17112 写入)和 USB 任务(上位机读取)共享
- 必须确保读写原子性，避免撕裂读（half-updated buffer）

### hidapi 前缀要求
- WB7720 HID 描述符不使用 Report ID
- hidapi 库在 write() 时要求首字节为 0x00，实际发送 65 字节
- 如果忘记这个前缀，CMD 字节会偏移一位，导致下位机无法解析命令

### 工程模式清除
- NU17112 处理完工程数据后必须将 REG_WORK_MODE (0x50) 清零
- 如果不清零，下次轮询时 NU17112 会重复处理，导致数据被覆写

### 生产模式断电风险
- product_info_write() 是先擦页再写入，断电会导致全页丢失
- 产线操作需确保写入期间供电稳定
- 上位机应在写入前给出警告

## 单位换算记忆

WB7720 本身不做单位换算，只是透传 i2c_buff：
- VBAT: NU17112 写入 mV，上位机读取后 ÷100 显示为 V（WB7720 HID 报告单位是 V×100）
- 实际上 NU17112 写入的是原始 mV，WB7720 原封不动放进 HID 报告
- 上位机做 ÷100 转换（因为 HID 协议定义单位是 V×100，即 mV÷10）

## 调试经验

- 初次调试时用 HID Wireshark 或 USB 协议分析器抓包，确认 SOF `05 A5 5A` 存在
- 若上位机收到全零报告，先检查 NU17112 是否已写入 i2c_buff（I2C 通信是否正常）
- 若上位机 CMD 无响应，检查 hidapi write 是否加了 0x00 前缀
