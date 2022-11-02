/*
*	Created by Jeff Dreyer on 5/26/16.
*	Copyright 2016, PTC, Inc.
*/

#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10
#define NAME_SIZE 14
#define CSDK_848_NUM_WORKER_THREADS 20

#include "twBaseTypes.h"
#include "twThreads.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

TEST_GROUP(ApiIntegration);
void test_ApiIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(ApiIntegration){

	eatLogs();

	/* set reconnect interval and connect delay very small to avoid connect delays */
	twcfg_pointer->connect_retry_interval = 100;
	twcfg_pointer->max_connect_delay = 100;

	/* initialize */
	
	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;

    
}

TEST_TEAR_DOWN(ApiIntegration){
	/* do nothing */
}

TEST_GROUP_RUNNER(ApiIntegration) {
    RUN_TEST_CASE(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_good_path);
    RUN_TEST_CASE(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_bad_path);
    RUN_TEST_CASE(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_null_path);
	RUN_TEST_CASE(ApiIntegration, test_twApi_InvokeServiceAsync);
	RUN_TEST_CASE(ApiIntegration, test_twApi_PushSubscribedPropertiesAsync);
	RUN_TEST_CASE(ApiIntegration, CSDK_848);
	RUN_TEST_CASE(ApiIntegration, CSDK_888);
	RUN_TEST_CASE(ApiIntegration, test_twApi_CheckFileTransferSettings);

#ifdef ENABLE_IGNORED_TESTS
	RUN_TEST_CASE(ApiIntegration, test_twApi_EnableFipsMode);
#endif
}

TEST(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_good_path) {
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_FILE);
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE);
	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	if (OFFLINE_MSG_STORE == 0) {
		TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	} /* else this should be enabled during api init */

	if(OFFLINE_MSG_STORE == 2 || OFFLINE_MSG_STORE == 0) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_DIFFERENT));
	} else {
		/* if the offline message store is enabled but not persisting to disk, 
		it will throw a different error when attempting to set the offline message store */
		TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON, twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_DIFFERENT));
	}

	/* cleanup api */
	twApi_Delete();
}

TEST(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_bad_path) {
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_FILE);
	twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE);
	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	if (OFFLINE_MSG_STORE == 0) {
		TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	} /* else this should be enabled during api init */

	if(OFFLINE_MSG_STORE == 2 || OFFLINE_MSG_STORE == 0) {
		int ret = twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_BAD);
		TEST_ASSERT_TRUE(TW_INVALID_MSG_STORE_DIR == ret||TW_ERROR_WRITING_FILE == ret);
	} else {
		/* if the offline message store is enabled but not persisting to disk, 
		it will throw a different error when attempting to set the offline message store */
		TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON, twApi_SetOfflineMsgStoreDir(OFFLINE_MSG_STORE_LOCATION_BAD));
	}
	
	/* cleanup api */
	twApi_Delete();
}

TEST(ApiIntegration, test_twApi_SetOfflineMsgStoreDir_null_path) {
	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	if (OFFLINE_MSG_STORE == 0) {
		TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	} /* else this should be enabled during api init */

	
    

	if(OFFLINE_MSG_STORE == 2 || OFFLINE_MSG_STORE == 0) {
		TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twApi_SetOfflineMsgStoreDir(NULL));
	} else {
		/* if the offline message store is enabled but not persisting to disk, 
		it will throw a different error when attempting to set the offline message store */
		TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON, twApi_SetOfflineMsgStoreDir(NULL));
	}
	
	/* cleanup api */
	twApi_Delete();
}

