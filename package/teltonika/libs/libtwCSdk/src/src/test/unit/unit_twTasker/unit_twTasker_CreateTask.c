/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/
#include "twOSPort.h"
#include "twTasker.h"
#include "twApi.h"
#include "twOSPort.h"
#include "unity.h"
#include "unity_fixture.h"
#include "unitTestDefs.h"
#include "TestUtilities.h"

TEST_GROUP(unit_twTasker_CreateTask);

TEST_SETUP(unit_twTasker_CreateTask){
	eatLogs();
	twApi_Initialize(TW_HOST, TW_PORT, TW_URI, TW_APP_KEY, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

TEST_TEAR_DOWN(unit_twTasker_CreateTask){
	twApi_Delete();
}
TEST_GROUP_RUNNER(unit_twTasker_CreateTask) {
	RUN_TEST_CASE(unit_twTasker_CreateTask, taskerNormalFlowSingleHandler);
	RUN_TEST_CASE(unit_twTasker_CreateTask, toManyTasks);
}

int taskerInitCallCount;
uint32_t runTimeIntervalMsec = 100; /* Every 500 ms */

void taskerInitTaskFunction(DATETIME now, void * params) {
	TEST_ASSERT_NOT_NULL((void*)now);
	TEST_ASSERT_NULL(params);
	taskerInitCallCount++;
}

/**
 * Test Plan: Start A tasker with one handler. Update it every 100ms while waiting 500ms.
 * 5 updates should have occured. Shut the tasker down and then verify that the hander
 * function is no longer being called.
 */
TEST(unit_twTasker_CreateTask,taskerNormalFlowSingleHandler){
	int i = 0;
	int taskId;
	taskerInitCallCount = 0;

	twTasker_Initialize();

	/* verify init as zero's out the task list */
	for (i = 0; i < TW_MAX_TASKS; i++) {
		twTask* task = twTaskerGetTask(i);
		TEST_ASSERT_NULL(task->func);
	}

	/* Add a new task */
	taskId =  twTasker_CreateTask(runTimeIntervalMsec, taskerInitTaskFunction);

	twSleepMsec(500);

	/* Verify call frequency of new task */
	TEST_ASSERT_TRUE(5<=taskerInitCallCount);
	TEST_ASSERT_TRUE(7>taskerInitCallCount);

	twTasker_RemoveTask(taskId);
	taskerInitCallCount = 0;

	twSleepMsec(500);
	TEST_ASSERT_EQUAL(0,taskerInitCallCount);

	twTasker_Stop();
	twTaskerRemoveAllTasks();

	/* verify init as zero's out the task list */
	for (i = 0; i < TW_MAX_TASKS; i++) {
		twTask* task = twTaskerGetTask(i);
		TEST_ASSERT_NULL(task->func);
	}

}

/**
 * Test Plan: Try to schedule more task than the tasker allocates memeory for.
 * Make sure it detects this and responds with TW_MAX_TASKS_EXCEEDED.
 */
TEST(unit_twTasker_CreateTask,toManyTasks){
	int i;
	for (i = 0; i < TW_MAX_TASKS; i++) {
		TEST_ASSERT_NOT_EQUAL(TW_MAX_TASKS_EXCEEDED,twTasker_CreateTask(runTimeIntervalMsec, taskerInitTaskFunction));
	}
	TEST_ASSERT_EQUAL(TW_MAX_TASKS_EXCEEDED,twTasker_CreateTask(runTimeIntervalMsec, taskerInitTaskFunction));

	twTaskerRemoveAllTasks();
}

