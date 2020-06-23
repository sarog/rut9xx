/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for 
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

void twDict_setDictionaryMode(twDictionaryMode dictionaryMode);
void twDictionaryModeMtx_Create();
void twDictionaryModeMtx_Delete();

static char* twDict_Integer_ParseFunction(void* anInt){
	char buff[100];
	int * theInt = (int*)anInt;
	snprintf(buff,90,"%i",(int) *theInt);
	return duplicateString(buff);
}

TEST_GROUP(unit_twDictionary_Create);

TEST_SETUP(unit_twDictionary_Create) {
	twDictionaryModeMtx_Create();
	eatLogs();
}

TEST_TEAR_DOWN(unit_twDictionary_Create) {
	twStubs_Reset();
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	twDictionaryModeMtx_Delete();
}

TEST_GROUP_RUNNER(unit_twDictionary_Create) {
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_asmap_Delete_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_asmap_Clear_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_asmap_Create_Delete);
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_aslist_Delete_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_aslist_Clear_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Create, test_twDict_aslist_Create_Delete);
}

TEST(unit_twDictionary_Create, test_twDict_asmap_Delete_Invalid_Parameters) {
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Delete(NULL));
}

TEST(unit_twDictionary_Create, test_twDict_asmap_Clear_Invalid_Parameters) {
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Clear(NULL));
}

TEST(unit_twDictionary_Create, test_twDict_asmap_ReplaceValue_Invalid_Parameters) {
	twDict *list = NULL;
	ListEntry *entry = NULL;
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);

	list = twDict_Create(&doNothing, twDict_Integer_ParseFunction);

	/* NULL entry */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_ReplaceValue(list, NULL, 0, FALSE));
	/* NULL list */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_ReplaceValue(NULL, entry, 0, FALSE));

	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

TEST(unit_twDictionary_Create, test_twDict_asmap_Create_Delete) {
	twDict *list;
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	list = twDict_Create(&doNothing, NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

TEST(unit_twDictionary_Create, test_twDict_aslist_Delete_Invalid_Parameters) {
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Delete(NULL));
}

TEST(unit_twDictionary_Create, test_twDict_aslist_Clear_Invalid_Parameters) {
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Clear(NULL));
}

TEST(unit_twDictionary_Create, test_twDict_aslist_ReplaceValue_Invalid_Parameters) {
	twDict *list = NULL;
	ListEntry *entry = NULL;
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);

	list = twDict_Create(NULL, NULL);

	/* NULL entry */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_ReplaceValue(list, NULL, 0, FALSE));
	/* NULL list */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_ReplaceValue(NULL, entry, 0, FALSE));

	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

TEST(unit_twDictionary_Create, test_twDict_aslist_Create_Delete) {
	twDict *list;
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	list = twDict_Create(&doNothing, NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}
