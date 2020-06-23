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

TEST_GROUP(unit_EncryptDES);

TEST_SETUP(unit_EncryptDES) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_EncryptDES) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_EncryptDES) {
	RUN_TEST_CASE(unit_EncryptDES, test_crypto_wrapper_DecryptDES);
	RUN_TEST_CASE(unit_EncryptDES, test_crypto_wrapper_check_Both_EncryptAndDecrypt_With_DifferentKey);
}

TEST(unit_EncryptDES, test_crypto_wrapper_DecryptDES) {
	const unsigned char *sym_key = "keytoencrypt";
	unsigned char key[8];
	unsigned char plainText1[8];
	const unsigned char * plainText = "Crypto.";
	unsigned char cipherText[8];
	createDESKey(sym_key, key);
	TEST_ASSERT_NOT_NULL(key);
	TEST_ASSERT_EQUAL(TW_OK, EncryptDES(key, cipherText, plainText));
	TEST_ASSERT_EQUAL(TW_OK, DecryptDES(key, cipherText, plainText1));
	TEST_ASSERT_EQUAL_STRING(plainText, plainText1);
}

/* Decrypt with different keys */
TEST(unit_EncryptDES, test_crypto_wrapper_check_Both_EncryptAndDecrypt_With_DifferentKey) {
	const unsigned char *sym_key = "keytoencrypt";
	const unsigned char *key1= "abcdef";
	unsigned char key[8];
	unsigned char newkey[8];
	const unsigned char * plainText = "Crypto.";
	unsigned char plainText1[8];
	unsigned char plainText2[8];
	unsigned char cipherText[8];
	createDESKey(sym_key, key);
	createDESKey(key1, newkey);
	TEST_ASSERT_EQUAL(8, sizeof(key));
	TEST_ASSERT_NOT_NULL(key);
	TEST_ASSERT_EQUAL(TW_OK, EncryptDES(key, cipherText, plainText));
	TEST_ASSERT_EQUAL(TW_OK, DecryptDES(key, cipherText, plainText1));
	TEST_ASSERT_EQUAL(TW_OK, DecryptDES(newkey, cipherText, plainText2));
	TEST_ASSERT_NOT_EQUAL(0, strcmp(plainText, plainText2));
}