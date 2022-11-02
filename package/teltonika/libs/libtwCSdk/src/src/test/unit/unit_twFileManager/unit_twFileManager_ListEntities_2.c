#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "unitTestDefs.h"
#include "twThreads.h"
#include "twFileManager.h"
#include "twApiStubs.h"

/* Test configuration definitions */
#define NUM_WORKER_THREADS 8
#define NUM_TEST_FILES 6

#ifdef WIN32
extern __declspec(dllimport) twFileManager * fm;
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
#else
extern twApi_Stubs * twApi_stub;
extern twFileManager * fm;
#endif

/**
 * Set the test group name
 */
TEST_GROUP(unit_twFileManager_ListEntities_2);

/* stubbed functions */
char STUB_twDirectory_FileExists(char * name);
int STUB_twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly);
int STUB_twDirectory_GetLastError();
twFile * STUB_twFile_Create();
void STUB_twFileManager_CloseFile(void * file);
twFile* STUB_twFileManager_GetOpenFile(const char * thingName, const char * path, const char * filename, const char * tid, char * isTimedOut);
char* STUB_twFileManager_GetRealPath(const char * thingName, const char * path, const char * filename);
void STUB_twFile_Delete(void * f);
TW_FILE_HANDLE STUB_twFile_FOpen(const char * name, const char * mode);
int STUB_twList_Remove(twList * list, ListEntry * entry, char deleteValue);

static uint64_t open_time = 0;
static uint64_t close_time = 0;

int STUB_twList_Remove(twList * list, ListEntry * entry, char deleteValue) {
	/* record time */
	close_time = twGetSystemMillisecondCount();

	/* remove form list */
	return twList_Remove(list, entry, deleteValue);
}

/* global thread to run fake api thread */
twThread * ft_unit_thread = NULL;
/**
 * Setup macro run before each test
 */
TEST_SETUP(unit_twFileManager_ListEntities_2) {
	/* set file transfer timeout for quicker tests*/
	twcfg_pointer->file_xfer_timeout = 250;

	/* stop logger */
	eatLogs();

	/* init the api */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init the file manager */
	twFileManager_Create();

	/* set stubs so only the file manager singleton starts */
	twApi_stub->twDirectory_FileExists		= STUB_twDirectory_FileExists;
	twApi_stub->twDirectory_GetFileInfo		= STUB_twDirectory_GetFileInfo;
	/* twApi_stub->twDirectory_GetLastError	= STUB_twDirectory_GetLastError; */
	/* twApi_stub->twFile_Create = STUB_twFile_Create; */

	twApi_stub->twFileManager_CloseFile		= STUB_twFileManager_CloseFile;
	twApi_stub->twFileManager_GetOpenFile	= STUB_twFileManager_GetOpenFile;
	twApi_stub->twFileManager_GetRealPath	= STUB_twFileManager_GetRealPath;
	twApi_stub->twFile_Delete				= STUB_twFile_Delete;
	twApi_stub->twFile_FOpen				= STUB_twFile_FOpen;
	twApi_stub->twList_Remove				= STUB_twList_Remove;

	/* start the fake API thread */
	ft_unit_thread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
}

/**
 * Tear down macro run after each test
 */
TEST_TEAR_DOWN(unit_twFileManager_ListEntities_2) {
	/* delete the fake API thread */
	twThread_Delete(ft_unit_thread);

	/* delete file manager*/
	twFileManager_Delete();

	/* delete the fake API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());

	/* reset stubs */
	twStubs_Reset();

	/* reset ft timeout */
	twcfg_pointer->file_xfer_timeout = FILE_XFER_TIMEOUT;
}

/**
 * Runs specified test cases for this group
 */
TEST_GROUP_RUNNER(unit_twFileManager_ListEntities_2) {
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_WithoutRealPath);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_TwList_Create);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_TwFile_Create);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_NO_MORE_FILES);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_listNoFiles);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_listRootDirectory);
	RUN_TEST_CASE(unit_twFileManager_ListEntities_2, check_timeout);
}

/* stubbed functions */
char STUB_twDirectory_FileExists(char * name) {
	return TRUE;
}

int STUB_twDirectory_GetFileInfo(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly) {
	return 0;
}

twFile * STUB_twFile_Create() {
	return NULL;
}

void STUB_twFileManager_CloseFile(void * file) {
	return;
}

twFile* STUB_twFileManager_GetOpenFile(const char * thingName, const char * path, const char * filename, const char * tid, char * isTimedOut) {
	return NULL;
}

char* STUB_twFileManager_GetRealPath(const char * thingName, const char * path, const char * filename) {
	/* allocat space for fake path */
	char * fake_path = (char*)TW_MALLOC(10);

	/* pupulate fake path */
	sprintf(fake_path, "fake_path");

	return fake_path;
}