char test_twApi_InvokeServiceAsyncCallbackPassFlag = FALSE;
char test_twApi_InvokeServiceAsyncCallbackDoneFlag = FALSE;
uint32_t test_twApi_InvokeServiceAsyncCallbackExpectedId = FALSE;
int test_twApi_InvokeServiceAsyncCallback(uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content){
	int result = TW_OK;
	if(test_twApi_InvokeServiceAsyncCallbackExpectedId!=id) {
		test_twApi_InvokeServiceAsyncCallbackDoneFlag = TRUE;
		result = TW_OK;
	} else {
		/* Expect two rows to be returned, description and name */
		if(content->rows->count!=2){
			test_twApi_InvokeServiceAsyncCallbackDoneFlag = TRUE;
			result = TW_OK;
		} else {
			test_twApi_InvokeServiceAsyncCallbackDoneFlag = TRUE;
			test_twApi_InvokeServiceAsyncCallbackPassFlag = TRUE;
		}
	}
	TW_FREE(content); /* Always delete this when you are done with it */
	return result;
}
/**
 * Test Plan: create a simple invoke request, wait 10 seconds for it to be called back.
 * Make sure the message ID matches the promided message id and that the returned into table is valid.
 */
TEST(ApiIntegration, test_twApi_InvokeServiceAsync) {
	twThread *apiThread = NULL;
	twThread *workerThreads[NUM_WORKER_THREADS];
	twInfoTable* params = NULL;
	int i = 0;
	int waitCount = 0;
	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES ));
	apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
	}

	/* Invoke this service Asynchronously */
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_unityTestThing.xml"));
	params = twInfoTable_CreateFromPrimitive("type", twPrimitive_CreateFromVariable(duplicateString("STRING"),TW_BASETYPENAME,NULL,0));
	test_twApi_InvokeServiceAsyncCallbackDoneFlag=TRUE;
	test_twApi_InvokeServiceAsyncCallbackPassFlag = TRUE;

	twApi_InvokeServiceAsync(TW_THING,"unityTestThing","GetPropertyDefinitions",params,TW_FORCE,test_twApi_InvokeServiceAsyncCallback,&test_twApi_InvokeServiceAsyncCallbackExpectedId);

	while(!test_twApi_InvokeServiceAsyncCallbackDoneFlag && waitCount<10){
		twSleepMsec(1000);
		waitCount++;
	}
	TEST_ASSERT_TRUE_MESSAGE(test_twApi_InvokeServiceAsyncCallbackPassFlag,"This test failed because the callback function test_twApi_InvokeServiceAsyncCallback() was never called.");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Test Complete"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreads[i]);
	}
	twThread_Delete(apiThread);
	twInfoTable_Delete(params);
	/* cleanup api */
	twApi_Delete();
}

enum msgCodeEnum test_twApi_PushSubscribedPropertiesCallback(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata){

}

twList* test_twApi_PushSubscribedPropertiesAsync_messageIdList;
char test_twApi_PushSubscribedPropertiesAsyncDoneFlag = FALSE;
char test_twApi_PushSubscribedPropertiesAsyncSyncFlag = FALSE;

int test_twApi_PushSubscribedPropertiesAsyncCallback(uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content){
	TW_FREE(content); /* Always delete this when you are done with it */
	test_twApi_PushSubscribedPropertiesAsyncDoneFlag = TRUE;
	TW_LOG(TW_FORCE,"test_twApi_PushSubscribedPropertiesAsyncCallback Called");
}

void test_twApi_PushSubscribedPropertiesAsyncSynchronizeStateCallback(char * entityName, twInfoTable* subscriptionInfo, void * userdata){
	test_twApi_PushSubscribedPropertiesAsyncSyncFlag = TRUE;
}

/**
 * Test Plan: Establish a remote thing. Attempt to update its property values using an async property push.
 * Wait for the returned handler to be called and then verify that the property values were received by the server.
 */
