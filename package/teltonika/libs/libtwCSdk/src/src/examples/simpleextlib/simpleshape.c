/***************************************
 *  Copyright 2016, PTC, Inc.
 ***************************************/

/**
 * \file simpleshape.C
 * \brief Provides a simple thing shape example which provides two properties and a service.
 * \author bill.reichardt@thingworx.com
 *
 * Provides a simple thing shape example which has two properties and a service. This example is meant as a
 * stating point for you to create your own shape libraries. It creates a shape having two string properties
 * and a service that operates on them. It uses the genericPropertyHandler() to store your current property
 * values on the heap and automatically make them available to the thingworx server.
*/
#include <twBaseTypes.h>
#include "twBaseTypes.h"
#include "twShapes.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "twThreadUtils.h"

#include "simpleshape.h"

#define SHAPE_NAME "SimpleShape"
#define TW_NO_INITIAL_CONFIG NULL
#define TW_NO_CONFIG_HANDLER NULL

/**
 * Service Implementation for the createMessage service. This service concatinates two property values and
 * a parameter value into a single string which is returned.
 * @param entityName The name of the thing using this shape
 * @param serviceName The name of this service
 * @param params an infotable containing the parameters this service was called with
 * @param content an infotable which contains values returned by this service. Single, unnamed return values are
 * usually placed in the result collumn of the returned InfoTable.
 * @param userdata
 * @return an error code to summerize the results of this service call. Should be TWX_SUCCESS on success.
 */
enum msgCodeEnum createMessageService(const char *entityName, const char *serviceName, twInfoTable *params,
                                      twInfoTable **content, void *userdata){
    char * propertyA = NULL;
    double propertyB = 0.0;
    char * parameter1 = NULL;
    char message[255];

    /* Get properties read into local variables */
    twInfoTable_GetString(params, "parameter1", 0, &parameter1);

    propertyA = twExt_GetPropertyValue(entityName, "propertyA")->val.bytes.data;
    propertyB = twExt_GetPropertyValue(entityName, "propertyB")->val.number;
    TW_LOG(TW_FORCE,"propertyA= %s",propertyA);
    TW_LOG(TW_FORCE,"propertyB= %f",propertyB);
    snprintf(message, 255,"This is a message combining propertyA=%s  and propertyB=%f and parameter1=%s",propertyA,propertyB,parameter1);

    /* Return Result */
    *content = twInfoTable_CreateFromString("result",duplicateString(message),FALSE);
    return TWX_SUCCESS;

}
int onSimpleProcessScanRequest(char *thingName) {
    /* Add one to the counter */
	twPrimitive *currentValue;
	double newDoubleValue;
    TW_LOG(TW_TRACE,"called onSimpleProcessScanRequest()");
    currentValue = twExt_GetPropertyValue(thingName, "count");
    newDoubleValue = currentValue->val.number;
    newDoubleValue++;
	twExt_SetPropertyValue(thingName, "count", twPrimitive_CreateFromNumber(newDoubleValue), TW_FOLD_TYPE_NO, TW_PUSH_NOW);
    twApi_PushSubscribedProperties(thingName, TW_PUSH_CONNECT_FORCE);
}

/**
 * An EdgeThingShape definition is a function in the C SDK. This function will be registered in the list of
 * available shapes. When this shape is used to create a thing, this function will be called. Its job is to
 * add properties and services to the thing with the name provided in the thingName argument.
 * @param thingName The name of the thing currently being constructed that will have this shape added to it.
 */
void constructSimpleShape(const char* thingName,const char* thing_namespace){
    twDataShapeEntry* inputParam1DsEntry;
    twDataShape* inputParamsDataShape;
    twExt_RegisterStandardProperty(thingName, "propertyA", thing_namespace, TW_STRING, "A Simple String Property",
                                   TW_PUSH_TYPE_ALWAYS,
                                   TW_PUSH_THRESHOLD_NONE);
    twExt_RegisterStandardProperty(thingName, "propertyB", thing_namespace, TW_NUMBER, "A Simple Number Property",
                                   TW_PUSH_TYPE_ALWAYS,
                                   TW_PUSH_THRESHOLD_NONE);

    twExt_RegisterStandardProperty(thingName, "count", thing_namespace, TW_NUMBER, "A simple counter", TW_PUSH_TYPE_ALWAYS,
                                   TW_PUSH_THRESHOLD_NONE);

    /* Update the counter periodically */
	twExt_SetPropertyValue(thingName, "count", twPrimitive_CreateFromNumber(0.0), TW_FOLD_TYPE_NO, TW_PUSH_NOW);
    twExt_RegisterPolledShapeFunction(onSimpleProcessScanRequest, SHAPE_NAME);

    /* Define a service for this shape */
    inputParam1DsEntry = twDataShapeEntry_Create("parameter1", "The first parameter", TW_STRING);
    inputParamsDataShape = twDataShape_CreateFromEntries(TW_SHAPE_NAME_NONE,inputParam1DsEntry,NULL);

    twExt_RegisterNamespacedService(thingName, "createMessage", thing_namespace,
                                    "Create a short message using propertyA, propertyB and parameter1 of this service.",
                                    inputParamsDataShape, TW_STRING, TW_NO_RETURN_DATASHAPE, createMessageService,
                                    TW_NO_USER_DATA);
}

/**
 * Any shape library must implement an init function which will get called when this shape is dynamically loaded
 * from a shared library. This function must have the same name as the library itself prefixed with "init_". Since
 * this library will be called simpleshapelib, this function must be "init_simpleshapelib".
 * This function is responsible for registering your shape handler function in a map of available shapes. This is done
 * by calling the twRegisterShape() function for each new shape you will be declaring.
 * @return Any value less than 0 is considered an error.
 */
int init_libsimpleext(){
    twExt_RegisterShape((const char *) SHAPE_NAME, constructSimpleShape);
    return TW_OK;
}
