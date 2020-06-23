#include "twBaseTypes.h"
#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"

/* Test specific #defines */
#define TEST_STRING_LENGTH 8
#define NUM_TEST_PROPERTIES 100 /* Should only be set to 1,000 or 10,000 */
#define PPT_TEST_THINGNAME "PropertyPerformanceTestThing"
#define NUM_WORKERS 5
#define PUSH_N_SUBSCRIBED_PROPERTIES_BASELINE 5000
#define PUSH_N_SUBSCRIBED_PROPERTTES_ROUND_TRIP_BASELINE 1250000
#define PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_BASELINE 4500
#define PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_ROUND_TRIP_BASELINE 1150000
#define TEST_N_PROPERTY_LOOKUPS 30 /* seconds */

/* Declare api pointer */
#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

/* Test specific globals */
typedef struct test_data {
	char** values;
	TW_MUTEX mtx;
} test_data;

test_data * performancePropertyNames = NULL;
test_data * performancePropertyData = NULL;

/* Utility functions */
char *getPerformanceTestPropertyName(int i) {
    char *propertyName = NULL;
    char *propertyId = NULL;
    propertyId = twItoa(i);
    propertyName = (char*)malloc(sizeof(char) * (strlen("Performance_Test_Property_") + strlen(propertyId) + 1));
    strcpy(propertyName, "Performance_Test_Property_");
    strcat(propertyName, propertyId);
    free(propertyId);
    return propertyName;
}

char **getPerformanceTestPropertyNames(int numProperties) {
    int i = 0;
    char **propertyNames = NULL;
    srand(time(NULL));
    propertyNames = (char**)TW_MALLOC(sizeof(char*) * numProperties);
    for (i = 0; i < numProperties; i++) {
        propertyNames[i] = getPerformanceTestPropertyName(i+1);
    }
    return propertyNames;
}

char **getPerformanceTestPropertyData(int numProperties, size_t stringLength) {
    int i = 0;
    char **propertyData = NULL;
    propertyData = (char**)TW_MALLOC(sizeof(char*) * numProperties);
    for (i = 0; i < numProperties; i++) {
        propertyData[i] = generateRandomString(stringLength);
    }
    return propertyData;
}

char ** registerPerformanceTestProperties(char *thingName, int numProperties) {
    int i = 0;
    char **propertyNames = getPerformanceTestPropertyNames(numProperties);
    for (i = 0; i < numProperties; i++) {
		TEST_ASSERT_EQUAL(TW_OK, twApi_RegisterProperty(TW_THING, thingName, propertyNames[i], TW_STRING, NULL, "ALWAYS", 0, (property_cb)doNothing, NULL));
	}
    return propertyNames;
}


int createPerformanceTestPropertyNames() {
	int res = TW_OK;
	if (NULL == performancePropertyNames) {
		/* create memory */
		performancePropertyNames = (test_data *)TW_MALLOC(sizeof(test_data));
		if (performancePropertyNames) {
			/* create mutex */
			performancePropertyNames->mtx = twMutex_Create();
			if (!performancePropertyNames->mtx) {
				res = TW_ERROR_ALLOCATING_MEMORY;
			} else {
				/* lock mutex and create values */
				twMutex_Lock(performancePropertyNames->mtx);
				performancePropertyNames->values = NULL;
				performancePropertyNames->values = registerPerformanceTestProperties(PPT_TEST_THINGNAME, NUM_TEST_PROPERTIES);
				twMutex_Unlock(performancePropertyNames->mtx);
			}
		} else {
			res = TW_ERROR_ALLOCATING_MEMORY;
		}

	} else {
		/* names already created */
		res = TW_ERROR_ITEM_EXISTS;
	}
	return res;
}

