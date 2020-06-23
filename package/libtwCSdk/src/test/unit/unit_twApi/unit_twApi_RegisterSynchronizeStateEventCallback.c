/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterSynchronizeStateEventCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test synchronize event callback functions */
static void testSynchronizeCallbackAlpha(char * entityName, twInfoTable* subscriptionInfo, void * userdata) {;}
static void testSynchronizeCallbackBeta(char * entityName, twInfoTable* subscriptionInfo, void * userdata) {;}

/* Search parameters to locate synchronize event callbacks in the bind event callback list */
typedef struct twSynchronizeEventCallbackSearchParam {
	char * entityName;
	synchronizeEvent_cb cb;
	void * userdata;
	void * found;
} twSynchronizeEventCallbackSearchParam;

/* List for each handler to locate synchronize event callbacks in the bind event callback list */
static int findSynchronizeEventCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	callbackInfo *info;
	twSynchronizeEventCallbackSearchParam* searchParams = (twSynchronizeEventCallbackSearchParam*)userData;
	info = (callbackInfo *) currentValue;
	if (info->cb == searchParams->cb && info->userdata == searchParams->userdata && !strcmp(info->entityName, searchParams->entityName)) {
		searchParams->found = key;
		return TW_FOREACH_EXIT;
	}
	return TW_FOREACH_CONTINUE;
}

/* Test helper function to locate synchronize event callbacks in the bind event callback list */
static char findSynchronizeEventCallback(char * entityName, synchronizeEvent_cb cb, void * userdata) {
	char foundCallback = FALSE;
	twSynchronizeEventCallbackSearchParam * searchParams;
	searchParams = (twSynchronizeEventCallbackSearchParam*)TW_MALLOC(sizeof(twSynchronizeEventCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->entityName = entityName;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->synchronizeStateEventCallbackList, findSynchronizeEventCallbackForeachHandler, searchParams);
	if(searchParams->found) {
		foundCallback = TRUE;
	}
	TW_FREE(searchParams);
	return foundCallback;
}

TEST_GROUP(unit_twApi_RegisterSynchronizeStateEventCallback);

TEST_SETUP(unit_twApi_RegisterSynchronizeStateEventCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterSynchronizeStateEventCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterSynchronizeStateEventCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterSynchronizeStateEventCallback, RegisterSynchronizeCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterSynchronizeStateEventCallback, RegisterSynchronizeCallbacks);
}

/**
 * Test Plan: Try to register a synchronize callback with a NULL API
 */
TEST(unit_twApi_RegisterSynchronizeStateEventCallback, RegisterSynchronizeCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
}

/**
 * Test Plan: Register and unregister some synchronize callbacks
 */
TEST(unit_twApi_RegisterSynchronizeStateEventCallback, RegisterSynchronizeCallbacks) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Check that our bind event callback list is empty */
	TEST_ASSERT_EQUAL(0, tw_api->synchronizeStateEventCallbackList->count);
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	/* Check that it is now present in our bind event callback list */
	TEST_ASSERT_EQUAL(1, tw_api->synchronizeStateEventCallbackList->count);
	TEST_ASSERT_TRUE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Register another callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Check that both callbacks are now present in the callback list */
	TEST_ASSERT_EQUAL(2, tw_api->synchronizeStateEventCallbackList->count);
	TEST_ASSERT_TRUE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Unregister the first callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(1, tw_api->synchronizeStateEventCallbackList->count);
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Try to unregister the first callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	/* Unregister the second callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(0, tw_api->synchronizeStateEventCallbackList->count);
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findSynchronizeEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
	/* Try to unregister the second callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterSynchronizeStateEventCallback(TEST_ENTITY_NAME, testSynchronizeCallbackBeta, NULL));
}