#ifndef CONFIG_H_
#define CONFIG_H_

/*---------------------------------- VER -----------------------------------*/
#define CUST_CODE 0x00
#define PROJ_CODE 0x00
#define PHAS_CODE 0x01
#define RELE_DATE 0x5715 //update 20250325
#define TX_FW_VER 0x15

#define BATTERY_CV_VALUE 4400
#define CONFIG_NU6801_BATLOW_VOLT 2500 // bat low, bat dead   [NEW-VICTOR]

// Cycle-based CV voltage reduction (default 4.40V/cell)
#define CONFIG_CYCLE_CV_REDUCTION_ENABLE 1 // 1=enable, 0=disable
#define CYCLE_CV_TIER1_COUNT 141           // cycles > 140 -> 4.35V/cell
#define CYCLE_CV_TIER1_OFFSET 50
#define CYCLE_CV_TIER2_COUNT 211 // cycles > 210 -> 4.30V/cell
#define CYCLE_CV_TIER2_OFFSET 100
#define POWERBANK_BUCK_EVK_V02

/*.....7.5w Debug......*/
#define CONFIG_WPC_SUPPORT 1

#ifdef POWERBANK_BUCK_EVK_V02
#define CONFIG_USBA_SUPPORT 0
#else
#define CONFIG_USBA_SUPPORT 1
#endif

#define BUCKBOOST_USED_NU6805 1
#define BUCKBOOST_USED_NU6801 0

#define CONFIG_NU6801_A0 0 // no not active

#define ONLY7_5W_ENALBE 0 // only effective for 7.5w application, if MPP,keep 0

#define CONFIG_USE_NTC_FOR_CHAGER 1
#define CONFIG_SUPPORT_PPS_CHAGER 0

#define CONFIG_USE_TYPEC_DOUBLE_MOS 1

#define CONFIG_DISCHG_IBAT_LIMIT 0x04 // 0:2A 1:3A 2:4A 3:6A 4:8A 5:10A 6:12A 7:LIMIT OFF
#define CONFIG_USBPD_ACCEPT_PRSWAP 0

#define CONFIG_DEADBATT_SLEEP_SUPPORT 1
#define CONFIG_DEADBATT_VOLTAGE 2900

#define CONFIG_TYPEC_MOS_R 8          //   in m ohm,
#define CONFIG_TYPEC_LIGHT_CURRENT 60 // in mA, for light load detect

#define CAPACITOR_300_NF 1     // 400 or 300 nF, the value will affects the Q and f, for sleep function wake-up
#define ENABLE_EPP_FUNC 0      //1: enable epp, 0: disable epp
#define OPTION_SAMSUNG_PPDE 1  // samsung PPDE protocol, 1 to enable.
#define OPTION_FOD_ENABLE 1    // power transfer FOD enable.
#define SLEEPQ_WAKEUP_ENABLE 0 // sleep Q wake up funtion, set 0 means no sleep Q function. when set 0,the sleep timer
#define SUPPORT_SLEEP_LOG 1    // sleep print log enable,

/***********Very important for Nuvolata internal engineers!***/
/*
 * * when the new added "macro definition"  changes the files under the "lib flolder"  add the  below, and update the lib for customers .
 *  otherwise, the changes will not effective, due to the "lib" is fixed.
 */
/*********** lib config ***************/

#define CONFIG_TYPECA_SUPPORT 1
#define CONFIG_TYPECB_SUPPORT 0
#define CONFIG_UFCS_SOURCE_SUPPORT 0 // current lib not included, contact nuvolta for support if needed.
#define CONFIG_AFC_SOURCE_SUPPORT 1
#define CONFIG_FCP_SOURCE_SUPPORT 1
#define CONFIG_SCP_SOURCE_SUPPORT 1
/*   ship mode reference */
/***Needs double check in real projects, SHALL!!! make sure the Q and F is calibrated, otherwise the auto wake-up function may be not active **/
//*** For those touch IC wake-up projects can also refer to this ****/
#define CONFIG_SHIP_MODE_ENABLE_DEBUG 0 // ship mode enable

#define SHIP_MODE_CNT 30                                   // 船运模式锁存标志值
#define SHIP_MODE_SLEEP_SECONDS (7UL * 24UL * 60UL * 60UL) // 连续休眠 7 天进入船运
#define SHIP_MODE_KEY_HOLD_10MS_TICKS 800                  // 短按一次后再长按 8s 进入船运
#define SHIP_MODE_KEY_ARM_10MS_TICKS 300                   // 短按后 3s 内开始第二次长按
#define SHIP_MODE_LED_BLINK_TICKS 10                       // 双色灯 on/off * 5 次，ui_update 周期 250ms
#define SHIP_MODE_LED_MASK 0x30                            // LED5 + LED6 作为双色灯显示

/*----------- New CCC Log Feature (新国标 GB31241) -----------*/
#define CONFIG_NEW_CCC_LOG_ENABLE 1

