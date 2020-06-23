/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterPropertyCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test property callback functions */
static enum msgCodeEnum testPropertyCallbackAlpha(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) { return TWX_SUCCESS; }
static enum msgCodeEnum testPropertyCallbackBeta(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) { return TWX_SUCCESS; }

static callbackInfo *getPropertyCallbackInfo(char *entityName, char *propertyName) {
	callbackInfo *cbInfoQuery;
	callbackInfo *result;
	/* Build Search Query */
	cbInfoQuery = TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = TW_PROPERTIES;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = propertyName;

	if(TW_OK!=twDict_Find(tw_api->callbackList, cbInfoQuery, (void**)&result)) {
		TW_FREE(cbInfoQuery);
		return NULL;
	}
	TW_FREE(cbInfoQuery);
	return result;
}

TEST_GROUP(unit_twApi_RegisterPropertyCallback);

TEST_SETUP(unit_twApi_RegisterPropertyCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterPropertyCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterPropertyCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullPropertyName);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullPropertyName);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterNonExistentPropertyCallback);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacks);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacksForTwoDifferentProperties);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacksForTwoDifferentThings);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbackTwice);
	RUN_TEST_CASE(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithValues);
}

/**
 * Test Plan: Try to register a property callback with a NULL API
 */
TEST(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &doNothing, NULL));
}

/**
 * Test Plan: Try to register a property callback with a NULL thing name
 */
TEST(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterPropertyCallback(TW_THING, NULL, TEST_PROPERTY_NAME, &doNothing, NULL));
}

/**
 * Test Plan: Try to register a property callback with a NULL service name
 */
TEST(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullPropertyName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, NULL, &doNothing, NULL));
}

/**
 * Test Plan: Try to register a property callback with a NULL callback function pointer
 */
TEST(unit_twApi_RegisterPropertyCallback, registerPropertyCallbackWithNullCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to unregister a property callback with a NULL API
 */
TEST(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &doNothing));
}

/**
 * Test Plan: Try to unregister a property callback with a NULL thing name
 */
TEST(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterPropertyCallback(NULL, TEST_PROPERTY_NAME, &doNothing));
}

/**
 * Test Plan: Try to unregister a property callback with a NULL service name
 */
TEST(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullPropertyName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, NULL, &doNothing));
}

/**
 * Test Plan: Try to unregister a property callback with a NULL callback function pointer
 */
TEST(unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithNullCallback) {
	TEST_IGNORE_MESSAGE("CSDK-1363: Unregister property argument is unused");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
}

/**
 * Test Plan: Try to unregister a property callback without registering one first
 */
TEST(unit_twApi_RegisterPropertyCallback, unregisterNonExistentPropertyCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
}

/**
 * Test Plan: Register a callback, unregister it, register a new callback, and then unregister the new callback
 */
TEST(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacks) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &testPropertyCallbackAlpha, NULL));
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackAlpha, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
	/* Register a new callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &testPropertyCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackBeta, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
}

/**
 * Test Plan: Register property callbacks for two different properties on the same thing
 */
TEST(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacksForTwoDifferentProperties) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2));
	/* Register the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1, &testPropertyCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2, &testPropertyCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME_1, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackAlpha, info->cb);
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME_2, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackBeta, info->cb);
	/* Unregister the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1, NULL));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2, NULL));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2));
}

/**
 * Test Plan: Register property callbacks for the same property on two different things
 */
TEST(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbacksForTwoDifferentThings) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME));
	/* Register the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME, &testPropertyCallbackAlpha, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME, &testPropertyCallbackBeta, NULL));
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME_1, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackAlpha, info->cb);
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME_2, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackBeta, info->cb);
	/* Unregister the callbacks */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME, NULL));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME));
	TEST_ASSERT_NOT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME, NULL));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_1, TEST_PROPERTY_NAME));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME_2, TEST_PROPERTY_NAME));
}

/**
 * Test Plan: Register a callback, try to register a new callback, unregister the callback, and then try to unregister it once more
 */
TEST(unit_twApi_RegisterPropertyCallback, registerAndUnregisterPropertyCallbackTwice) {
	callbackInfo * info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &testPropertyCallbackAlpha, NULL));
	/* Get the callback info and verify the fields */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackAlpha, info->cb);
	/* Try to register a new callback without unregistering the first one */
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_RegisterPropertyCallback(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &testPropertyCallbackBeta, NULL));
	/* Get the callback info and verify that the fields have not changed */
	info = getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_NOT_NULL(info);
	TEST_ASSERT_EQUAL(TW_THING, info->entityType);
	TEST_ASSERT_EQUAL_STRING(TEST_ENTITY_NAME, info->entityName);
	TEST_ASSERT_EQUAL(TW_PROPERTIES, info->characteristicType);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME, info->characteristicName);
	TEST_ASSERT_EQUAL(testPropertyCallbackAlpha, info->cb);
	/* Unregister the callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
	/* Try to unregister the callback a second time */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterPropertyCallback(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME));
}
/**
 * Test Plan: Submits property updates for several properties on the same Thing, then removes the properties
 */
TEST (unit_twApi_RegisterPropertyCallback, unregisterPropertyCallbackWithValues) {
	/* Enable the (file backed) offline message store. */
	TEST_ASSERT_EQUAL (TW_OK, enableOfflineMsgStore (TRUE, TRUE));

	/* Initialize the API */
	TEST_ASSERT_EQUAL (TW_OK, twApi_Initialize (TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                            MESSAGE_CHUNK_SIZE, TRUE));

	/* Register property 1 - push type ALWAYS*/
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1,
	                                                  TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));

	/* Register property 2 - push type VALUE*/
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2,
	                                                  TW_INTEGER, NULL, "VALUE", 0, &doNothing, NULL));

	/* Queue an update for property 1 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Queue an update for property 2 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Push updates for all entities by specifying NULL for the entity name */
	TEST_ASSERT_EQUAL (TW_OK, twApi_PushSubscribedProperties (NULL, FALSE));

	/* Other tests validate the updates made it through; we just want to unregister these and make sure cleanup succeeds*/

	/* Unregister/remove the two properties */
	TEST_ASSERT_EQUAL (TW_OK, twApi_UnregisterPropertyCallback (TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1, &doNothing));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_1));
	TEST_ASSERT_EQUAL (TW_OK, twApi_UnregisterPropertyCallback (TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2, &doNothing));
	/* Verify the property callback has been unregistered */
	TEST_ASSERT_NULL(getPropertyCallbackInfo(TEST_ENTITY_NAME, TEST_PROPERTY_NAME_2));
}