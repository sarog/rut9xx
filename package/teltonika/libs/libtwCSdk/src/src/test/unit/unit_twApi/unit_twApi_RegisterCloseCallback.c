/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterCloseCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test close callbacks */
static int testCloseCallbackAlpha(struct twWs * ws, const char * data, size_t length) { return TW_OK; }
static int testCloseCallbackBeta(struct twWs * ws, const char * data, size_t length) { return TW_OK; }

TEST_GROUP(unit_twApi_RegisterCloseCallback);

TEST_SETUP(unit_twApi_RegisterCloseCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterCloseCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterCloseCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackSuccess);
}

/**
 * Test Plan: Try to register a close callback with a NULL API
 */
TEST(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterCloseCallback(&testCloseCallbackAlpha));
}

/**
 * Test Plan: Register a close callback and verify it is called on close (valid use case to NULL the callback)
 */
TEST(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackWithNullCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterCloseCallback(NULL));
}

/**
 * Test Plan: Register a close callback and verify it is set properly
 */
TEST(unit_twApi_RegisterCloseCallback, RegisterCloseCallbackSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(tw_api->mh->on_ws_close);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterCloseCallback(&testCloseCallbackAlpha));
	TEST_ASSERT_EQUAL(testCloseCallbackAlpha, tw_api->mh->on_ws_close);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterCloseCallback(&testCloseCallbackBeta));
	TEST_ASSERT_EQUAL(testCloseCallbackBeta, tw_api->mh->on_ws_close);
}