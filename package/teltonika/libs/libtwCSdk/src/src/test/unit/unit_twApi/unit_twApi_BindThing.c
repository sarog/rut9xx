/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_BindThing()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_BindThing);

TEST_SETUP(unit_twApi_BindThing) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_BindThing) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_BindThing) {
	RUN_TEST_CASE(unit_twApi_BindThing, bindThingWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_BindThing, bindThingWithNullName);
	RUN_TEST_CASE(unit_twApi_BindThing, bindAndUnbindThing);
	RUN_TEST_CASE(unit_twApi_BindThing, bindAndUnbindMultipleThingsMultipleTimes);
}

/**
 * Test Plan: Bind a thing without initializing the API
 */
TEST(unit_twApi_BindThing, bindThingWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_BindThing(TEST_ENTITY_NAME));
}

/**
 * Test Plan: Attempt to bind a thing with a NULL thing name
 */
TEST(unit_twApi_BindThing, bindThingWithNullName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_BindThing(NULL));
}

/**
 * Test Plan: Bind and unbind a thing
 */
TEST(unit_twApi_BindThing, bindAndUnbindThing) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	/* Bind a thing */
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing(TEST_ENTITY_NAME));
	/* Check it is bound */
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	/* Unbind the thing */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(TEST_ENTITY_NAME));
	/* Check it is unbound */
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_ENTITY_NAME));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
}

/**
 * Test Plan: Bind and unbind a few things twice
 */
TEST(unit_twApi_BindThing, bindAndUnbindMultipleThingsMultipleTimes) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	twApi_stub->sendMessageBlocking = stub_sendMessageBlocking_MockSuccess;
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Bind a few things */
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing(TEST_THING_NAME_1));
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing(TEST_THING_NAME_2));
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing(TEST_THING_NAME_3));
	/* Check that they're bound */
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_1));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_2));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(TEST_THING_NAME_3));
	/* Try to bind them again */
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_BindThing(TEST_THING_NAME_1));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_BindThing(TEST_THING_NAME_2));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_BindThing(TEST_THING_NAME_3));
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