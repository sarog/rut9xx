
#include <string.h>
#include "unity_fixture.h"
#include "crossPlatformTestSupport.h"

TEST_GROUP(unit_xPlat_NodeInstalled);

TEST_SETUP(unit_xPlat_NodeInstalled) {
}

TEST_TEAR_DOWN(unit_xPlat_NodeInstalled) {
}
TEST_GROUP_RUNNER(unit_xPlat_NodeInstalled) {
	RUN_TEST_CASE(unit_xPlat_NodeInstalled, testNodeExists);
}

TEST(unit_xPlat_NodeInstalled, testNodeExists) {
	char resBuffer[255];
	TEST_IGNORE_MESSAGE("DVOP-2031 install docker and node.js on CI machines");
	TEST_ASSERT_TRUE_MESSAGE(xPlat_NodeInstalled(),"This test will not pass unless you have node.js on your path.");
}