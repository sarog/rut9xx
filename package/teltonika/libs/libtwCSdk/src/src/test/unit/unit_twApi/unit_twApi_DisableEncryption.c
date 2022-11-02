/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_DisableEncryption()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_DisableEncryption);

TEST_SETUP(unit_twApi_DisableEncryption) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_DisableEncryption) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_DisableEncryption) {
	RUN_TEST_CASE(unit_twApi_DisableEncryption, disableEncryption);
	RUN_TEST_CASE(unit_twApi_DisableEncryption, disableEncryptionTwice);
}

/**
 * Test Plan: Disable encryption and verify that the connection info structure has been updated
 */
TEST(unit_twApi_DisableEncryption, disableEncryption) {
	twConnectionInfo *info = twApi_GetConnectionInfo();
	TEST_ASSERT_FALSE(info->disableEncryption);
	twApi_DisableEncryption();
	TEST_ASSERT_EQUAL(TRUE, info->disableEncryption);
}

/**
 * Test Plan: Disable encryption twice and verify that the connection info structure has been updated
 */
TEST(unit_twApi_DisableEncryption, disableEncryptionTwice) {
	twConnectionInfo *info = twApi_GetConnectionInfo();
	TEST_ASSERT_FALSE(info->disableEncryption);
	twApi_DisableEncryption();
	TEST_ASSERT_EQUAL(TRUE, info->disableEncryption);
	twApi_DisableEncryption();
	TEST_ASSERT_EQUAL(TRUE, info->disableEncryption);
}