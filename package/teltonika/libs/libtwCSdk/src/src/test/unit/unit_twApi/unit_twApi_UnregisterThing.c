/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_UnregisterThing()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test property callback functions */
static enum msgCodeEnum testPropertyCallbackAlpha(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) { return TWX_SUCCESS; }
static enum msgCodeEnum testPropertyCallbackBeta(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) { return TWX_SUCCESS; }

/* Test service callback functions */
static enum msgCodeEnum testServiceCallbackAlpha(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) { return TWX_SUCCESS; }
static enum msgCodeEnum testServiceCallbackBeta(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) { return TWX_SUCCESS; }

static callbackInfo *getPropertyCallbackInfo(char *entityName, char *propertyName) {
	callbackInfo *cbInfoQuery;
	callbackInfo *result;
	/* Build Search Query */
	cbInfoQuery = TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = TW_PROPERTIES;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = propertyName;

	if(TW_OK!=twDict_Find(tw_api->callbackList, cbInfoQuery, (void**)&result)){
		result = NULL;
	}
	TW_FREE(cbInfoQuery);
	return result;
}

static callbackInfo *getServiceCallbackInfo(char *entityName, char *serviceName) {
	callbackInfo *cbInfoQuery;
	callbackInfo *result;
	/* Build Search Query */
	cbInfoQuery = TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = TW_SERVICES;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = serviceName;

	if(TW_OK!=twDict_Find(tw_api->callbackList, cbInfoQuery, (void**)&result)){
		TW_FREE(cbInfoQuery);
		return NULL;
	}
	TW_FREE(cbInfoQuery);
	return result;
}

TEST_GROUP(unit_twApi_UnregisterThing);

TEST_SETUP(unit_twApi_UnregisterThing) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_UnregisterThing) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_UnregisterThing) {
	RUN_TEST_CASE(unit_twApi_UnregisterThing, unregisterThingWithNullApi);
	RUN_TEST_CASE(unit_twApi_UnregisterThing, unregisterThingWithNullThingName);
	RUN_TEST_CASE(unit_twApi_UnregisterThing, unregisterThingsWithCallbacks);
}

/**
 * Test Plan: Try to unregister a thing with a NULL API
 */
TEST(unit_twApi_UnregisterThing, unregisterThingWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_UnregisterThing(TEST_ENTITY_NAME));
}

/**
 * Test Plan: Try to unregister a thing with a NULL thing name
 */
TEST(unit_twApi_UnregisterThing, unregisterThingWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterThing(NULL));
}

/**
 * Test Plan: Register some callbacks to some things and then unregister them
 */
TEST(unit_twApi_UnregisterThing, unregisterThingsWithCallbacks) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Register property callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_1, &testPropertyCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_2, &testPropertyCallbackBeta, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_1, &testPropertyCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_2, &testPropertyCallbackBeta, NULL));
	/* Register service callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_1, &testServiceCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_2, &testServiceCallbackBeta, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_1, &testServiceCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_2, &testServiceCallbackBeta, NULL));
	/* Check that all callbacks have been registered */
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_2));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_2));
	/* Unregister the first thing */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterThing(TEST_ENTITY_NAME_1));
	/* Check that the first thing's callbacks have been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_2));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_2));
	/* Unregister the second thing */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterThing(TEST_ENTITY_NAME_2));
	/* Check that the second thing's callbacks have been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME_2));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME_2));
}
