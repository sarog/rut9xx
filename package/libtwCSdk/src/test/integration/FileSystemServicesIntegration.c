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

#define FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME "CSDKFileTransferIntegrationTestThing"

/**
 * Unity Test Macros
 */

TEST_GROUP(FileSystemServicesIntegration);

void test_FileSystemServicesIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(FileSystemServicesIntegration) {
	char *stagingPath = NULL;
	char *dir1Path = NULL;

	eatLogs();

	stagingPath = getCurrentDirectory();
	dir1Path = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(stagingPath);
	TEST_ASSERT_NOT_NULL(dir1Path);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&stagingPath, "/staging"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&dir1Path, "/dir1"));

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_FileSystemServicesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "FileSystemServicesIntegration SETUP: failed to initialize API");
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twcfg_pointer->file_xfer_staging_dir = stagingPath;
	{
		TW_MAKE_THING(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, TW_THING_TEMPLATE_GENERIC);
		TW_ADD_FILE_TRANSFER_SHAPE();
		TW_BIND();
	}
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("dir1"), "FileSystemServicesIntegration SETUP: failed to create \"dir1\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("dir1/dir2"), "FileSystemServicesIntegration SETUP: failed to create \"dir2\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateDirectory("dir1/dir2/dir3"), "FileSystemServicesIntegration SETUP: failed to create \"dir3\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateFile("dir1/file1.txt"), "FileSystemServicesIntegration SETUP: failed to file \"file1\" file");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateFile("dir1/dir2/file2.txt"), "FileSystemServicesIntegration SETUP: failed to file \"file2\" file");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_CreateFile("dir1/dir2/dir3/file3.txt"), "FileSystemServicesIntegration SETUP: failed to file \"file3\" file");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twFileManager_AddVirtualDir(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "dir1", dir1Path), "FileSystemServicesIntegration SETUP: failed to register \"dir1\" virtual directory");
	twExt_Start(1000, TW_THREADING_MULTI, 5);
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES), "FileSystemServicesIntegration SETUP: failed to connect to server");
	createServerThing(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "RemoteThingWithFileTransfer");

	TW_FREE(stagingPath);
	TW_FREE(dir1Path);
}

TEST_TEAR_DOWN(FileSystemServicesIntegration) {
	char *stagingPath = NULL;
	char *dir1Path = NULL;

	stagingPath = getCurrentDirectory();
	dir1Path = getCurrentDirectory();
	TEST_ASSERT_NOT_NULL(stagingPath);
	TEST_ASSERT_NOT_NULL(dir1Path);
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&stagingPath, "/staging"));
	TEST_ASSERT_EQUAL(TW_OK, concatenateStrings(&dir1Path, "/dir1"));

	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME));
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twExt_Stop(), "FileSystemServicesIntegration TEAR_DOWN: failed to stop threads");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("FileSystemServicesIntegration TEAR_DOWN: test is ending"));
	TEST_ASSERT_FALSE_MESSAGE(twApi_isConnected(), "FileSystemServicesIntegration TEAR_DOWN: failed to disconnect from server");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(stagingPath), "FileSystemServicesIntegration SETUP: failed to delete \"in\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twDirectory_DeleteDirectory(dir1Path), "FileSystemServicesIntegration SETUP: failed to delete \"in\" directory");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "FileSystemServicesIntegration TEAR_DOWN: failed to delete API");

	TW_FREE(stagingPath);
	TW_FREE(dir1Path);
}

TEST_GROUP_RUNNER(FileSystemServicesIntegration) {
	RUN_TEST_CASE(FileSystemServicesIntegration, BrowseDirectory);
	RUN_TEST_CASE(FileSystemServicesIntegration, DeleteFile);
	RUN_TEST_CASE(FileSystemServicesIntegration, GetFileInfo);
	RUN_TEST_CASE(FileSystemServicesIntegration, ListDirectories);
	RUN_TEST_CASE(FileSystemServicesIntegration, ListFiles);
	RUN_TEST_CASE(FileSystemServicesIntegration, MoveFile);
	RUN_TEST_CASE(FileSystemServicesIntegration, BrowseFileSystem);
	RUN_TEST_CASE(FileSystemServicesIntegration, GetDirectoryStructure);
}

