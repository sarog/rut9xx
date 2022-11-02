/*
 * Copyright 2016, PTC, Inc.
 */
#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE

#include "twApi.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twInfoTable_Create);

TEST_SETUP(unit_twInfoTable_Create){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twInfoTable_Create){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twInfoTable_Create){
	RUN_TEST_CASE(unit_twInfoTable_Create, test_twInfoTable_CreateRowFromEntries);
}

/**
 * Test Plan: Create an InfoTable row from a set of primitives.
 */
TEST(unit_twInfoTable_Create,test_twInfoTable_CreateRowFromEntries){
	twPrimitive* col1;
	twPrimitive* col2;
	twPrimitive* col3;
	twInfoTableRow* row = twInfoTable_CreateRowFromEntries(TW_MAKE_NUMBER(1),TW_MAKE_NUMBER(2),TW_MAKE_STRING("THREE"),NULL);
	TEST_ASSERT_NOT_NULL(row);
	TEST_ASSERT_EQUAL(3,row->numFields);
	col1 = (twPrimitive*)row->fieldEntries->first->value;
	col2 = (twPrimitive*)row->fieldEntries->first->next->value;
	col3 = (twPrimitive*)row->fieldEntries->first->next->next->value;
	TEST_ASSERT_NOT_NULL(col1);
	TEST_ASSERT_EQUAL(TW_NUMBER,col1->type);
	TEST_ASSERT_TRUE(1==col1->val.number);
	TEST_ASSERT_NOT_NULL(col2);
	TEST_ASSERT_EQUAL(TW_NUMBER,col2->type);
	TEST_ASSERT_TRUE(2==col2->val.number);
	TEST_ASSERT_NOT_NULL(col3);
	TEST_ASSERT_EQUAL(TW_STRING,col3->type);
	TEST_ASSERT_EQUAL_STRING("THREE",col3->val.bytes.data);
}