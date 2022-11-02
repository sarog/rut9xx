/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Thingworx Subscribed Properties
 */

#include "twSubscribedProps.h"
#include "twOfflineMsgStore.h"
#include "twProperties.h"
#include "twApiStubs.h"

#include <limits.h>
#include <math.h>

/* Reference to tw_api */
extern twApi * tw_api;

/***************************************/
/*     Subscribed Property struct      */
/***************************************/
#define TW_UPDATE_ALWAYS 0
#define TW_UPDATE_NEVER 1
#define TW_UPDATE_VALUE 2
#define TW_UPDATE_ON 3
#define TW_UPDATE_OFF 4
#define TW_UPDATE_DEADBAND 5

#define SPM_PERSIST_PROPERTIES_PERMIT TRUE
#define SPM_PERSIST_PROPERTIES_PREVENT FALSE
#define SPM_CALL_BLOCKING FALSE
#define SPM_CALL_ASYNCHRONOUS TRUE
#define SPM_CALLBACK_NONE NULL
#define SPM_CONNECT_DONT_FORCE FALSE
#define SPM_NO_MESSAGE_ID_LIST NULL
#define SPM_ENTITIES_ALL NULL

#define MAX_FIELD_LEN 1024
#define MAX_PATH_LEN 4096

const char* twSubscribedProperty_Parse (void * item);


void twEntityCurrentValues_Delete(void *entity) {
	twEntityCurrentValues *e = (twEntityCurrentValues *)entity;
	if (!e) return;
	if (e->name) TW_FREE(e->name);
	if (e->props) twDict_Delete(e->props);
	TW_FREE(e);
}

const char *twEntityCurrentValues_Parse(void *entity) {
	twEntityCurrentValues *e = (twEntityCurrentValues *)entity;
	return duplicateString(e->name);
}

int twSubscribedProperty_ToStream(twSubscribedProperty * p, twStream * s) {
	int res = TW_UNKNOWN_ERROR;
	twInfoTableRow * row = NULL;
	if (!p || !p->prop || !s) return TW_INVALID_PARAM;
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(p->entity, TRUE));
	if (!row)
		return TW_ERROR_ALLOCATING_MEMORY;

	if (twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(p->prop->name, TRUE))) {
		twInfoTableRow_Delete(row);
		return TW_ERROR;
	}

	if (twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(p->prop->value))) {
		twInfoTableRow_Delete(row);
		return TW_ERROR;
	}
	if (twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(p->prop->timestamp))) {
		twInfoTableRow_Delete(row);
		return TW_ERROR;
	}
	if (twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(p->prop->quality, TRUE))) {
		twInfoTableRow_Delete(row);
		return TW_ERROR;
	}
	if (twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(p->fold))) {
		twInfoTableRow_Delete(row);
		return TW_ERROR;
	}
	res = twInfoTableRow_ToStream(row, s);
	twInfoTableRow_Delete(row);
	return res;
}

uint32_t  twSubscribedProperty_GetSize(twSubscribedProperty *p) {
	uint32_t len = 0;
	if (!p || !p->prop || !p->prop->name || !p->prop->quality || !p->prop->value || !p->entity) return UINT_MAX;
	/* in order to properly calculate the size of the queue we must account for all allocated data */
	len += sizeof(twSubscribedProperty); /* account for twSubscribedProperty struct size */
	len += strnlen(p->entity, MAX_FIELD_LEN) + 1; /* account for twSubscribedProperty->entity string size */
	len += sizeof(twProperty); /* account for twSubscribedProperty->prop (twProperty) struct size */
	len += strnlen(p->prop->name, MAX_FIELD_LEN) + 1; /* account for twSubscribedProperty->prop->name (twProperty->name) string size */
	len += strnlen(p->prop->quality, MAX_FIELD_LEN) + 1; /* account for twSubscribedProperty->prop->quality (twProperty->quality) string size */
	len += sizeof(twPrimitive); /* account for twSubscribedProperty->prop->value (twPrimitive) struct size */
	len += p->prop->value->length; /* account for twSubscribedProperty->prop->value->data (twPrimitive->data) size */

	return len;
}

void twSubscribedProperty_Delete(void * prop) {
	twSubscribedProperty * p = (twSubscribedProperty *)prop;
	if (!prop) return;
	if (p->entity) TW_FREE((void *)p->entity);
	if (p->prop) twProperty_Delete(p->prop);
	TW_FREE(p);
}

twSubscribedProperty * twSubscribedProperty_Create (char * e, char * n, twPrimitive * v, DATETIME t, char * q, char fold) {
	twSubscribedProperty * tmp = NULL;
	if (!e || !n || !v) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_Create: NULL parameter passed in");
		return NULL;
	}
	tmp = (twSubscribedProperty *)TW_CALLOC(sizeof(twSubscribedProperty), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_Create: Error allocating memory");
		return NULL;
	}
	tmp->entity = duplicateString(e);
	tmp->prop = twPropertyVTQ_Create(n,twPrimitive_FullCopy(v),t,q);
	tmp->fold = fold;
	tmp->foldedPropertyNeedsUpdate = FALSE;
	if (!tmp->entity || !tmp->prop) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_Create: Error allocating memory");
		twSubscribedProperty_Delete(tmp);
		return NULL;
	}
	return tmp;
}

twSubscribedProperty * twSubscribedProperty_CreateFromStream(twStream * s) {
	twSubscribedProperty * tmp = NULL;
	twPrimitive * p;
	twInfoTableRow * row = NULL;
	int i = 0;
	if (!s) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	tmp = (twSubscribedProperty *)TW_CALLOC(sizeof(twSubscribedProperty), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Error allocating memory");
		return NULL;
	}
	tmp->prop = (twProperty *)TW_CALLOC(sizeof(twProperty), 1);
	if (!tmp->prop) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Error allocating memory for twProperty");
		return NULL;
	}
	/* We have stored this as an infotable row */
	row = twInfoTableRow_CreateFromStream(s);
	if (!row) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Error creating row from stream");
		twSubscribedProperty_Delete(tmp);
		return NULL;
	}
	/* Sanity check */
	if (twInfoTableRow_GetCount(row) != 6) {
		TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Invalid primitive count in row");
		twSubscribedProperty_Delete(tmp);
		return NULL;
	}
	for (i = 0; i < 6; i++) {
		char err = FALSE;
		p = twInfoTableRow_GetEntry(row, i);
		if (!p) {
			TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Error getting primitive from row %d", i);
			twSubscribedProperty_Delete(tmp);
			return NULL;
		}
		switch (i) {
			case 0:
				if (p->type != TW_STRING) {
					err = TRUE;
					break;
				}
				tmp->entity = duplicateString(p->val.bytes.data);
				break;
			case 1:
				if (p->type != TW_STRING) {
					err = TRUE;
					break;
				}
				tmp->prop->name = duplicateString(p->val.bytes.data);
				break;
			case 2:
				tmp->prop->value = twPrimitive_FullCopy(p);
				break;
			case 3:
				if (p->type != TW_DATETIME) {
					err = TRUE;
					break;
				}
				tmp->prop->timestamp = p->val.datetime;
				break;
			case 4:
				if (p->type != TW_STRING) {
					err = TRUE;
					break;
				}
				tmp->prop->quality = duplicateString(p->val.bytes.data);
				break;
			case 5:
				if (p->type != TW_BOOLEAN) {
					err = TRUE;
					break;
				}
				tmp->fold = p->val.boolean;
				break;
		}
		if (err) {
			TW_LOG(TW_ERROR,"twSubscribedProperty_CreateFromStream: Invalid primitive type in row");
			twInfoTableRow_Delete(row);
			twSubscribedProperty_Delete(tmp);
			return NULL;
		}
	}
	twInfoTableRow_Delete(row);
	return tmp;
}

void twEntitySavedProperties_Delete(void *entity) {
	twEntitySavedProperties * e = (twEntitySavedProperties *)entity;
	if (!e) return;
	if (e->name) TW_FREE(e->name);
	if (e->props) twList_Delete(e->props);
	TW_FREE(e);
}

