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

TEST_GROUP(unit_twTunnelManager_SetX509Fields);

TEST_SETUP(unit_twTunnelManager_SetX509Fields) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_SetX509Fields_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_SetX509Fields) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_SetX509Fields_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_SetX509Fields) {
	RUN_TEST_CASE(unit_twTunnelManager_SetX509Fields, test_twTunnelManager_SetX509Fields);
}

TEST(unit_twTunnelManager_SetX509Fields, test_twTunnelManager_SetX509Fields) {
	/* no return values to test here, just code coverage */
	twTunnelManager_SetX509Fields(TEST_SUBJECT_CN_1, TEST_SUBJECT_O_1, TEST_SUBJECT_OU_1, TEST_ISSUER_CN_1, TEST_ISSUER_O_1, TEST_ISSUER_OU_1);
	twTunnelManager_SetX509Fields(NULL, NULL, NULL, NULL, NULL, NULL);
}