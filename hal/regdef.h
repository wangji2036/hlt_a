#ifndef REGDEF_H_
#define REGDEF_H_

#include "typdef.h"

/*++++++++++++++++++++++++++++++++++++++++++ WDT define ++++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_WDT::CTRL
	 * Offset: 0x00  WatchDog Timer Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[0]     |MODU_EN   |WatchDog Timer Enable bit
	 * |        |          |0 = WDT Disabled.
	 * |        |          |1 = WDT Enabled.
	 * |[1]     |LOAD_EN   |load register to counter register Enable bit
	 * |        |          |0 = No effect.
	 * |        |          |1 = Enable.
	 * |        |          |Note: W1C-> W:1/0 clears/no effect on matching bit, R:no effect
	 * |[2]     |RST_EN    |WatchDog Timer Time-Out Reset Enable Control bit
	 * |        |          |0 = WDT time-out reset function Disabled.
	 * |        |          |1 = WDT time-out reset function Enabled.
	 * |[3]     |INT_EN    |WatchDog Timer Time-Out Interrupt Enable Control bit
	 * |        |          |If this bit is enabled, the WDT time-out interrupt signal is generated and inform to CPU.
	 * |        |          |0 = WDT time-out interrupt Disabled.
	 * |        |          |1 = WDT time-out interrupt Enabled.
	 * |[31:16] |WDT_CNT   |WatchDog Timer Interval Selection
	 * |        |          |These 16-bits select the time-out interval for the WatchDog Timer.
	 * |        |          |timeout = (CNT + 1) * TWDT
	 * |        |          |TWDT: 64K LIRC
	 */
	struct {
		uint32_t MODU_EN : 1;
		uint32_t LOAD_EN : 1; //W1C-> W:1/0 clears/no effect on matching bit, R:no effect
		uint32_t RST_EN  : 1;
		uint32_t INT_EN  : 1;
		uint32_t         :12;
		uint32_t WDT_CNT :16;
	} BITS;
	uint32_t WORD;
} TS_WDT_CTRL;

typedef union {
	/**
	 * @var TS_WDT::FLAG
	 * Offset: 0x04  WatchDog Timer timeout flag Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[0]     |INT_FLAG  |WatchDog timeout Flag
	 * |        |          |0 = A WDT timeout has not occurred.
	 * |        |          |1 = A WDT timeout has occurred.
	 * |        |          |Note 1: W1C, write 1 to clear this bit, write 0 no effect.
	 * |        |          |Note 2: if (INT_FLAG == 1) and (TS_WDT::CTRL::INT_EN == 1), a interrupt will be sent to CPU.
	 */
	struct {
		uint32_t INT_FLAG : 1; //W1C
		uint32_t          :31;
	} BITS;
	uint32_t WORD;
} TS_WDT_FLAG;

typedef struct {
	__IO TS_WDT_CTRL CTRL; //4000_0000
	__IO TS_WDT_FLAG FLAG; //4000_0004
} TS_WDT;

#define WDT_CTRL_MODU_EN_Pos             (0)                                /*!< TS_WDT::CTRL: MODU_EN Position   */
#define WDT_CTRL_MODU_EN_Msk             (0x1UL << WDT_CTRL_MODU_EN_Pos)    /*!< TS_WDT::CTRL: MODU_EN Mask       */
#define WDT_CTRL_LOAD_EN_Pos             (1)                                /*!< TS_WDT::CTRL: LOAD_EN Position   */
#define WDT_CTRL_LOAD_EN_Msk             (0x1UL << WDT_CTRL_LOAD_EN_Pos)    /*!< TS_WDT::CTRL: LOAD_EN Mask       */
#define WDT_CTRL_RST_EN_Pos              (2)                                /*!< TS_WDT::CTRL: RST_EN Position    */
#define WDT_CTRL_RST_EN_Msk              (0x1UL << WDT_CTRL_RST_EN_Pos)     /*!< TS_WDT::CTRL: RST_EN Mask        */
#define WDT_CTRL_INT_EN_Pos              (3)                                /*!< TS_WDT::CTRL: INT_EN Position    */
#define WDT_CTRL_INT_EN_Msk              (0x1UL << WDT_CTRL_INT_EN_Pos)     /*!< TS_WDT::CTRL: INT_EN Mask        */
#define WDT_CTRL_WDT_CNT_Pos             (16)                               /*!< TS_WDT::CTRL: WDT_CNT Position   */
#define WDT_CTRL_WDT_CNT_Msk             (0xFFFFUL << WDT_CTRL_WDT_CNT_Pos) /*!< TS_WDT::CTRL: WDT_CNT Mask       */

#define WDT_FLAG_INT_FLAG_Pos            (0)                                /*!< TS_WDT::FLAG: INT_FLAG Position  */
#define WDT_FLAG_INT_FLAG_Msk            (0x1UL << WDT_FLAG_INT_FLAG_Pos)   /*!< TS_WDT::FLAG: INT_FLAG Mask      */
/*------------------------------------------ WDT define ------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ TMR define ++++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_TMR::GEN_CTRL
	 * Offset: 0x00  Timer General Control Register, x = 0, 1, 2, 3
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[4:0]   |CLK_PSC   |Timer clock prescale counter
	 * |        |          |Timer input clock source is divided by 2^CLK_PSC
	 * |        |          |If this field is 0 (CLK_PSC = 0), then there is no scaling.
	 * |[5]     |OP_MODE   |Timer Operation Mode Control
	 * |        |          |0 = one shot mode
	 * |        |          |1 = Periodic mode
	 * |[6]     |INT_EN    |Interrupt Enable Control
	 * |        |          |0 = Timer Interrupt function Disabled.
	 * |        |          |1 = Timer Interrupt function Enabled.
	 * |        |          |If this bit is enabled, when the timer interrupt flag is set to 1, the timer interrupt signal is generated and inform to CPU.
	 * |[8]     |CNT_EN    |Timer module Enable Control
	 * |        |          |0 = Disable.
	 * |        |          |1 = Enable.
	 */
	struct {
		uint32_t CLK_PSC : 5;
		uint32_t OP_MODE : 1; //1:Periodic mode, 0:one shot mode
		uint32_t INT_EN  : 1;
		uint32_t         : 1;
		uint32_t CNT_EN  : 1;
		uint32_t         :23;
	} BITS;
	uint32_t WORD;
} TS_TMR_GEN_CTRL;

typedef union {
	/**
	 * @var TS_TMR::SPL_CTRL
	 * Offset: 0x10  Timer Special Control Register, x = 0, this control is only for TMR0
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[0]     |CLK_SRC   |TMR0 clock source select
	 * |        |          |0 = HCLK
	 * |        |          |1 = LIRC, 64K
	 * |[1]     |WKUP_EN   |TMR0 Wake-Up Enable
	 * |        |          |When WKUP_EN is set and the TMR0::FLAG::INT_FLAG is set, the TMR0 controller will generator a wake-up event to CPU.
	 * |        |          |0 = Disable.
	 * |        |          |1 = Enable.
	 */
	struct {
		uint32_t CLK_SRC : 1; //0:HCLK 1:LIRC
		uint32_t WKUP_EN : 1;
		uint32_t         :30;
	} BITS;
	uint32_t WORD;
} TS_TMR_SPL_CTRL;

typedef union {
	/**
	 * @var TS_TMR::LOAD_CNT
	 * Offset: 0x04  Timer auto load count Register, x = 0, 1, 2, 3
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[31:0]  |LOAD_CNT  |Timer load data Register
	 * |        |          |Timer start value for downward counting.
	 */
	struct {
		uint32_t LOAD_CNT :32;
	} BITS;
	uint32_t WORD;
} TS_TMR_LOAD_CNT;

typedef union {
	/**
	 * @var TS_TMR::REAL_CNT
	 * Offset: 0x04  Timer real time count Register, x = 0, 1, 2, 3
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[31:0]  |REAL_CNT  |Timer real time count value Register
	 * |        |          |Timer real time count value for downward counting.
	 * |        |          |Read Only
	 */
	struct {
		uint32_t REAL_CNT :32;
	} BITS;
	uint32_t WORD;
} TS_TMR_REAL_CNT;

typedef union {
	/**
	 * @var TS_TMR::STS_FLAG
	 * Offset: 0x0C  Timer status flag Register, x = 0, 1, 2, 3
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field     |Descriptions
	 * | :----: | :----:   | :---- |
	 * |[0]     |CNT_FLAG  |timer counter status flag
	 * |        |          |This bit indicates flag status of Timer while TMRx::REAL_CNT value count down to 0.
	 * |        |          |0 = No effect
	 * |        |          |1 = REAL_CNT value count down to 0
	 * |        |          |When TMRx::INT_EN is set and the STS_FLAG is set, the timer interrupt signal is generated and inform to CPU.
	 * |        |          |Note: This bit is cleared by writing 1 to it.
	 */
	struct {
		uint32_t CNT_FLAG : 1; //W1C
		uint32_t          :31;
	} BITS;
	uint32_t WORD;
} TS_TMR_STS_FLAG;

typedef struct {
	/*--------------------------------- TMR0 ---- TMR0 ---- TMR0 ---- TMR0 --*/
    __IO TS_TMR_GEN_CTRL GEN_CTRL; //4000_0020 4000_0040 4000_0060 4000_0080
    __IO TS_TMR_LOAD_CNT LOAD_CNT; //4000_0024 4000_0044 4000_0064 4000_0084
    __I  TS_TMR_REAL_CNT REAL_CNT; //4000_0028 4000_0048 4000_0068 4000_0088
    __IO TS_TMR_STS_FLAG STS_FLAG; //4000_002C 4000_004C 4000_006C 4000_008C
    __IO TS_TMR_SPL_CTRL SPL_CTRL; //4000_0030 RESE_RVED RESE_RVED RESE_RVED
} TS_TMR;

#define TMR_GEN_CTRL_CLK_PSC_Pos         (0)                                    /*!< TS_TMR::GEN_CTRL: CLK_PSC Position   */
#define TMR_GEN_CTRL_CLK_PSC_Msk         (0x1FUL << TMR_GEN_CTRL_CLK_PSC_Pos)   /*!< TS_TMR::GEN_CTRL: CLK_PSC Mask       */
#define TMR_GEN_CTRL_OP_MODE_Pos         (5)                                    /*!< TS_TMR::GEN_CTRL: OP_MODE Position   */
#define TMR_GEN_CTRL_OP_MODE_Msk         (0x1UL << TMR_GEN_CTRL_OP_MODE_Pos)    /*!< TS_TMR::GEN_CTRL: OP_MODE Mask       */
#define TMR_GEN_CTRL_INT_EN_Pos          (6)                                    /*!< TS_TMR::GEN_CTRL: INT_EN Position    */
#define TMR_GEN_CTRL_INT_EN_Msk          (0x1UL << TMR_GEN_CTRL_INT_EN_Pos)     /*!< TS_TMR::GEN_CTRL: INT_EN Mask        */
#define TMR_GEN_CTRL_CNT_EN_Pos          (8)                                    /*!< TS_TMR::GEN_CTRL: CNT_EN Position    */
#define TMR_GEN_CTRL_CNT_EN_Msk          (0x1UL << TMR_GEN_CTRL_CNT_EN_Pos)     /*!< TS_TMR::GEN_CTRL: CNT_EN Mask        */

#define TMR_SPL_CTRL_CLK_SRC_Pos         (0)                                    /*!< TS_TMR::SPL_CTRL: CLK_SRC Position   */
#define TMR_SPL_CTRL_CLK_SRC_Msk         (0x1UL << TMR_SPL_CTRL_CLK_SRC_Pos)    /*!< TS_TMR::SPL_CTRL: CLK_SRC Mask       */
#define TMR_SPL_CTRL_WKUP_EN_Pos         (1)                                    /*!< TS_TMR::SPL_CTRL: WKUP_EN Position   */
#define TMR_SPL_CTRL_WKUP_EN_Msk         (0x1UL << TMR_SPL_CTRL_WKUP_EN_Pos)    /*!< TS_TMR::SPL_CTRL: WKUP_EN Mask       */

#define TMR_STS_FLAG_CNT_FLAG_Pos        (0)                                    /*!< TS_TMR::STS_FLAG: CNT_FLAG Position  */
#define TMR_STS_FLAG_CNT_FLAG_Msk        (0x1UL << TMR_STS_FLAG_CNT_FLAG_Pos)   /*!< TS_TMR::STS_FLAG: CNT_FLAG Mask      */

enum {
	_TMR_OP_MODE_ONE_SHOT = 0,
	_TMR_OP_MODE_PERIODIC = 1,
};

enum {
	_TMR_CLK_SRC_HCLK = 0,
	_TMR_CLK_SRC_LIRC = 1,
};
/*------------------------------------------ TMR define ------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ SYS define ++++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_SYS::EXT_CTRL
	 * Offset: 0x00  external I2C Control chip Register, MCU read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[1]     |WAKEUP_DIS   |wake_up function disable control bit
	 * |        |             |0 = wake_up function enable.
	 * |        |             |1 = wake_up function disable.
	 * |[3]     |FORCE_SCL_EN |force SCL enable control bit
	 * |        |             |0 = No effect.
	 * |        |             |1 = Enable.
	 * |[4]     |FORCE_PLL_EN |force PLL enable control bit
	 * |        |             |0 = No effect.
	 * |        |             |1 = Enable.
	 * |[5]     |AP_CTLAPB_EN |external I2C read and write APB register Enable Control bit
	 * |        |             |0 = external I2C R/W APB register disable.
	 * |        |             |1 = external I2C R/W APB register enable.
	 */
	struct {
		uint32_t              : 1;
		uint32_t WAKEUP_DIS   : 1;
		uint32_t              : 1;
		uint32_t FORCE_SCL_EN : 1;
		uint32_t FORCE_PLL_EN : 1;
		uint32_t AP_CTLAPB_EN : 1;
		uint32_t              : 1;
		uint32_t              : 1;
		uint32_t              :24;
	} BITS;
	uint32_t WORD;
} TS_SYS_EXT_CTRL;

typedef union {
	/**
	 * @var TS_SYS::CLK_CTRL
	 * Offset: 0x04  chip clock control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[2:0]   |CPU_CLK_SEL  |chip CPU clock select control bits
	 * |        |             |0 = 36M
	 * |        |             |1 = 24M
	 * |        |             |2 = 18M
	 * |        |             |3 = 12M
	 * |        |             |4 =  9M
	 * |        |             |5 =  6M
	 * |        |             |others = 36M
	 * |[3]     |PLL_RDY_DET  |PLL ready detect enable control bit
	 * |        |             |0 = disable.
	 * |        |             |1 = Enable.
	 * |[4]     |XTAL_GD_SET  |XTAL good signal force enable control bit
	 * |        |             |0 = enable delay based XTAL good detection.
	 * |        |             |1 = force XTAL good to high.
	 * |[5]     |PLL_SRC_SEL  |PLL source select control bit
	 * |        |             |0 = HIRC
	 * |        |             |1 = XTAL
	 * |[6]     |XTAL_EN      |external XTAL enable control bit
	 * |        |             |0 = disable.
	 * |        |             |1 = enable.
	 * |[8:7]   |XTAL_PREDIV  |external XTAL pre_divided control bits
	 * |        |             |0 = XTAL_CLK/1 as reference for PLL.
	 * |        |             |1 = XTAL_CLK/2 as reference for PLL.
	 * |        |             |2 = XTAL_CLK/3 as reference for PLL.
	 * |        |             |3 = XTAL_CLK/3 as reference for PLL.
	 */
	struct {
		uint32_t CPU_CLK_SEL : 3;
		uint32_t PLL_RDY_DET : 1;
		uint32_t XTAL_GD_SET : 1;
		uint32_t PLL_SRC_SEL : 1;
		uint32_t XTAL_EN     : 1;
		uint32_t XTAL_PREDIV : 2;
		uint32_t             :23;
	} BITS;
	uint32_t WORD;
} TS_SYS_CLK_CTRL;

typedef union {
	/**
	 * @var TS_SYS::PWR_CTRL
	 * Offset: 0x08  system power and wake_up control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |SLEEP_MODE_EN |System sleep mode Enable control bit
	 * |        |              |When this bit is set to 1, sleep mode is enabled.
	 * |        |              |When chip wake_up, this bit is auto cleared.
	 * |        |              |Users need to set this bit again for next sleep.
	 * |        |              |0 = no effect.
	 * |        |              |1 = force MCU go into sleep mode.
	 * |        |              |W1C: this bit will cleared by hardware.
	 * |[1]     |GPIO_WKUP_DIS |GPIO interrupt wake_up disable control bit
	 * |        |              |0 = enable GPIO INT wake_up MCU.
	 * |        |              |1 = disable GPIO INT wake_up MCU.
	 * |[2]     |TCPC_WKUP_DIS |TCPC protocol wake_up disable control bit
	 * |        |              |0 = enable TCPC protocol wake_up MCU.
	 * |        |              |1 = disable TCPC protocol wake_up MCU.
	 */
	struct {
		uint32_t SLEEP_MODE_EN : 1; //W1C
		uint32_t GPIO_WKUP_DIS : 1;
		uint32_t TCPC_WKUP_DIS : 1;
		uint32_t               :29;
	} BITS;
	uint32_t WORD;
} TS_SYS_PWR_CTRL;

typedef union {
	/**
	 * @var TS_SYS::OPA_STAT
	 * Offset: 0x0C system operation status register. read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[0]     |PLL_SRC      |PLL source come from status
	 * |        |             |0 = From HIRC
	 * |        |             |1 = From XTAL
	 * |[2:1]   |FSM_STS      |digital main FSM state
	 * |        |             |0 = IDLE.
	 * |        |             |1 = normal mode.
	 * |        |             |2 = sleep mode.
	 * |        |             |3 = reserved.
	 * |[5:3]   |RST_SRC      |System reset source status
	 * |        |             |0 = IDle.
	 * |        |             |1 = power on reset.
	 * |        |             |2 = wake_up by timer0.
	 * |        |             |3 = wake_up by GPIO interrupt.
	 * |        |             |4 = wake_up by TCPC protocol.
	 * |        |             |others = reserved.
	 * |[6]     |TSD_STS      |system temperature shut down status
	 * |        |             |0 = normal state
	 * |        |             |1 = an over temperature was detected
	 * |[7]     |PVD_STS      |system power voltage down status
	 * |        |             |0 = normal state
	 * |        |             |1 = an under VDD voltage was detected
	 */
	struct {
		uint32_t PLL_SRC : 1;
		uint32_t FSM_STS : 2;
		uint32_t RST_SRC : 3;
		uint32_t TSD_STS : 1;
		uint32_t PVD_STS : 1;
		uint32_t         :24;
	} BITS;
	uint32_t WORD;
} TS_SYS_OPR_STAT;

typedef union {
	/**
	 * @var TS_SYS::RST_CTRL
	 * Offset: 0x10 chip reset control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[0]     |FMC_RST      |flash memory controller reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[1]     |RAM_RST      |SRAM controller reset
	 * |        |             |reset the peripheral control circuit of SRAM, which has no effect on the values inside SRAM
	 * |        |             |This reset doesn't have impact on FW either
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[2]     |DDM_RST      |DDM controller reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[3]     |EADC_RST     |EADC Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[4]     |BADC_RST     |BADC Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[5]     |EPWM1_RST    |Enhanced PWM1 Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[6]     |EPWM2_RST    |Enhanced PWM2 Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[7]     |BPWM_RST     |Basic PWM Controller Reset
	 * |        |             |BPWM3/BPWM4/BPWM7/BPWM8
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[8]     |TMR_RST      |Timer Controller Reset
	 * |        |             |TMR0/TMR1/TMR2/TMR3
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[10]    |UART1_RST    |UART1 Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[11]    |UART2_RST    |UART2 Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[12]    |ECAP_RST     |ECAP Controller Reset
	 * |        |             |ECAP1/ECAP2/ECAP3/ECAP4/ECAP5
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[13]    |GPIO_RST     |GPIO Controller Reset
	 * |        |             |GPA/GPB/GPC/GPD
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[15]    |TCPC_RST     |Type-C Port Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[16]    |UFCS_RST     |UFCS Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[17]    |DPDM_SNK_RST |DPDM_SNK Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[18]    |DPDM_SRC_RST |DPDM_SRC Controller Reset
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[19]    |USBPD_RST    |USBPD Controller Reset
	 * |        |             |Reset the digital logic related to PD, excluding registers. The relevant registers are reset by TCPC_RST.
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[20]    |I2C_RST      |I2C Controller Reset
	 * |        |             |I2CS/I2CM
	 * |        |             |0 = no effect
	 * |        |             |1 = reset module
	 * |[21]    |CPU_RST      |CPU core Reset, just reset CPU core, put PC to 0 address.
	 * |        |             |0 = no effect
	 * |        |             |1 = reset CPU
	 * |[22]    |MCU_RST      |MCU Reset, reset chip, like POR
	 * |        |             |0 = no effect
	 * |        |             |1 = reset chip
	 */
	struct {
		uint32_t FMC_RST      : 1;
		uint32_t RAM_RST      : 1;
		uint32_t DDM_RST      : 1;
		uint32_t EADC_RST     : 1;
		uint32_t BADC_RST     : 1;
		uint32_t EPWM1_RST    : 1;
		uint32_t EPWM2_RST    : 1;
		uint32_t BPWM_RST     : 1;
		uint32_t TMR_RST      : 1;
		uint32_t              : 1;
		uint32_t UART1_RST    : 1;
		uint32_t UART2_RST    : 1;
		uint32_t ECAP_RST     : 1;
		uint32_t GPIO_RST     : 1;
		uint32_t              : 1;
		uint32_t TCPC_RST     : 1;
		uint32_t UFCS_RST     : 1;
		uint32_t DPDM_SNK_RST : 1;
		uint32_t DPDM_SRC_RST : 1;
		uint32_t USBPD_RST    : 1;
		uint32_t I2C_RST      : 1;
		uint32_t CPU_RST      : 1;
		uint32_t MCU_RST      : 1;
		uint32_t              : 9;
	} BITS;
	uint32_t WORD;
} TS_SYS_RST_CTRL;

typedef union {
	/**
	 * @var TS_SYS::PRO_CTRL
	 * Offset: 0x14 system protection control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[0]     |PVD_EN       |PVD module enable control bit
	 * |        |             |0 = disable power voltage down detect
	 * |        |             |1 = enable power voltage down detect
	 * |[3:1]   |PVD_THD_SEL  |PVD threshold falling voltage setting bits, recovery voltage V_HYST = 0.1V
	 * |        |             |0 = 2.4V
	 * |        |             |1 = 2.7V
	 * |        |             |2 = 3.0V
	 * |        |             |3 = 3.7V
	 * |        |             |4 = 4.0V
	 * |        |             |5 = 4.3V
	 * |        |             |others = 4.3V
	 * |[5]     |TSD_THD_SEL  |TSD threshold setting bits, T_HYST = 20C
	 * |        |             |0 = 105C
	 * |        |             |1 = 125C
	 * |[6]     |TSD_RST_EN   |When TSD is triggered, reset MCU or not control bit
	 * |        |             |0 = disable
	 * |        |             |1 = enable
	 */
	struct {
		uint32_t PVD_EN      : 1;
		uint32_t PVD_THD_SEL : 3;
		uint32_t             : 1;
		uint32_t TSD_THD_SEL : 1;
		uint32_t TSD_RST_EN  : 1;
		uint32_t             : 1;
		uint32_t             :24;
	} BITS;
	uint32_t WORD;
} TS_SYS_PRO_CTRL;

typedef union {
	/**
	 * @var TS_SYS::PRO_INTE
	 * Offset: 0x18 system protection interrupt control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[0]     |PVD_INT_EN   |PVD Interrupt Enable Control
	 * |        |             |0 = disable
	 * |        |             |1 = enable
	 * |        |             |If this bit is enabled, when the PVD_FLAG is set to 1, the interrupt signal is generated and inform to CPU.
	 * |[1]     |TSD_INT_EN   |TSD Interrupt Enable Control
	 * |        |             |0 = disable
	 * |        |             |1 = enable
	 * |        |             |If this bit is enabled, when the TSD_FLAG is set to 1, the interrupt signal is generated and inform to CPU.
	 */
	struct {
		uint32_t PVD_INT_EN : 1;
		uint32_t TSD_INT_EN : 1;
		uint32_t            :30;
	} BITS;
	uint32_t WORD;
} TS_SYS_PRO_INTE;

typedef union {
	/**
	 * @var TS_SYS::PRO_STSF
	 * Offset: 0x1C system protection status flag control register.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[0]     |PVD_FLAG     |PVD trigger flag
	 * |        |             |when a power voltage down was detected, this bit will set to 1.
	 * |        |             |0 = a PVD was not detected.
	 * |        |             |1 = a PVD was detected.
	 * |        |             |When TS_SYS::PRO_INTE::PVD_INT_EN is set and the PVD_FLAG is set, the interrupt signal is generated and inform to CPU.
	 * |        |             |Note: This bit is cleared by writing 1 to it.
	 * |[1]     |TSD_FLAG     |TSD trigger flag
	 * |        |             |when a THERMAL shut down was detected, this bit will set to 1.
	 * |        |             |0 = a TSD was not detected.
	 * |        |             |1 = a TSD was detected.
	 * |        |             |When TS_SYS::PRO_INTE::TSD_INT_EN is set and the TSD_FLAG is set, the interrupt signal is generated and inform to CPU.
	 */
	struct {
		uint32_t PVD_FLAG : 1;
		uint32_t TSD_FLAG : 1;
		uint32_t          :30;
	} BITS;
	uint32_t WORD;
} TS_SYS_PRO_STSF;

typedef union {
	/**
	 * @var TS_SYS::PID_INFO
	 * Offset: 0x20 product identification information. read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[15:0]  |PID          |PID, chip product identification code, read only.
	 * |        |             |0x42D7 = 17111 -> NU17111
	 * |        |             |0x42D8 = 17112 -> NU17112
	 * |        |             |...
	 * |        |             |0x0EE3 =  3811 -> SP3811
	 * |        |             |...
	 * |[19:16] |VER          |chip version
	 * |        |             |BIT[19]: major version, 0-A, 1-B
	 * |        |             |BIT[18-16]: minor version
	 * |        |             |BIT[19:16] = 0, A0
	 */
	struct {
		uint32_t PID :16;
		uint32_t VER : 4;
		uint32_t     : 4;
		uint32_t DFT : 8;
	} BITS;
	uint32_t WORD;
} TS_SYS_PID_INFO;

typedef union {
	/**
	 * @var TS_SYS::UID_INFO
	 * Offset: 0x24 chip unique identification information. read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[31:0]  |UID          |unique ID, read only.
	 * |        |             |32-bit chip unique identification code
	 */
	struct {
		uint32_t UID_CODE :26;
		uint32_t SUB_CODE : 3;
		uint32_t DIE_CODE : 3;
	} BITS;
	uint32_t WORD;
} TS_SYS_UID_INFO;