const char* twEntitySavedProperties_Parse(void *entity){
	twEntitySavedProperties * e = (twEntitySavedProperties *)entity;
	if(e->name == NULL)
		return NULL;
	return duplicateString(e->name);
}

twEntitySavedProperties * twEntitySavedProperties_Create(char *name) {
	twEntitySavedProperties * e = NULL;
	if (!name) return NULL;
	e = (twEntitySavedProperties *)TW_CALLOC(sizeof(twEntitySavedProperties), 1);
	if (!e) return NULL;
	e->name = duplicateString(name);
	e->props = twList_Create(twSubscribedProperty_Delete);
	if (!e->name || !e->props) {
		twEntitySavedProperties_Delete(e);
		return NULL;
	}
	return e;
}


/* Our singleton */
#ifdef WIN32
__declspec(dllexport) twSubscribedPropsMgr * spm = NULL;
#else
twSubscribedPropsMgr * spm = NULL;
#endif

twSubscribedPropsMgr * twSubscribedPropsMgr_Get(){
	return spm;
}

#define MSG_OVERHEAD 100

int getSPMSavedValuesCount(const char * thingName) {
	int res = -1;
	twEntitySavedProperties * match;
	twEntitySavedProperties query;
	query.name = (char*)thingName;
	if (TW_OK == twDict_Find( spm->savedValues ,& query,(void*)&match) ){
		 res= match->props->count;

	}
	return res;
}

int getSPMPersistedValuesLength() {
	int res = -1;
	if (spm && spm->persistedValues) {
		res = spm->persistedValues->length;
	}
	return res;
}

twSubscribedPropsMgr* getSPM() {
	return spm;
}

const char* twSubscribedProperty_Parse(void * item){
	int bufferSize=0;
	char * buffer;
	twSubscribedProperty * subscribedProperty = (twSubscribedProperty * )item;
	bufferSize = strnlen(subscribedProperty->entity,256)+strnlen(subscribedProperty->prop->name,256)+2;
	buffer=(char*)TW_MALLOC(bufferSize);
	snprintf(buffer,bufferSize,"%s:%s",subscribedProperty->entity,subscribedProperty->prop->name);
	return buffer;
}

twDict *twSubscribedPropsMgr_getEntityCurrentValuesDict(char *entityName) {
	twEntityCurrentValues *match;
	twEntityCurrentValues query;
	query.name = entityName;
	if (TW_OK == twDict_Find(spm->currentValues, &query, (void**)&match)) {
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_getEntityCurrentValuesDict: Found currentValues for entity %s", entityName);
		return match->props;
	}
	TW_LOG(TW_TRACE, "twSubscribedPropsMgr_getEntityCurrentValuesDict: Could not find currentValues for entity %s", entityName);
	return NULL;
}

twSubscribedProperty *twSubscribedPropsMgr_getPropertyCurrentValueFromEntityDict(twDict *entityCurrentValuesDict, char *entityName,
																	  char *propertyName) {
	twSubscribedProperty *query;
	twSubscribedProperty *result;

	query = TW_MALLOC(sizeof(twSubscribedProperty));
	query->prop = TW_MALLOC(sizeof(twProperty));
	query->entity = entityName;
	query->prop->name = propertyName;
	if (TW_OK == twDict_Find(entityCurrentValuesDict, query, (void**)&result)) {
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_getPropertyCurrentValue: Found currentValue for property %s", propertyName);
		TW_FREE(query->prop);
		TW_FREE(query);
		return result;
	}
	TW_LOG(TW_TRACE, "twSubscribedPropsMgr_getPropertyCurrentValue: Could not find currentValue for property %s", propertyName);
	TW_FREE(query->prop);
	TW_FREE(query);
	return NULL;
}

twSubscribedProperty *twSubscribedPropsMgr_getPropertyCurrentValue(char *entityName, char *propertyName) {
	twDict *entityCurrentValuesDict = NULL;
	twSubscribedProperty *entry = NULL;

	entityCurrentValuesDict = twSubscribedPropsMgr_getEntityCurrentValuesDict(entityName);
	if (!entityCurrentValuesDict) {
		return NULL;
	}
	entry = twSubscribedPropsMgr_getPropertyCurrentValueFromEntityDict(entityCurrentValuesDict, entityName,
																	   propertyName);
	if (entry) {
		return entry;
	}
	return NULL;
}

int twSubscribedPropsMgr_replacePropertyCurrentValue(char *entityName, char *propertyName, twSubscribedProperty *old,
													 twSubscribedProperty *new) {
	twDict *entityCurrentValuesDict = NULL;
    twSubscribedProperty *entry = NULL;
	entityCurrentValuesDict = twSubscribedPropsMgr_getEntityCurrentValuesDict(entityName);
	entry = twSubscribedPropsMgr_getPropertyCurrentValueFromEntityDict(entityCurrentValuesDict, entityName,
																	   propertyName);
	return twDict_ReplaceValue(entityCurrentValuesDict, entry, new, TRUE);
}

int twSubscribedPropsMgr_removePropertyCurrentValue(char *entityName, char *propertyName) {
	twDict *entityCurrentValuesDict = NULL;
    twSubscribedProperty *entry = NULL;

	entityCurrentValuesDict = twSubscribedPropsMgr_getEntityCurrentValuesDict(entityName);
	if (!entityCurrentValuesDict) {
		return TW_ERROR_ITEM_DOES_NOT_EXIST;
	}
	entry = twSubscribedPropsMgr_getPropertyCurrentValueFromEntityDict(entityCurrentValuesDict, entityName,
																	   propertyName);
	if (!entry) {
		return TW_OK;
		}
	return twDict_Remove(entityCurrentValuesDict, entry, TRUE);
}

int twSubscribedPropsMgr_addPropertyCurrentValue(char *entityName, twSubscribedProperty *prop) {
	twDict *entityCurrentValuesDict = NULL;

	entityCurrentValuesDict = twSubscribedPropsMgr_getEntityCurrentValuesDict(entityName);
	if (!entityCurrentValuesDict) {
		twEntityCurrentValues *e = NULL;
		TW_LOG(TW_TRACE, "Creating new current value list for entity %s", entityName);
		e = TW_MALLOC(sizeof(twEntityCurrentValues));
		entityCurrentValuesDict = twDict_Create(twSubscribedProperty_Delete, twSubscribedProperty_Parse);
		e->name = duplicateString(entityName);
		e->props = entityCurrentValuesDict;
		twDict_Add(spm->currentValues, e);
	}
	return twDict_Add(entityCurrentValuesDict, prop);
}

int twChangeListToInfoTableForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
	twSubscribedProperty * subscribedPropertyChange = (twSubscribedProperty *)data;
	twInfoTable* changeListInfoTable = (twInfoTable*)arg;
	twInfoTableRow * row = NULL;

	row = twInfoTableRow_Create(twPrimitive_CreateFromString(subscribedPropertyChange->prop->name, TRUE));
	if (!row) {
		TW_LOG(TW_ERROR,"twChangeListToInfoTableForEachHandler: Error creating infotable row");
		return TW_FOREACH_EXIT;
	}
	if (twInfoTableRow_GetLength(row) + changeListInfoTable->length + MSG_OVERHEAD > twcfg.max_message_size) {
		twInfoTableRow_Delete(row);
		return TW_FOREACH_EXIT;
	}
	twInfoTableRow_AddEntry(row, twPrimitive_CreateVariant(twPrimitive_FullCopy(subscribedPropertyChange->prop->value)));
	twInfoTableRow_AddEntry(row,twPrimitive_CreateFromDatetime(subscribedPropertyChange->prop->timestamp));
	twInfoTableRow_AddEntry(row,twPrimitive_CreateFromString(subscribedPropertyChange->prop->quality,TRUE));
	twInfoTable_AddRow(changeListInfoTable,row);
	return TW_FOREACH_CONTINUE;

}

