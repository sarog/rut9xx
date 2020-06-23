/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include "twBaseTypes.h"
#include "twServices.h"
#include "twApi.h"
#include "twShapes.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

enum msgCodeEnum getSteamSensorReadingsServiceunit_twService_Create(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	/* Do nothing */
	return TWX_SUCCESS;
}

enum msgCodeEnum getBigString(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	*content = twInfoTable_CreateFromString("result", "OK, this is not a very big string.",TRUE);
	return TWX_SUCCESS;
}

TEST_GROUP(unit_twService_Create);

TEST_SETUP(unit_twService_Create){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twcfg_pointer->offline_msg_store_dir = SUBSCRIBED_PROPERTY_LOCATION;
}
TEST_TEAR_DOWN(unit_twService_Create){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twService_Create) {
	RUN_TEST_CASE(unit_twService_Create, test_declareServiceMacro);
}

/**
 * Test Plan: Declare two things each with separate services then confirm that
 * the correct metadata was generated.
 */
TEST(unit_twService_Create,test_declareServiceMacro) {
	twServiceDef* getSteamSensorReadingsDef = NULL;
	twDataShape* steamInputShape = NULL;
	twDataShape* steamOutputShape = NULL;
	twServiceDef* getBigStringDefinition = NULL;
	{
		TW_MAKE_THING("thing1",TW_THING_TEMPLATE_GENERIC);
		TW_DECLARE_SERVICE("GetSteamSensorReadings",
		                   "Returns readings as in InfoTable",
		                   TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
		                                     TW_DS_ENTRY("a", TW_NO_DESCRIPTION, TW_NUMBER),
		                                     TW_DS_ENTRY("b", TW_NO_DESCRIPTION, TW_BOOLEAN),
		                                     TW_DS_ENTRY("c", TW_NO_DESCRIPTION, TW_STRING)
		                   ),
		                   TW_INFOTABLE,
		                   TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
		                                     TW_DS_ENTRY("Temperature", TW_NO_DESCRIPTION ,TW_NUMBER),
		                                     TW_DS_ENTRY("Pressure", TW_NO_DESCRIPTION ,TW_NUMBER),
		                                     TW_DS_ENTRY("FaultStatus", TW_NO_DESCRIPTION ,TW_BOOLEAN),
		                                     TW_DS_ENTRY("InletValve", TW_NO_DESCRIPTION ,TW_BOOLEAN),
		                                     TW_DS_ENTRY("TemperatureLimit", TW_NO_DESCRIPTION ,TW_NUMBER),
		                                     TW_DS_ENTRY("TotalFlow", TW_NO_DESCRIPTION ,TW_INTEGER)
		                   ),
		                   getSteamSensorReadingsServiceunit_twService_Create
		);
	}

	{
		TW_MAKE_THING("thing2",TW_THING_TEMPLATE_GENERIC);
		TW_DECLARE_SERVICE("GetBigString",
		                   "Generates a big string.",
		                   TW_NO_PARAMETERS,
		                   TW_STRING,
		                   TW_NO_RETURN_DATASHAPE,
		                   getBigString
		);
	}

	getSteamSensorReadingsDef = findService("thing1", "GetSteamSensorReadings");
	TEST_ASSERT_NOT_NULL(getSteamSensorReadingsDef);
	TEST_ASSERT_EQUAL_STRING("GetSteamSensorReadings",getSteamSensorReadingsDef->name);
	TEST_ASSERT_NOT_NULL(getSteamSensorReadingsDef->inputs);
	steamInputShape = getSteamSensorReadingsDef->inputs;
	TEST_ASSERT_EQUAL(3,steamInputShape->numEntries);
	TEST_ASSERT_EQUAL(TW_INFOTABLE,getSteamSensorReadingsDef->outputType);
	steamOutputShape = getSteamSensorReadingsDef->outputDataShape;
	TEST_ASSERT_EQUAL(6,steamOutputShape->numEntries);

	getBigStringDefinition = findService("thing2", "GetBigString");
	TEST_ASSERT_NOT_NULL(getBigStringDefinition);
	TEST_ASSERT_EQUAL_STRING("GetBigString",getBigStringDefinition->name);
	TEST_ASSERT_EQUAL(TW_STRING,getBigStringDefinition->outputType);

}