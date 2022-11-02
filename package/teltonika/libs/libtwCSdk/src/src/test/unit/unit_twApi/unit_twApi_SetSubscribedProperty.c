/*
*  Copyright 2018, PTC, Inc.
*  All rights reserved.
*
*  Unit tests for twApi_SetSubscribedProperty()
*/

#include <twTls.h>
#include <twBaseTypes.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twExt.h"

/**
 * Internal SDK Functions
 */
twSubscribedPropsMgr* getSPM();
int getSPMPersistedValuesLength();
char twSubscribedProperty_Queued_Hit;
int getSPMSavedValuesCount(const char * thingName);

/**
 * Test Helper Functions
 */
static int twSubscribedPropsMgr_QueueValueForSending_Confirm (struct twSubscribedProperty * pProp, twDict * pList, char * src)	{
	twSubscribedProperty_Queued_Hit = TRUE;
	twSubscribedProperty_Delete (pProp);
	return TW_OK;
}

static twList *getSPMSavedValueList(char *thingName) {
	int values[10] = {-1};
	int i = 0;
	twEntitySavedProperties *match;
	twEntitySavedProperties query;
	query.name = thingName;
	if (TW_OK == twDict_Find(getSPM()->savedValues, &query, (void*)&match) ){
		twList *props = match->props;
		for (i = 0; i < props->count; i++) {
			ListEntry *le = twList_GetByIndex(props, i);
			twSubscribedProperty *p = le->value;
			values[i] = p->prop->value->val.integer;
		}
		return match->props;
	}
	return NULL;
}

static int CSDK_748_PropertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	return TWX_SUCCESS;
}

/**
 * Unity Test Macros
 */
TEST_GROUP(unit_twApi_SetSubscribedProperty);

TEST_SETUP(unit_twApi_SetSubscribedProperty) {
	eatLogs();
	twTest_DeleteAndSetPersistedBinFileLocations();
}

TEST_TEAR_DOWN(unit_twApi_SetSubscribedProperty) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}

TEST_GROUP_RUNNER(unit_twApi_SetSubscribedProperty) {
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, setSubscribedPropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushValueNoThreshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushValue99Threshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushValue100Threshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushDeadbandNoThreshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushDeadband99Threshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, PushDeadband100Threshold);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndPush);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndUnregister);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndUnbind);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, CSDK_746);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, CSDK_748);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, CSDK_747);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, CSDK_750);
	RUN_TEST_CASE(unit_twApi_SetSubscribedProperty, CSDK_883);
}

TEST(unit_twApi_SetSubscribedProperty, setSubscribedPropertyWithNullApi) {
	TEST_IGNORE_MESSAGE("CSDK-1370: SDK crashes when TW_SET_PROPERTY is called with NULL api");
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(1)));
}

/**
 * Push Type Tests
 */

/**
 * Test Plan: Create a property with a VALUE push threshold of 0, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 0 from the previous value  were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushValueNoThreshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 0.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - values[i-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Test Plan: Create a property with a VALUE push threshold of 99, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 99 from the previous value  were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushValue99Threshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 99.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - values[i-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Test Plan: Create a property with a VALUE push threshold of 100, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 100 from the previous value were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushValue100Threshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 100.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - values[i-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Test Plan: Create a property with a VALUE push threshold of 100, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 0 from the previous saved value were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushDeadbandNoThreshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 0.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, "DEADBAND", threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - savedValues[savedValueCount-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Test Plan: Create a property with a VALUE push threshold of 99, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 99 from the previous saved value were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushDeadband99Threshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 99.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, "DEADBAND", threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - savedValues[savedValueCount-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Test Plan: Create a property with a VALUE push threshold of 100, set a bunch of values for the property, and verify
 * that all values that exceed a difference of 100 from the previous saved value were queued to be pushed to the server.
 */
