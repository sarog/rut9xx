#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_levelString);

TEST_SETUP(unit_levelString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_levelString) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_levelString) {
	RUN_TEST_CASE(unit_levelString, test_twLogger_levelString);
}

extern twApi *tw_api;

TEST(unit_levelString, test_twLogger_levelString) {
	TEST_ASSERT_EQUAL_STRING("TRACE", levelString(TW_TRACE));
	TEST_ASSERT_EQUAL_STRING("DEBUG", levelString(TW_DEBUG));
	TEST_ASSERT_EQUAL_STRING("INFO", levelString(TW_INFO));
	TEST_ASSERT_EQUAL_STRING("WARN", levelString(TW_WARN));
	TEST_ASSERT_EQUAL_STRING("ERROR", levelString(TW_ERROR));
	TEST_ASSERT_EQUAL_STRING("FORCE", levelString(TW_FORCE));
	TEST_ASSERT_EQUAL_STRING("AUDIT", levelString(TW_AUDIT));
	TEST_ASSERT_EQUAL_STRING("TRACE", levelString(TW_TRACE));
	TEST_ASSERT_EQUAL_STRING("UNKNOWN", levelString(-1));
}