# Platform USB Agent Knowledge

**管辖范围**: USB-C TypeC/PD协议栈实现，双端口DRP支持，TCPM协调层，WPC工作模式集成
**关键文件**: fml/tcpm.c/h, lib/typec.c, lib/pd_tc.c, lib/usb_pd.c, usbpd/pdlib.c/h + 6个头文件

---

## 1. Module Overview

### 1.1 Architecture Layering

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                       │
│         TCPM Task (USB_TASK) + Port Manager                 │
└─────────────────────────────────────────────────────────────┘
                            ↓ Events
┌─────────────────────────────────────────────────────────────┐
│           TCPM Coordination Layer (fml/tcpm.c)              │
│  - PDO管理 (Source/Sink PDO配置)                            │
│  - WPC工作模式切换 (FIX5V/BOOST/PD_PPS/ADP_FIX)             │
│  - USB-A端口控制                                            │
│  - 事件处理 (TCPM_EVT_*)                                    │
└─────────────────────────────────────────────────────────────┘
        ↓ pdlib API                      ↓ tcpm_update_wpc_work_mode
┌──────────────────────┐         ┌──────────────────────────┐
│   PD Library Wrapper │         │   WPC Integration        │
│   (usbpd/pdlib.c)    │         │   (buckboost + adp)      │
└──────────────────────┘         └──────────────────────────┘
        ↓ usb_pd_* / usb_tc_*
┌─────────────────────────────────────────────────────────────┐
│             USB PD Policy Engine (lib/usb_pd.c)             │
│  - SNK States: PE_SNK_Startup → Evaluate → Select → Ready  │
│  - SRC States: PE_SRC_Startup → Send_Cap → Negotiate → Ready│
│  - Hard/Soft Reset, PR_SWAP, DR_SWAP                        │
└─────────────────────────────────────────────────────────────┘
        ↓ hal_tcpc_*
┌─────────────────────────────────────────────────────────────┐
│          TypeC State Machine (lib/typec.c + pd_tc.c)        │
│  - DRP Toggle: TC_DRP_TOGGLE (Rd/Rp切换)                    │
│  - SNK: Unattached → AttachWait → Attached                  │
│  - SRC: Unattached → AttachWait → Attached                  │
│  - Try.SRC/Try.SNK (DRP尝试角色)                             │
└─────────────────────────────────────────────────────────────┘
        ↓ TCPC Register
┌─────────────────────────────────────────────────────────────┐
│         Hardware TCPC + PD PHY (lib/pd_tc.c)                │
│  - CC检测 (CCA/CCB双端口)                                   │
│  - PD消息收发 (SOP/HARD_RESET)                              │
│  - VCONN/VBUS/Polarity控制                                  │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 Dual-Port Architecture (SM3级)

**双TypeC端口 (PORT0_INDEX / PORT1_INDEX)**:
- **PORT0_INDEX (TYPEC_PORT_A)**: 主端口，支持PD PHY通信
- **PORT1_INDEX (TYPEC_PORT_B)**: 副端口，可独立工作但PD PHY共享
- **PD端口映射**: `g_tcpc.tc_port_map` 控制PD PHY连接到哪个物理端口
- **配置**: `CONFIG_TYPECA_SUPPORT` / `CONFIG_TYPECB_SUPPORT` / `CONFIG_USBPD_POWER_ROLR`

**关键数据结构**:
```c
struct tc_s g_tc[TYPEC_PORT_MAX_N];  // 双端口TypeC状态机
struct tcpc_s g_tcpc;                 // 单一PD PHY控制器
struct usb_pd_s g_usb_pd_s;          // PD Policy Engine状态
```

---

## 2. Public Interface (TCPM API)

### 2.1 TCPM Task Management

**初始化**:
```c
void tcpm_task_init(void);
// - 注册USB_TASK事件处理器
// - 启动USB_TC_PD_TIMER (1ms周期)
// - 初始化pdlib (TypeC/PD状态机)
// - 配置默认Source/Sink PDO
```

**事件处理** (tcpm_task_event_handler):
```c
// 周期事件
TCPM_EVT_TIME_PERIOD        // 1ms周期，调用pdlib_run()驱动状态机

// 端口扫描事件
TCPM_EVT_USBA_SCAN          // USB-A端口插拔检测 + 小电流检测
TCPM_EVT_QI_WORK            // 无线充电启动事件

// 电压控制事件
TCPM_EVT_QI_SET_VOLT        // 设置WPC输出电压 (BOOST/PPS模式)
TCPM_EVT_PD_READY           // PD协商完成，延迟500ms触发
```

