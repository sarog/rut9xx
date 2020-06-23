/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/
#include <stdio.h>

#ifndef TW_C_SDK_TWDICT_H
#define TW_C_SDK_TWDICT_H

#define TW_DICT_MISSING -3  /* No such element */
#define TW_DICT_FULL -2 	/* Dictionary is full */
#define TW_DICT_OMEM -1 	/* Out of Memory */
#define TW_DICT_OK 0 		/* OK */

#ifndef TW_FOREACH_CONTINUE
#define TW_FOREACH_CONTINUE 0
#define TW_FOREACH_EXIT 1
#endif

#define TW_DICTIONARY_LIST 1
#define TW_DICTIONARY_MAP 2
typedef int twDictionaryMode;

typedef void twDict;
typedef void (*dict_del_func) (void * item);
typedef const char* (*dict_key_parse_func) (void * item);
typedef int (*twDict_foreach_fn)(void *key, size_t key_size, void *data, size_t data_size,void *arg);

/**
* \brief Cleans up any globally allocated data structures. 
*
* \return TW_OK if cleanup succeeded. TW_UNKNOWN_ERROR if an error was encountered.
*/
int twDict_Cleanup ();

/**
 * \brief Creates a new ::twDict
 *
 * \param[in]     delete_function   An optional pointer to a function to call when
 *                                   deleting a value.  If NULL is
 *                                   passed, the default function free() is
 *                                   used.
 *
 * \param[in]     parse_function   An optional pointer to a function to call when
 *                                 the value stored in the dictionary needs to be converted to a key.
 *                                 This key can be used in conjunction with the twDict_Find() function.
 *
 * \return A pointer to the newly allocated ::twDict structure.  Returns NULL if
 * an error was encountered.
 *
*/
twDict * twDict_Create(dict_del_func delete_function,dict_key_parse_func parse_func);

/**
 * \brief Frees all memory associated with a ::twDict and all its owned
 * substructures.
 *
 * \param[in]     in      A pointer to the ::twDict to delete.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDict_Delete(twDict* in);

/**
 * \brief Deletes all entry items within a ::twDict and frees all memory
 * associated with them.
 *
 * \param[in]     in      A pointer to the ::twDict to clear.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDict_Clear(twDict* in);

/**
 * \brief Creates a new Dictionary Entry and adds it to a ::twDict.
 *
 * \param[in]     in      A pointer to the ::twDict to add the entry to
 *                          to.
 * \param[in]     value     The value to be assigned to the Dictionary.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDict_Add(twDict* in, void *value);

/**
 * \brief Removes a dictionary entry from a ::twDict and frees all memory associated
 * it (the value pointer may be optionally freed via the \p deleteValue
 * parameter).
 *
 * \param[in]     in          A pointer to the ::twDict to remove the
 *                              entry from.
 * \param[in]     item         A pointer to the value to remove.
 * \param[in]     deleteValue   If TRUE, the value  will be
 *                              deleted via the ::twDict#delete_function
 *                              associated with the ::twDict.  If FALSE, the
 *                              value is not deleted.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twDict_Remove(twDict* in, void * item, char deleteValue);

/**
 * \brief Provides thread safe iteration of a dictionary.
 *
 * When called, the provided dictionary will be locked, then the provided
 * listHandler will be called once for each item in twDict. Once each item has been processed by listHandler, the
 * twDict will be unlocked. The listHandler function may return TW_FOREACH_EXIT at any time to terminate the iteration.
 * Otherwisem the listHandler function is expected to return TW_FOREACH_CONTINUE.
 * This function will not return until the iteration of the dictionary is complete. userData can be used to pass data
 * which can be shared between each call of the listHandler function and can be used to pass arguments into the function
 * which it can update and use as a return value.
 *
 * Note: During a call to listHandler, it is a very bad idea to try to remove items from the dictionary or to perform any
 * other operation that attemps to lock the dictionary as this will cause the operation to block indefinatly.
 *
 * @param in
 * @param listHandler
 * @param userData
 */
size_t twDict_Foreach(twDict* in, twDict_foreach_fn listHandler, void *userData);

/**
 * This function searches the dictionary by example. A query structure is used to generate a string by passing the query to
 * the parse function provided when the dictionary was constructed. The prase function is then applied to each member value of
 * the dictionary which will produce a key string. If the key string produced matches the key string generated from your query
 * then this dictionary value will be returned. During this find operation the dictionary will be locked and will not be permitted to
 * be changed by any other thread.
 *
 * @param in a twDict that has been created with a parse function.
 * @param query a pointer to a structure that will return a char * key when passed to your parse function.
 * @param result The first item in the dictionary who's output from the parse function matches the key generated when they
 * query is passed to the parse function.
 * @return Either TW_OK if a result was found or TW_MAP_MISSING is there was no key match or if no parse function was
 * provided when the dictionary was constructed.
 */
int twDict_Find(twDict* in, void* query, void** results);

/**
 * \brief Gets the number of entries in a dictionary via ::twDict#count.
 *
 * \param[in]     in      A pointer to the ::twDict to get the number of
 *                          entries of.
 *
 * \return The number of entries in the dictionary (0 if \p in is NULL).
*/
int twDict_GetCount(twDict* in);

/*
Replaces the value of the specified dictionary entry with the new value supplied.
Parameters:
    int - pointer to the dictionary to operate on
	value - pointer to the value whose value should be replaced.
	new_value - the new value
	dispose - Boolean: delete the old value using the delete function specified when the dictionary was created
Return:
	int - zero if successful, non-zero if an error occurred
*/
int twDict_ReplaceValue(twDict* in, void * value, void * new_value, char dispose);

#endif /* TW_C_SDK_TWDICT_H */
