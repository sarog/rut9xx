/**
 * Generic File Transfer Integration Tests
 *
 * This test group has a generic setup/teardown macro to initialize the API, connect, and set up a remote thing for
 * generic file transfer integration testing.
 */

#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "TestUtilities.h"
#include "twExt.h"
#include "twMacros.h"

/**
 * API
 */

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi *tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/**
 * Test Specific Definitions
 */

#define FILE_TRANSFER_INTEGRATION_TEST_THING_NAME "CSDKFileTransferIntegrationTestThing"
#define FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME "CSDKFileTransferIntegrationTestRepo"
#define FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT 60*1000*5

#define SYNCHRONOUS_TRANSFER FALSE
#define ASYNCHRONOUS_TRANSFER TRUE

/**
 * Test Helper Functions
 */

void fileTransferIntegrationCreateTestFileInRepo(char *fileName, size_t fileSize) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;
	char *path = NULL;

	TW_LOG(TW_FORCE, "fileTransferIntegrationCreateTestFileInRepo: Creating file %s of size %d in test file repository", fileName, fileSize);

	path = TW_MALLOC(sizeof(char) * (strlen("out/") + 1));
	strcpy(path, "out/");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, concatenateStrings(&path, fileName), "fileTransferIntegrationCreateTestFileInRepo: failed to concatenate path string");

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("fileName", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("fileSize", TW_NO_DESCRIPTION, TW_INTEGER)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(fileName),
					TW_MAKE_INT(fileSize)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(
			TW_OK,
			twApi_InvokeService(TW_THING,
			                    FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME,
			                    "CreateTestFile",
			                    params,
			                    &result,
			                    FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT,
			                    FALSE),
			"fileTransferIntegrationCreateTestFileInRepo: failed to create test file"
	);

	TW_FREE(path);
	twInfoTable_Delete(params);
	twInfoTable_Delete(result);
}

char *fileTransferIntegrationGetTestFileFromRepo(char *fileName, char async) {
	char *tid;

	TW_LOG(TW_FORCE, "fileTransferIntegrationGetTestFileFromRepo: Getting file %s", fileName);

	TEST_ASSERT_EQUAL_MESSAGE(
			TW_OK,
			twFileManager_GetFile(FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME,
			                      "out",
			                      fileName,
			                      FILE_TRANSFER_INTEGRATION_TEST_THING_NAME,
			                      "in",
			                      fileName,
			                      FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT,
			                      async,
			                      &tid),
			"fileTransferIntegrationGetTestFileFromRepo: failed to download test file"
	);

	return tid;
}

char *fileTransferIntegrationSendTestFileToRepo(char *fileName, char async) {
	char *tid;

	TW_LOG(TW_FORCE, "fileTransferIntegrationSendTestFileToRepo: Sending file %s", fileName);

	TEST_ASSERT_EQUAL_MESSAGE(
			TW_OK,
			twFileManager_SendFile(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME,
			                      "out",
			                      fileName,
			                       FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME,
			                      "in",
			                      fileName,
			                      FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT,
			                      async,
			                      &tid),
			"fileTransferIntegrationSendTestFileToRepo: failed to download test file"
	);

	return tid;
}

