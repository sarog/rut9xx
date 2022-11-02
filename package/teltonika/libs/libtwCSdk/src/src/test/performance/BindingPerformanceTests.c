/*
*	Created by Jeff Dreyer on 5/26/16.
*	Copyright 2016, PTC, Inc.
*/

#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10
#define NAME_SIZE 64

#include "twBaseTypes.h"
#include "twThreads.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"

#define TW_NO_RETURN_DATASHAPE NULL
#define TW_NO_USER_DATA NULL
#define CONNECTION_RETRIES 1
#define CONNECTION_TIMEOUT 2000

#define NUMBER_OF_THINGS 1000
#define ERROR_MESSAGE_LENGTH 100

TEST_GROUP(BindingPerformance);

void test_BindingPerformanceAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(BindingPerformance) {
    eatLogs();
    /* errorLogs(); */
}

TEST_TEAR_DOWN(BindingPerformance) {
}

TEST_GROUP_RUNNER(BindingPerformance) {
    RUN_TEST_CASE(BindingPerformance, perfBindAfterConnect);
	RUN_TEST_CASE(BindingPerformance, perfBindBeforeConnect);
}

char areAllEntitiesBound(){
	int index;
	char thingName[50];

	/* Wait for all things to report being bound */
	for(index=0;index<NUMBER_OF_THINGS;index++){
		sprintf(thingName,"SteamPerf-%i",index);
		if(twApi_IsEntityBound(thingName))
			return FALSE;
	}

	return TRUE;

}

/**
 * Test Plan: Repeatedly bind a thing after a connection is established and measure the time for a round trip to the
 * server to confirm the bind request. Repeat this processess for 1000 Things to measure how service execution time
 * degrades more bind requests are sent to the server.
 *
 * Pre-conditions: The server must have NUMBER_OF_THINGS pre installed on it, each using the name pattern SteamPerf-%i
 * and having 50 services, each having the pattern Service1 - Service50
 *
 * Driving Metric: The total number of bound things in the bind list.
 *
 * Expected Outcome: averageDuration should be insensitive to NUMBER_OF_THINGS used in the test and should never
 * exceed 300ms on average for a bind request.
 *
 * Expected Execution Time: Total test execution time will be approximatly NUMBER_OF_THINGS * 0.3 seconds or 5-8
 * Minutes for 1000 Things
 */
TEST(BindingPerformance,perfBindAfterConnect){
    int index;
    char thingName[50], errorMessageBuffer[ERROR_MESSAGE_LENGTH];
	long totalDuration = 0;
	double averageDuration;
	twThread *apiThread;
	twThread *messageHandlerThread;

    TEST_ASSERT_EQUAL(
		TW_OK,
		twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE)
	);
    twApi_DisableCertValidation();
    apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	messageHandlerThread = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);

    TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES));

    for(index=0;index<NUMBER_OF_THINGS;index++){
        sprintf(thingName,"SteamPerf-%i",index);
        {
			MARK_START("csdk.performance.perfBindAfterConnect.bindtime");
			METRIC("csdk.performance.perfBindAfterConnect.thingcount",index);
			twApi_BindThing(thingName);
            MARK_END("csdk.performance.perfBindAfterConnect.bindtime")
			totalDuration+=duration;
        }
    }

	averageDuration = totalDuration / NUMBER_OF_THINGS;

    TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Disconnect("End perfBindAfterConnect"));
    TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Delete());
    twThread_Delete(apiThread);
	twThread_Delete(messageHandlerThread);

	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 300 ms, was %f ms for %i things ",averageDuration,NUMBER_OF_THINGS);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<300.0,errorMessageBuffer);

	return;
}

/**
 * Test Plan: Repeatedly bind a things before a connection is established and measure the time for a round trip to the
 * server to confirm the bind request which should contain all bound things in one message completes.
 *
 * Pre-conditions: The server must have NUMBER_OF_THINGS pre installed on it, each using the name pattern SteamPerf-%i
 * and having 50 services, each having the pattern Service1 - Service50
 *
 * Driving Metric: The total number of pre bound things in the bind list.
 *
 * Expected Outcome: duration should be increase linearly relative to NUMBER_OF_THINGS used in the test and should never
 * exceed 5 ms on average for a bind request containing 1000 things.
 *
 * Expected Execution Time: Total test execution time will be approximatly 4-5 seconds for 1000 things
 * for 1000 Things
 */
TEST(BindingPerformance,perfBindBeforeConnect){
	int index,retryCount;
	char thingName[50], errorMessageBuffer[ERROR_MESSAGE_LENGTH];
	double averageDuration;
	twThread *apiThread;
	twThread *messageHandlerThread;

	TEST_ASSERT_EQUAL(
			TW_OK,
			twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE)
	);
	twApi_DisableCertValidation();
	apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	messageHandlerThread = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);

	for(index=0;index<NUMBER_OF_THINGS;index++){
		sprintf(thingName,"SteamPerf-%i",index);
		twApi_BindThing(thingName);
	}

	{
		MARK_START("csdk.performance.perfBindBeforeConnect.bindtime");
		TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES));

		retryCount = 0;
		while(!areAllEntitiesBound()&& retryCount < 20){
			retryCount++;
			twSleepMsec(100);
		}

		MARK_END("csdk.performance.perfBindBeforeConnect.bindtime")
		averageDuration = duration / NUMBER_OF_THINGS;
	}
	TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Disconnect("End perfBindAfterConnect"));
	TEST_ASSERT_EQUAL_INT(TW_OK,twApi_Delete());
	twThread_Delete(apiThread);
	twThread_Delete(messageHandlerThread);

	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 150 ms, was %f ms for %i things ",averageDuration,NUMBER_OF_THINGS);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<5.0,errorMessageBuffer);

}