TEST(ApiIntegration, test_twApi_PushSubscribedPropertiesAsync) {
	int index,numberOfProperties = 88,i,waitCount=0;
	char thingPropertyNameBuffer[255];
	char stringValueBuffer[64],errorMessage[255];
	char* thingPropertyValue;
	twInfoTable* params, *result;
	twThread *apiThread;
	twThread *workerThreads[NUM_WORKER_THREADS];

	TEST_IGNORE_MESSAGE("Causes segfault in later tests on twApi_Delete()");
	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES ));

	apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
	}

	/* Import a thing with 88 string properties, named Performance_Test_Property_0-87 */
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING,"PropertyPerformanceTestThing"));
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_PropertyPerformanceTestThing_100_properties.xml"));

	/* Register and Bind to those properties */
	for(index=0;index<numberOfProperties;index++){
		snprintf(thingPropertyNameBuffer,255,"Performance_Test_Property_%i",index);
		TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"PropertyPerformanceTestThing",thingPropertyNameBuffer,TW_STRING,
							   TW_NO_DESCRIPTION,TW_PUSH_TYPE_ALWAYS,TW_PUSH_THRESHOLD_NONE,
														test_twApi_PushSubscribedPropertiesCallback,TW_NO_USER_DATA));
	}

	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterSynchronizeStateEventCallback("PropertyPerformanceTestThing",
							test_twApi_PushSubscribedPropertiesAsyncSynchronizeStateCallback,TW_NO_USER_DATA));

	/* Wait for synchronization */
	test_twApi_PushSubscribedPropertiesAsyncSyncFlag = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing("PropertyPerformanceTestThing"));
	/* Wait up to 15 seconds to see if the callback is called */
	while(!test_twApi_PushSubscribedPropertiesAsyncSyncFlag && waitCount<15){
		TW_LOG(TW_TRACE,"Waiting...");
		twSleepMsec(1000);
		waitCount++;
	}
	TEST_ASSERT_TRUE_MESSAGE(waitCount<15,"Timeout exceeded on synchronization.");

	/* Update property values */
	for(index=0;index<numberOfProperties;index++) {
		snprintf(thingPropertyNameBuffer,255,"Performance_Test_Property_%i",index);
		snprintf(stringValueBuffer,64,"Test_Property_Value_%i",index);
		twApi_SetSubscribedProperty("PropertyPerformanceTestThing",thingPropertyNameBuffer,TW_MAKE_STRING(stringValueBuffer),
									TW_FOLD_TYPE_NO,TW_PUSH_CONNECT_LATER);
	}

	/* Push them Asynchronously */
	test_twApi_PushSubscribedPropertiesAsyncDoneFlag = FALSE;
	twcfg_pointer->max_message_size = 2097152;
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedPropertiesAsync("PropertyPerformanceTestThing",TW_FORCE,
																 test_twApi_PushSubscribedPropertiesAsyncCallback,
																 &test_twApi_PushSubscribedPropertiesAsync_messageIdList));

	/* Wait up to 15 seconds to see if the callback is called */
	while(!test_twApi_PushSubscribedPropertiesAsyncDoneFlag && waitCount<15){
		twSleepMsec(1000);
		waitCount++;
	}
	TEST_ASSERT_TRUE_MESSAGE(waitCount<15,"Timeout exceeded on Asyc callback for property update.");

	/* Request the current property values from the server and verify them */
	params = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING,"PropertyPerformanceTestThing","GetPropertyValues",params,&result,5000,TW_PUSH_CONNECT_FORCE));
	TEST_ASSERT_NOT_NULL(result);
	for(index=0;index<numberOfProperties;index++) {
		snprintf(thingPropertyNameBuffer, 255, "Performance_Test_Property_%i", index);
		snprintf(stringValueBuffer,64,"Test_Property_Value_%i",index);
		twInfoTable_GetString(result, thingPropertyNameBuffer, 0, &thingPropertyValue);
		snprintf(errorMessage,255,"%s should match returned property value %s.",thingPropertyNameBuffer,thingPropertyValue);
		TEST_ASSERT_EQUAL_MESSAGE(0,strncmp(thingPropertyValue,stringValueBuffer,255),errorMessage);
		TW_FREE(thingPropertyValue);
	}

	/* Shutdown */
	twList_Delete(test_twApi_PushSubscribedPropertiesAsync_messageIdList);
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing("PropertyPerformanceTestThing"));
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Test Complete"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreads[i]);
	}

	twThread_Delete(apiThread);

	/* cleanup api */
	twApi_Delete();
}

