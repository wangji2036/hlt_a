;/**************************************************************************//**
; * @file     startup_wb7720.s
; * @brief    CMSIS Core Device Startup File for
; *           WB7720 Device Series
; * @version  V0.1.1
; * @date     13-January-2025
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2020 - 2023 Westberry Technology (ChangZhou) Corp., Ltd.
;   All rights reserved.
;  
;   SPDX-License-Identifier: Apache-2.0
;  
;   Licensed under the Apache License, Version 2.0 (the License); you may
;   not use this file except in compliance with the License.
;   You may obtain a copy of the License at
;  
;   www.apache.org/licenses/LICENSE-2.0
;  
;   Unless required by applicable law or agreed to in writing, software
;   distributed under the License is distributed on an AS IS BASIS, WITHOUT
;   WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
;   See the License for the specific language governing permissions and
;   limitations under the License.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     BOD_IRQHandler            ; 16+0:  Brown-Out Detection through EXTI Line detect
                DCD     EXTI_Internal_IRQHandler  ; 16+1:  EXTI Line 1..0
                DCD     EXTI3_2_IRQHandler        ; 16+2:  EXTI Line 3..2
                DCD     FLASH_IRQHandler          ; 16+3:  FLASH
                DCD     EXTI5_4_IRQHandler        ; 16+4:  EXTI Line 5..4
                DCD     EXTI7_6_IRQHandler        ; 16+5:  EXTI Line 7..6
                DCD     EXTI15_8_IRQHandler       ; 16+6:  EXTI Line 15..8
                DCD     UART0_IRQHandler          ; 16+7:  UART0
                DCD     UART1_IRQHandler          ; 16+8:  UART1
                DCD     SPIM_IRQHandler           ; 16+9:  SPIM
                DCD     SPI1_IRQHandler           ; 16+10: SPI1
                DCD     I2C_IRQHandler            ; 16+11: I2C
                DCD     ARGB_IRQHandler           ; 16+12: ARGB
                DCD     TIM_Internal_IRQHandler   ; 16+13: TIM6
                DCD     PCT0_IRQHandler           ; 16+14: PCT0
                DCD     PCT1_IRQHandler           ; 16+15: PCT1
                DCD     PCT2_IRQHandler           ; 16+16: PCT2
                DCD     PCT3_IRQHandler           ; 16+17: PCT3
                DCD     PCT4_IRQHandler           ; 16+18: PCT4
                DCD     USBP_WKUP_IRQHandler      ; 16+19: USB PIN
                DCD     RTC_IRQHandler            ; 16+20: RTC
                DCD     ADC_IRQHandler            ; 16+21: ADC
                DCD     USB_IRQHandler            ; 16+22: USB
                DCD     CMP0_IRQHandler           ; 16+23: CMP0
                DCD     CMP1_IRQHandler           ; 16+24: CMP1
                DCD     CRS_IRQHandler            ; 16+25: CRS
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT  BOD_IRQHandler            [WEAK]
                EXPORT  EXTI_Internal_IRQHandler  [WEAK]
                EXPORT  EXTI3_2_IRQHandler        [WEAK]
                EXPORT  FLASH_IRQHandler          [WEAK]
                EXPORT  EXTI5_4_IRQHandler        [WEAK]
                EXPORT  EXTI7_6_IRQHandler        [WEAK]
                EXPORT  EXTI15_8_IRQHandler       [WEAK]
                EXPORT  UART0_IRQHandler          [WEAK]
                EXPORT  UART1_IRQHandler          [WEAK]
                EXPORT  SPIM_IRQHandler           [WEAK]
                EXPORT  SPI1_IRQHandler           [WEAK]
                EXPORT  I2C_IRQHandler            [WEAK]
                EXPORT  ARGB_IRQHandler           [WEAK]
                EXPORT  TIM_Internal_IRQHandler   [WEAK]
                EXPORT  PCT0_IRQHandler           [WEAK]
                EXPORT  PCT1_IRQHandler           [WEAK]
                EXPORT  PCT2_IRQHandler           [WEAK]
                EXPORT  PCT3_IRQHandler           [WEAK]
                EXPORT  PCT4_IRQHandler           [WEAK]
                EXPORT  USBP_WKUP_IRQHandler      [WEAK]
                EXPORT  RTC_IRQHandler            [WEAK]
                EXPORT  ADC_IRQHandler            [WEAK]
                EXPORT  USB_IRQHandler            [WEAK]
                EXPORT  CMP0_IRQHandler           [WEAK]
                EXPORT  CMP1_IRQHandler           [WEAK]
                EXPORT  CRS_IRQHandler            [WEAK]


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

                B       .

                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
