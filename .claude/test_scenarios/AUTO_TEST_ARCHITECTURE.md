# 工程模式自动测试架构 v1.0

> 创建: 2026-03-01 | 作者: powerbank-leader

## 1. 设计原则

| 原则 | 实现 |
|------|------|
| **通信层复用** | 从 `battery_monitor.py` 提取 HID 协议函数，headless runner 直接复用 |
| **GUI 解耦** | `phase1_test.py` 零 GUI 依赖，仅 import `hid` + stdlib |
| **用例驱动** | 每个 `tc_eng_xxx()` 函数是独立用例，可选择性执行 |
| **断言与超时** | `TestResult.add_step()` 记录 expected/actual，`read_telemetry_once()` 内置超时 |
| **报告自动化** | JSON (机读) + Markdown (人读) 双格式输出 |

## 2. 系统架构

```
┌──────────────────────────────────────────────────┐
│                   Test Runner                     │
│  phase1_test.py (CLI, headless)                  │
│                                                   │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐          │
│  │TC-ENG-  │  │TC-ENG-  │  │TC-ENG-  │  ...     │
│  │  101    │  │  102    │  │  103    │          │
│  └────┬────┘  └────┬────┘  └────┬────┘          │
│       │             │            │                │
│  ┌────┴─────────────┴────────────┴────┐          │
│  │        Protocol Layer              │          │
│  │  parse_hid_report()                │          │
│  │  parse_telemetry()                 │          │
│  │  write_reg() / hid_write/read()    │          │
│  └────────────────┬───────────────────┘          │
│                   │                               │
│  ┌────────────────┴───────────────────┐          │
│  │        HID Transport               │          │
│  │  find_and_open_device()            │          │
│  │  hid.device (hidapi)               │          │
│  └────────────────┬───────────────────┘          │
│                   │                               │
│  ┌────────────────┴───────────────────┐          │
│  │        Report Generator            │          │
│  │  JSON  → phase1_results.json       │          │
│  │  MD    → reports/ENG_TEST_REPORT_*.md│         │
│  └────────────────────────────────────┘          │
└──────────────────────────────────────────────────┘
                    │ USB HID
                    ▼
           ┌────────────────┐
           │   WB7720 Bridge │  I2C @0x42
           └───────┬────────┘
                   ▼
           ┌────────────────┐
           │    NU17112 MCU  │
           └────────────────┘
```

## 3. 用例覆盖矩阵

| Phase | 用例 ID | 名称 | 优先级 | 自动化状态 | 需硬件 |
|-------|---------|------|--------|-----------|--------|
| 1 | TC-ENG-101 | USB 连接验证 | P0 | **已实现** | 是 |
| 1 | TC-ENG-102 | 遥测数据正确性 | P0 | **已实现** | 是 |
| 1 | TC-ENG-103 | 工程模式进入 | P0 | **已实现** | 是 |
| 1 | TC-ENG-104 | 工程模式退出 | P0 | **已实现** | 是 |
| 1 | TC-ENG-105 | 重复进入/退出 3x | P1 | **已实现** | 是 |
| 1 | TC-ENG-106 | 遥测持续性 (30s) | P1 | **已实现** | 是 |
| 2 | TC-ENG-201+ | 虚拟参数注入 | P0 | 待实现 | 是 |
| 3 | TC-ENG-301+ | 异常记录全链路 | P0 | 待实现 | 是 |

## 4. 关键接口

### 4.1 HID Transport

```python
# 设备发现 (VID=0xFFFF, PID=0xFFFF, UsagePage=0xFF00)
dev, info = find_and_open_device()

# 写命令 (65B: [0x00][CMD][payload...])
hid_write(dev, CMD_WRITE_REGISTER, payload)

# 读报告 (64B, 带超时)
raw = hid_read(dev, timeout_ms=1000)

# 寄存器写入 (自动封装 CMD 0x0C)
write_reg(dev, reg_addr, data_bytes)
```

### 4.2 Protocol Parsing

```python
# 解析 HID 报告头 (SOF 0x05/0xA5/0x5A + Type + Seq + Len)
report_type, payload = parse_hid_report(raw_64bytes)

# 解析遥测 (Type 0x01, 35+ bytes payload)
bd = parse_telemetry(payload)
# bd = {soc, total_voltage, cell1_voltage, cell2_voltage, bat_temp, cycle_count, ...}

# 组合: 发请求 + 等回复 + 解析
bd, raw = read_telemetry_once(dev, timeout_s=3.0)
```