### 2.2 PDO Configuration

**Source PDO** (NU6805平台):
```c
const uint32_t source_pdo[] = {
    PDO_FIXED(5000, 3000, SOURCE_PDO_FIXED_FLAGS),  // 5V/3A
    PDO_FIXED(9000, 3000, 0),                       // 9V/3A
    PDO_FIXED(12000, 3000, 0),                      // 12V/3A
    PDO_FIXED(15000, 3000, 0),                      // 15V/3A
    PDO_PPS_APDO(5000, 16000, 3000),                // 5-16V/3A PPS
};
```

**Sink PDO**:
```c
const uint32_t sink_pdo[] = {
    PDO_FIXED(5000, 3000, SINK_PDO_FIXED_FLAGS),   // 5V/3A
    PDO_FIXED(9000, 2000, 0),                      // 9V/2A
};
```

**动态更新**:
```c
void tcpm_update_pdo_for_ntc(void);      // 温度保护模式 (仅5V/2A)
void tcpm_update_pdo_for_normal(void);   // 恢复正常PDO
```

### 2.3 WPC Integration

**工作模式定义**:
```c
enum wpc_work_mode {
    TCPM_WPC_WORK_FIX5V,      // 固定5V输出
    TCPM_WPC_WORK_BOOST,      // Boost模式 (5-16.5V)
    TCPM_WPC_WORK_ADP_FIX,    // 适配器固定电压 (9V)
    TCPM_WPC_WORK_PD_PPS,     // PD PPS动态调压
    TCPM_WPC_WORK_DISABLE,    // 禁用
};
```

**模式切换**:
```c
void tcpm_update_wpc_work_mode(enum wpc_work_mode mode);
// 调用链: 更新g_adp.volt_max/min → pid_set_volt_limit
// PPS模式: 提取PD协商的PDO最大电压
```

**WPC控制**:
```c
void tcpm_stop_wpc(uint8_t delay_ping_unit);  // 停止WPC并延迟重启
uint16_t qi_volt;                              // WPC目标电压 (全局变量)
```

---

## 3. Internal Logic

### 3.1 TypeC State Machine (SM3级)

**DRP Toggle机制** (TC_DRP_TOGGLE):
```c
// 交替Rd/Rp探测 (35ms Rd + 45ms Rp)
static uint8_t a_toggle_rp_or_rd;  // 0=Rd, 1=Rp
static uint8_t a_toggle_rd_cnt;    // Rd周期计数
static uint8_t a_toggle_rp_cnt;    // Rp周期计数

// 检测逻辑 (1ms周期):
if (a_toggle_rp_or_rd == 0) {  // Rd阶段
    hal_tcpc_set_cc(tc_index, TYPEC_CC_RD);
    if (tc_snk_is_connected(cc1,cc2) && delay_cnt >= 10) {
        usb_tc_set_state(tc, TC_SNK_Unattached, enter_state);
    }
} else {  // Rp阶段
    hal_tcpc_set_cc(tc_index, TYPEC_CC_RP_3_0);
    if (tc_src_is_connected(cc1,cc2) && delay_cnt >= 10) {
        usb_tc_set_state(tc, TC_SRC_Unattached, enter_state);
    }
}
```

**Sink连接流程**:
```
TC_SNK_Unattached (设置Rd)
  → 检测到Rp → TC_SNK_AttachWait (CC去抖动)
  → tPdDebounce后 + VBUS存在 → TC_SNK_Attached
  → 触发PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS
```

**Source连接流程**:
```
TC_SRC_Unattached (设置Rp 3.0A)
  → 检测到Rd → TC_SRC_AttachWait (CC去抖动)
  → tCcDebounce后 → TC_SRC_Attached
  → 使能VBUS → 触发PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS
```

