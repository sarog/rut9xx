/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Foreach()
*/

#include "twApi.h"
#include "twThreads.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static char iterateThroughList_ThreadTaskFunctionRan = FALSE;
static char deleteListItem_ThreadTaskFunctionRan = FALSE;
static TW_MUTEX deleteFuncMtx = NULL;

TEST_GROUP(unit_twList_Foreach);

TEST_SETUP(unit_twList_Foreach) {
	eatLogs();
	iterateThroughList_ThreadTaskFunctionRan = FALSE;
	deleteListItem_ThreadTaskFunctionRan = FALSE;
}

TEST_TEAR_DOWN(unit_twList_Foreach) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Foreach) {
	RUN_TEST_CASE(unit_twList_Foreach, iterateNullList);
	RUN_TEST_CASE(unit_twList_Foreach, iterateEmptyList);
	RUN_TEST_CASE(unit_twList_Foreach, countFoosInList);
	RUN_TEST_CASE(unit_twList_Foreach, countFoosAndBarsInList);
	RUN_TEST_CASE(unit_twList_Foreach, countTwoBarsAndExit);
	RUN_TEST_CASE(unit_twList_Foreach, copyListWithoutBars);
	RUN_TEST_CASE(unit_twList_Foreach, removeEntryWhileIteratingThroughList);
	RUN_TEST_CASE(unit_twList_Foreach, iterateThroughListWhileRemovingEntry);
}

/**
 * Test Plan: Try to call iterate over a NULL list.
 */
TEST(unit_twList_Foreach, iterateNullList) {
	TEST_ASSERT_EQUAL(0, twList_Foreach(NULL, NULL, NULL));
}

/**
 * Test Plan: Try to iterate over an empty list.
 */
TEST(unit_twList_Foreach, iterateEmptyList) {
	twList *list = NULL;
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(0, twList_Foreach(list, NULL, NULL));
	twList_Delete(list);
}

/**
 * Test Plan: Create a list and add some strings to it, then use twList_Foreach to count the number of "foo" strings
 * in the list.
 */
static int countFoos_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	char *value = (char*)currentValue;
	int *count = (int*)arg;

	if (strcmp(value, "foo") == 0) {
		(*count)++;
	}

	return TW_FOREACH_CONTINUE;
}

TEST(unit_twList_Foreach, countFoosInList) {
	twList *list = NULL;
	int count = 0;

	/* Create a list and add some strings to it */
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foobar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));

	/* Call twList_Foreach with our for each handler and pass in the address to our count variable, expect the number
	 * of items processed to be equal to the number of items in the list */
	TEST_ASSERT_EQUAL(list->count, twList_Foreach(list, countFoos_ForeachHandler, &count));

	/* Check that our count is correct */
	TEST_ASSERT_EQUAL(3, count);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}


/**
 * Test Plan: Create a list and add some strings to it, then use twList_Foreach and a parameter structure to count the
 * number of "foo" and "bar" strings in the list.
 */
typedef struct countFoosAndBars_ForeachParams {
	int fooCount;
	int barCount;
} countFoosAndBars_ForeachParams;


static int countFoosAndBars_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	char *value = (char*)currentValue;
	countFoosAndBars_ForeachParams *params = (countFoosAndBars_ForeachParams*)arg;

	if (strcmp(value, "foo") == 0) {
		params->fooCount++;
	} else if (strcmp(value, "bar") == 0) {
		params->barCount++;
	}

	return TW_FOREACH_CONTINUE;
}

TEST(unit_twList_Foreach, countFoosAndBarsInList) {
	twList *list = NULL;
	countFoosAndBars_ForeachParams *params = NULL;

	/* Create a list and add some strings to it */
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foobar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));

	/* Allocate our params structure and initialize our counts to 0 */
	params = TW_MALLOC(sizeof(countFoosAndBars_ForeachParams));
	params->fooCount = 0;
	params->barCount = 0;

	/* Call twList_Foreach with our for each handler and pass in the pointer to our params structure, expect the number
	 * of items processed to be equal to the number of items in the list */
	TEST_ASSERT_EQUAL(list->count, twList_Foreach(list, countFoosAndBars_ForeachHandler, params));

	/* Check that our counts are correct */
	TEST_ASSERT_EQUAL(3, params->fooCount);
	TEST_ASSERT_EQUAL(2, params->barCount);

	/* Clean up */
	TW_FREE(params);
	twList_Delete(list);
}

