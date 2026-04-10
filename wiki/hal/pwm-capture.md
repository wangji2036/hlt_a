# PWM 与捕获驱动

**最后更新**: 2026-04-08
**来源文件**: hal/epwm.c, hal/epwm.h, hal/bpwm.c, hal/bpwm.h, hal/ecap.c, hal/ecap.h

---

## EPWM — 增强 PWM (Qi TX 全桥驱动)

**用途**: 驱动 Qi 无线充电发射端全桥 MOSFET
**频率范围**: 110 kHz ~ 148 kHz (配置 360 kHz 载波)
**占空比**: 0~500 比率
**相位角**: 0°~180°

### API

| 函数 | 说明 |
|------|------|
| hal_epwm_pwm_start() | 启动 PWM 输出 |
| hal_epwm_pwm_update(duty, phase) | 更新占空比和相位 |
| hal_epwm_pwm_stop() | 停止 PWM |
| hal_epwm_afd_start() | 启动自动频率检测 (AFD) |
| hal_epwm_afd_stop() | 停止 AFD |

### 在 WPC 系统中的角色

```
WPC_TASK (app/_wpc.c)
  → PID 控制器 (app/pid.c) 计算 duty/phase
    → hal_epwm_pwm_update(duty, phase)
      → MOSFET 全桥驱动
        → 发射线圈
```

## BPWM — 基础 PWM

**用途**: 辅助 PWM 控制

| 函数 | 说明 |
|------|------|
| hal_bpwm_start(period, duty) | 启动 |
| hal_bpwm_update(period, duty) | 更新 |
| hal_bpwm_stop() | 停止 |

## ECAP — 增强捕获 (Qi 谐振频率测量)

**用途**: 测量 Qi TX 线圈谐振频率，用于 Q-factor 检测和 FOD

### API

| 函数 | 说明 |
|------|------|
| hal_ecap_init() | 初始化捕获模块 |
| hal_ecap_open() | 开启捕获 |
| hal_ecap_close() | 关闭捕获 |
| hal_ecap_reg_int_cb(callback) | 注册捕获中断回调 |
| hal_ecap_dig_ddm_init() | DDM 数字解调模式 |

**中断**: ECAP1~5_IRQHandler (5 个通道)

### 在 WPC 系统中的角色

```
NU103x AFE → 谐振信号 → ECAP 捕获
  → 频率测量 → Q-factor 计算 (fml/qdt.c)
    → FOD 异物检测 (app/fod.c, app/qfod.c)
```

## 交叉引用

- WPC 状态机: [../architecture/state-machines.md](../architecture/state-machines.md)
- NU103x AFE: [../chips/nu103x.md](../chips/nu103x.md)
- HAL 概览: [hal-overview.md](hal-overview.md)