int createPerformanceTestPropertyData() {
	int res = TW_OK;
	if (NULL == performancePropertyData) {
		/* create memory */
		performancePropertyData = (test_data *)TW_MALLOC(sizeof(test_data));
		if (performancePropertyData) {
			/* create mutex */
			performancePropertyData->mtx = twMutex_Create();
			if (!performancePropertyData->mtx) {
				res = TW_ERROR_ALLOCATING_MEMORY;
			} else {
				/* lock mutex and create values */
				twMutex_Lock(performancePropertyData->mtx);
				performancePropertyData->values = NULL;
				performancePropertyData->values = getPerformanceTestPropertyData(NUM_TEST_PROPERTIES, TEST_STRING_LENGTH);
				twMutex_Unlock(performancePropertyData->mtx);
			}
		} else {
			res = TW_ERROR_ALLOCATING_MEMORY;
		}
	} else {
		/* names already created */
		res = TW_ERROR_ITEM_EXISTS;
	}
	return res;
}

int deletePerformanceTestPropertyNames() {
    int i = 0;
	int res = TW_UNKNOWN_ERROR;

	if (performancePropertyNames && performancePropertyNames->mtx) {
		twMutex_Lock(performancePropertyNames->mtx);
		if (performancePropertyData->values) {
			for (i = 0; i < NUM_TEST_PROPERTIES; i++) {
				TW_FREE(performancePropertyNames->values[i]);
			}
			TW_FREE(performancePropertyNames->values);
			performancePropertyNames->values = NULL;
		}
		twMutex_Delete(performancePropertyNames->mtx);
		TW_FREE(performancePropertyNames);
		performancePropertyNames = NULL;
		res = TW_OK;
	}

	return res;
}

int deletePerformanceTestPropertyData() {
    int i = 0;
	int res = TW_UNKNOWN_ERROR;

	if (performancePropertyData && performancePropertyData->mtx) {
		twMutex_Lock(performancePropertyData->mtx);
		if(performancePropertyData->values) {
			for (i = 0; i < NUM_TEST_PROPERTIES; i++) {
				TW_FREE(performancePropertyData->values[i]);
			}
			TW_FREE(performancePropertyData->values);
			performancePropertyData->values = NULL;
		}
		twMutex_Delete(performancePropertyData->mtx);
		performancePropertyData->mtx = NULL;
		TW_FREE(performancePropertyData);
		performancePropertyData = NULL;
		res = TW_OK;
	}

	return res;
}

int writeNProperties(char *thingName, int num_test_properties, char push) {
	int i = 0;
	int res = TW_ERROR_BAD_OPTION;
	twPrimitive *value = NULL;
	if (performancePropertyNames && performancePropertyNames->values && performancePropertyNames->mtx && performancePropertyData && performancePropertyData->values && performancePropertyData->mtx) {
		twMutex_Lock(performancePropertyNames->mtx);
		twMutex_Lock(performancePropertyData->mtx);

		for (i = 0; i < num_test_properties; i++) {
			value = twPrimitive_CreateFromString(performancePropertyData->values[i], TRUE);
			TEST_ASSERT_EQUAL_MESSAGE(TW_OK, twApi_SetSubscribedProperty(thingName, performancePropertyNames->values[i], twPrimitive_FullCopy(value), FALSE, push), performancePropertyNames->values[i]);
			twPrimitive_Delete(value);
		}
		twMutex_Unlock(performancePropertyData->mtx);
		twMutex_Unlock(performancePropertyNames->mtx);
		res = TW_OK;
	}
	return res;
}

int readNProperties(char *thingName, int num_test_properties) {
    int i = 0;
	int res = TW_ERROR_BAD_OPTION;
    twPrimitive *result = NULL;
	char *data = NULL;
	if (performancePropertyNames && performancePropertyNames->values && performancePropertyNames->mtx && performancePropertyData && performancePropertyData->values && performancePropertyData->mtx) {
		twMutex_Lock(performancePropertyNames->mtx);
		twMutex_Lock(performancePropertyData->mtx);
		for (i = 0; i < num_test_properties; i++) {
			twApi_ReadProperty(TW_THING, thingName, performancePropertyNames->values[i], &result, -1, FALSE);
			data = twPrimitive_DecoupleStringAndDelete(result);
			TEST_ASSERT_EQUAL_STRING(performancePropertyData->values[i], data);
			free(data);
		}
		twMutex_Unlock(performancePropertyData->mtx);
		twMutex_Unlock(performancePropertyNames->mtx);
		res = TW_OK;
	}
	return res;
}

