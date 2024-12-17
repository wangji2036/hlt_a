#ifndef ADP_H_
#define ADP_H_

enum adp_type_t {
	EADP_TYPE_IDUNKNOWN = 0x00,
	EADP_TYPE_BC1P2_SDP = 0x01,
	EADP_TYPE_BC1P2_CDP = 0x02,
	EADP_TYPE_BC1P2_DCP = 0x03,
	EADP_TYPE_QC2P0_09V = 0x05,
	EADP_TYPE_QC3P0_12V = 0x06,
	EADP_TYPE_PD2P0_12V = 0x07,
	EADP_TYPE_QC3P0_20V = 0x09,
	EADP_TYPE_PD3P0_30W = 0x0C,
	EADP_TYPE_PD3P0_50W = 0x0E,
	//////////////////////////
	EADP_TYPE_PD2P0_05V = 0x20,
	EADP_TYPE_PD2P0_09V = 0x21,
	EADP_TYPE_PD3P0_10W = 0x22,
	EADP_TYPE_PD3P0_20W = 0x23,
	//////////////////////////
	EADP_TYPE_DCSRC_05V = 0x30,
	EADP_TYPE_DCSRC_09V = 0x31,
	EADP_TYPE_DCSRC_12V = 0x32,

	EADP_TYPE_POWERBANK_05V = 0x40,// wireless and C discharge
	EADP_TYPE_POWERBANK_09V = 0x41,// wireless and C charge (no-PPS)
	EADP_TYPE_POWERBANK_PPS = 0x42,// wireless only
	EADP_TYPE_POWERBANK_WIRELESS_ONLY = 0x43, // wireless and C charge (PPS)
};

struct adp_t {
	uint16_t adp_type: 8;
	uint16_t pwr_high: 8; //unit:0.5W
	uint16_t volt_min;
	uint16_t volt_max;
};

void fml_adp_init(void);
void fml_adp_volt_set(uint16_t);
void fml_adp_type_set(enum adp_type_t adp_type, uint16_t volt_min, uint16_t volt_max, uint16_t pwr_high);

#endif /* ADP_H_ */
