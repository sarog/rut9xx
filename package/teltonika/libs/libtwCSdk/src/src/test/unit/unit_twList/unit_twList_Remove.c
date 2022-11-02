/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Remove()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_Remove);

TEST_SETUP(unit_twList_Remove) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_Remove) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Remove) {
	RUN_TEST_CASE(unit_twList_Remove, removeValueWithNullEntryParameter);
	RUN_TEST_CASE(unit_twList_Remove, removeValueWithNullListParameter);
	RUN_TEST_CASE(unit_twList_Remove, removeStrListEntriesFirstToLast);
	RUN_TEST_CASE(unit_twList_Remove, removeStrListEntriesLastToFirst);
}

/**
 * Test Plan: Try to remove a value from a list with NULL entry parameter, expect TW_INVALID_PARAM return status.
 */
TEST(unit_twList_Remove, removeValueWithNullEntryParameter) {
	twList *list = NULL;

	/* Create a list and add something to it so that we can get a valid list to try to remove from */
	list = twList_Create(&doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, 0));
	/* Try to remove a NULL ListEntry from a valid twList */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_Remove(list, NULL, FALSE));
	/* Clean up the list we created */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Try to remove a value from a list with NULL list parameter, expect TW_INVALID_PARAM return status.
 */
TEST(unit_twList_Remove, removeValueWithNullListParameter) {
	twList *list = NULL;
	ListEntry *entry = NULL;

	/* Create a list and add something to it so that we can get a valid ListEntry to remove */
	list = twList_Create(&doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, 0));
	entry = twList_GetByIndex(list, 0);
	/* Try to remove a valid ListEntry from a NULL twList */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_Remove(NULL, entry, FALSE));
	/* Clean up the list we created */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list, add TEST_LIST_BASIC_TEST_SIZE strings, and then remove each one by repeatedly removing
 * the first entry.
 */
TEST(unit_twList_Remove, removeStrListEntriesFirstToLast) {
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
		ListEntry *entry = twList_GetByIndex(list, 0);
		/* Check its value is set as expected */
		TEST_ASSERT_NOT_NULL(entry);
		s = twItoa(i);
		TEST_ASSERT_EQUAL_STRING(s, entry->value);
		TW_FREE(s);
		/* Remove the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_Remove(list, entry, TRUE));
	}

	/* Check that the list now has 0 entries */
	TEST_ASSERT_EQUAL(0, twList_GetCount(list));

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list, add TEST_LIST_BASIC_TEST_SIZE strings, and then remove each one by repeatedly removing
 * the last entry.
 */
TEST(unit_twList_Remove, removeStrListEntriesLastToFirst) {
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

	/* Remove all entries from the list by repeatedly removing the last entry of the list */
	for (i = TEST_LIST_BASIC_TEST_SIZE - 1; i >= 0; i--) {
		/* Get the first entry in the list */
		ListEntry *entry = twList_GetByIndex(list, i);
		/* Check its value is set as expected */
		TEST_ASSERT_NOT_NULL(entry);
		s = twItoa(i);
		TEST_ASSERT_EQUAL_STRING(s, entry->value);
		TW_FREE(s);
		/* Remove the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_Remove(list, entry, TRUE));
	}

	/* Check that the list now has 0 entries */
	TEST_ASSERT_EQUAL(0, twList_GetCount(list));

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}