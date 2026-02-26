;/**************************************************************************//**
; * @file     startup_wb7720.s
; * @brief    CMSIS Core Device Startup File for
; *           WB7720 Device Series
; * @version  V0.1.1
; * @date     13-January-2025
; ******************************************************************************/
;/*
; * Copyright (c) 2020 - 2023 Westberry Technology (ChangZhou) Corp., Ltd. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

;
; The modules in this file are included in the libraries, and may be replaced
; by any user-defined modules that define the PUBLIC symbol _program_start or
; a user defined start symbol.
; To override the cstartup defined in the library, simply add your modified
; version to the workbench project.
;
; The vector table is normally located at address 0.
; When debugging in RAM, it can be located in RAM, aligned to at least 2^6.
; The name "__vector_table" has special meaning for C-SPY:
; it is where the SP start value is found, and the NVIC vector
; table register (VTOR) is initialized to this address if != 0.
;
; Cortex-M version
;

                MODULE   ?cstartup

                ;; Forward declaration of sections.
                SECTION  CSTACK:DATA:NOROOT(3)

                SECTION  .intvec:CODE:NOROOT(2)

                EXTERN   __iar_program_start
                EXTERN   SystemInit
                PUBLIC   __vector_table
                PUBLIC   __vector_table_0x1c
                PUBLIC   __Vectors
                PUBLIC   __Vectors_End
                PUBLIC   __Vectors_Size

                DATA

__vector_table
                DCD      sfe(CSTACK)                         ;     Top of Stack
                DCD      Reset_Handler                       ;     Reset Handler
                DCD      NMI_Handler                         ; -14 NMI Handler
                DCD      HardFault_Handler                   ; -13 Hard Fault Handler
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
__vector_table_0x1c
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      SVC_Handler                         ;  -5 SVCall Handler
                DCD      0                                   ;     Reserved
                DCD      0                                   ;     Reserved
                DCD      PendSV_Handler                      ;  -2 PendSV Handler
                DCD      SysTick_Handler                     ;  -1 SysTick Handler

                ; Interrupts
                DCD      BOD_IRQHandler                      ;   0 Brown-Out Detection through EXTI Line detect
                DCD      EXTI_Internal_IRQHandler            ;   1 EXTI Line 1..0
                DCD      EXTI3_2_IRQHandler                  ;   2 EXTI Line 3..2
                DCD      FLASH_IRQHandler                    ;   3 FLASH
                DCD      EXTI5_4_IRQHandler                  ;   4 EXTI Line 5..4
                DCD      EXTI7_6_IRQHandler                  ;   5 EXTI Line 7..6
                DCD      EXTI15_8_IRQHandler                 ;   6 EXTI Line 15..8
                DCD      UART0_IRQHandler                    ;   7 UART0
                DCD      UART1_IRQHandler                    ;   8 UART1
                DCD      SPIM_IRQHandler                     ;   9 SPIM
                DCD      SPI1_IRQHandler                     ;  10 SPI1
                DCD      I2C_IRQHandler                      ;  11 I2C
                DCD      ARGB_IRQHandler                     ;  12 ARGB
                DCD      TIM_Internal_IRQHandler             ;  13 TIM6
                DCD      PCT0_IRQHandler                     ;  14 PCT0
                DCD      PCT1_IRQHandler                     ;  15 PCT1
                DCD      PCT2_IRQHandler                     ;  16 PCT2
                DCD      PCT3_IRQHandler                     ;  17 PCT3
                DCD      PCT4_IRQHandler                     ;  18 PCT4
                DCD      USBP_WKUP_IRQHandler                ;  19 USB PIN
                DCD      RTC_IRQHandler                      ;  20 RTC
                DCD      ADC_IRQHandler                      ;  21 ADC
                DCD      USB_IRQHandler                      ;  22 USB
                DCD      CMP0_IRQHandler                     ;  23 CMP0
                DCD      CMP1_IRQHandler                     ;  24 CMP1
                DCD      CRS_IRQHandler                      ;  25 CRS
__Vectors_End

__Vectors       EQU      __vector_table
__Vectors_Size  EQU      __Vectors_End - __Vectors


                THUMB

; Reset Handler

                PUBWEAK  Reset_Handler
                SECTION  .text:CODE:REORDER:NOROOT(2)
Reset_Handler
                LDR      R0, =SystemInit
                BLX      R0
                LDR      R0, =__iar_program_start
                BX       R0


                PUBWEAK NMI_Handler
                PUBWEAK HardFault_Handler
                PUBWEAK SVC_Handler
                PUBWEAK PendSV_Handler
                PUBWEAK SysTick_Handler

                PUBWEAK BOD_IRQHandler
                PUBWEAK EXTI_Internal_IRQHandler
                PUBWEAK EXTI3_2_IRQHandler
                PUBWEAK FLASH_IRQHandler
                PUBWEAK EXTI5_4_IRQHandler
                PUBWEAK EXTI7_6_IRQHandler
                PUBWEAK EXTI15_8_IRQHandler
                PUBWEAK UART0_IRQHandler
                PUBWEAK UART1_IRQHandler
                PUBWEAK SPIM_IRQHandler
                PUBWEAK SPI1_IRQHandler
                PUBWEAK I2C_IRQHandler
                PUBWEAK ARGB_IRQHandler
                PUBWEAK TIM_Internal_IRQHandler
                PUBWEAK PCT0_IRQHandler
                PUBWEAK PCT1_IRQHandler
                PUBWEAK PCT2_IRQHandler
                PUBWEAK PCT3_IRQHandler
                PUBWEAK PCT4_IRQHandler
                PUBWEAK USBP_WKUP_IRQHandler
                PUBWEAK RTC_IRQHandler
                PUBWEAK ADC_IRQHandler
                PUBWEAK USB_IRQHandler
                PUBWEAK CMP0_IRQHandler
                PUBWEAK CMP1_IRQHandler
                PUBWEAK CRS_IRQHandler
                SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
HardFault_Handler
SVC_Handler
PendSV_Handler
SysTick_Handler

BOD_IRQHandler
EXTI_Internal_IRQHandler
EXTI3_2_IRQHandler
FLASH_IRQHandler
EXTI5_4_IRQHandler
EXTI7_6_IRQHandler
EXTI15_8_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
SPIM_IRQHandler
SPI1_IRQHandler
I2C_IRQHandler
ARGB_IRQHandler
TIM_Internal_IRQHandler
PCT0_IRQHandler
PCT1_IRQHandler
PCT2_IRQHandler
PCT3_IRQHandler
PCT4_IRQHandler
USBP_WKUP_IRQHandler
RTC_IRQHandler
ADC_IRQHandler
USB_IRQHandler
CMP0_IRQHandler
CMP1_IRQHandler
CRS_IRQHandler
                B        .


                END
