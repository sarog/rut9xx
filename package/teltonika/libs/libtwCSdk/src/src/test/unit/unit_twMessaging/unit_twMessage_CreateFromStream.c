/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twMessage_CreateFromStream()
 */

#include "twApi.h"
#include "TestUtilities.h"
#include "unitTestDefs.h"
#include "unity.h"
#include "unity_fixture.h"

/* Clean up the mock API singleton */
static int mock_twApi_Delete() {
	TW_FREE(tw_api);
	tw_api = NULL;

	/* deleting stubs */
	twApi_DeleteStubs();

	return TW_OK;
}

/* Initialize an empty twApi singleton, and set the isAuthenticated flag*/
static int mock_twApi_Initialize(char * host, uint16_t port, char * resource, char * app_key, char * gatewayName, uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {
	tw_api = (twApi *) TW_CALLOC(1, sizeof(twApi));
	TEST_ASSERT_TRUE(NULL != tw_api);
	tw_api->isAuthenticated = TRUE;

	/* create stubs for the Api */
	twApi_CreateStubs();

	return TWX_SUCCESS;
}

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/* Helper struct to capture the messages to sent to the SendMessage stub */
struct _stub_sendmessage_args {
	twStream** streams;
	unsigned invocationCount;
} _stub_sm_capture;

/* Stub that captures messages sent and stores them */
int test_twWs_SendMessage(twWs * ws, char * buf, uint32_t length, char isText) {
	twStream** ppStream = NULL;
	if (!_stub_sm_capture.streams) {
		_stub_sm_capture.streams = TW_MALLOC(1000 * sizeof (twStream*));
	}
	ppStream = &_stub_sm_capture.streams[_stub_sm_capture.invocationCount];
	*ppStream = twStream_Create();
	twStream_AddBytes(*ppStream, buf, length);
	twStream_Reset(*ppStream);
	++_stub_sm_capture.invocationCount;
	return TW_OK;
}

/* WebSocket decompression function prototype */
int twDecompressStream (twStream** s, struct twWs * ws);

TEST_GROUP(unit_twMessage_CreateFromStream);

TEST_SETUP(unit_twMessage_CreateFromStream) {
	eatLogs();
	twcfgResetAll();
	mock_twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twStubs_Reset();
}

TEST_TEAR_DOWN(unit_twMessage_CreateFromStream) {
	mock_twApi_Delete();
	/* Mock API init/delete won't clean this up */
	twLogger_Delete();

	/* reset chunk size */
	twcfg_pointer->message_chunk_size = MESSAGE_CHUNK_SIZE;
}

TEST_GROUP_RUNNER(unit_twMessage_CreateFromStream) {
	RUN_TEST_CASE(unit_twMessage_CreateFromStream, test_twMultipartBody_CreateFromStream);
	RUN_TEST_CASE(unit_twMessage_CreateFromStream, test_twMultipartBody_CreateFromStrm_NoCompression);
}

/* Ensure that the chunk ID in many-chunk multipart messages is deserialized properly */
TEST(unit_twMessage_CreateFromStream, test_twMultipartBody_CreateFromStream) {
	unsigned char sourceBytes[8192];
	int err = 0;
	unsigned u = 0;
	unsigned chunkCount = 0;
	twInfoTable* it = NULL;
	twMessage* msg = NULL;
	twWs *ws = NULL;

	/* Small chunk size so we can get a message with many chunks*/
	const unsigned test_chunk_size = 50;

	/* set bytecount */
	const unsigned bytecount = sizeof(sourceBytes) / sizeof(sourceBytes[0]);

	/* use stubs to mock send message so we can capture the bytes*/
	if (twStubs_Use()) {
		/* stubs are disabled, exit test */
		return;
	}
	twcfg_pointer->message_chunk_size = test_chunk_size;
	twApi_stub->twWs_SendMessage = test_twWs_SendMessage;

	srand((unsigned int)time(NULL));
	for (u = 0; u < bytecount; ++u)	{
		sourceBytes[u] = (unsigned char) rand();
	}

	/* Generate an arbitrary, large infotable to use as the request 'params' */
	it = twInfoTable_CreateFromBlob("name", (char*)sourceBytes, bytecount, FALSE, TRUE);
	/* Create the Request message */
	msg = twMessage_CreateRequestMsg(TWX_GET);
	/* Set the body of the message */
	twRequestBody_SetEntity((twRequestBody *) (msg->body), TW_THING, "ThingName");
    twRequestBody_SetCharacteristic((twRequestBody *) (msg->body), TW_PROPERTIES, "PropName");
	twRequestBody_SetParams((twRequestBody *) (msg->body), twInfoTable_ZeroCopy(it));

	/* Initialize the ws instance with our test chunk size */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, test_chunk_size, test_chunk_size, &ws));

	/* Initialize the helper struct */
	memset(&_stub_sm_capture, 0, sizeof(_stub_sm_capture));

	/* Enable WebSocket Compression*/
	ws->bSupportsPermessageDeflate = TRUE;

	/* Send the large message, which will cause our stub to be invoked with each chunk */
	TEST_ASSERT_EQUAL(TW_OK, twMessage_Send(&msg, ws));
	twMessage_Delete(msg);
	twStream_Reset(_stub_sm_capture.streams[0]);
	u = 0;
	while (u < _stub_sm_capture.invocationCount) {
		twStream* tempStream = twStream_CreateFromCharArray (twStream_GetData (_stub_sm_capture.streams[u]), twStream_GetLength (_stub_sm_capture.streams[u]));
		twDecompressStream (&tempStream, ws);
		msg = twMessage_CreateFromStream(tempStream);
		TEST_ASSERT_NOT_NULL(msg);
		TEST_ASSERT_TRUE(msg->multipartMarker);
		TEST_ASSERT_NOT_NULL(msg->body);
		/* Chunk ID should monotonically increase*/
		TEST_ASSERT_EQUAL(u + 1, ((twMultipartBody *) (msg->body))->chunkId);
		twMessage_Delete(msg);
		twStream_Delete(_stub_sm_capture.streams[u]);
		++u;
	}
	twInfoTable_Delete(it);
	twWs_Delete(ws);
	TW_FREE(_stub_sm_capture.streams);
}

