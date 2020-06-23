#include "twApi.h"
#include "unity.h"
#include "unity_fixture.h"
#include "TestUtilities.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"
#include "twThreads.h"

TEST_GROUP(OfflineMsgStoreIntegrationSlow);

void test_OfflineMsgStoreIntegrationSlow_callback(char *passWd, unsigned int len){
	strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(OfflineMsgStoreIntegrationSlow) {
	int res = 0;
	eatLogs();

	/* delete any previous message store binaries */
	/* accept "file does not exist"(2) as a result */
	res = twDirectory_DeleteFile(OFFLINE_MSG_STORE_LOCATION_FILE);
	TEST_ASSERT_TRUE(TW_OK == res || 2 == res);

	/* set offline msg store file */
	twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_LOCATION;

	/* set connect delay to minimize test time */
	twcfg_pointer->max_connect_delay = OFFLINE_MSG_STORE_SMALL_DELAY;
	
	/* create a huge max message size to allow the entire stores to be flushed */
	twcfg_pointer->max_message_size = OFFLINE_STORE_10M + 1024;

	TEST_ASSERT_EQUAL(TW_OK, start_threaded_api(TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE));
}

TEST_TEAR_DOWN(OfflineMsgStoreIntegrationSlow) {
	twStubs_Reset();
	TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());
}

TEST_GROUP_RUNNER(OfflineMsgStoreIntegrationSlow) {
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_RAM_1M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_RAM_1M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_Disk_1M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_Disk_1M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_RAM_10M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_RAM_10M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_Disk_10M);
	RUN_TEST_CASE(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_Disk_10M);
}


/* fill up offline msg store and flush - in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_RAM_1M) {
	TEST_IGNORE_MESSAGE("Ignoring 1M offline msg store tests because they are very slow");
	test_fill_store(FALSE, FALSE, OFFLINE_STORE_1M, OFFLINE_STORE_MESSAGE_TIMEOUT_1M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store, tear down api, flush - enabled in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_RAM_1M) {
	TEST_IGNORE_MESSAGE("Ignoring 1M offline msg store tests because they are very slow");
	test_fill_store(TRUE, FALSE, OFFLINE_STORE_1M, OFFLINE_STORE_MESSAGE_TIMEOUT_1M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store and flush - in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_Disk_1M) {
	TEST_IGNORE_MESSAGE("Ignoring 1M offline msg store tests because they are very slow");
	test_fill_store(FALSE, TRUE, OFFLINE_STORE_1M, OFFLINE_STORE_MESSAGE_TIMEOUT_1M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store, tear down api, flush - enabled in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_Disk_1M) {
	TEST_IGNORE_MESSAGE("Ignoring 1M offline msg store tests because they are very slow");
	test_fill_store(TRUE, TRUE, OFFLINE_STORE_1M, OFFLINE_STORE_MESSAGE_TIMEOUT_1M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store and flush - in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_RAM_10M) {
	TEST_IGNORE_MESSAGE("Ignoring 10M offline msg store tests because they are very slow");
	test_fill_store(FALSE, FALSE, OFFLINE_STORE_10M, OFFLINE_STORE_MESSAGE_TIMEOUT_10M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store, tear down api, flush - enabled in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_RAM_10M) {
	TEST_IGNORE_MESSAGE("Ignoring 10M offline msg store tests because they are very slow");
	test_fill_store(TRUE, FALSE, OFFLINE_STORE_10M, OFFLINE_STORE_MESSAGE_TIMEOUT_10M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store and flush - in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Flush_Enabled_Disk_10M) {
	TEST_IGNORE_MESSAGE("Ignoring 10M offline msg store tests because they are very slow");
	test_fill_store(FALSE, TRUE, OFFLINE_STORE_10M, OFFLINE_STORE_MESSAGE_TIMEOUT_10M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}

/* fill up offline msg store, tear down api, flush - enabled in RAM */
TEST(OfflineMsgStoreIntegrationSlow, twOfflineMsgStore_Fill_Exit_Flush_Enabled_Disk_10M) {
	TEST_IGNORE_MESSAGE("Ignoring 10M offline msg store tests because they are very slow");
	test_fill_store(TRUE, TRUE, OFFLINE_STORE_10M, OFFLINE_STORE_MESSAGE_TIMEOUT_10M, TW_HOST, TW_PORT, TW_URI, test_OfflineMsgStoreIntegrationSlow_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
}
