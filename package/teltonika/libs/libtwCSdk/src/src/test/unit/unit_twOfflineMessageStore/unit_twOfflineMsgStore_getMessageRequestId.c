/*
 * Copyright 2018, PTC, Inc.
 * All rights reserved.
 */
#include "TestUtilities.h"
#include "unity_fixture.h"

/* Forward declare the function to test, since it is not exposed in the header
 * file */
uint32_t getMessageRequestId(char *msg);

TEST_GROUP(unit_twOfflineMsgStore_getMessageRequestId);

TEST_SETUP(unit_twOfflineMsgStore_getMessageRequestId) {
    eatLogs();
}

TEST_TEAR_DOWN(unit_twOfflineMsgStore_getMessageRequestId) {
}

TEST_GROUP_RUNNER(unit_twOfflineMsgStore_getMessageRequestId) {
    RUN_TEST_CASE(
        unit_twOfflineMsgStore_getMessageRequestId,
        test_single_part_message
    );
    RUN_TEST_CASE(
        unit_twOfflineMsgStore_getMessageRequestId,
        test_multipart_message
    );
}

/*
 * Test getMessageRequestId with single-part messages.
 *
 * Request ID should increment for each message.
 */
TEST(unit_twOfflineMsgStore_getMessageRequestId, test_single_part_message) {
    char msg[] = {
        /* Header */
        0x01,                    /* version */
        0x03,                    /* code */
        0x00, 0x00, 0x00, 0x00,  /* request ID */
        0x00, 0x00, 0x00, 0x00,  /* endpoint ID */
        0x00, 0x00, 0x00, 0x01,  /* session ID */
        0x00,                    /* multipart marker -- OFF */
        /* Body */
        0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A
    };
    uint32_t current_requestID = getMessageRequestId(msg);
    uint32_t next_requestID = getMessageRequestId(msg);

    TEST_ASSERT_NOT_EQUAL(current_requestID, next_requestID);
}

/*
 * Test getMessageRequestId with multi-part messages.
 *
 * Request ID should increment for the beginning of each multi-part message,
 * and remain constant for the other multi-part message blocks.
 */
TEST(unit_twOfflineMsgStore_getMessageRequestId, test_multipart_message) {
    char msg0[] = {
        /* Header */
        0x01,                    /* version */
        0x03,                    /* code */
        0x00, 0x00, 0x00, 0x00,  /* request ID */
        0x00, 0x00, 0x00, 0x00,  /* endpoint ID */
        0x00, 0x00, 0x00, 0x01,  /* session ID */
        0x00,                    /* multipart marker -- OFF */
        /* Body */
        0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A
    };
    char msg1[] = {
        /* Header */
        0x01,                    /* version */
        0x03,                    /* code */
        0x00, 0x00, 0x00, 0x00,  /* request ID */
        0x00, 0x00, 0x00, 0x00,  /* endpoint ID */
        0x00, 0x00, 0x00, 0x01,  /* session ID */
        0x01,                    /* multipart marker -- ON */
        /* Multipart Header */
        0x00, 0x01,              /* current chunk */
        0x00, 0x02,              /* total chunk */
        0x00, 0x00,              /* chunk size */
        /* Body */
        0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A
    };
    char msg2[] = {
        /* Header */
        0x01,                    /* version */
        0x03,                    /* code */
        0x00, 0x00, 0x00, 0x00,  /* request ID */
        0x00, 0x00, 0x00, 0x00,  /* endpoint ID */
        0x00, 0x00, 0x00, 0x01,  /* session ID */
        0x01,                    /* multipart marker -- ON */
        /* Multipart Header */
        0x00, 0x02,              /* current chunk */
        0x00, 0x02,              /* total chunk */
        0x00, 0x00,              /* chunk size */
        /* Body */
        0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A, 0x7B, 0x7A
    };

    /* Baseline non-multipart message. */
    uint32_t requestID_0 = getMessageRequestId(msg0);

    /* Next, a multipart message. */
    uint32_t requestID_1 = getMessageRequestId(msg1);
    uint32_t requestID_2 = getMessageRequestId(msg2);

    /* And another for good measure. */
    uint32_t requestID_3 = getMessageRequestId(msg1);
    uint32_t requestID_4 = getMessageRequestId(msg2);

    /* request IDs 0 and 1 should be different. */
    TEST_ASSERT_NOT_EQUAL(requestID_0, requestID_1);

    /* request IDs 1 and 2 should be the same. */
    TEST_ASSERT_EQUAL(requestID_1, requestID_2);

    /* request IDs 2 and 3 should be different. */
    TEST_ASSERT_NOT_EQUAL(requestID_2, requestID_3);

    /* request IDs 3 and 4 should be the same. */
    TEST_ASSERT_EQUAL(requestID_3, requestID_4);

}