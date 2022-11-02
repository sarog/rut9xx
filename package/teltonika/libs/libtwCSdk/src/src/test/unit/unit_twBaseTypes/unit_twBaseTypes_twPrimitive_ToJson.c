/*
 * Copyright (c) 2017 PTC, Inc. All rights reserved.
 */
#include <stddef.h>

#include "cJSON.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"

/* Test variables */

static cJSON * json = NULL;
static twPrimitive * primitive = NULL;

/* Tests */

TEST_GROUP(unit_twBaseTypes_twPrimitive_ToJson);

TEST_SETUP(unit_twBaseTypes_twPrimitive_ToJson) {
    eatLogs();
    json = NULL;
    primitive = NULL;
}

TEST_TEAR_DOWN(unit_twBaseTypes_twPrimitive_ToJson) {
    if (json) {
        cJSON_Delete(json);
        json = NULL;
    }
    if (primitive) {
        twPrimitive_Delete(primitive);
        primitive = NULL;
    }
}

TEST_GROUP_RUNNER(unit_twBaseTypes_twPrimitive_ToJson) {
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_ErrorCodes);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Nothing);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Json);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Query);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Other);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Blob);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Variant);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Number);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Integer);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Boolean);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Datetime);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Timespan);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Location);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Infotable);
    RUN_TEST_CASE(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_default);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_ErrorCodes) {
    primitive = twPrimitive_Create();
    TEST_ASSERT_NULL(twPrimitive_ToJson(NULL, primitive, NULL));
    TEST_ASSERT_NULL(twPrimitive_ToJson(NULL, NULL, NULL));
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson) {
    cJSON * element = NULL;

    primitive = twPrimitive_Create();
    primitive->typeFamily = TW_UNKNOWN_TYPE;
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_NULL, element->type);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Nothing) {
    cJSON * element = NULL;

    primitive = twPrimitive_Create();
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_NULL, element->type);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Json) {
    cJSON * element = NULL;

    primitive = twPrimitive_CreateFromVariable("42", TW_JSON, TRUE, 0);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(42, element->valueint);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Query) {
    cJSON * element = NULL;

    primitive = twPrimitive_CreateFromVariable("42", TW_QUERY, TRUE, 0);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(42, element->valueint);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_String_Other) {
    cJSON * element = NULL;

    primitive = twPrimitive_CreateFromString("42", TRUE);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_String, element->type);
    TEST_ASSERT_EQUAL_STRING("42", element->valuestring);

}
TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Blob) {
    cJSON * element = NULL;
    unsigned long declen = 50;
    char * dec = (char *) TW_CALLOC(50, 1);

    primitive = twPrimitive_CreateFromBlob("blob", 4, FALSE, TRUE);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    base64_decode(
        (unsigned char *) element->valuestring, strlen(element->valuestring),
        (unsigned char *) dec, &declen
    );
    TEST_ASSERT_EQUAL_STRING("blob", dec);
    TW_FREE(dec);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Variant) {
    cJSON * element = NULL;
    twPrimitive * input = twPrimitive_CreateFromString("sub-primitive", TRUE);
    primitive = twPrimitive_CreateVariant(input);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "baseType");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_String, element->type);
    TEST_ASSERT_EQUAL_STRING("VARIANT", element->valuestring);
    element = cJSON_GetObjectItem(json, "value");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_String, element->type);
    TEST_ASSERT_EQUAL_STRING("sub-primitive", element->valuestring);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Number) {
    cJSON * element = NULL;
    primitive = twPrimitive_CreateFromNumber(123);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(123, element->valuedouble);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Integer) {
    cJSON * element = NULL;
    primitive = twPrimitive_CreateFromInteger(1792);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(1792, element->valueint);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Boolean) {
    char boolean = TRUE;
    cJSON * element = NULL;
    primitive = twPrimitive_CreateFromVariable(&boolean, TW_BOOLEAN, TRUE, 0);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_True, element->type);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Datetime) {
    cJSON * element = NULL;
    primitive = twPrimitive_CreateFromDatetime(123456);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    element = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(123456, element->valuedouble);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Timespan) {
    primitive = twPrimitive_Create();
    primitive->typeFamily = primitive->type = TW_TIMESPAN;
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NULL(json);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Location) {
    cJSON * element = NULL;
    twLocation loc = {12.34, 56.78, 90};
    primitive = twPrimitive_CreateFromLocation(&loc);
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    json = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(json);

    element = cJSON_GetObjectItem(json, "latitude");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(12.34, element->valuedouble);

    element = cJSON_GetObjectItem(json, "longitude");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(56.78, element->valuedouble);

    element = cJSON_GetObjectItem(json, "elevation");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Number, element->type);
    TEST_ASSERT_EQUAL(90, element->valuedouble);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_Infotable) {
    struct twInfoTable * infotable = twInfoTable_CreateFromInteger("foo", 409);
    cJSON * element = NULL;
    char * out = NULL;

    primitive = twPrimitive_CreateFromInfoTable(infotable);
    twInfoTable_Delete(infotable);

    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NOT_NULL(json);
    json = cJSON_GetObjectItem(json, "foo");
    TEST_ASSERT_NOT_NULL(json);

    element = cJSON_GetObjectItem(json, "rows");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Array, element->type);
    out = cJSON_Print(element);
    TEST_ASSERT_EQUAL_STRING("[{\n\t\t\"foo\":\t409\n\t}]", out);
    TW_FREE(out);

    element = cJSON_GetObjectItem(json, "datashape");
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL(cJSON_Object, element->type);
}

TEST(unit_twBaseTypes_twPrimitive_ToJson, twPrimitive_ToJson_default) {
    primitive = twPrimitive_Create();
    primitive->typeFamily = primitive->type = TW_UNKNOWN_TYPE;
    json = twPrimitive_ToJson("foo", primitive, NULL);
    TEST_ASSERT_NULL(json);
}
