/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

TEST_GROUP(unit_twOfflineMsgStore_SetDir);

TEST_SETUP(unit_twOfflineMsgStore_SetDir) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_SetDir) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_SetDir) {
	RUN_TEST_CASE(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Null_Path);
	RUN_TEST_CASE(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Bad_Path);
	RUN_TEST_CASE(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Good_Path);
	RUN_TEST_CASE(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Not_Empty);
}

TEST(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Null_Path) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_SetDir(NULL));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Bad_Path) {
	int ret;
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	ret = twOfflineMsgStore_SetDir(OFFLINE_MSG_STORE_LOCATION_BAD);
	TEST_ASSERT_TRUE(TW_INVALID_MSG_STORE_DIR==ret||TW_ERROR_WRITING_FILE==ret);
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Good_Path) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_SetDir(OFFLINE_MSG_STORE_LOCATION_DIFFERENT));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_SetDir, twOfflineMsgStore_SetDir_Test_Not_Empty) {

	twMessage * msg = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create message */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_HandleRequest(&msg, ws, OFFLINE_MSG_STORE_WRITE));

	/* cleanup msg store singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* init */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* attempt to switch dirs (should fail because dir is not empty */
	TEST_ASSERT_EQUAL(TW_MSG_STORE_FILE_NOT_EMPTY, twOfflineMsgStore_SetDir(OFFLINE_MSG_STORE_LOCATION_DIFFERENT));

	/* cleanup again */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);

}