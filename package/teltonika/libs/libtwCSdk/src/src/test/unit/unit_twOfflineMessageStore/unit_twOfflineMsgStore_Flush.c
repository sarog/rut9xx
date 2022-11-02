/*
 * Copyright 2018, PTC, Inc.
 * All rights reserved.
 */
#include "TestUtilities.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

/* Test variables. */

static twWs * ws = NULL;
static twMessage * msg = NULL;
static uint32_t first_requestId;


/* Forward declare interfaces. */

int twOfflineMsgStore_Write(struct twMessage * msg, struct twWs * ws);
int twOfflineMsgStore_Flush(struct twMessage * msg, struct twWs * ws);

/* Helper functions. */

/*
 * Create a request body that is small enough to fit into a non-multipart
 * request.
 */
twRequestBody * build_simple_request() {
    char const data[] = "garbage in";
    twRequestBody * request = twRequestBody_Create();
    twDataShapeEntry * entry = twDataShapeEntry_Create(
        "entry",
        "description",
        TW_STRING
    );
    twDataShape * shape = twDataShape_Create(entry);
    twInfoTable * params = twInfoTable_Create(shape);
    twPrimitive * row_entry = twPrimitive_CreateFromVariable(
        data,
        TW_STRING,
        TRUE,
        sizeof(data)
    );
    twInfoTableRow * row = twInfoTableRow_Create(row_entry);
    twInfoTable_AddRow(params, row);

    twRequestBody_SetEntity(
        request,
        TW_THING,
        "unit_twOfflineMsgStore_FlushTests_Thing"
    );
    twRequestBody_SetParams(request, params);
    return request;
}

/*
 * Create a request body large enough to be broken into a multipart request.
 */
twRequestBody * build_multipart_request() {
    char const data[] = "garbage in";
    twRequestBody * request = twRequestBody_Create();
    twDataShapeEntry * entry = twDataShapeEntry_Create(
        "entry",
        "description",
        TW_STRING
    );
    twDataShape * shape = twDataShape_Create(entry);
    twInfoTable * params = twInfoTable_Create(shape);

    /*
     * Add enough data to ensure a multi-part message in the offline message
     * store.
     */
    while (params->length <= ws->messageChunkSize) {
        twPrimitive * row_entry = twPrimitive_CreateFromVariable(
            data,
            TW_STRING,
            TRUE,
            sizeof(data)
        );
        twInfoTableRow * row = twInfoTableRow_Create(row_entry);
        twInfoTable_AddRow(params, row);
    }

    twRequestBody_SetEntity(
        request,
        TW_THING,
        "unit_twOfflineMsgStore_FlushTests_Thing"
    );
    twRequestBody_SetParams(request, params);
    return request;
}

/* Tests. */

TEST_GROUP(unit_twOfflineMsgStore_Flush);

TEST_SETUP(unit_twOfflineMsgStore_Flush) {
    int res;

    eatLogs();

    /* Reset request ID. */
    first_requestId = 0;

    /* Delete any previous message store. */
    res = twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_FILE);
    TEST_ASSERT_TRUE(TW_OK == res || 2 == res || 3 == res);
    res = twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE);
    TEST_ASSERT_TRUE(TW_OK == res || 2 == res || 3 == res);

    /* Build test environment. */
    TEST_ASSERT_EQUAL(
        TW_OK,
        twOfflineMsgStore_Initialize(
            TRUE,
            OFFLINE_MSG_STORE_LOCATION,
            OFFLINE_MSG_STORE_SIZE,
            TRUE
        )
    );
    TEST_ASSERT_EQUAL(
        TW_OK,
        twWs_Create(
            TW_HOST,
            TW_PORT,
            TW_URI,
            TW_APP_KEY,
            NULL,
            MESSAGE_CHUNK_SIZE,
            MESSAGE_CHUNK_SIZE,
            &ws
        )
    );
    TEST_ASSERT_NOT_NULL(ws);
    ws->bDisableCompression = TRUE;

    tw_api = TW_CALLOC(sizeof(twApi), 1);
    twApi_CreateStubs();
    tw_api->isAuthenticated = TRUE;
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_Flush) {
    twApi_DeleteStubs();

    if (tw_api) {
        TW_FREE(tw_api);
        tw_api = NULL;
    }

    if (ws) {
        twWs_Delete(ws);
        ws = NULL;
    }

    if (msg) {
        twMessage_Delete(msg);
        msg = NULL;
    }
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_Flush) {
    RUN_TEST_CASE(unit_twOfflineMsgStore_Flush, test_Multipart_RequestID);
}

int STUB_twWs_SendMessage_Offline_MsgStore_Flush(
    twWs * ws,
    char * buf,
    uint32_t length,
    char isText
) {
    uint32_t requestId = * (uint32_t *) (buf + 2);
    char multipart_msg = *(buf + 14);
    uint16_t current_chunk;

    memcpy(&current_chunk, (buf + 15), sizeof(current_chunk));
    swap2bytes((char *) &current_chunk);

    if (0 == first_requestId) {
        /* Save first buffer's request ID. */
        first_requestId = requestId;
    } else if (!multipart_msg || 1 == current_chunk) {
        /*
         * Not a multipart message, or new multipart message. Request ID should
         * be different. Need to save new request ID for following chunks.
         */
        TEST_ASSERT_NOT_EQUAL(first_requestId, requestId);
        first_requestId = requestId;
    } else {
        /*
         * All subsequent buffer's request ID should match the first buffer's
         * request ID.
         */
        TEST_ASSERT_EQUAL(first_requestId, requestId);
    }

    /*
     * Sanity check to make sure request ID is stored in buffer correctly.
     * Request IDs are non-zero.
     */
    TEST_ASSERT_NOT_EQUAL(0, first_requestId);

    return TW_OK;
}

/*
 * Place single and multipart messages in the offline message store, then flush
 * the store. Verify that the request ID is the same for all the multipart
 * message chunks, and different for the unique messages.
 *
 * See CSDK-1171.
 */
TEST(unit_twOfflineMsgStore_Flush, test_Multipart_RequestID) {
    twRequestBody * body;
    twApi_stub->twWs_SendMessage = STUB_twWs_SendMessage_Offline_MsgStore_Flush;

    /* Non-multipart message. */
    body = build_simple_request();
    msg = twMessage_CreateRequestMsg(TWX_POST);

    TEST_ASSERT_NOT_NULL(msg);

    TEST_ASSERT_EQUAL(TW_OK, twMessage_SetBody(msg, body));
    TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Write(msg, ws));

    /* multipart message. */
    body = build_multipart_request();
    msg = twMessage_CreateRequestMsg(TWX_POST);

    TEST_ASSERT_NOT_NULL(msg);

    TEST_ASSERT_EQUAL(TW_OK, twMessage_SetBody(msg, body));
    TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Write(msg, ws));

    twMessage_Delete(msg);
    msg = NULL;

    /* Non-multipart message. */
    body = build_simple_request();
    msg = twMessage_CreateRequestMsg(TWX_POST);

    TEST_ASSERT_NOT_NULL(msg);

    TEST_ASSERT_EQUAL(TW_OK, twMessage_SetBody(msg, body));
    TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Write(msg, ws));

    twMessage_Delete(msg);
    msg = NULL;

    /*
     * twOfflineMsgStore_Flush() will call
     * STUB_twWs_SendMessage_Offline_MsgStore_Flush(), where we verify the
     * request ID is being set properly for all messages.
     */
    TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Flush(NULL, ws));
}