#include <efi.h>
#include <efiutil.h>
#include <stdarg.h>

static void OutputChar(efi_char16_t ch)
{
	efi_char16_t buffer[2];

	buffer[0] = ch;
	buffer[1] = 0;
	st->con_out->output_string(st->con_out, buffer);
}

static void OutputString(efi_char16_t *str)
{
	st->con_out->output_string(st->con_out, str);
}

static const efi_char16_t num_lookup[] = L"0123456789abcdef";

static void printptr(uintptr_t ptr)
{
	for (int i = sizeof(ptr) * 8; i; i -= 4) {
		OutputChar(num_lookup[(ptr >> (i - 4)) & 0xf]);
	}
}

static void printusig(unsigned n, unsigned b)
{
	char buf[10], *p = buf;

	do {
		*p++ = num_lookup[n % b];
	} while (n /= b);

	--p;
	for (; buf <= p; --p)
		OutputChar(*p);
}

static void printsig(int n, unsigned b)
{
	if (n < 0) {
		n *= -1;
		OutputChar('-');
	}
	printusig(n, b);
}

void Print(efi_char16_t *format, ...)
{
	va_list ap;

	va_start(ap, format);

	for (; *format; ++format) {
		if (*format == '%')
			switch (*++format) {
			case 'c':
				OutputChar((efi_char16_t) va_arg(ap, int));
				break;
			case 's':
				OutputString(va_arg(ap, efi_char16_t *));
				break;
			case 'p':
				printptr(va_arg(ap, uintptr_t));
				break;
			case 'x':
				printusig(va_arg(ap, unsigned), 16);
				break;
			case 'd':
				printsig(va_arg(ap, int), 10);
				break;
			case 'u':
				printusig(va_arg(ap, unsigned), 10);
				break;
			default:
				OutputChar('?');
				break;
			}
		else
			OutputChar(*format);
	}

	va_end(ap);
}