/**
 * Builds an InfoTable of the format required to call the UpdateSubscribedPropertyValues() service.
 * As each property change is copied over to the table, it is deleted from the passed in propertyChangeList.
 * Since the returned infotable is limited by the max message size, check the size of propertyChangeList after this
 * function returns to see if it must be called again to empty the remaining changes from propertyChangeList.
 * This function will also deduct the size of the messages removed from the propertyChangeList from spm->queueSize
 * since they are getting dequeued and moved to an the returned InfoTable.
 * @param propertyChangeList a list of twSubscribedProperty*
 * @return an InfoTable containing the changes from the propertyChangeList
 */
twInfoTable* twConvertChangeListToInfoTable(twList* propertyChangeList){
	int index;
	twInfoTable* propChangesInfoTable = twInfoTable_FullCopy(spm->itTemplate);
	twList_Foreach(propertyChangeList,twChangeListToInfoTableForEachHandler,(void*)propChangesInfoTable);

	/* Delete one property change for each row in the returned InfoTable */
	for(index = 0;index<propChangesInfoTable->rows->count;index++){

		/* Every time you remove an entry from a property change list, deduct its size from the overall size of
		 * the spm->queueSize */
		twSubscribedProperty* topMostSubscribedProperty = (twSubscribedProperty* )propertyChangeList->first->value;
		spm->queueSize-= topMostSubscribedProperty->size;

		/* Unlink and delete the topmost entry in the list */
		twList_Remove(propertyChangeList,propertyChangeList->first,TRUE);

	}

	return propChangesInfoTable;
}

extern twOfflineMsgStore * tw_offline_msg_store;

int twInvokeUpdateSubscribedPropertyValues(const twEntitySavedProperties *entitySavedPropertiesInfo, char forceConnect,
										   char async, response_cb cb, twList **messageListRef) {

	int res;
	twInfoTable *resultIt = NULL;
	twInfoTable *changeListInfoTable = NULL;
	twInfoTable *values = NULL;
	changeListInfoTable = twConvertChangeListToInfoTable(entitySavedPropertiesInfo->props);
	TW_LOG(TW_TRACE, "twInvokeUpdateSubscribedPropertyValues: Pushing all properties for %s.",
		   entitySavedPropertiesInfo->name);
	values = twInfoTable_Create(twDataShape_Create(twDataShapeEntry_Create("values", NULL, TW_INFOTABLE)));
	twInfoTable_AddRow(values, twInfoTableRow_Create(twPrimitive_CreateFromInfoTable(changeListInfoTable)));

	if (async) {
		uint32_t messageId = 0;
		uint32_t* messageIdCopy = NULL;
		res = twApi_InvokeServiceAsync(TW_THING, entitySavedPropertiesInfo->name, "UpdateSubscribedPropertyValues", values, forceConnect, cb, &messageId);
		messageIdCopy = TW_MALLOC(sizeof(uint32_t));
		*messageIdCopy = messageId;
		twList_Add(*messageListRef,messageIdCopy);
	} else {
		res = s_twApi_InvokeService(TW_THING, entitySavedPropertiesInfo->name, "UpdateSubscribedPropertyValues", values,
									&resultIt, -1, forceConnect);
	}

	twInfoTable_Delete(changeListInfoTable);
	twInfoTable_Delete(values);
	twInfoTable_Delete(resultIt);
	return res;
}

int twSubscribedPropsMgr_WritePropertiesListToPersistedStore(twEntitySavedProperties *entitySavedPropertiesInfo,
															 char permitPersistProperties) {
	int res = TW_OK;

	/* Do we have a directory to create a persisted property store in? */
	if (twcfg.offline_msg_store_dir) {
		char persistError = FALSE;

		/* Write all the rows plus the subscribed property extra data (entityName & fold) to our persisted file */
		TW_LOG(TW_TRACE,
			   "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: Currently not connected. Writing properties for %s to persisted storage",
			   entitySavedPropertiesInfo->name);

		/* Is there a valid persisted value stream and are we permitted to persist to it? If we are posting
		 * properties from the persisted stream as a source, we would not want to duplicate them by re-writing them back
		 * to the persisted value stream. */
		if (spm->persistedValues) {
			if (permitPersistProperties) {
				ListEntry *persistRow = twList_Next(entitySavedPropertiesInfo->props, NULL);
				ListEntry *deleteRow;
				while (persistRow && persistRow->value) {
					if (twStream_GetLength(spm->persistedValues) < twcfg.offline_msg_queue_size) {
						s_twSubscribedProperty_ToStream((twSubscribedProperty *) persistRow->value,
														spm->persistedValues);
					} else {
						TW_LOG(TW_WARN,
							   "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: Writing persisted property would exceed maximum storage");
						persistError = TRUE;
						break;
					}

					/* As each property update is copied to the persistent store, remove it from the subscribed property
					 * update list */
					spm->queueSize-=((twSubscribedProperty *) persistRow->value)->size;
					deleteRow = persistRow;
					persistRow = twList_Next(entitySavedPropertiesInfo->props, persistRow);
					twList_Remove(entitySavedPropertiesInfo->props,deleteRow,TRUE);
				}
			}
		} else {
			/* There is no persistedValue Stream allocated */
			if (permitPersistProperties) {
				/* We have been asked to persist properties to a value stream that is not allocated, tell the user.*/
				TW_LOG(TW_ERROR,
					   "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: Persisted property storage is not initialized. Property changes are accumulating in memory until your connection is restored.");
				res = TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
				persistError = TRUE;

			}
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: property list will not persist offline pushes because the offline message directory was not set or the offline message store was disabled");

		}

		/*
		If we aren't connected but persisted message store is enabled we have written all values to disk
		and should remove them from RAM so we don't double send them when we are reconnected
		*/
		if (persistError) {
			TW_LOG(TW_ERROR, "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: property list can not persist offline pushes");
			res = TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR;
		} else {
			TW_LOG(TW_INFO, "twSubscribedPropsMgr_WritePropertiesListToPersistedStore: Not connected. Send will not be attempted at this time.");
			res = TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED;
		}
		return res;
	} else {
		return TW_SUBSCRIBED_PROPERTY_LIST_UNABLE_TO_PERSIST_ERROR;
	}

}
int twSubscribedPropsMgr_PushPropertyInfoList(twEntitySavedProperties *entitySavedPropertiesInfo, char forceConnect,
											  char permitPersistProperties, char async, response_cb cb,
											  twList **messageListRef) {
	int res = TW_OK;

	/* Are we connected or permitted to establish a connection? */
	if (!twApi_isConnected() && !forceConnect) {

		/* All the properties in this list must be placed in the persisted message store. This list is now empty. */
		res = twSubscribedPropsMgr_WritePropertiesListToPersistedStore(entitySavedPropertiesInfo,
																	   permitPersistProperties);
		return res;
	}

	/* Call twInvokeUpdateSubscribedPropertyValues() until entitySavedPropertiesInfo's props list is empty
	 * Repeated calls may be required if you are online and your maximum message size will not support
	 * transferring all of your property changes in a single message */
	while (entitySavedPropertiesInfo->props->count > 0) {
		res = twInvokeUpdateSubscribedPropertyValues(entitySavedPropertiesInfo, forceConnect, async, cb, messageListRef);

		if (!(TW_OK == res || res == TW_WROTE_TO_OFFLINE_MSG_STORE)) {
			TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushPropertyInfoList: Error pushing properties for %s.  Error: %d.", entitySavedPropertiesInfo->name, res);
			break;
		}
		/* This list now may or may not be empty */
	}

	return res;
}
typedef struct twSubscribedPropsMgr_PushPropertyListParams {
	twList* deleteList;
	twEntitySavedProperties *entitySavedPropertiesInfo;
	char permitPersistProperties;
	char async;
	char forceConnect;
	response_cb cb;
	twList **messageListRef;
} twSubscribedPropsMgr_PushPropertyListParams ;

