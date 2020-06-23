/*
*	Created by Jeff Dreyer on 5/26/16.
*	Copyright 2016, PTC, Inc.
*/

#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10
#define NAME_SIZE 64

#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"

#define TW_NO_RETURN_DATASHAPE NULL
#define TW_NO_USER_DATA NULL
#define TW_MAX_LIST_SIZE 1000
#define CONNECTION_RETRIES 1
#define CONNECTION_TIMEOUT 2000
#define NUM_WORKERS 1

int makeAuthOrBindCallbacks(char * entityName, enum entityTypeEnum entityType, char type, char * value);
void deleteCallbackInfo(void * info);
void bindListEntry_Delete(void * entry);
const char* bindListEntry_Parser(void * data);

TEST_GROUP(ForeachPerformance);

void test_ForeachPerformanceAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(ForeachPerformance) {
	/* logging will GREATLY reduce performance of these tests */
    eatLogs();
}

TEST_TEAR_DOWN(ForeachPerformance) {
}

TEST_GROUP_RUNNER(ForeachPerformance) {
    RUN_TEST_CASE(ForeachPerformance, perfTestInfoTableRowAccess);
    RUN_TEST_CASE(ForeachPerformance, perfMakeAuthOrBindCallbacks);
    RUN_TEST_CASE(ForeachPerformance, perfTwApi_BindThings);
	RUN_TEST_CASE(ForeachPerformance, perfTwApi_PushProperties);
	RUN_TEST_CASE(ForeachPerformance, perfTwApi_UnregisterBindEventCallback);
	RUN_TEST_CASE(ForeachPerformance, perfTwApi_UnregisterOnAuthenticatedCallback);
	RUN_TEST_CASE(ForeachPerformance,perfInfotableToJson);
}


/**
 * Test Plan: Build an info table up from 1 row to TW_MAX_LIST_SIZE rows and after adding each row, measure the time it takes
 * to enumerate it to the end.
 */
TEST(ForeachPerformance,perfTestInfoTableRowAccess){
    int index;
    int max_it_rows = TW_MAX_LIST_SIZE;
    char namebuffer[200];
    twDataShapeEntry* dse;
    twDataShape* ds;
    twInfoTable* it;
    twInfoTableRow* row;
    char* value;
    long totalRunTimeMs=0;

	TW_LOG(TW_TRACE,"perfTestInfoTableRowAccess");

    /* Create Thing */
    dse = twDataShapeEntry_Create("name",NULL,TW_STRING);
    ds = twDataShape_Create(dse);
    it = twInfoTable_Create(ds);

    for(index = 0; index < max_it_rows; index++){
        sprintf(namebuffer,"row %i",index);
        row = twInfoTableRow_Create(twPrimitive_CreateFromString(namebuffer,TRUE));
        twInfoTable_AddRow(it,row);
        METRIC("csdk.performance.perfTestInfoTableRowAccess.rowCount",index);
        {
            MARK_START("csdk.performance.perfTestInfoTableRowAccess.accessTime");
            twInfoTable_GetString(it, "name", index, &value);
            MARK_END("csdk.performance.perfTestInfoTableRowAccess.accessTime");
            totalRunTimeMs=totalRunTimeMs+duration;
        }
        TEST_ASSERT_EQUAL_STRING(namebuffer,value);
	    TW_FREE(value);
    }

    twInfoTable_Delete(it);
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Adverage Test Run Time = %f\n",(double)totalRunTimeMs/max_it_rows);
    TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<1000,"Enumeration of an infotable should take less than a second.");

}

void perfMakeAuthOrBindCallbacks_BindEventHandler(char * entityName, char isBound, void * userdata) {
    /* DO nothing */
}

/**
 * Test Plan: Bind 1 to TW_MAX_LIST_SIZE callbacks and measure the total time taken as the callback list increases.
 */
