#include <twThreads.h>
#include <twBaseTypes.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"


TEST_GROUP(MetadataIntegration);

void test_MetadataIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}


TEST_SETUP(MetadataIntegration) {
    eatLogs();
	
	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;
}

TEST_TEAR_DOWN(MetadataIntegration) {
    twStubs_Reset();
}

TEST_GROUP_RUNNER(MetadataIntegration) {
    RUN_TEST_CASE(MetadataIntegration, test_EvaluateRemoteThingMetadata);
}

enum msgCodeEnum EvaluateRemoteThingMetadataPropertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	if (value) {
		if (isWrite && *value) {
			return TWX_NOT_FOUND;
		} else {
			return TWX_BAD_REQUEST;
		}
	}
	return TWX_BAD_REQUEST;
}


enum msgCodeEnum evalRemoteMetadataMultiServiceHandler(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	return TWX_NOT_FOUND;
}

void evalRemoteMetadataRegisterProperties(const char *thingName) {/* Regsiter our properties */
	twApi_RegisterProperty(TW_THING, thingName, "FaultStatus", TW_BOOLEAN, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "InletValve", TW_BOOLEAN, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Pressure", TW_NUMBER, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Temperature", TW_NUMBER, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "TemperatureLimit", TW_NUMBER, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "TotalFlow", TW_NUMBER, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "BigGiantString", TW_STRING, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	twApi_RegisterProperty(TW_THING, thingName, "Location", TW_LOCATION, NULL, "ALWAYS", 0, EvaluateRemoteThingMetadataPropertyHandler, NULL);

	/* Add any additoinal aspects to our properties */
	twApi_AddAspectToProperty(thingName, "TemperatureLimit", "defaultValue", twPrimitive_CreateFromNumber(50.5));
}

void evalRemoteMetadataRegisterEvents(const char *thingName) {
	twDataShape* ds = twDataShape_Create(twDataShapeEntry_Create("message",NULL,TW_STRING));
	if (!ds) {
		TW_LOG(TW_ERROR, "Error Creating datashape.");
		exit(1);
	}
	/* Event datashapes require a name */
	twDataShape_SetName(ds, "SteamSensor.Fault");
	/* Register the service */
	twApi_RegisterEvent(TW_THING, thingName, "SteamSensorFault", "Steam sensor event", ds);

}

void evalRemoteMetadataRegisterServices(const char *thingName) {

	/* Register our services that have inputs */
	/* Create DataShape */
	twDataShape* ds = twDataShape_Create(twDataShapeEntry_Create("a",NULL,TW_NUMBER));
	if (!ds) {
		TW_LOG(TW_ERROR, "Error Creating datashape.");
		exit(1);
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("b",NULL,TW_NUMBER));
	/* Register the service */
	twApi_RegisterService(TW_THING, thingName, "AddNumbers", NULL, ds, TW_NUMBER, NULL, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	/* API now owns that datashape pointer, so we can reuse it */
	ds = NULL;
	/* Create a DataShape for the SteamSensorReadings service output */
	ds = twDataShape_Create( twDataShapeEntry_Create("SensorName", NULL, TW_STRING));
	if (!ds) {
		TW_LOG(TW_ERROR, "Error Creating datashape.");
		exit(1);
	}
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("Temperature", NULL, TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("Pressure", NULL, TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("FaultStatus", NULL, TW_BOOLEAN));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("InletValve", NULL, TW_BOOLEAN));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("TemperatureLimit", NULL, TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("TotalFlow", NULL, TW_INTEGER));
	/* Name the DataShape for the SteamSensorReadings service output */
	twDataShape_SetName(ds, "SteamSensorReadings");
	/* Register the service */
	twApi_RegisterService(TW_THING, thingName, "GetSteamSensorReadings", NULL, NULL, TW_INFOTABLE, ds, EvaluateRemoteThingMetadataPropertyHandler, NULL);
	/* API now owns that datashape pointer, so we can reuse it */


	/* Register our services that don't have inputs */
	twApi_RegisterService(TW_THING, thingName, "GetBigString", NULL, NULL, TW_STRING, NULL, evalRemoteMetadataMultiServiceHandler, NULL);
	twApi_RegisterService(TW_THING, thingName, "Shutdown", NULL, NULL, TW_NOTHING, NULL, evalRemoteMetadataMultiServiceHandler, NULL);
}
/**
 * Test Plan: Establish a server entity and then query its metadata, verify its content.
 * See EDGE-2166 https://thingworx.jira.com/browse/EDGE-2166
 */
TEST(MetadataIntegration, test_EvaluateRemoteThingMetadata){

    int res=0;
    twInfoTable* itout;
	const char* thingName = "SteamSensorEdge2166";
	twThread * api_thread = NULL;
	twThread * workerThread = NULL;
	twDataShape* ds;
	twInfoTable* it;
	twInfoTableRow* tableRow;
	twPrimitive* retJsonPrimitive;
	char* jsonData;
	cJSON* json;
	cJSON* pd;
	cJSON* sd;
	cJSON* ed;

	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("CSDK-1044");
	}

	// Upload required entity
    TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_SteamSensorEdge2166.xml"));

    // Establish Connection
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_MetadataIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();

	api_thread = twThread_Create(twApi_TaskerFunction, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != api_thread, "api thread cannot be created");

	workerThread = twThread_Create(twMessageHandler_msgHandlerTask, THREAD_RATE, NULL, TRUE);
	TEST_ASSERT_TRUE_MESSAGE(NULL != workerThread, "message thread cannot be created");

	evalRemoteMetadataRegisterProperties(thingName);
	evalRemoteMetadataRegisterServices(thingName);
	evalRemoteMetadataRegisterEvents(thingName);
	twApi_BindThing("SteamSensorEdge2166");


	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));

	ds = twDataShape_Create(NULL);
	it = twInfoTable_Create(ds);
	TEST_ASSERT_EQUAL(TW_OK,twApi_InvokeService(TW_THING, thingName, "GetRemoteMetadata", NULL, &itout, 5000, TRUE));
	TEST_ASSERT_EQUAL(1,itout->rows->count);
	TEST_ASSERT_EQUAL(1,itout->ds->numEntries);
	tableRow = twInfoTable_GetEntry(itout,0);
	TEST_ASSERT_NOT_NULL(tableRow);
	retJsonPrimitive = twInfoTableRow_GetEntry(tableRow,0);
	TEST_ASSERT_EQUAL(TW_JSON,retJsonPrimitive->type);
	jsonData = retJsonPrimitive->val.bytes.data;
	TEST_ASSERT_NOT_NULL(jsonData);

	// Is it parse-able?
	json = cJSON_Parse(jsonData);
	TEST_ASSERT_NOT_NULL(json);

	// Perform simple checks that these objects exist
	pd = cJSON_GetObjectItem(json,"propertyDefinitions");
	TEST_ASSERT_NOT_NULL(pd);
	sd = cJSON_GetObjectItem(json,"serviceDefinitions");
	TEST_ASSERT_NOT_NULL(sd);
	ed = cJSON_GetObjectItem(json,"eventDefinitions");
	TEST_ASSERT_NOT_NULL(cJSON_GetObjectItem(json,"eventDefinitions"));

	/* In future introspect deeper */

	/* cleanup  */
	cJSON_Delete(json);
	if (it) twInfoTable_Delete(it);
    if (itout) twInfoTable_Delete(itout);
	twThread_Delete(api_thread);
	twThread_Delete(workerThread);
    twApi_Disconnect("testing");
    twApi_Delete();
}



