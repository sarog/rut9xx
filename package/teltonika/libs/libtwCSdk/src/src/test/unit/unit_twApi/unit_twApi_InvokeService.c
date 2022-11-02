/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_InvokeService()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_InvokeService);

TEST_SETUP(unit_twApi_InvokeService) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_InvokeService) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_InvokeService) {
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceWithNullApi);
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceWithNullThingName);
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceWithNullServiceName);
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceWithNullResultPointer);
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceWhileDisconnected);
	RUN_TEST_CASE(unit_twApi_InvokeService, invokeServiceSuccess);
}

/**
 * Test Plan: Invoke a service with a NULL API
 */
TEST(unit_twApi_InvokeService, invokeServiceWithNullApi) {
	twInfoTable * result = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_InvokeService should return TW_NULL_OR_INVALID_API_SINGLETON");
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_InvokeService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, &result, FALSE, FALSE));
}

/**
 * Test Plan: Invoke a service with a NULL thing name
 */
TEST(unit_twApi_InvokeService, invokeServiceWithNullThingName) {
	twInfoTable * result = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_InvokeService should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_InvokeService(TW_THING, NULL, TEST_SERVICE_NAME, NULL, &result, FALSE, FALSE));
}

/**
 * Test Plan: Invoke a service with a NULL service name
 */
TEST(unit_twApi_InvokeService, invokeServiceWithNullServiceName) {
	twInfoTable * result = NULL;
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_InvokeService should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_InvokeService(TW_THING, TEST_ENTITY_NAME, NULL, NULL, &result, FALSE, FALSE));
}

/**
 * Test Plan: Invoke a service with a NULL result pointer
 */
TEST(unit_twApi_InvokeService, invokeServiceWithNullResultPointer) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_InvokeService should return TW_INVALID_PARAM");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_InvokeService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, FALSE, FALSE));
}

/**
 * Test Plan: Invoke a service while disconnected
 */
TEST(unit_twApi_InvokeService, invokeServiceWhileDisconnected) {
	twInfoTable * result = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_InvokeService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, &result, FALSE, FALSE));
}

/**
 * Test Plan: Mock successful message sending and invoke a service
 */
TEST(unit_twApi_InvokeService, invokeServiceSuccess) {
	twInfoTable * result = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, &result, FALSE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}