void STUB_twFile_Delete(void * f) {
	twFile * tmp = (twFile*)f;
	tmp->handle = NULL;
	twFile_Delete(f);
	return;
}

TW_FILE_HANDLE STUB_twFile_FOpen(const char * name, const char * mode) {
	return 0x01;
}

/* twFileManager_ListEntities Tests*/
TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_WithoutRealPath) {
	TEST_ASSERT_EQUAL(NULL, twFileManager_ListEntities(NULL, TEST_THING_NAME_1, NULL, LIST_ALL));
}

twList* mock_twList_Create(del_func delete_function) {
	return NULL;
}

TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_TwList_Create) {
	twApi_stub->twList_Create = mock_twList_Create;
	TEST_ASSERT_EQUAL(NULL, twFileManager_ListEntities(TEST_ENTITY_NAME, TEST_THING_NAME_1, NULL, LIST_ALL));
}

TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_TwFile_Create) {
	twApi_stub->twFile_Create = STUB_twFile_Create;
	TEST_ASSERT_EQUAL(NULL, twFileManager_ListEntities(TEST_ENTITY_NAME, TEST_THING_NAME_1, NULL, LIST_ALL));
}

TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_ErrorCode_NO_MORE_FILES) {
	TEST_ASSERT_EQUAL(NULL, twFileManager_ListEntities(TEST_ENTITY_NAME, TEST_THING_NAME_1, NULL, LIST_ALL));
}

TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_listNoFiles) {
	twList *li1 = NULL;
	char *fileToDelete = NULL;
	/* It's only for Validation of path start with '\\' and it does not add anything to the list. */
	twList *li = twFileManager_ListEntities(TEST_ENTITY_NAME, "\\testEntityName", NULL, LIST_FILES);
	TEST_ASSERT_NOT_NULL(li);
	TEST_ASSERT_EQUAL(0, li->count);
	twList_Delete(li1);
	twList_Delete(li);
}

TEST(unit_twFileManager_ListEntities_2, test_twFileTransfer_twFileManager_ListEntities_listRootDirectory) {
	twList *li1 = NULL;
	/* I think this List Entity is showing the root directory */
	twList *li = twFileManager_ListEntities("*", "\\testEntityName", NULL, LIST_DIRS);
	li1 = twFileManager_ListVirtualDirs("*");
	TEST_ASSERT_NOT_NULL(li);
	TEST_ASSERT_EQUAL(li->count, li1->count);
	twList_Delete(li1);
	twList_Delete(li);
}

/**
 * check_timeout
 *
 * starts a transfer to place the active TID in the FileManager
 * sleeps for 2X file_xfer_timeout
 * calls GetFileInfo to force a timeout
 * EXPECTED: actual timeout interval should be within +/- THRESHOLD ms of file_xfer_timeout
 */
TEST(unit_twFileManager_ListEntities_2, check_timeout) {
	/* tid to track file */
	char * tid = "fake_tid";

	/* timed out flag*/
	char isTimedOut = FALSE;

	/* error message buffer */
	char err_msg[ERR_MSG_SIZE];

	/* total timeout time */
	uint64_t rt_timeout = 0;

	/* set a threshold to 10% of the total timeout */
	int threshold = .2* twcfg_pointer->file_xfer_timeout;

	/* set max and min times*/
	int max_time = twcfg_pointer->file_xfer_timeout + threshold;
	int min_time = twcfg_pointer->file_xfer_timeout - threshold;

	/* call open file to add file to file transfer manager */
	twFileManager_OpenFile(TEST_ENTITY_NAME, TEST_ENTITY_NAME, TEST_ENTITY_NAME, "rb");
	open_time = twGetSystemMillisecondCount();

	/* verify valid timestamp */
	TEST_ASSERT_TRUE(0 != open_time);

	/* sleep to trigger timeout*/
	twSleepMsec(twcfg_pointer->file_xfer_timeout * 2);

	/* call get file info to clean up old transfers*/
	twFileManager_GetOpenFile(TEST_ENTITY_NAME, TEST_ENTITY_NAME, TEST_ENTITY_NAME, tid, NULL);

	/* verify valid timestamp */
	TEST_ASSERT_TRUE(0 != close_time);

	/* get total time for file transfer */
	rt_timeout = close_time - open_time;

	/* compare file_xfer_timeout with actual timeout time */
	snprintf(err_msg, ERR_MSG_SIZE, "Timeout occurred outside of the allowable threshold: %dms, max time: %dms, min time: %dms, actual time: %ldms", threshold, max_time, min_time, rt_timeout);
	TEST_ASSERT_TRUE_MESSAGE((rt_timeout > min_time) && (rt_timeout < max_time), err_msg);

	/* exit test */
}