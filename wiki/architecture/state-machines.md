# 状态机

**最后更新**: 2026-04-08
**来源文件**: app/_wpc.h, fml/tcpm.h, usbpd/usb_pd.h, fml/dpdm.h, power/buckboost.h, app/port_manager.h, app/sleep.c

---

## 1. WPC Qi TX 状态机 (6 阶段)

**文件**: app/_wpc.h L149-156, app/_wpc.c, app/wpc_*.c

### 主阶段 (`ptx_protocol_phase_t`)

| 阶段 | 值 | 说明 | 源文件 |
|------|---|------|--------|
| WPC_PHASE_IDLE | 0 | 空闲/传输完成 | app/wpc_idle.c |
| WPC_PHASE_PING | 1 | 设备检测 | app/wpc_ping.c |
| WPC_PHASE_CNFG | 2 | 配置 | app/wpc_cnfg.c |
| WPC_PHASE_NEGO | 3 | 协商 | app/wpc_nego.c |
| WPC_PHASE_XFER | 4 | 功率传输 | app/wpc_xfer.c |
| WPC_PHASE_CLOAK | 5 | 隐身 (Qi 2.x) | app/_wpc.c |

### 阶段转换

```
                    ┌─────────────────────────────────┐
                    │                                 │
IDLE ──(Q检测/定时)──▶ PING ──(SIG_01)──▶ CNFG ──(配置完成)──▶ NEGO
  ▲                                                          │
  │                                                     (协商完成)
  │                                                          │
  ├──(异常/EPT)──── XFER ◀──────────────────────────────────┘
  │                  │
  │             (Cloak请求)
  │                  │
  └──(退出)──── CLOAK
```

### IDLE 子状态 (`ptx_idle_phase_state_t`)

| 子状态 | 值 | 含义 |
|--------|---|------|
| WPC_IDLE_STAT_STANDBY | 0 | 待机 |
| WPC_IDLE_STAT_XER_COM | 1 | 传输完成 |
| WPC_IDLE_STAT_XER_FOD | 2 | 传输 FOD |
| WPC_IDLE_STAT_QDT_FOD | 3 | QDT FOD |
| WPC_IDLE_STAT_LAR_MET | 4 | LAR 满足 |
| WPC_IDLE_STAT_EPT_ERR | 5 | EPT 错误 |
| WPC_IDLE_STAT_EPT_RES | 6 | EPT 恢复 |
| WPC_IDLE_STAT_EPT_REP | 7 | EPT 重复 |
| WPC_IDLE_STAT_CLOAKING | 8 | Cloaking |
| WPC_IDLE_STAT_QDT_CALI | 9 | QDT 校准 |

### 功率配置 (`power_profile_mode_t`)

| 模式 | 值 | 功率 |
|------|---|------|
| BPP | 0 | 5W 基线 |
| EPP | 1 | 15W 扩展 |
| MPP | 2 | 磁性功率 |

### XFER 子文件

| 文件 | 内容 |
|------|------|
| wpc_5_xfer_1_bpp.c | BPP 传输 |
| wpc_5_xfer_2_epp.c | EPP 传输 |
| wpc_5_xfer_3_mpp.c | MPP 传输 |

### 入口/出口动作

| 阶段 | 入口动作 | 出口动作 |
|------|---------|---------|
| IDLE | 停止 EPWM1，禁用 ASK，NU103x POR 复位 | — |
| PING | idle_qfod_init() 初始化 FOD 状态机 | — |
| CNFG | 设置 T_NEXT 定时器，清除选项，初始化 auth/pfod | — |
| XFER | PID 控制启动 | wpc_stop_power() |
| CLOAK | cloak_tx_init | cloak_tx_exit |

---

## 2. TypeC 状态机

**文件**: fml/tcpm.h L9-31, lib/typec.c

### 状态 (`usb_tc_state_e`)

| 状态 | 值 | 说明 |
|------|---|------|
| TC_Disable | 0 | 禁用 |
| TC_SNK_Unattached | 1 | Sink 未连接 |
| TC_SNK_AttachWait | 2 | Sink 等待连接 |
| TC_SNK_Attached | 3 | Sink 已连接 |
| TC_SRC_Unattached | 4 | Source 未连接 |
| TC_SRC_AttachWait | 5 | Source 等待连接 |
| TC_SRC_Attached | 6 | Source 已连接 |
| TC_DEBUG_Attached | 7 | Debug 连接 |
| TC_DRP_TOGGLE | 8 | DRP 切换 |
| TC_Try_SNK | 9 | 尝试 Sink |
| TC_TryWAIT_SRC | 10 | 等待 Source |
| TC_Try_SRC | 11 | 尝试 Source |
| TC_TryWAIT_SNK | 12 | 等待 Sink |
| TC_ACCESSORY_Attached | 13 | 附件模式 |
| TC_ErrorRecovery | 14 | 错误恢复 |

