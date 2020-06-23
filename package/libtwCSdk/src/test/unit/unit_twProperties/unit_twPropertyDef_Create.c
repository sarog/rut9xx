#include <twTls.h>
#include "twApi.h"
#include "twProperties.h"
#include "twServices.h"
#include "twVersion.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif


TEST_GROUP(unit_twPropertyDef_Create);

TEST_SETUP(unit_twPropertyDef_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twPropertyDef_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twPropertyDef_Create) {
	RUN_TEST_CASE(unit_twPropertyDef_Create, test_twProperties_twPropertyDef_Create_Invalid_Input);
	RUN_TEST_CASE(unit_twPropertyDef_Create, test_twProperties_twPropertyDef_Create_Delete);
}

char * stub_unit_twPropertyDef_CreateTests_streamToString(twStream *s) {
	return duplicateString(TEST_DATA);
}

twPrimitive *stub_unit_twPropertyDef_CreateTests_twPrimitive_CreateFromStream(twStream *s) {
	return twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
}

TEST(unit_twPropertyDef_Create, test_twProperties_twPropertyDef_Create_Invalid_Input) {
	TEST_ASSERT_NULL(twPropertyDef_Create(NULL, TW_INTEGER, NULL, "ALWAYS", 0));
}

TEST(unit_twPropertyDef_Create, test_twProperties_twPropertyDef_Create_Delete) {
	twPropertyDef *def = NULL;
	def = twPropertyDef_Create(TEST_PROPERTY_NAME, TW_INTEGER, NULL, "ALWAYS", 0);
	TEST_ASSERT_NOT_NULL(def);
	twPropertyDef_Delete(def);
}