#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                               0x5624	//update 20250325
#define TX_FW_VER                               0x15


#define BATTERY_CV_VALUE					4200
#define CONFIG_NU6801_BATLOW_VOLT			2800 // bat low, bat dead
#define POWERBANK_BUCK_EVK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT					1

#ifdef POWERBANK_BUCK_EVK_V02
#define CONFIG_USBA_SUPPORT					0
#else
#define CONFIG_USBA_SUPPORT					1
#endif


#define BUCKBOOST_USED_NU6805				0
#define BUCKBOOST_USED_NU6801				1

#define CONFIG_NU6801_A0					0 // no not active

#define ONLY7_5W_ENALBE     				0// only effective for 7.5w application, if MPP,keep 0

#define CONFIG_USE_NTC_FOR_CHAGER			1
#define CONFIG_SUPPORT_PPS_CHAGER			0

#define CONFIG_USE_TYPEC_DOUBLE_MOS			1

#define CONFIG_DISCHG_IBAT_LIMIT			0x04 // 0:2A 1:3A 2:4A 3:6A 4:8A 5:10A 6:12A 7:LIMIT OFF
#define CONFIG_USBPD_ACCEPT_PRSWAP			0

#define CONFIG_DEADBATT_SLEEP_SUPPORT		1
#define CONFIG_DEADBATT_VOLTAGE				2900


#define CONFIG_TYPEC_MOS_R					8//   in m ohm,
#define CONFIG_TYPEC_LIGHT_CURRENT			60   // in mA, for light load detect

#define CAPACITOR_300_NF         			1    // 400 or 300 nF, the value will affects the Q and f, for sleep function wake-up
#define ENABLE_EPP_FUNC                     1   //1: enable epp, 0: disable epp
#define OPTION_SAMSUNG_PPDE                 1   // samsung PPDE protocol, 1 to enable.
#define OPTION_FOD_ENABLE                   0   // power transfer FOD enable.
#define SLEEPQ_WAKEUP_ENABLE                1   // sleep Q wake up funtion, set 0 means no sleep Q function. when set 0,the sleep timer
#define SUPPORT_SLEEP_LOG                   1   // sleep print log enable,

/***********Very important for Nuvolata internal engineers!***/
/*
 * * when the new added "macro definition"  changes the files under the "lib flolder"  add the  below, and update the lib for customers .
 *  otherwise, the changes will not effective, due to the "lib" is fixed.
 */
/*********** lib config ***************/

#define CONFIG_TYPECA_SUPPORT				1
#define CONFIG_TYPECB_SUPPORT				1
#define CONFIG_UFCS_SOURCE_SUPPORT			0 // current lib not included, contact nuvolta for support if needed.
#define CONFIG_AFC_SOURCE_SUPPORT			1
#define CONFIG_FCP_SOURCE_SUPPORT			1
#define CONFIG_SCP_SOURCE_SUPPORT			0
#endif /* CONFIG_H_ */
