/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for twApi_PushSubscribedProperties()
 */

#include "twApi.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Internal function definition for testing */
twSubscribedProperty * twSubscribedProperty_CreateFromStream(twStream * s);

/* Internal structure used to observe property persistence */
struct twApiUnitTests_PersistedValues {
	struct twStream * s;
	int32_t streamedEntityCount;
} _apiUnitTest_persistedValues;

TEST_GROUP(unit_twApi_PushSubscribedProperties);

TEST_SETUP(unit_twApi_PushSubscribedProperties) {
	eatLogs();
	/* setup stream observer for multiple entity persistence tests */
	if (!_apiUnitTest_persistedValues.s) {
		_apiUnitTest_persistedValues.s = twStream_Create();
	}
}

TEST_TEAR_DOWN(unit_twApi_PushSubscribedProperties) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());
	twStream_Delete (_apiUnitTest_persistedValues.s);
	_apiUnitTest_persistedValues.s = 0;
	_apiUnitTest_persistedValues.streamedEntityCount = 0;
}

TEST_GROUP_RUNNER(unit_twApi_PushSubscribedProperties) {
	RUN_TEST_CASE(unit_twApi_PushSubscribedProperties, setSubscribedPropertyVTQWithNullApi);
	RUN_TEST_CASE(unit_twApi_PushSubscribedProperties, setSubscribedPropertyWithNullApi);
	RUN_TEST_CASE(unit_twApi_PushSubscribedProperties, setSubscribedPropertiesForSingleEntity);
	RUN_TEST_CASE(unit_twApi_PushSubscribedProperties, setSubscribedPropertiesForAllEntities);
	RUN_TEST_CASE(unit_twApi_PushSubscribedProperties, clearSpmCurrentValues);
}

/**
 * Test Plan: Set a subscribed property with a NULL API
 */
TEST(unit_twApi_PushSubscribedProperties, setSubscribedPropertyVTQWithNullApi) {
	twPrimitive *value = NULL;
	value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetSubscribedPropertyVTQ(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, value, twGetSystemTime(TRUE), "GOOD", FALSE, FALSE));
}

/**
 * Test Plan: Set a subscribed property with a NULL API
 */
TEST(unit_twApi_PushSubscribedProperties, setSubscribedPropertyWithNullApi) {
	twPrimitive *value = NULL;
	value = twPrimitive_CreateFromInteger(TEST_PRIMITIVE_INT_VALUE);
	TEST_ASSERT_EQUAL(TW_NULL_OR_INVALID_API_SINGLETON, twApi_SetSubscribedProperty(TEST_ENTITY_NAME, TEST_PROPERTY_NAME, value, FALSE, FALSE));
}

int stub_ApiUnitTests_twSubscribedProperty_ToStream (struct twSubscribedProperty * p, struct twStream * s)	{
	if (twSubscribedProperty_ToStream (p, _apiUnitTest_persistedValues.s) == TW_OK) {
		_apiUnitTest_persistedValues.streamedEntityCount++;
		return twSubscribedProperty_ToStream (p, s);
	}

	return TW_ERROR;
}

/**
 * Test Plan: This test assumes that there is no platform connection, and so all property values that are pushed end
 * up in the offline property value cache. A stub stream handler is installed so that the content of the
 * streamed property values may be examined.
 */
