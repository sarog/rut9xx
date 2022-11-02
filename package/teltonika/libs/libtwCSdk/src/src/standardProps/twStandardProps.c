/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include "twStandardProps.h"

extern twApi* tw_api;
static twList* tw_changeListeners_list = NULL;
static twPrimitive* safeEmptyPrimitive = NULL;

/**
 * Property dictionary entry structure to store entity name, property name, and primitive
 */
typedef struct twStandardPropEntry {
	const char *entityName;
	const char *propertyName;
	twPrimitive *value;
} twStandardPropEntry;

/**
 * Creates a property entry, copies all data
 */
twStandardPropEntry *twStandardPropEntry_Create(const char *entityName, const char *propertyName, twPrimitive *value) {
	twStandardPropEntry *entry = NULL;
	entry = TW_MALLOC(sizeof(twStandardPropEntry));
	if (!entry) return NULL;
	entry->entityName = duplicateString(entityName);
	entry->propertyName = duplicateString(propertyName);
	entry->value = twPrimitive_FullCopy(value);
	return entry;
}

/**
 * Deletes a property entry, frees all allocated data
 */
void twStandardPropEntry_Delete(twStandardPropEntry *entry) {
	if (!entry) return;
	if (entry->entityName) TW_FREE(entry->entityName);
	if (entry->propertyName) TW_FREE(entry->propertyName);
	if (entry->value) twPrimitive_Delete(entry->value);
	TW_FREE(entry);
}

/**
 * Parse function for property dictionary entries, tracks entries using entity name and property name
 */
const char *twStandardPropEntry_Parse(void *item) {
	int bufferSize = 0;
	char *buffer;
	twStandardPropEntry *entry = (twStandardPropEntry *)item;
	bufferSize = strnlen(entry->entityName, 256) + strnlen(entry->propertyName, 256) + 2;
	buffer = TW_MALLOC(bufferSize);
	if (!buffer) return NULL;
	snprintf(buffer, bufferSize,"%s:%s", entry->entityName, entry->propertyName);
	return buffer;
}

/**
 * Returns the master hash map of thingname & property name to a property's primitive value.
 */
twDict* twGetPropertyDict(){
    if(NULL == tw_api->tw_property_dict){
        tw_api->tw_property_dict = twDict_Create(twStandardPropEntry_Delete, twStandardPropEntry_Parse);
    }
    return tw_api->tw_property_dict;
}

twMap* twGetSafeEmptyPrimitive(){
    if(NULL == safeEmptyPrimitive){
        safeEmptyPrimitive = twPrimitive_Create();
    }
    return safeEmptyPrimitive;
}

void twChangeListeners_Delete(void *item) {
    twPropertyChangeRecord * tmp = (twPropertyChangeRecord *) item;
    if (!item) {return;}
    if (tmp->entityName) TW_FREE(tmp->entityName);
    if (tmp->propertyName) TW_FREE(tmp->propertyName);
    TW_FREE(tmp);
}

twList* twExt_GetChangeListenersList(){
    if(NULL == tw_changeListeners_list){
        tw_changeListeners_list = twList_Create(twChangeListeners_Delete);
    }
    return tw_changeListeners_list;
}

void twNotifyPropertyChangeListeners(const char *entityName, const char *propertyName, twPrimitive *newValue) {
    twList* changeListeners = twExt_GetChangeListenersList();
    struct ListEntry * node = NULL;
    twPropertyChangeRecord * val = NULL;
    node = changeListeners->first;
    while (node) {
        val = node->value;
        if(strcmp(val->entityName,entityName)==0){

            if(val->propertyName==NULL||strcmp(val->propertyName,propertyName)==0) {
				twPrimitive* copyForListener = twPrimitive_FullCopy(newValue);
                val->listenerFunction(entityName, propertyName, copyForListener);
				twPrimitive_Delete(copyForListener);
                break;
            }
        }
        node = node->next;
    }
}

int twExt_SetPropertyValue(const char *thingName, const char *propertyName, twPrimitive *value, char fold, char push){
    int res = TW_OK;
	twStandardPropEntry *query = NULL;
	twStandardPropEntry *entry = NULL;

	/* Create a query to look up the property dictionary entry */
	query = twStandardPropEntry_Create(thingName, propertyName, NULL);

	if (TW_OK == twDict_Find(twGetPropertyDict(), query, &entry)) {
		/* If we found an entry for this thingName/propertyName, replace the primitive value */
		twPrimitive_Delete(entry->value);
		entry->value = twPrimitive_FullCopy(value);
	} else {
		/* If we didn't find an entry for this thingName/propertyName, add one */
		entry = twStandardPropEntry_Create(thingName, propertyName, value);
		twDict_Add(twGetPropertyDict(), entry);
	}

	/* Delete the query once we're done looking up the entry */
	twStandardPropEntry_Delete(query);

	/* Record change for delivery to server */
	res = twApi_SetSubscribedProperty((char*)thingName,(char*)propertyName, twPrimitive_FullCopy(value), fold, push);
	if (TW_OK == res)
		twNotifyPropertyChangeListeners(thingName, propertyName, value);

	return res;
}

