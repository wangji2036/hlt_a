# 任务间消息与事件流

**最后更新**: 2026-04-08
**来源文件**: 全局 osal_set_event() 调用 (87 处), osal/osal.h, 各 task 源文件

---

## 事件矩阵总览

全系统共 **87 处** `osal_set_event()` 调用，涉及 6 个 Task (HAL_TASK 无事件)。

```
                   ┌──────────┐
       ASK/FSK ISR │ FML_TASK │ ← lib/ask.c (FML_EVT_ASK_INT_RECVD)
                   └────┬─────┘
                        │ 定时器驱动
        ┌───────────────┼───────────────┐
        ▼               ▼               ▼
  ┌──────────┐   ┌───────────┐   ┌────────────┐
  │ WPC_TASK │   │ USB_TASK  │   │ BUCKBOOST  │
  │ (Qi TX)  │   │ (TypeC/PD)│   │ _TASK      │
  └──┬───┬───┘   └─┬────┬────┘   └──────┬─────┘
     │   │         │    │               │
     │   │    ┌────┘    │    ┌──────────┘
     │   │    ▼         ▼    ▼
     │   │  ┌──────────────────┐
     │   └──│ PORT_MANAGER_TASK│ ← 端口枚举/仲裁中枢
     │      └────────┬─────────┘
     │               │
     │               ▼
     │      ┌──────────────────┐
     └─────▶│ USB_DPDM_TASK    │ ← 快充协议检测
            └──────────────────┘
```

---

## 按目标 Task 分类的事件

### WPC_TASK ← (7 个事件源)

| 事件 | 发送者 | 源文件:行 | 触发条件 |
|------|--------|----------|---------|
| WPC_EVT_HDR_START | ISR | lib/ask.c:499 | ASK 包头开始接收 |
| WPC_EVT_HDR_RECVD | ISR | lib/ask.c:349 | ASK 包头接收完成 |
| WPC_EVT_PKT_RECVD | ISR | lib/ask.c:401 | ASK 完整数据包接收 |
| WPC_EVT_DM_CRITICAL | ISR | lib/ask.c:925 | ASK 中断 + 临界 DM |
| WPC_EVT_FSK_RESP_DONE | ISR | fml/fsk.c:301 | FSK 响应发送完成 |
| WPC_EVT_STOP_POWER | WPC 自身 | app/_wpc.c:88 | 停止功率传输命令 |
| WPC_EVT_PFOD | WPC/EPP | app/epp.c:788 | EPP 功率反馈 FOD |
| WPC_EVT_FOD_REPORTED | WPC 自身 | app/_wpc.c:504 | FOD 已上报事件 |

### USB_TASK ← (14 个事件源)

| 事件 | 发送者 | 源文件:行 | 触发条件 |
|------|--------|----------|---------|
| TCPM_EVT_QI_WORK | WPC | app/wpc_ping.c:24 | Ping 检测到 SIG_01 |
| TCPM_EVT_QI_SET_VOLT | FML | fml/adp.c:100 | 适配器电压设置请求 |
| TCPM_EVT_DPDM_DONE | DPDM | fml/dpdm.c:321,325,355,375 | DPDM 协议检测完成 |
| TCPM_EVT_USBA_REDETECT | PORT_MGR/BB | port_manager.c + buckboost.c | USB-A 重检测 |
| TCPM_EVT_USBA_SCAN | BUCKBOOST | power/buckboost.c:641 | USB-A 扫描请求 |

### PORT_MANAGER_TASK ← (26 个事件源)

| 事件 | 发送者 | 源文件 | 触发条件 |
|------|--------|--------|---------|
| PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS | TypeC | lib/typec.c:238,399 | Port0 TypeC 连接成功 |
| PORT_ENUM_EVT_PORT1_CONNECT_SUCCESS | TypeC | lib/typec.c:240,401 | Port1 TypeC 连接成功 |
| PORT_ENUM_EVT_PORT0_SINK_SETVOLT | 自身/多处 | port_manager.c (5处) | Port0 Sink 电压设置 |
| PORT_ENUM_EVT_PORT1_SINK_SETVOLT | 自身/多处 | port_manager.c (5处) | Port1 Sink 电压设置 |
| PORT_ENUM_EVT_PORT0_ENUM_DONE | 自身 | port_manager.c:1027 | Port0 枚举完成 |
| PORT_ENUM_EVT_PORT1_ENUM_DONE | 自身 | port_manager.c:1111 | Port1 枚举完成 |
| PORT_ENUM_EVT_PORT*_CONNECT_CLOSED | 自身 | port_manager.c:1372-1393 | 端口断开 |
| PORT_ENUM_EVT_PORT*_CONNECT_START | 自身 | port_manager.c:1404-1429 | 端口连接启动 |

### USB_DPDM_TASK ← (25 个事件源)

