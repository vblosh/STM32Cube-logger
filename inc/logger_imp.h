#ifndef INC_LOGGER_IMP_H_
#define INC_LOGGER_IMP_H_

// max length of log line
#define MAX_LINE_LENGTH 128

// max length of temporary buffer
#define MAX_TEMP_LENGTH 32

// length of logger message queue
// should be power of 2 because of circular buffer it use
#define MSGQUEUE_LENGTH 64

// size of string buffer
#define STRING_BUF_SIZE 128
// max length of char* parameter
#define MAX_PCHAR_LENGTH 32

// use ITM to logging
//#define USE_LOG_ITM 1
// use UART for logging
//#define USE_LOG_UART 1

#ifndef USE_LOG_UART
#ifndef USE_LOG_ITM
	#define USE_LOG_UART 1
#endif
#endif

// uart handle to use for logging
#define LOG_UART_HANDLE huart3
// uart is configured to use DMA
#define LOG_UART_DMA 1


#endif /* INC_LOGGER_IMP_H_ */
