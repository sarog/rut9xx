/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_ReadProperty()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_ReadProperty);

TEST_SETUP(unit_twApi_ReadProperty) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_ReadProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_ReadProperty) {
	RUN_TEST_CASE(unit_twApi_ReadProperty, readPropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_ReadProperty, readPropertyWithNullResultPointer);
	RUN_TEST_CASE(unit_twApi_ReadProperty, readPropertyWhileUnconnected);
	RUN_TEST_CASE(unit_twApi_ReadProperty, readProperty);
}

/**
 * Test Plan: Read a property with a NULL API
 */
TEST(unit_twApi_ReadProperty, readPropertyWithNullApi) {
	twPrimitive * result = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_ReadProperty should return TW_NULL_OR_INVALID_API_SINGLETON");
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_ReadProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &result, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Read a property with a NULL result pointer
 */
TEST(unit_twApi_ReadProperty, readPropertyWithNullResultPointer) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_ReadProperty should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_ReadProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Read a property while disconnected
 */
TEST(unit_twApi_ReadProperty, readPropertyWhileUnconnected) {
	twPrimitive * result = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_ReadProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &result, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Mock successful message sending and read a property value
 */
TEST(unit_twApi_ReadProperty, readProperty) {
	twPrimitive * result = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, &result, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}