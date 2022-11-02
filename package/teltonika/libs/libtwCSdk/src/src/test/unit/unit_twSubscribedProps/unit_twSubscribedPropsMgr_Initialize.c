/**
 * Subscribed Property Unit Tests
 */

#include <twTls.h>
#include <twBaseTypes.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twExt.h"

/**
 * API
 */
#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/**
 * Unity Test Macros
 */
TEST_GROUP(unit_twSubscribedPropsMgr_Initialize);

TEST_SETUP(unit_twSubscribedPropsMgr_Initialize) {
	eatLogs();
	twTest_DeleteAndSetPersistedBinFileLocations();
}
TEST_TEAR_DOWN(unit_twSubscribedPropsMgr_Initialize) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}
TEST_GROUP_RUNNER(unit_twSubscribedPropsMgr_Initialize) {
	RUN_TEST_CASE(unit_twSubscribedPropsMgr_Initialize, unit_twSubscribedPropsMgr_Initialize_Initialize);
	RUN_TEST_CASE(unit_twSubscribedPropsMgr_Initialize, InitializeApiWithNullPersistedStorageDirectories);
}

/**
 * unit_twSubscribedPropsMgr_Initialize_Initialize
 *
 *
 * Test Plan: initialize and delete the subscribed property manager with multiple different starting conditions
 */
TEST(unit_twSubscribedPropsMgr_Initialize, unit_twSubscribedPropsMgr_Initialize_Initialize) {
	twApi_CreateStubs();

	/* default properties */
	TEST_ASSERT_EQUAL(TW_OK, twSubscribedPropsMgr_Initialize());
	twSubscribedPropsMgr_Delete();

	/* null directories */
	twcfg_pointer->subscribed_props_dir = NULL;
	twcfg_pointer->offline_msg_store_dir = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twSubscribedPropsMgr_Initialize());
	twSubscribedPropsMgr_Delete();

	/* reset twcfg members */
	twcfg_pointer->subscribed_props_dir = OFFLINE_MSG_STORE_DIR;
	twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_DIR;

	twApi_DeleteStubs();
}
TEST(unit_twSubscribedPropsMgr_Initialize, InitializeApiWithNullPersistedStorageDirectories) {
	twcfg_pointer->subscribed_props_dir = NULL;
	twcfg_pointer->offline_msg_store_dir = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}