/**
 * CSDK_848
 *
 * Summary: The message handler thread can access deleted memory if it is handling a message while either: another message was just handled, or, the tasker thread is cleaning up timed out messages.
 *
 * Resolution: Wrapped lock msg handler while iterating cb list in handleMessage.
 *
 * Test plan: Spin up a bunch of worker threads (20+) and spam some async calls to maximize chances of encountering this access violation.
 */
TEST(ApiIntegration, CSDK_848) {
    int index,numberOfProperties = 88,i,waitCount=0;
    char thingPropertyNameBuffer[255];
    char stringValueBuffer[64],errorMessage[255];
    char* thingPropertyValue;
    twInfoTable* params, *result;
    twThread *apiThread;
    twThread *workerThreads[CSDK_848_NUM_WORKER_THREADS];

    /* initialize */
    TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES ));

    apiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
    for (i = 0; i < CSDK_848_NUM_WORKER_THREADS; i++) {
        workerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
    }

    /* Import a thing with 88 string properties, named Performance_Test_Property_0-87 */
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING,"PropertyPerformanceTestThing"));
    TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_PropertyPerformanceTestThing_100_properties.xml"));

    /* Register and Bind to those properties */
    for(index=0;index<numberOfProperties;index++){
        snprintf(thingPropertyNameBuffer,255,"Performance_Test_Property_%i",index);
        TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"PropertyPerformanceTestThing",thingPropertyNameBuffer,TW_STRING,
                                                        TW_NO_DESCRIPTION,TW_PUSH_TYPE_ALWAYS,TW_PUSH_THRESHOLD_NONE,
                                                        test_twApi_PushSubscribedPropertiesCallback,TW_NO_USER_DATA));
    }

    TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterSynchronizeStateEventCallback("PropertyPerformanceTestThing",
                                                                         test_twApi_PushSubscribedPropertiesAsyncSynchronizeStateCallback,TW_NO_USER_DATA));

    /* Wait for synchronization */
    test_twApi_PushSubscribedPropertiesAsyncSyncFlag = FALSE;
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing("PropertyPerformanceTestThing"));
    /* Wait up to 10 seconds to see if the callback is called */
    while(!test_twApi_PushSubscribedPropertiesAsyncSyncFlag && waitCount<100){
        TW_LOG(TW_TRACE,"Waiting...");
        twSleepMsec(1000);
        waitCount++;
    }

	/* Update property values */
    for (i = 0; i < 100; i++) {
        for (index = 0; index < numberOfProperties; index++) {
            snprintf(thingPropertyNameBuffer, 255, "Performance_Test_Property_%i", index);
            snprintf(stringValueBuffer, 64, "Test_Property_Value_%i", i);
            twApi_SetSubscribedProperty("PropertyPerformanceTestThing", thingPropertyNameBuffer,
                                        TW_MAKE_STRING(stringValueBuffer),
                                        TW_FOLD_TYPE_NO, TW_PUSH_CONNECT_LATER);
        }
        TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedPropertiesAsync("PropertyPerformanceTestThing",TRUE,
                                                                     test_twApi_PushSubscribedPropertiesAsyncCallback,
                                                                     &test_twApi_PushSubscribedPropertiesAsync_messageIdList));
	    twList_Delete(test_twApi_PushSubscribedPropertiesAsync_messageIdList);
    }

    /* Give the test ~5s run */
    twSleepMsec(5000);

    /* Shutdown */
    TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing("PropertyPerformanceTestThing"));
    TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Test Complete"));
    TEST_ASSERT_FALSE(twApi_isConnected());

	for (i = 0; i < CSDK_848_NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreads[i]);
	}
	twThread_Delete(apiThread);

    /* cleanup api */
    twApi_Delete();
}