typedef union {
	/**
	 * @var TS_SYS::DIG_INFO
	 * Offset: 0x28 chip digital information. read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field        |Descriptions
	 * | :----: | :----:      | :---- |
	 * |[1:0]   |DEV_I2C_ADDR |slave I2C device address configuration
	 * |        |             |0 = slave I2C device address is 0x41(7bit)
	 * |        |             |1 = slave I2C device address is 0x51(7bit)
	 * |        |             |2 = slave I2C device address is 0x61(7bit)
	 * |        |             |3 = slave I2C device address is 0x71(7bit)
	 */
	struct {
		uint32_t DEV_I2C_ADDR : 2;
		uint32_t              :30;
	} BITS;
	uint32_t WORD;
} TS_SYS_DIG_INFO;

typedef struct {
	__I  TS_SYS_EXT_CTRL EXT_CTRL; //4000_1000
	__IO TS_SYS_CLK_CTRL CLK_CTRL; //4000_1004
	__IO TS_SYS_PWR_CTRL PWR_CTRL; //4000_1008
	__I  TS_SYS_OPR_STAT OPR_STAT; //4000_100C
	__IO TS_SYS_RST_CTRL RST_CTRL; //4000_1010
	__IO TS_SYS_PRO_CTRL PRO_CTRL; //4000_1014
	__IO TS_SYS_PRO_INTE PRO_INTE; //4000_1018
	__IO TS_SYS_PRO_STSF PRO_STSF; //4000_101C
	__I  TS_SYS_PID_INFO PID_INFO; //4000_1020
	__I  TS_SYS_UID_INFO UID_INFO; //4000_1024
	__I  TS_SYS_DIG_INFO DIG_INFO; //4000_1028
} TS_SYS;

#define SYS_EXT_CTRL_WAKEUP_DIS_Pos         (1)                                         /*!< TS_SYS::EXT_CTRL: WAKEUP_DIS Position   */
#define SYS_EXT_CTRL_WAKEUP_DIS_Msk         (0x1UL << SYS_EXT_CTRL_WAKEUP_DIS_Pos)      /*!< TS_SYS::EXT_CTRL: WAKEUP_DIS Mask       */
#define SYS_EXT_CTRL_FORCE_SCL_EN_Pos       (3)                                         /*!< TS_SYS::EXT_CTRL: FORCE_SCL_EN Position */
#define SYS_EXT_CTRL_FORCE_SCL_EN_Msk       (0x1UL << SYS_EXT_CTRL_FORCE_SCL_EN_Pos)    /*!< TS_SYS::EXT_CTRL: FORCE_SCL_EN Mask     */
#define SYS_EXT_CTRL_FORCE_PLL_EN_Pos       (4)                                         /*!< TS_SYS::EXT_CTRL: FORCE_PLL_EN Position */
#define SYS_EXT_CTRL_FORCE_PLL_EN_Msk       (0x1UL << SYS_EXT_CTRL_FORCE_PLL_EN_Pos)    /*!< TS_SYS::EXT_CTRL: FORCE_PLL_EN Mask     */
#define SYS_EXT_CTRL_AP_CTLAPB_EN_Pos       (5)                                         /*!< TS_SYS::EXT_CTRL: AP_CTLAPB_EN Position */
#define SYS_EXT_CTRL_AP_CTLAPB_EN_Msk       (0x1UL << SYS_EXT_CTRL_AP_CTLAPB_EN_Pos)    /*!< TS_SYS::EXT_CTRL: AP_CTLAPB_EN Mask     */

#define SYS_CLK_CTRL_CPU_CLK_SEL_Pos        (0)                                         /*!< TS_SYS::CLK_CTRL: CPU_CLK_SEL Position  */
#define SYS_CLK_CTRL_CPU_CLK_SEL_Msk        (0x7UL << SYS_CLK_CTRL_CPU_CLK_SEL_Pos)     /*!< TS_SYS::CLK_CTRL: CPU_CLK_SEL Mask      */
#define SYS_CLK_CTRL_PLL_RDY_DET_Pos        (3)                                         /*!< TS_SYS::CLK_CTRL: PLL_RDY_DET Position  */
#define SYS_CLK_CTRL_PLL_RDY_DET_Msk        (0x1UL << SYS_CLK_CTRL_PLL_RDY_DET_Pos)     /*!< TS_SYS::CLK_CTRL: PLL_RDY_DET Mask      */
#define SYS_CLK_CTRL_XTAL_GD_SET_Pos        (4)                                         /*!< TS_SYS::CLK_CTRL: XTAL_GD_SET Position  */
#define SYS_CLK_CTRL_XTAL_GD_SET_Msk        (0x1UL << SYS_CLK_CTRL_XTAL_GD_SET_Pos)     /*!< TS_SYS::CLK_CTRL: XTAL_GD_SET Mask      */
#define SYS_CLK_CTRL_PLL_SRC_SEL_Pos        (5)                                         /*!< TS_SYS::CLK_CTRL: PLL_SRC_SEL Position  */
#define SYS_CLK_CTRL_PLL_SRC_SEL_Msk        (0x1UL << SYS_CLK_CTRL_PLL_SRC_SEL_Pos)     /*!< TS_SYS::CLK_CTRL: PLL_SRC_SEL Mask      */
#define SYS_CLK_CTRL_XTAL_EN_Pos            (6)                                         /*!< TS_SYS::CLK_CTRL: XTAL_EN Position      */
#define SYS_CLK_CTRL_XTAL_EN_Msk            (0x1UL << SYS_CLK_CTRL_XTAL_EN_Pos)         /*!< TS_SYS::CLK_CTRL: XTAL_EN Mask          */
#define SYS_CLK_CTRL_XTAL_PREDIV_Pos        (7)                                         /*!< TS_SYS::CLK_CTRL: XTAL_PREDIV Position  */
#define SYS_CLK_CTRL_XTAL_PREDIV_Msk        (0x3UL << SYS_CLK_CTRL_XTAL_PREDIV_Pos)     /*!< TS_SYS::CLK_CTRL: XTAL_PREDIV Mask      */

#define SYS_PWR_CTRL_SLEEP_MODE_EN_Pos      (0)                                         /*!< TS_SYS::PWR_CTRL: SLEEP_MODE_EN Position*/
#define SYS_PWR_CTRL_SLEEP_MODE_EN_Msk      (0x1UL << SYS_PWR_CTRL_SLEEP_MODE_EN_Pos)   /*!< TS_SYS::PWR_CTRL: SLEEP_MODE_EN Mask    */
#define SYS_PWR_CTRL_GPIO_WKUP_DIS_Pos      (1)                                         /*!< TS_SYS::PWR_CTRL: GPIO_WKUP_DIS Position*/
#define SYS_PWR_CTRL_GPIO_WKUP_DIS_Msk      (0x1UL << SYS_PWR_CTRL_GPIO_WKUP_DIS_Pos)   /*!< TS_SYS::PWR_CTRL: GPIO_WKUP_DIS Mask    */
#define SYS_PWR_CTRL_TCPC_WKUP_DIS_Pos      (2)                                         /*!< TS_SYS::PWR_CTRL: TCPC_WKUP_DIS Position*/
#define SYS_PWR_CTRL_TCPC_WKUP_DIS_Msk      (0x1UL << SYS_PWR_CTRL_TCPC_WKUP_DIS_Pos)   /*!< TS_SYS::PWR_CTRL: TCPC_WKUP_DIS Mask    */

#define SYS_OPR_STAT_PLL_SRC_Pos            (0)                                         /*!< TS_SYS::OPR_STAT: PLL_SRC Position      */
#define SYS_OPR_STAT_PLL_SRC_Msk            (0x1UL << SYS_OPR_STAT_PLL_SRC_Pos)         /*!< TS_SYS::OPR_STAT: PLL_SRC Mask          */
#define SYS_OPR_STAT_FSM_STS_Pos            (1)                                         /*!< TS_SYS::OPR_STAT: FSM_STS Position      */
#define SYS_OPR_STAT_FSM_STS_Msk            (0x3UL << SYS_OPR_STAT_FSM_STS_Pos)         /*!< TS_SYS::OPR_STAT: FSM_STS Mask          */
#define SYS_OPR_STAT_RST_SRC_Pos            (3)                                         /*!< TS_SYS::OPR_STAT: RST_SRC Position      */
#define SYS_OPR_STAT_RST_SRC_Msk            (0x7UL << SYS_OPR_STAT_RST_SRC_Pos)         /*!< TS_SYS::OPR_STAT: RST_SRC Mask          */
#define SYS_OPR_STAT_TSD_STS_Pos            (6)                                         /*!< TS_SYS::OPR_STAT: TSD_STS Position      */
#define SYS_OPR_STAT_TSD_STS_Msk            (0x1UL << SYS_OPR_STAT_TSD_STS_Pos)         /*!< TS_SYS::OPR_STAT: TSD_STS Mask          */
#define SYS_OPR_STAT_PVD_STS_Pos            (7)                                         /*!< TS_SYS::OPR_STAT: PVD_STS Position      */
#define SYS_OPR_STAT_PVD_STS_Msk            (0x1UL << SYS_OPR_STAT_PVD_STS_Pos)         /*!< TS_SYS::OPR_STAT: PVD_STS Mask          */

#define SYS_RST_CTRL_FMC_RST_Pos            (0)                                         /*!< TS_SYS::RST_CTRL: FMC_RST Position      */
#define SYS_RST_CTRL_FMC_RST_Msk            (0x1UL << SYS_RST_CTRL_FMC_RST_Pos)         /*!< TS_SYS::RST_CTRL: FMC_RST Mask          */
#define SYS_RST_CTRL_RAM_RST_Pos            (1)                                         /*!< TS_SYS::RST_CTRL: RAM_RST Position      */
#define SYS_RST_CTRL_RAM_RST_Msk            (0x1UL << SYS_RST_CTRL_RAM_RST_Pos)         /*!< TS_SYS::RST_CTRL: RAM_RST Mask          */
#define SYS_RST_CTRL_DDM_RST_Pos            (2)                                         /*!< TS_SYS::RST_CTRL: DDM_RST Position      */
#define SYS_RST_CTRL_DDM_RST_Msk            (0x1UL << SYS_RST_CTRL_DDM_RST_Pos)         /*!< TS_SYS::RST_CTRL: DDM_RST Mask          */
#define SYS_RST_CTRL_EADC_RST_Pos           (3)                                         /*!< TS_SYS::RST_CTRL: EADC_RST Position     */
#define SYS_RST_CTRL_EADC_RST_Msk           (0x1UL << SYS_RST_CTRL_EADC_RST_Pos)        /*!< TS_SYS::RST_CTRL: EADC_RST Mask         */
#define SYS_RST_CTRL_BADC_RST_Pos           (4)                                         /*!< TS_SYS::RST_CTRL: BADC_RST Position     */
#define SYS_RST_CTRL_BADC_RST_Msk           (0x1UL << SYS_RST_CTRL_BADC_RST_Pos)        /*!< TS_SYS::RST_CTRL: BADC_RST Mask         */
#define SYS_RST_CTRL_EPWM1_RST_Pos          (5)                                         /*!< TS_SYS::RST_CTRL: EPWM1_RST Position    */
#define SYS_RST_CTRL_EPWM1_RST_Msk          (0x1UL << SYS_RST_CTRL_EPWM1_RST_Pos)       /*!< TS_SYS::RST_CTRL: EPWM1_RST Mask        */
#define SYS_RST_CTRL_EPWM2_RST_Pos          (6)                                         /*!< TS_SYS::RST_CTRL: EPWM2_RST Position    */
#define SYS_RST_CTRL_EPWM2_RST_Msk          (0x1UL << SYS_RST_CTRL_EPWM2_RST_Pos)       /*!< TS_SYS::RST_CTRL: EPWM2_RST Mask        */
#define SYS_RST_CTRL_BPWM_RST_Pos           (7)                                         /*!< TS_SYS::RST_CTRL: BPWM_RST Position     */
#define SYS_RST_CTRL_BPWM_RST_Msk           (0x1UL << SYS_RST_CTRL_BPWM_RST_Pos)        /*!< TS_SYS::RST_CTRL: BPWM_RST Mask         */
#define SYS_RST_CTRL_TMR_RST_Pos            (8)                                         /*!< TS_SYS::RST_CTRL: TMR_RST Position      */
#define SYS_RST_CTRL_TMR_RST_Msk            (0x1UL << SYS_RST_CTRL_TMR_RST_Pos)         /*!< TS_SYS::RST_CTRL: TMR_RST Mask          */
#define SYS_RST_CTRL_UART1_RST_Pos          (10)                                        /*!< TS_SYS::RST_CTRL: UART1_RST Position    */
#define SYS_RST_CTRL_UART1_RST_Msk          (0x1UL << SYS_RST_CTRL_UART1_RST_Pos)       /*!< TS_SYS::RST_CTRL: UART1_RST Mask        */
#define SYS_RST_CTRL_UART2_RST_Pos          (11)                                        /*!< TS_SYS::RST_CTRL: UART2_RST Position    */
#define SYS_RST_CTRL_UART2_RST_Msk          (0x1UL << SYS_RST_CTRL_UART2_RST_Pos)       /*!< TS_SYS::RST_CTRL: UART2_RST Mask        */
#define SYS_RST_CTRL_ECAP_RST_Pos           (12)                                        /*!< TS_SYS::RST_CTRL: ECAP_RST Position     */
#define SYS_RST_CTRL_ECAP_RST_Msk           (0x1UL << SYS_RST_CTRL_ECAP_RST_Pos)        /*!< TS_SYS::RST_CTRL: ECAP_RST Mask         */
#define SYS_RST_CTRL_GPIO_RST_Pos           (12)                                        /*!< TS_SYS::RST_CTRL: GPIO_RST Position     */
#define SYS_RST_CTRL_GPIO_RST_Msk           (0x1UL << SYS_RST_CTRL_GPIO_RST_Pos)        /*!< TS_SYS::RST_CTRL: GPIO_RST Mask         */
#define SYS_RST_CTRL_TCPC_RST_Pos           (15)                                        /*!< TS_SYS::RST_CTRL: TCPC_RST Position     */
#define SYS_RST_CTRL_TCPC_RST_Msk           (0x1UL << SYS_RST_CTRL_TCPC_RST_Pos)        /*!< TS_SYS::RST_CTRL: TCPC_RST Mask         */
#define SYS_RST_CTRL_UFCS_RST_Pos           (16)                                        /*!< TS_SYS::RST_CTRL: UFCS_RST Position     */
#define SYS_RST_CTRL_UFCS_RST_Msk           (0x1UL << SYS_RST_CTRL_UFCS_RST_Pos)        /*!< TS_SYS::RST_CTRL: UFCS_RST Mask         */
#define SYS_RST_CTRL_DPDM_SNK_RST_Pos       (17)                                        /*!< TS_SYS::RST_CTRL: DPDM_SNK_RST Position */
#define SYS_RST_CTRL_DPDM_SNK_RST_Msk       (0x1UL << SYS_RST_CTRL_DPDM_SNK_RST_Pos)    /*!< TS_SYS::RST_CTRL: DPDM_SNK_RST Mask     */
#define SYS_RST_CTRL_DPDM_SRC_RST_Pos       (18)                                        /*!< TS_SYS::RST_CTRL: DPDM_SRC_RST Position */
#define SYS_RST_CTRL_DPDM_SRC_RST_Msk       (0x1UL << SYS_RST_CTRL_DPDM_SRC_RST_Pos)    /*!< TS_SYS::RST_CTRL: DPDM_SRC_RST Mask     */
#define SYS_RST_CTRL_USBPD_RST_Pos          (19)                                        /*!< TS_SYS::RST_CTRL: USBPD_RST Position    */
#define SYS_RST_CTRL_USBPD_RST_Msk          (0x1UL << SYS_RST_CTRL_USBPD_RST_Pos)       /*!< TS_SYS::RST_CTRL: USBPD_RST Mask        */
#define SYS_RST_CTRL_I2C_RST_Pos            (20)                                        /*!< TS_SYS::RST_CTRL: I2C_RST Position      */
#define SYS_RST_CTRL_I2C_RST_Msk            (0x1UL << SYS_RST_CTRL_I2C_RST_Pos)         /*!< TS_SYS::RST_CTRL: I2C_RST Mask          */
#define SYS_RST_CTRL_CPU_RST_Pos            (21)                                        /*!< TS_SYS::RST_CTRL: CPU_RST Position      */
#define SYS_RST_CTRL_CPU_RST_Msk            (0x1UL << SYS_RST_CTRL_CPU_RST_Pos)         /*!< TS_SYS::RST_CTRL: CPU_RST Mask          */
#define SYS_RST_CTRL_MCU_RST_Pos            (22)                                        /*!< TS_SYS::RST_CTRL: MCU_RST Position      */
#define SYS_RST_CTRL_MCU_RST_Msk            (0x1UL << SYS_RST_CTRL_MCU_RST_Pos)         /*!< TS_SYS::RST_CTRL: MCU_RST Mask          */

#define SYS_PRO_CTRL_PVD_EN_Pos             (0)                                         /*!< TS_SYS::PRO_CTRL: PVD_EN Position       */
#define SYS_PRO_CTRL_PVD_EN_Msk             (0x1UL << SYS_PRO_CTRL_PVD_EN_Pos)          /*!< TS_SYS::PRO_CTRL: PVD_EN Mask           */
#define SYS_PRO_CTRL_PVD_THD_SEL_Pos        (1)                                         /*!< TS_SYS::PRO_CTRL: PVD_THD_SEL Position  */
#define SYS_PRO_CTRL_PVD_THD_SEL_Msk        (0x7UL << SYS_PRO_CTRL_PVD_THD_SEL_Pos)     /*!< TS_SYS::PRO_CTRL: PVD_THD_SEL Mask      */
#define SYS_PRO_CTRL_TSD_THD_SEL_Pos        (5)                                         /*!< TS_SYS::PRO_CTRL: TSD_THD_SEL Position  */
#define SYS_PRO_CTRL_TSD_THD_SEL_Msk        (0x1UL << SYS_PRO_CTRL_TSD_THD_SEL_Pos)     /*!< TS_SYS::PRO_CTRL: TSD_THD_SEL Mask      */
#define SYS_PRO_CTRL_TSD_RST_EN_Pos         (6)                                         /*!< TS_SYS::PRO_CTRL: TSD_RST_EN Position   */
#define SYS_PRO_CTRL_TSD_RST_EN_Msk         (0x1UL << SYS_PRO_CTRL_TSD_RST_EN_Pos)      /*!< TS_SYS::PRO_CTRL: TSD_RST_EN Mask       */

#define SYS_PRO_INTE_PVD_INT_EN_Pos         (0)                                         /*!< TS_SYS::PRO_INTE: PVD_INT_EN Position   */
#define SYS_PRO_INTE_PVD_INT_EN_Msk         (0x1UL << SYS_PRO_INTE_PVD_INT_EN_Pos)      /*!< TS_SYS::PRO_INTE: PVD_INT_EN Mask       */
#define SYS_PRO_INTE_TSD_INT_EN_Pos         (1)                                         /*!< TS_SYS::PRO_INTE: TSD_INT_EN Position   */
#define SYS_PRO_INTE_TSD_INT_EN_Msk         (0x1UL << SYS_PRO_INTE_TSD_INT_EN_Pos)      /*!< TS_SYS::PRO_INTE: TSD_INT_EN Mask       */

#define SYS_PRO_STSF_PVD_FLAG_Pos           (0)                                         /*!< TS_SYS::PRO_STSF: PVD_FLAG Position     */
#define SYS_PRO_STSF_PVD_FLAG_Msk           (0x1UL << SYS_PRO_STSF_PVD_FLAG_Pos)        /*!< TS_SYS::PRO_INTE: PVD_FLAG Mask         */
#define SYS_PRO_STSF_TSD_FLAG_Pos           (1)                                         /*!< TS_SYS::PRO_STSF: TSD_FLAG Position     */
#define SYS_PRO_STSF_TSD_FLAG_Msk           (0x1UL << SYS_PRO_STSF_TSD_FLAG_Pos)        /*!< TS_SYS::PRO_INTE: TSD_FLAG Mask         */

enum {
	_SYS_CPU_CLK_36M = 0,
	_SYS_CPU_CLK_24M = 1,
	_SYS_CPU_CLK_18M = 2,
	_SYS_CPU_CLK_12M = 3,
	_SYS_CPU_CLK_9M  = 4,
	_SYS_CPU_CLK_6M  = 5,
};

enum {
	_SYS_PLL_SRC_HIRC = 0,
	_SYS_PLL_SRC_XTAL = 1,
};

enum {
	_SYS_XTAL_PREDIV_1 = 0,
	_SYS_XTAL_PREDIV_2 = 1,
	_SYS_XTAL_PREDIV_3 = 2,
};

enum {
	_SYS_RST_SRC_POR  = 1,
	_SYS_RST_SRC_TMR  = 2,
	_SYS_RST_SRC_GPIO = 3,
	_SYS_RST_SRC_TCPC = 4,
};

enum {
	_SYS_PVD_THD_V2P4 = 0,
	_SYS_PVD_THD_V2P7 = 1,
	_SYS_PVD_THD_V3P0 = 2,
	_SYS_PVD_THD_V3P7 = 3,
	_SYS_PVD_THD_V4P0 = 4,
	_SYS_PVD_THD_V4P3 = 5,
};

enum {
	_SYS_TSD_THD_105C = 0,
	_SYS_TSD_THD_125C = 1,
};

#define PLL_CLK     (144000000) //144M
#define HIRC        (  8000000) //  8M
#define LIRC        (    64000) // 64K
#define HCLK        (SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 0 ? (36000000) \
		           : SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 1 ? (24000000) \
		           : SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 2 ? (18000000) \
		           : SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 3 ? (12000000) \
		           : SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 4 ? ( 9000000) \
		           : SYS->CLK_CTRL.BITS.CPU_CLK_SEL == 5 ? ( 6000000) \
		           : (36000000))

#define  SP3800         ( 3800)
#define  SP3802         ( 3802)
#define  SP3803         ( 3803)
#define  SP3811         ( 3811)
#define  SP3820         ( 3820)
#define NU17100         (17100)
#define NU17102         (17102)
#define NU17103         (17103)
#define NU17111         (17111)
#define NU17112         (17112)
#define NU17113         (17113)
#define NU17121         (17121)
#define NU17122         (17122)
#define NU17123         (17123)

#define CHIP_VER_A0     (    0)
#define CHIP_VER_A1     (    1)

