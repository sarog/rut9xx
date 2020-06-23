#include "twBaseTypes.h"
#include "twExt.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "SteamThing.h"
#include "twMacros.h"

const char * const arraySteamSensorPropertyNames[] = {"name","description","isConnected","TotalFlow","TemperatureLimit","Temperature","Pressure","Logfile","Location","InletValve","FaultStatus"};
enum BaseType arraySteamSensorPrimitiveTypes[] = {TW_STRING,TW_STRING,TW_BOOLEAN,TW_NUMBER,TW_NUMBER,TW_NUMBER,TW_NUMBER,TW_STRING,TW_LOCATION,TW_BOOLEAN,TW_BOOLEAN};
int arraySteamSensorPropertyCount = 11;
const char * const arraySteamSensorPropertiesThatChange[] = {"TotalFlow","Temperature","Pressure","Location","InletValve","FaultStatus"};
int arraySteamSensorPropertiesThatChangeCount = 6;

TEST_GROUP(SteamSensorIntegration);

void test_SteamSensorIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(SteamSensorIntegration) {
	eatLogs();

	/* Import test entities */
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Entities_SteamSensor.xml"));

	/* delete any previous message store binaries */
	/* accept "file does not exist"(2) as a result */
	twDirectory_DeleteFile(SUBSCRIBED_PROPERTY_LOCATION_FILE);

	/* set offline msg store file */
	twcfg_pointer->offline_msg_store_dir = SUBSCRIBED_PROPERTY_LOCATION;
	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;
	twDirectory_DeleteDirectory("./staging");
	twDirectory_CreateDirectory("./staging");
	twcfg_pointer->file_xfer_staging_dir = "./staging";
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, test_SteamSensorIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
											  MESSAGE_CHUNK_SIZE, TRUE));
	/* set connection params*/
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twExt_Start(1000, TW_THREADING_MULTI, 5);

	/* setup internal entity */
	createSteamSensorThing(STEAM_SENSOR_NAME);

	/* connect */
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(5000, 3));

	/* setup entities */
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Entities_SteamSensor.xml"));
}

TEST_TEAR_DOWN(SteamSensorIntegration) {
	destroySteamSensorThing(STEAM_SENSOR_NAME);
	TEST_ASSERT_EQUAL(TW_OK, twExt_Stop());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Test is ending."));
	TEST_ASSERT_FALSE(twApi_isConnected());
	twApi_Delete();
}

TEST_GROUP_RUNNER(SteamSensorIntegration) {
	RUN_TEST_CASE(SteamSensorIntegration, testAllSteamPropertiesExist);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorValueStream);
	RUN_TEST_CASE(SteamSensorIntegration, testInletValveFaultStatusRelationship);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorFetchLogService);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorGetBigStringService);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorGetSteamSensorReadings);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorSetFaultStatus);
	RUN_TEST_CASE(SteamSensorIntegration, testSteamSensorUpdateTemperatureLimit);
}

void testAllSteamPropertiesExistVerifyProperties(twInfoTable* propValues){
	int index;
	twPrimitive* value;
	char buffer[255];
	for(index=0;index<arraySteamSensorPropertyCount;index++){
		snprintf(buffer,255,"%s is missing.",arraySteamSensorPropertyNames[index]);
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK,twInfoTable_GetPrimitive(propValues,arraySteamSensorPropertyNames[index],0,&value),buffer);
		TEST_ASSERT_NOT_NULL(value);
		TEST_ASSERT_EQUAL(arraySteamSensorPrimitiveTypes[index],value->type);
	}
}

void testAllSteamPropertiesExistVerifyValuesHaveChanged(twInfoTable* propValues1,twInfoTable* propValues2){
	int index;
	for(index=0;index<arraySteamSensorPropertiesThatChangeCount;index++){
		twPrimitive * twPrimitive1,*twPrimitive2;
		char buffer[255];
		TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(propValues1,arraySteamSensorPropertiesThatChange[index],0,&twPrimitive1));
		TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(propValues1,arraySteamSensorPropertiesThatChange[index],0,&twPrimitive2));
		snprintf(buffer,255,"Primitive values for %s did not change as expected.",(char*)arraySteamSensorPropertiesThatChange);
		TEST_ASSERT_FALSE_MESSAGE(twPrimitive_Compare(twPrimitive1,twPrimitive2),buffer);
	}
}

/**
 * Test Plan: Verify that all properties exist, are the types expected and that the ones that should change over time, are.
 */
