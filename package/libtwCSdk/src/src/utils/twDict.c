/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/
#include <memory.h>
#include "twErrors.h"
#include "twDict.h"
#include "twMap.h"
#include "twList.h"
#include "twOSPort.h"

#ifndef TW_DICTIONARY_MODE
#define TW_DICTIONARY_MODE TW_DICTIONARY_MAP
#endif
twList *twList_CreateSearchable(del_func delete_function, parse_func parse_function);

twDictionaryMode tw_dictionary_mode=TW_DICTIONARY_MODE;

/***** setting twDictionaryMode at runtime should be reserved for testing only! *****/
TW_MUTEX twDictionaryModeMtx = NULL;

int twDict_Cleanup (){
	return TW_DICTIONARY_MAP == tw_dictionary_mode ? twMap_Cleanup () : twList_Cleanup ();
	}

void twDictionaryModeMtx_Create() {
	if (NULL == twDictionaryModeMtx) {
		/* create the mutex */
		twDictionaryModeMtx = twMutex_Create();
	}
}

void twDictionaryModeMtx_Delete() {
	/* create the mutex */
	if (twDictionaryModeMtx) {
		twMutex_Lock(twDictionaryModeMtx);
		twMutex_Delete(twDictionaryModeMtx);
		twDictionaryModeMtx = NULL;
	}
}

int twDict_setDictionaryMode(twDictionaryMode dictionaryMode){
	if (twDictionaryModeMtx) {
		twMutex_Lock(twDictionaryModeMtx);
		tw_dictionary_mode = dictionaryMode;
		twMutex_Unlock(twDictionaryModeMtx);
		return TW_OK;
	} else {
		return TW_UNKNOWN_ERROR;
	}
}
/***** setting twDictionaryMode at runtime should be reserved for testing only! *****/

twDict * twDict_Create(dict_del_func delete_function,dict_key_parse_func parse_function){
	if(TW_DICTIONARY_LIST == tw_dictionary_mode){
		return twList_CreateSearchable((del_func) delete_function,parse_function);
	} else {
		return twMap_Create((map_del_func)delete_function,(map_key_parse_func) parse_function);
	}
}

int twDict_Delete(twDict* in){
	if(TW_DICTIONARY_LIST == tw_dictionary_mode){
		return twList_Delete((twList*)in);
	} else {
		return twMap_Delete((twMap*)in);
	}
}

int twDict_Clear(twDict* in){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_Clear((twList *) in);
	} else {
		return twMap_Clear((twMap*)in);
	}
}

int twDict_Add(twDict* in, void *value){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_Add((twList*)in,value);
	} else {
		return twMap_Add((twMap*)in,value);
	}
}

typedef struct twDict_Remove_ForEachHandlerParams {
	void * queryValue;
	ListEntry* matchingListItem;
	parse_func itemParseFunction;
} twDict_Remove_ForEachHandlerParams;

int twDict_Remove_ForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *userData){
	/* Compare Parse Function on data to parse function on query */
	char* keyForData;
	char* keyForQuery;
	ListEntry* currentListItem = (ListEntry*)key;
	twDict_Remove_ForEachHandlerParams* params = (twDict_Remove_ForEachHandlerParams*)userData;
	keyForData = params->itemParseFunction(data);
	keyForQuery = params->itemParseFunction(params->queryValue);
	if(strncmp(keyForData,keyForQuery,sizeof(keyForQuery))==0){
		params->matchingListItem = currentListItem;
		TW_FREE(keyForData);
		TW_FREE(keyForQuery);
		return TW_FOREACH_EXIT;
	}
	TW_FREE(keyForData);
	TW_FREE(keyForQuery);
	return TW_FOREACH_CONTINUE;

}

int twDict_Remove(twDict* in, void * item, char deleteValue){
	int res = TW_OK;
	twList* listFromDict = NULL;
	ListEntry * listItemToRemove = NULL;
	twDict_Remove_ForEachHandlerParams* params = NULL;

	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		if(NULL == item)
			return TW_INVALID_PARAM;
		/* You must have a parse function for this to work */
		listFromDict = (twList*)in;
		if(NULL == listFromDict->parse_function){
			return TW_INVALID_PARAM;
		}

		/* Assuming item is a value to delete, Search entire list for a ListItem* to which maps to the key of item */
		params = TW_MALLOC(sizeof(twDict_Remove_ForEachHandlerParams));
		params->queryValue = item;
		params->itemParseFunction = listFromDict->parse_function;
		params->matchingListItem = NULL;

		twList_Foreach((twList*)in,twDict_Remove_ForEachHandler,(void*)params);

		res = twList_Remove((twList*)in, params->matchingListItem, deleteValue);
	} else {
		if (NULL == item) {
			return TW_INVALID_PARAM;
			}
		res = twMap_Remove((twMap*)in,item,deleteValue);
	}
	
	TW_FREE(params);
	return res;
}

size_t twDict_Foreach(twDict* in, twDict_foreach_fn listHandler, void *userData){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_Foreach((twList*)in,(twForEachHander)listHandler,userData);
	} else {
		return twMap_Foreach(in,(twMap_foreach_fn)listHandler,userData);
	}
}

int twDict_Find(twDict* in, void* query, void** results){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_Find((twList*)in,query,results);
	} else {
		return twMap_Find((twMap*)in,query,results);
	}
}

int twDict_GetCount(twDict* in){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_GetCount((twList*)in);
	} else {
		return twMap_GetCount((twMap*)in);
	}
}

int twDict_ReplaceValue(twDict* in, void * value, void * new_value, char dispose){
	if(TW_DICTIONARY_LIST==tw_dictionary_mode){
		return twList_ReplaceValue((twList*)in,value,new_value,dispose);
	} else {
		return twMap_ReplaceValue((twMap*)in,value,new_value,dispose);
	}
}

