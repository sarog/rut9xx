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


TEST_GROUP(unit_TWSHA256BinToHex);

TEST_SETUP(unit_TWSHA256BinToHex) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_TWSHA256BinToHex) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_TWSHA256BinToHex) {
	RUN_TEST_CASE(unit_TWSHA256BinToHex, test_simpleTWSHA256BinToHex);
}

extern twApi *tw_api;

/**
 * Test Plan: Feed a simple binary value into this function and read back the hex equivalent.
 */
TEST(unit_TWSHA256BinToHex, test_simpleTWSHA256BinToHex) {
#ifdef TW_USING_AXTLS
	TEST_IGNORE_MESSAGE("Test disabled for AXTLS builds etc etc");
#else
	unsigned char binary[32]={ 10, 15, 10, 15, 10, 15, 10, 15, 10, 15,
	                           10, 15, 10, 15, 10, 15, 10, 15, 10, 15,
	                           10, 15, 10, 15, 10, 15, 10, 15, 10, 15,
	                           10,11};
	unsigned char hexidecimal[65];
	TWSHA256BinToHex(binary,hexidecimal);
	TEST_ASSERT_EQUAL_STRING("0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0f0a0b", hexidecimal);
#endif
}