TEST(FileSystemServicesIntegration, BrowseDirectory) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *fileType = NULL;
	char *path = NULL;
	DATETIME lastModifiedDate = 0;
	double size = 0;
	int i = 0;

	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("dir1", name);
	TEST_ASSERT_EQUAL_STRING("/dir1", path);
	TEST_ASSERT_TRUE(size >= 0);
	TEST_ASSERT_EQUAL_STRING("D", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	for (i = 0; i < result->rows->count; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", i, &fileType));
		if (strcmp("F", fileType) == 0) {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("file1.txt", name);
			TEST_ASSERT_EQUAL_STRING("/dir1", path);
			TEST_ASSERT_EQUAL(0, size);
			TEST_ASSERT_EQUAL_STRING("F", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		} else {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("dir2", name);
			TEST_ASSERT_EQUAL_STRING("/dir1", path);
			TEST_ASSERT_TRUE(size >= 0);
			TEST_ASSERT_EQUAL_STRING("D", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		}
		TW_FREE(name);
		TW_FREE(path);
		TW_FREE(fileType);
	}
	twInfoTable_Delete(result);

	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	for (i = 0; i < result->rows->count; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", i, &fileType));
		if (strcmp("F", fileType) == 0) {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("file2.txt", name);
			TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
			TEST_ASSERT_EQUAL(0, size);
			TEST_ASSERT_EQUAL_STRING("F", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		} else {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("dir3", name);
			TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
			TEST_ASSERT_TRUE(size >= 0);
			TEST_ASSERT_EQUAL_STRING("D", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		}
		TW_FREE(name);
		TW_FREE(path);
		TW_FREE(fileType);
	}
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, DeleteFile) {
	twInfoTable *result = NULL;

	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	twInfoTable_Delete(result);
	twTest_InvokeService_DeleteFile(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/file1.txt");
	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, GetFileInfo) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *fileType = NULL;
	char *path = NULL;
	DATETIME lastModifiedDate = 0;
	double size = 0;

	result = twTest_InvokeService_GetFileInfo(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/file1.txt");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("file1.txt", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/file1.txt", path);
	TEST_ASSERT_EQUAL(0, size);
	TEST_ASSERT_EQUAL_STRING("F", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_GetFileInfo(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "\\dir1\\dir2\\file2.txt");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("file2.txt", name);
	TEST_ASSERT_EQUAL_STRING("\\dir1\\dir2\\file2.txt", path);
	TEST_ASSERT_EQUAL(0, size);
	TEST_ASSERT_EQUAL_STRING("F", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, ListDirectories) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *path = NULL;
	char *parentPath = NULL;

	result = twTest_InvokeService_ListDirectories(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 0, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir1", name);
	TEST_ASSERT_EQUAL_STRING("/dir1", path);
	TEST_ASSERT_EQUAL_STRING("/", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_ListDirectories(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 0, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir2", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
	TEST_ASSERT_EQUAL_STRING("/dir1", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_ListDirectories(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 0, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir3", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2/dir3", path);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, ListFiles) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *fileType = NULL;
	char *path = NULL;
	DATETIME lastModifiedDate = 0;
	double size = 0;

	result = twTest_InvokeService_ListFiles(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 0);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_ListFiles(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("file1.txt", name);
	TEST_ASSERT_EQUAL_STRING("/dir1", path);
	TEST_ASSERT_EQUAL(0, size);
	TEST_ASSERT_EQUAL_STRING("F", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_ListFiles(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("file2.txt", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
	TEST_ASSERT_EQUAL(0, size);
	TEST_ASSERT_EQUAL_STRING("F", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, MoveFile) {
	twInfoTable *result = NULL;

	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	twInfoTable_Delete(result);
	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	twInfoTable_Delete(result);
	twTest_InvokeService_MoveFile(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/file1.txt", "/dir1/dir2/file1.txt", TRUE);
	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	twInfoTable_Delete(result);
	result = twTest_InvokeService_BrowseDirectory(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 3);
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, BrowseFileSystem) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *fileType = NULL;
	char *path = NULL;
	DATETIME lastModifiedDate = 0;
	double size = 0;
	int i = 0;

	result = twTest_InvokeService_BrowseFileSystem(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 1);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", 0, &fileType));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", 0, &size));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", 0, &lastModifiedDate));
	TEST_ASSERT_EQUAL_STRING("dir1", name);
	TEST_ASSERT_EQUAL_STRING("/dir1", path);
	TEST_ASSERT_TRUE(size >= 0);
	TEST_ASSERT_EQUAL_STRING("D", fileType);
	TEST_ASSERT_TRUE(lastModifiedDate > 0);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(fileType);
	twInfoTable_Delete(result);

	result = twTest_InvokeService_BrowseFileSystem(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	for (i = 0; i < result->rows->count; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", i, &fileType));
		if (strcmp("F", fileType) == 0) {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("file1.txt", name);
			TEST_ASSERT_EQUAL_STRING("/dir1", path);
			TEST_ASSERT_EQUAL(0, size);
			TEST_ASSERT_EQUAL_STRING("F", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		} else {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("dir2", name);
			TEST_ASSERT_EQUAL_STRING("/dir1", path);
			TEST_ASSERT_TRUE(size >= 0);
			TEST_ASSERT_EQUAL_STRING("D", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		}
		TW_FREE(name);
		TW_FREE(path);
		TW_FREE(fileType);
	}
	twInfoTable_Delete(result);

	result = twTest_InvokeService_BrowseFileSystem(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME, "/dir1/dir2");
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 2);
	for (i = 0; i < result->rows->count; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "fileType", i, &fileType));
		if (strcmp("F", fileType) == 0) {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("file2.txt", name);
			TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
			TEST_ASSERT_EQUAL(0, size);
			TEST_ASSERT_EQUAL_STRING("F", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		} else {
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", i, &name));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", i, &path));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetNumber(result, "size", i, &size));
			TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetDatetime(result, "lastModifiedDate", i, &lastModifiedDate));
			TEST_ASSERT_EQUAL_STRING("dir3", name);
			TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
			TEST_ASSERT_TRUE(size >= 0);
			TEST_ASSERT_EQUAL_STRING("D", fileType);
			TEST_ASSERT_TRUE(lastModifiedDate > 0);
		}
		TW_FREE(name);
		TW_FREE(path);
		TW_FREE(fileType);
	}
	twInfoTable_Delete(result);
}

TEST(FileSystemServicesIntegration, GetDirectoryStructure) {
	twInfoTable *result = NULL;
	char *name = NULL;
	char *path = NULL;
	char *parentPath = NULL;

	result = twTest_InvokeService_GetDirectoryStructure(FILE_SYSTEM_SERVICES_INTEGRATION_TEST_THING_NAME);
	TEST_ASSERT_NOT_NULL(result);
	TEST_ASSERT_EQUAL(result->rows->count, 3);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 0, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 0, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 0, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir1", name);
	TEST_ASSERT_EQUAL_STRING("/dir1", path);
	TEST_ASSERT_EQUAL_STRING("/", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 1, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 1, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 1, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir2", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2", path);
	TEST_ASSERT_EQUAL_STRING("/dir1", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "name", 2, &name));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "path", 2, &path));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result, "parentPath", 2, &parentPath));
	TEST_ASSERT_EQUAL_STRING("dir3", name);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2/dir3", path);
	TEST_ASSERT_EQUAL_STRING("/dir1/dir2", parentPath);
	TW_FREE(name);
	TW_FREE(path);
	TW_FREE(parentPath);
	twInfoTable_Delete(result);
}