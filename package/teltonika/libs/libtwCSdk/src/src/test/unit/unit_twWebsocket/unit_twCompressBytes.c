#include "twOSPort.h"
#include "twTasker.h"
#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "twWebsocket.h"

/* WebSocket decompression function prototype */
int twCompressBytes(char * buf, uint32_t length, twStream* s, struct twWs * ws);
int twDecompressStream(twStream** s, struct twWs * ws);

TEST_GROUP(unit_twCompressBytes);

TEST_SETUP(unit_twCompressBytes){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twCompressBytes){
	twStubs_Reset ();
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twCompressBytes) {
	RUN_TEST_CASE(unit_twCompressBytes, test_WebSocketCompression);
}

/**
 * Test Plan: Compress and decompress a stream and verify the contents are the same.
 */
TEST (unit_twCompressBytes, test_WebSocketCompression) {
	/* Test data for compression */
	char testString[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv0123456789";
	twStream* testStream = twStream_Create ();
	twStream* temp = NULL;
	twStream* testStream2 = twStream_Create ();

	/* We need a WebSocket since it owns the compression and decompression contexts */
	twWs* ws = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twWs_Create(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, &ws));

	TEST_ASSERT_TRUE (testStream != NULL);
	twStream_AddBytes (testStream, (void*)(&testString), sizeof (testString));

	/* Perform the compression */
	temp = twStream_Create ();
	TEST_ASSERT_EQUAL (TW_OK, twCompressBytes (twStream_GetData(testStream), twStream_GetLength(testStream), temp, ws));
	twStream_Delete (testStream);
	testStream = temp;
	temp = NULL;

	/* Verify the contents have changed */
	TEST_ASSERT_NOT_EQUAL (sizeof (testString), twStream_GetLength (testStream));

	/* Create a second stream with the same data */
	TEST_ASSERT_TRUE (testStream2 != NULL);
	twStream_AddBytes (testStream2, (void*)(&testString), sizeof (testString));

	/* Compress it */
	temp = twStream_Create ();
	TEST_ASSERT_EQUAL (TW_OK, twCompressBytes (twStream_GetData(testStream2), twStream_GetLength(testStream2), temp, ws));
	twStream_Delete (testStream2);
	testStream2 = temp;
	temp = NULL;

	/* With context takeover, the second stream should be compressed smaller than the first */
	TEST_ASSERT_TRUE (twStream_GetLength (testStream2) < twStream_GetLength (testStream));

	/* Perform the decompression */
	TEST_ASSERT_EQUAL (TW_OK, twDecompressStream (&testStream, ws));

	/* Verify the contents are the same as before compression */
	TEST_ASSERT_EQUAL_STRING (testString, twStream_GetData (testStream));

	twWs_Delete(ws);
	twStream_Delete (testStream);
	twStream_Delete (testStream2);
}