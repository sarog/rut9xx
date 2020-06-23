#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogger_SetLevel);

TEST_SETUP(unit_twLogger_SetLevel) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogger_SetLevel) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogger_SetLevel) {
	RUN_TEST_CASE(unit_twLogger_SetLevel, test_twLogger_SetLevel);
}

extern twApi *tw_api;

TEST(unit_twLogger_SetLevel, test_twLogger_SetLevel) {
	twLogger *logger = twLogger_Instance();
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_TRACE));
	TEST_ASSERT_EQUAL(TW_TRACE, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_DEBUG));
	TEST_ASSERT_EQUAL(TW_DEBUG, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_INFO));
	TEST_ASSERT_EQUAL(TW_INFO, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_WARN));
	TEST_ASSERT_EQUAL(TW_WARN, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_ERROR));
	TEST_ASSERT_EQUAL(TW_ERROR, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_FORCE));
	TEST_ASSERT_EQUAL(TW_FORCE, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_AUDIT));
	TEST_ASSERT_EQUAL(TW_AUDIT, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_TRACE));
	TEST_ASSERT_EQUAL(TW_TRACE, logger->level);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}