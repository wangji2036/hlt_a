#include "regdef.h"
#include "eadc.h"
#include "nu6801.h"
#include "printk.h"
#include "delay.h"
#include "usb_pd.h"

#if(BUCKBOOST_USED_NU6801 == 1)

bool nu6801_dead_bat = false;

#define BAT_CELL_FULL_VOLT   4200
#define BAT_CELL_EMPTY_VOLT   3000

#define BAT_CELL_NUM 1

void hal_nu6801_buckboost_bat_ivcfg(void);

void hal_nu6801_buckboost_init(void)
{
	uint8_t revision = hal_nu6801_buckboost_get_verision();
	{
		hal_nu6801_buckboost_wake_up();



		hal_nu6801_buckboost_set_busiv(5000,3000);  //5v3a
		hal_nu6801_buckboost_bat_ivcfg();
		hal_nu6801_buckboost_typeca_gate_en(false);
		hal_nu6801_buckboost_typecb_gate_en(false);
		hal_nu6801_buckboost_usb_a_gate_en(false);
		hal_nu6801_buckboost_usba_detect_enable(true);
		hal_nu6801_buckboost_set_mode(BUCKBOOST_SHUTDOWM_MODE);

		/*UNLOCK*/
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2d);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xf9);


		uint16_t vbat = hal_nu6801_buckboost_get_bat_voltage();
		if(vbat < 2500)    //
		{
			nu6801_dead_bat = true;
			hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x68,0x03);
			hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x67,0x20);

			printk("\nvbat= %d debug\n",vbat);
		}


//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x29);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xcb);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0xe2);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x51,0x6a);
//
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0xF8,0xc0);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0xF9,0x0b);
//
		uint8_t read;
		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0x50,&read);
//
//		uint8_t f8,f9;
//		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0xf8,&f8);
//		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0xf9,&f9);
//		printk("UNLOCK6801 =0x%x [F8]=0x%x [F9]=0x%x\n",read,f8,f9);

		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0x61,&read);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x61,read | 0x06);

		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xFF);

		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0x50,&read);
		printk("LOCK6801 =0x%x\n",read);

	}
	printk("nu6801 revision =0x%x\n",revision);
}

void hal_nu6801_buckboost_wake_up(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,read | 0x02);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x01);

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,&read);
	//hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,((read & 0x1C) | 0xC0 | 0x20| 0x03));  //设置nu6801的工作频率
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,((read & 0x1C) | 0x00 | 0x00| 0x00));  //设置nu6801的工作频率
}

void hal_nu6801_buckboost_set_cv(void)
{

}

void hal_nu6801_buckboost_bat_ivcfg(void)
{
	//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBAT_CTRL,0x09);

//	ITRCKLE=400mA
//	ITERM: VBUS=5V/200mA
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,0x17);  //
}


void hal_nu6801_buckboost_typeca_dischg(bool en) //vac2
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x20);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x20));
}
void hal_nu6801_buckboost_typecb_dischg(bool en) //vac3
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x10);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x10));
}
void hal_nu6801_buckboost_usb_a_dischg(bool en)  //vac1
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x40);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x40));
}

void hal_nu6801_buckboost_vbus_dischg(bool en)
{

}

uint8_t hal_nu6801_buckboost_get_protect(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_FAULT_FLAG,&read);
	return read;
}

bool hal_nu6801_buckboost_usba_detect_enable(bool en)
{
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AC_DET_CTRL,0x03);
	return en;
}

bool hal_nu6801_buckboost_get_usba_state(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_FLAG,&read);

	if(read & 0x01)
	{
		return true;
	}
	return false;
}

void hal_nu6801_buckboost_set_mode(enum buckboost_mode woke_mode)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read & 0xF3);
	if(woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		read = (read & 0xF3) | 0x08;
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,(0x05 << 5));
	}
	else if(woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		read = (read & 0xF3);
		uint16_t vbus = (4400 -4400) / 20;
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x08)|(vbus >> 8));
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,vbus & 0xFF);
	}
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read);
	if(woke_mode != BUCKBOOST_SHUTDOWM_MODE) hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read | 0x04);
}

void hal_nu6801_buckboost_set_busiv(uint16_t vbus,uint16_t ibus)
{
	//printk("%s= %d\n",__func__,vbus);
	if(vbus < 4400 || vbus > 19000) return;
	vbus = (vbus - 4400) / 20;
	if(g_buckboost.woke_mode != BUCKBOOST_DISCHG_MODE) return;
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x08)|(vbus >> 8));
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,vbus & 0xFF);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,(0x05 << 5));

	if(ibus < 150) ibus = 150;
	ibus = (ibus - 150) / 50;
	printk("ibus_limit = %d\n",ibus);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBUS_SET,ibus);

	if(g_usb_pd_s.is_in_pps && g_usb_pd_s.explicit_contract)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x00);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x02);
}

