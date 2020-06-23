#ifndef TW_C_SDK_CONFIG_H

#define TW_C_SDK_CONFIG_H
#define HAVE_STRNCASECMP
#define HAVE_TWX_THREAD_H
#define HAVE_SNPRINTF
#define HAVE_VSNPRINTF

#ifdef WIN32
#if _MSC_VER < 1900
#define snprintf   _snprintf
#endif
#else
#define HAVE_STRINGS_H
#endif

#endif
