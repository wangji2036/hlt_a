#ifndef SLP_H_
#define SLP_H_
void SLP_vNormalToSleep(void);
void RST_vCheck(void);
/* 船运模式入口/出口：按键组合或 7 天休眠进入，按键/充电器唤醒退出。 */
void SLP_vEnterShipMode(void);
void SLP_vExitShipMode(void);
uint8_t SLP_u8IsShipMode(void);

#if SUPPORT_SLEEP_LOG
#define sleep_printk printk
#else
#define sleep_printk(...)
#endif

#endif /* SLP_H_ */
