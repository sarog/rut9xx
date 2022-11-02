/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for streamToString()
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

static twStream *unit_twBaseTypes_streamToString_s = NULL;

TEST_GROUP(unit_twBaseTypes_streamToString);

TEST_SETUP(unit_twBaseTypes_streamToString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_streamToString) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_streamToString) {
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString);
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString1);
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString2);
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Small);
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Large);
	RUN_TEST_CASE(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Huge);
}

TEST(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString) {
	char * str = NULL;
	TEST_ASSERT_EQUAL(0, streamToString(NULL));
}

TEST(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString1) {
	char * str = NULL;
	TEST_IGNORE_MESSAGE ("Test will fail until CSDK-1103  is complete");
	unit_twBaseTypes_streamToString_s = twStream_Create();
	stringToStream(NULL, unit_twBaseTypes_streamToString_s);
	str = streamToString(unit_twBaseTypes_streamToString_s);
	TEST_ASSERT_EQUAL_STRING("", str);
	twStream_Delete(unit_twBaseTypes_streamToString_s);
	TW_FREE(str);
}

TEST(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_NoString2) {
	char * str = NULL;
	unit_twBaseTypes_streamToString_s = twStream_Create();
	stringToStream("\0", unit_twBaseTypes_streamToString_s);
	twStream_Reset(unit_twBaseTypes_streamToString_s);
	str = streamToString(unit_twBaseTypes_streamToString_s);
	TEST_ASSERT_NOT_NULL(str);
	TEST_ASSERT_EQUAL(0, strlen(str));
	twStream_Delete(unit_twBaseTypes_streamToString_s);
	TW_FREE(str);
}

TEST(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Small) {
	char * input = "data we're adding to the stream. i have to work a lot on that";
	char * str = NULL;
	TEST_ASSERT_EQUAL(0, streamToString(NULL));
	unit_twBaseTypes_streamToString_s = twStream_Create();
	stringToStream(input, unit_twBaseTypes_streamToString_s);
	twStream_Reset(unit_twBaseTypes_streamToString_s);
	str = streamToString(unit_twBaseTypes_streamToString_s);
	TEST_ASSERT_EQUAL(0, strcmp(input, str));
	twStream_Delete(unit_twBaseTypes_streamToString_s);
	TW_FREE(str);
}

TEST(unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Large) {
	char * input = "data we're adding to the stream to check that it is greater than one twenty eight bytes or not and also check that its working right or not . This will make this case to execute well";
	char * str = NULL;
	TEST_ASSERT_EQUAL(0, streamToString(NULL));

	unit_twBaseTypes_streamToString_s = twStream_Create();
	stringToStream(input, unit_twBaseTypes_streamToString_s);
	twStream_Reset(unit_twBaseTypes_streamToString_s);
	str = streamToString(unit_twBaseTypes_streamToString_s);
	TEST_ASSERT_EQUAL(0, strcmp(input, str));
	twStream_Delete(unit_twBaseTypes_streamToString_s);
	TW_FREE(str);
}

TEST (unit_twBaseTypes_streamToString, twBaseTypes_streamToString_Huge) {
	const size_t huge_string_length = 2 * twcfg_pointer->max_string_prop_length;
	char * input = TW_CALLOC(huge_string_length + 1, 1);
	char * str = NULL;

	memset (input, 'A', huge_string_length);
	/* Case1: bigger than the default max_string_prop_length should cause truncation */
	{
		unit_twBaseTypes_streamToString_s = twStream_Create ();
		stringToStream (input, unit_twBaseTypes_streamToString_s);
		twStream_Reset (unit_twBaseTypes_streamToString_s);
		str = streamToString (unit_twBaseTypes_streamToString_s);
		TEST_ASSERT_EQUAL (twcfg_pointer->max_string_prop_length, strlen (str));
		TW_FREE (str);
		str = NULL;
		twStream_Delete (unit_twBaseTypes_streamToString_s);
	}
	/* Case2: A big enough max_string_prop_length allows full serialization */
	{
		twcfg_pointer->max_string_prop_length = 3 * twcfg_pointer->max_string_prop_length;
		unit_twBaseTypes_streamToString_s = twStream_Create ();
		stringToStream (input, unit_twBaseTypes_streamToString_s);
		twStream_Reset (unit_twBaseTypes_streamToString_s);
		str = streamToString (unit_twBaseTypes_streamToString_s);
		TEST_ASSERT_EQUAL_STRING (input, str);
		TW_FREE (str);
		twStream_Delete (unit_twBaseTypes_streamToString_s);
	}
	TW_FREE (input);
}