/**
CSDK_888

Summary: Request ID reuse was observed by customer causing messages to not get
    through. Logs showed when responses from platform did not get processed by
	edge device, Request IDs could get re-used.
Resolution: All request IDs are created through a common interface.
Test Plan: Push message onto offline message store. Send another message, which
    should flush the offline message store. Send a third message, which would
    have a duplicate request ID without the fix, but should have a unique
    request ID with the fix.
*/
#ifdef WIN32
extern __declspec(dllimport) twOfflineMsgStore * tw_offline_msg_store;
#else
extern twOfflineMsgStore * tw_offline_msg_store;
#endif

enum msgCodeEnum sendMessageBlocking(twMessage ** msg, int32_t timeout, twInfoTable ** result);
int twOfflineMsgStore_Write(struct twMessage * msg, struct twWs * ws);

uint32_t * requestIdsSent = NULL;
int requestIdsIdx = 0;
twResponseCallbackStruct static_callback;

int STUB_twWs_SendMessage(twWs * ws, char * buf, uint32_t length, char isText) {
	uint32_t requestId;

	if (!requestIdsSent) {
		TEST_FAIL_MESSAGE("requestIdsSent not initialized.");
		return TW_OK;
	}

	/* request ID is in buf[2..5]. It may be byte-swapped, but we don't care 
	   because the value will still be unique. */
	requestId = * (uint32_t *) &buf[2];
	requestIdsSent[requestIdsIdx++] = requestId;

	return TW_OK;
}

int stub_ApiIntegrationTests_twWs_Connect(twWs * ws, uint32_t timeout) {
	tw_api->connectionInProgress = FALSE;
	tw_api->mh->ws->isConnected = TRUE;
	return TW_OK;
}

int stub_twWs_Receive(twWs * ws, uint32_t timeout) {
	return 0;
}

twResponseCallbackStruct * stub_twMessageHandler_GetCompletedResponseStruct(
	twMessageHandler * handler,
	uint32_t id
) {
	return & static_callback;
}

void PopulateStaticCallback() {
	static_callback.code = TWX_SUCCESS;
}