int twSubscribedPropsMgr_PushPropertyList_ForeachHandler(void *key, size_t key_size, void *data, size_t data_size,void *userData) {
	int res = TW_OK;
	twEntitySavedProperties *entitySavedPropertiesInfo = (twEntitySavedProperties *) data;
	twSubscribedPropsMgr_PushPropertyListParams* params = (twSubscribedPropsMgr_PushPropertyListParams*) userData;
	res = twSubscribedPropsMgr_PushPropertyInfoList(entitySavedPropertiesInfo, params->forceConnect,
													params->permitPersistProperties, params->async, params->cb, params->messageListRef);

	/* Create a list of empty entity property update lists that should be deleted */
	if(0 == twList_GetCount(entitySavedPropertiesInfo->props)){
		twList_Add(params->deleteList,entitySavedPropertiesInfo);
	}

	return TW_FOREACH_CONTINUE;
}
int twSubscribedPropsMgr_PushPropertyListCleanup_ForeachHandler(void *key, size_t key_size, void *data, size_t data_size,void *userData) {
	twDict * propertyListByEntity = (twDict *)userData;
	ListEntry * entityListEntry = (ListEntry *)data;
	twDict_Remove(propertyListByEntity, entityListEntry, TRUE);
	return TW_FOREACH_CONTINUE;
}

void twSubscribedPropsMgr_DoNothingDeletionHandler(void *ptr){
	/* Use this deletion handler when maintaining a list of things that you do not own */
}

int twSubscribedPropsMgr_PushPropertyList(const char *entityName, twDict *propertyListByEntity, char forceConnect,
										  char permitPersistProperties, char async, response_cb cb,
										  twList **messageListRef) {
	int res = TW_OK;
	if (!spm || !spm->itTemplate || !propertyListByEntity) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushPropertyList: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}

	if(async && !(cb && messageListRef)) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushPropertyList: async mode requires callback to be provided.");
		return TW_PRECONDITION_FAILED;
	}

	if (async) {
		*messageListRef = twList_Create(NULL);
	}

	/* Push a single entities properties or all of them */
	if(entityName){
		twEntitySavedProperties *entitySavedPropertiesInfo;
		twEntitySavedProperties query;
		query.name = entityName;
		if(TW_OK == twDict_Find(propertyListByEntity,&query,&entitySavedPropertiesInfo)) {
			res = twSubscribedPropsMgr_PushPropertyInfoList(entitySavedPropertiesInfo, forceConnect,
															permitPersistProperties, async, cb, messageListRef);

			/* twSubscribedPropsMgr_PushPropertyInfoList() will attempt to push as many property changes as it can
			 * but there may still be some that could not be push this call, if there are, don't delete them. */
			if(0 == twList_GetCount(entitySavedPropertiesInfo->props)){
				twDict_Remove(propertyListByEntity, entitySavedPropertiesInfo, TRUE);
			}
		}
	} else {
		twSubscribedPropsMgr_PushPropertyListParams params;

		/* Build a list of entries to be removed from propertyListByEntity */
		twList* deleteList = twList_Create(twSubscribedPropsMgr_DoNothingDeletionHandler);
		params.deleteList = deleteList;
		params.forceConnect = forceConnect;
		params.async = async;
		params.permitPersistProperties = permitPersistProperties;
		params.cb = cb;
		params.messageListRef = messageListRef;

		/* Push all entities properties */
		twDict_Foreach(propertyListByEntity,twSubscribedPropsMgr_PushPropertyList_ForeachHandler,(void*)&params);

		/* Build a list of entries to be removed from propertyListByEntity */
		twList_Foreach(deleteList,twSubscribedPropsMgr_PushPropertyListCleanup_ForeachHandler,(void*)propertyListByEntity);
		twList_Delete(deleteList);

	}

	return res;
}

int twSubscribedPropsMgr_QueueValueForSending(twSubscribedProperty * propertyToQueue, twDict * propertyListByEntity,char* src) {
	ListEntry * propListEntry = NULL;

	twEntitySavedProperties query;
	twEntitySavedProperties *matchedPropertyInfo;

	if (!propertyToQueue || !propertyToQueue->entity || !propertyToQueue->prop || !propertyToQueue->prop->name || !propertyToQueue->prop->value || !propertyToQueue->prop->quality || !propertyListByEntity) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_QueueValueForSending: NULL pointer found");
		return TW_INVALID_PARAM;
	}
	TW_LOG(TW_TRACE, "twSubscribedPropsMgr_QueueValueForSending: Updating saved property value. Property: %s. Folding is %s", propertyToQueue->prop->name, propertyToQueue->fold ? "ON" : "OFF");

	/* See if this property is contained in propertyListByEntity, if not add it. */
	query.name = propertyToQueue->entity;
	if(TW_OK != twDict_Find(propertyListByEntity,&query,&matchedPropertyInfo)){
		/* Need to create a list for this entity */
		matchedPropertyInfo = twEntitySavedProperties_Create((char *) propertyToQueue->entity);
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_QueueValueForSending: Creating property list for entity %s", propertyToQueue->entity);
		if (!matchedPropertyInfo) {
			TW_LOG(TW_ERROR, "twSubscribedPropsMgr_QueueValueForSending: Error creating property list for %s", propertyToQueue->entity);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		twDict_Add(propertyListByEntity,matchedPropertyInfo);
	}

	/* Now matchedPropertyInfo points to the list specific to this entity */
	if (propertyToQueue->fold) {
		char foundProperty = FALSE;
		/* We are folding so we need to find and replace the property, or add it if it doesn't exist */
		propListEntry = twList_Next(matchedPropertyInfo->props, NULL);
		while (propListEntry && propListEntry->value) {
			twSubscribedProperty * previousPropertyChange = (twSubscribedProperty *)propListEntry->value;
			if (previousPropertyChange->prop && previousPropertyChange->prop->name && !strcmp(previousPropertyChange->prop->name, propertyToQueue->prop->name)) {
				uint32_t previousSize = previousPropertyChange->size;
				TW_LOG(TW_TRACE, "twSubscribedPropsMgr_QueueValueForSending: Folding property %s into entity %s list.  Source: %s", propertyToQueue->prop->name, propertyToQueue->entity, src);
				twList_ReplaceValue(matchedPropertyInfo->props, propListEntry, propertyToQueue, TRUE);

				/* Perform spm->queueSize accounting for swapped change */
				spm->queueSize += propertyToQueue->size;
				spm->queueSize -= previousSize;

				foundProperty = TRUE;
				break;
			}
			propListEntry = twList_Next(matchedPropertyInfo->props, propListEntry);
		}
		if (!foundProperty) {
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_QueueValueForSending: Adding first instance of property %s to entity %s list", propertyToQueue->prop->name,  propertyToQueue->entity);
			twList_Add(matchedPropertyInfo->props, propertyToQueue);

			/* Now that we have definitely added this property update to the queue, increase the current queue size. */
			spm->queueSize += propertyToQueue->size;
		}
	} else {
		/* if we are not folding just add the property */
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_QueueValueForSending: Adding property %s to entity %s list. Source: %s", propertyToQueue->prop->name, propertyToQueue->entity, src);
		twList_Add(matchedPropertyInfo->props, propertyToQueue);

		/* Now that we have definitely added this property update to the queue, increase the current queue size. */
		spm->queueSize += propertyToQueue->size;
	}

	return TW_OK;
}

