/**
 * Subscribed Property Integration Unique Tests
 *
 * This test group has no API setup so that tests can exercise specific test scenarios.
 */

#include "twTls.h"
#include "twExt.h"
#include "TestUtilities.h"
#include "unity.h"
#include "twThreads.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twMacros.h"

/**
 * API
 */
#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/**
 * Test Helper Functions
 */
void CSDK_1107_propertyPushTask(DATETIME now, void *params) {
	char* things = (char*)params;
	twApi_PushSubscribedProperties(things, TW_PUSH_CONNECT_FORCE);
}
void CSDK_1107_polledfunc(char* entityName) {
	int i = 0;
	double value = 0;
	for (i = 0; i < 250; i++) {
		char *propertyName = NULL;
		propertyName = TW_MALLOC(sizeof(char) * 100);
		strcpy(propertyName, "Property_");
		concatenateStrings(&propertyName, twItoa(i));
		value = rand() % 10;
		twApi_SetSubscribedProperty(entityName, propertyName, TW_MAKE_NUMBER(value), TW_FOLD_TYPE_NO, TW_PUSH_LATER);
		TW_FREE(propertyName);
	}
}
int CSDK_1112_cb(uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content) {
	return TW_OK;
}
int CSDK_1112_twWs_SendMessage(twWs * ws, char * buf, uint32_t length, char isText) {
	return TW_ERROR_WRITING_TO_WEBSOCKET;
}

/**
 * Unity Test Macros
 */
TEST_GROUP(SubscribedPropertyIntegrationUnique);

void test_SubscribedPropertyIntegrationUniqueAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(SubscribedPropertyIntegrationUnique) {
	eatLogs();
	twTest_DeleteAndSetPersistedBinFileLocations();
}
TEST_TEAR_DOWN(SubscribedPropertyIntegrationUnique) {
	twApi_Delete();
}
TEST_GROUP_RUNNER(SubscribedPropertyIntegrationUnique) {
	RUN_TEST_CASE(SubscribedPropertyIntegrationUnique, CSDK_1107);
	RUN_TEST_CASE(SubscribedPropertyIntegrationUnique, CSDK_1112);
	RUN_TEST_CASE(SubscribedPropertyIntegrationUnique, twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression);
	RUN_TEST_CASE(SubscribedPropertyIntegrationUnique, test_VALUE_with_0_threshold_int_No_WebSocket_Compression);
}

/**
 * Test Plan: Create 20 things with 250 properties each. Set the queuesize to 16 meg and establish
 * a polled functions across all 20 things that updates each property once ever 100ms. Allow this
 * data to build up for about 5 seconds and then establish another thread that pushes those property
 * values. Observe the current queuesize for integer overflow for about 20 seconds.
*/
TEST(SubscribedPropertyIntegrationUnique, CSDK_1107) {
	int thingIndex = 0;
	int propertyIndex = 0;
	uint16_t tickCount = 0;
	twThread *msgHandlerThread = NULL;
	twSubscribedPropsMgr *spm;

	srand(time(NULL));
	twcfg_pointer->max_message_size = 16384 * 8;
	twcfg_pointer->offline_msg_queue_size = 160000000;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SubscribedPropertyIntegrationUniqueAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	importEntityFileFromEtc("Entities_CSDK_1107.xml");

	twApi_Disconnect("SubscribedPropertyIntegration: CSDK_1107");
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* Reporter: Set up a number of things with a decent set of property updates. I created 20 SS Things, each with 100
	 * properties */
	/* Create 20 things with 250 properties each */
	for (thingIndex = 0; thingIndex < 20; thingIndex++) {
		char *thingName = NULL;
		thingName = TW_MALLOC(sizeof(char) * 100);
		strcpy(thingName, "CSDK_1107_Thing_");
		concatenateStrings(&thingName, twItoa(thingIndex));
		{
			TW_MAKE_THING(thingName, TW_THING_TEMPLATE_GENERIC);
			for (propertyIndex = 0; propertyIndex < 250; propertyIndex++) {
				char *propertyName = NULL;
				propertyName = TW_MALLOC(sizeof(char) * 100);
				strcpy(propertyName, "Property_");
				concatenateStrings(&propertyName, twItoa(propertyIndex));
				TW_PROPERTY(propertyName, "This is a description.", TW_NUMBER);
				TW_FREE(propertyName);
			}
			TW_BIND();
		}
		TW_FREE(thingName);
	}

	/* Reporter: Update the properties in a tight loop in {{onSteamSensorProcessScanRequest}} */
	/* Set up on all things using 100ms changes to all things property values */
	twExt_RegisterPolledTemplateFunction(CSDK_1107_polledfunc,TW_THING_TEMPLATE_GENERIC);
	twExt_Start(500, TW_THREADING_MULTI, 2);
	twSleepMsec(5000);

	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	/* Reporter:  Create a standalone thread to publish the property updates (use something like beginthreadex or
	 * CreateThread on Windows). This guarantees that {{spm->queueSize}} is accessed concurrently from different
	 * threads.*/
	/* Create a thread that performs rapid push requests for each thing */
	msgHandlerThread = twThread_Create(CSDK_1107_propertyPushTask, 500, TW_NO_USER_DATA,
									   TW_THREAD_AUTOSTART);


	/* Reporter: Using a conditional break-point in Visual Studio, I was able to break on
	 * {{spm->queueSize > 4000000000}} (four billion) consistently, generally within a few seconds. */
	/* Observe the size of the queue for around 20 seconds for an integer underflow */
	spm = twSubscribedPropsMgr_Get();
	while(spm->queueSize < twcfg_pointer->offline_msg_queue_size && tickCount < 2000){
		TW_LOG(TW_TRACE,"queueSize %u, tickcount %u", spm->queueSize, tickCount);
		twSleepMsec(10);
		tickCount++;
	}

	/* Did we run for 20 seconds or did we observe an overflow? */
	TEST_ASSERT_EQUAL(tickCount, 2000);

	/* Clean up */
	twThread_Delete(msgHandlerThread);
	twExt_Stop();
	twExt_RemovePolledFunction(CSDK_1107_polledfunc);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegration: CSDK_1107"));
}

