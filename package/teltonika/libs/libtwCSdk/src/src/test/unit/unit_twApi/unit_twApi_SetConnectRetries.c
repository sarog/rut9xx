/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetConnectRetries()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetConnectRetries);

TEST_SETUP(unit_twApi_SetConnectRetries) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetConnectRetries) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetConnectRetries) {
	RUN_TEST_CASE(unit_twApi_SetConnectRetries, setConnectRetriesWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetConnectRetries, setConnectRetries);
}

/**
 * Test Plan: Set connect retries with an uninitialized API
 */
TEST(unit_twApi_SetConnectRetries, setConnectRetriesWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetConnectRetries(TEST_CONNECT_RETRIES));
}

/**
 * Test Plan: Set connect retries and verify the API structure has been updated
 */
TEST(unit_twApi_SetConnectRetries, setConnectRetries) {
	TEST_IGNORE_MESSAGE("CSDK-1364: Connect retries default does not match the value defined in twDefaultSettings.h");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(CONNECT_RETRIES, tw_api->connect_retries);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetConnectRetries(TEST_CONNECT_RETRIES));
	TEST_ASSERT_EQUAL_UINT32(TEST_CONNECT_RETRIES, tw_api->connect_retries);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetConnectRetries(TEST_CONNECT_RETRIES_1));
	TEST_ASSERT_EQUAL(TEST_CONNECT_RETRIES_1, tw_api->connect_retries);
}