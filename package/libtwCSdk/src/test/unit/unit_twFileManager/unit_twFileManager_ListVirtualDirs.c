/*
 * Copyright 2018, PTC, Inc. All rights reserved.
 */
#include "TestUtilities.h"
#include "unity_fixture.h"

#define THING "ListVirtualDirsTestThing"

/*
 * Be sure at least one dirname and one test file name contains at least one
 * space. At one time it was reported that spaces in the path caused the SDK to
 * crash.
 */
static char * pathnames[] = {"path0", "path1"};
static char const * dirnames[] = {
    "ListVirtualDirectoryTest",
    "List Virtual Directory Test"
};
static size_t const n_test_paths = 2;  /* Number of dirname entries */
static char * test_files[] = {"virtual_file", "virtual file"};
static size_t const n_test_files = 2;
static char ** test_paths = NULL;
static twList * list = NULL;

/* Tests
 *
 * Exercise the twFileManager_ListVirtualDirs() interface. There are two states
 * that need to be tested -- virtual directories have been added, and no virtual
 * directories have been added. And name notwithstanding, actual directories
 * must exist that the virtual directories point to for the whole thing to work.
 * The setup and tear down interfaces take care of the actual directories for
 * us.
 */
TEST_GROUP(unit_twFileManager_ListVirtualDirs);

TEST_SETUP(unit_twFileManager_ListVirtualDirs) {
    size_t idx;
    char * cwd;

    eatLogs();

    TEST_ASSERT_EQUAL(TW_OK, twFileManager_Create());

    test_paths = TW_CALLOC(sizeof(*test_paths), n_test_paths);
    cwd = getCurrentDirectory();
    for (idx = 0; idx < n_test_paths; ++idx) {
        size_t fidx;

        test_paths[idx] = joinPath(cwd, dirnames[idx]);
        twDirectory_CreateDirectory(test_paths[idx]);

        for (fidx = 0; fidx < n_test_files; ++fidx) {
            char * path = joinPath(test_paths[idx], test_files[fidx]);
            twDirectory_CreateFile(path);
            TW_FREE(path);
        }
    }
    TW_FREE(cwd);
}

TEST_TEAR_DOWN(unit_twFileManager_ListVirtualDirs) {
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        char * path = test_paths[idx];
        twDirectory_DeleteDirectory(path);
        TW_FREE(test_paths[idx]);
    }
    TW_FREE(test_paths);

    if (list) {
        TW_FREE(list);
        list = NULL;
    }

    TEST_ASSERT_EQUAL(TW_OK, twFileManager_Delete());
}

TEST_GROUP_RUNNER(unit_twFileManager_ListVirtualDirs) {
    RUN_TEST_CASE(unit_twFileManager_ListVirtualDirs, test_virtual_dirs_exist);
    RUN_TEST_CASE(unit_twFileManager_ListVirtualDirs, test_no_virtual_dirs);
}

/*
 * Test Case: Virtual directories for the thing have been added.
 *
 * ListVirtualDirs should return a list with the added directories.
 */
TEST(unit_twFileManager_ListVirtualDirs, test_virtual_dirs_exist) {
    size_t idx;
    ListEntry * le = NULL;

    for (idx = 0; idx < n_test_paths; ++idx) {
        twFileManager_AddVirtualDir(THING, pathnames[idx], test_paths[idx]);
    }

    list = twFileManager_ListVirtualDirs(THING);

    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL(n_test_paths, list->count);

    for (
        le = twList_Next(list, NULL);
        le && le->value;
        le = twList_Next(list, le)
    ) {
        twFile * f = le->value;
        TEST_ASSERT_TRUE(f->isDir);
        TEST_ASSERT_TRUE(inArray(f->name, pathnames, n_test_paths));
        TEST_ASSERT_TRUE(inArray(f->realPath, test_paths, n_test_paths));
    }
}

/*
 * Test Case: No virtual directories for the thing have been added.
 *
 * ListVirtualDirs should return a zero-length list.
 */
TEST(unit_twFileManager_ListVirtualDirs, test_no_virtual_dirs) {
    list = twFileManager_ListVirtualDirs(THING);

    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL(0, list->count);
}
