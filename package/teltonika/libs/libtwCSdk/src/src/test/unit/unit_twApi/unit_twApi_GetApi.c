/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_GetApi()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_GetApi);

TEST_SETUP(unit_twApi_GetApi) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_GetApi) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_GetApi) {
	RUN_TEST_CASE(unit_twApi_GetApi, getApi);
}

/**
 * Test Plan: Get the API pointer
 */
TEST(unit_twApi_GetApi, getApi) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(tw_api, twApi_GetApi());
}