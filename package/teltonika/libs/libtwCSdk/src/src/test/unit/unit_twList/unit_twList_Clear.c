/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Clear()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_Clear);

TEST_SETUP(unit_twList_Clear) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_Clear) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Clear) {
	RUN_TEST_CASE(unit_twList_Clear, clearNullList);
	RUN_TEST_CASE(unit_twList_Clear, clearStrListEntries);
}

/**
 * Test Plan: Attempts to clear a NULL list, expect TW_INVALID_PARAM return status.
 */
TEST(unit_twList_Clear, clearNullList) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_Clear(NULL));
}

/**
 * Test Plan: Create list, clear list with twList_Clear and verify number of item count is 0.
 */
static int deleteStringCalls = 0;
static void deleteString(void *p) {
	++deleteStringCalls;
	TW_FREE(p);
}

TEST(unit_twList_Clear, clearStrListEntries) {
	twList *list = NULL;
	int i = 0;
	deleteStringCalls = 0;

	/* Create a list */
	list = twList_Create(deleteString);

	/* Add TEST_LIST_BASIC_TEST_SIZE integers */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, twItoa(i)));
	}

	/* Check that our list now has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Clear the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Clear(list));

	/* Check that the list now has 0 entries */
	TEST_ASSERT_EQUAL(0, twList_GetCount(list));

	/* Check that our delete function has been called for each item in the list */
	TEST_ASSERT_EQUAL(i, deleteStringCalls);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}