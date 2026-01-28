#ifndef TYPEC_H_
#define TYPEC_H_

#include "typdef.h"
#include "tcpc.h"
#include "usb_pd.h"
#include "pd_tc.h"
#include "tcpm.h"
#include "usbpd_config.h"
#include "pdlib.h"



#define TC_T_CC_DEBOUNCE	        100	/* 100 - 200 ms */
#define TC_T_PD_DEBOUNCE	        10	/* 10 - 20 ms */
#define TC_T_TRY_CC_DEBOUNCE	    10	/* 10 - 20 ms */
#define TC_T_DRP_TRY		        550	/* 75 - 150 ms */
#define TC_T_DRP_TRYWAIT	        600	/* 400 - 800 ms */
#define TC_T_DRP_TRYTIMEOUT	        800	/* 550 - 1100 ms */
#define TC_T_ERROR_RECOVERY	        500	/* 550 - 1100 ms */

#define usb_tc_substate_e usb_pd_substate_e

struct tc_s
{
	enum usb_tc_state_e usb_tc_state;
	enum usb_pd_substate_e usb_tc_substate;
	enum tc_cc_polarity polarity;
	uint8_t tc_index;
	bool is_deadbattery;
	bool is_in_prswap;
	enum tc_cc_status cc1;
	enum tc_cc_status cc2;
	uint32_t tc_timer_cnt;
	uint8_t try_snk_cnt;
	uint8_t try_src_cnt;
	uint16_t typec_delay_ms;
	uint16_t snk_voltage;
};



struct usb_tc_state_task_t
{
	void (*enter_cb)(struct tc_s * tc);
	void (*exit_cb)(struct tc_s * tc);
};


extern struct tc_s g_tc[];
void usb_tc_init(void);
void usb_tc_run(void);
void usb_tc_set_state(struct tc_s * tc,enum usb_tc_state_e tc_state,enum usb_pd_substate_e tc_substate);
extern volatile uint32_t tc_sys_ticks;
#endif
