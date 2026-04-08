#ifndef LED_H_
#define LED_H_

#include "config.h"

#if SUPPORT_LED_LOG
	#define led_printk 	printk
#else
	#define led_printk(...)
#endif
#define _UI_PIN1_PORT     GPA
#define _UI_PIN2_PORT     GPC
#define _UI_PIN3_PORT     GPC
#define _UI_PIN4_PORT     GPC
#define _UI_PIN5_PORT     GPB

#define PORT_GPA          GPA
#define PORT_GPB          GPB

#define _UI_PIN1_PINx     PIN4
#define _UI_PIN2_PINx     PIN5
#define _UI_PIN3_PINx     PIN4
#define _UI_PIN4_PINx     PIN3
#define _UI_PIN5_PINx     PIN3

#define _KEY_PORT    GPC
#define _KEY_PINx    PIN6
#define _PIN_LEVEL_HI     (1)
#define _PIN_LEVEL_LO     (0)
#define _KEY_LEVEL    (_KEY_PORT->D_IN.BITS._KEY_PINx)
// for long press and click time definition, can update according to real application
#define LONG_PRESS_TIME_MS 3000
#define DOUBLE_CLICK_TIME_MS 1000
#define _SET_ALL_PINS_IN_PUT() do{\
		_UI_PIN1_PORT-> O_EN.BITS._UI_PIN1_PINx = 0; _UI_PIN1_PORT->I_EN.BITS._UI_PIN1_PINx = 1;\
		_UI_PIN2_PORT-> O_EN.BITS._UI_PIN2_PINx = 0; _UI_PIN2_PORT->I_EN.BITS._UI_PIN2_PINx = 1;\
		_UI_PIN3_PORT-> O_EN.BITS._UI_PIN3_PINx = 0; _UI_PIN3_PORT->I_EN.BITS._UI_PIN3_PINx = 1;\
		_UI_PIN4_PORT-> O_EN.BITS._UI_PIN4_PINx = 0; _UI_PIN4_PORT->I_EN.BITS._UI_PIN4_PINx = 1;\
		_UI_PIN5_PORT-> O_EN.BITS._UI_PIN5_PINx = 0; _UI_PIN5_PORT->I_EN.BITS._UI_PIN5_PINx = 1;\
} while(0)

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

#define UI_EVENT_STATE_CHANGE		0x01ul << 0
#define UI_EVENT_KEY_CLICK			0x01ul << 1
#define UI_EVENT_FAULT				0x01ul << 2
#define UI_EVENT_KEY_LONG			0x01ul << 3
#define UI_EVENT_QDT_CALI_START     0x01ul << 4
#define UI_EVENT_QDT_CALI_SUCCESS   0x01ul << 5
#define UI_EVENT_BAT_FULL            0x01ul << 6

#define KEY_UI_DISPLAY_TICKS         20

extern uint8_t ui_evt;
void ui_update(void);
void ui_display (void);
void led_init(void);
void led_display(void);
void detectSingleKey(void);
void initKey(void);
void led_all_off_before_sleep(void);
#endif /* LED_H_ */