TEST(ForeachPerformance,perfMakeAuthOrBindCallbacks) {
    int index;
    int maxCallbacks=TW_MAX_LIST_SIZE;
    char thingName[200];
    long totalRunTimeMs=0;

	TW_LOG(TW_TRACE,"perfMakeAuthOrBindCallbacks");
    for(index = 0 ; index < maxCallbacks; index++) {
        sprintf(thingName,"perfMakeAuthOrBindCallbacks_ThingName-%i",index);
        twApi_RegisterBindEventCallback(thingName,
                                        perfMakeAuthOrBindCallbacks_BindEventHandler, NULL);
        METRIC("csdk.performance.perfMakeAuthOrBindCallbacks.handlerCount",index);
        {
            MARK_START("csdk.performance.perfMakeAuthOrBindCallbacks.bindCallbackTime");
            makeAuthOrBindCallbacks(thingName, TW_THING, 1, NULL);
            MARK_END("csdk.performance.perfMakeAuthOrBindCallbacks.bindCallbackTime");
            totalRunTimeMs=totalRunTimeMs+duration;
        }

    }
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",(double)totalRunTimeMs/maxCallbacks);
    TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<1500,"Callback Test Total Runtime should be under 1.5 seconds.");
}

void perfTwApi_BindThingsStringDeleter (void * item){

}
/**
 * Test Plan: Generate a list of 1 to TW_MAX_LIST_SIZE Things to bind all at once and measure how fast they get bound.
 */
TEST(ForeachPerformance,perfTwApi_BindThings) {
    int index;
    int maxBindListSize=TW_MAX_LIST_SIZE;
    char thingName[200];
    long totalRunTimeMs=0;
    twList* bindThingnameList = twList_Create(perfTwApi_BindThingsStringDeleter);

    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ForeachPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));
	TW_LOG(TW_TRACE,"perfTwApi_BindThings");

    for(index = 0 ; index < maxBindListSize; index++) {
        sprintf(thingName,"perfTwApi_BindThings_ThingName-%i",index);
        twList_Add(bindThingnameList,thingName);
        METRIC("csdk.performance.perfTwApi_BindThings.thingCount",index);
        {
            MARK_START("csdk.performance.perfTwApi_BindThings.bindTime");
            twApi_BindThings(bindThingnameList);
            MARK_END("csdk.performance.perfTwApi_BindThings.bindTime");
            totalRunTimeMs=totalRunTimeMs+duration;
        }
	    /* Flush the bound list to prevent it from growing huge. */
	    twMap_Delete(tw_api->boundList);
	    tw_api->boundList = twMap_Create(bindListEntry_Delete,bindListEntry_Parser);

    }
	twList_Delete(bindThingnameList);

	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",(double)totalRunTimeMs/maxBindListSize);
    TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<150000,"BindThings Test Total Runtime should be under 15 seconds.");

}

int perfTwApi_PushProperties_InvokeService(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	return TW_OK;
}

/**
 * Test Plan: Increase size of list of properties to be posted from 1 to TW_MAX_LIST_SIZE and measure time
 * for twApi_PushProperties() to execute.
 */
TEST(ForeachPerformance,perfTwApi_PushProperties) {
    int index;
    int maxPropertyListSize=TW_MAX_LIST_SIZE;
    char* thingName="perfTwApi_PushProperties_Thing";
    long totalRunTimeMs=0;
	char propName[200];
    twList* properties = twApi_CreatePropertyList("firstProp", twPrimitive_CreateFromString("firstValue",TRUE), twGetSystemTime(TRUE));

	TW_LOG(TW_TRACE,"perfTwApi_PushProperties");

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ForeachPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));

	twApi_stub->twApi_InvokeService = perfTwApi_PushProperties_InvokeService;

    for(index = 0 ; index < maxPropertyListSize; index++) {
        sprintf(propName,"perfTwApi_BindThings_ThingName-%i",index);
	    twApi_AddPropertyToList(properties, propName,  twPrimitive_CreateFromString("firstValue",TRUE), twGetSystemTime(TRUE));
        METRIC("csdk.performance.perfTwApi_PushProperties.propertyCount",index);
        {
            MARK_START("csdk.performance.perfTwApi_PushProperties.pushTime");
	        TEST_ASSERT_EQUAL_INT(TW_OK, twApi_PushProperties(TW_THING, thingName, properties, 1000, TRUE));
            MARK_END(  "csdk.performance.perfTwApi_PushProperties.pushTime");
            totalRunTimeMs=totalRunTimeMs+duration;
        }

    }
	twApi_DeletePropertyList(properties);
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",(double)totalRunTimeMs/maxPropertyListSize);
    TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<200000,"PushProperties Test Total Runtime should be under 200 seconds.");
	twApi_Delete();
}

