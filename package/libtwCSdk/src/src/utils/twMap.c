#include "twApiStubs.h"
#include "twErrors.h"
#include "twMap.h"
#include "cfuhash.h"
#include "twOSPort.h"
#include "twApiStubs.h"

/* Lazily created globals */
static twMap *twMap_DeleteFunctionMap;
static twMap *twMap_ParseFunctionMap;

/*
* Clean up lazily created globals. Can't use twMap_Delete, 
* which depends on the globals that are being deallocated. 
*/
int twMap_Cleanup (){
	int res = TW_OK;
	if (twMap_DeleteFunctionMap){
		size_t count;
		count = cfuhash_num_entries ((cfuhash_table_t *)twMap_DeleteFunctionMap);
		if (count){
			TW_LOG (TW_WARN, "twMap_Cleanup: destroying twMap_DeleteFunctionMap with %u entries", count);
			res = TW_UNKNOWN_ERROR;
			}
		if (0 == s_cfuhash_destroy ((cfuhash_table_t*)twMap_DeleteFunctionMap)){
			TW_LOG (TW_WARN, "twMap_Cleanup: failed to destroy twMap_DeleteFunctionMap");
			res = TW_UNKNOWN_ERROR;
			}
		twMap_DeleteFunctionMap = 0;
		}
	if (twMap_ParseFunctionMap){
		size_t count;
		count = cfuhash_num_entries ((cfuhash_table_t *)twMap_ParseFunctionMap);
		if (count){
			TW_LOG (TW_WARN, "twMap_Cleanup: destroying twMap_ParseFunctionMap with %u entries", count);
			res = TW_UNKNOWN_ERROR;
			}
		if (0 == s_cfuhash_destroy ((cfuhash_table_t*)twMap_ParseFunctionMap)){
			TW_LOG (TW_WARN, "twMap_Cleanup: failed to destroy twMap_ParseFunctionMap");
			res = TW_UNKNOWN_ERROR;
			}
		twMap_ParseFunctionMap = 0;
		}
	return res;
	}
	

twMap * twMap_new(){
	cfuhash_table_t *hash = cfuhash_new_with_initial_size(30);
	return (void*)hash;
}

char * twAddr2String(void* ptr){
	const static int keyLength = 50;
	char * indexKey = (char *)TW_MALLOC(keyLength+1);
	snprintf(indexKey,keyLength, "%p", ptr);
	return indexKey;
}

/*
 * Add an element to the hashmap. Return TW_MAP_OK or TW_MAP_OMEM.
 */
int twMap_put(twMap* in, const char*key, void * value){
	cfuhash_put((cfuhash_table_t *)in, key, value);
	return TW_MAP_OK;
}

/*
 * Get an element from the hashmap. Return TW_MAP_OK or TW_MAP_MISSING.
 */
int twMap_get(twMap* in, const char*key, void * *arg){
	void * val = cfuhash_get((cfuhash_table_t *)in, key);
	*arg = val;
	if(val)
		return TW_MAP_OK;
	else
		return TW_MAP_MISSING;
}

/*
 * Remove an element from the hashmap. Return TW_MAP_OK or TW_MAP_MISSING.
 */
int twMap_remove(twMap* in, const char*key){
	void * val = cfuhash_delete((cfuhash_table_t *)in, key);
	if(val == NULL){
		return TW_MAP_MISSING;
	} else {
		return TW_MAP_OK;
	}
}


/*
 * Free the hashmap
 */
void twMap_free(twMap* in){
	cfuhash_destroy((cfuhash_table_t *)in);
}

/*
 * Get the current size of a hashmap
 */
int twMap_length(twMap* in){
	return cfuhash_num_entries((cfuhash_table_t *)in);
}


/* Common Dictionary Interface */
twMap* twMapGetDeleteFunctionMap(void){
	if(NULL == twMap_DeleteFunctionMap){
		twMap_DeleteFunctionMap = twMap_new();
	}
	return twMap_DeleteFunctionMap;
}

