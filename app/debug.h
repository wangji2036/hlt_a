#ifndef DEBUG_H_
#define DEBUG_H_

#define DEBUG_PORT    UART1
//#define DEBUG_PORT    UART2

#define _PRINT_RST_MSG
#define _PRINT_VER_MSG
#define _PRINT_ADP_MSG
#define _PRINT_AUX_MSG
#define _PRINT_DDM_MSG
//#define _PRINT_FSK_MSG
//#define _PRINT_SSS_MSG
#define _PRINT_APL_MSG
#define _PRINT_USB_MSG
//#define _PRINT_NTC_MSG
#define _PRINT_PID_MSG
#define _PRINT_VIN_IIN
//#define _PRINT_QDT_MSG
#define _PRINT_ERR_MSG
#define _PRINT_ERR_BUF
#define _PRINT_FOD_MSG
#define _PRINT_WPC_PKT

#define _PRINT_REPING_MSG
//#define _CLOAK_TX_INIT

//------MPP_25W_DEBUG
#define MPP_25W_HPM_PING_ENALBE     (1)//enable: 360K 16V 468nF high K, 115nF low K, disable: 360K 11V 115nF/68nF
#define MPP_25W_LOW_K_VALUE         (8100)//0mm 8000, 2mm 7600, times 1.07
#define MPP_25W_360K_DIG_PING_PHASE (50)//spec is 50 (goodd for iphone), lower to 20 for DDM (good for GRL)

#define MPP_25W_FOD_ENABLE          (0)
#define MPP_25W_FOD_LOOSE_PFO       (0)//loose some pfo if false report FOD during MPLA

#define MPP_25W_MATE_Q_ENABLE       (0)

#define MPP_25W_POWER_MODE_TRANS_W_EPTR         (0)
#define MPP_25W_POWER_MODE_TRANS_W_CLOAK        (0)
#define MPP_25W_POWER_MODE_CPM_ENABLE           (1)

#define DIG_DDM_ENABLE              (1)

#endif /* DEBUG_H_ */
