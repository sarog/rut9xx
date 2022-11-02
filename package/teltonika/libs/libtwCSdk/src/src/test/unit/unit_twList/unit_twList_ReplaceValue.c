/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_ReplaceValue()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_ReplaceValue);

TEST_SETUP(unit_twList_ReplaceValue) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_ReplaceValue) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_ReplaceValue) {
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceValueWithNullEntryParameter);
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceValueWithNullListParameter);
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceIntListEntriesFirstToLast);
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceIntListEntriesLastToFirst);
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceStrListEntriesFirstToLast);
	RUN_TEST_CASE(unit_twList_ReplaceValue, replaceStrListEntriesLastToFirst);
}

/**
 * Test Plan: Try to replace a value in a list with NULL entry parameter, expect TW_INVALID_PARAM return status
 */
TEST(unit_twList_ReplaceValue, replaceValueWithNullEntryParameter) {
	twList *list = NULL;

	/* Create a list and add something to it so that we can get a valid list to try to replace a value in */
	list = twList_Create(&doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, 0));
	/* Try to replace a NULL entry in a valid twList */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_ReplaceValue(list, NULL, 0, FALSE));
	/* Clean up the list we created */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Try to replace a value in a list with NULL list parameter, expect TW_INVALID_PARAM return status.
 */
TEST(unit_twList_ReplaceValue, replaceValueWithNullListParameter) {
	twList *list = NULL;
	ListEntry *entry = NULL;

	/* Create a list and add something to it so that we can get a valid ListEntry to replace */
	list = twList_Create(&doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, 0));
	entry = twList_GetByIndex(list, 0);
	/* Try to replace a valid ListEntry from a NULL twList */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_ReplaceValue(NULL, entry, 0, FALSE));
	/* Clean up the list we created */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list add TEST_LIST_BASIC_TEST_SIZE integers, and then replace all the list entry values with
 * -1 starting with the first entry.
*/
TEST(unit_twList_ReplaceValue, replaceIntListEntriesFirstToLast) {
	twList *list = NULL;
	ListEntry *entry = NULL;
	int i = 0;

	/* Create a list */
	list = twList_Create(&doNothing);

	/* Add TEST_LIST_BASIC_TEST_SIZE integers */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, i));
	}

	/* Check that our list now has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Replace all list entry values with -1 */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		/* Get the entry and check that its value is what we expect */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(i, entry->value);
		/* Replace the value of the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_ReplaceValue(list, entry, -1, FALSE));
		/* Get the entry again and check that the value has changed */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(-1, entry->value);
	}

	/* Check that our list still has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Iterate through the entire list and check that all entry values are -1 */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(-1, entry->value);
	}

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list add TEST_LIST_BASIC_TEST_SIZE integers, and then replace all the list entry values with
 * -1 starting with the last entry.
*/
TEST(unit_twList_ReplaceValue, replaceIntListEntriesLastToFirst) {
	twList *list = NULL;
	ListEntry *entry = NULL;
	int i = 0;

	/* Create a list */
	list = twList_Create(&doNothing);

	/* Add TEST_LIST_BASIC_TEST_SIZE integers */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, i));
	}

	/* Check that our list now has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Replace all list entry values with -1 */
	for (i = TEST_LIST_BASIC_TEST_SIZE - 1; i >= 0; i--) {
		/* Get the entry and check that its value is what we expect */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(i, entry->value);
		/* Replace the value of the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_ReplaceValue(list, entry, -1, FALSE));
		/* Get the entry again and check that the value has changed */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(-1, entry->value);
	}

	/* Check that our list still has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Iterate through the entire list and check that all entry values are -1 */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL(-1, entry->value);
	}

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list add TEST_LIST_BASIC_TEST_SIZE strings, and then replace all the list entry values with
 * "foo" starting with the first entry.
*/
TEST(unit_twList_ReplaceValue, replaceStrListEntriesFirstToLast) {
	twList *list = NULL;
	ListEntry *entry = NULL;
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

	/* Replace all list entry values with "foo" */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		/* Get the entry and check that its value is what we expect */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		s = twItoa(i);
		TEST_ASSERT_EQUAL_STRING(s, entry->value);
		TW_FREE(s);
		/* Replace the value of the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_ReplaceValue(list, entry, duplicateString("foo"), TRUE));
		/* Get the entry again and check that the value has changed */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL_STRING("foo", entry->value);
	}

	/* Check that our list still has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Iterate through the entire list and check that all entry values are "foo" */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL_STRING("foo", entry->value);
	}

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list add TEST_LIST_BASIC_TEST_SIZE strings, and then replace all the list entry values with
 * "foo" starting with the last entry.
*/
TEST(unit_twList_ReplaceValue, replaceStrListEntriesLastToFirst) {
	twList *list = NULL;
	ListEntry *entry = NULL;
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

	/* Replace all list entry values with "foo" */
	for (i = TEST_LIST_BASIC_TEST_SIZE - 1; i >= 0; i--) {
		/* Get the entry and check that its value is what we expect */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		s = twItoa(i);
		TEST_ASSERT_EQUAL_STRING(s, entry->value);
		TW_FREE(s);
		/* Replace the value of the entry */
		TEST_ASSERT_EQUAL(TW_OK, twList_ReplaceValue(list, entry, duplicateString("foo"), TRUE));
		/* Get the entry again and check that the value has changed */
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL_STRING("foo", entry->value);
	}

	/* Check that our list still has TEST_LIST_BASIC_TEST_SIZE entries */
	TEST_ASSERT_EQUAL(TEST_LIST_BASIC_TEST_SIZE, twList_GetCount(list));

	/* Iterate through the entire list and check that all entry values are "foo" */
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		entry = twList_GetByIndex(list, i);
		TEST_ASSERT_NOT_NULL(entry);
		TEST_ASSERT_EQUAL_STRING("foo", entry->value);
	}

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}