TEST(unit_twApi_SetSubscribedProperty, PushDeadband100Threshold) {
	int values[10] = {100, 100, 200, 300, 500, 500, 500, 400, 100, 100};
	int savedValueCount = 0;
	int savedValues[10] = {-1};
	twList *spmSavedValues = NULL;
	double threshold = 100.00;
	int i = 0;
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, "DEADBAND", threshold);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 0; i < 10; i++) {
		if (i == 0 || abs(values[i] - savedValues[savedValueCount-1]) > threshold) {
			savedValues[savedValueCount] = values[i];
			savedValueCount++;
		}
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(values[i]));
	}
	spmSavedValues = getSPMSavedValueList("TestThing");
	TEST_ASSERT_EQUAL(savedValueCount, spmSavedValues->count);
	for (i = 0; i < savedValueCount; i++) {
		ListEntry *le = twList_GetByIndex(spmSavedValues, i);
		int savedValue = ((twSubscribedProperty*)le->value)->prop->value->val.integer;
		TEST_ASSERT_EQUAL(savedValues[i], savedValue);
	}
}

/**
 * Queue Size Tests
 */

/**
 * Test Plan: Create a thing with a single subscribed property and verify that the spm queue size grows as expected
 * while we set new values for said property.  Then, push all property values and verify that the queue size has reset.
 */
TEST(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndPush) {
	int i = 0;
	int queueSizeOfOneProperty = twTest_GetQueueSizeOfOnePropertyChange("TestThing", "TestProperty");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, TW_PUSH_THRESHOLD_NONE);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 1; i <= 10; i++) {
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(i));
		TEST_ASSERT_EQUAL(queueSizeOfOneProperty * i, getSPM()->queueSize);
	}
	TEST_ASSERT_EQUAL(TW_OK, twApi_PushSubscribedProperties("TestThing", FALSE));
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	TEST_ASSERT_TRUE(0 < getSPMPersistedValuesLength());
}

/**
 * Test Plan: Create a thing with a single subscribed property and verify that the spm queue size grows as expected
 * while we set new values for said property.  Then, unregister the property and verify that the queue size has reset.
 */
TEST(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndUnregister) {
	int i = 0;
	int queueSizeOfOneProperty = twTest_GetQueueSizeOfOnePropertyChange("TestThing", "TestProperty");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, TW_PUSH_THRESHOLD_NONE);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 1; i <= 10; i++) {
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(i));
		TEST_ASSERT_EQUAL(queueSizeOfOneProperty * i, getSPM()->queueSize);
	}
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnregisterPropertyCallback("TestThing", "TestProperty", NULL));
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	TEST_ASSERT_EQUAL(0, getSPMPersistedValuesLength());
}

/**
 * Test Plan: Create a thing with a single subscribed property and verify that the spm queue size grows as expected
 * while we set new values for said property.  Then, unbind the thing and verify that the queue size has reset.
 */
TEST(unit_twApi_SetSubscribedProperty, SetPropertyCheckQueueSizeAndUnbind) {
	int i = 0;
	int queueSizeOfOneProperty = twTest_GetQueueSizeOfOnePropertyChange("TestThing", "TestProperty");
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	{
		TW_MAKE_THING("TestThing", TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY_LONG("TestProperty", TW_NO_DESCRIPTION, TW_INTEGER, TW_PUSH_TYPE_VALUE, TW_PUSH_THRESHOLD_NONE);
		TW_BIND();
	}
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	for (i = 1; i <= 10; i++) {
		TW_SET_PROPERTY("TestThing", "TestProperty", TW_MAKE_INT(i));
		TEST_ASSERT_EQUAL(queueSizeOfOneProperty * i, getSPM()->queueSize);
	}
	TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_UnbindThing("TestThing"));
	TEST_ASSERT_EQUAL(0, getSPM()->queueSize);
	TEST_ASSERT_EQUAL(0, getSPMPersistedValuesLength());
}

