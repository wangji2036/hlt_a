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

#define ONLY7_5W_ENALBE     				(0)

#define PGA_CURRENT_SENSE_ENABLE			1

#define CONFIG_USE_NTC_FOR_CHAGER			0

#define CONFIG_USBPD_ACCEPT_PRSWAP			0

#define CAPACITOR_300_NF         			1

#define OPTION_SAMSUNG_PPDE                       OPTION_ENABLED
#endif /* CONFIG_H_ */
