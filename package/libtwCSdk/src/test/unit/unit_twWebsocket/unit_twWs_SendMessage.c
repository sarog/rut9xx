//
// Created by William Reichardt on 7/17/16.
//
#include "twOSPort.h"
#include "twTasker.h"
#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "twWebsocket.h"

/* WebSocket decompression function prototype */
int twCompressBytes (char * buf, uint32_t length, twStream* s, struct twWs * ws);
int twDecompressStream (twStream** s, struct twWs * ws);


/*
		*** WS Wire format, for reference (RFD-6455 5.2) ***

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+

RSV1 is used to indicate a compressed message, per RFC-7692. If
the message is fragmented across frames, only the first frame
receives this marker bit.

*/

/*****************************************************************************/
/* WS wire protocol handling tests*/

/* Craft the header of a WS frame to feed to the state machine/parser */
static const char FINAL = TRUE;
static const char NONFINAL = FALSE;
static int GenerateWebsocketHeader (
		char final,					/* Boolean, sets FIN bit */
		char compressed,			/* Boolean, sets RSV1 bit */
		unsigned char opcode,		/* 4 bit WS opcode */
		unsigned char payloadLen,	/* 7 bit payload length */
		unsigned char* buffer) {	/* At least 2 bytes */

	TEST_ASSERT_TRUE (opcode < 16);
	buffer[0] = opcode | (final ? 0x80 : 0x00) | (compressed ? 0x40 : 0x00);
	TEST_ASSERT_TRUE (payloadLen < 126);	/* This helper would need to populate more header fields */
	buffer[1] = payloadLen;
	return 2;
}

/* Generate an arbitrary payload to use as the payload of a WS frame.
 * buffer[n] = seed + n;  for all bytes
 */
static int GeneratePayload (int count, unsigned char* buffer, size_t seed) {
	size_t u;
	for (u = 0; u < count; ++u) { buffer[u] = u + seed; }
	return count;
}

/* WS message processing mocks and helpers */

/* Iterate through returning a number of payloads, since a single call to twWs_Recv
 * repeatedly reads off the socket until it has a whole frame (header and then payload) */
#define MOCK_READ_COUNT 8
static char mockTwTlsClient_Read_Payload[MOCK_READ_COUNT][255] = { 0 };	/* A series of buffers sequentially returned */
static int mockTwTlsClient_Read_Length[MOCK_READ_COUNT] = { 0 };			/* A series of bytecounts sequentially returned */
static size_t mockTwTlsClient_Read_Index = 0;								/* Which buffer/count we're returning next */
static int mockTwTlsClient_Read_MessageProcessing (twTlsClient * t, char * buf, int len, int timeout) {
	TEST_ASSERT_TRUE (mockTwTlsClient_Read_Index < MOCK_READ_COUNT);
	if (mockTwTlsClient_Read_Length[mockTwTlsClient_Read_Index] > 0) {
		TEST_ASSERT_TRUE (len >= mockTwTlsClient_Read_Length[mockTwTlsClient_Read_Index]);	/* Mock doesn't support coming back for the rest of the buffer */
		memcpy (buf, mockTwTlsClient_Read_Payload[mockTwTlsClient_Read_Index], mockTwTlsClient_Read_Length[mockTwTlsClient_Read_Index]);
	}
	return mockTwTlsClient_Read_Length[mockTwTlsClient_Read_Index++];
}

/* Captures the payload and length passed to a binary write handler */
static char mockBinaryWriteHandlerPayload[1024] = { 0 };
static size_t mockBinaryWriteHandlerLength = 0;
static char mockBinaryWriteHandlerCompressed = FALSE;
static int binary_write_handler_MessageProcessing (struct twWs * ws, const char *at, size_t length, char compressed) {
	memset (mockBinaryWriteHandlerPayload, 0, 1024);
	TEST_ASSERT_TRUE (length <= 1024);
	memcpy (mockBinaryWriteHandlerPayload, at, length);
	mockBinaryWriteHandlerLength = length;
	mockBinaryWriteHandlerCompressed = compressed;
}

