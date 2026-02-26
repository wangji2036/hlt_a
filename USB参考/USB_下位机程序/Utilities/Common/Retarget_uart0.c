/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005 Keil Software. All rights reserved.                     */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include "wb7720.h"


__asm(".global __use_no_semihosting");

FILE __stdout;
FILE __stdin;



int fputc(int ch, FILE *f) {
  while(!(UART0->ISR & UART0_ISR_TXE));

  UART0->TDR = (ch & 0x01FF);

  while(!(UART0->ISR & UART0_ISR_TC));

  return ch;
}

int fgetc(FILE *f) {
  while(!(UART0->ISR & UART0_ISR_RXNE));

  return (UART0->RDR & 0x01FF);
}


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
}

int __backspace()
{
  return 0;
}

void _sys_exit(int return_code) {
  while (1);    /* endless loop */
}
