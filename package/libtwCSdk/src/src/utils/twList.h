/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file list.h
 * \brief Linked List definitions and function prototypes
 *
 * Contains structure type definitions and function prototypes for the
 * ThingWorx linked list implementation.  ::twList is dynamically sized,
 * thread-safe, untyped, and doubly linked.
*/

#ifndef TW_LIST_H
#define TW_LIST_H

#ifndef TW_FOREACH_CONTINUE
#define TW_FOREACH_CONTINUE 0
#define TW_FOREACH_EXIT 1
#endif

#include "twOSPort.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ListEntry;

/**
* \brief Frees any lazily allocated global data
*/
int twList_Cleanup ();

/**
 * \brief Signature of a function called to delete the values in a ::ListEntry.
 *
 * \param[in]     item      A pointer to an item to be deleted.
 *
 * \return Nothing.
*/
typedef void (*del_func) (void * item);
typedef const char* (*parse_func) (void * item);

/**
 * \brief Linked list entry structure definition.
*/
typedef struct ListEntry {
    struct ListEntry *next; /**< A pointer to the next ::ListEntry in the ::twList. **/
    struct ListEntry *prev; /**< A pointer to the previous ::ListEntry in the ::twList. **/
    void *value;            /**< The data item of this ::ListEntry. **/
} ListEntry;

typedef int (*twForEachHander)(void *key, size_t key_size, void *data, size_t data_size,void *userData);

/**
 * \brief Linked list structure definition.
*/
typedef struct twList {
	int count;                /**< The number of elements in the linked list. **/
	struct ListEntry *first;  /**< A pointer to the first ::ListEntry in the ::twList. **/
	struct ListEntry *last;   /**< A pointer to the last ::ListEntry in the ::twList. **/
	TW_MUTEX mtx;             /**< A mutex for this ::twList structure. **/
	del_func delete_function; /**< A deletion function associated with this ::twList. **/
	parse_func parse_function; /**< A find function associated with this ::twList. **/
} twList;

/**
 * \brief Creates a new ::twList.
 *
 * \param[in]     delete_function   A pointer to a function to call when
 *                                   deleting a ::ListEntry value.  If NULL is
 *                                   passed, the default function free() is
 *                                   used.
 *
 * \return A pointer to the newly allocated ::twList structure.  Returns NULL if
 * an error was encountered.
 *
 * \note The new ::twList gains ownership of all ::ListEntry and
 * ::ListEntry#value pointers in the ::ListEntry.
 * \note The calling function gains ownership of the returned ::twList and is
 * responsible for deleting it via twList_Delete().
*/
twList *twList_Create(del_func delete_function);
twList *twList_CreateSearchable(del_func delete_function, parse_func parse_function);

/**
 * \brief Frees all memory associated with a ::twList and all its owned
 * substructures.
 *
 * \param[in]     list      A pointer to the ::twList to delete.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twList_Delete(struct twList *list);

/**
 * \brief Deletes all ::ListEntry items within a ::twList and frees all memory
 * associated with them.
 *
 * \param[in]     list      A pointer to the ::twList to clear.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twList_Clear(struct twList *list);

/**
 * \brief Deletes all ::ListEntry items within a ::twList without freeing memory
 * associated with their values.
 *
 * \param[in]     list          A pointer to the ::twList to clear.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twList_ClearEntries(struct twList *list);

/**
 * \brief Creates a new ::ListEntry and adds it to a ::twList.
 *
 * \param[in]     list      A pointer to the ::twList to add the ::ListEntry
 *                          to.
 * \param[in]     value     The value to be assigned to the ::ListEntry.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twList_Add(twList *list, void *value);

/**
 * \brief Removes a ::ListEntry from a ::twList and frees all memory associated
 * it (the value pointer may be optionally freed via the \p deleteValue
 * parameter).
 *
 * \param[in]     list          A pointer to the ::twList to remove the
 *                              ::ListEntry from.
 * \param[in]     entry         A pointer to the ::ListEntry to remove.
 * \param[in]     deleteValue   If TRUE, the value of the ::ListItem will be
 *                              deleted via the ::twList#delete_function
 *                              associated with the ::twList.  If FALSE, the
 *                              value is not deleted.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twList_Remove(struct twList *list, struct ListEntry * entry, char deleteValue);

/**
 * \brief Gets the next ::ListEntry in a ::twList via the ::ListEntry#next
 * pointer.
 *
 * \param[in]     list      A pointer to the ::twList to operate on.
 * \param[in]     entry     A pointer to the current entry (NULL will get
 *                          ::twList#first entry).
 *
 * \return A pointer to the next ::ListEntry in the ::twList.  Returns NULL if
 * \p entry was the last ::ListEntry or if an error was encountered.
 *
 * \note \p list will maintain ownership of the \p entry pointer so the calling
 * function should <b>not</b> delete it.
*/
ListEntry * twList_Next(struct twList *list, struct ListEntry * entry);

