#include <twBaseTypes.h>
#include "twExt.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twMacros.h"

char * extUseThingName = "ExtUseExample";

TEST_GROUP(ExtUseExampleIntegration);

void test_ExtUseExampleIntegrationAppKey_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(ExtUseExampleIntegration) {
	int res;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	eatLogs();
	twLogger_SetLevel(TW_ERROR);
	/* delete any previous message store binaries */
	res=twDirectory_DeleteFile(SUBSCRIBED_PROPERTY_LOCATION_FILE);

	/* set offline msg store file */
	twcfg_pointer->offline_msg_store_dir = SUBSCRIBED_PROPERTY_LOCATION;

	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = INTEGRATION_TEST_SMALL_DELAY;
	twcfg_pointer->connect_retry_interval = INTEGRATION_TEST_SMALL_DELAY;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, (uint16_t)TW_PORT, TW_URI, test_ExtUseExampleIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE,
											  MESSAGE_CHUNK_SIZE, TRUE));
	/* set connection params*/
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twExt_Start(1000, TW_THREADING_MULTI, 5);

	{
		char buffer[256];
		char* extDirectory = twGetPreferedExtensionLoadingDirectory();
		TEST_ASSERT_NOT_NULL(extDirectory);
		snprintf(buffer,256,"TWXLIB=%s/",extDirectory);
		TW_FREE(extDirectory);
		putenv(buffer);
	}

	/* setup internal entity */

	TEST_ASSERT_NOT_NULL(twExt_LoadExtensionLibrary("libsimpleext"));
	TEST_ASSERT_NOT_NULL(twExt_LoadExtensionLibrary("libwarehouseext"));
	twExt_CreateThingFromTemplate(extUseThingName, "WarehouseTemplate", "SimpleShape", "AddressShape","InventoryShape",NULL);
	TEST_ASSERT_EQUAL(TW_OK,twApi_BindThing(extUseThingName));

	/* connect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(5000, 3));

	/* setup entities */
	TEST_ASSERT_TRUE(importEntityFileFromEtc("Entities_ExtUseExample.xml"));
}

TEST_TEAR_DOWN(ExtUseExampleIntegration) {
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	twApi_UnbindThing(extUseThingName);
	TEST_ASSERT_EQUAL(TW_OK, twExt_Stop());
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Test is ending."));
	TEST_ASSERT_FALSE(twApi_isConnected());
	twApi_Delete();
}

TEST_GROUP_RUNNER(ExtUseExampleIntegration) {
	RUN_TEST_CASE(ExtUseExampleIntegration, testAddressAndInventoryShapeVerifyProperties);
	RUN_TEST_CASE(ExtUseExampleIntegration, testSimpleShapeCreateMessageWorks);
	RUN_TEST_CASE(ExtUseExampleIntegration, testSimpleShapeCounterWorks);
	RUN_TEST_CASE(ExtUseExampleIntegration, testInventoryShapeCurrentInventoryService);
	RUN_TEST_CASE(ExtUseExampleIntegration, testAddressShapeGenerateSalutationService);
	RUN_TEST_CASE(ExtUseExampleIntegration, testDoubleLoadShapeWithNamespace);

}

const char * const arrayExtPropertyNames[] = {"propertyA","propertyB","count","category","firstName","lastName","address","city","state","zip","yearsAtLocation","instanceName","inventory"};
int arrayExtPropertyNamesCount = 13;
enum BaseType arrayExtPrimitiveTypes[] = {TW_STRING,TW_NUMBER,TW_NUMBER,TW_STRING,TW_STRING,TW_STRING,TW_STRING,TW_STRING,TW_STRING,TW_STRING,TW_NUMBER,TW_STRING,TW_INFOTABLE};

void testAllExtPropertiesExist(twInfoTable* propValues){
	int index;
	twPrimitive* value;
	char buffer[255];
	for(index=0;index<arrayExtPropertyNamesCount;index++){
		snprintf(buffer,255,"%s is missing.",arrayExtPropertyNames[index]);
		TEST_ASSERT_EQUAL_MESSAGE(TW_OK,twInfoTable_GetPrimitive(propValues,arrayExtPropertyNames[index],0,&value),buffer);
		TEST_ASSERT_NOT_NULL(value);
		TEST_ASSERT_EQUAL(arrayExtPrimitiveTypes[index],value->type);
	}
}

/**
 * Test Plan: Verify properties provided by address shape exists, are of the right type and can be written and read from.
 */
