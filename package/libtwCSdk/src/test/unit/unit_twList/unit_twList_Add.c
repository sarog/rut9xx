/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Add()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_Add);

TEST_SETUP(unit_twList_Add) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_Add) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Add) {
	RUN_TEST_CASE(unit_twList_Add, addValueWithNullValueParameter);
	RUN_TEST_CASE(unit_twList_Add, addValueWithNullListParameter);
	RUN_TEST_CASE(unit_twList_Add, addStrValuesToList);
}

/**
 * Test Plan: Try to add a NULL value to a list, expect TW_OK return status because ListEntires with NULL values are
 * supported.
 */
TEST(unit_twList_Add, addValueWithNullValueParameter) {
	twList *list = NULL;

	/* Create a list */
	list = twList_Create(&doNothing);
	/* Try to add a NULL value to the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, NULL));
	/* Clean up the list we created */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Try to add a value to a NULL list, expect TW_INVALID_PARAM return status.
 */
TEST(unit_twList_Add, addValueWithNullListParameter) {
	/* Try to add a value to a NULL list */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_Add(NULL, 0));
}

/**
 * Test Plan: Create a list, add TEST_LIST_BASIC_TEST_SIZE strings, and then remove each one by repeatedly removing
 * the first entry.
 */
TEST(unit_twList_Add, addStrValuesToList) {
	twList *list = NULL;
	int i = 0;
	char *s = NULL;

	/* Create a list */
	list = twList_Create(&free);

	/* Add TEST_LIST_BASIC_TEST_SIZE integers */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, twItoa(i)));
	}

	/* Check that our list now has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Remove all entries from the list by repeatedly removing the first entry of the list */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		/* Get the first entry in the list */
		ListEntry *entry = twList_GetByIndex(list, i);
		/* Check its value is set as expected */
		TEST_ASSERT_NOT_NULL(entry);
		s = twItoa(i);
		TEST_ASSERT_EQUAL_STRING(s, entry->value);
		TW_FREE(s);
	}

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}