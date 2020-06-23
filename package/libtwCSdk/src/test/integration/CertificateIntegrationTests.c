/**
 * Certificate Integration Tests
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

/**
 * Unity Test Macros
 */
TEST_GROUP(CertificateIntegration);

void test_CertificateIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(CertificateIntegration) {
	eatLogs();
}
TEST_TEAR_DOWN(CertificateIntegration) {
	twApi_Delete();
}
TEST_GROUP_RUNNER(CertificateIntegration) {
	RUN_TEST_CASE(CertificateIntegration, disableEncryptionAndConnect);
	RUN_TEST_CASE(CertificateIntegration, disableCertValidationAndConnect);
	RUN_TEST_CASE(CertificateIntegration, allowSelfSignedCertsAndConnect);
	RUN_TEST_CASE(CertificateIntegration, loadCACertAndConnect);
	RUN_TEST_CASE(CertificateIntegration, disableCertValidationAndConnectNoRoot);
	RUN_TEST_CASE(CertificateIntegration, allowSelfSignedCertsAndConnectNoRoot);
	RUN_TEST_CASE(CertificateIntegration, loadCACertAndConnectNoRoot);
}

TEST(CertificateIntegration, disableEncryptionAndConnect) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_SSL, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_FALSE(tw_api->connectionInfo->disableEncryption);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	twApi_DisableEncryption();
	/* Check that we are not validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->connectionInfo->disableEncryption);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, disableCertValidationAndConnect) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_SELF_SIGNED, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	twApi_DisableCertValidation();
	/* Check that we are not validating and not allowing self signed certs */
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, allowSelfSignedCertsAndConnect) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_SELF_SIGNED, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	twApi_SetSelfSignedOk();
	/* Check that we are validating and are allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_TRUE(tw_api->connectionInfo->selfsignedOk);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, loadCACertAndConnect) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_SELF_SIGNED, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, disableCertValidationAndConnectNoRoot) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_ROOT_CERT_IN_CHAIN, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	twApi_DisableCertValidation();
	/* Check that we are not validating and not allowing self signed certs */
	TEST_ASSERT_FALSE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, allowSelfSignedCertsAndConnectNoRoot) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_ROOT_CERT_IN_CHAIN, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	twApi_SetSelfSignedOk();
	/* Check that we are validating and are allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_TRUE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	/* Clean up */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST(CertificateIntegration, loadCACertAndConnectNoRoot) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT_NO_ROOT_CERT_IN_CHAIN, TW_URI, test_CertificateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Try to connect, expect certificate error */
	TEST_ASSERT_EQUAL(TW_SOCKET_INIT_ERROR, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, 1));
	TEST_ASSERT_EQUAL(TW_OK, loadCACertFromEtc(TEST_CA_CERT_FILE));
	/* Check that we are validating and not allowing self signed certs */
	TEST_ASSERT_TRUE(tw_api->mh->ws->connection->validateCert);
	TEST_ASSERT_FALSE(tw_api->connectionInfo->selfsignedOk);
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));
	/* Check that we're both connected and authenticated */
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_GetIsAuthenticated());
	/* Clean up */
	twApi_Disconnect("End Test");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}