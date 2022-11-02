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

int twDict_Foreach_stateCounter;
int twDict_Foreach_Pass;
twDict* list_Foreach;
char is_Foreach_MutextLocked;
int twDict_Foreach_callCounter;
int twDict_Foreach_stateCounter;
int twDict_Foreach_Pass;
twDict* list_Foreach;
char is_Foreach_MutextLocked;
int twDict_Foreach_callCounter;

void twDict_setDictionaryMode(twDictionaryMode dictionaryMode);
void twDictionaryModeMtx_Create();
void twDictionaryModeMtx_Delete();

static char* twDict_Integer_ParseFunction(void* anInt){
	char buff[100];
	int * theInt = (int*)anInt;
	snprintf(buff,90,"%i",(int) *theInt);
	return duplicateString(buff);
}

const char* test_twDict_aslist_Find_parserFunction(void* item){
	return duplicateString((char*)item);
}
void test_twDict_aslist_FindDontDeleteFunction(void* ptr){
	// Do nothing
}

int twDict_ForEach_CountHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twDict_Foreach_callCounter++;
	if(twDict_Foreach_callCounter>2)
		return TW_FOREACH_EXIT;
	else
		return TW_FOREACH_CONTINUE;
}

int twDict_ForEach_ForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	TEST_ASSERT_EQUAL_STRING("User Data",(char*)userData);
	TEST_ASSERT_TRUE(is_Foreach_MutextLocked);
	switch(twDict_Foreach_stateCounter){
		case 0:
			if(strcmp("A",(char*)currentValue)!=0) {
				return TW_FOREACH_EXIT;
			}
			else
				twDict_Foreach_stateCounter++;
			break;
		case 1:
			if(strcmp("B",(char*)currentValue)!=0) {
				return TW_FOREACH_EXIT;
			}
			else
				twDict_Foreach_stateCounter++;
			break;
		case 2:
			if(strcmp("C",(char*)currentValue)!=0) {
				return TW_FOREACH_EXIT;
			}
			else
				twDict_Foreach_stateCounter++;
			twDict_Foreach_Pass = 1;
			break;
	}
	return TW_FOREACH_CONTINUE;
}

void test_twDict_aslist_Foreach_mutex_lock(TW_MUTEX m){
	twMutex_Lock(m);
	is_Foreach_MutextLocked = TRUE;
}

void test_twDict_aslist_Foreach_mutex_unlock(TW_MUTEX m){
	twMutex_Unlock(m);
	is_Foreach_MutextLocked = FALSE;
}

TEST_GROUP(unit_twDictionary_Add);

TEST_SETUP(unit_twDictionary_Add) {
	twDictionaryModeMtx_Create();
	eatLogs();
}

TEST_TEAR_DOWN(unit_twDictionary_Add) {
	twStubs_Reset();
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	twDictionaryModeMtx_Delete();
}

TEST_GROUP_RUNNER(unit_twDictionary_Add) {
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_asmap_ReplaceValue_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_asmap_Remove_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_asmap_Add_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_asmap_Find);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_aslist_ReplaceValue_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_aslist_Remove_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_aslist_Add_Invalid_Parameters);
	RUN_TEST_CASE(unit_twDictionary_Add, test_twDict_aslist_Find);
}

TEST(unit_twDictionary_Add, test_twDict_asmap_ReplaceValue_Invalid_Parameters) {
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

TEST(unit_twDictionary_Add, test_twDict_asmap_Remove_Invalid_Parameters) {
	twDict *list = NULL;
	ListEntry *entry = NULL;

	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	list = twDict_Create(&doNothing, twDict_Integer_ParseFunction);

	/* NULL entry */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Remove(list, NULL, FALSE));
	/* NULL list */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Remove(NULL, entry, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

TEST(unit_twDictionary_Add, test_twDict_asmap_Add_Invalid_Parameters) {
	/* NULL list */
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Add(NULL, 0));
}

TEST(unit_twDictionary_Add, test_twDict_asmap_Create_Delete) {
	twDict *list;
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	list = twDict_Create(&doNothing, NULL);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

/**
 * Test Plan: Create a list containing three items. Use Foreach to enumerate this list.
 * Verify list is locked and then unlocked. Verify that list is processed in order.
 * Verify that user data is passed.
 */
TEST(unit_twDictionary_Add, test_twDict_asmap_Foreach) {
	char* userData = "User Data";
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("test_twDict_asmap_Foreach");
	}
	twDict_Foreach_stateCounter = 0;
	twDict_Foreach_Pass = 0;
	is_Foreach_MutextLocked = FALSE;

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	twApi_stub->twMutex_Lock = test_twDict_aslist_Foreach_mutex_lock;
	twApi_stub->twMutex_Unlock = test_twDict_aslist_Foreach_mutex_unlock;

	list_Foreach = twDict_Create(NULL, NULL);
	twDict_Add(list_Foreach,(void*)"A");
	twDict_Add(list_Foreach,(void*)"B");
	twDict_Add(list_Foreach,(void*)"C");
	twDict_Foreach(list_Foreach, twDict_ForEach_ForEachHandler, (void *) userData);

	TEST_ASSERT_FALSE(is_Foreach_MutextLocked);

	if(0 == twDict_Foreach_Pass){
		TEST_FAIL_MESSAGE("Failed to reach final state.");
	}
	twDict_Delete(list_Foreach);

	list_Foreach = NULL;

}

/**
 * Test Plan: A list handler should be able to abort a foreach operation when a result has been found by
 * returning false. In this test this should cause the list to only have two of its three values enumerated.
 */
TEST(unit_twDictionary_Add,test_twDict_asmap_Foreach_Abort){
	char* userData = "User Data";
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("test_twDict_asmap_Foreach_Abort");
	}
	twDict_Foreach_callCounter = 0;

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	list_Foreach = twDict_Create(NULL, NULL);
	twDict_Add(list_Foreach,(void*)"A");
	twDict_Add(list_Foreach,(void*)"B");
	twDict_Add(list_Foreach,(void*)"C");
	twDict_Foreach(list_Foreach, twDict_ForEach_CountHandler, (void *) userData);

	/* This test condition may seem odd but it would be 4 if the foreach ran to completion */
	TEST_ASSERT_EQUAL(3,twDict_Foreach_callCounter);

	twDict_Delete(list_Foreach);

	list_Foreach = NULL;
}

