/*
 * Copyright 2016, PTC, Inc.
 *
 */

#include "twBaseTypes.h"
#include "warehouse.h"
#include "twProperties.h"
#include "twSubscribedProps.h"
#include "twShapes.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

twList* twExt_GetChangeListenersList();

TEST_GROUP(unit_TW_DECLARE_SHAPE);

TEST_SETUP(unit_TW_DECLARE_SHAPE){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_TW_DECLARE_SHAPE){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_TW_DECLARE_SHAPE){
	RUN_TEST_CASE(unit_TW_DECLARE_SHAPE, test_DeclareShapeMacro);
}

/**
 * Test Plan: The TW_DECLARE_SHAPE Macro should create two local variables
 * _tw_thing_name and _tw_shape_name to be used to add properties to.
 * Verify their presence and value after it is used.
 */
TEST(unit_TW_DECLARE_SHAPE,test_DeclareShapeMacro) {
	TW_DECLARE_SHAPE("testThingName", "Test Shape",TW_NO_NAMESPACE);
	TEST_ASSERT_NULL(_tw_thing_namespace);
	TEST_ASSERT_EQUAL_STRING("testThingName",_tw_thing_name);
	TEST_ASSERT_EQUAL_STRING("Test Shape",_tw_shape_name);
}