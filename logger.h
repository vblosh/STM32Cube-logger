#ifndef USER_INC_LOGGER_H_
#define USER_INC_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { eLogOK, eLogError, eLogWrongParam } eLogErrorCode;

typedef enum {
	log_TRACE = 0,
	log_DEBUG,
	log_INFO,
	log_WARN,
	log_ERROR,
	log_FATAL,
	log_NONE
} eLogLevel;

typedef enum {
	log_END = 0,
	log_PVOID,
	log_PCCHAR,
	log_PCHAR,
	log_INT,
	log_UINT,
	log_FLOAT,
} eParamType;

#ifndef LOG_MAX_LEVEL
#ifdef DEBUG
#define LOG_MAX_LEVEL log_TRACE
#else
#define LOG_MAX_LEVEL log_ERROR
#endif // DEBUG
#endif // LOG_MAX_LEVEL

#define LOG(level, ...) \
do { if (level < LOG_MAX_LEVEL) ; else { log_log(level, __VA_ARGS__, log_END); } } while(0);

#define LOG_TRACE(...) LOG(log_TRACE, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(log_DEBUG, __VA_ARGS__)
#define LOG_INFO(...) LOG(log_INFO, __VA_ARGS__)
#define LOG_WARN(...) LOG(log_WARN, __VA_ARGS__)
#define LOG_ERROR(...) LOG(log_ERROR, __VA_ARGS__)
#define LOG_FATAL(...) LOG(log_FATAL, __VA_ARGS__)

eLogErrorCode log_init();
eLogErrorCode log_deinit();
void log_log(eLogLevel level, const char* msg, ...);
eLogErrorCode log_setLevel(eLogLevel level);
eLogLevel log_getLevel();

#ifdef __cplusplus
}
#endif

#endif /* USER_INC_LOGGER_H_ */
