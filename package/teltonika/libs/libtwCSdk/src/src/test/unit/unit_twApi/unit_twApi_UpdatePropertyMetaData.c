/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_UpdatePropertyMetaData()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

enum msgCodeEnum doNothing_propertyCb(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
}

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

TEST_GROUP(unit_twApi_UpdatePropertyMetaData);

TEST_SETUP(unit_twApi_UpdatePropertyMetaData) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_UpdatePropertyMetaData) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_UpdatePropertyMetaData) {
	RUN_TEST_CASE(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullApi);
	RUN_TEST_CASE(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullThingName);
	RUN_TEST_CASE(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullPropertyName);
	RUN_TEST_CASE(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNonExistentProperty);
	RUN_TEST_CASE(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataSuccess);
}

/**
 * Test Plan: Try to update metadata with a NULL API
 */
TEST(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_UpdatePropertyMetaData(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0));
}

/**
 * Test Plan: Try to update metadata with a NULL thing name
 */
TEST(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UpdatePropertyMetaData(TW_THING, NULL, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0));
}

/**
* Test Plan: Try to update metadata with a NULL thing name
*/
TEST(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNullPropertyName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UpdatePropertyMetaData(TW_THING, TEST_ENTITY_NAME, NULL, TW_INTEGER, NULL, "ALWAYS", 0));
}

/**
* Test Plan: Try to update metadata to a non-existent property
*/
TEST(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataWithNonExistentProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_UpdatePropertyMetaData(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0));
}

/**
* Test Plan: Try to update metadata to a property
*/
TEST(unit_twApi_UpdatePropertyMetaData, updatePropertyMetaDataSuccess) {
	cJSON *aspects = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
	aspects = getPropertyAspects(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_EQUAL_STRING("ALWAYS", cJSON_GetObjectItem(aspects, "pushType")->valuestring);
	TEST_ASSERT_EQUAL(TW_OK, twApi_UpdatePropertyMetaData(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "NEVER", 0));
	aspects = getPropertyAspects(TEST_ENTITY_NAME, TEST_PROPERTY_NAME);
	TEST_ASSERT_EQUAL_STRING("NEVER", cJSON_GetObjectItem(aspects, "pushType")->valuestring);
}