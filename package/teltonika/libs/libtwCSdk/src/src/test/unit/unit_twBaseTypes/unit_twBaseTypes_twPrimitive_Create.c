/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twPrimitive create functions
 */

#include "twApi.h"
#include "twExt.h"
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

static uint32_t mock_twPrimitive_CreateFromInfoTable_count;
static uint32_t mock_twInfoTable_Delete_count;
static twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
static twStream *unit_twBaseTypes_twPrimitive_Create_s = NULL;

TEST_GROUP(unit_twBaseTypes_twPrimitive_Create);

TEST_SETUP(unit_twBaseTypes_twPrimitive_Create) {
	eatLogs();
	mock_twPrimitive_CreateFromInfoTable_count = 0;
	mock_twInfoTable_Delete_count = 0;
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_Create) {
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_Create);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStream);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ErrorCodes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToNothing);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToString);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToNumber);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToInteger);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToBoolean);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToDatetime);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToTimespan);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToLocation);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToInfotable);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateVariant);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromString);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromNumber);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromInteger);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromLocation);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromBlob);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromDateTime);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromCurrentTime);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromBoolean);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromInfoTable);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ErrorCodes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToNothing);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToString);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToJson);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToNumber);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInteger);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToBoolean);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToDateTime);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToBlob);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToTimeSpan);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToLocation);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInfoTable_ErrorCodes);
	RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInfoTable);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_Create) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_Create();
	TEST_ASSERT_NOT_NULL(unit_twBaseTypes_twPrimitive_Create_p);
	TEST_ASSERT_EQUAL(1, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStream) {
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	TEST_ASSERT_NULL(twPrimitive_CreateFromStream(NULL));
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStream(unit_twBaseTypes_twPrimitive_Create_s);
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Create_p->typeFamily, TW_NOTHING);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ErrorCodes) {
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	TEST_ASSERT_EQUAL(NULL, twPrimitive_CreateFromStreamTyped(NULL, TW_NOTHING));
	TEST_ASSERT_EQUAL(NULL, twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_UNKNOWN_TYPE));
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToNothing) {
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_NOTHING);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(1, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToString) {
	char *input = "fooandfund";
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	stringToStream(input, unit_twBaseTypes_twPrimitive_Create_s);
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_STRING);
	TEST_ASSERT_EQUAL_STRING(input, unit_twBaseTypes_twPrimitive_Create_p->val.bytes.data);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToNumber) {
	double d = 123.23;
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	swap8bytes((char *) &d);
	twStream_AddBytes(unit_twBaseTypes_twPrimitive_Create_s, &d, sizeof(double));
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_NUMBER);
	TEST_ASSERT_EQUAL(sizeof(double), unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_NUMBER, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NUMBER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_FLOAT(123.23, unit_twBaseTypes_twPrimitive_Create_p->val.number);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToInteger) {
	int i = 9;
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	swap4bytes((char *) &i);
	twStream_AddBytes(unit_twBaseTypes_twPrimitive_Create_s, &i, sizeof(int));
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_INTEGER);
	TEST_ASSERT_EQUAL(TW_INTEGER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(TW_INTEGER, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(sizeof(int), unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(9, unit_twBaseTypes_twPrimitive_Create_p->val.integer);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToBoolean) {
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_BOOLEAN);
	TEST_ASSERT_EQUAL(1, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(0, unit_twBaseTypes_twPrimitive_Create_p->val.boolean);
	TEST_ASSERT_EQUAL(TW_BOOLEAN, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_BOOLEAN, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToDatetime) {
	uint64_t i = 12;
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	swap8bytes((char *) &i);
	twStream_AddBytes(unit_twBaseTypes_twPrimitive_Create_s, &i, sizeof(uint64_t));
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_DATETIME);
	TEST_ASSERT_EQUAL(sizeof(double), unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_FLOAT(12, unit_twBaseTypes_twPrimitive_Create_p->val.datetime);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToTimespan) {
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	TEST_ASSERT_EQUAL(0, twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_TIMESPAN));
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToLocation) {
	double d = 123.23;
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	swap8bytes((char *) &d);
	twStream_AddBytes(unit_twBaseTypes_twPrimitive_Create_s, &d, sizeof(double));
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_LOCATION);
	TEST_ASSERT_EQUAL(sizeof(double)*3, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_LOCATION, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(TW_LOCATION, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL_FLOAT(123.23, unit_twBaseTypes_twPrimitive_Create_p->val.location.longitude);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_twPrimitive_CreateFromStreamTyped_ToInfotable) {
	char *input = "fooandfund";
	unit_twBaseTypes_twPrimitive_Create_s = twStream_Create();
	stringToStream(input, unit_twBaseTypes_twPrimitive_Create_s);
	twStream_Reset(unit_twBaseTypes_twPrimitive_Create_s);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromStreamTyped(unit_twBaseTypes_twPrimitive_Create_s, TW_INFOTABLE);
	TEST_ASSERT_EQUAL(TW_INFOTABLE, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(TW_INFOTABLE, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(sizeof("fooandfund"), unit_twBaseTypes_twPrimitive_Create_p->val.infotable->length);
	TEST_ASSERT_TRUE((unit_twBaseTypes_twPrimitive_Create_p->val.infotable->ds->numEntries)!=0);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateVariant) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p1 = NULL;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_Create();
	TEST_ASSERT_EQUAL(NULL, twPrimitive_CreateVariant(NULL));
	unit_twBaseTypes_twPrimitive_Create_p1 = twPrimitive_CreateVariant(unit_twBaseTypes_twPrimitive_Create_p);
	TEST_ASSERT_NOT_NULL(unit_twBaseTypes_twPrimitive_Create_p1);
	TEST_ASSERT_TRUE_MESSAGE(unit_twBaseTypes_twPrimitive_Create_p1->type == TW_VARIANT, "Expected is VARIANT");
	TEST_ASSERT_TRUE_MESSAGE(unit_twBaseTypes_twPrimitive_Create_p1->typeFamily == TW_VARIANT, "Expected is VARIANT");
	TEST_ASSERT_EQUAL(unit_twBaseTypes_twPrimitive_Create_p, unit_twBaseTypes_twPrimitive_Create_p1->val.variant);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p1);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromString) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromString("foo", TRUE);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_STRING("foo", unit_twBaseTypes_twPrimitive_Create_p->val.bytes.data);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromNumber) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromNumber(12345.23);
	TEST_ASSERT_EQUAL(TW_NUMBER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_FLOAT(12345.23, unit_twBaseTypes_twPrimitive_Create_p->val.number);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromInteger) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromInteger(12345);
	TEST_ASSERT_EQUAL(TW_INTEGER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_INT(12345, unit_twBaseTypes_twPrimitive_Create_p->val.integer);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromLocation) {
	twLocation *location = twCreateLocationFrom(123.123, 345.34, 567.56);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromLocation(location);
	TEST_ASSERT_EQUAL(TW_LOCATION, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_FLOAT(location->elevation, unit_twBaseTypes_twPrimitive_Create_p->val.location.elevation);
	TEST_ASSERT_EQUAL_FLOAT(location->latitude, unit_twBaseTypes_twPrimitive_Create_p->val.location.latitude);
	TEST_ASSERT_EQUAL_FLOAT(location->longitude, unit_twBaseTypes_twPrimitive_Create_p->val.location.longitude);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromBlob) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromBlob("foo", 4, FALSE, TRUE);
	TEST_ASSERT_EQUAL(TW_BLOB, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_STRING("foo", unit_twBaseTypes_twPrimitive_Create_p->val.bytes.data);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromDateTime) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromDatetime(1234523);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(1234523, unit_twBaseTypes_twPrimitive_Create_p->val.datetime);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromCurrentTime) {
	DATETIME now;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromCurrentTime();
	now = twGetSystemTime(TRUE);
	TEST_ASSERT_TRUE((now - unit_twBaseTypes_twPrimitive_Create_p->val.datetime) >= 0 && (now - unit_twBaseTypes_twPrimitive_Create_p->val.datetime) < 2);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromBoolean) {
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromBoolean(TRUE);
	TEST_ASSERT_EQUAL(TW_BOOLEAN, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(TRUE, unit_twBaseTypes_twPrimitive_Create_p->val.boolean);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromInfoTable) {
	twStream *unit_twBaseTypes_twPrimitive_Create_s = NULL;
	twDataShapeEntry *se = twDataShapeEntry_CreateFromStream(unit_twBaseTypes_twPrimitive_Create_s);
	twDataShape *ds  = twDataShape_Create(se);
	twInfoTable *t = twInfoTable_Create(ds);
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromInfoTable(t);
	TEST_ASSERT_EQUAL(TW_INFOTABLE, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL(t->length, unit_twBaseTypes_twPrimitive_Create_p->length);
	twStream_Delete(unit_twBaseTypes_twPrimitive_Create_s);
	twDataShapeEntry_Delete(se);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ErrorCodes) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	TEST_ASSERT_NULL(twPrimitive_CreateFromJson(NULL, "foo", TW_NOTHING));
	TEST_ASSERT_NULL(twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_UNKNOWN_TYPE));
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p1 = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_NOTHING);
	TEST_ASSERT_EQUAL(1, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToNothing) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, "foo", TW_NOTHING);
	TEST_ASSERT_EQUAL(1, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NOTHING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToString) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valuestring = duplicateString("foo");
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_STRING);
	TEST_ASSERT_EQUAL_STRING(unit_twBaseTypes_twPrimitive_Create_json->valuestring, unit_twBaseTypes_twPrimitive_Create_p->val.bytes.data);
	TEST_ASSERT_EQUAL(4, unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToJson) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valuestring = duplicateString("foo");
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_JSON);
	TEST_ASSERT_EQUAL(strlen("foo"), unit_twBaseTypes_twPrimitive_Create_p->length);
	TEST_ASSERT_EQUAL(TW_JSON, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_STRING, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToNumber) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valuedouble = 123.12;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_NUMBER);
	TEST_ASSERT_EQUAL_FLOAT(123.12, unit_twBaseTypes_twPrimitive_Create_p->val.number);
	TEST_ASSERT_EQUAL(TW_NUMBER, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_NUMBER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInteger) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valueint = 1111;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_INTEGER);
	TEST_ASSERT_EQUAL(1111, unit_twBaseTypes_twPrimitive_Create_p->val.integer);
	TEST_ASSERT_EQUAL(TW_INTEGER, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_INTEGER, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToBoolean) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateBool(1);
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_BOOLEAN);
	TEST_ASSERT_EQUAL(0, ('1' == unit_twBaseTypes_twPrimitive_Create_p->val.boolean));
	TEST_ASSERT_EQUAL(TW_BOOLEAN, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_BOOLEAN, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToDateTime) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valuedouble = 123.00;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_DATETIME);
	TEST_ASSERT_EQUAL(123, unit_twBaseTypes_twPrimitive_Create_p->val.datetime);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_DATETIME, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToBlob) {
	twPrimitive *unit_twBaseTypes_twPrimitive_Create_p = NULL;
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_json->valuestring = duplicateString("foo");
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_BLOB);
	TEST_ASSERT_EQUAL(strlen(unit_twBaseTypes_twPrimitive_Create_json->valuestring), unit_twBaseTypes_twPrimitive_Create_p->val.bytes.len);
	TEST_ASSERT_EQUAL(TW_BLOB, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToTimeSpan) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateObject();
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_TIMESPAN);
	TEST_ASSERT_NULL(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToLocation) {
	cJSON * json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "latitude", 12.34);
	cJSON_AddNumberToObject(json, "longitude", 56.78);
	cJSON_AddNumberToObject(json, "elevation", 90);

	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(json, NULL, TW_LOCATION);
	TEST_ASSERT_EQUAL(TW_LOCATION, unit_twBaseTypes_twPrimitive_Create_p->type);
	TEST_ASSERT_EQUAL(TW_LOCATION, unit_twBaseTypes_twPrimitive_Create_p->typeFamily);
	TEST_ASSERT_EQUAL_FLOAT(12.34, unit_twBaseTypes_twPrimitive_Create_p->val.location.latitude);
	TEST_ASSERT_EQUAL_FLOAT(56.78, unit_twBaseTypes_twPrimitive_Create_p->val.location.longitude);
	TEST_ASSERT_EQUAL_FLOAT(90, unit_twBaseTypes_twPrimitive_Create_p->val.location.elevation);

	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(json);
}

