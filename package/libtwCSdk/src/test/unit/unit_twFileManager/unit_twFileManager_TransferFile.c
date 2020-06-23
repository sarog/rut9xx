/*
 * Copyright 2018, PTC, Inc. All rights reserved.
 *
 * Unit tests for twFileManager_TransferFile().
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"
#include "TestUtilities.h"
#include "unity_fixture.h"

int twFileManager_TransferFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
                               const char * targetRepo, const char * targetPath, const char * targetFile,
                               uint32_t timeout, char asynch, char ** tid);

int mock_twApi_InvokeService(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	return TW_OK;
}

TEST_GROUP(unit_twFileManager_TransferFile);

TEST_SETUP(unit_twFileManager_TransferFile) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
}

TEST_TEAR_DOWN(unit_twFileManager_TransferFile) {
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twFileManager_TransferFile) {
	RUN_TEST_CASE(unit_twFileManager_TransferFile, TransferFile_EqualMaximumTimeout);
	RUN_TEST_CASE(unit_twFileManager_TransferFile, TransferFile_ExceedMaximumTimeout);
}

/**
 * Test Plan: Call twFileManager_TransferFile() with a timeout value of INT32_MAX, the maximum allowed timeout value,
 * and expect TW_OK.
 *
 * Note: This simple test serves as a good example of stub usage.
 */
TEST(unit_twFileManager_TransferFile, TransferFile_EqualMaximumTimeout) {
	char *tid = NULL;
	int ret;
	twApi_stub->twApi_InvokeService = mock_twApi_InvokeService;
	ret = twFileManager_TransferFile("sourceRepo",
	                                 "sourcePath",
	                                 "sourceFile",
	                                 "targetRepo",
	                                 "targetPath",
	                                 "targetFile",
	                                 INT32_MAX,
	                                 FALSE,
	                                 &tid);
	TEST_ASSERT_EQUAL(TW_OK, ret);
}

/**
 * Test Plan: Call twFileManager_TransferFile() with a timeout value of INT32_MAX+1, exceeding the maximum allowed
 * timeout value, and expect TW_INVALID_PARAM.
 */
TEST(unit_twFileManager_TransferFile, TransferFile_ExceedMaximumTimeout) {
	char *tid = NULL;
	int ret;
	twApi_stub->twApi_InvokeService = mock_twApi_InvokeService;
	ret = twFileManager_TransferFile("sourceRepo",
	                           "sourcePath",
	                           "sourceFile",
	                           "targetRepo",
	                           "targetPath",
	                           "targetFile",
	                           INT32_MAX + 1,
	                           FALSE,
	                           &tid);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, ret);
}