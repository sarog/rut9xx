/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterOnAuthenticatedCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test authenticated event callback functions */
static void testAuthenticatedCallbackAlpha(char * entityName, char *isBound, void * userdata) { }
static void testAuthenticatedCallbackBeta(char * entityName, char *isBound, void * userdata) { }

/* Search parameters to locate authenticated event callbacks in the bind event callback list */
typedef struct twAuthenticatedCallbackSearchParam {
	authEvent_cb cb;
	void * userdata;
	void * found;
} twAuthenticatedCallbackSearchParam;

/* List for each handler to locate authenticated event callbacks in the bind event callback list */
static int findAuthenticatedCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	callbackInfo *info;
	twAuthenticatedCallbackSearchParam* searchParams = (twAuthenticatedCallbackSearchParam*)userData;
	info = (callbackInfo *) currentValue;
	if (info->cb == searchParams->cb && info->userdata == searchParams->userdata && info->entityType == TW_APPLICATIONKEYS) {
		searchParams->found = key;
		return TW_FOREACH_EXIT;
	}
	return TW_FOREACH_CONTINUE;
}

/* Test helper function to locate authenticated event callbacks in the bind event callback list */
static char findAuthenticatedCallback(authEvent_cb cb, void * userdata) {
	char foundCallback = FALSE;
	twAuthenticatedCallbackSearchParam * searchParams;
	searchParams = (twAuthenticatedCallbackSearchParam*)TW_MALLOC(sizeof(twAuthenticatedCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->bindEventCallbackList, findAuthenticatedCallbackForeachHandler, searchParams);
	if(searchParams->found) {
		foundCallback = TRUE;
	}
	TW_FREE(searchParams);
	return foundCallback;
}

TEST_GROUP(unit_twApi_RegisterOnAuthenticatedCallback);

TEST_SETUP(unit_twApi_RegisterOnAuthenticatedCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterOnAuthenticatedCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterOnAuthenticatedCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbacks);
}

/**
 * Test Plan: Try to register a authenticated callback with a NULL API
 */
TEST(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterOnAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
}

/**
 * Test Plan: Try to register a authenticated callback with a NULL callback
 */
TEST(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbackWithNullCallback) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_RegisterOnAuthenticatedCallback() should return TW_INVALID_PARAM when passed a NULL callback function pointer");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterOnAuthenticatedCallback(NULL, NULL));
}

/**
 * Test Plan: Register and unregister some authenticated callbacks
 */
TEST(unit_twApi_RegisterOnAuthenticatedCallback, registerAuthenticatedCallbacks) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Check that our bind event callback list is empty */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterOnAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	/* Check that it is now present in our bind event callback list */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Register another callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterOnAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Check that both callbacks are now present in the callback list */
	TEST_ASSERT_EQUAL(2, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Unregister the first callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterOnAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Try to unregister the first callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterOnAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	/* Unregister the second callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterOnAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
	/* Try to unregister the second callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterOnAuthenticatedCallback(&testAuthenticatedCallbackBeta, NULL));
}