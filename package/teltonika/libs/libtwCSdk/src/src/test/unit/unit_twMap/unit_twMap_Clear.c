/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_Clear()
*/

#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static char deleteHandlerCalled;
static void deleteString(void *item) {
	char *string = (char *)item;
	deleteHandlerCalled = TRUE;
	TW_FREE(string);
}

static char parseHandlerCalled;
static const char *parseString(void *item) {
	char *string;
		string = (char *)item;
		parseHandlerCalled = TRUE;
		return duplicateString(string);
}

TEST_GROUP(unit_twMap_Clear);

TEST_SETUP(unit_twMap_Clear) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_Clear) {
}

TEST_GROUP_RUNNER(unit_twMap_Clear) {
	RUN_TEST_CASE(unit_twMap_Clear, clearNegativeCase);
	RUN_TEST_CASE(unit_twMap_Clear, addValuesToMapClear);
}

/**
* Test Plan: Attempt to clear null map.
*/
TEST(unit_twMap_Clear, clearNegativeCase) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Clear(NULL));
}

/**
* Test Plan: Add 100 items to a map clear the map and verify count is 0.
*/
TEST(unit_twMap_Clear, addValuesToMapClear) {
	twMap *map = NULL;
	int i = 0;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_NOT_NULL(map);
	for (i = 0; i < 100; i++) {
		char *string = twItoa(i);
		TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, string));
	}
	deleteHandlerCalled = FALSE;
	TEST_ASSERT_EQUAL(100, twMap_GetCount(map));
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Clear(map));
	TEST_ASSERT_EQUAL(0, twMap_GetCount(map));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	TEST_ASSERT_TRUE(deleteHandlerCalled);
}