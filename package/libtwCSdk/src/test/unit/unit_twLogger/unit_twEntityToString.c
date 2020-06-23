#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twEntityToString);

TEST_SETUP(unit_twEntityToString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twEntityToString) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twEntityToString) {
	RUN_TEST_CASE(unit_twEntityToString, test_twLogger_twEntityToString);
}

extern twApi *tw_api;

TEST(unit_twEntityToString, test_twLogger_twEntityToString) {
	TEST_ASSERT_EQUAL_STRING("THING", twEntityToString(TW_THING));
	TEST_ASSERT_EQUAL_STRING("RESOURCE", twEntityToString(TW_RESOURCE));
	TEST_ASSERT_EQUAL_STRING("SUBSYSTEM", twEntityToString(TW_SUBSYSTEM));
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", twEntityToString(-1));
}
