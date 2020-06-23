/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for findCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_findCallback);

TEST_SETUP(unit_twApi_findCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_findCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_findCallback) {
	RUN_TEST_CASE(unit_twApi_findCallback, CSDK_1262);
}

void * findCallback(enum entityTypeEnum entityType, char *entityName,
                    enum characteristicEnum characteristicType, char *characteristicName, void **userdata);

/**
 * CSDK-1262
 *
 * Test Plan: Verify FileTransfer and Tunneling callbacks in findCallback return properly, unit test for CSDK-1258
 */
TEST(unit_twApi_findCallback, CSDK_1262) {
	void * userdata = NULL;
	service_cb callback = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI,
	                                          TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twFileManager_Create());
	callback = (service_cb)findCallback(TW_THING, "TestFileTransfer",
	                                    TW_SERVICES, "StartFileTransfer", &userdata);
	TEST_ASSERT_NOT_NULL(callback);
	callback = NULL;
	callback = (service_cb)findCallback(TW_THING, "TestTunneling",
	                                    TW_SERVICES, "StartTunnel", &userdata);
	TEST_ASSERT_NOT_NULL(callback);
	TEST_ASSERT_EQUAL(TW_OK, twFileManager_Delete());
}