/**
 * @mainpage  NU171XX MPP Transmitter User Guide Documentation
 *
 * **NU171xx is a high efficiency, high integration Qi 2.x compliant transmitter IC for wireless charging with input voltage from 3V to 21V.** <br>
 * It integrates low Rdson Full bridge FETs and gate driver, 32bit 36MHz MCU core, 8kB SRAM and 128kB Flash, and fast charging block TCPC/PD/BC1.2/QC/FCP/ SCP/AFC. 
 *
 * # Introduction
 * This project is aimed to develop a high-efficiency, high-integration Qi 2.x compliant transmitter IC for wireless charging with input voltage from 3V to 21V. <br> 
 * The project is developed using the C-Sky Development Suite for C-SKY CDS C/C++ Developers (V5.2.14 B20231027) development environment and the MPP-TX board.
 *
 * # Features
 * - Core: 
 *   - 32-bit CK802 core running up to 36 MHz
 *   - Built-in Nested Vectored Interrupt Controller(NVIC)
 * - Memory: 
 *   - 8KB SRAM
 *   - 64KB Flash
 * - Input Voltage:
 *   - 3V to 21V input voltage
 * - Charging Block:
 *   - TCPC/PD/BC1.2/QC/FCP/SCP/AFC
 * - Package:
 *   - LQFP-100 package
 * - Dimensions:
 *   - 3.5X4.5
 * - Interface:
 *   - 2*UARTs
 *   - 12*GPIOs
 *   - I2C Slave:100K/400KHz
 * - Temperature:
 *   - -40°C to 85°
 * - Operating Voltage:
 *   - 2.8V
 * - Operating Frequency:
 *   - 36MHz
 * - Protocal Wire Charging:
 *   - PD Sink
 * - Qi 2.x Compliance:
 *   - Qi 2.0
 * - Development Kit:
 *   - C-SKY CDS C/C++ Developers (V5.2.14 B20231027)
 *
 * # Getting Started
 * Follow these steps to get started with the project:
 * 1. Please download the CK-CPU C/C++ Developers ( V5.2.14 B20231027) development environment from the official website.\n
 *    - https://www.xrvm.cn/community/download
 * 2. Please check with AE team for the MPP-TX board and get the datasheet.
 *
 * # API Reference
 * For detailed information on the project's API, You can refer to the following modules and drivers documentation.
 *  # Table of Contents Peripheral Modules Drivers Documentation
 * - [GPIO](@ref gpio.c)
 * - [UART](@ref uart.c)
 * - [I2S](@ref i2cs.c)
 * - [Watchdog Timer](@ref wdt.c)
 * - [Timer](@ref timer.c)
 * - [Flash](@ref fmc.c)
 * - [System](@ref sys.c)
 * - [Vector Table](@ref vic.c)
 * - [Isr](@ref isr.c)
 * - [BADC](@ref badc.c)
 * - [EADC](@ref eadc.c)
 * - [ECAP](@ref ecap.c)
 * - [PWM](@ref bpwm.c)
 * - [EPWM](@ref epwm.c)
 * - [FSK](@ref fml/fsk.c)
 * - [ASK](@ref fml/ask.c)
*/