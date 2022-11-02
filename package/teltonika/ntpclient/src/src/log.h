#ifndef _LOG_H_
#define _LOG_H_

#define LOG(...) do { \
	fprintf(stdout, ##__VA_ARGS__); fflush(stdout); \
} while (0);

#define ERR(fmt, ...) do { \
	fprintf(stdout, "[%s:%d] error: " fmt, __func__,\
		__LINE__, ##__VA_ARGS__); fflush(stdout); \
} while (0);

#define DBG(...) do { \
	if (g_debug) { fprintf(stdout, ##__VA_ARGS__); fflush(stdout); } \
} while (0);

#endif