**Try.SRC/SNK机制** (CONFIG_TC_TRY_SOURCE_SUPPORT_EN):
```c
// Sink侧: AttachWait后先尝试成为Source
if (tc->try_src_cnt >= 3 || tc->is_deadbattery) {
    usb_tc_set_state(tc, TC_SNK_Attached, enter_state);
} else {
    tc->try_src_cnt++;
    usb_tc_set_state(tc, TC_Try_SRC, enter_state);  // 尝试Rp
}

// Source侧: AttachWait后先尝试成为Sink
if (tc->try_snk_cnt >= 5) {
    usb_tc_set_state(tc, TC_SRC_Attached, enter_state);
} else {
    usb_tc_set_state(tc, TC_Try_SNK, enter_state);  // 尝试Rd
}
```

### 3.2 PD Policy Engine (SM4级)

**Sink PE状态转换**:
```
PE_SNK_Startup (初始化PHY + 设置SINK角色)
  ↓
PE_SNK_Discovery (等待VBUS)
  ↓
PE_SNK_Wait_for_Capabilities (启动SinkWaitCapTimer=365ms)
  ↓ 收到Source_Capabilities
PE_SNK_Evaluate_Capability (选择PDO)
  ↓
PE_SNK_Select_Capability (发送Request)
  ↓ 收到Accept
PE_SNK_Transition_Sink (等待PS_RDY)
  ↓ 收到PS_RDY
PE_SNK_Ready (PPS模式启动1500ms周期定时器)
```

**Source PE状态转换**:
```
PE_SRC_Startup (初始化PHY + 设置SOURCE角色)
  ↓
PE_SRC_Discovery (启动SourceCapabilityTimer=150ms)
  ↓ 定时器到期
PE_SRC_Send_Capabilities (发送Source_Cap, caps_counter++)
  ↓ 收到Request
PE_SRC_Negotiate_Capability (检查RDO合法性)
  ↓ 检查通过
PE_SRC_Transition_Supply (发送Accept → 等待VBUS就绪 → 发送PS_RDY)
  ↓
PE_SRC_Ready (PPS模式启动14000ms监控定时器)
```

**PPS动态调压** (Sink侧):
```c
// 触发: SinkPPSPeriodicTimer超时 (1500ms)
if (g_usb_pd_s.is_in_pps && usb_pd_timer_is_timeout(SinkPPSPeriodicTimer)) {
    usb_pd_set_state(PE_SNK_Select_Capability, enter_state);  // 重新发送Request
}

// RDO构造:
g_usb_pd_s.snk_rdo = RDO_PROG(pdo_index, qi_volt, max_current, 0);
```

**Hard Reset处理**:
```c
// Sink侧:
PE_SNK_Hard_Reset → PE_SNK_Transition_to_default (复位角色 + PHY)
  → 100ms后 → PE_SNK_Startup

// Source侧:
PE_SRC_Hard_Reset → 启动PSHardResetTimer(28ms)
  → PE_SRC_Transition_to_default (关VBUS → 800ms → 开VBUS)
  → PE_SRC_Startup
```

### 3.3 PD Message Handling

**消息收发** (lib/pd_tc.c):
```c
void hal_tcpc_send_request_mgs(uint32_t rdo);
// - 构造PD_DATA_REQUEST消息
// - 设置MessageID (tx_sop_msgid)
// - 配置重试次数 (PD3.0=2, PD2.0=3)
// - 写入TCPC TX Buffer

void hal_tcpc_send_source_caps(uint32_t *pdos, uint32_t pdo_n);
// - 过滤: PD2.0仅发送Fixed PDO
// - PD3.0发送所有PDO (含PPS/AVS)
```

**RDO检查** (Source侧):
```c
uint32_t usb_pd_check_request(struct usb_pd_request_packet_t *rqt);
// 返回值: 0=合法, 0x01=索引错误, 0x02=电流超限, 0x03/0x04=PPS参数错误

// Fixed PDO检查:
if (rdo_op_current > pdo_max_current) return check_current_error;

// PPS APDO检查:
if (voltage > pdo_max_voltage || voltage < pdo_min_voltage)
    return check_pps_voltage_error;
if (rdo_op_current > pdo_max_current)
    return check_pps_current_error;
```

**UVDM扩展** (PowerZ表计识别):
```c
// 识别标志: is_power_z = (vdm_head == 0xFF000220)
// 支持命令:
0xFF000220: PowerZ握手
0xFF000230: 发送异常告警 (hal_tcpc_uvdm_send_warming_Info)
0xFF000240: 电池单体电压 (hal_tcpc_uvdm_send_bat_Info)
0xFF000248: 电池容量信息 (hal_tcpc_uvdm_send_BatteryCapacity)
0xFF000250/260: 厂商/产品字符串 ("NuVolta"/"Nu17113")
```

