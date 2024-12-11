#include "regdef.h"
#include "printk.h"
#include "gui.h"
#include "g_data.h"
#include "config.h"

uint8_t app_reg_buff[APP_REG_BUFF_SIZE] =
{
    0x02, //00-> TX FW version
	0xBB, //01-> chip id
	0x00, //02-> TX status
	0x00, //03-> reserved
	0x88, //04-> TX required voltage LSB
	0x13, //05-> TX required voltage MSB
	0x88, //06-> TX working voltage LSB
	0x13, //07-> TX working voltage MSB
	0x00, //08-> TX working current LSB
	0x00, //09-> TX working current MSB
	0x15, //0A-> TX NTC voltage LSB
	0x05, //0B-> TX NTC voltage MSB
	0x00, //0C-> adaptor message
	0x00, //0D-> TX QDT VALUE
	0x00, //0E-> TX FREQ VALUE LSB
	0x00, //0F-> TX FREQ VALUE MSB
////////////////////////////////////////////////////////////////////
    0x00, //10-> reserved
	0x00, //11-> reserved
	0x00, //12-> reserved
	0x00, //13-> reserved
	0x00, //14-> reserved
	0x00, //15-> reserved
	0x00, //16-> reserved
	0x00, //17-> reserved
	0x00, //18-> reserved
	0x00, //19-> reserved
	0x00, //1A-> reserved
	0x00, //1B-> reserved
	0x00, //1C-> reserved
	0x00, //1D-> reserved
	0x00, //1E-> reserved
	0x00, //1F-> reserved
};

typedef union {
	struct {
		uint8_t MOD : 1;
		uint8_t LEN : 2;
		uint8_t CMD : 5;
	} BITS;
	uint8_t BYTE;
} TS_I2CS_APP2MCU_CMD;

typedef union {
	struct {
		uint8_t adp_type : 3;
		uint8_t reserved : 4;
		uint8_t ready	: 1;
	} bits;
	uint8_t byte;
} iic_master_msg;

typedef union {
	struct {
		uint8_t pin8 : 1;
		uint8_t pin19 : 1;
		uint8_t reserved	: 6;
	} bits;
	uint8_t byte;
} iic_io_ctl;

void apl_i2cs_GR03_int_handler(void)
{
	TS_I2CS_APP2MCU_CMD app2mcu_cmd;
	TS_I2CS_MCU_RO_DATA app2mcu_dat;
	TS_I2CS_MCU_RW_DATA mcu2app_dat;

	app2mcu_dat.WORD = I2CS->RO_DATA.WORD;
	app2mcu_cmd.BYTE = app2mcu_dat.BITS.GR03;
	mcu2app_dat.WORD = app2mcu_dat.WORD;

	uint8_t reg_addr = app2mcu_dat.BITS.GR02;

	if (app2mcu_cmd.BITS.MOD) //1: read
	{
		if (app2mcu_cmd.BITS.LEN == 1)
		{
			mcu2app_dat.BITS.GR10 = app_reg_buff[reg_addr + 0];
		}
		else if (app2mcu_cmd.BITS.LEN == 2)
		{
			mcu2app_dat.BITS.GR10 = app_reg_buff[reg_addr + 0];
			mcu2app_dat.BITS.GR11 = app_reg_buff[reg_addr + 1];
		}
	}
	else //0: write
	{
		if (app2mcu_cmd.BITS.LEN == 1)
		{
			app_reg_buff[reg_addr + 0] = app2mcu_dat.BITS.GR00;
		}
		else if (app2mcu_cmd.BITS.LEN == 2)
		{
			app_reg_buff[reg_addr + 0] = app2mcu_dat.BITS.GR00;
			app_reg_buff[reg_addr + 1] = app2mcu_dat.BITS.GR01;
		}
	}

	I2CS->RW_DATA.WORD = mcu2app_dat.WORD;
}

void apl_gui_init(void)
{
	hal_i2cs_reg_int_cb(apl_i2cs_GR03_int_handler);
}

