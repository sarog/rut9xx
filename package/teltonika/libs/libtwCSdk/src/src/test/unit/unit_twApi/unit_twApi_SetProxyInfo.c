/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetProxyInfo()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetProxyInfo);

TEST_SETUP(unit_twApi_SetProxyInfo) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetProxyInfo) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetProxyInfo) {
	RUN_TEST_CASE(unit_twApi_SetProxyInfo, setProxyInfoWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetProxyInfo, setProxyInfoWithNullHost);
	RUN_TEST_CASE(unit_twApi_SetProxyInfo, setProxyInfoWithPortZero);
	RUN_TEST_CASE(unit_twApi_SetProxyInfo, setProxyInfoSuccess);
}

static void testProxyPassWdCb(char *passWd, unsigned int len){
	strncpy(passWd, TEST_PROXY_PASS, len);
}

static void testProxyPassWdCb_1(char *passWd, unsigned int len){
	strncpy(passWd, TEST_PROXY_PASS_1, len);
}

/**
 * Test Plan: Set ping rate with an uninitialized API
 */
TEST(unit_twApi_SetProxyInfo, setProxyInfoWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetProxyInfo(TEST_PROXY_HOST, TEST_PROXY_PORT, TEST_PROXY_USER, testProxyPassWdCb));
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_ClearProxyInfo());
}

/**
 * Test Plan: Set proxy info with NULL proxy host
 */
TEST(unit_twApi_SetProxyInfo, setProxyInfoWithNullHost) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetProxyInfo(NULL, TEST_PROXY_PORT, TEST_PROXY_USER, testProxyPassWdCb));
	/* Verify default values */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
}

/**
 * Test Plan: Set proxy info with port zero
 */
TEST(unit_twApi_SetProxyInfo, setProxyInfoWithPortZero) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetProxyInfo(TEST_PROXY_HOST, 0, TEST_PROXY_USER, testProxyPassWdCb));
	/* Verify default values */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
}

/**
 * Test Plan: Set proxy info and verify that the API structure has been updated
 */
TEST(unit_twApi_SetProxyInfo, setProxyInfoSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Verify default values */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	/* Set new proxy info */
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetProxyInfo(TEST_PROXY_HOST, TEST_PROXY_PORT, TEST_PROXY_USER, testProxyPassWdCb));
	/* Verify updated values */
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_HOST, tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(TEST_PROXY_PORT, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_USER, tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_EQUAL(testProxyPassWdCb, tw_api->mh->ws->connection->connection->proxyPassCallback);
	/* Set new proxy info */
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetProxyInfo(TEST_PROXY_HOST_1, TEST_PROXY_PORT_1, TEST_PROXY_USER_1, testProxyPassWdCb_1));
	/* Verify updated values */
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_HOST_1, tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(TEST_PROXY_PORT_1, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_USER_1, tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_EQUAL(testProxyPassWdCb_1, tw_api->mh->ws->connection->connection->proxyPassCallback);
	/* Clear proxy info */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ClearProxyInfo());
	/* Verify cleared values */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
	/* Set new proxy info */
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetProxyInfo(TEST_PROXY_HOST, TEST_PROXY_PORT, TEST_PROXY_USER, testProxyPassWdCb));
	/* Verify updated values */
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_HOST, tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(TEST_PROXY_PORT, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_EQUAL_STRING(TEST_PROXY_USER, tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_EQUAL(testProxyPassWdCb, tw_api->mh->ws->connection->connection->proxyPassCallback);
	/* Clear proxy info */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ClearProxyInfo());
	/* Verify cleared values */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
	/* Clear proxy info again */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ClearProxyInfo());
	/* Verify values are still cleared */
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyHost);
	TEST_ASSERT_EQUAL(0, tw_api->mh->ws->connection->connection->proxyPort);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyUser);
	TEST_ASSERT_NULL(tw_api->mh->ws->connection->connection->proxyPassCallback);
}