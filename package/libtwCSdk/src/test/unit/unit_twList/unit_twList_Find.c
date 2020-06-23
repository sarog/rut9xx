/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Find()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_Find);

TEST_SETUP(unit_twList_Find) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_Find) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Find) {
	RUN_TEST_CASE(unit_twList_Find, searchListForNonExistentStrEntry);
	RUN_TEST_CASE(unit_twList_Find, searchListForStrEntryWithNullParseFunction);
	RUN_TEST_CASE(unit_twList_Find, searchListForStringEntry);
	RUN_TEST_CASE(unit_twList_Find, searchListForDynamicStringEntry);
}

static const char *parseInt(void *item){
	int i = (int*)item;
	return twItoa(i);
}

static const char *parseString(void *item){
	char *s = (char*)item;
	return duplicateString(s);
}

static void deleteString(void *item){
	char *s = (char*)item;
	TW_FREE(s);
}

/**
 * Test Plan: Create a searchable list, add some strings to the list, and then search for a string that does not
 * exist in the list.
 */
TEST(unit_twList_Find, searchListForNonExistentStrEntry) {
	twList *list = NULL;
	ListEntry *result = NULL;

	/* Create a searchable list with a delete function that does nothing (because the values we're adding are
	 * statically allocated) and a parse function that parses string values */
	list = twList_CreateSearchable(doNothing, parseString);

	/* Add some stuff to the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "baz"));

	/* Search the list for the value "foobar", which is not present in the list */
	TEST_ASSERT_EQUAL(TW_MAP_MISSING, twList_Find(list, "foobar", &result));

	/* Check that result is still NULL */
	TEST_ASSERT_NULL(result);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a searchable list with a NULL parse function, add some strings to the list, and then search for
 * a string in the list.  Expect TW_MAP_MISSING because the list has no parse function.
 */
TEST(unit_twList_Find, searchListForStrEntryWithNullParseFunction) {
	twList *list = NULL;
	ListEntry *result = NULL;

	/* Create a searchable list with a delete function that does nothing (because the values we're adding are
	 * statically allocated) and a NULL parse function */
	list = twList_CreateSearchable(doNothing, NULL);

	/* Add some stuff to the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "baz"));

	/* Search the list for the value "bar" */
	TEST_ASSERT_EQUAL(TW_MAP_MISSING, twList_Find(list, "bar", &result));

	/* Check that result is still NULL */
	TEST_ASSERT_NULL(result);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a searchable list, add some string literals to the list, and then search for one of the strings.
 */
TEST(unit_twList_Find, searchListForStringEntry) {
	twList *list = NULL;
	ListEntry *result = NULL;

	/* Create a searchable list with a delete function that does nothing (because the values we're adding are
	 * statically allocated) and a parse function that parses string values */
	list = twList_CreateSearchable(doNothing, parseString);

	/* Add some stuff to the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "baz"));

	/* Search the list for the value "bar" */
	TEST_ASSERT_EQUAL(TW_OK, twList_Find(list, "bar", (void**)&result));

	/* Check that result now points to the proper entry */
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL_STRING("bar", result->value);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a searchable list, add some dynamic strings to the list, and then search for one of the strings.
 */
TEST(unit_twList_Find, searchListForDynamicStringEntry) {
	twList *list = NULL;
	ListEntry *result = NULL;

	/* Create a searchable list with a delete function that free's string values and a parse function that parses
	 * string values */
	list = twList_CreateSearchable(deleteString, parseString);

	/* Add some stuff to the list */
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, duplicateString("foo")));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, duplicateString("bar")));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, duplicateString("baz")));

	/* Search the list for the value "bar" */
	TEST_ASSERT_EQUAL(TW_OK, twList_Find(list, "bar", (void**)&result));

	/* Check that result now points to the proper entry */
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL_STRING("bar", result->value);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}