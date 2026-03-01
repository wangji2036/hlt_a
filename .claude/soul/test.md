# Test Agent 经验

## 已确认的模式 (Confirmed Patterns)

### [P-001] 工程模式联机测试必须考虑 846ms 轮询延迟
- **原则**: NU17112 的 round-robin 调度器每步 47ms，18 步一个完整周期 (846ms)
- **影响**: PC 写入虚拟参数后，NU17112 最慢需要 846ms 才能在 step 14 读取到 0x50 的变化
- **推论**: 测试用例中所有"等待 MCU 响应"的步骤必须至少等待 1 秒（含安全余量），推荐 2 秒
- **来源**: ENGINEERING_MODE_LOGIC.md §2.3 + fml/_fml.c:ubsd_wb7720_report_update()

### [P-002] 异常记录检测周期决定测试等待时间
- **原则**: `battery_record_periodic_check()` 在 APL_TASK 100ms 定时器中调用，异常记录从检测到 I2C 写出需 ~3 秒（EXC_WRITE_INTERVAL=64 × 47ms）
- **影响**: 注入虚拟过压值后，至少等待 5 秒才能在 PC 端看到异常记录
- **推论**: 测试步骤中"验证异常记录"操作需设置 5-10 秒超时

### [P-003] Zero-means-disabled 虚拟参数约定
- **原则**: 虚拟 Cell1/Cell2/Temp 值为 0 表示"使用真实传感器"，非零表示"使用虚拟值"
- **注意**: 无法测试 0.0°C 温度（0 被占用为 disabled），需使用 ±1 (0.1°C / -0.1°C)
- **来源**: app/usb_bridge.c 静态变量 + bat_record.c 检测逻辑

### [P-004] 0xAA 刷新命令的双步骤语义
- **原则**: 0xAA 先触发 exit（step 17），下一次 step 14 重新 enter
- **影响**: 新参数生效需 ~1.7 秒（846ms step17 处理 + 846ms step14 重入）
- **推论**: 连续修改参数时，每次 0xAA 后至少等待 2 秒再验证

### [P-005] 构建质量报告标准格式
- **工具**: `csky-abiv2-elf-size Debug/PowerBankEvk_25W.elf`
- **阈值**: ROM < 95% (122880B), RAM < 90% (8192B)
- **警告**: 编译警告 < 5 个（不含 #if 0 块）

## 已知陷阱 (Known Pitfalls)

### [T-001] WB7720 exc_cache 不被 0xEE 清理
- **现象**: NU17112 执行 `battery_record_erase_all()` 后，Flash 清空，但 WB7720 的 `exc_cache[5][20]` 保留旧记录
- **影响**: PC 可能在擦除后仍看到过期异常记录，直到 WB7720 重启
- **缓解**: PC 端 `_exception_seen_ids.clear()` + `_exception_list.clear()` 可清除显示，但 WB7720 下次 Type 0x02 输出仍含旧数据
- **测试对策**: 擦除测试后，验证 PC 端清除效果即可；完整验证需拔插 USB 重启 WB7720

### [T-002] 虚拟循环次数是 u8 实际是 u16
- **来源**: REG_ENG_CYCLE_CHG_COUNT (0x80-0x81) 是 u16 LE，但 gd->Battery_cycle_count 是 u8
- **影响**: 虚拟循环次数 > 255 会被截断
- **测试对策**: 测试值限制在 0-255 范围内

### [T-003] 工程模式退出时的非原子性
- **现象**: PC 写 0x50=0x00 后，NU17112 需等到下次 step 14 才检测到退出
- **影响**: 最长 846ms 内 NU17112 仍处于工程模式状态
- **测试对策**: 退出操作后等待 1.5 秒再验证状态恢复

## 调试经验 (Debugging Lessons)

### [D-001] 异常记录未出现的排查顺序
1. 检查虚拟值是否超过阈值（OV: >4450mV, OT: >600 即 60.0°C）
2. 检查 0x50 是否已设为 0xA5（工程模式未激活则虚拟值不生效）
3. 检查 0xAA 刷新是否已发送且状态 0x89==0x02
4. 等待时间是否足够（至少 5 秒）
5. 检查 WB7720 exc_cache_count 是否已满（max 5）

## 待验证假设 (Unverified Hypotheses)

- [H-001] UT (under-temperature, error_type=0x03) 在代码中已定义但未实现检测逻辑，需确认是否后续会添加
- [H-002] 工程模式下如果 BuckBoost 正在充电，虚拟过压记录的 sub_type.charge_state 是否正确反映当前状态
