#include "regdef.h"
#include "printk.h"
#include "delay.h"
#include "i2cm.h"
#include "nu6801.h"
#include "sw7201.h"
//#define HW_I2CM_


#define _I2C_WRIT_BIT    0
#define _I2C_READ_BIT    1
#define _I2C_RESP_ACK    0
#define _I2C_RESP_NAK    1

#ifdef HW_I2CM_

void hal_i2cm_init(uint32_t bps)
{
	//5*X + 3*(X>>2) = (apb_clk_freq / scl_freq) - 8;
	//where X is the frequency division value that needs to be set
	if (bps != 0)
	{
		I2CM->GEN_CTRL.BITS.BRGEN_CLKDIV = (HCLK / bps - 8) * 4 / 23;
	}
	else
	{
		I2CM->GEN_CTRL.BITS.BRGEN_CLKDIV = HCLK / 100000;
	}
}

static int I2CM_iBusCheckAndSet(void)
{
	__IO uint32_t timeout = 0;

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	do
	{
		if (I2CM->STS_FLAG.BITS.BUS_BUSY == 1 || I2CM->STS_FLAG.BITS.ARB_LOST == 1)
		{
			delay_1us(1);
		}
		else
		{
			break;
		}
	} while (timeout++ < 10);

	if (timeout >= 10)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
		return -1;
	}
	else
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 1;
		return 0;
	}
}

static int I2CM_iWaitProcessDone(void)
{
	__IO uint32_t timeout = 2000; //@36M
	while (I2CM->INT_FLAG.BITS.CMD_DONE_FLAG == 0)
	{
		delay_1us(1);
		if (--timeout == 0)
		{
			return -1;
		}
	}
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	return 0;
}

static int I2CM_iWaitACK(void)
{
	__IO uint32_t timeout = 500; //@36M

	while (I2CM->STS_FLAG.BITS.ACK_DATA == 1)
	{
		delay_1us(1);
		if (--timeout == 0)
		{
			return -1;
		}
	}

	return 0;
}

static int I2CM_iSendStop(void)
{
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_STOP;
	if (I2CM_iWaitProcessDone() < 0)
	{
		return -1;
	}
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;

	return 0;
}

static int I2CM_iSendStartBitAndDevAddr(uint8_t devAddr, uint8_t rw_bit)
{
	I2CM->TXD_DATA.BITS.DATA = devAddr << 1 | rw_bit;
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_STAR | I2CM_PROTOCOL_CMD_SEND;
	if (I2CM_iWaitProcessDone() < 0)
	{
//		printk("!");
		return -1;
	}
	if (I2CM_iWaitACK() < 0)
	{
//		printk("~");
		return -2;
	}
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;

	return 0;
}

//static int hal_i2cm_byte_send(uint8_t byte)
int hal_i2cm_byte_send(uint8_t byte)
{
	I2CM->TXD_DATA.BITS.DATA = byte;
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_SEND;
	if (I2CM_iWaitProcessDone() < 0)
	{
		return -1;
	}
	if (I2CM_iWaitACK() < 0)
	{
		return -2;
	}
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;

	return 0;
}

//static int hal_i2cm_byte_read(uint8_t *byte, uint8_t resp_typ)
int hal_i2cm_byte_read(uint8_t *byte, uint8_t resp_typ)
{
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = (resp_typ == _I2C_RESP_ACK) ? I2CM_PROTOCOL_CMD_READ : (I2CM_PROTOCOL_CMD_READ | I2CM_PROTOCOL_CMD_NACK);
	if (I2CM_iWaitProcessDone() < 0)
	{
		return -1;
	}
	*byte = I2CM->RXD_DATA.BITS.DATA;
	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;

	return 0;
}


int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -10;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -11;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -12;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_READ_BIT) < 0)
		{
			rst = -13;
			break;
		}

		if (hal_i2cm_byte_read(data, _I2C_RESP_NAK) < 0)
		{
			rst = -14;
			break;
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -15;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -20;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -21;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -22;
			break;
		}

		if (hal_i2cm_byte_send(data) < 0)
		{
			rst = -23;
			break;
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -24;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_read_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -30;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -31;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -32;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_READ_BIT) < 0)
		{
			rst = -33;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_read(data++, (len == 0) ? _I2C_RESP_NAK : _I2C_RESP_ACK) < 0)
			{
				rst = -34;
				break;
			}
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -35;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_write_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -40;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -41;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -42;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_send(*data++) < 0)
			{
				rst = -43;
				break;
			}
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -44;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_read_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -110;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -111;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -112;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -113;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_READ_BIT) < 0)
		{
			rst = -114;
			break;
		}

		if (hal_i2cm_byte_read(data, _I2C_RESP_NAK) < 0)
		{
			rst = -115;
			break;
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -116;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_write_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t data)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -120;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -121;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -122;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -123;
			break;
		}

		if (hal_i2cm_byte_send(data) < 0)
		{
			rst = -124;
			break;
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -125;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_read_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -130;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -131;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -132;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -133;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_READ_BIT) < 0)
		{
			rst = -134;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_read(data++, (len == 0) ? _I2C_RESP_NAK : _I2C_RESP_ACK) < 0)
			{
				rst = -135;
				break;
			}
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -136;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

