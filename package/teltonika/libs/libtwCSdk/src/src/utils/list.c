/*
 *  Copyright 2016, PTC, Inc.
 *
 *  Doubly linked list utilities
 */

#include "twList.h"
#include "twOSPort.h"
#include "twApiStubs.h"

#ifdef _DEBUG
#include "twLogger.h"
#include "twMessaging.h"
#include "twApi.h"
extern twApi * tw_api;
#endif

int twList_Cleanup () {
	/* no-op */
	return TW_OK;
	}

void twList_Unlock(struct twList *list) {
	if(twApi_stub) {
		s_twMutex_Unlock(list->mtx);
	} else {
		twMutex_Unlock(list->mtx);
	}
}

void twList_Lock(struct twList *list) {
	if(twApi_stub) {
		s_twMutex_Lock(list->mtx);
	} else {
		twMutex_Lock(list->mtx);
	}
}

twList *twList_CreateSearchable(del_func delete_function, parse_func parse_function) {
	twList *list = (twList *) TW_CALLOC(sizeof(twList), 1);
	if (list) {
		list->mtx = s_twMutex_Create();
		if (!list->mtx) {
			TW_LOG(TW_ERROR, "twList_CreateSearchable: Error allocating mutex");
			twList_Delete(list);
			return NULL;
		}
		list->delete_function = delete_function;
		list->parse_function = parse_function;
	}
	return list;
}

twList *twList_Create(del_func delete_function) {
	return twList_CreateSearchable(delete_function,NULL);
}

int twList_Delete(struct twList * list) {
	if (list) {
		twList_Clear(list);
		twMutex_Delete(list->mtx);
		TW_FREE(list);
		return TW_OK;
	}
	return TW_INVALID_PARAM;
}


int twList_Clear(struct twList *list) {
	struct ListEntry * entry = NULL;
	if (list) {
		if(0 == list->count)
			return TW_OK;
		twList_Lock(list);
		entry = list->first;
		while (entry) {
			ListEntry * tmp = entry->next;
			if (list->delete_function) {
				list->delete_function(entry->value);
			} else {
				TW_FREE(entry->value);
			}
			TW_FREE(entry);
			entry = tmp;
		}
		list->count = 0;
		list->first = NULL;
		list->last = NULL;
		twList_Unlock(list);
		return TW_OK;
	}
	return TW_INVALID_PARAM;
}

int twList_ClearEntries(struct twList *list) {
	struct ListEntry * entry = NULL;
	if (list) {
		if(0 == list->count)
			return TW_OK;
		twList_Lock(list);
		entry = list->first;
		while (entry) {
			ListEntry * tmp = entry->next;
			TW_FREE(entry);
			entry = tmp;
		}
		list->count = 0;
		list->first = NULL;
		list->last = NULL;
		twList_Unlock(list);
		return TW_OK;
	}
	return TW_INVALID_PARAM;
}

int twList_Add(struct twList *list, void *value) {
	struct ListEntry * newEntry = NULL;
	if (list) {
		newEntry = (ListEntry *)TW_CALLOC(sizeof(ListEntry), 1);
		if (!newEntry) return TW_ERROR_ALLOCATING_MEMORY;
		newEntry->value = value;
		/* Find the last entry */
		twList_Lock(list);
		if (!list->first) {
			/* This will be the first entry in the list */
			list->first = newEntry;
			list->last = newEntry;
			newEntry->prev = NULL;
			newEntry->next = NULL;
		} else {
			newEntry->prev = list->last;
			newEntry->next = NULL;
			list->last->next = newEntry;
			list->last = newEntry;
		}
		list->count++;
		twList_Unlock(list);
		return TW_OK;
	}
	return TW_INVALID_PARAM;
}

int twList_Remove(struct twList *list, struct ListEntry * entry, char deleteValue) {
	struct ListEntry * node = NULL;
	void * val = NULL;
	if (!list || !entry) return TW_INVALID_PARAM;
	/* find the entry */
	twList_Lock(list);
	node = list->first;
	while (node) {
		if (node == entry) {
			val = node->value;
			if (node == list->first) list->first = node->next;
			if (node == list->last) list->last = node->prev;
			if (node->prev) node->prev->next = node->next;
			if (node->next) node->next->prev = node->prev;
			break;
		}
		node = node->next;
	}
	if (deleteValue && val) {
		if (list->delete_function) {
			list->delete_function(val);
		} else TW_FREE(val);
	}
	TW_FREE (entry);
	list->count--;
	twList_Unlock(list);
	return TW_OK;
}

ListEntry * twList_Next(twList *list, ListEntry * entry) {
	struct ListEntry * node = NULL;
	if (!list || list->count == 0) return NULL;
	/* find the entry */
	/* TW_LOG(TW_TRACE,"The twList_Next() function has been deprecated. Please use twList_Foreach().");*/
	twList_Lock(list);
	node = list->first;
	/* If entry is NULL just return the first entry in the list */
	/* seems simple, but if this or the "next" element has been removed, it isn't */
	if (entry) {
		while (node) {
			if (node == entry) {
				/* the passed in entry still exists */
				node = node->next;
				break;
			}
			node = node->next;
		}
	}
	twList_Unlock(list);
	return node;
}

