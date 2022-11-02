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

static void test_TunnelingUnitProxyPass_callback(char *passWd, unsigned int len) {
	strncpy(passWd, TEST_PROXY_PASS, len);
}

TEST_GROUP(unit_twTunnelManager_SetProxyInfo);

TEST_SETUP(unit_twTunnelManager_SetProxyInfo) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_SetProxyInfo_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_SetProxyInfo) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_SetProxyInfo_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_SetProxyInfo) {
	RUN_TEST_CASE(unit_twTunnelManager_SetProxyInfo, test_twTunnelManager_SetProxyInfo);
}

TEST(unit_twTunnelManager_SetProxyInfo, test_twTunnelManager_SetProxyInfo) {
	/* no return values to test here, just code coverage */
	twTunnelManager_SetProxyInfo(NULL, TEST_PROXY_PORT, TEST_PROXY_USER, test_TunnelingUnitProxyPass_callback);
	twTunnelManager_SetProxyInfo(TEST_PROXY_HOST, 0, TEST_PROXY_USER, test_TunnelingUnitProxyPass_callback);
	twTunnelManager_SetProxyInfo(TEST_PROXY_HOST, TEST_PROXY_PORT, TEST_PROXY_USER, test_TunnelingUnitProxyPass_callback);
}
