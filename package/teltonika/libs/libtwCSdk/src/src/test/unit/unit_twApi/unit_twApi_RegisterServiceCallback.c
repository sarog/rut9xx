/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterServiceCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static enum msgCodeEnum doNothing_serviceCb(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	return TWX_SUCCESS;
}


/* Test service callback functions */
static enum msgCodeEnum testServiceCallbackAlpha(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) { return TWX_SUCCESS; }
static enum msgCodeEnum testServiceCallbackBeta(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) { return TWX_SUCCESS; }

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

TEST_GROUP(unit_twApi_RegisterServiceCallback);

TEST_SETUP(unit_twApi_RegisterServiceCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterServiceCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterServiceCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullServiceName);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullServiceName);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, unregisterNonExistentServiceCallback);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacks);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacksForTwoDifferentProperties);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacksForTwoDifferentThings);
	RUN_TEST_CASE(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbackTwice);
}

/**
 * Test Plan: Try to register a service callback with a NULL API
 */
TEST(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service callback with a NULL thing name
 */
TEST(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterServiceCallback(TW_THING, NULL, TEST_SERVICE_NAME, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service callback with a NULL service name
 */
TEST(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullServiceName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, NULL, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service callback with a NULL callback function pointer
 */
TEST(unit_twApi_RegisterServiceCallback, registerServiceCallbackWithNullCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to unregister a service callback with a NULL API
 */
TEST(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
}

/**
 * Test Plan: Try to unregister a service callback with a NULL thing name
 */
TEST(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, doNothing_serviceCb, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterServiceCallback(NULL, TEST_SERVICE_NAME, NULL));
}

/**
 * Test Plan: Try to unregister a service callback with a NULL service name
 */
TEST(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullServiceName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, doNothing_serviceCb, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to unregister a service callback with a NULL callback function pointer
 */
TEST(unit_twApi_RegisterServiceCallback, unregisterServiceCallbackWithNullCallback) {
	TEST_IGNORE_MESSAGE("CSDK-1363: Unregister property argument is unused");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, doNothing_serviceCb, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
}

/**
 * Test Plan: Try to unregister a service callback without registering one first
 */
TEST(unit_twApi_RegisterServiceCallback, unregisterNonExistentServiceCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
}

/**
 * Test Plan: Register a callback, unregister it, register a new callback, and then unregister the new callback
 */
TEST(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacks) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, testServiceCallbackAlpha, NULL));
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackAlpha, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
	/* Verify the service callback is now NULL */
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
	/* Register a new callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, testServiceCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackBeta, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
	/* Verify the service callback is now NULL */
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
}

/**
 * Test Plan: Register service callbacks for two different properties on the same thing
 */
TEST(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacksForTwoDifferentProperties) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_2));
	/* Register the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME_1, testServiceCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME_2, testServiceCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_1);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME_1, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackAlpha, info->cb);
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_2);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME_2, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackBeta, info->cb);
	/* Unregister the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME_1, NULL));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_2));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME_2, NULL));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_1));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME_2));
}

/**
 * Test Plan: Register service callbacks for the same service on two different things
 */
TEST(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbacksForTwoDifferentThings) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME));
	/* Register the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_SERVICE_NAME, testServiceCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_SERVICE_NAME, testServiceCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME_1, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackAlpha, info->cb);
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME_2, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackBeta, info->cb);
	/* Unregister the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME, NULL));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME));
	TEST_ASSERT_NOT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME, NULL));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_1, TEST_SERVICE_NAME));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME_2, TEST_SERVICE_NAME));
}

/**
 * Test Plan: Register a callback, try to register a new callback, unregister the callback, and then try to unregister it once more
 */
TEST(unit_twApi_RegisterServiceCallback, registerAndUnregisterServiceCallbackTwice) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, testServiceCallbackAlpha, NULL));
	/* Get the callback info and verify the fields */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackAlpha, info->cb);
	/* Try to register a new callback without unregistering the first one */
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_RegisterServiceCallback(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, testServiceCallbackBeta, NULL));
	/* Get the callback info and verify that the fields have not changed */
	info = getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_SERVICES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testServiceCallbackAlpha, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
	/* Verify the service callback is now NULL */
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
	/* Try to unregister the callback a second time */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterServiceCallback(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL));
	/* Verify the service callback is now NULL */
	TEST_ASSERT_NULL(getServiceCallbackInfo(TEST_ENTITY_NAME, TEST_SERVICE_NAME));
}