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

enum msgCodeEnum getString(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	*content = twInfoTable_CreateFromString("result", "This is a string",TRUE);
	return TWX_SUCCESS;
}

TEST_GROUP(unit_twExt_GetCallbackForService);

TEST_SETUP(unit_twExt_GetCallbackForService){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	twcfg_pointer->offline_msg_store_dir = SUBSCRIBED_PROPERTY_LOCATION;
}
TEST_TEAR_DOWN(unit_twExt_GetCallbackForService){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twExt_GetCallbackForService) {
	RUN_TEST_CASE(unit_twExt_GetCallbackForService, test_CallServiceLocallyByName);
}

/**
 * Test Plan: Look up a registered service at runtime and call it locally. This
 * model is used for inter shape communication at runtime.
 */
TEST(unit_twExt_GetCallbackForService,test_CallServiceLocallyByName){
	twInfoTable * params = NULL;
	twInfoTable * content = NULL;
	void * userdata = NULL;
	service_cb getStringService;
	enum msgCodeEnum result;
	char * returnedBigString;
	/* Declare Thing With Service */
	{
		TW_MAKE_THING("thingLocalServiceCall",TW_THING_TEMPLATE_GENERIC);
		TW_DECLARE_SERVICE("GetBigString",
		                   "Generates a big string.",
		                   TW_NO_PARAMETERS,
		                   TW_STRING,
		                   TW_NO_RETURN_DATASHAPE,
		                   getString
		);
	}
	/* Now try to call that service locally */
	getStringService = (service_cb) twExt_GetCallbackForService("thingLocalServiceCall", "GetBigString");
	TEST_ASSERT_NOT_NULL(getStringService);

	result = getStringService("thingLocalServiceCall", "GetBigString",  params, &content, userdata);
	TEST_ASSERT_EQUAL(TWX_SUCCESS,result);


	twInfoTable_GetString(content, "result", 0, &returnedBigString);
	TEST_ASSERT_EQUAL_STRING("This is a string",returnedBigString);

}