int twSubscribedPropsMgr_SendPersistedValues() {
	if (twcfg.subscribed_props_enabled) {
		twSubscribedProperty * p = NULL;
		twDict * valuesToPush = NULL;
		TW_LOG(TW_TRACE, "subscribedPropsMgr_RetrievePersistedValues: Retrieving persisted values from file %s", tw_api->subscribedPropsFile ? tw_api->subscribedPropsFile : "UNKNOWN");
		if (!spm || !spm->persistedValues) {
			//TW_LOG(TW_ERROR, "subscribedPropsMgr_UpdateSavedValues: spm->persistedValues not initialized");
			return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
		}
		/* If the file stream is empty there is nothing to do */
		if (twStream_GetLength(spm->persistedValues) == 0) {
			TW_LOG(TW_TRACE, "subscribedPropsMgr_UpdateSavedValues: %s is empty, nothing to do", tw_api->subscribedPropsFile ? tw_api->subscribedPropsFile : "UNKNOWN");
			return TW_OK;
		}
		/* Need to do any property folding here since it is not done when persisting */
		valuesToPush = twDict_Create(twEntitySavedProperties_Delete, twEntitySavedProperties_Parse);
		if (!valuesToPush) {
			TW_LOG(TW_ERROR, "subscribedPropsMgr_UpdateSavedValues: Error allocating memory for temporary list");
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		/* Persisted properties are stored as a contiguouos list of serialized twSubscribedProperties */
		p = twSubscribedProperty_CreateFromStream(spm->persistedValues);
		while (p) {
			if (s_twSubscribedPropsMgr_QueueValueForSending(p, valuesToPush,"FILE")) {
				TW_LOG(TW_ERROR, "subscribedPropsMgr_UpdateSavedValues: Error queueing persisted property %s:%s", p->entity, p->prop->name);
			}
			p = twSubscribedProperty_CreateFromStream(spm->persistedValues);
		}
		/* Push the properties to the server */
		if (twSubscribedPropsMgr_PushPropertyList(SPM_ENTITIES_ALL, valuesToPush, SPM_CONNECT_DONT_FORCE,
					SPM_PERSIST_PROPERTIES_PREVENT, SPM_CALL_BLOCKING, SPM_CALLBACK_NONE, SPM_NO_MESSAGE_ID_LIST)) {
			TW_LOG(TW_ERROR, "subscribedPropsMgr_UpdateSavedValues: Error pushing properties to the server");
			twDict_Delete(valuesToPush);
			/* Reset to the beginning of the stream for next time */
			twStream_Reset(spm->persistedValues);
			return TW_ERROR;
		} else {
			/* If we are here we have successfully pushed all persisted properties, reopen the file to clear it */
			TW_FILE_HANDLE f = NULL;
			twStream_Delete(spm->persistedValues);
			spm->persistedValues = NULL;
			f = TW_FOPEN(spm->subscribedPropsFile, "w");
			TW_FCLOSE(f);
			spm->persistedValues = twStream_CreateFromFile(spm->subscribedPropsFile);
			twDict_Delete(valuesToPush);
		}
	}
	return TW_OK;
}

int twSubscribedPropsMgr_Initialize() {
	twDataShape * ds = NULL;
	TW_LOG(TW_DEBUG, "subscribedPropsMgr_Initialize: Initializing subscribed properties manager");
	spm = (twSubscribedPropsMgr *)TW_CALLOC(sizeof(twSubscribedPropsMgr), 1);
	if (!spm) {
		TW_LOG(TW_ERROR, "subscribedPropsMgr_Initialize: Error allocating memory");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	spm->mtx = twMutex_Create();
	spm->currentValues = twDict_Create(twEntityCurrentValues_Delete, twEntityCurrentValues_Parse);
	spm->savedValues = twDict_Create(twEntitySavedProperties_Delete, twEntitySavedProperties_Parse);
	/* Create our infotable template */
	/* Create the data shape */
	ds = twDataShape_Create(twDataShapeEntry_Create("name", NULL, TW_STRING));
	if (!ds) {
		twSubscribedPropsMgr_Delete();
		TW_LOG(TW_ERROR,"subscribedPropsMgr_Initialize: Error allocating data shape");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("value", NULL, TW_VARIANT));
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("time", NULL, TW_DATETIME));
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("quality", NULL, TW_STRING));
	spm->itTemplate = twInfoTable_Create(ds);
	if (twcfg.subscribed_props_enabled) {
		/* generate subscribed properties file */

		/* Check to see an offline message store directory exists */
		if(twcfg.offline_msg_store_dir != NULL) {
			size_t offline_msg_store_dir_len =  strnlen(twcfg.offline_msg_store_dir, MAX_PATH_LEN);
			spm->subscribedPropsFile = (char*)TW_CALLOC(offline_msg_store_dir_len + strnlen("subscribed_properties.bin",25) + 2, sizeof(char));
		} else {
			/* adding an offset of 3 to account for '.' + '/' + 0x00 (null terminator) */
			spm->subscribedPropsFile = (char*)TW_CALLOC(strnlen("subscribed_properties.bin", 25) + 3, sizeof(char));
		}

		if (!spm->subscribedPropsFile) {
			twSubscribedPropsMgr_Delete();
			TW_LOG(TW_ERROR, "subscribedPropsMgr_Initialize: Error opening: %s", spm->subscribedPropsFile ? spm->subscribedPropsFile : "no filename found");
			return TW_ERROR_ALLOCATING_MEMORY;
		}

		/* configure the path to include either the offline message store dir or the
		   present working dir */
		if(twcfg.offline_msg_store_dir != NULL) {
			strcat(spm->subscribedPropsFile, twcfg.offline_msg_store_dir);
		} else {
			strcat(spm->subscribedPropsFile, ".");
		}
		strcat(spm->subscribedPropsFile, "/");
		strcat(spm->subscribedPropsFile, "subscribed_properties.bin");
		spm->persistedValues = twStream_CreateFromFile(spm->subscribedPropsFile);
		if (!spm->persistedValues) {
			TW_LOG(TW_ERROR, "subscribedPropsMgr_Initialize: Error opening: %s", spm->subscribedPropsFile ? spm->subscribedPropsFile : "no filename found");
		}
	} else {
		TW_LOG(TW_INFO, "subscribedPropsMgr_Initialize: subscribed property manager will not persist offline updates because the offline message directory was not set or the offline message store was disabled");
	}
	if (!spm->mtx || !spm->currentValues || !spm->savedValues || !spm->itTemplate) {
		twSubscribedPropsMgr_Delete();
		TW_LOG(TW_ERROR, "subscribedPropsMgr_Initialize: Error allocating structure member");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	return TW_OK;
}

void twSubscribedPropsMgr_SetFolding(char fold) {
	if (spm) spm->fold = fold;
}

void twSubscribedPropsMgr_Delete() {
	TW_LOG(TW_INFO, "subscribedPropsMgr_Delete: Deleting subscribed propery manager");
	if (!spm) return;
	/* attempt to persist values */
	twSubscribedPropsMgr_PushPropertyList(SPM_ENTITIES_ALL, spm->savedValues, SPM_CONNECT_DONT_FORCE,
							SPM_PERSIST_PROPERTIES_PERMIT, SPM_CALL_BLOCKING, SPM_CALLBACK_NONE, SPM_NO_MESSAGE_ID_LIST);
	if (spm->mtx) twMutex_Lock(spm->mtx);
	if (spm->currentValues) twDict_Delete(spm->currentValues);
	if (spm->savedValues) twDict_Delete(spm->savedValues);
	if (spm->persistedValues) twStream_Delete(spm->persistedValues);
	if (spm->subscribedPropsFile) TW_FREE(spm->subscribedPropsFile);
	if (spm->itTemplate) twInfoTable_Delete(spm->itTemplate);
	if (spm->mtx) {
		twMutex_Unlock(spm->mtx);
		twMutex_Delete(spm->mtx);
	}
	TW_FREE(spm);
}

typedef struct twSubscribedPropsMgr_PushFoldedPropertiesData {
	twDict * foldedSavedValues;
	const char* forEntity;
	char entityFound;
} twSubscribedPropsMgr_PushFoldedPropertiesData;


int  twSubscribedPropsMgr_PushFoldedPropertiesForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twEntitySavedProperties query,*result;

	/* Generate an twEntitySavedProperties list with all changes for each thing with a folded property */
	twSubscribedProperty * propertyData = (twSubscribedProperty *)data;
	twSubscribedPropsMgr_PushFoldedPropertiesData* params = (twSubscribedPropsMgr_PushFoldedPropertiesData*)arg;
	twDict *  dictUpdatesByEntityName = params->foldedSavedValues;

	/* Is this property change for a folded property that needs an update ? */
	if(!(propertyData->fold && propertyData->foldedPropertyNeedsUpdate))
		return TW_FOREACH_CONTINUE;

	propertyData->foldedPropertyNeedsUpdate = FALSE;

	/* Does this entity exist in the dictUpdatesByEntityName list?, if not, create a sub list for this entity */
	query.name = propertyData->entity;
	if(TW_OK != twDict_Find(dictUpdatesByEntityName,&query, &result)){
		twEntitySavedProperties* newSavedPropertyListForEntity = twEntitySavedProperties_Create(propertyData->entity);
		twDict_Add(dictUpdatesByEntityName,newSavedPropertyListForEntity);
		result = newSavedPropertyListForEntity;
	}

	/* This folded property qualifies for a pushed update, add it to the list */
	{
		twSubscribedProperty *subscribedProperty = twSubscribedProperty_Create(propertyData->entity,
																			   propertyData->prop->name,
																			   propertyData->prop->value,
																			   propertyData->prop->timestamp,
																			   propertyData->prop->quality,
																			   propertyData->fold);
		if (!subscribedProperty) {
			TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushFoldedPropertiesForEachHandler: Error allocating memory");
			return TW_FOREACH_CONTINUE;
		}
		twList_Add(result->props, subscribedProperty);
	}
	return TW_FOREACH_CONTINUE;

}