void iic_read_info_sync(void)
{
	uint8_t tmp_tx_status = 0;
	uint8_t tmp_err_otp = 0;
	uint8_t tmp_err_ovp = 0;
	uint8_t tmp_err_fod = 0;
	uint8_t tmp_err = 0;
	tmp_tx_status |= (gd->tx_infos.rx_status)? 0x01 : 0x00;

	if (gd->prot_sts.tdie_otp_flag || gd->prot_sts.tntc_otp_flag)
	{
		tmp_err_otp = 1;
	}
	else
	{
		tmp_err_otp = 0;
	}

	if (gd->prot_sts.vbus_ovp_flag || gd->prot_sts.vpwr_ovp_flag)
	{
		tmp_err_ovp = 1;
	}
	else
	{
		tmp_err_ovp = 0;
	}

	if ((gd->prot_sts.q_fod_flag) || (gd->prot_sts.xfer_fod_flag))
	{
		tmp_err_fod = 1;
	}
	else
	{
		tmp_err_fod = 0;
	}

	if (gd->prot_sts.tdie_otp_flag || gd->prot_sts.tdie_utp_flag || gd->prot_sts.tntc_otp_flag || gd->prot_sts.tntc_utp_flag ||
		gd->prot_sts.isns_ocp_flag || gd->prot_sts.vbus_ovp_flag || gd->prot_sts.vbus_uvp_flag || gd->prot_sts.vbus_dpl_flag ||
		gd->prot_sts.vpwr_ovp_flag || gd->prot_sts.pout_opp_flag || tmp_err_fod)
	{
		tmp_err = 1;
	}
	else
	{
		tmp_err = 0;
	}

	app_reg_buff[tx_fw_version] = TX_FW_VER;
	app_reg_buff[tx_chip_id] = SYS->PID_INFO.BITS.VER;

	tmp_tx_status |= tmp_err ? 0x80 : 0x00;//error
	tmp_tx_status |= (tmp_err_otp & 0x000F) ? 0x40 : 0x00;//otp
	tmp_tx_status |= (tmp_err_ovp & 0x00C0) ? 0x20 : 0x00;//ovp
	tmp_tx_status |= (tmp_err_fod & 0x0030) ? 0x10 : 0x00;//fod
	app_reg_buff[tx_status] = tmp_tx_status;

	app_reg_buff[tx_working_volt_h] = (gd->vbus >> 8) & 0xFF;
	app_reg_buff[tx_working_volt_l] = gd->vbus & 0xFF;

	app_reg_buff[tx_working_current_h] = (gd->isns >> 8) & 0xFF;
	app_reg_buff[tx_working_current_l] = gd->isns & 0xFF;

	app_reg_buff[tx_ntc_tempr_h] = (gd->sys_infos.ntc_temp >> 8) & 0xFF;
	app_reg_buff[tx_ntc_tempr_l] = gd->sys_infos.ntc_temp & 0xFF;

	app_reg_buff[tx_q_value] = (gd->tx_infos.q_fact >> 8) & 0xFF;

	app_reg_buff[tx_lc_freq_value_h] = (gd->tx_infos.f_self >> 8) & 0xFF;
	app_reg_buff[tx_lc_freq_value_l] = gd->tx_infos.f_self & 0xFF;

	app_reg_buff[tx_pwr_volt_h] = (gd->vpwr >> 8) & 0xFF;
	app_reg_buff[tx_pwr_volt_l] = gd->vpwr & 0xFF;

	app_reg_buff[tx_pwr_isns_h] = (gd->isns >> 8) & 0xFF;
	app_reg_buff[tx_pwr_isns_l] = gd->isns & 0xFF;
}

void iic_write_info_sync(void)
{
	iic_master_msg master_adp_msg;
	uint16_t master_adp_volt, master_adp_cur;
	iic_io_ctl io_ctl_msg;
	master_adp_msg.byte = app_reg_buff[master_msg];

	if (1 == master_adp_msg.bits.ready)
	{
		master_adp_volt = app_reg_buff[master_volt_h] << 8 | app_reg_buff[master_volt_l];
		master_adp_cur = app_reg_buff[master_curr_h] << 8 | app_reg_buff[master_curr_l];

		if (master_adp_volt < 5000 || master_adp_cur < 1000)
		{
			//error, ignore it.
		}
		else
		{
			if (master_adp_volt >= 9000)
			{
				master_adp_volt = 9000;
				app_reg_buff[tx_required_volt_l] = 0x28;
				app_reg_buff[tx_required_volt_h] = 0x23;
			}
			else
			{
				master_adp_volt = 5000;
				app_reg_buff[tx_required_volt_l] = 0x88;
				app_reg_buff[tx_required_volt_h] = 0x13;
			}

			//add new adaptor type
		}
	}

	if (io_ctl_msg.bits.pin8)
	{
		fml_nu103x_config(_1030_CFG_DRVH2_TURN_ON_);
	}
	else
	{
		fml_nu103x_config(_1030_CFG_DRVH2_TURN_OFF);
	}

	if (io_ctl_msg.bits.pin19)
	{
		GPA->DOUT.BITS.PIN5 = 1;
	}
	else
	{
		GPA->DOUT.BITS.PIN5 = 0;
	}
}