### 3.4 Timer System

**定时器数组**:
```c
static union usb_pd_timer_u usb_pd_timers[USBPD_TIMER_MAX];

struct usb_pd_timer_t {
    uint16_t time_cnt;     // 倒计时 (单位: ms)
    enum usb_pd_timer_state state;  // TIMER_STOP/TIMER_RUNNING
    bool timeout;          // 超时标志
};
```

**关键定时器**:
- **SinkWaitCapTimer**: 365ms (等待Source_Capabilities)
- **SenderResponseTimer**: 25ms (等待Accept/Reject)
- **PSTransitionTimer**: 500ms (等待PS_RDY)
- **SinkPPSPeriodicTimer**: 1500ms (PPS重新请求周期)
- **SourcePPSCommTimer**: 14000ms (Source侧PPS超时监控)

---

## 4. Key Values

### 4.1 PD Voltage Profiles

**NU6805平台** (支持15V+PPS):
```
PDO1: 5V/3A (Fixed, Unconstrained)
PDO2: 9V/3A (Fixed)
PDO3: 12V/3A (Fixed)
PDO4: 15V/3A (Fixed)
PDO5: 5-16V/3A (PPS APDO)
```

**NU6801平台** (支持11V+AVS):
```
PDO1: 5V/3A (Fixed, DualRole, USB_COMM)
PDO2: 9V/2A (Fixed)
PDO3: 12V/1.5A (Fixed)
PDO4: 5-11V/2A (PPS APDO)
PDO5: 2-10V/1A (SPR_AVS)
```

### 4.2 Timeout Values

```c
// TypeC层
#define TC_T_CC_DEBOUNCE        100   // CC去抖动 (100-200ms)
#define TC_T_PD_DEBOUNCE        10    // PD去抖动 (10-20ms)
#define TC_T_DRP_TRY            240   // Try.SRC/SNK持续时间
#define TC_T_ERROR_RECOVERY     500   // 错误恢复延迟

// PD层
#define tSinkWaitCapTime        365   // 等待Source_Cap
#define tSenderResponseTime     25    // 等待Accept
#define tPSTransitionTime       500   // 等待PS_RDY
#define tSinkPPSPeriodicTime    14000 // PPS超时 (Sink侧1500ms重发)
#define tPSHardResetTime        28    // Hard Reset后延迟
```

### 4.3 State Transition Conditions

**DRP Toggle切换条件**:
```c
// Rd阶段 → Rp阶段: a_toggle_rd_cnt >= 35
// Rp阶段 → Rd阶段: a_toggle_rp_cnt >= 45
// 确认连接: delay_cnt >= 10 (10ms连续检测)
```

**小电流模式检测**:
```c
// TypeC-A: adc_ibus in [-120, 0] → light0_cnt >= 250 → 标记照明模式
// TypeC-B: adc_ibus in [-60, 0] → light1_cnt >= 250 → 标记照明模式
// 照明模式: 进入TC_DRP_TOGGLE但不切换角色，1000周期后超时退出
```

---

## 5. Interaction Map

### 5.1 With Port Manager

**事件通知**:
```c
port_manager_set_event(PORT0_EVENT_TRY_CONNECT);       // TypeC开始连接
port_manager_set_event(PORT0_EVENT_UNCONNECT);        // TypeC断开
osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS); // 连接成功
port_manager_set_event(PORT_EVENT_RESET_CHARGE);      // PD重新协商后复位充电
```

### 5.2 With WPC Module

**模式协调**:
```c
// TCPM → WPC
tcpm_update_wpc_work_mode(TCPM_WPC_WORK_PD_PPS);
  → fml_adp_type_set(EADP_TYPE_POWERBANK_PPS, 5000, pdo_max_volt, 15*2);
  → pid_set_volt_limit(volt_max, volt_min, volt_min);

// WPC → TCPM
osal_set_event(USB_TASK, TCPM_EVT_QI_WORK);  // 无线充电接收机检测到
osal_set_event(USB_TASK, TCPM_EVT_QI_SET_VOLT);  // 请求调压

// 停止协调
tcpm_stop_wpc(delay) → wpc_stop_to_idle(ESYS_ERR_CODE_TYPEC_CHANGE);
```

