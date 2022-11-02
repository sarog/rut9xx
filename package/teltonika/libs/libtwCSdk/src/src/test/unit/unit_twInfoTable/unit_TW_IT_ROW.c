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

TEST_GROUP(unit_TW_IT_ROW);

TEST_SETUP(unit_TW_IT_ROW){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_TW_IT_ROW){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_TW_IT_ROW){
	RUN_TEST_CASE(unit_TW_IT_ROW, test_TW_IT_ROW_macro);
}


/**
 * Test Plan: Specifically tests the TW_IT_ROW() macro. Confirm that it creates an unit_TW_IT_ROW row
 * by reading back the values.
 */
TEST(unit_TW_IT_ROW,test_TW_IT_ROW_macro){
	twInfoTableRow * aRow = NULL;
	twPrimitive * col1 = NULL;
	twPrimitive * col2 = NULL;
	twPrimitive * col3 = NULL;
	aRow = TW_IT_ROW(
			TW_MAKE_NUMBER(1),
			TW_MAKE_BOOL(TRUE),
			TW_MAKE_STRING("Hello World")
	);
	TEST_ASSERT_EQUAL(3,aRow->numFields);
	col1=(twPrimitive*)aRow->fieldEntries->first->value;
	TEST_ASSERT_EQUAL(TW_NUMBER,col1->type);
	TEST_ASSERT_EQUAL(1,col1->val.number);
	col2=(twPrimitive*)aRow->fieldEntries->first->next->value;
	TEST_ASSERT_EQUAL(TW_BOOLEAN,col2->type);
	TEST_ASSERT_TRUE(col2->val.boolean);
	col3=(twPrimitive*)aRow->fieldEntries->first->next->next->value;
	TEST_ASSERT_EQUAL(TW_STRING,col3->type);
	TEST_ASSERT_EQUAL_STRING("Hello World",col3->val.bytes.data);

}