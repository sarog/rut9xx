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

static twStream *unit_twBaseTypes_twStream_Create_s = NULL;

TEST_GROUP(unit_twBaseTypes_twStream_Create);

TEST_SETUP(unit_twBaseTypes_twStream_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_twStream_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twStream_Create) {
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_Create);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_CreateFromFile);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_CreateFromCharArrayZeroCopy);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetData);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetLength);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetIndex);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_AddBytes_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_AddBytes);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetBytes);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_Reset_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_Reset);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_Create) {
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	TEST_ASSERT_NOT_NULL(unit_twBaseTypes_twStream_Create_s);
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_CreateFromFile) {
	twStream *unit_twBaseTypes_twStream_Create_s1 = NULL;
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	TEST_ASSERT_NULL(twStream_CreateFromFile(NULL));
	unit_twBaseTypes_twStream_Create_s1 = twStream_CreateFromFile("fileName");
	TEST_ASSERT_EQUAL(0, unit_twBaseTypes_twStream_Create_s1==unit_twBaseTypes_twStream_Create_s);
	TEST_ASSERT_NOT_NULL(twStream_CreateFromFile("fileName"));
	TEST_ASSERT_NULL(twStream_CreateFromFile("fie_01./,'][-="));
	twStream_Delete(unit_twBaseTypes_twStream_Create_s1);
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_check_twStream_CreateFromCharArrayZeroCopy) {
	unit_twBaseTypes_twStream_Create_s = twStream_CreateFromCharArrayZeroCopy("foo", 4);
	TEST_ASSERT_NULL(twStream_CreateFromCharArrayZeroCopy(NULL, 0));
	TEST_ASSERT_NOT_NULL(unit_twBaseTypes_twStream_Create_s);
	TEST_ASSERT_EQUAL(4, twStream_GetLength(unit_twBaseTypes_twStream_Create_s));
	TEST_ASSERT_EQUAL(0, strcmp("foo", twStream_GetData(unit_twBaseTypes_twStream_Create_s)));
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetData) {
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "foo", 4);
	TEST_ASSERT_NULL(twStream_GetData(NULL));
	TEST_ASSERT_EQUAL(0, strcmp("foo", unit_twBaseTypes_twStream_Create_s->data));
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetLength) {
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "foo", 4);
	TEST_ASSERT_EQUAL(-1, twStream_GetLength(NULL));
	TEST_ASSERT_EQUAL(4, unit_twBaseTypes_twStream_Create_s->length);
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetIndex) {
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "foo", 4);
	TEST_ASSERT_EQUAL(0, twStream_GetIndex(NULL));
	TEST_ASSERT_TRUE(twStream_GetIndex(unit_twBaseTypes_twStream_Create_s)>=0);
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_AddBytes_Error_Codes) {
	twStream *unit_twBaseTypes_twStream_Create_sFile = twStream_CreateFromFile("file1");
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twStream_AddBytes(NULL, "foo", 4));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twStream_AddBytes(unit_twBaseTypes_twStream_Create_sFile, NULL, 4));
	TEST_ASSERT_EQUAL(TW_ERROR_WRITING_FILE, twStream_AddBytes(unit_twBaseTypes_twStream_Create_sFile, "foo", 0));
	twStream_Delete(unit_twBaseTypes_twStream_Create_sFile);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_AddBytes) {
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	TEST_ASSERT_EQUAL(TW_OK, twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "foo", 1));
	unit_twBaseTypes_twStream_Create_s->file = TW_FOPEN("fileName", "a+b");
	TEST_ASSERT_EQUAL(TW_OK, twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "foo", 1));
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_GetBytes) {
	int length = 4;
	char buf[10] = "";
	char *add = "abcdefghi";
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, add, length);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twStream_GetBytes(NULL, "foo", 0));
	twStream_Reset(unit_twBaseTypes_twStream_Create_s);
	twStream_GetBytes(unit_twBaseTypes_twStream_Create_s, buf, length);
	TEST_ASSERT_EQUAL(length, strlen(buf));
	TEST_ASSERT_EQUAL(0, strcmp(unit_twBaseTypes_twStream_Create_s->data, buf));
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_Reset_Error_Codes) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twStream_Reset(NULL));
}

TEST(unit_twBaseTypes_twStream_Create, test_twBaseTypes_twStream_Reset) {
	char buf[10] = "";
	unit_twBaseTypes_twStream_Create_s = twStream_Create();
	twStream_AddBytes(unit_twBaseTypes_twStream_Create_s, "abcdefg", 6);
	TEST_ASSERT_NULL(twStream_GetBytes(unit_twBaseTypes_twStream_Create_s, buf, 4));
	TEST_ASSERT_EQUAL(TW_OK, twStream_Reset(unit_twBaseTypes_twStream_Create_s));
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twStream_Create_s->data, unit_twBaseTypes_twStream_Create_s->ptr);
	twStream_Delete(unit_twBaseTypes_twStream_Create_s);
}