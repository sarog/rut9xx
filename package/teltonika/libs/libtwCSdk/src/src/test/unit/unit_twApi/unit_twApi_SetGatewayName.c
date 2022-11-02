/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetGatewayName()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetGatewayName);

TEST_SETUP(unit_twApi_SetGatewayName) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetGatewayName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetGatewayName) {
	RUN_TEST_CASE(unit_twApi_SetGatewayName, setGatewayNameWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetGatewayName, setGatewayNameWithNullName);
	RUN_TEST_CASE(unit_twApi_SetGatewayName, setGatewayNameSuccess);
}

/**
 * Test Plan: Set gateway name with an uninitialized API
 */
TEST(unit_twApi_SetGatewayName, setGatewayNameWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetGatewayName(TEST_GATEWAY_NAME));
}

/**
 * Test Plan: Set gateway name with NULL name
 */
TEST(unit_twApi_SetGatewayName, setGatewayNameWithNullName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetGatewayName(NULL));
}

/**
 * Test Plan: Set gateway name and verify the API structure has been updated
 */
TEST(unit_twApi_SetGatewayName, setGatewayNameSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(tw_api->mh->ws->gatewayName);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetGatewayName(TEST_GATEWAY_NAME));
	TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_NAME, tw_api->mh->ws->gatewayName);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetGatewayName(TEST_GATEWAY_NAME_1));
	TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_NAME_1, tw_api->mh->ws->gatewayName);
}