# Platform HAL Agent Knowledge

**Scope**: HAL layer (20 drivers), OSAL scheduler, utilities, and startup code (53 files total)
**Purpose**: Hardware abstraction, interrupt management, task scheduling, and peripheral control
**Core Architecture**: CK802 MCU @ 36MHz with event-driven OSAL scheduler

---

## 1. Module Overview & Architecture

### 1.1 File Organization (53 Files)

```
hal/ (20 drivers + regdef.h)
├── Core Infrastructure
│   ├── _hal.c/h         - HAL event handler stub
│   ├── isr.c/h          - Interrupt Service Routines (26 ISRs)
│   ├── vic.c/h          - Vector Interrupt Controller + priority config
│   ├── sys.c/h          - Clock/PLL/power management
│   ├── regdef.h         - Complete register map (88K+ lines)
│   └── overview.h       - Documentation header
│
├── Timing & Watchdog
│   ├── timer.c/h        - TMR0-3: 250ms/1ms/850ms/1ms timers
│   └── wdt.c/h          - Hardware watchdog
│
├── Analog & Capture
│   ├── eadc.c/h         - Enhanced ADC (demodulation, Vref/Vcap)
│   ├── badc.c/h         - Basic ADC
│   └── ecap.c/h         - Enhanced Capture (5 channels)
│
├── PWM & Modulation
│   ├── epwm.c/h         - Enhanced PWM (wireless charging)
│   ├── bpwm.c/h         - Basic PWM
│   └── ddm.c/h          - Digital Demodulation
│
├── Communication
│   ├── uart.c/h         - 2x UART @ 250Kbps
│   ├── i2cm.c/h         - I2C Master (hardware + software modes)
│   ├── i2cs.c/h         - I2C Slave
│   └── gpio.c/h         - 23 GPIO + 7 GPIN
│
├── USB Power Delivery
│   └── tcpc.c/h         - Type-C PHY abstraction (voltage/gate control)
│
├── External Charging ICs (via I2C)
│   ├── nu6801.c/h       - NU6801 buck-boost controller (1-cell)
│   └── nu6805.c/h       - NU6805 buck-boost controller (2-cell)
│
└── Memory
    └── fmc.c/h          - Flash Memory Controller

osal/
├── osal.c/h             - Event-driven task scheduler (31 timers, 8 tasks)

util/
├── delay.c/h            - Microsecond/millisecond delays
├── printk.c/h           - printf-like debug output
├── algo.c/h             - Algorithms (CRC, etc.)
└── typdef.h             - Type definitions

startup/
├── crt0.S               - C-Sky CK802 startup assembly
└── ckcpu.ld             - Linker script (120KB ROM, 7KB SRAM)
```

### 1.2 System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                       Application Layer                      │
│            (APL_TASK, WPC_TASK, USB_TASK, etc.)             │
└─────────────────────┬───────────────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────────────┐
│                    OSAL Scheduler                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ 31 Timers    │  │  8 Tasks     │  │ Event Queue  │      │
│  │ (1ms tick)   │──┤  (callbacks) │──┤ (32-bit mask)│      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
└─────────────────────┬───────────────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────────────┐
│                       HAL Layer                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  VIC (Nested Vectored Interrupt Controller)         │   │
│  │  - Priority 0-3 (0=highest)                         │   │
│  │  - 26 interrupt sources enabled                     │   │
│  └──────────────┬───────────────────────────────────────┘   │
│                 │                                            │
│  ┌──────────────▼───────────────────────────────────────┐   │
│  │ Peripheral Drivers                                   │   │
│  ├─────────────┬──────────────┬──────────────┬─────────┤   │
│  │ Timers      │ ADC/PWM      │ Comms        │ USB-PD  │   │
│  │ (TMR0-3)    │ (EADC/EPWM)  │ (UART/I2C)   │ (TCPC)  │   │
│  └─────────────┴──────────────┴──────────────┴─────────┘   │
└─────────────────────┬───────────────────────────────────────┘
                      │
┌─────────────────────▼───────────────────────────────────────┐
│           Hardware Registers (regdef.h)                      │
│  - WDT, TMR, SYS, GPIO, UART, I2C, TCPC, EADC, EPWM, etc.  │
│  - Memory-mapped @ 0x4000_0000 - 0x5FFF_FFFF               │
└──────────────────────────────────────────────────────────────┘
```

---

## 2. OSAL Scheduler Deep Dive

### 2.1 Core Concepts

**File**: `osal/osal.c` (324 lines)

The OSAL (Operating System Abstraction Layer) is a **lightweight cooperative scheduler** with:
- **Event-driven architecture**: Tasks execute when events are posted
- **Software timers**: 31 timers (1ms granularity) for timeout/periodic events
- **8 task slots**: Each task has a callback + 32-bit event mask
- **No preemption**: Tasks run to completion (cooperative)

### 2.2 Task Enumeration

```c
enum {
    HAL_TASK = 0,           // Hardware events
    FML_TASK = 1,           // Firmware/modulation layer
    USB_TASK = 2,           // USB protocols
    WPC_TASK = 3,           // Wireless Power Consortium (Qi charging)
    APL_TASK = 4,           // Application logic
    BUCKBOOST_TASK = 5,     // DC-DC converter control
    USB_DPDM_TASK = 6,      // USB D+/D- detection
    PORT_MANAGER_TASK = 7,  // Port state machine
};
```

### 2.3 Timer Enumeration (31 Timers)

```c
enum {
    APP_010ms_TIMER = 0,
    APP_100ms_TIMER = 1,
    APP_250ms_TIMER = 2,
    USB_TIMER = 3,
    WPC_PING_TIMER = 4,
    WPC_NEXT_TIMER = 5,
    WPC_RESP_TIMER = 6,
    WPC_NEGO_TIMER = 7, WPC_AUTH_TIMER = 7,
    WPC_CEP_TIMER  = 8,
    WPC_RPP_TIMER  = 9,
    WPC_DDM_TIMER  = 10,
    USB_TC_PD_TIMER = 11,
    USB_BC12_TIMER = 12,
    USB_QC_TIMER = 13,
    BUCKBOOST_PERIOD_TIMER = 14,
    BUCKBOOST_REGULATOR_TIMER = 15,
    // ... up to MAX_TIMER (31)
};
```

### 2.4 Event Declaration (WPC Task Example)

```c
#define osal_event_declare(nr)    (1 << nr)

