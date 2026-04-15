#ifndef EPP_PORT_H
#define EPP_PORT_H

#include "epp.h"
#include "_wpc.h"



//get power input data use for EPP FOD dectection
// #define EPP_get_power()  xxx  

#define EPP_FSK_Transmit(EPWM1, delay_ms , pattern)  fml_fsk_patt_send(EPWM1, delay_ms, pattern)        //fsk send function

#define EPP_Debug_Print(fmt, args...)  wpc_printk(fmt, ##args)  //debug print function


#endif /* EPP_PORT_H */
