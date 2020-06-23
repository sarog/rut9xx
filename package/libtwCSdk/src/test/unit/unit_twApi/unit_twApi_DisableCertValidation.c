/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_DisableCertValidation()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_DisableCertValidation);

TEST_SETUP(unit_twApi_DisableCertValidation) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
}

TEST_TEAR_DOWN(unit_twApi_DisableCertValidation) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_DisableCertValidation) {
	RUN_TEST_CASE(unit_twApi_DisableCertValidation, disableCertValidation);
	RUN_TEST_CASE(unit_twApi_DisableCertValidation, disableCertValidationTwice);
}

/**
 * Test Plan: Disable certificate validation and verify that the connection info structure has been updated
 */
TEST(unit_twApi_DisableCertValidation, disableCertValidation) {
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	twApi_DisableCertValidation();
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->validateCert);
}

/**
 * Test Plan: Disable certificate validation twice and verify that the connection info structure has been updated
 */
TEST(unit_twApi_DisableCertValidation, disableCertValidationTwice) {
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	twApi_DisableCertValidation();
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->validateCert);
	twApi_DisableCertValidation();
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->validateCert);
}