#define WPC_EVT_DIG_PING          osal_event_declare(0)
#define WPC_EVT_PIN_NO_PKT        osal_event_declare(1)
#define WPC_EVT_HDR_START         osal_event_declare(2)
#define WPC_EVT_HDR_RECVD         osal_event_declare(3)
#define WPC_EVT_PKT_RECVD         osal_event_declare(4)
#define WPC_EVT_PING_1st_PKT_TO   osal_event_declare(5)
#define WPC_EVT_STOP_POWER        osal_event_declare(6)
// ... 27 events defined for WPC_TASK
```

### 2.5 API Reference

```c
// Initialization
void osal_init(void);                    // Clear all tasks/timers
void osal_start_system(void);            // Enter infinite event loop

// Event Management
void osal_set_event(uint8_t task_id, uint32_t event);
void osal_clear_event(uint8_t task_id, uint32_t event);

// Timer Management
void osal_start_timerEx(uint8_t timer_id, uint16_t timeout,
                        uint16_t period, uint8_t task_id, uint32_t event);
void osal_stop_timerEx(uint8_t timer_id);

// Task Registration
void osal_task_handler_reg(uint8_t task_id, void (*handler)(uint32_t));

// Utilities
void osal_mem_copy(void *dst, const void *src, int len);
void osal_mem_set(void *mem, uint8_t val, int len);
void osal_mem_clear(void *mem, int len);
```

### 2.6 Execution Flow

```
Main Loop (osal_start_system):
  ┌──────────────────────────────┐
  │  1. hal_wdt_feed()           │  Feed hardware watchdog
  ├──────────────────────────────┤
  │  2. osal_timer_update()      │  Update all running timers
  │     - Check sys_ticks delta  │    (driven by TMR1 1ms ISR)
  │     - Decrement remainders   │
  │     - Post events if timeout │
  ├──────────────────────────────┤
  │  3. osal_event_handle()      │  Process posted events
  │     - Scan task event masks  │
  │     - Call task callbacks    │  (one event at a time)
  │     - Clear processed events │
  └──────────────────────────────┘
         ↓ (repeat forever)
```

### 2.7 Event Priority Algorithm

**Key Feature**: Events are processed **lowest bit first** using a bitmap lookup table.

```c
static const unsigned char bit_map[] = {
    0xFF, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    // ... 256-byte lookup table for fast bit scanning
};

// Event processing order:
while (event != 0) {
    if (event & 0x000000FF)
        evt_msk = (1 << (0 + bit_map[(event & 0x000000FF) >> 0]));
    else if (event & 0x0000FF00)
        evt_msk = (1 << (8 + bit_map[(event & 0x0000FF00) >> 8]));
    // ... handle event ...
    event &= ~evt_msk;
}
```

**Implication**: Lower event IDs have higher priority (bit 0 > bit 31).

---

## 3. Peripheral Drivers Index

### 3.1 Timers (timer.c/h)

**Hardware**: 4 timers with different clock sources and modes

| Timer | Mode      | Interval | Clock Source | ISR Function      | Purpose                     |
|-------|-----------|----------|--------------|-------------------|-----------------------------|
| TMR0  | One-shot  | 250ms    | LIRC 16KHz   | TMR0_IRQHandler   | ECAP edge detection events  |
| TMR1  | Periodic  | 1ms      | HCLK 9MHz    | TMR1_IRQHandler   | System tick (sys_ticks++)   |
| TMR2  | Periodic  | 850ms    | HCLK 1.125MHz| TMR2_IRQHandler   | Soft watchdog (resets MCU)  |
| TMR3  | Periodic  | 1ms      | HCLK 9MHz    | TMR3_IRQHandler   | Duty ramp-up, FSK timing    |

**API**:
```c
void hal_timer_init(TS_TMR *timer);  // Configure timer (LOAD_CNT, prescaler, mode)
void hal_timer_stop(TS_TMR *timer);  // Disable timer
```

**Critical Variables**:
- `volatile uint16_t sys_ticks` - 1ms counter (wraps at 65535)
- `volatile uint32_t tc_sys_ticks` - Extended tick counter

**TMR1 ISR Responsibilities** (1ms heartbeat):
1. Increment `sys_ticks`, `tc_sys_ticks`
2. Update USB PD library timers (`usb_pdlib_timer_update()`)
3. UI display refresh (`ui_display()`)
4. Key handling every 10ms (`key_handle_10ms()`)
5. RTC time tracking (`gd->Bat_RTC_Seconds`)

**TMR2 ISR** (850ms soft watchdog):
```c
void soft_wdt_reset() {
    gd->power_on_magic = 0x00;
    SYS->RST_CTRL.BITS.MCU_RST = 1;  // Force MCU reset
}
```
**WARNING**: TMR2 **must** be reset periodically or system will reboot!

---

### 3.2 GPIO (gpio.c/h)

**Hardware**: 23 configurable GPIOs + 7 input-only GPINs

**Initialization** (`hal_gpio_init()`):
- **PA0/PA1**: I2C1 Slave (SCL1_S/SDA1_S, open-drain, no pull)
- **PA4/PA5**: Conditional on chip PID (NU17111 vs NU17112)
  - NU17112: Input mode for special functions
  - NU17111: Output mode (drive high)
- **PA6/PA7**: Output high (power control)
- **PB0/PB1**: Output low (default state)

**Registers**:
- `I_EN` - Input enable
- `O_EN` - Output enable
- `DOUT` - Data output
- `DIN` - Data input (read-only)
- `ODEN` - Open-drain enable
- `PUEN/PDEN` - Pull-up/down enable
- `MODE` - Function mux (2 bits per pin, 4 modes)
- `ITEN/ITTP` - Interrupt enable/trigger type

**Function Muxing Examples**:
- PA0: `00=SCL1_S, 01=PA0, 10=UART2_TXD, 11=DP_C`
- PA1: `00=SDA1_S, 01=PA1, 10=UART2_RXD, 11=DM_C`

---

### 3.3 UART (uart.c/h)

**Hardware**: 2 independent UARTs

**Configuration**:
- **Baud Rate**: 250Kbps (default)
  - `clk_div = 16`, `clk_cnt = 9`
  - Formula: `baud = 36MHz / clk_div / clk_cnt = 250,000`
- **Mode**: USCI_A0, Multi-mode, TX+RX enabled

**API**:
```c
void hal_uart_init(TS_UART *uart);               // Initialize UART1/2
void hal_uart_putc(TS_UART *uart, uint8_t chr);  // Transmit 1 byte (blocking)
void hal_uart_reg_int_cb(TS_UART *uart, void (*func)(void));  // Register RX ISR