int hal_i2cm_write_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len)
{
	int rst = 0;

	do
	{
		if (I2CM_iBusCheckAndSet() < 0)
		{
			rst = -140;
			break;
		}

		if (I2CM_iSendStartBitAndDevAddr(devAddr, _I2C_WRIT_BIT) < 0)
		{
			rst = -141;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -142;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -143;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_send(*data++) < 0)
			{
				rst = -144;
				break;
			}
		}

		if (I2CM_iSendStop() < 0)
		{
			rst = -145;
			break;
		}
	} while (0);

	I2CM->GEN_CTRL.BITS.PROTOCOL_CMD = I2CM_PROTOCOL_CMD_NULL;
	I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;

	if (rst < 0)
	{
		I2CM->GEN_CTRL.BITS.I2CM_FUNC_EN = 0;
	}

	return rst;
}

#else

//#define _I2CM_SDA_D_IN    (GPA->D_IN.BITS.PIN7)
//#define _I2CM_SDA_DOUT    (GPA->DOUT.BITS.PIN7)
//#define _I2CM_SCL_DOUT    (GPA->DOUT.BITS.PIN6)
//
//#define _SET_I2CM_SDA_IN_PUT()    do { GPA->O_EN.BITS.PIN7 = 0; GPA->I_EN.BITS.PIN7 = 1; } while (0)
//#define _SET_I2CM_SDA_OUTPUT()    do { GPA->I_EN.BITS.PIN7 = 0; GPA->O_EN.BITS.PIN7 = 1; } while (0)



void hal_i2cm_init(uint32_t u32BusClock)
{
	return;
}

//static void hal_i2cm_start(void)
void hal_i2cm_start(void)
{
	_SET_I2CM_SDA_OUTPUT();
	delay_1us(2);

	_I2CM_SDA_DOUT = _PIN_LEVEL_HI;
	_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
	delay_1us(5);

	_I2CM_SDA_DOUT = _PIN_LEVEL_LO; //START: when CLK is high, DATA change form high to low
	delay_1us(5);
	_I2CM_SCL_DOUT = _PIN_LEVEL_LO; //Clamp the IIC bus, ready to send or receive data
	delay_1us(2);
}

//static void hal_i2cm_stop(void)
void hal_i2cm_stop(void)
{
	_SET_I2CM_SDA_OUTPUT();
	delay_1us(2);

	_I2CM_SDA_DOUT = _PIN_LEVEL_LO; //STOP: when CLK is high DATA change form low to high
	delay_1us(5);
	_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
	delay_1us(5);
	_I2CM_SDA_DOUT = _PIN_LEVEL_HI;
	delay_1us(2);
}

//static int hal_i2cm_byte_send(uint8_t byte)
int hal_i2cm_byte_send(uint8_t byte)
{
	int rst = 0;
	int timeout = 0;

	for (int i=0; i<8; i++)
	{
		delay_1us(1);
		_I2CM_SDA_DOUT = (byte & 0x80) >> 7;
		byte <<= 1;
		delay_1us(2);
		_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
		delay_1us(5);
		_I2CM_SCL_DOUT = _PIN_LEVEL_LO;
		delay_1us(2);
	}

//	_I2CM_SDA_DOUT = _PIN_LEVEL_LO;
	_SET_I2CM_SDA_IN_PUT();
	delay_1us(5);
	while (_I2CM_SDA_D_IN == _PIN_LEVEL_HI)
	{
		delay_1us(1);
		if (++timeout > 10) //10us
		{
			break;
		}
	}

	_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
	delay_1us(3);
	if (_I2CM_SDA_D_IN == _PIN_LEVEL_HI)
	{
		rst = -1;
	}
	delay_1us(2);
	_I2CM_SCL_DOUT = _PIN_LEVEL_LO;
	_SET_I2CM_SDA_OUTPUT();
	delay_1us(2);

	return rst;
}

