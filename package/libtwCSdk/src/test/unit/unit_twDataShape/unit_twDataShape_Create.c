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

TEST_GROUP(unit_twDataShape_Create);

TEST_SETUP(unit_twDataShape_Create){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twDataShape_Create){
	twApi_Delete();
}

TEST_GROUP_RUNNER(unit_twDataShape_Create){
	RUN_TEST_CASE(unit_twDataShape_Create, test_twDataShape_CreateFromEntries);
	RUN_TEST_CASE(unit_twDataShape_Create, test_DataShapeCreationMacros);
}

/**
 * Test Plan: twDataShape_CreateFromEntries() is a "shapes only" function that the TW_MAKE_DATASHAPE() macro requires.
 * Provide it with two shape definitions and verify that they are created.
 *
 * WARNING: If this test does not pass, your platform may not be able to support thing shape macros.
 */
TEST(unit_twDataShape_Create,test_twDataShape_CreateFromEntries){
	twDataShapeEntry * thirdShape = NULL;
	twDataShapeEntry * secondShape = NULL;
	twDataShapeEntry * firstShape = NULL;
	ListEntry* third = NULL;
	ListEntry* second = NULL;
	ListEntry* first = NULL;
	twDataShape* dataShape = NULL;
	twDataShapeEntry *shapeEntry3 = NULL;
	twDataShapeEntry *shapeEntry2 = NULL;
	twDataShapeEntry *shapeEntry1 = NULL;

	shapeEntry1 = twDataShapeEntry_Create("price", "How much it costs.", TW_NUMBER);
	TEST_ASSERT_NOT_NULL(shapeEntry1);
	shapeEntry2 = twDataShapeEntry_Create("description", "What it's for.", TW_STRING);
	TEST_ASSERT_NOT_NULL(shapeEntry2);
	shapeEntry3 = twDataShapeEntry_Create("truth", "Is it the?", TW_BOOLEAN);
	TEST_ASSERT_NOT_NULL(shapeEntry3);
	dataShape = twDataShape_CreateFromEntries(TW_SHAPE_NAME_NONE,shapeEntry1, shapeEntry2,shapeEntry3,NULL);
	TEST_ASSERT_NOT_NULL(dataShape);
	TEST_ASSERT_EQUAL_INT(3,dataShape->numEntries);

	first =  dataShape->entries->first;
	TEST_ASSERT_NOT_NULL(first);
	second = twList_Next(dataShape->entries, first);
	TEST_ASSERT_NOT_NULL(second);
	third = twList_Next(dataShape->entries, second);
	TEST_ASSERT_NOT_NULL(third);

	firstShape = first->value;
	TEST_ASSERT_NOT_NULL(firstShape);
	TEST_ASSERT_EQUAL_STRING("price",firstShape->name);
	TEST_ASSERT_EQUAL_STRING("How much it costs.",firstShape->description);
	TEST_ASSERT_EQUAL_INT(TW_NUMBER,firstShape->type);

	secondShape = second->value;
	TEST_ASSERT_NOT_NULL(secondShape);
	TEST_ASSERT_EQUAL_STRING("description",secondShape->name);
	TEST_ASSERT_EQUAL_STRING("What it's for.",secondShape->description);
	TEST_ASSERT_EQUAL_INT(TW_STRING,secondShape->type);

	thirdShape = third->value;
	TEST_ASSERT_NOT_NULL(thirdShape);
	TEST_ASSERT_EQUAL_STRING("truth",thirdShape->name);
	TEST_ASSERT_EQUAL_STRING("Is it the?",thirdShape->description);
	TEST_ASSERT_EQUAL_INT( TW_BOOLEAN,thirdShape->type);
}


TEST(unit_twDataShape_Create,test_DataShapeCreationMacros) {

	twDataShapeEntry * thirdShape = NULL;
	twDataShapeEntry * secondShape  = NULL;
	twDataShapeEntry * firstShape = NULL;
	ListEntry* third = NULL;
	ListEntry* second = NULL;
	ListEntry* first = NULL;

	twDataShape* ds = TW_MAKE_DATASHAPE("inventoryShape",
	                                    TW_DS_ENTRY("price", "How much it costs." ,TW_NUMBER),
	                                    TW_DS_ENTRY("description", "What it's for.", TW_STRING),
	                                    TW_DS_ENTRY("truth","Is it the?", TW_BOOLEAN)
	);

	TEST_ASSERT_NOT_NULL(ds);
	TEST_ASSERT_EQUAL_STRING("inventoryShape",ds->name);
	TEST_ASSERT_EQUAL_INT(3,ds->numEntries);

	first =  ds->entries->first;
	TEST_ASSERT_NOT_NULL(first);
	second = twList_Next(ds->entries, first);
	TEST_ASSERT_NOT_NULL(second);
	third = twList_Next(ds->entries, second);
	TEST_ASSERT_NOT_NULL(third);

	firstShape = first->value;
	TEST_ASSERT_NOT_NULL(firstShape);
	TEST_ASSERT_EQUAL_STRING("price",firstShape->name);
	TEST_ASSERT_EQUAL_STRING("How much it costs.",firstShape->description);
	TEST_ASSERT_EQUAL_INT(TW_NUMBER,firstShape->type);

	secondShape = second->value;
	TEST_ASSERT_NOT_NULL(secondShape);
	TEST_ASSERT_EQUAL_STRING("description",secondShape->name);
	TEST_ASSERT_EQUAL_STRING("What it's for.",secondShape->description);
	TEST_ASSERT_EQUAL_INT(TW_STRING,secondShape->type);

	thirdShape = third->value;
	TEST_ASSERT_NOT_NULL(thirdShape);
	TEST_ASSERT_EQUAL_STRING("truth",thirdShape->name);
	TEST_ASSERT_EQUAL_STRING("Is it the?",thirdShape->description);
	TEST_ASSERT_EQUAL_INT( TW_BOOLEAN,thirdShape->type);

}