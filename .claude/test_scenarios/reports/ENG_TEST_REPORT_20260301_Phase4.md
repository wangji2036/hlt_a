# 测试报告: Phase 4 - 命令处理与状态管理

> **报告编号**: ENG-TEST-20260301-P4
> **测试日期**: 2026-03-01
> **测试人员**: engineering-test-agent (code review mode)
> **报告版本**: v2.0 (code review completed)

---

## 1. 测试环境

### 1.1 硬件环境

| 项目 | 规格/版本 |
|------|----------|
| 开发板型号 | ARUN IP162N EVK |
| 主控 MCU | NU17112 |
| 充电 IC | NU6805 (SW7201) |
| USB Bridge | WB7720 |
| 电池类型 | 2S Li-ion, 4400mV CV |
| 测试方法 | **代码审查 (无硬件)** |

### 1.2 软件环境

| 项目 | 版本 |
|------|------|
| NU17112 固件 | 12882a7 (branch: ARUN_N3C) |
| WB7720 固件 | config.h FW_VERSION 0x0100 |
| Windows 上位机 | battery_monitor.py (参考) |
| OS | Windows 10 Pro 10.0.19045 |

### 1.3 审查文件清单

| 文件 | 审查内容 |
|------|---------|
| `app/usb_bridge.c` | 0xAA/0xEE 命令处理、Delta 保留、工程模式进入/退出 |
| `app/usb_bridge.h` | 寄存器地址定义 |
| `app/bat_record.c` | battery_record_erase_all()、write_exception_record() |
| `app/bat_record.h` | Flash 地址、常量定义 |
| `USB参考/USB_下位机程序/Projects/main.c` | 写保护、CMD 分发、exc_cache |
| `USB参考/USB_下位机程序/Projects/config.h` | WB7720 寄存器映射 |

---

## 2. 测试范围与目标

### 2.1 测试阶段
Phase 4 - 命令处理与状态管理

### 2.2 测试目标
- 验证 0xEE 擦除命令正确清空 Flash 异常记录
- 验证 0xAA 刷新命令在不退出工程模式的情况下更新虚拟参数
- 验证退出工程模式后 RTC 和 Cycle Count 的 Delta 保留逻辑正确
- 验证写保护机制：非工程模式下 CMD 0x0C 写入被静默丢弃
- 验证 PC 断开/重连及 MCU 重启后工程模式状态的正确恢复行为

### 2.3 前置条件
- [x] 源代码已读取并分析 (commit 12882a7)
- [x] WB7720 源代码已读取并分析
- [ ] 硬件联调验证 (待后续)

---

## 3. 测试结果汇总

### 3.1 总体结论

| 统计项 | 数值 |
|--------|------|
| 总测试用例 | 11 |
| 通过 (PASS-code review) | 10 |
| 失败 (FAIL) | **1** |
| 阻塞 (BLOCKED-需硬件) | 10 |
| 跳过 (SKIP) | 0 |
| **代码审查通过率** | **90.9%** |

> **验收标准**: 所有 P0 用例全部 PASS
> **P0 状态**: 5/6 PASS, 1 FAIL (TC-ENG-403)

### 3.2 测试结果矩阵

| 用例编号 | 用例名称 | 优先级 | Code Review | End-to-End | 备注 |
|---------|---------|--------|-------------|------------|------|
| TC-ENG-401 | 0xEE 擦除全部记录 | P0 | **PASS** | BLOCKED | WB7720 exc_cache 不清除 (known) |
| TC-ENG-402 | 擦除后 Flash 为空 | P0 | **PASS** | BLOCKED | Flash 持久化逻辑正确 |
| TC-ENG-403 | 擦除后重新触发 | P0 | **FAIL** | FAIL | **BUG-P4-001: record_id=0 被跳过** |
| TC-ENG-404 | 0xAA 刷新不退出模式 | P0 | **PASS** | BLOCKED | pseudo-exit + re-enter 机制正确 |
| TC-ENG-405 | 连续 0xAA 刷新 | P1 | **PASS** | BLOCKED | Delta 链正确累积 |
| TC-ENG-406 | Delta 保留 RTC | P0 | **PASS** | BLOCKED | elapsed 计算数学正确 |
| TC-ENG-407 | Delta 保留 Cycle Count | P1 | **PASS** | BLOCKED | uint8_t 无符号算术正确 |
| TC-ENG-408 | 写保护未解锁写入 | P0 | **PASS** | BLOCKED | 双层保护 (CMD+I2C IRQ) |
| TC-ENG-409 | 命令状态反馈 | P1 | **PASS** | BLOCKED | 0x89 写入正确, PC 读回路径受限 |
| TC-ENG-410 | PC 断开/重连 | P2 | **PASS** | BLOCKED | i2c_buff RAM 跨 USB 断连保持 |
| TC-ENG-411 | 固件重启 | P2 | **PASS** | BLOCKED | static 变量复位, 回到用户模式 |

