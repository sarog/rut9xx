/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterConnectCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test connect callbacks */
static int testConnectCallbackAlpha(struct twWs * ws, const char * data, size_t length) { return TW_OK; }
static int testConnectCallbackBeta(struct twWs * ws, const char * data, size_t length) { return TW_OK; }

TEST_GROUP(unit_twApi_RegisterConnectCallback);

TEST_SETUP(unit_twApi_RegisterConnectCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterConnectCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterConnectCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterConnectCallback, registerConnectCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterConnectCallback, registerConnectCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterConnectCallback, registerConnectCallbackSuccess);
}

/**
 * Test Plan: Try to register a connect callback with a NULL API
 */
TEST(unit_twApi_RegisterConnectCallback, registerConnectCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterConnectCallback(&testConnectCallbackAlpha));
}

/**
 * Test Plan: Try to register a connect callback with a NULL callback (valid use case to NULL the callback)
 */
TEST(unit_twApi_RegisterConnectCallback, registerConnectCallbackWithNullCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterConnectCallback(NULL));
}

/**
 * Test Plan: Register a connect callback and verify it is set properly
 */
TEST(unit_twApi_RegisterConnectCallback, registerConnectCallbackSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(tw_api->mh->on_ws_connected);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterConnectCallback(&testConnectCallbackAlpha));
	TEST_ASSERT_EQUAL(testConnectCallbackAlpha, tw_api->mh->on_ws_connected);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterConnectCallback(&testConnectCallbackBeta));
	TEST_ASSERT_EQUAL(testConnectCallbackBeta, tw_api->mh->on_ws_connected);
}