TEST(SteamSensorIntegration, testAllSteamPropertiesExist) {
	twInfoTable* itPropertyValues1 = NULL;
	twInfoTable* itPropertyValues2 = NULL;

	/* Get the current property values */
	TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)STEAM_SENSOR_NAME, "GetPropertyValues", NULL, &itPropertyValues1, 5000, TRUE));

	testAllSteamPropertiesExistVerifyProperties(itPropertyValues1);
	twSleepMsec(5000);

	TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)STEAM_SENSOR_NAME, "GetPropertyValues", NULL, &itPropertyValues2, 5000, TRUE));
	testAllSteamPropertiesExistVerifyValuesHaveChanged(itPropertyValues1,itPropertyValues2);

	twInfoTable_Delete(itPropertyValues1);
	twInfoTable_Delete(itPropertyValues2);
}

/**
 * Test Plan: Set the temperature limit low, set fault status false and wait for Inlet Valve to open. Once open,
 * verify that Fault status is now set
 */
TEST(SteamSensorIntegration, testInletValveFaultStatusRelationship) {
	twInfoTable *params_it = NULL;
	twInfoTable *result_it = NULL;
	twPrimitive *result_prim = NULL;
	char inletValve = FALSE;
	int retries = 0;

	TEST_ASSERT_TRUE(twApi_isConnected());
	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("status", TW_NO_DESCRIPTION, TW_BOOLEAN)
			),TW_IT_ROW(TW_MAKE_BOOL(FALSE))
	);
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "setFaultStatus", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	twInfoTable_Delete(result_it);
	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("limit", TW_NO_DESCRIPTION, TW_NUMBER)
			),TW_IT_ROW(TW_MAKE_NUMBER(300.0))
	);
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "updateTemperatureLimit", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	while (!inletValve) {
		if (retries > 10) TEST_FAIL_MESSAGE("Timed out waiting for inlet valve to open");
		TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, STEAM_SENSOR_NAME, "InletValve", &result_prim, twcfg_pointer->default_message_timeout, FALSE));
		inletValve = result_prim->val.boolean;
		twPrimitive_Delete(result_prim);
		twSleepMsec(1000);
		retries++;
	}
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, STEAM_SENSOR_NAME, "FaultStatus", &result_prim, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(true, result_prim->val.boolean);
	twInfoTable_Delete(result_it);
	twPrimitive_Delete(result_prim);
}

/**
 * Test Plan: Verify that the SteamSensorValueStream is expanding by first clearing its history and then
 * waiting for it to build back up.
 */
TEST(SteamSensorIntegration, testSteamSensorValueStream) {
	/* setup vars */
	int vs_size_1, vs_size_2=0;
	char errorBuffer[255];
	int waitSeconds = 0;
	int res;

	twInfoTable * result_it = NULL;

	TEST_IGNORE_MESSAGE("CSDK-1184 Test is passing locally, failing in CI");

	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)STEAM_SENSOR_NAME, "clearAllHistory", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	twInfoTable_Delete(result_it);
	twSleepMsec(10000);

	/* read value stream size */
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)STEAM_SENSOR_NAME, "QueryPropertyHistory", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	vs_size_1 = result_it->rows->count;
	TW_LOG(TW_TRACE, "testSteamSensorValueStream: value stream size 1 = %d", vs_size_1);
	twInfoTable_Delete(result_it);

	while(waitSeconds<20 && vs_size_2<=vs_size_1) {

		/* read value stream size */
		res = twApi_InvokeService(TW_THING, (char *) STEAM_SENSOR_NAME, "QueryPropertyHistory", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE);
		vs_size_2 = result_it->rows->count;
		TW_LOG(TW_TRACE, "testSteamSensorValueStream: value stream size 2 = %d", vs_size_2);
		twInfoTable_Delete(result_it);
		waitSeconds++;
		twSleepMsec(1000);
	}
	TEST_ASSERT_EQUAL(TW_OK, res);
	/* Assert first read is smaller than second read */
	snprintf(errorBuffer,255,"%i should be larger than %i. The value stream is not growing.",vs_size_2 ,vs_size_1);
	TEST_ASSERT_TRUE_MESSAGE(vs_size_2 > vs_size_1,errorBuffer);
}

void onSteamSensorProcessScanRequest(char *thingName);

/**
 * Test Plan: Verify that the SteamSensor fetchLog() service copies the current log,SteamSensor-log.txt, down to the SteamSensorMashupFileRepository.
 */