int twList_Foreach(twList *list, twForEachHander listHandler, void *userData){
	struct ListEntry * node = NULL;
	int processedCount = 0;
	if (!list || list->count == 0)
		return 0;

	twList_Lock(list);
	node = list->first;
	while (node) {
		processedCount++;
		if(listHandler(node,sizeof(node),node->value,sizeof(node->value),userData)!=0)
			break;
		node = node->next;
	}
	twList_Unlock(list);
	return processedCount;
}

ListEntry * twList_GetByIndex(struct twList *list, int index) {
	ListEntry * le = NULL;
	int count = 0;
	if (!list) return NULL;
	if (index >= list->count) return NULL;
	twList_Lock(list);
	le = list->first;
	while (le) {
		if (count++ == index){
			twList_Unlock(list);
			return le;
		}
		le = le->next;
	}
	twList_Unlock(list);
	return NULL;
}

int twList_GetCount(struct twList *list) {
	int count = 0;
	if (list) {
		ListEntry * le = NULL;
		twList_Lock(list);
		le = list->first;
		while (le) {
			count++;
			le = le->next;
		}
		twList_Unlock(list);
	}
	return count;
}


int twList_ReplaceValue(struct twList *list, struct ListEntry * entry, void * new_value, char dispose) {
	struct ListEntry * node = NULL;
	if (!list || list->count == 0 || !entry) return TW_INVALID_PARAM;
	/* find the entry */
	twList_Lock(list);
	node = list->first;
	/* If entry is NULL just return the first entry in the list */
	/* seems simple, but if this or the "next" element has been removed, it isn't */
	if (entry) {
		while (node) {
			if (node == entry) {
				if (dispose) {
					if (list->delete_function) list->delete_function(node->value);
					else TW_FREE(node->value);
				}
				node->value = new_value;
				twList_Unlock(list);
				return TW_OK;
			}
			node = node->next;
		}
	}
	twList_Unlock(list);
	return TW_LIST_ENTRY_NOT_FOUND;
}

typedef struct twList_FindForEachHandlerParams {
	parse_func listParseFunction;
	char* searchKey;
	void* match;
	void* listEntry;
} twList_FindForEachHandlerParams;

int twList_FindForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	char * currentKey;
	twList_FindForEachHandlerParams* params = (twList_FindForEachHandlerParams*)userData;
	currentKey = params->listParseFunction(currentValue);
	if(strcmp(currentKey,params->searchKey)==0){
		if(currentKey) TW_FREE(currentKey);
		params->match = currentValue;
		params->listEntry = key;
		return TW_FOREACH_EXIT;
	}
	if(currentKey) TW_FREE(currentKey);
	return TW_FOREACH_CONTINUE;
}

int twList_Find(twList* list, void* query, void** results){
	int res;
	twList_FindForEachHandlerParams* params;
	if(NULL == list->parse_function){
		return TW_MAP_MISSING;
	}
	params = (twList_FindForEachHandlerParams*)TW_MALLOC(sizeof(twList_FindForEachHandlerParams));
	params->listParseFunction = list->parse_function;
	params->searchKey = list->parse_function(query);
	params->match = NULL;
	twList_Foreach(list, twList_FindForEachHandler, (void*)params);
	if(params->match){
		*results=params->listEntry;
		res = TW_OK;
	} else {
		*results=NULL;
		res = TW_MAP_MISSING;
	}
	TW_FREE(params->searchKey);
	TW_FREE(params);
	return res;
}

#ifdef _DEBUG
void twList_CheckTask(DATETIME now, void * params) {
	static uint32_t callbackListSize = 0;
	static uint32_t offlineMessageListSize = 0;
	static uint32_t offlineMessageSize = 0;
	static uint32_t responseCallbackListSize = 0;
	static uint32_t requestCallbackListSize = 0;
	static uint32_t multipartMessageListSize = 0;

	twMessageHandler * tmh = twMessageHandler_Instance((twWs *) NULL);

	if(tw_api && tw_api->mtx) {
		twMutex_Lock(tw_api->mtx);
		callbackListSize = twDict_GetCount(tw_api->callbackList);
		offlineMessageListSize = twList_GetCount(tw_api->offlineMsgList);
		offlineMessageSize = tw_api->offlineMsgSize;
		twMutex_Unlock(tw_api->mtx);
	}

	if(tmh && tmh->mtx) {
		twMutex_Lock(tmh->mtx);
		requestCallbackListSize = twList_GetCount(tmh->incomingRequestCallbacks);
		responseCallbackListSize = twList_GetCount(tmh->responseCallbackList);
		multipartMessageListSize = twList_GetCount(tmh->multipartMessageList);
		twMutex_Unlock(tmh->mtx);
	}

	TW_LOG(TW_INFO, "List Sizes: callback list [%lu], offline message list [%lu], offline message size [%lu]",
			callbackListSize, offlineMessageListSize, offlineMessageSize);
	TW_LOG(TW_INFO, "List Sizes: requests list [%lu], response list [%lu], multipart list [%lu]",

	requestCallbackListSize, responseCallbackListSize, multipartMessageListSize);
}
#endif