// Printf redirection
void retarget_fputc(uint8_t chr);  // Called by printk()
```

**ISR Pattern**:
```c
void __attribute__((isr)) UART1_IRQHandler(void) {
    if (UART1->STS_FLAG.WORD & UART_STS_FLAG_RXEND_FLAG_Msk) {
        if (m_pfn_UART1_RxIntHandler != NULL)
            m_pfn_UART1_RxIntHandler();
        UART1->STS_FLAG.WORD = UART_STS_FLAG_RXEND_FLAG_Msk;  // W1C clear
    }
}
```

---

### 3.4 I2C Master (i2cm.c/h)

**Dual Implementation**:
- **HW_I2CM**: Hardware I2C controller (faster, recommended)
- **SW_I2CM**: Bit-banged software I2C (fallback)

**API** (HW mode):
```c
void hal_i2cm_init(uint32_t bps);  // Set bus speed (e.g., 100000, 400000)

int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data);

int hal_i2cm_read_multi_byte(uint8_t devAddr, uint8_t regAddr,
                              uint8_t *data, uint8_t length);
int hal_i2cm_wirte_multi_byte(uint8_t devAddr, uint8_t regAddr,
                               uint8_t *data, uint8_t length);
```

**Protocol Sequence** (read single byte):
1. Check bus not busy (`BUS_BUSY`, `ARB_LOST`)
2. Send START + DEV_ADDR (write) + wait ACK
3. Send REG_ADDR + wait ACK
4. Send RESTART + DEV_ADDR (read) + wait ACK
5. Read data byte + send NACK
6. Send STOP

**Error Codes**:
- `-10`: Bus check failed
- `-11`: Device address NACK
- `-12`: Register address NACK
- `-13`: Restart address NACK
- `-14`: Read data timeout
- `-15`: Stop condition failed

---

### 3.5 EADC (Enhanced ADC) (eadc.c/h)

**Modes**:
1. **Digital Demodulation** (`_EADC_MODE_DIG_DDM`)
2. **Analog Demodulation** (`_EADC_MODE_ANA_DDM`)

**Channels**:
- `_EADC_CH_INR_V1P2` - Internal 1.2V reference
- `_EADC_CH_VCAP` - Coil voltage capture
- Custom channels for VBUS, IBAT, temperature, etc.

**Calibration** (stored in flash trim area):
- `eadc_vref_gain/bias` @ 0x00001C9C/0x00001CA2
- `eadc_vcap_gain/bias` @ 0x00001C96/0x00001C94

**Vref Update** (`hal_eadc_vref_update()`):
- Samples internal 1.2V reference
- Returns 3.3V if EPWM disabled
- Averages 20 samples for noise reduction
- Clock source: 40x EPWM frequency

**Key Values**:
- `EADC_VCAP_CHAN_DC_OFFSET = 1650`
- `EADC_VCAP_CHAN_FIXD_GAIN = 1385`

---

### 3.6 EPWM (Enhanced PWM) (epwm.c/h)

**Purpose**: Drive wireless charging coil (Qi 2.0)

**Capabilities**:
- Dual-channel PWM with programmable period/duty/phase
- FSK modulation support
- Synchronization with ADC sampling

**API**:
```c
void hal_epwm_init(TS_EPWM *epwm);
void hal_epwm_pwm_start(TS_EPWM *epwm, uint16_t period, uint16_t duty, uint16_t phase);
void hal_epwm_pwm_stop(TS_EPWM *epwm);
```

**Typical Frequencies**:
- **360KHz**: `period = 400` (for MPP mode)
- **110-205KHz**: Variable for BPP/EPP modes

**FSK Modulation**: Frequency-shift keying for data transmission back to receiver.

---

### 3.7 TCPC (Type-C PHY Control) (tcpc.c/h)

**Purpose**: Abstract interface between USB-PD stack and external buck-boost IC

**Key Functions**:
```c
bool hal_tcpc_vbus_is_present(uint8_t tc_index);     // Check VBUS >= 3.8V
bool hal_tcpc_vbus_is_removed(uint8_t tc_index);     // Check VBUS < 2.0V
bool hal_tcpc_vbus_is_vsafe0v(uint8_t tc_index);     // Check VBUS < 0.8V
bool hal_tcpc_vbus_is_vsafe5v(void);                 // Check VBUS <= 5.5V

void hal_tcpc_pd_set_bus_iv(uint8_t tc_index, uint16_t voltage,
                             uint16_t current, uint16_t wait, uint16_t delay);