void hal_nu6801_buckboost_typeca_gate_en(bool en)
{
	//printk("%s :%d\n",__func__,en);
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x02);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x02));
}

void hal_nu6801_buckboost_typecb_gate_en(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x01);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x01));
}

void hal_nu6801_buckboost_usb_a_gate_en(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x04);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x04));
}


void hal_nu6801_buckboost_charge_ibus_limit(uint16_t ibus_limit)
{
	if(ibus_limit < 150) ibus_limit = 150;
	ibus_limit = (ibus_limit - 150) / 50;
	printk("ibus_limit = %d\n",ibus_limit);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBUS_SET,ibus_limit);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x02);
}

void hal_nu6801_buckboost_charge_ibat_limit(uint16_t ibat_limit)
{
	uint16_t vbus = (4400 -4400) / 20;
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x08)|(vbus >> 8));
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,vbus & 0xFF);

	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,&read);
	if(ibat_limit  == 0) read = (read & 0x1F) | (0x07 << 5);
	else if(ibat_limit  <= 2000) read = (read & 0x1F) | (0x00 << 5);
	else if(ibat_limit  <= 3000) read = (read & 0x1F) | (0x01 << 5);
	else if(ibat_limit  <= 4000) read = (read & 0x1F) | (0x02 << 5);
	else if(ibat_limit  <= 6000) read = (read & 0x1F) | (0x03 << 5);
	else if(ibat_limit  <= 8000) read = (read & 0x1F) | (0x04 << 5);
	else if(ibat_limit  <= 10000) read = (read & 0x1F) | (0x05 << 5);
	else if(ibat_limit  <= 12000) read = (read & 0x1F) | (0x06 << 5);
	else read = (read & 0x1F) | (0x07 << 5);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,read);
}

uint16_t nu6801_vref = 1200;

int16_t hal_nu6801_buckboost_get_bus_current(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x05);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 25 / nu6801_vref;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		return vbat;
	else
		return -vbat;
}

uint8_t hal_nu6801_buckboost_is_ibus_loop(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MAIN_STAT,&read);

	if((read & 0x03) == 0x02) return true;
	return false;
}

int16_t hal_nu6801_buckboost_get_bat_current(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x07);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		return vbat;
	else
		return -vbat;
}


uint16_t hal_nu6801_buckboost_typeca_vbus_present(void)//vac2
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x02);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;

	printk("typeca = %d adc_vac2 = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}

uint16_t hal_nu6801_buckboost_typecb_vbus_present(void)//vac3
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x03);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);

	uint32_t vbat = row* 120  * 100 / nu6801_vref;

	//printk("typecb = %d\n",vbat);
	printk("typecb = %d adc_vac2 = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}

uint16_t hal_nu6801_buckboost_get_iac1(void)//iac1
{
	uint8_t read;

	if(g_buckboost.set_usb_a_gate_en == false) return 0;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x08);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 30 * 15  / nu6801_vref;

	printk("adc_ivac1 = %d row = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}

uint16_t hal_nu6801_buckboost_get_bat_voltage(void)
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x0F);
	delay_1us(300);
	nu6801_vref =  hal_badc_meas(_BADC_CH_PD3_ADC9);

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x00);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);

	uint32_t vbat = row* 120  * 25 / nu6801_vref;

	//printk("vbat = %d adc_vbat = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}
uint16_t hal_nu6801_buckboost_get_bus_voltage(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0xE0) | 0x010 | 0x04);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;
	return vbat;
}

uint16_t hal_nu6801_buckboost_get_bat_temperature(void)
{
	return 25;// in mOhm,
}



uint8_t hal_nu6801_buckboost_get_verision(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_REVISION,&read);
	return read;
}

void hal_nu6801_buckboost_write_reset_check(void)
{

}

void hal_nu6801_buckboost_dis_indetb(void)
{
}

void hal_nu6801_buckboost_charge_vbus_uv(uint16_t vbus_uv)
{

}

void hal_nu6801_buckboost_charge_target_volt(uint16_t volt)
{

}

void hal_nu6801_buckboost_charge_set_trickle_volt(uint16_t volt)
{

}

void hal_nu6801_buckboost_discharge_set_bat_uv_volt(uint16_t volt)
{

}

uint8_t hal_nu6801_buckboost_get_charge_flag(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_CHG_FLAG,&read);
	return read & 0x03;
}


#endif