void writePropertyNTimes(char *thingName, int num_test_properties, char push) {
    int i = 0;
    DATETIME now = 0;
	int res = TW_ERROR_BAD_OPTION;
    now = (DATETIME)getTimestamp();
	if (performancePropertyData && performancePropertyData->values && performancePropertyData->mtx && performancePropertyNames && performancePropertyNames->values && performancePropertyNames->mtx) {
		twMutex_Lock(performancePropertyData->mtx);
		for (i = 0; i < num_test_properties; i++) {
			now++;
			twApi_SetSubscribedPropertyVTQ(thingName, performancePropertyNames->values[i], twPrimitive_CreateFromString(performancePropertyData->values[i], TRUE), now, "GOOD", FALSE, FALSE);
		}
		twMutex_Unlock(performancePropertyData->mtx);
		res = TW_OK;
	}
}

void importPropertyPerformanceTestThing() {
    if (doesServerEntityExist("Thing", PPT_TEST_THINGNAME)) {
        deleteServerThing(TW_THING, PPT_TEST_THINGNAME);
    }
    TEST_ASSERT_FALSE(doesServerEntityExist("Thing", PPT_TEST_THINGNAME));
    if (NUM_TEST_PROPERTIES <= 100) {
		importEntityFileFromEtc("Things_PropertyPerformanceTestThing_100_properties.xml");
	} else if (NUM_TEST_PROPERTIES <= 10000) {
		importEntityFileFromEtc("Things_PropertyPerformanceTestThing_1000_properties.xml");
	} else if (NUM_TEST_PROPERTIES <= 1000) {
		importEntityFileFromEtc("Things_PropertyPerformanceTestThing_100000_properties.xml");
	}

    TEST_ASSERT_TRUE(doesServerEntityExist("Thing", PPT_TEST_THINGNAME));
    restartServerThing(TW_THING, PPT_TEST_THINGNAME);
}

/* property lookup TEST helper functions */
/**
* start_write_benchmark
*
* helper funciton for test_n_property_lookups
*
* starts grafana metric to track total property write time
* should be triggered immediately before invoking the platform
* service to execute property writes
*
**/
int start_write_benchmark() {
  int err = 0;
  twInfoTable *result = NULL;

  /* execute service to trigger property writes */
  err = twApi_InvokeService(TW_THING, PPT_TEST_THINGNAME, "Update_All_Props", NULL, &result, -1, FALSE);

  /* cleanup result */
  twInfoTable_Delete(result);

  /* return status */
  return err;
}

/* global boolean used to communicate end of test */
char end = FALSE;

/**
* end_write_benchmark_handler
*
* helper funciton for test_n_property_lookups
*
* ends grafana metric to track total property write time
* should be executed in a service callback that is ececuted by a
* platform subscription
*
**/
enum msgCodeEnum end_write_benchmark_handler(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	TW_LOG(TW_TRACE,"end_write_benchmark - Function called");

	end = TRUE;
	return TWX_SUCCESS;
}

/* Stubs */
enum msgCodeEnum fake_sendMessageBlocking(twMessage * msg, int32_t timeout, twInfoTable ** result) {
    return TWX_SUCCESS;
}


/** subscribed property TEST helper functions **/

/** helper function for push_n_subscribed_property_n_times related tests **/
/** this will push n properties 1 time OR 1 property n times **/
long push_subscribed_properties_fn(const char * grafana_metric, const char * grafana_mark, char n_props_1_time, char round_trip) {
	long totalDuration = 0;
    int i = 0;
	if (round_trip) {
		twStubs_Use();
		twApi_stub->sendMessageBlocking = fake_sendMessageBlocking;
	}
    for (i = 0; i < NUM_TEST_PROPERTIES; i++) {
        if (n_props_1_time) {
			writeNProperties(PPT_TEST_THINGNAME, i, FALSE);
		} else {
			writePropertyNTimes(PPT_TEST_THINGNAME, i, FALSE);
		}
        METRIC(grafana_metric, i);
        {
            MARK_START(grafana_mark);
			twSubscribedPropsMgr_PushSubscribedProperties(PPT_TEST_THINGNAME, FALSE, TRUE);
            MARK_END(grafana_mark);
            totalDuration += duration;
        }
    }
    printf("total duration for %s: %d\n", grafana_mark, totalDuration);
	return totalDuration;
}

