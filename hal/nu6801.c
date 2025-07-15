#include "regdef.h"
#include "eadc.h"
#include "nu6801.h"
#include "printk.h"
#include "delay.h"
#include "pdlib.h"
#include "config.h"

#if(BUCKBOOST_USED_NU6801 == 1)
uint8_t nu6801_adc_chennel;
bool nu6801_dead_bat = false;

#define BAT_CELL_FULL_VOLT   4200
#define BAT_CELL_EMPTY_VOLT   3000

#define BAT_CELL_NUM 1

void hal_nu6801_buckboost_bat_ivcfg(void);

void hal_nu6801_buckboost_enter_force_trickle(bool enter)
{
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2d);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xf9);

	if(enter)
	{
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x67,0x03);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x68,0x20);

		printk("----force trick \n");
	}
	else
	{

		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x68,0xA0);

		printk("----nu6801_dead_bat-- release \n");
	}

	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xFF);

}

void hal_nu6801_buckboost_init(void)
{
	uint8_t revision = hal_nu6801_buckboost_get_verision();
	g_buckboost.ibat_level = 0;

	{
		hal_nu6801_buckboost_wake_up();

		hal_nu6801_buckboost_set_busiv(5000,3300);  //5v3a
		hal_nu6801_buckboost_bat_ivcfg();
		hal_nu6801_buckboost_set_cv(BATTERY_CV_VALUE);
		hal_nu6801_buckboost_typeca_gate_en(false);
		hal_nu6801_buckboost_typecb_gate_en(false);
		hal_nu6801_buckboost_usb_a_gate_en(false);
		hal_nu6801_buckboost_set_mode(BUCKBOOST_SHUTDOWM_MODE);

		/*UNLOCK*/
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2d);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xf9);


		uint16_t vbat = hal_nu6801_buckboost_get_bat_voltage();
		if(vbat < 2500)    //
		{
			printk("\nvbat= %d debug\n",vbat);
			nu6801_dead_bat = true;
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

		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0x6D,&read);
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x6D,read & ~0x02);

		printk("LOCK6801 =0x%x\n",read);

	}
	printk("nu6801 revision =0x%x\n",revision);
}

void hal_nu6801_buckboost_wake_up(void)
{
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,0x10); //RESET
	delay_1ms(2);
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,read | 0x02);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_MASK,0x01);

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,&read);
	//hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,((read & 0x1C) | 0xC0 | 0x20| 0x03));  //设置nu6801的工作频率
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,((read & 0x1C) | 0x00 | 0x00| 0x01));  //设置nu6801的工作频率
}

void hal_nu6801_buckboost_set_cv(uint16_t volt)
{
	uint8_t read;

	if(volt < 4100 || volt > 4500) return;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VBAT_CTRL,&read);

	read = read & (~0x07);

	uint8_t value = 0;

	if(volt <= 4100) value = 0;
	else
	{
		value = (volt - 4200) / 50 + 1;
	}

	read |= value;

	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBAT_CTRL,read);
}

void hal_nu6801_buckboost_bat_ivcfg(void)
{
	//
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBAT_CTRL,0x19);

//	ITRCKLE=400mA
//	ITERM: VBUS=5V/200mA
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,0x07);  //
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
#ifdef POWERBANK_BUCK_EVK_V02
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x40);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x40));
#else
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x10);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x10));
#endif
}
void hal_nu6801_buckboost_usb_a_dischg(bool en)  //vac1
{
#ifndef POWERBANK_BUCK_EVK_V02
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x40);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x40));
#endif
}

void hal_nu6801_buckboost_vbus_dischg(bool en)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x08);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x08));
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
	printk("%s= %d\n",__func__,woke_mode);
	if(woke_mode == BUCKBOOST_DISCHG_MODE)
	{
		read = (read & 0xF3) | 0x08;
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,(CONFIG_DISCHG_IBAT_LIMIT << 5));
	}
	else// if(woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		read = (read & 0xF3);
		uint16_t vbus = (4400 -4400) / 20;
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x08)|(vbus >> 8));
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,vbus & 0xFF);
	}
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read);
	if(woke_mode != BUCKBOOST_SHUTDOWM_MODE) hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read | 0x04);
}

