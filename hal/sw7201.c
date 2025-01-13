#include "regdef.h"
#include "eadc.h"
#include "sw7201.h"
#include "printk.h"

#if(BUCKBOOST_USED_SW7201 == 1)
#define BAT_CELL_FULL_VOLT   4200
#define BAT_CELL_EMPTY_VOLT   3000

#define BAT_CELL_NUM 2

void hal_sw7201_buckboost_init(void)
{
	//if(hal_sw7201_buckboost_get_verision() == 0x11)

	uint8_t revision = hal_sw7201_buckboost_get_verision();
	{
		hal_sw7201_buckboost_dis_indetb();
		hal_sw7201_buckboost_charge_target_volt(BAT_CELL_FULL_VOLT*BAT_CELL_NUM);
		hal_sw7201_buckboost_discharge_set_bat_uv_volt(BAT_CELL_EMPTY_VOLT*BAT_CELL_NUM);

		hal_sw7201_buckboost_set_busiv(5000,3000);  //5v3a
		hal_sw7201_buckboost_write_reset_check();
		hal_sw7201_buckboost_typeca_gate_en(false);
		hal_sw7201_buckboost_typecb_gate_en(false);
		hal_sw7201_buckboost_usb_a_gate_en(false);
		hal_sw7201_buckboost_charge_vbus_uv(4000);
		hal_sw7201_buckboost_charge_ibus_limit(1000);
		hal_sw7201_buckboost_charge_ibat_limit(500);
		hal_sw7201_buckboost_charge_set_trickle_volt(3000);
		//hal_sw7201_buckboost_a2_detect_enable(true);
		hal_sw7201_buckboost_set_mode(BUCKBOOST_SHUTDOWM_MODE);
		hal_sw7201_buckboost_set_cv();


		//return;
	}
	printk("sw7201 revision =0x%x\n",revision);
}

void hal_sw7201_buckboost_set_cv(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_RESEVERD,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_RESEVERD,0x08 | read);
}

void hal_sw7201_buckboost_typeca_dischg(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read | 0x02);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x02));
}
void hal_sw7201_buckboost_typecb_dischg(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read | 0x04);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x04));
}
void hal_sw7201_buckboost_usb_a_dischg(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read | 0x01);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x01));
}

void hal_sw7201_buckboost_vbus_dischg(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read | 0x08);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,read & (~0x08));
}

uint8_t hal_sw7201_buckboost_get_protect(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event2,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event2,read);
	return read;
}

bool hal_sw7201_buckboost_a2_detect_enable(bool en)
{
	//hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_discharge_Control,0x00);
	//hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event1,0x02);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x00);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Indt_Control,0x01);
	return en;
}

bool hal_sw7201_buckboost_get_a2_state(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event1,&read);
	//printk("REG_IRQ_Event1 =0x%x\n",read);
	if(read & 0x02)
	{
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IRQ_Event1,0x02);
		return true;
	}
	return false;
}

void hal_sw7201_buckboost_set_mode(enum buckboost_mode woke_mode)
{
	uint8_t write_data = 0;
	if(woke_mode == BUCKBOOST_DISCHG_MODE)
		write_data = 0x01;
	else if(woke_mode == BUCKBOOST_CHAGER_MODE)
		write_data = 0x10;

	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Mode_Control,write_data);
}

void hal_sw7201_buckboost_set_busiv(uint16_t vbus,uint16_t ibus)
{
	//printk("%s= %d\n",__func__,vbus);
	if(vbus < 3000 || vbus > 22000) return;
	vbus = (vbus -3000) / 10;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Vbus_Vol_High,vbus >> 3);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Vbus_Vol_Low,vbus & 0x7);

	if(ibus < 500) ibus = 500;
	ibus = (ibus - 500) / 50;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Ibus_Limit,ibus);

	//hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Ibus_Limit,ibus);
}

void hal_sw7201_buckboost_typeca_gate_en(bool en)
{
	//printk("%s :%d\n",__func__,en);
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read | 0x02);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x02));
}

void hal_sw7201_buckboost_usb_a_gate_en(bool en)
//void hal_sw7201_buckboost_typecb_gate_en(bool en)
{
	//printk("%s :%d\n",__func__,en);
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read | 0x01);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x01));
}