/**
 * Test Plan: Create a list and add some strings to it, then use twList_Foreach to count the number of "foo" strings
 * in the list.
 */
static int countTwoBarsAndExit_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	char *value = (char*)currentValue;
	int *count = (int*)arg;

	if (strcmp(value, "bar") == 0) {
		(*count)++;
		if (*count == 2) {
			return TW_FOREACH_EXIT;
		}
	}

	return TW_FOREACH_CONTINUE;
}

TEST(unit_twList_Foreach, countTwoBarsAndExit) {
	twList *list = NULL;
	int count = 0;

	/* Create a list and add some strings to it */
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foobar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));

	/* Call twList_Foreach with our for each handler and pass in the address to our count variable, expect the number
	 * of items processed to be equal to 4 because the second "bar" is the fourth item in the list */
	TEST_ASSERT_EQUAL(4, twList_Foreach(list, countTwoBarsAndExit_ForeachHandler, &count));

	/* Check that our count is correct */
	TEST_ASSERT_EQUAL(2, count);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}

/**
 * Test Plan: Create a list and add some strings to it, then use twList_Foreach and a parameter structure create a new
 * list without any "bar strings", finally use two more calls to twList_Foreach to count the number of "bar" strings in
 * each list.
 */
static int copyListWithoutBars_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	char *value = (char*)currentValue;
	twList *newList = (twList*)arg;

	if (strcmp(value, "bar") != 0) {
		twList_Add(newList, value);
	}

	return TW_FOREACH_CONTINUE;
}

static int countBars_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	char *value = (char*)currentValue;
	int *count = (int*)arg;

	if (strcmp(value, "bar") == 0) {
		(*count)++;
	}

	return TW_FOREACH_CONTINUE;
}

TEST(unit_twList_Foreach, copyListWithoutBars) {
	twList *list = NULL;
	twList *newList = NULL;
	int oldListBarCount = 0;
	int newListBarCount = 0;

	/* Create a list and add some strings to it */
	list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "bar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foobar"));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, "foo"));

	/* Create a second list which will be a copy of the first list without any "bar" strings in it */
	newList = twList_Create(doNothing);

	/* Call twList_Foreach with our for each handler and pass in the pointer to our new list, expect the number
	 * of items processed to be equal to the number of items in the list */
	TEST_ASSERT_EQUAL(list->count, twList_Foreach(list, copyListWithoutBars_ForeachHandler, newList));

	/* Make another two calls to twList_Foreach with a handler to count the number of "bar" strings in each list */
	TEST_ASSERT_EQUAL(list->count, twList_Foreach(list, countBars_ForeachHandler, &oldListBarCount));
	TEST_ASSERT_EQUAL(newList->count, twList_Foreach(newList, countBars_ForeachHandler, &newListBarCount));

	/* Check that our counts are correct */
	TEST_ASSERT_EQUAL(2, oldListBarCount);
	TEST_ASSERT_EQUAL(0, newListBarCount);

	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(newList));
}

/* Params struct for the following two tests */
typedef struct listLockingTestParams {
	twList *list;
	TW_MUTEX mtx;
	int iterations;
	int deleteRes;
} listLockingTestParams;

/**
 * Test Plan: Lock an auxiliary mutex and create a thread which will wait to acquire the auxiliary mutex and then call
 * twList_Remove() while the main thread is still iterating through the list with twList_Foreach.
 */
