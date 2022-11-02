/**
 * The Kepware tests represent hight bandwith throughput of properties on the order of unique 10000 integer properties
 * pushed up to a single thing in under 1 second. This test suite targets this goal, measures average performance
 * in the range of 1 to 10000 properties with the server both mocked out and present and makes sure these
 * access times conform to this use case.
 *
 * Expect this suite to run for approximatly 4 minutes when successful.
 *
 */
#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10
#define NUMBER_OF_PASSES 100
#define ERROR_MESSAGE_LENGTH 100
#define CONNECTION_RETRIES 1
#define CONNECTION_TIMEOUT 2000
#define NUMBER_OF_THINGS 50

#include "twBaseTypes.h"
#include "twThreads.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "twConstants.h"
#include "integrationTestDefs.h"

TEST_GROUP(KepwarePerformance);

void test_KepwarePerformanceAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(KepwarePerformance) {
	eatLogs();
}

TEST_TEAR_DOWN(KepwarePerformance) {
}

TEST_GROUP_RUNNER(KepwarePerformance) {
	RUN_TEST_CASE(KepwarePerformance, perfTimeToPush10000PropertiesNTimes);
	RUN_TEST_CASE(KepwarePerformance, perfPushTimeAsPropertiesIncrease);
//	RUN_TEST_CASE(KepwarePerformance, perfPushTimeAsPropertiesIncreaseWithServerRoundTrip);
}

/**
 * This property handler does not accept values written from the server
 * and rejects any request to fetch a property from the edge.
 * All property updates must be pushed through a subscription.
 * @param entityName
 * @param propertyName
 * @param itValue
 * @param isWrite
 * @param userdata
 * @return
 */
enum msgCodeEnum perfPushNPropertiesPropertyCallbackHandler(const char * entityName, const char * propertyName,  twInfoTable ** itValue, char isWrite, void * userdata){
	TW_LOG(TW_TRACE,"genericPropertyHandler() Function called for Entity %s, Property %s", entityName, propertyName);
	if (itValue) {
		if (isWrite && *itValue) {

			/* Set a new value */
			twPrimitive *currentPropertyValue = NULL;
			if(TW_ERROR_GETTING_PRIMITIVE == twInfoTable_GetPrimitive(*itValue, propertyName, 0, &currentPropertyValue)){
				return TWX_NOT_FOUND;
			}

			/* Ignore any property change request coming from the server quietly */
			return TWX_SUCCESS;

		} else {
			/* Refuse to return a property value if asked for from the server */
			return TWX_NOT_FOUND;
		}
	} else {
		return TWX_BAD_REQUEST;
	}
}

int randomPropertyValue(int max) {
	srand((unsigned)time(NULL));
	return (rand() % max) + 1;
}

/**
 * Creates a mock implementation of twApi_InvokeService() which measures how many properties are pushed up when
 * UpdateSubscribedPropertyValues() is called.
 * @param entityType
 * @param entityName
 * @param serviceName
 * @param params
 * @param result
 * @param timeout
 * @param forceConnect
 * @return
 */
int twApi_InvokeService_perfPushNPropertiesStub(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	twInfoTableRow * row = NULL;
	twPrimitive * propUpdateInfoTable = NULL;
	twInfoTable* updateTable = NULL;

	TEST_ASSERT_EQUAL(TW_THING,entityType);
	TEST_ASSERT_EQUAL_STRING("TagThing",entityName);
	TEST_ASSERT_EQUAL_STRING("UpdateSubscribedPropertyValues",serviceName);

	row = twInfoTable_GetEntry(params,0);
	propUpdateInfoTable = twInfoTableRow_GetEntry(row,0);
	updateTable = propUpdateInfoTable->val.infotable;
	TEST_ASSERT_EQUAL(10000,updateTable->rows->count);
	return TW_OK;
}

/**
 * A mock implementation of this api call which always succeeds
 * @param entityType
 * @param entityName
 * @param serviceName
 * @param params
 * @param result
 * @param timeout
 * @param forceConnect
 * @return
 */
int twApi_InvokeService_perfPushTimeAsPropertiesIncrease(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	TEST_ASSERT_EQUAL(TW_THING,entityType);
	TEST_ASSERT_EQUAL_STRING("TagThing",entityName);
	TEST_ASSERT_EQUAL_STRING("UpdateSubscribedPropertyValues",serviceName);
	return TW_OK;
}

