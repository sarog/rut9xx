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

TEST_GROUP(unit_concatenateStrings);

TEST_SETUP(unit_concatenateStrings) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_concatenateStrings) {
}

TEST_GROUP_RUNNER(unit_concatenateStrings) {
	RUN_TEST_CASE(unit_concatenateStrings, test_stringUtils_concatenateStrings);
	RUN_TEST_CASE(unit_concatenateStrings, test_stringUtils_concatenateStrings_maxlen);
}

TEST(unit_concatenateStrings, test_stringUtils_concatenateStrings) {
	char *string1 = NULL;
	char *string2 = NULL;

	/* Verify passing NULL params in will fail with Invalid Parameter */
	TEST_ASSERT_EQUAL(concatenateStrings(NULL, NULL),TW_INVALID_PARAM);

	string1 = (char*)malloc(sizeof(char) * strlen(TEST_STRING_LOWER) + 1);
	string2 = (char*)malloc(sizeof(char) * strlen(TEST_STRING_UPPER) + 1);
	strcpy(string1, TEST_STRING_LOWER);
	strcpy(string2, TEST_STRING_UPPER);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&string1, string2));
	TEST_ASSERT_EQUAL_STRING(TEST_STRING_CAT, string1);

	TW_FREE(string1);
	TW_FREE(string2);
}

TEST (unit_concatenateStrings, test_stringUtils_concatenateStrings_maxlen) {
	char *input = NULL;
	char *output = NULL;
	const unsigned max_len1 = 1234;						/* Truncation */
	const unsigned max_len2 = 100000;                   /* Whole string */

	/* Create a relatively large untrusted input string */
	input = TW_CALLOC (max_len2 + 1, 1);
	memset (input, 'A', max_len2);

	/* Concatenate to a pre-exisiting string, which will be truncated*/
	output = (char*)malloc(sizeof(char) * strlen(TEST_STRING_LOWER) + 1);
	strcpy(output, TEST_STRING_LOWER);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStringsN (&output, input, max_len1));
	TEST_ASSERT_EQUAL (max_len1, strlen(output));

	/* Now allow for the whole string */
	TEST_ASSERT_EQUAL(TW_OK, concatenateStringsN (&output, input, max_len2 * 2));
	TEST_ASSERT_TRUE (max_len2 + max_len1 == strlen (output));

	/* Now try to cat a string that is already the max length */
	TEST_ASSERT_EQUAL(TW_OK, concatenateStringsN (&output, input, max_len2));
	TEST_ASSERT_TRUE (max_len2 == strlen (output));

	/* Decreasing the size should just truncate the string */
	TEST_ASSERT_EQUAL(TW_OK, concatenateStringsN (&output, input, max_len1));
	TEST_ASSERT_TRUE (max_len1 == strlen (output));

	TW_FREE (output);
	TW_FREE (input);
}