/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_SetClientKey()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

extern char * configurationDirectory;

void setClientKey_PasswdCallback(char *passWd, unsigned int len) {
	strncpy(passWd, "thingworx", len);
}

TEST_GROUP(unit_twApi_SetClientKey);

TEST_SETUP(unit_twApi_SetClientKey) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twApi_SetClientKey) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetClientKey) {
	RUN_TEST_CASE(unit_twApi_SetClientKey, setClientKeyWithUninitializedApi);
	RUN_TEST_CASE(unit_twApi_SetClientKey, setClientKeyWithNullMsgHandler);
	RUN_TEST_CASE(unit_twApi_SetClientKey, setClientKeyWithNullCert);
	RUN_TEST_CASE(unit_twApi_SetClientKey, setClientKeyWithNullPassphrase);
	RUN_TEST_CASE(unit_twApi_SetClientKey, setClientKeySuccess);
}

static void testClientKeyPassWdCb(char *passWd, unsigned int len){
	strncpy(passWd, TEST_CLIENT_KEY_PASS, len);
}

/**
 * Test Plan: Set client key with an uninitialized API
 */
TEST(unit_twApi_SetClientKey, setClientKeyWithUninitializedApi) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetClientKey(TEST_CLIENT_KEY_FILE, testClientKeyPassWdCb, SSL_FILETYPE_PEM));
}

/**
 * Test Plan: Set client key with a null message handler
 */
TEST(unit_twApi_SetClientKey, setClientKeyWithNullMsgHandler) {
	twMessageHandler *mh = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	mh = tw_api->mh;
	tw_api->mh = NULL;
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_MSG_HANDLER, twApi_SetClientKey(TEST_CLIENT_KEY_FILE, testClientKeyPassWdCb, SSL_FILETYPE_PEM));
	tw_api->mh = mh;
}

/**
 * Test Plan: Set client key with a NULL cert path
 */
TEST(unit_twApi_SetClientKey, setClientKeyWithNullCert) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetClientKey(NULL, testClientKeyPassWdCb, SSL_FILETYPE_PEM));
}

/**
 * Test Plan: Set client key with a NULL passphrase
 */
TEST(unit_twApi_SetClientKey, setClientKeyWithNullPassphrase) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetClientKey(TEST_CLIENT_KEY_FILE, NULL, SSL_FILETYPE_PEM));
}

/**
 * Test Plan: Set ClientKey cert
 */
TEST(unit_twApi_SetClientKey, setClientKeySuccess) {
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
	strncat(certFile, TEST_CLIENT_KEY_FILE, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetClientKey(certFile, setClientKey_PasswdCallback, SSL_FILETYPE_PEM));
	TEST_ASSERT_EQUAL_STRING(info->client_key_file, certFile);

	/* Save the full path, including the file name of TEST_CA_CERT_FILE_1, in certFile */
	certFile[0] = '\0';
	strncpy(certFile, certFilePath, MAX_FILE_PATH - strlen(certFile) - 1);
	certFile[MAX_FILE_PATH - 1] = '\0';
	strncat(certFile, TEST_CLIENT_KEY_FILE_1, MAX_FILE_PATH - strlen(certFile) - 1);

	TEST_ASSERT_TRUE(fileExists(certFile));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetClientKey(certFile, setClientKey_PasswdCallback, SSL_FILETYPE_PEM));
	TEST_ASSERT_EQUAL_STRING(info->client_key_file, certFile);
}