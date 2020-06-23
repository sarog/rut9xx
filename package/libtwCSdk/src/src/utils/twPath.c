/*
 * Copyright 2018, PTC, Inc.
 * All rights reserved.
 */
#ifdef _WIN32
#include TW_OS_INCLUDE
#else
#include <stdlib.h>
#endif

char * twPath_GetFullPath(char const * relativePath) {
#ifdef _WIN32
    DWORD nBufferLength = MAX_PATH;
    char * buffer = (char *) TW_CALLOC(nBufferLength, 1);
    DWORD returnValue = GetFullPathName(
        relativePath,
        nBufferLength,
        buffer,
        NULL
    );
    return (0 == returnValue) ? NULL : buffer;
#else
    return realpath(relativePath, NULL);
#endif
}