#define NU103X_VER_A0   (    0)
#define NU103X_VER_A1   (    1)
/*------------------------------------------ SYS define ------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ TCPC define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	struct {
		uint32_t                        : 1;
		uint32_t                        : 1;
		uint32_t PHY_RX_DATA_ERROR_FLAG : 1; //USBPD_INT_FLAG
		uint32_t                        : 1;
		uint32_t                        : 1;
		uint32_t PHY_RX_BUFF_UPDAT_FLAG : 1; //USBPD_INT_FLAG
		uint32_t PHY_TX_BUFF_EMPTY_FLAG : 1; //USBPD_INT_FLAG
		uint32_t                        : 1;
		uint32_t CCA_STATUS_CHANGE_FLAG : 1; //TCPC_INT_FLAG
		uint32_t CCB_STATUS_CHANGE_FLAG : 1; //TCPC_INT_FLAG
		uint32_t PHY_RX_SUCCESSFUL_FLAG : 1; //USBPD_INT_FLAG
		uint32_t PHY_RX_HARD_RESET_FLAG : 1; //USBPD_INT_FLAG
		uint32_t PHY_TX_NO_GOODCRC_FLAG : 1; //USBPD_INT_FLAG
		uint32_t PHY_TX_CC_DISCARD_FLAG : 1; //USBPD_INT_FLAG
		uint32_t PHY_TX_SUCCESSFUL_FLAG : 1; //USBPD_INT_FLAG
		uint32_t                        :17;
	} BITS;
	uint32_t WORD;
} TS_TCPC_INT_FLAG;

typedef union {
	struct {
		uint32_t                        : 1;
		uint32_t                        : 1;
		uint32_t PHY_RX_DATA_ERROR_INTE : 1; //USBPD_INT_EN
		uint32_t                        : 1;
		uint32_t                        : 1;
		uint32_t PHY_RX_BUFF_UPDAT_INTE : 1; //USBPD_INT_EN
		uint32_t PHY_TX_BUFF_EMPTY_INTE : 1; //USBPD_INT_EN
		uint32_t                        : 1;
		uint32_t CCA_STATUS_CHANGE_TNTE : 1; //TCPC_INT_EN
		uint32_t CCB_STATUS_CHANGE_INTE : 1; //TCPC_INT_EN
		uint32_t PHY_RX_SUCCESSFUL_INTE : 1; //USBPD_INT_EN
		uint32_t PHY_RX_HARD_RESET_INTE : 1; //USBPD_INT_EN
		uint32_t PHY_TX_NO_GOODCRC_INTE : 1; //USBPD_INT_EN
		uint32_t PHY_TX_CC_DISCARD_INTE : 1; //USBPD_INT_EN
		uint32_t PHY_TX_SUCCESSFUL_INTE : 1; //USBPD_INT_EN
		uint32_t                        :17;
	} BITS;
	uint32_t WORD;
} TS_TCPC_INT_CTRL;

typedef union {
	struct {
		uint32_t CC1_ROLE : 2;
		uint32_t CC2_ROLE : 2;
		uint32_t RP_VALUE : 2;
		uint32_t DRP_MODE : 1;
		uint32_t          :25;
	} BITS;
	uint32_t WORD;
} TS_TCPC_CCx_ROLE;

typedef union {
	struct {
		uint32_t CC1_STATUS : 2;
		uint32_t CC2_STATUS : 2;
		uint32_t DRP_RESULT : 1;
		uint32_t DRP_STATUS : 1;
		uint32_t            : 2;
		uint32_t            : 4;
		uint32_t            : 4;
		uint32_t            :16;
	} BITS;
	uint32_t WORD;
} TS_TCPC_CCx_STAT;

typedef union {
	struct {
		uint32_t CMD_TYPE : 8;
		uint32_t          :24;
	} BITS;
	uint32_t WORD;
} TS_TCPC_CCx_CMD_;

typedef union {
	struct {
		uint32_t RXD_SOP_EN      : 1;
		uint32_t RXD_SOP1_EN     : 1;
		uint32_t RXD_SOP2_EN     : 1;
		uint32_t RXD_SOP1_DBG_EN : 1;
		uint32_t RXD_SOP2_DBG_EN : 1;
		uint32_t RXD_HARD_RST_EN : 1;
		uint32_t RXD_CABLERST_EN : 1;
		uint32_t RXD_DET_EOP_DIS : 1;
		uint32_t PHY_GDCRC_PPR   : 1;
		uint32_t PHY_GDCRC_REV   : 2;
		uint32_t PHY_GDCRC_PDR   : 1;
		uint32_t                 : 1;
		uint32_t                 :19;
	} BITS;
	uint32_t WORD;
} TS_TCPC_RXD_CTRL;

typedef union {
	struct {
		uint32_t RXD_MSG_HDR :16;
		uint32_t RXD_SOP_TYP : 8;
		uint32_t RXD_OBJ_CNT : 8; //byte count
	} BITS;
	uint32_t WORD;
} TS_TCPC_RXD_INFO;

typedef union {
	struct {
		uint32_t BYTE0 : 8;
		uint32_t BYTE1 : 8;
		uint32_t BYTE2 : 8;
		uint32_t BYTE3 : 8;
	} BITS;
	uint32_t WORD;
} TS_TCPC_RXD_BUFF;

typedef union {
	struct {
		uint32_t TXD_SOP_TYP : 3;
		uint32_t             :29;
	} BITS;
	uint32_t WORD;
} TS_TCPC_TXD_CTRL;

typedef union {
	struct {
		uint32_t TXD_MSG_HDR :16;
		uint32_t TXD_OBJ_CNT : 8; //byte count
		uint32_t TXD_ERR_IGG : 1;
		uint32_t             : 7;
	} BITS;
	uint32_t WORD;
} TS_TCPC_TXD_INFO;

typedef union {
	struct {
		uint32_t BYTE0 : 8;
		uint32_t BYTE1 : 8;
		uint32_t BYTE2 : 8;
		uint32_t BYTE3 : 8;
	} BITS;
	uint32_t WORD;
} TS_TCPC_TXD_BUFF;

typedef union {
	struct {
		uint32_t PD_PHY_EN      : 1;
		uint32_t PD_RX_VREF_SEL : 3;
		uint32_t PD_CC_PORT_SEL : 2;
		uint32_t PD_RX_DEBOUNCE : 1;
		uint32_t CC_RX_TBMC_SEL : 1;
		uint32_t                :24;
	} BITS;
	uint32_t WORD;
} TS_TCPC_PHY_CTRL;

typedef union {
	struct {
		uint32_t CCA_STAT : 3;
		uint32_t CCB_STAT : 3;
		uint32_t          :26;
	} BITS;
	uint32_t WORD;
} TS_TCPC_CCx_FSM_;

typedef union {
	struct {
		uint32_t CC_BLOCK_DIS : 1;
		uint32_t CC_LPMODE_EN : 1; //0-Low_Power Mode disable, the TCPC enter to manual mode, all comp and  Rp/Ip/Rd is controlled by Register
		uint32_t              : 1;
		uint32_t CC_DB_RD_DIS : 1;
		uint32_t              : 4;
		uint32_t CC_T_DRP_SEL : 2;
		uint32_t CC_DCSRC_DRP : 2;
		uint32_t              : 4;
		uint32_t CC_LPMODE_RP : 1; //standby mode Rp source select
		uint32_t CC_PD_CH_SEL : 1;
		uint32_t              :14;
	} BITS;
	uint32_t WORD;
} TS_TCPC_CCx_CTRL;

typedef struct {
	__IO TS_TCPC_INT_FLAG INT_FLAG; //4000_2000
	__IO TS_TCPC_INT_CTRL INT_CTRL; //4000_2004
	__IO TS_TCPC_CCx_ROLE CCA_ROLE; //4000_2008
	__I  TS_TCPC_CCx_STAT CCA_STAT; //4000_200C
	__IO TS_TCPC_CCx_CMD_ CCA_CMD_; //4000_2010
	__IO TS_TCPC_CCx_ROLE CCB_ROLE; //4000_2014
	__I  TS_TCPC_CCx_STAT CCB_STAT; //4000_2018
	__IO TS_TCPC_CCx_CMD_ CCB_CMD_; //4000_201C
	__IO TS_TCPC_RXD_CTRL RXD_CTRL; //4000_2020
	__I  TS_TCPC_RXD_INFO RXD_INFO; //4000_2024
	__I  TS_TCPC_RXD_BUFF RXD_BUFF; //4000_2028
	__IO TS_TCPC_TXD_CTRL TXD_CTRL; //4000_202C
	__IO TS_TCPC_TXD_INFO TXD_INFO; //4000_2030
	__IO TS_TCPC_TXD_BUFF TXD_BUFF; //4000_2034
	__IO TS_TCPC_PHY_CTRL PHY_CTRL; //4000_2038
	__I  TS_TCPC_CCx_FSM_ FSM_STAT; //4000_203C
	__IO TS_TCPC_CCx_CTRL CCA_CTRL; //4000_2040
	__IO TS_TCPC_CCx_CTRL CCB_CTRL; //4000_2044
} TS_TCPC;

#define TCPC_INT_FLAG_MSK_PHY_RX_DATA_ERROR    0x0004
#define TCPC_INT_FLAG_MSK_PHY_RX_BUFF_UPDAT    0x0020
#define TCPC_INT_FLAG_MSK_PHY_TX_BUFF_EMPTY    0x0040
#define TCPC_INT_FLAG_MSK_CCA_STATUS_CHANGE    0x0100
#define TCPC_INT_FLAG_MSK_CCB_STATUS_CHANGE    0x0200
#define TCPC_INT_FLAG_MSK_PHY_RX_SUCCESSFUL    0x0400
#define TCPC_INT_FLAG_MSK_PHY_RX_HARD_RESET    0x0800
#define TCPC_INT_FLAG_MSK_PHY_TX_NO_GOODCRC    0x1000
#define TCPC_INT_FLAG_MSK_PHY_TX_CC_DISCARD    0x2000
#define TCPC_INT_FLAG_MSK_PHY_TX_SUCCESSFUL    0x4000
/*------------------------------------------ TCPC define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ UART define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_UART::GEN_CTRL
	 * Offset: 0x00  UARTx general control register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |RXD_ISTS      |Receive interrupt status of UART, read only
	 * |        |              |0 = A received interrupt status has not occurred.
	 * |        |              |1 = A received interrupt status has occurred.
	 * |        |              |Note:
	 * |        |              |1. Set by hardware at the end of the 8th bit time in mode0, or at the beginning of the stop bit in other modes.
	 * |        |              |2. This bit is the real-time status of interrupt pulse, not recommended software cannot use it.
	 * |[1]     |TXD_ISTS      |Transmit interrupt status of UART, read only
	 * |        |              |0 = A transmit interrupt status has not occurred.
	 * |        |              |1 = A transmit interrupt status has occurred.
	 * |        |              |Note:
	 * |        |              |1. Set by hardware at the end of the 8th bit time in mode0, or at the beginning of the stop bit in other modes.
	 * |        |              |2. This bit is the real-time status of interrupt pulse, not recommended software cannot use it.
	 * |[2]     |RXD_BIT8      |The 9th bit that received of UART
	 * |        |              |In mode0, RXD_BIT8 is not used
	 * |        |              |In mode1, if receive interrupt occurs, RXD_BIT8 is the stop bit that was received
	 * |        |              |In mode2/3, it is the 9th bit that was received
	 * |[3]     |TXD_BIT8      |The 9th bit that transmit of UART
	 * |        |              |In mode0, TXD_BIT8 is not used
	 * |        |              |In mode1/2/3, TXD_BIT8 will be transmit at the 9th bit, this bit set or clear by software
	 * |[4]     |RX_ENABLE     |UART receive enable bit
	 * |        |              |0: receive disable in mode1/2/3, mode0 receiver/transmit enable
	 * |        |              |1: receive enable, mode0 transmit disable
	 * |[5]     |MUTI_MOD      |UART serial communication interface muti_mode enable bit (9th bit'1' checker)
	 * |        |              |0: in mode0, baud rate is 1/12 of system clock
	 * |        |              |   in mode1, disable stop bit validation check, any stop bit will set RXD_ISTS to generate interrupt
	 * |        |              |   in mode2/3, any byte will set RXD_ISTS to generate interrupt
	 * |        |              |1: in mode0, baud rate is 1/4 of system clock
	 * |        |              |   in mode1, enable stop bit validation check, only valid stop bit(1) will set RXD_ISTS to generate interrupt
	 * |        |              |   in mode2/3, only address byte (9th bit = 1) will set RXD_ISTS to generate interrupt
	 * |[7:6]   |USCI_MOD      |UART serial communication interface mode control bits
	 * |        |              |00 = mode0, synchronous mode, fixed baud rate
	 * |        |              |01 = mode1, 8bit asynchronous mode, variable baud rate
	 * |        |              |10 = mode2, 9bit asynchronous mode, fixed baud rate
	 * |        |              |11 = mode3, 9bit asynchronous mode, variable baud rate
	 * |[9]     |RXOV_DIS      |UART receive overrun disable bit
	 * |        |              |0: enable
	 * |        |              |1: disable
	 * |[10]    |RXFE_DIS      |UART receive frame error disable bit
	 * |        |              |0: enable
	 * |        |              |1: disable
	 * |[11]    |BRDBL_EN      |baud rate double enable bit
	 * |        |              |0: disable
	 * |        |              |1: if set in mode2, the baud rate of UART is doubled
	 */
	struct {
		uint32_t RXD_ISTS : 1; //RO, RX interrupt status of UART, cleared by HW
		uint32_t TXD_ISTS : 1; //RO, RX interrupt status of UART, cleared by HW
		uint32_t RXD_BIT8 : 1;
		uint32_t TXD_BIT8 : 1;
		uint32_t RXD_EN   : 1;
		uint32_t MUTI_MOD : 1;
		uint32_t USCI_MOD : 2;
		uint32_t          : 1;
		uint32_t RXOV_DIS : 1; //default:1, disable RX overrun.
		uint32_t RXFE_DIS : 1; //default:1, disable frame error.
		uint32_t BRDBL_EN : 1;
		uint32_t          :20;
	} BITS;
	uint32_t WORD;
} TS_UART_GEN_CTRL;

typedef union {
	/**
	 * @var TS_UART::DAT_BUFF
	 * Offset: 0x04  UARTx TXD Data Buffer Register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[7:0]   |DATA          |UARTx TX Data Buffer Register
	 * |        |              |for TXD, this is the transmit buffer
	 * |        |              |Note:
	 * |        |              |  TXD and RXD share the same buffer.
	 */
	struct {
		uint32_t DATA : 8;
		uint32_t      :24;
	} TXDB;
	/**
	 * @var TS_UART::DAT_BUFF
	 * Offset: 0x04  UARTx RX Data Buffer Register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[7:0]   |DATA          |UARTx RX Data Buffer Register
	 * |        |              |for RXD, this is the receiver buffer
	 * |        |              |Note:
	 * |        |              |  TXD and RXD share the same buffer.
	 */
	struct {
		uint32_t DATA : 8;
		uint32_t      :24;
	} RXDB;
	uint32_t WORD;
} TS_UART_DAT_BUFF;

typedef union {
	/**
	 * @var TS_UART::SLA_ADDR
	 * Offset: 0x08  UARTx slave address and address mask register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[7:0]   |SADDR         |UARTx slave address
	 * |        |              |SADDR defines the UART slave address
	 */
	struct {
		uint32_t SADDR : 8;
		uint32_t       :24;
	} BITS;
	uint32_t WORD;
} TS_UART_SLA_ADDR;

typedef union {
	/**
	 * @var TS_UART::SLA_ADEN
	 * Offset: 0x0C  UARTx slave address enable register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[7:0]   |SADEN         |UARTx slave address enable
	 * |        |              |SADEN is a bit mask to determine which bits of SADDR are checked against a received address
	 */
	struct {
		uint32_t SADEN : 8;
		uint32_t       :24;
	} BITS;
	uint32_t WORD;
} TS_UART_SLA_ADEN;

typedef union {
	/**
	 * @var TS_UART::BRG_CTRL
	 * Offset: 0x18  UARTx baud rate generator control register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[14:0]  |CLK_CNT       |UARTx pre_divider for sample counter
	 * |        |              |This bit field defines the divide ratio of the clock division from sample clock
	 * |        |              |baud_rate = HCLK / ( (CLK_DIV + 1) * (0x7FFF - CLK_CNT + 1))
	 * |[15]    |BRG_EN        |UARTx baud rate generator enable bit
	 * |        |              |0: disable
	 * |        |              |1: enable
	 * |[22:16] |CLK_DIV       |UARTx baud rate clock divider bits
	 * |        |              |This bit field defines the ratio between the protocol clock frequency and the clock divider frequency.
	 * |        |              |baud_rate = HCLK / ( (CLK_DIV + 1) * (0x7FFF - CLK_CNT + 1))
	 */
	struct {
		uint32_t CLK_CNT  :15;
		uint32_t BRG_EN   : 1;
		uint32_t CLK_DIV  : 7;
		uint32_t          : 9;
	} BITS;
	uint32_t WORD;
} TS_UART_BRG_CTRL;

typedef union {
	/**
	 * @var TS_UART::STS_FLAG
	 * Offset: 0x18  UARTx protocol status flag register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |RXEND_FLAG    |UARTx Receive End Interrupt Flag
	 * |        |              |0 = A receive finish interrupt status has not occurred.
	 * |        |              |1 = A receive finish interrupt status has occurred.
	 * |        |              |Note: This bit can be cleared by writing "1" to it.
	 * |[1]     |TXEND_FLAG    |UARTx Transmit End Interrupt Flag
	 * |        |              |0 = A transmit end interrupt status has not occurred.
	 * |        |              |1 = A transmit end interrupt status has occurred.
	 * |        |              |Note: This bit can be cleared by writing "1" to it.
	 * |[2]     |FMERR_FLAG    |UARTx Framing Error Flag
	 * |        |              |This bit is set to logic 1 whenever the received character does not have a valid "stop bit",
	 * |        |              |that is, the stop bit following the last data bit or parity bit is detected as logic 0.
	 * |        |              |0 = No framing error is generated.
	 * |        |              |1 = Framing error is generated.
	 * |        |              |Note: This bit can be cleared by writing "1" to it.
	 * |[3]     |RXDOV_FLAG    |UARTx receive data overflow flag
	 * |        |              |0 = data receive is not overflow.
	 * |        |              |1 = data receive is overflow
	 * |        |              |Note: This bit can be cleared by writing "1" to it.
	 * |[4]     |TXCOL_FLAG    |UARTx transmit collision flag
	 * |        |              |0 = A transmit collision has not occurred.
	 * |        |              |1 = A transmit collision has not occurred.
	 * |        |              |Note: This bit can be cleared by writing "1" to it.
	 */
	struct {
		uint32_t RXEND_FLAG : 1;
		uint32_t TXEND_FLAG : 1;
		uint32_t FMERR_FLAG : 1;
		uint32_t RXDOV_FLAG : 1;
		uint32_t TXCOL_FLAG : 1;
		uint32_t            :27;
	} BITS;
	uint32_t WORD;
} TS_UART_STS_FLAG;

typedef struct {
	/*--------------------------------- UART1 --- UART2 -*/
	__IO TS_UART_GEN_CTRL GEN_CTRL; //4000_3000 4000_4000
	__IO TS_UART_DAT_BUFF DAT_BUFF; //4000_3004 4000_4004
	__IO TS_UART_SLA_ADDR SLA_ADDR; //4000_3008 4000_4008
	__IO TS_UART_SLA_ADEN SLA_ADEN; //4000_300C 4000_400C
	__IO uint32_t         RESERVE0; //4000_3010 4000_4010
	__IO uint32_t         RESERVE1; //4000_3014 4000_4014
	__IO TS_UART_BRG_CTRL BRG_CTRL; //4000_3018 4000_4018
	__IO TS_UART_STS_FLAG STS_FLAG; //4000_301C 4000_401C
} TS_UART;

#define UART_GEN_CTRL_RXD_ISTS_Pos                (0)                                      /*!< TS_UART::GEN_CTRL: RXD_ISTS Position    */
#define UART_GEN_CTRL_RXD_ISTS_Msk                (0x1UL << UART_GEN_CTRL_RXD_ISTS_Pos)    /*!< TS_UART::GEN_CTRL: RXD_ISTS Mask        */
#define UART_GEN_CTRL_TXD_ISTS_Pos                (1)                                      /*!< TS_UART::GEN_CTRL: TXD_ISTS Position    */
#define UART_GEN_CTRL_TXD_ISTS_Msk                (0x1UL << UART_GEN_CTRL_TXD_ISTS_Pos)    /*!< TS_UART::GEN_CTRL: TXD_ISTS Mask        */
#define UART_GEN_CTRL_RXD_BIT8_Pos                (2)                                      /*!< TS_UART::GEN_CTRL: RXD_BIT8 Position    */
#define UART_GEN_CTRL_RXD_BIT8_Msk                (0x1UL << UART_GEN_CTRL_RXD_BIT8_Pos)    /*!< TS_UART::GEN_CTRL: RXD_BIT8 Mask        */
#define UART_GEN_CTRL_TXD_BIT8_Pos                (3)                                      /*!< TS_UART::GEN_CTRL: RXD_BIT8 Position    */
#define UART_GEN_CTRL_TXD_BIT8_Msk                (0x1UL << UART_GEN_CTRL_TXD_BIT8_Pos)    /*!< TS_UART::GEN_CTRL: RXD_BIT8 Mask        */
#define UART_GEN_CTRL_RXD_EN_Pos                  (4)                                      /*!< TS_UART::GEN_CTRL: RXD_EN Position      */
#define UART_GEN_CTRL_RXD_EN_Msk                  (0x1UL << UART_GEN_CTRL_RXD_EN_Pos)      /*!< TS_UART::GEN_CTRL: RXD_EN Mask          */
#define UART_GEN_CTRL_MUTI_MOD_Pos                (5)                                      /*!< TS_UART::GEN_CTRL: MUTI_MOD Position    */
#define UART_GEN_CTRL_MUTI_MOD_Msk                (0x1UL << UART_GEN_CTRL_MUTI_MOD_Pos)    /*!< TS_UART::GEN_CTRL: MUTI_MOD Mask        */
#define UART_GEN_CTRL_USCI_MOD_Pos                (6)                                      /*!< TS_UART::GEN_CTRL: USCI_MOD Position    */
#define UART_GEN_CTRL_USCI_MOD_Msk                (0x3UL << UART_GEN_CTRL_USCI_MOD_Pos)    /*!< TS_UART::GEN_CTRL: USCI_MOD Mask        */
#define UART_GEN_CTRL_RXOV_DIS_Pos                (9)                                      /*!< TS_UART::GEN_CTRL: RXOV_DIS Position    */
#define UART_GEN_CTRL_RXOV_DIS_Msk                (0x1UL << UART_GEN_CTRL_RXOV_DIS_Pos)    /*!< TS_UART::GEN_CTRL: RXOV_DIS Mask        */
#define UART_GEN_CTRL_RXFE_DIS_Pos                (10)                                     /*!< TS_UART::GEN_CTRL: RXFE_DIS Position    */
#define UART_GEN_CTRL_RXFE_DIS_Msk                (0x1UL << UART_GEN_CTRL_RXFE_DIS_Pos)    /*!< TS_UART::GEN_CTRL: RXFE_DIS Mask        */
#define UART_GEN_CTRL_BRDBL_EN_Pos                (11)                                     /*!< TS_UART::GEN_CTRL: BRDBL_EN Position    */
#define UART_GEN_CTRL_BRDBL_EN_Msk                (0x1UL << UART_GEN_CTRL_BRDBL_EN_Pos)    /*!< TS_UART::GEN_CTRL: BRDBL_EN Mask        */

#define UART_DAT_BUFF_TXDB_DATA_Pos               (0)                                      /*!< TS_UART::DAT_BUFF: DATA Position        */
#define UART_DAT_BUFF_TXDB_DATA_Msk               (0xFFUL << UART_DAT_BUFF_TXDB_DATA_Pos)  /*!< TS_UART::DAT_BUFF: DATA Mask            */
#define UART_DAT_BUFF_RXDB_DATA_Pos               (0)                                      /*!< TS_UART::DAT_BUFF: DATA Position        */
#define UART_DAT_BUFF_RXDB_DATA_Msk               (0xFFUL << UART_DAT_BUFF_RXDB_DATA_Pos)  /*!< TS_UART::DAT_BUFF: DATA Mask            */

#define UART_SLA_ADDR_SADDR_Pos                   (0)                                      /*!< TS_UART::SLA_ADDR: SADDR Position       */
#define UART_SLA_ADDR_SADDR_Msk                   (0xFFUL << UART_SLA_ADDR_SADDR_Pos)      /*!< TS_UART::SLA_ADDR: SADDR Mask           */
#define UART_SLA_ADEN_SADEN_Pos                   (0)                                      /*!< TS_UART::SLA_ADEN: SADEN Position       */
#define UART_SLA_ADEN_SADEN_Msk                   (0xFFUL << UART_SLA_ADEN_SADEN_Pos)      /*!< TS_UART::SLA_ADEN: SADEN Mask           */

#define UART_BRG_CTRL_CLK_CNT_Pos                 (0)                                      /*!< TS_UART::BRG_CTRL: CLK_CNT Position     */
#define UART_BRG_CTRL_CLK_CNT_Msk                 (0x7FFFUL << UART_BRG_CTRL_CLK_CNT_Pos)  /*!< TS_UART::BRG_CTRL: CLK_CNT Mask         */
#define UART_BRG_CTRL_BRG_EN_Pos                  (15)                                     /*!< TS_UART::BRG_CTRL: BRG_EN Position      */
#define UART_BRG_CTRL_BRG_EN_Msk                  (0x1UL << UART_BRG_CTRL_BRG_EN_Pos)      /*!< TS_UART::BRG_CTRL: BRG_EN Mask          */
#define UART_BRG_CTRL_CLK_DIV_Pos                 (16)                                     /*!< TS_UART::BRG_CTRL: CLK_DIV Position     */
#define UART_BRG_CTRL_CLK_DIV_Msk                 (0x3FUL << UART_BRG_CTRL_CLK_DIV_Pos)    /*!< TS_UART::BRG_CTRL: CLK_DIV Mask         */

#define UART_STS_FLAG_RXEND_FLAG_Pos              (0)                                      /*!< TS_UART::STS_FLAG: RXEND_FLAG Position  */
#define UART_STS_FLAG_RXEND_FLAG_Msk              (0x1UL << UART_STS_FLAG_RXEND_FLAG_Pos)  /*!< TS_UART::STS_FLAG: RXEND_FLAG Mask      */
#define UART_STS_FLAG_TXEND_FLAG_Pos              (1)                                      /*!< TS_UART::STS_FLAG: TXEND_FLAG Position  */
#define UART_STS_FLAG_TXEND_FLAG_Msk              (0x1UL << UART_STS_FLAG_TXEND_FLAG_Pos)  /*!< TS_UART::STS_FLAG: TXEND_FLAG Mask      */
#define UART_STS_FLAG_FMERR_FLAG_Pos              (2)                                      /*!< TS_UART::STS_FLAG: FMERR_FLAG Position  */
#define UART_STS_FLAG_FMERR_FLAG_Msk              (0x1UL << UART_STS_FLAG_FMERR_FLAG_Pos)  /*!< TS_UART::STS_FLAG: FMERR_FLAG Mask      */
#define UART_STS_FLAG_RXDOV_FLAG_Pos              (3)                                      /*!< TS_UART::STS_FLAG: RXDOV_FLAG Position  */
#define UART_STS_FLAG_RXDOV_FLAG_Msk              (0x1UL << UART_STS_FLAG_RXDOV_FLAG_Pos)  /*!< TS_UART::STS_FLAG: RXDOV_FLAG Mask      */
#define UART_STS_FLAG_TXCOL_FLAG_Pos              (4)                                      /*!< TS_UART::STS_FLAG: TXCOL_FLAG Position  */
#define UART_STS_FLAG_TXCOL_FLAG_Msk              (0x1UL << UART_STS_FLAG_TXCOL_FLAG_Pos)  /*!< TS_UART::STS_FLAG: TXCOL_FLAG Mask      */