/**
 * CSDK-1112
 *
 * 	Summary:	Kepware's store and forward mechanism retries a property set/push if the push fails but the current
 * 				values cache was preventing push by value properties from going up since the failed set/push already
 * 				cached that value.
 *
 * 	Test Plan:	Set/push up 10 property updates.  Drop the 5th push and replicate the store and forward mechanism's
 * 				behavior by forgetting current values then redoing the set/push.  Query the property's history to
 * 				confirm that all 10 updates were recieved.
 */
TEST(SubscribedPropertyIntegrationUnique, CSDK_1112) {
	int i = 0;
	char *thingName = "CSDK_1112";
	twList *messageList = NULL;
	twInfoTable *query = NULL;
	DATETIME *timestamps = NULL;
	eatLogs();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SubscribedPropertyIntegrationUniqueAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	twStubs_Use();
	/* Create test thing */
	{
		TW_MAKE_THING(thingName, TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, 0.9);
		TW_ADD_BOOLEAN_ASPECT("Pressure", TW_ASPECT_ISLOGGED, TRUE);
		TW_BIND();
	}
	/* Connect */
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	/* Start multithreaded model */
	twExt_Start(1000, TW_THREADING_MULTI, 5);
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Entities_CSDK_1112.xml"));
	/* Purge any existing property history on the thing */
	TEST_ASSERT_TRUE(purgeAllPropertyHistory(thingName));
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), 10);
	/* Set 10 properties in total */
	for (i = 1; i <= 10; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(thingName, "TestProperty", TW_MAKE_INT(i), timestamps[i-1], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_LATER));
		/* Stub out makeAsyncRequest to return TWX_GATEWAY_TIMEOUT on the 5th push */
		if (i == 5) {
			twApi_stub->twWs_SendMessage = CSDK_1112_twWs_SendMessage;
			TEST_ASSERT_EQUAL(TW_PRECONDITION_FAILED, twApi_PushSubscribedPropertiesAsync(thingName, TW_PUSH_CONNECT_LATER, CSDK_1112_cb, &messageList));
			twStubs_Reset();
			/* Replicate Kepware store and forward behavior by clearing current values and resetting property */
			TEST_ASSERT_EQUAL(TW_OK, twApi_ClearSubscribedPropertyCurrentValues());
			TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(thingName, "TestProperty", TW_MAKE_INT(i), timestamps[i-1], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_LATER));
		}
		TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedPropertiesAsync(thingName, TW_PUSH_CONNECT_LATER, CSDK_1112_cb, &messageList));
		TEST_ASSERT_EQUAL(TW_OK, twList_Delete(messageList));
		twSleepMsec(100);
	}
	twSleepMsec(10000);
	query = queryIntegerPropertyHistory(thingName, 100, "TestProperty", 0, twGetSystemTime(TRUE), FALSE, "");
	TEST_ASSERT_NOT_NULL(query);
	/* We should have gotten 10 property changes from our property history */
	TEST_ASSERT_EQUAL(10, query->rows->count);
	/* Shutdown and cleanup */
	TEST_ASSERT_EQUAL(TW_OK, twExt_Stop());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("AsyncPropertyWriteIntegration:CSDK_1112"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	TW_FREE(timestamps);
}