void testTwApi_UnregisterBindEventCallback(char * entityName, char isBound, void * userdata){}

/**
 * Test Plan: Increase size of list of the bind event callback list from 1 to TW_MAX_LIST_SIZE and measure time
 * to delete the last member using twApi_UnregisterBindEventCallback() to execute.
 */
TEST(ForeachPerformance,perfTwApi_UnregisterBindEventCallback) {
	int index;
	int maxPropertyListSize=TW_MAX_LIST_SIZE;
	long totalRunTimeMs=0;
	char thingName[200];

	TW_LOG(TW_TRACE,"perfTwApi_UnregisterBindEventCallback");

	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ForeachPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                              MESSAGE_CHUNK_SIZE, TRUE));

	for(index = 0 ; index < maxPropertyListSize; index++) {
		sprintf(thingName,"perfTwApi_UnregisterBindEventCallback-%i",index);
		METRIC("csdk.performance.perfTwApi_UnregisterBindEventCallback.callbackCount",index);
		/* Insert a bind event callback */
		TEST_ASSERT_EQUAL_INT(TW_OK,twApi_RegisterBindEventCallback(thingName, testTwApi_UnregisterBindEventCallback, (void *)thingName));


		{
			MARK_START("csdk.performance.perfTwApi_UnregisterBindEventCallback.deleteTime");
			TEST_ASSERT_EQUAL_INT(TW_OK, twApi_UnregisterBindEventCallback(thingName, testTwApi_UnregisterBindEventCallback,(void*)thingName ));
			MARK_END(  "csdk.performance.perfTwApi_UnregisterBindEventCallback.deleteTime");
			totalRunTimeMs=totalRunTimeMs+duration;

			/* Put it back */
			TEST_ASSERT_EQUAL_INT(TW_OK,twApi_RegisterBindEventCallback(thingName, testTwApi_UnregisterBindEventCallback, (void *)thingName));
		}

	}
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",(double)totalRunTimeMs/maxPropertyListSize);
	TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<3000,"UnregisterBindEventCallback Test Total Runtime should be under 3 seconds.");
	twApi_Delete();

}

void testTwApi_UnregisterOnAuthenticatedCallbackCallback(char * credentialType, char * credentialValue, void * userdata){}

/**
 * Test Plan: Increase size of list of the bind event callback list from 1 to TW_MAX_LIST_SIZE and measure time
 * to delete the last member using twApi_UnregisterOnAuthenticatedCallback() to execute.
 */