bool hal_tcpc_pd_bus_ready(uint8_t tc_index);        // Check regulation done

void hal_tcpc_set_gate_en(uint8_t tc_index, bool en);  // Enable/disable port
void hal_tcpc_port_dummyload_en(uint8_t tc_index, bool en);  // Discharge VBUS
```

**Port Mapping**:
- `tc_index = 0`: Type-C Port A
- `tc_index = 1`: Type-C Port B
- `tc_index = 2`: USB-A Port

**Voltage Control**: Delegates to `buckboost_set_bus_iv()` for actual I2C commands.

---

### 3.8 NU6801 Buck-Boost IC (nu6801.c/h)

**I2C Address**: `NU6801_I2C_DEV_ADDR` (defined in header)

**Capabilities**:
- **1-cell battery** (3.0V - 4.5V)
- **Charge Mode**: 5V-20V input → battery charging
- **Discharge Mode**: Battery → 5V-20V output (3.3A max)
- **3x output gates**: Type-C A/B, USB-A

**Key Registers**:
- `REG_MISC_CTRL` (0x00): Mode control, reset
- `REG_BUBO_CTRL` (0x01): Buck-boost frequency config
- `REG_VBAT_CTRL` (0x02): CV voltage setting (4.1V-4.5V in 50mV steps)
- `REG_IBAT_CTRL` (0x03): Trickle/termination current
- `REG_VAC_DRV_CTRL` (0x04): Output gate & discharge control

**Initialization Sequence**:
1. Wake up (reset + enable)
2. Set default 5V/3.3A output
3. Configure battery CV (from `BATTERY_CV_VALUE`)
4. Disable all gates
5. **Dead battery detection**: If VBAT < 2.5V, enter trickle mode
6. **Unlock test mode** (0x50 = 0x65, 0x37, 0x2D, 0xF9)
7. Force IBAT_SNS off (0x6D |= 0x02)

**Dead Battery Handling**:
```c
if (vbat < 2500) {
    nu6801_dead_bat = true;
    hal_nu6801_buckboost_enter_force_trickle(true);  // 400mA trickle charge
}
```

**CV Voltage Formula**:
```c
value = (volt - 4200) / 50 + 1;  // volt in [4100, 4500]
```

---

### 3.9 NU6805 Buck-Boost IC (nu6805.c/h)

**I2C Address**: `NU6805_I2C_DEV_ADDR`

**Capabilities**:
- **2-cell battery** (6.1V - 9.0V)
- **Charge Mode**: 5V-22V input → battery charging
- **Discharge Mode**: Battery → 3V-22V output

**Key Registers**:
- `REG_Mode_Control` (0x00): 0x01=Discharge, 0x10=Charge
- `REG_Discharge_Vbus_Vol_High/Low` (0x01/0x02): Output voltage in 10mV steps
- `REG_Discharge_Ibus_Limit` (0x03): Current limit in 50mA steps (500mA base)
- `REG_Powerpath_Control` (0x04): Gate enable (bits 0-2)
- `REG_discharge_Control` (0x05): Discharge path control (bits 0-3)

**Initialization**:
1. Disable INDETB (insertion detection)
2. Set battery CV (from `BATTERY_CV_VALUE * 2`)
3. Set UV protection (`BAT_CELL_EMPTY_VOLT * 2 = 6100mV`)
4. Disable IEC62368 protection
5. Configure default 5V/3A output
6. Disable all gates
7. Set charge limits (IBUS=1A, IBAT=500mA)

**Voltage Calculation**:
```c
vbus = (vbus_mV - 3000) / 10;  // Range: 3000-22000mV
reg_high = vbus >> 3;
reg_low = vbus & 0x7;
```

---

### 3.10 System Control (sys.c/h)

**Purpose**: Clock, PLL, and power management

**Clock Configuration** (`hal_sys_init()`):
- **CPU Clock**: 36MHz (selected from 6/9/12/18/24/36MHz)
- **PLL Source**: XTAL (12MHz external crystal) or HIRC (internal RC)
- **XTAL Pre-divider**: Divide by 3 → 4MHz reference
- **Protection**: TSD threshold = 125°C, PVD threshold = 2.4V

**Register Map**:
- `SYS->CLK_CTRL` - Clock source/frequency selection
- `SYS->PWR_CTRL` - Sleep mode, wake-up config
- `SYS->RST_CTRL` - Individual peripheral resets
- `SYS->OPR_STAT` - PLL lock, FSM state, reset source
- `SYS->PRO_CTRL` - Thermal/voltage protection

**Reset Options** (SYS->RST_CTRL):
- `CPU_RST` - Reset CPU core only (PC → 0)
- `MCU_RST` - Full chip reset (like power-on)
- Per-peripheral resets: FMC, EADC, TIMER, UART, etc.

**Sleep Mode**:
```c
SYS->PWR_CTRL.BITS.SLEEP_MODE_EN = 1;  // Auto-cleared on wake-up
```

**Wake-up Sources**:
- TMR0 (if WKUP_EN set)
- GPIO interrupt
- TCPC protocol event

---

## 4. Interrupt System

### 4.1 ISR List (isr.c/h)

All ISRs use `__attribute__((isr, weak))` for easy overriding.

| IRQ Name          | Priority | Enabled | Handler                  | Purpose                          |
|-------------------|----------|---------|--------------------------|----------------------------------|
| default_IRQHandler| -        | Always  | System exceptions        | NMI, HardFault → force MCU reset |
| PROT_IRQHandler   | 0        | No      | Protection events        | TSD, PVD (empty stub)            |
| WDT_IRQHandler    | 3        | No      | Watchdog timeout         | (empty stub)                     |
| TMR0_IRQHandler   | 2        | Yes     | ECAP edge events         | Handles ECAP1/2/4 interrupts     |
| TMR1_IRQHandler   | 1        | Yes     | 1ms system tick          | sys_ticks++, UI, PD timers       |
| TMR2_IRQHandler   | 3        | Yes     | 850ms soft watchdog      | Forces MCU reset                 |
| TMR3_IRQHandler   | 3        | Yes     | 1ms PWM ramp/FSK         | Duty control, FSK timing         |
| EADC_IRQHandler   | 2        | No      | ADC conversion done      | (empty stub)                     |
| BADC_IRQHandler   | 3        | No      | Basic ADC done           | (empty stub)                     |
| ECAP1-5_IRQHandler| 2        | Yes     | Edge capture events      | (empty stubs)                    |
| GPIO_IRQHandler   | 3        | No      | GPIO pin change          | (empty stub)                     |
| UART1_IRQHandler  | 3        | Yes     | UART1 RX complete        | Calls registered callback        |
| UART2_IRQHandler  | 3        | No      | UART2 RX complete        | Calls registered callback        |
| I2CS_IRQHandler   | 3        | Yes     | I2C Slave events         | (empty stub)                     |
| I2CM_IRQHandler   | 1        | No      | I2C Master events        | (empty stub)                     |
| USBPD_IRQHandler  | 0        | Yes     | USB-PD protocol          | **Highest priority**             |
| UFCS_IRQHandler   | 1        | Cond    | UFCS fast charge         | If CONFIG_UFCS_SOURCE_SUPPORT    |
| DPDM_SINK_IRQHandler | 1     | Yes     | D+/D- detection          | BC1.2, DCP detection             |
| DCP_HVDCP_IRQHandler | 1     | Yes     | HVDCP negotiation        | QC2.0/3.0                        |
| QC_SRC_IRQHandler | 1        | Yes     | QC source mode           | Quick Charge                     |
| AFC_SCP_SRC_IRQHandler | 1   | Yes     | AFC/SCP protocols        | Samsung/Huawei fast charge       |
| FSK1_IRQHandler   | 1        | Yes     | FSK1 modulation          | (empty stub)                     |
| FSK2_IRQHandler   | 1        | Yes     | FSK2 modulation          | (empty stub)                     |
| DMA_IRQHandler    | 3        | No      | DMA transfer done        | (empty stub)                     |
| TCPC_IRQHandler   | 2        | No      | Type-C PHY events        | (empty stub)                     |

### 4.2 VIC Configuration (vic.c)

**Initialization** (`hal_vic_init()`):
```c
VIC_vModuleEnable();         // Enable NVIC
VIC->IPTR = 0x00000000;      // Clear pending
VIC->IABR = 0x00000000;      // Clear active

