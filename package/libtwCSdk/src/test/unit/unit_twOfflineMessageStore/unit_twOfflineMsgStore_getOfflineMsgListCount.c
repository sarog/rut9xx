/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for getOfflineMsgListCount()
*/

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) struct twOfflineMsgStore * tw_offline_msg_store;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
extern struct twOfflineMsgStore * tw_offline_msg_store;
#endif

int getOfflineMsgListCount();

TEST_GROUP(unit_twOfflineMsgStore_getOfflineMsgListCount);

TEST_SETUP(unit_twOfflineMsgStore_getOfflineMsgListCount) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_getOfflineMsgListCount) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_getOfflineMsgListCount) {
	RUN_TEST_CASE(unit_twOfflineMsgStore_getOfflineMsgListCount, twOfflineMsgStore_getOfflineMsgListsCount_Success);
}

/* Test plan: getOfflineMsgListCount() of twOfflineMsgStore will return offline message list count. */
TEST(unit_twOfflineMsgStore_getOfflineMsgListCount, twOfflineMsgStore_getOfflineMsgListsCount_Success) {
	twList *list = twList_Create(&doNothing);
	int i;
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	for (i = 0; i < TEST_LIST_BASIC_TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(list, i));
	}
	/* OfflineMsgStore does not contain any list so it will return error status -1. */
	TEST_ASSERT_EQUAL(-1, getOfflineMsgListCount());

	tw_offline_msg_store->offlineMsgList = list;
	TEST_ASSERT_EQUAL(list->count, getOfflineMsgListCount());
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
	tw_offline_msg_store->offlineMsgList = NULL;
	TEST_ASSERT_NULL(tw_offline_msg_store->offlineMsgList);
}
