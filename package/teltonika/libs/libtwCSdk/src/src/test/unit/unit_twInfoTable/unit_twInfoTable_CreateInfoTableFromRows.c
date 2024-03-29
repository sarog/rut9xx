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

TEST_GROUP(unit_twInfoTable_CreateInfoTableFromRows);

TEST_SETUP(unit_twInfoTable_CreateInfoTableFromRows){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twInfoTable_CreateInfoTableFromRows){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twInfoTable_CreateInfoTableFromRows){
	RUN_TEST_CASE(unit_twInfoTable_CreateInfoTableFromRows, test_twInfoTable_CreateInfoTableFromRows);
}

/**
 * Test Plan: Test the vararg function twInfoTable_CreateInfoTableFromRows() by manually constructing
 * InfoTable rows and then examine the created infotable to make sure that all the rows are present
 * and have the values that were originally set.
 */
TEST(unit_twInfoTable_CreateInfoTableFromRows,test_twInfoTable_CreateInfoTableFromRows){
	twDataShape* dataShape;
	twDataShapeEntry* dse;
	twInfoTableRow* row1test;
	twPrimitive* colarow1;
	twInfoTableRow* row1 = twInfoTable_CreateRowFromEntries(TW_MAKE_NUMBER(1),TW_MAKE_STRING("THREE"),TW_MAKE_BOOL(TRUE),NULL);
	twInfoTableRow* row2 = twInfoTable_CreateRowFromEntries(TW_MAKE_NUMBER(2),TW_MAKE_STRING("FOUR"),TW_MAKE_BOOL(FALSE),NULL);
	twInfoTableRow* row3 = twInfoTable_CreateRowFromEntries(TW_MAKE_NUMBER(3),TW_MAKE_STRING("FIVE"),TW_MAKE_BOOL(TRUE),NULL);
	twInfoTable* it = twInfoTable_CreateInfoTableFromRows(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("a", TW_NO_DESCRIPTION ,TW_NUMBER),TW_DS_ENTRY("b", TW_NO_DESCRIPTION ,TW_STRING),
			                  TW_DS_ENTRY("c", TW_NO_DESCRIPTION ,TW_BOOLEAN)
			),
			row1,row2,row3,NULL
	);
	TEST_ASSERT_EQUAL(3,(int)it->rows->count);
	dataShape=it->ds;
	TEST_ASSERT_EQUAL(3,dataShape->numEntries);
	dse = dataShape->entries->first->value;
	TEST_ASSERT_EQUAL_STRING("a",dse->name);
	TEST_ASSERT_EQUAL(3,it->rows->count);
	row1test = it->rows->first->value;
	TEST_ASSERT_EQUAL(3,row1test->numFields);
	colarow1 = row1test->fieldEntries->first->value;
	TEST_ASSERT_EQUAL(TW_NUMBER,colarow1->type);
	TEST_ASSERT_TRUE(1.0 == colarow1->val.number);
}