//static int hal_i2cm_byte_read(uint8_t *byte, uint8_t resp_typ)
int hal_i2cm_byte_read(uint8_t *byte, uint8_t resp_typ)
{
	_SET_I2CM_SDA_IN_PUT();
	delay_1us(2);

	for (int i=0; i<8; i++)
	{
		_I2CM_SCL_DOUT = _PIN_LEVEL_LO;
		delay_1us(5);
		_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
		delay_1us(3);
		*byte <<= 1;
		if (_I2CM_SDA_D_IN == _PIN_LEVEL_HI) *byte += 1;
		delay_1us(2);
	}

	_I2CM_SCL_DOUT = _PIN_LEVEL_LO;
	delay_1us(2);
	_SET_I2CM_SDA_OUTPUT();
	_I2CM_SDA_DOUT = (resp_typ == _I2C_RESP_ACK) ? _PIN_LEVEL_LO : _PIN_LEVEL_HI;
	delay_1us(3);

	_I2CM_SCL_DOUT = _PIN_LEVEL_HI;
	delay_1us(5);
	_I2CM_SCL_DOUT = _PIN_LEVEL_LO;
	delay_1us(2);

	return 0;
}

int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -50;
			break;
		}
		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -51;
			break;
		}

		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_READ_BIT) < 0)
		{
			rst = -52;
			break;
		}
		if (hal_i2cm_byte_read(data, _I2C_RESP_NAK) < 0)
		{
			rst = -53;
			break;
		}
	} while (0);

	hal_i2cm_stop();

//	if(devAddr == SW7201_I2C_DEV_ADDR )
//	{
//		printk("SW7201 R [0x%x] = 0x%x\n",regAddr,(uint8_t)*data);
//	}

	return rst;
}

int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();

		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -60;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -61;
			break;
		}

		if (hal_i2cm_byte_send(data) < 0)
		{
			rst = -62;
			break;
		}
	} while (0);

	hal_i2cm_stop();

//	if(devAddr == SW7201_I2C_DEV_ADDR )
//		printk("SW7201 W [0x%x] = 0x%x\n",regAddr,data);
//
//	if(devAddr == SW7201_I2C_DEV_ADDR )
//	{
//		uint8_t read_data;
//		hal_i2cm_read_one_byte(devAddr, regAddr, &read_data);
//	}

	return rst;
}

int hal_i2cm_read_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -70;
			break;
		}
		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -71;
			break;
		}

		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_READ_BIT) < 0)
		{
			rst = -72;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_read(data++, (len == 0) ? _I2C_RESP_NAK : _I2C_RESP_ACK) < 0)
			{
				rst = -73;
				break;
			}
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

int hal_i2cm_write_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();

		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -80;
			break;
		}

		if (hal_i2cm_byte_send(regAddr) < 0)
		{
			rst = -81;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_send(*data++) < 0)
			{
				rst = -82;
				break;
			}
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

int hal_i2cm_read_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -150;
			break;
		}
		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -151;
			break;
		}
		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -152;
			break;
		}

		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_READ_BIT) < 0)
		{
			rst = -153;
			break;
		}
		if (hal_i2cm_byte_read(data, _I2C_RESP_NAK) < 0)
		{
			rst = -154;
			break;
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

int hal_i2cm_write_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t data)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();

		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -160;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -161;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -162;
			break;
		}

		if (hal_i2cm_byte_send(data) < 0)
		{
			rst = -163;
			break;
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

int hal_i2cm_read_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -170;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -171;
			break;
		}
		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -172;
			break;
		}

		hal_i2cm_start();
		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_READ_BIT) < 0)
		{
			rst = -173;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_read(data++, (len == 0) ? _I2C_RESP_NAK : _I2C_RESP_ACK) < 0)
			{
				rst = -174;
				break;
			}
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

int hal_i2cm_write_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len)
{
	int32_t rst = 0;

	do
	{
		hal_i2cm_start();

		if (hal_i2cm_byte_send(devAddr << 1 | _I2C_WRIT_BIT) < 0)
		{
			rst = -180;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 8) & 0xFF) < 0)
		{
			rst = -181;
			break;
		}

		if (hal_i2cm_byte_send((regAddr >> 0) & 0xFF) < 0)
		{
			rst = -182;
			break;
		}

		while (len--)
		{
			if (hal_i2cm_byte_send(*data++) < 0)
			{
				rst = -183;
				break;
			}
		}
	} while (0);

	hal_i2cm_stop();

	return rst;
}

#endif

void __attribute__((isr)) I2CM_IRQHandler(void)
{
	if (I2CM->INT_FLAG.BITS.CMD_DONE_FLAG == 1)
	{
		//add your code
		I2CM->INT_FLAG.BITS.CMD_DONE_FLAG = 1;
	}

	if (I2CM->INT_FLAG.BITS.ARB_LOST_FLAG == 1)
	{
		I2CM->INT_FLAG.BITS.ARB_LOST_FLAG = 1;
	}
}
