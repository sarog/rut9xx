/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_LoadClientCert()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

extern char * configurationDirectory;

TEST_GROUP(unit_twApi_LoadClientCert);

TEST_SETUP(unit_twApi_LoadClientCert) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_LoadClientCert) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_LoadClientCert) {
	RUN_TEST_CASE(unit_twApi_LoadClientCert, loadClientCertWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_LoadClientCert, loadClientCertWithNullMsgHandler);
	RUN_TEST_CASE(unit_twApi_LoadClientCert, loadClientCertWithNullCert);
	RUN_TEST_CASE(unit_twApi_LoadClientCert, loadClientCert);
}

/**
 * Test Plan: Load client cert with an uninitialized API
 */
TEST(unit_twApi_LoadClientCert, loadClientCertWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_LoadClientCert(TEST_CLIENT_CERT_FILE));
}

/**
 * Test Plan: Load client cert with a null message handler
 */
TEST(unit_twApi_LoadClientCert, loadClientCertWithNullMsgHandler) {
	twMessageHandler *mh = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	mh = tw_api->mh;
	tw_api->mh = NULL;
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_MSG_HANDLER, twApi_LoadClientCert(TEST_CLIENT_CERT_FILE));
	tw_api->mh = mh;
}

/**
 * Test Plan: Load client cert with a NULL cert path
 */
TEST(unit_twApi_LoadClientCert, loadClientCertWithNullCert) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_LoadClientCert(NULL));
}

/**
 * Test Plan: Load client cert
 */
TEST(unit_twApi_LoadClientCert, loadClientCert) {
	int err = 0;
	twConnectionInfo *info = NULL;
	char *cwd = NULL;
	char certFile[MAX_FILE_PATH] = {'\0'};
	char certFilePath[MAX_FILE_PATH] = {'\0'};

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	/* use stubs to mock tls client behaviour */
	err = twStubs_Use();
	if (err) {
		/* stubs are disabled, exit test */
		return;
	}

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

	/* Save the full path, including the file name of TEST_CLIENT_CERT_FILE, in certFile */
	certFile[0] = '\0';
	strncpy(certFile, certFilePath, MAX_FILE_PATH - strlen(certFile) - 1);
	certFile[MAX_FILE_PATH - 1] = '\0';
	strncat(certFile, TEST_CLIENT_CERT_FILE, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_LoadClientCert(certFile));
	info = twApi_GetConnectionInfo();
	TEST_ASSERT_EQUAL_STRING(info->client_cert_file, certFile);

	/* Save the full path, including the file name of TEST_CLIENT_CERT_FILE_1, in certFile */
	certFile[0] = '\0';
	strncpy(certFile, certFilePath, MAX_FILE_PATH - strlen(certFile) - 1);
	certFile[MAX_FILE_PATH - 1] = '\0';
	strncat(certFile, TEST_CLIENT_CERT_FILE_1, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_LoadClientCert(certFile));
	info = twApi_GetConnectionInfo();
	TEST_ASSERT_EQUAL_STRING(info->client_cert_file, certFile);
}