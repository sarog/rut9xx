#include <twBaseTypes.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
extern char* programName;

TEST_GROUP(TestUtilitiesIntegration);

void test_TestUtilitiesIntegrationAppKey_callback(char *passWd, unsigned int len){
    strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(TestUtilitiesIntegration) {
    eatLogs();
	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;
}

TEST_TEAR_DOWN(TestUtilitiesIntegration) {
}

TEST_GROUP_RUNNER(TestUtilitiesIntegration) {
//    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_twGetPreferedExtensionLoadingDirectory);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_twGetParentDirectory);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_CreateAndDestroyThing);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_CreateAndDestroyProperty);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_ImportEntity);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_CreateUniqueThing);
    RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_CreateServerTestThing);
	RUN_TEST_CASE(TestUtilitiesIntegration, testUtils_ImportUniqueEntity);
}

//TEST(TestUtilitiesIntegration, testUtils_twGetPreferedExtensionLoadingDirectory) {
//    const char* case1="C:\\parent\\examples\\child\\";
//    const char* case2="/parent/examples/child";
//    const char* case3="C:\\parent\\duuuurr\\child\\examples";
//	const char* case4="c:\\parent";
//    char * tempPName = programName;
//
//    programName = case1;
//    TEST_ASSERT_EQUAL_STRING("C:\\parent\\examples/ExtUseExample/ext",twGetPreferedExtensionLoadingDirectory());
//
//    programName = case2;
//    TEST_ASSERT_EQUAL_STRING("/parent/examples/ExtUseExample/ext",twGetPreferedExtensionLoadingDirectory());
//
//	programName = case3;
//	TEST_ASSERT_EQUAL_STRING("/parent/examples/ExtUseExample/ext",twGetPreferedExtensionLoadingDirectory());
//
//
//    programName = case3;
//    TEST_ASSERT_NULL(twGetPreferedExtensionLoadingDirectory());
//
//    programName = tempPName;
//
//
//
//}

TEST(TestUtilitiesIntegration, testUtils_twGetParentDirectory) {
    const char* case1="C:\\parent\\child\\";
    const char* case2="C:\\parent\\child";
    const char* case3="C:\\";
    const char* case4="C:";
    const char* case5="/home/parent/child";
    const char* case6="/home/parent/child/";
    const char* case7="/home";
    char* result;

    result = twGetParentDirectory(case1);
    TEST_ASSERT_EQUAL(0,strcmp("C:\\parent",result));
    TW_FREE(result);

    result = twGetParentDirectory(case2);
    TEST_ASSERT_EQUAL(0,strcmp("C:\\parent",result));
    TW_FREE(result);

    result = twGetParentDirectory(case3);
    TEST_ASSERT_NULL(result);

    result = twGetParentDirectory(case4);
    TEST_ASSERT_NULL(result);

    result = twGetParentDirectory(case5);
    TEST_ASSERT_EQUAL(0,strcmp("/home/parent",result));
    TW_FREE(result);

    result = twGetParentDirectory(case6);
    TEST_ASSERT_EQUAL(0,strcmp("/home/parent",result));
    TW_FREE(result);

    result = twGetParentDirectory(case7);
    TEST_ASSERT_NULL(result);

}

/**
 * Test Plan: Create A thing, verify it exists and then delete it again and verify that it does not exist.
 */
TEST(TestUtilitiesIntegration, testUtils_CreateAndDestroyThing) {

    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));

    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "testUtils_CreateAndDestroyThing1"));
    TEST_ASSERT_TRUE(createServerThing("testUtils_CreateAndDestroyThing1", "RemoteThing"));
    TEST_ASSERT_TRUE(doesServerEntityExist("Thing","testUtils_CreateAndDestroyThing1"));
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "testUtils_CreateAndDestroyThing1"));
    TEST_ASSERT_FALSE(doesServerEntityExist("Thing","testUtils_CreateAndDestroyThing1"));

    twApi_Disconnect("testing");
    twApi_Delete();
}

TEST(TestUtilitiesIntegration, testUtils_CreateAndDestroyProperty) {

    char *thingName = "testUtils_CreateAndDestroyPropertyThing";
    char *propertyName = "testUtils_CreateAndDestroyPropertyProperty";

    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));
    TEST_ASSERT_TRUE(createServerThing(thingName, "RemoteThing"));
    if(doesServerThingPropertyExist(TW_THING, thingName, "String", propertyName)){
        removePropertyFromServerEntity(TW_THING,thingName,propertyName);
    }

    TEST_ASSERT_TRUE(addPropertyDefinitionToServerThing(thingName,propertyName,"STRING",FALSE,TRUE,
                                                        propertyName,5000,"ALWAYS",0.0,TRUE,
                                                        0.0,"ALWAYS","Test",FALSE,NULL,
                                                        twPrimitive_CreateFromString("Default",TRUE), "A Test Property"));

    TEST_ASSERT_TRUE(restartServerThing(TW_THING, thingName));
    TEST_ASSERT_TRUE(doesServerThingPropertyExist(TW_THING, thingName, "String", propertyName));
    TEST_ASSERT_TRUE(removePropertyFromServerEntity(TW_THING,thingName,propertyName));
    TEST_ASSERT_TRUE(restartServerThing(TW_THING, thingName));
    TEST_ASSERT_FALSE(doesServerThingPropertyExist(TW_THING, thingName, "String", propertyName));
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, thingName));

    twApi_Disconnect("testing");
    twApi_Delete();

}

