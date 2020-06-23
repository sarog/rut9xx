/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_LoadCACert()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

extern char * configurationDirectory;

TEST_GROUP(unit_twApi_LoadCACert);

TEST_SETUP(unit_twApi_LoadCACert) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_LoadCACert) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_LoadCACert) {
	RUN_TEST_CASE(unit_twApi_LoadCACert, loadCACertWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_LoadCACert, loadCACertWithNullMsgHandler);
	RUN_TEST_CASE(unit_twApi_LoadCACert, loadCACertWithNullCert);
	RUN_TEST_CASE(unit_twApi_LoadCACert, loadCACert);
	RUN_TEST_CASE(unit_twApi_LoadCACert, loadCACertXL);
}

/**
 * Test Plan: Load CA cert with an uninitialized API
 */
TEST(unit_twApi_LoadCACert, loadCACertWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_LoadCACert(TEST_CA_CERT_FILE, SSL_FILETYPE_PEM));
}

/**
 * Test Plan: Load CA cert with a null message handler
 */
TEST(unit_twApi_LoadCACert, loadCACertWithNullMsgHandler) {
	twMessageHandler *mh = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	mh = tw_api->mh;
	tw_api->mh = NULL;
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_MSG_HANDLER, twApi_LoadCACert(TEST_CA_CERT_FILE, SSL_FILETYPE_PEM));
	tw_api->mh = mh;
}

/**
 * Test Plan: Load CA cert with a NULL cert path
 */
TEST(unit_twApi_LoadCACert, loadCACertWithNullCert) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_LoadCACert(NULL, SSL_FILETYPE_PEM));
}

/**
 * Test Plan: Load CA cert
 */
TEST(unit_twApi_LoadCACert, loadCACert) {
	int err = 0;
	twConnectionInfo *info = NULL;
	char *cwd = NULL;
	char certFile[MAX_FILE_PATH] = {'\0'};
	char certFilePath[MAX_FILE_PATH] = {'\0'};

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	info = twApi_GetConnectionInfo();
	TEST_ASSERT_NOT_NULL(info);

	/* Get current working dir. It should be tw-c-sdk/bin/test */
	cwd = getCurrentDirectory();

	/* Validate current working directory */
	TEST_ASSERT_NOT_NULL(cwd);
	TEST_ASSERT_TRUE(strcmp(cwd, "") != 0);

	/* Get the path to the certFile. It should be tw-c-sdk/test/etc/ i.e. tw-c-sdk/bin/test/../../test/etc/ */
	certFilePath[0] = '\0';
	strncpy(certFilePath, cwd, MAX_FILE_PATH - 1);
	/* strncpy: If there is no null byte among the first n bytes of src, the string placed in dest will not be null-terminated. */
	certFilePath[MAX_FILE_PATH - 1] = '\0';
#ifdef WIN32
	strncat(certFilePath, "\\..\\..\\test\\etc", MAX_FILE_PATH - strlen(certFilePath) - 1);
	if(!fileExists(certFilePath)) {
		certFilePath[0] = '\0';
		strncpy(certFilePath, cwd, MAX_FILE_PATH - 1);
		certFilePath[MAX_FILE_PATH - 1] = '\0';
		strncat(certFilePath, "\\..\\..\\..\\test\\etc", MAX_FILE_PATH - strlen(certFilePath) - 1);
	}
	if(!fileExists(certFilePath) && (NULL != configurationDirectory)) {
		certFilePath[0] = '\0';
		strncpy(certFilePath, configurationDirectory, MAX_FILE_PATH - 1);
		certFilePath[MAX_FILE_PATH - 1] = '\0';
	} else {
		strncat(certFilePath, "\\", MAX_FILE_PATH - strlen(certFilePath) - 1);
	}
#else
	strncat(certFilePath, "/../../test/etc", MAX_FILE_PATH - strlen(certFilePath) - 1);
	if(!fileExists(certFilePath)) {
		certFilePath[0] = '\0';
		strncpy(certFilePath, cwd, MAX_FILE_PATH - 1);
		certFilePath[MAX_FILE_PATH - 1] = '\0';
		strncat(certFilePath, "/../../../test/etc", MAX_FILE_PATH - strlen(certFilePath) - 1);
	}
	if(!fileExists(certFilePath) && (NULL != configurationDirectory)) {
		certFilePath[0] = '\0';
		strncpy(certFilePath, configurationDirectory, MAX_FILE_PATH - 1);
		certFilePath[MAX_FILE_PATH - 1] = '\0';
	} else {
		strncat(certFilePath, "/", MAX_FILE_PATH - strlen(certFilePath) - 1);
	}
#endif

	/* Save the full path, including the file name of TEST_CA_CERT_FILE, in certFile */
	certFile[0] = '\0';
	strncpy(certFile, certFilePath, MAX_FILE_PATH - strlen(certFile) - 1);
	certFile[MAX_FILE_PATH - 1] = '\0';
	strncat(certFile, TEST_CA_CERT_FILE, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_LoadCACert(certFile, SSL_FILETYPE_PEM));
	TEST_ASSERT_EQUAL_STRING(info->ca_cert_file, certFile);

	/* Save the full path, including the file name of TEST_CA_CERT_FILE_1, in certFile */
	certFile[0] = '\0';
	strncpy(certFile, certFilePath, MAX_FILE_PATH - strlen(certFile) - 1);
	certFile[MAX_FILE_PATH - 1] = '\0';
	strncat(certFile, TEST_CA_CERT_FILE_1, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_LoadCACert(certFile, SSL_FILETYPE_PEM));
	TEST_ASSERT_EQUAL_STRING(info->ca_cert_file, certFile);
}

/**
* Test Plan: Load AXTLS CA Cert and delete  
*/

TEST(unit_twApi_LoadCACert, loadCACertXL) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	if (!AXTLS_VERSION) {
		TEST_IGNORE_MESSAGE("Ignoring 'XL' CA certificate load failure test because this is a non-AxTLS build and the test relies on behavior specific to AxTLS");
	} else {
		TEST_ASSERT_EQUAL(TW_TLS_ERROR_LOADING_FILE, loadCACertFromEtc(TEST_CA_CERT_FILE_XL));
	}
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}