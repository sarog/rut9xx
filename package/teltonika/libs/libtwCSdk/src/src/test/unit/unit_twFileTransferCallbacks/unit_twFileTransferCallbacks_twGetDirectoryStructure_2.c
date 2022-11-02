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

/* The intent of the mock is to flush all data. */
static twDataShapeEntry * mock_twDataShapeEntry_Create(const char * name, const char * description, enum BaseType type) {
	return NULL;
}

/* The intent of the mock is to flush all data. */
static twDataShape * mock_twDataShape_Create(twDataShapeEntry * firstEntry) {
	return NULL;
}

static twInfoTable * mock_twInfoTable_Create(twDataShape * shape) {
	return NULL;
}

static twInfoTableRow * mock_twInfoTableRow_Create(twPrimitive * firstEntry) {
	return NULL;
}

static int mock_twInfoTable_AddRow(twInfoTable * it, twInfoTableRow * row) {
	return 1;
}

static int mock_listDirsInInfoTable(char * entityName, char * virtualPath, twInfoTable * it) {
	return 1;
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

TEST_GROUP(unit_twFileTransferCallbacks_twGetDirectoryStructure_2);

/* Stubbed functions*/
int twDirectory_CreateDirectory(char * name);

TEST_SETUP(unit_twFileTransferCallbacks_twGetDirectoryStructure_2) {
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

TEST_TEAR_DOWN(unit_twFileTransferCallbacks_twGetDirectoryStructure_2) {
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

TEST_GROUP_RUNNER(unit_twFileTransferCallbacks_twGetDirectoryStructure_2) {
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_DSE_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_DS_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_content_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_ListVirtualDirectory_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_twInfoTableRowCreate_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_twInfoTableAddRow_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_listDirsInInfoTable_INTERNAL_SERVER_ERROR);
}

/* Test Plan: twGetDirectoryStructure function will fail as we are not providing the valid parameters to the function. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_DSE_TWX_INTERNAL_SERVER_ERROR) {
	twApi_stub->twDataShapeEntry_Create = mock_twDataShapeEntry_Create;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}

/* Test Plan: The following test will fail as its having a problem while creating an output dataShape. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_DS_TWX_INTERNAL_SERVER_ERROR) {
	twApi_stub->twDataShape_Create = mock_twDataShape_Create;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}

/* Test Plan: The following test will fail as its having a problem while creating an output infoTable. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_content_TWX_INTERNAL_SERVER_ERROR) {
	twApi_stub->twInfoTable_Create = mock_twInfoTable_Create;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}

/* Test Plan: The following test will fail as its not getting the list of elements. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_ListVirtualDirectory_INTERNAL_SERVER_ERROR) {
	fm = NULL;
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(NULL, NULL, &contentForCallback));
	contentForCallback = NULL;
}

/* Test Plan: The following test will fail as its having a problem while fetching rows from the infoTable. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_twInfoTableRowCreate_INTERNAL_SERVER_ERROR) {
	int idx;
	twApi_stub->twInfoTableRow_Create = mock_twInfoTableRow_Create;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}

/* Test Plan: The following test will fail as its having a problem while adding infotable row. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_twInfoTableAddRow_INTERNAL_SERVER_ERROR) {
	int idx;
	twApi_stub->twInfoTable_AddRow = mock_twInfoTable_AddRow;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}

/* Test Plan: The following test will fail as its having a problem while copying directories into infotable. */
TEST(unit_twFileTransferCallbacks_twGetDirectoryStructure_2, test_twFileTransferCallback_twGetDirectoryStructure_listDirsInInfoTable_INTERNAL_SERVER_ERROR) {
	int idx;
	twApi_stub->listDirsInInfoTable = mock_listDirsInInfoTable;
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twGetDirectoryStructure(THINGFORCALLBACK, NULL, &contentForCallback));
}