TEST(ForeachPerformance,perfTwApi_UnregisterOnAuthenticatedCallback) {
	int index,insideIndex,listSize=0;
	int maxCallbackListSize=TW_MAX_LIST_SIZE;
	long totalRunTimeMs=0;
	char thingName[200];
	char* badName="badName";

	TW_LOG(TW_TRACE,"perfTwApi_UnregisterOnAuthenticatedCallback");
 	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ForeachPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                              MESSAGE_CHUNK_SIZE, TRUE));

	for(index = 0 ; index < maxCallbackListSize; index++) {
		sprintf(thingName, "perfTwApi_UnregisterOnAuthenticatedCallback-%i", index);
		for(insideIndex = 0;insideIndex< listSize;insideIndex++) {
			TEST_ASSERT_EQUAL_INT(TW_OK, twApi_RegisterOnAuthenticatedCallback(
					testTwApi_UnregisterOnAuthenticatedCallbackCallback, (void *) badName));
		}
		listSize++;
		TEST_ASSERT_EQUAL_INT(TW_OK, twApi_RegisterOnAuthenticatedCallback(
				testTwApi_UnregisterOnAuthenticatedCallbackCallback, (void *) thingName));


		METRIC("csdk.performance.perfTwApi_UnregisterOnAuthenticatedCallback.callbackCount", index);
		{
			MARK_START("csdk.performance.perfTwApi_UnregisterOnAuthenticatedCallback.deleteTime");
			TEST_ASSERT_EQUAL_INT(TW_OK, twApi_UnregisterOnAuthenticatedCallback(
					testTwApi_UnregisterOnAuthenticatedCallbackCallback, (void *) thingName));
			MARK_END("csdk.performance.perfTwApi_UnregisterOnAuthenticatedCallback.deleteTime");
			totalRunTimeMs = totalRunTimeMs + duration;

		}

		/* Dispose of the current callback list and start over */
		twList_Delete(tw_api->bindEventCallbackList);
		tw_api->bindEventCallbackList = twList_Create(deleteCallbackInfo);

	}
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",(double)totalRunTimeMs/maxCallbackListSize);
	TEST_ASSERT_TRUE_MESSAGE(totalRunTimeMs<3000,"UnregisterOnAuthenticatedCallback Test Total Runtime should be under 3 seconds.");


}

/**
 * Test Plan: Increase size of list of the bind event callback list from 1 to TW_MAX_LIST_SIZE and measure time
 * to delete the last member using twApi_UnregisterOnAuthenticatedCallback() to execute.
 */
TEST(ForeachPerformance,perfInfotableToJson) {
	int index=0;
	int totalRowsToCreate = 5000;
	double average;
	long totalRunTimeMs=0;
	struct cJSON *json;
	twDataShape* ds = TW_MAKE_DATASHAPE(TW_SHAPE_NAME_NONE,
										TW_DS_ENTRY("a", TW_NO_DESCRIPTION ,TW_NUMBER),
										TW_DS_ENTRY("b", TW_NO_DESCRIPTION ,TW_STRING),
										TW_DS_ENTRY("c", TW_NO_DESCRIPTION ,TW_BOOLEAN)
	);
	twInfoTable* testInfoTable = TW_MAKE_IT(
			ds,
			TW_IT_ROW(TW_MAKE_NUMBER(1),TW_MAKE_BOOL(TRUE),TW_MAKE_STRING("Hello World"))
	);

	for(index=2;index<totalRowsToCreate;index++){
		METRIC("csdk.performance.perfInfotableAccessAddRow.rowCount", index);
		twInfoTable_AddRow(testInfoTable,
				TW_IT_ROW(TW_MAKE_NUMBER(index),TW_MAKE_BOOL(TRUE),TW_MAKE_STRING("Hello World"))
		);
		{
			MARK_START("csdk.performance.perfInfotableAccessAddRow.time");
			json = twInfoTable_ToJson(testInfoTable);
			MARK_END("csdk.performance.perfInfotableAccessAddRow.time");
			TW_LOG(TW_TRACE, "%lu Duration = %lu \n",index,duration);
			cJSON_Delete(json);
			totalRunTimeMs=totalRunTimeMs+duration;
		}

	}
	twInfoTable_Delete(testInfoTable);
	TW_LOG(TW_TRACE, "\nTotal Test Run Time = %lu \n",totalRunTimeMs);
	average = (double)totalRunTimeMs/totalRowsToCreate;
	TW_LOG(TW_TRACE, "Average Test Run Time = %f\n",average);
	TEST_ASSERT_TRUE_MESSAGE(average<40,"perfInfotableAccessAddRow Average time should be under 40ms.");

}
