/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twLinux.h
 * \brief Wrappers for Linux-specific functionality using AxTLS
*/

#ifndef TW_LINUX_H
#define TW_LINUX_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <dirent.h>
#include <stdio.h>
#include <CFNetwork/CFSocketStream.h>


/**
 * \name Logging
*/
#ifndef TW_LEAN_AND_MEAN
/**
 * \brief The maximum size of the log buffer.
*/
#define TW_LOGGER_BUF_SIZE 4096
#define TW_LOG(level, fmt, ...)  twLog(level, fmt, ##__VA_ARGS__)
#define TW_LOG_HEX(msg, preamble, length)   twLogHexString(msg, preamble, length)
#define TW_LOG_MSG(msg, preamble)   twLogMessage(msg, preamble)

/**
 * \name Proxies
*/
#ifndef ENABLE_HTTP_PROXY_SUPPORT
#define ENABLE_HTTP_PROXY_SUPPORT
#endif
#ifndef USE_NTLM_PROXY
#define USE_NTLM_PROXY
#endif

#else /* LEAN AND MEAN */
/**
 * \name Lean and Mean Options
 * \brief  Disables logging.  Can also use this to override other resource/code
 * size impacting functions such as file transfer, offline message store, and
 * tunneling.
*/
#define TW_LOGGER_BUF_SIZE 1
#define TW_LOG(level, fmt, ...)
#define TW_LOG_HEX(msg, preamble, length)
#define TW_LOG_MSG(msg, preamble) 

#undef OFFLINE_MSG_STORE
#define OFFLINE_MSG_STORE 0
#undef  ENABLE_HTTP_PROXY_SUPPORT
#undef USE_NTLM_PROXY
#undef ENABLE_FILE_XFER
#undef ENABLE_TUNNELING
#endif

/**
 * \brief Date/time type definition.
*/
typedef uint64_t DATETIME;

/**
 * \brief For Linux builds a #TW_MUTEX is a pthread_mutex_t.
*/
#define TW_MUTEX pthread_mutex_t *

/* Forward declaration */
struct twSocket;

/**
 * \name Sockets
*/
typedef struct iosStream {
	struct twSocket * parent;
	CFReadStreamRef readStream;
	CFWriteStreamRef writeStream;
	CFStreamClientContext ctx;
} iosStream;

#define IPV4 AF_INET
#define IPV6 AF_INET6
#define TW_SOCKET_TYPE iosStream*
#define TW_ADDR_INFO struct addrinfo

/**
 * \name Tasks
*/
#define TICKS_PER_MSEC 1

/**
 * \name Memory
*/
#define TW_MALLOC(a) malloc(a) 
#define TW_CALLOC(a, b) calloc(a,b)
#define TW_REALLOC(a, b) realloc(a, b)
#define TW_FREE(a) free(a)

/**
 * \name File Transfer
*/
#define TW_FOPEN(a,b) fopen(a,b)
#define TW_FCLOSE(a) fclose(a)
#define TW_FREAD(a,b,c,d) fread(a,b,c,d)
#define TW_FWRITE(a,b,c,d) fwrite(a,b,c,d)
#define TW_FSEEK(a,b,c) fseeko(a,b,c)
#define TW_FERROR(a) ferror(a)
#define TW_FTELL(a) ftell(a)

#define TW_FILE_HANDLE FILE*
#define TW_FILE_DELIM '/'
#define TW_FILE_DELIM_STR "/"
#define TW_FILE_CASE_SENSITVE TRUE
#define TW_DIR DIR *
#define ERROR_NO_MORE_FILES 0

/**
 * \name Threads
*/
#define TW_THREAD_ID pthread_t    

/**
 * \name Misc
*/
#define INLINE 
#ifndef OS_IOS

char getch();

#endif


/**
 * \name Utils
*/

char * stringCopyInUTF8FromCFString(CFStringRef string);
CFStringRef cFStringFromUTF8String(char *string);
void clearLastStreamError();
void setLastErrorFromReadStream(CFReadStreamRef stream);
void setLastErrorFromWriteStream(CFWriteStreamRef stream);

#endif
