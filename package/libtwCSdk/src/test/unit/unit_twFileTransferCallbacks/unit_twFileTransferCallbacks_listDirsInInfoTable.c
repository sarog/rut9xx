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

twInfoTableRow * mock_twInfoTableRow_Create(twPrimitive * firstEntry) {
	return NULL;
}

static int mock_twInfoTable_AddRow(twInfoTable * it, twInfoTableRow * row) {
	return 1;
}

static int mock_listDirsInInfoTable(char * entityName, char * virtualPath, twInfoTable * it) {
	return 1;
}

static int mock_listDirsInInfoTable1(char * entityName, char * virtualPath, twInfoTable * it) {
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

TEST_GROUP(unit_twFileTransferCallbacks_listDirsInInfoTable);

/* Stubbed functions*/
int twDirectory_CreateDirectory(char * name);

TEST_SETUP(unit_twFileTransferCallbacks_listDirsInInfoTable) {
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

TEST_TEAR_DOWN(unit_twFileTransferCallbacks_listDirsInInfoTable) {
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

TEST_GROUP_RUNNER(unit_twFileTransferCallbacks_listDirsInInfoTable) {
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_ErrorCode_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_GetRealPath_ErrorCode_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_twInfoTableRowCreate_ErrorCode_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_twInfoTableAddRow_ErrorCode_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_ErrorCode_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_FileCreate_ErrorCode_TW_ERROR_ALLOCATING_MEMORY);
}

/* Test Plan: listDirsInInfoTable function fails as we are not providing the actual parameters to the function. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_ErrorCode_INTERNAL_SERVER_ERROR) {
	char *realPath;
	int idx;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	realPath = twFileManager_GetRealPath(THINGFORCALLBACK, pathnames_ForCallback[0], NULL);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, listDirsInInfoTable(NULL, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, listDirsInInfoTable(THINGFORCALLBACK, NULL, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, listDirsInInfoTable(NULL, realPath, NULL));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, listDirsInInfoTable(NULL, NULL, contentForCallback));
	TW_FREE(realPath);
}

/* Test Plan: listDirsInInfoTable function fails while getting the real path. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_GetRealPath_ErrorCode_INTERNAL_SERVER_ERROR) {
	twInfoTable * contentForInfoTable;
	int idx;
	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForInfoTable));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, listDirsInInfoTable(THINGFORCALLBACK, "foo", contentForInfoTable));
	twInfoTable_Delete(contentForInfoTable);
}

/* Test Plan: The test will succeed and list all directories in infotable. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable) {
	twInfoTable * contentForInfoTable;
	int idx;
	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForInfoTable));
	TEST_ASSERT_EQUAL(n_test_pathsCallbackCallback, contentForInfoTable->rows->count);
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		TEST_ASSERT_EQUAL(TW_OK, listDirsInInfoTable(THINGFORCALLBACK, pathnames_ForCallback[idx], contentForInfoTable));
	}
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twInfoTableRow * row = twInfoTable_GetEntry(contentForInfoTable, idx);
		twPrimitive * p = twInfoTableRow_GetEntry(row, 0);
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(TW_STRING, p->type);
		TEST_ASSERT_TRUE(inArray(p->val.bytes.data, pathnames_ForCallback, n_test_pathsCallbackCallback));
	}
	twInfoTable_Delete(contentForInfoTable);
}

/* Test Plan: listDirsInInfoTable function fails while allocating infotable row. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_twInfoTableRowCreate_ErrorCode_TWX_INTERNAL_SERVER_ERROR) {
	char *testPath1;
	twInfoTable * contentForInfoTable1;
	char *testPath = NULL;
	char *cwDirectory = getCurrentDirectory();
	testPath = joinPath(cwDirectory, "newDirectory");
	twDirectory_CreateDirectory(testPath);

	/* Sub directory*/
	testPath1 = joinPath(testPath, "subDirectory");
	twDirectory_CreateDirectory(testPath1);

	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	twFileManager_AddVirtualDir(THING, "vdir", testPath);
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THING, NULL, &contentForInfoTable1));
	twStubs_Reset();
	twApi_stub->twInfoTableRow_Create = mock_twInfoTableRow_Create;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, listDirsInInfoTable(THING, "vdir", contentForInfoTable1));
	twDirectory_DeleteDirectory(testPath);
	twInfoTable_Delete(contentForInfoTable1);
	TW_FREE(testPath);
	TW_FREE(testPath1);
}

