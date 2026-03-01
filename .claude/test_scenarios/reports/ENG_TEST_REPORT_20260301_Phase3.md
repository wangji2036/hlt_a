# 测试报告: Phase 3 — 异常记录全链路

> **报告编号**: ENG-TEST-20260301-Phase3
> **测试日期**: 2026-03-01
> **测试人员**: engineering-test-agent (Leader 调度)
> **测试方法**: 静态代码审查 + 逻辑链路追踪 (无硬件)
> **报告版本**: v2.0

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
| 电池当前电量 | N/A (代码审查模式) |
| 外部供电 | N/A |
| USB 连接 | N/A |
| UART 调试 | N/A |

### 1.2 软件环境

| 项目 | 版本 |
|------|------|
| NU17112 固件 | Git commit `12882a7` (ARUN_N3C branch) |
| WB7720 固件 | HEAD of `USB参考/USB_下位机程序/` |
| Windows 上位机 | battery_monitor.py (latest) |
| 编译工具链 | CDS (未执行构建) |
| Python | N/A (代码审查) |
| OS | Windows 10 Pro 10.0.19045 |

### 1.3 构建质量快照

| 指标 | 值 | 阈值 | 状态 |
|------|---|------|------|
| ROM 使用 | N/A (未构建) | < 95% | N/A |
| RAM 使用 | N/A (未构建) | < 90% | N/A |
| 编译警告 | N/A | < 5 | N/A |

---

## 2. 测试范围与目标

### 2.1 测试阶段
Phase 3 — 异常记录全链路 (TC-ENG-301 ~ TC-ENG-314)

### 2.2 测试目标
- 验证异常记录从触发到 PC 显示的完整链路正确性
- 验证过压 (OV) 和过温 (OT) 阈值检测逻辑
- 验证 Flash 持久化、多条记录管理、滚动覆盖
- 验证 record_id 去重机制和 Type 0x02 HID 分页传输
- 验证边界阈值行为

### 2.3 测试方法说明
本次采用 **代码审查 (Static Code Analysis)** 方式执行:
- 逐用例追踪代码逻辑链 (NU17112 → WB7720 → PC)
- 标记需要硬件验证的用例为 BLOCKED
- 代码逻辑可直接判定的用例标记 PASS 或 FAIL

### 2.4 前置条件
- [x] 已读取全部相关源代码 (usb_bridge.c/h, bat_record.c/h, config.h, g_data.h, WB7720 main.c/config.h, battery_monitor.py)
- [x] 已理解三方数据流架构
- [ ] 硬件设备未连接 (代码审查模式)

---

## 3. 测试结果汇总

### 3.1 总体结论

| 统计项 | 数值 |
|--------|------|
| 总测试用例 | 14 |
| 通过 (PASS) | 3 |
| 失败 (FAIL) | 2 |
| 阻塞 (BLOCKED) | 9 |
| 跳过 (SKIP) | 0 |
| **通过率 (含 BLOCKED)** | **21.4%** |
| **通过率 (排除 BLOCKED)** | **60.0%** (3/5) |
| **代码审查逻辑正确率** | **85.7%** (12/14) |

### 3.2 测试结果矩阵

| 用例编号 | 用例名称 | 优先级 | 结果 | 代码审查 | 备注 |
|---------|---------|--------|------|---------|------|
| TC-ENG-301 | 过压异常触发 (Cell1) | P0 | BLOCKED | PASS | 逻辑链完整，需硬件端到端验证 |
| TC-ENG-302 | 过压异常触发 (Cell2) | P0 | BLOCKED | PASS | sub_type=2 正确 |
| TC-ENG-303 | 过温异常触发 | P0 | BLOCKED | PASS | 温度用 > (strict)，正确 |
| TC-ENG-304 | 异常记录时间戳正确 | P0 | **FAIL** | FAIL | BUG: date_to_seconds 不支持 <2026 |
| TC-ENG-305 | 异常记录 Flash 持久化 | P0 | BLOCKED | PASS | 需断电重启验证 |
| TC-ENG-306 | 多条异常记录 | P1 | BLOCKED | PASS | record_id 递增正确 |
| TC-ENG-307 | 5 条记录滚动写入 | P1 | BLOCKED | PASS | MAX_RECORDS=5 逻辑正确 |
| TC-ENG-308 | 超过 5 条记录覆盖 | P2 | BLOCKED | PASS* | Flash 覆盖正确，exc_cache 不刷新(已知限制) |
| TC-ENG-309 | record_id 去重验证 | P1 | BLOCKED | PASS | 双层去重正确 (WB7720 + PC) |
| TC-ENG-310 | Type 0x02 分页验证 | P1 | BLOCKED | PASS | 3 页循环 (2,2,1) 正确 |
| TC-ENG-311 | 边界阈值 (Cell1=4450) | P2 | **FAIL** | FAIL | BUG: >= 应为 > |
| TC-ENG-312 | 边界阈值 (Cell1=4451) | P2 | PASS | PASS | 4451 >= 4450 触发正确 |
| TC-ENG-313 | 边界阈值 (Temp=600) | P2 | PASS | PASS | 600 > 600 = FALSE，不触发 |
| TC-ENG-314 | 边界阈值 (Temp=601) | P2 | PASS | PASS | 601 > 600 = TRUE，触发正确 |

