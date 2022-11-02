/*
*	Created by Jeff Dreyer on 5/26/16.
*	Copyright 2016, PTC, Inc.
*/

#include <twBaseTypes.h>
#include "unity.h"
#include "unity_fixture.h"
#include "twProperties.h"
#include "stringUtils.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"

TEST_GROUP(BindingIntegrationSlow);

void test_BindingIntegrationSlowAppKey_callback(char *passWd, unsigned int len){
    strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(BindingIntegrationSlow){
    eatLogs();
}

TEST_TEAR_DOWN(BindingIntegrationSlow){
}

TEST_GROUP_RUNNER(BindingIntegrationSlow) {
    RUN_TEST_CASE(BindingIntegrationSlow, test_BindMassThingsBeforeConnect100kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegrationSlow, test_BindMassThingsAfterConnect100kMaxMessageSize);
    RUN_TEST_CASE(BindingIntegrationSlow, test_BindMassThingsBeforeConnect1mMaxMessageSize);
    RUN_TEST_CASE(BindingIntegrationSlow, test_BindMassThingsAfterConnect1mMaxMessageSize);
}

/* bind multiple things before connecting */
TEST(BindingIntegrationSlow,test_BindMassThingsBeforeConnect100kMaxMessageSize){
    /* create var to collect results */
    int err = 0;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;


    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationSlowAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 100;

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
    err = twApi_Connect(30000, -1);
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
TEST(BindingIntegrationSlow,test_BindMassThingsAfterConnect100kMaxMessageSize){
    /* create var to collect results */
    int err = 0;

	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationSlowAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 100;

    /* disable cert validation */
    twApi_DisableCertValidation();

    /* connect api */
    err = twApi_Connect(30000, -1);
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
TEST(BindingIntegrationSlow,test_BindMassThingsBeforeConnect1mMaxMessageSize){
    /* create var to collect results */
    int err = 0;
	twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;

    twcfg_pointer->max_message_size = 1024 * 1024;

    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationSlowAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
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
    err = twApi_Connect(30000, -1);
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
TEST(BindingIntegrationSlow,test_BindMassThingsAfterConnect1mMaxMessageSize){
    /* create var to collect results */
    int err = 0;

    twList *bindListLt = NULL;
    twList *bindListEq = NULL;
    twList *bindListGt = NULL;
    twList *bindListGt2 = NULL;


    /* init api */
    err = twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_BindingIntegrationSlowAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    TEST_ASSERT_EQUAL_INT(TW_OK,err);

    twcfg_pointer->max_message_size = 1024 * 1024;

    /* disable cert validation */
    twApi_DisableCertValidation();

    /* connect api */
    err = twApi_Connect(30000, -1);
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
