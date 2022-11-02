/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for getPreventIncomingMsgListDump()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twMessaging_getPreventIncomingMsgListDump);

TEST_SETUP(unit_twMessaging_getPreventIncomingMsgListDump) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMessaging_getPreventIncomingMsgListDump) {
}

TEST_GROUP_RUNNER(unit_twMessaging_getPreventIncomingMsgListDump) {
	RUN_TEST_CASE(unit_twMessaging_getPreventIncomingMsgListDump, dumpIncomingMsgListCallback);
}

static int requestId = 0;

twList * getIncomingMsgList();
char getPreventIncomingMsgListDump();

int msgHandlerOnBinaryMessage (struct twWs * ws, const char *at, size_t length, char bCompressed);
void resetIncomingMsgList();

twMessage * stub_twMessage_CreateFromStream(twStream * s) {
    twMessage * msg;
    msg = (twMessage *)TW_CALLOC(sizeof(twMessage), 1);
    if (!msg) { TW_LOG(TW_ERROR, "twMessage_CreateFromStream: Error allocating msg storage"); return 0; }
    
    requestId++;
    msg->requestId = requestId;
    msg->endpointId = 0;
    msg->sessionId = 0;
    msg->length = MSG_HEADER_SIZE;
    if (msg->requestId % 2) {
        msg->type = TW_REQUEST;
        msg->body = NULL;
        msg->length += 0;
        msg->code = TWX_POST;
    } else {
        msg->type = TW_RESPONSE;
        msg->body = NULL;
        msg->length += 0;
        msg->code = TWX_SUCCESS;
    }
    return msg;
}

twStream * stub_twStream_CreateFromCharArray(const char * data, uint32_t length) {
    return NULL;
}

TEST(unit_twMessaging_getPreventIncomingMsgListDump, dumpIncomingMsgListCallback) {
    int err = 0;
	int i = 0;
    ListEntry *entryToRemove = NULL;
    
    /* Set trace/verbose to view incomingMsgList dump */
    twLogger_SetLevel(TW_TRACE);
    twLogger_SetIsVerbose(TRUE);
    
    TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
                                              MESSAGE_CHUNK_SIZE, TRUE));
    
    err = twStubs_Use();
    if (err) {
        /* stubs are disabled, exit test */
        return;
    }
    
    twApi_stub->twStream_CreateFromCharArray = stub_twStream_CreateFromCharArray;
    twApi_stub->twMessage_CreateFromStream = stub_twMessage_CreateFromStream;
    
    twApi_stub->twWs_Connect = stub_twWs_Connect_MockSuccess;
    twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
    TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
    TEST_ASSERT_TRUE(twApi_isConnected());
    
    
    /* Fill incomingMsgList to capacity */
    i = 0;
    while (i < MAX_MESSAGES) {
        msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE);
        i++;
    }
    
    TEST_ASSERT_FALSE(getPreventIncomingMsgListDump());
    msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE); /* Add one more message to cause dump of incomingMsgList */
    TEST_ASSERT_TRUE(getPreventIncomingMsgListDump()); /* incomingMsgList won't get dumped until count is below TW_DUMP_MSGLIST_THRESHOLD (75%) */
    
    /* Remove entry from incomingMsgList */
    entryToRemove = twList_Next(getIncomingMsgList(), NULL);
    twList_Remove(getIncomingMsgList(), entryToRemove, TRUE);
    msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE);
    TEST_ASSERT_TRUE(getPreventIncomingMsgListDump());
    
    /* Clear incomingMsgList */
    resetIncomingMsgList();
    msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE);
    twMessageHandler_msgHandlerTask(twGetSystemTime(TRUE), NULL); /* preventIncomingMsgListDump resets */
    
    /* Refill and check if incomingMsgList gets dumped */
    i = 0;
    while (i < MAX_MESSAGES) {
        msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE);
        i++;
    }
    
    TEST_ASSERT_FALSE(getPreventIncomingMsgListDump());
    msgHandlerOnBinaryMessage(NULL, NULL, 0, FALSE);
    TEST_ASSERT_TRUE(getPreventIncomingMsgListDump());

    twApi_Delete();
}