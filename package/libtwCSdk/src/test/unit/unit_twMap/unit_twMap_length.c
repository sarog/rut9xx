/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_length()
*/

#include <twTls.h>
#include <cfuhash.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

char parseHandlerCalled;
static const char *parseString(void *item) {
	char *string;
	string = (char *)item;
	parseHandlerCalled = TRUE;
	return duplicateString(string);
}

static char deleteHandlerCalled;
static void deleteString(void *item) {
	char *string = (char *)item;
	deleteHandlerCalled = TRUE;
	TW_FREE(string);
}

TEST_GROUP(unit_twMap_length);

TEST_SETUP(unit_twMap_length) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_length) {
}

TEST_GROUP_RUNNER(unit_twMap_length) {
	RUN_TEST_CASE(unit_twMap_length, getLengthFromNullMap);
	RUN_TEST_CASE(unit_twMap_length, verifyValidLength);
}

/**
* Test Plan: Attempts to get length of NULL map.
* 
*/
TEST(unit_twMap_length, getLengthFromNullMap) {
	TEST_ASSERT_EQUAL(0, twMap_length(NULL));
}

/**
* Test Plan: Adds a value to the map, checks the number of items in the map, clears the map then
* check that the number of items in the map is 0.
*/
TEST(unit_twMap_length, verifyValidLength) {
	twMap *map = NULL;
	void *item = NULL;
	char *string = NULL;
	string = duplicateString("A");
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Add(map, string));
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_get(map, "A", &item));
	TEST_ASSERT_EQUAL(1, twMap_length(map));
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Clear(map));
	TEST_ASSERT_EQUAL(0, twMap_length(map));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}