### CC 线状态 (`tc_cc_status`)

| 状态 | 值 | 含义 |
|------|---|------|
| TYPEC_CC_OPEN | 0 | 开路 |
| TYPEC_CC_RA | 1 | Ra 电阻 (附件) |
| TYPEC_CC_RD | 2 | Rd 电阻 (设备) |
| TYPEC_CC_RP_DEF | 3 | Rp 默认 (900mA) |
| TYPEC_CC_RP_1_5 | 4 | Rp 1.5A |
| TYPEC_CC_RP_3_0 | 5 | Rp 3.0A |
| TYPEC_CC_TOGGLE | 6 | DRP Toggle |

### 关键转换

| From | 条件 | To | 副作用 |
|------|------|---|--------|
| SRC_Unattached | CC 检测到 Rd | SRC_AttachWait | — |
| SRC_AttachWait | 稳定连接 | SRC_Attached | → PORT_MGR (CONNECT_SUCCESS) |
| SRC_Attached | CC 断开 | SRC_Unattached | → DPDM (SRC_UNATTCHED) |
| SNK_Unattached | CC 检测到 Rp | SNK_AttachWait | — |
| SNK_AttachWait | 稳定连接 | SNK_Attached | → PORT_MGR (CONNECT_SUCCESS) |
| DRP_TOGGLE | 交替 | Try_SNK / Try_SRC | — |

---

## 3. USB PD 策略引擎

**文件**: usbpd/usb_pd.h L42-114

### Sink 状态 (16 个)

| 状态 | 值 | 说明 |
|------|---|------|
| PE_SNK_RSC_Disable | 0 | 起始状态 |
| PE_SNK_Startup | 1 | 启动 |
| PE_SNK_Discovery | 2 | 发现 |
| PE_SNK_Wait_for_Capabilities | 3 | 等待能力 |
| PE_SNK_Evaluate_Capability | 4 | 评估能力 |
| PE_SNK_Select_Capability | 5 | 选择能力 |
| PE_SNK_Transition_Sink | 6 | 过渡 |
| **PE_SNK_Ready** | **7** | **工作状态** |
| PE_SNK_Hard_Reset | 8 | 硬复位 |
| PE_SNK_Transition_to_default | 9 | 恢复默认 |
| PE_SNK_Give_Sink_Cap | 10 | 上报 Sink Cap |
| PE_SNK_Send_Soft_Reset | 11 | 软复位 |
| PE_SNK_Soft_Reset | 12 | 收到软复位 |
| PE_SNK_Not_Supported_Received | 13 | 不支持 |
| PE_SNK_Send_Not_Supported | 14 | 发送不支持 |
| PE_SNK_Give_Sink_Cap_Ext | 15 | 扩展 Sink Cap |

### Source 状态 (18 个)

| 状态 | 值 | 说明 |
|------|---|------|
| PE_SRC_Startup | 16 | 启动 |
| PE_SRC_Discovery | 17 | 发现 |
| PE_SRC_Send_Capabilities | 18 | 发送能力 |
| PE_SRC_Negotiate_Capability | 19 | 协商 |
| PE_SRC_Transition_Supply | 20 | 电压过渡 |
| **PE_SRC_Ready** | **21** | **工作状态** |
| PE_SRC_Disabled | 22 | 禁用 |
| PE_SRC_Hard_Reset | 24 | 硬复位 |
| PE_SRC_Give_PPS_Status | 33 | PPS 状态 |

### 功率角色交换 (PRS) 状态

| 状态 | 值 | 方向 |
|------|---|------|
| PE_PRS_SRC_SNK_Evaluate_Swap | 39 | SRC→SNK 评估 |
| PE_PRS_SRC_SNK_Accept_Swap | 40 | SRC→SNK 接受 |
| PE_PRS_SRC_SNK_Transition_to_off | 41 | SRC→SNK 过渡 |
| PE_PRS_SRC_SNK_Assert_Rd | 42 | SRC→SNK 切换 Rd |
| PE_PRS_SRC_SNK_Wait_Source_on | 43 | SRC→SNK 等待 |
| PE_PRS_SNK_SRC_Evaluate_Swap | 47 | SNK→SRC 评估 |
| PE_PRS_SNK_SRC_Source_on | 51 | SNK→SRC 源打开 |

