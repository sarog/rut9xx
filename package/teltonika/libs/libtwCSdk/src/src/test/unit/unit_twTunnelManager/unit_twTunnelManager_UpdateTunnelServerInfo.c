#include "twApi.h"
#include "twTunnelManager.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

TEST_GROUP(unit_twTunnelManager_UpdateTunnelServerInfo);

TEST_SETUP(unit_twTunnelManager_UpdateTunnelServerInfo) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_UpdateTunnelServerInfo_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_UpdateTunnelServerInfo) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_UpdateTunnelServerInfo_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_UpdateTunnelServerInfo) {
	RUN_TEST_CASE(unit_twTunnelManager_UpdateTunnelServerInfo, test_twTunnelManager_UpdateTunnelServerInfo);
}

TEST(unit_twTunnelManager_UpdateTunnelServerInfo, test_twTunnelManager_UpdateTunnelServerInfo) {
	/* no return values to test here, just code coverage */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twTunnelManager_UpdateTunnelServerInfo(NULL, TW_PORT, TW_APP_KEY));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twTunnelManager_UpdateTunnelServerInfo(TW_HOST, NULL, TW_APP_KEY));
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twTunnelManager_UpdateTunnelServerInfo(TW_HOST, TW_PORT, NULL));
	TEST_ASSERT_EQUAL(TW_OK, twTunnelManager_UpdateTunnelServerInfo(TW_HOST, TW_PORT, TW_APP_KEY));
}