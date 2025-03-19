#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                             0x5310	//update 20250219
#define TX_FW_VER                               0x08

//#define POWERBANK_BACK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT					1

#ifdef POWERBANK_BACK_V02
#define CONFIG_USBA_SUPPORT					0
#else
#define CONFIG_USBA_SUPPORT					1
#endif
#define CONFIG_TYPECA_SUPPORT				1
#define CONFIG_TYPECB_SUPPORT				1
#define ONLY7_5W_ENALBE     (0)

#define CONFIG_USE_NTC_FOR_CHAGER					0

#define CHRG_NTC_UT_VALUE							159	   // 13¡ãC 15.9K ½µ¹¦ÂÊ
#define CHRG_NTC_UT_RESTORE_VALUE					141		//16¡ãC 14.1K »Ö¸´È«¹¦ÂÊ
#define CHRG_NTC_OT_VALUE							52	   // 43¡ãC 5.25K ½µ¹¦ÂÊ
#define CHRG_NTC_OT_RESTORE_VALUE					60		//39¡ãC 6.03K »Ö¸´È«¹¦ÂÊ
#define CHRG_NTC_UT_LOCK_VALUE						252	   	//2¡ãC 25.2K 	±£»¤
#define CHRG_NTC_UT_LOCK_RESTORE_VALUE				224		//7¡ãC 24.4K 	»Ö¸´
#define CHRG_NTC_OT_LOCK_VALUE						44	   // 48¡ãC 4.4K ±£»¤
#define CHRG_NTC_OT_LOCK_RESTORE_VALUE				52		//43¡ãC 5.2K »Ö¸´

#define DISG_NTC_UT_VALUE							252	   // 2¡ãC 25.2K ½µ¹¦ÂÊ
#define DISG_NTC_UT_RESTORE_VALUE					221		//5¡ãC 22.1K »Ö¸´È«¹¦ÂÊ
#define DISG_NTC_OT_VALUE							49	   // 45¡ãC 4.91K ½µ¹¦ÂÊ
#define DISG_NTC_OT_RESTORE_VALUE					60		//39¡ãC 6.03K »Ö¸´È«¹¦ÂÊ
#define DISG_NTC_UT_LOCK_VALUE						631	   	// 	-18¡ãC 63.1K ±£»¤
#define DISG_NTC_UT_LOCK_RESTORE_VALUE				520		// 	-14¡ãC 52.0K »Ö¸´
#define DISG_NTC_OT_LOCK_VALUE						32	   // 58¡ãC 3.2K ±£»¤
#define DISG_NTC_OT_LOCK_RESTORE_VALUE				38		//53¡ãC 3.8K »Ö¸´


#endif /* CONFIG_H_ */