void hal_nu6801_disable_bubo(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_CTRL,read & (~0x04));
}

void hal_nu6801_buckboost_set_busiv(uint16_t vbus,uint16_t ibus)
{
	//printk("%s= %d\n",__func__,vbus);
	if(vbus < 4400 || vbus > 19000) return;
	vbus = (vbus - 4400) / 20;
	if(g_buckboost.woke_mode != BUCKBOOST_DISCHG_MODE) return;
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x1C)|(vbus >> 8));
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_L,vbus & 0xFF);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBAT_CTRL,(CONFIG_DISCHG_IBAT_LIMIT << 5));

	if(ibus < 150) ibus = 150;
	ibus = (ibus - 150) / 50;
	printk("ibus_limit = %d\n",ibus);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBUS_SET,ibus);

	if(pdlib_is_pps_source())
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x00);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x00);
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

#ifdef POWERBANK_BUCK_EVK_V02
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x04);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x04));
#else
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,&read);
	if(en)
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read | 0x01);
	else
		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VAC_DRV_CTRL,read & (~0x01));
#endif

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

uint8_t hal_nu6801_buckboost_get_main_stat(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MAIN_STAT,&read);
	return read;
}

void hal_nu6801_buckboost_charge_ibus_limit(uint16_t ibus_limit)
{
	if(ibus_limit < 150) ibus_limit = 150;
	ibus_limit = (ibus_limit - 150) / 50;
	printk("ibus_limit = %d\n",ibus_limit);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IBUS_SET,ibus_limit);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_IR_COMP,0x00);
}

void hal_nu6801_buckboost_charge_ibat_limit(uint16_t ibat_limit)
{
	uint16_t vbus = (4400 -4400) / 20;
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, (0x1C)|(vbus >> 8));
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
uint16_t ntc2_v = 0;
//uint16_t ntc2_v_switch = 0;
uint8_t ntc2_level = 1;

uint16_t ntc1_v = 0;
//uint16_t ntc1_v_switch = 0;
uint8_t ntc1_level = 1;

//void hal_nu6801_buckboost_switch_isrc(void)
//{
//	uint8_t read = 0;
//	uint8_t need_switch = 0;
//
//	if(ntc_v_switch < 200)
//	{
//		need_switch = 1;
//	}
//	else if(ntc_v_switch > 2600)
//	{
//		need_switch = 2;
//	}
//
//	if(need_switch)
//	{
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x65);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x37);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0x2d);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xf9);
//
//		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,0x63,&read);
//		if(need_switch == 1)  read |= 0x04;
//		else read &= ~0x04;
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x63, read);
//		hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,0x50,0xFF);
//		printk("switch Reg = 0x%x\n",read);
//
//		need_switch = 0;
//	}
//}

