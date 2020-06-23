
#include "twBaseTypes.h"
#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"

TEST_GROUP(unit_twStream_AddBytes);

TEST_SETUP(unit_twStream_AddBytes){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twStream_AddBytes){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twStream_AddBytes){
	RUN_TEST_CASE(unit_twStream_AddBytes, test_ResizingStream);
}

/**
 * Test Plan: Create an in memory stream with an initial block size of 10 bytes. Write more than 10 bytes and
 * observe the state of the stream, confirming that all new bytes were written to the end of the stream.
 */
TEST(unit_twStream_AddBytes,test_ResizingStream){

	uint16_t currentBlockSize = STREAM_BLOCK_SIZE;

	const char* firstBlockToWrite="0123456789";
	const char* secondBlockToWrite="ABCDEFGHIJ";
	const char* totalBlockData="0123456789ABCDEFGHIJ";
	twStream* stream;


	stream = twStream_Create();
	stream->maxlength = 10;
	TEST_ASSERT_NOT_NULL(stream);
	TEST_ASSERT_EQUAL(0,stream->length);

	/* First call fill the stream to max block size, verify the buffer contents */
	TEST_ASSERT_EQUAL(TW_OK,twStream_AddBytes(stream,firstBlockToWrite,10));
	TEST_ASSERT_EQUAL(10,stream->length);
	TEST_ASSERT_EQUAL(10,stream->ptr-stream->data);
	TEST_ASSERT_TRUE(strncmp(firstBlockToWrite,stream->data,10)== 0);

	/* Add another 10 bytes*/
	/* First call fill the stream to max block size, verify the buffer contents */
	TEST_ASSERT_EQUAL(TW_OK,twStream_AddBytes(stream,secondBlockToWrite,10));
	TEST_ASSERT_EQUAL(20,stream->length);
	TEST_ASSERT_EQUAL(20,stream->ptr-stream->data);
	TEST_ASSERT_TRUE(strncmp(totalBlockData,stream->data,20)== 0);

	/* Clean Up */
	twStream_Delete(stream);


}
