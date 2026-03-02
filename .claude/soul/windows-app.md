# windows-app-agent Soul — Windows 上位机经验积累

## 项目背景

- 项目: ARUN N3C
- 技术栈: Python 3.x + tkinter + hidapi
- 核心文件: `battery_monitor.py`
- UI 主题: 浅蓝灰背景(#F0F4F8) + 白色卡片(#FFFFFF) + 绿色正常(#4CAF50) + 红色异常(#FF4444)

## 已知陷阱

### hidapi 前缀
- `hid.write()` 必须以 `bytes([0x00])` 开头（无 Report ID 设备的 hidapi 要求）
- 格式: `device.write(bytes([0x00, CMD, ...填充至64字节]))`
- 实际发送 65 字节，下位机收到 64 字节

### HID 线程安全
- tkinter 非线程安全，HID 轮询必须在后台线程运行
- GUI 更新必须通过 `root.after()` 或 `queue` 传回主线程
- 不可在后台线程直接操作 tkinter Widget

### 温度智能检测
- 旧固件直接传°C（值通常 20-45），新固件传 °C×10（值 200-450）
- 上位机用 `< 100 则直接用，≥ 100 则 ÷10` 判断，保持向后兼容
- 此逻辑是刻意设计，不要"修复"它

### 工程模式写入顺序
- 4 步必须顺序执行，每步间隔 ≥50ms
- 顺序: 解锁(0x50=0xA5) → 当前日期(0x60) → 生产日期(0x70) → 循环次数(0x80)
- 不能并发，不能跳步

### 当前日期寄存器 0x60 包含 H/M/S (P2)
- 0x60 写入 7 字节: `struct.pack('<H', y) + bytes([m, d, h, mi, s])`
- H/M/S 在 `_write_engineering_worker` 写入时通过 `datetime.now()` 实时取，不依赖 UI 输入框
- UI 输入框只提供 Y/M/D（用户可能在填完表单后延迟点写入，所以 H:M:S 必须取发送瞬间时刻）
- `from datetime import datetime` 已加入顶层 import（line 23）
- 生产日期 0x70 仍只写 4 字节（Y/M/D），无时间字段
- 0xAA refresh (0x88) 不重发日期，无需再次写 0x60

### 生产模式并发
- 生产模式写入期间必须暂停遥测轮询（FLAG 0x90=0xB5 已设置时）
- 否则 NU17112 可能在读取 ProductInfo 时被遥测写入打断

## 数据解析注意

- 所有多字节: Little-Endian (`struct.unpack('<H', ...)` 等)
- 电流 s16: 充电为正，放电为负（显示时加符号）
- SOH: u16 单位 pct×100，显示时 ÷100（如 9800 → 98.00%）
- 异常日志每条 **20 字节** (BatteryExceptionRecord_t), ErrType 0x01=过压/0x02=过温/0x03=欠温
- record_id == 0 为无效记录（WB7720 exc_cache 空槽），上位机需跳过

## 调试经验

- 连接失败时先确认 VID/PID 正确（Windows 设备管理器查看）
- 数据全零: 下位机尚未收到 NU17112 的 I2C 数据
- 温度显示异常: 检查是否触发了智能检测的边界（值接近 100）
- 工程模式写入无反应: 检查 NU17112 固件是否实现了轮询 REG_WORK_MODE

## UI 布局经验

### 连接栏布局 (600px 窗口宽度)
- 左侧 VID/PID/UsagePage 输入框 + 刷新/设备列表/连接/状态 占满 ~560px
- 模式 Radiobutton (用户/工程/生产) 与连接控件在同一行时会被裁剪
- **解决方案**: 连接栏分两行 — row1 连接控件, row2 模式切换（居左对齐）
- 使用 `fill='x'` + `padx=8` 保证两行都撑满宽度

### 标题区与 SOC 圆环间距
- 标题 `title_frame.pack(pady=(12, 4))` — 减小顶部间距避免与圆环重叠
- SOC 容器 `container.pack(pady=(0, 0))` — 紧贴标题
- SOCRingWidget canvas 高度 260px，圆心 cy=125，无需额外 padding

### 断连时状态清除
- `_clear_all_data()` 中 `_exception_seen_ids.clear()` + `_exception_list.clear()` 很重要
- WB7720 掉电后 exc_cache 清空，重连后 record_id 可能从 0 重新开始
- 若不清除 seen_ids，重连后相同 record_id 的新记录会被误判为重复而丢弃
