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

TEST_GROUP(unit_duplicateString);

TEST_SETUP(unit_duplicateString) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_duplicateString) {
}

TEST_GROUP_RUNNER(unit_duplicateString) {
	RUN_TEST_CASE(unit_duplicateString, test_stringUtils_duplicateString);
	RUN_TEST_CASE(unit_duplicateString, test_stringUtils_duplicateString_maxlen);
}

TEST(unit_duplicateString, test_stringUtils_duplicateString) {
	char *string1 = NULL;
	char *string2 = NULL;
	char *string3 = NULL;
	char *string4 = NULL;

	TEST_ASSERT_NULL(duplicateString(NULL));
	string1 = (char*)malloc(sizeof(char) * strlen(TEST_STRING) + 1);
	string2 = NULL;
	string3 = (char*)malloc(sizeof(char) * strlen(TEST_STRING) + 1);
	string4 = NULL;
	strcpy(string1, TEST_STRING);
	string2 = duplicateString(TEST_STRING);
	TEST_ASSERT_EQUAL_STRING(string1, string2);
	strcpy(string3, string1);
	string4 = duplicateString(string2);
	TEST_ASSERT_EQUAL_STRING(string3, string4);
	free(string1);
	free(string2);
	free(string3);
	free(string4);
}

TEST (unit_duplicateString, test_stringUtils_duplicateString_maxlen) {
	char *input = NULL;
	char *output = NULL;
	const unsigned huge_string_length = 100000;
	const unsigned max_len1 = 1234;						/* Truncation */
	const unsigned max_len2 = huge_string_length * 2; /* Whole string */

	/* Create a relatively large untrusted input string */
	input = TW_CALLOC (huge_string_length + 1, 1);
	memset (input, 'A', huge_string_length);

	/* Duplicate the first N characters of it, causing truncation*/
	output = duplicateStringN (input, max_len1);
	TEST_ASSERT_EQUAL (max_len1, strlen(output));
	TW_FREE (output);

	/* Duplicate the string, allowing for the whole string */
	output = duplicateStringN (input, max_len2);
	TEST_ASSERT_EQUAL (huge_string_length, strlen (output));
	TW_FREE (output);

	TW_FREE (input);
}