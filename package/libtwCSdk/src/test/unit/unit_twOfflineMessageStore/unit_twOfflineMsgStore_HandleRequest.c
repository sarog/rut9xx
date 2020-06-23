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

/* stubs and fake functions */
int fake_twApi_Initialize(char * host, uint16_t port, char * resource, char * app_key, char * gatewayName, uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {
	/* Allocate space for the structure */
	tw_api = (twApi *)TW_CALLOC(sizeof(twApi), 1);
	TEST_ASSERT_TRUE(NULL != tw_api);
	twApi_CreateStubs();

	return TWX_SUCCESS;
}

int fake_twApi_Delete() {
	TW_FREE(tw_api);
	tw_api = NULL;
	return TWX_SUCCESS;
}


TEST_GROUP(unit_twOfflineMsgStore_HandleRequest);

TEST_SETUP(unit_twOfflineMsgStore_HandleRequest) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_HandleRequest) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_HandleRequest) {
	/* handle request - write*/
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Success);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Null_Msg);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Null_Ws);

	/* handle request - flush*/
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Success);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Null_Msg);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Null_Ws);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Empty_Store);
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Empty_Store_UnAuth);

	/* handle request - mixed */
	RUN_TEST_CASE(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Mixed_Bad_Request);
}


TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Success) {
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

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Null_Msg) {
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_HandleRequest(NULL, ws, OFFLINE_MSG_STORE_WRITE));

	/* cleanup msg store singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (ws) twWs_Delete(ws);
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Write_Null_Ws) {
	twMessage * msg = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create message */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_HandleRequest(&msg, NULL, OFFLINE_MSG_STORE_WRITE));

	/* cleanup msg store singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Success) {
	twMessage * msg = NULL;
	twMessage * msg_2 = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* init fake api */
	fake_twApi_Initialize(NULL, 0, NULL, NULL, NULL, 0, 0, 0);
	tw_api->isAuthenticated = FALSE;

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create messages */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg);

	/* create messages */
	msg_2 = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg_2);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_HandleRequest(&msg, ws, OFFLINE_MSG_STORE_WRITE));

	/* set fake auth to TRUE */
	tw_api->isAuthenticated = TRUE;

	/* attempt to flush */
	/* allowing websocket not connected since the message will never send in the unit test */
	TEST_ASSERT_EQUAL(TW_WEBSOCKET_NOT_CONNECTED, twOfflineMsgStore_HandleRequest(&msg_2, ws, OFFLINE_MSG_STORE_FLUSH));

	/* delete singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (msg_2) twMessage_Delete(msg_2);
	if (ws) twWs_Delete(ws);
	if (tw_api) fake_twApi_Delete();
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Null_Msg) {
	twMessage * msg = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create messages */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_HandleRequest(&msg, ws, OFFLINE_MSG_STORE_WRITE));

	/* attempt to flush */
	/* allowing websocket not connected since the message will never send in the unit test */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_HandleRequest(NULL, ws, OFFLINE_MSG_STORE_FLUSH));

	/* delete singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Null_Ws) {
	twMessage * msg = NULL;
	twMessage * msg_2 = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create messages */
	msg = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg);

	/* create messages */
	msg_2 = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg_2);

	/* write the message */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_HandleRequest(&msg, ws, OFFLINE_MSG_STORE_WRITE));

	/* attempt to flush */
	/* allowing websocket not connected since the message will never send in the unit test */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_HandleRequest(&msg_2, NULL, OFFLINE_MSG_STORE_FLUSH));

	/* delete singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (msg_2) twMessage_Delete(msg_2);
	if (ws) twWs_Delete(ws);
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Empty_Store) {
	twMessage * msg_2 = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* init fake api */
	fake_twApi_Initialize(NULL, 0, NULL, NULL, NULL, 0, 0, 0);
	tw_api->isAuthenticated = FALSE;

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create messages */
	msg_2 = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg_2);

	/* set fake auth to TRUE */
	tw_api->isAuthenticated = TRUE;

	/* attempt to flush */
	/* allowing websocket not connected since the message will never send in the unit test */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_HandleRequest(&msg_2, ws, OFFLINE_MSG_STORE_FLUSH));

	/* delete singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	twMessage_Delete(msg_2);
	twWs_Delete(ws);
	if (tw_api) fake_twApi_Delete();
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Flush_Empty_Store_UnAuth) {
	twMessage * msg_2 = NULL;
	twWs * ws = NULL;

	/* init offline message store */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* init fake api */
	fake_twApi_Initialize(NULL, 0, NULL, NULL, NULL, 0, 0, 0);
	tw_api->isAuthenticated = FALSE;

	/* create websocket */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));
	TEST_ASSERT_TRUE(NULL != ws);

	/* create messages */
	msg_2 = twMessage_CreateResponseMsg(TWX_SUCCESS, OFFLINE_MSG_TEST_REQ_ID, OFFLINE_MSG_TEST_SESSION_ID, OFFLINE_MSG_TEST_ENDPOINT_ID);
	TEST_ASSERT_TRUE(NULL != msg_2);

	/* attempt to flush */
	/* allowing websocket not connected since the message will never send in the unit test */
	TEST_ASSERT_EQUAL(TW_WEBSOCKET_NOT_CONNECTED, twOfflineMsgStore_HandleRequest(&msg_2, ws, OFFLINE_MSG_STORE_FLUSH));

	/* delete singleton */
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	twMessage_Delete(msg_2);
	twWs_Delete(ws);
	if (tw_api) fake_twApi_Delete();
}

TEST(unit_twOfflineMsgStore_HandleRequest, twOfflineMsgStore_HandleRequest_Test_Mixed_Bad_Request) {
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
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_HandleRequest(&msg, ws, -1));

	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());

	/* cleanup memory */
	if (msg) twMessage_Delete(msg);
	if (ws) twWs_Delete(ws);
}