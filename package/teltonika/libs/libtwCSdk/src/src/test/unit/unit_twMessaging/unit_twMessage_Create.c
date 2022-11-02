/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twMessage_Create()
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

TEST_GROUP(unit_twMessage_Create);

TEST_SETUP(unit_twMessage_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMessage_Create) {
}

TEST_GROUP_RUNNER(unit_twMessage_Create) {
	RUN_TEST_CASE(unit_twMessage_Create, test_twMessages_Create_Assigned_RequestIds);
	RUN_TEST_CASE(unit_twMessage_Create, test_twMessages_Create_Set_RequestId);
}

TEST(unit_twMessage_Create, test_twMessages_Create_Assigned_RequestIds) {
	twMessage * msgs[3];
	int i, j;

	for (i = 0; i < 3; ++i) {
		msgs[i] = twMessage_Create(TWX_AUTH, 0);
	}

	/* Verify request IDs are unique */
	for (i = 0; i < 3; ++i) {
		for (j = i + 1; j < 3; ++j) {
			TEST_ASSERT_NOT_EQUAL(msgs[i]->requestId, msgs[j]->requestId);
		}
	}

	/* Clean up */
	for (i = 0; i < 3; ++i) {
		TW_FREE(msgs[i]);
	}
}

TEST(unit_twMessage_Create, test_twMessages_Create_Set_RequestId) {
	/* On the off chance that the function returns a set magic number, try many. */
	const uint32_t requestId[] = { 42, 5, 3 };
	int i;

	for (i = 0; i < 3; ++i) {
		twMessage * msg = twMessage_Create(TWX_AUTH, requestId[i]);

		TEST_ASSERT_EQUAL(requestId[i], msg->requestId);
		TW_FREE(msg);
	}


}