/**
 * A mock implementation of this api call which makes the api think it is always connected to the server when it is not.
 * @return
 */
char twApi_isConnected_perfPushNPropertiesStub(){
	// Mock that I am always connected
	return TRUE;
}

/**
* Test Plan: Create a remote thing with 10000 integer properties. Simulate connecting to the server and sending updates
* to all 10000 properties repeatedly. Repeatedly measure throughput from entry into the API from twApi_SetSubscribedProperty()
* to its exit at InvokeService() UpdateSubscribedPropertyValues(). Determine average time required to enter and exit
* the API.
*
* Pre-conditions: None. All server calls are mocked using API stubs
*
* Driving Metric: None, This test is one of measuring system throughput
*
* Expected Outcome: The entire process from property update to api exit should occur in under ? ms
*
* Expected Execution Time: For 100 passes should be 0.5 seconds
*/
TEST(KepwarePerformance, perfTimeToPush10000PropertiesNTimes) {
	static const char thingName[] = "TagThing";
	char propertyName[50],errorMessageBuffer[100];
	int initialCallbackCount = 0;
	int numberOfProperties = 10000,index,pass,totalDuration=0;
	double averageDuration;

	twStubs_Use();
	twcfg_pointer->offline_msg_queue_size = 1000000;
	twcfg_pointer->max_message_size = 10000000;

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_KepwarePerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));
	// Use mock version of InvokeService
	twApi_stub->twApi_InvokeService = twApi_InvokeService_perfPushNPropertiesStub;
	twApi_stub->twApi_isConnected = twApi_isConnected_perfPushNPropertiesStub;

	// Create 1 Thing with 10000 properties and verify that 10000 new callbacks were added
	initialCallbackCount = twDict_GetCount(tw_api->callbackList);
	for(index=0;index<numberOfProperties;index++){
		sprintf(propertyName,"tag%i",index);
		twApi_RegisterProperty(TW_THING, thingName,propertyName, TW_INTEGER,
			TW_NO_DESCRIPTION, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE, perfPushNPropertiesPropertyCallbackHandler, TW_NO_USER_DATA);
	}
	TEST_ASSERT_EQUAL(10000,twDict_GetCount(tw_api->callbackList)-initialCallbackCount);


	/*
	 * Simulate receiving response from getPropertySubscriptions() service - normally this would be a blocking
	 * call to the server to get the subscription information after a bind call which we are skipping.
	 * This is required for an update to be pushed to the server
	 */
	for(index=0;index<numberOfProperties;index++) {
		sprintf(propertyName, "tag%i", index);
		twApi_UpdatePropertyMetaData(TW_THING, (char*)thingName, propertyName, TW_INTEGER, TW_NO_DESCRIPTION, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE);
	}

	for(pass=0;pass<NUMBER_OF_PASSES;pass++) {

		/* Change all thing properties to a new random integer value */
		for (index = 0; index < numberOfProperties; index++) {
			char errorMessage[150];
			twPrimitive *propertyValue = twPrimitive_CreateFromInteger(randomPropertyValue(100));
			sprintf(propertyName, "tag%i", index);
			sprintf(errorMessage,"Property update buffer has overflowed after adding %i updates. Increase value of twcfg.offline_msg_queue_size.",index);
			TEST_ASSERT_EQUAL_MESSAGE(TW_OK,twApi_SetSubscribedProperty((char*)thingName, propertyName, propertyValue, TW_FOLD_TYPE_NO, TW_PUSH_LATER),errorMessage);
		}

		{
			/* Measure Results */
			MARK_START("csdk.performance.perfPushNProperties.pushtime");
			METRIC("csdk.performance.perfPushNProperties.pass",pass);
			twApi_PushSubscribedProperties((char*)thingName, TW_PUSH_CONNECT_FORCE);
			MARK_END("csdk.performance.perfPushNProperties.pushtime");
			totalDuration+=duration;
		}
	}

	averageDuration = totalDuration / NUMBER_OF_PASSES;

	twApi_UnregisterThing((char*)thingName);
	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 50 ms, was %f ms for %i passes.",averageDuration,NUMBER_OF_PASSES);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<50.0,errorMessageBuffer);
	GRAPHXY("perfTimeToPush10000PropertiesNTimes","csdk.performance.perfPushNProperties.pass","pass","csdk.performance.perfPushNProperties.pushtime","ms");

}

