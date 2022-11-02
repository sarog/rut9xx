/*
 * Copyright 2016, PTC, Inc.
 *
 */


#include "twBaseTypes.h"
#include "warehouse.h"
#include "twProperties.h"
#include "twSubscribedProps.h"
#include "twShapes.h"
#include "twMacros.h"
#include "twConstants.h"
#include "twPrimitiveUtils.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"

twList* twExt_GetChangeListenersList();

TEST_GROUP(unit_twExt_RegisterStandardProperty);

TEST_SETUP(unit_twExt_RegisterStandardProperty){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twExt_RegisterStandardProperty){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twExt_RegisterStandardProperty){
	RUN_TEST_CASE(unit_twExt_RegisterStandardProperty, test_AddPropertyWithNamespace);
	RUN_TEST_CASE(unit_twExt_RegisterStandardProperty, test_DeclareShapeMacro);
	RUN_TEST_CASE(unit_twExt_RegisterStandardProperty, test_PropertyDeclarationMacro);
	RUN_TEST_CASE(unit_twExt_RegisterStandardProperty, test_PropertyLongDeclaration);
	RUN_TEST_CASE(unit_twExt_RegisterStandardProperty, test_setAndGetPropertyMacros);
}

char* aspectStringValueOfProperty(twPropertyDef* propDef,char* aspectName){
	cJSON* currentAspect;
	cJSON* inventoryAspects = propDef->aspects;
	TEST_ASSERT_NOT_NULL(inventoryAspects);
	currentAspect = inventoryAspects->child;
	TEST_ASSERT_NOT_NULL(currentAspect);
	while( NULL != currentAspect ){
		if(strcmp(aspectName,currentAspect->string) == 0){
			return currentAspect->valuestring;
		}
		currentAspect = currentAspect->next;
	}
	return NULL;
}

double aspectDoubleValueOfProperty(twPropertyDef* propDef,char* aspectName){
	cJSON* inventoryAspects = propDef->aspects;
	cJSON* currentAspect;
	TEST_ASSERT_NOT_NULL(inventoryAspects);
	currentAspect = inventoryAspects->child;
	TEST_ASSERT_NOT_NULL(currentAspect);
	while( NULL != currentAspect ){
		if(strcmp(aspectName,currentAspect->string) == 0){
			return currentAspect->valuedouble;
		}
		currentAspect = currentAspect->next;
	}
	return -666;
}

/**
 * Test Plan: The TW_DECLARE_SHAPE Macro should create two local variables
 * _tw_thing_name and _tw_shape_name to be used to add properties to.
 * Verify their presence and value after it is used.
 */
TEST(unit_twExt_RegisterStandardProperty,test_DeclareShapeMacro) {
	TW_DECLARE_SHAPE("testThingName", "Test Shape",TW_NO_NAMESPACE);
	TEST_ASSERT_NULL(_tw_thing_namespace);
	TEST_ASSERT_EQUAL_STRING("testThingName",_tw_thing_name);
	TEST_ASSERT_EQUAL_STRING("Test Shape",_tw_shape_name);
}

/**
 * Test Plan: The TW_PROPERTY Macro should declare a property on the
 * Thing declared in the TW_DECLARE_SHAPE macro. Create a STRING, NUMBER,
 * BOOLEAN and INFOTABLE. Verify their existence in the callback map.
 */
