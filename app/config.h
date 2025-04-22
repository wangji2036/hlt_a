#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                             0x53127	//update 20250325
#define TX_FW_VER                               0x10


#define BATTERY_CV_VALUE					4200
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

#define BUCKBOOST_USED_NU6805				0
#define BUCKBOOST_USED_NU6801				1

#define ONLY7_5W_ENALBE     				0

#define CONFIG_USE_NTC_FOR_CHAGER			0

#define CONFIG_SUPPORT_PPS_CHAGER			0


#define CONFIG_USBPD_ACCEPT_PRSWAP			0

#define CAPACITOR_300_NF         			1
#define ENABLE_EPP_FUNC                     1   //1: enable epp, 0: disable epp
#define OPTION_SAMSUNG_PPDE                 1   // samsung PPDE protocol, 1 to enable.
#define OPTION_FOD_ENABLE                   0   // power transfer FOD enable.
#define SLEEPQ_WAKEUP_ENABLE                1   // sleep Q wake up funtion, set 0 means no sleep Q function. when set 0,the sleep timer
#define SUPPORT_SLEEP_LOG                   1   // sleep print log enable,
#endif /* CONFIG_H_ */