/** helper function for push_n_subscribed_property_n_times related tests **/
/** this will push n properties 1 time OR 1 property n times **/
long push_subscribed_property_fn(const char * grafana_metric, const char * grafana_mark, char n_props_1_time) {
	return push_subscribed_properties_fn(grafana_metric, grafana_mark, n_props_1_time, FALSE);
}

/** helper function for push_n_subscribed_property_n_times_round_trip_fn related tests */
/** this will round trip push n properties 1 time OR 1 property n times **/
long push_subscribed_property_round_trip_fn(const char * grafana_metric, const char * grafana_mark, char n_props_1_time) {
	return push_subscribed_properties_fn(grafana_metric, grafana_mark, n_props_1_time, TRUE);
}

/** helper function for push_n_subscribed_properties related tests **/
long push_n_subscribed_properties_fn(const char * grafana_metric, const char * grafana_mark) {
	return push_subscribed_property_fn(grafana_metric, grafana_mark, TRUE);
}

/** helper function for push_1_subscribed_property_n_times related tests **/
long push_1_subscribed_property_n_times_fn(const char * grafana_metric, const char * grafana_mark) {
	return push_subscribed_property_fn(grafana_metric, grafana_mark, FALSE);
}

/** helper function for push_n_subscribed_properties_round_trip related tests **/
long push_n_subscribed_properties_round_trip_fn(const char * grafana_metric, const char * grafana_mark) {
	return push_subscribed_property_round_trip_fn(grafana_metric, grafana_mark, TRUE);
}

/** helper function for push_1_subscribed_property_n_times_round_trip related tests **/
long push_1_subscribed_property_n_times_round_trip_fn(const char * grafana_metric, const char * grafana_mark) {
	return push_subscribed_property_round_trip_fn(grafana_metric, grafana_mark, FALSE);
}

int test_n_property_lookups_fn () {
    int i = 0;
	int res = TW_OK;
    /* register end_write_benchmark_handler */
    res = twApi_RegisterService(TW_THING, PPT_TEST_THINGNAME, "end_write_benchmark", NULL, NULL, TW_NOTHING, NULL, end_write_benchmark_handler, NULL);

    /* start test */
    /* start metric */
    if (TW_OK == res) {
		MARK_START("csdk.performance.perfPropertyLookup.total_time");
		res = start_write_benchmark();

		/* spin here until test ends */
		for(i = 0; end != TRUE && i < TEST_N_PROPERTY_LOOKUPS; i++) {
			/* wait a second then do nothing */
			twSleepMsec(1000);
		}
		if (i >= TEST_N_PROPERTY_LOOKUPS) {
			printf("\nERROR: test_n_property_lookups_fn: timed out waiting for end\n");
			res = TW_UNKNOWN_ERROR;
		}

		/* stop metric */
		METRIC("csdk.performance.perfPropertyLookup.property_count",NUM_TEST_PROPERTIES);
		MARK_END("csdk.performance.perfPropertyLookup.total_time");
	}
	return res;
}


/** helper threads **/

void push_n_subscribed_properties_task(DATETIME now, void * params) {
	twTest_Data * td = (twTest_Data*)params;
	if (!td->isDone) {
		if(push_n_subscribed_properties_fn("csdk.performance.push_n_subscribed_properties.properties", "csdk.performance.push_n_subscribed_properties.time") < PUSH_N_SUBSCRIBED_PROPERTIES_BASELINE) {
			td->result = TW_OK;
		} else {
			td->result = TW_UNKNOWN_ERROR;
		}
		td->isDone = TRUE;
	} else {
		twSleepMsec(1000);
	}
}
void push_1_subscribed_property_n_times_task(DATETIME now, void * params) {
	twTest_Data * td = (twTest_Data*)params;
	if (!td->isDone) {
		if(push_n_subscribed_properties_round_trip_fn("csdk.performance.push_n_subscribed_properties_round_trip.properties", "csdk.performance.push_n_subscribed_properties_round_trip.time") < PUSH_N_SUBSCRIBED_PROPERTTES_ROUND_TRIP_BASELINE) {
			td->result = TW_OK;
		} else {
			td->result = TW_UNKNOWN_ERROR;
		}
		td->isDone = TRUE;
	} else {
		twSleepMsec(1000);
	}
}

