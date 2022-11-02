#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"
#include "twThreads.h"

#define NUM_WORKER_THREADS 2
twThread *apiThreadServiceIntegration = NULL;
twThread *workerThreadsServiceIntegration[NUM_WORKER_THREADS] = {NULL};
char cJSON_Delete_Hit = FALSE;
char CSDK_911_callback_hit = FALSE;

#define SERVICE_INTEGRATION_THINGNAME "unityTestThing"

enum msgCodeEnum
testService(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
            void *userdata) {
    TW_LOG(TW_FORCE, "Test_EDGE_1647 service invoked", entityName);
    return TWX_SUCCESS;
}

void * global_user_data = FALSE;
enum msgCodeEnum
testUserData(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content,
            void *userdata) {
    TW_LOG(TW_FORCE, "Test_EDGE_1647 service invoked", entityName);
	global_user_data = userdata;
    return TWX_SUCCESS;
}

void cJSON_Delete_Confirm(cJSON *c) {
    cJSON_Delete_Hit = TRUE;
    cJSON_Delete(c);
}

TEST_GROUP(ServiceIntegration);

void test_ServiceIntegrationAppKey_callback(char *passWd, unsigned int len){
    strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(ServiceIntegration) {
    int i = 0;
    CSDK_911_callback_hit = FALSE;
    eatLogs();

    /* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;

    TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_ServiceIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                              MESSAGE_CHUNK_SIZE, TRUE));
    twApi_RegisterService(TW_THING, SERVICE_INTEGRATION_THINGNAME, "Test_EDGE_1647", NULL, NULL, TW_NOTHING, NULL, testService, NULL);
    twApi_SetSelfSignedOk();
    twApi_DisableCertValidation();
    twApi_BindThing(SERVICE_INTEGRATION_THINGNAME);
    TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
    TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Things_unityTestThing.xml"));
    TEST_ASSERT_TRUE(twApi_IsEntityBound(SERVICE_INTEGRATION_THINGNAME));
    apiThreadServiceIntegration = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        workerThreadsServiceIntegration[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
    }
}

TEST_TEAR_DOWN(ServiceIntegration) {
    int i = 0;
    twStubs_Reset();
    for (i = 0; i < NUM_WORKER_THREADS; i++) {
        twThread_Delete(workerThreadsServiceIntegration[i]);
    }
    twThread_Delete(apiThreadServiceIntegration);
    TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("ServiceIntegrationTests Complete"));
    TEST_ASSERT_FALSE(twApi_isConnected());
    twApi_Delete();
}

TEST_GROUP_RUNNER(ServiceIntegration) {

#ifdef ENABLE_IGNORED_TESTS
    RUN_TEST_CASE(ServiceIntegration, EDGE_1647);
#endif
    RUN_TEST_CASE(ServiceIntegration, TestUserData);
    RUN_TEST_CASE(ServiceIntegration, CSDK_911)
}

int CSDK_911_cb(uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content) {
    CSDK_911_callback_hit = TRUE;
}


TEST(ServiceIntegration, CSDK_911) {
    uint32_t messageId = -1;


    TEST_ASSERT_FALSE(CSDK_911_callback_hit);
    TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeServiceAsync(TW_THING, SERVICE_INTEGRATION_THINGNAME, "Test_CSDK_911", NULL, FALSE, CSDK_911_cb, &messageId));
    twSleepMsec(1000);
    TEST_ASSERT_TRUE(CSDK_911_callback_hit);
}

/**
 * EDGE-1647
 *
 * Description: If a service is defined without an output shape (returns nothing), then there is a possible memory leak
 * when creating a json object to store aspect on twApi:649 aspects = cJSON_CreateObject(); when the output datashape
 * exists, the associated aspec memory will be cleaned up, but when the shape does not exist, the aspect json object is
 * created, never populated, and never cleaned up.
 *
 * Fix: Moved if (aspects) cJSON_Delete(aspects); outside if (out && aspects) conditional statement so that the aspects
 * cJSON object is always cleaned up.
 *
 * Test Plan: Register a service with no output datashape and invoke the GetRemoteMetadata service which will in turn
 * invoke GetMetadata() and produce the leak. Confirm cJSON_Delete() is hit by stubbing it out to set the
 * cJSON_Delete_Hit flag.
 */
TEST(ServiceIntegration, EDGE_1647) {
    twInfoTable *result = NULL;
	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("EDGE-1647");
	}
    if (twStubs_Use()) return;

    TEST_ASSERT_TRUE(doesServerEntityExist("Thing", SERVICE_INTEGRATION_THINGNAME));

    twApi_stub->cJSON_Delete = cJSON_Delete_Confirm;
    TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, SERVICE_INTEGRATION_THINGNAME, "Test_EDGE_1647", NULL, &result, 5000, FALSE));
    
    /* delete result before making next request */
    if (result) twInfoTable_Delete(result);
    result = NULL;

    cJSON_Delete_Hit = FALSE;
    TEST_ASSERT_FALSE(cJSON_Delete_Hit);
    TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, SERVICE_INTEGRATION_THINGNAME, "GetRemoteMetadata", NULL, &result, 5000, FALSE));
    TEST_ASSERT_TRUE(cJSON_Delete_Hit);

    /* cleanup result if necessary */
    if (result) twInfoTable_Delete(result);
}


/**
 * TestUserData
 *
 * Description: Make sure userdata is being properly transferred to callbacks
 *
 */
TEST(ServiceIntegration, TestUserData) {
    twInfoTable *result = NULL;
	char * data = NULL;

    if (twStubs_Use()) return;
    TEST_ASSERT_TRUE(doesServerEntityExist("Thing", SERVICE_INTEGRATION_THINGNAME));
	data = duplicateString("stuff");
	TEST_ASSERT_EQUAL(TW_OK,twApi_RegisterService(TW_THING, SERVICE_INTEGRATION_THINGNAME, "testUserData", NULL, NULL, TW_NOTHING, NULL, testUserData, (void *) data));
    TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, SERVICE_INTEGRATION_THINGNAME, "testUserData", NULL, &result, 5000, FALSE));
    TEST_ASSERT_TRUE(global_user_data);
    /* delete result before making next request */
    if (result) twInfoTable_Delete(result);
    result = NULL;

	/* cleanup data */
	if (data) TW_FREE(data);
}
