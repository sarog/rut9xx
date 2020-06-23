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
#include "string.h"
#include <stdio.h>
#include <ctype.h>

TEST_GROUP(unit_lowercase);

TEST_SETUP(unit_lowercase) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_lowercase) {
}

TEST_GROUP_RUNNER(unit_lowercase) {
	RUN_TEST_CASE(unit_lowercase, test_stringUtils_lowercase);
}

TEST(unit_lowercase, test_stringUtils_lowercase) {
	char *string1 = NULL;
	char *string2 = NULL;

	TEST_ASSERT_NULL(lowercase(NULL));
	string1 = (char*)TW_MALLOC(sizeof(char) * strlen(TEST_STRING) + 1);
	string2 = (char*)TW_MALLOC(sizeof(char) * strlen(TEST_STRING) + 1);
	strcpy(string1, TEST_STRING);
	strcpy(string2, TEST_STRING);
	TEST_ASSERT_EQUAL_STRING(TEST_STRING_LOWER, lowercase(string2));
	TW_FREE(string1);
	TW_FREE(string2);
}