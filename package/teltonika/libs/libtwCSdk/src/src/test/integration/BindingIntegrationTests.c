#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "twProperties.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twExt.h"
#include "twMacros.h"

#define BIND_TEST_NAME_SIZE 10

TEST_GROUP(BindingIntegration);

void test_BindingIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(BindingIntegration){
    eatLogs();
	
	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;

    /* reset to the old default max message size */
    twcfg_pointer->max_message_size = 16384;
}

TEST_TEAR_DOWN(BindingIntegration){
}

TEST_GROUP_RUNNER(BindingIntegration) {
    RUN_TEST_CASE(BindingIntegration, test_BindThingBeforeConnect);
    RUN_TEST_CASE(BindingIntegration, test_BindThingAfterConnect);
    RUN_TEST_CASE(BindingIntegration, test_BindThingsBeforeConnect);
    RUN_TEST_CASE(BindingIntegration, test_BindThingsAfterConnect);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsBeforeConnectDefaultMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsAfterConnectDefaultMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsBeforeConnect8kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsAfterConnect8kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsBeforeConnect10kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsAfterConnect10kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsBeforeConnect16kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, test_BindMassThingsAfterConnect16kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegration, CSDK_1101);
}

enum msgCodeEnum sendMessageBlocking(twMessage ** msg, int32_t timeout, twInfoTable ** result);
enum msgCodeEnum sendMessageBlockingFailOnTimeout(twMessage ** msg, int32_t timeout, twInfoTable ** result) {
	enum msgCodeEnum res;
	res = sendMessageBlocking(msg, timeout, result);
	TEST_ASSERT_NOT_EQUAL_MESSAGE(TWX_GATEWAY_TIMEOUT, res, "A message timed out!");
	return res;
}

TEST(BindingIntegration, CSDK_1101) {
	int i = 0;
	int j = 0;

	twcfg_pointer->max_message_size = 16384 * 8;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
	twStubs_Use();
	twApi_stub->sendMessageBlocking = sendMessageBlockingFailOnTimeout;
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	importEntityFileFromEtc("Entities_CSDK_1101.xml");
	twApi_Disconnect("BindingIntegration: CSDK_1101");
	TEST_ASSERT_FALSE(twApi_isConnected());


	for (i = 0; i < 10; i++) {
		char *thingName = NULL;
		thingName = TW_MALLOC(sizeof(char) * 100);
		strcpy(thingName, "CSDK_1101_Thing_");
		concatenateStrings(&thingName, twItoa(i));
		{
			TW_MAKE_THING(thingName, TW_THING_TEMPLATE_GENERIC);
			for (j = 0; j < 250; j++) {
				char *propertyName = NULL;
				propertyName = TW_MALLOC(sizeof(char) * 100);
				strcpy(propertyName, "Property_");
				concatenateStrings(&propertyName, twItoa(j));
				TW_PROPERTY(propertyName, "This is a description.", TW_NUMBER);
				TW_FREE(propertyName);
			}
			TW_BIND();
		}
		TW_FREE(thingName);
	}

	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	twExt_Start(1000, TW_THREADING_MULTI, 5);
	twSleepMsec(10000);
	TEST_ASSERT_EQUAL(TW_OK, twExt_Stop());
	twApi_Disconnect("BindingIntegration: CSDK_1101");
	TEST_ASSERT_FALSE(twApi_isConnected());
	twStubs_Reset();
	twApi_Delete();
}

/* bind a single thing before connecting */
TEST(BindingIntegration,test_BindThingBeforeConnect){
	/* create var to collect results */
	int err = TW_OK;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* bind thing */
	err = twApi_BindThing(SUBSCRIBED_PROP_THINGNAME);
	TEST_ASSERT_EQUAL_INT(0,err);

	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return;
}