void hal_nu6801_buckboost_set_adc_channel(uint8_t channel)
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	nu6801_adc_chennel = channel;

	switch(channel)
	{
		case NU6801_ADC_VBAT:
			read = (read & 0x80) | 0x010 | 0x00 | 0x80;
			break;
		case NU6801_ADC_IBAT:
			printk("\r\n LEVEL:%d \r\n",g_buckboost.ibat_level);
			if(g_buckboost.ibat_level == 0)
			{
			  read = (read & 0x80) | 0x010 | 0x07; // 1/125k
			}
			else //level == 1
			{
			  read = (read & 0x80) | 0x010 | 0x07 | 0x20; // 1/50k
			}
			break;
		case NU6801_ADC_VBUS:
			read = (read & 0x80) | 0x010 | 0x04;
			break;
		case NU6801_ADC_IBUS:
			read = (read & 0x80) | 0x010 | 0x05;
			break;
		case NU6801_ADC_IAC1:
			read = (read & 0x80) | 0x010 | 0x08;
			break;
		case NU6801_ADC_IAC2:
			read = (read & 0x00) | 0x010 | 0x09;
			break;
		case NU6801_ADC_RNTC1:
			if(ntc1_v < 1200)
			{
				if(ntc1_level > 1)  ntc1_level--;
			}
			else if(ntc1_v > 2400)
			{
				ntc1_level ++;
				if(ntc1_level >= 2) ntc1_level = 2;
			}
//			else
			if(ntc1_level == 1)
			{
				read = (read & 0x00) | 0x010 | 0x0d | 0x20;
			}
			else
			{
				read = (read & 0x00) | 0x010 | 0x0d;
			}
			//printk("NTC REG_AMUX_CTRL = 0x%x\n",read);
			break;
		case NU6801_ADC_RNTC2:

			if(ntc2_v < 1200)
			{
				if(ntc2_level > 1)  ntc2_level--;
			}
			else if(ntc2_v > 2400)
			{
				ntc2_level ++;
				if(ntc2_level >= 2) ntc2_level = 2;
			}
//			else
			if(ntc2_level == 1)
			{
				read = (read & 0x00) | 0x010 | 0x0a | 0x20 | 0x80;
			}
			else
			{
				read = (read & 0x00) | 0x010 | 0x0a | 0x80;
			}
			//printk("NTC2 ntc2_v = %d ntc2_level = %d\n",ntc2_v,ntc2_level);
			break;
		case NU6801_ADC_VREF:
			read = (read & 0x80) | 0x010 | 0x0F;
			break;
	}

	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, read);
	osal_start_timerEx(BUCKBOOST_ADC_TIMER, 2, 0, BUCKBOOST_TASK, BUCKBOOST_EVT_ADC_PERIOD);
}

int16_t hal_nu6801_buckboost_get_bus_current(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x05);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 25 / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_IBUS;
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
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x07);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_IBAT;
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
		return vbat;
	else
		return -vbat;
}


uint16_t hal_nu6801_buckboost_typeca_vbus_present(void)//vac2
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x02);
	delay_1us(300);
	nu6801_adc_chennel = NU6801_ADC_OTHER;
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;

	printk("typeca = %d adc_vac2 = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}

uint16_t hal_nu6801_buckboost_typecb_vbus_present(void)//vac3
{

#ifdef POWERBANK_BUCK_EVK_V02
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x01);
	delay_1us(300);
	nu6801_adc_chennel = NU6801_ADC_OTHER;
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);

	uint32_t vbat = row* 120  * 100 / nu6801_vref;

	//printk("typecb = %d\n",vbat);

	printk("typecb = %d adc_vac1 = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
#else
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x03);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);

	uint32_t vbat = row* 120  * 100 / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_OTHER;
	//printk("typecb = %d\n",vbat);
	printk("typecb = %d adc_vac1 = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
#endif

}

uint16_t hal_nu6801_buckboost_get_iac1(void)//iac1
{
	uint8_t read;

	if(g_buckboost.set_usb_a_gate_en == false) return 0;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x08);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 30 * 15  / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_IAC1;
	printk("adc_ivac1 = %d row = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}

uint16_t hal_nu6801_buckboost_get_bat_voltage(void)
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x0F);
	delay_1us(300);
	nu6801_vref =  hal_badc_meas(_BADC_CH_PD3_ADC9);

	if(nu6801_vref < 1000)
	{
		uint8_t read_0x10,read_0x11,read_0x06,read_0x00;
		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read_0x11);
		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_MISC_CTRL,&read_0x10);
		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_BUBO_FAULT_FLAG,&read_0x06);
		hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_INT_FLAG,&read_0x00);
		printk("adc_err [0x00]=0x%x [0x06]=0x%x [0x10]=0x%x [0x11]=0x%x\n",read_0x00,read_0x06,read_0x10,read_0x11);
	}

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x00);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);

	uint32_t vbat = row* 120  * 25 / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_VBAT;
	//printk("vbat = %d adc_vbat = %d nu6801_vref = %d\n",vbat,row,nu6801_vref);
	return vbat;
}
uint16_t hal_nu6801_buckboost_get_bus_voltage(void)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x04);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t vbat = row* 120  * 100 / nu6801_vref;
	nu6801_adc_chennel = NU6801_ADC_VBUS;
	return vbat;
}

