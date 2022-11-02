/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for isOfflineMsgListEnabled()
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

char isOfflineMsgListEnabled();

TEST_GROUP(unit_twOfflineMsgStore_isOfflineMsgListEnabled);

TEST_SETUP(unit_twOfflineMsgStore_isOfflineMsgListEnabled) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_isOfflineMsgListEnabled) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_isOfflineMsgListEnabled) {
	RUN_TEST_CASE(unit_twOfflineMsgStore_isOfflineMsgListEnabled, twOfflineMsgStore_isOfflineMsgListEnabled_Success);
}

/* Test plan: isOfflineMsgListEnabled of twOfflineMsgStore will return the status of offline message list. */
TEST(unit_twOfflineMsgStore_isOfflineMsgListEnabled, twOfflineMsgStore_isOfflineMsgListEnabled_Success) {
	twList *list = twList_Create(&doNothing);
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));

	/* There is not list exist in twOfflineMsgStore so it will return FALSE. */
	TEST_ASSERT_EQUAL(FALSE, isOfflineMsgListEnabled());

	tw_offline_msg_store->offlineMsgList = list;
	TEST_ASSERT_EQUAL(TRUE, isOfflineMsgListEnabled());
	TEST_ASSERT_EQUAL(TW_OK, twList_Delete(list));
	tw_offline_msg_store->offlineMsgList = NULL;
	TEST_ASSERT_NULL(tw_offline_msg_store->offlineMsgList);
}
