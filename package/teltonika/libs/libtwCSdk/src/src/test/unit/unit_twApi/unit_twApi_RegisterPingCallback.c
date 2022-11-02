/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterPingCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_RegisterPingCallback);

TEST_SETUP(unit_twApi_RegisterPingCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterPingCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterPingCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterPingCallback, registerCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterPingCallback, registerCallbackSuccess);
}

static int doNothing_pingCb(struct twWs * ws, const char * data, size_t length) {
	return TW_OK;
}

/**
 * Test Plan: Register a ping callback with a NULL API
 */
TEST(unit_twApi_RegisterPingCallback, registerCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterPingCallback(doNothing_pingCb));
}

/**
 * Test Plan: Register a ping callback and verify it has been set properly
 */
TEST(unit_twApi_RegisterPingCallback, registerCallbackSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPingCallback(doNothing_pingCb));
	TEST_ASSERT_EQUAL(doNothing_pingCb, tw_api->mh->on_ping);
	// TODO: This doesn't actually restore default pong behavior.  Is this intentional?
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterPingCallback(NULL));
	TEST_ASSERT_NULL(tw_api->mh->on_ping);
}