#include "algo.h"

void str_concat(char *dest, const char *src)
{
    while (*dest != '\0')
    {
        dest++;
    }
    while ((*dest++ = *src++) != '\0');
}

unsigned long quick_sqrt(const unsigned long data)
{
	unsigned long radicand, remained, divisor, root;

	radicand = data;
	remained = divisor = root = 0;

	for (int i=0; i<16; i++)
	{
		root <<= 1;
		remained = ((remained << 2) + (radicand >> 30));
		radicand <<= 2;
		divisor = (root << 1) + 1;
		if (divisor <= remained)
		{
			remained -= divisor;
			root++;
		}
	}

	return root;
}

unsigned short crc16_ccitt(unsigned char *data, unsigned int len, unsigned short init_crc)
{
	unsigned char i;
	unsigned short crc = init_crc;

	while (len--)
	{
		crc ^= (unsigned short)(*data++) << 8;
		for (i=0; i<8; ++i)
		{
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021;
			}
			else
			{
				crc <<= 1;
			}
		}
	}

	return crc;
}

unsigned long long mlp_32bit_x_32bit(const unsigned long u32_Multiplicand, const unsigned long u32_Multiplier)
{
	unsigned long long result;
	unsigned long u32_HighBit32, u32_LowBit32;
	register unsigned long u32_MidCalc1, u32_MidCalc2, u32_MidCalc3, u32_MidCalc4;
	
	u32_MidCalc1 = (u32_Multiplicand >> 16) * (u32_Multiplier >> 16);
	u32_MidCalc2 = (u32_Multiplicand >> 16) * ((unsigned short)u32_Multiplier);
	u32_MidCalc3 = ((unsigned short)u32_Multiplicand) * (u32_Multiplier >> 16);
	u32_MidCalc4 = ((unsigned short)u32_Multiplicand) * ((unsigned short)u32_Multiplier);
	
	if (u32_MidCalc2 & 0x80000000)
	{
		if (u32_MidCalc3 & 0x80000000)
		{
			u32_MidCalc1 += 0x10000;
			u32_MidCalc2 += u32_MidCalc3;
		}
		else
		{
			u32_MidCalc2 += u32_MidCalc3;
			if (!(u32_MidCalc2 & 0x80000000))
			{
				u32_MidCalc1 += 0x10000;
			}
		}
	}
	else
	{
		u32_MidCalc2 += u32_MidCalc3;
		if (u32_MidCalc3 & 0x80000000)
		{
			if (!(u32_MidCalc2 & 0x80000000))
			{
				u32_MidCalc1 += 0x10000;
			}
		}
	}
	
	u32_HighBit32 = u32_MidCalc1 + (u32_MidCalc2 >> 16);
	
	if (u32_MidCalc4 & 0x80000000)
	{
		if (u32_MidCalc2 & 0x8000)
		{
			u32_HighBit32++;
			u32_LowBit32 = u32_MidCalc4 + (u32_MidCalc2 << 16);
		}
		else
		{
			u32_LowBit32 = u32_MidCalc4 + (u32_MidCalc2 << 16);
			if (!(u32_LowBit32 & 0x80000000))
			{
				u32_HighBit32++;
			}
		}
	}
	else
	{
		u32_LowBit32 = u32_MidCalc4 + (u32_MidCalc2 << 16);
		if (u32_MidCalc2 & 0x8000)
		{
			if (!(u32_LowBit32 & 0x80000000))
			{
				u32_HighBit32++;
			}
		}
	}

	result = (((unsigned long long)u32_HighBit32) << 32) | u32_LowBit32;
	return result;
}
