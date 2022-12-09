#ifndef _FMTPASER_H_
#define _FMTPASER_H_
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	/*
replacement_field :: = "{"[arg_id][":"format_spec]"}"
arg_id            :: = integer
integer           :: = digit +
digit             :: = "0".."9"

format_spec ::=  [[align]width][["."precision]["e"|"g"]]|["r"radix]]
align       ::=  "<" | ">" | "^"
width       ::=  integer
precision   ::=  integer
radix		::=	 integer

escaping = "{{"
*/

typedef unsigned int c_idx;
typedef char const* c_str;

typedef struct {
	unsigned integer;
	bool valid;
} s_int_valid;

typedef struct {
	char ch;
	bool valid;
} s_char_valid;

struct replacement_format
{
	s_int_valid arg_id;
	s_char_valid align;
	s_int_valid width;
	s_int_valid radix;
	s_int_valid precision;
	s_char_valid exponent;
};

typedef void (*copy_callback)(c_idx start, c_idx end);
typedef void (*replacement_callback)(struct replacement_format* format);

void logger_fmt_parse(c_str fmt, copy_callback copy, replacement_callback callback);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_FMTPASER_H_

