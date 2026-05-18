#ifndef I2CM_H_
#define I2CM_H_

void hal_i2cm_init(uint32_t u32BusClock);

int hal_i2cm_read_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t *data);
int hal_i2cm_wirte_one_byte(uint8_t devAddr, uint8_t regAddr, uint8_t data);
int hal_i2cm_read_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);
int hal_i2cm_write_multi_bytes(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint8_t len);

int hal_i2cm_read_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data);
int hal_i2cm_write_one_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t data);
int hal_i2cm_read_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len);
int hal_i2cm_write_multi_byte_16bit(uint8_t devAddr, uint16_t regAddr, uint8_t *data, uint8_t len);

//These sub functions are designed for fm1210 se_ic
void hal_i2cm_start(void);
void hal_i2cm_stop(void);
int hal_i2cm_byte_send(uint8_t byte);
int hal_i2cm_byte_read(uint8_t *byte, uint8_t resp_typ); //0-ack 1-nak
#define _I2CM_SDA_PORT GPA
#define _I2CM_SDA_PINx PIN7
#define _I2CM_SCL_PORT GPA
#define _I2CM_SCL_PINx PIN6

#define _PIN_LEVEL_HI (1)
#define _PIN_LEVEL_LO (0)

#define _I2CM_SDA_D_IN (_I2CM_SDA_PORT->D_IN.BITS._I2CM_SDA_PINx)
#define _I2CM_SDA_DOUT (_I2CM_SDA_PORT->DOUT.BITS._I2CM_SDA_PINx)
#define _I2CM_SCL_D_IN (_I2CM_SCL_PORT->D_IN.BITS._I2CM_SCL_PINx)
#define _I2CM_SCL_DOUT (_I2CM_SCL_PORT->DOUT.BITS._I2CM_SCL_PINx)

#define _SET_I2CM_SDA_IN_PUT()                        \
	do                                                \
	{                                                 \
		VIC_vModuleDisable();                         \
		_I2CM_SDA_PORT->O_EN.BITS._I2CM_SDA_PINx = 0; \
		_I2CM_SDA_PORT->I_EN.BITS._I2CM_SDA_PINx = 1; \
		VIC_vModuleEnable();                          \
	} while (0)
#define _SET_I2CM_SDA_OUTPUT()                        \
	do                                                \
	{                                                 \
		VIC_vModuleDisable();                         \
		_I2CM_SDA_PORT->I_EN.BITS._I2CM_SDA_PINx = 0; \
		_I2CM_SDA_PORT->O_EN.BITS._I2CM_SDA_PINx = 1; \
		VIC_vModuleEnable();                          \
	} while (0)
#define _SET_I2CM_SCL_OUTPUT()                        \
	do                                                \
	{                                                 \
		VIC_vModuleDisable();                         \
		_I2CM_SCL_PORT->I_EN.BITS._I2CM_SCL_PINx = 0; \
		_I2CM_SCL_PORT->O_EN.BITS._I2CM_SCL_PINx = 1; \
		VIC_vModuleEnable();                          \
	} while (0)
#define _SET_I2CM_SCL_IN_PUT()                        \
	do                                                \
	{                                                 \
		VIC_vModuleDisable();                         \
		_I2CM_SCL_PORT->O_EN.BITS._I2CM_SCL_PINx = 0; \
		_I2CM_SCL_PORT->I_EN.BITS._I2CM_SCL_PINx = 1; \
		VIC_vModuleEnable();                          \
	} while (0)

#endif /* I2CM_H_ */
