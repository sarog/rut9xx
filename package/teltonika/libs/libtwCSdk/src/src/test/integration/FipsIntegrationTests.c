/**
 * FIPS Integration Tests
 */

#include "twTls.h"
#include "twExt.h"
#include "TestUtilities.h"
#include "unity.h"
#include "twThreads.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twMacros.h"

/**
 * API
 */
#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

static int fipsEnabled;

/**
 * Unity Test Macros
 */
TEST_GROUP(FipsIntegration);
TEST_SETUP(FipsIntegration) {
	eatLogs();
	twcfgResetAll();

#ifdef ENABLE_FIPS_MODE
	fipsEnabled = 1;
#else
	fipsEnabled = 0;
#endif
}
TEST_TEAR_DOWN(FipsIntegration) {
	twApi_Delete();
}

TEST_GROUP_RUNNER(FipsIntegration) {
	RUN_TEST_CASE(FipsIntegration, noFipsManualCipherConnect);
	RUN_TEST_CASE(FipsIntegration, noFipsPlatformConnect);
	RUN_TEST_CASE(FipsIntegration, fipsOnlyPlatformConnect);
	RUN_TEST_CASE(FipsIntegration, defaultCiphersPlatformConnect);
}

void test_FipsIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST(FipsIntegration, noFipsManualCipherConnect) {
	TEST_IGNORE_MESSAGE("CSDK-1538: Test needs to be rewritten, FIPS client does not connect on port 4444");
	if (fipsEnabled) {
		/* Set list of ciphers that are none fips and only work on a specific platform TW_PORT_NO_FIPS_CIPHERS */
		twcfg_pointer->cipher_set = "RC4-SHA:RC4-MD5";

		TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_FIPS_CIPHERS, TW_URI,
												  test_FipsIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));
        /* Load CA so we dont run into certificate issues for the test */
        TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
		/* Just incase fips was previously left on, make sure its disabled
		TEST_ASSERT_EQUAL(TW_OK, twApi_DisableFipsMode());
        /* Connect */
        TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
        /* Check that we're both connected and authenticated */
        TEST_ASSERT_TRUE(twApi_isConnected());

        TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
        /* Clean up */
        twApi_Disconnect("End Test");
        TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
        twcfgResetAll();
	} else {
        /* Ignore test for non-FIPS build */
		TEST_IGNORE_MESSAGE("Ignoring FIPS based test for non-FIPS build, only supported on openssl-fips build");
	}
}

TEST(FipsIntegration, noFipsPlatformConnect) {
	TEST_IGNORE_MESSAGE("CSDK-1538: Test needs to be rewritten, FIPS client does not connect on port 4444");
	if (fipsEnabled) {
        /* Enable FIPS mode if openssl build */
        TEST_ASSERT_EQUAL(TW_OK,twApi_EnableFipsMode());
        /* Check if FIPS is enabled */
        TEST_ASSERT_TRUE(twApi_IsFipsModeEnabled());
		
		TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_FIPS_CIPHERS, TW_URI,
												  test_FipsIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));

        /* Load CA so we dont run into certificate issues for the test */
        TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
        /* Try to connect, expect error */
        TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));

        TEST_ASSERT_FALSE(twApi_isConnected());
    
        TEST_ASSERT_FALSE(twApi_GetIsAuthenticated());
		/* Disable fips to not mess up other tests */
		TEST_ASSERT_EQUAL(TW_OK, twApi_DisableFipsMode());
        
		/* Clean up */
        twApi_Disconnect("End Test");

		TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	} else {
        /* Ignore test for non-FIPS build */
		TEST_IGNORE_MESSAGE("Ignoring FIPS based test for non-FIPS build, only supported on openssl-fips build");
	}
}

TEST(FipsIntegration, fipsOnlyPlatformConnect) {
	TEST_IGNORE_MESSAGE("CSDK-1538: Test needs to be rewritten, FIPS client does not connect on port 4444");
	if (fipsEnabled) {
        /* Enable FIPS mode if openssl build */
        TEST_ASSERT_EQUAL(TW_OK,TW_ENABLE_FIPS_MODE());
        /* Check if FIPS is enabled */
        TEST_ASSERT_TRUE(TW_IS_FIPS_MODE_ENABLED());

		TEST_ASSERT_EQUAL(TW_OK,
						  twApi_Initialize(TW_HOST, TW_PORT_SELF_SIGNED, TW_URI, test_FipsIntegrationAppKey_callback,
										   NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	
        /* Load CA so we dont run into certificate issues for the test */
        TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
        /* Connect */
        TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
        /* Check that we're both connected and authenticated */
        TEST_ASSERT_TRUE(twApi_isConnected());
        TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
        /* Clean up */
        twApi_Disconnect("End Test");
		TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	} else {
        /* Ignore test for non-FIPS build */
		TEST_IGNORE_MESSAGE("Ignoring FIPS based test for non-FIPS build, only supported on openssl-fips build");
	}

}

TEST(FipsIntegration, defaultCiphersPlatformConnect) {
	TEST_IGNORE_MESSAGE("CSDK-1538: Test needs to be rewritten, FIPS client does not connect on port 4444");
	if (fipsEnabled) {
		/* Sets the cipher_set config to NULL, this means the API should auto select a default cipher  configuration */
		twcfg_pointer->cipher_set = NULL;
        /* Enable FIPS mode if openssl build */
        TEST_ASSERT_EQUAL(TW_OK,twApi_EnableFipsMode());
        /* Check if FIPS is enabled */
        TEST_ASSERT_TRUE(twApi_IsFipsModeEnabled());
		
		TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_DEFAULT_CIPHERS, TW_URI,
												  test_FipsIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));
	
        /* Load CA so we dont run into certificate issues for the test */
        TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
        /* Connect */
        TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
        /* Check that we're both connected and authenticated */
        TEST_ASSERT_TRUE(twApi_isConnected());
        TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
        /* Clean up */
		TEST_ASSERT_EQUAL(TW_OK, twApi_DisableFipsMode());
        twApi_Disconnect("End Test");

		TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	} else {
        /* Ignore test for non-FIPS build */
		TEST_IGNORE_MESSAGE("Ignoring FIPS based test for non-FIPS build, only supported on openssl-fips build");
	}
}