**按优先级统计**:

| 优先级 | 总数 | PASS | FAIL | BLOCKED |
|--------|------|------|------|---------|
| P0 | 5 | 0 | 1 | 4 |
| P1 | 4 | 0 | 0 | 4 |
| P2 | 5 | 3 | 1 | 1 |
| 合计 | 14 | 3 | 2 | 9 |

---

## 4. 详细测试步骤与结果

### TC-ENG-301: 过压异常触发 (Cell1)

**优先级**: P0
**前置条件**: 进入工程模式，虚拟参数已就绪
**预期结果**: NU17112 创建 OV 记录 (error_type=0x01, sub_type=0x01), PC 异常列表显示

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 进入工程模式 | 0x50=0xA5, eng_mode_active=true | usb_bridge.c:335→404: 状态转换正确 | PASS |
| 2 | 设 Cell1=4500 | eng_virtual_cell1=4500 | usb_bridge.c:399: 从 I2C 0x82-0x83 读取 | PASS |
| 3 | 写入+等待 10s | OV 检测触发 | bat_record.c:271: 4500>=4450→TRUE | PASS |
| 4 | 记录创建 | error_type=0x01, sub_type=0x01 | bat_record.c:292-296: 字段赋值正确 | PASS |
| 5 | Flash 写入 | save_storage_to_flash() | bat_record.c:210: 调用链完整 | PASS |
| 6 | I2C 传输 | i2c_buff[0x37-0x4D] | usb_bridge.c:281-293: 23B payload 正确 | PASS |
| 7 | WB7720 缓存 | exc_cache 新增记录 | WB7720 main.c:151-169: dedup+缓存正确 | PASS |
| 8 | PC 显示 | 异常列表新增 | battery_monitor.py:1619-1624: record_id 过滤 | PASS |

**结论**: BLOCKED — 代码审查全链路 PASS，需硬件验证端到端

---

### TC-ENG-302: 过压异常触发 (Cell2)

**优先级**: P0
**预期结果**: OV 记录 sub_type=0x02

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 设 Cell2=4460, Cell1=0 | eng_c2=4460, eng_c1=0 | bat_record.c:394: eng_c2>0 进入虚拟模式 | PASS |
| 2 | Cell2 电压赋值 | cell2_voltage=4460 | bat_record.c:398: (eng_c2>0)→4460 | PASS |
| 3 | Cell1 使用真实值/2 | cell1_voltage=real_total/2 | bat_record.c:397: (eng_c1==0)→real/2 | PASS |
| 4 | OV 触发 | 4460>=4450→TRUE | bat_record.c:271 | PASS |
| 5 | 记录 sub_type | sub_type=2 (cell_num) | bat_record.c:293 | PASS |

**结论**: BLOCKED — 代码审查 PASS

---

### TC-ENG-303: 过温异常触发

**优先级**: P0
**预期结果**: OT 记录 (error_type=0x02)

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 设 Temp=650 | eng_virtual_temp=650 | usb_bridge.c:401: I2C 0x86-0x87 读取 | PASS |
| 2 | 温度覆盖 | ntc_temp=650 | bat_record.c:437: eng_temp!=0→使用虚拟值 | PASS |
| 3 | OT 判断 | 650>600→TRUE | bat_record.c:446: strict > 运算符 | PASS |
| 4 | 记录创建 | error_type=0x02 | bat_record.c:474: EXCEPTION_TYPE_OVERTEMP | PASS |

**结论**: BLOCKED — 代码审查 PASS

---

### TC-ENG-304: 异常记录时间戳正确

