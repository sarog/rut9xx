/*
 *	Copyright 2016, PTC, Inc.
 */

#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10
#define NAME_SIZE 64

#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "twApi.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"

TEST_GROUP(Bind);

void test_BindAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(Bind){
	eatLogs();
}

TEST_TEAR_DOWN(Bind){
}

TEST_GROUP_RUNNER(Bind){
    RUN_TEST_CASE(Bind, test_BindThingBeforeConnect);
    RUN_TEST_CASE(Bind, test_BindThingAfterConnect);
    RUN_TEST_CASE(Bind, test_BindThingsBeforeConnect);
    RUN_TEST_CASE(Bind, test_BindThingsAfterConnect);
}


#define CONNECTION_RETRIES 1
#define CONNECTION_TIMEOUT 2000

/* bind a single thing before connecting */ 
TEST(Bind,test_BindThingBeforeConnect){
	/* create var to collect results */
	int err = TW_OK;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* bind thing */
	err = twApi_BindThing("RickAstley");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* connect api */
	err = twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return;
}

/* bind a single thing after connecting */
TEST(Bind,test_BindThingAfterConnect){
	/* create var to collect results */
	int err = 0;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* connect api */
	err = twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* bind thing */
	err = twApi_BindThing("RickRoss");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

/* bind multiple things before connecting */
TEST(Bind,test_BindThingsBeforeConnect){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
    char * name = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	tbb_list = twList_Create(free_list);
	
	for (i = 0; i < NUM_NAMES; i++) {
		/* set name */
		name = (char * )TW_MALLOC(NAME_SIZE);
		sprintf(name, "Steam-%u", i);

		/* add to list */
		twList_Add(tbb_list, name);
	}

	/* bind thing */
	err = twApi_BindThings(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	err = twList_Delete(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	
	/* connect api */
	err = twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

/* bind multiple things after connecting */
TEST(Bind,test_BindThingsAfterConnect){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
	char * name = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* connect api */
	err = twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	tbb_list = twList_Create(free_list);
	
	for (i = 0; i < NUM_NAMES; i++) {
		/* set name */
		name = (char * )TW_CALLOC(NAME_SIZE,sizeof(char));
		sprintf(name, "Steam-%i", i);

		/* add to list */
		twList_Add(tbb_list, name);

		/* list takes ownership of pointer, so NULL name before continuing */
		name = NULL;
	}

	/* bind thing */
	err = twApi_BindThings(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	err = twList_Delete(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

