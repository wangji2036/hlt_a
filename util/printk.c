#include "printk.h"

void __attribute__((weak)) retarget_fputc(unsigned char ch);

typedef char             *VAR_LIST;
#define _INTSIZEOF(X)    ((sizeof(X) + sizeof(int) - 1) & ~(sizeof(int) - 1))
#define VAR_START(AP,V)  (AP = (VAR_LIST)&V + _INTSIZEOF(V))
#define VAR_ARG(AP,T)    (*(T *)((AP += _INTSIZEOF(T)) - _INTSIZEOF(T)))
#define VAR_END(AP)      (AP = (VAR_LIST)0)

static const char *digits_lower = "0123456789abcdef";
static const char *digits_upper = "0123456789ABCDEF";

static void put_char(unsigned char ch)
{
	retarget_fputc(ch);
}

static void put_str(const char *str)
{
	while (*str != '\0') put_char(*str++);
}

static void put_num(signed long num, unsigned char base, unsigned char lead, unsigned char max_width, unsigned char type)
{
	unsigned long tmp = 0;
	unsigned char char_buf[40], *str = char_buf + sizeof(char_buf);
	unsigned char cnt = 0, i = 0;

	if (base < 2 || base > 16) return;

	*--str = '\0';
	tmp = (base == 10 && num < 0) ? 0 - num : num;

	do {
		*--str = (type == 0) ? digits_lower[tmp % base] : digits_upper[tmp % base];
		cnt++;
	} while ((tmp /= base) != 0);

	if (max_width != 0 && cnt < max_width)
	{
		for (i = max_width - cnt; i != 0; i--)
		{
			*--str = lead;
		}
	}

	if (base == 10 && num < 0) *--str = '-';

	put_str((const char *)str);
}

static void my_printf(const char *fmt, VAR_LIST arg_list)
{
	unsigned char lead, max_width;

	for (; *fmt != '\0'; fmt++)
	{
		if (*fmt != '%')
		{
			put_char(*fmt);
			continue;
		}

		lead = ' ';
		max_width = 0;
		fmt++;

		if (*fmt == '0')
		{
			lead = '0';
			fmt++;
		}

		while (*fmt >= '0' && *fmt <= '9')
		{
			max_width *= 10;
			max_width += (*fmt - '0');
			fmt++;
		}

		switch (*fmt)
		{
			case 'c': put_char(VAR_ARG(arg_list, int)); break;
			case 's': put_str(VAR_ARG(arg_list, char*)); break;
			case 'b': put_num(VAR_ARG(arg_list, unsigned int),  2, lead, max_width, 0); break;
			case 'o': put_num(VAR_ARG(arg_list, unsigned int),  8, lead, max_width, 0); break;
			case 'd': put_num(VAR_ARG(arg_list,          int), 10, lead, max_width, 0); break;
			case 'u': put_num(VAR_ARG(arg_list, unsigned int), 10, lead, max_width, 0); break;
			case 'x': put_num(VAR_ARG(arg_list, unsigned int), 16, lead, max_width, 0); break;
			case 'X': put_num(VAR_ARG(arg_list, unsigned int), 16, lead, max_width, 1); break;
			default: put_char(*fmt); break;
		}
	}
}

void printk(const char *fmt, ...)
{
	VAR_LIST ap;
	VAR_START(ap, fmt);
	my_printf(fmt, ap);
	VAR_END(ap);
}