### 4.3 Assertion Framework

```python
# TestResult 记录每个 step 的 expected vs actual
r = TestResult("TC-ENG-101", "USB 连接验证", "P0")
r.add_step(description, expected, actual, passed: bool)
r.log("带时间戳的日志")
r.set_pass() / r.set_blocked(reason)
```

## 5. 运行方式

### 依赖安装
```bash
pip install hidapi
```

### 执行命令
```bash
# 运行全部 Phase 1 用例
cd USB参考/ARUN_N3C_WIN上位机/
python phase1_test.py

# 仅运行指定用例 (v1.1 新增)
python phase1_test.py --cases 101,102

# 生成 Markdown 报告 (v1.1 新增)
python phase1_test.py --report-md
```

### 前置条件
1. WB7720 USB Bridge 已通过 USB 连接到 PC
2. NU17112 已上电并运行工程模式固件
3. 无其他程序占用 HID 设备 (关闭 battery_monitor.py)
4. Python 3.8+ 已安装

### 预期输出 (设备连接时)
```
======================================================================
  Phase 1: 连通性与基础功能 — 自动化测试
  开始时间: 2026-03-01 14:00:00
======================================================================

==================================================
[TC-ENG-101] USB 连接验证
==================================================
  [14:00:00.100] 开始 USB 连接验证
  [14:00:00.150] 设备: WB7720 BatteryMonitor path=...
  [14:00:01.200] 遥测: SOC=85% Vtotal=8.35V Cell1=4.18V Cell2=4.17V Temp=25.3°C
  --> PASS

==================================================
[TC-ENG-102] 遥测数据正确性
==================================================
  [14:00:02.300] 开始遥测数据正确性验证 (连续 3 次采样)
  ...
  --> PASS

======================================================================
  测试结果汇总
======================================================================
  [OK] TC-ENG-101 (P0) USB 连接验证
  [OK] TC-ENG-102 (P0) 遥测数据正确性
  ...
  总计: 6 | PASS: 6 | FAIL: 0 | BLOCKED: 0
  通过率: 100.0%
======================================================================
```

### 预期输出 (无设备时)
```
  [NG] TC-ENG-101 (P0) USB 连接验证
  [BK] TC-ENG-102~106 — 依赖 TC-ENG-101 (设备连接)
  通过率: 0.0%
```

## 6. 报告规范

### JSON 输出 (机读)
- 位置: `USB参考/ARUN_N3C_WIN上位机/phase1_results.json`
- 每次运行覆盖

### Markdown 输出 (人读)
- 位置: `.claude/test_scenarios/reports/ENG_TEST_REPORT_{YYYYMMDD}_Phase1.md`
- 符合 TEST_REPORT_TEMPLATE.md 7 章节结构
- 自动填充：环境信息、结果汇总、每个用例的详细步骤和日志

## 7. 扩展路线 (Phase 2+)

| 阶段 | 新增能力 | 复用层 |
|------|---------|--------|
| Phase 2 | 虚拟参数注入 (OV/OT trigger) | write_reg() + 0x82/0x84/0x86 |
| Phase 3 | 异常记录全链路 (Flash→HID→PC) | parse_exception + Type 0x02 |
| Phase 4 | 0xAA/0xEE 命令处理 | write_reg(0x88) + CMD_STATUS poll |
| Phase 5 | 压力测试 (连续 100 轮) | 现有框架 + 循环计数 |

每个 Phase 新增 `tc_eng_Nxx()` 函数，注册到 main() 即可。无需修改框架层。

## 8. 已知限制与修复清单

| # | 问题 | 影响 | 修复方案 | 优先级 |
|---|------|------|---------|--------|
| L1 | 需真实硬件才能运行 | 无法在 CI 中执行 | 接受 (嵌入式测试本质限制) | — |
| L2 | 无 Markdown 报告输出 | 报告需手工填写 | **v1.1: 添加 `--report-md` 选项** | HIGH |
| L3 | 无用例选择 CLI | 每次运行全部用例 | **v1.1: 添加 `--cases` 选项** | MEDIUM |
| L4 | 温度解析兼容逻辑 | raw<100 按°C, ≥100 按÷10 | 已在代码中处理，无需修复 | — |
| L5 | VID/PID=0xFFFF | 开发阶段 placeholder | 量产前需更新 | LOW |
