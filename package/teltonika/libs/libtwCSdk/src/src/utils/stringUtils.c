/*
 *  Copyright 2016, PTC, Inc.
 *
 *  String Utils
 */

#include "twOSPort.h"
#include "string.h"

#define MAX_STR_LEN 65536

INLINE char locase(char c) 
    { if (c >= 'A' && c <= 'Z') return (char)(c + 32); return c; }

INLINE char upcase(char c) 
    { if (c >= 'a' && c <= 'z') return (char)(c - 32); return c; }

char * lowercase(char *input) {
	if (input != 0) {
		char * x = input;
        while (*x != 0) {
            *x = locase(*x);
			x++;
		}
    }
	return input;
}

char * uppercase(char *input) {
	if (input != 0) {
		char * x = input;
        while (*x != 0) {
            *x = upcase(*x);
			x++;
		}
    }
	return input;
}

char * duplicateStringN(const char * input, size_t maxlen) {
	char * res = NULL;
	size_t len = 0;
	if (!input) return NULL;
	len = strnlen (input, maxlen);
	res = (char *) TW_CALLOC(len + 1, 1);	/* Calloc ensures null termination, as strncpy does not */
	if (res) strncpy(res, input, len);
	return res;
	}

char * duplicateString(const char * input) {
	/* Be safe; use some sane maximum. */
	return duplicateStringN(input, MAX_STR_LEN);
	}

int concatenateStringsN(char ** dest, const char * src, size_t maxlen) {
	/* variable to store new string */
	char * tmp = NULL;
	size_t destLen = 0;
	size_t srcLen = 0;

	/* check inputs */
	if (!src || !dest || !(*dest)) {
		/* invalid params, take no action */
		return TW_INVALID_PARAM;
	}

	destLen = strnlen(*dest, maxlen);
	srcLen = strnlen(src, maxlen);

	/* if the two strings would be longer than maxlen we will truncate */
	if (destLen + srcLen > maxlen) {
		srcLen = maxlen - destLen;
		}

	/* allocate memory dest len + src len + null terminator */
	tmp = (char *)TW_CALLOC(destLen + srcLen + 1, 1);
	if (!tmp) {
		/* error allocating memory */
		return TW_ERROR_ALLOCATING_MEMORY;
	}

	/* copy memory */
	strncpy(tmp, *dest, destLen);
	strncat(tmp, src, srcLen);

	/* free old dest memory */
	TW_FREE(*dest);

	/* replace dest pointer with new string */
	*dest = tmp;

	/* return success */
	return TW_OK;
}

int concatenateStrings(char ** dest, const char * src) {
	/* Use MAX_STR_LEN if no size is passed in. */
	return concatenateStringsN(dest, src, MAX_STR_LEN);
}

char stringEndsWithSuffix(const char *str, const char *suf) {

	const char *a = str + strnlen(str, MAX_STR_LEN);
	const char *b = suf + strnlen(suf, MAX_STR_LEN);

	while (a != str && b != suf) {
		if (*--a != *--b) break;
	}
	return b == suf && *a == *b;
}
