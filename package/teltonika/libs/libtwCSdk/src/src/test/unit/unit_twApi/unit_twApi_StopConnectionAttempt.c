/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_StopConnectionAttempt()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_StopConnectionAttempt);

TEST_SETUP(unit_twApi_StopConnectionAttempt) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_StopConnectionAttempt) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_StopConnectionAttempt) {
	RUN_TEST_CASE(unit_twApi_StopConnectionAttempt, checkStopConnectionAttempt);
}

/**
 * Test Plan: Set the connection in progress API flag, stop the connection attempt, and confirm that the flag has been reset
 */
TEST(unit_twApi_StopConnectionAttempt, checkStopConnectionAttempt) {
	TEST_ASSERT_FALSE(tw_api->connectionInProgress);
	tw_api->connectionInProgress = TRUE;
	TEST_ASSERT_EQUAL(TW_OK, twApi_StopConnectionAttempt());
	TEST_ASSERT_FALSE(tw_api->connectionInProgress);
}