/* Ensure that the chunk ID in many-chunk multipart messages is deserialized properly */
TEST(unit_twMessage_CreateFromStream, test_twMultipartBody_CreateFromStrm_NoCompression) {
	unsigned char sourceBytes[8192];
	int err = 0;
	unsigned u = 0;
	unsigned chunkCount = 0;
	twInfoTable* it = NULL;
	twMessage* msg = NULL;
	twWs *ws = NULL;

	/* Small chunk size so we can get a message with many chunks*/
	const unsigned test_chunk_size = 50;

	/* set bytecount */
	const unsigned bytecount = sizeof(sourceBytes) / sizeof(sourceBytes[0]);


	/* use stubs to mock send message so we can capture the bytes*/
	if (twStubs_Use()) {
		/* stubs are disabled, exit test */
		return;
	}
	twcfg_pointer->message_chunk_size = test_chunk_size;
	twApi_stub->twWs_SendMessage = test_twWs_SendMessage;

	srand((unsigned int)time(NULL));
	for (u = 0; u < bytecount; ++u)	{
		sourceBytes[u] = (unsigned char) rand();
	}

	/* Generate an arbitrary, large infotable to use as the request 'params' */
	it = twInfoTable_CreateFromBlob("name", (char*)sourceBytes, bytecount, FALSE, TRUE);
	/* Create the Request message */
	msg = twMessage_CreateRequestMsg(TWX_GET);
	/* Set the body of the message */
	twRequestBody_SetEntity((twRequestBody *) (msg->body), TW_THING, "ThingName");
    twRequestBody_SetCharacteristic((twRequestBody *) (msg->body), TW_PROPERTIES, "PropName");
	twRequestBody_SetParams((twRequestBody *) (msg->body), twInfoTable_ZeroCopy(it));

	/* Initialize the ws instance with our test chunk size */
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, test_chunk_size, test_chunk_size, &ws));

	/* Initialize the helper struct */
	memset(&_stub_sm_capture, 0, sizeof(_stub_sm_capture));

	/* Disable WebSocket Compression*/
	ws->bSupportsPermessageDeflate = FALSE;

	/* Send the large message, which will cause our stub to be invoked with each chunk */
	TEST_ASSERT_EQUAL(TW_OK, twMessage_Send(&msg, ws));
	twMessage_Delete(msg);
	twStream_Reset(_stub_sm_capture.streams[0]);
	u = 0;
	while (u < _stub_sm_capture.invocationCount) {
		twStream* tempStream = twStream_CreateFromCharArray (twStream_GetData (_stub_sm_capture.streams[u]), twStream_GetLength (_stub_sm_capture.streams[u]));
		msg = twMessage_CreateFromStream(tempStream);
		TEST_ASSERT_NOT_NULL(msg);
		TEST_ASSERT_TRUE(msg->multipartMarker);
		TEST_ASSERT_NOT_NULL(msg->body);
		/* Chunk ID should monotonically increase*/
		TEST_ASSERT_EQUAL(u + 1, ((twMultipartBody *) (msg->body))->chunkId);
		twMessage_Delete(msg);
		twStream_Delete(_stub_sm_capture.streams[u]);
		++u;
	}
	twInfoTable_Delete(it);

	twWs_Delete(ws);
	TW_FREE(_stub_sm_capture.streams);
}