#include <stdio.h>
#ifndef TW_C_SDK_TWMAP_H
#define TW_C_SDK_TWMAP_H

#ifndef TW_FOREACH_CONTINUE
#define TW_FOREACH_CONTINUE 0
#define TW_FOREACH_EXIT 1
#endif

#define TW_MAP_MISSING -3  /* No such element */
#define TW_MAP_FULL -2 	/* Hashmap is full */
#define TW_MAP_OMEM -1 	/* Out of Memory */
#define TW_MAP_OK 0 	/* OK */

/*
 * twMap is a pointer to an internally maintained data structure.
 * Clients of this package do not need to know how hashmaps are
 * represented.  They see and manipulate only twMap's.
 */
typedef void twMap;

/*
 * Tears down lazy-created globals. Return TW_OK or TW_UNKNOWN_ERROR
 * if invoked with one or more dangling map references.
*/
int twMap_Cleanup();

/*
 * Return an empty hashmap. Returns NULL if empty.
*/
twMap* twMap_new();

/*
 * Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
 */
int twMap_put(twMap* in, const char* key, void * value);

/*
 * Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int twMap_get(twMap* in, const char* key, void * *arg);

/*
 * Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
 */
int twMap_remove(twMap* in, const char* key);

/*
 * Free the hashmap
 */
void twMap_free(twMap* in);

/*
 * Get the current size of a hashmap
 */
int twMap_length(twMap* in);


/* Mock List Interface */
typedef void (*map_del_func) (void * item);
typedef const char* (*map_key_parse_func) (void * item);
twMap * twMap_Create(map_del_func delete_function,map_key_parse_func parse_func);
int twMap_Delete(twMap* in);
int twMap_Clear(twMap* in);

int twMap_Add(twMap* in, void *value);
int twMap_Remove(twMap* in, void * item, char deleteValue);

typedef int (*twMap_foreach_fn)(void *key, size_t key_size, void *data, size_t data_size,void *arg);

size_t twMap_Foreach(twMap* in, twMap_foreach_fn listHandler, void *userData);


int twMap_Find(twMap* in, void* query, void** results);

int twMap_GetCount(twMap* in);
int twMap_ReplaceValue(twMap* in, void * value, void * new_value, char dispose);


#endif /* TW_C_SDK_TWHASHMAP_H */
