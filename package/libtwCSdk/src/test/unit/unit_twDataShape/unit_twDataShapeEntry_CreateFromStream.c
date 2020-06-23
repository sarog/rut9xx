#include <twTls.h>
#include "twApi.h"
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

TEST_GROUP(unit_twDataShapeEntry_CreateFromStream);

TEST_SETUP(unit_twDataShapeEntry_CreateFromStream) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twDataShapeEntry_CreateFromStream) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twDataShapeEntry_CreateFromStream) {
	RUN_TEST_CASE(unit_twDataShapeEntry_CreateFromStream, test_twDataShapeEntry_CreateFromStream_Invalid_Input);
	RUN_TEST_CASE(unit_twDataShapeEntry_CreateFromStream, test_twDataShapeEntry_CreateFromStream);
}

TEST(unit_twDataShapeEntry_CreateFromStream, test_twDataShapeEntry_CreateFromStream_Invalid_Input) {
	TEST_ASSERT_NULL(twDataShapeEntry_CreateFromStream(NULL));
}

TEST(unit_twDataShapeEntry_CreateFromStream, test_twDataShapeEntry_CreateFromStream) {
	twStream *stream = NULL;
	twDataShapeEntry *entry = NULL;
	ListEntry *aspects = NULL;

	/* Create initial twDataShapeEntry with one aspect */
	entry = twDataShapeEntry_Create("DataShapeEntry", "A test DataShapeEntry.", TW_STRING);
	twDataShapeEntry_AddAspect(entry, "isPrimaryKey", twPrimitive_CreateFromBoolean("true"));
	TEST_ASSERT_NOT_NULL(entry);

	/* Create stream for testing twDataShapeEntry_CreateFromStream */
	stream = twStream_Create();
	twDataShapeEntry_ToStream(entry, stream);
	twDataShapeEntry_Delete(entry);
	TEST_ASSERT_NOT_NULL(stream);

	twStream_Reset(stream);
	entry = twDataShapeEntry_CreateFromStream(stream);
	TEST_ASSERT_NOT_NULL(entry);

	TEST_ASSERT_EQUAL_STRING("DataShapeEntry", entry->name);
	TEST_ASSERT_EQUAL_STRING("A test DataShapeEntry.", entry->description);
	TEST_ASSERT_EQUAL(TW_STRING, entry->type);
	TEST_ASSERT_EQUAL_INT(1, twList_GetCount(entry->aspects));

	aspects = twList_GetByIndex(entry->aspects, 0);
	TEST_ASSERT_EQUAL_STRING("isPrimaryKey", ((twDataShapeAspect *)aspects->value)->name);
	TEST_ASSERT_TRUE(((twDataShapeAspect *)aspects->value)->value);

	twDataShapeEntry_Delete(entry);
	twStream_Delete(stream);
}