void hal_sw7201_buckboost_typecb_gate_en(bool en)
//void hal_sw7201_buckboost_usb_a_gate_en(bool en)
{
	//printk("%s :%d\n",__func__,en);
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,&read);
	if(en)
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read | 0x04);
	else
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Powerpath_Control,read & (~0x04));
}

void hal_sw7201_buckboost_charge_ibus_limit(uint16_t ibus_limit)
{
	if(ibus_limit < 500) ibus_limit = 500;
	ibus_limit = (ibus_limit - 500) / 50;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Charger_Ibus_Limit,ibus_limit);
}

void hal_sw7201_buckboost_charge_ibat_limit(uint16_t ibat_limit)
{
	if(ibat_limit < 100) ibat_limit = 100;
	ibat_limit = (ibat_limit - 100) / 100;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Charger_Ibat_Limit,ibat_limit);
}

int16_t hal_sw7201_buckboost_get_bus_current(void)
{
	uint16_t ibus = 0;
	uint8_t read = 0;


	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,5);
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);
		ibus = read << 4;
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
		ibus |= read & 0x0F;
		return ibus *5;
	}
	else
	{
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,7);
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);
		ibus = read << 4;
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
		ibus |= read & 0x0F;
		return -ibus *5;
	}
}

int16_t hal_sw7201_buckboost_get_bat_current(void)
{
	uint16_t ibus = 0;
	uint8_t read = 0;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,4);
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);
		ibus = read << 4;
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
		ibus |= read & 0x0F;
		return ibus *5;
	}
	else
	{
		hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,6);
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);
		ibus = read << 4;
		hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
		ibus |= read & 0x0F;
		return -ibus *5;
	}
}

uint16_t hal_sw7201_buckboost_get_bat_voltage(void)
{
	uint16_t vbat = 0;
	uint8_t read = 0;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,0);

	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);

	vbat = read << 4;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
	vbat |= read & 0x0F;
	return vbat *75 / 10;
}
uint16_t hal_sw7201_buckboost_get_bus_voltage(void)
{
	uint16_t vbus = 0;
	uint8_t read = 0;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,1);

	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);

	vbus = read << 4;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
	vbus |= read & 0x0F;
	return vbus *75 / 10;
}

uint16_t hal_sw7201_buckboost_get_bat_temperature(void)
{
	uint16_t ntc = 0;
	uint8_t read = 0;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Type,9);
	//	delay_1us(50);
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_High,&read);

	ntc = read << 4;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_ADC_Data_Low,&read);
	ntc |= read & 0x0F;
	return ntc*11/400;// in mOhm,

}



uint8_t hal_sw7201_buckboost_get_verision(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Version_info,&read);
	return read;
}

void hal_sw7201_buckboost_write_reset_check(void)
{
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_IC_Reset_Check,0x01);
}

void hal_sw7201_buckboost_dis_indetb(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Setting3,&read);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Discharge_Setting3,read | 0x02);


}

void hal_sw7201_buckboost_charge_vbus_uv(uint16_t vbus_uv)
{
	if(vbus_uv < 4000) vbus_uv = 4000;
	vbus_uv = (vbus_uv - 4000) / 100;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Charger_HoldVol,vbus_uv);
}

void hal_sw7201_buckboost_charge_target_volt(uint16_t volt)
{
	if(volt < 3000 || volt > 19200) return;
	volt = (volt -3000) / 10;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Charger_VbatVol_High,volt >> 3);
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Charger_VbatVol_Low,volt & 0x7);
}

void hal_sw7201_buckboost_charge_set_trickle_volt(uint16_t volt)
{
	if(volt < 2500) volt = 2500;
	volt = (volt -2500) / 100;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Trickle_Vol,volt);
}

void hal_sw7201_buckboost_discharge_set_bat_uv_volt(uint16_t volt)
{
	if(volt < 3000) volt = 3000;
	volt = (volt -2700) /100;
	hal_i2cm_wirte_one_byte(SW7201_I2C_DEV_ADDR,REG_Vin_Uvlo,volt);
}

#endif