enum {
	_UART_USCI_MODE_0 = 0,
	_UART_USCI_MODE_1 = 1,
	_UART_USCI_MODE_2 = 2,
	_UART_USCI_MODE_3 = 3,
};
/*------------------------------------------ UART define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ GPIO define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	struct {
		uint32_t PIN0 : 1;
		uint32_t PIN1 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t PIN4 : 1;
		uint32_t PIN5 : 1;
		uint32_t PIN6 : 1;
		uint32_t PIN7 : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPA_PINx_CTRL;

typedef union {
	struct {
		uint32_t PIN0 : 2; //00:SCL1_S 01:PA0 10:UART2_TXD 11:DP_C
		uint32_t PIN1 : 2; //00:SDA1_S 01:PA1 10:UART2_RXD 11:DM_C
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t PIN4 : 2; //00:PA4 01:SDA3_S 10:RESERVED 11:RESERVED
		uint32_t PIN5 : 2; //00:PA5 01:SCL3_S 10:LS_ISNS_PGA_N 11:RESERVED
		uint32_t PIN6 : 2; //00:PA6 01:SCL2_M 10:RESERVED 11:RESERVED
		uint32_t PIN7 : 2; //00:PA7 01:SDA2_M 10:RESERVED 11:RESERVED
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPA_MODE_CTRL;

typedef union {
	struct {
		uint32_t      : 1;
		uint32_t PIN1 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPA_INTx_CTRL;

typedef union {
	struct {
		uint32_t      : 2;
		uint32_t PIN1 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPA_ITTY_CTRL;

typedef struct {
	__I  TS_GPA_PINx_CTRL D_IN; //4000_5000, input data register
	__IO TS_GPA_PINx_CTRL I_EN; //4000_5004, input enable register
	__IO TS_GPA_PINx_CTRL DOUT; //4000_5008, output data register
	__IO TS_GPA_PINx_CTRL O_EN; //4000_500C, output enable register
	__IO TS_GPA_PINx_CTRL ODEN; //4000_5010, open drain enable control register
	__IO TS_GPA_PINx_CTRL PUEN; //4000_5014, pull-up enable control register
	__IO TS_GPA_PINx_CTRL PDEN; //4000_5018, pull-down enable control register
	__IO TS_GPA_MODE_CTRL MODE; //4000_501C, alternate function control register
	__IO TS_GPA_INTx_CTRL ITEN; //4000_5020, interrupt function enable register
	__IO TS_GPA_ITTY_CTRL ITTP; //4000_5024, interrupt trigger mode register
	__IO TS_GPA_INTx_CTRL FLAG; //4000_5028, interrupt flag register
} TS_GPA;

/*--------------------------------------------------------------------------*/
typedef union {
	struct {
		uint32_t PIN0 : 1;
		uint32_t PIN1 : 1;
		uint32_t PIN2 : 1;
		uint32_t PIN3 : 1;
		uint32_t PIN4 : 1;
		uint32_t PIN5 : 1;
		uint32_t PIN6 : 1;
		uint32_t PIN7 : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPB_PINx_CTRL;

typedef union {
	struct {
		uint32_t PIN0 : 2; //00:PB0 01:OSC_IN 10:RESERVED 11:RESERVED
		uint32_t PIN1 : 2; //00:PB1 01:OSC_OUT 10:RESERVED 11:RESERVED
		uint32_t PIN2 : 2; //00:PB2 01:BPWM3 10:BADC2 11:DP_C2
		uint32_t PIN3 : 2; //00:PB3 01:JTAG_CLK 10:BPWM7 11:RESERVED
		uint32_t PIN4 : 2; //00:PB4 01:JTAG_DAT 10:BPWM8 11:RESERVED
		uint32_t PIN5 : 2; //00:PB5 01:JTAG_RST 10:BADC6 11:LS_ISNS_PGA_P
		uint32_t PIN6 : 2; //00:PB6 01:BADC7 10:RESERVED 11:RESERVED
		uint32_t PIN7 : 2; //00:PB7 01:UART1_TXD 10:RESERVED 11:RESERVED
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPB_MODE_CTRL;

typedef union {
	struct {
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t PIN4 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPB_INTx_CTRL;

typedef union {
	struct {
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t PIN4 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPB_ITTY_CTRL;

typedef struct {
	__I  TS_GPB_PINx_CTRL D_IN; //4000_5040, input data register
	__IO TS_GPB_PINx_CTRL I_EN; //4000_5044, input enable register
	__IO TS_GPB_PINx_CTRL DOUT; //4000_5048, output data register
	__IO TS_GPB_PINx_CTRL O_EN; //4000_504C, output enable register
	__IO TS_GPB_PINx_CTRL ODEN; //4000_5050, open drain enable control register
	__IO TS_GPB_PINx_CTRL PUEN; //4000_5054, pull-up enable control register
	__IO TS_GPB_PINx_CTRL PDEN; //4000_5058, pull-down enable control register
	__IO TS_GPB_MODE_CTRL MODE; //4000_505C, alternate function control register
	__IO TS_GPB_INTx_CTRL ITEN; //4000_5060, interrupt function enable register
	__IO TS_GPB_ITTY_CTRL ITTP; //4000_5064, interrupt trigger mode register
	__IO TS_GPB_INTx_CTRL FLAG; //4000_5068, interrupt flag register
} TS_GPB;

/*--------------------------------------------------------------------------*/
typedef union {
	struct {
		uint32_t PIN0 : 1;
		uint32_t PIN1 : 1;
		uint32_t PIN2 : 1;
		uint32_t PIN3 : 1;
		uint32_t PIN4 : 1;
		uint32_t PIN5 : 1;
		uint32_t PIN6 : 1;
		uint32_t PIN7 : 1;
		uint32_t PIN8 : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPC_PINx_CTRL;

typedef union {
	struct {
		uint32_t PIN0 : 2; //00:PC0 01:EPWM1 10:RESERVED 11:RESERVED
		uint32_t PIN1 : 2; //00:PC1 01:EPWM2 10:RESERVED 11:RESERVED
		uint32_t PIN2 : 2; //00:PC2 01:RESERVED 10:RESERVED 11:RESERVED
		uint32_t PIN3 : 2; //00:PC3 01:BPWM4 10:DP_A1 11:RESERVED
		uint32_t PIN4 : 2; //00:PC4 01:EPWM5 10:DM_A1 11:RESERVED
		uint32_t PIN5 : 2; //00:PC5 01:EPWM6 10:RESERVED 11:RESERVED
		uint32_t PIN6 : 2; //00:PC6 01:BADC1 10:ECAP4 11:RESERVED
		uint32_t PIN7 : 2; //00:PC7 01:BADC4 10:RESERVED 11:RESERVED
		uint32_t PIN8 : 2; //00:CC2_L 01:PC8 10:BADC5 11:RESERVED
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPC_MODE_CTRL;

typedef union {
	struct {
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t PIN2 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t PIN6 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPC_INTx_CTRL;

typedef union {
	struct {
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t PIN2 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t PIN6 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPC_ITTY_CTRL;

typedef struct {
	__I  TS_GPC_PINx_CTRL D_IN; //4000_5080, input data register
	__IO TS_GPC_PINx_CTRL I_EN; //4000_5084, input enable register
	__IO TS_GPC_PINx_CTRL DOUT; //4000_5088, output data register
	__IO TS_GPC_PINx_CTRL O_EN; //4000_508C, output enable register
	__IO TS_GPC_PINx_CTRL ODEN; //4000_5090, open drain enable control register
	__IO TS_GPC_PINx_CTRL PUEN; //4000_5094, pull-up enable control register
	__IO TS_GPC_PINx_CTRL PDEN; //4000_5098, pull-down enable control register
	__IO TS_GPC_MODE_CTRL MODE; //4000_509C, alternate function control register
	__IO TS_GPC_INTx_CTRL ITEN; //4000_50A0, interrupt function enable register
	__IO TS_GPC_ITTY_CTRL ITTP; //4000_50A4, interrupt trigger mode register
	__IO TS_GPC_INTx_CTRL FLAG; //4000_50A8, interrupt flag register
} TS_GPC;

/*--------------------------------------------------------------------------*/
typedef union {
	struct {
		uint32_t PIN0 : 1;
		uint32_t PIN1 : 1;
		uint32_t PIN2 : 1;
		uint32_t PIN3 : 1;
		uint32_t PIN4 : 1;
		uint32_t PIN5 : 1;
		uint32_t PIN6 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPD_PINx_CTRL;

typedef union {
	struct {
		uint32_t PIN0 : 2; //00:PD0 01:BADC8 10:DM_C2 11:RESERVED
		uint32_t PIN1 : 2; //00:PD1 01:UART1_RXD 10:ECAP5 11:RESERVED
		uint32_t PIN2 : 2; //00:CC1_L 01:PD2 10:ECAP3 11:RESERVED
		uint32_t PIN3 : 2; //00:PD3 01:BADC9 10:RESERVED 11:RESERVED
		uint32_t PIN4 : 2; //00:PD4 01:ECAP1 10:RESERVED 11:RESERVED
		uint32_t PIN5 : 2; //00:PD5 01:ECAP2 10:RESERVED 11:RESERVED
		uint32_t PIN6 : 2; //00:PD6 01:BADC3 10:RESERVED 11:RESERVED
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPD_MODE_CTRL;

typedef union {
	struct {
		uint32_t      : 1;
		uint32_t PIN1 : 1;
		uint32_t      : 1;
		uint32_t PIN3 : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      : 1;
		uint32_t      :23;
	} BITS;
	uint32_t WORD;
} TS_GPD_INTx_CTRL;

typedef union {
	struct {
		uint32_t      : 2;
		uint32_t PIN1 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t PIN3 : 2; //00:Falling Edge 01:Rising Edge 1x:both edge
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      : 2;
		uint32_t      :14;
	} BITS;
	uint32_t WORD;
} TS_GPD_ITTY_CTRL;

typedef struct {
	__I  TS_GPD_PINx_CTRL D_IN; //4000_50C0, input data register
	__IO TS_GPD_PINx_CTRL I_EN; //4000_50C4, input enable register
	__IO uint32_t         REV0; //4000_50C8,
	__IO uint32_t         REV1; //4000_50CC,
	__IO uint32_t         REV2; //4000_50D0,
	__IO uint32_t         REV3; //4000_50D4,
	__IO uint32_t         REV4; //4000_50D8,
	__IO TS_GPD_MODE_CTRL MODE; //4000_50DC, alternate function control register
	__IO TS_GPD_INTx_CTRL ITEN; //4000_50E0, interrupt function enable register
	__IO TS_GPD_ITTY_CTRL ITTP; //4000_50E4, interrupt trigger mode register
	__IO TS_GPD_INTx_CTRL FLAG; //4000_50E8, interrupt flag register
} TS_GPD;
/*------------------------------------------ GPIO define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ ECAP define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_ECAPx::GEN_CTRL, x = 1, 2, 3, 4, 5
	 * Offset: 0x00, 0x20, 0x40, 0x60, 0x80, ECAPx General Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |CAP_EN            |ECAPx input capture timer/counter enable bit
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |[1]     |EDGE_DET_INT_EN   |ECAPx edge detect interrupt enable bit
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |[2]     |OVERFLOW_INT_EN   |ECAPx overflow interrupt enable bit
	 * |        |                  |0 = Disable OVERFLOW_FLAG can trigger ECAPx interrupt
	 * |        |                  |1 = Enable OVERFLOW_FLAG can trigger ECAPx interrupt
	 * |        |                  |Note: The overflow function is only available when ECAPx is working in DDM mode.
	 * |[4:3]   |DEGLITCH_TIME_SEL |ECAPx de_glitch time select
	 * |        |                  |0 = 80us
	 * |        |                  |1 = 60us
	 * |        |                  |2 = 40us
	 * |        |                  |3 =  0us
	 * |        |                  |Note: The de_giltch time is effective for both DDM and QDT mode, for QDT mode you should set [DEGLITCH_TIME_SEL = 0us].
	 * |[6:5]   |INPUT_CHANNEL_SEL |ECAPx input channel source select
	 * |        |                  |ECAP1: 0->external pad pin(PD4), others->reserved
	 * |        |                  |ECAP2: 0->external pad pin(PD5), 1->internal digital demodulation PHA_OUT, others->reserved
	 * |        |                  |ECAP3: 0->external pad pin(PD2), others->reserved
	 * |        |                  |ECAP4: 0->external pad pin(PC6), 1->internal digital demodulation MAG_OUT, others->reserved
	 * |        |                  |ECAP5: 0->external pad pin(PD1), others->reserved
	 * |[7]     |FUNC_WORKING_MODE |ECAPx working mode select
	 * |        |                  |0 = general edge detect for DDM
	 * |        |                  |1 = special edge detect for QDT
	 * |        |                  |Note: only for x = 1, 2, 3, 5. ECAP4 fixed to work in DDM mode
	 */
	struct {
		uint32_t CAP_EN            : 1;
		uint32_t EDGE_DET_INT_EN   : 1;
		uint32_t OVERFLOW_INT_EN   : 1;
		uint32_t DEGLITCH_TIME_SEL : 2;
		uint32_t INPUT_CHANNEL_SEL : 2;
		uint32_t FUNC_WORKING_MODE : 1; //0:DDM 1:QDT
		uint32_t                   :24;
	} BITS;
	uint32_t WORD;
} TS_ECAP_GEN_CTRL;

typedef union {
	/**
	 * @var TS_ECAPx::STS_FLAG, x = 1, 2, 3, 4, 5
	 * Offset: 0x04, 0x24, 0x44, 0x64, 0x84, ECAPx DDM mode status flag register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |EDGE_DET_FLAG     |ECAPx input capture Captured Flag
	 * |        |                  |for DDM mode
	 * |        |                  |0 = No valid edge change is detected
	 * |        |                  |1 =  A valid edge change is detected
	 * |        |                  |Note: This bit is only cleared by writing 1 to itself through software.
	 * |[1]     |OVERFLOW_FLAG     |ECAPx input capture counter overflow flag
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |Note: The overflow function is only available when ECAPx is working in DDM mode.
	 * |        |                  |Note: This bit is only cleared by writing 1 to itself through software.
	 */
	struct {
		uint32_t EDGE_DET_FLAG : 1;
		uint32_t OVERFLOW_FLAG : 1;
		uint32_t               :30;
	} _DDM;

	/**
	 * @var TS_ECAPx::STS_FLAG, x = 1, 2, 3, 5
	 * Offset: 0x04, 0x24, 0x44, 0x84, ECAPx QDT mode status flag register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |QDT_DONE_FLAG     |ECAPx QDT detect done flag
	 * |        |                  |for QDT mode
	 * |        |                  |ECAP1: 0->not done, 1->QDT Q-factor decay time measures done
	 * |        |                  |ECAP2: 0->not done, 1->QDT resonance frequency measures done
	 * |        |                  |ECAP3: 0->not done, 1->QDT resonance frequency measures done
	 * |        |                  |ECAP5: 0->not done, 1->QDT Q-factor decay time measures done
	 * |        |                  |Note: This bit is only cleared by writing 1 to itself through software.
	 */
	struct {
		uint32_t QDT_DONE_FLAG : 1;
		uint32_t               :31;
	} _QDT;

	uint32_t WORD;
} TS_ECAP_STS_FLAG;

typedef union {
	/**
	 * @var TS_ECAPx::DDM_CTRL, x = 1, 2, 3, 4, 5
	 * Offset: 0x08, 0x28, 0x48, 0x68, 0x88, ECAPx DMM mode Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[1:0]   |EDGE_TYPE_SEL     |ECAPx trigger edge selection
	 * |        |                  |ECAPx can detect falling edge change only, rising edge change only or one of both edge change
	 * |        |                  |00 = falling edge
	 * |        |                  |01 =  rising edge
	 * |        |                  |1x = both rising and falling edge
	 * |        |                  |Note: The control is only available when ECAPx is working in DDM mode.
	 * |[4:2]   |CLOCK_DIV_SEL     |ECAPx Timer Clock Divide Selection
	 * |        |                  |ECAPx timer clock has a pre_divider with 8 divided options controlled by DDM_CTRL[4:2]
	 * |        |                  |ECAPx timer clock fixed to 36M
	 * |        |                  |000 = 36M /  1
	 * |        |                  |001 = 36M /  2
	 * |        |                  |010 = 36M /  4
	 * |        |                  |011 = 36M /  8
	 * |        |                  |100 = 36M / 16
	 * |        |                  |101 = 36M / 32
	 * |        |                  |110 = 36M / 64
	 * |        |                  |111 = 36M /128
	 * |        |                  |Note: The control is only available when ECAPx is working in DDM mode.
	 */
	struct {
		uint32_t EDGE_TYPE_SEL : 2;
		uint32_t CLOCK_DIV_SEL : 3;
		uint32_t               :27;
	} BITS;
	uint32_t WORD;
} TS_ECAP_DDM_CTRL;

typedef union {
	/**
	 * @var TS_ECAPx::OVER_CNT , x = 1, 2, 3, 4, 5
	 * Offset: 0x0C, 0x2C, 0x4C, 0x6C, 0x8C, ECAPx DDM mode overflow counter setting register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[15:0]  |OVERFLOW_CNT      |ECAPx overflow counter setting bits
	 * |        |                  |if the up_counting value reaches this OVERFLOW_CNT set value, a OVERFLOW_FLAG will be generated.
	 * |        |                  |if OVERFLOW_INT_EN=1, an interrupt signal is generated and inform to CPU.
	 * |        |                  |Note: The overflow function is only available when ECAPx is working in DDM mode.
	 */
	struct {
		uint32_t OVERFLOW_CNT :16;
		uint32_t              :16;
	} BITS;
	uint32_t WORD;
} TS_ECAP_OVER_CNT;

typedef union {
	/**
	 * @var TS_ECAPx::EDGE_CNT , x = 1, 2, 3, 4, 5
	 * Offset: 0x10, 0x30, 0x50, 0x70, 0x90, ECAPx DDM mode counter hold register, read only.
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[15:0]  |EDGE_DET_CNT      |ECAPx counter hold register
	 * |        |                  |when an active input capture detects a valid edge signal change, the EDGE_DET_CNT value is latched into the corresponding holding register.
	 * |        |                  |Note: This register is only available when ECAPx is working in DDM mode.
	 */
	struct {
		uint32_t EDGE_DET_CNT :16;
		uint32_t              :16;
	} BITS;
	uint32_t WORD;
} TS_ECAP_EDGE_CNT;

typedef union {
	/**
	 * @var TS_ECAPx::QDT_CTRL, x = 1, 5
	 * Offset: 0x14, 0x94, ECAPx QDT mode Q-factor detection Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |PLUSE_END_TYPE    |ECAPx QDT Q-factor detection pulse end type selection
	 * |        |                  |0 = end with high
	 * |        |                  |1 = end with low
	 * |        |                  |Note: The control is only available when ECAPx is working in QDT mode.
	 */
	struct {
		uint32_t PLUSE_END_TYPE : 1;
		uint32_t                :31;
	} _VQM;

	/**
	 * @var TS_ECAPx::QDT_CTRL, x = 2, 3
	 * Offset: 0x34, 0x54, ECAPx QDT mode resonance frequency detection Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[4:1]   |MEAS_TIMES_SET    |ECAPx QDT resonance frequency detection measure times selection
	 * |        |                  |This bits determined how many rising edges will be counted
	 * |        |                  |xxxx = (xxxx + 1) times rising edges will be counted
	 * |        |                  |Note: The control is only available when ECAPx is working in QDT mode.
	 * |[6:5]   |SKIP_TIMES_SET    |ECAPx QDT resonance frequency detection ignore times selection
	 * |        |                  |00 = 1
	 * |        |                  |01 = 2
	 * |        |                  |10 = 3
	 * |        |                  |11 = 4
	 * |        |                  |Note: The control is only available when ECAPx is working in QDT mode.
	 */
	struct {
		uint32_t                : 1;
		uint32_t MEAS_TIMES_SET : 4;
		uint32_t SKIP_TIMES_SET : 2;
		uint32_t                :25;
	} _NQM;

	uint32_t WORD;
} TS_ECAP_QDT_CTRL;

typedef union {
	/**
	 * @var TS_ECAPx::QDT_MEAS, x = 1, 5
	 * Offset: 0x18, 0x98, ECAPx QDT mode Q-factor detection counter hold register, read only
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[15:0]  |DECAY_TIME_CNT    |ECAPx QDT detection decay time hold count
	 * |        |                  |VQM drop from VTH_1.8V to VTH_0.3V decay time counter, from Q measure start to last down cross VTH_0.3V
	 * |        |                  |Note: This value is only available when ECAPx is working in QDT mode.
	 * |[31:16] |WIDTH_LAST_CNT    |ECAPx QDT detection last pulse width hold count
	 * |        |                  |VQM drop cross last pulse_width count
	 * |        |                  |Note: This value is only available when ECAPx is working in QDT mode.
	 */
	struct {
		uint32_t DECAY_TIME_CNT :16;
		uint32_t WIDTH_LAST_CNT :16;
	} _VQM;

	/**
	 * @var TS_ECAPx::QDT_MEAS, x = 2, 3
	 * Offset: 0x38, 0x58, ECAPx QDT mode resonance frequency detection counter hold register, read only
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[15:0]  |RESON_FREQ_CNT    |ECAPx QDT resonance frequency detection hold count
	 * |        |                  |measure [MEAS_TIMES_SET] + 1 times pulse count number
	 * |        |                  |Note: This value is only available when ECAPx is working in QDT mode.
	 */
	struct {
		uint32_t RESON_FREQ_CNT :16;
		uint32_t                :16;
	} _NQM;

	uint32_t WORD;
} TS_ECAP_QDT_MEAS;

typedef struct {
	/*--------------------------------- ECAP1 --- ECAP2 --- ECAP3 --- ECAP4 --- ECAP5 -*/
	__IO TS_ECAP_GEN_CTRL GEN_CTRL; //4000_6000 4000_6020 4000_6040 4000_6060 4000_6080
	__IO TS_ECAP_STS_FLAG STS_FLAG; //4000_6004 4000_6024 4000_6044 4000_6064 4000_6084
	__IO TS_ECAP_DDM_CTRL DDM_CTRL; //4000_6008 4000_6028 4000_6048 4000_6068 4000_6088
	__IO TS_ECAP_OVER_CNT OVER_CNT; //4000_600C 4000_602C 4000_604C 4000_606C 4000_608C
	__I  TS_ECAP_EDGE_CNT EDGE_CNT; //4000_6010 4000_6030 4000_6050 4000_6070 4000_6090
	__IO TS_ECAP_QDT_CTRL QDT_CTRL; //4000_6014 4000_6034 4000_6054 RESE_RVED 4000_6094
	__I  TS_ECAP_QDT_MEAS QDT_MEAS; //4000_6018 4000_6038 4000_6058 RESE_RVED 4000_6098
} TS_ECAP;

#define ECAP_GEN_CTRL_CAP_EN_Pos                    (0)                                                /*!< TS_ECAP::GEN_CTRL: CAP_EN position               */
#define ECAP_GEN_CTRL_CAP_EN_Msk                    (0x1UL << ECAP_GEN_CTRL_CAP_EN_Pos)                /*!< TS_ECAP::GEN_CTRL: CAP_EN Mask                   */
#define ECAP_GEN_CTRL_EDGE_DET_INT_EN_Pos           (1)                                                /*!< TS_ECAP::GEN_CTRL: EDGE_DET_INT_EN position      */
#define ECAP_GEN_CTRL_EDGE_DET_INT_EN_Msk           (0x1UL << ECAP_GEN_CTRL_EDGE_DET_INT_EN_Pos)       /*!< TS_ECAP::GEN_CTRL: EDGE_DET_INT_EN Mask          */
#define ECAP_GEN_CTRL_OVERFLOW_INT_EN_Pos           (2)                                                /*!< TS_ECAP::GEN_CTRL: OVERFLOW_INT_EN position      */
#define ECAP_GEN_CTRL_OVERFLOW_INT_EN_Msk           (0x1UL << ECAP_GEN_CTRL_OVERFLOW_INT_EN_Pos)       /*!< TS_ECAP::GEN_CTRL: OVERFLOW_INT_EN Mask          */
#define ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos         (3)                                                /*!< TS_ECAP::GEN_CTRL: DEGLITEC_TIME_SEL position    */
#define ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Msk         (0x3UL << ECAP_GEN_CTRL_DEGLITEC_TIME_SEL_Pos)     /*!< TS_ECAP::GEN_CTRL: DEGLITEC_TIME_SEL Mask        */
#define ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos         (5)                                                /*!< TS_ECAP::GEN_CTRL: INPUT_CHANNEL_SEL position    */
#define ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Msk         (0x3UL << ECAP_GEN_CTRL_INPUT_CHANNEL_SEL_Pos)     /*!< TS_ECAP::GEN_CTRL: INPUT_CHANNEL_SEL Mask        */
#define ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos         (7)                                                /*!< TS_ECAP::GEN_CTRL: FUNC_WORKING_MODE position    */
#define ECAP_GEN_CTRL_FUNC_WORKING_MODE_Msk         (0x1UL << ECAP_GEN_CTRL_FUNC_WORKING_MODE_Pos)     /*!< TS_ECAP::GEN_CTRL: FUNC_WORKING_MODE Mask        */

#define ECAP_STS_FLAG_EDGE_DET_FLAG_Pos             (0)                                                /*!< TS_ECAP::STS_FLAG: EDGE_DET_FLAG position        */
#define ECAP_STS_FLAG_EDGE_DET_FLAG_Msk             (0x1UL << ECAP_STS_FLAG_EDGE_DET_FLAG_Pos)         /*!< TS_ECAP::STS_FLAG: EDGE_DET_FLAG Mask            */
#define ECAP_STS_FLAG_OVERFLOW_FLAG_Pos             (1)                                                /*!< TS_ECAP::STS_FLAG: OVERFLOW_FLAG position        */
#define ECAP_STS_FLAG_OVERFLOW_FLAG_Msk             (0x1UL << ECAP_STS_FLAG_OVERFLOW_FLAG_Pos)         /*!< TS_ECAP::STS_FLAG: OVERFLOW_FLAG Mask            */
#define ECAP_STS_FLAG_QDT_DONE_FLAG_Pos             (0)                                                /*!< TS_ECAP::STS_FLAG: QDT_DONE_FLAG position        */
#define ECAP_STS_FLAG_QDT_DONE_FLAG_Msk             (0x1UL << ECAP_STS_FLAG_QDT_DONE_FLAG_Pos)         /*!< TS_ECAP::STS_FLAG: QDT_DONE_FLAG Mask            */

#define ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos             (0)                                                /*!< TS_ECAP::DDM_CTRL: EDGE_TYPE_SEL position        */
#define ECAP_DDM_CTRL_EDGE_TYPE_SEL_Msk             (0x3UL << ECAP_DDM_CTRL_EDGE_TYPE_SEL_Pos)         /*!< TS_ECAP::DDM_CTRL: EDGE_TYPE_SEL Mask            */
#define ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos             (2)                                                /*!< TS_ECAP::DDM_CTRL: CLOCK_DIV_SEL position        */
#define ECAP_DDM_CTRL_CLOCK_DIV_SEL_Msk             (0x7UL << ECAP_DDM_CTRL_CLOCK_DIV_SEL_Pos)         /*!< TS_ECAP::DDM_CTRL: CLOCK_DIV_SEL Mask            */

#define ECAP_OVER_CNT_OVERFLOW_CNT_Pos              (0)                                                /*!< TS_ECAP::EDGE_CNT: OVERFLOW_CNT position         */
#define ECAP_OVER_CNT_OVERFLOW_CNT_Msk              (0xFFFFUL << ECAP_OVER_CNT_OVERFLOW_CNT_Pos)       /*!< TS_ECAP::EDGE_CNT: OVERFLOW_CNT Mask             */
#define ECAP_EDGE_CNT_EDGE_DET_CNT_Pos              (0)                                                /*!< TS_ECAP::EDGE_CNT: EDGE_DET_CNT position         */
#define ECAP_EDGE_CNT_EDGE_DET_CNT_Msk              (0xFFFFUL << ECAP_EDGE_CNT_EDGE_DET_CNT_Pos)       /*!< TS_ECAP::EDGE_CNT: EDGE_DET_CNT Mask             */

#define ECAP_QDT_CTRL_VDM_PLUSE_END_TYPE_Pos        (0)                                                /*!< TS_ECAP::QDT_CTRL: VDM_PLUSE_END_TYPE Position   */
#define ECAP_QDT_CTRL_VDM_PLUSE_END_TYPE_Msk        (0x1UL << ECAP_QDT_CTRL_VDM_PLUSE_END_TYPE_Pos)    /*!< TS_ECAP::QDT_CTRL: VDM_PLUSE_END_TYPE Mask       */
#define ECAP_QDT_CTRL_NQM_MEAS_TIMES_SET_Pos        (1)                                                /*!< TS_ECAP::QDT_CTRL: NQM_MEAS_TIMES_SET Position   */
#define ECAP_QDT_CTRL_NQM_MEAS_TIMES_SET_Msk        (0xFUL << ECAP_QDT_CTRL_NQM_MEAS_TIMES_SET_Pos)    /*!< TS_ECAP::QDT_CTRL: NQM_MEAS_TIMES_SET Mask       */
#define ECAP_QDT_CTRL_NQM_SKIP_TIMES_SET_Pos        (5)                                                /*!< TS_ECAP::QDT_CTRL: NQM_SKIP_TIMES_SET Position   */
#define ECAP_QDT_CTRL_NQM_SKIP_TIMES_SET_Msk        (0x3UL << ECAP_QDT_CTRL_NQM_SKIP_TIMES_SET_Pos)    /*!< TS_ECAP::QDT_CTRL: NQM_SKIP_TIMES_SET Mask       */

#define ECAP_QDT_MEAS_VDM_DECAY_TIME_CNT_Pos        (0)                                                /*!< TS_ECAP::QDT_MEAS: VDM_DECAY_TIME_CNT Position   */
#define ECAP_QDT_MEAS_VDM_DECAY_TIME_CNT_Msk        (0xFFFFUL << ECAP_QDT_MEAS_VDM_DECAY_TIME_CNT_Pos) /*!< TS_ECAP::QDT_MEAS: VDM_DECAY_TIME_CNT Mask       */
#define ECAP_QDT_MEAS_VDM_WIDTH_LAST_CNT_Pos        (16)                                               /*!< TS_ECAP::QDT_MEAS: VDM_WIDTH_LAST_CNT Position   */
#define ECAP_QDT_MEAS_VDM_WIDTH_LAST_CNT_Msk        (0xFFFFUL << ECAP_QDT_MEAS_VDM_WIDTH_LAST_CNT_Pos) /*!< TS_ECAP::QDT_MEAS: VDM_WIDTH_LAST_CNT Mask       */
#define ECAP_QDT_MEAS_NQM_RESON_FREQ_CNT_Pos        (0)                                                /*!< TS_ECAP::QDT_MEAS: NQM_RESON_FREQ_CNT Position   */
#define ECAP_QDT_MEAS_NQM_RESON_FREQ_CNT_Msk        (0xFFFFUL << ECAP_QDT_MEAS_NQM_RESON_FREQ_CNT_Pos) /*!< TS_ECAP::QDT_MEAS: NQM_RESON_FREQ_CNT Mask       */

enum ECAP_FUNC_MODE {
	_ECAP_FUNC_MODE_DDM = 0,
	_ECAP_FUNC_MODE_QDT = 1,
};

enum {
	_ECAP_INPUT_CHAN_EXT_PAD_PIN = 0,
	_ECAP_INPUT_CHAN_INR_DDM_OUT = 1,
};

enum {
	_ECAP_DEGILTC_TIME_80us = 0,
	_ECAP_DEGILTC_TIME_60us = 1,
	_ECAP_DEGILTC_TIME_40us = 2,
	_ECAP_DEGILTC_TIME_00us = 3,
};

enum {
	_ECAP_EDGE_TYPE_FALLING = 0,
	_ECAP_EDGE_TYPE_RISEING = 1,
	_ECAP_EDGE_TYPE_RISEING_FALLING = 2,
};

enum {
	_ECAP_DDMCAP_TIMER_CLKDIV_1   = 0,
	_ECAP_DDMCAP_TIMER_CLKDIV_2   = 1,
	_ECAP_DDMCAP_TIMER_CLKDIV_4   = 2,
	_ECAP_DDMCAP_TIMER_CLKDIV_8   = 3,
	_ECAP_DDMCAP_TIMER_CLKDIV_16  = 4,
	_ECAP_DDMCAP_TIMER_CLKDIV_32  = 5,
	_ECAP_DDMCAP_TIMER_CLKDIV_64  = 6,
	_ECAP_DDMCAP_TIMER_CLKDIV_128 = 7,
};

enum {
	_ECAP_QDT_VDM_PLUSE_END_TYPE_HIGH = 0,
	_ECAP_QDT_VDM_PLUSE_END_TYPE_LOW  = 1,
};
/*------------------------------------------ ECAP define -----------------------------------------*/

///*++++++++++++++++++++++++++++++++++++++++++ UFCS define +++++++++++++++++++++++++++++++++++++++++*/
//typedef union {
//	struct {
//		uint32_t src_snk_hdrest : 1;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 3;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 4;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 8;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_GEN_CTRL;
//
//typedef union {
//	struct {
//		uint32_t src_snk_hdrest : 1;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 3;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 4;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 8;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_INT_MASK;
//
//typedef union {
//	struct {
//		uint32_t src_snk_hdrest : 1;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 3;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 4;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 8;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_INT_FLAG;
//
//typedef union {
//	struct {
//		uint32_t src_snk_hdrest : 1;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 3;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 4;
//		uint32_t                : 1;
//		uint32_t                : 1;
//		uint32_t                : 2;
//		uint32_t                : 2;
//		uint32_t                : 8;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_INT_STAT;
//
//typedef union {
//	struct {
//		uint32_t LEN : 8;
//		uint32_t     :24;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_TXD_LENG;
//
//typedef union {
//	struct {
//		uint32_t DAT :32;
//	} BITS;
//	uint32_t WORD;
//} TS_UFCS_TXD_BUFF;
//
//typedef struct {
//	__IO TS_UFCS_GEN_CTRL GEN_CTRL; //4000_7000
//	__IO TS_UFCS_INT_MASK INT_MASK; //4000_7004
//	__IO TS_UFCS_INT_FLAG INT_FLAG; //4000_7008
//	__I  TS_UFCS_INT_STAT INT_STAT; //4000_700C
//	__IO TS_UFCS_TXD_LENG TXD_LENG; //4000_7010
//	__IO TS_UFCS_TXD_BUFF TXD_BUFF; //4000_7014
//} TS_UFCS;
/*------------------------------------------ UFCS define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ BADC define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_BADC::CTRL
	 * Offset: 0x00  basic ADC Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |ADC_EN            |BADC Converter module Enable
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |Before starting the A/D conversion function, this bit should be set to "1".
	 * |        |                  |Clear it to "0" to disable A/D converter analog circuit power consumption.
	 * |[1]     |INT_EN            |BADC interrupt enable control bit
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |A/D conversion end interrupt request is generated if INT_EN bit is set to "1".
	 * |[2]     |CONV_START        |BADC Conversion Start
	 * |        |                  |0 = Conversion stopped and A/D converter entered idle state.
	 * |        |                  |1 = Conversion start.
	 * |        |                  |W1C, CONV_START will be cleared to "0" by hardware automatically.
	 * |[6:5]   |VREF_SEL          |BADC reference voltage selection
	 * |        |                  |0 = VDD
	 * |        |                  |1 = V1P2
	 * |        |                  |2 = V1P1
	 * |        |                  |3 = VDD
	 * |[9:7]   |SAMPLE_CLK_SEL    |BADC sampling clock counter selection, BADC clock is fixed to 4M.
	 * |        |                  |0 =   1 * BADC Clock
	 * |        |                  |1 =   3 * BADC Clock
	 * |        |                  |2 =   5 * BADC Clock
	 * |        |                  |3 =   9 * BADC Clock
	 * |        |                  |4 =  17 * BADC Clock
	 * |        |                  |5 =  33 * BADC Clock
	 * |        |                  |6 =  65 * BADC Clock
	 * |        |                  |7 = 129 * BADC Clock
	 * |[11:10] |SAMPLE_DLY_SEL    |discard first N BADC conversion data when BADC convert start
	 * |        |                  |0 = 0 data
	 * |        |                  |1 = 1 data
	 * |        |                  |2 = 2 data
	 * |        |                  |3 = 3 data
	 * |[13:12] |SAMPLE_AVG_SEL    |average every N BADC conversion data
	 * |        |                  |0 = 1 data
	 * |        |                  |1 = 2 data
	 * |        |                  |2 = 4 data
	 * |        |                  |3 = 8 data
	 * |[17:14] |CHAN_SEL          |BADC channel select
	 * |        |                  |0000 = AVSS_BG
	 * |        |                  |0001 = BADC1
	 * |        |                  |0010 = BADC2
	 * |        |                  |0011 = BADC3
	 * |        |                  |0100 = BADC4
	 * |        |                  |0101 = BADC5
	 * |        |                  |0110 = BADC6
	 * |        |                  |0111 = BADC7
	 * |        |                  |1000 = BADC8
	 * |        |                  |1001 = BADC9
	 * |        |                  |1010 = internal v055 BAND_GAP
	 * |        |                  |1011 = internal V1P1 BAND_GAP
	 * |        |                  |1100 = internal V1P2 BAND_GAP
	 * |        |                  |1101 = internal temperature_L
	 * |        |                  |1110 = internal temperature_H
	 * |        |                  |1111 = ISNS_VOUT_P, differential signal amplified by ISNS_PGA, ISNS_VOUT_N will auto switch to the negative input of BADC.
	 * |        |                  |       PA5 and PB5 need to be set as ISNS_PGA mode, and ISNS_PGA_EN need enabled.
	 * |        |                  |Note:
	 * |        |                  |  When ADC is enabled, if channel changed it is recommended to delay 10us before conversion start, ensure new channel data stability.
	 * |[19]    |ISNS_PGA_EN       |Programmable Gain Amplifier for current sense enable control
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |only take effect when CHAN_SEL=0b1111(ISNS_VOUT_P).
	 * |        |                  |if CH15 ISNS_VOUT_P is selected but ISNS_PGA_EN=0, CH15 cannot sample data.
	 * |[21:20] |ISNS_PGA_GAIN_SEL |ISNS_PGA gain selection
	 * |        |                  |0 = 10
	 * |        |                  |1 = 20
	 * |        |                  |2 = 30
	 * |        |                  |3 = 40
	 * |        |                  |only take effect when CHAN_SEL=0b1111(ISNS_VOUT_P).
	 * |[23:22] |ISNS_PGA_PHAS_SEL |ISNS_PGA phase selection
	 * |        |                  |0 = auto  8KHz from LIRC
	 * |        |                  |1 = auto 16KHz from LIRC
	 * |        |                  |2 = phase1, d2a_isns_clk=0
	 * |        |                  |3 = phase2, d2a_isns_clk=1
	 * |        |                  |Note:
	 * |        |                  |  only take effect when CHAN_SEL=0b1111(ISNS_VOUT_P).
	 */
	struct {
		uint32_t ADC_EN            : 1;
		uint32_t INT_EN            : 1;
		uint32_t CONV_START        : 1;
		uint32_t                   : 2;
		uint32_t VREF_SEL          : 2;
		uint32_t SAMPLE_CLK_SEL    : 3;
		uint32_t SAMPLE_DLY_SEL    : 2;
		uint32_t SAMPLE_AVG_SEL    : 2;
		uint32_t CHAN_SEL          : 4;
		uint32_t                   : 1;
		uint32_t ISNS_PGA_EN       : 1;
		uint32_t ISNS_PGA_GAIN_SEL : 2;
		uint32_t ISNS_PGA_PHAS_SEL : 2;
		uint32_t                   : 8;
	} BITS;
	uint32_t WORD;
} TS_BADC_CTRL;

typedef union {
	/**
	 * @var TS_BADC::FLAG
	 * Offset: 0x04  basic ADC flag Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |DONE_FLAG         |BADC Conversion End Flag, a status flag that indicates the end of A/D conversion.
	 * |        |                  |0 = not end
	 * |        |                  |1 = end
	 * |        |                  |DONE_FLAG is set to "1" When A/D conversion ends.
	 * |        |                  |W1C, this flag can be cleared by writing "1" to itself.
	 * |        |                  |if INT_EN is set, when the DONE_FLAG is set, the BADC interrupt signal is generated and inform to CPU.
	 */
	struct {
		uint32_t DONE_FLAG : 1;
		uint32_t           :31;
	} BITS;
	uint32_t WORD;
} TS_BADC_FLAG;

typedef union {
	/**
	 * @var TS_BADC::DATA
	 * Offset: 0x04  basic ADC Conversion Result register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[11:0]  |CONV_DATA         |BADC Conversion Result
	 * |        |                  |This field contains conversion result of BADC.
	 * |[12]    |NEGA_SIGN         |BADC Conversion Result is negative or positive
	 * |        |                  |0 = positive
	 * |        |                  |1 = negative
	 * |        |                  |if CHAN_SEL=CH15, this sign should be checked. it is not necessary for other channels.
	 */
	struct {
		uint32_t CONV_DATA :12;
		uint32_t NEGA_SIGN : 1;
		uint32_t           :19;
	} BITS;
	uint32_t WORD;
} TS_BADC_DATA;

typedef struct {
	__IO TS_BADC_CTRL CTRL; //4000_8000
	__IO TS_BADC_FLAG FLAG; //4000_8004
	__I  TS_BADC_DATA DATA; //4000_8008
} TS_BADC;

#define BADC_CTRL_ADC_EN_Pos               (0)                                         /*!< TS_BADC::CTRL: ADC_EN Position              */
#define BADC_CTRL_ADC_EN_Msk               (0x1UL << BADC_CTRL_ADC_EN_Pos)             /*!< TS_BADC::CTRL: ADC_EN Mask                  */
#define BADC_CTRL_INT_EN_Pos               (1)                                         /*!< TS_BADC::CTRL: INT_EN Position              */
#define BADC_CTRL_INT_EN_Msk               (0x1UL << BADC_CTRL_INT_EN_Pos)             /*!< TS_BADC::CTRL: INT_EN Mask                  */
#define BADC_CTRL_CONV_START_Pos           (2)                                         /*!< TS_BADC::CTRL: CONV_START Position          */
#define BADC_CTRL_CONV_START_Msk           (0x1UL << BADC_CTRL_CONV_START_Pos)         /*!< TS_BADC::CTRL: CONV_START Mask              */
#define BADC_CTRL_VREF_SEL_Pos             (5)                                         /*!< TS_BADC::CTRL: VREF_SEL Position            */
#define BADC_CTRL_VREF_SEL_Msk             (0x3UL << BADC_CTRL_VREF_SEL_Pos)           /*!< TS_BADC::CTRL: VREF_SEL Mask                */
#define BADC_CTRL_SAMPLE_CLK_SEL_Pos       (7)                                         /*!< TS_BADC::CTRL: SAMPLE_CLK_SEL Position      */
#define BADC_CTRL_SAMPLE_CLK_SEL_Msk       (0x7UL << BADC_CTRL_SAMPLE_CLK_SEL_Pos)     /*!< TS_BADC::CTRL: SAMPLE_CLK_SEL Mask          */
#define BADC_CTRL_SAMPLE_DLY_SEL_Pos       (10)                                        /*!< TS_BADC::CTRL: SAMPLE_DLY_SEL Position      */
#define BADC_CTRL_SAMPLE_DLY_SEL_Msk       (0x3UL << BADC_CTRL_SAMPLE_DLY_SEL_Pos)     /*!< TS_BADC::CTRL: SAMPLE_DLY_SEL Mask          */
#define BADC_CTRL_SAMPLE_AVG_SEL_Pos       (12)                                        /*!< TS_BADC::CTRL: SAMPLE_AVG_SEL Position      */
#define BADC_CTRL_SAMPLE_AVG_SEL_Msk       (0x3UL << BADC_CTRL_SAMPLE_AVG_SEL_Pos)     /*!< TS_BADC::CTRL: SAMPLE_AVG_SEL Mask          */
#define BADC_CTRL_CHAN_SEL_Pos             (14)                                        /*!< TS_BADC::CTRL: CHAN_SEL Position            */
#define BADC_CTRL_CHAN_SEL_Msk             (0xFUL << BADC_CTRL_CHAN_SEL_Pos)           /*!< TS_BADC::CTRL: CHAN_SEL Mask                */
#define BADC_CTRL_ISNS_PAG_EN_Pos          (19)                                        /*!< TS_BADC::CTRL: ISNS_PAG_EN Position         */
#define BADC_CTRL_ISNS_PAG_EN_Msk          (0x1UL << BADC_CTRL_ISNS_PAG_EN_Pos)        /*!< TS_BADC::CTRL: ISNS_PAG_EN Mask             */
#define BADC_CTRL_ISNS_PAG_GAIN_SEL_Pos    (20)                                        /*!< TS_BADC::CTRL: ISNS_PAG_GAIN_SEL Position   */
#define BADC_CTRL_ISNS_PAG_GAIN_SEL_Msk    (0x3UL << BADC_CTRL_ISNS_PAG_GAIN_SEL_Pos)  /*!< TS_BADC::CTRL: ISNS_PAG_GAIN_SEL Mask       */
#define BADC_CTRL_ISNS_PAG_PHAS_SEL_Pos    (22)                                        /*!< TS_BADC::CTRL: ISNS_PAG_PHAS_SEL Position   */
#define BADC_CTRL_ISNS_PAG_PHAS_SEL_Msk    (0x3UL << BADC_CTRL_ISNS_PAG_PHAS_SEL_Pos)  /*!< TS_BADC::CTRL: ISNS_PAG_PHAS_SEL Mask       */

#define BADC_FLAG_DONE_FLAG_Pos            (0)                                         /*!< TS_BADC::FLAG: DONE_FLAG Position           */
#define BADC_FLAG_DONE_FLAG_Msk            (0x1UL << BADC_FLAG_DONE_FLAG_Pos)          /*!< TS_BADC::FLAG: DONE_FLAG Mask               */

#define BADC_DATA_CONV_DATA_Pos            (0)                                         /*!< TS_BADC::DATA: CONV_DATA Position           */
#define BADC_DATA_CONV_DATA_Msk            (0xFFFUL << BADC_DATA_CONV_DATA_Pos)        /*!< TS_BADC::DATA: CONV_DATA Mask               */
#define BADC_DATA_NEGA_SIGN_Pos            (12)                                        /*!< TS_BADC::DATA: NEGA_SIGN Position           */
#define BADC_DATA_NEGA_SIGN_Msk            (0x1UL << BADC_DATA_NEGA_SIGN_Pos)          /*!< TS_BADC::DATA: NEGA_SIGN Mask               */

enum {
	_BADC_VREF_V3P3 = 0,
	_BADC_VREF_V1P2 = 1,
	_BADC_VREF_V1P1 = 2,
};

enum {
	_BADC_SAMPLE_CLK_1   = 0,
	_BADC_SAMPLE_CLK_3   = 1,
	_BADC_SAMPLE_CLK_5   = 2,
	_BADC_SAMPLE_CLK_9   = 3,
	_BADC_SAMPLE_CLK_17  = 4,
	_BADC_SAMPLE_CLK_33  = 5,
	_BADC_SAMPLE_CLK_65  = 6,
	_BADC_SAMPLE_CLK_129 = 7,
};

enum {
	_BADC_SAMPLE_DLY_0 = 0,
	_BADC_SAMPLE_DLY_1 = 1,
	_BADC_SAMPLE_DLY_2 = 2,
	_BADC_SAMPLE_DLY_3 = 3,
};

enum {
	_BADC_SAMPLE_AVG_1 = 0,
	_BADC_SAMPLE_AVG_2 = 1,
	_BADC_SAMPLE_AVG_4 = 2,
	_BADC_SAMPLE_AVG_8 = 3,
};

enum {
	_BADC_ISNS_PGA_GAIN_10 = 0,
	_BADC_ISNS_PGA_GAIN_20 = 1,
	_BADC_ISNS_PGA_GAIN_30 = 2,
	_BADC_ISNS_PGA_GAIN_40 = 3,
};

enum {
	_BADC_ISNS_PGA_PHAS_08K = 0,
	_BADC_ISNS_PGA_GAIN_16K = 1,
};
/*------------------------------------------ BADC define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ EADC define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_EADC::CTRL
	 * Offset: 0x20  Enhanced ADC Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |ADC_EN            |EADC Converter module Enable
	 * |        |                  |Before starting the A/D conversion function, this bit should be set to "1".
	 * |        |                  |Clear it to "0" to disable A/D converter analog circuit power consumption.
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |[1]     |INT_EN            |EADC interrupt enable control bit
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |Note:
	 * |        |                  |if INT_EN bit is set to "1",
	 * |        |                  |   a. if ADC_MODE=0 & VCAP_DETECT_EN=1, A/D conversion end interrupt request is generated.
	 * |        |                  |   b. if ADC_MODE=1, A/D conversion end interrupt request is generated.
	 * |[2]     |CONV_START        |EADC Conversion Start
	 * |        |                  |0 = Conversion stopped and A/D converter entered idle state.
	 * |        |                  |1 = Conversion start.
	 * |[3]     |ADC_MODE          |EADC operation mode
	 * |        |                  |0 = DIG_DDM
	 * |        |                  |1 = ANA_DDM
	 * |        |                  |Note:
	 * |        |                  |for DIG_DDM mode
	 * |        |                  |    a. EADC channel needs to choose channel-2(VCAP), the data is sampled continuously.
	 * |        |                  |    b. if VCAP_DETECT_EN = 0, the data sampled by EADC is not stored in the CONV_DATA buffer registers, the data is directly
	 * |        |                  |        sent to the digital module for demodulation; it will not have DONE_FLAG and will not send an interrupt even INT_EN is enabled.
	 * |        |                  |    c. if VCAP_DETECT_EN = 1, after sampling continuously 20-data and transfer them to the CONV_DATA[0]-CONV_DATA[19] registers,
	 * |        |                  |       then set DONE_FLAG=1; if INT_EN is enabled, send an interrupt to inform MCU. Firmware use these 20-data for calculating Vcap_Icap.
	 * |        |                  |    d. EDAC will output demodulation PHA_OUT to ECAP2, demodulation MAG_OUT to ECAP4.
	 * |        |                  |for ANA_DDM mode
	 * |        |                  |    e. EADC channel needs to choose channel-3(VPGA)
	 * |        |                  |    f. each time sampling 10-data continuously, and transfer them to the CONV_DATA[0]-CONV_DATA[9] registers, then set DONE_FLAG=1;
	 * |        |                  |       if INT_EN is enabled, send an interrupt to inform MCU.
	 * |        |                  |    g. EADC continuous sampling 10-data per 500us, firmware use these data for demodulation.
	 * |[5:4]   |SOURCE_CLK_SEL    |EADC source clock selection
	 * |        |                  |00 = 40*EPWM1 frequency, also need set CLK_DIV[31:25], for Vcap_Icap detect sampling not include MPP_360 and APPLE_128K
	 * |        |                  |01 = 40*360K, if ADC_MODE = DIG_DDM for MPP 360K, should choose this clock source
	 * |        |                  |10 = 40*128K, if ADC_MODE = DIG_DDM & VCAP_DETECT_EN for iPhone 7.5W Vcap_Icap sampling , should choose this clock source
	 * |        |                  |11 = 40*100K, if ADC_MODE = ANA_DDM, output 10-ADC data per 500us for firmware DDM, should choose this clock source
	 * |[7:6]   |SAMPLE_DLY_SEL    |discard first N EADC conversion data when EADC convert start
	 * |        |                  |0 = 0 data
	 * |        |                  |1 = 1 data
	 * |        |                  |2 = 2 data
	 * |        |                  |3 = 3 data
	 * |        |                  |Note:
	 * |        |                  |  a. if ADC_MODE=DIG_DDM & VCAP_DETECT_EN=0, this configuration not effective for digital demodulation
	 * |        |                  |  b. if ADC_MODE=DIG_DDM & VCAP_DETECT_EN=1, this configuration take effect for VCAP_ICAP Vcap_Icap detect sampling
	 * |        |                  |  c. if ADC_MODE=ANA_DDM, this configuration take effect for sampling 10-data each time, firmware use these data for demodulation.
	 * |[8]     |VCAP_DETECT_EN    |EADC VCAP detection enable
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |NOTE:
	 * |        |                  |  a. this bit only effective when ADC_MODE=DIG_DDM, after sampling continuously 20-data and transfer them to the CONV_DATA[0]-CONV_DATA[19] registers,
	 * |        |                  |     then set DONE_FLAG=1; if INT_EN is enabled, send an interrupt to inform MCU. Firmware can use these 20-data for calculating Vcap_Icap.
	 * |        |                  |  b. after sampling done, EDCA will return to DIG_DDM(digital demodulation) mode automatically.
	 * |        |                  |  c. this bit cleared by hardware automatically, write rising pulse will start Vcap_Icap detect sampling.
	 * |[10:9]  |VREF_SEL          |EADC reference voltage selection
	 * |        |                  |0 = VDD
	 * |        |                  |1 = V1P2, for test and debug
	 * |        |                  |2 = V1P1, for test and debug
	 * |        |                  |3 = VDD
	 * |[14]    |VPGA_PGA_EN       |EADC ANA_DDM mode PGA enable
	 * |        |                  |0 = disable
	 * |        |                  |1 = enable
	 * |        |                  |Note: only effective when CHAN_SEL=3(VPGA)
	 * |[17:15] |VPGA_PGA_GAIN_SEL |EADC ANA_DDM mode PGA gain selection
	 * |        |                  |0 =  1
	 * |        |                  |1 =  2
	 * |        |                  |2 =  5
	 * |        |                  |3 = 10
	 * |        |                  |4 = 20
	 * |        |                  |others = 1
	 * |        |                  |Note: only take effect when CHAN_SEL=3(VPGA) & VPGA_PGA_EN=1
	 * |[18]    |VPGA_PGA_LPF_DIS  |EADC ANA_DDM mode low pass filter disable
	 * |        |                  |used to select whether to disable the 1st-order filter of PGA input signal
	 * |        |                  |0 = enable
	 * |        |                  |1 = disable
	 * |        |                  |Note: only take effect when CHAN_SEL=3(VPGA) & VPGA_PGA_EN=1
	 * |[20:19] |VCAP_LPF_RC_SEL   |EADC DIG_DDM mode low pass filter RC time selection
	 * |        |                  |0 = 200ns
	 * |        |                  |1 = 150ns
	 * |        |                  |2 = 100ns
	 * |        |                  |3 =  50ns
	 * |        |                  |Note: Only take effect when CHAN_SEL=2(VCAP)
	 * |[23:21] |CHAN_SEL          |EADC channel select
	 * |        |                  |000 = AVSS_BG
	 * |        |                  |001 = TEST
	 * |        |                  |010 = VCAP, for digital demodulation, need choose ADC_MODE=DIG_DDM
	 * |        |                  |011 = VPGA, for  analog demodulation, need choose ADC_MODE=ANA_DDM
	 * |        |                  |100 = V1P2
	 * |        |                  |101 = V055
	 * |        |                  |others reserved
	 * |        |                  |Note:
	 * |        |                  |a. if ADC_MODE=DIG_DDM & CHAN_SEL=VCAP,
	 * |        |                  |   a.1 EDAC will output demodulation PHA_OUT to ECAP2, demodulation MAG_OUT to ECAP4
	 * |        |                  |   a.2 ECAP2 & ECAP4 input channel should select internal EADC digital demodulation output
	 * |        |                  |   a.3 Under this configuration, if set VCAP_DETECT_EN=1, EADC will sampling 20-data for calculating Vcap_Icap
	 * |        |                  |b. if ADC_MODE=ANA_DDM & CHAN_SEL=VPGA,
	 * |        |                  |   b.1 PD5/ECAP2 GPD_MODE is not important, suggest configuring it as ECAP2 mode
	 * |        |                  |   b.2 EADC continuous sampling 10-data per 500us, firmware will use for demodulation
	 * |        |                  |c. When ADC is enabled, if channel changed it is recommended to delay 10us before conversion start, ensure new channel data stability.
	 * |[31:25] |CLK_DIV           |EADC clock divider selection
	 * |        |                  |[ 0,  18] = 16M
	 * |        |                  |[19,  71] = 144M / CLK_DIV, need 40*EPWM1 frequency
	 * |        |                  |[72, 127] =  4M
	 * |        |                  |Note: Only take effect when SOURCE_CLK_SEL=0, for Vcap_Icap detect sampling.
	 */
	struct {
		uint32_t ADC_EN            : 1;
		uint32_t INT_EN            : 1;
		uint32_t CONV_START        : 1;
		uint32_t ADC_MODE          : 1;
		uint32_t SOURCE_CLK_SEL    : 2;
		uint32_t SAMPLE_DLY_SEL    : 2;
		uint32_t VCAP_DETECT_EN    : 1;
		uint32_t VREF_SEL          : 2;
		uint32_t                   : 3;
		uint32_t VPGA_PGA_EN       : 1;
		uint32_t VPGA_PGA_GAIN_SEL : 3;
		uint32_t VPGA_PGA_LPF_DIS  : 1;
		uint32_t VCAP_LPF_RC_SEL   : 2;
		uint32_t CHAN_SEL          : 3;
		uint32_t                   : 1;
		uint32_t CLK_DIV           : 7;
	} BITS;
	uint32_t WORD;
} TS_EADC_CTRL;

typedef union {
	/**
	 * @var TS_EADC::FLAG
	 * Offset: 0x24  Enhanced ADC flag Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[0]     |DONE_FLAG         |EADC Conversion End Flag, a status flag that indicates the end of A/D conversion.
	 * |        |                  |0 = not end
	 * |        |                  |1 = end
	 * |        |                  |DONE_FLAG is set to "1" When A/D conversion ends.
	 * |        |                  |W1C, this flag can be cleared by writing "1" to itself.
	 * |        |                  |if INT_EN is set, when the DONE_FLAG is set, the EADC interrupt signal is generated and inform to CPU.
	 * |        |                  |Note:
	 * |        |                  |for DIG_DDM mode
	 * |        |                  |    a. if VCAP_DETECT_EN = 0, the data sampled by EADC is not stored in the CONV_DATA buffer registers, the data is directly
	 * |        |                  |        sent to the digital module for demodulation; it will not have DONE_FLAG and will not send an interrupt even INT_EN is enabled.
	 * |        |                  |    b. if VCAP_DETECT_EN = 1, after sampling continuously 20-data and transfer them to the CONV_DATA[0]-CONV_DATA[19] registers,
	 * |        |                  |       then set DONE_FLAG=1; if INT_EN is enabled, send an interrupt to inform MCU.
	 * |        |                  |for ANA_DDM mode
	 * |        |                  |    c. each time sampling 10-data continuously, and transfer them to the CONV_DATA[0]-CONV_DATA[9] registers, then set DONE_FLAG=1;
	 * |        |                  |       if INT_EN is enabled, send an interrupt to inform MCU.
	 */
	struct {
		uint32_t DONE_FLAG : 1;
		uint32_t           :31;
	} BITS;
	uint32_t WORD;
} TS_EADC_FLAG;

typedef union {
	/**
	 * @var TS_EADC::DATA
	 * Offset: 0x28-0x74  Enhanced ADC Conversion Result register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field             |Descriptions
	 * | :----: | :----:           | :---- |
	 * |[11:0]  |CONV_DATA         |EADC Conversion Result
	 * |        |                  |This field contains conversion result of EADC.
	 * |[12]    |NEGA_SIGN         |EADC Conversion Result is negative or positive
	 * |        |                  |0 = positive
	 * |        |                  |1 = negative
	 * |        |                  |Note:
	 * |        |                  |for DIG_DDM mode
	 * |        |                  |    a. if VCAP_DETECT_EN = 0, the data sampled by EADC is not stored in the CONV_DATA buffer registers, the data is directly
	 * |        |                  |        sent to the digital module for demodulation.
	 * |        |                  |    b. if VCAP_DETECT_EN = 1, after sampling continuously 20-data and transfer them to the CONV_DATA[0]-CONV_DATA[19] registers,
	 * |        |                  |       then set DONE_FLAG=1; if INT_EN is enabled, send an interrupt to inform MCU.
	 * |        |                  |for ANA_DDM mode
	 * |        |                  |    c. each time sampling 10-data continuously, and transfer them to the CONV_DATA[0]-CONV_DATA[9] registers, then set DONE_FLAG=1;
	 * |        |                  |       if INT_EN is enabled, send an interrupt to inform MCU.
	 * |        |                  |    d. CONV_DATA[10]-CONV_DATA[19] are shadow buffer for this mode.
	 */
	struct {
		uint32_t CONV_DATA :12;
		uint32_t NEGA_SIGN : 1;
		uint32_t           :19;
	} BITS;
	uint32_t WORD;
} TS_EADC_DATA;

typedef struct {
	__IO TS_EADC_CTRL CTRL; //4000_8020
	__IO TS_EADC_FLAG FLAG; //4000_8024
	__I  TS_EADC_DATA DATA[20]; //4000_8028 - 4000_8074
} TS_EADC;

#define EADC_CTRL_ADC_EN_Pos               (0)                                         /*!< TS_EADC::CTRL: ADC_EN Position              */
#define EADC_CTRL_ADC_EN_Msk               (0x1UL << EADC_CTRL_ADC_EN_Pos)             /*!< TS_EADC::CTRL: ADC_EN Mask                  */
#define EADC_CTRL_INT_EN_Pos               (1)                                         /*!< TS_EADC::CTRL: INT_EN Position              */
#define EADC_CTRL_INT_EN_Msk               (0x1UL << EADC_CTRL_INT_EN_Pos)             /*!< TS_EADC::CTRL: INT_EN Mask                  */
#define EADC_CTRL_CONV_START_Pos           (2)                                         /*!< TS_EADC::CTRL: CONV_START Position          */
#define EADC_CTRL_CONV_START_Msk           (0x1UL << EADC_CTRL_CONV_START_Pos)         /*!< TS_EADC::CTRL: CONV_START Mask              */
#define EADC_CTRL_ADC_MODE_Pos             (3)                                         /*!< TS_EADC::CTRL: ADC_MODE Position            */
#define EADC_CTRL_ADC_MODE_Msk             (0x1UL << EADC_CTRL_ADC_MODE_Pos)           /*!< TS_EADC::CTRL: ADC_MODE Mask                */
#define EADC_CTRL_SOURCE_CLK_SEL_Pos       (4)                                         /*!< TS_EADC::CTRL: SOURCE_CLK_SEL Position      */
#define EADC_CTRL_SOURCE_CLK_SEL_Msk       (0x3UL << EADC_CTRL_SOURCE_CLK_SEL_Pos)     /*!< TS_EADC::CTRL: SOURCE_CLK_SEL Mask          */
#define EADC_CTRL_SAMPLE_DLY_SEL_Pos       (6)                                         /*!< TS_EADC::CTRL: SAMPLE_DLY_SEL Position      */
#define EADC_CTRL_SAMPLE_DLY_SEL_Msk       (0x3UL << EADC_CTRL_SAMPLE_DLY_SEL_Pos)     /*!< TS_EADC::CTRL: SAMPLE_DLY_SEL Mask          */
#define EADC_CTRL_VCAP_DETECT_EN_Pos       (8)                                         /*!< TS_EADC::CTRL: VCAP_DETECT_EN Position      */
#define EADC_CTRL_VCAP_DETECT_EN_Msk       (0x1UL << EADC_CTRL_VCAP_DETECT_EN_Pos)     /*!< TS_EADC::CTRL: VCAP_DETECT_EN Mask          */
#define EADC_CTRL_VREF_SEL_Pos             (9)                                         /*!< TS_EADC::CTRL: VREF_SEL Position            */
#define EADC_CTRL_VREF_SEL_Msk             (0x3UL << EADC_CTRL_VREF_SEL_Pos)           /*!< TS_EADC::CTRL: VREF_SEL Mask                */
#define EADC_CTRL_VPGA_PAG_EN_Pos          (14)                                        /*!< TS_EADC::CTRL: VPGA_PAG_EN Position         */
#define EADC_CTRL_VPGA_PAG_EN_Msk          (0x1UL << EADC_CTRL_VPGA_PAG_EN_Pos)        /*!< TS_EADC::CTRL: VPGA_PAG_EN Mask             */
#define EADC_CTRL_VPGA_PAG_GAIN_SEL_Pos    (15)                                        /*!< TS_EADC::CTRL: VPGA_PAG_GAIN_SEL Position   */
#define EADC_CTRL_VPGA_PAG_GAIN_SEL_Msk    (0x7UL << EADC_CTRL_VPGA_PAG_GAIN_SEL_Pos)  /*!< TS_EADC::CTRL: VPGA_PAG_GAIN_SEL Mask       */
#define EADC_CTRL_VPGA_PAG_LPF_DIS_Pos     (18)                                        /*!< TS_EADC::CTRL: VPGA_PAG_LPF_DIS Position    */
#define EADC_CTRL_VPGA_PAG_LPF_DIS_Msk     (0x1UL << EADC_CTRL_VPGA_PAG_LPF_DIS_Pos)   /*!< TS_EADC::CTRL: VPGA_PAG_LPF_DIS Mask        */
#define EADC_CTRL_VCAP_LPF_RC_SEL_Pos      (19)                                        /*!< TS_EADC::CTRL: VCAP_LPF_RC_SEL Position     */
#define EADC_CTRL_VCAP_LPF_RC_SEL_Msk      (0x3UL << EADC_CTRL_DIG_DDM_LPF_RC_SEL_Pos) /*!< TS_EADC::CTRL: VCAP_LPF_RC_SEL Mask         */
#define EADC_CTRL_CHAN_SEL_Pos             (21)                                        /*!< TS_EADC::CTRL: CHAN_SEL Position            */
#define EADC_CTRL_CHAN_SEL_Msk             (0x7UL << EADC_CTRL_CHAN_SEL_Pos)           /*!< TS_EADC::CTRL: CHAN_SEL Mask                */
#define EADC_CTRL_CLK_DIV_Pos              (25)                                        /*!< TS_EADC::CTRL: CLK_DIV Position             */
#define EADC_CTRL_CLK_DIV_Msk              (0x3FUL << EADC_CTRL_CLK_DIV_Pos)           /*!< TS_EADC::CTRL: CLK_DIV Mask                 */

#define EADC_FLAG_DONE_FLAG_Pos            (0)                                         /*!< TS_EADC::FLAG: DONE_FLAG Position           */
#define EADC_FLAG_DONE_FLAG_Msk            (0x1UL << EADC_FLAG_DONE_FLAG_Pos)          /*!< TS_EADC::FLAG: DONE_FLAG Mask               */

#define EADC_DATA_CONV_DATA_Pos            (0)                                         /*!< TS_EADC::DATA: CONV_DATA Position           */
#define EADC_DATA_CONV_DATA_Msk            (0xFFFUL << EADC_DATA_CONV_DATA_Pos)        /*!< TS_EADC::DATA: CONV_DATA Mask               */
#define EADC_DATA_NEGA_SIGN_Pos            (12)                                        /*!< TS_EADC::DATA: NEGA_SIGN Position           */
#define EADC_DATA_NEGA_SIGN_Msk            (0x1UL << EADC_DATA_NEGA_SIGN_Pos)          /*!< TS_EADC::DATA: NEGA_SIGN Mask               */

enum eadc_mode_t {
	_EADC_MODE_DIG_DDM = 0,
	_EADC_MODE_ANA_DDM = 1,
};

enum eadc_chan_t {
	_EADC_CH_INR_AVSS = 0,
	_EADC_CH_INR_TEST = 1,
	_EADC_CH_INR_VCAP = 2,
	_EADC_CH_INR_VPGA = 3,
	_EADC_CH_INR_V1P2 = 4,
	_EADC_CH_INR_V055 = 5,
};

enum {
	_EADC_SOURCE_CLK_40xEPWM = 0,
	_EADC_SOURCE_CLK_40x360K = 1,
	_EADC_SOURCE_CLK_40x128K = 2,
	_EADC_SOURCE_CLK_40x100K = 3,
};

enum {
	_EADC_SAMPLE_DLY_0 = 0,
	_EADC_SAMPLE_DLY_1 = 1,
	_EADC_SAMPLE_DLY_2 = 2,
	_EADC_SAMPLE_DLY_3 = 3,
};

enum {
	_EADC_VREF_V3P3 = 0,
	_EADC_VREF_V1P2 = 1,
	_EADC_VREF_V1P1 = 2,
};

enum {
	_EADC_ANA_DDM_PGA_GAIN_1  = 0,
	_EADC_ANA_DDM_PGA_GAIN_2  = 1,
	_EADC_ANA_DDM_PGA_GAIN_5  = 2,
	_EADC_ANA_DDM_PGA_GAIN_10 = 3,
	_EADC_ANA_DDM_PGA_GAIN_20 = 4,
};

enum {
	_EADC_DIG_DDM_LPF_RC_200ns = 0,
	_EADC_DIG_DDM_LPF_RC_150ns = 1,
	_EADC_DIG_DDM_LPF_RC_100ns = 2,
	_EADC_DIG_DDM_LPF_RC_050ns = 3,
};
/*------------------------------------------ EADC define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ I2CS define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	struct {
		uint32_t GR00 : 8;
		uint32_t GR01 : 8;
		uint32_t GR02 : 8;
		uint32_t GR03 : 8;
	} BITS;
	uint32_t WORD;
} TS_I2CS_MCU_RO_DATA;

typedef union {
	struct {
		uint32_t GR10 : 8;
		uint32_t GR11 : 8;
		uint32_t GR12 : 8;
		uint32_t GR13 : 8;
	} BITS;
	uint32_t WORD;
} TS_I2CS_MCU_RW_DATA;

typedef union {
	struct {
		uint32_t GR03_FLAG : 1;
		uint32_t SRAM_FLAG : 1;
		uint32_t           :30;
	} BITS;
	uint32_t WORD;
} TS_I2CS_STS_FLAG;

typedef union {
	struct {
		uint32_t GR03_INT_EN : 1;
		uint32_t SRAM_INT_EN : 1;
		uint32_t             :30;
	} BITS;
	uint32_t WORD;
} TS_I2CS_INT_CTRL;

typedef struct {
	__I  TS_I2CS_MCU_RO_DATA RO_DATA; //4000_9000
	__IO TS_I2CS_MCU_RW_DATA RW_DATA; //4000_9004
	__IO TS_I2CS_STS_FLAG       FLAG; //4000_9008
	__IO TS_I2CS_INT_CTRL       CTRL; //4000_900C
} TS_I2CS;
/*------------------------------------------ I2CS define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ I2CM define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	struct {
		uint32_t I2CM_FUNC_EN : 1;
		uint32_t I2CM_INT_EN  : 1;
		uint32_t              : 1;
		uint32_t PROTOCOL_CMD : 5;
		uint32_t              : 8;
		uint32_t BRGEN_CLKDIV :15; //Baud Rate Generator Clock Divider
		uint32_t              : 1;
	} BITS;
	uint32_t WORD;
} TS_I2CM_GEN_CTRL;

typedef union {
	struct {
		uint32_t DATA : 8;
		uint32_t      :24;
	} BITS;
	uint32_t WORD;
} TS_I2CM_TXD_DATA;

typedef union {
	struct {
		uint32_t DATA : 8;
		uint32_t      :24;
	} BITS;
	uint32_t WORD;
} TS_I2CM_RXD_DATA;

typedef union {
	struct {
		uint32_t          : 1;
		uint32_t TIP      : 1;
		uint32_t          : 1;
		uint32_t          : 1;
		uint32_t ARB_LOST : 1;
		uint32_t BUS_BUSY : 1;
		uint32_t ACK_DATA : 1;
		uint32_t          : 1;
		uint32_t          :24;
	} BITS;
	uint32_t WORD;
} TS_I2CM_STS_FLAG;

typedef union {
	struct {
		uint32_t CMD_DONE_FLAG : 1;
		uint32_t ARB_LOST_FLAG : 1;
		uint32_t               :30;
	} BITS;
	uint32_t WORD;
} TS_I2CM_INT_FLAG;

#define I2CM_PROTOCOL_CMD_NULL    (0b00000)
#define I2CM_PROTOCOL_CMD_NACK    (0b00001)
#define I2CM_PROTOCOL_CMD_SEND    (0b00010)
#define I2CM_PROTOCOL_CMD_READ    (0b00100)
#define I2CM_PROTOCOL_CMD_STOP    (0b01000)
#define I2CM_PROTOCOL_CMD_STAR    (0b10000)

typedef struct {
	__IO TS_I2CM_GEN_CTRL GEN_CTRL; //4000_9800
	__IO TS_I2CM_TXD_DATA TXD_DATA; //4000_9804
	__I  TS_I2CM_RXD_DATA RXD_DATA; //4000_9808
	__I  TS_I2CM_STS_FLAG STS_FLAG; //4000_980C
	__IO TS_I2CM_INT_FLAG INT_FLAG; //4000_9810
} TS_I2CM;
/*------------------------------------------ I2CM define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ EPWM define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_EPWM::PWM_CTRL
	 * Offset: 0x00  EPWMx basic Control Register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |EPWM_EN       |EPWMx enable control bit
	 * |        |              |0 = disable CH0 and CH1 output
	 * |        |              |1 =  enable CH0 and CH1 output
	 * |        |              |EPWM1-> CH0 = PWM1, CH1 = PWM2
	 * |        |              |EPWM2-> CH0 = PWM5, CH1 = PWM6
	 * |[3:2]   |CH0_OUT       |EPWMx CH0 output mode selection
	 * |        |              |0 = CH0 output normal PWM
	 * |        |              |1 = CH0 output force to high
	 * |        |              |2 = CH0 output force to low
	 * |        |              |3 = CH0 output normal PWM
	 * |        |              |EPWM1-> CH0 = PWM1
	 * |        |              |EPWM2-> CH0 = PWM5
	 * |[5:4]   |CH1_OUT       |EPWMx CH1 output mode selection
	 * |        |              |0 = CH1 output normal PWM
	 * |        |              |1 = CH1 output force to high
	 * |        |              |2 = CH1 output force to low
	 * |        |              |3 = CH1 output normal PWM
	 * |        |              |EPWM1-> CH1 = PWM2
	 * |        |              |EPWM2-> CH1 = PWM6
	 */
	struct {
		uint32_t EPWM_EN : 1;
		uint32_t         : 1;
		uint32_t CH0_OUT : 2;
		uint32_t CH1_OUT : 2;
		uint32_t         :26;
	} BITS;
	uint32_t WORD;
} TS_EPWM_PWM_CTRL;

typedef union {
	/**
	 * @var TS_EPWM::PWM_DUTY
	 * Offset: 0x04  EPWMx duty cycle control register, x = 1, 2
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[11:0]  |CH0_DUTY      |EPWMx CH0 duty cycle
	 * |        |              |duty ratio = CH0_DUTY / (PWM_PERD + 1)
	 * |        |              |EPWM1-> CH0 = PWM1
	 * |        |              |EPWM2-> CH0 = PWM5
	 * |[27:16] |CH1_DUTY      |EPWMx CH1 duty cycle
	 * |        |              |duty ratio = CH1_DUTY / (PWM_PERD + 1)
	 * |        |              |EPWM1-> CH1 = PWM2
	 * |        |              |EPWM2-> CH1 = PWM6
	 */
	struct {
		uint32_t CH0_DUTY :12;
		uint32_t          : 4;
		uint32_t CH1_DUTY :12;
		uint32_t          : 4;
	} BITS;
	uint32_t WORD;
} TS_EPWM_PWM_DUTY;

typedef union {
	/**
	 * @var TS_EPWM::PWM_PERD, x = 1
	 * Offset: 0x08  EPWMx period cycle and phase shift control register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[11:0]  |PWM_PERD      |EPWMx period cycles
	 * |        |              |EPWM frequency = 144M / (PWM_PERD + 1)
	 * |        |              |The frequency of CH0 and CH1 is same
	 * |        |              |EPWM1-> PWM1 and PWM2 period
	 * |        |              |EPWM2-> PWM5 and PWM6 period
	 * |[27:16] |PWM_PHAS      |EPWMx phase shift between CH0 and CH1
	 * |        |              |shift_phase = 2 * pi * PWM_PHAS / (PWM_PERD + 1)
	 * |        |              |EPWM1-> CH0 = PWM1, CH1 = PWM2
	 * |        |              |EPWM2-> CH0 = PWM5, CH1 = PWM6
	 */
	struct {
		uint32_t PWM_PERD :12;
		uint32_t          : 4;
		uint32_t PWM_PHAS :12;
		uint32_t          : 4;
	} BITS;
	uint32_t WORD;
} TS_EPWM_PWM_PERD;

typedef union {
	/**
	 * @var TS_EPWMx::AFJ_CTRL, x = 1, 2
	 * Offset: 0x10  EPWMx Auto_Frequency_Dither Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |AFD_EN        |EPWMx Auto_Frequency_Dither function enable control bit
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |[7:4]   |STEP_CYC      |PWM frequency pre_sale change/step
	 * |        |              |The period change counts during two consecutive PWM cycles.
	 * |[15:8]  |STEP_CNT      |PWM cycles in 1/4(quarter) AFD period
	 * |        |              |The number of complete PWM cycle contained in a quarter cycle of AFD.
	 * |        |              |number of PWM cycles that frequency change from center frequency to max frequency, it is the 1/4 of AFD period.
	 */
	struct {
		uint32_t AFD_EN   : 1;
		uint32_t          : 3;
		uint32_t STEP_CYC : 4;
		uint32_t STEP_CNT : 8;
		uint32_t          :16;
	} BITS;
	uint32_t WORD;
} TS_EPWM_AFD_CTRL;

typedef union {
	/**
	 * @var TS_EPWMx::FSK_CTRL, x = 1, 2
	 * Offset: 0x20  EPWMx FSK basic Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |FSK_EN        |EPWMx FSK enable control bit
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |        |              |Note: rising pulse trigger
	 * |[1]     |INT_EN        |EPWMx FSK interrupt enable control bit
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |[4]     |POLAR_SEL     |FSK polarity select bit
	 * |        |              |0 = positive
	 * |        |              |1 = negative
	 * |[7:5]   |BIT_CYCLE     |FSK cycles of one bit select bits
	 * |        |              |0 = 512 PWM cycles for 1 bit
	 * |        |              |1 = 256 PWM cycles for 1 bit
	 * |        |              |2 = 128 PWM cycles for 1 bit
	 * |        |              |3 =  64 PWM cycles for 1 bit
	 * |        |              |4 =  32 PWM cycles for 1 bit
	 * |        |              |5 =  16 PWM cycles for 1 bit
	 * |        |              |6 =   8 PWM cycles for 1 bit
	 * |        |              |7 = reserved
	 * |[15:8]  |DEPTH_SEL     |The change counts of PWM running cycles when sending data for different FSK depths.
	 * |        |              |Note: Do not use bit field operations here.
	 */
	struct {
		uint32_t FSK_EN    : 1;
		uint32_t INT_EN    : 1;
		uint32_t           : 2;
		uint32_t POLAR_SEL : 1;
		uint32_t BIT_CYCLE : 3;
		uint32_t DEPTH_SEL : 7;
		uint32_t           :17;
	} BITS;
	uint32_t WORD;
} TS_EPWM_FSK_CTRL;

typedef union {
	/**
	 * @var TS_EPWMx::FSK_CTRL, x = 1, 2
	 * Offset: 0x24  EPWMx FSK delay Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[15:0]  |PWMCYC_DELAY  |PWM cycles delay
	 * |        |              |When FSK is enabled, wait for how many cycles of PWM before starting to send FSK data.
	 * |        |              |This is designed for FSK response_delay(T_RESP, 3ms~10ms), and this value is related to the current PWM operating frequency.
	 */
	struct {
		uint32_t PWMCYC_DELAY :16;
		uint32_t              :16;
	} BITS;
	uint32_t WORD;
} TS_EPWM_FSK_DLY_;

typedef union {
	/**
	 * @var TS_EPWMx::FSK_CTRL, x = 1, 2
	 * Offset: 0x28  EPWMx FSK send buffer Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[25:0]  |BMC_BIT_DATA  |Bi-phase Mark Coding bit sets for FSK data
	 * |        |              |LSB first out
	 * |[31:26] |BMC_BIT_SIZE  |Bi-phase Mark Coding bit size for FSK data
	 * |        |              |This BMC_BIT_SIZE represents the number of significant digits in BMC_BIT_DATA[25:0]
	 */
	struct {
		uint32_t BMC_BIT_DATA :26;
		uint32_t BMC_BIT_SIZE : 6;
	} BITS;
	uint32_t WORD;
} TS_EPWM_FSK_BUFF;

typedef union {
	/**
	 * @var TS_EPWMx::FSK_CTRL, x = 1, 2
	 * Offset: 0x2C  EPWMx FSK flag Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field               |Descriptions
	 * | :----: | :----:             | :---- |
	 * |[0]     |BMC_BUFF_EMPTY_FLAG |BMC_BUFF_EMPTY_FLAG
	 * |        |                    |0 = no effect
	 * |        |                    |1 = The digital logic has read the data from the FSK_BUFF.
	 * |        |                    |Note: 1-The digital logic has read the data from the buffer, and the MCU can write the next data to FSK_BUFF.
	 * |[1]     |LAST_BUFF_DONE_FLAG |LAST_BUFF_DONE_FLAG
	 * |        |                    |0 = no effect
	 * |        |                    |1 = The digital logic has sent the last FSK_BUFF done.
	 * |        |                    |Note: 1-The digital logic has sent the last FSK_BUFF done, and the MCU can set FSK_EN 0 to disable FSK function.
	 * |        |                    |Note: write 0 to FSK_BUFF, and digital logic known that is the last FSK_BUFF.
	 */
	struct {
		uint32_t BMC_BUFF_EMPTY_FLAG : 1;
		uint32_t LAST_BUFF_DONE_FLAG : 1;
		uint32_t                     :30;
	} BITS;
	uint32_t WORD;
} TS_EPWM_FSK_FLAG;

typedef struct {
	/*--------------------------------- EPMW1 --- EPWM2 -*/
	__IO TS_EPWM_PWM_CTRL PWM_CTRL; //4000_A000 4000_B000
	__IO TS_EPWM_PWM_DUTY PWM_DUTY; //4000_A004 4000_B004
	__IO TS_EPWM_PWM_PERD PWM_PERD; //4000_A008 4000_B008
	__IO uint32_t         RESERVE0; //4000_A00C 4000_B00C
	__IO TS_EPWM_AFD_CTRL AFD_CTRL; //4000_A010 4000_B010
	__IO uint32_t         RESERVE1; //4000_A014 4000_B014
	__IO uint32_t         RESERVE2; //4000_A018 4000_B018
	__IO uint32_t         RESERVE3; //4000_A01C 4000_B01C
	__IO TS_EPWM_FSK_CTRL FSK_CTRL; //4000_A020 4000_B020 /*! this control please don't use bit-field write operation */
	__IO TS_EPWM_FSK_DLY_ FSK_DLY_; //4000_A024 4000_B024
	__IO TS_EPWM_FSK_BUFF FSK_BUFF; //4000_A028 4000_B028
	__IO TS_EPWM_FSK_FLAG FSK_FLAG; //4000_A02C 4000_B02C
} TS_EPWM;

#define EPWM_PWM_CTRL_EPWM_EN_Pos                (0)                                               /*!< TS_EPWM::PWM_CTRL: EPWM_EN Position              */
#define EPWM_PWM_CTRL_EPWM_EN_Msk                (0x1UL << EPWM_PWM_CTRL_EPWM_EN_Pos)              /*!< TS_EPWM::PWM_CTRL: EPWM_EN Mask                  */
#define EPWM_PWM_CTRL_CH0_OUT_Pos                (2)                                               /*!< TS_EPWM::PWM_CTRL: CH0_OUT Position              */
#define EPWM_PWM_CTRL_CH0_OUT_Msk                (0x3UL << EPWM_PWM_CTRL_CH0_OUT_Pos)              /*!< TS_EPWM::PWM_CTRL: CH0_OUT Mask                  */
#define EPWM_PWM_CTRL_CH1_OUT_Pos                (4)                                               /*!< TS_EPWM::PWM_CTRL: CH1_OUT Position              */
#define EPWM_PWM_CTRL_CH1_OUT_Msk                (0x3UL << EPWM_PWM_CTRL_CH1_OUT_Pos)              /*!< TS_EPWM::PWM_CTRL: CH1_OUT Mask                  */
#define EPWM_PWM_DUTY_CH0_DUTY_Pos               (0)                                               /*!< TS_EPWM::PWM_DUTY: CH0_DUTY Position             */
#define EPWM_PWM_DUTY_CH0_DUTY_Msk               (0xFFFUL << EPWM_PWM_DUTY_CH0_DUTY_Pos)           /*!< TS_EPWM::PWM_DUTY: CH0_DUTY Mask                 */
#define EPWM_PWM_DUTY_CH1_DUTY_Pos               (16)                                              /*!< TS_EPWM::PWM_DUTY: CH1_DUTY Position             */
#define EPWM_PWM_DUTY_CH1_DUTY_Msk               (0xFFFUL << EPWM_PWM_DUTY_CH1_DUTY_Pos)           /*!< TS_EPWM::PWM_DUTY: CH1_DUTY Mask                 */
#define EPWM_PWM_PERD_PWM_PERD_Pos               (0)                                               /*!< TS_EPWM::PWM_PERD: PWM_PERD Position             */
#define EPWM_PWM_PERD_PWM_PERD_Msk               (0xFFFUL << EPWM_PWM_PERD_PWM_PERD_Pos)           /*!< TS_EPWM::PWM_PERD: PWM_PERD Mask                 */
#define EPWM_PWM_PERD_PWM_PHAS_Pos               (16)                                              /*!< TS_EPWM::PWM_PERD: PWM_PHAS Position             */
#define EPWM_PWM_PERD_PWM_PHAS_Msk               (0xFFFUL << EPWM_PWM_PERD_PWM_PHAS_Pos)           /*!< TS_EPWM::PWM_PERD: PWM_PHAS Mask                 */
#define EPWM_AFD_CTRL_AFD_EN_Pos                 (0)                                               /*!< TS_EPWM::AFD_CTRL: AFD_EN Position               */
#define EPWM_AFD_CTRL_AFD_EN_Msk                 (0x1UL << EPWM_AFD_CTRL_AFD_EN_Pos)               /*!< TS_EPWM::AFD_CTRL: AFD_EN Mask                   */
#define EPWM_AFD_CTRL_STEP_CYC_Pos               (4)                                               /*!< TS_EPWM::AFD_CTRL: STEP_CYC Position             */
#define EPWM_AFD_CTRL_STEP_CYC_Msk               (0xFUL << EPWM_AFD_CTRL_STEP_CYC_Pos)             /*!< TS_EPWM::AFD_CTRL: STEP_CYC Mask                 */
#define EPWM_AFD_CTRL_STEP_CNT_Pos               (8)                                               /*!< TS_EPWM::AFD_CTRL: STEP_CNT Position             */
#define EPWM_AFD_CTRL_STEP_CNT_Msk               (0xFFUL << EPWM_AFD_CTRL_STEP_CNT_Pos)            /*!< TS_EPWM::AFD_CTRL: STEP_CNT Mask                 */
#define EPWM_FSK_CTRL_FSK_EN_Pos                 (0)                                               /*!< TS_EPWM::FSK_CTRL: FSK_EN Position               */
#define EPWM_FSK_CTRL_FSK_EN_Msk                 (0x1UL << EPWM_FSK_CTRL_FSK_EN_Pos)               /*!< TS_EPWM::FSK_CTRL: FSK_EN Mask                   */
#define EPWM_FSK_CTRL_INT_EN_Pos                 (1)                                               /*!< TS_EPWM::FSK_CTRL: INT_EN Position               */
#define EPWM_FSK_CTRL_INT_EN_Msk                 (0x1UL << EPWM_FSK_CTRL_INT_EN_Pos)               /*!< TS_EPWM::FSK_CTRL: INT_EN Mask                   */
#define EPWM_FSK_CTRL_POLAR_SEL_Pos              (4)                                               /*!< TS_EPWM::FSK_CTRL: POLAR_SEL Position            */
#define EPWM_FSK_CTRL_POLAR_SEL_Msk              (0x1UL << EPWM_FSK_CTRL_POLAR_SEL_Pos)            /*!< TS_EPWM::FSK_CTRL: POLAR_SEL Mask                */
#define EPWM_FSK_CTRL_BIT_CYCLE_Pos              (5)                                               /*!< TS_EPWM::FSK_CTRL: BIT_CYCLE Position            */
#define EPWM_FSK_CTRL_BIT_CYCLE_Msk              (0x7UL << EPWM_FSK_CTRL_BIT_CYCLE_Pos)            /*!< TS_EPWM::FSK_CTRL: BIT_CYCLE Mask                */
#define EPWM_FSK_CTRL_DEPTH_SEL_Pos              (8)                                               /*!< TS_EPWM::FSK_CTRL: DEPTH_SEL Position            */
#define EPWM_FSK_CTRL_DEPTH_SEL_Msk              (0xFFUL << EPWM_FSK_CTRL_DEPTH_SEL_Pos)           /*!< TS_EPWM::FSK_CTRL: DEPTH_SEL Mask                */
#define EPWM_FSK_DLY_PWMCYC_DELAY_Pos            (0)                                               /*!< TS_EPWM::FSK_DLY_: PWMCYC_DELAY Position         */
#define EPWM_FSK_DLY_PWMCYC_DELAY_Msk            (0xFFFFUL << EPWM_FSK_DLY_PWMCYC_DELAY_Pos)       /*!< TS_EPWM::FSK_DLY_: PWMCYC_DELAY Mask             */
#define EPWM_FSK_BUFF_BMC_BIT_DATA_Pos           (0)                                               /*!< TS_EPWM::FSK_BUFF: BMC_BIT_DATA Position         */
#define EPWM_FSK_BUFF_BMC_BIT_DATA_Msk           (0x3FFFFFFUL << EPWM_FSK_BUFF_BMC_BIT_DATA_Pos)   /*!< TS_EPWM::FSK_BUFF: BMC_BIT_DATA Mask             */
#define EPWM_FSK_BUFF_BMC_BIT_SIZE_Pos           (26)                                              /*!< TS_EPWM::FSK_BUFF: BMC_BIT_SIZE Position         */
#define EPWM_FSK_BUFF_BMC_BIT_SIZE_Msk           (0x3FUL << EPWM_FSK_BUFF_BMC_BIT_SIZE_Pos)        /*!< TS_EPWM::FSK_BUFF: BMC_BIT_SIZE Mask             */
#define EPWM_FSK_FLAG_BMC_BUFF_EMPTY_FLAG_Pos    (0)                                               /*!< TS_EPWM::FSK_FLAG: BMC_BUFF_EMPTY_FLAG Position  */
#define EPWM_FSK_FLAG_BMC_BUFF_EMPTY_FLAG_Msk    (0x1UL << EPWM_FSK_FLAG_BMC_BUFF_EMPTY_FLAG_Pos)  /*!< TS_EPWM::FSK_FLAG: BMC_BUFF_EMPTY_FLAG Mask      */
#define EPWM_FSK_FLAG_LAST_BUFF_DONE_FLAG_Pos    (1)                                               /*!< TS_EPWM::FSK_FLAG: LAST_BUFF_DONE_FLAG Position  */
#define EPWM_FSK_FLAG_LAST_BUFF_DONE_FLAG_Msk    (0x1UL << EPWM_FSK_FLAG_LAST_BUFF_DONE_FLAG_Pos)  /*!< TS_EPWM::FSK_FLAG: LAST_BUFF_DONE_FLAG Mask      */

enum {
	_EPWM_OUT_NORMAL_PWM  = 0,
	_EPWM_OUT_FIXED2_HIGH = 1,
	_EPWM_OUT_FIXED2_LOW  = 2,
};
/*------------------------------------------ EPWM define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ DPDM define +++++++++++++++++++++++++++++++++++++++++*/
//typedef union {
//	struct {
//		uint32_t                         : 3;
//		uint32_t DCD_TIMEOUT_INT_STAT    : 1;
//		uint32_t                         : 3;
//		uint32_t BC1P2_DET_DONE_INT_STAT : 1;
//		uint32_t Unstandard_TYPE         : 3;
//		uint32_t                         : 1;
//		uint32_t BC1P2_TYPE              : 3;
//		uint32_t                         : 1;
//		uint32_t                         :16;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_BC1P2_STAT;

//typedef union {
//	struct {
//		uint32_t                         : 3;
//		uint32_t DCD_TIMEOUT_INT_FLAG    : 1;
//		uint32_t                         : 3;
//		uint32_t BC1P2_DET_DONE_INT_FLAG : 1;
//		uint32_t                         : 3;
//		uint32_t                         : 1;
//		uint32_t                         : 3;
//		uint32_t                         : 1;
//		uint32_t                         :16;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_BC1P2_FLAG;

//typedef union {
//	struct {
//		uint32_t              : 1;
//		uint32_t BC1P2_EN     : 1;
//		uint32_t              : 1;
//		uint32_t NSD_SKIP     : 1;
//		uint32_t DPDM_PRI_THR : 1;
//		uint32_t DPDM_EN      : 1;
//		uint32_t              : 1;
//		uint32_t DCD_Timeout  : 1;
//		uint32_t              : 3;
//		uint32_t DCD_TIMEOUT_INT_MASK : 1;
//		uint32_t              : 3;
//		uint32_t BC1P2_DET_DONE_INT_MASK : 1;
//		uint32_t                         :16;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_BC1P2_CTRL;

//typedef union {
//	struct {
//		uint32_t DM_COT_PULSE_DONE_INT_STAT : 1;
//		uint32_t DP_COT_PULSE_DONE_INT_STAT : 1;
//		uint32_t DPDM_2PULSE_DONE_INT_STAT  : 1;
//		uint32_t DPDM_3PULSE_DONE_INT_STAT  : 1;
//		uint32_t DM_16PULSE_DONE_INT_STAT   : 1;
//		uint32_t DP_16PULSE_DONE_INT_STAT   : 1;
//		uint32_t HVDCP_DET_FAIL_INT_STAT    : 1;
//		uint32_t HVDCP_DET_OK_INT_STAT      : 1;
//		uint32_t                            :24;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_QC3P0_STAT;

//typedef union {
//	struct {
//		uint32_t DM_COT_PULSE_DONE_INT_FLAG : 1;
//		uint32_t DP_COT_PULSE_DONE_INT_FLAG : 1;
//		uint32_t DPDM_2PULSE_DONE_INT_FLAG  : 1;
//		uint32_t DPDM_3PULSE_DONE_INT_FLAG  : 1;
//		uint32_t DM_16PULSE_DONE_INT_FLAG   : 1;
//		uint32_t DP_16PULSE_DONE_INT_FLAG   : 1;
//		uint32_t HVDCP_DET_FAIL_INT_FLAG    : 1;
//		uint32_t HVDCP_DET_OK_INT_FLAG      : 1;
//		uint32_t                            :24;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_QC3P0_FLAG;
//
//typedef union {
//	struct {
//		uint32_t QC_FALLING_DET_TIME : 1;
//		uint32_t QC3P5_PULSE         : 3;
//		uint32_t QC_MODE             : 2;
//		uint32_t QC_COMMAND          : 1;
//		uint32_t QC_EN               : 1;
//
//		uint32_t DM_COT_PULSE_DONE_INT_MASK : 1;
//		uint32_t DP_COT_PULSE_DONE_INT_MASK : 1;
//		uint32_t DPDM_2PULSE_DONE_INT_MASK  : 1;
//		uint32_t DPDM_3PULSE_DONE_INT_MASK  : 1;
//		uint32_t DM_16PULSE_DONE_INT_MASK   : 1;
//		uint32_t DP_16PULSE_DONE_INT_MASK   : 1;
//		uint32_t HVDCP_DET_FAIL_INT_MASK    : 1;
//		uint32_t HVDCP_DET_OK_INT_MASK      : 1;
//
//		uint32_t                            :16;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_QC3P0_CTRL;
//
//typedef union {
//	struct {
//		uint32_t DPDM_COT_PULSE      : 7;
//		uint32_t PULSE_INACTIVE_TIME : 1;
//		uint32_t                     :24;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_COT_PLUSE;
//
//typedef union {
//	struct {
//		uint32_t DPDM_Manual_EN  : 1;
//		uint32_t                 : 2;
//		uint32_t DP_SRC_10UA     : 1;
//		uint32_t DP_SINK_EN      : 1;
//		uint32_t DM_SINK_EN      : 1;
//		uint32_t DM_20K_PD_EN    : 1;
//		uint32_t DPDM_500K_PD_EN : 1;
//
//		uint32_t            : 1;
//		uint32_t DP_BUFF_EN : 1;
//		uint32_t DM_BUFF_EN : 1;
//		uint32_t DP_BUF     : 2;
//		uint32_t DM_BUF     : 2;
//		uint32_t            : 1;
//
//		uint32_t VDP_RD   : 3;
//		uint32_t VDM_RD   : 3;
//		uint32_t DP_RD_EN : 1;
//		uint32_t DM_RD_EN : 1;
//
//		uint32_t          : 8;
//	} BITS;
//	uint32_t WORD;
//} TS_DPDM_MANUAL_REG;

//typedef struct {
//	__I  TS_DPDM_BC1P2_STAT BC1P2_STAT; //4000_C000
//	__IO TS_DPDM_BC1P2_FLAG BC1P2_FLAG; //4000_C004
//	__IO TS_DPDM_BC1P2_CTRL BC1P2_CTRL; //4000_C008
//	__I  TS_DPDM_QC3P0_STAT QC3P0_STAT; //4000_C00C
//	__IO TS_DPDM_QC3P0_FLAG QC3P0_FLAG; //4000_C010
//	__IO TS_DPDM_QC3P0_CTRL QC3P0_CTRL; //4000_C014
//	__IO TS_DPDM_COT_PLUSE  COT_PLUSE;  //4000_C018
//	__IO TS_DPDM_MANUAL_REG MANUAL_REG; //4000_C01C
//} TS_DPDM;



typedef union {
	struct {
		uint32_t EN_Source_DPDM_Protocol    : 1;
		uint32_t Soft_Reset_Source_Protocol : 1;
		uint32_t EN_Auto_DCP                : 1;
		uint32_t EN_HVDCP_DET               : 1;
		uint32_t EN_QC_SRC_DET              : 2;
		uint32_t EN_SCP_SRC_DET             : 1;
		uint32_t EN_AFC_SRC_DET             : 1;

		uint32_t EN_UFCS_SRC_DET            : 1;
		uint32_t EN_DPDM_900k_PD            : 1;
		uint32_t DPDM_MUX_Port_NUM          : 3;
		uint32_t DPDM_Port1_CTRL            : 1;
		uint32_t DPDM_Port2_CTRL            : 1;
		uint32_t DPDM_Port3_CTRL            : 1;

		uint32_t                            :16;
	} BITS;
	uint32_t WORD;
} TS_DPDM_SRC_CTRL;

typedef struct {
	__IO TS_DPDM_SRC_CTRL PROTOCOL_CTRL; //4000_C080
} TS_DPDM_SRC;
/*------------------------------------------ DPDM define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ DDM define ++++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	struct {
		uint32_t TUNE_T_I         : 2;
		uint32_t TUNE_COEFF_I     : 2;
		uint32_t TUNE_T_Q         : 2;
		uint32_t TUNE_COEFF_Q     : 2;
		uint32_t ORTHO_PHASE_OPT  : 4;
		uint32_t LPF_IN_SCLAE_OPT : 2;
		uint32_t                  : 2;
		uint32_t ADC_OFFSET       :12;
		uint32_t ADC_SCALE_OPT    : 2;
		uint32_t                  : 2;
	} BITS;
	uint32_t WORD;
} TS_DDM_GEN_CTRL;

typedef union {
	struct {
		uint32_t MAG_MA_T      : 2;
		uint32_t MAG_MA_COEFF  : 2;
		uint32_t               : 4;
		uint32_t MAG_TUNE_HYST : 3;
		uint32_t               :21;
	} BITS;
	uint32_t WORD;
} TS_DDM_MAG_CTRL;

typedef union {
	struct {
		uint32_t PHASE_MA_T        : 2;
		uint32_t PHASE_MA_COEFF    : 2;
		uint32_t                   : 4;
		uint32_t PHASE_TUNE_HYST   : 3;
		uint32_t                   : 5;
		uint32_t ALGO_TRI_HYST     : 2;
		uint32_t ALGO_TRI_STEP_OPT : 2;
		uint32_t                   : 4;
		uint32_t PHASE_OUT_SEL     : 1;
		uint32_t                   : 7;
	} BITS;
	uint32_t WORD;
} TS_DDM_PHA_CTRL;

typedef union {
	struct {
		uint32_t MAG_AVG_SLOW :12;
		uint32_t              : 4;
		uint32_t MAG_AVG_FAST :12;
		uint32_t              : 4;
	} BITS;
	uint32_t WORD;
} TS_DDM_MAG_DATA;

typedef union {
	struct {
		uint32_t PHASE_AVG_SLOW :12;
		uint32_t                : 4;
		uint32_t PHASE_AVG_FAST :12;
		uint32_t                : 4;
	} BITS;
	uint32_t WORD;
} TS_DDM_PHA_DATA;

typedef union {
	struct {
		uint32_t DATA_I_SEL     : 2;
		uint32_t                : 2;
		uint32_t DATA_Q_SEL     : 2;
		uint32_t                : 2;
		uint32_t DEBUG_DATA_SEL : 3;
		uint32_t                : 1;
		uint32_t DEBUG_BIT_SEL  : 2;
		uint32_t                :18;
	} BITS;
	uint32_t WORD;
} TS_DDM_I_Q_CTRL;

typedef union {
	struct {
		uint32_t CHAN_I_DATA :12;
		uint32_t             : 4;
		uint32_t CHAN_Q_DATA :12;
		uint32_t             : 4;
	} BITS;
	uint32_t WORD;
} TS_DDM_I_Q_DATA;

typedef struct {
	__IO TS_DDM_GEN_CTRL GEN_CTRL; //4000_D000
	__IO TS_DDM_MAG_CTRL MAG_CTRL; //4000_D004
	__IO TS_DDM_PHA_CTRL PHA_CTRL; //4000_D008
	__I  TS_DDM_MAG_DATA MAG_DATA; //4000_D00C
	__I  TS_DDM_PHA_DATA PHA_DATA; //4000_D010
	__IO TS_DDM_I_Q_CTRL I_Q_CTRL; //4000_D014
	__I  TS_DDM_I_Q_DATA I_Q_DATA; //4000_D018
} TS_DDM;
/*------------------------------------------ DDM define ------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ BPWM define +++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var BPWMx::CTRL0, x = 3, 4, 7, 8
	 * Offset: 0x00, 0x10, 0x20, 0x30  BPWMx general Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |EN            |BPWMx enable control bit
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |[1]     |MODE          |BPWMx operation mode selection
	 * |        |              |0 = continuous mode
	 * |        |              |1 = one-shot mode (EN bit rising pulse trigger)
	 * |[2]     |CLK_SRC       |BPWMx clock source selection
	 * |        |              |0 = HCLK
	 * |        |              |1 = LIRC
	 */
	struct {
		uint32_t EN      : 1;
		uint32_t MODE    : 1;
		uint32_t CLK_SRC : 1;
		uint32_t         :29;
	} BITS;
	uint32_t WORD;
} TS_BPWM_GEN_CTRL;

typedef union {
	/**
	 * @var BPWMx::CTRL0, x = 3, 4, 7, 8
	 * Offset: 0x00, 0x10, 0x20, 0x30  BPWMx period and duty cycle Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[14:0]  |PERD          |BPWMx period counter Register
	 * |        |              |PERD data determines the PWM period.
	 * |        |              |PWM frequency = BPWM_CLK/(PERD+1)
	 * |[30:16] |DUTY          |BPWMx duty cycle counter register
	 * |        |              |BPWMx duty ratio = (DUTY+1)/(PERD+1)
	 * |        |              |DUTY >= PERD: PWM output is always high
	 * |        |              |DUTY  < PERD: PWM low width = (PERD-DUTY+1) unit; PWM high width = (DUTY) unit.
	 * |        |              |DUTY = 0: PWM low width = (PERD+1) unit; PWM high width = 0 unit.
	 */
	struct {
		uint32_t PERD :15;
		uint32_t      : 1;
		uint32_t DUTY :15;
		uint32_t      : 1;
	} BITS;
	uint32_t WORD;
} TS_BPWM_PWM_CTRL;

typedef struct {
	/*--------------------------------- BPWM3 --- BPWM4 --- BPWM7 --- BPWM8 -*/
	__IO TS_BPWM_GEN_CTRL GEN_CTRL; //4000_E000 4000_E010 4000_E020 4000_E030
	__IO TS_BPWM_PWM_CTRL PWM_CTRL; //4000_E004 4000_E014 4000_E024 4000_E034
} TS_BPWM;

#define BPWM_GEN_CTRL_EN_Pos                      (0)                                   /*!< TS_BPWM::GEN_CTRL: EN Position       */
#define BPWM_GEN_CTRL_EN_Msk                      (0x1UL << BPWM_GEN_CTRL_EN_Pos)       /*!< TS_BPWM::GEN_CTRL: EN Mask           */
#define BPWM_GEN_CTRL_MODE_Pos                    (1)                                   /*!< TS_BPWM::GEN_CTRL: MODE Position     */
#define BPWM_GEN_CTRL_MODE_Msk                    (0x1UL << BPWM_GEN_CTRL_MODE_Pos)     /*!< TS_BPWM::GEN_CTRL: MODE Mask         */
#define BPWM_GEN_CTRL_CLK_SRC_Pos                 (2)                                   /*!< TS_BPWM::GEN_CTRL: CLK_SRC Position  */
#define BPWM_GEN_CTRL_CLK_SRC_Msk                 (0x1UL << BPWM_GEN_CTRL_CLK_SRC_Pos)  /*!< TS_BPWM::GEN_CTRL: CLK_SRC Mask      */

#define BPWM_PWM_CTRL_PERD_Pos                    (0)                                   /*!< TS_BPWM::PWM_CTRL: PERD Position     */
#define BPWM_PWM_CTRL_PERD_Msk                    (0x7FFFUL << BPWM_PWM_CTRL_PERD_Pos)  /*!< TS_BPWM::PWM_CTRL: PERD Mask         */
#define BPWM_PWM_CTRL_DUTY_Pos                    (16)                                  /*!< TS_BPWM::PWM_CTRL: DUTY Position     */
#define BPWM_PWM_CTRL_DUTY_Msk                    (0x7FFFUL << BPWM_PWM_CTRL_DUTY_Pos)  /*!< TS_BPWM::PWM_CTRL: DUTY Mask         */

enum {
	_BPWM_MODE_PERIODIC = 0,
	_BPWM_MODE_ONE_SHOT = 1,
};

enum {
	_BPWM_CLK_SRC_HCLK = 0,
	_BPWM_CLK_SRC_LIRC = 1,
};
/*------------------------------------------ BPWM define -----------------------------------------*/

/*++++++++++++++++++++++++++++++++++++++++++ FMC define ++++++++++++++++++++++++++++++++++++++++++*/
typedef union {
	/**
	 * @var TS_FMC::DMA_ROM_CTRL
	 * Offset: 0x00 FMC DMA ROM Control Register, Used for external I2C flash operation, this register does not need to be operated during firmware development
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[15:0]  |DMA_ROM_ADDR  |flash start address when external I2C operation
	 * |        |              |for external I2C DMA write and erase operation
	 * |        |              |  if NVR = 1
	 * |        |              |     sector0  : 0x0000-0x007F
	 * |        |              |     sector1  : 0x0080-0x00FF
	 * |        |              |       ...
	 * |        |              |     sector15 : 0x0780-0x07FF
	 * |        |              |  if NVR = 0
	 * |        |              |     sector0  : 0x0000-0x007F
	 * |        |              |     sector1  : 0x0080-0x00FF
	 * |        |              |       ...
	 * |        |              |     sector255: 0x7E00-0x7FFF
	 * |        |              |  Note:
	 * |        |              |    sectorN: 0x0080 * N ~ (0x0080 * N + 0x007F)
	 * |        |              |for external I2C read operation
	 * |        |              |  if NVR = 1
	 * |        |              |     DMA_I2C_ADDR = 0
	 * |        |              |        sector0  : 0x8000-0x81FF
	 * |        |              |        sector1  : 0x8200-0x83FF
	 * |        |              |          ...
	 * |        |              |        sector15 : 0x9E00-0x9FFF
	 * |        |              |  if NVR = 0
	 * |        |              |     DMA_I2C_ADDR = 0
	 * |        |              |        sector0  : 0x8000-0x81FF
	 * |        |              |        sector1  : 0x8200-0x83FF
	 * |        |              |          ...
	 * |        |              |        sector63 : 0xFE00-0xFFFF
	 * |        |              |     DMA_I2C_ADDR = 1
	 * |        |              |          ...
	 * |        |              |     DMA_I2C_ADDR = 2
	 * |        |              |          ...
	 * |        |              |     DMA_I2C_ADDR = 3
	 * |        |              |        sector192: 0x8000-0x81FF
	 * |        |              |        sector193: 0x8200-0x83FF
	 * |        |              |          ...
	 * |        |              |        sector255: 0xFE00-0xFFFF
	 * |        |              |  Note:
	 * |        |              |    sectorN: (0x8000 + 0x0200 * (N - addr_h * 64)) ~ ((0x8000 + 0x0200 * (N - addr_h * 64)) + 0x01FF)
	 * |[17:16] |DMA_I2C_ADDR  |I2C direct serial read flash, as 2 highest bits address
	 * |        |              |for external I2C read operation
	 * |        |              |  if NVR = 1
	 * |        |              |     DMA_I2C_ADDR = 0
	 * |        |              |  if NVR = 0
	 * |        |              |     DMA_I2C_ADDR = (sectorN) / 64,  0<= N <= 255
	 * |[24]    |NVR           |Non-Volatile Register
	 * |        |              |0 = 0x00000000 - 0x00001FFF
	 * |        |              |1 = 0x00002000 - 0x00021FFF
	 * |[25]    |RCELL         |select reference enable
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |        |              |Note: please keep disable
	 * |[24]    |TERS          |select ter
	 * |        |              |0 = disable
	 * |        |              |1 = enable
	 * |        |              |Note: please keep disable
	 */
	struct {
		uint32_t DMA_ROM_ADDR :16;
		uint32_t DMA_I2C_ADDR : 2;
		uint32_t              : 6;
		uint32_t NVR          : 1;
		uint32_t RCELL        : 1;
		uint32_t TERS         : 1;
		uint32_t              : 5;
	} BITS;
	uint32_t WORD;
} TS_DMA_ROM_CTRL;

typedef union {
	/**
	 * @var TS_FMC::DMA_ROM_CTRL
	 * Offset: 0x04 FMC DMA RAM Control Register, Used for external I2C flash operation, this register does not need to be operated during firmware development
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[10:0]  |DMA_RAM_ADDR  |SRAM start address for external I2C DAM write operation
	 * |        |              |DMA_RAM_ADDR = (SRAM address - 0x20000000 + 0x2000)
	 * |[23:16] |DMA_RAM_SIZE  |I2C DMA auto write flash word numbers, max 128 WORD, 512 bytes
	 * |        |              |for external I2C DMA write operation
	 * |        |              |Note: size unit WORD
	 */
	struct {
		uint32_t DMA_RAM_ADDR :11;
		uint32_t              : 5;
		uint32_t DMA_RAM_SIZE : 8; //WORD count, max to 128
		uint32_t              : 8;
	} BITS;
	uint32_t WORD;
} TS_DMA_RAM_CTRL;

typedef union {
	/**
	 * @var TS_FMC::DMA_INT_CTRL
	 * Offset: 0x08 FMC DMA write interrupt Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[2]     |DMA_DONE_INTE |firmware DMA write flash finished interrupt enable
	 * |        |              |0: disable
	 * |        |              |1: enable
	 * |        |              |Note: firmware use DMA write flash is not recommended.
	 */
	struct {
		uint32_t               : 1;
		uint32_t               : 1;
		uint32_t DMA_DONE_INTE : 1;
		uint32_t               :29;
	} BITS;
	uint32_t WORD;
} TS_DMA_INT_CTRL;

typedef union {
	/**
	 * @var TS_FMC::FMC_CMD_CTRL
	 * Offset: 0x0C FMC command Control Register
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[3:0]   |CMD           |choose flash memory control command
	 * |        |              |0000: reserved
	 * |        |              |0001: warm_up reload
	 * |        |              |0010: DMA write enable
	 * |        |              |0011: reserved
	 * |        |              |0100: AHB write enable
	 * |        |              |0101: AHB page erase enable
	 * |        |              |0110: reserved
	 * |        |              |0111: I2C read enable
	 * |        |              |1000: I2C write enable
	 * |        |              |1001: I2C page erase enable
	 * |        |              |1010: I2C chip erase enable
	 * |        |              |1011: reserved
	 * |        |              |1100: reserved
	 * |        |              |1101: reserved
	 * |        |              |1110: reserved
	 * |        |              |1111: FMC disable
	 */
	struct {
		uint32_t CMD : 4;
		uint32_t     :28;
	} BITS;
	uint32_t WORD;
} TS_FMC_CMD_CTRL;

typedef union {
	/**
	 * @var TS_FMC::FMC_FSM_STAT
	 * Offset: 0x10 FMC Finite State Machine current state register, read only
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |IDLE_FLAG     |FMC idle state flag
	 * |        |              |0: FMC is busy
	 * |        |              |1: FMC is idle, can setting next command
	 * |[5:1]   |RST_STATE     |warm_up FSM current state
	 * |        |              |Not recommended to use
	 * |[12:8]  |AHB_STATE     |AHB FSM current state
	 * |        |              |Not recommended to use
	 * |[20:16] |I2C_STATE     |I2C FSM current state
	 * |        |              |Not recommended to use
	 * |[27:24] |DMA_STATE     |DMA FSM current state
	 * |        |              |Not recommended to use
	 */
	struct {
		uint32_t IDLE_FLAG : 1;
		uint32_t RST_STATE : 5;
		uint32_t           : 2;
		uint32_t AHB_STATE : 5;
		uint32_t           : 3;
		uint32_t I2C_STATE : 5;
		uint32_t           : 3;
		uint32_t DMA_STATE : 4;
		uint32_t           : 4;
	} BITS;
	uint32_t WORD;
} TS_FMC_FSM_STAT;

typedef union {
	/**
	 * @var TS_FMC::FLASH_R_LOCK, SPROM_2_LOCK, SPROM_1_LOCK, SPROM_0_LOCK
	 * Offset: 0x14, 0x18, 0x1C, 0x20,  flash memory lock status register, read only
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |LOCK          |flash memory lock status
	 * |        |              |for FLASH_R_LOCK
	 * |        |              |    0: not lock
	 * |        |              |    1: lock, external I2C cannot read all flash
	 * |        |              |for SPROM_x_LOCK, x=0, 1, 2
	 * |        |              |    0: not lock
	 * |        |              |    1: lock, external I2C cannot access SPROMx unless the correct KEY is provided
	 */
	struct {
		uint32_t LOCK : 1;
		uint32_t      :31;
	} BITS;
	uint32_t WORD;
} TS_ROM_SCY_STAT;

typedef union {
	/**
	 * @var TS_FMC::DMA_INT_FLAG
	 * Offset: 0x24 FMC DMA write finished status flag, read only
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field         |Descriptions
	 * | :----: | :----:       | :---- |
	 * |[0]     |DMA_DONE_FLAG |firmware DMA write flash finished flag
	 * |        |              |0: disable
	 * |        |              |1: enable
	 * |        |              |Note: firmware use DMA write flash is not recommended.
	 */
	struct {
		uint32_t DMA_DONE_FLAG : 1;
		uint32_t               :31;
	} BITS;
	uint32_t WORD;
} TS_DMA_INT_FLAG;

typedef struct {
	__IO TS_DMA_ROM_CTRL DMA_ROM_CTRL; //4000_F000
	__IO TS_DMA_RAM_CTRL DMA_RAM_CTRL; //4000_F004
	__IO TS_DMA_INT_CTRL DMA_INT_CTRL; //4000_F008
	__IO TS_FMC_CMD_CTRL FMC_CMD_CTRL; //4000_F00C
	__I  TS_FMC_FSM_STAT FMC_FSM_STAT; //4000_F010
	__I  TS_ROM_SCY_STAT FLASH_R_LOCK; //4000_F014
	__I  TS_ROM_SCY_STAT SPROM_2_LOCK; //4000_F018
	__I  TS_ROM_SCY_STAT SPROM_1_LOCK; //4000_F01C
	__I  TS_ROM_SCY_STAT SPROM_0_LOCK; //4000_F020
	__IO TS_DMA_INT_FLAG DMA_INT_FLAG; //4000_F024
} TS_FMC;

#define FMC_FMC_CMD_CTRL_CMD_Pos                  (0)                                   /*!< TS_FMC::FMC_CMD_CTRL: CMD Position  */
#define FMC_FMC_CMD_CTRL_CMD_Msk                  (0xFUL << FMC_FMC_CMD_CTRL_CMD_Pos)   /*!< TS_FMC::FMC_CMD_CTRL: CMD Mask      */

#define FMC_FLASH_R_LOCK_LOCK_Pos                 (0)                                    /*!< TS_FMC::FLASH_R_LOCK: LOCK Position */
#define FMC_FLASH_R_LOCK_LOCK_Msk                 (0x1UL << FMC_FLASH_R_LOCK_LOCK_Pos)   /*!< TS_FMC::FLASH_R_LOCK: LOCK Mask     */
#define FMC_SPROM_2_LOCK_LOCK_Pos                 (0)                                    /*!< TS_FMC::SPROM_2_LOCK: LOCK Position */
#define FMC_SPROM_2_LOCK_LOCK_Msk                 (0x1UL << FMC_SPROM_2_LOCK_LOCK_Pos)   /*!< TS_FMC::SPROM_2_LOCK: LOCK Mask     */
#define FMC_SPROM_1_LOCK_LOCK_Pos                 (0)                                    /*!< TS_FMC::SPROM_1_LOCK: LOCK Position */
#define FMC_SPROM_1_LOCK_LOCK_Msk                 (0x1UL << FMC_SPROM_1_LOCK_LOCK_Pos)   /*!< TS_FMC::SPROM_1_LOCK: LOCK Mask     */
#define FMC_SPROM_0_LOCK_LOCK_Pos                 (0)                                    /*!< TS_FMC::SPROM_0_LOCK: LOCK Position */
#define FMC_SPROM_0_LOCK_LOCK_Msk                 (0x1UL << FMC_SPROM_0_LOCK_LOCK_Pos)   /*!< TS_FMC::SPROM_0_LOCK: LOCK Mask     */

enum {
	_FMC_CMD_AHB_WRITE_ENABLE = 0x4,
	_FMC_CMD_PAG_ERASE_ENABLE = 0x5,
	_FMC_CMD_ALL_CTRL_DISABLE = 0xF,
};
/*------------------------------------------ FMC define ------------------------------------------*/

/*++++++++++++++++++++++++++++++++++++ Peripheral Memory Map +++++++++++++++++++++++++++++++++++++*/
#define APB_BASE                             (                     0x40000000) //Advanced Peripheral Bus Base Address

#define   WDT_APB_ADDR_OFFSET                (                         0x0000)
#define  TMR0_APB_ADDR_OFFSET                (                         0x0020)
#define  TMR1_APB_ADDR_OFFSET                (                         0x0040)
#define  TMR2_APB_ADDR_OFFSET                (                         0x0060)
#define  TMR3_APB_ADDR_OFFSET                (                         0x0080)
#define   SYS_APB_ADDR_OFFSET                (                         0x1000)
#define  TCPC_APB_ADDR_OFFSET                (                         0x2000)
#define UART1_APB_ADDR_OFFSET                (                         0x3000)
#define UART2_APB_ADDR_OFFSET                (                         0x4000)
#define   GPA_APB_ADDR_OFFSET                (                         0x5000)
#define   GPB_APB_ADDR_OFFSET                (                         0x5040)
#define   GPC_APB_ADDR_OFFSET                (                         0x5080)
#define   GPD_APB_ADDR_OFFSET                (                         0x50C0)
#define ECAP1_APB_ADDR_OFFSET                (                         0x6000)
#define ECAP2_APB_ADDR_OFFSET                (                         0x6020)
#define ECAP3_APB_ADDR_OFFSET                (                         0x6040)
#define ECAP4_APB_ADDR_OFFSET                (                         0x6060)
#define ECAP5_APB_ADDR_OFFSET                (                         0x6080)
#define  UFCS_APB_ADDR_OFFSET                (                         0x7000)
#define  BADC_APB_ADDR_OFFSET                (                         0x8000)
#define  EADC_APB_ADDR_OFFSET                (                         0x8020)
#define  I2CS_APB_ADDR_OFFSET                (                         0x9000)
#define  I2CM_APB_ADDR_OFFSET                (                         0x9800)
#define EPWM1_APB_ADDR_OFFSET                (                         0xA000)
#define EPWM2_APB_ADDR_OFFSET                (                         0xB000)
//#define  DPDM_APB_ADDR_OFFSET                (                         0xC000)
#define  DPDM_SRC_APB_ADDR_OFFSET            (                         0xC080)
#define   DDM_APB_ADDR_OFFSET                (                         0xD000)
#define BPWM3_APB_ADDR_OFFSET                (                         0xE000)
#define BPWM4_APB_ADDR_OFFSET                (                         0xE010)
#define BPWM7_APB_ADDR_OFFSET                (                         0xE020)
#define BPWM8_APB_ADDR_OFFSET                (                         0xE030)
#define   FMC_APB_ADDR_OFFSET                (                         0xF000)

#define   WDT_BASE                           (APB_BASE +   WDT_APB_ADDR_OFFSET)
#define  TMR0_BASE                           (APB_BASE +  TMR0_APB_ADDR_OFFSET)
#define  TMR1_BASE                           (APB_BASE +  TMR1_APB_ADDR_OFFSET)
#define  TMR2_BASE                           (APB_BASE +  TMR2_APB_ADDR_OFFSET)
#define  TMR3_BASE                           (APB_BASE +  TMR3_APB_ADDR_OFFSET)
#define   SYS_BASE                           (APB_BASE +   SYS_APB_ADDR_OFFSET)
#define  TCPC_BASE                           (APB_BASE +  TCPC_APB_ADDR_OFFSET)
#define UART1_BASE                           (APB_BASE + UART1_APB_ADDR_OFFSET)
#define UART2_BASE                           (APB_BASE + UART2_APB_ADDR_OFFSET)
#define   GPA_BASE                           (APB_BASE +   GPA_APB_ADDR_OFFSET)
#define   GPB_BASE                           (APB_BASE +   GPB_APB_ADDR_OFFSET)
#define   GPC_BASE                           (APB_BASE +   GPC_APB_ADDR_OFFSET)
#define   GPD_BASE                           (APB_BASE +   GPD_APB_ADDR_OFFSET)
#define ECAP1_BASE                           (APB_BASE + ECAP1_APB_ADDR_OFFSET)
#define ECAP2_BASE                           (APB_BASE + ECAP2_APB_ADDR_OFFSET)
#define ECAP3_BASE                           (APB_BASE + ECAP3_APB_ADDR_OFFSET)
#define ECAP4_BASE                           (APB_BASE + ECAP4_APB_ADDR_OFFSET)
#define ECAP5_BASE                           (APB_BASE + ECAP5_APB_ADDR_OFFSET)
#define  UFCS_BASE                           (APB_BASE +  UFCS_APB_ADDR_OFFSET)
#define  BADC_BASE                           (APB_BASE +  BADC_APB_ADDR_OFFSET)
#define  EADC_BASE                           (APB_BASE +  EADC_APB_ADDR_OFFSET)
#define  I2CS_BASE                           (APB_BASE +  I2CS_APB_ADDR_OFFSET)
#define  I2CM_BASE                           (APB_BASE +  I2CM_APB_ADDR_OFFSET)
#define EPWM1_BASE                           (APB_BASE + EPWM1_APB_ADDR_OFFSET)
#define EPWM2_BASE                           (APB_BASE + EPWM2_APB_ADDR_OFFSET)
#define  DPDM_BASE                           (APB_BASE +  DPDM_APB_ADDR_OFFSET)
#define DPDM_SRC_BASE                        (APB_BASE +  DPDM_SRC_APB_ADDR_OFFSET)
#define   DDM_BASE                           (APB_BASE +   DDM_APB_ADDR_OFFSET)
#define BPWM3_BASE                           (APB_BASE + BPWM3_APB_ADDR_OFFSET)
#define BPWM4_BASE                           (APB_BASE + BPWM4_APB_ADDR_OFFSET)
#define BPWM7_BASE                           (APB_BASE + BPWM7_APB_ADDR_OFFSET)
#define BPWM8_BASE                           (APB_BASE + BPWM8_APB_ADDR_OFFSET)
#define   FMC_BASE                           (APB_BASE +   FMC_APB_ADDR_OFFSET)
/*------------------------------------ Peripheral Memory Map -------------------------------------*/

/*++++++++++++++++++++++++++++++++++++ Peripheral Declaration ++++++++++++++++++++++++++++++++++++*/
#define   WDT                                (          (TS_WDT  *)  WDT_BASE)
#define  TMR0                                (          (TS_TMR  *) TMR0_BASE)
#define  TMR1                                (          (TS_TMR  *) TMR1_BASE)
#define  TMR2                                (          (TS_TMR  *) TMR2_BASE)
#define  TMR3                                (          (TS_TMR  *) TMR3_BASE)
#define   SYS                                (          (TS_SYS  *)  SYS_BASE)
#define  TCPC                                (          (TS_TCPC *) TCPC_BASE)
#define UART1                                (          (TS_UART *)UART1_BASE)
#define UART2                                (          (TS_UART *)UART2_BASE)
#define   GPA                                (          (TS_GPA  *)  GPA_BASE)
#define   GPB                                (          (TS_GPB  *)  GPB_BASE)
#define   GPC                                (          (TS_GPC  *)  GPC_BASE)
#define   GPD                                (          (TS_GPD  *)  GPD_BASE)
#define ECAP1                                (          (TS_ECAP *)ECAP1_BASE)
#define ECAP2                                (          (TS_ECAP *)ECAP2_BASE)
#define ECAP3                                (          (TS_ECAP *)ECAP3_BASE)
#define ECAP4                                (          (TS_ECAP *)ECAP4_BASE)
#define ECAP5                                (          (TS_ECAP *)ECAP5_BASE)
#define  UFCS                                (          (TS_UFCS *) UFCS_BASE)
#define  BADC                                (          (TS_BADC *) BADC_BASE)
#define  EADC                                (          (TS_EADC *) EADC_BASE)
#define  I2CS                                (          (TS_I2CS *) I2CS_BASE)
#define  I2CM                                (          (TS_I2CM *) I2CM_BASE)
#define EPWM1                                (          (TS_EPWM *)EPWM1_BASE)
#define EPWM2                                (          (TS_EPWM *)EPWM2_BASE)
#define  DPDM                                (          (TS_DPDM *) DPDM_BASE)
#define DPDM_SRC                             (          (TS_DPDM_SRC*)DPDM_SRC_BASE)
#define   DDM                                (          (TS_DDM  *)  DDM_BASE)
#define BPWM3                                (          (TS_BPWM *)BPWM3_BASE)
#define BPWM4                                (          (TS_BPWM *)BPWM4_BASE)
#define BPWM7                                (          (TS_BPWM *)BPWM7_BASE)
#define BPWM8                                (          (TS_BPWM *)BPWM8_BASE)
#define   FMC                                (          (TS_FMC  *)  FMC_BASE)
/*------------------------------------ Peripheral Declaration ------------------------------------*/

/*+++++++++++++++++++++++++++++++++ Interrupt Number Definition ++++++++++++++++++++++++++++++++++*/
typedef enum {
	/******  CK802 Processor Exceptions Numbers ******************/
	IRQn_Reset                       = -32, // 0
	IRQn_Misaligned_Access           = -31, // 1
	IRQn_Access_Error                = -30, // 2
	IRQn_Exception_Reserved_03       = -29, // 3
	IRQn_Illegal                     = -28, // 4
	IRQn_Privlege_Violation          = -27, // 5
	IRQn_Exception_Reserved_06       = -26, // 6
	IRQn_Breakpoint_Exception        = -25, // 7
	IRQn_Unrecoverable_Error         = -24, // 8
	IRQn_Exception_Reserved_09       = -23, // 9
	IRQn_Exception_Reserved_10       = -22, //10
	IRQn_Exception_Reserved_11       = -21, //11
	IRQn_Exception_Reserved_12       = -20, //12
	IRQn_Exception_Reserved_13       = -19, //13
	IRQn_Exception_Reserved_14       = -18, //14
	IRQn_Exception_Reserved_15       = -17, //15
	IRQn_Trap_Instruction_0          = -16, //16
	IRQn_Trap_Instruction_1          = -15, //17
	IRQn_Trap_Instruction_2          = -14, //18
	IRQn_Trap_Instruction_3          = -13, //19 //ISR_Divided_By_Zero
	/******  CK802 Specific Interrupt Numbers ********************/
	IRQn_PROT                        =   0,  /*!< PVD and TSD Interrupt                          */
	IRQn_WDT                         =   1,  /*!< Watch Dog Timer Interrupt                      */
	IRQn_TMR0                        =   2,  /*!< TMR0 Interrupt                                 */
	IRQn_TMR1                        =   3,  /*!< TMR1 Interrupt                                 */
	IRQn_TMR2                        =   4,  /*!< TMR2 Interrupt                                 */
	IRQn_TMR3                        =   5,  /*!< TMR3 Interrupt                                 */
	IRQn_EADC                        =   6,  /*!< Enhanced ADC Interrupt                         */
	IRQn_BADC                        =   7,  /*!< Basic ADC Interrupt                            */
	IRQn_ECAP1                       =   8,  /*!< Enhanced capture 1 Interrupt                   */
	IRQn_ECAP2                       =   9,  /*!< Enhanced capture 2 Interrupt                   */
	IRQn_ECAP3                       =  10,  /*!< Enhanced capture 3 Interrupt                   */
	IRQn_ECAP4                       =  11,  /*!< Enhanced capture 4 Interrupt                   */
	IRQn_ECAP5                       =  12,  /*!< Enhanced capture 5 Interrupt                   */
	IRQn_GPIO                        =  13,  /*!< GPIO_PA/PB/PC/PD Interrupt                     */
	IRQn_UART1                       =  14,  /*!< UART1 Interrupt                                */
	IRQn_UART2                       =  15,  /*!< UART2 Interrupt                                */
	IRQn_I2CS                        =  16,  /*!< I2C slave Interrupt                            */
	IRQn_I2CM                        =  17,  /*!< I2C master Interrupt                           */
	IRQn_USBPD                       =  18,  /*!< USBPD PHY-layer Interrupt                      */
	IRQn_UFCS                        =  19,  /*!< UFCS PHY-layer Interrupt                       */
	IRQn_DPDM_SINK                   =  20,  /*!< DPDM sink Interrupt                            */
	IRQn_DCP_HVDCP                   =  21,  /*!< DPDM HVDCP Interrupt                           */
	IRQn_QC_SRC                      =  22,  /*!< DPDM QC_SRC Interrupt                          */
	IRQn_AFC_SCP_SRC                 =  23,  /*!< DPDM AFC_SCP_SRC Interrupt                     */
	IRQn_FSK1                        =  24,  /*!< FSK1 Interrupt                                 */
	IRQn_FSK2                        =  25,  /*!< FSK2 Interrupt                                 */
	IRQn_SPEC_INT_RESERVED_26        =  26,
	IRQn_DMA                         =  27,  /*!< Flash memory control DMA done Interrupt        */
	IRQn_TCPC                        =  28,  /*!< Type-C Port Controller status change Interrupt */
	IRQn_SPEC_INT_RESERVED_29        =  29,
	IRQn_SPEC_INT_RESERVED_30        =  30,
	IRQn_SPEC_INT_RESERVED_31        =  31,
} TE_IRQn_Type;
/*--------------------------------- Interrupt Number Definition ----------------------------------*/

/*+++++++++++++++++++++++++++++++++++ CK802-Core Peripheral VIC ++++++++++++++++++++++++++++++++++*/
typedef struct {
	__IO uint32_t ISER;           /*!< E000_E100: Interrupt Set Enable Register               */
	     uint32_t RESERVED0[15];  /*!< E000_E104-E000_E13F                                    */
	__IO uint32_t IWER;           /*!< E000_E140: Interrupt Low power wake-up Enable Register */
	     uint32_t RESERVED1[15];  /*!< E000_E144-E000_E17F                                    */
	__IO uint32_t ICER;           /*!< E000_E180: Interrupt Clear Enable Register             */
	     uint32_t RESERVED2[15];  /*!< E000_E184-E000_E1BF                                    */
	__IO uint32_t IWDR;           /*!< E000_E1C0: Interrupt Low power wake-up Clear Register  */
	     uint32_t RESERVED3[15];  /*!< E000_E1C4-E000_E1FF                                    */
	__IO uint32_t ISPR;           /*!< E000_E200: Interrupt Set Pending Register              */
	     uint32_t RESERVED4[31];  /*!< E000_E204-E000_E27F                                    */
	__IO uint32_t ICPR;           /*!< E000_E280: Interrupt Clear Pending Register            */
	     uint32_t RESERVED5[31];  /*!< E000_E284-E000_E2FF                                    */
	__IO uint32_t IABR;           /*!< E000_E300: Interrupt Active Handle Register            */
	     uint32_t RESERVED6[63];  /*!< E000_E304-E000_E3FF                                    */
	__IO uint32_t IPR[8];         /*!< E000_E400-E000_E41F: Interrupt Priority Register       */
	     uint32_t RESERVED7[504]; /*!< E000_E420-E000_EBFF                                    */
	__I  uint32_t ISR;            /*!< E000_EC00: Interrupt Status Register                   */
	__IO uint32_t IPTR;           /*!< E000_EC04: Interrupt Priority Threshold Register       */
} TS_VIC;

#define VIC_BASE                       (         0xE000E100) /*!< ck802 core VIC Base Address */
#define VIC                            ((TS_VIC *) VIC_BASE)

static inline void VIC_vModuleEnable(void)
{
	__asm volatile("psrset ee, ie");
}

static inline void VIC_vModuleDisable(void)
{
	__asm volatile("psrclr ie");
}

static inline void VIC_vEnableIRQ(TE_IRQn_Type IRQn)
{
	VIC->ISER = (uint32_t)(1 << (((uint32_t)(int32_t)IRQn) & 0x1F));
}

static inline void VIC_vDisableIRQ(TE_IRQn_Type IRQn)
{
	VIC->ICER = (uint32_t)(1 << (((uint32_t)(int32_t)IRQn) & 0x1F));
}

static inline void VIC_vSetPendingIRQ(TE_IRQn_Type IRQn)
{
	VIC->ISPR = (uint32_t)(1 << (((uint32_t)(int32_t)IRQn) & 0x1F));
}

static inline void VIC_vClearPendingIRQ(TE_IRQn_Type IRQn)
{
	VIC->ICPR = (uint32_t)(1 << (((uint32_t)(int32_t)IRQn) & 0x1F));
}

#define _BIT_SHIFT(IRQn)         (((((uint32_t)(int32_t)(IRQn))) & 0x03UL) << 3UL)
#define _IPR_INDEX(IRQn)         ( (((uint32_t)(int32_t)(IRQn)) >>    2UL)       )

static inline void VIC_vSetPriority(TE_IRQn_Type IRQn, uint32_t priority)
{
	if ((int32_t)(IRQn) >= 0)
	{
		VIC->IPR[_IPR_INDEX(IRQn)] = ((uint32_t)(VIC->IPR[_IPR_INDEX(IRQn)] & ~(0xFFUL << _BIT_SHIFT(IRQn))) | (((priority << 6U) & (uint32_t)0xFFUL) << _BIT_SHIFT(IRQn)));
	}
}
/*----------------------------------- CK802-Core Peripheral VIC ----------------------------------*/
#define __write_08bits(addr, value)    (*((volatile uint8_t  *)(addr))) = (value)
#define __write_16bits(addr, value)    (*((volatile uint16_t *)(addr))) = (value)
#define __write_32bits(addr, value)    (*((volatile uint32_t *)(addr))) = (value)
#define __read_08bits(addr)            (*((volatile uint8_t  *)(addr)))
#define __read_16bits(addr)            (*((volatile uint16_t *)(addr)))
#define __read_32bits(addr)            (*((volatile uint32_t *)(addr)))

#define __swap_16(x)    ((uint16_t)((((uint16_t)(x) & 0xff00) >> 8) | (((uint16_t)(x) & 0x00ff) << 8)))
#define __swap_32(x)    ((uint32_t)((((uint32_t)(x) & 0xff000000) >> 24) | (((uint32_t)(x) & 0x00ff0000) >> 8) | (((uint32_t)(x) & 0x0000ff00) << 8) | (((uint32_t)(x) & 0x000000ff) << 24)))

/* Byte Mask Definitions */
#define BYTE0_Msk                      (                    0x000000FF)
#define BYTE1_Msk                      (                    0x0000FF00)
#define BYTE2_Msk                      (                    0x00FF0000)
#define BYTE3_Msk                      (                    0xFF000000)
#define _GET_BYTE0(u32Param)           (((u32Param) & BYTE0_Msk) >>  0)  /*!< Extract Byte 0 (Bit  0~ 7) from parameter u32Param */
#define _GET_BYTE1(u32Param)           (((u32Param) & BYTE1_Msk) >>  8)  /*!< Extract Byte 1 (Bit  8~15) from parameter u32Param */
#define _GET_BYTE2(u32Param)           (((u32Param) & BYTE2_Msk) >> 16)  /*!< Extract Byte 2 (Bit 16~23) from parameter u32Param */
#define _GET_BYTE3(u32Param)           (((u32Param) & BYTE3_Msk) >> 24)  /*!< Extract Byte 3 (Bit 24~31) from parameter u32Param */

#define __COMPILE_DATE_YAR ((((__DATE__[7] - '0') * 10 + (__DATE__[8] - '0')) * 10 + (__DATE__[9] - '0')) * 10 + (__DATE__[10] - '0'))
#define __COMPILE_DATE_MTH (__DATE__[2] == 'n' ? (__DATE__[1] == 'a' ? 1 : 6) \
		: __DATE__[2] == 'b' ? 2 \
		: __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 3 : 4) \
		: __DATE__[2] == 'y' ? 5 \
		: __DATE__[2] == 'n' ? 6 \
		: __DATE__[2] == 'l' ? 7 \
		: __DATE__[2] == 'g' ? 8 \
		: __DATE__[2] == 'p' ? 9 \
		: __DATE__[2] == 't' ? 10 \
		: __DATE__[2] == 'v' ? 11 : 12)
#define __COMPILE_DATE_DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

/******************************************************************************/
/*                         Peripheral header files                            */
/******************************************************************************/
#include "badc.h"
#include "bpwm.h"
#include "ddm.h"
#include "dpdm.h"
#include "eadc.h"
#include "ecap.h"
#include "epwm.h"
#include "fmc.h"
#include "gpio.h"
#include "i2cm.h"
#include "i2cs.h"
#include "isr.h"
#include "sys.h"
#include "tcpc.h"
#include "timer.h"
#include "uart.h"
#include "vic.h"
#include "wdt.h"

#endif /* REGDEF_H_ */