/**
* CSDK-883
*
* Summary: The SPM should send changes when the property push type is VALUE and only the quality changes
*
* Fix: Add a quality comparison to the VALUE case of twSubscribedPropsMgr_SetPropertyVTQ
*
* Test Plan:
* 1. Initialize the API.
* 2. Register a property.
* 3. Invoke twApi_SetSubscribedPropertyVTQ with good quality.
* 4. Install a stub for twSubscribedPropsMgr_QueueValueForSending.
* 5. Invoke twApi_SetSubscribedPropertyVTQ with the same value but bad quality.
* 6. Verify that the queue for sending stub is hit.
*/
TEST (unit_twApi_SetSubscribedProperty, CSDK_883) {
	char *thingName = "CSDK_883";
	char *propertyName = "MyInt";
	int i, res, timestamp;
	twPrimitive *value = 0;
	const char* goodQuality = "GOOD";
	const char* badQuality = "BAD";

	TEST_ASSERT_EQUAL (TW_OK, twApi_Initialize (TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                            MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL (TW_OK,
	                   twApi_RegisterProperty (TW_THING, thingName, propertyName, TW_INTEGER, "A test property", "VALUE",
	                                           0.0, doNothing, NULL));
	twOfflineMsgStore_Disable ();
	twcfg_pointer->offline_msg_queue_size = (1024 * 1024 * 1); /* 1mb queue size */

	value = twPrimitive_CreateFromInteger (1);
	timestamp = twGetSystemTime (TRUE);
	TEST_ASSERT_EQUAL (TW_OK,
	                   twApi_SetSubscribedPropertyVTQ (thingName, propertyName, value, timestamp, goodQuality, FALSE, FALSE));

	twStubs_Use ();
	twApi_stub->twSubscribedPropsMgr_QueueValueForSending = twSubscribedPropsMgr_QueueValueForSending_Confirm;

	value = twPrimitive_CreateFromInteger (1);
	timestamp = twGetSystemTime (TRUE);
	TEST_ASSERT_EQUAL (TW_OK,
	                   twApi_SetSubscribedPropertyVTQ (thingName, propertyName, value, timestamp, badQuality, FALSE, FALSE));

	TEST_ASSERT_TRUE (twSubscribedProperty_Queued_Hit);
}

/**
 * CSDK-747
 *
 * Summary: When all persistence is disabled, the SPM uses more memory than expected, based on configured maximum buffer
 * sizes.
 *
 * Cause: twSubscribedProperty_GetLength() was not properly calculating data allocated by
 * twApi_SetSubscribedPropertyVTQ().
 *
 * Fix: Corrected twSubscribedProperty_GetLength()'s calculations.
 *
 * Test Plan: This test is designed to be run alone using -n through a memory profiling tool like valgrind's massif. The
 * tester will have to manually analyze the memory profiling tool's report in order to verify the SDK is properly
 * managing memory.
 */
TEST(unit_twApi_SetSubscribedProperty, CSDK_747) {
	char *thingName = "CSDK_747";
	char *propertyName = "MyInt";
	int i, res;
	DATETIME timestamp;
	char bufferFull = FALSE;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,
	                  twApi_RegisterProperty(TW_THING, thingName, propertyName, TW_INTEGER, "A test property", "ALWAYS",
	                                         0.0, doNothing, NULL));
	twOfflineMsgStore_Disable();
	twcfg_pointer->offline_msg_queue_size = (1024 * 1024 * 1); /* 1mb queue size */
	for (i = 0; !bufferFull; i++) {
		twPrimitive *value = twPrimitive_CreateFromInteger(i);
		timestamp = twGetSystemTime(TRUE);
		res = twApi_SetSubscribedPropertyVTQ(thingName, propertyName, value, timestamp, "GOOD", FALSE, FALSE);
		if (res == TW_PROPERTY_CHANGE_BUFFER_FULL) bufferFull = TRUE;
	}
}

/**
 * CSDK-746
 *
 * Summary: An improperly defined return code causes the SPM to report pushes that fail to either be published or
 * persisted as "persisted", resetting the enqueued count and allowing the SPM's VQT buffer to grow indefinitely.
 *
 * Cause: The function twSubscribedPropsMgr_PushPropertyList eventually calls twSubscribedPropsMgr_PushPropertyList.
 * Here, no persistence layer is configured, so persistError ends up true. This function returns
 * TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR which is defined the same as TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED - the
 * calling function therefore thinks the Push was persisted, and resets the enqueued VTQ bytecount.
 *
 * Fix: Redefined TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR to a unique error code (1024).
 *
 * Test Plan: Disable offline msg store and attempt to push up subscribed properties. Now that
 * TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR is no longer defined to the same value as
 * TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED, the function will properly return TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR
 * instead of TW_OK.
 */
TEST(unit_twApi_SetSubscribedProperty, CSDK_746) {
	char *thingName = "CSDK_746";
	char *propertyName = "MyInt";
	int i;
	DATETIME timestamps[100];
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,
	                  twApi_RegisterProperty(TW_THING, thingName, propertyName, TW_INTEGER, "A test property", "ALWAYS",
	                                         0.0, doNothing, NULL));
	for (i = 0; i < 100; i++) {
		twPrimitive *value = twPrimitive_CreateFromInteger(i);
		twSleepMsec(1); /* Sleep 1ms so that we get more varied timestamps */
		timestamps[i] = twGetSystemTime(TRUE);
		TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ(thingName, propertyName, value, timestamps[i], "GOOD", FALSE, FALSE));
	}
	twOfflineMsgStore_Disable();
	TEST_ASSERT_EQUAL(TW_SUBSCRIBED_PROPERTY_LIST_PERSIST_ERROR, twApi_PushSubscribedProperties(thingName, FALSE));
}

