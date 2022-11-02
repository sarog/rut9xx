#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogger_SetFunction);

TEST_SETUP(unit_twLogger_SetFunction) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogger_SetFunction) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogger_SetFunction) {
	RUN_TEST_CASE(unit_twLogger_SetFunction, test_twLogger_SetFunction);
}

extern twApi *tw_api;

TEST(unit_twLogger_SetFunction, test_twLogger_SetFunction) {
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetFunction(&doNothing));
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}