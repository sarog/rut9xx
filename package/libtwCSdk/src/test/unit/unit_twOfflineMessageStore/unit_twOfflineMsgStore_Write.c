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

int twOfflineMsgStore_Write(struct twMessage * msg, struct twWs * ws);

TEST_GROUP(unit_twOfflineMsgStore_Write);

TEST_SETUP(unit_twOfflineMsgStore_Write) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_Write) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_Write) {
	RUN_TEST_CASE(unit_twOfflineMsgStore_Write, twOfflineMsgStore_Write_Test_Offline_Storage_Exceeded);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Write, twOfflineMsgStore_Write_Test_Offline_Storage_Available);
}


TEST(unit_twOfflineMsgStore_Write, twOfflineMsgStore_Write_Test_Offline_Storage_Exceeded) {
	twMessage * msg = NULL;
	twWs * ws = NULL;
	uint32_t tmp;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_NOT_NULL(ws);

	/* create message */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_NOT_NULL(msg);

	/* Save state. */
	tmp = twcfg_pointer->offline_msg_queue_size;

	/* write will exceed offline storage size. */
	twcfg_pointer->offline_msg_queue_size = 0;
	TEST_ASSERT_EQUAL(TW_ERROR_OFFLINE_MSG_STORE_FULL, twOfflineMsgStore_Write(msg, ws));

	/* Restore state. */
	twcfg_pointer->offline_msg_queue_size = tmp;

	/* Clean up */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Write, twOfflineMsgStore_Write_Test_Offline_Storage_Available) {
	twMessage * msg = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_NOT_NULL(ws);

	/* create message */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_NOT_NULL(msg);

	/* write will not exceed offline storage size. */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Write(msg, ws));

	/* Clean up */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Delete());
}