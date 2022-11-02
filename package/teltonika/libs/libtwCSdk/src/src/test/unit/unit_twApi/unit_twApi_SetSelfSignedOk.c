/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetSelfSignedOk()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetSelfSignedOk);

TEST_SETUP(unit_twApi_SetSelfSignedOk) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_SetSelfSignedOk) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetSelfSignedOk) {
	RUN_TEST_CASE(unit_twApi_SetSelfSignedOk, setSelfSignedOk);
}

/**
 * Test Plan: Set self signed certificates OK and verify that the API structure has been updated
 */
TEST(unit_twApi_SetSelfSignedOk, setSelfSignedOk) {
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->selfSignedOk);
	twApi_SetSelfSignedOk();
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->selfSignedOk);
}