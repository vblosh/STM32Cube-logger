#ifndef INC_LOGGER_CONVERSIONS_H_
#define INC_LOGGER_CONVERSIONS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char* safe_memcpy(char * dest, char* buff_end, unsigned size, const char * src);
char* safe_strcpy(char * dest, char* buff_end, const char * src);
char* safe_memfill(char * dest, char* buff_end, unsigned size, char ch);

char* u32toa(uint32_t value, char* buffer, char* buff_end, unsigned base);
char* i32toa(int32_t value, char* buffer, char* buff_end, unsigned base);
char* ftoa(float value, char* buffer, char* buff_end, unsigned precision);
char* fetoa(float value, char* buffer, char* buff_end, unsigned precision);

#ifdef __cplusplus
}
#endif

#endif /* INC_LOGGER_CONVERSIONS_H_ */
