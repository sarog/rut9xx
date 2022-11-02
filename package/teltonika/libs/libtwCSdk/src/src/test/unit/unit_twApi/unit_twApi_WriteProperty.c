/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_WriteProperty()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_WriteProperty);

TEST_SETUP(unit_twApi_WriteProperty) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_WriteProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_WriteProperty) {
	RUN_TEST_CASE(unit_twApi_WriteProperty, writePropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_WriteProperty, writePropertyWithNullResultPointer);
	RUN_TEST_CASE(unit_twApi_WriteProperty, writePropertyWhileUnconnected);
	RUN_TEST_CASE(unit_twApi_WriteProperty, writePropertySuccess);
}

/**
 * Test Plan: Write a property with a NULL API
 */
TEST(unit_twApi_WriteProperty, writePropertyWithNullApi) {
	twPrimitive * value = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_WriteProperty should return TW_NULL_OR_INVALID_API_SINGLETON");
	value = twPrimitive_Create();
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_WriteProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, value, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Write a property with a NULL result pointer
 */
TEST(unit_twApi_WriteProperty, writePropertyWithNullResultPointer) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_WriteProperty should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_WriteProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, NULL, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Write a property while disconnected
 */
TEST(unit_twApi_WriteProperty, writePropertyWhileUnconnected) {
	twPrimitive * value = NULL;
	value = twPrimitive_Create();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_WriteProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, value, TEST_TIMEOUT, FALSE));
}

/**
 * Test Plan: Mock successful message sending and write a property value
 */
TEST(unit_twApi_WriteProperty, writePropertySuccess) {
	twPrimitive * value = NULL;
	value = twPrimitive_Create();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_EQUAL(TW_OK, twApi_WriteProperty(TW_THING, TEST_ENTITY_NAME, TEST_PROPERTY_NAME, value, TEST_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}