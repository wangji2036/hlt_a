.file "crt0.S"

/*
 * For importing variable or functions from script file "ckcpu.ld".
 */
.import __stack     /* The stack pointer from linking script file */
.import __sbss      /* The end of bss from linking script file    */
.import __ebss      /* The end of bss from linking script file    */
/*
 * For importing variable or functions from entry.S.
 */
.import default_exception_handler /* default exception service routine */

.export __start             /* The system entry. */
.section .exp_table
/* Vector table space. */
.align 10
ck802_vector_table:
.long Reset_Handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
.long default_exception_handler
//----------------------------------------------------------------------
// Interrupts (32 - )
//----------------------------------------------------------------------
.long PROT_IRQHandler           // 0
.long WDT_IRQHandler            // 1
.long TMR0_IRQHandler           // 2
.long TMR1_IRQHandler           // 3
.long TMR2_IRQHandler           // 4
.long TMR3_IRQHandler           // 5
.long EADC_IRQHandler           // 6
.long BADC_IRQHandler           // 7
.long ECAP1_IRQHandler          // 8
.long ECAP2_IRQHandler          // 9
.long ECAP3_IRQHandler          //10
.long ECAP4_IRQHandler          //11
.long ECAP5_IRQHandler          //12
.long GPIO_IRQHandler           //13
.long UART1_IRQHandler          //14
.long UART2_IRQHandler          //15
.long I2CS_IRQHandler           //16
.long I2CM_IRQHandler           //17
.long USBPD_IRQHandler          //18
.long UFCS_IRQHandler           //19
.long DPDM_SINK_IRQHandler      //20
.long DCP_HVDCP_IRQHandler      //21
.long QC_SRC_IRQHandler         //22
.long AFC_SCP_SRC_IRQHandler    //23
.long FSK1_IRQHandler           //24
.long FSK2_IRQHandler           //25
.long default_exception_handler //26
.long DMA_IRQHandler            //27
.long TCPC_IRQHandler           //28
.long default_exception_handler //29
.long default_exception_handler //30
.long default_exception_handler //31


/* The ckcpu startup codes. */
.text
.align 4
    /*
     * This is the codes first entry point. This is where it all
     * begins...
     */
Reset_Handler:
__start:
    /*
     * Init psr value, enable exception, disable interrupt and fast interrupt.
     * psr = 0x80000100
     */
    lrw     a3, 0x80000100
    mtcr    a3, psr

    /*
     * Setup initial vector base table for interrupts and exceptions
     */
    lrw     a2, ck802_vector_table
    mtcr    a2, vbr
    
    /* Initialize the normal stack pointer from the linker definition. */
    lrw     a1, __stack
    mov     sp, a1

    /*
     * Copy data section from it's LMA (_end_rodata) .
     * NOTE: _end_rodata and _start_data must align 4
     */
    lrw     r7, __sdata
    lrw     r6, __edata
    lrw     r5, __erodata

    cmphs   r7, r6
    bt      2f
1:
    ld.w    r4, (r5, 0)
    st.w    r4, (r7, 0)
    addi    r5, 4
    addi    r7, 4
    cmphs   r7, r6
    bf      1b
2:
    /*
     * Zero out the bss region.
     * NOTE: __sbss must align 4
     */
    lrw     a3, __sbss  /* Get start of bss from linking script file */
    lrw     a2, __ebss  /* Get end of bss from linking script file */
    subu    a2, a3      /* Calculate size of bss */
    lsri    a2, 2       /* Size of whole words */
    cmpnei  a2, 0
    bf      __goto_c
    movi    a1, 0       /* Set zero value to write */
2:
    stw     a1, (a3)    /* Zero next word */
    addi    a3, 4       /* Increase bss pointer */
    subi    a2, 1       /* Decrease counter */
    cmpnei  a2, 0
    bt      2b          /* Repeat for all bss */
    
    /*
     * Assember start up done, C codes start here.
     */
__goto_c:
    jbsr    main

    /* Should never get here. */
__exit:
    br      __exit

.export delay_1us
delay_1us:
    cmplti  a0, 1
    bt      .L2
.L1:
    subi    a0, 1
    cmplti  a0, 1
    .rept   18
    nop
    .endr
    bf      .L1
.L2:
    rts

default_exception_handler:
    nie
    ipush
    subi    sp, 4
    stw     lr, (sp, 0x0)

    mfcr    r0, psr         /* Get psr register */
    lsri    r0, 16          /* Get vector in 7 bits */
    zextb   r0              /* Fill upper bytes with zero */
    subi    r0, 32
    mfcr    r1, epc
    mov     r2, sp
    bsr     default_IRQHandler
    ldw     lr, (sp, 0x0)
    addi    sp, 4
    ipop
