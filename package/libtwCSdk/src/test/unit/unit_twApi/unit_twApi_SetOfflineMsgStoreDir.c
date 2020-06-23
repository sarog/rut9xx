/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetOfflineMsgStoreDir()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

char * getOfflineMsgDirectory();

TEST_GROUP(unit_twApi_SetOfflineMsgStoreDir);

TEST_SETUP(unit_twApi_SetOfflineMsgStoreDir) {
	eatLogs();
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_FILE);
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_DIFFERENT);
}

TEST_TEAR_DOWN(unit_twApi_SetOfflineMsgStoreDir) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetOfflineMsgStoreDir) {
	RUN_TEST_CASE(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithNullApi);
	RUN_TEST_CASE(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithGoodPath);
	RUN_TEST_CASE(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithBadPath);
	RUN_TEST_CASE(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithNullPath);
}

/**
 * Test Plan: Try to set the offline msg with a NULL API
 */
TEST(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON, twApi_SetOfflineMsgStoreDir(TEST_OFFLINE_MSG_STORE_DIR));
}

/**
 * Test Plan: Try to set the offline msg dir to a valid non-default path
 */
TEST(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithGoodPath) {
	char * dir = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_NULL(getOfflineMsgDirectory());
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	dir = getOfflineMsgDirectory();
	TEST_ASSERT_EQUAL_STRING(OFFLINE_MSG_STORE_LOCATION_FILE, dir);
	TW_FREE(dir);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_DIFFERENT));
	dir = getOfflineMsgDirectory();
	TEST_ASSERT_EQUAL_STRING(OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE, dir);
	TW_FREE(dir);
}

/**
 * Test Plan: Try to set the offline msg dir to an invalid path
 */
TEST(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithBadPath) {
	int ret;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	ret = twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_BAD);
	TEST_ASSERT_TRUE(TW_INVALID_MSG_STORE_DIR == ret||TW_ERROR_WRITING_FILE == ret);
}

/**
 * Test Plan: Try to set the offline msg dir to a NULL path
 */
TEST(unit_twApi_SetOfflineMsgStoreDir, setOfflineMsgStoreDirWithNullPath) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetOfflineMsgStoreDir(NULL));
}