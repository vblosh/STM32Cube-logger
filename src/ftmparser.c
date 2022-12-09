#include <assert.h>
#include "Logger/inc/fmtparser.h"

static void skip(c_str str, c_idx* idx)
{
	while (true) {
		while (str[*idx] != 0 && str[*idx] != '{') ++(*idx);
		if (str[(*idx)+1] == '{') {
			// skip escaping = "{{"
			idx += 2;
			continue;
		}
		else
			break;
	}
}

__attribute__( ( always_inline ) )
static inline bool check_digit(char ch)
{
	return (ch >= '0' && ch <= '9');
}

static s_int_valid parse_integer(c_str str, c_idx* idx)
{
	s_int_valid res = { 0, false };
	while (check_digit(str[*idx])) {
		res.integer = res.integer * 10 + (str[(*idx)++] - '0');
		res.valid = true;
	}
	return res;
}

static struct replacement_format repl_format;

static void parse_replacement(c_str str, c_idx* idx, replacement_callback callback)
{

	assert(str[(*idx)] == '{');
	++(*idx); // skip '{'
	// parse arg_id
	repl_format.arg_id = parse_integer(str, idx);
	repl_format.align.valid = false;
	repl_format.width.valid = false;
	repl_format.precision.valid = false;
	repl_format.radix.valid = false;
	repl_format.exponent.valid = false;
	if (repl_format.arg_id.valid) {
		if (str[(*idx)] == ':') { 
			++(*idx);
			// parse format_spec
			if (str[*idx] == '<' || str[*idx] == '>' || str[*idx] == '^') {
				// alignment
				repl_format.align.ch = str[(*idx)++];
				repl_format.align.valid = true;
			}
			repl_format.width = parse_integer(str, idx); // width
			if (str[*idx] == '.') {
				// precision
				++(*idx);
				repl_format.precision = parse_integer(str, idx);
			}
			if(str[*idx] == 'e' || str[*idx] == 'g') {
				repl_format.exponent.ch = str[(*idx)++];
				repl_format.exponent.valid = true;
			}
			else if (str[*idx] == 'r') {
				// radix
				++(*idx);
				repl_format.radix = parse_integer(str, idx);
			}
		}
	}
	if (str[*idx] != '}') {
		repl_format.arg_id.valid = false;
	}
	else {
		++(*idx);
	}

	// process replacement
	callback(&repl_format);
}

void logger_fmt_parse(c_str str, copy_callback copy, replacement_callback callback)
{
	c_idx idx = 0;
	c_idx old_idx;
	do {
		old_idx = idx;
		skip(str, &idx);
		copy(old_idx, idx);
		if(str[idx] != 0) {
			parse_replacement(str, &idx, callback);
		}

	} while (str[idx] != 0);
}
