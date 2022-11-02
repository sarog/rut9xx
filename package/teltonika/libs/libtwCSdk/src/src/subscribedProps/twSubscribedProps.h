/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Thingworx Subscribed Properties
 */

#include "twOSPort.h"
#include "twLogger.h"
#include "twApi.h"
#include "twInfoTable.h"
#include "twProperties.h"

#ifndef TW_SUBSCRIBED_PROPS_H
#define TW_SUBSCRIBED_PROPS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct twSubscribedProperty {
	const char * entity;
	twProperty * prop;
	char fold;
	char foldedPropertyNeedsUpdate;
	uint32_t size;
} twSubscribedProperty;

/***************************************/
/*     Entity Saved Property List     */
/***************************************/
typedef struct twEntitySavedProperties {
	char * name;
	twList * props;
} twEntitySavedProperties;

/***************************************/
/*     Entity Current Value List     */
/***************************************/
typedef struct twEntityCurrentValues {
	char *name;
	twDict *props;
} twEntityCurrentValues;


/***************************************/
/*    Subscribed Properties Manager    */
/***************************************/
typedef struct twSubscribedPropsMgr {
	TW_MUTEX mtx;
	twDict * currentValues;
	twDict * savedValues;
	twStream * persistedValues;
	char fold;
	twInfoTable * itTemplate;
	uint32_t queueSize;
	char * subscribedPropsFile;
} twSubscribedPropsMgr;

twSubscribedPropsMgr * twSubscribedPropsMgr_Get();

void twSubscribedProperty_Delete(void * prop);

int twSubscribedProperty_ToStream (twSubscribedProperty * p, twStream * s);

int twSubscribedPropsMgr_Initialize();

void twSubscribedPropsMgr_Delete();

void twSubscribedPropsMgr_SetFolding(char fold);

int twSubscribedPropsMgr_PushSubscribedProperties(char *entityName, char forceConnect, char requiresLock);
int twSubscribedPropsMgr_PushSubscribedPropertiesAsync(char *entityName, char forceConnect,
													   response_cb cb,twList** messageListRef);

int twSubscribedPropsMgr_SetPropertyVTQ(char * entityName, char * propertyName, twPrimitive * value,  DATETIME timestamp, char * quality, char fold, char pushUpdate);
int twSubscribedPropsMgr_PurgeCurrentValueForProperty(char * entityName, char * propertyName);
int twSubscribedPropsMgr_PurgeCurrentValuesForThing(char *entityName);
int twSubscribedPropsMgr_PurgeChangesForProperty(char * entityName, char * propertyName);
int twSubscribedPropsMgr_PurgeChangesForThing(char * entityName);
int twSubscribedPropsMgr_QueueValueForSending (twSubscribedProperty * propertyToQueue, twDict * propertyListByEntity, char* src);
twSubscribedProperty *twSubscribedPropsMgr_getPropertyCurrentValue(char *entityName, char *propertyName);

void twSubscribedProperty_Delete(void * prop);

#ifdef __cplusplus
}
#endif

#endif
