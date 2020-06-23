/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_AddAspectToService()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

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

static cJSON *getServiceAspects(char *entityName, char *serviceName) {
	callbackInfo *info = getServiceCallbackInfo(entityName, serviceName);
	if (!info) {
		return NULL;
	}
	return ((twServiceDef*)info->characteristicDefinition)->aspects;
}

TEST_GROUP(unit_twApi_AddAspectToService);

TEST_SETUP(unit_twApi_AddAspectToService) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_AddAspectToService) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_AddAspectToService) {
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToServiceWithNullApi);
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToServiceWithNullThingName);
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToServiceWithNullServiceName);
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToServiceWithNullAspectName);
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToNonExistentService);
	RUN_TEST_CASE(unit_twApi_AddAspectToService, addAspectToService);
}

/**
 * Test Plan: Try to add an aspect with a NULL API
 */
TEST(unit_twApi_AddAspectToService, addAspectToServiceWithNullApi) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_AddAspectToService(TEST_ENTITY_NAME, TEST_SERVICE_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
 * Test Plan: Try to add an aspect with a NULL thing name
 */
TEST(unit_twApi_AddAspectToService, addAspectToServiceWithNullThingName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToService(NULL, TEST_SERVICE_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a NULL service name
*/
TEST(unit_twApi_AddAspectToService, addAspectToServiceWithNullServiceName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToService(TEST_ENTITY_NAME, NULL, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a NULL aspect name
*/
TEST(unit_twApi_AddAspectToService, addAspectToServiceWithNullAspectName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToService(TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect to a non-existent service
*/
TEST(unit_twApi_AddAspectToService, addAspectToNonExistentService) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToService(TEST_ENTITY_NAME, TEST_SERVICE_NAME, TEST_ASPECT_NAME, value));
}

/**
* Test Plan: Try to add an aspect to a service
*/
TEST(unit_twApi_AddAspectToService, addAspectToService) {
	cJSON *aspects = NULL;
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, &doNothing, NULL));
	aspects = getServiceAspects(TEST_ENTITY_NAME, TEST_SERVICE_NAME);
	TEST_ASSERT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddAspectToService(TEST_ENTITY_NAME, TEST_SERVICE_NAME, TEST_ASPECT_NAME, value));
	TEST_ASSERT_NOT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TEST_PRIMITIVE_INT_VALUE, cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME)->valueint);
}