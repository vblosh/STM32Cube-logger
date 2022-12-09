#ifndef USER_INC_CIRCULAR_BUFFER_H_
#define USER_INC_CIRCULAR_BUFFER_H_

#include <stdint.h>

#include "Logger/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_NUM_PARAMS 6

typedef struct {
	unsigned int time;
	eLogLevel level;
	const char * message;
	unsigned char paramTypes[LOG_NUM_PARAMS];
	union {
		void* pvoid;
		const char * pcchar;
		char * pchar;
		int vint;
		unsigned int vuint;
		float vfloat;
	} params[LOG_NUM_PARAMS];
} sMessageItem;

typedef struct {
  void *buf;    // block of memory
  uint16_t size_1;    // size-1, size must be a power of two !
  uint16_t read;    // holds current read position: 0 to (size-1)
  uint16_t write;   // holds current write position: 0 to (size-1)
} sCircularBuffer;

typedef enum { eCBOk, eCBErrorWrongSize, eCBErrorBufferFull, eCBErrorBufferEmpty } eCBError;

eCBError CBInit(sCircularBuffer *cb, void * pBuf, uint16_t size);
uint16_t CBCapacity(sCircularBuffer *cb);
uint16_t CBSize(sCircularBuffer *cb);
uint16_t CBEmpty(sCircularBuffer *cb);
uint16_t CBFull(sCircularBuffer *cb);

uint16_t CBWriteIdx(sCircularBuffer *cb);
uint16_t CBReadIdx(sCircularBuffer *cb);

char * 	 CBBufferPtr(sCircularBuffer *cb);
eCBError CBReadChar(sCircularBuffer *cb, char * data);
eCBError CBWriteChar(sCircularBuffer *cb, char * data);

eCBError CBReadMessage(sCircularBuffer *cb, sMessageItem * data);
eCBError CBWriteMessage(sCircularBuffer *cb, sMessageItem * data);

#ifdef __cplusplus
}
#endif

#endif /* USER_INC_CIRCULAR_BUFFER_H_ */