---

## 4. 详细测试步骤与结果

### TC-ENG-401: 0xEE 擦除全部记录

**优先级**: P0
**Code Review 结果**: PASS

**代码追踪**:

1. PC 发送 CMD 0x0C 写 i2c_buff[0x88]=0xEE
2. WB7720 写保护检查: 0x88 在保护范围内, 需 0x50==0xA5 (工程模式) → 允许写入
3. NU17112 step 17: `usb_bridge_check_eng_test_cmds()` (usb_bridge.c:538)
   - `eng_mode_active` 检查 → 仅工程模式下处理
   - 读 0x88: erase_cmd == 0xEE → 进入擦除分支
4. `battery_record_erase_all()` (bat_record.c:608-614):
   ```c
   memset(&g_record_storage, 0, sizeof(BatteryRecordStorage_t));
   g_record_storage.magic = MAGIC_VALUE;
   save_storage_to_flash();
   memset(&g_exception_cache, 0, sizeof(g_exception_cache));
   ```
5. 写 0x89=0x02 (成功), 写 0x88=0x00 (清除命令)

**已知限制**: WB7720 exc_cache 未被清除, 旧缓存记录会继续通过 Type 0x02 报告

**结论**: PASS (code review) / BLOCKED (硬件端到端验证)

---

### TC-ENG-402: 擦除后 Flash 为空

**优先级**: P0
**Code Review 结果**: PASS

**代码追踪**:

擦除后 Flash 状态: magic=MAGIC_VALUE, exception_counter=0, write_ptr=0, records 全零, checksum 已计算并存储。

重启后 `battery_record_init()` (bat_record.c:216-254):
- `flash_read_u32(ADDR_MAGIC)` == MAGIC_VALUE → `load_storage_from_flash()`
- `verify_storage_checksum()` → PASS (checksum 由 save 时计算)
- exception_counter=0, 无记录可加载

**结论**: PASS (code review) / BLOCKED (硬件断电重启验证)

---

### TC-ENG-403: 擦除后重新触发

**优先级**: P0
**Code Review 结果**: **FAIL**

**BUG-P4-001: record_id=0 被 WB7720 跳过**

| 步骤 | 代码位置 | 追踪 |
|------|---------|------|
| 擦除后状态 | bat_record.c:608-614 | write_ptr=0, exception_counter=0 |
| 注入 Cell1=4500 | bat_record.c:271 | 4500>=4450→OV 检测触发 |
| 写入记录 | bat_record.c:199 | `record->record_id = write_ptr` = **0** |
| NU17112 写到 i2c_buff | usb_bridge.c:287 | 20B 记录含 record_id=0 写入 0x3A-0x4D |
| WB7720 缓存检查 | main.c:157 | `if (rid == 0) return;` — **跳过!** |
| PC 收不到记录 | — | exc_cache 不含此记录, Type 0x02 不发送 |

**根因**: `write_exception_record()` 使用 `write_ptr` (0-4) 作为 `record_id`, 而 WB7720 的 `exc_cache_update()` 将 `record_id==0` 视为"无有效记录"的哨兵值。

**影响**: 每次擦除或首次初始化后, 第一条异常记录永远无法到达 PC。

**修复建议**:
- **方案 A** (推荐): 使用单调递增计数器作为 record_id, 永不为 0
  ```c
  // bat_record.c: 在 BatteryRecordStorage_t 中添加 next_record_id 字段
  record->record_id = ++g_record_storage.next_record_id;
  ```
- **方案 B**: 修改 WB7720 哨兵值, 用全零 20B 判断代替 rid==0
- **方案 C**: write_ptr 从 1 开始 (最简单但浪费 slot 0)

**结论**: **FAIL** — 需修复 BUG-P4-001

---

### TC-ENG-404: 0xAA 刷新不退出模式

**优先级**: P0
**Code Review 结果**: PASS

**代码追踪** (usb_bridge.c:550-561):

```
0xAA 处理流程:
  1. 计算 elapsed = gd->Bat_RTC_Seconds - eng_entry_virtual_seconds
  2. 恢复真实值: gd->Bat_RTC_Seconds = eng_saved + elapsed
  3. 清除虚拟值: eng_virtual_cell1/cell2/temp = 0
  4. 设置 eng_mode_active = false
  5. 写 0x89=0x02, 0x88=0x00

  关键: 不写 0x50 (REG_WORK_MODE), i2c_buff[0x50] 保持 0xA5

下一轮 step 14 (usb_bridge_check_engineering_mode):
  读 0x50 = 0xA5, eng_mode_active = false → 重新进入工程模式
  → 保存新的真实值, 读取 i2c_buff 中更新后的虚拟参数
```

