/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twMap_Delete()
*/

#include <twTls.h>
#include <cfuhash.h>
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

TEST_GROUP(unit_twMap_Delete);

TEST_SETUP(unit_twMap_Delete) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_Delete) {
}

TEST_GROUP_RUNNER(unit_twMap_Delete) {
	RUN_TEST_CASE(unit_twMap_Delete, deleteNull);
	RUN_TEST_CASE(unit_twMap_Delete, addDeleteValid);
}

/**
* Test Plan: Attempts to delete case with NULL map.
*/
TEST(unit_twMap_Delete, deleteNull) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twMap_Delete(NULL));
}

/**
* Test Plan: Creates valid map and value, deletes the map and verifies the delete handler was called.
*/
TEST(unit_twMap_Delete, addDeleteValid) {
	twMap *map = NULL;
	char *stringA = NULL;
	stringA = duplicateString("A");
	deleteHandlerCalled = FALSE;
	map = twMap_Create(deleteString, parseString);
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Add(map, stringA));
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	TEST_ASSERT_TRUE(deleteHandlerCalled);
}