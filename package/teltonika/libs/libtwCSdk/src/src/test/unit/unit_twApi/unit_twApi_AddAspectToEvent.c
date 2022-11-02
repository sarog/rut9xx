/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_AddAspectToEvent()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static callbackInfo *getEventCallbackInfo(char *entityName, char *eventName) {
	callbackInfo *cbInfoQuery;
	callbackInfo *result;
	/* Build Search Query */
	cbInfoQuery = TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = TW_EVENTS;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = eventName;

	if(TW_OK!=twDict_Find(tw_api->callbackList, cbInfoQuery, (void**)&result)){
		TW_FREE(cbInfoQuery);
		return NULL;
	}
	TW_FREE(cbInfoQuery);
	return result;
}

static cJSON *getEventAspects(char *entityName, char *eventName) {
	callbackInfo *info = getEventCallbackInfo(entityName, eventName);
	if (!info) {
		return NULL;
	}
	return ((twServiceDef*)info->characteristicDefinition)->aspects;
}

TEST_GROUP(unit_twApi_AddAspectToEvent);

TEST_SETUP(unit_twApi_AddAspectToEvent) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_AddAspectToEvent) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_AddAspectToEvent) {
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToEventWithNullApi);
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToEventWithNullThingName);
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToEventWithNullEventName);
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToEventWithNullAspectName);
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToNonExistentEvent);
	RUN_TEST_CASE(unit_twApi_AddAspectToEvent, addAspectToEvent);
}

/**
 * Test Plan: Try to add an aspect with a NULL API
 */
TEST(unit_twApi_AddAspectToEvent, addAspectToEventWithNullApi) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_AddAspectToEvent(TEST_ENTITY_NAME, TEST_EVENT_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
 * Test Plan: Try to add an aspect with a NULL thing name
 */
TEST(unit_twApi_AddAspectToEvent, addAspectToEventWithNullThingName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToEvent(NULL, TEST_EVENT_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a NULL thing name
*/
TEST(unit_twApi_AddAspectToEvent, addAspectToEventWithNullEventName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToEvent(TEST_ENTITY_NAME, NULL, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a NULL thing name
*/
TEST(unit_twApi_AddAspectToEvent, addAspectToEventWithNullAspectName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToEvent(TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect to a non-existent event
*/
TEST(unit_twApi_AddAspectToEvent, addAspectToNonExistentEvent) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToEvent(TEST_ENTITY_NAME, TEST_EVENT_NAME, TEST_ASPECT_NAME, value));
}

/**
* Test Plan: Try to add an aspect to a service
*/
TEST(unit_twApi_AddAspectToEvent, addAspectToEvent) {
	cJSON *aspects = NULL;
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
	aspects = getEventAspects(TEST_ENTITY_NAME, TEST_EVENT_NAME);
	TEST_ASSERT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddAspectToEvent(TEST_ENTITY_NAME, TEST_EVENT_NAME, TEST_ASPECT_NAME, value));
	TEST_ASSERT_NOT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TEST_PRIMITIVE_INT_VALUE, cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME)->valueint);
}