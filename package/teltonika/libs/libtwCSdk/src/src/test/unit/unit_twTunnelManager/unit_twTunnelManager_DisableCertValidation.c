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

TEST_GROUP(unit_twTunnelManager_DisableCertValidation);

TEST_SETUP(unit_twTunnelManager_DisableCertValidation) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_DisableCertValidation_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_DisableCertValidation) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_DisableCertValidation_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_DisableCertValidation) {
	RUN_TEST_CASE(unit_twTunnelManager_DisableCertValidation, test_twTunnelManager_DisableCertValidation);
}

TEST(unit_twTunnelManager_DisableCertValidation, test_twTunnelManager_DisableCertValidation) {
	/* no return values to test here, just code coverage */
	twTunnelManager_DisableCertValidation(TRUE);
	twTunnelManager_DisableCertValidation(FALSE);
}