#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twCharacteristicToString);

TEST_SETUP(unit_twCharacteristicToString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twCharacteristicToString) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twCharacteristicToString) {
	RUN_TEST_CASE(unit_twCharacteristicToString, test_twLogger_twCharacteristicToString);
}

extern twApi *tw_api;

TEST(unit_twCharacteristicToString, test_twLogger_twCharacteristicToString) {
	TEST_ASSERT_EQUAL_STRING("PROPERTIES", twCharacteristicToString(TW_PROPERTIES));
	TEST_ASSERT_EQUAL_STRING("SERVICES", twCharacteristicToString(TW_SERVICES));
	TEST_ASSERT_EQUAL_STRING("EVENTS", twCharacteristicToString(TW_EVENTS));
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", twCharacteristicToString(-1));
}