/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twPrimitive copy functions
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

static twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p = NULL;
twLocation* twCreateLocationFrom(double latitude, double longitude, double elevation);

TEST_GROUP(unit_twBaseTypes_twPrimitive_Copy);

TEST_SETUP(unit_twBaseTypes_twPrimitive_Copy) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_Copy) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_Copy) {
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_ZeroCopy);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopy);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromString);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromNumber);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromBlob);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromInteger);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromBoolean);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromDateTime);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromTimeSpan);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromLocation);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromInfoTable);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyForUnknownType);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyForUnknownType);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_ZeroCopy) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_Create();
	TEST_ASSERT_NULL(twPrimitive_ZeroCopy(NULL));
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_ZeroCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopy) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_Create();
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->length, unit_twBaseTypes_twPrimitive_Copy_p1->length);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromString) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromString("foo", TRUE);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL_STRING(unit_twBaseTypes_twPrimitive_Copy_p->val.bytes.data, unit_twBaseTypes_twPrimitive_Copy_p1->val.bytes.data);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromNumber) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromNumber(12345.23);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL_FLOAT(unit_twBaseTypes_twPrimitive_Copy_p->val.number, unit_twBaseTypes_twPrimitive_Copy_p1->val.number);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromBlob) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromString("foo", TRUE);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL_STRING(unit_twBaseTypes_twPrimitive_Copy_p->val.bytes.data, unit_twBaseTypes_twPrimitive_Copy_p1->val.bytes.data);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromInteger) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromInteger(12345);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL_INT(unit_twBaseTypes_twPrimitive_Copy_p->val.integer, unit_twBaseTypes_twPrimitive_Copy_p1->val.integer);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromBoolean) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromBoolean(TRUE);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->val.boolean, unit_twBaseTypes_twPrimitive_Copy_p1->val.boolean);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromDateTime) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromDatetime(1234523);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->val.datetime, unit_twBaseTypes_twPrimitive_Copy_p1->val.datetime);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromTimeSpan) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_Create();
	unit_twBaseTypes_twPrimitive_Copy_p->typeFamily = TW_TIMESPAN;
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_EQUAL(0, unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromLocation) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	twLocation *location = twCreateLocationFrom(123.123, 345.34, 567.56);
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromLocation(location);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL_FLOAT(unit_twBaseTypes_twPrimitive_Copy_p->val.location.elevation, unit_twBaseTypes_twPrimitive_Copy_p1->val.location.elevation);
	TEST_ASSERT_EQUAL_FLOAT(unit_twBaseTypes_twPrimitive_Copy_p->val.location.longitude, unit_twBaseTypes_twPrimitive_Copy_p1->val.location.longitude);
	TEST_ASSERT_EQUAL_FLOAT(unit_twBaseTypes_twPrimitive_Copy_p->val.location.latitude, unit_twBaseTypes_twPrimitive_Copy_p1->val.location.latitude);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
}
TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyFromInfoTable) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	twStream *unit_twBaseTypes_twPrimitive_Copy_s = NULL;
	twDataShapeEntry *se = twDataShapeEntry_CreateFromStream(unit_twBaseTypes_twPrimitive_Copy_s);
	twDataShape *ds  = twDataShape_Create(se);
	twInfoTable *t = twInfoTable_Create(ds);
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_CreateFromInfoTable(t);
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(twPrimitive_FullCopy(NULL));
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->length, unit_twBaseTypes_twPrimitive_Copy_p1->length);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->typeFamily, unit_twBaseTypes_twPrimitive_Copy_p1->typeFamily);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Copy_p->type, unit_twBaseTypes_twPrimitive_Copy_p1->type);
	twDataShapeEntry_Delete(se);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p1);
}

TEST(unit_twBaseTypes_twPrimitive_Copy, twBaseTypes_twPrimitive_FullCopyForUnknownType) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Copy_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Copy_p = twPrimitive_Create();
	unit_twBaseTypes_twPrimitive_Copy_p->typeFamily = TW_UNKNOWN_TYPE;
	unit_twBaseTypes_twPrimitive_Copy_p1 = twPrimitive_FullCopy(unit_twBaseTypes_twPrimitive_Copy_p);
	TEST_ASSERT_NULL(unit_twBaseTypes_twPrimitive_Copy_p1);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Copy_p);
}