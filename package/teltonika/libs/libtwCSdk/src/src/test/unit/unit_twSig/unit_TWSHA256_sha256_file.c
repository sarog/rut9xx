/*
 *  Copyright 2018, PTC, Inc.
 *
 */
#include "twExt.h"
#include "twVersion.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twSig.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif


TEST_GROUP(unit_TWSHA256_sha256_file);

TEST_SETUP(unit_TWSHA256_sha256_file) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_TWSHA256_sha256_file) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_TWSHA256_sha256_file) {
	RUN_TEST_CASE(unit_TWSHA256_sha256_file, test_TWSHA256_sha256_abc_file);
	RUN_TEST_CASE(unit_TWSHA256_sha256_file, test_TWSHA256_sha256_file);
}

extern twApi *tw_api;

/**
 * Test Plan: Generate a SHA256 digest for a well known binary file and compare it to what is expected.
 * See https://www.di-mgt.com.au/sha_testvectors.html
 */
TEST(unit_TWSHA256_sha256_file, test_TWSHA256_sha256_abc_file) {
#ifdef TW_USING_AXTLS
	TEST_IGNORE_MESSAGE("Test disabled for AXTLS builds etc etc");
#else
	unsigned char hexidecimal[65];
	char * pathToFile = NULL;
	if (twApi_IsFipsModeEnabled) {
		TEST_IGNORE_MESSAGE("Test disabled, low level API call to digest SHA256 forbidden in FIPS mode");
	}
	pathToFile = duplicateString(getConfigDir());
	concatenateStrings(&pathToFile,"/abc.txt");
	TEST_ASSERT_EQUAL(TW_OK,TWSHA256_sha256_file(pathToFile, (char*)hexidecimal));
	TEST_ASSERT_EQUAL_STRING("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", hexidecimal);
	TW_FREE(pathToFile);
#endif
}


/**
 * Test Plan: Generate a SHA256 digest for a binary file and compare it to what is expected.
 */
TEST(unit_TWSHA256_sha256_file, test_TWSHA256_sha256_file) {
#ifdef TW_USING_AXTLS
	TEST_IGNORE_MESSAGE("Test disabled for AXTLS builds etc etc");
#else
	unsigned char hexidecimal[65];
	char * pathToFile = NULL;
	if (twApi_IsFipsModeEnabled) {
		TEST_IGNORE_MESSAGE("Test disabled, low level API call to digest SHA256 forbidden in FIPS mode");
	}
	pathToFile = duplicateString(getConfigDir());
	concatenateStrings(&pathToFile,"/simplePayload.zip");
	TEST_ASSERT_EQUAL(TW_OK,TWSHA256_sha256_file(pathToFile, (char*)hexidecimal));
	TEST_ASSERT_EQUAL_STRING("f0d243ca743d600768b9a264a72521278c7e85ff0e759fddfb6f3c5e8660d9ff", hexidecimal);
	TW_FREE(pathToFile);
#endif
}