TEST(ExtUseExampleIntegration, testAddressAndInventoryShapeVerifyProperties) {
	twInfoTable* itPropertyValues1 = NULL;
	twInfoTable* itPropertyValues3 = NULL;
	twPrimitive* twPrimitive1;
	twInfoTable* result_it,*params_it;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	/* Get the current property values */
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)extUseThingName, "GetPropertyValues", NULL, &itPropertyValues1, 5000, TRUE));

	testAllExtPropertiesExist(itPropertyValues1);
	twInfoTable_Delete(itPropertyValues1);

	/* Update property Values */
	TW_SET_PROPERTY(extUseThingName,"propertyA",TW_MAKE_STRING("testPropA"));
	TW_SET_PROPERTY(extUseThingName,"propertyB",TW_MAKE_NUMBER(12345));
	TW_SET_PROPERTY(extUseThingName,"category",TW_MAKE_STRING("testA"));
	TW_SET_PROPERTY(extUseThingName,"firstName",TW_MAKE_STRING("Testy"));
	TW_SET_PROPERTY(extUseThingName,"lastName",TW_MAKE_STRING("McTestface"));
	TW_SET_PROPERTY(extUseThingName,"address",TW_MAKE_STRING("1234 Counting Way"));
	TW_SET_PROPERTY(extUseThingName,"city",TW_MAKE_STRING("Deptford"));
	TW_SET_PROPERTY(extUseThingName,"state",TW_MAKE_STRING("NJ"));
	TW_SET_PROPERTY(extUseThingName,"zip",TW_MAKE_STRING("08093"));
	TW_SET_PROPERTY(extUseThingName,"yearsAtLocation",TW_MAKE_NUMBER(5));
	TW_SET_PROPERTY(extUseThingName,"instanceName",TW_MAKE_STRING("testInstance"));
	TW_PUSH_PROPERTIES_FOR(extUseThingName,TW_PUSH_CONNECT_FORCE);

	/* Read them back */
	TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)extUseThingName, "GetPropertyValues", NULL, &itPropertyValues3, 5000, TRUE));
	TEST_ASSERT_NOT_NULL(itPropertyValues3);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"propertyB",0,&twPrimitive1));
	TEST_ASSERT_EQUAL(12345,twPrimitive1->val.number);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"propertyA",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("testPropA",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"category",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("testA",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"firstName",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("Testy",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"lastName",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("McTestface",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"address",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("1234 Counting Way",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"city",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("Deptford",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"state",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("NJ",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"zip",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("08093",twPrimitive1->val.bytes.data);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"yearsAtLocation",0,&twPrimitive1));
	TEST_ASSERT_EQUAL(5,twPrimitive1->val.number);


	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetPrimitive(itPropertyValues3,"instanceName",0,&twPrimitive1));
	TEST_ASSERT_EQUAL_STRING("testInstance",twPrimitive1->val.bytes.data);

	twInfoTable_Delete(itPropertyValues3);

}


/**
 * Test Plan: Verify that the count property introduced by simple shape is readable and increasing.
 */
TEST(ExtUseExampleIntegration, testSimpleShapeCounterWorks) {
	int index = 0;
	double firstValue,currentValue;
	twInfoTable* itPropertyValues1 = NULL;
	twInfoTable* itPropertyValues2 = NULL;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}
	/* Get current count */
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)extUseThingName, "GetPropertyValues", NULL, &itPropertyValues1, 5000, TRUE));

	twInfoTable_GetNumber(itPropertyValues1,"count",0,&firstValue);
	/* Wait for it to change */
	for(index=0;index < 10;index++){
		TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)extUseThingName, "GetPropertyValues", NULL, &itPropertyValues2, 5000, TRUE));
		twInfoTable_GetNumber(itPropertyValues2,"count",0,&currentValue);
		if(currentValue>firstValue)
			break;
		twSleepMsec(1000);
	}

	TEST_ASSERT_TRUE(currentValue>firstValue);
	twInfoTable_Delete(itPropertyValues1);
	twInfoTable_Delete(itPropertyValues2);

}

/**
 * Test Plan: Verify the createMessage() service exists by calling it after setting property A,B and passing in a
 * single parameter. Validate the returned string is what is expected.
 */
