#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                             0x5310	//update 20250219
#define TX_FW_VER                               0x08

#define POWERBANK_BUCK_EVK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT					1

#ifdef POWERBANK_BUCK_EVK_V02
#define CONFIG_USBA_SUPPORT					0
#else
#define CONFIG_USBA_SUPPORT					1
#endif
#define CONFIG_TYPECA_SUPPORT				1
#define CONFIG_TYPECB_SUPPORT				1

#define BUCKBOOST_USED_SW7201		0
#define BUCKBOOST_USED_NU6801		1

#define ONLY7_5W_ENALBE     (0)

#define PGA_CURRENT_SENSE_ENABLE		1

#define CONFIG_USE_NTC_FOR_CHAGER					0

#define CHRG_NTC_UT_VALUE							159	   // 13??C 15.9K ?米1|?那
#define CHRG_NTC_UT_RESTORE_VALUE					141		//16??C 14.1K ???∩豕?1|?那
#define CHRG_NTC_OT_VALUE							52	   // 43??C 5.25K ?米1|?那
#define CHRG_NTC_OT_RESTORE_VALUE					60		//39??C 6.03K ???∩豕?1|?那
#define CHRG_NTC_UT_LOCK_VALUE						252	   	//2??C 25.2K 	㊣㏒?∟
#define CHRG_NTC_UT_LOCK_RESTORE_VALUE				224		//7??C 24.4K 	???∩
#define CHRG_NTC_OT_LOCK_VALUE						44	   // 48??C 4.4K ㊣㏒?∟
#define CHRG_NTC_OT_LOCK_RESTORE_VALUE				52		//43??C 5.2K ???∩

#define DISG_NTC_UT_VALUE							252	   // 2??C 25.2K ?米1|?那
#define DISG_NTC_UT_RESTORE_VALUE					221		//5??C 22.1K ???∩豕?1|?那
#define DISG_NTC_OT_VALUE							49	   // 45??C 4.91K ?米1|?那
#define DISG_NTC_OT_RESTORE_VALUE					60		//39??C 6.03K ???∩豕?1|?那
#define DISG_NTC_UT_LOCK_VALUE						631	   	// 	-18??C 63.1K ㊣㏒?∟
#define DISG_NTC_UT_LOCK_RESTORE_VALUE				520		// 	-14??C 52.0K ???∩
#define DISG_NTC_OT_LOCK_VALUE						32	   // 58??C 3.2K ㊣㏒?∟
#define DISG_NTC_OT_LOCK_RESTORE_VALUE				38		//53??C 3.8K ???∩


#endif /* CONFIG_H_ */
