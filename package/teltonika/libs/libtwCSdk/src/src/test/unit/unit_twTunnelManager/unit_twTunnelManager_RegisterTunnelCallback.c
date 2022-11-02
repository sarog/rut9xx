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

/*      Tunneling Event Callback           */
void tunnelCallbackFunc (char started, const char * tid, const char * thingName, const char * peerName, const char * host, int16_t port, DATETIME startTime,  DATETIME endTime, uint64_t bytesSent, uint64_t bytesRcvd, const char * type, const char * msg, void * userdata) {
	char startString[100] = "UNKNOWN";
	char endString[100] = "Still in Progress";
	uint32_t duration = endTime ? endTime - startTime : 0;

	if (startTime) twGetTimeString(startTime, startString, "%Y-%m-%d %H:%M:%S",99, TRUE, FALSE);
	if (endTime) twGetTimeString(endTime, endString, "%Y-%m-%d %H:%M:%S",99, TRUE, FALSE);

	TW_LOG(TW_AUDIT,"\n\n*****************\nTUNNEL NOTIFICATION:\nID: %s\nThingName: %s\nState: %s\nTarget: %s:%d\nStartTime: %s\nEndTime: %s\nDuration: %d msec\nUser: %s\nBytes Sent: %llu\nBytes Rcvd: %llu\nMessage: %s\n*****************\n", tid ? tid : "UNKNOWN", thingName ? thingName : "UNKNOWN", started ? "STARTED" : "ENDED", host ? host : "UNKNOWN", port, startString, endString, duration, peerName ? peerName : "Unknown", bytesSent, bytesRcvd, msg ? msg : "" );

}

TEST_GROUP(unit_twTunnelManager_RegisterTunnelCallback);

TEST_SETUP(unit_twTunnelManager_RegisterTunnelCallback) {
	eatLogs();

	/* need to init api in order to init stubs */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	/* init tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Create(), "unit_twTunnelManager_RegisterTunnelCallback_SETUP: Error creating tunnel manager ");
}

TEST_TEAR_DOWN(unit_twTunnelManager_RegisterTunnelCallback) {
	/* delete tunnel manager */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_Delete(), "unit_twTunnelManager_RegisterTunnelCallback_TEAR_DOWN: Error deleting tunnel manager ");

	/* reset stubs */
	twStubs_Reset();

	/* delete api */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "Error deleting Api");

}

TEST_GROUP_RUNNER(unit_twTunnelManager_RegisterTunnelCallback) {
	RUN_TEST_CASE(unit_twTunnelManager_RegisterTunnelCallback, test_twTunnelManager_RegisterTunnelCallback);
}

TEST(unit_twTunnelManager_RegisterTunnelCallback, test_twTunnelManager_RegisterTunnelCallback) {
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twTunnelManager_RegisterTunnelCallback(tunnelCallbackFunc, NULL, NULL), "Error registering tunnel callback");
	TEST_ASSERT_EQUAL_MESSAGE(TW_INVALID_PARAM, twTunnelManager_RegisterTunnelCallback(NULL, NULL, NULL), "Error registering null callback");
}