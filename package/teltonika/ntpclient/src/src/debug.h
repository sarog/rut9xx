#ifndef _DEBUG_H_
#define _DEBUG_H_

/* when present, debug is a true global */
#ifdef ENABLE_DEBUG
extern int g_debug;
#else
#define debug 0
#endif

#endif