/**
 * Generic File Transfer Performance Tests
 *
 * This test group has a generic setup/teardown macro to initialize the API, connect, and set up a remote thing for
 * generic file transfer performance testing.
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

void fileTransferIntegrationCreateTestFileInRepo(char *fileName, size_t fileSize);

char *fileTransferIntegrationGetTestFileFromRepo(char *fileName, char async);

char *fileTransferIntegrationSendTestFileToRepo(char *fileName, char async);

char fileTransferIntegrationCompareTestFilesInRepo(char *fileName);

const char *fileTransferIntegrationTestActiveTransferParseFunc(void *data);

void fileTransferIntegrationTestFileEventCallback(char fileRcvd, twFileTransferInfo *info, void *userData);

void fileTransferIntegrationWaitForAsyncTransfers(twDict *activeTransferIds);

/**
 * Unity Test Macros
 */

TEST_GROUP(FileTransferPerformance);

void test_FileTransferPerformanceAppKey_callback(char *passWd, unsigned int len){
    strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(FileTransferPerformance) {
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

    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_FileTransferPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "FileTransferPerformance SETUP: failed to initialize API");
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
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("in"), "FileTransferPerformance SETUP: failed to create \"in\" directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("out"), "FileTransferPerformance SETUP: failed to create \"out\" directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twFileManager_AddVirtualDir(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "in", inPath), "FileTransferPerformance SETUP: failed to register \"in\" virtual directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twFileManager_AddVirtualDir(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "out", outPath), "FileTransferPerformance SETUP: failed to register \"out\" virtual directory");
    twExt_Start(1000, TW_THREADING_MULTI, 5);
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES), "FileTransferPerformance SETUP: failed to connect to server");
    createServerThing(FILE_TRANSFER_INTEGRATION_TEST_THING_NAME, "RemoteThingWithFileTransfer");
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, FILE_TRANSFER_INTEGRATION_TEST_REPO_NAME));
    TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_CSDKFileTransferIntegrationTestRepo.xml"));

    TW_FREE(stagingPath);
    TW_FREE(inPath);
    TW_FREE(outPath);
}

TEST_TEAR_DOWN(FileTransferPerformance) {
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
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twExt_Stop(), "FileTransferPerformance TEAR_DOWN: failed to stop threads");
    TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("FileTransferPerformance TEAR_DOWN: test is ending"));
    TEST_ASSERT_FALSE_MESSAGE(twApi_isConnected(), "FileTransferPerformance TEAR_DOWN: failed to disconnect from server");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(stagingPath), "FileTransferPerformance TEAR_DOWN: failed to delete \"in\" directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(inPath), "FileTransferPerformance TEAR_DOWN: failed to delete \"in\" directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(outPath), "FileTransferPerformance TEAR_DOWN: failed to delete \"out\" directory");
    TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "FileTransferPerformance TEAR_DOWN: failed to delete API");

    TW_FREE(stagingPath);
    TW_FREE(inPath);
    TW_FREE(outPath);
}

TEST_GROUP_RUNNER(FileTransferPerformance) {
    RUN_TEST_CASE(FileTransferPerformance, RoundTripSynchronousFileTransfers);
}

TEST(FileTransferPerformance, RoundTripSynchronousFileTransfers) {
    long totalDuration = 0;
    int i = 0;
    /* This can be set to less than the number of defined file names/sizes in order to skip the large/long transfers */
    const int numTestFiles = 8;
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

    printf("\n");

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
    totalDuration = 0;
    for (i = 0; i < numTestFiles; i++) {
        METRIC("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.getFile.size", fileSizes[i]);
        {
            char *tid = NULL;
            MARK_START("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.getFile.time");
            tid = fileTransferIntegrationGetTestFileFromRepo(fileNames[i], SYNCHRONOUS_TRANSFER);
            MARK_END("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.getFile.time");
            TW_FREE(tid);
            totalDuration += duration;
            printf("FileTransferPerformance.getFile: Got file %s of size %d in %d ms\n", fileNames[i], fileSizes[i], duration);
        }
    }
	printf("FileTransferPerformance.getFile: Total duration %d ms\n", totalDuration);
	TEST_ASSERT_TRUE_MESSAGE(totalDuration < 1000*60*5, "FileTransferPerformance.getFile: Total duration exceeded 5m")

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
	totalDuration = 0;
	for (i = 0; i < numTestFiles; i++) {
        METRIC("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.sendFile.size", fileSizes[i]);
        {
            char *tid = NULL;
            MARK_START("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.sendFile.time");
            tid = fileTransferIntegrationSendTestFileToRepo(fileNames[i], SYNCHRONOUS_TRANSFER);
            MARK_END("csdk.performance.fileTransferPerformance.roundTripSynchronousFileTransfers.sendFile.time");
	        TW_FREE(tid);
	        totalDuration += duration;
	        printf("FileTransferPerformance.sendFile: Sent file %s of size %d in %d ms\n", fileNames[i], fileSizes[i], duration);
        }
    }
	printf("FileTransferPerformance.sendFile: Total duration %d ms\n", totalDuration);
	TEST_ASSERT_TRUE_MESSAGE(totalDuration < 1000*60*10, "FileTransferPerformance.sendFile: Total duration exceeded 10m")

	/* Compare the contents of the files in the "in" and "out" directories of the file repository */
    for (i = 0; i < numTestFiles; i++) {
	    fileTransferIntegrationCompareTestFilesInRepo(fileNames[i]);
    }

    /* Clean up */
    TW_FREE(inPath);
    TW_FREE(outPath);
}