/* bind a single thing after connecting */
TEST(BindingIntegration,test_BindThingAfterConnect){
	/* create var to collect results */
	int err = 0;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* bind thing */
	err = twApi_BindThing("RickRoss");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

/* bind multiple things before connecting */
TEST(BindingIntegration,test_BindThingsBeforeConnect){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
    char * name = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	tbb_list = twList_Create(free_list);
	
	for (i = 0; i < NUM_NAMES; i++) {
		/* set name */
		name = (char * )TW_MALLOC(BIND_TEST_NAME_SIZE);
		sprintf(name, "Steam-%u", i);

		/* add to list */
		twList_Add(tbb_list, name);
	}

	/* bind thing */
	err = twApi_BindThings(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	err = twList_Delete(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	
	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

/* bind multiple things after connecting */
TEST(BindingIntegration,test_BindThingsAfterConnect){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
	char * name = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	tbb_list = twList_Create(free_list);
	
	for (i = 0; i < NUM_NAMES; i++) {
		/* set name */
		name = (char * )TW_CALLOC(BIND_TEST_NAME_SIZE,sizeof(char));
		sprintf(name, "Steam-%ud", i); 

		/* add to list */
		twList_Add(tbb_list, name);

		/* list takes ownership of pointer, so NULL name before continuing */
		name = NULL;
	}

	/* bind thing */
	err = twApi_BindThings(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	err = twList_Delete(tbb_list);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return; 
}

/* bind multiple things before connecting */
TEST(BindingIntegration,test_BindMassThingsBeforeConnectDefaultMaxMessageSize){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
	char * name = NULL;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return;
}

/* bind multiple things after connecting */
TEST(BindingIntegration,test_BindMassThingsAfterConnectDefaultMaxMessageSize){
	/* create var to collect results */
	int err = 0;
	int i = 0;
	twList * tbb_list = NULL;
	char * name = NULL;
	
	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

	/* init api */
	err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* disable cert validation */
	twApi_DisableCertValidation();

	/* connect api */
	err = twApi_Connect(30000, CONNECT_RETRIES);
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

    /* disconnect api */
	err = twApi_Disconnect("End test_BindThingBeforeConnect");
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	/* delete api */
	err = twApi_Delete();
	TEST_ASSERT_EQUAL_INT(TW_OK,err);

	return;
}

/* bind multiple things before connecting */
TEST(BindingIntegration,test_BindMassThingsBeforeConnect8kMaxMessageSize){
    /* create var to collect results */
    int err = 0;
    int i = 0;
    twList * tbb_list = NULL;
    char * name = NULL;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 8;

    /* disable cert validation */
    twApi_DisableCertValidation();

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));


    /* connect api */
    err = twApi_Connect(30000, CONNECT_RETRIES);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* disconnect api */
    err = twApi_Disconnect("End test_BindThingBeforeConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

/* bind multiple things after connecting */
TEST(BindingIntegration,test_BindMassThingsAfterConnect8kMaxMessageSize){
    /* create var to collect results */
    int err = 0;
    int i = 0;
    twList * tbb_list = NULL;
    char * name = NULL;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 8;

    /* disable cert validation */
    twApi_DisableCertValidation();

    /* connect api */
    err = twApi_Connect(30000, CONNECT_RETRIES);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

    /* disconnect api */
    err = twApi_Disconnect("End test_BindMassThingsAfterConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

/* bind multiple things before connecting */
TEST(BindingIntegration,test_BindMassThingsBeforeConnect10kMaxMessageSize){
    /* create var to collect results */
    int err = 0;
    int i = 0;
    twList * tbb_list = NULL;
    char * name = NULL;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 10;

    /* disable cert validation */
    twApi_DisableCertValidation();

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

    /* connect api */
    err = twApi_Connect(30000, CONNECT_RETRIES);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* disconnect api */
    err = twApi_Disconnect("End test_BindThingBeforeConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

/* bind multiple things after connecting */
TEST(BindingIntegration,test_BindMassThingsAfterConnect10kMaxMessageSize){
    /* create var to collect results */
    int err = 0;
    int i = 0;
    twList * tbb_list = NULL;
    char * name = NULL;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 10;

    /* disable cert validation */
    twApi_DisableCertValidation();

    /* connect api */
    err = twApi_Connect(30000, CONNECT_RETRIES);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

    /* disconnect api */
    err = twApi_Disconnect("End test_BindMassThingsAfterConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

/* bind multiple things before connecting */
TEST(BindingIntegration,test_BindMassThingsBeforeConnect16kMaxMessageSize){
    /* create var to collect results */
    int err = 0;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 16;

    /* disable cert validation */
    twApi_DisableCertValidation();

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));


    /* connect api */
    err = twApi_Connect(30000, CONNECT_RETRIES);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* disconnect api */
    err = twApi_Disconnect("End test_BindThingBeforeConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

/* bind multiple things after connecting */
TEST(BindingIntegration,test_BindMassThingsAfterConnect16kMaxMessageSize){
    /* create var to collect results */
    int err = 0;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 16;

    /* disable cert validation */
    twApi_DisableCertValidation();

    bindListLt = generateBindListLt(twcfg_pointer->max_message_size);
    bindListEq = generateBindListEq(twcfg_pointer->max_message_size);
    bindListGt = generateBindListGt(twcfg_pointer->max_message_size);
    bindListGt2 = generateBindListGt2(twcfg_pointer->max_message_size);

    /* bind thing */
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListLt));
    twList_Foreach(bindListLt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListEq));
    twList_Foreach(bindListEq, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt));
    twList_Foreach(bindListGt, verifyIsBoundHandler, NULL);
    TEST_ASSERT_EQUAL(TW_OK, twApi_BindThings(bindListGt2));
    twList_Foreach(bindListGt2, verifyIsBoundHandler, NULL);

    /* free lists */
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListLt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListEq));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt));
    TEST_ASSERT_EQUAL(TW_OK, twList_Delete(bindListGt2));

    /* disconnect api */
    err = twApi_Disconnect("End test_BindMassThingsAfterConnect");
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    /* delete api */
    err = twApi_Delete();
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    return;
}

