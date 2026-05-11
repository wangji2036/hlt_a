#ifndef PRINTK_H_
#define PRINTK_H_

#include "config.h"

void printk(const char *fmt, ...);

#if SUPPORT_LIB_LOG
	#define lib_printk 	printk
#else
	#define lib_printk(...)
#endif

#endif /* PRINTK_H_ */
