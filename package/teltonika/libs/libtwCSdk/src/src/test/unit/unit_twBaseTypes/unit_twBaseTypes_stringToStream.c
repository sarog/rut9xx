/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for stringToStream()
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

static char mock_addBytes_called;
static uint32_t mock_addBytes_count;
static twStream *unit_twBaseTypes_stringToStream_s = NULL;

TEST_GROUP(unit_twBaseTypes_stringToStream);

TEST_SETUP(unit_twBaseTypes_stringToStream) {
	eatLogs();
	mock_addBytes_called = FALSE;
	mock_addBytes_count = 0;
}

TEST_TEAR_DOWN(unit_twBaseTypes_stringToStream) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_stringToStream) {
	RUN_TEST_CASE(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_NULL);
	RUN_TEST_CASE(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_empty);
	RUN_TEST_CASE(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_small);
	RUN_TEST_CASE(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_large);
}

TEST(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_Error_Codes) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, stringToStream("data we're adding to the stream" , NULL));
}

int mock_stringToStream_addBytes(struct twStream * s, void * b, uint32_t count) {
	++mock_addBytes_called;
	mock_addBytes_count += count;
	return TW_OK;
}

TEST(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_NULL) {
	twApi_stub->twStream_AddBytes = mock_stringToStream_addBytes;
	stringToStream(NULL, NULL);
	TEST_ASSERT_EQUAL(0, mock_addBytes_called);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, stringToStream(NULL, NULL));
}

TEST(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_empty) {
	unit_twBaseTypes_stringToStream_s = twStream_Create();
	twApi_stub->twStream_AddBytes = mock_stringToStream_addBytes;
	stringToStream(NULL, unit_twBaseTypes_stringToStream_s);
	TEST_ASSERT_EQUAL(1, mock_addBytes_called);
	TEST_ASSERT_EQUAL(1, mock_addBytes_count);
	twStream_Delete(unit_twBaseTypes_stringToStream_s);
}

TEST(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_small) {
	char * teststr = "less than 128 characters.";
	unit_twBaseTypes_stringToStream_s = twStream_Create();
	twApi_stub->twStream_AddBytes = mock_stringToStream_addBytes;
	stringToStream(teststr, unit_twBaseTypes_stringToStream_s);
	TEST_ASSERT_EQUAL(2, mock_addBytes_called);
	strlen(teststr);
	TEST_ASSERT_EQUAL(strlen(teststr) + 1, mock_addBytes_count);
	twStream_Delete(unit_twBaseTypes_stringToStream_s);
}

TEST(unit_twBaseTypes_stringToStream, twBaseTypes_stringToStream_large) {
	char * teststr = "data we're adding to the stream to check that it is greater than one twenty eight bytes or not and also check that its working right or not . This will make this case to execute well";
	unit_twBaseTypes_stringToStream_s = twStream_Create();
	twApi_stub->twStream_AddBytes = mock_stringToStream_addBytes;
	stringToStream(teststr, unit_twBaseTypes_stringToStream_s);
	TEST_ASSERT_EQUAL(2, mock_addBytes_called);
	TEST_ASSERT_EQUAL(strlen(teststr) + 4, mock_addBytes_count);
	twStream_Delete(unit_twBaseTypes_stringToStream_s);
}