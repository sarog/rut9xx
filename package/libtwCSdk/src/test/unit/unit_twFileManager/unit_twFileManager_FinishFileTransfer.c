/*
 * Copyright (c) 2017 PTC, Inc. All rights reserved.
 */
#include <unity_fixture.h>
#include <TestUtilities.h>

/* Test variables */
twFileTransferInfo * ft;
twFile * fp1;
twFile * fp2;
int close_calls = 0;
int file_callbacks = 0;

void resetTestVariables() {
    ft = NULL;
    fp1 = NULL;
    fp2 = NULL;
    close_calls = 0;
    file_callbacks = 0;
}

/* Mocks */
twFile * GetOpenFile_return_null(
    const char * thingName,
    const char * path,
    const char * filename,
    const char * tid,
    char * isTimedOut
) {
    return NULL;
}

twFile * GetOpenFile_return_path(
    const char * thingName,
    const char * path,
    const char * filename,
    const char * tid,
    char * isTimedOut
) {
    if(path)
        TW_LOG(TW_INFO,"GetOpenFile_return_path %s: fp NOT NULL",path);
    else
        TW_LOG(TW_INFO,"GetOpenFile_return_path: fp == NULL");
    return (path) ? fp1 : NULL;
}

twFile * GetOpenFile_return_tid(
    const char * thingName,
    const char * path,
    const char * filename,
    const char * tid,
    char * isTimedOut
) {
    return (tid) ? fp1 : NULL;
}

twFile * GetOpenFile_return_same(
    const char * thingName,
    const char * path,
    const char * filename,
    const char * tid,
    char * isTimedOut
) {
    return fp1;
}

twFile * GetOpenFile_return_different(
    const char * thingName,
    const char * path,
    const char * filename,
    const char * tid,
    char * isTimedOut
) {
    return (path) ? fp1 : fp2;
}

void CloseFile(void * fp) {
    ++close_calls;
}

void MakeFileCallback(char rcvd, twFileTransferInfo * fti) {
    ++file_callbacks;
}

/* Tests */

TEST_GROUP(unit_twFileManager_FinishFileTransfer);

TEST_SETUP(unit_twFileManager_FinishFileTransfer) {
    eatLogs();
    resetTestVariables();
    ft = (twFileTransferInfo *) TW_CALLOC(sizeof(twFileTransferInfo), 1);
    fp1 = (twFile *) TW_CALLOC(sizeof(twFile), 1);
    fp2 = (twFile *) TW_CALLOC(sizeof(twFile), 1);

    twApi_stub->twFileManager_MakeFileCallback = MakeFileCallback;
    twApi_stub->twFileManager_CloseFile = CloseFile;

    TEST_ASSERT_EQUAL(TW_OK, twFileManager_Create());
}

TEST_TEAR_DOWN(unit_twFileManager_FinishFileTransfer) {
    TEST_ASSERT_EQUAL(TW_OK, twFileManager_Delete());
    TW_FREE(fp2);
    TW_FREE(fp1);
    TW_FREE(ft);
    twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twFileManager_FinishFileTransfer) {
    RUN_TEST_CASE(
        unit_twFileManager_FinishFileTransfer,
        test_FinishFileTransfer_no_tid_no_path
    );
    RUN_TEST_CASE(
        unit_twFileManager_FinishFileTransfer,
        test_FinishFileTransfer_no_tid
    );
    RUN_TEST_CASE(
        unit_twFileManager_FinishFileTransfer,
        test_FinishFileTransfer_no_path
    );
    RUN_TEST_CASE(
        unit_twFileManager_FinishFileTransfer,
        test_FinishFileTransfer_tid_path_different
    );
    RUN_TEST_CASE(
        unit_twFileManager_FinishFileTransfer,
        test_FinishFileTransfer_tid_path_same
    );
}

/*
 * Test Case: No transfer ID nor path info in file manager.
 *
 * twFileManager_FinishFileTransfer should return an error.
 */
