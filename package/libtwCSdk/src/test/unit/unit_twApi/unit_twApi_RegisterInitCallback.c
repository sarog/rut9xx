/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_RegisterInitCallback()
 */

#include <stddef.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

/* Test variables */
#ifdef _WIN32
extern __declspec(dllimport) twConfig * twcfg_pointer;
#endif
static init_cb * saved_init_cb = NULL;
static int global_userdata=0;

/* Helper functions */

/*
 * Return the allocated size of the heap space pointed to by p in bytes.
 *
 * NOTE: This is non-standard code, and relies upon compiler extensions and
 * preprocessor definitions identifying the compiler.
 *
 * References:
 * https://msdn.microsoft.com/en-us/library/b0084kay.aspx
 * https://stackoverflow.com/questions/1281686/
 * https://stackoverflow.com/questions/1936719/
 */
static size_t alloc_size(void * p) {
#if defined(__APPLE__)
    /* Apple. Must come before __GNUC__ check, because __GNUC__ is also defined
     * on MacOS builds.
     */
    return malloc_size(p);
#elif defined(__GNUC__)
    /* gcc. */
    return malloc_usable_size(p);
#elif defined(_WIN32)
    /* Visual Studio. */
    return _msize(p);
#else
    #error "alloc size not known to be supported in target compiler."
#endif
}

/*
 * Callback function for test.
 * Save off userdata in callback
 */
static void test_callback(int * userdata) {global_userdata = *userdata;};

/* Tests */

TEST_GROUP(unit_twApi_RegisterInitCallback);

TEST_SETUP(unit_twApi_RegisterInitCallback) {
    eatLogs();

    /* Save current state. */
#ifdef _WIN32
	saved_init_cb = twcfg_pointer->initCallback;
    twcfg_pointer->initCallback = NULL;
#else
    saved_init_cb = twcfg.initCallback;
    twcfg.initCallback = NULL;
#endif
}

TEST_TEAR_DOWN(unit_twApi_RegisterInitCallback) {
    twcfg_pointer->initCallback = NULL;
	twApi_Delete();
}

TEST_GROUP_RUNNER(unit_twApi_RegisterInitCallback) {
    RUN_TEST_CASE(unit_twApi_RegisterInitCallback, registerCallbackSuccess);
	RUN_TEST_CASE(unit_twApi_RegisterInitCallback, registerCallbackWithUserdata);
	RUN_TEST_CASE(unit_twApi_RegisterInitCallback, invokeInitCallback);
}

/**
 * Test Plan: Register an init callback and verify it has been set successfully
 */
TEST(unit_twApi_RegisterInitCallback, registerCallbackSuccess) {
    int userdata;

    /* Initial condition */
#ifdef _WIN32
    TEST_ASSERT_NULL(twcfg_pointer->initCallback);
#else
    TEST_ASSERT_NULL(twcfg.initCallback);
#endif

    /* Run */
    twApi_RegisterInitCallback(test_callback, &userdata);

    /* Verify */
#ifdef _WIN32
    TEST_ASSERT_NOT_NULL(twcfg_pointer->initCallback);
    TEST_ASSERT_EQUAL(test_callback, twcfg_pointer->initCallback->cb);
    TEST_ASSERT_EQUAL(&userdata, twcfg_pointer->initCallback->userdata);
    TEST_ASSERT_TRUE(alloc_size(twcfg_pointer->initCallback) >= sizeof(init_cb));
#else
    TEST_ASSERT_NOT_NULL(twcfg.initCallback);
    TEST_ASSERT_EQUAL(test_callback, twcfg.initCallback->cb);
    TEST_ASSERT_EQUAL(&userdata, twcfg.initCallback->userdata);
    TEST_ASSERT_TRUE(alloc_size(twcfg.initCallback) >= sizeof(init_cb));
#endif
}

/**
 * Test Plan: Set the userdata and register the callback, make sure the userdata is accessible through the callback by
 * copying the contents into a different variable (ie store it off somewhere else)
 */
TEST(unit_twApi_RegisterInitCallback, registerCallbackWithUserdata) {
    int userdata = 555;

    /* Run */
    twApi_RegisterInitCallback(test_callback, &userdata);
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);

    /* Verify */
    TEST_ASSERT_EQUAL(global_userdata, userdata);
}

static char initCallbackInvoked = FALSE;

int testInitCallback() {
    initCallbackInvoked = TRUE;
}

/**
* Test Plan: Registers and unregisters an api initialize callback function
*/

TEST(unit_twApi_RegisterInitCallback, invokeInitCallback) {
    TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterInitCallback(testInitCallback, NULL));
    TEST_ASSERT_EQUAL(testInitCallback, twcfg_pointer->initCallback->cb);
    TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
                                              MESSAGE_CHUNK_SIZE, TRUE));
    TEST_ASSERT_TRUE(initCallbackInvoked);
    TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterInitCallback(NULL, NULL));
}