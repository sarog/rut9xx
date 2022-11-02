/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_AddAspectToProperty()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

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

static cJSON *getPropertyAspects(char *entityName, char *propertyName) {
	callbackInfo *info = getPropertyCallbackInfo(entityName, propertyName);
	if (!info) {
		return NULL;
	}
	return ((twPropertyDef*)info->characteristicDefinition)->aspects;
}

TEST_GROUP(unit_twApi_AddAspectToProperty);

TEST_SETUP(unit_twApi_AddAspectToProperty) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_AddAspectToProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_AddAspectToProperty) {
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullThingName);
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullPropertyName);
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullAspectName);
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToNonExistentProperty);
	RUN_TEST_CASE(unit_twApi_AddAspectToProperty, addAspectToProperty);
}

/**
 * Test Plan: Try to add an aspect with a NULL API
 */
TEST(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullApi) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_AddAspectToProperty(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
 * Test Plan: Try to add an aspect with a NULL thing name
 */
TEST(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullThingName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToProperty(NULL, TEST_PROPERTY_NAME, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a property name
*/
TEST(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullPropertyName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToProperty(TEST_ENTITY_NAME, NULL, TEST_ASPECT_NAME, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect with a NULL aspect name
*/
TEST(unit_twApi_AddAspectToProperty, addAspectToPropertyWithNullAspectName) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToProperty(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL, value));
	twPrimitive_Delete(value);
}

/**
* Test Plan: Try to add an aspect to a non-existent property
*/
TEST(unit_twApi_AddAspectToProperty, addAspectToNonExistentProperty) {
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_AddAspectToProperty(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TEST_ASPECT_NAME, value));
}

/**
* Test Plan: Try to add an aspect to a property
*/
TEST(unit_twApi_AddAspectToProperty, addAspectToProperty) {
	cJSON *aspects = NULL;
	twPrimitive * value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));
	aspects = getPropertyAspects(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddAspectToProperty(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TEST_ASPECT_NAME, value));
	TEST_ASSERT_NOT_NULL(cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME));
	TEST_ASSERT_EQUAL(TEST_PRIMITIVE_INT_VALUE, cJSON_GetObjectItem(aspects, TEST_ASPECT_NAME)->valueint);
}