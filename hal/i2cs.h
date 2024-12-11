#ifndef I2CS_H_
#define I2CS_H_

void hal_i2cs_init(void);
void hal_i2cs_reg_int_cb(void (*func)(void));

#endif /* I2CS_H_ */
