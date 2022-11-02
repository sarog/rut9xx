#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogger_SetIsVerbose);

TEST_SETUP(unit_twLogger_SetIsVerbose) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogger_SetIsVerbose) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogger_SetIsVerbose) {
	RUN_TEST_CASE(unit_twLogger_SetIsVerbose, test_twLogger_SetIsVerbose);
}

extern twApi *tw_api;

TEST(unit_twLogger_SetIsVerbose, test_twLogger_SetIsVerbose) {
	twLogger *logger = twLogger_Instance();
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetIsVerbose(TRUE));
	TEST_ASSERT_EQUAL(TRUE, logger->isVerbose);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetIsVerbose(FALSE));
	TEST_ASSERT_EQUAL(FALSE, logger->isVerbose);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}