TEST(unit_twApi_PushSubscribedProperties, setSubscribedPropertiesForSingleEntity) {
	twSubscribedProperty * p1 = NULL;

	/* Enable the (file backed) offline message store. */
	TEST_ASSERT_EQUAL (TW_OK, enableOfflineMsgStore (TRUE, TRUE));

	/* Initialize the API */
	TEST_ASSERT_EQUAL (TW_OK, twApi_Initialize (TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                            MESSAGE_CHUNK_SIZE, TRUE));

	/* Register property for entity 1 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty(TW_THING, TEST_MULTIPLE_ENTITY_NAME_1, TEST_PROPERTY_NAME,
	                                                 TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));

	/* Register property for entity 2 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, TEST_MULTIPLE_ENTITY_NAME_2, TEST_PROPERTY_NAME,
	                                                  TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));

	/* Queue an update for entity 1 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_MULTIPLE_ENTITY_NAME_1, TEST_PROPERTY_NAME,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Queue an update for entity 2 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_MULTIPLE_ENTITY_NAME_2, TEST_PROPERTY_NAME,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Stub in the unit test stream handler */
	twApi_stub->twSubscribedProperty_ToStream = stub_ApiUnitTests_twSubscribedProperty_ToStream;

	/* Push updates for entity 2 only, which will invoke the stub stream handler */
	TEST_ASSERT_EQUAL (TW_OK, twApi_PushSubscribedProperties (TEST_MULTIPLE_ENTITY_NAME_2, FALSE));
	TEST_ASSERT_EQUAL_INT32 (1, _apiUnitTest_persistedValues.streamedEntityCount);

	/* Examine the stream to validate that the appropriate entity/properties were pushed */
	twStream_Reset (_apiUnitTest_persistedValues.s);
	p1 = twSubscribedProperty_CreateFromStream (_apiUnitTest_persistedValues.s);
	TEST_ASSERT_EQUAL_STRING (p1->entity, TEST_MULTIPLE_ENTITY_NAME_2);
	TEST_ASSERT_EQUAL_STRING (p1->prop->name, TEST_PROPERTY_NAME);
	TEST_ASSERT_EQUAL (p1->prop->value->type, TW_INTEGER);
	TEST_ASSERT_EQUAL_INT32 (TEST_PRIMITIVE_INT_VALUE, p1->prop->value->val.integer);
	twSubscribedProperty_Delete(p1);
}

/**
 * Test Plan: This test assumes that there is no platform connection, and so all property values that are pushed end
 * up in the offline property value cache. A stub stream handler is installed so that the content of the
 * streamed property values may be examined.
 */

