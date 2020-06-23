/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetX509Fields()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetX509Fields);

TEST_SETUP(unit_twApi_SetX509Fields) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetX509Fields) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetX509Fields) {
	RUN_TEST_CASE(unit_twApi_SetX509Fields, setX509FieldsWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetX509Fields, setX509FieldsSuccess);
}

/**
 * Test Plan: Set X509 fields with an uninitialized API
 */
TEST(unit_twApi_SetX509Fields, setX509FieldsWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetX509Fields(TEST_SUBJECT_CN, TEST_SUBJECT_O, TEST_SUBJECT_OU, TEST_ISSUER_CN, TEST_ISSUER_O, TEST_ISSUER_OU));
}

/**
 * Test Plan: Set X509 fields and verify that the connection info structure has been updated
 */
TEST(unit_twApi_SetX509Fields, setX509FieldsSuccess) {
	twConnectionInfo *info = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	info = twApi_GetConnectionInfo();
	TEST_ASSERT_NULL(info->subject_o);
	TEST_ASSERT_NULL(info->subject_ou);
	TEST_ASSERT_NULL(info->issuer_cn);
	TEST_ASSERT_NULL(info->issuer_o);
	TEST_ASSERT_NULL(info->issuer_ou);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetX509Fields(TEST_SUBJECT_CN, TEST_SUBJECT_O, TEST_SUBJECT_OU, TEST_ISSUER_CN, TEST_ISSUER_O, TEST_ISSUER_OU));
	TEST_ASSERT_EQUAL_STRING(TEST_SUBJECT_O, info->subject_o);
	TEST_ASSERT_EQUAL_STRING(TEST_SUBJECT_OU, info->subject_ou);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_CN, info->issuer_cn);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_O, info->issuer_o);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_OU, info->issuer_ou);
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetX509Fields(TEST_SUBJECT_CN_1, TEST_SUBJECT_O_1, TEST_SUBJECT_OU_1, TEST_ISSUER_CN_1, TEST_ISSUER_O_1, TEST_ISSUER_OU_1));
	TEST_ASSERT_EQUAL_STRING(TEST_SUBJECT_O_1, info->subject_o);
	TEST_ASSERT_EQUAL_STRING(TEST_SUBJECT_OU_1, info->subject_ou);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_CN_1, info->issuer_cn);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_O_1, info->issuer_o);
	TEST_ASSERT_EQUAL_STRING(TEST_ISSUER_OU_1, info->issuer_ou);
}