/* Test Plan: listDirsInInfoTable function fails while adding infotable row. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_twInfoTableAddRow_ErrorCode_TWX_INTERNAL_SERVER_ERROR) {
	char *testPath1;
	twInfoTable * contentForInfoTable1;
	char *testPath = NULL;
	char *cwDirectory = getCurrentDirectory();
	testPath = joinPath(cwDirectory, "newDirectory1");
	twDirectory_CreateDirectory(testPath);

	/* Sub directory*/
	testPath1 = joinPath(testPath, "subDirectory1");
	twDirectory_CreateDirectory(testPath1);

	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	twFileManager_AddVirtualDir(THING, "vdir1", testPath);
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THING, NULL, &contentForInfoTable1));
	twStubs_Reset();
	twApi_stub->twInfoTable_AddRow = mock_twInfoTable_AddRow;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, listDirsInInfoTable(THING, "vdir1", contentForInfoTable1));
	twDirectory_DeleteDirectory(testPath);
	twInfoTable_Delete(contentForInfoTable1);
	TW_FREE(testPath);
	TW_FREE(testPath1);
}

/* Test Plan: listDirsInInfoTable function fails while adding directory into infotable. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_ErrorCode_TWX_INTERNAL_SERVER_ERROR) {
	char *testPath1;
	twInfoTable * contentForInfoTable1;
	char *testPath = NULL;
	char *cwDirectory = getCurrentDirectory();
	testPath = joinPath(cwDirectory, "newDirectory2");
	twDirectory_CreateDirectory(testPath);

	/* Sub directory*/
	testPath1 = joinPath(testPath, "subDirectory2");
	twDirectory_CreateDirectory(testPath1);

	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	twFileManager_AddVirtualDir(THING, "vdir2", testPath);
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THING, NULL, &contentForInfoTable1));
	twStubs_Reset();
	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, listDirsInInfoTable(THING, "vdir2", contentForInfoTable1));
	twDirectory_DeleteDirectory(testPath);
	twInfoTable_Delete(contentForInfoTable1);
	TW_FREE(testPath);
	TW_FREE(testPath1);
}

twFile * mock_twFile_Create() {
	return NULL;
}

/* Test Plan: listDirsInInfoTable function fails while allocating twFile struct. */
TEST(unit_twFileTransferCallbacks_listDirsInInfoTable, test_twFileTransferCallback_listDirsInInfoTable_FileCreate_ErrorCode_TW_ERROR_ALLOCATING_MEMORY) {
	char *testPath1;
	twInfoTable * contentForInfoTable1;
	char *testPath = NULL;
	char *cwDirectory = getCurrentDirectory();
	testPath = joinPath(cwDirectory, "newDirectory3");
	twDirectory_CreateDirectory(testPath);

	/* Sub directory*/
	testPath1 = joinPath(testPath, "subDirectory3");
	twDirectory_CreateDirectory(testPath1);

	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable1;
	twFileManager_AddVirtualDir(THING, "vdir3", testPath);
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twGetDirectoryStructure(THING, NULL, &contentForInfoTable1));
	twApi_stub->twFile_Create = mock_twFile_Create;
	TEST_ASSERT_EQUAL(TW_ERROR_ALLOCATING_MEMORY, listDirsInInfoTable(THING, "vdir3", contentForInfoTable1));
	twDirectory_DeleteDirectory(testPath);
	twInfoTable_Delete(contentForInfoTable1);
	TW_FREE(testPath);
	TW_FREE(testPath1);
}