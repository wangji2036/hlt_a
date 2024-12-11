#ifndef NU103x_H_
#define NU103x_H_

union nu103x_t {
	/**
	 * @var TS_NU1030_STS
	 * NU1030 status, read only. default value 0x00000000
	 * ---------------------------------------------------------------------------------------------------
	 * |Bits    |Field               |Descriptions
	 * | :----: | :----:             | :---- |
	 * |[0]     |ALL_RST             |Reset all configuration to default state
	 * |        |                    |0 = no effect
	 * |        |                    |1 = reset.
	 * |        |                    |Note: W1C, will always show 0.
	 * |[1]     |OCP_THD             |NU1030 I-peak OCP Threshold
	 * |        |                    |0 =  8A
	 * |        |                    |1 = 10A
	 * |[2]     |LPM_STS             |Low Power Mode status
	 * |        |                    |0 = normal mode
	 * |        |                    |1 = low power mode
	 * |[3]     |QDT_STS             |Enable Q Measurement
	 * |        |                    |0 = not in Q measure state
	 * |        |                    |1 = in Q measure state
	 * |        |                    |Note: W1C, will always show 0.
	 * |[4]     |VDD_LDO_V4P8_STS    |VDD LDO V4P8 output status
	 * |        |                    |0 = LDO VDD output 4.8V on
	 * |        |                    |1 = LDO VDD output 4.8V off
	 * |[5]     |VDD_V5V_BUCK_STS    |VDD V5V Buck output status
	 * |        |                    |0 = disable
	 * |        |                    |1 = enable
	 * |[7:6]   |DRVH2_CONN_STS      |DRVH2 work mode status
	 * |        |                    |0 = null
	 * |        |                    |1 = connect to SW2
	 * |        |                    |2 = connect to PVIN
	 * |        |                    |3 = null
	 * |        |                    |Note: after POR, you should initial this pin configure.
	 * |[8]     |DRVH2_TURN_STS      |DRVH2_TURN status
	 * |        |                    |0 = off
	 * |        |                    |1 = on
	 * |[9]     |DRVH1_TURN_STS      |DRVH1_TURN status
	 * |        |                    |0 = off
	 * |        |                    |1 = on
	 * |[10]    |DRVHx_SLEW_RATE     |DRVH1 and DRVH2 slew rate status
	 * |        |                    |0 = fast, 5~10ns
	 * |        |                    |1 = slow, 40ns
	 * |[11]    |DRVHx_DEAD_TIME     |DRVH1 and DRVH2 dead time status
	 * |        |                    |0 = adaptive control
	 * |        |                    |1 = fixed to 10ns
	 * |[12]    |VDM_PIN_MFC_STS     |VDM PIN Multiple Function control status
	 * |        |                    |0 = VDM pin as VCAP output
	 * |        |                    |1 = VDM pin as EVDM input
	 * |[13]    |DMO1_OUT_MODE       |DMO1 output mode status
	 * |        |                    |0 = DDM
	 * |        |                    |1 = QDT, Q-Factor
	 * |[14]    |DMO1_DDM_SRC        |if DMO1_OUT_MODE = DDM, this bit show DMO1 DDM input source
	 * |        |                    |0 = IAVG, inner input average current of PVIN
	 * |        |                    |1 = EVDM, outer VCOIL signal filtered by LPF and HPF
	 * |[15]    |DMO1_DDM_BPF        |if DMO1_OUT_MODE = DDM, this bit show DMO1 DDM Band Pass Filter status
	 * |        |                    |0 = 2ord
	 * |        |                    |1 = 1ord
	 * |[16]    |DMO1_DDM_GAIN_MOD   |if DMO1_OUT_MODE = DDM, this bit show DMO1 DDM gain configure mode
	 * |        |                    |0 = auto mode
	 * |        |                    |1 = fixed mode
	 * |[17]    |DMO1_DDM_GAIN_FIX   |if DMO1_OUT_MODE = DDM, and DMO1_DDM_GAIN_MOD = fixed mode, this bit show DMO1 DDM fixed gain configure value
	 * |        |                    |0 = 36
	 * |        |                    |1 = 60
	 * |[19:18] |DMO2_OUT_MODE       |DMO2 output mode status
	 * |        |                    |0 = DDM
	 * |        |                    |1 = QDT, resonant frequency
	 * |        |                    |2 = VCAP, analog signal for mason digital DDM.
	 * |        |                    |3 = null
	 * |[21:20] |DMO2_VCAP_RATIO_K   |DMO2 original high-voltage VCAP R3||R4 configure status
	 * |        |                    |0 = K2, 180K/26K
	 * |        |                    |1 = K3, 180K/13K
	 * |        |                    |2 = K1, 180K/52K
	 * |        |                    |3 = null
	 * |[22]    |DMO2_DDM_SRC        |if DMO2_OUT_MODE = DDM, this bit show DMO2 DDM input source
	 * |        |                    |0 = VCAP, voltage of resonant capacitor of LC tank after a reasonable ratio amplitude
	 * |        |                    |1 = PHASE
	 * |[23]    |DMO2_DDM_BPF        |if DMO2_OUT_MODE = DDM, this bit show DMO2 DDM Band Pass Filter status
	 * |        |                    |0 = 2ord
	 * |        |                    |1 = 1ord
	 * |[24]    |DMO2_DDM_GAIN_MOD   |if DMO2_OUT_MODE = DDM, this bit show DMO2 DDM gain configure mode
	 * |        |                    |0 = auto mode
	 * |        |                    |1 = fixed mode
	 * |[25]    |DMO2_DDM_GAIN_FIX   |if DMO2_OUT_MODE = DDM, and DMO2_DDM_GAIN_MOD = fixed mode, this bit show DMO2 DDM fixed gain configure value
	 * |        |                    |0 = 36
	 * |        |                    |1 = 60
	 * |[26]    |DMOx_DDM_CMP_HYST   |if DMOx_OUT_MODE = DDM, this bit show DMOx DDM comparator hysteresis configure value
	 * |        |                    |0 = 12.5mV
	 * |        |                    |1 = 30.0mV
	 * |        |                    |Note: DMO1 and DMO2 share same comparator hysteresis.
	 */
	struct {
		uint32_t ALL_RST           : 1;
		uint32_t OCP_THD           : 1;
		uint32_t LPM_STS           : 1;
		uint32_t QDT_STS           : 1;
		uint32_t VDD_LDO_V4P8_STS  : 1;
		uint32_t VDD_V5V_BUCK_STS  : 1;
		uint32_t DRVH2_CONN_STS    : 2;
		uint32_t DRVH2_TURN_STS    : 1;
		uint32_t DRVH1_TURN_STS    : 1;
		uint32_t DRVHx_SLEW_RATE   : 1;
		uint32_t DRVHx_DEAD_TIME   : 1;
		uint32_t VDM_PIN_MFP_STS   : 1;
		uint32_t DMO1_OUT_MODE     : 1;
		uint32_t DMO1_DDM_SRC      : 1;
		uint32_t DMO1_DDM_BPF      : 1;
		uint32_t DMO1_DDM_GAIN_MOD : 1;
		uint32_t DMO1_DDM_GAIN_FIX : 1;
		uint32_t DMO2_OUT_MODE     : 2;
		uint32_t DMO2_VCAP_RATIO_K : 2;
		uint32_t DMO2_DDM_SRC      : 1;
		uint32_t DMO2_DDM_BPF      : 1;
		uint32_t DMO2_DDM_GAIN_MOD : 1;
		uint32_t DMO2_DDM_GAIN_FIX : 1;
		uint32_t DMOx_DDM_CMP_HYST : 1;
		uint32_t                   : 5;
	} BITS;
	uint32_t WORD;
};

