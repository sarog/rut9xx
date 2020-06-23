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


TEST_GROUP(unit_decryptSignatureRsaFile);

TEST_SETUP(unit_decryptSignatureRsaFile) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_decryptSignatureRsaFile) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_decryptSignatureRsaFile) {
	RUN_TEST_CASE(unit_decryptSignatureRsaFile, test_decryptSignatureRsaFile);

}

/**
 * Test Plan: Decrypt a sample RSA encrypted file and verify its result.
 */
TEST(unit_decryptSignatureRsaFile, test_decryptSignatureRsaFile) {
#ifndef TW_USING_AXTLS
	char decryptedHex[1000];
	char* encryptedData = "d6lo42EKj8L58wR1Qdg0fpz93ANPXGCUbV+TVuJqbGFKrEgHOWOKvm/FDbXpMSme+hIMFWMyMLT+aGVculWS5L0BobZoZUxGeyScgvrZcoWs62WK5FwHjZP9mbGd+fCseQBUWlw8YetQFEIc1SIMnlTdo91h40waFCDvCe+rYWCpIbF9WEYTu+WjGgK5C8rVjkZzFsgD6d6mgSpQQnDzJAcF8smvamrnFlAYa9O39Cdp1kcBAjN1ZTRun04okws4RECUc25Ne9nFcAME8TqaRqt5qyTpYxn3gHpJ0wNCaKBbrwhIknQQRHP5rF67fwO+87iRp5Uuvwa0nvp0mOw2mw==";
	char * publicKeyFilePath = duplicateString(getConfigDir());
	int ret=TW_OK;
	unsigned char decodedSignatureBuffer[400];
	unsigned long decodedSignatureLength=400;

	concatenateStrings(&publicKeyFilePath,"/test_public_key.pem");

	ret = base64_decode((const unsigned char*)encryptedData, 344, (unsigned char*)decodedSignatureBuffer, &decodedSignatureLength);
	TEST_ASSERT_EQUAL(TW_OK,ret);


	TEST_ASSERT_EQUAL(TW_OK, decryptSignatureRsaFile(publicKeyFilePath,(char*)decodedSignatureBuffer,decodedSignatureLength, decryptedHex,NULL));
	TEST_ASSERT_EQUAL_STRING("a7c6ba9dedabc489ec99195dfa87cfb5b5a6fc543fc0896408fa53d62f54495a", decryptedHex);
	TW_FREE(publicKeyFilePath);

#endif
}
