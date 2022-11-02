/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetDutyCycle()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP(unit_twApi_SetDutyCycle);

TEST_SETUP(unit_twApi_SetDutyCycle) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetDutyCycle) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetDutyCycle) {
	RUN_TEST_CASE(unit_twApi_SetDutyCycle, setDutyCycleWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetDutyCycle, setDutyCycleAndVerify);
	RUN_TEST_CASE(unit_twApi_SetDutyCycle, setDutyCycleGtHundredAndVerify);
}

/**
 * Test Plan: Try to set the duty cycle with an uninitialized API
 */
TEST(unit_twApi_SetDutyCycle, setDutyCycleWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetDutyCycle(DUTY_CYCLE, DUTY_CYCLE_PERIOD));
}

/**
 * Test Plan: Set the duty cycle and verify the API structure has been updated
 */
TEST(unit_twApi_SetDutyCycle, setDutyCycleAndVerify) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetDutyCycle(50, 75));
	TEST_ASSERT_EQUAL(50, tw_api->duty_cycle);
	TEST_ASSERT_EQUAL(75, tw_api->duty_cycle_period);
}

/**
 * Test Plan: Set the duty cycle to a value greater than one hundred and verify it has been set properly
 */
TEST(unit_twApi_SetDutyCycle, setDutyCycleGtHundredAndVerify) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetDutyCycle(150, 150));
	/* Duty cycle caps at 100 */
	TEST_ASSERT_EQUAL(100, tw_api->duty_cycle);
	TEST_ASSERT_EQUAL(150, tw_api->duty_cycle_period);
}
