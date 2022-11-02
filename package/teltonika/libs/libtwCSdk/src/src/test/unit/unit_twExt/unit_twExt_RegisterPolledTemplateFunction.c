/*
 * Created by William Reichardt on 7/17/16.
 */

#include <twBaseTypes.h>
#include "twApi.h"
#include "twShapes.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twThreadUtils.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"

TEST_GROUP(unit_twExt_RegisterPolledTemplateFunction);

TEST_SETUP(unit_twExt_RegisterPolledTemplateFunction){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twExt_RegisterPolledTemplateFunction){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twExt_RegisterPolledTemplateFunction){
	RUN_TEST_CASE(unit_twExt_RegisterPolledTemplateFunction, test_PollingFunctionForThingTemplate);
}

void onPollingTestThingProcessScanRequest(char *thingName) {
	double count = TW_GET_PROPERTY(thingName, "count").number;
	TW_SET_PROPERTY("pollingTestThing","count",TW_MAKE_NUMBER(count+1));
}

/**
 * Test Plan: Establish a function that should be polled periodically at 1 second intervals.
 * Call the twStart() function with a new thread and then allow that thread to run for 5 seconds.
 * Verify that the polled function was called at least 5 times then kill the thread.
 */
TEST(unit_twExt_RegisterPolledTemplateFunction,test_PollingFunctionForThingTemplate){
	int taskId = 0;
	double count = 0;
	char* thingName ="pollingTestThing";

	{
		TW_MAKE_THING(thingName, TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY("count", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_BIND();
	}

	TW_SET_PROPERTY("pollingTestThing","count",TW_MAKE_NUMBER(1));

	twExt_RegisterPolledTemplateFunction(onPollingTestThingProcessScanRequest, TW_THING_TEMPLATE_GENERIC);

	twExt_Start(1000, TW_THREADING_TASKER, 0);

	twSleepMsec(5000);

	twTasker_Stop();

	count = TW_GET_PROPERTY(thingName, "count").number;

	TW_LOG(TW_FORCE,"Ending Test onPollingTestThingProcessScanRequest.");

	TEST_ASSERT_TRUE_MESSAGE(5<=count,"This function should have been called at least 5");
	TEST_ASSERT_TRUE_MESSAGE(10>count,"This function should not have been called over 10 times");

}
