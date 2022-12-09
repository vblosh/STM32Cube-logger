#include <math.h>
#include "Logger/inc/conversions.h"

#define MIN(a,b) (((a)<(b))?(a):(b))

inline char* safe_strcpy(char * dest, char* buff_end, const char * src)
{
	while(*src != '\0' && dest != buff_end) *dest++ = *src++;
	return dest;
}

inline char* safe_memcpy(char * dest, char* buff_end, unsigned size, const char * src)
{
	char * end = MIN(buff_end, dest + size);
	while(dest != end) *dest++ = *src++;
	return dest;
}

inline char* safe_memfill(char * dest, char* buff_end, unsigned size, char ch)
{
	char * end = MIN(buff_end, dest + size);
	while(dest != end) *dest++ = ch;
	return dest;
}

char* u32toa(uint32_t value, char* buffer, char* buff_end, unsigned base)
{
    char temp[34];
    char *p = temp;
    do {
    	char residue = (char)(value % base);
        *p++ = residue > 9 ? (residue + 'a' -  10): (residue + '0');
        value /= base;
    } while (value > 0);

    do {
        *buffer++ = *--p;
    } while (p != temp && buffer < buff_end);

    return buffer;
}

char* i32toa(int32_t value, char* buffer, char* buff_end, unsigned base)
{
    uint32_t u = (uint32_t)value;
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1;
    }
    return u32toa(u, buffer, buff_end, base);
}

static char* uftoa(uint64_t value, char* buffer, char* buff_end, unsigned precision)
{
    char temp[20];

	char *p = temp;
    do {
        *p++ = (char)(value % 10) + '0';
        value /= 10;
    } while (value > 0);


    unsigned value_len = p - temp;
    if(value_len <= precision && buffer < buff_end - 2) {
        // value < 1.0, output leading zeros and 0. prefix
    	// output 0. prefix
        *buffer++ = '0';
        *buffer++ = '.';
        // output leading zeros
 		unsigned num_leading_zeros = precision - value_len;
        for (unsigned i = 0; i < num_leading_zeros && buffer < buff_end; ++i) {
            *buffer++ = '0';
		}
        // reverse and output digits
        while (p > temp && buffer < buff_end) {
            *buffer++ = *--p;
        }
    }
    else {
		char * point = temp + precision; // where to put separator
		// reverse and output digits
		while (p > temp && buffer < buff_end) {
			if(p == point) {
				*buffer++ = '.';
				if(buffer == buff_end) break;
			}
			*buffer++ = *--p;
		}
    }

    return buffer;
}

static const int multiplier[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};

char* ftoa(float value, char* buffer, char* buff_end, unsigned precision)
{
	if(precision > 8) precision = 8;

	if(value < 0.0f) {
		*buffer++ = '-';
		value *= -1.0f;
	}

    float m1 = value * multiplier[precision];
    uint64_t fraction = (uint64_t)(m1 + 0.5f);

    return uftoa(fraction, buffer, buff_end, precision);
}

char* fetoa(float value, char* buffer, char* buff_end, unsigned precision)
{
	if(precision > 8) precision = 8;

	if(value < 0.0f) {
		*buffer++ = '-';
		value *= -1.0f;
	}

	float exponent = floorf(log10f(value));
	float fraction = value/expf(exponent * (float)M_LN10);

	buffer = ftoa(fraction, buffer, buff_end, precision);
	*buffer++ = 'e';
	buffer = i32toa(exponent, buffer, buff_end, 10);
	return buffer;
}
