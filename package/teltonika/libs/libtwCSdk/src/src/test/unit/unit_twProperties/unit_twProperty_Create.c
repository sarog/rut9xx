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


TEST_GROUP(unit_twProperty_Create);

TEST_SETUP(unit_twProperty_Create) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twProperty_Create) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twProperty_Create) {
	RUN_TEST_CASE(unit_twProperty_Create, test_twProperties_twProperty_Create_Invalid_Input);
	RUN_TEST_CASE(unit_twProperty_Create, test_twProperties_twProperty_CreateFromStream_Invalid_Input);
	RUN_TEST_CASE(unit_twProperty_Create, test_twProperties_twProperty_Create_Delete);
	RUN_TEST_CASE(unit_twProperty_Create, test_twProperties_twProperty_CreateFromStream_Delete_Stubbed);
}

char * stub_unit_twProperty_CreateTests_streamToString(twStream *s) {
	return duplicateString(TEST_DATA);
}

twPrimitive *stub_unit_twProperty_CreateTests_twPrimitive_CreateFromStream(twStream *s) {
	return twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
}

TEST(unit_twProperty_Create, test_twProperties_twProperty_Create_Invalid_Input) {
	TEST_ASSERT_NULL(twProperty_Create(NULL, 0, NULL));
	TEST_ASSERT_NULL(twProperty_Create(TEST_PROPERTY_NAME, NULL, NULL));
}

TEST(unit_twProperty_Create, test_twProperties_twPropertyVTQ_Create_Invalid_Input) {
	TEST_ASSERT_NULL(twPropertyVTQ_Create(NULL, 0, NULL, "GOOD"));
	TEST_ASSERT_NULL(twPropertyVTQ_Create(TEST_PROPERTY_NAME, NULL, NULL, "GOOD"));
}

TEST(unit_twProperty_Create, test_twProperties_twProperty_CreateFromStream_Invalid_Input) {
	TEST_ASSERT_NULL(twProperty_CreateFromStream(NULL));
}

TEST(unit_twProperty_Create, test_twProperties_twProperty_Create_Delete) {
	twPrimitive *prim = NULL;
	twProperty *prop = NULL;

	prim = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_NOT_NULL(prim);
	prop = twProperty_Create(TEST_PROPERTY_NAME, prim, NULL);
	TEST_ASSERT_NOT_NULL(prop);
	twProperty_Delete(prop);
}

TEST(unit_twProperty_Create, test_twProperties_twProperty_CreateFromStream_Delete_Stubbed) {
	int err = 0;
	twStream *stream = NULL;
	twProperty *prop = NULL;

	/* use stubs to mock stream manipulation */
	err = twStubs_Use();
	if (err) {
		/* stubs is not enabled, exit test */
		return;
	}
	twApi_stub->streamToString = stub_unit_twProperty_CreateTests_streamToString;
	twApi_stub->twPrimitive_CreateFromStream = stub_unit_twProperty_CreateTests_twPrimitive_CreateFromStream;
	stream = twStream_Create();
	prop = twProperty_CreateFromStream(stream);
	TEST_ASSERT_NOT_NULL(prop);
	twProperty_Delete(prop); /* malloc: *** error for object 0x108e0d5c9: pointer being freed was not allocated */
	twStream_Delete(stream);
}
