ENTRY(__start)

MEMORY
{
    ROM : ORIGIN = 0x00002000, LENGTH = 0x1E000  /* APROM 120K */
    CFG : ORIGIN = 0x20000000, LENGTH = 0x00400  /* GLO_D 1.0K */
    RAM : ORIGIN = 0x20000400, LENGTH = 0x01C00  /*  SRAM 7.0K */
}

PROVIDE (__stack = 0x20002000 - 0x08);

SECTIONS
{
    .text : {
        . = ALIGN(0x4);
        *crt0.o(.exp_table)
        __stext = .;
        *(.text)
        *(.text*)
        *(.init*)
        *(.fini*)
        *(.eh_frame*)
        *(.ctors*)
        *(.dtors*)
        *(.jcr*)
        . = ALIGN(0x4);
        __etext = .;
    } > ROM

    .rodata : {
        . = ALIGN(0x4);
        __srodata = .;
        *(.rodata)
        *(.rodata*)
        *(.rodata.str1.4)
        . = ALIGN(0x4);
        __erodata = .;
    } > ROM

    .data : {
        . = ALIGN(0x4);
        __sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(0x4); 
        KEEP(*(.chip*))
        __edata = .;
    } > RAM AT > ROM

    .bss : {
        . = ALIGN(0x4);
        __sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(0x4);
        __ebss = .;
        __end = .;
    } > RAM
}
