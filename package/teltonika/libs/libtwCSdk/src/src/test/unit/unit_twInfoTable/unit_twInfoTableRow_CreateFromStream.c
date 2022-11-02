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

TEST_GROUP(unit_twInfoTableRow_CreateFromStream);

TEST_SETUP(unit_twInfoTableRow_CreateFromStream) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twInfoTableRow_CreateFromStream) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twInfoTableRow_CreateFromStream) {
	RUN_TEST_CASE(unit_twInfoTableRow_CreateFromStream, test_twInfoTableRow_CreateFromStream_Invalid_Input);
	RUN_TEST_CASE(unit_twInfoTableRow_CreateFromStream, test_twInfoTableRow_CreateFromStream);
}

TEST(unit_twInfoTableRow_CreateFromStream, test_twInfoTableRow_CreateFromStream_Invalid_Input) {
	TEST_ASSERT_NULL(twInfoTableRow_CreateFromStream(NULL));
}

TEST(unit_twInfoTableRow_CreateFromStream, test_twInfoTableRow_CreateFromStream) {
	twStream *stream = NULL;
	twInfoTableRow *row = NULL;
	ListEntry *entry = NULL;

	// Create initial twInfoTableRow
	row = twInfoTableRow_Create(twPrimitive_CreateFromInteger(42));
	TEST_ASSERT_NOT_NULL(row);

	// Create stream for testing twInfoTableRow_CreateFromStream
	stream = twStream_Create();
	twInfoTableRow_ToStream(row, stream);
	twInfoTableRow_Delete(row);
	TEST_ASSERT_NOT_NULL(stream);

	twStream_Reset(stream);
	row = twInfoTableRow_CreateFromStream(stream);
	TEST_ASSERT_NOT_NULL(row);
	TEST_ASSERT_EQUAL_INT(1, twInfoTableRow_GetCount(row));

	entry = twList_GetByIndex(row->fieldEntries, 0);
	TEST_ASSERT_EQUAL_INT(42, ((twPrimitive *)entry->value)->val.integer);

	twInfoTableRow_Delete(row);
	twStream_Delete(stream);
}
