#include "twTls.h"
#include "cfuhash.h"
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "crypto_wrapper.h"
#include "twApiStubs.h"
#include "twPrimitiveUtils.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
#else
extern twApi_Stubs * twApi_stub;
#endif

TEST_GROUP(unit_MD4Hash);

TEST_SETUP(unit_MD4Hash) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_MD4Hash) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_MD4Hash) {
	RUN_TEST_CASE(unit_MD4Hash, test_crypto_wrapper_MD4Hash);
}

TEST(unit_MD4Hash, test_crypto_wrapper_MD4Hash) {
	unsigned char hash[16] = {0};
	unsigned char expected_hash[16] = {0xbd, 0xe5, 0x2c, 0xb3, 0x1d, 0xe3, 0x3e, 0x46, 0x24, 0x5e, 0x05, 0xfb, 0xdb, 0xd6, 0xfb, 0x24};
	int i;
	TEST_ASSERT_EQUAL(0, MD4Hash("a", 1, hash));
	TEST_ASSERT_NOT_NULL(hash);
	for(i = 0; i<16; i++) {
		TEST_ASSERT_EQUAL(expected_hash[i], hash[i]);
	}
}