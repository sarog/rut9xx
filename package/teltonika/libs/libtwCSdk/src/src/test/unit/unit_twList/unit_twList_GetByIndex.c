/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_GetByIndex()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_GetByIndex);

TEST_SETUP(unit_twList_GetByIndex) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_GetByIndex) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_GetByIndex) {
	RUN_TEST_CASE(unit_twList_GetByIndex, getByIndexOfNullList);
	RUN_TEST_CASE(unit_twList_GetByIndex, getByIndexOfEmptyList);
	RUN_TEST_CASE(unit_twList_GetByIndex, getByIndexOutOfBounds);
	RUN_TEST_CASE(unit_twList_GetByIndex, getByIndex);
}

/**
 * Test Plan: Attempt to get a list entry from a NULL list.
 */
TEST(unit_twList_GetByIndex, getByIndexOfNullList) {
	TEST_ASSERT_NULL(twList_GetByIndex(NULL, 0));;
}

/**
 * Test Plan: Attempt to get a list entry for an empty list.
 */
TEST(unit_twList_GetByIndex, getByIndexOfEmptyList) {
	twList *list = NULL;
	list = twList_Create(doNothing);
	TEST_ASSERT_NULL(twList_GetByIndex(list, 0));;
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Attempt to get out of bounds list entries.
 */
TEST(unit_twList_GetByIndex, getByIndexOutOfBounds) {
	twList *list = NULL;
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_NULL(twList_GetByIndex(list, -1));
	TEST_ASSERT_NULL(twList_GetByIndex(list, 2));
}

/**
 * Test Plan: Get entries from a list.
 */
TEST(unit_twList_GetByIndex, getByIndex) {
	twList *list = NULL;
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_NOT_NULL(twList_GetByIndex(list, 0));
	TEST_ASSERT_NOT_NULL(twList_GetByIndex(list, 1));
}