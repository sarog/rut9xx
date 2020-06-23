/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_Initialize()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_Initialize);

TEST_SETUP(unit_twApi_Initialize) {
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	TEST_ASSERT_NULL(twApi_GetApi());
}

TEST_TEAR_DOWN(unit_twApi_Initialize) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_Initialize) {
	RUN_TEST_CASE(unit_twApi_Initialize, initializeAndDeleteApi);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeApiTwiceAndDeleteApiTwice);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeWithNullHost);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeWithPortZero);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeWithNullUri);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeWithNullAppKey);
	RUN_TEST_CASE(unit_twApi_Initialize, initializeWithChunkSizeGtFrameSize);
	RUN_TEST_CASE(unit_twApi_Initialize, deleteApiWithDanglingDictionary);
	RUN_TEST_CASE(unit_twApi_Initialize, deleteApiWithDicitonaryHashCleanupFailure);
	RUN_TEST_CASE(unit_twApi_Initialize, CSDK_1188);
	RUN_TEST_CASE(unit_twApi_Initialize, CSDK_1278);
}

/**
 * Test Plan: Try to initialize the API with valid parameters and then delete it
 */
TEST(unit_twApi_Initialize, initializeAndDeleteApi) {
	/* Initialize with valid params */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Make sure our API has been initialized */
	TEST_ASSERT_NOT_NULL(twApi_GetApi());
	/* Delete the API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	/* Make sure our API pointer is now NULL */
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API twice with valid parameters and then delete it twice
 */
TEST(unit_twApi_Initialize, initializeApiTwiceAndDeleteApiTwice) {
	/* Initialize with valid params */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Make sure our API has been initialized */
	TEST_ASSERT_NOT_NULL(twApi_GetApi());
	/* Initialize the API a second time */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Make sure our API pointer is still not NULL */
	TEST_ASSERT_NOT_NULL(twApi_GetApi());
	/* Delete the API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	/* Make sure our API pointer is now NULL */
	TEST_ASSERT_NULL(twApi_GetApi());
	/* Delete the API a second time */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	/* Make sure our API pointer is still NULL */
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API with a NULL host
 */
TEST(unit_twApi_Initialize, initializeWithNullHost) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(NULL, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API with port 0
 */
TEST(unit_twApi_Initialize, initializeWithPortZero) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(TW_HOST, 0, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API with a NULL URI
 */
TEST(unit_twApi_Initialize, initializeWithNullUri) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(TW_HOST, TW_PORT, NULL, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API with a NULL app key
 */
TEST(unit_twApi_Initialize, initializeWithNullAppKey) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, NULL, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_NULL(twApi_GetApi());
}

/**
 * Test Plan: Try to initialize the API with a chunk size greater than the frame size
 */
TEST(unit_twApi_Initialize, initializeWithChunkSizeGtFrameSize) {
	twcfg_pointer->connect_retry_interval = UNIT_TEST_SMALL_DELAY;
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE+1, MESSAGE_CHUNK_SIZE, TRUE));
	twcfg_pointer->connect_retry_interval = CONNECT_RETRY_INTERVAL;
	TEST_ASSERT_NULL(twApi_GetApi());
}

/* No-op dict parse/delete stubs used in the dictionary cleanup test */
static const char * test_twApi_noop_dictParseFunction(void * item) { return ""; }
static void test_twApi_noop_dictDeleteFunction(void * ptr) { }

/**
 * Test Plan: This test asserts that twApi_Delete returns TW_UNKNOWN_ERROR if it is invoked with a dangling dictionary
 * reference
 */
TEST(unit_twApi_Initialize, deleteApiWithDanglingDictionary) {
	twDict * dict;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	dict = twDict_Create(test_twApi_noop_dictParseFunction, test_twApi_noop_dictDeleteFunction);
	TEST_ASSERT_EQUAL(TW_UNKNOWN_ERROR, twApi_Delete());
	eatLogs();
	/* Once the API is deallocated, it can be reinitialized and free'd without failure */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	twDict_Delete(dict);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

static int stub_ApiUnitTests_cfuhash_destroy(struct cfuhash_table * ht) { return 0; }

/**
 * Test Plan: This test asserts that twApi_Delete returns TW_UNKNOWN_ERROR if it twMap's parse and delete functions
 * cannot be cleaned up due to a cfuhash_destroy failure
 */
TEST(unit_twApi_Initialize, deleteApiWithDicitonaryHashCleanupFailure) {
	int err;
	twDict * dict;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Use());
	/* Ensure that the global parse/delete function maps are allocated */
	dict = twDict_Create(test_twApi_noop_dictParseFunction, test_twApi_noop_dictDeleteFunction);
	twDict_Delete(dict);
	/* Install the stub, all subsequent map deallocations will fail */
	twApi_stub->cfuhash_destroy = stub_ApiUnitTests_cfuhash_destroy;
	TEST_ASSERT_EQUAL(TW_UNKNOWN_ERROR, twApi_Delete());
	/* Reset stubs */
	TEST_ASSERT_EQUAL(TW_OK, twStubs_Reset());
	/* Deallocate the API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

/**
 * CSDK-1188
 *
 * Test Plan: twApi_SetSelfSignedOk() & twApi_DisableCertValidation() dereference a NULL pointer (tw_api) if twApi_Initialize fails
 */
TEST(unit_twApi_Initialize, CSDK_1188) {

	/* Fail Initialization */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_Initialize(NULL, NULL, NULL, NULL, NULL, MESSAGE_CHUNK_SIZE * 1000, MESSAGE_CHUNK_SIZE * 1000, FALSE));

	/* Check we're no longer dereferencing a null pointer (tw_api) in the following functions */
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
}

/**
 * CSDK-1278
 *
 * Test Plan: Should be able to set the frame size to a value larger than the chunk size
 */
TEST(unit_twApi_Initialize, CSDK_1278) {
	/* Frame size > chunk size */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE + (MESSAGE_CHUNK_SIZE*0.1), TRUE));
	TEST_ASSERT_NOT_NULL(twApi_GetApi());
}