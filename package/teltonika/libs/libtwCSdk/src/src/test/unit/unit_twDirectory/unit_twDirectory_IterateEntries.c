/*
 * Copyright (c) 2017 PTC, Inc. All rights reserved.
 */
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

static char const * dirnames[] = {
    "IterateEntriesTest",
    "Iterate Entries Test"
};
static size_t const n_test_paths = 2;  /* Number of dirname entries */
static char ** test_paths = NULL;

/* Tests */
TEST_GROUP(unit_twDirectory_IterateEntires);

TEST_SETUP(unit_twDirectory_IterateEntires) {
    size_t idx;
    char * cwd;

    eatLogs();

    test_paths = TW_CALLOC(sizeof(*test_paths), n_test_paths);
    cwd = getCurrentDirectory();
    for (idx = 0; idx < n_test_paths; ++idx) {
        test_paths[idx] = joinPath(cwd, dirnames[idx]);
        twDirectory_CreateDirectory(test_paths[idx]);
    }
    TW_FREE(cwd);
}

TEST_TEAR_DOWN(unit_twDirectory_IterateEntires) {
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        char * path = test_paths[idx];
        twDirectory_DeleteDirectory(path);
        TW_FREE(test_paths[idx]);
    }
    TW_FREE(test_paths);
}

TEST_GROUP_RUNNER(unit_twDirectory_IterateEntires) {
    RUN_TEST_CASE(unit_twDirectory_IterateEntires, test_iterate_empty_directory);
    RUN_TEST_CASE(unit_twDirectory_IterateEntires, test_iterate_nonempty_directory);
	RUN_TEST_CASE(unit_twDirectory_IterateEntires, test_iterate_file);
	RUN_TEST_CASE(unit_twDirectory_IterateEntires, test_iterate_missing);
}

/*
 * Test Case: Iterate empty directory.
 *
 * IterateEntries should return the current directory ('.') and the parent
 * directory (..')
 */
TEST(unit_twDirectory_IterateEntires, test_iterate_empty_directory) {
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        TW_DIR hnd = 0;
        twFile fileInfo;
        char * path = test_paths[idx];
        size_t dirItems = 0;

        for (
            hnd = twDirectory_IterateEntries(
                path,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            );
            hnd != 0;
            hnd = twDirectory_IterateEntries(
                path,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            )
            ) {
            ++dirItems;
        }
        TEST_ASSERT_EQUAL(2, dirItems); /* '.' and '..' */
    }

}

/*
 * Test Case: Iterate non-empty directory.
 *
 * IterateEntries should return the current directory ('.'), the parent
 * directory ('..'), and all the files in the directory.
 */
TEST(unit_twDirectory_IterateEntires, test_iterate_nonempty_directory) {
    char const * directories[] = { ".", ".." };
    char const * files[] = { "foo", "bar", "space in name" };
    size_t const n_files = 3;
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        char * path = test_paths[idx];
        size_t dirItems = 0;
        TW_DIR hnd = 0;
        twFile fileInfo;
        size_t i;

        /* Populate directory. */
        for (i = 0; i < n_files; ++i) {
            char const * fname = files[i];
            char * pathname = joinPath(path, fname);
            twDirectory_CreateFile(pathname);
            TW_FREE(pathname);
        }

        /* Iterate */
        for (
            hnd = twDirectory_IterateEntries(
                path,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            );
            hnd != 0;
            hnd = twDirectory_IterateEntries(
                path,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            )
        ) {
            TEST_ASSERT_TRUE(
                inArray(fileInfo.name, directories, 2) ||
                    inArray(fileInfo.name, files, 3)
            );
            ++dirItems;
        }

        TEST_ASSERT_EQUAL(n_files + 2, dirItems); /* '.', '..', and existing files */
    }
}

/*
 * Test Case: Iterate over a file.
 *
 * IterateEntries should return a zero handle, and no iterations should occur.
 */
TEST(unit_twDirectory_IterateEntires, test_iterate_file) {
    size_t idx;

    for (idx = 0; idx < n_test_paths; ++idx) {
        char * path = test_paths[idx];
        TW_DIR hnd = 0;
        twFile fileInfo;
        size_t dirItems = 0;

        /* Populate directory. */
        char * pathname = joinPath(path, "foo");
        twDirectory_CreateFile(pathname);

        /* Iterate */
        for (
            hnd = twDirectory_IterateEntries(
                pathname,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            );
            hnd != 0;
            hnd = twDirectory_IterateEntries(
                pathname,
                hnd,
                &fileInfo.name,
                &fileInfo.size,
                &fileInfo.lastModified,
                &fileInfo.isDir,
                &fileInfo.readOnly
            )
            ) {
            ++dirItems;
        }

        TEST_ASSERT_EQUAL(0, dirItems);
    }
}

/*
 * Test Case: Iterate a bad path.
 *
 * IterateEntries should return a zero handle, and no iteration should occur.
 */
TEST(unit_twDirectory_IterateEntires, test_iterate_missing) {
	char * path = "nonexistant_directory";
    TW_DIR hnd = 0;
    twFile fileInfo;
	size_t dirItems = 0;

	/* Iterate */
    for (
        hnd = twDirectory_IterateEntries(
            path,
            hnd,
            &fileInfo.name,
            &fileInfo.size,
            &fileInfo.lastModified,
            &fileInfo.isDir,
            &fileInfo.readOnly
        );
        hnd != 0;
        hnd = twDirectory_IterateEntries(
            path,
            hnd,
            &fileInfo.name,
            &fileInfo.size,
            &fileInfo.lastModified,
            &fileInfo.isDir,
            &fileInfo.readOnly
        )
    ) {
        ++dirItems;
    }

    TEST_ASSERT_EQUAL(0, dirItems);
}
