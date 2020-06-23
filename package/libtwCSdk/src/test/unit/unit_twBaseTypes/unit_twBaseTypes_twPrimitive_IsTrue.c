/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twPrimitive_IsTrue()
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

static twPrimitive *unit_twBaseTypes_twPrimitive_IsTrue_p = NULL;

TEST_GROUP(unit_twBaseTypes_twPrimitive_IsTrue);

TEST_SETUP(unit_twBaseTypes_twPrimitive_IsTrue) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_IsTrue) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_IsTrue) {
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_IsTrue, test_twBaseTypes_twPrimitive_IsTrue_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_IsTrue, test_twBaseTypes_twPrimitive_IsTrue);
}

TEST(unit_twBaseTypes_twPrimitive_IsTrue, test_twBaseTypes_twPrimitive_IsTrue_Error_Codes) {
	unit_twBaseTypes_twPrimitive_IsTrue_p = twPrimitive_Create();
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(NULL));
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	/*  checking data types   */
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily == TW_STRING;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily == TW_BLOB;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_VARIANT;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_NUMBER;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_INTEGER;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_BOOLEAN;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_DATETIME;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_TIMESPAN;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_LOCATION;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_INFOTABLE;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_UNKNOWN_TYPE;
	TEST_ASSERT_FALSE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_IsTrue_p);
}

TEST(unit_twBaseTypes_twPrimitive_IsTrue, test_twBaseTypes_twPrimitive_IsTrue) {
	unit_twBaseTypes_twPrimitive_IsTrue_p = twPrimitive_Create();

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_STRING;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.bytes.len = 1;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_BLOB;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.bytes.len = 1;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_NUMBER;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.number = 1;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_INTEGER;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.number = 1;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_DATETIME;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.datetime = 12345;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));

	unit_twBaseTypes_twPrimitive_IsTrue_p->typeFamily = TW_LOCATION;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.location.latitude = 1;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.location.longitude = 1;
	unit_twBaseTypes_twPrimitive_IsTrue_p->val.location.elevation = 1;
	TEST_ASSERT_TRUE(twPrimitive_IsTrue(unit_twBaseTypes_twPrimitive_IsTrue_p));
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_IsTrue_p);
}