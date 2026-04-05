#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE                               0x00
#define PROJ_CODE                               0x00
#define PHAS_CODE                               0x01
#define RELE_DATE                               0x5715	//update 20250325
#define TX_FW_VER                               0x15


#define BATTERY_CV_VALUE					4350
#define CONFIG_NU6801_BATLOW_VOLT			2500 // bat low, bat dead   [NEW-VICTOR]

// Cycle-based CV voltage reduction
#define CONFIG_CYCLE_CV_REDUCTION_ENABLE	1	// 1=enable, 0=disable
#define CYCLE_CV_TIER1_COUNT				68
#define CYCLE_CV_TIER1_OFFSET				100   // CV reduced by 100mV after 68 cycles (4.3V/cell)
#define CYCLE_CV_TIER2_COUNT				135
#define CYCLE_CV_TIER2_OFFSET				150   // CV reduced by 150mV after 135 cycles (4.25V/cell)
#define CYCLE_CV_TIER3_COUNT				200
#define CYCLE_CV_TIER3_OFFSET				200   // CV reduced by 200mV after 200 cycles (4.2V/cell)
#define POWERBANK_BUCK_EVK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT					1

#ifdef POWERBANK_BUCK_EVK_V02
#define CONFIG_USBA_SUPPORT					0
#else
#define CONFIG_USBA_SUPPORT					1
#endif


#define BUCKBOOST_USED_NU6805				1
#define BUCKBOOST_USED_NU6801				0

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
#define ENABLE_EPP_FUNC                     0   //1: enable epp, 0: disable epp
#define OPTION_SAMSUNG_PPDE                 1   // samsung PPDE protocol, 1 to enable.
#define OPTION_FOD_ENABLE                   1   // power transfer FOD enable.
#define SLEEPQ_WAKEUP_ENABLE                0   // sleep Q wake up funtion, set 0 means no sleep Q function. when set 0,the sleep timer
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
#define CONFIG_SCP_SOURCE_SUPPORT			1
/*   ship mode reference */
/***Needs double check in real projects, SHALL!!! make sure the Q and F is calibrated, otherwise the auto wake-up function may be not active **/
//*** For those touch IC wake-up projects can also refer to this ****/
#define CONFIG_SHIP_MODE_ENABLE_DEBUG       0   // ship mode enable

#define SHIP_MODE_CNT  30                       // ship mode 30 times Q wake-up and no RX,

/*----------- New CCC Log Feature (新国标 GB31241) -----------*/
#define CONFIG_NEW_CCC_LOG_ENABLE       1
#define OVER_VOLTAGE_THRESHOLD          4450    // Per-cell OV record threshold (mV)
#define OVER_VOLTAGE_HYSTERESIS         40      // OV recovery hysteresis (mV)
#define OVER_VOLTAGE_FORBID_THRESHOLD   4600    // Per-cell OV permanent forbid (mV), GB31241
#define Lion_Battery_Overcharge_Voltage 4450
#define OVER_VOLTAGE_FORBID_CONSEC_COUNT 5      // 5 consecutive 100ms samples = 500ms
#define OV_FORBID_FLASH_PERSIST         0       // 0=RAM only, cleared by power cycle
#define OV_FORBID_FORCE_CLEAR           1       // 1=erase forbid flag on boot (debug/recovery), set 0 for production
#define UNDER_VOLTAGE_FORBID_THRESHOLD  1200    // Per-cell UV permanent forbid (mV), GB31241
#define UV_FORBID_CONSEC_COUNT          100     // 100 consecutive 100ms samples = 10s
#define CHRG_NTC_OT_TEMP_VALUE          600     // Charging over-temperature threshold (0.1degC = 60.0degC)
#define DISG_NTC_OT_TEMP_VALUE          650     // Discharging over-temperature threshold (0.1degC = 65.0degC)
#define CYCLE_COUNT_FLASH_PERSIST       1       // 1=persist cycle count to Flash
#define CYCLE_COUNT_FLASH_OFFSET        20      // AP_CFG_ROM_ADDR_BASE + 20
#define EXCEPTION_WINDOW_SECONDS        120     // Exception record window (seconds): 120=2min, 3600=1h

#define CONFIG_RTC_USE_CUSTOM_TIME      1
#define CONFIG_RTC_DEFAULT_YEAR         2026
#define CONFIG_RTC_DEFAULT_MONTH        2
#define CONFIG_RTC_DEFAULT_DAY          25
#define CONFIG_RTC_DEFAULT_HOUR         0
#define CONFIG_RTC_DEFAULT_MINUTE       0
#define CONFIG_RTC_DEFAULT_SECOND       0

/*----------- USB Bridge Configuration -----------*/
#define CONFIG_USB_BRIDGE_ENABLE        1       // USB Bridge (WB7720) total switch
#define CONFIG_BATTERY_CAPACITY_MAH     5000    // Rated capacity (mAh)
#define CONFIG_BATTERY_CELL_COUNT       2       // Cell count (2S series)

/*----------- Triple-Click Communication Activation -----------*/
#define CONFIG_TRIPLE_CLICK_COMM_ENABLE 1
#define CONFIG_USB_COMM_LED5_BLINK      1       // LED5 blinks in USB_COM mode (debug)
#define CONFIG_USB_COMM_LED4_WB_STATE   1       // LED4: WB7720 wakeup=solid, sleep=blink
#define CONFIG_USB_COM_FORCE_SINK       1       // Force SINK role in USB_COM for VBUS from phone

#endif /* CONFIG_H_ */