TEST(unit_twFileManager_FinishFileTransfer, test_FinishFileTransfer_no_tid_no_path) {
    char * entity = "ThingName";
    char * path = NULL;
    char * adjustedPath = NULL;
    enum msgCodeEnum res;

    /* Setup */
    twApi_stub->twFileManager_GetOpenFile = GetOpenFile_return_null;

    /* Run */
    res = twFileManager_FinishFileTransfer(entity, ft, path, adjustedPath);

    /* Verify */
    TEST_ASSERT_EQUAL(TWX_INTERNAL_SERVER_ERROR, res);
}

/*
 * Test Case: No transfer ID, path exists.
 *
 * twFileManager_FinishFileTransfer should succeed.
 */
TEST(unit_twFileManager_FinishFileTransfer, test_FinishFileTransfer_no_tid) {
    char * entity = "ThingName";
    char * path = "path";
    char * adjustedPath = "adjusted_path";
    enum msgCodeEnum res;
    TW_LOG(TW_INFO,"STARTING test_FinishFileTransfer_no_tid");
    /* Setup */
    twApi_stub->twFileManager_GetOpenFile = GetOpenFile_return_path;

    /* Run */
    res = twFileManager_FinishFileTransfer(entity, ft, path, adjustedPath);

    /* Verify */
    TEST_ASSERT_EQUAL(TWX_SUCCESS, res);
    TEST_ASSERT_EQUAL(1, file_callbacks);
    TEST_ASSERT_EQUAL(1, close_calls);
    TW_LOG(TW_INFO,"ENDING test_FinishFileTransfer_no_tid");
}

/*
 * Test Case: transfer ID exits, path does not.
 *
 * twFileManager_FinishFileTransfer should succeed.
 */
TEST(unit_twFileManager_FinishFileTransfer, test_FinishFileTransfer_no_path) {
    char * entity = "ThingName";
    char * path = "path";
    char * adjustedPath = "adjusted_path";
    enum msgCodeEnum res;

    /* Setup */
    twApi_stub->twFileManager_GetOpenFile = GetOpenFile_return_tid;
    ft->transferId = (void *) 1;

    /* Run */
    res = twFileManager_FinishFileTransfer(entity, ft, path, adjustedPath);

    /* Verify */
    TEST_ASSERT_EQUAL(TWX_SUCCESS, res);
    TEST_ASSERT_EQUAL(1, file_callbacks);
    TEST_ASSERT_EQUAL(1, close_calls);
}

/*
 * Test Case: transfer ID, path exist, point to different files.
 *
 * twFileManager_FinishFileTransfer should succeed.
 */
TEST(unit_twFileManager_FinishFileTransfer, test_FinishFileTransfer_tid_path_different) {
    char * entity = "ThingName";
    char * path = "path";
    char * adjustedPath = "adjusted_path";
    enum msgCodeEnum res;

    /* Setup */
    twApi_stub->twFileManager_GetOpenFile = GetOpenFile_return_different;
    ft->transferId = (void *) 1;

    /* Run */
    res = twFileManager_FinishFileTransfer(entity, ft, path, adjustedPath);

    /* Verify */
    TEST_ASSERT_EQUAL(TWX_SUCCESS, res);
    TEST_ASSERT_EQUAL(1, file_callbacks);
    TEST_ASSERT_EQUAL(2, close_calls);
}

/*
 * Test Case: transfer ID, path exist, point to same file.
 *
 * twFileManager_FinishFileTransfer should succeed.
 */

TEST(unit_twFileManager_FinishFileTransfer, test_FinishFileTransfer_tid_path_same) {
    char * entity = "ThingName";
    char * path = "path";
    char * adjustedPath = "adjusted_path";
    enum msgCodeEnum res;

    /* Setup */
    twApi_stub->twFileManager_GetOpenFile = GetOpenFile_return_same;
    ft->transferId = (void *) 1;

    /* Run */
    res = twFileManager_FinishFileTransfer(entity, ft, path, adjustedPath);

    /* Verify */
    TEST_ASSERT_EQUAL(TWX_SUCCESS, res);
    TEST_ASSERT_EQUAL(1, file_callbacks);
    TEST_ASSERT_EQUAL(1, close_calls);
}