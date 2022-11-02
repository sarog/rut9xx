/*
 *  Copyright 2018, PTC, Inc.
 *  All rights reserved.
 *
 *  Unit tests for swapXbytes()
 */

#include "TestUtilities.h"
#include "unity_fixture.h"

/* Forward declare module functions. */
#ifdef _WIN32
extern __declspec(dllimport) signed char twBaseTypes_isLittleEndian;
#else
extern signed char twBaseTypes_isLittleEndian;
#endif
void swapbytes(char * bytes, size_t n);

TEST_GROUP(unit_twBaseTypes_swapbytes);

TEST_SETUP(unit_twBaseTypes_swapbytes) {
    eatLogs();
}

TEST_TEAR_DOWN(unit_twBaseTypes_swapbytes) {
}

TEST_GROUP_RUNNER(unit_twBaseTypes_swapbytes) {
    RUN_TEST_CASE(unit_twBaseTypes_swapbytes, swap2bytes);
    RUN_TEST_CASE(unit_twBaseTypes_swapbytes, swap4bytes);
    RUN_TEST_CASE(unit_twBaseTypes_swapbytes, swap8bytes);
}

/*
 * Test 2-byte swap.
 *
 * Test Plan: Swap bytes. Verify byte swapping is correct for the architecture
 * being tested on.
 */
TEST(unit_twBaseTypes_swapbytes, swap2bytes) {
    uint16_t const initial = 0xF00D;
    uint16_t const mirror = 0x0DF0;

    uint16_t testCase = initial;

    swapbytes((char *) &testCase, 2);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }

    /* Reset and try the convenience function. */
    testCase = initial;

    swap2bytes((char *) &testCase);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }
}

/*
 * Test 4-byte swap.
 *
 * Test Plan: Swap bytes. Verify byte swapping is correct for the architecture
 * being tested on.
 */
TEST(unit_twBaseTypes_swapbytes, swap4bytes) {
    uint32_t const initial = 0x0A0B0C0D;
    uint32_t const mirror = 0x0D0C0B0A;

    uint32_t testCase = initial;

    swapbytes((char *) &testCase, 4);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }

    /* Reset and try the convenience function. */
    testCase = initial;

    swap4bytes((char *) &testCase);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }
}

/*
 * Test 8-byte swap.
 *
 * Test Plan: Swap bytes. Verify byte swapping is correct for the architecture
 * being tested on.
 */
TEST(unit_twBaseTypes_swapbytes, swap8bytes) {
    uint64_t const initial = 0x707172730A0B0C0D;
    uint64_t const mirror = 0x0D0C0B0A73727170;

    uint64_t testCase = initial;

    swapbytes((char *) &testCase, 8);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }

    /* Reset and try the convenience function. */
    testCase = initial;

    swap8bytes((char *) &testCase);

    if (twBaseTypes_isLittleEndian) {
        TEST_ASSERT_EQUAL(mirror, testCase);
    } else {
        TEST_ASSERT_EQUAL(initial, testCase);
    }
}