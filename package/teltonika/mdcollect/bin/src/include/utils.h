#ifndef __UTILS_H
#define __UTILS_H

#if defined(MDC_DEBUG)
#include  <libubox/blobmsg_json.h>
#endif

enum { IP_TYPE_V4, IP_TYPE_V6, IP_TYPE_V4V6 };

void print_usage(const char *prog_name);
void restore_DB(char *from, char *to);
md_status_t md_runscript(const char *script, char *ifname);
time_t get_stime(int year, int month, int day, int hour, int min, int sec);

/**
 * @brief Helper functiom. Check if memory allocation was successful and exits program on fail
 * 
 * @param ptr - newly allocated pointer
 */
void check_alloc(void *ptr);

#if defined(MDC_DEBUG)
void debug_blobmsg(struct blob_attr *msg, const char *file,
		const char *func, int line);

#define DEBUG_BLOBMSG(x) debug_blobmsg(x, __FILE__, __func__, __LINE__)
#endif

#endif //__UTILS_H