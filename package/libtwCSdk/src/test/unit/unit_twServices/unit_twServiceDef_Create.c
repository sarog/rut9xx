#include <twTls.h>
#include "twApi.h"
#include "twProperties.h"
#include "twServices.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twServiceDef_Create);

TEST_SETUP(unit_twServiceDef_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twServiceDef_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twServiceDef_Create) {
	RUN_TEST_CASE(unit_twServiceDef_Create, test_twServices_twServiceDef_Create_Delete);
}

extern twApi *tw_api;

TEST(unit_twServiceDef_Create, test_twServices_twServiceDef_Create_Delete) {
	twServiceDef *def = NULL;
	TEST_ASSERT_EQUAL(NULL, twServiceDef_Create(NULL, TEST_SERVICE_DESCRIPTION, NULL, TW_NOTHING, NULL));
	def = twServiceDef_Create(TEST_SERVICE_NAME, TEST_SERVICE_DESCRIPTION, NULL, TW_NOTHING, NULL);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, def->name);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_DESCRIPTION, def->description);
	TEST_ASSERT_NULL(def->inputs);
	TEST_ASSERT_NULL(def->outputDataShape);
	TEST_ASSERT_EQUAL(TW_NOTHING, def->outputType);
	TEST_ASSERT_NOT_NULL(def->aspects);
	TEST_ASSERT_EQUAL_STRING(TEST_SERVICE_NAME, def->name);
	twServiceDef_Delete(def);
}