/*
* test if the subscribed property manager is correctly saving messages in RAM,
* then verify that the messages are flushed when the api reconnects
* Test without WebSocket Compression eanbled
*/
TEST(SubscribedPropertyIntegrationUnique, twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression) {
	int32_t i = 0;
	int res = 0;
	char *thingName = NULL;
	char *originalThingName = SUBSCRIBED_PROP_THINGNAME;
	twPrimitive * value = NULL;
	twThread * api_thread = NULL;
	twThread *workerThreads[NUM_WORKER_THREADS];

	/* initialize */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SubscribedPropertyIntegrationUniqueAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: TwApi Failed To Initialize");
	/* set cert params*/
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twApi_DisableWebSocketCompression ();
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: cannot connect to server");
	thingName = importAndCloneUniqueTestEntity("Things_One_Int_Prop.xml", SUBSCRIBED_PROP_THINGNAME);
	twApi_Disconnect("twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression");

	/* enable offline message store */
	enable_msg_store(TRUE);

	/* bind thing */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_BindThing(thingName), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: thing failed to bind");

	/* register property */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_RegisterProperty(TW_THING, thingName, "SomeInt", TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: property failed to register");

	/* start api task thread */
	api_thread = twThread_Create(twApi_TaskerFunction, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != api_thread, "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: api thread cannot be created");

	/* start message handling threads */
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, THREAD_RATE, NULL, TRUE);
	}

	/* update property SET_SUBSCRIBED_PROPERTY_ITERATIONS times with SET_SUBSCRIBED_PROPERTY_INTERVAL msec sleeps in between */
	for(i = 0; i < SUBSCRIBED_PROPERTY_ITERATIONS; i++){
		/* create primitive */
		value = twPrimitive_CreateFromInteger(i);
		TEST_ASSERT_TRUE_MESSAGE(NULL != value, "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: primitive failed to create");

		/* send value */
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_SetSubscribedProperty(thingName, "SomeInt", value, FALSE, FALSE), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: subscribed property was not properly saved");

		/* reset primitive */
		value = NULL;

		/* sleep */
		twSleepMsec(SUBSCRIBED_PROPERTY_INTERVAL);
	}

	/* verify spm->savedValues is filling up */
	TEST_ASSERT_EQUAL_MESSAGE(SUBSCRIBED_PROPERTY_ITERATIONS, getSPMSavedValuesCount(thingName), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: subscribed property list was not properly filled");

	/* connect to the server */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(INTEGRATION_TEST_CONNECT_TIMEOUT, INTEGRATION_TEST_CONNECT_RETRIES), "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: cannot connect to server");

	/* close when subscribed properties are sent */
	for(i = 0; i < SUBSCRIBED_PROPERTY_ITERATIONS && res !=-1; i++) {
		/* verify spm->savedValues is  empty */
		res = getSPMSavedValuesCount(thingName);
		/* sleep */
		twSleepMsec(SUBSCRIBED_PROPERTY_INTERVAL);
	}

	/* verify Subscribed Properties Manager has no saved values */
	TEST_ASSERT_EQUAL_MESSAGE(-1, res, "twApi_SetSubscribedProperty_Offline_Connect_No_WS_Compression: offline message stream not properly flushed");

	twApi_UnbindThing(thingName);
	deleteServerThing(TW_THING, thingName);
	TW_FREE(thingName);

	/* cleanup thread */
	twThread_Delete(api_thread);
	/* delete mh threads */
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreads[i]);
	}
	twApi_Delete();
}

