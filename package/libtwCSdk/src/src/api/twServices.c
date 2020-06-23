/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Service Metadata browsing service functions
 */

#include "twOSPort.h"
#include "twServices.h"
#include "stringUtils.h"

twServiceDef * twServiceDef_Create(char * name, char * description, twDataShape * inputs, 
								   enum BaseType outputType, twDataShape * outputDataShape) {
	twServiceDef * tmp = NULL;
	if (!name) {
		TW_LOG(TW_ERROR,"twServiceDef_Create: NULL name pointer passed in");
		return NULL;
	}
	tmp = (twServiceDef *)TW_CALLOC(sizeof(twServiceDef), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twServiceDef_Create: Error allocating memory");
		return NULL;
	}
	tmp->name = duplicateString(name);
	tmp->description = duplicateString(description);
	tmp->inputs = inputs;
	tmp->outputType = outputType;
	tmp->outputDataShape = outputDataShape;
	tmp->aspects = cJSON_CreateObject();
	return tmp;
}

void twServiceDef_Delete(void * input) {
	if (input) {
		twServiceDef * tmp = (twServiceDef *)input;
		if (tmp->name) {
			TW_FREE(tmp->name);
			tmp->name = NULL;
		}
		if (tmp->description) {
			TW_FREE(tmp->description);
			tmp->description = NULL;
		}
		if (tmp->inputs) {
			twDataShape_Delete(tmp->inputs);
			tmp->inputs = NULL;
		}
		if (tmp->outputDataShape) {
			twDataShape_Delete(tmp->outputDataShape);
			tmp->outputDataShape = NULL;
		}
		if (tmp->aspects) {
			cJSON_Delete(tmp->aspects);
			tmp->aspects = NULL;
		}
		TW_FREE(tmp);
	}
}

