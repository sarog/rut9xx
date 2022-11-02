/*
 * Copyright (c) 2018 PTC, Inc. All rights reserved.
 */

#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "twExt.h"
#include "twPasswds.h"

TEST_GROUP(unit_twConvertCallbackToPasswd);

void test_unit_twConvertCallbackToPasswdAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,"Hey, Have you Tried Value?",len);
}


TEST_SETUP(unit_twConvertCallbackToPasswd) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twConvertCallbackToPasswd) {
}

TEST_GROUP_RUNNER(unit_twConvertCallbackToPasswd) {
	RUN_TEST_CASE(unit_twConvertCallbackToPasswd, testResolveCallbackToPassword);
	RUN_TEST_CASE(unit_twConvertCallbackToPasswd, testResolveNullCallbackToPassword);
	RUN_TEST_CASE(unit_twConvertCallbackToPasswd, testResolveLooksLikeAppKeyCallback);
}

/**
 * Test Plan: Try to convert a password callback to a password. Verify it and then make sure
 * it is removed from memory.
 */
TEST(unit_twConvertCallbackToPasswd, testResolveCallbackToPassword) {
	char* password = twConvertCallbackToPasswd(test_unit_twConvertCallbackToPasswdAppKey_callback);
	TEST_ASSERT_NOT_NULL(password);
	TEST_ASSERT_EQUAL_STRING("Hey, Have you Tried Value?",password);
	twFreePasswd(password);
}
/**
 * Test Plan: Try to convert a NULL callback to a password. Verify that is is the empty string.
 */
TEST(unit_twConvertCallbackToPasswd, testResolveNullCallbackToPassword) {
	char* password = twConvertCallbackToPasswd(NULL);
	TEST_ASSERT_NOT_NULL(password);
	TEST_ASSERT_EQUAL_STRING("",password);
	twFreePasswd(password);
}

/**
 * Test Plan: Try to convert an app key string callback to a password. Verify that is is the empty string.
 */
TEST(unit_twConvertCallbackToPasswd, testResolveLooksLikeAppKeyCallback) {
	char* password = twConvertCallbackToPasswd("badbadba-dbad-badb-adba-badbadbadbad");
	TEST_ASSERT_NOT_NULL(password);
	TEST_ASSERT_EQUAL_STRING("",password);
	twFreePasswd(password);
}
