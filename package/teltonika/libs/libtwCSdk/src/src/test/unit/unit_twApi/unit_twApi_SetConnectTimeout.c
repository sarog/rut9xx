/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetConnectTimeout()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetConnectTimeout);

TEST_SETUP(unit_twApi_SetConnectTimeout) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetConnectTimeout) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetConnectTimeout) {
	RUN_TEST_CASE(unit_twApi_SetConnectTimeout, setConnectTimeoutWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetConnectTimeout, setConnectTimeoutSuccess);
}

/**
 * Test Plan: Set connect timeout with an uninitialized API
 */
TEST(unit_twApi_SetConnectTimeout, setConnectTimeoutWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetConnectTimeout(TEST_CONNECT_TIMEOUT));
}

/**
 * Test Plan: Set connect timeout and verify the API structure has been updated
 */
TEST(unit_twApi_SetConnectTimeout, setConnectTimeoutSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(CONNECT_TIMEOUT, tw_api->connect_timeout);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetConnectTimeout(TEST_CONNECT_TIMEOUT));
	TEST_ASSERT_EQUAL(TEST_CONNECT_TIMEOUT, tw_api->connect_timeout);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetConnectTimeout(TEST_CONNECT_TIMEOUT_1));
	TEST_ASSERT_EQUAL(TEST_CONNECT_TIMEOUT_1, tw_api->connect_timeout);
}