VIC_vEnableIRQ(IRQn_TMR1);   VIC_vSetPriority(IRQn_TMR1, 1);
VIC_vEnableIRQ(IRQn_USBPD);  VIC_vSetPriority(IRQn_USBPD, 0);  // Highest
// ... 26 interrupts configured
```

**Priority Levels**:
- **0**: USBPD (critical protocol timing)
- **1**: TMR1, FSK, USB protocols (high)
- **2**: TMR0, ECAP, EADC (medium)
- **3**: TMR2/3, UART, WDT (low)

**Nesting**: CK802 NVIC supports priority-based nesting (higher priority can preempt lower).

---

## 5. Key Values & Constants

### 5.1 Clock Frequencies

| Clock         | Frequency | Source         | Usage                        |
|---------------|-----------|----------------|------------------------------|
| HCLK          | 36 MHz    | PLL (XTAL/3)   | CPU, most peripherals        |
| LIRC          | 64 KHz    | Internal RC    | TMR0, Watchdog               |
| XTAL          | 12 MHz    | External       | PLL reference                |
| EPWM Freq     | 110-360KHz| HCLK / divider | Wireless charging coil       |
| UART Baud     | 250 Kbps  | HCLK / 144     | Debug/command interface      |
| I2C Speed     | 100/400KHz| HCLK / formula | External IC communication    |

### 5.2 Memory Map (ckcpu.ld)

| Region | Base       | Size   | Usage                        |
|--------|------------|--------|------------------------------|
| ROM    | 0x00002000 | 120KB  | Flash (code + const data)    |
| CFG    | 0x20000000 | 1KB    | Global config data           |
| RAM    | 0x20000400 | 7KB    | SRAM (stack + heap + BSS)    |
| Stack  | 0x20001FF8 | -      | Grows downward from top      |

**APB Registers**: 0x4000_0000 - 0x5FFF_FFFF (memory-mapped peripherals)

### 5.3 Battery Parameters

| Parameter              | NU6801 (1S) | NU6805 (2S) |
|------------------------|-------------|-------------|
| Cell Configuration     | 1-cell      | 2-cell      |
| CV Voltage Range       | 4.1-4.5V    | 8.2-9.0V    |
| Empty Voltage (UV)     | 3.0V        | 6.1V        |
| Full Voltage           | 4.2V        | 8.4V        |
| Trickle Current (6801) | 400mA       | -           |
| Termination Current    | 25mA (VBUS) | 200mA       |

### 5.4 Type-C Voltage Thresholds

| Threshold   | Voltage  | Function                |
|-------------|----------|-------------------------|
| vSafe0V     | < 0.8V   | Safe to switch paths    |
| vSafe5V     | 5.0V ±5% | Default USB-PD voltage  |
| Present     | > 3.8V   | VBUS detected           |
| Removed     | < 2.0V   | VBUS gone               |

---

## 6. Interaction Map

### 6.1 HAL → OSAL → App Flow

```
Hardware Event (e.g., TMR1 ISR @ 1ms)
  ↓
hal/timer.c: TMR1_IRQHandler()
  ├─ sys_ticks++                           // Global tick counter
  ├─ usb_pdlib_timer_update()              // Update USB-PD timers
  └─ ui_display()                          // Refresh LED indicators
  ↓
