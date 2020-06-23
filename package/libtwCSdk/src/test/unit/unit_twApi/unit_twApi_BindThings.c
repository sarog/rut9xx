/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_BindThings()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_BindThings);

TEST_SETUP(unit_twApi_BindThings) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_BindThings) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_BindThings) {
	RUN_TEST_CASE(unit_twApi_BindThings, bindThingWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_BindThings, bindThingWithNullName);
	RUN_TEST_CASE(unit_twApi_BindThings, bindAndUnbindThing);
	RUN_TEST_CASE(unit_twApi_BindThings, bindAndUnbindMultipleThingsMultipleTimes);
}

/**
 * Test Plan: Bind things without initializing the API
 */
TEST(unit_twApi_BindThings, bindThingWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_BindThings(TEST_ENTITY_NAME));
}

/**
 * Test Plan: Attempt to bind things with a NULL things list
 */
TEST(unit_twApi_BindThings, bindThingWithNullName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_BindThings(NULL));
}

/**
 * Test Plan: Bind and unbind a list of one thing
 */
TEST(unit_twApi_BindThings, bindAndUnbindThing) {
	twList * list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	/* Bind a thing */
	twList_Add(list, TEST_ENTITY_NAME);
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(list));
	/* Check it is bound */
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	/* Unbind the thing */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_ENTITY_NAME));
	/* Check it is unbound */
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}

/**
 * Test Plan: Bind and unbind a list of multiple things twice
 */
TEST(unit_twApi_BindThings, bindAndUnbindMultipleThingsMultipleTimes) {
	twList * list = twList_Create(doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Bind a few things */
	twList_Add(list, TEST_THING_NAME_1);
	twList_Add(list, TEST_THING_NAME_2);
	twList_Add(list, TEST_THING_NAME_3);
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(list));
	/* Check that they're bound */
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Try to bind them again */
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_BindThings(list));
	/* Check that they're still bound */
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Unbind things */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_1));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_2));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_3));
	/* Check that they're unbound */
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Unbind things again */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_1));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_2));
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_THING_NAME_3));
	/* Check that they're still unbound */
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_3));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}