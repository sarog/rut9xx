/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterDefaultRequestHandler()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test request handler functions */
static twMessage * testDefaultRequestHandlerAlpha(twMessage * msg) { return NULL; }
static twMessage * testDefaultRequestHandlerBeta(twMessage * msg) { return NULL; }

TEST_GROUP(unit_twApi_RegisterDefaultRequestHandler);

TEST_SETUP(unit_twApi_RegisterDefaultRequestHandler) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterDefaultRequestHandler) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterDefaultRequestHandler) {
	RUN_TEST_CASE(unit_twApi_RegisterDefaultRequestHandler, registerHandlerWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterDefaultRequestHandler, registerHandlerSuccess);
}

/**
 * Test Plan: Try to register a handler with a NULL API
 */
TEST(unit_twApi_RegisterDefaultRequestHandler, registerHandlerWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterDefaultRequestHandler(&testDefaultRequestHandlerAlpha));
}

/**
 * Test Plan: Register a couple different default request handlers
 */
TEST(unit_twApi_RegisterDefaultRequestHandler, registerHandlerSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterDefaultRequestHandler(&testDefaultRequestHandlerAlpha));
	TEST_ASSERT_EQUAL(testDefaultRequestHandlerAlpha, tw_api->defaultRequestHandler);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterDefaultRequestHandler(&testDefaultRequestHandlerBeta));
	TEST_ASSERT_EQUAL(testDefaultRequestHandlerBeta, tw_api->defaultRequestHandler);
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterDefaultRequestHandler(NULL));
	TEST_ASSERT_NULL(tw_api->defaultRequestHandler);
}
