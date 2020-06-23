/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_put()
*/

#include <twTls.h>
#include <cfuhash.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static const char *parseString(void *item) {
	char *string;
	string = (char *)item;
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

TEST_GROUP(unit_twMap_put);

TEST_SETUP(unit_twMap_put) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_put) {
}

TEST_GROUP_RUNNER(unit_twMap_put) {
	RUN_TEST_CASE(unit_twMap_put, putGetLength);
}

/**
* Test Plan: Puts multiple values into a map and verifies the results. 
*/
TEST(unit_twMap_put, putGetLength) {
	twMap *map = NULL;
	int i = 0;
	map = twMap_Create(deleteString, parseString);

	for (i = 0; i < 100; i++) {
		char *string = twItoa(i);
		twMap_put(map, string, string);
	}

	TEST_ASSERT_EQUAL(twMap_GetCount(map), 100);
	for (i = 0; i < 100; i++) {
		int result = 0;
		char *item = NULL;
		char *string = twItoa(i);
		result = twMap_get(map, string, (void **)&item);
		TEST_ASSERT_EQUAL(TW_MAP_OK, result);
		TEST_ASSERT_EQUAL_STRING(string, item);
		TW_FREE(string);
	}
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
}