int twSubscribedPropsMgr_currentValuesByEntityForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	char* entityName = (char*)key;
	twEntityCurrentValues* currentValuesRecord = (twEntityCurrentValues *)data;
	twDict * propertyChangeDictForEntity = currentValuesRecord->props;
	twSubscribedPropsMgr_PushFoldedPropertiesData* params = (twSubscribedPropsMgr_PushFoldedPropertiesData*)arg;
	if(NULL == params->forEntity){
		twDict_Foreach(propertyChangeDictForEntity, twSubscribedPropsMgr_PushFoldedPropertiesForEachHandler, arg);
		params->entityFound = TRUE;
		return TW_FOREACH_CONTINUE;
	}
	if(0 == strcmp(entityName,params->forEntity)){
		twDict_Foreach(propertyChangeDictForEntity, twSubscribedPropsMgr_PushFoldedPropertiesForEachHandler, arg);
		params->entityFound = TRUE;
		return TW_FOREACH_EXIT;
	}

    return TW_FOREACH_CONTINUE;
}

int twSubscribedPropsMgr_PushFoldedProperties(char * entityName, char forceConnect,
											  char permitPersistProperties, char async, response_cb cb,
											  twList **messageListRef){
	int res;
	twSubscribedPropsMgr_PushFoldedPropertiesData params;
	params.entityFound = FALSE;

	/* Allocate a folded saved values list */
	params.foldedSavedValues = twDict_Create(twEntitySavedProperties_Delete,twEntitySavedProperties_Parse);
	params.forEntity = entityName;

	/* Loop through this list to generate a folded saved values list */
	twDict_Foreach(spm->currentValues,twSubscribedPropsMgr_currentValuesByEntityForEachHandler,&params);

	res = twSubscribedPropsMgr_PushPropertyList(entityName, params.foldedSavedValues, forceConnect, permitPersistProperties,
													async, cb, messageListRef);

	twDict_Delete(params.foldedSavedValues);

	return res;

}

int twSubscribedPropsMgr_PushSubscribedProperties(char *entityName, char forceConnect, char requiresLock) {

	int res1 = TW_OK;
	int res2 = TW_OK;
	if (!spm || !spm->mtx || !spm->savedValues) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushProperties: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	if(requiresLock)
		twMutex_Lock(spm->mtx);

	/* Always try to push any persisted properties first */
	if (twApi_isConnected() || (!twApi_isConnected() && forceConnect)) {
		twSubscribedPropsMgr_SendPersistedValues();
	}
	/* Now send any queued properties that are in RAM */
	TW_LOG(TW_TRACE, "twSubscribedPropsMgr_PushProperties: Attempting to push queued properties");
	res1 = twSubscribedPropsMgr_PushPropertyList(entityName, spm->savedValues, forceConnect, SPM_PERSIST_PROPERTIES_PERMIT,
												SPM_CALL_BLOCKING, SPM_CALLBACK_NONE, SPM_NO_MESSAGE_ID_LIST);

	/* Now Send any folded properties that need an update, if they exist */
	res2 = twSubscribedPropsMgr_PushFoldedProperties(entityName, forceConnect, SPM_PERSIST_PROPERTIES_PERMIT,
													 SPM_CALL_BLOCKING, SPM_CALLBACK_NONE, SPM_NO_MESSAGE_ID_LIST);

	if (!(TW_OK == res1 || TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED == res1)) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushProperties: error pushing or persisting property list, code: %d", res1);
	}
	if(requiresLock)
		twMutex_Unlock(spm->mtx);

	if(res1 !=TW_OK)
		return res1;

	return res2;
}

int twSubscribedPropsMgr_PushSubscribedPropertiesAsyncForeachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	twList* destinationList = (twList*)arg;
	uint32_t* itemToAdd = (uint32_t*)data;
	uint32_t* messageIdCopy = (uint32_t*)TW_MALLOC(sizeof(uint32_t));
	*messageIdCopy = *itemToAdd;
	twList_Add(destinationList,messageIdCopy);
	return TW_FOREACH_CONTINUE;
}

int twSubscribedPropsMgr_PushSubscribedPropertiesAsync(char *entityName, char forceConnect,
													   response_cb cb,twList** messageListRef) {
	int res1 = TW_OK;
	int res2 = TW_OK;
	twList* foldedMessageList;

	if (!spm || !spm->mtx || !spm->savedValues ) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushSubscribedPropertiesAsync: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}

	twMutex_Lock(spm->mtx);

	/* Always try to push any persisted properties first */
	if (twApi_isConnected() || (!twApi_isConnected() && forceConnect)) {
		twSubscribedPropsMgr_SendPersistedValues();
	}

	/* Now send any queued properties that are in RAM */
	TW_LOG(TW_TRACE, "twSubscribedPropsMgr_PushSubscribedPropertiesAsync: Attempting to push queued properties");
	res1 = twSubscribedPropsMgr_PushPropertyList(entityName,spm->savedValues, forceConnect,SPM_PERSIST_PROPERTIES_PERMIT,
												SPM_CALL_ASYNCHRONOUS, cb, messageListRef);

	/* Now Send any folded properties that need an update, if they exist */
	res2 = twSubscribedPropsMgr_PushFoldedProperties(entityName, forceConnect,SPM_PERSIST_PROPERTIES_PERMIT,
													 SPM_CALL_ASYNCHRONOUS, cb, &foldedMessageList);

	/* Merge foldedMessageList into messageListRef, release foldedMessageList */
	twList_Foreach(*messageListRef,twSubscribedPropsMgr_PushSubscribedPropertiesAsyncForeachHandler,foldedMessageList);
	twList_Delete(foldedMessageList);

	twMutex_Unlock(spm->mtx);

	if(res1 !=TW_OK)
		return res1;

	return res2;
}

