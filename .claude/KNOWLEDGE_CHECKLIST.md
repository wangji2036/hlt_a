# NU17112 PowerBank Knowledge Checklist

此文件列出平台 Agent 团队需要的学习资料。状态标记：❌ 待提供 / 📋 已提供 / ✅ 已学习

## 1. 芯片与硬件规格

| 资料名称 | 重要性 | 状态 | 放置路径 | 用途 | 请求Agent |
|---------|-------|------|---------|------|---------|
| NU171xx MCU Datasheet | 高 | ❌ | references/datasheets/ | 寄存器定义、电气参数、时序要求 | platform-hal-agent |
| NU6801 Charger IC Datasheet | 高 | ❌ | references/datasheets/ | Buck-boost充电IC寄存器、I2C命令、电气规格 | platform-buckboost-agent |
| NU6805 Charger IC Datasheet | 高 | ❌ | references/datasheets/ | 替代充电IC规格（与NU6801对比） | platform-buckboost-agent |
| NU103x WPC AFE Datasheet | 高 | ❌ | references/datasheets/ | WPC模拟前端寄存器定义、ASK/FSK配置 | platform-wpc-hw-agent |
| FM1210 SE IC Datasheet | 中 | ❌ | references/datasheets/ | Qi认证芯片I2C接口 | platform-wpc-hw-agent |
| T91206 SE IC Datasheet | 中 | ❌ | references/datasheets/ | 替代Qi认证芯片规格 | platform-wpc-hw-agent |
| WB7720 HID IC Datasheet | 低 | ❌ | references/datasheets/ | 电量显示HID设备I2C协议 | platform-fml-agent |

## 2. 协议规范与标准

| 资料名称 | 重要性 | 状态 | 放置路径 | 用途 | 请求Agent |
|---------|-------|------|---------|------|---------|
| Qi 2.0 Wireless Power Specification | 高 | ❌ | references/specs/ | WPC Qi协议合规性、状态机要求、packet定义 | platform-wpc-protocol-agent |
| Qi EPP Specification | 高 | ❌ | references/specs/ | 扩展功率配置协议 | platform-wpc-protocol-agent |
| USB Power Delivery 3.0 Specification | 高 | ❌ | references/specs/ | PD协议合规性、电压协商流程 | platform-usb-agent |
| USB Type-C Cable and Connector Specification | 中 | ❌ | references/specs/ | TypeC物理层、CC通信 | platform-usb-agent |
| USB Battery Charging 1.2 Specification | 中 | ❌ | references/specs/ | BC1.2 DCP/CDP/SDP识别 | platform-dpdm-agent |
| Qualcomm Quick Charge 3.0 Specification | 中 | ❌ | references/specs/ | QC3.0 D+/D-电压编码 | platform-dpdm-agent |
| Samsung AFC Protocol Specification | 中 | ❌ | references/specs/ | AFC快充协议 | platform-dpdm-agent |
| Huawei SCP/FCP Protocol Specification | 中 | ❌ | references/specs/ | SCP/FCP快充协议 | platform-dpdm-agent |
| UFCS Fusion Fast Charging Specification | 低 | ❌ | references/specs/ | 融合快充协议 | platform-dpdm-agent |

## 3. 设计文档与应用笔记

| 资料名称 | 重要性 | 状态 | 放置路径 | 用途 | 请求Agent |
|---------|-------|------|---------|------|---------|
| NU17112 Reference Design Schematic | 高 | ❌ | references/schematics/ | 硬件连接、引脚分配、外围电路 | platform-hal-agent |
| BMS Algorithm Design Document | 中 | ❌ | references/app_notes/ | SOC估算算法原理、参数调优指南 | platform-gauge-agent |
| FOD Tuning Guide | 中 | ❌ | references/app_notes/ | FOD异物检测参数校准方法 | platform-wpc-hw-agent |
| PID Controller Tuning Guide | 中 | ❌ | references/app_notes/ | WPC功率控制PID参数调优 | platform-wpc-hw-agent |
| NTC Temperature Curve Data | 低 | ❌ | references/app_notes/ | NTC查表温度校准 | platform-buckboost-agent |
| Multi-Port Arbitration Strategy | 低 | ❌ | references/app_notes/ | 端口冲突处理策略 | platform-port-manager-agent |

## 4. 测试报告与验证数据

| 资料名称 | 重要性 | 状态 | 放置路径 | 用途 | 请求Agent |
|---------|-------|------|---------|------|---------|
| Qi 2.0 Certification Test Report | 高 | ❌ | references/test_reports/ | 已验证场景、已知问题、性能数据 | platform-wpc-protocol-agent |
| USB PD Compliance Test Report | 高 | ❌ | references/test_reports/ | PD合规性测试结果 | platform-usb-agent |
| FOD Detection Test Data | 中 | ❌ | references/test_reports/ | FOD误报/漏报案例 | platform-wpc-hw-agent |
| Multi-Device Compatibility Matrix | 中 | ❌ | references/test_reports/ | 兼容性测试结果（手机/平板/耳机等） | powerbank-leader |
| Thermal Stress Test Report | 低 | ❌ | references/test_reports/ | 温升测试数据 | platform-buckboost-agent |

## 5. 经验教训与调试指南

| 资料名称 | 重要性 | 状态 | 放置路径 | 用途 | 请求Agent |
|---------|-------|------|---------|------|---------|
| Common Qi Interoperability Issues | 中 | ❌ | references/lessons_learned/ | Qi协议已知互操作性问题 | platform-wpc-protocol-agent |
| PD Negotiation Failure Cases | 中 | ❌ | references/lessons_learned/ | PD协商失败根因分析 | platform-usb-agent |
| DPDM Protocol Detection Edge Cases | 低 | ❌ | references/lessons_learned/ | 快充协议识别边界条件 | platform-dpdm-agent |
| Buck-Boost Oscillation Troubleshooting | 低 | ❌ | references/lessons_learned/ | 电源振荡调试经验 | platform-buckboost-agent |
| Real-Time Debugging Best Practices | 低 | ❌ | references/lessons_learned/ | 实时系统调试技巧 | platform-hal-agent |

---

## 使用说明

1. **提供资料**：将对应资料放入指定路径，状态改为 📋
2. **Agent学习**：Agent Read资料后提取关键信息，状态改为 ✅
3. **新增需求**：Agent发现新的资料需求时，在对应类别新增 ❌ 条目
4. **定期检查**：Leader 定期检查 📋 状态条目，通知对应 Agent 学习

---

**统计**: 总计 31 条资料需求，0 已提供，0 已学习
