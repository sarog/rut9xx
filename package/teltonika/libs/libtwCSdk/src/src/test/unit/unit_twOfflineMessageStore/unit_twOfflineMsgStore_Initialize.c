/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twOfflineMsgStore_Initialize()
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

TEST_GROUP(unit_twOfflineMsgStore_Initialize);

TEST_SETUP(unit_twOfflineMsgStore_Initialize) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_Initialize) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_Initialize) {
	/* init/delete on disk */
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Null_File_Path_OnDisk);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_OnDisk);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Success_OnDisk);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Double_Delete_OnDisk);

	/* init/delete in RAM */
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_InRam);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_InRam);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Double_Delete_InRam);

	/* init/delete mixed in ram and on disk (only the cases that apply) */
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_Mixed);
	RUN_TEST_CASE(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Delete_Nothing_Mixed);
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Null_File_Path_OnDisk) {
	TEST_ASSERT_EQUAL(TW_INVALID_PARAM, twOfflineMsgStore_Initialize(TRUE, NULL, OFFLINE_MSG_STORE_SIZE, TRUE));
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_OnDisk) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Success_OnDisk) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Double_Delete_OnDisk) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON,twOfflineMsgStore_Delete());
}

/* in RAM */
TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_InRam) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Success_InRam) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Double_Delete_InRam) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON,twOfflineMsgStore_Delete());
}

/* mixed on disk and in RAM (only the tests that apply) */
TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Initialize_Test_Double_Init_Mixed) {
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twOfflineMsgStore_Initialize(TRUE, OFFLINE_MSG_STORE_LOCATION, OFFLINE_MSG_STORE_SIZE, FALSE));
	TEST_ASSERT_EQUAL(TW_OK,twOfflineMsgStore_Delete());
}

TEST(unit_twOfflineMsgStore_Initialize, twOfflineMsgStore_Delete_Nothing_Mixed) {
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON,twOfflineMsgStore_Delete());
}