TEST(unit_twApi_PushSubscribedProperties, setSubscribedPropertiesForAllEntities) {
	twSubscribedProperty * p1 = NULL;
	twSubscribedProperty * p2 = NULL;

	/* Enable the (file backed) offline message store. */
	TEST_ASSERT_EQUAL (TW_OK, enableOfflineMsgStore (TRUE, TRUE));

	/* Initialize the API */
	TEST_ASSERT_EQUAL (TW_OK, twApi_Initialize (TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                            MESSAGE_CHUNK_SIZE, TRUE));

	/* Register property for entity 1 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, TEST_MULTIPLE_ENTITY_NAME_1, TEST_PROPERTY_NAME,
	                                                  TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));

	/* Register property for entity 2 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, TEST_MULTIPLE_ENTITY_NAME_2, TEST_PROPERTY_NAME,
	                                                  TW_INTEGER, NULL, "ALWAYS", 0, &doNothing, NULL));

	/* Queue an update for entity 1 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_MULTIPLE_ENTITY_NAME_1, TEST_PROPERTY_NAME,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Queue an update for entity 2 */
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (TEST_MULTIPLE_ENTITY_NAME_2, TEST_PROPERTY_NAME,
	                                                          twPrimitive_CreateFromInteger (TEST_PRIMITIVE_INT_VALUE),
	                                                          twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));

	/* Stub in the unit test stream handler */
	twApi_stub->twSubscribedProperty_ToStream = stub_ApiUnitTests_twSubscribedProperty_ToStream;

	/* Push updates for all entities by specifying NULL for the entity name */
	TEST_ASSERT_EQUAL (TW_OK, twApi_PushSubscribedProperties (NULL, FALSE));
	TEST_ASSERT_EQUAL_INT32 (2, _apiUnitTest_persistedValues.streamedEntityCount);

	/* Examine the stream to validate that the appropriate entity/properties were pushed */
	twStream_Reset (_apiUnitTest_persistedValues.s);
	/* You can't depend on the sequence properties will come out of the stream since entity name as stored in a twDict
	 * which does not retain the sequence of insertion. */
	p1 = twSubscribedProperty_CreateFromStream (_apiUnitTest_persistedValues.s);
	if(strcmp(p1->entity, TEST_MULTIPLE_ENTITY_NAME_1)==0) {
		TEST_ASSERT_EQUAL_STRING (p1->entity, TEST_MULTIPLE_ENTITY_NAME_1);
		TEST_ASSERT_EQUAL_STRING (p1->prop->name, TEST_PROPERTY_NAME);
		TEST_ASSERT_EQUAL (p1->prop->value->type, TW_INTEGER);
		TEST_ASSERT_EQUAL_INT32 (TEST_PRIMITIVE_INT_VALUE, p1->prop->value->val.integer);
	} else {
		TEST_ASSERT_EQUAL_STRING (p1->entity, TEST_MULTIPLE_ENTITY_NAME_2);
		TEST_ASSERT_EQUAL_STRING (p1->prop->name, TEST_PROPERTY_NAME);
		TEST_ASSERT_EQUAL (p1->prop->value->type, TW_INTEGER);
		TEST_ASSERT_EQUAL_INT32 (TEST_PRIMITIVE_INT_VALUE, p1->prop->value->val.integer);
	}

	p2 = twSubscribedProperty_CreateFromStream (_apiUnitTest_persistedValues.s);
	if(strcmp(p2->entity, TEST_MULTIPLE_ENTITY_NAME_1)==0) {
		TEST_ASSERT_EQUAL_STRING (p2->entity, TEST_MULTIPLE_ENTITY_NAME_1);
		TEST_ASSERT_EQUAL_STRING (p2->prop->name, TEST_PROPERTY_NAME);
		TEST_ASSERT_EQUAL (p2->prop->value->type, TW_INTEGER);
		TEST_ASSERT_EQUAL_INT32 (TEST_PRIMITIVE_INT_VALUE, p1->prop->value->val.integer);
	} else {
		TEST_ASSERT_EQUAL_STRING (p2->entity, TEST_MULTIPLE_ENTITY_NAME_2);
		TEST_ASSERT_EQUAL_STRING (p2->prop->name, TEST_PROPERTY_NAME);
		TEST_ASSERT_EQUAL (p2->prop->value->type, TW_INTEGER);
		TEST_ASSERT_EQUAL_INT32 (TEST_PRIMITIVE_INT_VALUE, p2->prop->value->val.integer);
	}
	twSubscribedProperty_Delete(p1);
	twSubscribedProperty_Delete(p2);
}

/**
 * Test Plan: Set some subscribed property values and then clear them
 */
TEST(unit_twApi_PushSubscribedProperties, clearSpmCurrentValues) {
	char *thingName = "clearSpmCurrentValuesTest";
	char *propertyName = "testProperty";
	twSubscribedPropsMgr *spm = NULL;
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE,
	                                          MESSAGE_CHUNK_SIZE, TRUE));
	TEST_ASSERT_EQUAL (TW_OK, twApi_RegisterProperty (TW_THING, thingName, propertyName, TW_INTEGER, NULL, "VALUE", 0, &doNothing, NULL));
	TEST_ASSERT_EQUAL (TW_OK, twApi_SetSubscribedPropertyVTQ (thingName, propertyName, twPrimitive_CreateFromInteger(1), twGetSystemTime (TRUE), "GOOD", FALSE, FALSE));
	spm = twSubscribedPropsMgr_Get();
	TEST_ASSERT_NOT_NULL(spm);
	TEST_ASSERT_NOT_NULL(spm->currentValues);
	TEST_ASSERT_EQUAL(1, twDict_GetCount(spm->currentValues));
	TEST_ASSERT_EQUAL(TW_OK, twApi_ClearSubscribedPropertyCurrentValues());
	TEST_ASSERT_EQUAL(0, twDict_GetCount(spm->currentValues));
}