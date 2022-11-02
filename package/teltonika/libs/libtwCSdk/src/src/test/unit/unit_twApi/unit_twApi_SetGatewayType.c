/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetGatewayType()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetGatewayType);

TEST_SETUP(unit_twApi_SetGatewayType) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetGatewayType) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetGatewayType) {
	RUN_TEST_CASE(unit_twApi_SetGatewayType, setGatewayTypeWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetGatewayType, setGatewayTypeWithNullType);
	RUN_TEST_CASE(unit_twApi_SetGatewayType, setGatewayTypeSuccess);
}

/**
 * Test Plan: Set gateway type with an uninitialized API
 */
TEST(unit_twApi_SetGatewayType, setGatewayTypeWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetGatewayType(TEST_GATEWAY_TYPE));
}

/**
 * Test Plan: Set gateway type with an uninitialized API
 */
TEST(unit_twApi_SetGatewayType, setGatewayTypeWithNullType) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetGatewayType(NULL));
}

/**
 * Test Plan: Set gateway type and verify the API structure has been updated
 */
TEST(unit_twApi_SetGatewayType, setGatewayTypeSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_NULL(tw_api->mh->ws->gatewayType);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetGatewayType(TEST_GATEWAY_TYPE));
	TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_TYPE, tw_api->mh->ws->gatewayType);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetGatewayType(TEST_GATEWAY_TYPE_1));
	TEST_ASSERT_EQUAL_STRING(TEST_GATEWAY_TYPE_1, tw_api->mh->ws->gatewayType);
}