osal/osal.c: osal_timer_update()
  ├─ Check elapsed ticks (sys_ticks - old_ticks)
  ├─ Decrement timer remainders
  └─ If timeout: osal_set_event(task_id, event)
  ↓
osal/osal.c: osal_event_handle()
  ├─ Scan task event masks (lowest bit first)
  ├─ Call task callbacks (e.g., WPC_task_handler(WPC_EVT_PKT_RECVD))
  └─ Clear processed events
  ↓
Application Layer (apl/wpc.c)
  └─ Process Qi packet, update state machine
```

### 6.2 I2C Communication to Buck-Boost IC

```
Application requests voltage change (e.g., PD 9V/3A)
  ↓
hal/tcpc.c: hal_tcpc_pd_set_bus_iv(tc_index=0, 9000, 3000, ...)
  ↓
buckboost/buckboost.c: buckboost_set_bus_iv(9000, 3000, ...)
  ↓
hal/nu6801.c: hal_nu6801_buckboost_set_busiv(9000, 3000)
  ├─ Calculate register values: (9000 - 3000) / 10 = 600
  ├─ hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR, REG_VBUS_HIGH, 600>>8)
  └─ hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR, REG_VBUS_LOW, 600&0xFF)
  ↓
hal/i2cm.c: hal_i2cm_wirte_one_byte(...)
  ├─ I2CM_iBusCheckAndSet()                // Wait for bus idle
  ├─ I2CM_iSendStartBitAndDevAddr(dev, WRITE)
  ├─ hal_i2cm_byte_send(regAddr)
  ├─ hal_i2cm_byte_send(data)
  └─ I2CM_iSendStop()
  ↓
Hardware I2C Controller (I2CM->GEN_CTRL, TXD_DATA, etc.)
  ↓
NU6801 IC receives command, adjusts DC-DC converter
  ↓
hal/tcpc.c: hal_tcpc_pd_bus_ready(tc_index=0)
  ├─ Polls buckboost_regulator_done()
  └─ Returns true when VBUS reaches 9V ± tolerance
```

### 6.3 Interrupt Nesting Example

```
Normal Execution (Priority 3)
  ↓
TMR2_IRQHandler (Priority 3) - Soft watchdog fires
  ↓
  USBPD_IRQHandler (Priority 0) - PD message received
    ↓ (preempts TMR2)
    Process critical PD timing
    ↓ (return)
  Resume TMR2_IRQHandler
  ↓
  soft_wdt_reset() - Reset MCU
```

**Key Rule**: Lower priority number = higher interrupt priority (0 > 1 > 2 > 3).

---

## 7. Expert Insights & Gotchas

### 7.1 Interrupt Re-entrancy

**Problem**: TMR1 ISR (1ms) increments `sys_ticks`, which OSAL reads in main loop.

**Risk**: If main loop reads `sys_ticks` mid-update (non-atomic on 32-bit), timer delta could be incorrect.

**Mitigation**:
- `sys_ticks` is `volatile uint16_t` (atomic read/write on CK802)
- OSAL disables interrupts during critical sections:
  ```c
  VIC_vModuleDisable();
  event = osal_tasks_tbl[id].event;
  osal_tasks_tbl[id].event = 0;
  VIC_vModuleEnable();
  ```

### 7.2 TMR2 Soft Watchdog Trap

**Danger**: TMR2 ISR **always** calls `soft_wdt_reset()` after 850ms, forcing MCU reboot.

**Purpose**: Catch firmware hangs (infinite loops, stuck I2C, etc.).

**Requirement**: Application **must** periodically reset TMR2 or disable it:
```c
// Option 1: Feed the watchdog
TMR2->GEN_CTRL.BITS.LOAD_EN = 1;  // Reload counter

// Option 2: Disable soft watchdog
hal_timer_stop(TMR2);
```

**Failure Mode**: If main loop blocks for >850ms, system will reboot unexpectedly.

---

### 7.3 Register Write Order Dependencies

**Example: EADC Initialization**

1. **MUST** set `ADC_MODE`, `CHAN_SEL`, `VREF_SEL` **before** `ADC_EN`
2. **MUST** wait 20µs after channel selection before starting conversion
3. **MUST** clear `DONE_FLAG` before starting new conversion

**Wrong Order**:
```c
EADC->CTRL.WORD = EADC_CTRL_ADC_EN_Msk;  // Enable first
EADC->CTRL.WORD |= (_EADC_CH_VCAP << EADC_CTRL_CHAN_SEL_Pos);  // Channel later
// → May sample wrong channel or garbage data
```

**Correct Order**:
```c
EADC->CTRL.WORD = (_EADC_MODE_DIG_DDM << EADC_CTRL_ADC_MODE_Pos) |
                  (_EADC_CH_VCAP << EADC_CTRL_CHAN_SEL_Pos) |
                  EADC_CTRL_ADC_EN_Msk;  // Enable last
delay_1us(20);  // Settling time
EADC->FLAG.WORD = EADC_FLAG_DONE_FLAG_Msk;  // Clear flag
EADC->CTRL.WORD |= EADC_CTRL_CONV_START_Msk;  // Start conversion
```

---

### 7.4 I2C Bus Stuck Recovery

**Symptom**: `I2CM_iBusCheckAndSet()` returns `-10` (BUS_BUSY or ARB_LOST).

**Causes**:
- Slave holding SDA low
- Master reset mid-transaction
- Noise/glitch on bus

**Recovery**:
```c
// Disable I2C module
I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;

// Toggle SCL 9 times (in software GPIO mode)
for (int i = 0; i < 9; i++) {
    gpio_set_scl(0); delay_1us(5);
    gpio_set_scl(1); delay_1us(5);
}

