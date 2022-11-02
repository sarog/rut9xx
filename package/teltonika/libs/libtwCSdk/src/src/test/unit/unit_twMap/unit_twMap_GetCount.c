#include <twTls.h>
#include <cfuhash.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static char parseHandlerCalled;
static const char *parseString(void *item) {
	char *string;
		string = (char *)item;
		parseHandlerCalled = TRUE;
		return duplicateString(string);
}

static char deleteHandlerCalled;
static void deleteString(void *item) {
	char *string = (char *)item;
	deleteHandlerCalled = TRUE;
	TW_FREE(string);
}

TEST_GROUP(unit_twMap_GetCount);

TEST_SETUP(unit_twMap_GetCount) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twMap_GetCount) {
}

TEST_GROUP_RUNNER(unit_twMap_GetCount) {
	RUN_TEST_CASE(unit_twMap_GetCount, getCountFromNullMap);
	RUN_TEST_CASE(unit_twMap_GetCount, verifyValidCount);
}

/**
* Test Plan: Attempts to GetCount from NULL map
*/
TEST(unit_twMap_GetCount, getCountFromNullMap) {
	TEST_ASSERT_EQUAL(0, twMap_GetCount(NULL));
}

/**
* Test Plan: Adds a value to the map, checks the number of items in the map, clears the map then
* check that the number of items in the map is 0.
*/
TEST(unit_twMap_GetCount, verifyValidCount) {
	int i = 0;
	twMap *map = NULL;
	map = twMap_Create(deleteString, parseString);
	for (i = 0; i <= 4; i++) {
		char *string = twItoa(i);
		TEST_ASSERT_EQUAL(TW_OK, twMap_Add(map, string));
		TEST_ASSERT_EQUAL(i+1, twMap_GetCount(map));
	}
	for (i = 4; i >= 0; i--) {
		char *string = twItoa(i);
		TEST_ASSERT_EQUAL(TW_OK, twMap_Remove(map, string, TRUE));
		TEST_ASSERT_EQUAL(i, twMap_GetCount(map));
	}
	deleteHandlerCalled = FALSE;
	TEST_ASSERT_FALSE(deleteHandlerCalled);
	TEST_ASSERT_EQUAL(TW_MAP_OK, twMap_Clear(map));
	TEST_ASSERT_EQUAL(0, twMap_GetCount(map));
	TEST_ASSERT_EQUAL(TW_OK, twMap_Delete(map));
	TEST_ASSERT_FALSE(deleteHandlerCalled);
}