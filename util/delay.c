#include "delay.h"

void delay_1ms(unsigned int ms)
{
	delay_1us(ms * 1000);
}
