#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "twThreads.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"

int receiveSynchronizeStateEventNumberOfTimesCalled;
TEST_GROUP(SynchronizeStateIntegration);

void test_SynchronizeStateIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(SynchronizeStateIntegration) {
	eatLogs();

	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;
}

TEST_TEAR_DOWN(SynchronizeStateIntegration) {
}

TEST_GROUP_RUNNER(SynchronizeStateIntegration) {
	RUN_TEST_CASE(SynchronizeStateIntegration, testReceiveSynchronizeStateEvent);
	RUN_TEST_CASE(SynchronizeStateIntegration, testReceiveAllSynchronizeStateEvent);
}

char receiveSynchronizeStateEventBindEventHandlerFlag=FALSE;
void receiveSynchronizeStateEventBindEventHandler(char * entityName,twInfoTable* subscriptionData, void * userdata){
	receiveSynchronizeStateEventBindEventHandlerFlag=TRUE;
	TEST_ASSERT_EQUAL_STRING("SteamSensorEdge2166",entityName);
	TEST_ASSERT_NOT_NULL(subscriptionData);
	TEST_ASSERT_NOT_NULL(subscriptionData->ds);
	TEST_ASSERT_TRUE(2<subscriptionData->rows);
}

void receiveSynchronizeStateEventBindEventHandler1(char * entityName,twInfoTable* subscriptionData, void * userdata){
	receiveSynchronizeStateEventNumberOfTimesCalled++;
	if(receiveSynchronizeStateEventNumberOfTimesCalled==2)
		receiveSynchronizeStateEventBindEventHandlerFlag=TRUE;
	TEST_ASSERT_NOT_NULL(entityName);
	TEST_ASSERT_TRUE(strncmp("SteamSensorEdge2166",entityName,19) == 0||strncmp("One_Int_Prop",entityName,12) == 0);
	TEST_ASSERT_NOT_NULL(subscriptionData);
	TEST_ASSERT_NOT_NULL(subscriptionData->ds);
	TEST_ASSERT_TRUE(2<subscriptionData->rows);
}


/**
 * Test Plan: Register a thing and a synchronized state event handler, then bind and connect.
 * Test should wait a reasonable amount of time for receipt of the synchronized state event
 * and verify its parameters.
 */
TEST(SynchronizeStateIntegration, testReceiveSynchronizeStateEvent) {

	int count=0;
	const char* thingName = "SteamSensorEdge2166";
	twThread * api_thread = NULL;
	twThread * workerThread = NULL;

	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("CSDK-1045");
	}


	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_SteamSensorEdge2166.xml"));
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_One_Int_Prop.xml"));

	TEST_ASSERT_EQUAL(TW_OK,twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SynchronizeStateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));

	TEST_ASSERT_FALSE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_FALSE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_EQUAL(TW_OK,twApi_BindThing((char*)thingName));

	twApi_RegisterSynchronizeStateEventCallback((char*)thingName,receiveSynchronizeStateEventBindEventHandler,NULL);

	twApi_DisableCertValidation();

	api_thread = twThread_Create(twApi_TaskerFunction, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != api_thread, "api thread cannot be created");

	workerThread = twThread_Create(twMessageHandler_msgHandlerTask, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != workerThread, "message thread cannot be created");

	TEST_ASSERT_EQUAL(TW_OK,twApi_Connect(15000,3));

	/* Wait for Synchronized State Event */
	for(count=0;count<15;count++){
		if(receiveSynchronizeStateEventBindEventHandlerFlag)
			break;
		twSleepMsec(1000);
	}

	TEST_ASSERT_TRUE_MESSAGE(receiveSynchronizeStateEventBindEventHandlerFlag,"The Synchronize State handler was not called within 10 seconds oc connecting.")
	TEST_ASSERT_TRUE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_EQUAL(TW_OK,twApi_UnregisterSynchronizeStateEventCallback((char*)thingName,receiveSynchronizeStateEventBindEventHandler,NULL));

	twThread_Delete(api_thread);
	twThread_Delete(workerThread);
	twApi_Disconnect("testing");
	twApi_Delete();

}

TEST(SynchronizeStateIntegration, testReceiveAllSynchronizeStateEvent) {

	int count=0;
	const char* thingName = "SteamSensorEdge2166";
	twThread * api_thread = NULL;
	twThread * workerThread = NULL;
	receiveSynchronizeStateEventNumberOfTimesCalled = 0;

	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("CSDK-1046");
	}

	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_SteamSensorEdge2166.xml"));
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_One_Int_Prop.xml"));

	TEST_ASSERT_EQUAL(TW_OK,twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SynchronizeStateIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));

	TEST_ASSERT_FALSE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_FALSE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_EQUAL(TW_OK,twApi_BindThing((char*)thingName));
	TEST_ASSERT_EQUAL(TW_OK,twApi_BindThing("One_Int_Prop"));

	twApi_RegisterSynchronizeStateEventCallback(NULL,receiveSynchronizeStateEventBindEventHandler1,NULL);

	twApi_DisableCertValidation();

	api_thread = twThread_Create(twApi_TaskerFunction, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != api_thread, "api thread cannot be created");

	workerThread = twThread_Create(twMessageHandler_msgHandlerTask, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != workerThread, "message thread cannot be created");

	TEST_ASSERT_EQUAL(TW_OK,twApi_Connect(15000,3));

	/* Wait for Synchronized State Event */
	for(count=0;count<10;count++){
		if(receiveSynchronizeStateEventBindEventHandlerFlag)
			break;
		twSleepMsec(1000);
	}

	TEST_ASSERT_TRUE_MESSAGE(receiveSynchronizeStateEventBindEventHandlerFlag,"The Synchronize State handler was not called within 10 seconds oc connecting.")
	TEST_ASSERT_TRUE(tw_api->firstSynchronizationComplete);

	TEST_ASSERT_EQUAL(TW_OK,twApi_UnregisterSynchronizeStateEventCallback(NULL,receiveSynchronizeStateEventBindEventHandler1,NULL));

	twThread_Delete(api_thread);
	twThread_Delete(workerThread);
	twApi_Disconnect("testing");
	twApi_Delete();

}