void push_n_subscribed_properties_round_trip_task(DATETIME now, void * params) {
	twTest_Data * td = (twTest_Data*)params;
	if (!td->isDone) {
		if (push_1_subscribed_property_n_times_fn("csdk.performance.push_1_subscribed_property_n_times.properties", "csdk.performance.push_1_subscribed_property_n_times.time")  < PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_BASELINE) {
			td->result = TW_OK;
		} else {
			td->result = TW_UNKNOWN_ERROR;
		}
		td->isDone = TRUE;
	} else {
		twSleepMsec(1000);
	}
}

void push_1_subscribed_property_n_times_round_trip_task(DATETIME now, void * params) {
	twTest_Data * td = (twTest_Data*)params;
	if (!td->isDone) {
		if (push_1_subscribed_property_n_times_round_trip_fn("csdk.performance.push_1_subscribed_property_n_times_round_trip.properties", "csdk.performance.push_1_subscribed_property_n_times_round_trip.time") < PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_ROUND_TRIP_BASELINE) {
			td->result = TW_OK;
		} else {
			td->result = TW_UNKNOWN_ERROR;
		}
		td->isDone = TRUE;
	} else {
		twSleepMsec(1000);
	}
}

void test_n_property_lookups_task(DATETIME now, void * params) {
	twTest_Data * td = (twTest_Data*)params;
	if (!td->isDone) {
		td->result =  test_n_property_lookups_fn();
		td->isDone = TRUE;
	} else {
		twSleepMsec(1000);
	}
}

/**
 * Define the test group name
 */
TEST_GROUP(PropertyPerformance);

void test_PropertyPerformanceAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

/**
 * Setup macro run before each test
 */
TEST_SETUP(PropertyPerformance) {
	/* modify logging */
    eatLogs();

    /* Initialize and configure API */
    createTestApi(PPT_TEST_THINGNAME, TW_HOST, TW_PORT, TW_URI, test_PropertyPerformanceAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);

    /* Register our performance test properties */
	TEST_ASSERT_EQUAL(TW_OK, createPerformanceTestPropertyNames());

    /* Generate random performance test data */
	TEST_ASSERT_EQUAL(TW_OK, createPerformanceTestPropertyData());

    /* Connect */
    connectTestApi(PPT_TEST_THINGNAME);

    /* Set up server thing */
    importPropertyPerformanceTestThing();
}
/**
 * Tear down macro run after each test
 */
TEST_TEAR_DOWN(PropertyPerformance) {
    /* Unbind and disconnect */
    disconnectTestApi(PPT_TEST_THINGNAME, "PropertyPerformance");

	/* delay 100ms to allow all worker threads to cleanup if they were previously doing something */
	twSleepMsec(100);

    /* Clean up */
    TEST_ASSERT_EQUAL(TW_OK, deletePerformanceTestPropertyNames());
    TEST_ASSERT_EQUAL(TW_OK, deletePerformanceTestPropertyData());
    deleteTestApi();
}
/**
 * Runs specified test cases for this group
 */
TEST_GROUP_RUNNER(PropertyPerformance) {
    RUN_TEST_CASE(PropertyPerformance, push_n_subscribed_properties);
    RUN_TEST_CASE(PropertyPerformance, push_n_subscribed_properties_round_trip);
    RUN_TEST_CASE(PropertyPerformance, push_1_subscribed_property_n_times);
    RUN_TEST_CASE(PropertyPerformance, push_1_subscribed_property_n_times_round_trip);
    RUN_TEST_CASE(PropertyPerformance, test_n_property_lookups);
    RUN_TEST_CASE(PropertyPerformance, test_n_property_lookups_and_pushes);
}