char twExt_PropertyExists(const char *thingName, char *propertyName) {
	twStandardPropEntry *result = NULL;
	twStandardPropEntry *query = NULL;
	char found = FALSE;

	/* Create a query to look up the property dictionary entry */
	query = twStandardPropEntry_Create(thingName, propertyName, NULL);
	if (TW_OK == twDict_Find(twGetPropertyDict(), query, &result)) found = TRUE;
	/* Delete the query once we're done looking up the entry */
	twStandardPropEntry_Delete(query);

	return found;
}

twPrimitive* twExt_GetPropertyValue(const char *thingName, char *propertyName){
	twStandardPropEntry *result = NULL;
	twStandardPropEntry *query = NULL;
	twPrimitive *value = NULL;

	/* Create a query to look up the property dictionary entry */
	query = twStandardPropEntry_Create(thingName, propertyName, NULL);
	if (TW_OK == twDict_Find(twGetPropertyDict(), query, &result)) value = result->value;
	else value = twGetSafeEmptyPrimitive();
	/* Delete the query once we're done looking up the entry */
	twStandardPropEntry_Delete(query);

	return value;
}

enum msgCodeEnum twExt_StandardPropertyHandler(const char *entityName, const char *propertyName, twInfoTable **itValue,
                                               char isWrite, void *userdata) {

    /* Verify Allocation of Global property hashmap
     If you never invoke a property handler you should not have to have a map */
    twPrimitive* currentPropertyValue;
    char* dontFold = FALSE;
    char push = TRUE;

    TW_LOG(TW_TRACE,"twExt_StandardPropertyHandler() Function called for Entity %s, Property %s", entityName, propertyName);
    if (itValue) {
        if (isWrite && *itValue) {

            /* Set a new value */
            if(TW_ERROR_GETTING_PRIMITIVE == twInfoTable_GetPrimitive(*itValue, propertyName, 0, &currentPropertyValue)){
                return TWX_NOT_FOUND;
            }

            twExt_SetPropertyValue(entityName, propertyName, currentPropertyValue, dontFold, push);

        } else {

            /* Get current value and return it */
            TW_LOG(TW_TRACE,"twExt_StandardPropertyHandler() Read using map %llu for Entity %s, Property %s", twGetPropertyDict(), entityName, propertyName);
            if(twExt_PropertyExists(entityName, (char*)propertyName)) {
                *itValue = twInfoTable_CreateFromPrimitive(propertyName,
                       twPrimitive_FullCopy(twExt_GetPropertyValue(entityName, (char*)propertyName))
                );
            } else {
                return TWX_NOT_FOUND;
            }
        }
        return TWX_SUCCESS;
    } else {
        TW_LOG(TW_ERROR,"twExt_StandardPropertyHandler() - NULL pointer for value");
        return TWX_BAD_REQUEST;
    }

}

void twExt_AddPropertyChangeListener(char *entityName, char *propertyName,
                                     PropertyChangeListenerFunction propertyChangeListenerFunction){
    twList* changeListeners = twExt_GetChangeListenersList();

    twPropertyChangeRecord * propertyChangeRecord = NULL;
    if (!entityName) {
        TW_LOG(TW_ERROR,"twExt_AddPropertyChangeListener: NULL entity pointer passed in.");
        return;
    }
    propertyChangeRecord = (twPropertyChangeRecord *)TW_CALLOC(sizeof(twPropertyChangeRecord), 1);
    if (!propertyChangeRecord) {
        TW_LOG(TW_ERROR,"twExt_AddPropertyChangeListener: Error allocating memory");
        return;
    }

    /* populate it */
    propertyChangeRecord->listenerFunction = propertyChangeListenerFunction;
    propertyChangeRecord->entityName = duplicateString(entityName);
    if(propertyName!=NULL)
        propertyChangeRecord->propertyName = duplicateString(propertyName);
    else
        propertyChangeRecord->propertyName = NULL;

    twList_Add(changeListeners, propertyChangeRecord);
}

void twExt_RemovePropertyChangeListener(PropertyChangeListenerFunction propertyChangeListenerFunction){
    twList* changeListeners = twExt_GetChangeListenersList();
    struct ListEntry * node = NULL;
    twPropertyChangeRecord * val = NULL;
    node = changeListeners->first;
    while (node) {
        val = node->value;
        if(val->listenerFunction == propertyChangeListenerFunction){
            twList_Remove(changeListeners, node,TRUE);
            break;
        }
        node = node->next;
    }
}
