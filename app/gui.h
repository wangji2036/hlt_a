#ifndef GUI_H_
#define GUI_H_

#define APP_REG_BUFF_SIZE    64

typedef enum {
	tx_fw_version = 0x00,
	tx_chip_id,
	tx_status,
	reserved_0,
	tx_required_volt_l,
	tx_required_volt_h,
	tx_working_volt_l,//vbus
	tx_working_volt_h,
	tx_working_current_l,//ipwr->ibus
	tx_working_current_h,
	tx_ntc_tempr_l,//Cels
	tx_ntc_tempr_h,
	master_msg,//R/W
	tx_q_value,
	tx_lc_freq_value_l,
	tx_lc_freq_value_h,
	master_volt_l,//R/W
	master_volt_h,
	master_curr_l,
	master_curr_h,
	reserved_1,
	reserved_2,
	tx_pwr_volt_l,
	tx_pwr_volt_h,
	tx_pwr_isns_l,
	tx_pwr_isns_h,
} iic_xfer_info_idx_t;

uint8_t app_reg_buff[APP_REG_BUFF_SIZE];

void apl_gui_init(void);
void iic_read_info_sync(void);
void iic_write_info_sync(void);

#endif /* GUI_H_ */