// Debug log switches (set to 0 to reduce Flash) -- synced with NF platform
#define SUPPORT_BUCKBOOST_LOG 1                // buckboost.c debug logs
#define SUPPORT_BAT_LOG 1                      // bat.c debug logs
#define SUPPORT_XGB_LOG 1                      // XGB/bat_record/usb_bridge debug logs
#define SUPPORT_BAT_RECORD_LOG SUPPORT_XGB_LOG // legacy alias
#define SUPPORT_GDATA_LOG 1                    // g_data.c debug logs
#define SUPPORT_MAIN_LOG 1                     // main.c debug logs
#define SUPPORT_GUI_LOG 0                      // disabled to save ROM (~1.5KB)
#define SUPPORT_TCPM_LOG 1                     // tcpm.c debug logs
#define SUPPORT_DPDM_LOG 1                     // dpdm.c debug logs
#define SUPPORT_QC_LOG 1                       // usb_qc.c debug logs
#define SUPPORT_LIB_LOG 0                      // non-WPC lib debug logs
#define SUPPORT_PORTMGR_LOG 1                  // port_manager.c debug logs
#define SUPPORT_LED_LOG 0                      // led.c debug logs
#define SUPPORT_NTC_LOG 0                      // ntc.c debug logs
#define SUPPORT_WPC_LOG 0                      // WPC 协议域 (epp/fod/pid/qfod/_wpc/wpc_*/fsk/nu103x/qdt/ask/pfod) debug logs
#define BAT_RECORD_USE_STACK_BUFFER 0          // 0=static buffer (safe), 1=local variable (saves RAM)
#define OVER_VOLTAGE_THRESHOLD 4450            // Per-cell OV record threshold (mV)
#define OVER_VOLTAGE_HYSTERESIS 40             // OV recovery hysteresis (mV)
#define OVER_VOLTAGE_FORBID_THRESHOLD 4800     // Per-cell OV permanent forbid (mV), GB31241
#define Lion_Battery_Overcharge_Voltage 4450
#define OVER_VOLTAGE_FORBID_CONSEC_COUNT 10 // 5 consecutive 100ms samples = 500ms
#define OV_FORBID_FLASH_PERSIST 1           // 0=RAM only, cleared by power cycle
#define OV_FORBID_FORCE_CLEAR 0             // 1=erase forbid flag on boot (debug/recovery), set 0 for production
#define OV_FORBID_KEY_CLEAR_ENABLE 0        // 1=single click clears OV forbid, 0=only power cycle clears
#define CHRG_NTC_OT_TEMP_VALUE 600          // Charging over-temperature threshold (0.1degC = 60.0degC)
#define DISG_NTC_OT_TEMP_VALUE 650          // Discharging over-temperature threshold (0.1degC = 65.0degC)
#define CYCLE_COUNT_FLASH_PERSIST 1         // 1=persist cycle count to Flash
#define CYCLE_COUNT_FLASH_OFFSET 20         // AP_CFG_ROM_ADDR_BASE + 20
#define VREF_FLASH_OFFSET 24                // AP_CFG_ROM_ADDR_BASE + 24
#define VREF_DEFAULT_MV 3270                // Default Vref before calibration
#define EXCEPTION_WINDOW_SECONDS 180       //180     // Exception record window (seconds): 120=2min, 3600=1h
#define Cali_Vref 1
#define CONFIG_RTC_USE_CUSTOM_TIME 1
#define CONFIG_RTC_DEFAULT_YEAR 2026
#define CONFIG_RTC_DEFAULT_MONTH 2
#define CONFIG_RTC_DEFAULT_DAY 25
#define CONFIG_RTC_DEFAULT_HOUR 0
#define CONFIG_RTC_DEFAULT_MINUTE 0
#define CONFIG_RTC_DEFAULT_SECOND 0

/*----------- USB Bridge Configuration -----------*/
#define CONFIG_USB_BRIDGE_ENABLE 1       // USB Bridge (WB7720) total switch
#define CONFIG_WLS_NTC_FAIL_MASK 1       // 1=屏蔽WB7720 I2C故障(调试用), 0=故障停WPC
#define CONFIG_BATTERY_CAPACITY_MAH 5000 // Rated capacity (mAh)
#define CONFIG_BATTERY_CELL_COUNT 2      // Cell count (2S series)

/*----------- Triple-Click Communication Activation -----------*/
#define CONFIG_TRIPLE_CLICK_COMM_ENABLE 1
#define CONFIG_USB_COMM_LED5_BLINK 1    // LED5 blinks in USB_COM mode (debug)
#define CONFIG_USB_COMM_LED4_WB_STATE 1 // LED4: WB7720 wakeup=solid, sleep=blink
#define CONFIG_USB_COM_FORCE_SINK 1     // Force SINK role in USB_COM for VBUS from phone

#endif /* CONFIG_H_ */
