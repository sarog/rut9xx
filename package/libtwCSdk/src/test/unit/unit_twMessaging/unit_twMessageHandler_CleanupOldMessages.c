/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twMessageHandler_CleanupOldMessages()
 */

#include "twApi.h"
#include "TestUtilities.h"
#include "unitTestDefs.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/* Clean up the mock API singleton */
static int mock_twApi_Delete() {
	TW_FREE(tw_api);
	tw_api = NULL;

	/* deleting stubs */
	twApi_DeleteStubs();

	return TW_OK;
}

/* Initialize an empty twApi singleton, and set the isAuthenticated flag*/
static int mock_twApi_Initialize(char * host, uint16_t port, char * resource, char * app_key, char * gatewayName, uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {
	tw_api = (twApi *) TW_CALLOC(1, sizeof(twApi));
	TEST_ASSERT_TRUE(NULL != tw_api);
	tw_api->isAuthenticated = TRUE;

	/* create stubs for the Api */
	twApi_CreateStubs();

	return TWX_SUCCESS;
}

TEST_GROUP(unit_twMessageHandler_CleanupOldMessages);

TEST_SETUP(unit_twMessageHandler_CleanupOldMessages) {
	eatLogs();
	mock_twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twMessageHandler_CleanupOldMessages) {
	mock_twApi_Delete();
	/* Mock API init/delete won't clean this up */
	twLogger_Delete();

	/* reset chunk size */
	twcfg_pointer->message_chunk_size = MESSAGE_CHUNK_SIZE;
}

TEST_GROUP_RUNNER(unit_twMessageHandler_CleanupOldMessages) {
	RUN_TEST_CASE(unit_twMessageHandler_CleanupOldMessages, CSDK_850);
}

/**
 * CSDK-850
 *
 * Summary:	twMessageHandler_CleanupOldMessages() was attempting to delete list entires while iterating through itself.
 * 			This resulted in only the first expired message being deleted from the list.
 *
 * 	Test Plan:	Create a handler, add 20 expired and 10 non-exipired responses to its callback list, check that there's
 * 				30 in total, call twMessageHandler_CleanupOldMessages(), check 10 are left.
 */
void twResponseCallbackStruct_Delete(void * s);
TEST(unit_twMessageHandler_CleanupOldMessages, CSDK_850) {
	int i = 0;
	twMessageHandler *h = NULL;
	h = TW_MALLOC(sizeof(twMessageHandler));
	h->mtx = twMutex_Create();
	h->responseCallbackList = twList_Create(twResponseCallbackStruct_Delete);
	TEST_ASSERT_NOT_NULL(h);
	for (i = 0; i < 20; i++) {
		twResponseCallbackStruct *r = TW_MALLOC(sizeof(twResponseCallbackStruct));
		/* these will get cleaned up */
		r->content = NULL;
		r->expirationTime = 0;
		r->cb = NULL;
		twList_Add(h->responseCallbackList, r);
	}
	for (i = 0; i < 10; i++) {
		twResponseCallbackStruct *r = TW_MALLOC(sizeof(twResponseCallbackStruct));
		/* these won't get cleaned up */
		r->content = NULL;
		r->expirationTime = UINT64_MAX;
		r->cb = NULL;
		TEST_ASSERT_EQUAL(TW_OK, twList_Add(h->responseCallbackList, r));
	}
	TEST_ASSERT_EQUAL(30, h->responseCallbackList->count);
	TEST_ASSERT_EQUAL(TW_OK, twMessageHandler_CleanupOldMessages(h));
	TEST_ASSERT_EQUAL(10, h->responseCallbackList->count);
	twMutex_Delete(h->mtx);
	twList_Delete(h->responseCallbackList);
	TW_FREE(h);
}