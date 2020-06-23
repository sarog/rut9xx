/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_ConnectionInProgress()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_ConnectionInProgress);

TEST_SETUP(unit_twApi_ConnectionInProgress) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_ConnectionInProgress) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_ConnectionInProgress) {
	RUN_TEST_CASE(unit_twApi_ConnectionInProgress, checkConnectionInProgress);
}

/**
 * Test Plan: Set the connection in progress API flag and verify that the tested function returns the proper state
 */
TEST(unit_twApi_ConnectionInProgress, checkConnectionInProgress) {
	TEST_ASSERT_FALSE(twApi_ConnectionInProgress());
	tw_api->connectionInProgress = TRUE;
	TEST_ASSERT_TRUE(twApi_ConnectionInProgress());
	tw_api->connectionInProgress = FALSE;
	TEST_ASSERT_FALSE(twApi_ConnectionInProgress());
}