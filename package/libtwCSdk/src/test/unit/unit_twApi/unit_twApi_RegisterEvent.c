/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterEvent()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_RegisterEvent);

TEST_SETUP(unit_twApi_RegisterEvent) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterEvent) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterEvent) {
	RUN_TEST_CASE(unit_twApi_RegisterEvent, registerEventWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterEvent, registerEventWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterEvent, registerEventWithNullEventName);
	RUN_TEST_CASE(unit_twApi_RegisterEvent, registerEventSuccess);
	RUN_TEST_CASE(unit_twApi_RegisterEvent, registerEventTwice);
}

/**
 * Test Plan: Try to register an event with a NULL API
 */
TEST(unit_twApi_RegisterEvent, registerEventWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterEvent(TW_THING, NULL, TEST_EVENT_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to register an event with a NULL thing name
 */
TEST(unit_twApi_RegisterEvent, registerEventWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterEvent(TW_THING, NULL, TEST_EVENT_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to register an event with a NULL event name
 */
TEST(unit_twApi_RegisterEvent, registerEventWithNullEventName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, NULL, NULL, NULL));
}

/**
 * Test Plan: Try to register an event
 */
TEST(unit_twApi_RegisterEvent, registerEventSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
}

/**
 * Test Plan: Try to register an event twice
 */
TEST(unit_twApi_RegisterEvent, registerEventTwice) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_RegisterEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, NULL));
}