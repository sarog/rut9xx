#include "TestUtilities.h"
#include "unity_fixture.h"

#define THINGFORFILETRANSFER "ListVirtualDirsTestThingForFileTransfer"

static char * pathnamesForListEntity[] = {"path0","path1"};
static char const * dirnamesForListEntity[] = {
		"ListEntityVirtualDirectoryTest",
		"ListEntity VirtualDirectory Test"
};
static size_t const n_test_paths_forListEntities = 2;  /* Number of dirname entries */
static char * test_files_forListEntities[] = {"virtual_file_ForFileTransfer", "virtual file ForFileTransfer"};
static size_t const n_test_files_forListEntities_forListEntities = 2;
static char ** test_paths_forListEntities = NULL;
static twList * list_forListEntities = NULL;

TEST_GROUP(unit_twFileManager_ListEntities);

TEST_SETUP(unit_twFileManager_ListEntities) {
	size_t idx_forListEntities;
	char * cwd_forListEntities;

	eatLogs();

	TEST_ASSERT_EQUAL(TW_OK, twFileManager_Create());

	test_paths_forListEntities = TW_CALLOC(sizeof(*test_paths_forListEntities), n_test_paths_forListEntities);
	cwd_forListEntities = getCurrentDirectory();
	for (idx_forListEntities = 0; idx_forListEntities < n_test_paths_forListEntities; ++idx_forListEntities) {
		size_t fidx_forListEntities;

		test_paths_forListEntities[idx_forListEntities] = joinPath(cwd_forListEntities, dirnamesForListEntity[idx_forListEntities]);
		twDirectory_CreateDirectory(test_paths_forListEntities[idx_forListEntities]);

		for (fidx_forListEntities = 0; fidx_forListEntities < n_test_files_forListEntities_forListEntities; ++fidx_forListEntities) {
			char * path = joinPath(test_paths_forListEntities[idx_forListEntities], test_files_forListEntities[fidx_forListEntities]);
			twDirectory_CreateFile(path);
			TW_FREE(path);
		}
	}
	TW_FREE(cwd_forListEntities);
}

TEST_TEAR_DOWN(unit_twFileManager_ListEntities) {
	size_t idx_forListEntities;

	for (idx_forListEntities = 0; idx_forListEntities < n_test_paths_forListEntities; ++idx_forListEntities) {
		char * path = test_paths_forListEntities[idx_forListEntities];
		twDirectory_DeleteDirectory(path);
		TW_FREE(test_paths_forListEntities[idx_forListEntities]);
	}
	TW_FREE(test_paths_forListEntities);

	if (list_forListEntities) {
		TW_FREE(list_forListEntities);
		list_forListEntities = NULL;
	}
	/* reset stubs */
	twStubs_Reset();
	TEST_ASSERT_EQUAL(TW_OK, twFileManager_Delete());
}

TEST_GROUP_RUNNER(unit_twFileManager_ListEntities) {
	RUN_TEST_CASE(unit_twFileManager_ListEntities, test_FileTransfer_ListEntities_DirectoriesVerification);
	RUN_TEST_CASE(unit_twFileManager_ListEntities, test_FileTransfer_ListEntities_FilesVerification);
}

char* mock_twFileManager_GetRealPath(const char * thingName, const char * path, const char * filename) {
	/* allocat space for fake path */
	char *cwd_forListEntities11 = getCurrentDirectory();

	return cwd_forListEntities11;
}


TEST(unit_twFileManager_ListEntities, test_FileTransfer_ListEntities_DirectoriesVerification) {
	size_t idx_forListEntities;
	twFile * file_forList;
	ListEntry * le_forListEntities = NULL;
	twList *liForList = NULL;
	int listCount = 0;
	TEST_IGNORE_MESSAGE("Test will fail until CSDK-1167 is fixed.");
	twApi_stub->twFileManager_GetRealPath	= mock_twFileManager_GetRealPath;

	for (idx_forListEntities = 0; idx_forListEntities < n_test_paths_forListEntities; ++idx_forListEntities) {
		twFileManager_AddVirtualDir(THINGFORFILETRANSFER, pathnamesForListEntity[idx_forListEntities], test_paths_forListEntities[idx_forListEntities]);
	}

	for (idx_forListEntities = 0; idx_forListEntities < n_test_paths_forListEntities; ++idx_forListEntities) {
		liForList = twFileManager_ListEntities(THINGFORFILETRANSFER, getCurrentDirectory(), dirnamesForListEntity[idx_forListEntities], LIST_DIRS);
		le_forListEntities = twList_Next(liForList, NULL);
		file_forList = le_forListEntities->value;
		TEST_ASSERT_TRUE(inArray(file_forList->name, dirnamesForListEntity, n_test_paths_forListEntities));
		listCount++;
	}
	/* Matching Number of Directory Entries */
	TEST_ASSERT_EQUAL(listCount, n_test_paths_forListEntities);
	twList_Delete(liForList);
}

TEST(unit_twFileManager_ListEntities, test_FileTransfer_ListEntities_FilesVerification) {
	size_t idx_listOfFiles;
	int flag = 0;
	ListEntry * le_forListEntities = NULL;
	twList *liForList1 = NULL;
	char * path_for_file = NULL;
	char * cwd_forlist = getCurrentDirectory();
	twApi_stub->twFileManager_GetRealPath	= mock_twFileManager_GetRealPath;

	path_for_file = joinPath(cwd_forlist, "foo");
	twDirectory_CreateFile(path_for_file);
	liForList1 = twFileManager_ListEntities(THINGFORFILETRANSFER, getCurrentDirectory(), "foo", LIST_FILES);
	TEST_ASSERT_NOT_NULL(liForList1);
	for(le_forListEntities = twList_Next(liForList1, NULL); le_forListEntities && le_forListEntities->value; le_forListEntities = twList_Next(liForList1, le_forListEntities)) {
		twFile *file_forList = le_forListEntities->value;
		TEST_ASSERT_FALSE(file_forList->isDir);
		TEST_ASSERT_EQUAL_STRING("foo", file_forList->name);
	}
	twDirectory_DeleteFile(path_for_file);
	twList_Delete(liForList1);
}