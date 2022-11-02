/**
 * Thread Utility twExt_WaitUntilFirstSynchronization Unit Tests
 *
 * Copyright 2019, PTC, Inc.
 *
 * Created by Thomas Henley on 1/16/19.
 */

#include "twApi.h"
#include "twThreadUtils.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

#define TIMEOUT 200

TEST_GROUP(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization);

TEST_SETUP(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization) {
    eatLogs();
    tw_api = (twApi *)TW_CALLOC(sizeof(twApi), 1);
    TEST_ASSERT_NOT_NULL(tw_api);
}

TEST_TEAR_DOWN(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization) {
    TW_FREE(tw_api);
    tw_api = NULL;
}

TEST_GROUP_RUNNER(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization) {
    RUN_TEST_CASE(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization, test_firstSyncCompleteInTime);
    RUN_TEST_CASE(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization, test_firstSyncTimesOut);
}

/**
 * Test Plan: Set firstSynchronizationComplete to TRUE and assert the function returns ok
 */
TEST(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization, test_firstSyncCompleteInTime) {
    tw_api->firstSynchronizationComplete = TRUE;
    TEST_ASSERT_EQUAL(TW_OK, twExt_WaitUntilFirstSynchronization(TIMEOUT));
}

/**
 * Test Plan: Set firstSynchronizationComplete to FALSE and assert the function returns an error
 */
TEST(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization, test_firstSyncTimesOut) {
    tw_api->firstSynchronizationComplete = FALSE;
    TEST_ASSERT_EQUAL(TW_SUBSCRIBED_PROPERTY_SYNCHRONIZATION_TIMEOUT, twExt_WaitUntilFirstSynchronization(TIMEOUT));
}
