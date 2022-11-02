/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twPrimitive_Compare()
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

TEST_GROUP(unit_twBaseTypes_twPrimitive_Compare);

TEST_SETUP(unit_twBaseTypes_twPrimitive_Compare) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_Compare) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_Compare) {
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Compare, twBaseTypes_twPrimitive_Compare_Error_Codes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Compare, twBaseTypes_twPrimitive_Compare);
}

TEST(unit_twBaseTypes_twPrimitive_Compare, twBaseTypes_twPrimitive_Compare_Error_Codes) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Compare_p1 = twPrimitive_Create();
	twPrimitive *unit_twBaseTypes_twPrimitive_Compare_p2 = twPrimitive_Create();
	TEST_ASSERT_EQUAL(-1, twPrimitive_Compare(NULL, NULL));

	/*  if family type is unknown data type    */
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_UNKNOWN_TYPE;
	TEST_ASSERT_EQUAL(-1, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Compare_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Compare_p2);
}

TEST(unit_twBaseTypes_twPrimitive_Compare, twBaseTypes_twPrimitive_Compare) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Compare_p1 = twPrimitive_Create();
	twPrimitive *unit_twBaseTypes_twPrimitive_Compare_p2 = twPrimitive_Create();
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p1));

	/*  if type of both the pointers are different   */
	unit_twBaseTypes_twPrimitive_Compare_p2->type = TW_STRING;
	TEST_ASSERT_EQUAL(1, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p2->type = TW_NOTHING; /*again setting it to default type*/

	/*  Rest of the comparisons based on the type family   */
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_STRING;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_BLOB;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_VARIANT;
	TEST_ASSERT_EQUAL(-1, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_NUMBER;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_INTEGER;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_BOOLEAN;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_DATETIME;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_TIMESPAN;
	TEST_ASSERT_EQUAL(-1, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_LOCATION;
	TEST_ASSERT_EQUAL(0, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));
	unit_twBaseTypes_twPrimitive_Compare_p1->typeFamily = TW_INFOTABLE;
	TEST_ASSERT_EQUAL(-1, twPrimitive_Compare(unit_twBaseTypes_twPrimitive_Compare_p1, unit_twBaseTypes_twPrimitive_Compare_p2));

	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Compare_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Compare_p2);
}