### 5.3 With DPDM Module

**端口切换**:
```c
void usb_dpdm_port0_switch(bool enable);
// - PORT0连接时切换DPDM到TC-A
// - 调用时机: TC_SRC_Attached_Entry/Exit, TC_SNK_Attached_Entry/Exit

void tcpm_set_port_sdp(uint8_t tc_index);
// - 设置端口为SDP模式 (DPDM->SOURCE_CTRL=0)
// - 设置CC为TYPEC_CC_RP_DEF (500mA)
```

### 5.4 With Buckboost

**VBUS控制**:
```c
hal_tcpc_set_gate_en(tc_index, true/false);      // 使能/禁用VBUS输出
hal_tcpc_port_dummyload_en(tc_index, true);      // 假负载使能 (防止VBUS过压)
hal_tcpc_pd_set_bus_iv(tc_index, voltage, current, ...);  // 设置VBUS电压/电流

// 电压就绪检查
hal_tcpc_vbus_is_present(tc_index);  // VBUS > 3.8V
hal_tcpc_vbus_is_vsafe5v();           // VBUS ≈ 5V ± tolerance
hal_tcpc_pd_bus_ready(tc_index);     // VBUS达到目标电压
```

---

## 6. Expert Insights

### 6.1 PD Compliance Risks

**PD协商超时风险**:
- **Issue**: `SinkWaitCapTimer=365ms` 比规范要求的`tTypeCSinkWaitCap=310-620ms`偏小
- **Impact**: 某些慢速Source可能导致超时 → Hard Reset
- **Mitigation**: 已设置`N_HARDRESET_COUNTER=50`允许多次重试

**PPS周期合规**:
- **Sink**: 1500ms重发Request (规范要求≤10s)
- **Source**: 14000ms超时 (规范要求10-15s)
- **Risk**: Source侧`tSinkPPSPeriodicTime=14000ms`超出规范上限

### 6.2 VBUS竞争问题

**DRP双角色冲突**:
```c
// 场景: 两台移动电源互连
// 问题: 双方同时输出VBUS → 电压竞争 → 无法协商
// 解决: Try.SRC/SNK机制 + try计数器限制
if (tc->try_src_cnt >= 3) {
    usb_tc_set_state(tc, TC_SNK_Attached, enter_state);  // 强制SNK
}
```

**VBUS放电不足**:
- **Issue**: Hard Reset后仅延迟100ms即重启 (规范要求VBUS降至vSafe0V)
- **Risk**: 残余电压导致Sink误判 → 协商失败
- **Fix**: 增强`hal_tcpc_port_dummyload_en`放电时间

### 6.3 Hard Reset风险

**Source侧Hard Reset丢失VBUS**:
```c
PE_SRC_Transition_to_default_Entry:
  hal_tcpc_set_gate_en(tc_index, false);  // 关VBUS
  // ... 800ms延迟 ...
  hal_tcpc_set_gate_en(tc_index, true);   // 重新开VBUS
```
- **Issue**: 800ms无VBUS → Sink可能进入低功耗/断电
- **Impact**: 协商失败率增加，尤其是对低容量Sink

**Soft Reset消息ID冲突**:
```c
usb_pd_reset_prl():
  g_usb_pd_s.rx_sop_msgid = -1;  // 复位MessageID
  g_usb_pd_s.tx_sop_msgid = 0;
```
- **Risk**: 如果对端未同步复位MessageID → GoodCRC校验失败 → Soft Reset循环

### 6.4 PPS调压精度

**20mV步进限制**:
```c
#define RDO_PROG_VOLT_MV_STEP  20  // PPS电压步进
voltage = rqt->request.PPS_BITS.output_voltage * 20;
```
- **Issue**: 无法实现精确到1mV的调压
- **Impact**: 快充协议兼容性降低 (部分手机要求±10mV)

**WPC-PPS联动延迟**:
```c
TCPM_EVT_QI_SET_VOLT:
  if (wpc_mode == TCPM_WPC_WORK_PD_PPS) {
      pdlib_snk_requsrt_voltage(index, qi_volt, current);  // 重新协商
  }
```
- **Issue**: PPS调压需要1个PD消息周期 (>50ms) + tPSTransition (500ms)
- **Impact**: 无线充电动态调压响应慢

