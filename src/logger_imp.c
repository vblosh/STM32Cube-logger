#include <stdatomic.h>
#include <string.h>
#include <stdalign.h>

#include "main.h"
#include "cmsis_os2.h"
#include "Logger/logger.h"
#include "Logger/inc/locks.h"
#include "Logger/inc/circular_buffer.h"
#include "Logger/inc/logger_imp.h"
#include "Logger/inc/fmtparser.h"
#include "Logger/inc/conversions.h"
#include "Logger/inc/chunk_allocator.h"

typedef void (*pWriteFunction)(int);

typedef char* (*print_function)(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx);

typedef char* (*align_strategy_funcion)(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end
		, unsigned arg_idx,
		print_function print_func);

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};

// circular buffer for message queue;
sMessageItem messageQueueBuffer[MSGQUEUE_LENGTH];
sCircularBuffer g_logMessageQueue;
static sMessageItem item;

// circular buffer for strings
static char stringBuffer[STRING_BUF_SIZE];
alloc_handle g_stringAllocator;

// logger thread
static osThreadId_t logTaskHandle;
static const osThreadAttr_t logTask_attributes = {
  .name = "logTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

static atomic_int run_log = 1;

#if defined(USE_LOG_UART) && defined(LOG_UART_DMA)
	static unsigned current_buffer = 0;
	#define NUM_LINE_BUFFERS 2
#else
	#define NUM_LINE_BUFFERS 1
#endif

static char line_buffers[NUM_LINE_BUFFERS][MAX_LINE_LENGTH];
static char* line_buffer = &line_buffers[0][0];
static char* pbuffer;

static char align_buffer[MAX_TEMP_LENGTH];

__attribute__( ( always_inline ) )
static inline char* line_buffer_end()
{
	return line_buffer + MAX_LINE_LENGTH;
}

__attribute__( ( always_inline ) )
static inline char* align_buffer_end()
{
	return align_buffer + MAX_TEMP_LENGTH;
}

static void log_Task(void *argument);

#ifdef USE_LOG_UART
extern UART_HandleTypeDef LOG_UART_HANDLE;

	#ifdef LOG_UART_DMA

	static void log_UART_DMA_write(int len) {
		HAL_StatusTypeDef hal_status;
		// switch buffers
		uint8_t* dma_buffer = (uint8_t*)line_buffers[current_buffer];
		current_buffer = (current_buffer + 1) % NUM_LINE_BUFFERS;
		line_buffer = line_buffers[current_buffer];
		while((hal_status = HAL_UART_Transmit_DMA(&LOG_UART_HANDLE, dma_buffer, len)) == HAL_BUSY) {
			osDelay(10);
		}
	}

	pWriteFunction pWriteFunc = log_UART_DMA_write;
	#else

	static void log_UART_write(int len) {
		HAL_StatusTypeDef hal_status;
		hal_status = HAL_UART_Transmit(&LOG_UART_HANDLE, (uint8_t*)line_buffer, len, HAL_MAX_DELAY);
	}
	pWriteFunction pWriteFunc = log_UART_write;

	#endif
#elif USE_LOG_ITM

	static void log_ITM_write(int len) {
		for (int i = 0; i < len; ++i) {
			ITM_SendChar(line_buffer[i]);
		}
	}
	pWriteFunction pWriteFunc = log_ITM_write;

#else
#error "Choose a supported USE_LOG method"
#endif


eLogErrorCode log_init() {
	if(eCBOk != CBInit(&g_logMessageQueue, &messageQueueBuffer, MSGQUEUE_LENGTH)) return eLogWrongParam;
	g_stringAllocator = alloc_init(stringBuffer, STRING_BUF_SIZE, MAX_PCHAR_LENGTH, alignof(char));
	if(0 == g_stringAllocator) return eLogWrongParam;

    logTaskHandle = osThreadNew(log_Task, pWriteFunc, &logTask_attributes);
    if (logTaskHandle == NULL)  { return eLogError; }

	return eLogOK;
}

eLogErrorCode log_deinit() {
	run_log = 0;

	if(osThreadJoin(logTaskHandle) != osOK) return eLogError;
	return eLogOK;
}

static void copy_func(c_idx start, c_idx end)
{
	pbuffer = safe_memcpy(pbuffer, line_buffer_end(), end - start, item.message + start);
}

static char * print_pvoid(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	char pvoidbuf[8];

	pbuffer = safe_strcpy(pbuffer, pbuffer_end, "0x"); // print prefix 0x
	char * pvoidend = u32toa((uint32_t)item->params[arg_idx].pvoid, pvoidbuf, pvoidbuf + sizeof(pvoidbuf), 16);
	unsigned pvoidlen = pvoidend - pvoidbuf;
	pbuffer = safe_memfill(pbuffer, pbuffer_end, 8 - pvoidlen, '0');
	pbuffer = safe_memcpy(pbuffer, pbuffer_end, pvoidlen, pvoidbuf);
	return pbuffer;
}

static char * print_pcchar(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	return safe_strcpy(pbuffer, pbuffer_end, item->params[arg_idx].pcchar);
}

static char * print_pchar(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	uint32_t interrupts_enabled;

	char * src = item->params[arg_idx].pchar;
	pbuffer = safe_strcpy(pbuffer, pbuffer_end, src);

	lock(&interrupts_enabled);
	deallocate(g_stringAllocator, src);
	unlock(&interrupts_enabled);

	return pbuffer;
}

static char * print_int(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	int radix = fmt->radix.valid ? fmt->radix.integer : 10; // default radix=10
	return i32toa(item->params[arg_idx].vint, pbuffer, pbuffer_end, radix);
}

static char * print_uint(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	int radix = fmt->radix.valid ? fmt->radix.integer : 10; // default radix=10
	return u32toa(item->params[arg_idx].vint, pbuffer, pbuffer_end, radix);
}

static char * print_float(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx)
{
	int precision = fmt->precision.valid ? fmt->precision.integer : 2; // default precision=2
	float fval = item->params[arg_idx].vfloat;
	if (fmt->exponent.valid) {
		if (fmt->exponent.ch == 'g') {
			float fval_abs = fabsf(fval);
			if (fval_abs > 1.0e3f || fval_abs < 1.0e-3f) {
				pbuffer = fetoa(fval, pbuffer, pbuffer_end, precision);
			} else {
				pbuffer = ftoa(fval, pbuffer, pbuffer_end, precision);
			}
		} else if (fmt->exponent.ch == 'e') {
			pbuffer = fetoa(fval, pbuffer, pbuffer_end, precision);
		}
	} else {
		pbuffer = ftoa(fval, pbuffer, pbuffer_end, precision);
	}
	return pbuffer;
}

static char* no_align_strategy(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx
		, print_function print_func)
{
	return print_func(item, fmt, pbuffer, pbuffer_end, arg_idx);
}

static char* left_align_strategy(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx
		, print_function print_func)
{
	char * start = pbuffer;
	pbuffer = print_func(item, fmt, pbuffer, pbuffer_end, arg_idx);
	int len = pbuffer - start;
	if(len < fmt->width.integer) { // fill only if printed length < width
		pbuffer = safe_memfill(pbuffer, pbuffer_end, fmt->width.integer - len, ' ');
	}
	return pbuffer;
}

static char* right_align_strategy(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx
		, print_function print_func)
{
	char * start = align_buffer;
	char * end = print_func(item, fmt, align_buffer, align_buffer_end(), arg_idx);
	int len = end - start;
	if(len < fmt->width.integer) { // fill only if printed length < width
		pbuffer = safe_memfill(pbuffer, pbuffer_end, fmt->width.integer - len, ' ');
	}
	pbuffer = safe_memcpy(pbuffer, pbuffer_end, len, align_buffer);

	return pbuffer;
}

static char* center_align_strategy(const sMessageItem *item
		, const struct replacement_format *fmt
		, char *pbuffer, char * pbuffer_end, unsigned arg_idx
		,print_function print_func)
{
	char * start = align_buffer;
	char * end = print_func(item, fmt, align_buffer, align_buffer_end(), arg_idx);
	int len = end - start;
	if(len < fmt->width.integer) { // fill only if printed length < width
		int margin1 = (fmt->width.integer - len) / 2;
		int margin2 = (fmt->width.integer - len) - margin1;
		pbuffer = safe_memfill(pbuffer, pbuffer_end, margin1, ' ');
		pbuffer = safe_memcpy(pbuffer, pbuffer_end, len, align_buffer);
		pbuffer = safe_memfill(pbuffer, pbuffer_end, margin2, ' ');
	}
	else
		pbuffer = safe_memcpy(pbuffer, pbuffer_end, len, align_buffer);

	return pbuffer;
}

static align_strategy_funcion get_align_strategy(const struct replacement_format *fmt)
{
	align_strategy_funcion align_strategy;
	if(fmt->width.valid) {
		// we have width in format, use alignment
		char align_ch;
		if(!fmt->align.valid) {
			align_ch = '<'; // default alignment is '<'
		}
		else {
			align_ch = fmt->align.ch;
		}

		if(align_ch == '<') {
			align_strategy = left_align_strategy;
		}
		else if(align_ch == '>') {
			align_strategy = right_align_strategy;
		}
		else if(align_ch == '^') {
			align_strategy = center_align_strategy;
		}
	}
	else {
		align_strategy = no_align_strategy;
	}

	return align_strategy;
}

static void replacement_func(struct replacement_format* fmt)
{
	unsigned arg_idx = fmt->arg_id.integer;
	char * pbuffer_end = line_buffer_end();
	align_strategy_funcion align_strategy;

	if(fmt->arg_id.valid && arg_idx < LOG_NUM_PARAMS) {
		// set align strategy
		align_strategy = get_align_strategy(fmt);

		switch (item.paramTypes[fmt->arg_id.integer]) {
			case log_PVOID:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_pvoid);
			break;
			case log_PCCHAR:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_pcchar);
				break;
			case log_PCHAR:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_pchar);
				break;
			case log_INT:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_int);
				break;
			case log_UINT:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_uint);
				break;
			case log_FLOAT:
				pbuffer = align_strategy(&item, fmt, pbuffer, pbuffer_end, arg_idx, print_float);
			default:
				break;
		}
	}
}

static void log_Task(void *argument)
{
	eCBError cbErr;
	pWriteFunction pWriteFn = (pWriteFunction)argument;
	uint32_t interrupts_enabled;

	while(run_log) {
		lock(&interrupts_enabled);

		cbErr = CBReadMessage(&g_logMessageQueue, &item);

		unlock(&interrupts_enabled);

		char * pbuffer_end = line_buffer_end();
		if (cbErr == eCBOk) {
			// process data
			pbuffer = line_buffer;
			// print time stamp
			pbuffer = u32toa(item.time, pbuffer, pbuffer_end, 10);
			pbuffer = safe_memfill(pbuffer, pbuffer_end, line_buffer + 8 - pbuffer, ' ');
			// print level
			pbuffer = safe_strcpy(pbuffer, pbuffer_end, level_strings[item.level]);
			pbuffer = safe_memfill(pbuffer, pbuffer_end, 1, ' ');
			// parse format string
			logger_fmt_parse(item.message, copy_func, replacement_func);
			// output line buffer
			pWriteFn(pbuffer - line_buffer);
		}
		else {
			osDelay(10);
		}
	}
	osThreadExit();
}