static int iterateThroughListAndReleaseLock_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	int *value = (int)currentValue;
	listLockingTestParams *params = (listLockingTestParams*)arg;

	if (value == 1) {
		/* Unlock the auxiliary mutex to allow the deletion thread to call twList_Remove() */
		twMutex_Unlock(params->mtx);
		/* Sleep briefly to allow twList_Remove() to wait on the list's mutex held by twList_Foreach() in the current thread */
		twSleepMsec(50);
	}

	return TW_FOREACH_CONTINUE;
}

static void deleteListItem_ThreadTaskFunction(DATETIME now, void *arg) {
	listLockingTestParams *params = (listLockingTestParams*)arg;
	ListEntry *entry = NULL;

	/* Check and set a flag so that this code only runs once */
	if (FALSE == deleteListItem_ThreadTaskFunctionRan) {
		deleteListItem_ThreadTaskFunctionRan = TRUE;
		/* Wait for the main thread to release the auxiliary mutex */
		entry = twList_GetByIndex(params->list, 3);
		twMutex_Lock(params->mtx);
		/* Get a list entry and try to remove it while twList_Foreach() still holds the list's mutex */
		/* twList_Remove() should wait for twList_Foreach() to release the list's mutex before removing the item */
		params->deleteRes = twList_Remove(params->list, entry, TRUE);
	}
}

TEST(unit_twList_Foreach, removeEntryWhileIteratingThroughList) {
	listLockingTestParams *params = NULL;
	int originalCount;
	int waitTime = 5000;
	int waitedTime = 0;
	twThread *deleteThread = NULL;

	/* Allocate a struct to keep track of the list, an auxiliary mutex, and function return codes */
	params = TW_MALLOC(sizeof(listLockingTestParams));

	/* Create an auxiliary mutex that will prevent the deletion thread from calling twList_Remove() until our Foreach
	 * handler releases it mid-operation */
	params->mtx = twMutex_Create();
	/* Set return codes to -1 so that we know when functions return */
	params->deleteRes = -1;
	params->iterations = -1;
	/* Create a list and add some items to it */
	params->list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 0));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 1));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 2));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 3));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 4));
	/* Record the original list count */
	originalCount = params->list->count;

	/* Acquire the auxiliary mutex in the main thread */
	twMutex_Lock(params->mtx);

	/* Create a new thread which will wait for our main thread to release the auxiliary mutex and then attempt to
	 * remove an item from the list with twList_Remove() while our Foreach handler is still iterating through the list */
	deleteThread = twThread_Create(deleteListItem_ThreadTaskFunction, 5, params, TRUE);
	/* Sleep for 50ms to allow the new thread to get a pointer to a ListEntry and wait on the auxiliary mutex */
	twSleepMsec(50);

	/* Call twList_Foreach() which will acquire the list's mutex and then release the auxiliary mutex inside the
	 * Foreach handler */
	params->iterations = twList_Foreach(params->list, iterateThroughListAndReleaseLock_ForeachHandler, params);

	/* Wait for twList_Remove() to return or time out */
	while (-1 == params->deleteRes && waitedTime < waitTime) {
		twSleepMsec(100);
		waitedTime += 100;
	}

	/* Assert that our remove operation returned successfully */
	TEST_ASSERT_EQUAL(TW_OK, params->deleteRes);
	/* Assert that the list is now one item smaller */
	TEST_ASSERT_EQUAL(originalCount-1, params->list->count);
	/* Assert that the deletion thread iterated waited for twList_Foeach() to iterate through the entire original list */
	TEST_ASSERT_EQUAL(originalCount, params->iterations);

	/* Clean up */
	twThread_Delete(deleteThread);
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(params->list));
	twMutex_Delete(params->mtx);
	TW_FREE(params);
}

/**
 * Test Plan: Lock an auxiliary mutex and create a thread which will wait to acquire the auxiliary mutex and then call
 * twList_Foreach() while the main thread is still removing an item with twList_remove().
 */