/**
* Test Plan: Create a remote thing with 1-10000 integer properties. Simulate connecting to the server and sending updates
* to all properties. Measure throughput from entry into the API from twApi_SetSubscribedProperty()
* to its exit at InvokeService() UpdateSubscribedPropertyValues() as the number of properties increases.
*
* Pre-conditions: None. All server calls are mocked using API stubs
*
* Driving Metric: The number of properties a thing mock pushes to the server
*
* Expected Outcome: Near Linear performance as number of properties increases with 10,000 properties finishing around 50 ms
* Average time should be below 25 ms
*
* Expected Execution Time:  Approximately 20 Seconds
*/
TEST(KepwarePerformance, perfPushTimeAsPropertiesIncrease) {
	static const char thingName[] = "TagThing";
	char propertyName[50],errorMessageBuffer[100];
	int maxNumberOfProperties = 10000,numberOfProperties,index,totalDuration=0;
	double averageDuration;

	twStubs_Use();
	twcfg_pointer->offline_msg_queue_size = 1000000;
	twcfg_pointer->max_message_size = 10000000;

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_KepwarePerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));

	/* Use mock version of InvokeService */
	twApi_stub->twApi_InvokeService = twApi_InvokeService_perfPushTimeAsPropertiesIncrease;
	twApi_stub->twApi_isConnected = twApi_isConnected_perfPushNPropertiesStub;

	/* Register all property callbacks */
	for(numberOfProperties=0;numberOfProperties<maxNumberOfProperties;numberOfProperties++) {
		sprintf(propertyName, "tag%i", numberOfProperties);
		twApi_RegisterProperty(TW_THING, thingName, propertyName, TW_INTEGER,
							   TW_NO_DESCRIPTION, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE,
							   perfPushNPropertiesPropertyCallbackHandler, TW_NO_USER_DATA);
	}

	/*
	 * Simulate receiving response from getPropertySubscriptions() service - normally this would be a blocking
	 * call to the server to get the subscription information after a bind call which we are skipping.
	 * This is required for an update to be pushed to the server
	 */
	for(index=0;index<maxNumberOfProperties;index++) {
		sprintf(propertyName, "tag%i", index);
		twApi_UpdatePropertyMetaData(TW_THING, (char*)thingName, propertyName, TW_INTEGER, TW_NO_DESCRIPTION, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE);
	}

	/* Create 1 Thing with 1 to 10000 properties */
	for(numberOfProperties=0;numberOfProperties<maxNumberOfProperties;numberOfProperties+=50){
		sprintf(propertyName,"tag%i",numberOfProperties);

		/* Change all existing thing properties to a new random integer value */
		for (index = 0; index < numberOfProperties+1; index++) {
			char errorMessage[150];
			twPrimitive *propertyValue = twPrimitive_CreateFromInteger(randomPropertyValue(100));
			sprintf(propertyName, "tag%i", index);
			sprintf(errorMessage,"Property update buffer has overflowed after adding %i updates. Increase value of twcfg.offline_msg_queue_size.",index);
			TEST_ASSERT_EQUAL_MESSAGE(TW_OK,twApi_SetSubscribedProperty((char*)thingName, propertyName, propertyValue, TW_FOLD_TYPE_NO, TW_PUSH_LATER),errorMessage);
		}
		
		{
			MARK_START("csdk.performance.perfPushTimeAsPropertiesIncrease.pushtime");
			METRIC("csdk.performance.perfPushTimeAsPropertiesIncrease.propertycount",numberOfProperties);
			twApi_PushSubscribedProperties((char*)thingName, TW_PUSH_CONNECT_FORCE);
			MARK_END("csdk.performance.perfPushTimeAsPropertiesIncrease.pushtime");
			totalDuration+=duration;
		}

	}

	averageDuration = totalDuration / (maxNumberOfProperties/50);

	twApi_UnregisterThing((char*)thingName);
	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 25 ms, was %f ms for %i passes.",averageDuration,maxNumberOfProperties/50);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<25.0,errorMessageBuffer);

}

