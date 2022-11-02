#include <twBaseTypes.h>
#include "twApi.h"
#include "twThreads.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"

#define PROPERTY_WRITE_TIMEOUT 5000
#define PROPERTY_READ_TIMEOUT 5000
#define NUM_WORKER_THREADS 2

char *thingNamePropertyWriteIntegration = NULL;
twThread *apiThreadPropertyWriteIntegration = NULL;
twThread *workerThreadsPropertyWriteIntegration[NUM_WORKER_THREADS] = {NULL};

TEST_GROUP(PropertyWriteIntegration);

void test_PropertyWriteIntegration_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}


TEST_SETUP(PropertyWriteIntegration) {
	int i = 0;
	/* Configure logger */
	eatLogs();
	/* Initialize and configure API */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, test_PropertyWriteIntegration_callback, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	/* Connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	/* Dynamically create server test thing */
	thingNamePropertyWriteIntegration = createServerTestThing();
	TEST_ASSERT_NOT_NULL(thingNamePropertyWriteIntegration);
	/* Bind */
	TEST_ASSERT_EQUAL(TW_OK, twApi_BindThing(thingNamePropertyWriteIntegration));
	TEST_ASSERT_TRUE(twApi_IsEntityBound(thingNamePropertyWriteIntegration));
	/* Spin up threads */
	apiThreadPropertyWriteIntegration = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreadsPropertyWriteIntegration[i] = twThread_Create(twMessageHandler_msgHandlerTask, 10, NULL, TRUE);
	}
}

TEST_TEAR_DOWN(PropertyWriteIntegration) {
	int i = 0;
	deleteServerThing(TW_THING, thingNamePropertyWriteIntegration);
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("PropertyWriteIntegrationTests: TEST_TEAR_DOWN"));
	TEST_ASSERT_FALSE(twApi_isConnected());
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreadsPropertyWriteIntegration[i]);
	}
	twThread_Delete(apiThreadPropertyWriteIntegration);
	twApi_Delete();
	TW_FREE(thingNamePropertyWriteIntegration);
}

TEST_GROUP_RUNNER(PropertyWriteIntegration) {
	RUN_TEST_CASE(PropertyWriteIntegration, InfoTablePropertyWrite);
	RUN_TEST_CASE(PropertyWriteIntegration, WrappedInfoTablePropertyWrite);
	RUN_TEST_CASE(PropertyWriteIntegration, MultiPropertyWrite);
	RUN_TEST_CASE(PropertyWriteIntegration, JSONObjectPropertyWrite);
}

typedef struct allPropertyBaseTypePrimitives {
	twPrimitive *booleanValue;
	twPrimitive *integerValue;
	twPrimitive *numberValue;
	twPrimitive *datetimeValue;
	twPrimitive *stringValue;
	twPrimitive *textValue;
	twPrimitive *thingNameValue;
	twPrimitive *groupNameValue;
	twPrimitive *userNameValue;
	twPrimitive *mashupNameValue;
	twPrimitive *menuNameValue;
	twPrimitive *imagelinkValue;
	twPrimitive *hyperlinkValue;
	twPrimitive *HTMLValue;
	twPrimitive *XMLValue;
	twPrimitive *JSONValue;
	twPrimitive *queryValue;
	twPrimitive *locationValue;
} allPropertyBaseTypePrimitives;

int compareAllPropertyBaseTypePrimitives(allPropertyBaseTypePrimitives *p1, allPropertyBaseTypePrimitives *p2) {
	int res = 0;
	res = twPrimitive_Compare(p1->booleanValue, p2->booleanValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->integerValue, p2->integerValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->numberValue, p2->numberValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->datetimeValue, p2->datetimeValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->stringValue, p2->stringValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->textValue, p2->textValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->thingNameValue, p2->thingNameValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->groupNameValue, p2->groupNameValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->userNameValue, p2->userNameValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->mashupNameValue, p2->mashupNameValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->menuNameValue, p2->menuNameValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->imagelinkValue, p2->imagelinkValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->hyperlinkValue, p2->hyperlinkValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->HTMLValue, p2->HTMLValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->XMLValue, p2->XMLValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->JSONValue, p2->JSONValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->queryValue, p2->queryValue); if (res != 0) return res;
	res = twPrimitive_Compare(p1->locationValue, p2->locationValue);
	return res;
}

