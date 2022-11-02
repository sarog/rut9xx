#include "twApi.h"
#include "twFileManager.h"
#include "twThreads.h"
#include "twExt.h"
#include "twMacros.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

#ifdef WIN32
extern __declspec(dllimport) twFileManager * fm;
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
#else
extern twApi_Stubs * twApi_stub;
extern twFileManager * fm;
#endif

#define THINGFORCALLBACK "GetDirectoryStructureTestThingForCallback"
#define NEWTHING "newThingForValidation"
#define THING "newThing"

enum msgCodeEnum twMakeDirectory(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twCreateBinaryFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twDeleteFile(const char * entityName, twInfoTable * params, twInfoTable ** content);
enum msgCodeEnum twGetDirectoryStructure(const char * entityName, twInfoTable * params, twInfoTable ** content);
int listDirsInInfoTable(char * entityName, char * virtualPath, twInfoTable * it);

static char mock_twDirectory_FileExists(char * name) {
	return TRUE;
}

static int mock_twDirectory_DeleteFile(char * name) {
	return 1;
}

static int mock_twDirectory_DeleteFile1(char * name) {
	return 0;
}

/*
 * Be sure at least one dirname and one test file name contains at least one
 * space. At one time it was reported that spaces in the path caused the SDK to
 * crash.
 */
static char * pathnames[] = {"path3"};
static char * pathnames_ForCallback[] = {"path0", "path1"};
static char const * dirnames_ForCallback[] = {
		"GetDirectoryStructureTest",
		"Get Directory Structure Test"
};
static size_t const n_test_pathsCallbackCallback = 2;  /* Number of dirname entries */
static char * test_filesCallback[] = {"virtual_file", "virtual file"};
static size_t const n_test_filesCallback = 2;
static char ** test_pathsCallback = NULL;
static twInfoTable * contentForCallback = NULL;

TEST_GROUP(unit_twFileTransferCallbacks_twDeleteFile);

/* Stubbed functions*/
int twDirectory_CreateDirectory(char * name);

TEST_SETUP(unit_twFileTransferCallbacks_twDeleteFile) {
	size_t idx;
	char * cwd;
	eatLogs();
	twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);

	TEST_ASSERT_EQUAL(TW_OK, twFileManager_Create());

	test_pathsCallback = TW_CALLOC(sizeof(*test_pathsCallback), n_test_pathsCallbackCallback);
	cwd = getCurrentDirectory();
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		size_t fidx;

		test_pathsCallback[idx] = joinPath(cwd, dirnames_ForCallback[idx]);
		twDirectory_CreateDirectory(test_pathsCallback[idx]);

		for (fidx = 0; fidx < n_test_filesCallback; ++fidx) {
			char * path = joinPath(test_pathsCallback[idx], test_filesCallback[fidx]);
			twDirectory_CreateFile(path);
			TW_FREE(path);
		}
	}
	TW_FREE(cwd);
}

TEST_TEAR_DOWN(unit_twFileTransferCallbacks_twDeleteFile) {
	size_t idx;

	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		char * path = test_pathsCallback[idx];
		twDirectory_DeleteDirectory(path);
		TW_FREE(test_pathsCallback[idx]);
	}
	TW_FREE(test_pathsCallback);

	if (contentForCallback) {
		twInfoTable_Delete(contentForCallback);
		contentForCallback = NULL;
	}
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twFileTransferCallbacks_twDeleteFile) {
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_ErrorCode_BadRequest);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_FileExists_TWX_SUCCESS);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_ErrorCode_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_TWXSUCCESS);
}


/* Test Plan: twDeleteFile function will fail as we are not passing the complete arguments. */
TEST(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_ErrorCode_BadRequest) {
	TEST_ASSERT_EQUAL(TWX_BAD_REQUEST, twDeleteFile(TEST_ENTITY_NAME, NULL, NULL));
}

/* Test Plan: The test will pass as its verifying that a file is present at a location for its deletion. */
TEST(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_FileExists_TWX_SUCCESS) {
	twInfoTable *it = NULL;
	int idx;
	it = createMockFtiIt();
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	/* File does not exist at the realPath for deletion. */
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twDeleteFile(THINGFORCALLBACK, it, &contentForCallback));
	twInfoTable_Delete(it);
}

/*Test Plan: Error in deleting a file as those files are already deleted form that location. */
TEST(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_ErrorCode_TWX_INTERNAL_SERVER_ERROR) {
	twInfoTable *it = NULL;
	twApi_stub->twDirectory_FileExists = mock_twDirectory_FileExists;
	twApi_stub->twDirectory_DeleteFile = mock_twDirectory_DeleteFile;
	it = createMockFtiIt();
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twDeleteFile(THINGFORCALLBACK, it, &contentForCallback));
	twInfoTable_Delete(it);
}

/* Test Plan: The test will pass as its deleting the file from the particular location. */
TEST(unit_twFileTransferCallbacks_twDeleteFile, test_twFileTransferCallback_twDeleteFile_TWXSUCCESS) {
	twInfoTable *it = NULL;
	int idx;
	char *realPath;
	char *path;
	twApi_stub->twDirectory_DeleteFile = mock_twDirectory_DeleteFile1;
	twApi_stub->twDirectory_FileExists = mock_twDirectory_FileExists;
	it = createMockFtiIt();
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	realPath = twFileManager_GetRealPath(THINGFORCALLBACK, pathnames_ForCallback[0], NULL);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&realPath, "/abc.txt"));
	TEST_ASSERT_EQUAL(TW_OK, twDirectory_CreateFile(realPath));
	TW_FREE(realPath);
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twDeleteFile(THINGFORCALLBACK, it, &contentForCallback));
	for(idx = 0; idx < it->rows->count; idx++) {
		twInfoTableRow * row = twInfoTable_GetEntry(it, idx);
		twPrimitive * p = twInfoTableRow_GetEntry(row, 17);
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(TW_STRING, p->type);
		path = getVirtualDirPath(THINGFORCALLBACK, pathnames_ForCallback[idx]);
		TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&path, "/abc.txt"));
		TEST_ASSERT_FALSE(twDirectory_FileExists(path));
		TW_FREE(path);
	}
	twInfoTable_Delete(it);
}