/* Captures the payload and length passed to a text write handler */
static char mockTextWriteHandlerPayload[1024] = { 0 };
static size_t mockTextWriteHandlerLength = 0;
static int text_write_handler_MessageProcessing (struct twWs * ws, const char *at, size_t length) {
	memset (mockTextWriteHandlerPayload, 0, 1024);
	TEST_ASSERT_TRUE (length <= 1024);
	memcpy (mockTextWriteHandlerPayload, at, length);
	mockTextWriteHandlerLength = length;
}

/* Captures the payload and length passed to a pinghandler */
static char mockPingHandlerPayload[125] = { 0 };
static size_t mockPingHandlerLength = 0;
static int ping_handler_MessageProcessing (struct twWs * ws, const char *at, size_t length) {
	memset (mockPingHandlerPayload, 0, 125);
	TEST_ASSERT_TRUE (length <= 125);
	memcpy (mockPingHandlerPayload, at, length);
	mockPingHandlerLength = length;
}

/* Cleans up the above structures before/after use */
static void CleanupMockRead_MessageProcessing () {
	memset (mockTwTlsClient_Read_Length, 0, sizeof (int) * MOCK_READ_COUNT);
	memset (mockTwTlsClient_Read_Payload, 0, sizeof (char) * 255 * MOCK_READ_COUNT);
	mockTwTlsClient_Read_Index = 0;

	memset (mockBinaryWriteHandlerPayload, 0, 1024);
	mockBinaryWriteHandlerLength = 0;
	mockBinaryWriteHandlerCompressed = FALSE;

	memset (mockTextWriteHandlerPayload, 0, 1024);
	mockTextWriteHandlerLength = 0;

	memset (mockPingHandlerPayload, 0, 125);
	mockPingHandlerLength = 0;
}

/* A WS that thinks it's connected, and is ready to get started reading a new frame */
static void InitializeFakeWebsocket (twWs* ws, char* frameBuffer) {
	memset (ws, 0, sizeof (*ws));
	/* Create a fake ws */
	ws->isConnected = TRUE;
	ws->recvMutex = twMutex_Create ();
	ws->frameSize = 8192;
	ws->frameBufferPtr = frameBuffer;
	ws->frameBuffer = frameBuffer;
	ws->bytesNeeded = 2;
	ws->read_state = READ_HEADER;
	ws->headerPtr = ws->ws_header;
	ws->previous_read_state = READ_BINARY_FRAME;
	ws->on_ws_binaryMessage = binary_write_handler_MessageProcessing;
	ws->on_ws_textMessage = text_write_handler_MessageProcessing;
	ws->on_ws_ping = ping_handler_MessageProcessing;
}

static void CleanupFakeWebsocket (twWs* ws) {
	twMutex_Delete (ws->recvMutex);
	ws->recvMutex = NULL;
}

/* WS HTTP Upgrade Header mocks/helpers (CSDK-1196) */
static char *mockTwTlsClient_Write_Payload = NULL;
static int mockTwTlsClient_Write_Length = 0;

static void CleanupCSDK1196Payload () {
	TW_FREE (mockTwTlsClient_Write_Payload);
	mockTwTlsClient_Write_Payload = NULL;
	mockTwTlsClient_Write_Length = 0;
}

static int mockTwTlsClient_Write_CSDK_1196 (twTlsClient *t, char *buf, int len, int timeout) {
	/* Retain a copy of the payload buffer */
	CleanupCSDK1196Payload ();
	mockTwTlsClient_Write_Payload = TW_CALLOC (1, len);
	mockTwTlsClient_Write_Length = len;
	memcpy (mockTwTlsClient_Write_Payload, buf, len);
	/* Return failure (no bytes written) */
	return 0;
}

static int mockTwTlsClient_Reconnect_CSDK_1196 (twTlsClient * t, const char * host, int16_t port) {
	return TW_OK;
}

/* Mock "compression" function that just returns a configurable amount of data. */
static size_t mock_compress_result_length = 10;