**优先级**: P0
**预期结果**: 记录时间戳为 2025/07/20 HH:MM:SS

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 设虚拟日期 2025/07/20 | gd->Bat_RTC_Seconds = f(2025,7,20) | usb_bridge.c:55: year(2025)<2026→**返回 0** | **FAIL** |
| 2 | 触发 OV 记录 | 时间戳=2025/07/20 | seconds_to_timestamp(0)→**2026-01-01 00:00:00** | **FAIL** |

**BUG 代码**:
```c
// usb_bridge.c:53-57
static uint32_t date_to_seconds(uint16_t year, uint8_t month, uint8_t day) {
    if (year < 2026) {
        return 0;   // ← BUG: 2025 年日期被映射为 epoch 0
    }
    // ...
}
```

**结论**: **FAIL** — BUG-P3-001: `date_to_seconds()` epoch=2026, 不支持更早日期

---

### TC-ENG-305: 异常记录 Flash 持久化

**优先级**: P0

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | write_exception_record() | save_storage_to_flash() | bat_record.c:210 ✓ | PASS |
| 2 | save_storage_to_flash() | erase + write Flash page | bat_record.c:175-183 ✓ | PASS |
| 3 | 重启后 init() | magic → load → checksum | bat_record.c:217-236 ✓ | PASS |
| 4 | 断电验证 | 记录存在 | 需硬件 | BLOCKED |

**结论**: BLOCKED — 代码逻辑 PASS，需硬件断电重启

---

### TC-ENG-306: 多条异常记录

**优先级**: P1

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 触发 OV(Cell1) | record_id=0 | bat_record.c:199: record_id=write_ptr=0 | PASS |
| 2 | 触发 OV(Cell2) | record_id=1 | write_ptr→1 | PASS |
| 3 | 触发 OT | record_id=2 | write_ptr→2 | PASS |

**注意**: 需先让 Cell1 恢复 (cell_over→false while tracking) 以清除 tracking 状态后才能为 Cell2 创建独立新记录。

**结论**: BLOCKED — 代码逻辑 PASS

---

### TC-ENG-307: 5 条记录滚动写入

**优先级**: P1

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 5 条写入 | write_ptr: 0→4 | bat_record.c:194-206 ✓ | PASS |
| 2 | counter | exception_counter=5 | bat_record.c:205-207 ✓ | PASS |
| 3 | WB7720 缓存 | exc_cache_count=5 | WB7720 main.c:160 ✓ | PASS |

**结论**: BLOCKED — 代码逻辑 PASS

---

### TC-ENG-308: 超过 5 条记录覆盖

**优先级**: P2

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | 第 6 条写入 | write_ptr wraps 4→0 | bat_record.c:202: (4+1)%5=0 ✓ | PASS |
| 2 | Flash 覆盖 | records[0] 覆盖 | ✓ | PASS |
| 3 | WB7720 缓存 | 新记录入缓存 | **已满→拒绝** | LIMITATION |

**已知限制**: WB7720 `exc_cache_count >= EXC_CACHE_MAX(5)` → 不再追加新记录。

**结论**: BLOCKED — Flash 逻辑 PASS，WB7720 缓存已知限制

---

### TC-ENG-309: record_id 去重验证

**优先级**: P1

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | WB7720 去重 | record_id 匹配 | main.c:163-164 ✓ | PASS |
| 2 | PC 去重 | _exception_seen_ids | battery_monitor.py:1622-1623 ✓ | PASS |

**结论**: BLOCKED — 双层去重逻辑 PASS

---

### TC-ENG-310: Type 0x02 分页验证

**优先级**: P1

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | page_start | 0, 2, 4 | main.c:281 ✓ | PASS |
| 2 | return_count | 2, 2, 1 | main.c:283-285 ✓ | PASS |
| 3 | 页码循环 | 0→1→2→0 | main.c:320-321 ✓ | PASS |

**结论**: BLOCKED — 分页逻辑 PASS

---

### TC-ENG-311: 边界阈值 (Cell1=4450)

**优先级**: P2
**预期结果**: 不触发 OV (需 > 4450)

| 步骤 | 操作 | 预期 | 实际 (代码分析) | 结果 |
|------|------|------|------|------|
| 1 | Cell1=4450 | 不触发 | `4450 >= 4450` → **TRUE → 触发 OV** | **FAIL** |

**BUG 代码**:
```c
// bat_record.c:271
bool cell_over = (cell_voltage >= OVER_VOLTAGE_THRESHOLD);
//                               ^^ 设计意图为 > (strict)
```

**对比**: 温度检测 bat_record.c:446 使用 `>` (strict)，OV 使用 `>=`。不一致。

