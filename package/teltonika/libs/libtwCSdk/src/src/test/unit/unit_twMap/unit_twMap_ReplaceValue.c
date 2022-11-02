/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for ReplaceValue()
*/

#include <twTls.h>
#include <cfuhash.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static char parseHandlerCalled;
static const char *parseString(void *item) {
	char *string;
	string = (char *) item;
	parseHandlerCalled = TRUE;
	return duplicateString(string);
}

static char deleteHandlerCalled;
static void deleteString(void *item) {
	char *string = (char *)item;
	deleteHandlerCalled = TRUE;
	TW_FREE(string);
}

TEST_GROUP(unit_twMap_ReplaceValue);

TEST_SETUP(unit_twMap_ReplaceValue) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_ReplaceValue) {
}

TEST_GROUP_RUNNER(unit_twMap_ReplaceValue) {
	RUN_TEST_CASE(unit_twMap_ReplaceValue, replaceValidWithValid);
	RUN_TEST_CASE(unit_twMap_ReplaceValue, replaceInNullMap);
	RUN_TEST_CASE(unit_twMap_ReplaceValue, replaceValidWithNull);
	RUN_TEST_CASE(unit_twMap_ReplaceValue, replaceNullWithValid);
	RUN_TEST_CASE(unit_twMap_ReplaceValue, replaceNullWithNull);
	RUN_TEST_CASE(unit_twMap_ReplaceValue, valueNotAddedToMap);
}

/**
* Test Plan: Creates two valid values and replaces value 1 with value 2 in a valid map. 
*/
TEST(unit_twMap_ReplaceValue, replaceValidWithValid) {
	twMap *map = NULL;
	void *item = NULL;
	char *twMap_testVariable1 = NULL;
	char *twMap_testVariable2 = NULL;
	twMap_testVariable1 = duplicateString("A");
	twMap_testVariable2 = duplicateString("B");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, twMap_testVariable1));
	TEST_ASSERT_EQUAL(TW_OK, twMap_ReplaceValue(map, twMap_testVariable1, twMap_testVariable2, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Find(map, twMap_testVariable2, &item));
	TEST_ASSERT_EQUAL(twMap_testVariable2,(char*)item);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Creates two valid values and attempts to replace them in a NULL map. 
*/
TEST(unit_twMap_ReplaceValue, replaceInNullMap) {
	char *twMap_testVariable1 = NULL;
	char *twMap_testVariable2 = NULL;
	twMap_testVariable1 = duplicateString("A");
	twMap_testVariable2 = duplicateString("B");
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_ReplaceValue(NULL, twMap_testVariable1, twMap_testVariable2,  FALSE));
}

/**
* Test Plan: Attempts to replace a valid value with NULL.
*/
TEST(unit_twMap_ReplaceValue, replaceValidWithNull) {
	twMap *map = NULL;
	char *twMap_testVariable1 = NULL;
	twMap_testVariable1 = duplicateString("A");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_ReplaceValue(map, twMap_testVariable1, NULL, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Attempts to replace a NULL value with a valid value.
*/
TEST(unit_twMap_ReplaceValue, replaceNullWithValid) {
	twMap *map = NULL;
	char *twMap_testVariable2 = NULL;
	twMap_testVariable2 = duplicateString("B");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_ReplaceValue(map, NULL, twMap_testVariable2, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Creates a valid map and attempts to replace NULL with NULL. 
* 
*/
TEST(unit_twMap_ReplaceValue, replaceNullWithNull) {
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_ReplaceValue(map, NULL, NULL, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Creates a single test variable then attempts to replace a non-existent value in a valid map. 
*/
TEST(unit_twMap_ReplaceValue, valueNotAddedToMap) {
	twMap *map = NULL;
	char *twMap_testVariable2 = NULL;
	twMap_testVariable2 = duplicateString("B");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_MISSING, twMap_ReplaceValue(map, "C", twMap_testVariable2, FALSE));
}