void deleteAllPropertyBaseTypePrimitives(allPropertyBaseTypePrimitives *primitives) {
	twPrimitive_Delete(primitives->booleanValue);
	twPrimitive_Delete(primitives->integerValue);
	twPrimitive_Delete(primitives->numberValue);
	twPrimitive_Delete(primitives->datetimeValue);
	twPrimitive_Delete(primitives->stringValue);
	twPrimitive_Delete(primitives->textValue);
	twPrimitive_Delete(primitives->thingNameValue);
	twPrimitive_Delete(primitives->groupNameValue);
	twPrimitive_Delete(primitives->userNameValue);
	twPrimitive_Delete(primitives->mashupNameValue);
	twPrimitive_Delete(primitives->menuNameValue);
	twPrimitive_Delete(primitives->imagelinkValue);
	twPrimitive_Delete(primitives->hyperlinkValue);
	twPrimitive_Delete(primitives->HTMLValue);
	twPrimitive_Delete(primitives->XMLValue);
	twPrimitive_Delete(primitives->JSONValue);
	twPrimitive_Delete(primitives->queryValue);
	twPrimitive_Delete(primitives->locationValue);
	TW_FREE(primitives);
}

TEST(PropertyWriteIntegration, InfoTablePropertyWrite) {
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row = NULL;
	twPrimitive *readValues = NULL;
	twPrimitive *writeValues = NULL;
	allPropertyBaseTypePrimitives *readPrimitives = NULL;
	allPropertyBaseTypePrimitives *writePrimitives = NULL;
	twLocation *location = NULL;

	/* Create primitives containing new non-default property values to be written */
	location = TW_MALLOC(sizeof(twLocation));
	location->latitude = 38.8977;
	location->longitude = 77.0365;
	location->elevation = 410;
	writePrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	writePrimitives->booleanValue = twPrimitive_CreateFromBoolean(TRUE);
	writePrimitives->integerValue = twPrimitive_CreateFromInteger(1);
	writePrimitives->numberValue = twPrimitive_CreateFromNumber(1.0);
	writePrimitives->datetimeValue = twPrimitive_CreateFromCurrentTime();
	writePrimitives->stringValue = twPrimitive_CreateFromString("NewString", TRUE);
	writePrimitives->textValue = twPrimitive_CreateFromString("NewText", TRUE);
	writePrimitives->thingNameValue = twPrimitive_CreateFromString("NewThingName", TRUE);
	writePrimitives->groupNameValue = twPrimitive_CreateFromString("NewGroupName", TRUE);
	writePrimitives->userNameValue = twPrimitive_CreateFromString("NewUserName", TRUE);
	writePrimitives->mashupNameValue = twPrimitive_CreateFromString("NewMashupName", TRUE);
	writePrimitives->menuNameValue = twPrimitive_CreateFromString("NewMenuName", TRUE);
	writePrimitives->imagelinkValue = twPrimitive_CreateFromString("http://NewImagelink.net", TRUE);
	writePrimitives->hyperlinkValue = twPrimitive_CreateFromString("http://NewHyperlink.net", TRUE);
	writePrimitives->HTMLValue = twPrimitive_CreateFromString("<html><body>NewHTML</body></html>", TRUE);
	writePrimitives->XMLValue = twPrimitive_CreateFromString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><xml><body>NewXML</body></xml>", TRUE);
	writePrimitives->JSONValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->queryValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->locationValue = twPrimitive_CreateFromLocation(location);
	TW_FREE(location);

	writePrimitives->booleanValue->type = TW_BOOLEAN;
	writePrimitives->integerValue->type = TW_INTEGER;
	writePrimitives->numberValue->type = TW_NUMBER;
	writePrimitives->datetimeValue->type = TW_DATETIME;
	writePrimitives->stringValue->type = TW_STRING;
	writePrimitives->textValue->type = TW_TEXT;
	writePrimitives->thingNameValue->type = TW_THINGNAME;
	writePrimitives->groupNameValue->type = TW_GROUPNAME;
	writePrimitives->userNameValue->type = TW_USERNAME;
	writePrimitives->mashupNameValue->type = TW_MASHUPNAME;
	writePrimitives->menuNameValue->type = TW_MENUNAME;
	writePrimitives->imagelinkValue->type = TW_IMAGELINK;
	writePrimitives->hyperlinkValue->type = TW_HYPERLINK;
	writePrimitives->HTMLValue->type = TW_HTML;
	writePrimitives->XMLValue->type = TW_XML;
	writePrimitives->JSONValue->type = TW_JSON;
	writePrimitives->queryValue->type = TW_QUERY;
	writePrimitives->locationValue->type = TW_LOCATION;
	
	/* Create an info table row conforming to the previously created data shape */
	row = twInfoTableRow_Create(twPrimitive_FullCopy(writePrimitives->booleanValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->integerValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->numberValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->datetimeValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->stringValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->textValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->thingNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->groupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->userNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->mashupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->menuNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->imagelinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->hyperlinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->HTMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->XMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->JSONValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->queryValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->locationValue));
	/* Add the newly populated info table row to the previously created info table */
	ds = createAllPropertyBaseTypesDataShape();
	it = twInfoTable_Create(ds);
	twInfoTable_AddRow(it, row);

	/* Create a primitive from the newly created info table */
	writeValues = twPrimitive_CreateFromInfoTable(it);
	twInfoTable_Delete(it);

	/* Write the new values to the AlwaysPush_InfoTable property */
	TEST_ASSERT_EQUAL(TW_OK, twApi_WriteProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_InfoTable", writeValues, PROPERTY_WRITE_TIMEOUT, FALSE));
	twPrimitive_Delete(writeValues);
	/* Read back the values */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_InfoTable", &readValues, PROPERTY_READ_TIMEOUT, FALSE));

	/* Get primitive values from the info table we read back */
	readPrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	twInfoTable_GetPrimitive(readValues->val.infotable, "Boolean", 0, &readPrimitives->booleanValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Integer", 0, &readPrimitives->integerValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Number", 0, &readPrimitives->numberValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Datetime", 0, &readPrimitives->datetimeValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "String", 0, &readPrimitives->stringValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Text", 0, &readPrimitives->textValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "ThingName", 0, &readPrimitives->thingNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "GroupName", 0, &readPrimitives->groupNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "UserName", 0, &readPrimitives->userNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "MashupName", 0, &readPrimitives->mashupNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "MenuName", 0, &readPrimitives->menuNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Imagelink", 0, &readPrimitives->imagelinkValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Hyperlink", 0, &readPrimitives->hyperlinkValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "HTML", 0, &readPrimitives->HTMLValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "XML", 0, &readPrimitives->XMLValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "JSON", 0, &readPrimitives->JSONValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Query", 0, &readPrimitives->queryValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Location", 0, &readPrimitives->locationValue);

	/* Check that the values match */
	TEST_ASSERT_EQUAL(0, compareAllPropertyBaseTypePrimitives(readPrimitives, writePrimitives));
	twPrimitive_Delete(readValues);
	/* The members of this struct were already deleted when we deleted readValues */
	TW_FREE(readPrimitives);
	deleteAllPropertyBaseTypePrimitives(writePrimitives);
}

TEST(PropertyWriteIntegration, WrappedInfoTablePropertyWrite) {
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row = NULL;
	twPrimitive *readValues = NULL;
	twPrimitive *writeValues = NULL;
	allPropertyBaseTypePrimitives *readPrimitives = NULL;
	allPropertyBaseTypePrimitives *writePrimitives = NULL;
	twLocation *location = NULL;
	twDataShape *wrapDs = NULL;
	twInfoTable *wrapIt = NULL;
	twPrimitive *wrapValue = NULL;

	/* Create primitives containing new non-default property values to be written */
	location = TW_MALLOC(sizeof(twLocation));
	location->latitude = 38.8977;
	location->longitude = 77.0365;
	location->elevation = 410;
	writePrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	writePrimitives->booleanValue = twPrimitive_CreateFromBoolean(TRUE);
	writePrimitives->integerValue = twPrimitive_CreateFromInteger(1);
	writePrimitives->numberValue = twPrimitive_CreateFromNumber(1.0);
	writePrimitives->datetimeValue = twPrimitive_CreateFromCurrentTime();
	writePrimitives->stringValue = twPrimitive_CreateFromString("NewString", TRUE);
	writePrimitives->textValue = twPrimitive_CreateFromString("NewText", TRUE);
	writePrimitives->thingNameValue = twPrimitive_CreateFromString("NewThingName", TRUE);
	writePrimitives->groupNameValue = twPrimitive_CreateFromString("NewGroupName", TRUE);
	writePrimitives->userNameValue = twPrimitive_CreateFromString("NewUserName", TRUE);
	writePrimitives->mashupNameValue = twPrimitive_CreateFromString("NewMashupName", TRUE);
	writePrimitives->menuNameValue = twPrimitive_CreateFromString("NewMenuName", TRUE);
	writePrimitives->imagelinkValue = twPrimitive_CreateFromString("http://NewImagelink.net", TRUE);
	writePrimitives->hyperlinkValue = twPrimitive_CreateFromString("http://NewHyperlink.net", TRUE);
	writePrimitives->HTMLValue = twPrimitive_CreateFromString("<html><body>NewHTML</body></html>", TRUE);
	writePrimitives->XMLValue = twPrimitive_CreateFromString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><xml><body>NewXML</body></xml>", TRUE);
	writePrimitives->JSONValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->queryValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->locationValue = twPrimitive_CreateFromLocation(location);
	TW_FREE(location);

	writePrimitives->booleanValue->type = TW_BOOLEAN;
	writePrimitives->integerValue->type = TW_INTEGER;
	writePrimitives->numberValue->type = TW_NUMBER;
	writePrimitives->datetimeValue->type = TW_DATETIME;
	writePrimitives->stringValue->type = TW_STRING;
	writePrimitives->textValue->type = TW_TEXT;
	writePrimitives->thingNameValue->type = TW_THINGNAME;
	writePrimitives->groupNameValue->type = TW_GROUPNAME;
	writePrimitives->userNameValue->type = TW_USERNAME;
	writePrimitives->mashupNameValue->type = TW_MASHUPNAME;
	writePrimitives->menuNameValue->type = TW_MENUNAME;
	writePrimitives->imagelinkValue->type = TW_IMAGELINK;
	writePrimitives->hyperlinkValue->type = TW_HYPERLINK;
	writePrimitives->HTMLValue->type = TW_HTML;
	writePrimitives->XMLValue->type = TW_XML;
	writePrimitives->JSONValue->type = TW_JSON;
	writePrimitives->queryValue->type = TW_QUERY;
	writePrimitives->locationValue->type = TW_LOCATION;

	/* Create an info table row conforming to the previously created data shape */
	row = twInfoTableRow_Create(twPrimitive_FullCopy(writePrimitives->booleanValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->integerValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->numberValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->datetimeValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->stringValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->textValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->thingNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->groupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->userNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->mashupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->menuNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->imagelinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->hyperlinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->HTMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->XMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->JSONValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->queryValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->locationValue));

	/* Add the newly populated info table row to an info table */
	ds = createAllPropertyBaseTypesDataShape();
	it = twInfoTable_Create(ds);
	twInfoTable_AddRow(it, row);

	/* Create a primitive from the newly created info table */
	writeValues = twPrimitive_CreateFromInfoTable(it);
	twInfoTable_Delete(it);

	/* Create a data shape to wrap the info table property */
	wrapDs = twDataShape_Create(twDataShapeEntry_Create("AlwaysPush_InfoTable", "", TW_INFOTABLE));
	/* Create an info table from the data shape we just created */
	wrapIt = twInfoTable_Create(wrapDs);
	/* Create an info table row from the values we want to write and add it to the info table we just created */
	twInfoTable_AddRow(wrapIt, twInfoTableRow_Create(writeValues));
	/* Create a primitive from this info table */
	wrapValue = twPrimitive_CreateFromInfoTable(wrapIt);
	twInfoTable_Delete(wrapIt);

	/* Write the new values to the AlwaysPush_InfoTable property */
	TEST_ASSERT_EQUAL(TW_OK, twApi_WriteProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_InfoTable", wrapValue, PROPERTY_WRITE_TIMEOUT, FALSE));
	twPrimitive_Delete(wrapValue);
	/* Read back the values */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_InfoTable", &readValues, PROPERTY_READ_TIMEOUT, FALSE));

	/* Get primitive values from the info table we read back */
	readPrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	twInfoTable_GetPrimitive(readValues->val.infotable, "Boolean", 0, &readPrimitives->booleanValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Integer", 0, &readPrimitives->integerValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Number", 0, &readPrimitives->numberValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Datetime", 0, &readPrimitives->datetimeValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "String", 0, &readPrimitives->stringValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Text", 0, &readPrimitives->textValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "ThingName", 0, &readPrimitives->thingNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "GroupName", 0, &readPrimitives->groupNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "UserName", 0, &readPrimitives->userNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "MashupName", 0, &readPrimitives->mashupNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "MenuName", 0, &readPrimitives->menuNameValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Imagelink", 0, &readPrimitives->imagelinkValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Hyperlink", 0, &readPrimitives->hyperlinkValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "HTML", 0, &readPrimitives->HTMLValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "XML", 0, &readPrimitives->XMLValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "JSON", 0, &readPrimitives->JSONValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Query", 0, &readPrimitives->queryValue);
	twInfoTable_GetPrimitive(readValues->val.infotable, "Location", 0, &readPrimitives->locationValue);

	/* Check that the values match */
	TEST_ASSERT_EQUAL(0, compareAllPropertyBaseTypePrimitives(readPrimitives, writePrimitives));
	twPrimitive_Delete(readValues);
	/* The members of this struct were already deleted when we deleted readValues */
	TW_FREE(readPrimitives);
	deleteAllPropertyBaseTypePrimitives(writePrimitives);
}

TEST(PropertyWriteIntegration, MultiPropertyWrite) {
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row = NULL;
	twPrimitive *writeValues = NULL;
	allPropertyBaseTypePrimitives *readPrimitives = NULL;
	allPropertyBaseTypePrimitives *writePrimitives = NULL;
	twLocation *location = NULL;

	/* Create primitives containing new non-default property values to be written */
	location = TW_MALLOC(sizeof(twLocation));
	location->latitude = 38.8977;
	location->longitude = 77.0365;
	location->elevation = 410;
	writePrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	writePrimitives->booleanValue = twPrimitive_CreateFromBoolean(TRUE);
	writePrimitives->integerValue = twPrimitive_CreateFromInteger(1);
	writePrimitives->numberValue = twPrimitive_CreateFromNumber(1.0);
	writePrimitives->datetimeValue = twPrimitive_CreateFromCurrentTime();
	writePrimitives->stringValue = twPrimitive_CreateFromString("NewString", TRUE);
	writePrimitives->textValue = twPrimitive_CreateFromString("NewText", TRUE);
	writePrimitives->thingNameValue = twPrimitive_CreateFromString("NewThingName", TRUE);
	writePrimitives->groupNameValue = twPrimitive_CreateFromString("NewGroupName", TRUE);
	writePrimitives->userNameValue = twPrimitive_CreateFromString("NewUserName", TRUE);
	writePrimitives->mashupNameValue = twPrimitive_CreateFromString("NewMashupName", TRUE);
	writePrimitives->menuNameValue = twPrimitive_CreateFromString("NewMenuName", TRUE);
	writePrimitives->imagelinkValue = twPrimitive_CreateFromString("http://NewImagelink.net", TRUE);
	writePrimitives->hyperlinkValue = twPrimitive_CreateFromString("http://NewHyperlink.net", TRUE);
	writePrimitives->HTMLValue = twPrimitive_CreateFromString("<html><body>NewHTML</body></html>", TRUE);
	writePrimitives->XMLValue = twPrimitive_CreateFromString("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?><xml><body>NewXML</body></xml>", TRUE);
	writePrimitives->JSONValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->queryValue = twPrimitive_CreateFromString("{\"number\":1,\"string\":\"new\"}", TRUE);
	writePrimitives->locationValue = twPrimitive_CreateFromLocation(location);
	TW_FREE(location);

	writePrimitives->booleanValue->type = TW_BOOLEAN;
	writePrimitives->integerValue->type = TW_INTEGER;
	writePrimitives->numberValue->type = TW_NUMBER;
	writePrimitives->datetimeValue->type = TW_DATETIME;
	writePrimitives->stringValue->type = TW_STRING;
	writePrimitives->textValue->type = TW_TEXT;
	writePrimitives->thingNameValue->type = TW_THINGNAME;
	writePrimitives->groupNameValue->type = TW_GROUPNAME;
	writePrimitives->userNameValue->type = TW_USERNAME;
	writePrimitives->mashupNameValue->type = TW_MASHUPNAME;
	writePrimitives->menuNameValue->type = TW_MENUNAME;
	writePrimitives->imagelinkValue->type = TW_IMAGELINK;
	writePrimitives->hyperlinkValue->type = TW_HYPERLINK;
	writePrimitives->HTMLValue->type = TW_HTML;
	writePrimitives->XMLValue->type = TW_XML;
	writePrimitives->JSONValue->type = TW_JSON;
	writePrimitives->queryValue->type = TW_QUERY;
	writePrimitives->locationValue->type = TW_LOCATION;

	/* Create an info table row conforming to the previously created data shape */
	row = twInfoTableRow_Create(twPrimitive_FullCopy(writePrimitives->booleanValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->integerValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->numberValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->datetimeValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->stringValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->textValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->thingNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->groupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->userNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->mashupNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->menuNameValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->imagelinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->hyperlinkValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->HTMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->XMLValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->JSONValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->queryValue));
	twInfoTableRow_AddEntry(row, twPrimitive_FullCopy(writePrimitives->locationValue));

	/* Add the newly populated info table row to an info table */
	ds = twDataShape_Create(twDataShapeEntry_Create("AlwaysPush_Boolean", NULL, TW_BOOLEAN));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Integer", NULL, TW_INTEGER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Number", NULL, TW_NUMBER));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Datetime", NULL, TW_DATETIME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_String", NULL, TW_STRING));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Text", NULL, TW_TEXT));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_ThingName", NULL, TW_THINGNAME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_GroupName", NULL, TW_GROUPNAME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_UserName", NULL, TW_USERNAME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_MashupName", NULL, TW_MASHUPNAME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_MenuName", NULL, TW_MENUNAME));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Imagelink", NULL, TW_IMAGELINK));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Hyperlink", NULL, TW_HYPERLINK));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_HTML", NULL, TW_HTML));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_XML", NULL, TW_XML));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_JSON", NULL, TW_JSON));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Query", NULL, TW_QUERY));
	twDataShape_AddEntry(ds, twDataShapeEntry_Create("AlwaysPush_Location", NULL, TW_LOCATION));
	it = twInfoTable_Create(ds);
	twInfoTable_AddRow(it, row);

	/* Create a primitive from the newly created info table */
	writeValues = twPrimitive_CreateFromInfoTable(it);
	twInfoTable_Delete(it);

	/* Write the new values to the AlwaysPush_InfoTable property */
	TEST_ASSERT_EQUAL(TW_OK, twApi_WriteProperty(TW_THING, thingNamePropertyWriteIntegration, "*", writeValues, PROPERTY_WRITE_TIMEOUT, FALSE));
	twPrimitive_Delete(writeValues);
	/* Read back the values */
	readPrimitives = TW_MALLOC(sizeof(allPropertyBaseTypePrimitives));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Boolean", &readPrimitives->booleanValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Integer", &readPrimitives->integerValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Number", &readPrimitives->numberValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Datetime", &readPrimitives->datetimeValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_String", &readPrimitives->stringValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Text", &readPrimitives->textValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_ThingName", &readPrimitives->thingNameValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_GroupName", &readPrimitives->groupNameValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_UserName", &readPrimitives->userNameValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_MashupName", &readPrimitives->mashupNameValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_MenuName", &readPrimitives->menuNameValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Imagelink", &readPrimitives->imagelinkValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Hyperlink", &readPrimitives->hyperlinkValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_HTML", &readPrimitives->HTMLValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_XML", &readPrimitives->XMLValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_JSON", &readPrimitives->JSONValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Query", &readPrimitives->queryValue, PROPERTY_READ_TIMEOUT, FALSE));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_Location", &readPrimitives->locationValue, PROPERTY_READ_TIMEOUT, FALSE));

	TEST_ASSERT_EQUAL(0, compareAllPropertyBaseTypePrimitives(readPrimitives, writePrimitives));
	deleteAllPropertyBaseTypePrimitives(readPrimitives);
	deleteAllPropertyBaseTypePrimitives(writePrimitives);
}

TEST(PropertyWriteIntegration, JSONObjectPropertyWrite) {
	cJSON *json = NULL;
	twInfoTable *it = NULL;
	twPrimitive *writeValue = NULL;
	twPrimitive *readValue = NULL;

	json = cJSON_Parse("{\"AlwaysPush_String\":\"NewString\"}");
	it = twInfoTable_CreateFromJson(json, NULL);
	cJSON_Delete(json);
	writeValue = twPrimitive_CreateFromInfoTable(it);
	twInfoTable_Delete(it);

	/* Write the new value to the AlwaysPush_String property */
	TEST_ASSERT_EQUAL(TW_OK, twApi_WriteProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_String", writeValue, PROPERTY_WRITE_TIMEOUT, FALSE));
	twPrimitive_Delete(writeValue);
	/* Read back the values */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, thingNamePropertyWriteIntegration, "AlwaysPush_String", &readValue, PROPERTY_READ_TIMEOUT, FALSE));

	TEST_ASSERT_EQUAL_STRING("NewString", readValue->val.bytes.data);
	twPrimitive_Delete(readValue);
}
