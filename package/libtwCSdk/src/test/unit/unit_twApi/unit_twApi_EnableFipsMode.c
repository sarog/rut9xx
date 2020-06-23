/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_EnableFipsMode()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_EnableFipsMode);

TEST_SETUP(unit_twApi_EnableFipsMode) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_EnableFipsMode) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_EnableFipsMode) {
	RUN_TEST_CASE(unit_twApi_EnableFipsMode, enableAndDisableFipsMode);
}

/**
 * Test Plan: Enable FIPS mode and verify that it has been enabled
 */
TEST(unit_twApi_EnableFipsMode, enableAndDisableFipsMode) {
	TEST_IGNORE_MESSAGE("Disabling this test due to changes made in CSDK-1403");
#ifdef ENABLE_FIPS_MODE
	/* Tests for FIPS mode enabled. */
	TEST_ASSERT_EQUAL(TW_OK, twApi_EnableFipsMode());
	TEST_ASSERT_TRUE(twApi_IsFipsModeEnabled());
	TEST_ASSERT_EQUAL(TW_OK, twApi_DisableFipsMode());
	TEST_ASSERT_FALSE(twApi_IsFipsModeEnabled());
#else
	/* Tests for FIPS mode disabled. */
	TEST_ASSERT_EQUAL(TW_FIPS_MODE_NOT_SUPPORTED, twApi_EnableFipsMode());
	TEST_ASSERT_EQUAL(TW_FIPS_MODE_NOT_SUPPORTED, twApi_IsFIPSCompatible());
	TEST_ASSERT_FALSE(twApi_IsFipsModeEnabled());
#endif
}