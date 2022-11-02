/***************************************
 *  Copyright 2016, PTC, Inc.
 ***************************************/

/**
 * \file stringUtils.h
 * \brief String utility function prototypes.
*/

#ifndef TW_STRING_UTILS_H
#define TW_STRING_UTILS_H

#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Converts a string to lowercase.
 *
 * \param[in]     input     The string to change to lowercase.
 *
 * \return The lowercase format of \p input.
 *
 * \note The string is directly modified, not copied.
*/
char * lowercase(char *input);

/**
 * \brief Converts a string to uppercase.
 *
 * \param[in]     input     The string to change to uppercase.
 *
 * \return The uppercase format of \p input.
 *
 * \note The string is directly modified, not copied.
*/
char * uppercase(char *input);

/**
 * \brief Copies a string.
 * \deprecated Prefer duplicateStringN() and supply a reasonable maximum based on context
 *
 * \param[in]     input     The string to copy.
 *
 * \return A copy of \p input.
 *
 * \note The calling function gains ownership of the returned string and
 * retains ownership of the \p input string.
*/
char * duplicateString(const char * input);

/**
* \brief Copies a string, up to some maximum length.
*
* \param[in]     input     The string to copy.
* \param[in]     maxlen    The maximum length of the resulting string, excluding the terminating null
*
* \return A copy of up to the first \p maxlen chars of \p input, with a null terminator.
*
* \note The calling function gains ownership of the returned string and
* retains ownership of the \p input string.
*/
char * duplicateStringN(const char * input, size_t maxlen);

/**
 * \brief concatenates strings.
 *
 * \param[in]     dest     reference to the destination string
 * \param[in]     src      pointer to string which will be appended to destination
 *
 * \return An integer status value, TW_OK on success, error code on failure
 *
 * \note the original dest pointer will be free'd and replaced after the new memory has been allocated 
*/
int concatenateStrings( char ** dest, const char * src);

/**
 * \brief concatenates strings uo to a maximum length.
 *
 * \param[in]     dest     reference to the destination string
 * \param[in]     src      pointer to string which will be appended to destination
 * \param[in]     maxlen    The maximum length of the resulting string, excluding the terminating null
 *
 * \return An integer status value, TW_OK on success, error code on failure
 *
 * \note the original dest pointer will be free'd and replaced after the new memory has been allocated
*/
int concatenateStringsN(char ** dest, const char * src, size_t maxlen);


/**
 * \brief returns TRUE if str ends with suf.
 *
 * \param[in]     str     reference to the string to search
 * \param[in]     suf     a string that str must end with for this function to return TRUE
 *
 * \return TRUE if suf is present at the end of str, otherwise FALSE;
 *
*/
char stringEndsWithSuffix(const char *str, const char *suf);

#ifdef __cplusplus
}
#endif

#endif