TEST(ExtUseExampleIntegration, testSimpleShapeCreateMessageWorks) {
	twInfoTable* itParamValues,*itReturnValue;
	char* returnValue;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}
	TW_SET_PROPERTY(extUseThingName,"propertyA",TW_MAKE_STRING("Hey, Hi Hello"));
	TW_SET_PROPERTY(extUseThingName,"propertyB",TW_MAKE_NUMBER(42));
	TW_PUSH_PROPERTIES_FOR(extUseThingName,TW_PUSH_CONNECT_FORCE);

	itParamValues = TW_MAKE_IT(
		TW_MAKE_DATASHAPE(
				TW_SHAPE_NAME_NONE,
				TW_DS_ENTRY("parameter1", TW_NO_DESCRIPTION, TW_STRING)
		),
		TW_IT_ROW(
				TW_MAKE_STRING("Its Parameter Time")
		)
	);

	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)extUseThingName, "createMessage", itParamValues, &itReturnValue, 5000, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetString(itReturnValue,"result",0,&returnValue));

	TEST_ASSERT_EQUAL_STRING("This is a message combining propertyA=Hey, Hi Hello  and propertyB=42.000000 and parameter1=Its Parameter Time",returnValue);
	twInfoTable_Delete(itReturnValue);


}

/**
 * Test Plan: Execute Address shape service generateSalutation() service, verify return string is as expected.
 */
TEST(ExtUseExampleIntegration, testAddressShapeGenerateSalutationService) {
	twInfoTable* itParamValues,*itReturnValue;
	char* returnValue1;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	TW_SET_PROPERTY(extUseThingName,"firstName",TW_MAKE_STRING("George"));
	TW_SET_PROPERTY(extUseThingName,"lastName",TW_MAKE_STRING("Washington"));
	TW_PUSH_PROPERTIES_FOR(extUseThingName,TW_PUSH_CONNECT_FORCE);

	itParamValues = TW_MAKE_IT(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("title", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING("President")
			)
	);

	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)extUseThingName, "generateSalutation", itParamValues, &itReturnValue, 5000, TRUE));
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetString(itReturnValue,"result",0,&returnValue1));

	TEST_ASSERT_EQUAL_STRING("Dear President George Washington",returnValue1);
	TW_FREE(returnValue1);
	twInfoTable_Delete(itReturnValue);


}

/**
 * Test Plan: Execute Inventory shape service currentInventory() service, verify return string is as expected.
 */
TEST(ExtUseExampleIntegration, testInventoryShapeCurrentInventoryService) {
	twInfoTable *itInitalInventory;
	twInfoTable *itReturnValue1;
	twInfoTable *itReturnValue2;
	twInfoTable *itParams;

	twPrimitive* returnValue1;
	char* invNameString;
	double price;
	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	/* Get the default inventory */
	TEST_ASSERT_EQUAL(TW_OK, twApi_InvokeService(TW_THING, (char*)extUseThingName, "currentInventory", NULL, &itInitalInventory, 5000, TRUE));
	TEST_ASSERT_EQUAL(1,itInitalInventory->rows->count);
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetString(itInitalInventory,"description",0,&invNameString));
	TEST_ASSERT_EQUAL_STRING("Hat",invNameString);
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetNumber(itInitalInventory,"price",0,&price));
	TEST_ASSERT_EQUAL(5,price);

	/* Add a new row */
	itParams = TW_MAKE_IT(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("description", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("price", TW_NO_DESCRIPTION, TW_NUMBER)
			),
			TW_IT_ROW(
					TW_MAKE_STRING("TestItem"),TW_MAKE_NUMBER(10)
			)
	);

	TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)extUseThingName, "addProduct", itParams, &itReturnValue1, 5000, TRUE));




	/* Get the updated inventory */
	TEST_ASSERT_EQUAL(TW_OK,invokeServiceWithRetries(TW_THING, (char*)extUseThingName, "currentInventory", NULL, &itReturnValue2, 5000, TRUE));
	TEST_ASSERT_EQUAL(2,itReturnValue2->rows->count);

	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetString(itReturnValue2,"description",1,&invNameString));
	TEST_ASSERT_EQUAL_STRING("TestItem",invNameString);
	TEST_ASSERT_EQUAL(TW_OK,twInfoTable_GetNumber(itReturnValue2,"price",1,&price));
	TEST_ASSERT_EQUAL(10,price);
	twInfoTable_Delete(itInitalInventory);
	twInfoTable_Delete(itReturnValue1);
	twInfoTable_Delete(itReturnValue2);
}

/* Test Plan: Load the address shape a second time with the namespace test. */
TEST(ExtUseExampleIntegration, testDoubleLoadShapeWithNamespace) {
	twInfoTable* itPropertyValues1 = NULL;
	twPrimitive* twPrimitive1;

	if(!isShared()) {
		/* Load Statically
		 * This test is not used when statically linking
		 */
		return;
	}

	TEST_ASSERT_EQUAL(TW_OK,twExt_AddEdgeThingShape((char*)extUseThingName,"AddressShape","test"));

}