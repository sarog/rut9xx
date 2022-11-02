#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogHexString);

TEST_SETUP(unit_twLogHexString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogHexString) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogHexString) {
	RUN_TEST_CASE(unit_twLogHexString, test_twLogger_twLogHexString);
}

TEST(unit_twLogHexString, test_twLogger_twLogHexString) {
	twMessage * m = NULL;

	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetIsVerbose(TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_TRACE));
	m = twMessage_CreateRequestMsg(TWX_SUCCESS);
	twLogHexString(m, TEST_LOG_PREAMBLE, 0);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}