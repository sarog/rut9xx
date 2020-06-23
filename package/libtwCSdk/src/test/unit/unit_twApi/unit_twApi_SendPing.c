/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SendPing()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SendPing);

TEST_SETUP(unit_twApi_SendPing) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SendPing) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SendPing) {
	RUN_TEST_CASE(unit_twApi_SendPing, sendPingWithNullApi);
	RUN_TEST_CASE(unit_twApi_SendPing, sendPingWhileDisconnected);
	RUN_TEST_CASE(unit_twApi_SendPing, sendPingSuccess);
}

/**
 * Test Plan: NULL api ping
 */
TEST(unit_twApi_SendPing, sendPingWithNullApi) {
	char * content = "ping";
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SendPing(content));
}

/**
 * Test Plan: Attempt to ping while disconnected
 */
TEST(unit_twApi_SendPing, sendPingWhileDisconnected) {
	char * content = "ping";
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_WEBSOCKET_NOT_CONNECTED, twApi_SendPing(content));
}

static int stub_twWs_SendPing_MockSuccess(twWs *ws, char *msg) {
	return TW_OK;
}

/**
 * Test Plan: Attempt successful ping
 */
TEST(unit_twApi_SendPing, sendPingSuccess) {
	char * content = "ping";
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->twWs_SendPing = stub_twWs_SendPing_MockSuccess;
	TEST_ASSERT_EQUAL(TW_OK, twApi_SendPing(content));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}