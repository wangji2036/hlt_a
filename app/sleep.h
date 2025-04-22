#ifndef SLP_H_
#define SLP_H_
void SLP_vNormalToSleep(void);
void RST_vCheck(void);


#if SUPPORT_SLEEP_LOG
	#define sleep_printk 	printk
#else
	#define sleep_printk(...)
#endif


#endif /* SLP_H_ */