twMap* twMapGetParseFunctionMap(void){
	if(NULL == twMap_ParseFunctionMap){
		twMap_ParseFunctionMap = twMap_new();
	}
	return twMap_ParseFunctionMap;
}


twMap * twMap_Create(map_del_func delete_function,map_key_parse_func parse_func){
	twMap *delFunctionMap,*parseFunctionMap,*newMap;
	char * hash1, * hash2;
	delFunctionMap = twMapGetDeleteFunctionMap();
	parseFunctionMap = twMapGetParseFunctionMap();

	newMap = twMap_new();
	hash1 = (char *)twAddr2String(newMap);
	hash2 = (char *)twAddr2String(newMap);

	/* Store the deletion and key parsing functions */
	if(delete_function!=NULL)
		cfuhash_put_data((cfuhash_table_t *)delFunctionMap, hash1, -1, delete_function, sizeof(delete_function), NULL);
	if(parse_func!=NULL)
		cfuhash_put_data((cfuhash_table_t *)parseFunctionMap, hash2, -1, parse_func, sizeof(parse_func), NULL);

	if (hash1) TW_FREE(hash1);
	if (hash2) TW_FREE(hash2);

	return newMap;

}

int twMap_Clear_ForEachHandler(void *key, size_t key_size, void *data, size_t data_size, void *arg){
	map_del_func del_func;
	void * val;
	char * hash1;
	twMap *delFunctionMap = twMapGetDeleteFunctionMap();
	hash1 = twAddr2String(arg);
	cfuhash_get_data((cfuhash_table_t *)delFunctionMap,hash1,-1,&val,NULL);
	if(val){
		del_func = (map_del_func)val;
		del_func(data);
	}

	if (hash1) TW_FREE(hash1);

	return TW_FOREACH_CONTINUE;
}

/* Deallocates Memory, removes all items, map itself is destroyed */
int twMap_Delete(twMap* in){
	twMap *delFunctionMap,*parseFunctionMap;
	char * hash1, * hash2;

	if(NULL == in) {
		return TW_INVALID_PARAM;
	}
	hash1 = (char *)twAddr2String(in);
	hash2 = (char *)twAddr2String(in);

	/* Call deallocate function on each item */
	cfuhash_foreach((cfuhash_table_t *)in, twMap_Clear_ForEachHandler, in);

	/* Remove from delete and parse function maps */
	delFunctionMap = twMapGetDeleteFunctionMap();
	cfuhash_delete_data((cfuhash_table_t *)delFunctionMap, hash1, -1);
	parseFunctionMap = twMapGetParseFunctionMap();
	cfuhash_delete_data((cfuhash_table_t *)parseFunctionMap, hash2, -1);

	if(0==cfuhash_destroy((cfuhash_table_t*)in)){
		return TW_INVALID_PARAM;
	}

	if (hash1) TW_FREE(hash1);
	if (hash2) TW_FREE(hash2);

	return TW_OK;
}


/* Deallocates Memory, removes all items, map remains valid */
int twMap_Clear(twMap* in){
	if(NULL == in)
		return TW_INVALID_PARAM;

	/* Call deallocate function on each item */
	cfuhash_foreach((cfuhash_table_t *)in, twMap_Clear_ForEachHandler, in);

	/* Remove and deallocate items */
	cfuhash_clear((cfuhash_table_t *)in);
	return TW_OK;
}

