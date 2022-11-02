#include <twTls.h>
#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "twApiStubs.h"

TEST_GROUP(unit_twLogMessage);

TEST_SETUP(unit_twLogMessage) {
	eatLogs();
}

TEST_TEAR_DOWN(unit_twLogMessage) {
	twStubs_Reset();
}

TEST_GROUP_RUNNER(unit_twLogMessage) {
	RUN_TEST_CASE(unit_twLogMessage, test_twLogger_LogMessage);
	RUN_TEST_CASE(unit_twLogMessage, test_twLog_massLog);
}

extern twApi *tw_api;

twInfoTable *generateResponseBodyInfoTable() {
	twDataShapeEntry *entry1 = NULL;
	twDataShapeEntry *entry2 = NULL;
	twDataShapeEntry *entry3 = NULL;
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row1 = NULL;
	twInfoTableRow *row2 = NULL;
	twInfoTableRow *row3 = NULL;
	twResponseBody *b = NULL;

	entry1 = twDataShapeEntry_Create(TEST_DS_ENTRY_NAME_1, TEST_DS_ENTRY_DESCRIPTION_1, TW_INTEGER);
	entry2 = twDataShapeEntry_Create(TEST_DS_ENTRY_NAME_2, TEST_DS_ENTRY_DESCRIPTION_2, TW_INTEGER);
	entry3 = twDataShapeEntry_Create(TEST_DS_ENTRY_NAME_3, TEST_DS_ENTRY_DESCRIPTION_3, TW_INTEGER);
	ds = twDataShape_Create(entry1);
	twDataShape_AddEntry(ds, entry2);
	twDataShape_AddEntry(ds, entry3);
	it = twInfoTable_Create(ds);
	row1 = twInfoTableRow_Create(twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row1, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row1, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	row2 = twInfoTableRow_Create(twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row2, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row2, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	row3 = twInfoTableRow_Create(twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row3, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTableRow_AddEntry(row3, twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE));
	twInfoTable_AddRow(it, row1);
	twInfoTable_AddRow(it, row2);
	twInfoTable_AddRow(it, row3);
	return it;
}

TEST(unit_twLogMessage, test_twLogger_LogMessage) {
	twMessage * auth = NULL;
	twMessage * bind = NULL;
	twMessage * request = NULL;
	twMessage * response = NULL;
	twResponseBody * responseBody = NULL;
	twInfoTable * responseBodyInfoTable = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetIsVerbose(TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_TRACE));
	auth = twMessage_CreateAuthMsg(TEST_CLAIM_NAME, TEST_CLAIM_VALUE);
	bind = twMessage_CreateBindMsg(TEST_BIND_NAME, FALSE);
	request = twMessage_CreateRequestMsg(TWX_POST);
	response = twMessage_CreateResponseMsg(TWX_SUCCESS, TEST_ID, TEST_SESSION_ID, TEST_ENDPOINT_ID);
	responseBody = twResponseBody_Create();
	responseBodyInfoTable = generateResponseBodyInfoTable();
	TEST_ASSERT_EQUAL(TW_OK, twResponseBody_SetContent(responseBody, responseBodyInfoTable));
	TEST_ASSERT_EQUAL(TW_OK, twMessage_SetBody(response, responseBody));
	twLogMessage(auth, TEST_LOG_PREAMBLE);
	twLogMessage(bind, TEST_LOG_PREAMBLE);
	twLogMessage(request, TEST_LOG_PREAMBLE);
	twLogMessage(response, TEST_LOG_PREAMBLE);
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
	twMessage_Delete(auth);
	twMessage_Delete(bind);
	twMessage_Delete(request);
	twMessage_Delete(response);
}

TEST(unit_twLogMessage, test_twLog_massLog) {
	int i = 0;
	twMessage * auth = NULL;
	twMessage * bind = NULL;
	twMessage * request = NULL;
	twMessage * response = NULL;

	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetIsVerbose(TRUE));
	TEST_ASSERT_EQUAL(TW_OK, twLogger_SetLevel(TW_TRACE));
	auth = twMessage_CreateAuthMsg(TEST_CLAIM_NAME, TEST_CLAIM_VALUE);
	bind = twMessage_CreateBindMsg(TEST_BIND_NAME, FALSE);
	request = twMessage_CreateRequestMsg(TWX_GET);
	TEST_ASSERT_NOT_NULL(request);
	response = twMessage_CreateResponseMsg(TWX_SUCCESS, TEST_ID, TEST_SESSION_ID, TEST_ENDPOINT_ID);
	twLogMessage(auth, TEST_LOG_PREAMBLE);
	twLogMessage(bind, TEST_LOG_PREAMBLE);
	twLogMessage(request, TEST_LOG_PREAMBLE);
	twLogMessage(response, TEST_LOG_PREAMBLE);
	for (i = 0; i < 1000; i++) {
		twLog(TW_TRACE, "test_twLogger_massLog");
		twLogMessage(auth, TEST_LOG_PREAMBLE);
		twLogMessage(bind, TEST_LOG_PREAMBLE);
		twLogMessage(request, TEST_LOG_PREAMBLE);
		twLogMessage(response, TEST_LOG_PREAMBLE);
	}
	TEST_ASSERT_EQUAL(TW_OK, twLogger_Delete());
	twMessage_Delete(auth);
	twMessage_Delete(bind);
	twMessage_Delete(request);
	twMessage_Delete(response);
}
