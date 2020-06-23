/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twBindBody_CreateFromStream()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

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

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

TEST_GROUP(unit_twBindBody_CreateFromStream);

TEST_SETUP(unit_twBindBody_CreateFromStream) {
	eatLogs();
	mock_twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twBindBody_CreateFromStream) {
	mock_twApi_Delete();
	/* Mock API init/delete won't clean this up */
	twLogger_Delete();

	/* reset chunk size */
	twcfg_pointer->message_chunk_size = MESSAGE_CHUNK_SIZE;
}

TEST_GROUP_RUNNER(unit_twBindBody_CreateFromStream) {
	RUN_TEST_CASE(unit_twBindBody_CreateFromStream, test_twBindBody_CreateFromStream);
}

/* Ensure that the entity count in bind messages is deserialized properly */
TEST(unit_twBindBody_CreateFromStream, test_twBindBody_CreateFromStream) {
	twBindBody* bindBody = NULL;
	twStream* stream = NULL;
	const char* entityName = "anEntityName";
	const unsigned entityCount = 135; /* CSDK-55: 127 < entityCount < 256 (MSb set trips defect)*/
	unsigned u = 0;
	ListEntry* next = NULL;

	/* Add entityCount names to the bind body */
	bindBody = twBindBody_Create(NULL);
	for (u = 0; u < entityCount; ++u) {
		twBindBody_AddName(bindBody, entityName);
	}

	/* Serialize the bindbody to a stream */
	stream = twStream_Create();
	twBindBody_ToStream(bindBody, stream, "gatewayName", "gatewayType");

	twBindBody_Delete(bindBody);
	bindBody = NULL;

	/* Reconstitute a new bindbody from the binary stream */
	twStream_Reset(stream);
	bindBody = twBindBody_CreateFromStream(stream);
	TEST_ASSERT_NOT_NULL(bindBody);

	/* Verify that all of the bind names came back */
	TEST_ASSERT_NOT_NULL(bindBody);
	TEST_ASSERT_EQUAL_UINT(entityCount, bindBody->count);
	next = twList_GetByIndex(bindBody->names, 0);
	for (u = 0; u < entityCount; ++u) {
		TEST_ASSERT_EQUAL_STRING(entityName, (char*)next->value);
		twList_Next(bindBody->names, next);
	}
	twStream_Delete(stream);
	twBindBody_Delete(bindBody);
}