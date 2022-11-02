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

TEST_GROUP(unit_twExt_AddPropertyChangeListener);

TEST_SETUP(unit_twExt_AddPropertyChangeListener){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
TEST_TEAR_DOWN(unit_twExt_AddPropertyChangeListener){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twExt_AddPropertyChangeListener){
	RUN_TEST_CASE(unit_twExt_AddPropertyChangeListener, test_simplePropertyChangeListener);
}

char calledWithCorrectValue = FALSE;
void simplePropertyObserver(const char * entityName, const char * thingName,twPrimitive* newValue){
	if(newValue->val.number == 50){
		calledWithCorrectValue = TRUE;
	}
}

/**
 * Test Plan: Establish a property listener on a thing and then test that it gets called when a property is modifed.
 */
TEST(unit_twExt_AddPropertyChangeListener,test_simplePropertyChangeListener) {

	twList * changeListeners = NULL;

	{
		TW_MAKE_THING("observedThing",TW_THING_TEMPLATE_GENERIC);
		TW_PROPERTY("TotalFlow", TW_NO_DESCRIPTION, TW_NUMBER);
	}
	twExt_AddPropertyChangeListener("observedThing", TW_OBSERVE_ALL_PROPERTIES, simplePropertyObserver);
	changeListeners = twExt_GetChangeListenersList();
	TEST_ASSERT_EQUAL(1,changeListeners->count);

	TW_SET_PROPERTY("observedThing","TotalFlow",TW_MAKE_NUMBER(50));

	twExt_RemovePropertyChangeListener(simplePropertyObserver);
	TEST_ASSERT_EQUAL(0,changeListeners->count);

	TEST_ASSERT_TRUE(calledWithCorrectValue);
}