uint16_t hal_nu6801_buckboost_get_bat_temperature(void)  //return 0.1K/bit
{
	uint8_t read;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL,&read);
	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_AMUX_CTRL, (read & 0x80) | 0x010 | 0x0d | 0x00);
	delay_1us(300);
	uint32_t row = (uint32_t) hal_badc_meas(_BADC_CH_PD3_ADC9);
	uint32_t adc_value = row* 120  * 10 / nu6801_vref;

	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_TEMP_STAT,&read);
	if(read & 0x04) adc_value = adc_value / 22; //220uA
	else adc_value = adc_value / 2; //220uA
	nu6801_adc_chennel = NU6801_ADC_RNTC1;
	return adc_value;
}

void hal_nu6801_buckboost_set_ovp(uint16_t set_volt)
{
	uint8_t read;
	hal_i2cm_read_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H,&read);
	if(set_volt < 5500)
		read = (read & 0x03) | (0x00 <<2);   //ovp 6.5v
	else if(set_volt < 12500)
		read = (read & 0x03) | (0x02 <<2);   //ovp 13.5v
	else
		read = (read & 0x03) | (0x07 <<2);   //ovp 19.8v

	//if(g_buckboost.woke_mode != BUCKBOOST_DISCHG_MODE) read = (read & 0x03) | (0x02 <<2);  // 充电设置 13.5V

	hal_i2cm_wirte_one_byte(NU6801_I2C_DEV_ADDR,REG_VBUS_SET_H, read);
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

void hal_nu6801_deadbat_patch(void)
{
	#define ABS(x,y)  x>y? x-y:y-x
	static uint16_t last_vbat;



	if(
			nu6801_dead_bat  && last_vbat > 2800
			&&(
					g_buckboost.adc_vbat > 3500 ||  ((ABS(g_buckboost.adc_vbat , last_vbat) >300) && g_buckboost.adc_vbat < last_vbat)
					)
			)
	{
		printk("vbat=%d  last_vbat=%d \n",g_buckboost.adc_vbat,last_vbat);
		hal_nu6801_buckboost_charge_ibus_limit(150);
		hal_nu6801_buckboost_enter_force_trickle(false);
		nu6801_dead_bat = false;
	}
	last_vbat = g_buckboost.adc_vbat;

//	static uint8_t delay_cnt = 0;
//	if(g_buckboost.adc_ibus < 200  && g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
//	{
//		uint8_t ret = hal_nu6801_buckboost_get_main_stat();
//		if(ret & 0x08 || !(ret & 0x03))
//		{
//			printk("main = 0x%x \n",ret);
//			delay_cnt++;
//			if(delay_cnt >= 100)
//			{
//				g_buckboost.vsnkdisconnect_flag = 1;
//				delay_cnt = 0;
//			}
//
//		}
//		else
//			delay_cnt = 0;
//	}


}

void hal_nu6801_get_charge_state(void)
{


	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
	{
		uint8_t main_stat = hal_nu6801_buckboost_get_main_stat();
		if((main_stat & 0xF0 ) != 0x40)
			g_buckboost.charging_stat = 0;
		else
			g_buckboost.charging_stat = 1;

		//printk("[MianStat]=0x%x charing=%d\n",main_stat,g_buckboost.charging_stat);
	}

}


#endif


