/**
 * Generic Subscribed Property Integration Tests
 *
 * This test group has a generic setup/teardown macro to initalize the API, connect, and set up a remote thing for basic
 * subscribed property testing.
 */

#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "TestUtilities.h"
#include "twExt.h"
#include "twMacros.h"

/**
 * API
 */
#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi *tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/**
 * Test Specific Definitions
 */
#define SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME "SubscribedPropertyIntegrationTestThing"
#define SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME "TestInteger"
#define SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS 100
#define SUBSCRIBED_PROPERTY_INTEGRATION_TEST_QUERY_RETRIES 20

/**
 * Internal SDK Functions 
 */
int getSPMSavedValuesCount(const char * thingName);
uint32_t twSubscribedProperty_GetSize(twSubscribedProperty *p);
twSubscribedProperty *twSubscribedProperty_Create (char * e, char * n, twPrimitive * v, DATETIME t, char * q, char fold) ;
int getSPMPersistedValuesLength();
twSubscribedPropsMgr *getSPM();

/**
 * Test Helper Functions
 */
twInfoTable *queryIntegerPropertyHistoryWithRetries(char *thingName, char *propertyName, int count, DATETIME *timestamps) {
	twInfoTable *it = NULL;
	int i = 0;
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_QUERY_RETRIES; i++) {
		it = queryIntegerPropertyHistory(thingName, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, propertyName, timestamps[0], timestamps[count-1], TRUE, "");
		if (it && count == it->rows->count) break;
		twInfoTable_Delete(it);
		it = NULL;
		twSleepMsec(1000);
	}
	return it;
}
twInfoTable *queryFoldedIntegerPropertyHistoryWithRetries(char *thingName, char *propertyName, int count, DATETIME *timestamps) {
	twInfoTable *it = NULL;
	int i = 0;
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_QUERY_RETRIES; i++) {
		it = queryIntegerPropertyHistory(thingName, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, propertyName, timestamps[0], timestamps[count-1], TRUE, "");
		if (it && it->rows->count == 1) break;
		twInfoTable_Delete(it);
		it = NULL;
		twSleepMsec(1000);
	}
	return it;
}
void verifyPropertyHistory(twInfoTable *it, DATETIME *timestamps, int count) {
	int i = 0;
	TEST_ASSERT_NOT_NULL_MESSAGE(it, "SubscribedPropertyIntegrationTests: failed to query property history")
	TEST_ASSERT_EQUAL_MESSAGE(count, it->rows->count, "SubscribedPropertyIntegrationTests: invalid property history count");
	for (i = 0; i < count; i++) {
		int value = 0;
		DATETIME timestamp;
		twInfoTable_GetInteger(it, "value", i, &value);
		twInfoTable_GetDatetime(it, "timestamp", i, &timestamp);
		TEST_ASSERT_EQUAL_MESSAGE(i, value, "Property history value mismatch");
		TEST_ASSERT_EQUAL_MESSAGE(timestamps[i], timestamp, "Property history timestamp mismatch");
	}
}
void verifyFoldedPropertyHistory(twInfoTable *it, DATETIME *timestamps, int count) {
	int value = 0;
	DATETIME timestamp;
	TEST_ASSERT_NOT_NULL_MESSAGE(it, "SubscribedPropertyIntegrationTests: failed to query property history")
	TEST_ASSERT_EQUAL_MESSAGE(1, it->rows->count, "SubscribedPropertyIntegrationTests: invalid property history count");
	twInfoTable_GetInteger(it, "value", 0, &value);
	twInfoTable_GetDatetime(it, "timestamp", 0, &timestamp);
	TEST_ASSERT_EQUAL_MESSAGE(count-1, value, "Property history value mismatch");
	TEST_ASSERT_EQUAL_MESSAGE(timestamps[count-1], timestamp, "Property history timestamp mismatch");
}
int getSizeofTestSubscribedProperty(char *thingName, char *propertyName) {
	int queueSizeOfOnePropertyChange = 0;
	twPrimitive *value = twPrimitive_CreateFromInteger(1);
	twSubscribedProperty *calibrationPropertyChange = twSubscribedProperty_Create(thingName, propertyName, value, NULL, NULL, FALSE);
	queueSizeOfOnePropertyChange = twSubscribedProperty_GetSize(calibrationPropertyChange);
	twSubscribedProperty_Delete(calibrationPropertyChange);
	twPrimitive_Delete(value);
	return queueSizeOfOnePropertyChange;
}
int doNothingAsyncCb(uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content) {;}
property_cb doNothingCb(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {;}

/**
 * Unity Test Macros
 */
TEST_GROUP(SubscribedPropertyIntegrationGeneric);

void test_SubscribedPropertyIntegrationGenericAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(SubscribedPropertyIntegrationGeneric) {
	twPrimitive *result = NULL;
	eatLogs();
	twTest_DeleteAndSetPersistedBinFileLocations();
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_SubscribedPropertyIntegrationGenericAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, enable_msg_store(TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to enable offline msg store");
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	{
		TW_MAKE_THING(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME, TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, TW_PUSH_THRESHOLD_NONE);
		TW_ADD_BOOLEAN_ASPECT(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME, TW_ASPECT_ISLOGGED, TRUE);
		TW_ADD_NUMBER_ASPECT(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME, TW_ASPECT_DEFAULT_VALUE, -1);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES), "SubscribedPropertyIntegrationTests SETUP: failed to connect to server");
	twExt_Start(1000, TW_THREADING_MULTI, 5);
	TEST_ASSERT_TRUE_MESSAGE(importEntityFileFromEtc("Entities_SubscribedPropertyIntegrationTests.xml"), "SubscribedPropertyIntegrationTests SETUP: failed to import XML file");
	TEST_ASSERT_TRUE_MESSAGE(doesServerEntityExist("Thing", SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegrationTests SETUP: failed to verify existence of server entity");
	TEST_ASSERT_EQUAL(TW_OK, twExt_WaitUntilFirstSynchronization(5000));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                                        TW_MAKE_INT(-1), twGetSystemTime(TRUE), "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_NOW));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME, &result, 5000, FALSE));
	TEST_ASSERT_EQUAL_MESSAGE(-1, result->val.integer, "SubscribedPropertyIntegrationTests SETUP: initial property read/write failed");
	twPrimitive_Delete(result);
	TEST_ASSERT_TRUE_MESSAGE(twApi_isConnected(), "SubscribedPropertyIntegrationTests SETUP: failed to connect to server");
	TEST_ASSERT_TRUE_MESSAGE(purgeAllPropertyHistory(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegrationTests SETUP: failed to purge property history");
}
TEST_TEAR_DOWN(SubscribedPropertyIntegrationGeneric) {
	TEST_ASSERT_TRUE_MESSAGE(purgeAllPropertyHistory(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegrationTests TEAR_DOWN: failed to purge property history");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twExt_Stop(), "SubscribedPropertyIntegrationTests TEAR_DOWN: failed to stop ext threading model");
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegrationTests TEAR_DOWN: test is ending"));
	TEST_ASSERT_FALSE_MESSAGE(twApi_isConnected(), "SubscribedPropertyIntegrationTests TEAR_DOWN: failed to disconnect from server");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Delete(), "SubscribedPropertyIntegrationTests TEAR_DOWN: failed to delete API");
}
TEST_GROUP_RUNNER(SubscribedPropertyIntegrationGeneric) {
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetPropsPushNowWhileOnline);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetPropsPushLaterWhileOnlineThenPushWhileOnline);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOnlineThenPushWhileOnline);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetPropsPushNowWhileOfflineThenReconnect);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetPropsPushLaterWhileOfflineThenPushWhileOfflineThenReconnect);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOfflineThenPushWhileOfflineThenReconnect);
	RUN_TEST_CASE(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOfflineThenPushAsyncWhileOfflineThenReconnect);
}