---

## 4. DPDM 快充协议状态机

**文件**: fml/dpdm.h L5-24

### 主状态 (`dpdm_state_e`)

| 状态 | 值 | 说明 |
|------|---|------|
| DPDM_OFF_STATE | 0 | 关闭 |
| DPDM_DCP_MODE | 1 | DCP (标准充电) |
| DPDM_HVDCP_IDLE_MODE | 2 | HVDCP 空闲 |
| DPDM_HVDCP_QC_MODE | 3 | QC 检测 |
| DPDM_HVDCP_AFC_MODE | 4 | AFC 检测 |
| DPDM_HVDCP_SCP_MODE | 5 | SCP 检测 |
| DPDM_UFCS_MODE | 6 | UFCS 检测 |

### 检测流程

```
SRC_ATTACHED
  → BC1.2 检测
    ├─ SDP → 标准 USB (500mA)
    ├─ CDP → 充电 USB (1.5A)
    ├─ DCP → HVDCP 检测
    │   ├─ QC 模式 → QC2.0/3.0 (5V/9V/12V/20V)
    │   ├─ AFC 模式 → Samsung AFC
    │   ├─ SCP 模式 → Huawei SCP/FCP
    │   └─ UFCS 模式 → 融合快充
    └─ Apple → Apple 充电
```

### QC 电压模式 (`qc_state_e`)

| 状态 | 值 | 电压 |
|------|---|------|
| QC_NOT_MODE | 0 | 未启用 |
| QC_FIX_5V_MODE | 1 | 5V |
| QC_FIX_9V_MODE | 2 | 9V |
| QC_FIX_12V_MODE | 3 | 12V |
| QC_FIX_20V_MODE | 4 | 20V |
| QC_CONTINUOUS_MODE | 5 | 连续可调 |

---

## 5. Buck-Boost 模式状态机

**文件**: power/buckboost.h L15-20

### 模式 (`buckboost_mode`)

| 模式 | 值 | 说明 |
|------|---|------|
| BUCKBOOST_SHUTDOWM_MODE | 0 | 关断 |
| BUCKBOOST_CHAGER_MODE | 1 | 充电 (电池为 Sink) |
| BUCKBOOST_DISCHG_MODE | 2 | 放电 (电池为 Source) |

### 转换条件

| From | 条件 | To |
|------|------|---|
| SHUTDOWN | TypeC SRC 连接 + PD 协商完成 | CHARGER |
| SHUTDOWN | TypeC SNK 连接 | DISCHARGE |
| CHARGER | 拔出 / 电池满 / 保护触发 | SHUTDOWN |
| DISCHARGE | 拔出 / 电池空 / 保护触发 | SHUTDOWN |
| CHARGER | 需要同时放电 | DISCHARGE (先停充再放) |

---

## 6. 端口管理状态

**文件**: app/port_manager.h L36-40, L57-59

### 端口状态 (`port_state_e`)

| 状态 | 值 | 说明 |
|------|---|------|
| PORT_IDLE_OR_READY | 0 | 空闲/就绪 |
| PORT_INHANDLING | 1 | 处理中 |

### 端口角色

| 角色 | 值 | 说明 |
|------|---|------|
| PORT_STATE_NONE | 0x00 | 未连接 |
| PORT_STATE_SOURCE | 0x01 | 作为 USB 电源 (放电) |
| PORT_STATE_SINK | 0x02 | 作为 USB 负载 (充电) |

### 端口索引

| 端口 | 索引 | 类型 |
|------|------|------|
| PORT0 | 0x00 | TypeC-A |
| PORT1 | 0x01 | TypeC-B |
| PORT2 | 0x02 | USB-A |
| PORT3 | 0x03 | WPC (无线) |

---

## 7. 睡眠/唤醒

**文件**: app/sleep.c

### 进入睡眠

函数: `SLP_vNormalToSleep()` (sleep.c L63-102)
- 禁用 WPC
- 降低功耗
- 配置唤醒中断源

### 唤醒条件

| 条件 | 检测方式 |
|------|---------|
| 触摸按键 | PB4_TOUCH_PRESSED (sleep.c L42-43) |
| 无线充电 RX 检测 | Q-factor 变化 |
| USB 连接 | CC 线电平变化 |

---

## 交叉引用

- 任务注册表: [osal-task-registry.md](osal-task-registry.md)
- 消息流: [message-flow.md](message-flow.md)
- 系统概览: [system-overview.md](system-overview.md)
- 保护阈值: [../config/build-flags.md](../config/build-flags.md)
