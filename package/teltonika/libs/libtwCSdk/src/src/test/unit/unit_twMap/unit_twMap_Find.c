/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_Find()
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

typedef struct twMapTestQuery {
	char *keyA;
	char *keyB;
	int keyC;
} twMapTestQuery;

static void deleteTestQuery(void *item) {
	twMapTestQuery *query = (twMapTestQuery *)item;
	deleteHandlerCalled = TRUE;
	TW_FREE(query->keyA);
	TW_FREE(query->keyB);
	TW_FREE(query);
}

static const char *parseTestQuery(void *item) {
	twMapTestQuery *query = (twMapTestQuery *)item;
	size_t maxKeyLength = 100;
	char *indexKey = TW_MALLOC(maxKeyLength - 1);

	parseHandlerCalled = TRUE;
	snprintf(
		indexKey,
		maxKeyLength,
		"%s|%s|%i",
		query->keyA,
		query->keyB,
		query->keyC
	);
	return indexKey;
}

TEST_GROUP(unit_twMap_Find);

TEST_SETUP(unit_twMap_Find) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_Find) {
}

TEST_GROUP_RUNNER(unit_twMap_Find) {
	RUN_TEST_CASE(unit_twMap_Find, findInNullMap);
	RUN_TEST_CASE(unit_twMap_Find, findInvalidValueInValidMap);
	RUN_TEST_CASE(unit_twMap_Find, addMultipleFind);
}

/**
* Test Plan: Attempts to find a value in a map that hasn't been created.
*/
TEST(unit_twMap_Find, findInNullMap) {
	void *item = NULL;
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Find(NULL, "A", &item));
}

/**
* Test Plan: Attempts to find a value that hasn't been added to a valid map
*/
TEST(unit_twMap_Find, findInvalidValueInValidMap) {
	void *item = NULL;
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Find(NULL, "A", &item));
	TEST_ASSERT_EQUAL(TW_MAP_MISSING, twMap_Find(map, "A", &item));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Add an item, verify that its parse handler was called and that the map counts grows by 1. 
* Repeat the process, verify growth to two. Use the item as a query to find the original item in the map. 
* Proceed to delete the items and verify that their delete handler is called.
*/
TEST(unit_twMap_Find, addMultipleFind) {
	void *data;
	twMap *map = NULL;
	twMapTestQuery *mapItem1, *mapItem2;
	map = twMap_Create(deleteTestQuery, parseTestQuery);
	TEST_ASSERT_NOT_NULL(map);

	deleteHandlerCalled = FALSE;
	parseHandlerCalled = FALSE;

	/* Add an Item */
	mapItem1 = TW_MALLOC(sizeof(twMapTestQuery));
	mapItem1->keyA = duplicateString("Bill");
	mapItem1->keyB = duplicateString("Test1");
	mapItem1->keyC = 1;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, mapItem1));
	TEST_ASSERT_TRUE(parseHandlerCalled);
	TEST_ASSERT_EQUAL(1, twMap_GetCount(map));

	/* Add an Item */
	parseHandlerCalled = FALSE;
	mapItem2 = TW_MALLOC(sizeof(twMapTestQuery));
	mapItem2->keyA = duplicateString("Fred");
	mapItem2->keyB = duplicateString("Test2");
	mapItem2->keyC = 2;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, mapItem2));
	TEST_ASSERT_TRUE(parseHandlerCalled);
	TEST_ASSERT_EQUAL(2, twMap_GetCount(map));

	/* Find Items */
	data = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Find(map, mapItem1, &data));
	TEST_ASSERT_EQUAL(mapItem1, data);

	data = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Find(map, mapItem2, &data));
	TEST_ASSERT_EQUAL(mapItem2, data);

	/* Clean Up */

	deleteHandlerCalled = FALSE;
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	TEST_ASSERT_TRUE(deleteHandlerCalled);
}