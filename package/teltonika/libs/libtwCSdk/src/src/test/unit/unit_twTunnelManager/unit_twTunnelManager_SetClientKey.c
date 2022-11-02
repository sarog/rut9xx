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

TEST_GROUP(unit_twTunnelManager_SetClientKey);

TEST_SETUP(unit_twTunnelManager_SetClientKey) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_SetClientKey_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_SetClientKey) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_SetClientKey_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_SetClientKey) {
	RUN_TEST_CASE(unit_twTunnelManager_SetClientKey, test_twTunnelManager_SetClientKey);
}

TEST(unit_twTunnelManager_SetClientKey, test_twTunnelManager_SetClientKey) {
	/* no return values to test here, just code coverage */
	twTunnelManager_SetClientKey(TEST_CLIENT_KEY_FILE, TEST_CLIENT_KEY_PASS, TEST_TYPE);
	twTunnelManager_SetClientKey(NULL, TEST_CLIENT_KEY_PASS, TEST_TYPE);
	twTunnelManager_SetClientKey(TEST_CLIENT_KEY_FILE, NULL, TEST_TYPE);
}