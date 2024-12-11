#ifndef WDT_H_
#define WDT_H_

/**
 * @brief 	Enable and configure the Watchdog timer timeout to 1000ms(1 second).
 * @details This function initializes the watchdog timer with a timeout of 1000ms(1 second). 
 * 			- If the watchdog timer is not fed within this time, the system will reset.
 * @param  void
 * @retval void
 */
void hal_wdt_init(void);

/**
 * @brief 	Watchdog timer feed function.
 * @details This function feeds the watchdog timer to prevent a system reset.
 * @param  void
 * @retval void
 */
void hal_wdt_feed(void);

/**
 * @brief 	Stop the watchdog timer.
 * @details This function stops the watchdog timer.
 * @param  	void
 * @retval 	void
 */
void hal_wdt_stop(void);

#endif /* WDT_H_ */
