#ifndef NTC_H_
#define NTC_H_

#define CHRG_NTC_UT_VALUE							159	   	// 15.9K
#define CHRG_NTC_UT_RESTORE_VALUE					141		//14.1K
#define CHRG_NTC_OT_VALUE							52	   	//5.25K
#define CHRG_NTC_OT_RESTORE_VALUE					60		//6.03K
#define CHRG_NTC_UT_LOCK_VALUE						252	   	//25.2K
#define CHRG_NTC_UT_LOCK_RESTORE_VALUE				224		//24.4K
#define CHRG_NTC_OT_LOCK_VALUE						44	   	// 4.4K
#define CHRG_NTC_OT_LOCK_RESTORE_VALUE				52		//5.2K

#define DISG_NTC_UT_VALUE							252	   	//25.2K
#define DISG_NTC_UT_RESTORE_VALUE					221		//22.1K
#define DISG_NTC_OT_VALUE							49	   	//4.91K
#define DISG_NTC_OT_RESTORE_VALUE					60		//6.03K
#define DISG_NTC_UT_LOCK_VALUE						631	   	//63.1K
#define DISG_NTC_UT_LOCK_RESTORE_VALUE				520		//52.0K
#define DISG_NTC_OT_LOCK_VALUE						32	   	// 3.2K
#define DISG_NTC_OT_LOCK_RESTORE_VALUE				38		//3.8K

extern bool ntc_ut_flag;
extern bool ntc_ot_flag;
extern bool ntc_stop_chrg_flag;
extern uint8_t ntc_lock_flag;

void buckboost_ntc_handle(void);
#endif