// Re-initialize I2C
hal_i2cm_init(100000);
```

**Prevention**: Always check return values and implement timeout logic.

---

### 7.5 NU6801 Dead Battery Edge Case

**Scenario**: Battery voltage < 2.5V (deeply discharged).

**Hardware Behavior**: NU6801 may not wake up from I2C commands.

**Software Workaround**:
```c
if (vbat < 2500) {
    nu6801_dead_bat = true;
    hal_nu6801_buckboost_enter_force_trickle(true);
    // Unlock sequence + force trickle mode (400mA @ 3.2V)
    // Monitor VBAT until > 3.0V, then exit trickle
}
```

**Implication**: Must delay normal charging until battery recovers above 3.0V.

---

### 7.6 OSAL Event Priority Inversion

**Problem**: Lower event IDs are processed first, but all events in a task share the same callback.

**Example**:
```c
osal_set_event(WPC_TASK, WPC_EVT_STOP_POWER | WPC_EVT_PKT_RECVD);
// WPC_EVT_PKT_RECVD (bit 4) will be processed BEFORE WPC_EVT_STOP_POWER (bit 6)
```

**Risk**: If state machine assumes `STOP_POWER` happens first, incorrect behavior may occur.

**Mitigation**: Design task handlers to be **order-independent** or use separate timers.

---

## 8. Quick Reference (HAL API Cheat Sheet)

### 8.1 Initialization Sequence (from main.c)

```c
// 1. Clock & PLL
hal_sys_init();                    // 36MHz from XTAL

// 2. Watchdog
hal_wdt_init();                    // Hardware watchdog

// 3. GPIO
hal_gpio_init();                   // Configure all pins

// 4. Interrupts
hal_vic_init();                    // Enable NVIC + priorities

// 5. Timers
hal_timer_init(TMR0);              // ECAP events
hal_timer_init(TMR1);              // System tick
hal_timer_init(TMR2);              // Soft watchdog
hal_timer_init(TMR3);              // PWM control

// 6. Communication
hal_uart_init(UART1);              // Debug console
hal_i2cm_init(100000);             // 100KHz I2C

// 7. ADC
hal_eadc_init();                   // Load calibration

// 8. Buck-Boost IC
#if BUCKBOOST_USED_NU6801
hal_nu6801_buckboost_init();       // 1S battery
#else
hal_nu6805_buckboost_init();       // 2S battery
#endif

// 9. OSAL
osal_init();                       // Clear all tasks/timers
osal_task_handler_reg(HAL_TASK, hal_event_handler);
osal_task_handler_reg(WPC_TASK, wpc_event_handler);
// ... register all tasks ...

// 10. Start
osal_start_system();               // Enter infinite loop (never returns)
```

### 8.2 Common HAL Operations

**Timer Control**:
```c
hal_timer_init(TMR1);              // Configure
// Timer starts automatically after init
hal_timer_stop(TMR1);              // Disable
```

**UART Debug Output**:
```c
printk("VBUS = %d mV\n", vbus);    // Calls retarget_fputc() → hal_uart_putc()
```

**I2C Read/Write**:
```c
uint8_t data;
hal_i2cm_read_one_byte(0x50, 0x00, &data);   // Read reg 0x00 from device 0x50
hal_i2cm_wirte_one_byte(0x50, 0x01, 0xAB);   // Write 0xAB to reg 0x01
```

**ADC Sampling**:
```c
uint16_t vref = hal_eadc_vref_update();      // Returns Vref in mV (e.g., 3300)
```

**Type-C Voltage Control**:
```c
hal_tcpc_pd_set_bus_iv(0, 9000, 3000, 100, 10);  // Port 0: 9V, 3A, wait 100ms, delay 10ms
while (!hal_tcpc_pd_bus_ready(0));              // Poll until regulation done
```

**Gate Control**:
```c
hal_tcpc_set_gate_en(0, true);     // Enable Type-C Port A output
hal_tcpc_set_gate_en(1, false);    // Disable Type-C Port B output
```

**OSAL Timer**:
```c
// Start 100ms one-shot timer
osal_start_timerEx(APP_100ms_TIMER, 100, 0, APL_TASK, APP_EVT_TIMEOUT);

// Start 250ms periodic timer
osal_start_timerEx(APP_250ms_TIMER, 250, 250, APL_TASK, APP_EVT_PERIODIC);

// Stop timer
osal_stop_timerEx(APP_100ms_TIMER);
```

**OSAL Events**:
```c
// Post event to task
osal_set_event(WPC_TASK, WPC_EVT_PKT_RECVD);

// In task handler
void wpc_event_handler(uint32_t event) {
    if (event & WPC_EVT_PKT_RECVD) {
        // Process packet
        osal_clear_event(WPC_TASK, WPC_EVT_PKT_RECVD);  // Optional, auto-cleared
    }
}
```

---

## 9. Peripheral Register Snapshot (regdef.h Highlights)

### 9.1 Critical Base Addresses

| Peripheral | Base Address | Typedef    |
|------------|--------------|------------|
| WDT        | 0x40000000   | TS_WDT     |
| TMR0       | 0x40000020   | TS_TMR     |
| TMR1       | 0x40000040   | TS_TMR     |
| TMR2       | 0x40000060   | TS_TMR     |
| TMR3       | 0x40000080   | TS_TMR     |
| SYS        | 0x400000A0   | TS_SYS     |
| GPIO_A     | 0x40000100   | TS_GPIO    |
| UART1      | 0x40000200   | TS_UART    |
| I2CM       | 0x40000400   | TS_I2CM    |
| EADC       | 0x40000600   | TS_EADC    |
| EPWM1      | 0x40000700   | TS_EPWM    |

### 9.2 Register Access Pattern

**All registers use bit-field unions**:
```c
typedef union {
    struct {
        uint32_t MODU_EN : 1;   // [0]
        uint32_t LOAD_EN : 1;   // [1]
        uint32_t RST_EN  : 1;   // [2]
        uint32_t INT_EN  : 1;   // [3]
        uint32_t         :12;   // [4:15] Reserved
        uint32_t WDT_CNT :16;   // [16:31]
    } BITS;
    uint32_t WORD;
} TS_WDT_CTRL;

