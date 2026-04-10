# Wiki 会话日志

---

## 2026-04-08 INIT | NU17112 PowerBank (ARUN X20)

- **操作**: 首次 wiki 构建
- **源文件扫描**: ~130 个 .c/.h 文件 (app/ fml/ hal/ power/ gauge/ osal/ util/)
- **Wiki 页面创建**: 15 个页面
  - architecture/ (4): system-overview, osal-task-registry, message-flow, state-machines
  - hal/ (4): hal-overview, i2c, timer-adc, pwm-capture
  - chips/ (4): nu17112, nu6805, wb7720, nu103x
  - config/ (1): build-flags
  - debug/ (2): patterns, tools
- **Task 文档化**: 8 个 OSAL Task 完整记录
- **状态机文档化**: 7 个 (WPC/TypeC/PD/DPDM/BuckBoost/PortMgr/Sleep)
- **事件流映射**: 87 处 osal_set_event() 调用
- **侦察方法**: 4 个并行 Explore Agent (OSAL/HAL/FSM/DataStruct)
- **备注**: gd_t (~1500B) 和 ap_t (~460B) 完整结构体已分析但未单独建页，信息分散在各 wiki 页面的交叉引用中。可按需后续添加 architecture/global-data.md 专页。