static int mock_twCompressBytes (char * buf, uint32_t length, twStream* s, struct twWs * ws) {
	size_t u = 0;
	char data = (char) 0xab;
	for (u = 0; u < mock_compress_result_length; ++u) {
		twStream_AddBytes (s, &data, sizeof(data));
	}
	return TW_OK;
}

/* Mock sendDataFrame that captures arguments */
static struct sdfArgs {
	char* pdata;
	uint16_t length;
	char isContinuation;
	char isFinal;
};

static struct sdfArgs sendDataFrame_args[6] = { 0 };
static size_t sendDataFrame_invokeCount = 0;

static int mock_sendDataFrame (twWs * ws, char * msg, uint16_t length, char isContinuation, char isFinal, char isText) {
	struct sdfArgs const newArgs = { msg, length, isContinuation, isFinal };
	sendDataFrame_args[sendDataFrame_invokeCount++] = newArgs;
	return TW_OK;
}

TEST_GROUP(unit_twWs_SendMessage);

TEST_SETUP(unit_twWs_SendMessage){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twWs_SendMessage){
	twStubs_Reset ();
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twWs_SendMessage) {
	RUN_TEST_CASE(unit_twWs_SendMessage, test_sendMessage_singleframe);
	RUN_TEST_CASE(unit_twWs_SendMessage, test_sendMessage_singlefullframe);
	RUN_TEST_CASE(unit_twWs_SendMessage, test_sendMessage_framepluspartial);
	RUN_TEST_CASE(unit_twWs_SendMessage, test_sendMessage_2fullframes);
}

/**
* Test Plan: Send a message that fits in a single WS frame
*/
TEST (unit_twWs_SendMessage, test_sendMessage_singleframe) {
	twWs ws;
	char frameBuffer[1024];
	char* msgNotUsed = "Hello";
	uint32_t lenNotUsed = 10;

	if (twStubs_Use ()) {
		/* stubs are disabled, exit test */
		return;
	}
	twApi_stub->twWs_SendDataFrame = mock_sendDataFrame;
	twApi_stub->twCompressBytes = mock_twCompressBytes;
	sendDataFrame_invokeCount = 0;

	InitializeFakeWebsocket (&ws, frameBuffer);
	ws.bSupportsPermessageDeflate = TRUE;
	/* Payload will fit in 1 frame:*/
	ws.messageChunkSize = 500;
	ws.frameSize = 500;
	mock_compress_result_length = 100;

	TEST_ASSERT_EQUAL (TW_OK, twWs_SendMessage (&ws, msgNotUsed, lenNotUsed, FALSE));
	TEST_ASSERT_EQUAL (1, sendDataFrame_invokeCount);
	TEST_ASSERT_EQUAL (mock_compress_result_length, sendDataFrame_args[0].length);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isContinuation);
	TEST_ASSERT_TRUE (sendDataFrame_args[0].isFinal);

	CleanupFakeWebsocket (&ws);
}

/**
* Test Plan: Send a message that completely fills single WS frame
*/
TEST (unit_twWs_SendMessage, test_sendMessage_singlefullframe) {
	twWs ws;
	char frameBuffer[1024];
	char* msgNotUsed = "Hello";
	uint32_t lenNotUsed = 10;

	if (twStubs_Use ()) {
		/* stubs are disabled, exit test */
		return;
	}
	twApi_stub->twWs_SendDataFrame = mock_sendDataFrame;
	twApi_stub->twCompressBytes = mock_twCompressBytes;
	sendDataFrame_invokeCount = 0;

	InitializeFakeWebsocket (&ws, frameBuffer);
	ws.bSupportsPermessageDeflate = TRUE;
	/* Payload will fit in 1 frame:*/
	ws.messageChunkSize = 500;
	ws.frameSize = 500;
	mock_compress_result_length = 500;

	TEST_ASSERT_EQUAL (TW_OK, twWs_SendMessage (&ws, msgNotUsed, lenNotUsed, FALSE));
	TEST_ASSERT_EQUAL (1, sendDataFrame_invokeCount);
	TEST_ASSERT_EQUAL (mock_compress_result_length, sendDataFrame_args[0].length);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isContinuation);
	TEST_ASSERT_TRUE (sendDataFrame_args[0].isFinal);

	CleanupFakeWebsocket (&ws);
}