/**
 * push_n_subscribed_properties
 *
 * Pushes strings of size TEST_STRING_LENGTH to 1 to NUM_TEST_PROPERTIES subscribed properties via
 * twSubscribedPropsMgr_PushSubscribedProperties(). sendMessageBlocking() is stubbed out to instantly return TWX_SUCCESS
 * in order to only measure the time it takes to marshall the message.
 */
TEST(PropertyPerformance, push_n_subscribed_properties) {
    TEST_ASSERT_TRUE(push_n_subscribed_properties_fn("csdk.performance.push_n_subscribed_properties.properties", "csdk.performance.push_n_subscribed_properties.time") < PUSH_N_SUBSCRIBED_PROPERTIES_BASELINE);
}
/**
 * push_n_subscribed_properties_round_trip
 *
 * Pushes strings of size TEST_STRING_LENGTH to 1 to NUM_TEST_PROPERTIES subscribed properties via
 * twSubscribedPropsMgr_PushSubscribedProperties()
 */
TEST(PropertyPerformance, push_n_subscribed_properties_round_trip) {
    TEST_ASSERT_TRUE(push_n_subscribed_properties_round_trip_fn("csdk.performance.push_n_subscribed_properties_round_trip.properties", "csdk.performance.push_n_subscribed_properties_round_trip.time") < PUSH_N_SUBSCRIBED_PROPERTTES_ROUND_TRIP_BASELINE);
}

/**
 * push_1_subscribed_property_n_times
 *
 * Pushes strings of size TEST_STRING_LENGTH to 1 to NUM_TEST_PROPERTIES subscribed properties via
 * twSubscribedPropsMgr_PushSubscribedProperties()
 */
TEST(PropertyPerformance, push_1_subscribed_property_n_times) {
    TEST_ASSERT_TRUE(push_1_subscribed_property_n_times_fn("csdk.performance.push_1_subscribed_property_n_times.properties", "csdk.performance.push_1_subscribed_property_n_times.time")  < PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_BASELINE);
}

/**
 * push_1_subscribed_property_n_times_round_trip
 *
 * Pushes strings of size TEST_STRING_LENGTH to 1 to NUM_TEST_PROPERTIES subscribed properties via
 * twSubscribedPropsMgr_PushSubscribedProperties()
 */
TEST(PropertyPerformance, push_1_subscribed_property_n_times_round_trip) {
    TEST_ASSERT_TRUE(push_1_subscribed_property_n_times_round_trip_fn("csdk.performance.push_1_subscribed_property_n_times_round_trip.properties", "csdk.performance.push_1_subscribed_property_n_times_round_trip.time") < PUSH_1_SUBSCRIBED_PROPERTY_N_TIMES_ROUND_TRIP_BASELINE);
}


/* global count to keep track of properties looked up */
/*int prop_index = 0;*/
/**
* always_write_success_handler
*
* helper handler for test_n_property_lookups
*
* returns success any time a property is written to
*
**/
/*
enum msgCodeEnum always_write_success_handler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata) {
	char * asterisk = "*";
	if (!propertyName) propertyName = asterisk;
	TW_LOG(TW_TRACE,"always_write_success_handler - Function called for Entity %s, Property %s", entityName, propertyName);
  prop_index++;
  METRIC("csdk.performance.perfPropertyLookup.property_count",prop_index);
	if (value) {
		if (isWrite && *value) {*/
			/* Property Writes *//*
			return TWX_SUCCESS;
		} else {*/
			/* Property Reads *//*
      TW_LOG(TW_ERROR,"always_write_success_handler - cannot handle property reads");
    	return TWX_BAD_REQUEST;
		}
		return TWX_SUCCESS;
	} else {
		TW_LOG(TW_ERROR,"propertyHandler - NULL pointer for value");
		return TWX_BAD_REQUEST;
	}
}
*/
/**  **/