/**
 * Basic Subscribed Property Write Tests
 *
 * Write 100 (configurable) always pushed subscribed properties online or offline with different pushing and folding
 * options.  Verify proper behaviour by checking SPM queue sizes and querying property history.
 *
 * Note: Property history queries rely on value streams which can be slow and sometimes inconsistent.
 */
TEST(SubscribedPropertyIntegrationGeneric, SetPropsPushNowWhileOnline) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_NOW));
	}

	/* Get property history */
	it = queryIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Verify property history */
	verifyPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetPropsPushLaterWhileOnlineThenPushWhileOnline) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_LATER));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME),
	                          "SubscribedPropertyIntegration: SPM queue count did not match property count");
	TEST_ASSERT_EQUAL_MESSAGE(
			getSizeofTestSubscribedProperty(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME) *
			SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, getSPM()->queueSize, "SubscribedPropertyIntegration: queue size did not match expected value");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values are not empty");

	/* Push our queued subscribed property updates */
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedProperties(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, TW_PUSH_CONNECT_LATER));

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Get property history */
	it = queryIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Verify property history */
	verifyPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOnlineThenPushWhileOnline) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_YES, TW_PUSH_LATER));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values are not empty");

	/* Push our queued subscribed property updates */
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedProperties(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, TW_PUSH_CONNECT_LATER));

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Get property history */
	it = queryFoldedIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Verify property history */
	verifyFoldedPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetPropsPushNowWhileOfflineThenReconnect) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegration: writeOfflinePushOnline disconnect"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED,
		                  twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                 SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                 TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_NOW));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_TRUE_MESSAGE(0 < getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values length is not increasing");

	/* Reconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	/* Get property history */
	it = queryIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Verify property history */
	verifyPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetPropsPushLaterWhileOfflineThenPushWhileOfflineThenReconnect) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegration: writeOfflinePushOnline disconnect"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_NO, TW_PUSH_LATER));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME),
	                          "SubscribedPropertyIntegration: SPM queue count did not match property count");
	TEST_ASSERT_EQUAL_MESSAGE(
			getSizeofTestSubscribedProperty(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME) *
			SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, getSPM()->queueSize, "SubscribedPropertyIntegration: queue size did not match expected value");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values are not empty");

	/* Push subscribed properties while still offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedProperties(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, FALSE));

	/* The push should have moved the updates from the queue to persisted values */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue did not properly flush");
	TEST_ASSERT_TRUE_MESSAGE(0 < getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values length is not increasing");

	/* Reconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	/* Get property history */
	it = queryIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                            SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Verify property history */
	verifyPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOfflineThenPushWhileOfflineThenReconnect) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;

	/* Disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegration: writeOfflinePushOnline disconnect"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_YES, TW_PUSH_LATER));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values are not empty");

	/* Push subscribed properties while still offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedProperties(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, FALSE));

	/* The push should have moved the updates from the queue to persisted values */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue did not properly flush");
	TEST_ASSERT_TRUE_MESSAGE(0 < getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values length is not increasing");

	/* Reconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	/* Get property history */
	it = queryFoldedIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                                  SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                                  SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue did not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Verify property history */
	verifyFoldedPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}