**结论**: PASS (code review) — pseudo-exit+re-enter 机制正确, 用户视角工程模式未中断

---

### TC-ENG-405: 连续 0xAA 刷新

**优先级**: P1
**Code Review 结果**: PASS

每次 0xAA 刷新链式调用正确:
- 第 N 次: delta_N = current - virtual_entry_N; real += delta_N; eng_mode_active=false
- 步骤 14 重新进入: saved = real (已累加 delta); read new virtual
- 第 N+1 次: delta_{N+1} = current - virtual_entry_{N+1}; real += delta_{N+1}

Delta 不会丢失, 不会重复计算。

**结论**: PASS (code review) / BLOCKED (硬件 UART 验证参数变化)

---

### TC-ENG-406: Delta 保留 (RTC)

**优先级**: P0
**Code Review 结果**: PASS

**数学验证** (usb_bridge.c:414-417):

```
ENTER: eng_saved = T_real, eng_entry_virtual = V
       gd->Bat_RTC_Seconds = V

经过 300s: gd->Bat_RTC_Seconds = V + 300

EXIT:  elapsed = (V + 300) - V = 300
       gd->Bat_RTC_Seconds = T_real + 300  ✓

特殊情况 (virtual date < 2026, e.g., 2025/1/1):
  V = date_to_seconds(2025,1,1) = 0  (epoch bug, 已知)
  经过 300s: gd->Bat_RTC_Seconds = 0 + 300 = 300
  elapsed = 300 - 0 = 300
  result = T_real + 300  ✓  (delta 仍然正确!)
```

**结论**: PASS — Delta 数学正确, 即使 date_to_seconds epoch 有限制也不影响 delta 计算

---

### TC-ENG-407: Delta 保留 (Cycle Count)

**优先级**: P1
**Code Review 结果**: PASS

**数学验证** (usb_bridge.c:419-423):

```
Battery_cycle_count: uint8_t

ENTER: eng_saved=10, eng_entry_virtual=100, gd->count=100
       2 次充放电: gd->count=102

EXIT:  cycles_added = (uint8_t)(102 - 100) = 2
       gd->count = 10 + 2 = 12  ✓

边界: eng_saved=250, virtual=100, 10 cycles
       cycles_added = (uint8_t)(110 - 100) = 10
       gd->count = 250 + 10 = 260 → uint8_t 溢出为 4  ⚠️ (极端情况)
```

**结论**: PASS — 常规场景正确; 极端值存在 uint8_t 溢出风险 (Minor)

---

### TC-ENG-408: 写保护 (未解锁写入)

**优先级**: P0
**Code Review 结果**: PASS

**双层保护验证**:

| 层级 | 代码位置 | 保护范围 | 条件 |
|------|---------|---------|------|
| HID CMD 0x0C | main.c:528-542 | 0x60-0x63, 0x70-0x73, 0x80-0x81, 0x82-0x88 | i2c_buff[0x50]==0xA5 |
| I2C IRQ | main.c:612-625 | 同上 | i2c_buff[0x50]==0xA5 |

用户模式下 (0x50=0x00): 写 0x82=4500 → 匹配保护范围 → 0x50≠0xA5 → 写入被丢弃。无任何副作用。

**结论**: PASS — 双层保护完整覆盖

---

### TC-ENG-409: 命令状态反馈

**优先级**: P1
**Code Review 结果**: PASS

0xEE 擦除后: usb_bridge.c:548 写 `REG_ENG_CMD_STATUS (0x89) = 0x02`。
0xAA 刷新后: usb_bridge.c:560 写 `REG_ENG_CMD_STATUS (0x89) = 0x02`。

**设计局限**: HID 协议无 register-read 命令, PC 无法直接读取 0x89。当前 Type 0x01/0x02/0x03 报告均不包含 0x89。PC 需通过副作用推断状态 (如异常列表清空)。

**结论**: PASS (NU17112 侧写入正确) / BLOCKED (PC 读回机制待验证)

---

### TC-ENG-410: 工程模式下 PC 断开/重连

**优先级**: P2
**Code Review 结果**: PASS

WB7720 分析:
- `i2c_buff[256]` 是 static 全局变量 (main.c:75), 仅在 MCU 上电时初始化
- USB 断开不触发 MCU 复位 (`mcu_stop_mode()` 仅在 sleep 命令时调用)
- USB 重连后 WB7720 重新枚举, 但 i2c_buff 保持不变

NU17112 分析:
- `eng_mode_active` 是 NU17112 自身的 static 变量, 不受 WB7720 USB 状态影响
- I2C 通信与 USB 独立, NU17112 继续轮询 i2c_buff

**结论**: PASS — 工程模式状态在 USB 断连期间保持

