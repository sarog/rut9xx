/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterProperty()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_RegisterProperty);

TEST_SETUP(unit_twApi_RegisterProperty) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterProperty) {
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertyWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertyWithNullPropertyName);
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertyWithNullPropertyHandler);
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertySuccess);
	RUN_TEST_CASE(unit_twApi_RegisterProperty, registerPropertyTwice);
}

static enum msgCodeEnum doNothing_propertyCb(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	return TWX_SUCCESS;
}


/**
 * Test Plan: Try to register a property with a NULL API
 */
TEST(unit_twApi_RegisterProperty, registerPropertyWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
}

/**
 * Test Plan: Try to register a property with a NULL thing name
 */
TEST(unit_twApi_RegisterProperty, registerPropertyWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterProperty(TW_THING, NULL, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
}

/**
 * Test Plan: Try to register a property with a NULL property name
 */
TEST(unit_twApi_RegisterProperty, registerPropertyWithNullPropertyName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, NULL, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
}

/**
 * Test Plan: Try to register a property with a NULL property handler
 */
TEST(unit_twApi_RegisterProperty, registerPropertyWithNullPropertyHandler) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, NULL, NULL));
}

/**
 * Test Plan: Try to register a property
 */
TEST(unit_twApi_RegisterProperty, registerPropertySuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
}

/**
 * Test Plan: Try to register a property twice
 */
TEST(unit_twApi_RegisterProperty, registerPropertyTwice) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_RegisterProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0, doNothing_propertyCb, NULL));
}