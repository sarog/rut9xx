/*
*	Created by Jeff Dreyer on 5/26/16.
*	Copyright 2016, PTC, Inc.
*/

#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE

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

/* Should default to 1000 things */
#define NUMBER_OF_THINGS 1000
#define ERROR_MESSAGE_LENGTH 100

TEST_GROUP(ServiceExecutionPerformance);

void test_ServiceExecutionPerformanceAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(ServiceExecutionPerformance) {
	eatLogs();
}

TEST_TEAR_DOWN(ServiceExecutionPerformance) {
}

TEST_GROUP_RUNNER(ServiceExecutionPerformance) {
	RUN_TEST_CASE(ServiceExecutionPerformance, perfServiceExecuteTimeVsCount);
}

enum msgCodeEnum genericPrefTestServiceHandler(const char *entityName, const char *serviceName, twInfoTable *params,
											   twInfoTable **content, void *userdata) {
	*content = twInfoTable_CreateFromString("result", "OK", TRUE);
	return TWX_SUCCESS;
}

void bindThingWithNServices(char *thingName, int serviceCount) {
	{
		char serviceName[50];
		int index;

		for (index = 1; index < serviceCount + 1; index++) {
			twDataShapeEntry *inputParam1DsEntry;
			twDataShape *inputParamsDataShape;
			sprintf(serviceName, "Service%i", index);

			// Define a service for this shape
			inputParam1DsEntry = twDataShapeEntry_Create("SleepMilliSeconds", "The first parameter",
																		   TW_NUMBER);
			inputParamsDataShape = twDataShape_Create(inputParam1DsEntry);

			twApi_RegisterService(TW_THING, thingName, serviceName,
								  "Create a short message using propertyA, propertyB and paramter1 of this service.",
								  inputParamsDataShape, TW_STRING, TW_NO_RETURN_DATASHAPE,
								  genericPrefTestServiceHandler,
								  TW_NO_USER_DATA);

		}
		twApi_BindThing(thingName);
	}

}

/**
 * Test Plan: Repeatedly bind a thing with 50 services. After each binding, call the 50th service and measure the time
 * it takes to execute that service. Repeat this processess for 1000 Things to measure how service execution time
 * degrades as more services are registered as more things are registered.
 *
 * Pre-conditions: The server must have NUMBER_OF_THINGS pre installed on it, each using the name pattern SteamPerf-%i
 * and having 50 services, each having the pattern Service1 - Service50
 *
 * Driving Metric: The total number of registered services in the master callback list. This is 50 x the number of
 * bound things. This should be evaluated from 1-1000 Things or 1-50,000 Registered services.
 *
 * Expected Outcome: averageDuration should be insensitive to NUMBER_OF_THINGS used in the test and should never
 * exceed 500ms per a service invocation.
 *
 * Expected Execution Time: Total test execution time will be approximatly NUMBER_OF_THINGS * 0.5 seconds or 8-10
 * Minutes for 1000 Things
 */
TEST(ServiceExecutionPerformance, perfServiceExecuteTimeVsCount) {
   twThread *apiThread;
   twThread *messageHandlerThread;
   int index;
   long totalDuration = 0;
   double averageDuration;
	char thingName[50], errorMessageBuffer[ERROR_MESSAGE_LENGTH];
	twInfoTable *result;
	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ServiceExecutionPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));
	twApi_DisableCertValidation();
	apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	messageHandlerThread = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(CONNECTION_TIMEOUT, CONNECTION_RETRIES));
	for (index = 0; index < NUMBER_OF_THINGS; index++) {
		sprintf(thingName, "SteamPerf-%i", index);
		bindThingWithNServices(thingName, 50);
		{
			twDataShapeEntry *inputParam1DsEntry = twDataShapeEntry_Create("SleepMilliSeconds", "The first parameter",
																		TW_NUMBER);
			twDataShape *inputParamsDataShape = twDataShape_Create(inputParam1DsEntry);
			twInfoTable *inputParams = twInfoTable_Create(inputParamsDataShape);
			twInfoTableRow * row = twInfoTableRow_Create(twPrimitive_CreateFromNumber(1));
			twInfoTable_AddRow(inputParams, row);
			{
				MARK_START("csdk.performance.perfServiceExecuteTimeVsCount.servicetime");
				METRIC("csdk.performance.perfServiceExecuteTimeVsCount.thingcount",index);
				METRIC("csdk.performance.perfServiceExecuteTimeVsCount.servicecount",index*50);
				TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, thingName, "Service50", inputParams, &result, -1, FALSE));
				MARK_END("csdk.performance.perfServiceExecuteTimeVsCount.servicetime");
                totalDuration+=duration;
			}
		}
	}
    averageDuration = totalDuration/NUMBER_OF_THINGS;

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Disconnect("End test_BindThingBeforeConnect"));
	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Delete());
	twThread_Delete(apiThread);
	twThread_Delete(messageHandlerThread);
	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 500 ms, was %f ms for %i things  ",averageDuration,NUMBER_OF_THINGS);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<500.0,errorMessageBuffer);

	return;
}