TEST(unit_twExt_RegisterStandardProperty,test_PropertyDeclarationMacro){
	twPropertyDef* zipProperty;
	twPropertyDef* yearsAtLocationProperty;
	twPropertyDef* singleProperty;
	twPropertyDef* inventoryProperty;

	/* Setup */
	{
		TW_DECLARE_SHAPE("testThingName", "Test Shape",TW_NO_NAMESPACE);
		TEST_ASSERT_EQUAL_STRING("Test Shape",_tw_shape_name);
		TW_PROPERTY("zip", "The Zipcode", TW_STRING);
		TW_PROPERTY("yearsAtLocation", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_PROPERTY("single", TW_NO_DESCRIPTION, TW_BOOLEAN);
		TW_PROPERTY("inventory", "A list of inventory", TW_INFOTABLE);

		/*Test results*/
		zipProperty = findProperty(_tw_thing_name, "zip");
		TEST_ASSERT_NOT_NULL(zipProperty);
		TEST_ASSERT_EQUAL_STRING("zip",zipProperty->name);
		TEST_ASSERT_EQUAL_STRING("The Zipcode",zipProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_STRING, zipProperty->type);

		yearsAtLocationProperty = findProperty(_tw_thing_name, "yearsAtLocation");
		TEST_ASSERT_NOT_NULL(yearsAtLocationProperty);
		TEST_ASSERT_EQUAL_STRING("yearsAtLocation",yearsAtLocationProperty->name);
		TEST_ASSERT_NULL(yearsAtLocationProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_NUMBER, yearsAtLocationProperty->type);

		singleProperty = findProperty(_tw_thing_name, "single");
		TEST_ASSERT_NOT_NULL(singleProperty);
		TEST_ASSERT_EQUAL_STRING("single",singleProperty->name);
		TEST_ASSERT_NULL(singleProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_BOOLEAN, singleProperty->type);

		inventoryProperty = findProperty(_tw_thing_name, "inventory");
		TEST_ASSERT_NOT_NULL(inventoryProperty);
		TEST_ASSERT_EQUAL_STRING("inventory",inventoryProperty->name);
		TEST_ASSERT_EQUAL_STRING("A list of inventory",inventoryProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_INFOTABLE, inventoryProperty->type);
	}
}

/**
 * Test Plan: The TW_PROPERTY_LONG() Macro should be able to declare more
 * complex properties such as INFOTABLEs and provide a much richer set of settings.
 * Create an infotable with aspects such as datashapes and verify that they are created.
 * Also verify adding of assorted ASPECTS.
 */
TEST(unit_twExt_RegisterStandardProperty,test_PropertyLongDeclaration){
	twPropertyDef* inventoryProperty;
	twPropertyDef* priceProperty;

	{
		TW_DECLARE_SHAPE("testThingName", "Test Shape",TW_NO_NAMESPACE);
		TEST_ASSERT_EQUAL_STRING("Test Shape",_tw_shape_name);
		TW_PROPERTY_LONG("inventory", "A list of inventory", TW_INFOTABLE, TW_PUSH_TYPE_ALWAYS, TW_PUSH_THRESHOLD_NONE);
		TW_PROPERTY("price", "How much it costs", TW_NUMBER);

		TEST_ASSERT_EQUAL_INT(TW_OK,
		                      TW_ADD_STRING_ASPECT("inventory",TW_ASPECT_ISREADONLY,"true")
		);
		TEST_ASSERT_EQUAL_INT(TW_OK,
		                      TW_ADD_STRING_ASPECT("inventory",TW_ASPECT_DATASHAPE,"customDatashape")
		);
		TEST_ASSERT_EQUAL_INT(TW_OK,
		                      TW_ADD_STRING_ASPECT("price",TW_ASPECT_DATACHANGETYPE,TW_PUSH_TYPE_VALUE)
		);
		TEST_ASSERT_EQUAL_INT( TW_OK,
		                       TW_ADD_NUMBER_ASPECT("price",TW_ASPECT_DATACHANGETHRESHOLD,5.0)
		);
		inventoryProperty = findProperty(_tw_thing_name, "inventory");
		TEST_ASSERT_NOT_NULL(inventoryProperty);
		TEST_ASSERT_EQUAL_STRING("inventory",inventoryProperty->name);
		TEST_ASSERT_EQUAL_STRING("A list of inventory",inventoryProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_INFOTABLE,inventoryProperty->type);

		TEST_ASSERT_EQUAL_STRING("customDatashape",aspectStringValueOfProperty(inventoryProperty,TW_ASPECT_DATASHAPE));
		TEST_ASSERT_EQUAL_STRING("true",aspectStringValueOfProperty(inventoryProperty,TW_ASPECT_ISREADONLY));
		TEST_ASSERT_EQUAL_STRING(TW_PUSH_TYPE_ALWAYS,aspectStringValueOfProperty(inventoryProperty,TW_ASPECT_PUSHTYPE));

		priceProperty = findProperty(_tw_thing_name, "price");
		TEST_ASSERT_NOT_NULL(priceProperty);
		TEST_ASSERT_EQUAL_STRING("price",priceProperty->name);
		TEST_ASSERT_EQUAL_STRING("How much it costs",priceProperty->description);
		TEST_ASSERT_EQUAL_INT(TW_NUMBER,priceProperty->type);
	}

	TEST_ASSERT_EQUAL_STRING(TW_PUSH_TYPE_VALUE,aspectStringValueOfProperty(priceProperty,TW_ASPECT_DATACHANGETYPE));
	TEST_ASSERT_EQUAL(5.0,aspectDoubleValueOfProperty(priceProperty,TW_ASPECT_DATACHANGETHRESHOLD));

}

/**
 * Test Plan: Use the Make thing, property and aspect macros to construct a thing and then verify that
 * the data is present in the main callback table.
 */
TEST(unit_twExt_RegisterStandardProperty,test_setAndGetPropertyMacros){
	twList *updateList;
	twSubscribedProperty * inletValveProperty;
	twSubscribedProperty * pressureProperty;
	twSubscribedProperty * BigGiantStringProperty;
	twSubscribedProperty * LocationProperty;
	twEntitySavedProperties query,*match;


	/* Establish a thing */
	{
		TW_MAKE_THING("testThing",TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY("FaultStatus", TW_NO_DESCRIPTION, TW_BOOLEAN);
		TW_PROPERTY("InletValve", TW_NO_DESCRIPTION, TW_BOOLEAN);
		TW_PROPERTY("InletValve", TW_NO_DESCRIPTION, TW_BOOLEAN);
		TW_PROPERTY("Pressure", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_PROPERTY("Temperature", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_PROPERTY("TemperatureLimit", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_ADD_NUMBER_ASPECT("TemperatureLimit", "defaultValue",50.5);
		TW_PROPERTY("TotalFlow", TW_NO_DESCRIPTION, TW_NUMBER);
		TW_PROPERTY("BigGiantString", TW_NO_DESCRIPTION, TW_STRING);
		TW_PROPERTY("Location", TW_NO_DESCRIPTION, TW_LOCATION);
	}

	/* Set some properties, watch the queue grow as the changes accumulate.*/
	TW_SET_PROPERTY("testThing", "InletValve", TW_MAKE_BOOL(TRUE));

	query.name = "testThing";
	TEST_ASSERT_EQUAL(TW_OK,twDict_Find( twSubscribedPropsMgr_Get()->savedValues, &query,(void **)&match));
	TEST_ASSERT_NOT_NULL(match);
	updateList = match->props;

	TEST_ASSERT_NOT_NULL(updateList);
	TEST_ASSERT_EQUAL(1,updateList->count);
	TW_SET_PROPERTY("testThing", "Pressure", TW_MAKE_NUMBER(50));
	TEST_ASSERT_EQUAL(2,updateList->count);
	TW_SET_PROPERTY("testThing", "BigGiantString", TW_MAKE_STRING("Lorem ipsum dolor sit amet."));
	TEST_ASSERT_EQUAL(3,updateList->count);
	TW_SET_PROPERTY("testThing", "Location", TW_MAKE_LOC(39.844639, -75.1200447, 17));
	TEST_ASSERT_EQUAL(4,updateList->count);

	/*Verify that the updates in the queue are what is expected*/
	TEST_ASSERT_NOT_NULL(updateList->first);
	inletValveProperty = updateList->first->value;
	TEST_ASSERT_NOT_NULL(inletValveProperty);
	TEST_ASSERT_EQUAL_STRING("InletValve",inletValveProperty->prop->name);
	TEST_ASSERT_TRUE(inletValveProperty->prop->value->val.boolean);

	TEST_ASSERT_NOT_NULL(updateList->first->next);
	pressureProperty = updateList->first->next->value;
	TEST_ASSERT_NOT_NULL(pressureProperty);
	TEST_ASSERT_EQUAL_STRING("Pressure",pressureProperty->prop->name);
	TEST_ASSERT_EQUAL(50,pressureProperty->prop->value->val.number);

	TEST_ASSERT_NOT_NULL(updateList->first->next->next);
	BigGiantStringProperty = updateList->first->next->next->value;
	TEST_ASSERT_NOT_NULL(BigGiantStringProperty);
	TEST_ASSERT_EQUAL_STRING("BigGiantString",BigGiantStringProperty->prop->name);
	TEST_ASSERT_EQUAL_STRING("Lorem ipsum dolor sit amet.",BigGiantStringProperty->prop->value->val.bytes.data);

	TEST_ASSERT_NOT_NULL(updateList->first->next->next->next);
	LocationProperty = updateList->first->next->next->next->value;
	TEST_ASSERT_NOT_NULL(LocationProperty);
	TEST_ASSERT_EQUAL_STRING("Location",LocationProperty->prop->name);

	/*Verify the current property values*/
	TEST_ASSERT_TRUE(TW_GET_PROPERTY("testThing", "InletValve").boolean);
	TEST_ASSERT_TRUE(TW_GET_PROPERTY("testThing", "Pressure").number == 50.0);
	TEST_ASSERT_EQUAL_STRING("Lorem ipsum dolor sit amet.",TW_GET_STRING_PROPERTY("testThing", "BigGiantString"));
	TEST_ASSERT_EQUAL(39.844639,TW_GET_PROPERTY("testThing", "Location").location.latitude );
	TEST_ASSERT_EQUAL(-75.1200447,TW_GET_PROPERTY("testThing", "Location").location.longitude);
	TEST_ASSERT_EQUAL(17,TW_GET_PROPERTY("testThing", "Location").location.elevation );

	/*Verify Kind or Template of this thing*/
	TEST_ASSERT_TRUE(twExt_DoesThingImplementTemplate("testThing", TW_THING_TEMPLATE_GENERIC));

}

/**
 * Test Plan: Create a thing using a shape with a namespace. Verify that the correct
 * property and service naming occur in the resulting thing.
 */
TEST(unit_twExt_RegisterStandardProperty,test_AddPropertyWithNamespace){
	twPropertyDef* testProperty;
	twServiceDef* serviceDef;

	TEST_IGNORE_MESSAGE("This test does not work properly and needs to be rewritten");
	{

		if(!isShared()) {
			return;
		}
		{
			char buffer[256];
			char* extDirectory = twGetPreferedExtensionLoadingDirectory();
			snprintf(buffer,256,"TWXLIB=%s/",extDirectory);
			TW_FREE(extDirectory);
			putenv(buffer);
		}
		TEST_ASSERT_NOT_NULL(twExt_LoadExtensionLibrary("libsimpleext"));
		{
			TW_MAKE_THING("namespaceTestThing", TW_THING_TEMPLATE_GENERIC);
			TEST_ASSERT_NULL(_tw_thing_namespace);
			twExt_AddEdgeThingShape(_tw_thing_name, "AddressShape", "testNamespace");

			/* Now verify that the properties were added with a namespace*/
			testProperty = findProperty(_tw_thing_name, "testNamespace_category");
			TEST_ASSERT_NOT_NULL(testProperty);
			TEST_ASSERT_EQUAL_STRING("testNamespace_category",testProperty->name);
			TEST_ASSERT_EQUAL_STRING("What category of business this is.",testProperty->description);
			TEST_ASSERT_EQUAL_INT(TW_STRING,testProperty->type);

			testProperty = findProperty(_tw_thing_name, "testNamespace_firstName");
			TEST_ASSERT_NOT_NULL(testProperty);
			TEST_ASSERT_EQUAL_STRING("testNamespace_firstName",testProperty->name);
			TEST_ASSERT_EQUAL_STRING("Your First Name",testProperty->description);
			TEST_ASSERT_EQUAL_INT(TW_STRING,testProperty->type);

			testProperty = findProperty(_tw_thing_name, "testNamespace_yearsAtLocation");
			TEST_ASSERT_NOT_NULL(testProperty);
			TEST_ASSERT_EQUAL_STRING("testNamespace_yearsAtLocation",testProperty->name);
			TEST_ASSERT_EQUAL_INT(TW_NUMBER,testProperty->type);

			serviceDef = findService(_tw_thing_name, "testNamespace_generateSalutation");
			TEST_ASSERT_NOT_NULL(serviceDef);
			TEST_ASSERT_EQUAL_STRING("testNamespace_generateSalutation",serviceDef->name);
			TEST_ASSERT_NOT_NULL(serviceDef->inputs);
		}
	}


}