enum nu103x_cmd_t {
	//reset all configuration to default state
	_1030_CFG_ALL_RST = 22,

	//I-peak OCP protect configure
	_1030_CFG_OCP_08A = 19, //default
	_1030_CFG_OCP_10A = 28, //if configured to 10A, can't configure back to 8A, need reset 1030. design bug.

	//Low Power Mode Control
	_1030_CFG_LPM_EN_ = 23,
	_1030_CFG_LPM_DIS =  3,

	//enable Q measurement
	_1030_CFG_QDT_EN_ = 24,

	//VDD LDO and Buck configure
	_1030_CFG_VDD_LDO_V4P8_ON_ = 14, //default
	_1030_CFG_VDD_LDO_V4P8_OFF = 15,
	_1030_CFG_VDD_V5V_BUCK_DIS = 54, //default, VDD will output 4.8V
	_1030_CFG_VDD_V5V_BUCK_EN_ = 55, //buck enable, VDD output 5.0V

	//DRVH2 work mode configure
	_1030_CFG_DRVH2_CONN_SW2 = 52,
	_1030_CFG_DRVH2_CONN_VIN = 53,
	//DRVH2 on or off configure
	_1030_CFG_DRVH2_TURN_OFF = 12, //default
	_1030_CFG_DRVH2_TURN_ON_ = 13,
	//DRVH1 on or off configure
	_1030_CFG_DRVH1_TURN_OFF = 10, //default
	_1030_CFG_DRVH1_TURN_ON_ = 11,
	//DRVHx slew rate and dead time configure
	_1030_CFG_DRVH_SLEW_RATE_FAST = 4, //default, 5~10ns
	_1030_CFG_DRVH_SLEW_RATE_SLOW = 5, //40ns
	_1030_CFG_DRVH_DEAD_TIME_AUTO = 6, //default
	_1030_CFG_DRVH_DEAD_TIME_FIXD = 7, //10ns

