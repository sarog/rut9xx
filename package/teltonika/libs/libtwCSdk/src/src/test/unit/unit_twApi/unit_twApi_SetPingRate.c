/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetPingRate()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetPingRate);

TEST_SETUP(unit_twApi_SetPingRate) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetPingRate) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetPingRate) {
	RUN_TEST_CASE(unit_twApi_SetPingRate, setPingRateWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetPingRate, setPingRate);
}

/**
 * Test Plan: Set ping rate with an uninitialized API
 */
TEST(unit_twApi_SetPingRate, setPingRateWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetPingRate(TEST_PING_RATE));
}

/**
 * Test Plan: Set ping rate and verify the API structure has been updated
 */
TEST(unit_twApi_SetPingRate, setPingRate) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(PING_RATE, tw_api->ping_rate);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetPingRate(TEST_PING_RATE));
	TEST_ASSERT_EQUAL(TEST_PING_RATE, tw_api->ping_rate);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetPingRate(TEST_PING_RATE_1));
	TEST_ASSERT_EQUAL(TEST_PING_RATE_1, tw_api->ping_rate);
}