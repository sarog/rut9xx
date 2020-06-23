/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_get()
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

TEST_GROUP(unit_twMap_get);

TEST_SETUP(unit_twMap_get) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_get) {
}

TEST_GROUP_RUNNER(unit_twMap_get) {
	RUN_TEST_CASE(unit_twMap_get, getInvalidValue);
	RUN_TEST_CASE(unit_twMap_get, getValidString);
}

/**
* Test Plan: Attempts to get value that has not been added to the map.  
*/
TEST(unit_twMap_get, getInvalidValue) {
	twMap *map = NULL;
	void *item = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_MISSING, twMap_get(map, "A", &item));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Adds a value to the map, gets the value using get then verifies that
* the correct value is returned in the return argument.
*/
TEST(unit_twMap_get, getValidString) {
	twMap *map = NULL;
	void *item = NULL;
	char *string = NULL;
	string = duplicateString("A");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Add(map, string));
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_get(map, "A", &item));
	TEST_ASSERT_EQUAL_STRING("A", (char*)item);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}