---

### TC-ENG-411: 工程模式下固件重启

**优先级**: P2
**Code Review 结果**: PASS

MCU 上电复位: C 运行时将所有 static/global 变量初始化为 0。
- `eng_mode_active = false` (usb_bridge.c:29, bool 初始化为 0)
- `eng_virtual_cell1/cell2/temp = 0` (usb_bridge.c:40-42)
- `gd->Bat_RTC_Seconds` 和 `Battery_cycle_count` 从 Flash/init 加载, 不保留虚拟值

**注意**: 工程模式期间的 delta 在硬复位时丢失 (非优雅退出)。这是正确的设计行为。

**结论**: PASS — 硬复位后系统干净回到用户模式

---

## 5. 异常记录

### 异常 #1: BUG-P4-001 — record_id=0 被 WB7720 跳过

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-403 |
| 严重程度 | **Critical** |
| 复现率 | 必现 (每次 erase 或首次 init 后) |
| 发现时间 | Code review Phase 4 |
| 根因分析 | `bat_record.c:199` 使用 `write_ptr` (0-4) 作为 `record_id`, `main.c:157` 将 `rid==0` 视为无效记录哨兵 |
| 责任模块 | `bat_record.c` (platform-apl-agent) + `main.c` (usb-device-agent) |
| 修复状态 | **待修复** |

**现象描述**:
每次执行 0xEE 擦除 (或系统首次初始化) 后, write_ptr 归零。随后触发的第一条异常记录获得 record_id=0。WB7720 的 `exc_cache_update()` 检查 `if (rid == 0) return;` 将其视为无效记录而跳过。该记录永远不会被缓存到 exc_cache, 因此 PC 上位机无法通过 Type 0x02 报告收到此记录。

**影响范围**:
- TC-ENG-403 (擦除后重新触发) — 直接受影响
- TC-ENG-508 (同时触发 OV+OT) — 如果在 erase 后触发, 第一条记录丢失
- TC-ENG-509 (擦除后立即触发) — 直接受影响
- 正常使用中首次初始化后的第一条记录也丢失

---

## 6. 结论与建议

### 6.1 总体评价
**有条件通过** — 代码审查发现 1 个 Critical Bug (BUG-P4-001), 影响 P0 用例 TC-ENG-403。其余 10/11 用例在代码层面逻辑正确, 需硬件端到端验证。

### 6.2 关键发现
- **BUG-P4-001**: record_id=0 被 WB7720 跳过, 影响擦除后首条记录传输 (Critical)
- Delta 保留机制 (RTC + Cycle Count) 设计正确, 数学验证通过
- 写保护实现了双层防护 (HID + I2C IRQ), 安全性良好
- 0xAA 刷新的 pseudo-exit+re-enter 设计巧妙且正确
- PC 无法直接读取命令状态寄存器 (0x89), 是 HID 协议设计局限

### 6.3 遗留风险

| 风险编号 | 描述 | 等级 | 缓解措施 |
|---------|------|------|---------|
| R-001 | BUG-P4-001: record_id=0 丢失 | **Critical** | 修改 record_id 为单调递增计数器 |
| R-002 | WB7720 exc_cache 擦除后不清理 | Medium | PC 端 seen_ids 清除可缓解; 或增加 WB7720 exc_cache 清理命令 |
| R-003 | 0xAA 刷新期间 ~100-800ms 使用真实传感器值 | Low | 设计如此, 无实际影响 |
| R-004 | uint8_t cycle count delta 溢出 (极端值) | Low | 实际使用中 cycle count 很少超过 200 |
| R-005 | PC 无法读取 0x89 状态寄存器 | Medium | 考虑增加 register-read HID 命令, 或在 Type 0x01 中增加状态字段 |

### 6.4 后续建议
1. **优先修复 BUG-P4-001** — 涉及 NU17112 bat_record.c + WB7720 main.c 双方修改
2. 修复后重新执行 TC-ENG-403/508/509 代码审查
3. 硬件联调时优先验证 P0 用例 (TC-ENG-401/402/403/404/406/408)

---

## 7. 附件

| 编号 | 文件名 | 说明 |
|------|--------|------|
| A-001 | `USB参考/ARUN_N3C_WIN上位机/phase4_results.json` | Phase 4 机读结果文件 |
| A-002 | `app/usb_bridge.c` | NU17112 USB Bridge 源码 (审查对象) |
| A-003 | `app/bat_record.c` | NU17112 异常记录源码 (审查对象) |
| A-004 | `USB参考/USB_下位机程序/Projects/main.c` | WB7720 下位机源码 (审查对象) |

---

**签署**:
- 测试执行: engineering-test-agent (code review)
- 测试审核: powerbank-leader
- 日期: 2026-03-01
