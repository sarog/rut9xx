/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_Create()
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

static const char *parseBadString(void *item) {
	return NULL;
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

static char secondParseHandlerCalled;
static const char *parseTestQuery2(void *item) {
	secondParseHandlerCalled = TRUE;
	return duplicateString("junk");
}


TEST_GROUP(unit_twMap_Create);

TEST_SETUP(unit_twMap_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_Create) {
}

TEST_GROUP_RUNNER(unit_twMap_Create) {
	RUN_TEST_CASE(unit_twMap_Create, createVerify);
	RUN_TEST_CASE(unit_twMap_Create, badParseFunction);
	RUN_TEST_CASE(unit_twMap_Create, multipleParseHandlers);
}

/**
* Test Plan: Creates a new map and verifies it is not NULL. 
*/
TEST(unit_twMap_Create, createVerify) {
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_NOT_NULL(map);
}

/**
* Test Plan: Creates map with a parse function that returns NULL.
*
*/
TEST(unit_twMap_Create, badParseFunction) {
	twMap *map = NULL;
	char *stringA = duplicateString("A");
	map = twMap_Create(deleteString, parseBadString);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Add(map, stringA));
	TW_FREE(stringA);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}

/**
* Test Plan: Creates two maps and adds items to them. Test verifies that two distinct parse handlers
* are called after calling twMap_Add.
*/
TEST(unit_twMap_Create, multipleParseHandlers) {
	twMapTestQuery *mapItem1;
	twMapTestQuery *mapItem2;
	twMap *map1 = NULL;
	twMap *map2 = NULL;
	map1 = twMap_Create(deleteTestQuery, parseTestQuery);
	map2 = twMap_Create(deleteTestQuery, parseTestQuery2);

	/* Add an Item */
	mapItem1 = TW_MALLOC(sizeof(twMapTestQuery));
	mapItem1->keyA = duplicateString("Bill");
	mapItem1->keyB = duplicateString("Test1");
	mapItem1->keyC = 1;

	mapItem2 = TW_MALLOC(sizeof(twMapTestQuery));
	mapItem2->keyA = duplicateString("Fred");
	mapItem2->keyB = duplicateString("Test2");
	mapItem2->keyC = 2;

	parseHandlerCalled = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map1, mapItem1));
	TEST_ASSERT_EQUAL(TRUE, parseHandlerCalled);
	TEST_ASSERT_EQUAL(1, twMap_GetCount(map1));

	secondParseHandlerCalled = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map2, mapItem2));
	TEST_ASSERT_EQUAL(TRUE, secondParseHandlerCalled);
	TEST_ASSERT_EQUAL(1, twMap_GetCount(map2));

	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map1));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map2));
}