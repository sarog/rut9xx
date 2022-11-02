#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLog);

TEST_SETUP(unit_twLog) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLog) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLog) {
	RUN_TEST_CASE(unit_twLog, test_twLogger_twLog);
}

extern twApi *tw_api;


TEST(unit_twLog, test_twLogger_twLog) {
	twLog(TW_TRACE, "test_twLogger_twLog");
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}