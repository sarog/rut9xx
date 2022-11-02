/*
 * Copyright 2018, PTC, Inc. All rights reserved.
 */
#include "twBaseTypes.h"
#include "TestUtilities.h"
#include "unity_fixture.h"

#define THING "GetDirectoryStructureTestThing"

/*
 * Be sure at least one dirname and one test file name contains at least one
 * space. At one time it was reported that spaces in the path caused the SDK to
 * crash.
 *
 * The pathnames and dirnames arrays must have the same number of elements.
 */
static char * pathnames[] = {"path0", "path1", "2", "C", "p"};
static char const * dirnames[] = {
    "A",
    "AB",
    "1",
    "GetDirectoryStructureTest",
    "Get Directory Structure Test"
};
static size_t const n_test_paths = 5;  /* Number of dirname entries */
static char * test_files[] = {"virtual_file", "virtual file"};
static size_t const n_test_files = 2;
static char ** test_paths = NULL;
static twInfoTable * content = NULL;

/*
 * twGetDirectoryStructure is not a public interface, so needs to be forward
 * declared.
 */
enum msgCodeEnum twGetDirectoryStructure(
    const char * entityName,
    twInfoTable * params,
    twInfoTable ** content
);

/* Tests. */

TEST_GROUP(unit_twFileTransfer_twGetDirectoryStructure);

TEST_SETUP(unit_twFileTransfer_twGetDirectoryStructure) {
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

TEST_TEAR_DOWN(unit_twFileTransfer_twGetDirectoryStructure) {
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        char * path = test_paths[idx];
        twDirectory_DeleteDirectory(path);
        TW_FREE(test_paths[idx]);
    }
    TW_FREE(test_paths);

    if (content) {
        twInfoTable_Delete(content);
        content = NULL;
    }

    TEST_ASSERT_EQUAL(TW_OK, twFileManager_Delete());
}

TEST_GROUP_RUNNER(unit_twFileTransfer_twGetDirectoryStructure) {
    RUN_TEST_CASE(unit_twFileTransfer_twGetDirectoryStructure, test_virtual_dirs_exist);
    RUN_TEST_CASE(unit_twFileTransfer_twGetDirectoryStructure, test_no_virtual_dirs);
}

/*
 * Test Case: Virtual directories for the thing have been added.
 *
 * Virtual directories should be returned in content.
 */
TEST(unit_twFileTransfer_twGetDirectoryStructure, test_virtual_dirs_exist) {
    size_t idx;
    enum msgCodeEnum result;

    for (idx = 0; idx < n_test_paths; ++idx) {
        twFileManager_AddVirtualDir(THING, pathnames[idx], test_paths[idx]);
    }

    result = twGetDirectoryStructure(THING, NULL, &content);

    TEST_ASSERT_EQUAL(TWX_SUCCESS, result);
    TEST_ASSERT_EQUAL(n_test_paths, content->rows->count);

    for (idx = 0; idx < n_test_paths; ++idx) {
        twInfoTableRow * row = twInfoTable_GetEntry(content, idx);
        twPrimitive * p = twInfoTableRow_GetEntry(row, 0);

        TEST_ASSERT_NOT_NULL(p);
        TEST_ASSERT_EQUAL(TW_STRING, p->type);
        TEST_ASSERT_TRUE(inArray(p->val.bytes.data, pathnames, n_test_paths));
    }
}

/*
 * Test Case: No virtual directories for the thing have been added.
 *
 * Content should contain no directories.
 */
TEST(unit_twFileTransfer_twGetDirectoryStructure, test_no_virtual_dirs) {
    enum msgCodeEnum result;

    result = twGetDirectoryStructure(THING, NULL, &content);

    TEST_ASSERT_EQUAL(TWX_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, content->rows->count);
}