int twSubscribedPropsMgr_PushTypeFromProperty(char * entityName, char * propertyName, int* pushType, double* pushThreshold){
	int res = TW_OK;
	callbackInfo *cbMatching;
	callbackInfo cbInfoQuery;
	cJSON *jasonFragment;
	twPropertyDef *def;
	cbInfoQuery.characteristicType = TW_PROPERTIES;
	cbInfoQuery.entityName = entityName;
	cbInfoQuery.entityType = TW_THING;
	cbInfoQuery.characteristicName = propertyName;

	if (TW_OK != twDict_Find(tw_api->callbackList, &cbInfoQuery, &cbMatching)) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushTypeFromProperty: Property %s metadata not found", propertyName);
		return TW_SUBSCRIBED_PROPERTY_NOT_FOUND;
	}

	/* Verify that this property has aspects */
	jasonFragment = NULL;
	def = (twPropertyDef *) cbMatching->characteristicDefinition;
	if (!def->aspects) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushTypeFromProperty: Could not find aspects for property %s",
			   propertyName);
		return TW_INVALID_PARAM;
	}

	jasonFragment = cJSON_GetObjectItem(def->aspects, "pushType");
	if (!jasonFragment || !jasonFragment->valuestring) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PushTypeFromProperty: Could not find pushType for property %s",
			   propertyName);
		return TW_INVALID_PARAM;
	}

	if (!strcmp(jasonFragment->valuestring, "NEVER")) {
		*pushType = TW_UPDATE_NEVER;
	} else if (!strcmp(jasonFragment->valuestring, "ALWAYS")) {
		*pushType = TW_UPDATE_ALWAYS;
	} else if (!strcmp(jasonFragment->valuestring, "VALUE")) {
		*pushType = TW_UPDATE_VALUE;
		jasonFragment = cJSON_GetObjectItem(def->aspects, "pushThreshold");
		if (jasonFragment && jasonFragment->type == cJSON_Number)
			*pushThreshold = jasonFragment->valuedouble;
	}
	else if (!strcmp (jasonFragment->valuestring, "DEADBAND")) {
		*pushType = TW_UPDATE_DEADBAND;
		jasonFragment = cJSON_GetObjectItem (def->aspects, "pushThreshold");
		if (jasonFragment && jasonFragment->type == cJSON_Number)
			*pushThreshold = jasonFragment->valuedouble;
	} else if (!strcmp(jasonFragment->valuestring, "ON")) {
		*pushType = TW_UPDATE_ON;
	} else if (!strcmp(jasonFragment->valuestring, "OFF")) {
		*pushType = TW_UPDATE_OFF;
	}

	return res;
}

/* Performs update processing logic for the pushtypes VALUE and DEADBAND, which both use
 * a threshold to check whether an update should be pushed. */
char twSubscribedPropsMgr_performUpdateByValueBehavior(char *entityName, char *propertyName, int pushType,
													   double pushThreshold,
													   twPrimitive *value, DATETIME timestamp, char *quality, char fold) {

	char saveProperty = FALSE;
	twSubscribedProperty *currentPropertyData = NULL;
	twSubscribedProperty *updatedPropertyData = NULL;
	const char deadband = pushType == TW_UPDATE_DEADBAND;

	/* This property value will become the new, current value */
	updatedPropertyData = twSubscribedProperty_Create(entityName, propertyName, value, timestamp, quality,
															fold);
	if (NULL == updatedPropertyData) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_performUpdateByValueBehavior: Error allocating subscribed property value");
		return FALSE;
	}

	currentPropertyData = twSubscribedPropsMgr_getPropertyCurrentValue(entityName, propertyName);
	if (currentPropertyData) {
		double currentValue = 0, newValue = 0;
		int res = TW_OK;
		const char* currentQuality = currentPropertyData->prop->quality;
		const char* newQuality = quality;

		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_performUpdateByValueBehavior: Found currentValue value of %s", propertyName);

		if (currentPropertyData->prop->value->type == TW_INTEGER ||
			currentPropertyData->prop->value->type == TW_NUMBER) {
			/* For integers and numbers the definition of change by value is that the difference between the current
			 * and the new value changes by more than the threshold */
			if (currentPropertyData->prop->value->type == TW_INTEGER) {
				newValue = value->val.integer;
				currentValue = currentPropertyData->prop->value->val.integer;
			} else {
				newValue = value->val.number;
				currentValue = currentPropertyData->prop->value->val.number;
			}

			/* Is the change significant enough to warrant pushing the new value */
			if (fabs(newValue - currentValue) > pushThreshold || strcmp (currentQuality, newQuality)) {
				saveProperty = TRUE;
				TW_LOG(TW_TRACE,
					   "twSubscribedPropsMgr_performUpdateByValueBehavior: Numeric Property %s has changed, sending to server",
					   propertyName);
			} else {
				TW_LOG(TW_TRACE,
					   "twSubscribedPropsMgr_performUpdateByValueBehavior: Numeric Property %s has changed but not by %f.  Not sending to server",
					   propertyName, pushThreshold);
			}

		} else if (twPrimitive_Compare(currentPropertyData->prop->value, updatedPropertyData->prop->value) != 0 || strcmp (currentQuality, newQuality)) {
			/* Property has changed */
			saveProperty = TRUE;
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_performUpdateByValueBehavior: Property %s has changed, sending to server",
				   propertyName);
		}

		if(fold) {
			/* For folded, updatedPropertyData->foldedPropertyNeedsUpdate = TRUE will cause the currentValue to be pushed */
			updatedPropertyData->foldedPropertyNeedsUpdate = TRUE;
		}

		/* For VALUE push type, retain every update we receive for comparison to the next update. For DEADBAND, retain only the
		 * last value that was pushed to the platform for comparison to the next value. */
		if (!deadband || saveProperty) {
			res = twSubscribedPropsMgr_replacePropertyCurrentValue (entityName, propertyName, currentPropertyData, updatedPropertyData);
		} else {
			twSubscribedProperty_Delete(updatedPropertyData);
		}

		if (fold) {
			/* Folded properties should never be saved */
			saveProperty = FALSE;
			}

		if (res) {
			TW_LOG(TW_WARN,
				   "twSubscribedPropsMgr_performUpdateByValueBehavior: Error updating Property %s in list.  Error: %d",
				   propertyName, res);
		}

	} else {
		/* If this is the first value for this property we should send it and save the value locally */
		/* Save the updated value but we need to copy it first */
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_performUpdateByValueBehavior: First write of Property %s, sending to server", propertyName);
		if(fold) {
			/* Folded properties should never be saved, updatedPropertyData->foldedPropertyNeedsUpdate = TRUE will
			 * cause the currentValue to be pushed */
			updatedPropertyData->foldedPropertyNeedsUpdate = TRUE;
			saveProperty = FALSE;
		} else {
			saveProperty = TRUE;
		}
		twSubscribedPropsMgr_addPropertyCurrentValue(entityName, updatedPropertyData);
	}

	return saveProperty;
}

/*
 * This function simplifies and combines the decision to save a property to the spm->savedValues list or just store
 * it in the spm->currentValues list.
 */
char twSubscribedPropsMgr_UpdateCurrentValueVTQOnFold(char *entityName, char *propertyName, twPrimitive *value,
													  DATETIME timestamp, char *quality, char fold) {
	if(TRUE==fold){
		twSubscribedProperty * newSubscribedPropertyData;
		newSubscribedPropertyData = twSubscribedProperty_Create(entityName, propertyName, value, timestamp, quality,fold);
		twSubscribedPropsMgr_addPropertyCurrentValue(entityName,newSubscribedPropertyData);
		return FALSE;
	}
	return TRUE;

}

