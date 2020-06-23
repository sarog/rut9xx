/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterBindEventCallback()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test bind event callback functions */
static void testBindCallbackAlpha(char * entityName, char isBound, void * userdata) { }
static void testBindCallbackBeta(char * entityName, char isBound, void * userdata) { }

/* Search parameters to locate bind event callbacks in the bind event callback list */
// TODO: Defined in two twApi.c
typedef struct twBindEventCallbackSearchParam {
	char * entityName;
	bindEvent_cb cb;
	void * userdata;
	void * found;
} twBindEventCallbackSearchParam;

/* List for each handler to locate bind event callbacks in the bind event callback list */
static int findBindEventCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	callbackInfo *info;
	twBindEventCallbackSearchParam* searchParams = (twBindEventCallbackSearchParam*)userData;
	info = (callbackInfo *) currentValue;
	//if (info->cb == searchParams->cb && info->userdata == searchParams->userdata && !strcmp(info->entityName, searchParams->entityName)) {
	if (info->cb == searchParams->cb && info->userdata == searchParams->userdata) {
		if (!info->entityName || !strcmp(info->entityName, searchParams->entityName)) {
			searchParams->found = key;
			return TW_FOREACH_EXIT;
		}
	}
	return TW_FOREACH_CONTINUE;
}

/* Test helper function to locate bind event callbacks in the bind event callback list */
static char findBindEventCallback(char * entityName, bindEvent_cb cb, void * userdata) {
	char foundCallback = FALSE;
	twBindEventCallbackSearchParam * searchParams;
	searchParams = (twBindEventCallbackSearchParam*)TW_MALLOC(sizeof(twBindEventCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->entityName = entityName;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->bindEventCallbackList, findBindEventCallbackForeachHandler, searchParams);
	if(searchParams->found) {
		foundCallback = TRUE;
	}
	TW_FREE(searchParams);
	return foundCallback;
}

TEST_GROUP(unit_twApi_RegisterBindEventCallback);

TEST_SETUP(unit_twApi_RegisterBindEventCallback) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterBindEventCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterBindEventCallback) {
	RUN_TEST_CASE(unit_twApi_RegisterBindEventCallback, registerBindCallbackWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterBindEventCallback, registerBindCallbackWithNullCallback);
	RUN_TEST_CASE(unit_twApi_RegisterBindEventCallback, registerBindCallbacksSuccess);
	RUN_TEST_CASE(unit_twApi_RegisterBindEventCallback, registerBindCallbacksWithNullEntityName);
}

/**
 * Test Plan: Try to register a bind callback with a NULL API
 */
TEST(unit_twApi_RegisterBindEventCallback, registerBindCallbackWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
}

/**
 * Test Plan: Try to register a bind callback with a NULL callback
 */
TEST(unit_twApi_RegisterBindEventCallback, registerBindCallbackWithNullCallback) {
	TEST_IGNORE_MESSAGE("CSDK-1361: twApi_RegisterBindEventCallback() should return TW_INVALID_PARAM when passed a NULL callback function pointer");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterBindEventCallback(TEST_ENTITY_NAME, NULL, NULL));
}

/**
 * Test Plan: Register and unregister some bind event callbacks
 */
TEST(unit_twApi_RegisterBindEventCallback, registerBindCallbacksSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Check that our bind event callback list is empty */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	/* Check that it is now present in our bind event callback list */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Register another callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Check that both callbacks are now present in the callback list */
	TEST_ASSERT_EQUAL(2, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Unregister the first callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Try to unregister the first callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	/* Unregister the second callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
	/* Try to unregister the second callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterBindEventCallback(TEST_ENTITY_NAME, testBindCallbackBeta, NULL));
}

/**
 * Test Plan: Register and unregister some bind event callbacks with no entity name
 */
TEST(unit_twApi_RegisterBindEventCallback, registerBindCallbacksWithNullEntityName) {
	TEST_IGNORE_MESSAGE("CSDK-1362: Registering a bind event cb with a NULL entity name segfaults when you try to unregister a callback");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* Check that our bind event callback list is empty */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Register a callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	/* Check that it is now present in our bind event callback list */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Register another callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Check that both callbacks are now present in the callback list */
	TEST_ASSERT_EQUAL(2, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_TRUE(findBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Unregister the first callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(1, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	TEST_ASSERT_TRUE(findBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Try to unregister the first callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	/* Unregister the second callback */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Check that the list has been updated accordingly */
	TEST_ASSERT_EQUAL(0, tw_api->bindEventCallbackList->count);
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackAlpha, NULL));
	TEST_ASSERT_FALSE(findBindEventCallback(NULL, testBindCallbackBeta, NULL));
	/* Try to unregister the second callback again */
	TEST_ASSERT_EQUAL(TW_NOT_FOUND, twApi_UnregisterBindEventCallback(NULL, testBindCallbackBeta, NULL));
}