| 事件 | 发送者 | 源文件 | 触发条件 |
|------|--------|--------|---------|
| DPDM_EVT_SRC_ATTACHED | PORT_MGR | port_manager.c:664 | 充电源已连接 |
| DPDM_EVT_SRC_UNATTCHED | TypeC | lib/typec.c:277,413,426,448 | 充电源断开 |
| DPDM_EVT_SNK_ATTACHED | PORT_MGR | port_manager.c:1019,1046,1103,1131 | Sink 设备已连接 |
| DPDM_EVT_ENTER_DCP | DPDM 自身 | fml/dpdm.c:406 | 进入 DCP 模式 |
| DPDM_EVT_ENTER_HVDCP | DPDM 自身 | fml/dpdm.c:412 | 进入 HVDCP 模式 |
| DPDM_EVT_QC_FIXED_5V~20V | DPDM 自身 | fml/dpdm.c:433-451 | QC 电压请求 |
| DPDM_EVT_QC_CONTINUES | DPDM 自身 | fml/dpdm.c:457 | QC 连续模式 |
| DPDM_EVT_QC_PLUSE_INC/DEC | DPDM 自身 | fml/dpdm.c:465,473 | QC 脉冲调压 |
| DPDM_EVT_AFC_RX_DATA | ISR | fml/dpdm.c:492 | AFC 数据接收 |
| DPDM_EVT_SCP_RX_DATA | ISR | fml/dpdm.c:499 | SCP 数据接收 |
| DPDM_EVT_SCP_TX_DATA | SCP lib | lib/afc_scp.c (8处) | SCP 发送就绪 |
| DPDM_EVT_AFC_SCP_OUT | SCP lib | lib/afc_scp.c (7处) | AFC/SCP 超时退出 |
| DPDM_EVT_SNK_BC12DONE | QC lib | fml/usb_qc.c:72 | BC1.2 检测完成 |
| DPDM_EVT_SNK_HVDCP_DONE/FAIL | QC lib | fml/usb_qc.c:78,84 | HVDCP 检测结果 |
| DPDM_EVT_UFCS_RX_PACKET | UFCS lib | lib/ufcs.c:346 | UFCS 数据包接收 |
| DPDM_EVT_UFCS_RX_RESET | UFCS lib | lib/ufcs.c:352 | UFCS 复位接收 |

### FML_TASK ← (1 个事件源)

| 事件 | 发送者 | 源文件:行 | 触发条件 |
|------|--------|----------|---------|
| FML_EVT_ASK_INT_RECVD | ISR | lib/ask.c:886 | ASK 中断接收 |

### BUCKBOOST_TASK ← (1 个事件源)

| 事件 | 发送者 | 源文件:行 | 触发条件 |
|------|--------|----------|---------|
| BUCKBOOST_EVT_SET_DISCHG_VBUS_VOLT | 自身 | power/buckboost.c:115 | 设置放电 VBUS 电压 |

---

## 关键数据路径

### 1. 有线充电连接流程

```
[TypeC 插入]
  → typec.c: CC 检测 → PORT_MANAGER_TASK (PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS)
    → port_manager.c: 枚举 → USB_DPDM_TASK (DPDM_EVT_SNK_ATTACHED)
      → dpdm.c: BC1.2 → QC → AFC → SCP 协议检测
        → USB_TASK (TCPM_EVT_DPDM_DONE)
          → tcpm.c: PD 协商完成
```

### 2. 无线充电启动流程

```
[RX 设备放上]
  → WPC_TASK: Digital Ping → ASK 包接收
    → WPC_PHASE_PING → CNFG → NEGO → XFER
      → wpc_ping.c: SIG_01 检测 → USB_TASK (TCPM_EVT_QI_WORK)
        → tcpm.c: 设置 VBUS 电压
          → adp.c → USB_TASK (TCPM_EVT_QI_SET_VOLT)
```

### 3. 端口断开流程

```
[TypeC 拔出]
  → typec.c: CC 断开检测
    → USB_DPDM_TASK (DPDM_EVT_SRC_UNATTCHED)
    → PORT_MANAGER_TASK (PORT_ENUM_EVT_PORT*_DISCONNECT)
      → port_manager.c: 端口关闭 → 重新仲裁功率分配
```

### 4. 异常保护流程

```
[NTC 温度超限]
  → APL_TASK (100ms): protection_check()
    → gd->prot_sts 标志置位
      → BUCKBOOST_TASK: 降功率/关断
      → WPC_TASK: WPC_EVT_STOP_POWER (无线充电停止)
```

---

## 时序约束

| 路径 | 延迟要求 | 说明 |
|------|---------|------|
| ASK ISR → WPC_TASK | < 1ms | Qi 协议时序敏感 |
| TypeC CC → PORT_MANAGER | < 10ms | 用户体验 |
| DPDM 检测完成 → USB_TASK | < 100ms | 快充协议握手窗口 |
| 保护触发 → BUCKBOOST 关断 | < 17ms | 安全要求 (1 个 BB 周期) |

---

## 全局数据共享 (非事件通信)

除了 osal_set_event 事件，Task 间还通过全局结构体共享状态：

| 结构体 | 主要写入者 | 主要读取者 |
|--------|----------|----------|
| `gd->tx_infos` / `gd->rx_infos` | WPC_TASK | WPC_TASK, FML_TASK |
| `g_buckboost.adc_*` | BUCKBOOST_TASK | APL_TASK, FML_TASK, PORT_MGR |
| `gd->prot_sts` | APL_TASK (保护检查) | BUCKBOOST_TASK, WPC_TASK |
| `gd->Battery_cycle_count` | APL_TASK (bat_record) | FML_TASK (USB bridge) |
| `gd->real_soc_show` | FML_TASK (gauge) | APL_TASK (LED), FML_TASK (USB) |

> 详细结构体定义见 config 页面和源码 fml/g_data.h。

## 交叉引用

- 任务注册表: [osal-task-registry.md](osal-task-registry.md)
- 状态机: [state-machines.md](state-machines.md)
- 系统概览: [system-overview.md](system-overview.md)
