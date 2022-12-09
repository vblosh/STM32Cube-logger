#include <stdarg.h>
#include <sys/_stdint.h>
#include <string.h>
#include "Logger/logger.h"
#include "Logger/inc/circular_buffer.h"
#include "Logger/inc/conversions.h"
#include "Logger/inc/locks.h"
#include "Logger/inc/chunk_allocator.h"

uint32_t HAL_GetTick();

static eLogLevel g_level = log_TRACE;

extern sCircularBuffer g_logMessageQueue;
extern sCircularBuffer g_stringBuffer;
extern alloc_handle g_stringAllocator;

void log_log(eLogLevel level, const char* msg, ...) {
	if(level >= g_level) {
		uint32_t interrupts_enabled;
		sMessageItem item;
		item.time = HAL_GetTick();
		item.level = level;
		item.message = msg;
		memset(item.paramTypes, 0, sizeof(item.paramTypes));

		// unpack variable arguments
		va_list params;
		va_start(params, msg);
		eParamType typ = va_arg(params, int);
		char * pend;

		unsigned idx = 0;
		while(typ != log_END && idx < LOG_NUM_PARAMS) {
			switch (typ) {
				case log_PVOID:
					item.paramTypes[idx] = typ;
					item.params[idx].pvoid = va_arg(params, void*);
					break;
				case log_PCCHAR:
					item.paramTypes[idx] = typ;
					item.params[idx].pcchar = va_arg(params, const char*);
					break;
				case log_PCHAR:
					lock(&interrupts_enabled);
					char* dest = allocate(g_stringAllocator);
					unlock(&interrupts_enabled);

					if(dest) {
						char* src = va_arg(params, char*);
						pend = safe_strcpy(dest, dest + chunk_size(g_stringAllocator) - 1, src);
						*pend = '\0';

						item.paramTypes[idx] = typ;
						item.params[idx].pchar = dest;
					}
					else {
						item.paramTypes[idx] = log_END;
					}
					break;
				case log_INT:
					item.paramTypes[idx] = typ;
					item.params[idx].vint = va_arg(params, int);
					break;
				case log_UINT:
					item.paramTypes[idx] = typ;
					item.params[idx].vuint = va_arg(params, unsigned int);
					break;
				case log_FLOAT:
					item.paramTypes[idx] = typ;
					item.params[idx].vfloat = (float)va_arg(params, double);
					break;
				default:
					break;
			}
			typ = va_arg(params, int);
			++idx;
		}
		va_end(params);

		// put item in the queue
		lock(&interrupts_enabled);
		CBWriteMessage(&g_logMessageQueue, &item);
		unlock(&interrupts_enabled);
	}
}

eLogErrorCode log_setLevel(eLogLevel level) {
	if(level < log_TRACE || level > log_NONE) { return eLogWrongParam; }
	g_level = level;
	return eLogOK;
}

eLogLevel log_getLevel() {
	return g_level;
}