TEST(SubscribedPropertyIntegrationGeneric, SetFoldedPropsPushLaterWhileOfflineThenPushAsyncWhileOfflineThenReconnect) {
	twInfoTable *it = NULL;
	DATETIME *timestamps = NULL;
	int i = 0;
	twList *messageList = NULL;

	/* Disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("SubscribedPropertyIntegration: writeOfflinePushOnline disconnect"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* Get timestamps */
	timestamps = twTest_GetConsecutiveTimestamps(twGetSystemTime(TRUE), SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Set properties */
	for (i = 0; i < SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
		                                                        SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
		                                                        TW_MAKE_INT(i), timestamps[i], "GOOD", TW_FOLD_TYPE_YES, TW_PUSH_LATER));
	}

	/* Verify persisted value and spm queue sizes */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values are not empty");

	/* Push subscribed properties while still offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedPropertiesAsync(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME, FALSE, doNothingAsyncCb, &messageList));

	/* The push should have moved the updates from the queue to persisted values */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue did not properly flush");
	TEST_ASSERT_TRUE_MESSAGE(0 < getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values length is not increasing");

	/* Reconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());

	/* Get property history */
	it = queryFoldedIntegerPropertyHistoryWithRetries(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME,
	                                                  SUBSCRIBED_PROPERTY_INTEGRATION_TEST_PROPERTY_NAME,
	                                                  SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS, timestamps);

	/* Check that our queued and/or persisted values flushed */
	TEST_ASSERT_EQUAL_MESSAGE(-1, getSPMSavedValuesCount(SUBSCRIBED_PROPERTY_INTEGRATION_TEST_THING_NAME), "SubscribedPropertyIntegration: SPM queue was not empty");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPM()->queueSize, "SubscribedPropertyIntegration: SPM queue did not properly flush");
	TEST_ASSERT_EQUAL_MESSAGE(0, getSPMPersistedValuesLength(), "SubscribedPropertyIntegration: persisted values did not properly flush");

	/* Verify property history */
	verifyFoldedPropertyHistory(it, timestamps, SUBSCRIBED_PROPERTY_INTEGRATION_TEST_ITERATIONS);

	/* Clean up */
	TW_FREE(timestamps);
	twInfoTable_Delete(it);
}