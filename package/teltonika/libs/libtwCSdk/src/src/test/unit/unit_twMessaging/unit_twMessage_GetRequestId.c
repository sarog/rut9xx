/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twMessage_GetRequestId()
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

TEST_GROUP(unit_twMessage_GetRequestId);

TEST_SETUP(unit_twMessage_GetRequestId) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMessage_GetRequestId) {
}

TEST_GROUP_RUNNER(unit_twMessage_GetRequestId) {
	RUN_TEST_CASE(unit_twMessage_GetRequestId, test_twMessage_GetRequestId);
}

TEST(unit_twMessage_GetRequestId, test_twMessage_GetRequestId) {
	uint32_t prev = 0, cur;
	int i;

	for (i = 0; i < 5; ++i) {
		cur = twMessage_GetRequestId();
		TEST_ASSERT_NOT_EQUAL(prev, cur);
		prev = cur;
	}
}
