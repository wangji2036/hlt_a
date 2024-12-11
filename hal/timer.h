#ifndef TIMER_H_
#define TIMER_H_

/**
 * @brief Timer 0/1/2/3 initialization. 
 * 		  You can initialize one of the timer according to your needs.
 * @param timer TIMER0/1/2/3
 * @retval void
 * @note  
 * -#	The timer clock source is LIRC, and the timer clock frequency is 64K.
 * -#	You must call this function before using the timer.
 */
void hal_timer_init(TS_TMR *timer);

/**
 * @brief Stops the input timer.
 * @param timer TS_TMR
 * @retval void
 */
void hal_timer_stop(TS_TMR *timer);

extern volatile uint8_t  g_u8Tmr0IntHaved;
extern volatile uint16_t g_u16Tmr0IntCnt;
extern volatile uint8_t g_u8Tmr0IntHaved_USBPD;
extern volatile uint16_t g_u16Tmr0IntCnt_USBPD;

#endif /* TIMER_H_ */
