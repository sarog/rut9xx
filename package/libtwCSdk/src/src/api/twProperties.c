/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Metadata browsing and property/service functions
 */

#include "twOSPort.h"
#include "twProperties.h"
#include "stringUtils.h"
#include "twApiStubs.h"


twPropertyDef * twPropertyDef_Create(char * name, enum BaseType type, char * description, char * pushType, double pushThreshold) {
	twPropertyDef * tmp = NULL;
	if (!name) {
		TW_LOG(TW_ERROR,"twPropertyDef_Create: NULL name pointer passed in");
		return NULL;
	}
	tmp = (twPropertyDef *)TW_CALLOC(sizeof(twPropertyDef), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twPropertyDef_Create: Error allocating memory");
		return NULL;
	}
	tmp->name = duplicateString(name);
	tmp->type = type;
	tmp->description = duplicateString(description);
	tmp->aspects = cJSON_CreateObject();
	if (!tmp->aspects) {
		TW_LOG(TW_ERROR,"twPropertyDef_Create: Error allocating memory");
		twPropertyDef_Delete(tmp);
		return NULL;
	}
	if (pushType) cJSON_AddStringToObject(tmp->aspects,"pushType", pushType);
	cJSON_AddNumberToObject(tmp->aspects,"pushThreshold", pushThreshold);
	return tmp;
}

void twPropertyDef_Delete(void * input) {
	if (input) {
		twPropertyDef * tmp = (twPropertyDef *)input;
		if(tmp->name) {
			TW_FREE(tmp->name);
			tmp->name = NULL;
		}
		if (tmp->description) {
			TW_FREE(tmp->description);
			tmp->description = NULL;
		}
		if (tmp->aspects) {
			cJSON_Delete(tmp->aspects);
			tmp->aspects = NULL;
		}
		TW_FREE(tmp);
	}
}

twProperty * twProperty_Create(char * name, twPrimitive * value, DATETIME timestamp) {
	return twPropertyVTQ_Create(name, value, timestamp, NULL);
}

twProperty * twPropertyVTQ_Create(char * name, twPrimitive * value, DATETIME timestamp, char * quality) {
	twProperty * tmp = NULL;
	if (!name || !value) {
		TW_LOG(TW_ERROR,"twPropertyVTQ_Create: NULL name or value pointer passed in");
		return NULL;
	}
	tmp = (twProperty *)TW_CALLOC(sizeof(twProperty), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twPropertyVTQ_Create: Error allocating memory");
		return NULL;
	}
	tmp->name = duplicateString(name);
	tmp->value = value; /* We own this pointer now */
	if (!timestamp) tmp->timestamp = twGetSystemTime(TRUE);
	else tmp->timestamp = timestamp;
	if (!quality) tmp->quality = duplicateString("GOOD");
	else tmp->quality = duplicateString(quality);
	return tmp;
}

twProperty * twProperty_CreateFromStream(twStream * s) {
	twProperty * tmp = NULL;
	if (!s) {
		TW_LOG(TW_ERROR,"twProperty_CreateFromStream: NULL stream pointer");
		return NULL;
	}
	tmp = (twProperty *)TW_CALLOC(sizeof(twProperty), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twProperty_CreateFromStream: Error allocating memory");
		return NULL;
	}
	tmp->name = s_streamToString(s);
	tmp->value = s_twPrimitive_CreateFromStream(s);
	twStream_GetBytes(s, &tmp->timestamp, 8);
	swap8bytes((char *)&tmp->timestamp);
	tmp->quality = s_streamToString(s);
	if (!tmp->name || !tmp->value || !tmp->quality) {
		TW_LOG(TW_ERROR,"twProperty_CreateFromStream: Error creating VTQ property from stream");
		twProperty_Delete(tmp);
		return NULL;
	}
	return tmp;
}

void twProperty_Delete(void * input) {
	if (input) {
		twProperty * tmp = (twProperty *)input;
		if (tmp->name) TW_FREE(tmp->name);
		if (tmp->value) twPrimitive_Delete(tmp->value);
		if (tmp->quality) TW_FREE(tmp->quality);
		TW_FREE(tmp);
	}
}
