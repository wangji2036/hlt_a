#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                             0x5310	//update 20250219
#define TX_FW_VER                               0x08

#define POWERBANK_BACK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT					1

#ifdef POWERBANK_BACK_V02
#define CONFIG_USBA_SUPPORT					0
#else
#define CONFIG_USBA_SUPPORT					1
#endif
#define CONFIG_TYPECA_SUPPORT				1
#define CONFIG_TYPECB_SUPPORT				1
#define ONLY7_5W_ENALBE     				(1)

#define CONFIG_USE_NTC_FOR_CHAGER					0


#endif /* CONFIG_H_ */
