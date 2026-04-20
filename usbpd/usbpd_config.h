#ifndef USBPD_CONFIG_H_
#define USBPD_CONFIG_H_

#define USBPD_VID		0x37A0
#define USBPD_PID		0x171F
#define USBPD_DID		0x00A1    					//DEVICE ID

#define USBPD_POWER_ROLR_SNK						0x01
#define USBPD_POWER_ROLR_SRC						0x02
#define USBPD_POWER_ROLR_DRP						(USBPD_POWER_ROLR_SRC | USBPD_POWER_ROLR_SNK)

#define TYPEC_PORT_A								0
#define TYPEC_PORT_B								1
#define TYPEC_PORT_MAX_N							2
#define TC_PORT_CCA									0x01
#define TC_PORT_CCB									0x02
#define TC_PORT_CCA_CCB								(TC_PORT_CCA | TC_PORT_CCB)
#define	PD_PORT_MAP									TYPEC_PORT_A //0:CCA    1:CCB


#define CONFIG_USBTC_PORT_SELECT					TC_PORT_CCA_CCB
#define CONFIG_TC_TRY_SINK_SUPPORT_EN				0
#define CONFIG_TC_TRY_SOURCE_SUPPORT_EN				1
#define CONFIG_USBPD_POWER_ROLR						USBPD_POWER_ROLR_DRP

#if(CONFIG_USBPD_POWER_ROLR != USBPD_POWER_ROLR_DRP)
#undef CONFIG_TC_TRY_SINK_SUPPORT_EN
#undef CONFIG_TC_TRY_SOURCE_SUPPORT_EN
#endif

#define SUPPORT_USBPD_LOG

#ifdef SUPPORT_USBPD_LOG
	#define usbpd_printk 	printk
#else
	#define usbpd_printk(...)
#endif

#endif