static twInfoTable * mock_twInfoTable_CreateFromJson(cJSON *json, char *singleEntryName) {
	return NULL;
}

static twPrimitive * mock_twPrimitive_CreateFromInfoTable(twInfoTable * it) {
	mock_twPrimitive_CreateFromInfoTable_count++;
	return NULL;
}

static void mock_twInfoTable_Delete(void * it) {
	mock_twInfoTable_Delete_count++;
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInfoTable_ErrorCodes) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateString("foo");
	twApi_stub->twInfoTable_CreateFromJson = mock_twInfoTable_CreateFromJson;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_INFOTABLE);
	TEST_ASSERT_NULL(unit_twBaseTypes_twPrimitive_Create_p);
	twPrimitive_Delete(unit_twBaseTypes_twPrimitive_Create_p);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}

TEST(unit_twBaseTypes_twPrimitive_Create, test_twBaseTypes_check_twPrimitive_CreateFromJson_ToInfoTable) {
	cJSON *unit_twBaseTypes_twPrimitive_Create_json = cJSON_CreateString("foo");
	twApi_stub->twPrimitive_CreateFromInfoTable = mock_twPrimitive_CreateFromInfoTable;
	twApi_stub->twInfoTable_Delete = mock_twInfoTable_Delete;
	unit_twBaseTypes_twPrimitive_Create_p = twPrimitive_CreateFromJson(unit_twBaseTypes_twPrimitive_Create_json, NULL, TW_INFOTABLE);
	TEST_ASSERT_EQUAL(1, mock_twPrimitive_CreateFromInfoTable_count);
	TEST_ASSERT_EQUAL(1, mock_twInfoTable_Delete_count);
	cJSON_Delete(unit_twBaseTypes_twPrimitive_Create_json);
}
