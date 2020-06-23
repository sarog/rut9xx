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

TEST_GROUP(unit_twExt_StandardPropertyHandler);

TEST_SETUP(unit_twExt_StandardPropertyHandler){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twExt_StandardPropertyHandler){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twExt_StandardPropertyHandler){
	RUN_TEST_CASE(unit_twExt_StandardPropertyHandler, test_twGenericPropertyHandlerWrite);
	RUN_TEST_CASE(unit_twExt_StandardPropertyHandler, twGenericPropertyHandlerWriteReadRead);
}

/**
 * Test Plan: Test the ability of the generic property handler to read back from what it writes.
 */
TEST(unit_twExt_StandardPropertyHandler,test_twGenericPropertyHandlerWrite){

	char * entityName="testThing";
	char * propertyName="name";
	char isWrite=TRUE;
	char isRead=FALSE;
	void * userdata=NULL;
	char duplicate = TRUE;
	char * nameValue = "TestThingName";
	twInfoTable * itRead;
	enum msgCodeEnum resultRead;
	twPrimitive* readBackPrimitive1;

	/*Assign a property value*/
	twInfoTable * it = twInfoTable_CreateFromString(propertyName, nameValue, duplicate);
	enum msgCodeEnum resultWrite = twExt_StandardPropertyHandler(entityName, propertyName, &it, isWrite, userdata);
	TEST_ASSERT_EQUAL(TWX_SUCCESS,resultWrite);

	/*Read it back*/
	resultRead = twExt_StandardPropertyHandler(entityName, propertyName, &itRead, isRead, userdata);
	TEST_ASSERT_NOT_NULL(itRead);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itRead, propertyName, 0, &readBackPrimitive1));
	TEST_ASSERT_EQUAL(TW_STRING,readBackPrimitive1->type);
	TEST_ASSERT_EQUAL_STRING("TestThingName",readBackPrimitive1->val.bytes.data);
}

/**
 * Test Plan: A Bug was reported where one write and two reads returns a damaged primitive.
 */
TEST(unit_twExt_StandardPropertyHandler,twGenericPropertyHandlerWriteReadRead){

	char * entityName="testThingRW";
	char * propertyName="lightOn";
	char Write=TRUE;
	char Read=FALSE;
	void * userdata=NULL;
	twPrimitive* readBackPrimitive;
	twPrimitive* readBackPrimitive1;
	twInfoTable * itRead;
	twInfoTable * itRead1;

	/* Assign a property value */
	twInfoTable * it = twInfoTable_CreateFromBoolean(propertyName, TRUE);
	enum msgCodeEnum resultWrite = twExt_StandardPropertyHandler(entityName, propertyName, &it, Write, userdata);
	TEST_ASSERT_EQUAL(TWX_SUCCESS,resultWrite);

	TEST_ASSERT_EQUAL(TWX_SUCCESS, twExt_StandardPropertyHandler(entityName, propertyName, &itRead, Read, userdata));
	TEST_ASSERT_NOT_NULL(itRead);
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itRead, propertyName, 0, &readBackPrimitive));
	TEST_ASSERT_EQUAL(TW_BOOLEAN,readBackPrimitive->type);
	TEST_ASSERT_TRUE(readBackPrimitive->val.boolean);

	TEST_ASSERT_EQUAL(TWX_SUCCESS, twExt_StandardPropertyHandler(entityName, propertyName, &itRead1, Read, userdata));
	TEST_ASSERT_NOT_NULL(itRead1);
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itRead, propertyName, 0, &readBackPrimitive1));
	TEST_ASSERT_EQUAL(TW_BOOLEAN,readBackPrimitive1->type);
	TEST_ASSERT_TRUE(readBackPrimitive1->val.boolean);

}