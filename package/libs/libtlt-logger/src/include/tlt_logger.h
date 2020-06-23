#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
	L_DEBUG,
	L_INFO,
	L_NOTICE,
	L_WARNING,
	L_ERROR,
	L_CRIT,
	L_ALERT,
	L_EMERG
} log_level_type;

enum {
    L_TYPE_SYSLOG =  1,
    L_TYPE_STDOUT = (1 << 1),
};

int logger_init(log_level_type _min_level, int logger_type, const char *prog_name);

void _log(log_level_type level, const char *fmt, ...)
	__attribute__((format (printf, 2, 3)));

#define log(level, fmt, ...) _log(level, fmt"\n", ##__VA_ARGS__)

#endif
