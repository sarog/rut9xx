/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_ForEach()
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
	twMapTestQuery *query = (twMapTestQuery *) item;
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

TEST_GROUP(unit_twMap_ForEach);

TEST_SETUP(unit_twMap_ForEach) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_ForEach) {
}

TEST_GROUP_RUNNER(unit_twMap_ForEach) {
	RUN_TEST_CASE(unit_twMap_ForEach, forEachMoveEvenValuesFromAToB);
	RUN_TEST_CASE(unit_twMap_ForEach, verifyForEachHandler);
}

twMap_foreach_fn forEachEvenValueInAMoveToBHandler(void *key, size_t key_size, void *data, size_t data_size, void *arg) {
	char* valFromA = (char *)data;
	int intValFromA = atoi(valFromA);
	twMap *mapBPassedAsArg = (twMap *)arg;
	if (intValFromA % 2 == 0) {
		TEST_ASSERT_EQUAL(TW_OK, twMap_Add(mapBPassedAsArg, duplicateString(valFromA)));
	}
	return TW_FOREACH_CONTINUE;
}

/**
* Test Plan: Creates two maps one with integers 1 - 10 converted to strings. It then calls the forEach function with the handler
* forEachEvenValueInAMoveToBHandler which will move all the even values into the second map.
*/
TEST(unit_twMap_ForEach, forEachMoveEvenValuesFromAToB) {
	int i = 0;
	void *item = NULL;
	twMap *mapA = NULL;
	twMap *mapB = NULL;
	mapA = twMap_Create(deleteString, parseString);
	mapB = twMap_Create(deleteString, parseString);
	TEST_ASSERT_NOT_NULL(mapA);
	TEST_ASSERT_NOT_NULL(mapB);

	for (i = 1; i <= 10; i++) {
		char *string = twItoa(i);
		twMap_Add(mapA, string);
	}
	/*Call ForEach with handler*/
	TEST_ASSERT_EQUAL(10, twMap_GetCount(mapA));
	twMap_Foreach(mapA, forEachEvenValueInAMoveToBHandler, mapB);

	/*Check map B for even values from map A*/
	TEST_ASSERT_EQUAL(5, twMap_GetCount(mapB));
	for (i = 1; i <= 10; i++) {
		if (i % 2 == 0) {
			char *string = NULL;
			string = twItoa(i);
			TEST_ASSERT_EQUAL(TW_OK,twMap_Find(mapB, string, &item));
			TEST_ASSERT_EQUAL(atoi((char*)item), i);
		}
	}
	
	/*Clean Up Map A*/
	deleteHandlerCalled = FALSE;
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(mapA));
	TEST_ASSERT_TRUE(deleteHandlerCalled);

	/*Clean Up Map B*/
	deleteHandlerCalled = FALSE;
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(mapB));
	TEST_ASSERT_TRUE(deleteHandlerCalled);
}
	

int handlerCallCount = 0;
int handlerContentCheck = TRUE;
int twMapForeachTestForEachHandler( void *key, size_t key_size, void *data, size_t data_size, void *arg){
	twMapTestQuery *testObject = (twMapTestQuery *) data;
	handlerCallCount++;

	if (strcmp((const char *)arg, "Winston")==0) {
		handlerContentCheck += 1;
	}
	if ( strcmp(testObject->keyA, "Peter") == 0 && strcmp(testObject->keyB, "Test1") == 0 && testObject->keyC==1 ) {
		handlerContentCheck += 10;
		return 0;
	}
	if ( strcmp(testObject->keyA, "Ray") == 0 && strcmp(testObject->keyB, "Test2") == 0 && testObject->keyC==2 ) {
		handlerContentCheck += 100;
		return 0;
	}
	if ( strcmp(testObject->keyA, "Egon") == 0 && strcmp(testObject->keyB, "Test3") == 0 && testObject->keyC == 3 ) {
		handlerContentCheck+=1000;
		return 0;
	}
	return 1;
}

/**
 * Test Plan: Build a small list, perform a for each, verify that the handler gets called once for each 
 * item. Clear the LIST when done and verify that it clears.
 */
TEST(unit_twMap_ForEach, verifyForEachHandler) {
	char *sampleUserData="Winston";
	twMap *map = NULL;
	twMapTestQuery *mapItem1, *mapItem2, *mapItem3;
	map = twMap_Create(deleteTestQuery, parseTestQuery);

	mapItem1 = (twMapTestQuery *)TW_MALLOC(sizeof(twMapTestQuery));
	mapItem1->keyA = duplicateString("Peter");
	mapItem1->keyB = duplicateString("Test1");
	mapItem1->keyC = 1;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, mapItem1));

	mapItem2 = (twMapTestQuery *)TW_MALLOC(sizeof(twMapTestQuery));
	mapItem2->keyA = duplicateString("Ray");
	mapItem2->keyB = duplicateString("Test2");
	mapItem2->keyC = 2;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, mapItem2));

	mapItem3 = (twMapTestQuery *)TW_MALLOC(sizeof(twMapTestQuery));
	mapItem3->keyA = duplicateString("Egon");
	mapItem3->keyB = duplicateString("Test3");
	mapItem3->keyC = 3;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, mapItem3));

	handlerCallCount = 0;
	handlerContentCheck = 0;
	twMap_Foreach(map, twMapForeachTestForEachHandler, sampleUserData);

	TEST_ASSERT_EQUAL_MESSAGE(1113, handlerContentCheck, "The foreach handler was not called will all of the list items");
	TEST_ASSERT_EQUAL_MESSAGE(3, handlerCallCount, "The foreach handler was not called 3 times as expected");

	deleteHandlerCalled = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twMap_Clear(map));
	TEST_ASSERT_TRUE(deleteHandlerCalled);

	/* cleanup any leftover map memory */
	if (map) { TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	}
}