int twMap_Add(twMap* in, void *value){
	void * data;
	void * result;
	void *val;
	char * hash1;
	int res = TW_OK;
	size_t dataSize;
	map_del_func del_func;
	twMap * delFunctionMap;
	twMap * parseFunctionMap;
	delFunctionMap = twMapGetDeleteFunctionMap();
	parseFunctionMap = twMapGetParseFunctionMap();
	data = NULL;

	if(NULL==value){
		return TW_INVALID_PARAM;
	}

	hash1 = (char *)twAddr2String(in);

	if(cfuhash_get_data((cfuhash_table_t *)parseFunctionMap, hash1,-1,&data,&dataSize)){
		map_key_parse_func parseFunction = (map_key_parse_func)data;
		const char* key = parseFunction(value);
		if(NULL==key){
			TW_LOG(TW_ERROR, "twMap_Add: parse function returned null.");
			return TW_INVALID_PARAM;
		}

		if (!twMap_Find(in, value, &result)) {
			TW_LOG(TW_DEBUG, "twMap_Add: key already exists, deleting new entry");
			cfuhash_get_data((cfuhash_table_t *)delFunctionMap,hash1,-1,&val,NULL);
			if(val){
				del_func = (map_del_func)val;
				del_func(value);
			} else {
				TW_LOG(TW_ERROR, "twMap_Add: new entry cannot be deleted");
			}
			res = TW_ERROR_ITEM_EXISTS;
		} else {
			twMap_put(in, key, value);
		}
		TW_FREE((void*)key);
	} else {
		res = TW_INVALID_PARAM;
	}

	if (hash1) TW_FREE(hash1);

	return res;
}

int twMap_Remove(twMap* in, void * item, char deleteValue){
	void * val;
	char * hash1;
	int res = TW_UNKNOWN_ERROR;

	twMap * parseFunctionMap = twMapGetParseFunctionMap();
	hash1 = twAddr2String(in);

	if(cfuhash_get_data((cfuhash_table_t *)parseFunctionMap, hash1,-1,&val,NULL)){
		const char* key;
		map_key_parse_func parseFunction = (map_key_parse_func)val;
		key = parseFunction(item);
		if(NULL==key){
			TW_LOG(TW_ERROR, "twMap_Remove: parse function returned null.");
			return TW_INVALID_PARAM;
		}

		if(cfuhash_delete((cfuhash_table_t *)in,key)) {
			if(deleteValue){
				twMap_Clear_ForEachHandler(key, -1, item,0, in);
			}
			TW_FREE(key);
			res =  TW_OK;
		} else {
			TW_FREE(key);
			res =  TW_MAP_MISSING;
		}
	} else {
		res =  TW_INVALID_PARAM;
	}

	if (hash1) TW_FREE(hash1);
	return res;
}

size_t twMap_Foreach(twMap* in, twMap_foreach_fn listHandler, void *userData){
	return cfuhash_foreach((cfuhash_table_t *)in, listHandler, userData);
}

int twMap_Find(twMap* in, void* query, void** results){
	void * data = NULL;
	size_t dataSize = 0;
	int res = TW_UNKNOWN_ERROR;
	char * hash1 = NULL;
	twMap * parseFunctionMap = NULL;

	if(NULL == in || NULL == query){
		return TW_INVALID_PARAM;
	}
	parseFunctionMap = twMapGetParseFunctionMap();
	hash1 = twAddr2String(in);
	cfuhash_get_data((cfuhash_table_t *)parseFunctionMap, hash1,-1,&data,&dataSize);

	if(data){
		const char* key;
		map_key_parse_func parseFunction = (map_key_parse_func)data;
		key = parseFunction(query);
		if(NULL == key){
			if (hash1) TW_FREE(hash1);
			return TW_INVALID_PARAM;
		}

		if(cfuhash_get_data((cfuhash_table_t *)in, key,-1,results,NULL)) {
			TW_FREE(key);
			res = TW_OK;
		} else {
			TW_FREE(key);
			res = TW_MAP_MISSING;
		}
	} else {
		res = TW_INVALID_PARAM;
	}

	if (hash1) TW_FREE(hash1);
	return res;
}

int twMap_GetCount(twMap* in){
	return (int)cfuhash_num_entries((cfuhash_table_t *)in);
}

int twMap_ReplaceValue(twMap* in, void * value, void * new_value, char dispose){
	if(NULL == in ||NULL == value||NULL == new_value){
		return TW_INVALID_PARAM;
	}
	if(TW_OK == twMap_Remove(in, value, dispose)){
		if(TW_OK == s_twMap_Add(in,new_value)){
			return TW_OK;
		} else {
			return TW_INVALID_PARAM;
		}
	} else {
		return TW_MAP_MISSING;
	}
}
