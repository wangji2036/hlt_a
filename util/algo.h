#ifndef ALGO_H_
#define ALGO_H_

void str_concat(char *dest, const char *src);
unsigned long quick_sqrt(const unsigned long data);
unsigned short crc16_ccitt(unsigned char *data, unsigned int len, unsigned short init_crc);
unsigned long long mlp_32bit_x_32bit(const unsigned long u32_Multiplicand, const unsigned long u32_Multiplier);

#endif /* ALGO_H_ */