TEST(TestUtilitiesIntegration, testUtils_ImportEntity) {
    char *thingName = "testUtils_ImportEntityThing";
    char *importFileName = "Things_testUtils_ImportEntityThing.xml";

    /* Connect to verify */
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, thingName));
    TEST_ASSERT_TRUE(importEntityFileFromEtc(importFileName));
    TEST_ASSERT_TRUE(doesServerEntityExist("Thing",thingName));
    TEST_ASSERT_TRUE(doesServerThingPropertyExist(TW_THING, thingName, "String", "testProperty"));
    TEST_ASSERT_TRUE(deleteServerThing(TW_THING, thingName));

    twApi_Disconnect("testing");
    twApi_Delete();

}

TEST(TestUtilitiesIntegration, testUtils_CreateUniqueThing) {
    int i = 0;
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));
    for (i = 0; i < 10; i++) {
        char thingName[1024];
        char *id;
        id = twItoa(i);
        strcpy(thingName, "MyThing_");
        strcat(thingName, id);
        free(id);
        deleteServerThing(TW_THING, thingName);
    }
    for (i = 0; i < 10; i++) {
        char thingName[1024];
        char *createdThingName = NULL;
        char *id = NULL;
        id = twItoa(i);
        strcpy(thingName, "MyThing_");
        strcat(thingName, id);
        free(id);
        createdThingName = createUniqueServerThing("MyThing", "RemoteThingWithFileTransfer");
        TEST_ASSERT_EQUAL_STRING(thingName, createdThingName);
        free(createdThingName);
    }
    for (i = 0; i < 10; i++) {
        char thingName[1024] = "MyThing_";
        char *id = twItoa(i);
        strcpy(thingName, "MyThing_");
        strcat(thingName, id);
        free(id);
        deleteServerThing(TW_THING, thingName);
    }
    twApi_Disconnect("testing");
    twApi_Delete();
}

TEST(TestUtilitiesIntegration, testUtils_CreateServerTestThing) {
    twInfoTable *it = NULL;
    twInfoTable *result = NULL;
    twPrimitive *readValue = NULL;
    twThread *workerThread = NULL;
    char *thingName = NULL;
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
                                                  MESSAGE_CHUNK_SIZE, TRUE));
    twApi_DisableCertValidation();
    TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));
    thingName = createServerTestThing();
    twApi_BindThing(thingName);
    restartServerThing(TW_THING, thingName);
    workerThread = twThread_Create(twMessageHandler_msgHandlerTask, 10, NULL, TRUE);
    it = twInfoTable_CreateFromInteger("value", 1);
    twApi_InvokeService(TW_THING, thingName, "Push_Integer", it, &result, 5000, FALSE);
    twSleepMsec(500);
    twApi_ReadProperty(TW_THING, thingName, "AlwaysPush_Integer", &readValue, 5000, FALSE);
    TEST_ASSERT_EQUAL(1, readValue->val.integer);
    deleteServerThing(TW_THING, thingName);
    twApi_Disconnect("testing");
    twThread_Delete(workerThread);
    twInfoTable_Delete(it);
    twInfoTable_Delete(result);
    twPrimitive_Delete(readValue);
    twApi_Delete();
    free(thingName);
}

TEST(TestUtilitiesIntegration, testUtils_ImportUniqueEntity) {
	char *thingName = NULL;
	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_TestUtilitiesIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                              MESSAGE_CHUNK_SIZE, TRUE));
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL_INT(TW_OK, twApi_Connect(10000, 3));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_0"));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_1"));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_2"));
	thingName = importAndCloneUniqueTestEntity("Things_SimpleTestThing.xml", "SimpleTestThing");
	TEST_ASSERT_EQUAL_STRING(thingName, "SimpleTestThing_0");
	TW_FREE(thingName);
	thingName = importAndCloneUniqueTestEntity("Things_SimpleTestThing.xml", "SimpleTestThing");
	TEST_ASSERT_EQUAL_STRING(thingName, "SimpleTestThing_1");
	TW_FREE(thingName);
	thingName = importAndCloneUniqueTestEntity("Things_SimpleTestThing.xml", "SimpleTestThing");
	TEST_ASSERT_EQUAL_STRING(thingName, "SimpleTestThing_2");
	TW_FREE(thingName);
	TEST_ASSERT_TRUE(doesServerEntityExist("Thing", "SimpleTestThing_0"));
	TEST_ASSERT_TRUE(doesServerEntityExist("Thing", "SimpleTestThing_1"));
	TEST_ASSERT_TRUE(doesServerEntityExist("Thing", "SimpleTestThing_2"));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_0"));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_1"));
	TEST_ASSERT_TRUE(deleteServerThing(TW_THING, "SimpleTestThing_2"));
}