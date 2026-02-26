#ifndef __BSP_UART0_H
#define __BSP_UART0_H
#include "wb7720.h"


void uart0_init(uint32_t apbclk, uint32_t baud);
void uart0_send(const uint8_t* buffer, uint32_t length);



#endif