/**
 * CSDK-748
 *
 * Description: When calling twApi_SetSubscribedPropertyVTQ() spm->currentValues searched only for Property names, not Thing names when using
 * aspect change by value.
 *
 * Test Plan: Create two things, each with the same property, call twSubscribedPropsMgr_SetPropertyVTQ() on this
 * property for each of these things. Verify that the changes get queued for the correct things.
 */
TEST(unit_twApi_SetSubscribedProperty, CSDK_748) {

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"testThing1","testProperty1",TW_NUMBER,TW_NO_DESCRIPTION,TW_PUSH_TYPE_VALUE,0.1,CSDK_748_PropertyHandler,TW_NO_USER_DATA));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"testThing2","testProperty1",TW_NUMBER,TW_NO_DESCRIPTION,TW_PUSH_TYPE_VALUE,0.1,CSDK_748_PropertyHandler,TW_NO_USER_DATA));

	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ("testThing1","testProperty1",TW_MAKE_NUMBER(1.0),TW_NO_TIMESTAMP,TW_QUALITY_GOOD,TW_FOLD_TYPE_NO,TW_PUSH_LATER));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ("testThing2","testProperty1",TW_MAKE_NUMBER(2.0),TW_NO_TIMESTAMP,TW_QUALITY_GOOD,TW_FOLD_TYPE_NO,TW_PUSH_LATER));

	/* Now inspect the subscribed property list (spm->currentValues) and see if it got this right. There should be two
	 * entries, not the one we were getting because we were using only property name to look up values. */
	TEST_ASSERT_EQUAL(2,twDict_GetCount(getSPM()->currentValues));
}

/**
 * CSDK-750
 *
 * SetPropertyVTQ returns CHANGE_BUFFER_FULL if prop does not change by configured Value.
 *
 * The function twSubscribedPropsMgr_SetPropertyVTQ returns TW_PROPERTY_CHANGE_BUFFER_FULL if the passed VTQ is for a
 * property with push type VALUE, and the new value in the VTQ does not exceed (oldValue + threshold). The same happens
 * if the property is set to a push type of NEVER.
 *
 * Test Plan: Create a thing with two properties, one set to push type VALUE and the other NEVER. Push an initial value and
 * then push a second value lower than the threshold. Verify that TW_OK is returned.
 */
TEST(unit_twApi_SetSubscribedProperty, CSDK_750) {

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"testThing1","testProperty1",TW_NUMBER,TW_NO_DESCRIPTION,
	                                                TW_PUSH_TYPE_VALUE,1.0,CSDK_748_PropertyHandler,TW_NO_USER_DATA));
	TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING,"testThing1","testProperty2",TW_NUMBER,TW_NO_DESCRIPTION,
	                                                TW_PUSH_TYPE_NEVER,0,CSDK_748_PropertyHandler,TW_NO_USER_DATA));

	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ("testThing1","testProperty2",TW_MAKE_NUMBER(1.0),TW_NO_TIMESTAMP,TW_QUALITY_GOOD,TW_FOLD_TYPE_NO,TW_PUSH_LATER));

	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ("testThing1","testProperty1",TW_MAKE_NUMBER(1.0),TW_NO_TIMESTAMP,TW_QUALITY_GOOD,TW_FOLD_TYPE_NO,TW_PUSH_LATER));
	TEST_ASSERT_EQUAL(TW_OK, twApi_SetSubscribedPropertyVTQ("testThing1","testProperty1",TW_MAKE_NUMBER(0.1),TW_NO_TIMESTAMP,TW_QUALITY_GOOD,TW_FOLD_TYPE_NO,TW_PUSH_LATER));

	twApi_Delete();
}