TEST(ApiIntegration, CSDK_888) {
	/* initialize */
#define nmsg 3
#define timeout 250 /* ms */
	int i, j;
	twMessage * msg[nmsg];
	uint32_t requestIds[nmsg];
	twWs_SendMessageStub saved_SendMessage;
	twWs_ReceiveStub saved_Receive;
	twWs_ConnectStub saved_Connect;
	twMessageHandler_GetCompletedResponseStructStub saved_GetCompletedResponseStruct;

	requestIdsSent = requestIds;
	PopulateStaticCallback();

	TEST_ASSERT_EQUAL(
		TW_OK,
        twApi_Initialize(
			TW_HOST,
			TW_PORT,
			TW_URI,
			test_ApiIntegrationAppKey_callback,
			NULL,
			MESSAGE_CHUNK_SIZE,
			MESSAGE_CHUNK_SIZE,
			TRUE
		)
	);
	TEST_ASSERT_EQUAL(
		TW_OK,
		twOfflineMsgStore_Initialize(
			TRUE,
			OFFLINE_MSG_STORE_LOCATION,
			OFFLINE_MSG_STORE_SIZE,
			TRUE
		)
	);

	/* Pre-existing offline messages will really screw this test up. */
	if (twDirectory_FileExists(tw_offline_msg_store->offlineMsgFile)) {
		twDirectory_DeleteFile(tw_offline_msg_store->offlineMsgFile);
		twDirectory_CreateFile(tw_offline_msg_store->offlineMsgFile);
	}

	/* Note to self: stub assignment has to happen after API Initialization. */
	saved_SendMessage = twApi_stub->twWs_SendMessage;
	twApi_stub->twWs_SendMessage = STUB_twWs_SendMessage;

	saved_Connect = twApi_stub->twWs_Connect;
	twApi_stub->twWs_Connect = stub_ApiIntegrationTests_twWs_Connect;

	saved_Receive = twApi_stub->twWs_Receive;
	twApi_stub->twWs_Receive = stub_twWs_Receive;

	saved_GetCompletedResponseStruct = twApi_stub->twMessageHandler_GetCompletedResponseStruct;
	twApi_stub->twMessageHandler_GetCompletedResponseStruct = stub_twMessageHandler_GetCompletedResponseStruct;

	for (i = 0; i < nmsg; ++i) {
		msg[i] = twMessage_CreateAuthMsg("claim", "value");
		if (!msg[i]) {
			TEST_FAIL_MESSAGE("twMessage_CreateRequestMsg returned NULL.");
		}
	}

	tw_api->isAuthenticated = 1;

	/* populate offline message store. */
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Write(msg[0], tw_api->mh->ws));

	/* send new message. This will flush the offline message store. */
	TEST_ASSERT_EQUAL(TWX_SUCCESS, sendMessageBlocking(&msg[1], timeout, NULL));

	/* send new message. Without the fix, this message will have the same
	   request ID as the previous message. */
	TEST_ASSERT_EQUAL(TWX_SUCCESS, sendMessageBlocking(&msg[2], timeout, NULL));

	/* Verify twWs_SendMessage called expected number of times. */
	TEST_ASSERT_EQUAL(nmsg, requestIdsIdx);

	/* verify all messages sent with unique request IDs. */
	for (i = 0; i < nmsg; ++i) {
		for (j = i + 1; j < nmsg; ++j) {
			TEST_ASSERT_NOT_EQUAL(requestIds[i], requestIds[j]);
		}
	}

	/* verify msg[0] request ID was not used. In the real world scenario, the
	 * message response was never received, so a new request ID must be issued
	 * when the message in the offline store is sent. */
	for (i = 0; i < nmsg; ++i) {
		TEST_ASSERT_NOT_EQUAL(msg[0]->requestId, requestIds[i]);
	}

	/* Clean up. */
	twApi_stub->twWs_Receive = saved_Receive;
	twApi_stub->twMessageHandler_GetCompletedResponseStruct = saved_GetCompletedResponseStruct;
	twApi_stub->twWs_SendMessage = saved_SendMessage;
	twApi_stub->twWs_Connect = saved_Connect;
	for (i = 0; i < 3; ++i) {
		if (msg[i]) twMessage_Delete(msg[i]);
	}
	/* Reset count for subsequent runs in case tests were run with -r */
	requestIdsIdx = 0;
	twOfflineMsgStore_Delete();
	twApi_Delete();
}

/* Check our settings for FileTransfer, Max Message Size needs to be greater than File Transfer Block Size */
TEST(ApiIntegration, test_twApi_CheckFileTransferSettings) {

	twcfg_pointer->file_xfer_block_size = twcfg_pointer->message_chunk_size;
	twcfg_pointer->max_message_size = twcfg_pointer->message_chunk_size;
	TEST_ASSERT_EQUAL(twcfg_pointer->file_xfer_block_size, twcfg_pointer->max_message_size);

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ApiIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));

	/* Setting these two settings equal should log a warning then adjust the values on Initialize*/
	TEST_ASSERT_TRUE(twcfg_pointer->message_chunk_size < twcfg_pointer->max_message_size);

	twcfg_pointer->file_xfer_block_size = twcfg_pointer->message_chunk_size;
	twcfg_pointer->max_message_size = twcfg_pointer->message_chunk_size;
	TEST_ASSERT_EQUAL(twcfg_pointer->file_xfer_block_size, twcfg_pointer->max_message_size);

	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES));

	/* Since these can be modified after Initialize, also check & fix on Connect*/
	TEST_ASSERT_TRUE(twcfg_pointer->message_chunk_size < twcfg_pointer->max_message_size);

	/* cleanup api */
	twApi_Delete();
}