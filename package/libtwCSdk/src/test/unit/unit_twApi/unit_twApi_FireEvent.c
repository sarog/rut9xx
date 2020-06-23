/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_FireEvent()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_FireEvent);

TEST_SETUP(unit_twApi_FireEvent) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_FireEvent) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_FireEvent) {
	RUN_TEST_CASE(unit_twApi_FireEvent, fireEventWithNullApi);
	RUN_TEST_CASE(unit_twApi_FireEvent, fireEventWithNullThingName);
	RUN_TEST_CASE(unit_twApi_FireEvent, fireEventWithNullEventName);
	RUN_TEST_CASE(unit_twApi_FireEvent, fireEventWhileDisconnected);
	RUN_TEST_CASE(unit_twApi_FireEvent, fireEvent);
}

/**
 * Test Plan: Fire an event with a NULL API
 */
TEST(unit_twApi_FireEvent, fireEventWithNullApi) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_FireEvent should return TW_NULL_OR_INVALID_API_SINGLETON");
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_FireEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, FALSE, FALSE));
}

/**
 * Test Plan: Fire an event with a NULL thing name
 */
TEST(unit_twApi_FireEvent, fireEventWithNullThingName) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_FireEvent should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_FireEvent(TW_THING, NULL, TEST_EVENT_NAME, NULL, FALSE, FALSE));
}

/**
 * Test Plan: Fire an event with a NULL service name
 */
TEST(unit_twApi_FireEvent, fireEventWithNullEventName) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_FireEvent should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_FireEvent(TW_THING, TEST_ENTITY_NAME, NULL, NULL, FALSE, FALSE));
}

/**
 * Test Plan: Fire an event while disconnected
 */
TEST(unit_twApi_FireEvent, fireEventWhileDisconnected) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_FireEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, FALSE, FALSE));
}

/**
 * Test Plan: Mock successful message sending and fire an event
 */
TEST(unit_twApi_FireEvent, fireEvent) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_EQUAL(TW_OK, twApi_FireEvent(TW_THING, TEST_ENTITY_NAME, TEST_EVENT_NAME, NULL, FALSE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}