	//VDM pin input or output configure
	_1030_CFG_VDM_PIN_VCAP_OUT = 29, //default
	_1030_CFG_VDM_PIN_EVDM_IN_ = 30,

	//DMO1 output mode configure
	_1030_CFG_DMO1_OUT_MODE_DDM = 31, //default
	_1030_CFG_DMO1_OUT_MODE_QDT = 32,
	//DMO1 DDM input source
	_1030_CFG_DMO1_DDM_SRC_IAVG = 36, //default
	_1030_CFG_DMO1_DDM_SRC_EVDM = 37, //external circuits VDM
	//DMO1 DDM Band Pass Filter option
	_1030_CFG_DMO1_DDM_BPF_2ORD = 46, //default
	_1030_CFG_DMO1_DDM_BPF_1ORD = 47,
	//DMO1 DDM gain mode configure
	_1030_CFG_DMO1_DDM_GAIN_MODE_AUTO = 40, //default
	_1030_CFG_DMO1_DDM_GAIN_MODE_FIXD = 41,
	//DMO1 DDM fixed gain value configure
	_1030_CFG_DMO1_DDM_FIXED_GAIN_X36 = 42, //default
	_1030_CFG_DMO1_DDM_FIXED_GAIN_X60 = 43,

	//DMO2 output mode configure
	_1030_CFG_DMO2_OUT_MODE_DDM = 33, //default
	_1030_CFG_DMO2_OUT_MODE_QDT = 34,
	_1030_CFG_DMO2_OUT_MODE_CAP = 35,
	//DMO2 VCAP K ratio configure
	_1030_CFG_DMO2_VCAP_RATIO_K2 = 25, //default, 180K/26K
	_1030_CFG_DMO2_VCAP_RATIO_K3 = 26, //180K/13K
	_1030_CFG_DMO2_VCAP_RATIO_K1 = 27, //180K/52K
	//DMO2 DDM input source
	_1030_CFG_DMO2_DDM_SRC_VCAP = 38, //default
	_1030_CFG_DMO2_DDM_SRC_PHAS = 39,
	//DMO2 DDM Band Pass Filter option
	_1030_CFG_DMO2_DDM_BPF_2ORD = 48, //default
	_1030_CFG_DMO2_DDM_BPF_1ORD = 49,
	//DMO2 DDM gain mode configure
	_1030_CFG_DMO2_DDM_GAIN_MODE_AUTO = 16, //default
	_1030_CFG_DMO2_DDM_GAIN_MODE_FIXD = 17,
	//DMO2 DDM fixed gain value configure
	_1030_CFG_DMO2_DDM_FIXED_GAIN_X36 = 44, //default
	_1030_CFG_DMO2_DDM_FIXED_GAIN_X60 = 45,

	//Configure Comparator hysteresis for DMO1 and DMO2 DDM
	_1030_CFG_DMOx_DDM_CMP_HYST_12P5mV = 50, //default
	_1030_CFG_DMOx_DDM_CMP_HYST_30P0mV = 51,
};

