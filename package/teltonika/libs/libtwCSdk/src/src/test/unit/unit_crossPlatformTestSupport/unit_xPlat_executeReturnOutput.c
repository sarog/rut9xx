
#include <string.h>
#include "unity_fixture.h"
#include "crossPlatformTestSupport.h"

TEST_GROUP(unit_xPlat_executeReturnOutput);

TEST_SETUP(unit_xPlat_executeReturnOutput) {
}

TEST_TEAR_DOWN(unit_xPlat_executeReturnOutput) {
}
TEST_GROUP_RUNNER(unit_xPlat_executeReturnOutput) {
	RUN_TEST_CASE(unit_xPlat_executeReturnOutput, testExecuteCommand);

}

TEST(unit_xPlat_executeReturnOutput, testExecuteCommand) {
	char resBuffer[255];
	xPlat_executeReturnOutput("echo TEST",resBuffer,255);
	TEST_ASSERT_EQUAL(0,strcmp(resBuffer,"TEST\n"));
}