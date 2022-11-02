/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_PushProperties()
 */

#include <twBaseTypes.h>
#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_PushProperties);

TEST_SETUP(unit_twApi_PushProperties) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_PushProperties) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_PushProperties) {
	RUN_TEST_CASE(unit_twApi_PushProperties, createPropertyList);
	RUN_TEST_CASE(unit_twApi_PushProperties, pushPropertiesWithNullApi);
	RUN_TEST_CASE(unit_twApi_PushProperties, pushPropertiesWhileDisconnected);
	RUN_TEST_CASE(unit_twApi_PushProperties, pushPropertiesWhithNullThingName);
	RUN_TEST_CASE(unit_twApi_PushProperties, pushPropertiesWhithNullPropertyList);
	RUN_TEST_CASE(unit_twApi_PushProperties, pushPropertiesSuccess);
}

/**
 * Test Plan: Create a property list, add some properties to it, and verify they have been added properly
 */
TEST(unit_twApi_PushProperties, createPropertyList) {
	propertyList * list = NULL;
	list = twApi_CreatePropertyList(TEST_PROPERTY_NAME_1, twPrimitive_CreateFromInteger(1), NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(1, list->count);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_2, twPrimitive_CreateFromInteger(2), NULL));
	TEST_ASSERT_EQUAL(2, list->count);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_3, twPrimitive_CreateFromInteger(3), NULL));
	TEST_ASSERT_EQUAL(3, list->count);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME_1, ((twProperty*)twList_GetByIndex(list, 0)->value)->name);
	TEST_ASSERT_EQUAL(1, ((twProperty*)twList_GetByIndex(list, 0)->value)->value->val.integer);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME_2, ((twProperty*)twList_GetByIndex(list, 1)->value)->name);
	TEST_ASSERT_EQUAL(2, ((twProperty*)twList_GetByIndex(list, 1)->value)->value->val.integer);
	TEST_ASSERT_EQUAL_STRING(TEST_PROPERTY_NAME_3, ((twProperty*)twList_GetByIndex(list, 2)->value)->name);
	TEST_ASSERT_EQUAL(3, ((twProperty*)twList_GetByIndex(list, 2)->value)->value->val.integer);
	TEST_ASSERT_EQUAL(TW_OK, twApi_DeletePropertyList(list));
}

/**
 * Test Plan: Push properties with a NULL API
 */
TEST(unit_twApi_PushProperties, pushPropertiesWithNullApi) {
	propertyList * list = NULL;
	list = twApi_CreatePropertyList(TEST_PROPERTY_NAME_1, twPrimitive_CreateFromInteger(1), NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_2, twPrimitive_CreateFromInteger(2), NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_3, twPrimitive_CreateFromInteger(3), NULL));
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_PushProperties(TW_THING, TEST_ENTITY_NAME, list, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_DeletePropertyList(list));
}

/**
 * Test Plan: Attempt to push to platform while not connected
 */
TEST(unit_twApi_PushProperties, pushPropertiesWhileDisconnected) {
	propertyList * list = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	list = twApi_CreatePropertyList(TEST_PROPERTY_NAME_1, twPrimitive_CreateFromInteger(1), NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_2, twPrimitive_CreateFromInteger(2), NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_3, twPrimitive_CreateFromInteger(3), NULL));
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_PushProperties(TW_THING, TEST_ENTITY_NAME, list, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_DeletePropertyList(list));
}

/**
 * Test Plan: Push properties with a NULL thing name
 */
TEST(unit_twApi_PushProperties, pushPropertiesWhithNullThingName) {
	propertyList * list = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_PushProperties should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	list = twApi_CreatePropertyList(TEST_PROPERTY_NAME_1, twPrimitive_CreateFromInteger(1), NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_2, twPrimitive_CreateFromInteger(2), NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_3, twPrimitive_CreateFromInteger(3), NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_PushProperties(TW_THING, NULL, list, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_DeletePropertyList(list));
}

/**
 * Test Plan: Push properties with a NULL property list
 */
TEST(unit_twApi_PushProperties, pushPropertiesWhithNullPropertyList) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_PushProperties should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_PushProperties(TW_THING, TEST_ENTITY_NAME, NULL, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Mock successful message sending and push properties
 */
TEST(unit_twApi_PushProperties, pushPropertiesSuccess) {
	propertyList * list = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	list = twApi_CreatePropertyList(TEST_PROPERTY_NAME_1, twPrimitive_CreateFromInteger(1), NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_2, twPrimitive_CreateFromInteger(2), NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_AddPropertyToList(list, TEST_PROPERTY_NAME_3, twPrimitive_CreateFromInteger(3), NULL));
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushProperties(TW_THING, TEST_ENTITY_NAME, list, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_DeletePropertyList(list));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}