/**
* Test Plan: Send a message that fills a WS frame plus a partial second frame
*/
TEST (unit_twWs_SendMessage, test_sendMessage_framepluspartial) {
	twWs ws;
	char frameBuffer[1024];
	const char* msgNotUsed = "Hello";
	const uint32_t lenNotUsed = 10;
	char* pData = NULL;

	if (twStubs_Use ()) {
		/* stubs are disabled, exit test */
		return;
	}
	twApi_stub->twWs_SendDataFrame = mock_sendDataFrame;
	twApi_stub->twCompressBytes = mock_twCompressBytes;
	sendDataFrame_invokeCount = 0;

	InitializeFakeWebsocket (&ws, frameBuffer);
	ws.bSupportsPermessageDeflate = TRUE;
	/* Payload will flow into a second continuation frame*/
	ws.messageChunkSize = 500;
	ws.frameSize = 500;
	mock_compress_result_length = 800;

	TEST_ASSERT_EQUAL (TW_OK, twWs_SendMessage (&ws, msgNotUsed, lenNotUsed, FALSE));
	TEST_ASSERT_EQUAL (2, sendDataFrame_invokeCount);

	TEST_ASSERT_EQUAL (ws.frameSize, sendDataFrame_args[0].length);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isContinuation);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isFinal);
	pData = sendDataFrame_args[0].pdata;	/* Capture to make sure subsequent frame starts in the right place */

	TEST_ASSERT_EQUAL (mock_compress_result_length - ws.frameSize, sendDataFrame_args[1].length);
	TEST_ASSERT_TRUE (sendDataFrame_args[1].isContinuation);
	TEST_ASSERT_TRUE (sendDataFrame_args[1].isFinal);
	TEST_ASSERT_EQUAL (pData + ws.frameSize, sendDataFrame_args[1].pdata);

	CleanupFakeWebsocket (&ws);
}

/**
* Test Plan: Send a message that exactly fills two WS frames
*/
TEST (unit_twWs_SendMessage, test_sendMessage_2fullframes) {
	twWs ws;
	char frameBuffer[1024];
	const char* msgNotUsed = "Hello";
	const uint32_t lenNotUsed = 10;
	char* pData = NULL;

	if (twStubs_Use ()) {
		/* stubs are disabled, exit test */
		return;
	}
	twApi_stub->twWs_SendDataFrame = mock_sendDataFrame;
	twApi_stub->twCompressBytes = mock_twCompressBytes;
	sendDataFrame_invokeCount = 0;

	InitializeFakeWebsocket (&ws, frameBuffer);
	ws.bSupportsPermessageDeflate = TRUE;
	/* Payload will flow into a second continuation frame*/
	ws.messageChunkSize = 400;
	ws.frameSize = 400;
	mock_compress_result_length = 800;

	TEST_ASSERT_EQUAL (TW_OK, twWs_SendMessage (&ws, msgNotUsed, lenNotUsed, FALSE));
	TEST_ASSERT_EQUAL (2, sendDataFrame_invokeCount);

	TEST_ASSERT_EQUAL (ws.frameSize, sendDataFrame_args[0].length);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isContinuation);
	TEST_ASSERT_FALSE (sendDataFrame_args[0].isFinal);
	pData = sendDataFrame_args[0].pdata;	/* Capture to make sure subsequent frame starts in the right place */

	TEST_ASSERT_EQUAL (mock_compress_result_length - ws.frameSize, sendDataFrame_args[1].length);
	TEST_ASSERT_TRUE (sendDataFrame_args[1].isContinuation);
	TEST_ASSERT_TRUE (sendDataFrame_args[1].isFinal);
	TEST_ASSERT_EQUAL (pData + ws.frameSize, sendDataFrame_args[1].pdata);

	CleanupFakeWebsocket (&ws);
}