---

## 7. Quick Reference

### 7.1 PD Commands

**Sink侧请求电压**:
```c
pdlib_snk_requsrt_voltage(pdo_index, voltage, current);
// - Fixed PDO: voltage忽略，使用PDO固定电压
// - PPS APDO: voltage精度20mV，current精度50mA
```

**查询协商结果**:
```c
uint32_t pdo = pdlib_snk_get_work_pdo();  // 获取当前工作PDO
uint8_t index = pdlib_snk_get_work_pdo_index();
uint16_t volt = pdlib_get_source_supply_voltage();  // 协商电压
uint16_t curr = pdlib_get_source_supply_current();  // 协商电流
```

**角色查询**:
```c
enum pwr_role_e role = pdlib_get_pwr_role();   // TYPEC_SINK/TYPEC_SOURCE
enum usb_tc_state_e state = pdlib_get_tc_state(index);  // TypeC状态
bool connected = pdlib_is_connect();            // PD显式合同建立?
bool pps_mode = pdlib_is_pps_sink();            // PPS模式?
```

### 7.2 TypeC Control

**端口控制**:
```c
pdlib_disable_typec(index);           // 禁用TypeC端口
pdlib_restart_typec(index);           // 重启TypeC端口
pdlib_delayms_restart_typec(index, delay_ms);  // 延迟重启

pdlib_tcpc_set_cc(index, TYPEC_CC_RP_3_0);  // 设置CC为Rp 3.0A
pdlib_tcpc_set_cc(index, TYPEC_CC_RD);      // 设置CC为Rd
```

**状态读取**:
```c
enum tc_cc_status cc1, cc2;
pdlib_tcpc_get_cc(index, &cc1, &cc2);  // 读取CC状态
// cc1/cc2: TYPEC_CC_OPEN/RA/RD/RP_DEF/RP_1_5/RP_3_0
```

### 7.3 Debugging Macros

**日志开关**:
```c
#define usbpd_printk(...)  // PD调试日志 (需定义USBPD_DEBUG)
```

**关键寄存器**:
```c
TCPC->CCA_STAT.WORD  // CC-A状态 (bit[1:0]=CC1, bit[3:2]=CC2)
TCPC->CCA_ROLE.WORD  // CC-A角色 (Rd/Rp/RA)
TCPC->PHY_CTRL.BITS.PD_CC_PORT_SEL  // PD PHY端口选择 (1=Port-A, 2=Port-B)
TCPC->RXD_CTRL.WORD  // PD接收控制 (SOP使能 + 角色)
```

---

## 8. Reference Material Needs

### 8.1 Specifications

**必需规范**:
1. **USB PD 3.1 Spec** (USB-IF) - PPS/AVS APDO定义
2. **USB Type-C Cable and Connector Spec 2.1** - DRP/Try.SRC/SNK机制
3. **USB-IF Compliance Test Spec** - PD消息定时要求

**推荐参考**:
4. **TCPCI Spec 2.0** (Type-C Port Controller Interface) - TCPC寄存器映射
5. **VESA DisplayPort Alt Mode Spec** (如需支持视频输出)

### 8.2 Code Navigation

**关键状态机入口**:
- **TypeC**: `usb_tc_run()` → `usb_tc_table[state].exit_cb()`
- **PD PE**: `usb_pd_run()` → `usb_pd_tasks_table[state].exit_cb()`
- **事件分发**: `tcpm_task_event_handler()` (USB_TASK)

**中断处理**:
- **CC变化**: 硬件中断 → 状态机在1ms周期内轮询
- **PD消息**: `TCPC中断` → `usb_pdevt_run()` → 设置`USB_PD_EVT_RX_SOP_PACKET`

**关键全局变量**:
```c
extern struct tc_s g_tc[TYPEC_PORT_MAX_N];  // TypeC状态
extern struct tcpc_s g_tcpc;                 // TCPC控制器
extern struct usb_pd_s g_usb_pd_s;          // PD PE状态
extern uint16_t port_vbus, qi_volt;          // VBUS + WPC电压
extern uint8_t wpc_mode;                     // WPC工作模式
```

---

**Last Updated**: 2026-02-15
**Platform**: NU17112 Powerbank (NU6805/NU6801)
**Complexity Level**: 4 (SM3 TypeC + SM4 PD PE)