/**
 * Create a sample list and search it.
 */
TEST(unit_twDictionary_Add,test_twDict_asmap_Find) {
	twDict* list_Find;
	void* result;
	twDict_setDictionaryMode(TW_DICTIONARY_MAP);
	twDict_Foreach_callCounter = 0;

	list_Find = twDict_Create(test_twDict_aslist_FindDontDeleteFunction, test_twDict_aslist_Find_parserFunction);
	twDict_Add(list_Find,(void*)"A");
	twDict_Add(list_Find,(void*)"B");
	twDict_Add(list_Find,(void*)"C");
	TEST_ASSERT_EQUAL(TW_OK,twDict_Find(list_Find,(void *) "B",&result));
	TEST_ASSERT_EQUAL_STRING("B",(char*)result);
	twDict_Delete(list_Find);
}

TEST(unit_twDictionary_Add, test_twDict_aslist_ReplaceValue_Invalid_Parameters) {
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

TEST(unit_twDictionary_Add, test_twDict_aslist_Remove_Invalid_Parameters) {
	twDict *list = NULL;
	ListEntry *entry = NULL;

	twDict_setDictionaryMode(TW_DICTIONARY_LIST);

	/* You can't delete from a list without a parswe function */
	list = twDict_Create(&doNothing, NULL);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Remove(list, NULL, FALSE));
	twDict_Delete(list);

	list = twDict_Create(&doNothing, twDict_Integer_ParseFunction);

	/* NULL entry */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Remove(list, NULL, FALSE));
	/* NULL list */
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Remove(NULL, entry, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twDict_Delete(list));
}

TEST(unit_twDictionary_Add, test_twDict_aslist_Add_Invalid_Parameters) {
	/* NULL list */
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twDict_Add(NULL, 0));
}

/**
 * Test Plan: Create a list containing three items. Use Foreach to enumerate this list.
 * Verify list is locked and then unlocked. Verify that list is processed in order.
 * Verify that user data is passed.
 */
TEST(unit_twDictionary_Add, test_twDict_aslist_Foreach) {
	char* userData = "User Data";
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("test_twDict_aslist_Foreach");
	}
	twDict_Foreach_stateCounter = 0;
	twDict_Foreach_Pass = 0;
	is_Foreach_MutextLocked = FALSE;

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	twApi_stub->twMutex_Lock = test_twDict_aslist_Foreach_mutex_lock;
	twApi_stub->twMutex_Unlock = test_twDict_aslist_Foreach_mutex_unlock;

	list_Foreach = twDict_Create(NULL, NULL);
	twDict_Add(list_Foreach,(void*)"A");
	twDict_Add(list_Foreach,(void*)"B");
	twDict_Add(list_Foreach,(void*)"C");
	twDict_Foreach(list_Foreach, twDict_ForEach_ForEachHandler, (void *) userData);

	TEST_ASSERT_FALSE(is_Foreach_MutextLocked);

	if(0 == twDict_Foreach_Pass){
		TEST_FAIL_MESSAGE("Failed to reach final state.");
	}
	twDict_Delete(list_Foreach);

	list_Foreach = NULL;

}

/**
 * Test Plan: A list handler should be able to abort a foreach operation when a result has been found by
 * returning false. In this test this should cause the list to only have two of its three values enumerated.
 */
TEST(unit_twDictionary_Add,test_twDict_aslist_Foreach_Abort){
	char* userData = "User Data";
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);
	if (IGNORE_UNSTABLE_TESTS) {
		TEST_IGNORE_MESSAGE("test_twDict_aslist_Foreach_Abort");
	}
	twDict_Foreach_callCounter = 0;

	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));

	list_Foreach = twDict_Create(NULL, NULL);
	twDict_Add(list_Foreach,(void*)"A");
	twDict_Add(list_Foreach,(void*)"B");
	twDict_Add(list_Foreach,(void*)"C");
	twDict_Foreach(list_Foreach, twDict_ForEach_CountHandler, (void *) userData);

	/* This test condition may seem odd but it would be 4 if the foreach ran to completion */
	TEST_ASSERT_EQUAL(3,twDict_Foreach_callCounter);

	twDict_Delete(list_Foreach);

	list_Foreach = NULL;
}

/**
 * Create a sample list and search it
 */
TEST(unit_twDictionary_Add,test_twDict_aslist_Find) {
	char *res;
	twDict * list_Find;
	ListEntry * result;
	twDict_Foreach_callCounter = 0;
	twDict_setDictionaryMode(TW_DICTIONARY_LIST);

	list_Find = twDict_Create(test_twDict_aslist_FindDontDeleteFunction, test_twDict_aslist_Find_parserFunction);
	twDict_Add(list_Find,(void*)"A");
	twDict_Add(list_Find,(void*)"B");
	twDict_Add(list_Find,(void*)"C");
	TEST_ASSERT_EQUAL(TW_OK,twDict_Find(list_Find,(void *) "B",&result));
	res = result->value;
	TEST_ASSERT_EQUAL_STRING("B",(char*)((ListEntry*)result)->value);
	twDict_Delete(list_Find);
}