/**
 * \brief Provides thread safe iteration of a list.
 *
 * When called, the provided list will be locked, then the provided
 * listHandler will be called once for each item in twList. Once each item has been processed by listHandler, the
 * twList will be unlocked. The listHandler function may return 0/FALSE at any time to terminate the iteration.
 * This function will not return until the iteration of the list is complete. userData can be used to pass data
 * which can be shared between each call of the listHandler function and can be used to pass arguments into the function
 * which it can update and use as a return value.
 *
 * Note: During a call to listHandler, it is a very bad idea to try to remove items from the list or to perform any
 * other operation that attempts to lock the list as this will cause the operation to block indefinitely.
 *
 * @param list
 * @param listHandler
 * @param userData
 */
int twList_Foreach(twList *list, twForEachHander listHandler, void *userData);

/**
 * \brief Given an \p index, gets the ::ListEntry associated with that index.
 *
 * \param[in]     list      A pointer to the ::twList to get the ::ListEntry
 *                          from.
 * \param[in]     index     The (zero-based) index of the ::ListEntry to
 *                          retrieve.
 *
 * \return A pointer to the ::ListEntry associated with \p index of \p list.
 *
 * \note \p list will maintain ownership of the \p entry pointer so the calling
 * function should <b>not</b> delete it.
*/
ListEntry * twList_GetByIndex(struct twList *list, int index);

/**
 * \brief Gets the number of entries in a list via ::twList#count.
 *
 * \param[in]     list      A pointer to the ::twList to get the number of
 *                          entries of.
 * 
 * \return The number of entries in the list (0 if \p list is NULL).
*/
int twList_GetCount(struct twList *list);

/*
twList_ReplaceValue - Replaces the value of the specified list entry with the new value supplied.
Parameters:
    list - pointer to the list to operate on
	entry - pointer to the entry whose value should be replaced.  
	new_value - the new value
	dispose - Boolean: delete the old value using the delete function specified when the list was created 
Return:
	int - zero if successful, non-zero if an error occurred
*/
int twList_ReplaceValue(struct twList *list, struct ListEntry * entry, void * new_value, char dispose);

/**
 * This function searches the list by example. A query structure is used to generate a string by passing the query to
 * the parse function provided when the list was constructed. The parse function is then applied to each member value of
 * the list which will produce a key string. If the hey string produced matches the key string generated from your query
 * then this list value will be returned. During this find operation the list will be locked and will not be permitted to
 * be changed by any other thread.
 *
 * @param list a twList that has been created with a parse function.
 * @param query a pointer to a structure that will return a char * key when passed to your parse function.
 * @param result The first item in the list who's output from the parse function matches the key generated when they
 * query is passed to the parse function.
 * @return Either TW_OK if a result was found or TW_MAP_MISSING is there was no key match or if no parse function was
 * provided when the list was constructed.
 */
int twList_Find(twList* list, void* query, void** result);


#ifdef _DEBUG
/** Debug only task to check allocation sizes of twLists during runtime */
void twList_CheckTask(DATETIME now, void * params);
#endif
#ifdef __cplusplus
}
#endif

#endif
