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

TEST_GROUP(unit_twFileManager_RegisterFileCallback);

/* Stubbed functions*/
int twDirectory_CreateDirectory(char * name);

TEST_SETUP(unit_twFileManager_RegisterFileCallback) {
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

TEST_TEAR_DOWN(unit_twFileManager_RegisterFileCallback) {
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

TEST_GROUP_RUNNER(unit_twFileManager_RegisterFileCallback) {
	RUN_TEST_CASE(unit_twFileManager_RegisterFileCallback, CSDK_1001);
	RUN_TEST_CASE(unit_twFileManager_RegisterFileCallback, test_twFileTransfer_twFileManager_UnregisterFileCallback);
	RUN_TEST_CASE(unit_twFileManager_RegisterFileCallback, test_twFileTransfer_twFileManager_UnregisterFileCallback_ErrorCode);
}

char CSDK_1001_Callback_Made = FALSE;
void CSDK_1001_Callback_Sleep50(char fileRcvd, twFileTransferInfo *info, void *userdata) {
	char *message = NULL;
	/* Sleep while main thread attempts to unregister this callback */
	twSleepMsec(50);
	/* Mess around with some memory, crashes before CSDK-1001 fix was implemented */
	message = duplicateString(info->message);
	TW_FREE(message);
}
void CSDK_1001_Callback_Sleep100(char fileRcvd, twFileTransferInfo *info, void *userdata) {
	char *message = NULL;
	/* Sleep while main thread attempts to unregister this callback */
	twSleepMsec(100);
	/* Mess around with some memory, crashes before CSDK-1001 fix was implemented */
	message = duplicateString(info->message);
	TW_FREE(message);
}

void CSDK_1001_Callback_Task(uint64_t sys_msecs, void *params) {
	/* Only run once */
	if (!CSDK_1001_Callback_Made) {
		twFileTransferInfo *ft = NULL;
		twInfoTable *it = NULL;
		it = createMockFtiIt();
		ft = twFileTransferInfo_Create(it);
		twFileManager_MakeFileCallback(TRUE, ft);
		CSDK_1001_Callback_Made = TRUE;
		twInfoTable_Delete(it);
	}
}

twFileManager *twFileManager_GetFileManager();

/**
 * CSDK-1001 twFileManager_MakeFileCallback doesn't acquire the necessary mutex
 *
 * Summary: The function twFileManager_MakeFileCallback does not acquire a lock before invoking a user callback when a
 * file transfer completes. In another thread, this callback structure could have already been deleted by the linking
 * application invoking twFileManager_UnregisterFileCallback (which acquires the mutex fm->mtx ).  This can lead to an
 * application crash or memory corruption.
 *
 * Test Plan: Trigger a file transfer callback that sleeps for some time then have a separate thread attempt to delete
 * it.
 *
 * Fix: Added a mutex to the twFileXferCallback struct and lock each individual callback during execution and
 * unregistration.
 */
TEST(unit_twFileManager_RegisterFileCallback, CSDK_1001) {
	twThread *callbackTaskThread = NULL;
	twFileManager *fm = NULL;
	twcfg_pointer->file_xfer_staging_dir = "./staging";
	twFileManager_Create();
	TEST_ASSERT_NULL_MESSAGE(fm,"Failed to create file manager. Confirm that twcfg.file_xfer_staging_dir is a writable directory.");
	fm = twFileManager_GetFileManager();
	CSDK_1001_Callback_Made = FALSE;
	TEST_ASSERT_EQUAL(0, fm->callbacks->count);
	twFileManager_RegisterFileCallback(CSDK_1001_Callback_Sleep50, NULL, TRUE, NULL);
	twFileManager_RegisterFileCallback(CSDK_1001_Callback_Sleep100, NULL, FALSE, NULL);
	TEST_ASSERT_EQUAL(2, fm->callbacks->count);
	callbackTaskThread = twThread_Create(CSDK_1001_Callback_Task, 5, NULL, TRUE);
	twSleepMsec(10);
	twFileManager_UnregisterFileCallback(CSDK_1001_Callback_Sleep100, NULL, NULL);
	twSleepMsec(500);
	TEST_ASSERT_TRUE(CSDK_1001_Callback_Made);
	TEST_ASSERT_EQUAL(0, fm->callbacks->count);
	twThread_Delete(callbackTaskThread);
	twFileManager_Delete();
}

/* twFileManager_UnregisterFileCallback Tests*/
TEST(unit_twFileManager_RegisterFileCallback, test_twFileTransfer_twFileManager_UnregisterFileCallback) {
	TEST_ASSERT_EQUAL(TW_OK, twFileManager_UnregisterFileCallback(NULL, "foo", "foo1"));
}

TEST(unit_twFileManager_RegisterFileCallback, test_twFileTransfer_twFileManager_UnregisterFileCallback_ErrorCode) {
	fm = NULL;
	TEST_ASSERT_EQUAL(TW_FILE_XFER_MANAGER_NOT_INITIALIZED, twFileManager_UnregisterFileCallback(NULL, "foo", "foo1"));
}