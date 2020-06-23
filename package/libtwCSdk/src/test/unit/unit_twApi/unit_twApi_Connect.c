/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_Connect()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_Connect);

TEST_SETUP(unit_twApi_Connect) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_Connect) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_Connect) {
	RUN_TEST_CASE(unit_twApi_Connect, connectWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_Connect, disconnectWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_Connect, mockConnect);
	RUN_TEST_CASE(unit_twApi_Connect, mockConnectTwiceDisconnectTwice);
}

/**
 * Test Plan: Try to connect with an uninitialized API
 */
TEST(unit_twApi_Connect, connectWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
}

/**
 * Test Plan: Try to disconnect with an uninitialized API
 */
TEST(unit_twApi_Connect, disconnectWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_Disconnect("disconnectWithUninitalizedApi"));
}

/**
 * Test Plan: Stub out internal functions to mock API connection then connect and disconnect
 */
TEST(unit_twApi_Connect, mockConnect) {
	/* Initialize API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Stub internal functions to mock API connection */
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->twWs_Connect = stub_twWs_Connect_MockSuccess;
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	/* Mock connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	/* Mock disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("mockConnect"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	/* Reset stubs */
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}

/**
 * Test Plan: Stub out internal functions to mock API connection then connect twice and disconnect twice
 */
TEST(unit_twApi_Connect, mockConnectTwiceDisconnectTwice) {
	/* Initialize API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Stub internal functions to mock API connection */
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->twWs_Connect = stub_twWs_Connect_MockSuccess;
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	/* Mock connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	/* Mock disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("mockConnectTwiceDisconnectTwice"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("mockConnectTwiceDisconnectTwice"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	/* Reset stubs */
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}