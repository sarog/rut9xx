/**
 * Subscribed Property Unit Tests
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
int stub_makeSynchronizedStateCallbacks_NoOpOK (char * entityName, enum entityTypeEnum entityType, twInfoTable* subscriptionData) {
	return TW_OK;
}
int twApi_PushSubscribedProperties_Stub_NoOpOK (char * entityName, char forceConnect) {
	return TW_OK;
}
int twApi_UpdatePropertyMetaData_Stub_NoOpOK (enum entityTypeEnum entityType, char * entityName, char * propertyName, enum BaseType propertyType, char * propertyDescription, char * propertyPushType, double propertyPushThreshold) {
	return TW_OK;
}
twInfoTable* invokeServiceParams = NULL;
int twApi_InvokeService_Stub_CaptureParams (enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	/* Avoid ambiguity - test should clean this up if you want to re-invoke this. */
	TEST_ASSERT_NULL (invokeServiceParams);
	invokeServiceParams = twInfoTable_FullCopy (params);
	/* Fail the service call, we only care about capturing the params*/
	return TW_INTERNAL_SERVER_ERROR;
}
int subscribedPropertyUpdateTaskForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg);
int stub_makeSynchronizedStateCallbacks_CSDK780 (char * entityName, enum entityTypeEnum entityType, twInfoTable* subscriptionData) {
	FAIL("makeSynchronizedStateCallbacks called with NULL InfoTable");
}

/**
 * Unity Test Macros
 */
TEST_GROUP(unit_subscribedPropertyUpdateTaskForEachHandler);
TEST_SETUP(unit_subscribedPropertyUpdateTaskForEachHandler) {
	eatLogs();
	twTest_DeleteAndSetPersistedBinFileLocations();
}
TEST_TEAR_DOWN(unit_subscribedPropertyUpdateTaskForEachHandler) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}
TEST_GROUP_RUNNER(unit_subscribedPropertyUpdateTaskForEachHandler) {
	RUN_TEST_CASE(unit_subscribedPropertyUpdateTaskForEachHandler, CSDK_780);
	RUN_TEST_CASE(unit_subscribedPropertyUpdateTaskForEachHandler, SubscribedPropertyUpdateTaskServiceInvocation);
}

/**
 * CSDK-780
 *
 * subscribedPropertyUpdateTaskForEachHandler should not call makeSynchronizedStateCallbacks unless it receives a TWX_SUCCESS
 * when invoking `GetPropertySubscriptions`.
 *
 * Test Plan: Create a bindListEntry with `needsPropertyUpdate` set to TRUE. Invoke `subscribedPropertyUpdateTaskForEachHandler`,
 * passing the bindListEntry as the data argument.
 */
TEST(unit_subscribedPropertyUpdateTaskForEachHandler, CSDK_780) {
	int err = 0;
	bindListEntry *entry = NULL;
	/* use stubs to mock makeSynchronizedStateCallbacks */
	err = twStubs_Use();
	if (err) {
		/* stubs are disabled, exit test */
		TEST_FAIL();
	}

	twApi_stub->makeSynchronizedStateCallbacks = stub_makeSynchronizedStateCallbacks_CSDK780;

	entry = s_bindListEntry_Create("TestThing");
	entry->needsPropertyUpdate = TRUE;
	subscribedPropertyUpdateTaskForEachHandler(NULL, NULL, entry, NULL, NULL);
	s_bindListEntry_Delete(entry);
	twApi_DeleteStubs();
}

/**
* SubscribedPropertyUpdateTaskServiceInvocation
*
* Tests only that GetPropertySubscriptions is invoked using the correct
* arguments (an "options" infotable describing the SDK's capabilities)
*/
TEST(unit_subscribedPropertyUpdateTaskForEachHandler, SubscribedPropertyUpdateTaskServiceInvocation) {
	twPrimitive* primitive = NULL;
	cJSON* json = NULL;
	bindListEntry ble;
	ble.name = "SomeProperty";
	ble.needsPropertyUpdate = TRUE;

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "SubscribedPropertyIntegrationTests SETUP: failed to initialize API");
	twStubs_Use();
	/* Stubbery */
	twApi_stub->makeSynchronizedStateCallbacks = stub_makeSynchronizedStateCallbacks_NoOpOK;
	twApi_stub->twApi_PushSubscribedProperties = twApi_PushSubscribedProperties_Stub_NoOpOK;
	twApi_stub->twApi_UpdatePropertyMetaData = twApi_UpdatePropertyMetaData_Stub_NoOpOK;
	twApi_stub->twApi_InvokeService = twApi_InvokeService_Stub_CaptureParams;

	subscribedPropertyUpdateTaskForEachHandler (NULL, NULL, &ble, 0, NULL);
	TEST_ASSERT_NOT_NULL (invokeServiceParams);

	TEST_ASSERT_EQUAL (TW_OK, twInfoTable_GetPrimitive (invokeServiceParams, "options", 0, &primitive));
	TEST_ASSERT_NOT_NULL (primitive);
	json = twPrimitive_ToJson ("", primitive, NULL);
	TEST_ASSERT_EQUAL_STRING ("hasDeadband", json->child->child->string);
	TEST_ASSERT_EQUAL (cJSON_True, json->child->child->type);
	cJSON_Delete (json);
	twInfoTable_Delete (invokeServiceParams);
	invokeServiceParams = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
}