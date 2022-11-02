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

TEST_GROUP(unit_TW_MAKE_IT);

TEST_SETUP(unit_TW_MAKE_IT){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_TW_MAKE_IT){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_TW_MAKE_IT){
	RUN_TEST_CASE(unit_TW_MAKE_IT, test_TW_MAKE_IT_macro);
}

TEST(unit_TW_MAKE_IT,test_TW_MAKE_IT_macro){
	twDataShape* dataShape;
	twDataShapeEntry* dse;
	twInfoTableRow* row1test;
	twPrimitive* colarow1;

	twInfoTable* it = TW_MAKE_IT(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("a", TW_NO_DESCRIPTION ,TW_NUMBER),
			                  TW_DS_ENTRY("b", TW_NO_DESCRIPTION ,TW_STRING),
			                  TW_DS_ENTRY("c", TW_NO_DESCRIPTION ,TW_BOOLEAN)
			),
			TW_IT_ROW(TW_MAKE_NUMBER(1),TW_MAKE_BOOL(TRUE),TW_MAKE_STRING("Hello World")),
			TW_IT_ROW(TW_MAKE_NUMBER(2),TW_MAKE_BOOL(FALSE),TW_MAKE_STRING("Hello Again, World")),
			TW_IT_ROW(TW_MAKE_NUMBER(3),TW_MAKE_BOOL(TRUE),TW_MAKE_STRING("Hello Also, World"))
	);

	TEST_ASSERT_NOT_NULL(it);

	/* Perform some spot checks */
	TEST_ASSERT_EQUAL(3,it->rows->count);
	dataShape=it->ds;
	TEST_ASSERT_EQUAL(3,dataShape->numEntries);
	dse = dataShape->entries->first->value;
	TEST_ASSERT_EQUAL_STRING("a",dse->name);

	TEST_ASSERT_EQUAL(3,it->rows->count);
	row1test = it->rows->first->value;
	TEST_ASSERT_EQUAL(3,row1test->numFields);
	colarow1 = row1test->fieldEntries->first->value;

	TEST_ASSERT_EQUAL(TW_NUMBER,colarow1->type);
	TEST_ASSERT_EQUAL(1,colarow1->val.number);

}