static void deleteListItem_deleteFunc(void *item) {
	/* Unlock the auxiliary mutex to allow the iteration thread to call twList_Foreach() */
	/* Using a static global here because delete functions don't get userdata */
	twMutex_Unlock(deleteFuncMtx);
	/* Sleep briefly to allow twList_Foreach() to wait on the list's mutex held by twList_Remove() in the current thread */
	twSleepMsec(50);
}

static int iterateThroughList_ForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *arg) {
	int *value = (int)currentValue;
	listLockingTestParams *params = (listLockingTestParams*)arg;
	/* Do nothing */
	return TW_FOREACH_CONTINUE;
}

static void acquireLockAndIterateThroughList_ThreadTaskFunction(DATETIME now, void *arg) {
	listLockingTestParams *params = (listLockingTestParams*)arg;
	/* Check and set a flag so that this code only runs once */
	if (FALSE == iterateThroughList_ThreadTaskFunctionRan) {
		iterateThroughList_ThreadTaskFunctionRan = TRUE;
		/* Wait for the main thread to release the auxiliary mutex */
		twMutex_Lock(params->mtx);
		/* Attempt to iterate over the list while twList_Remove() still holds the list's mutex */
		/* twList_Foreach() should wait for twList_Remove() to release the list's mutex before iterating over the list */
		params->iterations = twList_Foreach(params->list, iterateThroughList_ForeachHandler, NULL);
	}
}


TEST(unit_twList_Foreach, iterateThroughListWhileRemovingEntry) {
	listLockingTestParams *params = NULL;
	int originalCount;
	int waitTime = 5000;
	int waitedTime = 0;
	twThread *iterateThread = NULL;
	ListEntry *entry = NULL;

	/* Allocate a struct to keep track of the list, an auxiliary mutex, and function return codes */
	params = TW_MALLOC(sizeof(listLockingTestParams));

	/* Create an auxiliary mutex that will prevent the iteration thread from calling twList_Foreach() until our delete
	 * function releases it mid-operation */
	params->mtx = twMutex_Create();
	/* Set a static global pointer because delete functions don't get userdata */
	deleteFuncMtx = params->mtx;
	/* Set return codes to -1 so that we know when functions return */
	params->deleteRes = -1;
	params->iterations = -1;
	/* Create a list and add some items to it */
	params->list = twList_Create(deleteListItem_deleteFunc);
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 0));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 1));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 2));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 3));
	TEST_ASSERT_EQUAL(TW_OK, twList_Add(params->list, 4));
	/* Record the original list count */
	originalCount = params->list->count;

	/* Acquire the auxiliary mutex in the main thread */
	twMutex_Lock(params->mtx);

	/* Create a new thread which will wait for our main thread to release the auxiliary mutex and then attempt to
	 * iterate through the list with twList_Foreach() while our delete function is still executing */
	iterateThread = twThread_Create(acquireLockAndIterateThroughList_ThreadTaskFunction, 5, params, TRUE);
	/* Sleep for 50ms to allow the new thread to wait on the auxiliary mutex */
	twSleepMsec(50);

	/* Get a pointer to the third list entry */
	entry = twList_GetByIndex(params->list, 3);
	/* Call twList_Remove() which will acquire the list's mutex and then release the auxiliary mutex inside the
	 * delete function */
	params->deleteRes = twList_Remove(params->list, entry, TRUE);

	/* Wait for twList_Foreach() to return or time out */
	while (-1 == params->iterations && waitedTime < waitTime) {
		twSleepMsec(100);
		waitedTime += 100;
	}

	/* Assert that our remove operation returned successfully */
	TEST_ASSERT_EQUAL(TW_OK, params->deleteRes);
	/* Assert that the list is now one item smaller */
	TEST_ASSERT_EQUAL(originalCount-1, params->list->count);
	/* Assert that the iteration thread iterated waited for twList_Remove() to return and then iterated through the
	 * list after an item had been removed */
	TEST_ASSERT_EQUAL(originalCount-1, params->iterations);

	/* Clean up */
	twThread_Delete(iterateThread);
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(params->list));
	twMutex_Delete(params->mtx);
	TW_FREE(params);
}