int twSubscribedPropsMgr_SetPropertyVTQ(char * entityName, char * propertyName, twPrimitive * value,  DATETIME timestamp, char * quality, char fold, char pushUpdate) {
	int pushType = TW_UPDATE_ALWAYS;
	double pushThreshold = 0.0;
	char saveProperty = FALSE;
	int res = TW_OK;
	twSubscribedProperty * newSubscribedPropertyData;

	/* API State Checks */
	if (!spm || !tw_api || !tw_api->callbackList) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_SetPropertyVTQ: spm or api not initialized");
		if (value) twPrimitive_Delete(value);
		return TW_NULL_OR_INVALID_API_SINGLETON;
	}

	/* Parameter Checks */
	if (!entityName || !propertyName || !value) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_SetPropertyVTQ: Missing parameters");
		if (value) twPrimitive_Delete(value);
		return TW_INVALID_PARAM;
	}
	if (!quality)
		quality = "GOOD";

	/* What push type does this property use? */
	res = twSubscribedPropsMgr_PushTypeFromProperty(entityName, propertyName,&pushType, &pushThreshold);
	if(TW_OK != res){
		twPrimitive_Delete(value);
			return res;
	}

	/* Determine if this change should be sent to the server based on configured push type */
	newSubscribedPropertyData = twSubscribedProperty_Create(entityName, propertyName, value, timestamp, quality,fold);
	newSubscribedPropertyData->size = twSubscribedProperty_GetSize(newSubscribedPropertyData);

	switch (pushType) {
		case TW_UPDATE_ALWAYS:
			saveProperty = twSubscribedPropsMgr_UpdateCurrentValueVTQOnFold(entityName, propertyName, value, timestamp,
																			quality, fold);
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_SetPropertyVTQ: Property %s pushType is ALWAYS", propertyName);
			break;
		case TW_UPDATE_NEVER:
			saveProperty = FALSE;
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_SetPropertyVTQ: Property %s pushType is NEVER", propertyName);
			break;
		case TW_UPDATE_VALUE:
		case TW_UPDATE_DEADBAND:
			saveProperty = twSubscribedPropsMgr_performUpdateByValueBehavior(entityName, propertyName, pushType,
																			 pushThreshold,
																			 value, timestamp, quality, fold);
			TW_LOG(TW_TRACE, "twSubscribedPropsMgr_SetPropertyVTQ: Property %s pushType is %s, checking change threshold",
					propertyName,
					pushType == TW_UPDATE_DEADBAND ? "DEADBAND" : "VALUE");
			break;
		case TW_UPDATE_ON:
			saveProperty = twSubscribedPropsMgr_UpdateCurrentValueVTQOnFold(entityName, propertyName, value, timestamp,
																			quality, fold);
			TW_LOG(TW_TRACE, "twSubscribedProps_Write: Property %s is %s, pushType is ON", propertyName, saveProperty ? "ON" : "OFF");
			break;
		case TW_UPDATE_OFF:
			saveProperty = !twPrimitive_IsTrue(value);
			TW_LOG(TW_TRACE, "twSubscribedProps_Write: Property %s is %s, pushType is OFF", propertyName, saveProperty ? "OFF" : "ON");
			break;
		default:
			saveProperty = FALSE;
			TW_LOG(TW_ERROR, "twSubscribedProps_Write: Unknown push type %d for property %s", pushType, propertyName);
	}

	if (saveProperty == TRUE) {
		uint32_t potentialNewQueueSize = 0;
		twMutex_Lock(spm->mtx);
		/* Does it fit in our maximum offline message queue size? (which is currently set by
		 * twcfg.offline_msg_queue_size) */
		potentialNewQueueSize = spm->queueSize + newSubscribedPropertyData->size;
		if (potentialNewQueueSize < twcfg.offline_msg_queue_size) {
			res = s_twSubscribedPropsMgr_QueueValueForSending(newSubscribedPropertyData, spm->savedValues, "RAM");
			TW_LOG(TW_TRACE, "twSubscribedProps_Write: Property %s being queued to be sent to server",
				   propertyName);
			if (TW_OK == res && TRUE == pushUpdate)
				res = twSubscribedPropsMgr_PushSubscribedProperties(entityName, FALSE, FALSE);
		} else {
			TW_LOG(TW_ERROR,
				   "twSubscribedPropsMgr_SetPropertyVTQ: Adding property %s to queue would exceed max queue size of %d",
				   propertyName, twcfg.offline_msg_queue_size);
			twSubscribedProperty_Delete(newSubscribedPropertyData);
			res = TW_PROPERTY_CHANGE_BUFFER_FULL;
		}
		twMutex_Unlock(spm->mtx);
	} else {
		s_twSubscribedProperty_Delete(newSubscribedPropertyData);
		if (FALSE == fold) {
			/* Folded properties have already been saved, they don't need this error message */
			TW_LOG(TW_TRACE,
				   "twSubscribedPropsMgr_SetPropertyVTQ: Property %s has not been queued to send to server due to a decision based on this properties configuration.",
				   propertyName);
			res = TW_OK;
		}
	}
	twPrimitive_Delete(value);
	return res;

}

int twSubscribedPropsMgr_PurgeCurrentValueForProperty(char *entityName, char *propertyName) {
	int res = TW_UNKNOWN_ERROR;
	if (!spm || !spm->mtx || !spm->currentValues) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PurgeCurrentValueForProperty: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	twMutex_Lock(spm->mtx);

	/* To save on performance we don't bother to figure out the property's push type, just search the cache */
	res = twSubscribedPropsMgr_removePropertyCurrentValue(entityName, propertyName);
	twMutex_Unlock(spm->mtx);

	return res;
}

int twSubscribedPropsMgr_PurgeCurrentValuesForThing(char *entityName) {
	int res = TW_UNKNOWN_ERROR;
	twEntityCurrentValues *e = NULL;
	twEntityCurrentValues *query = NULL;
	twDict *entityCurrentValuesDict = NULL;

	if (!spm || !spm->mtx || !spm->currentValues) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PurgeCurrentValuesForThing: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	twMutex_Lock(spm->mtx);

	query = TW_MALLOC(sizeof(twEntityCurrentValues));
	query->name = entityName;
	query->props = entityCurrentValuesDict;
	res = twDict_Find(spm->currentValues, query, &e);
	if (TW_OK == res) {
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_PurgeCurrentValuesForThing: Removing current values dict for entity %s", entityName);
		res = twDict_Remove(spm->currentValues, e, TRUE);
	} else {
		TW_LOG(TW_TRACE, "twSubscribedPropsMgr_PurgeCurrentValuesForThing: Could not find an existing current values dict for entity %s", entityName);
	}
	TW_FREE(query);

	twMutex_Unlock(spm->mtx);
	return res;
}

int twSubscribedPropsMgr_PurgeChangesForProperty(char * entityName, char * propertyName){
	int res = TW_OK;
	twEntitySavedProperties *entitySavedPropertiesInfo;
	twEntitySavedProperties query;

	if (!spm || !spm->mtx || !spm->savedValues) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PurgeChangesForProperty: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	twMutex_Lock(spm->mtx);

	query.name = entityName;
	if(TW_OK == twDict_Find(spm->savedValues,&query,&entitySavedPropertiesInfo)) {
		ListEntry* le,*lePrev;
		le = entitySavedPropertiesInfo->props->first;
		while(NULL != le) {
			twSubscribedProperty * propertyChange = (twSubscribedProperty * )le->value;
			if(0==strcmp(propertyChange->entity,entityName)&& 0==strcmp(propertyChange->prop->name,propertyName)) {
				lePrev = le;
				getSPM()->queueSize-=propertyChange->size;
				le = le->next;
				twList_Remove(entitySavedPropertiesInfo->props, lePrev, TRUE);
			} else
				le = le->next;
		}
	}

	twMutex_Unlock(spm->mtx);
	return res;
}

int twSubscribedPropsMgr_PurgeChangesForThingEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	int* listSize = (int*)arg;
	twSubscribedProperty * propertyChange  = (twSubscribedProperty * )data;
	*listSize+=propertyChange->size;
	return TW_FOREACH_CONTINUE;
}

int twSubscribedPropsMgr_PurgeChangesForThing(char * entityName){
	int res = TW_OK;
	twEntitySavedProperties *entitySavedPropertiesInfo;
	twEntitySavedProperties query;

	if (!spm || !spm->mtx || !spm->savedValues) {
		TW_LOG(TW_ERROR, "twSubscribedPropsMgr_PurgeChangesForProperty: spm not initialized");
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	twMutex_Lock(spm->mtx);

	query.name = entityName;
	res = twDict_Find(spm->savedValues,&query,&entitySavedPropertiesInfo);
	if(TW_OK == res) {
		/* Calculate size of the list we are dropping and deduct them from spm->queueSize */
		int sizeOfDroppedChanges = 0;
		twList_Foreach(entitySavedPropertiesInfo->props,twSubscribedPropsMgr_PurgeChangesForThingEachHandler,&sizeOfDroppedChanges);
		spm->queueSize-=sizeOfDroppedChanges;

		/* Drop the list */
		res = twDict_Remove(spm->savedValues,entitySavedPropertiesInfo,TRUE);
	}

	twMutex_Unlock(spm->mtx);
	return res;

}

