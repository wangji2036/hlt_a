/**************************************************************************//**
 * @file     startup_wb7720.S
 * @brief    CMSIS-Core Device Startup File for
             WB7720 Device Series
 * @version  V0.1.0
 * @date     20-December-2024
 ******************************************************************************/
/*
 * Copyright (c) 2020 - 2023 Westberry Technology (ChangZhou) Corp., Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

                .syntax  unified
                .arch    armv6-m

                .section .vectors
                .align   2
                .globl   __Vectors
                .globl   __Vectors_End
                .globl   __Vectors_Size
__Vectors:
                .long    __StackTop                         /*     Top of Stack */
                .long    Reset_Handler                      /*     Reset Handler */
                .long    NMI_Handler                        /* -14 NMI Handler */
                .long    HardFault_Handler                  /* -13 Hard Fault Handler */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    SVC_Handler                        /*  -5 SVCall Handler */
                .long    0                                  /*     Reserved */
                .long    0                                  /*     Reserved */
                .long    PendSV_Handler                     /*  -2 PendSV Handler */
                .long    SysTick_Handler                    /*  -1 SysTick Handler */

                /* Interrupts */
                .long    BOD_IRQHandler                     /*   0: Brown-Out Detection through EXTI Line detect */
                .long    EXTI1_0_IRQHandler                 /*   1: EXTI Line 1..0 */
                .long    EXTI3_2_IRQHandler                 /*   2: EXTI Line 3..2 */
                .long    FLASH_IRQHandler                   /*   3: FLASH */
                .long    EXTI5_4_IRQHandler                 /*   4: EXTI Line 5..4 */
                .long    EXTI7_6_IRQHandler                 /*   5: EXTI Line 7..6 */
                .long    EXTI15_8_IRQHandler                /*   6: EXTI Line 15..8 */
                .long    UART0_IRQHandler                   /*   7: UART0 */
                .long    UART1_IRQHandler                   /*   8: UART1 */
                .long    SPIM_IRQHandler                    /*   9: SPIM */
                .long    SPI1_IRQHandler                    /*  10: SPI1 */
                .long    I2C_IRQHandler                     /*  11: I2C */
                .long    ARGB_IRQHandler                    /*  12: ARGB */
                .long    TIM6_IRQHandler                    /*  13: TIM6 */
                .long    PCT0_IRQHandler                    /*  14: PCT0 */
                .long    PCT1_IRQHandler                    /*  15: PCT1 */
                .long    PCT2_IRQHandler                    /*  16: PCT2 */
                .long    PCT3_IRQHandler                    /*  17: PCT3 */
                .long    PCT4_IRQHandler                    /*  18: PCT4 */
                .long    USBP_WKUP_IRQHandler               /*  19: USB PIN */
                .long    RTC_IRQHandler                     /*  20: RTC */
                .long    ADC_IRQHandler                     /*  21: ADC */
                .long    USB_IRQHandler                     /*  22: USB */
                .long    CMP0_IRQHandler                    /*  23: CMP0 */
                .long    CMP1_IRQHandler                    /*  24: CMP1 */
                .long    CRS_IRQHandler                     /*  25: CRS */
__Vectors_End:
                .equ     __Vectors_Size, __Vectors_End - __Vectors
                .size    __Vectors, . - __Vectors


                .thumb
                .section .text
                .align   2

                .thumb_func
                .type    Reset_Handler, %function
                .globl   Reset_Handler
Reset_Handler:
                bl       SystemInit

                ldr      r4, =__copy_table_start__
                ldr      r5, =__copy_table_end__

.L_loop0:
                cmp      r4, r5
                bge      .L_loop0_done
                ldr      r1, [r4]
                ldr      r2, [r4, #4]
                ldr      r3, [r4, #8]

.L_loop0_0:
                subs     r3, #4
                blt      .L_loop0_0_done
                ldr      r0, [r1, r3]
                str      r0, [r2, r3]
                b        .L_loop0_0

.L_loop0_0_done:
                adds     r4, #12
                b        .L_loop0

.L_loop0_done:

                ldr      r3, =__zero_table_start__
                ldr      r4, =__zero_table_end__

.L_loop2:
                cmp      r3, r4
                bge      .L_loop2_done
                ldr      r1, [r3]
                ldr      r2, [r3, #4]
                movs     r0, 0

.L_loop2_0:
                subs     r2, #4
                blt      .L_loop2_0_done
                str      r0, [r1, r2]
                b        .L_loop2_0
.L_loop2_0_done:

                adds     r3, #8
                b        .L_loop2
.L_loop2_done:

                bl       _start

                .size    Reset_Handler, . - Reset_Handler


                .thumb_func
                .type    Default_Handler, %function
                .weak    Default_Handler
Default_Handler:
                b        .
                .size    Default_Handler, . - Default_Handler

/* Macro to define default exception/interrupt handlers.
 * Default handler are weak symbols with an endless loop.
 * They can be overwritten by real handlers.
 */
                .macro   Set_Default_Handler  Handler_Name
                .weak    \Handler_Name
                .set     \Handler_Name, Default_Handler
                .endm


/* Default exception/interrupt handler */

                Set_Default_Handler  NMI_Handler
                Set_Default_Handler  HardFault_Handler
                Set_Default_Handler  SVC_Handler
                Set_Default_Handler  PendSV_Handler
                Set_Default_Handler  SysTick_Handler

                Set_Default_Handler  BOD_IRQHandler
                Set_Default_Handler  EXTI1_0_IRQHandler
                Set_Default_Handler  EXTI3_2_IRQHandler
                Set_Default_Handler  FLASH_IRQHandler
                Set_Default_Handler  EXTI5_4_IRQHandler
                Set_Default_Handler  EXTI7_6_IRQHandler
                Set_Default_Handler  EXTI15_8_IRQHandler
                Set_Default_Handler  UART0_IRQHandler
                Set_Default_Handler  UART1_IRQHandler
                Set_Default_Handler  SPIM_IRQHandler
                Set_Default_Handler  SPI1_IRQHandler
                Set_Default_Handler  I2C_IRQHandler
                Set_Default_Handler  ARGB_IRQHandler
                Set_Default_Handler  TIM6_IRQHandler
                Set_Default_Handler  PCT0_IRQHandler
                Set_Default_Handler  PCT1_IRQHandler
                Set_Default_Handler  PCT2_IRQHandler
                Set_Default_Handler  PCT3_IRQHandler
                Set_Default_Handler  PCT4_IRQHandler
                Set_Default_Handler  USBP_WKUP_IRQHandler
                Set_Default_Handler  RTC_IRQHandler
                Set_Default_Handler  ADC_IRQHandler
                Set_Default_Handler  USB_IRQHandler
                Set_Default_Handler  CMP0_IRQHandler
                Set_Default_Handler  CMP1_IRQHandler
                Set_Default_Handler  CRS_IRQHandler


                .end
