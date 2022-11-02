/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterService()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_RegisterService);

TEST_SETUP(unit_twApi_RegisterService) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_RegisterService) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_RegisterService) {
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceWithNullApi);
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceWithNullThingName);
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceWithNullServiceName);
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceWithNullServiceHandler);
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceSuccess);
	RUN_TEST_CASE(unit_twApi_RegisterService, registerServiceTwice);
}

enum msgCodeEnum doNothing_serviceCb(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	return TWX_SUCCESS;
}

/**
 * Test Plan: Try to register a service with a NULL API
 */
TEST(unit_twApi_RegisterService, registerServiceWithNullApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service with a NULL thing name
 */
TEST(unit_twApi_RegisterService, registerServiceWithNullThingName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterService(TW_THING, NULL, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service with a NULL property name
 */
TEST(unit_twApi_RegisterService, registerServiceWithNullServiceName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, NULL, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service with a NULL service handler
 */
TEST(unit_twApi_RegisterService, registerServiceWithNullServiceHandler) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, NULL, NULL));
}

/**
 * Test Plan: Try to register a service
 */
TEST(unit_twApi_RegisterService, registerServiceSuccess) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
}

/**
 * Test Plan: Try to register a service twice
 */
TEST(unit_twApi_RegisterService, registerServiceTwice) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
	TEST_ASSERT_EQUAL(TW_ERROR_ITEM_EXISTS, twApi_RegisterService(TW_THING, TEST_ENTITY_NAME, TEST_SERVICE_NAME, NULL, NULL, TW_NOTHING, NULL, doNothing_serviceCb, NULL));
}