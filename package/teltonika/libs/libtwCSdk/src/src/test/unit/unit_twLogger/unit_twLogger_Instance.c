#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogger_Instance);

TEST_SETUP(unit_twLogger_Instance) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogger_Instance) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogger_Instance) {
	RUN_TEST_CASE(unit_twLogger_Instance, test_twLogger_Instance_Delete);
	RUN_TEST_CASE(unit_twLogger_Instance, test_twLogger_Delete_Twice);
}

extern twApi *tw_api;

TEST(unit_twLogger_Instance, test_twLogger_Instance_Delete) {
	/* this will create a new logger singleton */
	twLogger *logger = twLogger_Instance();
	/* should return the same logger singleton */
	TEST_ASSERT_EQUAL(logger, twLogger_Instance());
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}

TEST(unit_twLogger_Instance, test_twLogger_Delete_Twice) {
	TEST_ASSERT_NOT_NULL(twLogger_Instance());
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
}