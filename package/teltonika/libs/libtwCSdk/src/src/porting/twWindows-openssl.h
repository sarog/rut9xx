/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twWindows-openssl.h
 *
 * \brief Wrappers for Windows-specific functionality using OpenSSL
*/

#ifndef TW_WINDOWS_H
#define TW_WINDOWS_H

#include "winsock2.h"
#include "windows.h"
#include "conio.h"
#include "stdio.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name TLS Library
*/
/**
 * \brief Define which pluggable TLS library is used.
 *
 * \note The NO_TLS option turns off encryption altogether which may be useful
 * for debugging but is also <b>not recommended for production environments</b>
 * as it may introduce serious security risks.
*/
#define TW_TLS_INCLUDE "twOpenSSL.h" 

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
#define ENABLE_HTTP_PROXY_SUPPORT
#define USE_NTLM_PROXY

#else /* LEAN AND MEAN */
/**
 * \name Lean and Mean Options
 * \brief  Disables logging.  Can also use this to override other resource
 * /code size impacting functions such as file transfer, offline message store,
 * and tunneling
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
 * \brief Date/time definition
*/
typedef DWORD64 DATETIME;

/**
 * \brief For Windows builds #TW_MUTEX is a LPHANDLE */
#define TW_MUTEX LPHANDLE

/**
 * \name Sockets
*/
#define IPV4 AF_INET
#define IPV6 AF_INET6
#define TW_SOCKET_TYPE int
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
#define TW_FILE_HANDLE FILE*
#define TW_FILE_DELIM '\\'
#define TW_FILE_DELIM_STR "\\"
#define TW_FILE_CASE_SENSITVE FALSE
#define TW_DIR HANDLE 

TW_FILE_HANDLE win_fopen(const char* name, const char* mode);
#define TW_FOPEN(a,b) win_fopen(a,b)
#define TW_FCLOSE(a) fclose(a)
#define TW_FREAD(a,b,c,d) fread(a,b,c,d)
#define TW_FWRITE(a,b,c,d) fwrite(a,b,c,d)
#define TW_FSEEK(a,b,c) _fseeki64(a,b,c)
#define TW_FTELL(a) _ftelli64(a)
#define TW_FERROR(a) ferror(a)

/**
 * \name Threads
*/
#define TW_THREAD_ID HANDLE  

/**
 * \name Misc
*/
/**
 * \brief Use __forceinline (VC++ specific).
*/
#define INLINE __forceinline
#if _MSC_VER < 1900
#define snprintf   _snprintf
#endif

/* Thingworx integration - be64toh not defined on windows*/
#	if BYTE_ORDER == LITTLE_ENDIAN 
#		define be64toh(x) ntohll(x) 
#	elif BYTE_ORDER == BIG_ENDIAN 
#		define be64toh(x) (x) 
#	else 
#		error byte order not supported 
#	endif 

#ifdef __cplusplus
}
#endif

#endif