/* Test "VALUE" push type with threshold value of zero for INTEGER types*/
/* Disable WebSocket compression */
TEST (SubscribedPropertyIntegrationUnique, test_VALUE_with_0_threshold_int_No_WebSocket_Compression) {
	const double pushThreshold = 0.0;
	const char* pushType = "VALUE";
	int i = 0;
	char *thingName = NULL;
	char *valueStreamName = NULL;
	const char *originalThingName = SUBSCRIBED_PROP_THINGNAME;
	const char *originalValueStreamName = "SPM_Integration_ValueStream";
	const char *propertyName = "MyInt";
	DATETIME startTime, endTime;
	char *offlineMsgStoreDir = NULL;
	twInfoTable *it = NULL;
	DATETIME timestamps[10];

	/* Get & remove offline msg store & subscribed property bin directory */
	offlineMsgStoreDir = getCurrentDirectory ();
	strcat (offlineMsgStoreDir, "/thingworx");
	twDirectory_DeleteDirectory (offlineMsgStoreDir);
	/* Initialize API */
	TEST_ASSERT_EQUAL (TW_OK, twApi_Initialize (TW_HOST, TW_PORT, TW_URI, test_SubscribedPropertyIntegrationUniqueAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                            MESSAGE_CHUNK_SIZE, TRUE));
	/* Configure  API */
	twApi_SetSelfSignedOk ();
	twApi_DisableCertValidation ();
	twApi_SetOfflineMsgStoreDir (offlineMsgStoreDir);
	twcfg_pointer->offline_msg_queue_size = (1024 * 1000);
	twApi_DisableWebSocketCompression ();
	/* Bind & Connect */
	TEST_ASSERT_EQUAL (TW_OK, twApi_Connect (CONNECT_TIMEOUT, CONNECT_RETRIES));
	thingName = importAndCloneUniqueTestEntity("Things_One_Int_Prop.xml", originalThingName);
	valueStreamName = importAndCloneUniqueTestEntity("Things_SPM_Integration_ValueStream.xml", originalValueStreamName);
	TEST_ASSERT_TRUE (assignValueStreamToServerThing (thingName, valueStreamName));
	TEST_ASSERT_EQUAL (TW_OK,
	                   twApi_RegisterProperty (TW_THING, thingName, propertyName, TW_INTEGER, "A test property", pushType,
	                                           pushThreshold, doNothing, NULL));
	TEST_ASSERT_EQUAL (TW_OK, twApi_BindThing (thingName));
	/* Check that we're now bound & ready to roll */
	TEST_ASSERT_TRUE (twApi_IsEntityBound (thingName));

	/* Write 10 integer subscribed property values, incrementing every other time */
	startTime = twGetSystemTime (TRUE);
	/* Log values 0 0 1 1 2 2 3 3 4 4 */
	for (i = 0; i < 5; i++) {
		/* Log once, keeping the timestamp since this update will pass the threshold check*/
		twPrimitive *value = twPrimitive_CreateFromInteger (i);
		twSleepMsec (1); /* Sleep 1ms so that we get more varied timestamps */
		timestamps[i] = twGetSystemTime (TRUE);
		TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (thingName, propertyName, value, timestamps[i], "GOOD", FALSE, FALSE));
		/* Log the same value again, but don't keep the timestamp (since this won't pass the threshold gate) */
		value = twPrimitive_CreateFromInteger (i);
		twSleepMsec (1); /* Sleep 1ms so that we get more varied timestamps */
		TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (thingName, propertyName, value, twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));
	}
	endTime = twGetSystemTime (TRUE);
	/* Push the subscribed property values up */
	twApi_PushSubscribedProperties (thingName, FALSE);
	/* Check the property history log to ensure only changed values were published */
	/* This loop compensates for the time it takes for the valuestream to be updated with the latest history.
	* Without it you get unpredictable failures in this test. */
	{
		int retries = 0, rowCount = 0;
		while (retries < 20 && rowCount < 5) {
			if (it) twInfoTable_Delete(it);
			it = queryIntegerPropertyHistory (thingName, 10, propertyName, startTime, endTime, TRUE, "");
			if (it) {
				rowCount = it->rows->count;
			}
			retries++;
			twSleepMsec (1000);
		}
		TEST_ASSERT_NOT_NULL_MESSAGE (it, "Failed to get a response to QueryIntegerPropertyHistory() from the server.")
		TEST_ASSERT_EQUAL (5, it->rows->count);
	}
	for (i = 0; i < 5; i++) {
		int value = 0;
		DATETIME timestamp;
		twInfoTable_GetInteger (it, "value", i, &value);
		twInfoTable_GetDatetime (it, "timestamp", i, &timestamp);
		TEST_ASSERT_EQUAL (i, value);
		TEST_ASSERT_EQUAL (timestamps[i], timestamp);
	}
	purgeAllPropertyHistory (thingName);

	/* Unbind, disconnect, delete API & clean up */
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(thingName));
	deleteServerThing(TW_THING, thingName);
	deleteServerThing(TW_THING, valueStreamName);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("EDGE_2131"));
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	if(offlineMsgStoreDir) TW_FREE(offlineMsgStoreDir);
	if (it) twInfoTable_Delete(it);
	TW_FREE(thingName);
	TW_FREE(valueStreamName);
}