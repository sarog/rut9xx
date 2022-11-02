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

static twPrimitive *unit_twBaseTypes_twPrimitive_ToStream_p = NULL;
static twStream *unit_twBaseTypes_twPrimitive_ToStream_s = NULL;

TEST_GROUP(unit_twBaseTypes_twPrimitive_ToStream);

TEST_SETUP(unit_twBaseTypes_twPrimitive_ToStream) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_ToStream) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_ToStream) {
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToStream, test_twBaseTypes_twPrimitive_ToStream_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToStream, test_twBaseTypes_twPrimitive_ToStream);
}

TEST(unit_twBaseTypes_twPrimitive_ToStream, test_twBaseTypes_twPrimitive_ToStream_Error_Codes) {
	unit_twBaseTypes_twPrimitive_ToStream_p = twPrimitive_Create();
	unit_twBaseTypes_twPrimitive_ToStream_s = twStream_Create();
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twPrimitive_ToStream(NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twPrimitive_ToStream(NULL, unit_twBaseTypes_twPrimitive_ToStream_s));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, NULL));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_UNKNOWN_TYPE;
	TEST_ASSERT_EQUAL(TW_INVALID_BASE_TYPE, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_ToStream_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_ToStream_s);
}

TEST(unit_twBaseTypes_twPrimitive_ToStream, test_twBaseTypes_twPrimitive_ToStream) {
	unit_twBaseTypes_twPrimitive_ToStream_p = twPrimitive_Create();
	unit_twBaseTypes_twPrimitive_ToStream_s = twStream_Create();
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_STRING;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_BLOB;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_VARIANT;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_NUMBER;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_INTEGER;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_BOOLEAN;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_DATETIME;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_TIMESPAN;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_LOCATION;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));
	unit_twBaseTypes_twPrimitive_ToStream_p->typeFamily = TW_INFOTABLE;
	TEST_ASSERT_EQUAL(TW_OK, twPrimitive_ToStream(unit_twBaseTypes_twPrimitive_ToStream_p, unit_twBaseTypes_twPrimitive_ToStream_s));

	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_ToStream_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_ToStream_s);
}