/**
* test_n_property_lookups
*
* test to measure the performance of n property lookups
* the lookups will be triggered from property writes delivered
* by the platform
*
**/
TEST(PropertyPerformance, test_n_property_lookups) {
	test_n_property_lookups_fn();
}

/**
* test_n_property_lookups_and_pushes
*
* test to measure the performance of n property lookups
* the lookups will be triggered from property writes delivered
* by the platform
*
**/
TEST(PropertyPerformance, test_n_property_lookups_and_pushes) {
	/* declare threads */
	twThread * tmp1 = NULL;
	twThread * tmp2 = NULL;
	twThread * tmp3 = NULL;
	twThread * tmp4 = NULL;
	twThread * tmp5 = NULL;

	/* declare structs */
	twTest_Data * td1 = NULL;
	twTest_Data * td2 = NULL;
	twTest_Data * td3 = NULL;
	twTest_Data * td4 = NULL;
	twTest_Data * td5 = NULL;

	char is_test_done_1 = FALSE;
	char is_test_done_2 = FALSE;
	char is_test_done_3 = FALSE;
	char is_test_done_4 = FALSE;
	char is_test_done_5 = FALSE;

	/* init thread data */
	td1 = twTest_CreateData();
	td2 = twTest_CreateData();
	td3 = twTest_CreateData();
	td4 = twTest_CreateData();
	td5 = twTest_CreateData();

    /* start property lookup thread */
	tmp1 = twThread_Create(test_n_property_lookups_task, 5, td1, TRUE);
	TEST_ASSERT_TRUE(tmp1);

	/* start property push threads */
	tmp2 = twThread_Create(push_n_subscribed_properties_task, 5, td2, TRUE);
	TEST_ASSERT_TRUE(tmp2);
	tmp3 = twThread_Create(push_1_subscribed_property_n_times_task, 5, td3, TRUE);
	TEST_ASSERT_TRUE(tmp3);
	tmp4 = twThread_Create(push_n_subscribed_properties_round_trip_task, 5, td4, TRUE);
	TEST_ASSERT_TRUE(tmp4);
	tmp5 = twThread_Create(push_1_subscribed_property_n_times_round_trip_task,5, td5, TRUE);
	TEST_ASSERT_TRUE(tmp5);

	/* allow for new threads to start if necessary */
	twSleepMsec(100);

	/* join threads */
	while(!is_test_done_1 || !is_test_done_2 || !is_test_done_3 || !is_test_done_4 || !is_test_done_5) {
		/* delete finished threads */
		if (td1->isDone && twThread_IsRunning(tmp1)) {
			twThread_Delete(tmp1);
			is_test_done_1 = TRUE;
			tmp1 = NULL;
		}
		if (td2->isDone && twThread_IsRunning(tmp2)) {
			twThread_Delete(tmp2);
			is_test_done_2 = TRUE;
			tmp2 = NULL;
		}
		if (td3->isDone && twThread_IsRunning(tmp3)) {
			twThread_Delete(tmp3);
			is_test_done_3 = TRUE;
			tmp3 = NULL;
		}
		if (td4->isDone && twThread_IsRunning(tmp4)) {
			twThread_Delete(tmp4);
			is_test_done_4 = TRUE;
			tmp4 = NULL;
		}
		if (td5->isDone && twThread_IsRunning(tmp5)) {
			twThread_Delete(tmp5);
			is_test_done_5 = TRUE;
			tmp5 = NULL;
		}

		/* sleep 100ms then check again */
		twSleepMsec(100);
	}
	/* check test statuses */
	TEST_ASSERT_EQUAL(TW_OK,td1->result);
	TEST_ASSERT_EQUAL(TW_OK,td2->result);
	TEST_ASSERT_EQUAL(TW_OK,td3->result);
	TEST_ASSERT_EQUAL(TW_OK,td4->result);
	TEST_ASSERT_EQUAL(TW_OK,td5->result);

	twTest_DeleteData(td1);
	twTest_DeleteData(td2);
	twTest_DeleteData(td3);
	twTest_DeleteData(td4);
	twTest_DeleteData(td5);
}

