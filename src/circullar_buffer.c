#include "Logger/inc/circular_buffer.h"

eCBError CBInit(sCircularBuffer *cb, void * pBuf, uint16_t size)
{
	if(cb == 0 || pBuf == 0 || size % 2) return eCBErrorWrongSize;

	cb->buf = pBuf;
	cb->size_1 = size - 1;
	cb->read = 0;
	cb->write = 0;
	return eCBOk;
}

inline uint16_t CBEmpty(sCircularBuffer *cb)
{
	return (CBSize(cb) == 0) ? 1 : 0;
}

inline uint16_t CBFull(sCircularBuffer *cb)
{
	return (CBSize(cb) == cb->size_1) ? 1 : 0;
}

inline uint16_t CBWriteIdx(sCircularBuffer *cb)
{
	return cb->write;
}

inline uint16_t CBReadIdx(sCircularBuffer *cb)
{
	return cb->read;
}

inline uint16_t CBCapacity(sCircularBuffer *cb)
{
	return cb->size_1 + 1;
}

inline uint16_t CBSize(sCircularBuffer *cb)
{
   return ((cb->write - cb->read) & cb->size_1);
}

char * CBBufferPtr(sCircularBuffer *cb)
{
	return cb->buf;
}

eCBError CBReadChar(sCircularBuffer *cb, char * data)
{
  if (CBSize(cb) == 0) { return eCBErrorBufferEmpty;}

  *data = ((char*)cb->buf)[cb->read++];
  cb->read &= cb->size_1;
  return eCBOk;
}

eCBError CBWriteChar(sCircularBuffer *cb, char * data)
{
  if (CBSize(cb) == cb->size_1) { return eCBErrorBufferFull;}

  ((char*)cb->buf)[cb->write++] = *data;
  cb->write &= cb->size_1;
  return eCBOk;
}

eCBError CBReadMessage(sCircularBuffer *cb, sMessageItem * data)
{
  if (CBSize(cb) == 0) { return eCBErrorBufferEmpty;}

  *data =  ((sMessageItem *)cb->buf)[cb->read++];
  cb->read &= cb->size_1;
  return eCBOk;
}

eCBError CBWriteMessage(sCircularBuffer *cb, sMessageItem * data)
{
  if (CBSize(cb) == cb->size_1) { return eCBErrorBufferFull;}

  ((sMessageItem *)cb->buf)[cb->write++] = *data;
  cb->write &= cb->size_1;
  return eCBOk;
}
