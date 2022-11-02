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

static int mock_twDirectory_CreateDirectory(char * name) {
	return 1;
}

static int mock_twDirectory_CreateDirectory1(char * name) {
	return 0;
}

static char mock_twDirectory_FileExists1(char * name) {
	return FALSE;
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

TEST_GROUP(unit_twFileTransferCallbacks_twMakeDirectory);

/* Stubbed functions*/
int twDirectory_CreateDirectory(char * name);

TEST_SETUP(unit_twFileTransferCallbacks_twMakeDirectory) {
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

TEST_TEAR_DOWN(unit_twFileTransferCallbacks_twMakeDirectory) {
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

TEST_GROUP_RUNNER(unit_twFileTransferCallbacks_twMakeDirectory) {
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_ErrorCode_BadRequest);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_ErrorCode_TWX_INTERNAL_SERVER_ERROR);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_TWX_SUCCESS_FileExists);
	RUN_TEST_CASE(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_TWX_SUCCESS);
}

/*Test plan: The test will fail because twMakeDirectory function is not containig all the valid parameters. */
TEST(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_ErrorCode_BadRequest) {
	TEST_ASSERT_EQUAL(TWX_BAD_REQUEST, twMakeDirectory(TEST_ENTITY_NAME, NULL, NULL));
}

/* Test plan: The following test will fail while creating a file in the location because those are not added in the virtual directory. */
TEST(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_ErrorCode_TWX_INTERNAL_SERVER_ERROR) {
	twInfoTable *it = NULL;
	twApi_stub->twDirectory_CreateDirectory = mock_twDirectory_CreateDirectory;
	it = createMockFtiIt();
	TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, twMakeDirectory(THINGFORCALLBACK, it, &contentForCallback));
	twInfoTable_Delete(it);
}

/* The intent of this mock is to create an infotable with one field as path. */
twInfoTable *createMockFtiIt1() {
	twInfoTable *it = NULL;
	it = TW_MAKE_IT(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING("path0")
			)
	);
	return it;
}

/* Test Plan: The following test will return SUCCESS if it creates a file at the desired location but before that 
*  they might be added to the Virtual Directory.
*/
TEST(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_TWX_SUCCESS_FileExists) {
	twInfoTable *it = NULL;
	int idx;
	char *pathValueString = NULL;
	it = createMockFtiIt1();
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	/* Directory already exists. */
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twMakeDirectory(THINGFORCALLBACK, it, &contentForCallback));
	for (idx = 0; idx < it->rows->count; ++idx) {
		twPrimitive * p = NULL;
		twInfoTableRow * row = twInfoTable_GetEntry(it, idx);
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(it, "path", idx, &pathValueString));
		p = twInfoTableRow_GetEntry(row, 0);
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(TW_STRING, p->type);
		TEST_ASSERT_TRUE(inArray(p->val.bytes.data, pathnames_ForCallback, n_test_pathsCallbackCallback));
		TW_FREE(pathValueString);
	}
	twInfoTable_Delete(it);
}

/* Test Plan: The test will pass with SUCCESS message as twMakeDirectory function creates some directories and files at its virtual path. */
TEST(unit_twFileTransferCallbacks_twMakeDirectory, test_twFileTransferCallback_twMakeDirectory_TWX_SUCCESS) {
	twInfoTable *it = NULL;
	char *pathValueString;
	int idx;
	twApi_stub->twDirectory_CreateDirectory = mock_twDirectory_CreateDirectory1;
	twApi_stub->twDirectory_FileExists = mock_twDirectory_FileExists1;
	it = createMockFtiIt1();
	for (idx = 0; idx < n_test_pathsCallbackCallback; ++idx) {
		twFileManager_AddVirtualDir(THINGFORCALLBACK, pathnames_ForCallback[idx], test_pathsCallback[idx]);
	}
	TEST_ASSERT_EQUAL(TWX_SUCCESS, twMakeDirectory(NEWTHING, it, &contentForCallback));
	for (idx = 0; idx < it->rows->count; ++idx) {
		twPrimitive * p = NULL;
		twInfoTableRow * row = twInfoTable_GetEntry(it, idx);
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(it, "path", idx, &pathValueString));
		p = twInfoTableRow_GetEntry(row, 0);
		TEST_ASSERT_NOT_NULL(p);
		TEST_ASSERT_EQUAL(TW_STRING, p->type);
		TEST_ASSERT_TRUE(inArray(p->val.bytes.data, pathnames_ForCallback, n_test_pathsCallbackCallback));
		TW_FREE(pathValueString);
	}
	twInfoTable_Delete(it);
}