/**
* Test Plan: Create a remote thing with 10000 integer properties. Upload and Bind a matching Thing called TagThing on the server.
* Measure roundtrip time from entry into the API at twApi_PushSubscribedProperties() until the server returns from the call to
* InvokeService() UpdateSubscribedPropertyValues(). Also determine average time required for a round trip.
*
* Pre-conditions: An uploaded entity called TagThing with 10000 integer properties called tag0 to tag 10000.
*
* Driving Metric: The number of properties sent in a single push
*
* Expected Outcome: Round trip time will increase linearly as properties increase. This test my fail with a slow server
* or poor network connection.
*
* Expected Execution Time: For 200 passes should be around three minutes.
*
*/
TEST(KepwarePerformance, perfPushTimeAsPropertiesIncreaseWithServerRoundTrip) {
	static const char thingName[] = "TagThing";
	char propertyName[50],errorMessageBuffer[100];
	int maxNumberOfProperties = 10000,numberOfProperties,index,totalDuration=0;
	double averageDuration;
	twPrimitive* defaultValue = twPrimitive_CreateFromInteger(0);

	twThread *apiThread;
	twThread *messageHandlerThread;
	apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	messageHandlerThread = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);

	twcfg_pointer->offline_msg_queue_size = 1000000;
	twcfg_pointer->max_message_size = 10000000;

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_KepwarePerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
												  MESSAGE_CHUNK_SIZE, TRUE));


	/* Disable cert validation and https if port is 8080 */
	twApi_DisableCertValidation();
	if(TW_PORT == 8080)
		twApi_DisableEncryption();/* for http only */

	/* Connect to server and bind thing */
	TEST_ASSERT_EQUAL(TW_OK,twApi_Connect(10000,3));

	/* Upload entity file to server */
	deleteServerThing(TW_THING,thingName);
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Kepware10000PropertyEntity.xml"));

	/* Register all properties */
	for(numberOfProperties=0;numberOfProperties<maxNumberOfProperties;numberOfProperties++) {
		sprintf(propertyName, "tag%i", numberOfProperties);
		twApi_RegisterProperty(TW_THING, thingName, propertyName, TW_INTEGER,
							   TW_NO_DESCRIPTION, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE,
							   perfPushNPropertiesPropertyCallbackHandler, TW_NO_USER_DATA);
	}

	TEST_ASSERT_EQUAL(TW_OK,twApi_BindThing((char*)thingName));
	twSleepMsec(10000);

	/* Create 1 Thing with 1 to 10000 properties */
	for(numberOfProperties=0;numberOfProperties<maxNumberOfProperties;numberOfProperties+=50){
		sprintf(propertyName,"tag%i",numberOfProperties);

		/* Change all existing thing properties to a new random integer value */
		for (index = 0; index < numberOfProperties+1; index++) {
			char errorMessage[150];
			twPrimitive *propertyValue = twPrimitive_CreateFromInteger(randomPropertyValue(100));
			sprintf(propertyName, "tag%i", index);
			sprintf(errorMessage,"Property update buffer has overflowed after adding %i updates. Increase value of twcfg.offline_msg_queue_size.",index);
			TEST_ASSERT_EQUAL_MESSAGE(TW_OK,twApi_SetSubscribedProperty((char*)thingName, propertyName, propertyValue, TW_FOLD_TYPE_NO, TW_PUSH_LATER),errorMessage);
		}
		
		{
			MARK_START("csdk.performance.perfPushTimeAsPropertiesIncreaseWithServerRoundTrip.pushtime");
			METRIC("csdk.performance.perfPushTimeAsPropertiesIncreaseWithServerRoundTrip.propertycount",numberOfProperties);
			twApi_PushSubscribedProperties((char*)thingName, TW_PUSH_CONNECT_FORCE);
			MARK_END("csdk.performance.perfPushTimeAsPropertiesIncreaseWithServerRoundTrip.pushtime");
			totalDuration+=duration;
		}

	}

	averageDuration = totalDuration / (maxNumberOfProperties/50);

	/* Disconnect from server */
	twApi_Disconnect("test ends");

	twPrimitive_Delete(defaultValue);
	twApi_UnregisterThing((char*)thingName);
	twThread_Delete(apiThread);
	twThread_Delete(messageHandlerThread);
	twApi_Delete();

	snprintf(errorMessageBuffer,ERROR_MESSAGE_LENGTH," Expected less than 660 ms, was %f ms for %i passes.",averageDuration,maxNumberOfProperties/50);
	TEST_ASSERT_TRUE_MESSAGE(averageDuration<660.0,errorMessageBuffer);

}

