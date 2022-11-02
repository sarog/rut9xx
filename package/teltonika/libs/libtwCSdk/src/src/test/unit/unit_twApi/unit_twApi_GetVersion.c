/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_GetVersion()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_GetVersion);

TEST_SETUP(unit_twApi_GetVersion) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_GetVersion) {
}

TEST_GROUP_RUNNER(unit_twApi_GetVersion) {
	RUN_TEST_CASE(unit_twApi_GetVersion, getCSdkVersion);
}

/**
 * Test Plan: Try to connect with an uninitialized API
 */
TEST(unit_twApi_GetVersion, getCSdkVersion) {
	TEST_ASSERT_EQUAL_STRING(C_SDK_VERSION, twApi_GetVersion());
}