char fileTransferIntegrationCompareTestFilesInRepo(char *fileName) {
	char compareResult = FALSE;
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	TW_LOG(TW_FORCE, "fileTransferIntegrationCompareTestFilesInRepo: Comparing contents of test file %s in \"in\" and \"out\" directories", fileName);

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("fileName", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(fileName)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(
		TW_OK,
		twApi_InvokeService(TW_THING,
		                    FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME,
		                    "CompareTestFileContents",
		                    params,
		                    &result,
		                    FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT,
		                    FALSE),
		"fileTransferIntegrationCompareTestFilesInRepo: failed to invoke service to compare test file contents"
	);

	twInfoTable_GetBoolean(result, "result", 0, &compareResult);
	twInfoTable_Delete(params);
	twInfoTable_Delete(result);

	return compareResult;
}

const char *fileTransferIntegrationTestActiveTransferParseFunc(void *data) {
	return duplicateString(data);
}

void fileTransferIntegrationTestFileEventCallback(char fileRcvd, twFileTransferInfo *info, void *userData) {
	twDict *activeTransferIds = NULL;
	TW_LOG(TW_FORCE, "fileTransferIntegrationTestFileEventCallback: Transfer ID %s completed", info->transferId);
	activeTransferIds = (twDict*)userData;
	twDict_Remove(activeTransferIds, info->transferId, FALSE);
}

void fileTransferIntegrationWaitForAsyncTransfers(twDict *activeTransferIds) {
	int waitTime = 0;
	while (twDict_GetCount(activeTransferIds) > 0 && waitTime < FILE_TRANSFER_INTEGRATION_TEST_SERVICE_TIMEOUT) {
		twSleepMsec(1000);
		waitTime += 1000;
	}
	TEST_ASSERT_EQUAL_MESSAGE(0, twDict_GetCount(activeTransferIds), "fileTransferIntegrationWaitForAsyncTransfers: async test file transfers timed out");
}

/**
 * Unity Test Macros
 */

TEST_GROUP(FileTransferIntegration);

void test_FileTransferIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(FileTransferIntegration) {
	char *stagingPath = NULL;
	char *inPath = NULL;
	char *outPath = NULL;

	stagingPath = getCurrentDirectory();
	inPath = getCurrentDirectory();
	outPath = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(stagingPath);
	TEST_ASSERT_NOT_NULL(inPath);
	TEST_ASSERT_NOT_NULL(outPath);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&stagingPath, "/staging"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inPath, "/in"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outPath, "/out"));

	eatLogs();

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_FileTransferIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "FileTransferIntegration SETUP: failed to initialize API");
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twcfg_pointer->file_xfer_staging_dir = stagingPath;
	twcfg_pointer->file_xfer_block_size = 1024*256;
	twcfg_pointer->max_message_size = 1024*1024*20;
	{
		TW_MAKE_THING(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, TW_THING_TEMPLATE_GENERIC);
		TW_ADD_FILE_TRANSFER_SHAPE();
		TW_BIND();
	}
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("in"), "FileTransferIntegration SETUP: failed to create \"in\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("out"), "FileTransferIntegration SETUP: failed to create \"out\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twFileManager_AddVirtualDir(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "in", inPath), "FileTransferIntegration SETUP: failed to register \"in\" virtual directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twFileManager_AddVirtualDir(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "out", outPath), "FileTransferIntegration SETUP: failed to register \"out\" virtual directory");
	twExt_Start(1000, TW_THREADING_MULTI, 5);
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES), "FileTransferIntegration SETUP: failed to connect to server");
	createServerThing(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "RemoteThingWithFileTransfer");
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME));
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_CSDKFileTransferIntegrationTestRepo.xml"));

	TW_FREE(stagingPath);
	TW_FREE(inPath);
	TW_FREE(outPath);
}

TEST_TEAR_DOWN(FileTransferIntegration) {
	char *stagingPath = NULL;
	char *inPath = NULL;
	char *outPath = NULL;

	stagingPath = getCurrentDirectory();
	inPath = getCurrentDirectory();
	outPath = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(stagingPath);
	TEST_ASSERT_NOT_NULL(inPath);
	TEST_ASSERT_NOT_NULL(outPath);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&stagingPath, "/staging"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inPath, "/in"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outPath, "/out"));

	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, FILE_TRANSFER_INTEGRATION_TEST_THING_NAME));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME));
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twExt_Stop(), "FileTransferIntegration TEAR_DOWN: failed to stop threads");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("FileTransferIntegration TEAR_DOWN: test is ending"));
	TEST_ASSERT_FALSE_MESSAGE(twApi_isConnected(), "FileTransferIntegration TEAR_DOWN: failed to disconnect from server");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(stagingPath), "FileTransferIntegration TEAR_DOWN: failed to delete \"in\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(inPath), "FileTransferIntegration TEAR_DOWN: failed to delete \"in\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(outPath), "FileTransferIntegration TEAR_DOWN: failed to delete \"out\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "FileTransferIntegration TEAR_DOWN: failed to delete API");

	TW_FREE(stagingPath);
	TW_FREE(inPath);
	TW_FREE(outPath);
}

TEST_GROUP_RUNNER(FileTransferIntegration) {
	RUN_TEST_CASE(FileTransferIntegration, RoundTripSynchronousFileTransfers);
	RUN_TEST_CASE(FileTransferIntegration, RoundTripAsynchronousFileTransfers);
}

TEST(FileTransferIntegration, RoundTripSynchronousFileTransfers) {
	int i = 0;
	/* This can be set to less than the number of defined file names/sizes in order to skip the large/long transfers */
	const int numTestFiles = 7;
	const size_t fileSizes[8] = {
			0,
			1,
			1024*32,
			1024*32+1,
			1024*64,
			1024*64+1,
			1024*1024,
			1024*1024*10
	};
	const char *fileNames[8] = {
			"RoundTripSynchronousFileTransfers0b.txt",
			"RoundTripSynchronousFileTransfers1b.txt",
			"RoundTripSynchronousFileTransfers32k.txt",
			"RoundTripSynchronousFileTransfers32k+1.txt",
			"RoundTripSynchronousFileTransfers64k.txt",
			"RoundTripSynchronousFileTransfers64k+1.txt",
			"RoundTripSynchronousFileTransfers1mb.txt",
			"RoundTripSynchronousFileTransfers10mb.txt"
	};
	char *inPath = NULL;
	char *outPath = NULL;

	inPath = getCurrentDirectory();
	outPath = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(inPath);
	TEST_ASSERT_NOT_NULL(outPath);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inPath, "/in/"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outPath, "/out/"));

	/* Create the test files in the file repository thing */
	for (i = 0; i < numTestFiles; i++) {
		fileTransferIntegrationCreateTestFileInRepo(fileNames[i], fileSizes[i]);
	}

	/* Download the files from the file repository thing to our remote thing */
	for (i = 0; i < numTestFiles; i++) {
		char *tid = NULL;
		tid = fileTransferIntegrationGetTestFileFromRepo(fileNames[i], SYNCHRONOUS_TRANSFER);
		TW_FREE(tid);
	}

	/* Move the files from the "in" to the "out" directory */
	for (i = 0; i < numTestFiles; i++) {
		char *inFilePath = NULL;
		char *outFilePath = NULL;
		inFilePath = duplicateString(inPath);
		outFilePath = duplicateString(outPath);
		TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inFilePath, fileNames[i]));
		TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outFilePath, fileNames[i]));
		twDirectory_MoveFile(inFilePath, outFilePath);
		TW_FREE(inFilePath);
		TW_FREE(outFilePath);
	}

	/* Upload the files from our remote thing to the file repository */
	for (i = 0; i < numTestFiles; i++) {
		char *tid = NULL;
		tid = fileTransferIntegrationSendTestFileToRepo(fileNames[i], SYNCHRONOUS_TRANSFER);
		TW_FREE(tid);
	}

	/* Compare the contents of the files in the "in" and "out" directories of the file repository */
	for (i = 0; i < numTestFiles; i++) {
		fileTransferIntegrationCompareTestFilesInRepo(fileNames[i]);
	}

	/* Clean up */
	TW_FREE(inPath);
	TW_FREE(outPath);
}

