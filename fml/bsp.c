#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "debug.h"
#include "isr.h"
#include "gui.h"
#include "bsp.h"
#include"sleep.h"

void fml_bsp_init(void)
{
	hal_gpio_init();
	hal_sys_init();
//	hal_timer_init(TMR0);
	hal_timer_init(TMR1);
	hal_timer_init(TMR2);
//	hal_timer_init(TMR3);
	hal_uart_init(UART1);
	hal_vic_init();
	hal_badc_init();
	hal_eadc_init();
	hal_i2cs_init();
	hal_i2cm_init(400000);

// Nu17113, power bank application ,no needed.
/*	hal_bpwm_start(BPWM7, 1500, 1500); //led control, 24KHz
	if (SYS->PID_INFO.BITS.PID == NU17111)
	{
		hal_bpwm_start(BPWM3,  900,  720); //boost control, 40KHz, y = -12.63.18x + 20091
	}
	else
	{
		/hal_bpwm_start(BPWM8,  900,  720); //boost control, 40KHz, y = -12.63.18x + 20091
	}*/

	printk("\r\n chip reset!");
	printk("\r\n ------SOC-> NU%d-A%d", SYS->PID_INFO.BITS.PID, SYS->PID_INFO.BITS.VER);
	//printk("\r\n ------DAT-> %d-%02d-%02d %s", __COMPILE_DATE_YAR, __COMPILE_DATE_MTH, __COMPILE_DATE_DAY, __TIME__);
	printk("\r\n ------VER-> %x.%02x.%02x.%02x", CUST_CODE, PROJ_CODE, PHAS_CODE, TX_FW_VER);
}