TEST(SteamSensorIntegration, testSteamSensorFetchLogService) {
	int retry=0, ret = 0;
	char fileExists = FALSE;
	twInfoTable *result_it = NULL;
	twInfoTable *params_it = NULL;

	twSleepMsec(5000);
	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION ,TW_STRING)
	),TW_IT_ROW(TW_MAKE_STRING("SteamSensor-log.txt")));
	twApi_InvokeService(TW_THING, STEAM_SENSOR_REPO_NAME, "DeleteFile", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE);
	twInfoTable_Delete(result_it);

	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
							  TW_DS_ENTRY("path", TW_NO_DESCRIPTION ,TW_STRING)
			),TW_IT_ROW(TW_MAKE_STRING("SteamSensor-log.txt.checksum_mismatch")));
	twApi_InvokeService(TW_THING, STEAM_SENSOR_REPO_NAME, "DeleteFile", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE);
	twInfoTable_Delete(result_it);

	twSleepMsec(5000);
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "fetchLog", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	twInfoTable_Delete(result_it);

	while(fileExists==FALSE && retry < 10) {
		params_it = TW_MAKE_INFOTABLE(
				TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
								  TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
				), TW_IT_ROW(TW_MAKE_STRING("SteamSensor-log.txt")));
		ret = twApi_InvokeService(TW_THING, STEAM_SENSOR_REPO_NAME, "GetFileInfo", params_it, &result_it,
							  twcfg_pointer->default_message_timeout, TRUE);
		twInfoTable_Delete(result_it);
		if(TW_OK==ret) {
			fileExists = TRUE;
			break;
		}
		retry++;
		twSleepMsec(1000);
	}
	TEST_ASSERT_TRUE_MESSAGE(fileExists,"File failed to be transferred when requested.");

}

char *makeBigString() {
	int len = 10000;
	char text[] = "This is a really big string. ";
	int textlen;
	char * bigString = (char *)TW_CALLOC(len,1);
	textlen = strlen(text);
	while (bigString && len > textlen) {
		strncat(bigString, text, len - textlen - 1);
		len = len - textlen;
	}
	return bigString;
}

/**
 * Test Plan: Call the getBigString service and verify its return value
 */
TEST(SteamSensorIntegration, testSteamSensorGetBigStringService) {
	twInfoTable *result_it = NULL;
	char *bigString1 = NULL;
	char *bigString2 = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "GetBigString", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twInfoTable_GetString(result_it, "result", 0, &bigString1));
	bigString2 = makeBigString();
	TEST_ASSERT_EQUAL_STRING(bigString1, bigString2);
	twInfoTable_Delete(result_it);
	TW_FREE(bigString1);
	TW_FREE(bigString2);
}

/**
 * Test Plan: Call the GetSteamSensorReadings service and verify its return value
 */
TEST(SteamSensorIntegration, testSteamSensorGetSteamSensorReadings) {
	int i = 0;
	twInfoTable *result_it = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "GetSteamSensorReadings", NULL, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(3, result_it->rows->count);
	TEST_ASSERT_EQUAL(8, result_it->ds->numEntries);
	for (i = 0; i < 3; i++) {
		twInfoTableRow *row = NULL;
		row = twInfoTable_GetEntry(result_it, 0);
		TEST_ASSERT_NOT_NULL(row);
		TEST_ASSERT_EQUAL(8, row->numFields);
	}
	twInfoTable_Delete(result_it);
}

/**
 * Test Plan: Call the SetFaultStatus service and verify its effect
 */
TEST(SteamSensorIntegration, testSteamSensorSetFaultStatus) {
	twInfoTable *params_it = NULL;
	twInfoTable *result_it = NULL;
	twPrimitive *result_prim = NULL;
	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("status", TW_NO_DESCRIPTION, TW_BOOLEAN)
			),TW_IT_ROW(TW_MAKE_BOOL(TRUE))
	);
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "setFaultStatus", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, STEAM_SENSOR_NAME, "FaultStatus", &result_prim, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_TRUE(result_prim->val.boolean);
	twInfoTable_Delete(result_it);
	twPrimitive_Delete(result_prim);
}

/**
 * Test Plan: Call the UpdateTemperatureLimit service and verify its effect
 */
TEST(SteamSensorIntegration, testSteamSensorUpdateTemperatureLimit) {
	twInfoTable *params_it = NULL;
	twInfoTable *result_it = NULL;
	twPrimitive *result_prim = NULL;
	params_it = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
			                  TW_DS_ENTRY("limit", TW_NO_DESCRIPTION, TW_NUMBER)
			),TW_IT_ROW(TW_MAKE_NUMBER(123.45))
	);
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, STEAM_SENSOR_NAME, "updateTemperatureLimit", params_it, &result_it, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, STEAM_SENSOR_NAME, "TemperatureLimit", &result_prim, twcfg_pointer->default_message_timeout, FALSE));
	TEST_ASSERT_EQUAL(123.45, result_prim->val.number);
	twInfoTable_Delete(result_it);
	twPrimitive_Delete(result_prim);
}