// Access methods:
WDT->CTRL.BITS.MODU_EN = 1;       // Set bit
WDT->CTRL.WORD = 0x00010000;      // Write entire register
uint32_t cnt = WDT->CTRL.BITS.WDT_CNT;  // Read field
```

### 9.3 W1C (Write-1-Clear) Flags

**Many status flags use W1C behavior**:
```c
// To clear interrupt flag:
TMR1->STS_FLAG.WORD = TMR_STS_FLAG_CNT_FLAG_Msk;  // Write 1 to clear

// WRONG:
TMR1->STS_FLAG.BITS.CNT_FLAG = 0;  // Writing 0 has no effect
```

**Common W1C Registers**:
- `WDT->FLAG.BITS.INT_FLAG`
- `TMR->STS_FLAG.BITS.CNT_FLAG`
- `UART->STS_FLAG.BITS.RXEND_FLAG`
- `EADC->FLAG.BITS.DONE_FLAG`

---

## 10. Debugging Tips

### 10.1 Enable Debug Output

**In `config.h` or `debug.h`**:
```c
#define DEBUG_ENABLE      1
#define DEBUG_PORT        UART1

// In code:
#if DEBUG_ENABLE
printk("TMR1 ISR: sys_ticks = %d\n", sys_ticks);
#endif
```

### 10.2 Common Breakpoints

**GDB / C-SKY CDS Debugger**:
- `TMR1_IRQHandler` - Verify 1ms tick
- `default_IRQHandler` - Catch unexpected exceptions
- `soft_wdt_reset` - Identify watchdog resets
- `hal_i2cm_read_one_byte` - Trace I2C errors
- `osal_event_handle` - Monitor task execution

### 10.3 Trace System Hangs

**If system resets unexpectedly**:
1. Check `SYS->OPR_STAT.BITS.RST_SRC`:
   - `1`: Power-on reset
   - `2`: Wake from TMR0
   - `3`: Wake from GPIO
   - Others: Check enum in `regdef.h`
2. Add counters in TMR2 ISR:
   ```c
   static uint32_t wdt_reset_count = 0;
   void __attribute__((isr)) TMR2_IRQHandler(void) {
       tmr2_250ms_int_flag++;
       wdt_reset_count++;
       printk("Soft WDT: %d resets\n", wdt_reset_count);
       soft_wdt_reset();
   }
   ```

---

## 11. File-Function Cross-Reference

| Function                        | File          | Purpose                              |
|---------------------------------|---------------|--------------------------------------|
| `hal_sys_init()`                | sys.c         | Clock/PLL configuration              |
| `hal_vic_init()`                | vic.c         | NVIC enable + priority setup         |
| `hal_timer_init()`              | timer.c       | Configure TMR0-3                     |
| `TMR1_IRQHandler()`             | timer.c       | 1ms system tick ISR                  |
| `hal_uart_init()`               | uart.c        | UART baud rate setup                 |
| `hal_uart_putc()`               | uart.c        | Transmit single byte                 |
| `printk()`                      | printk.c      | Printf-style debug output            |
| `hal_gpio_init()`               | gpio.c        | Pin mux + I/O direction              |
| `hal_i2cm_init()`               | i2cm.c        | I2C clock configuration              |
| `hal_i2cm_read_one_byte()`      | i2cm.c        | I2C register read                    |
| `hal_i2cm_wirte_one_byte()`     | i2cm.c        | I2C register write                   |
| `hal_eadc_init()`               | eadc.c        | Load ADC calibration from flash      |
| `hal_eadc_vref_update()`        | eadc.c        | Sample internal 1.2V reference       |
| `hal_epwm_pwm_start()`          | epwm.c        | Start wireless charging PWM          |
| `hal_tcpc_vbus_is_present()`    | tcpc.c        | Check VBUS > 3.8V                    |
| `hal_tcpc_pd_set_bus_iv()`      | tcpc.c        | Request voltage/current change       |
| `hal_nu6801_buckboost_init()`   | nu6801.c      | Initialize 1S buck-boost IC          |
| `hal_nu6805_buckboost_init()`   | nu6805.c      | Initialize 2S buck-boost IC          |
| `osal_init()`                   | osal.c        | Clear all tasks/timers               |
| `osal_start_system()`           | osal.c        | Enter infinite event loop            |
| `osal_start_timerEx()`          | osal.c        | Start software timer                 |
| `osal_set_event()`              | osal.c        | Post event to task                   |
| `delay_1ms()`                   | delay.c       | Blocking millisecond delay           |

---

## 12. Platform Constraints & Limitations

1. **No RTOS**: Bare-metal with cooperative OSAL scheduler (no preemptive multitasking)
2. **Limited SRAM**: 7KB usable (stack grows from 0x20001FF8 downward)
3. **Flash Wear**: NU171xx flash has limited write cycles (check datasheet for exact value)
4. **I2C Speed**: Maximum 400KHz (hardware limitation)
5. **UART Buffer**: No hardware FIFO, must handle byte-by-byte in ISR
6. **ADC Channels**: EADC supports limited channels (Vref, Vcap, VBUS, IBAT)
7. **Interrupt Latency**: Priority 0 ISR must complete in < 10µs to avoid PD timing violations
8. **TMR2 Watchdog**: Cannot be disabled after boot (must periodically reload)

---

**End of HAL Agent Knowledge Document**

*Last updated: 2026-02-15*
*Total HAL files analyzed: 53 (20 drivers + OSAL + utils + startup)*
*Code base: NU17112 Powerbank Platform*
