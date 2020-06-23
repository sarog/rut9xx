/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_Add()
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

TEST_GROUP(unit_twMap_Add);

TEST_SETUP(unit_twMap_Add) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_Add) {
}

TEST_GROUP_RUNNER(unit_twMap_Add) {
	RUN_TEST_CASE(unit_twMap_Add, addVerifyCorrectValues);
	RUN_TEST_CASE(unit_twMap_Add, addValidValueTwice);
	RUN_TEST_CASE(unit_twMap_Add, addValidValueToNullMap);
	RUN_TEST_CASE(unit_twMap_Add, addNullValueToNullMap);
	RUN_TEST_CASE(unit_twMap_Add, addStringLiteralToValidMap);
	RUN_TEST_CASE(unit_twMap_Add, addNullToValidMap);
	RUN_TEST_CASE(unit_twMap_Add, addMultipleAndVerifyHandlers);
}

/**
* Test Plan: Add items to a map and verify that the entries are correct.
*/
TEST(unit_twMap_Add, addVerifyCorrectValues) {
	int i = 0;
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_NOT_NULL(map);
	for (i = 0; i < 100; i++) {
		char *string = twItoa(i);
		TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, string));
	}

	TEST_ASSERT_EQUAL(100, twMap_GetCount(map));
	for (i = 0; i < 100; i++) {
		char *item = NULL;
		char *string = twItoa(i);
		TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Find(map, string, (void **)&item));
		TEST_ASSERT_EQUAL_STRING(string, item);
		TW_FREE(string);
	}

	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Creates valid value and map and attempts to add the same value twice.
*/
TEST(unit_twMap_Add, addValidValueTwice) {
	char *stringA = NULL;
	char *stringAToo = NULL;
	twMap *map = NULL;
	stringA = duplicateString("A");
	stringAToo = duplicateString("A"); 
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Add(map, stringA));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twMap_Add(map, stringAToo));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map)); 
}

/**
* Test Plan: Creates a valid value and attempts to add it to a NULL map
*/
TEST(unit_twMap_Add, addValidValueToNullMap) {
	char *stringA = NULL;
	stringA = duplicateString("A");
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Add(NULL, stringA));
}

/**
* Test Plan: Attempt to add a NULL value to a NULL map 
*/
TEST(unit_twMap_Add, addNullValueToNullMap) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Add(NULL, NULL));
}

/**
* Test Plan: Attempts to add string literal to a valid map.  
*/
TEST(unit_twMap_Add, addStringLiteralToValidMap) {
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Add("B", map)); 
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Create a map and attempt to add a Null value.
*/
TEST(unit_twMap_Add, addNullToValidMap) {
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Add(map, NULL)); 
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Add multiple items to a map to and verify that the parse handler is called. Then delete the map and 
* verify that the delete handler is called.
*/
TEST(unit_twMap_Add, addMultipleAndVerifyHandlers) {
	twMap *map = NULL;
	char *stringA = NULL;
	char *stringB = NULL;
	stringA = duplicateString("A");
	stringB = duplicateString("B");

	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_NOT_NULL(map);

	deleteHandlerCalled = FALSE;
	parseHandlerCalled = FALSE;

	/* Add an item and verify that the parse handler was called and item was added*/
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, stringA));
	TEST_ASSERT_TRUE(parseHandlerCalled);
	TEST_ASSERT_EQUAL(1, twMap_GetCount(map));

	/* Add another item and verify that the parse handler was called and item was added*/
	parseHandlerCalled = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, stringB));
	TEST_ASSERT_TRUE(parseHandlerCalled);
	TEST_ASSERT_EQUAL(2, twMap_GetCount(map));

	/* Delete the map and verify delete handler was called */
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	TEST_ASSERT_TRUE(deleteHandlerCalled);
}