#define _NU1030_OCP_THD_08A                      0
#define _NU1030_OCP_THD_10A                      1
#define _NU1030_LPM_STS_DIS                      0
#define _NU1030_LPM_STS_EN_                      1

#define _NU1030_VDD_LDO_V4P8_STS_ON_             0
#define _NU1030_VDD_LDO_V4P8_STS_OFF             1
#define _NU1030_VDD_V5V_BUCK_STS_OFF             0
#define _NU1030_VDD_V5V_BUCK_STS_ON_             1

#define _NU1030_DRVH2_CONN_STS_SW2               1
#define _NU1030_DRVH2_CONN_STS_VIN               2
#define _NU1030_DRVH2_TURN_STS_OFF               0
#define _NU1030_DRVH2_TURN_STS_ON_               1
#define _NU1030_DRVH1_TURN_STS_OFF               0
#define _NU1030_DRVH1_TURN_STS_ON_               1
#define _NU1030_DRVHx_SLEW_RATE_10ns             0
#define _NU1030_DRVHx_SLEW_RATE_40ns             1
#define _NU1030_DRVHx_DEAD_TIME_Self_Adaptive    0
#define _NU1030_DRVHx_DEAD_TIME_Fixed_to_10ns    1

#define _NU1030_VDM_PIN_MFC_STS_VCAP_OUT         0
#define _NU1030_VDM_PIN_MFC_STS_EVDM_IN_         1

#define _NU1030_DMO1_OUT_MODE_DDM                0
#define _NU1030_DMO1_OUT_MODE_QDT                1
#define _NU1030_DMO1_DDM_SRC_IAVG                0
#define _NU1030_DMO1_DDM_SRC_EVDM                1
#define _NU1030_DMO1_DDM_BPF_2ord                0
#define _NU1030_DMO1_DDM_BPF_1ord                1
#define _NU1030_DMO1_DDM_GAIN_MODE_AUTO          0
#define _NU1030_DMO1_DDM_GAIN_MODE_FIXD          1
#define _NU1030_DMO1_DDM_GAIN_FIXED_X36          0
#define _NU1030_DMO1_DDM_GAIN_FIXED_X60          1

#define _NU1030_DMO2_OUT_MODE_DDM                0
#define _NU1030_DMO2_OUT_MODE_QDT                1
#define _NU1030_DMO2_OUT_MODE_CAP                2
#define _NU1030_DMO2_VCAP_RATIO_K2               0
#define _NU1030_DMO2_VCAP_RATIO_K3               1
#define _NU1030_DMO2_VCAP_RATIO_K1               2
#define _NU1030_DMO2_DDM_SRC_VCAP                0
#define _NU1030_DMO2_DDM_SRC_PHAS                1
#define _NU1030_DMO2_DDM_BPF_2ord                0
#define _NU1030_DMO2_DDM_BPF_1ord                1
#define _NU1030_DMO2_DDM_GAIN_MODE_AUTO          0
#define _NU1030_DMO2_DDM_GAIN_MODE_FIXD          1
#define _NU1030_DMO2_DDM_GAIN_FIXED_X36          0
#define _NU1030_DMO2_DDM_GAIN_FIXED_X60          1
#define _NU1030_DMOx_DDM_CMP_HYST_12P5mV         0
#define _NU1030_DMOx_DDM_CMP_HYST_30P0mV         1

void fml_nu103x_por_init(void);
void fml_nu103x_por_rst(void);
void fml_nu103x_ddm_init(void);
void fml_nu103x_qdt_init(void);
void fml_nu103x_config(enum nu103x_cmd_t cmd);

void fml_nu103x_dmo1_param_set(enum nu103x_cmd_t ddm_src, enum nu103x_cmd_t gain_mod, enum nu103x_cmd_t gain_amp);
void fml_nu103x_dmo2_param_set(enum nu103x_cmd_t ddm_src, enum nu103x_cmd_t gain_mod, enum nu103x_cmd_t gain_amp, enum nu103x_cmd_t vcap_k_ratio);

void fml_nu103x_dmo1_ping_param_chose(void);
void fml_nu103x_dmo1_xfer_param_chose(void);
void fml_nu103x_dmo2_ping_param_chose(void);
void fml_nu103x_dmo2_xfer_param_chose(void);

void fml_nu103x_ddm_param_change(void);

#endif /* NU103x_H_ */