**结论**: **FAIL** — BUG-P3-002

---

### TC-ENG-312: 边界阈值 (Cell1=4451)

**优先级**: P2 | **结论**: **PASS**

`4451 >= 4450` → TRUE → 触发 OV。无论 `>=` 或 `>` 均触发。

---

### TC-ENG-313: 边界阈值 (Temp=600)

**优先级**: P2 | **结论**: **PASS**

`600 > 600` → FALSE → 不触发 OT。温度使用 strict `>`，边界行为正确。

---

### TC-ENG-314: 边界阈值 (Temp=601)

**优先级**: P2 | **结论**: **PASS**

`601 > 600` → TRUE → 触发 OT。正确。

---

## 5. 异常记录

### 异常 #1: date_to_seconds 不支持 2026 年之前日期 (BUG-P3-001)

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-304 |
| 严重程度 | Major |
| 复现率 | 必现 |
| 发现时间 | 代码审查阶段 |
| 根因分析 | date_to_seconds() epoch=2026-01-01, year<2026 返回 0 |
| 责任模块 | app/usb_bridge.c:55 (platform-apl-agent) |
| 修复状态 | 待修复 |

**现象描述**:
虚拟日期设为 2025/07/20 时，`date_to_seconds(2025, 7, 20)` 判断 `year < 2026` 直接返回 0。异常记录时间戳显示为 2026-01-01 00:00:00 而非预期的 2025/07/20。

**修复方案**:
- 方案 A: 将 epoch 改为 2020 年，扩展支持范围
- 方案 B: 将测试用例日期改为 >= 2026/01/01

---

### 异常 #2: OV 阈值比较使用 >= 而非 > (BUG-P3-002)

| 字段 | 内容 |
|------|------|
| 关联用例 | TC-ENG-311 |
| 严重程度 | Minor |
| 复现率 | 必现 |
| 发现时间 | 代码审查阶段 |
| 根因分析 | OV 用 `>=`，OT 用 `>`，运算符不一致 |
| 责任模块 | app/bat_record.c:271 (platform-apl-agent) |
| 修复状态 | 待修复 |

**现象描述**:
Cell1=4450mV (恰好等于 `OVER_VOLTAGE_THRESHOLD`) 时触发 OV 记录。设计意图应为 `>` (严格大于)，与温度检测一致。

**修复方案**:
`bat_record.c:271`: `>=` → `>`

---

## 6. 结论与建议

### 6.1 总体评价
**有条件通过** — 代码逻辑审查覆盖 14 个用例，12/14 逻辑正确 (85.7%)。发现 2 个 Bug (1 Major + 1 Minor)，9 个用例需硬件验证。

### 6.2 关键发现
- **BUG-P3-001 (Major)**: `date_to_seconds()` 不支持 2026 年之前日期
- **BUG-P3-002 (Minor)**: OV 阈值 `>=` vs OT 阈值 `>` 不一致
- **异常记录全链路逻辑正确**: OV/OT 检测 → Flash → I2C 轮转 → WB7720 缓存 → HID 分页 → PC 去重
- **WB7720 exc_cache**: 满后 (5 条) 不再接受新记录，需 USB 重连刷新

### 6.3 遗留风险

| 风险编号 | 描述 | 等级 | 缓解措施 |
|---------|------|------|---------|
| R-P3-001 | record_id=write_ptr(0-4)，覆盖场景 ID 重复 | Medium | PC erase 时 clear seen_ids |
| R-P3-002 | WB7720 exc_cache 满后不接受新记录 | Medium | USB 重连刷新，或增加淘汰机制 |
| R-P3-003 | 9 个用例未经硬件验证 | High | 安排硬件测试 |

### 6.4 后续建议
1. **高优**: 修复 BUG-P3-001 或调整测试用例日期
2. **中优**: 修复 BUG-P3-002 (`>=` → `>`)
3. **高优**: 安排硬件测试完成 9 个 BLOCKED 用例
4. 考虑 record_id 改为全局递增计数器

---

## 7. 附件

| 编号 | 文件名 | 说明 |
|------|--------|------|
| A-001 | `USB参考/ARUN_N3C_WIN上位机/phase3_results.json` | Phase 3 机器可读结果 |
| A-002 | `app/usb_bridge.c:53-57` | BUG-P3-001 位置 |
| A-003 | `app/bat_record.c:271` | BUG-P3-002 位置 |

---

**签署**:
- 测试执行: engineering-test-agent
- 测试审核: powerbank-leader
- 日期: 2026-03-01
