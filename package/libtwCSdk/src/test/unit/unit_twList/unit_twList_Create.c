/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twList_Create() and twList_Delete()
*/

#include "twApi.h"
#include "twApiStubs.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twList_Create);

TEST_SETUP(unit_twList_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twList_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twList_Create) {
	RUN_TEST_CASE(unit_twList_Create, deleteNullList);
	RUN_TEST_CASE(unit_twList_Create, createAndDeleteList);
}

/**
 * Test Plan: Attempts to delete a list before being created.
 */
TEST(unit_twList_Create, deleteNullList) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twList_Delete(NULL));
}

/**
 * Test Plan: Creates a list and deletes it and verifies deletion.
 */
TEST(unit_twList_Create, createAndDeleteList) {
	twList *list = twList_Create(&doNothing);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
}