TEST(FileTransferIntegration, RoundTripAsynchronousFileTransfers) {
	int i = 0;
	/* This can be set to less than the number of defined file names/sizes in order to skip the large/long transfers */
	const int numTestFiles = 7;
	const size_t fileSizes[8] = {
			0,
			1,
			1024*32,
			1024*32+1,
			1024*64,
			1024*64+1,
			1024*1024,
			1024*1024*10
	};
	const char *fileNames[8] = {
			"RoundTripAsynchronousFileTransfers0b.txt",
			"RoundTripAsynchronousFileTransfers1b.txt",
			"RoundTripAsynchronousFileTransfers32k.txt",
			"RoundTripAsynchronousFileTransfers32k+1.txt",
			"RoundTripAsynchronousFileTransfers64k.txt",
			"RoundTripAsynchronousFileTransfers64k+1.txt",
			"RoundTripAsynchronousFileTransfers1mb.txt",
			"RoundTripAsynchronousFileTransfers10mb.txt"
	};
	twDict *activeTransferIds = NULL;
	char *tids[8] = {NULL};
	char *inPath = NULL;
	char *outPath = NULL;

	inPath = getCurrentDirectory();
	outPath = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(inPath);
	TEST_ASSERT_NOT_NULL(outPath);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inPath, "/in/"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outPath, "/out/"));

	/* Create a dictionary to track active transfer IDs and pass it to our callback func which will remove them as they finish */
	activeTransferIds = twDict_Create(NULL, fileTransferIntegrationTestActiveTransferParseFunc);
	twFileManager_RegisterFileCallback(fileTransferIntegrationTestFileEventCallback, NULL, FALSE, activeTransferIds);

	/* Create the test files in the file repository thing */
	for (i = 0; i < numTestFiles; i++) {
		fileTransferIntegrationCreateTestFileInRepo(fileNames[i], fileSizes[i]);
	}

	/* Download the files from the file repository thing to our remote thing */
	for (i = 0; i < numTestFiles; i++) {
		tids[i] = fileTransferIntegrationGetTestFileFromRepo(fileNames[i], ASYNCHRONOUS_TRANSFER);
		twDict_Add(activeTransferIds, tids[i]);
		TW_FREE(tids[i]);
	}

	/* Wait for the transfers to finish */
	fileTransferIntegrationWaitForAsyncTransfers(activeTransferIds);

	/* Move the files from the "in" to the "out" directory */
	for (i = 0; i < numTestFiles; i++) {
		char *inFilePath = NULL;
		char *outFilePath = NULL;
		inFilePath = duplicateString(inPath);
		outFilePath = duplicateString(outPath);
		TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&inFilePath, fileNames[i]));
		TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&outFilePath, fileNames[i]));
		twDirectory_MoveFile(inFilePath, outFilePath);
		TW_FREE(inFilePath);
		TW_FREE(outFilePath);
	}

	/* Upload the files from our remote thing to the file repository */
	for (i = 0; i < numTestFiles; i++) {
		tids[i] = fileTransferIntegrationSendTestFileToRepo(fileNames[i], ASYNCHRONOUS_TRANSFER);
		twDict_Add(activeTransferIds, tids[i]);
		TW_FREE(tids[i]);
	}

	/* Wait for the transfers to finish */
	fileTransferIntegrationWaitForAsyncTransfers(activeTransferIds);

	/* Compare the contents of the files in the "in" and "out" directories of the file repository */
	for (i = 0; i < numTestFiles; i++) {
		fileTransferIntegrationCompareTestFilesInRepo(fileNames[i]);
	}

	/* Clean up */
	twDict_Delete(activeTransferIds);
	TW_FREE(inPath);
	TW_FREE(outPath);
}