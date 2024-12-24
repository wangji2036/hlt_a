#ifndef LED_H_
#define LED_H_

typedef enum {
	ELED_STATE_NULL     = 0,
	ELED_STATE_POWERON  = 1,
	ELED_STATE_STANDBY  = 2,
	ELED_STATE_CHARGING = 3,
	ELED_STATE_CHARGED  = 4,
	ELED_STATE_ERROR    = 5,
} TE_LED_STATE;
extern TE_LED_STATE g_eLedState;

enum led_state_t {
	ELED_STS_NULL     = 0,
	ELED_STS_POWERON  = 1,
	ELED_STS_STANDBY  = 2,
	ELED_STS_CHARGING = 3,
	ELED_STS_CHARGED  = 4,
	ELED_STS_ERROR    = 5,
};
void ui_update(void);
void ui_display (void);
void led_init(void);
void led_display(void);

#endif /* LED_H_ */
