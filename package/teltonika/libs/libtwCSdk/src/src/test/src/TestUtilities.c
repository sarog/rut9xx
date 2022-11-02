#include "twApi.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "stringUtils.h"
#include "twApi.h"
#include "twThreads.h"
#include "TestServices.h"
#include "twMacros.h"
#include "twExt.h"
#include <stdarg.h>
#include "twDefaultSettings.h"

#ifdef WIN32
#include <direct.h>
#include <io.h>
#else

#include <ftw.h>
#include <unistd.h>
#include "twPasswds.h"

#endif
#define DO_NOTHING (property_cb)doNothing
#define TU_NAME_SIZE 15
extern twInfoTable *testConfigInfoTable;
extern char *test_host;
extern int test_port;
extern char *test_app_key;
extern char *configurationDirectory;
extern char * programName;

twThread *testUtilsApiThread = NULL;
twThread *testUtilsWorkerThreads[NUM_WORKER_THREADS] = {NULL};

#ifndef WIN32
twConfig *twcfg_pointer = &twcfg;
#endif

twThread *workerThreads[NUM_WORKER_THREADS];

char * twGetParentDirectory(char * path){
	int index,stlen;
	char * copyOfPath = duplicateString(path);
	stlen = strnlen(path,256);
	for(index = stlen;index>0;index--){
		if('/'==copyOfPath[index-1] || '\\'==copyOfPath[index-1]){
			if(index==stlen||index==1)
				continue;
			copyOfPath[index-1]=0;
			return copyOfPath;
		}
	}
	return NULL;

}

char* twGetPreferedExtensionLoadingDirectory(){
#ifdef _WIN32

	char cwd[1024],buffer[256],buffer2[256];
	char* currentDir,*nextDir;
	char* progNameCopy = duplicateString(programName);
    /*allLogs();
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        TW_LOG(TW_INFO, "******** Current working dir: %s *********", cwd);
    else
        perror("getcwd() error");
    eatLogs();*/
    currentDir = twGetParentDirectory(progNameCopy);
	while(NULL!=currentDir){

		snprintf(buffer,255,"%s/Examples",currentDir);
		if(twDirectory_FileExists(buffer)){
			snprintf(buffer2,255,"%s/Examples/ExtUseExample/ext",currentDir);
			TW_FREE(currentDir);
			return duplicateString(buffer2);
		}
		nextDir = twGetParentDirectory(currentDir);
		TW_FREE(currentDir);
		currentDir=nextDir;
	}
	return NULL;

#else
	return duplicateString("../examples/ExtUseExample/ext");
#endif
}

void * findCallback(enum entityTypeEnum entityType, char *entityName,
					enum characteristicEnum characteristicType, char *characteristicName, void **userdata);

void eatLogsHandler( enum LogLevel level, const char * timestamp, const char * message){ }
void eatLogs() {
	twLogger_SetFunction(eatLogsHandler);
}

void errorLogs() {
	twLogger_SetFunction(LOGGING_FUNCTION);
	twLogger_SetIsVerbose(TRUE);
	twLogger_SetLevel(TW_ERROR);
}

void allLogs() {
	twLogger_SetFunction(LOGGING_FUNCTION);
	twLogger_SetIsVerbose(TRUE);
	twLogger_SetLevel(TW_TRACE);
}

void doNothing(void *ptr) {}

int findn(int num) {
	int n = 0;
	if (num == 0) return 1;
	while (num) {
		num /= 10;
		n++;
	}
	return n;
}

char *twItoa(int num) {
	char *str = NULL;
	size_t len = sizeof(char) * (findn(num)) + 1;
	str = (char *) malloc(len);
	if (str) {
		snprintf(str, len, "%d", num);
	}
	return str;
}

void printTimestamp() {
#ifdef _WIN32
	DWORD milliseconds;
	milliseconds = GetTickCount();
#else
	struct timeval te;
	long long milliseconds;
	gettimeofday(&te, NULL); /* get current time */
	milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; /* calculate milliseconds */
#endif
	TW_LOG(TW_FORCE, "%lld", milliseconds);
}

long getTimestamp() {
#ifdef _WIN32
	DWORD milliseconds;
	milliseconds = GetTickCount();
#else
	struct timeval te;
	long long milliseconds;
	gettimeofday(&te, NULL); /* get current time */
	milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; /* calculate milliseconds */
#endif
	return milliseconds;
}

int isShared(){
	if(NULL==strstr(programName,"csdk_tests_static")){
		return 1;
	}
	return 0;
}

callbackInfo * findCallbackInfo(enum entityTypeEnum entityType, char *entityName,
					enum characteristicEnum characteristicType, char *characteristicName, void **userdata) {
	void *results;
	callbackInfo *cbInfoQuery;
	if (!tw_api->callbackList || !entityName || !characteristicName) {
		TW_LOG(TW_ERROR, "findCallback: NULL input parameter found");
		return NULL;
	}

	/* Build a query structure */
	cbInfoQuery = (callbackInfo *) TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->entityType = entityType;
	cbInfoQuery->characteristicType = characteristicType;

	// Search for an exact match
	cbInfoQuery->characteristicName = characteristicName;
	if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
		callbackInfo *cbInfo = (callbackInfo *) results;
		*userdata = cbInfo->userdata;
		TW_FREE(cbInfoQuery);
		return cbInfo;
	}

	// Search for a generic callback (*)
	cbInfoQuery->characteristicName = "*";
	if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
		callbackInfo *cbInfo = (callbackInfo *) results;
		*userdata = cbInfo->userdata;
		TW_FREE(cbInfoQuery);
		return cbInfo;
	}

	TW_FREE(cbInfoQuery);

	*userdata = NULL;
	return NULL;

}

/**
 * Extracts thing properties from the master callback list to see if they are registered.
 */
twPropertyDef * findProperty(const char* thingName, const char* propertyName){
	callbackInfo * cb;
	twPropertyDef* propertyDef;
	void* value=NULL;
	void* userData = NULL;
	value = findCallbackInfo(TW_THING, (char*)thingName,TW_PROPERTIES, (char*)propertyName,&userData);

	if(NULL==value)
		return NULL;
	cb = (callbackInfo *)value;

	if(NULL==cb->characteristicDefinition)
		return NULL;
	propertyDef = (twPropertyDef *)cb->characteristicDefinition;
	return propertyDef;
}

twServiceDef * findService(char* thingName, char* serviceName){
	callbackInfo * cb = NULL;
	twServiceDef* serviceDef = NULL;
	void* value = NULL;
	void* userData = NULL;
	value = findCallbackInfo(TW_THING,thingName,TW_SERVICES, serviceName,&userData);

	if(NULL==value)
		return NULL;
	cb = (callbackInfo *)value;
	if(NULL==cb->characteristicDefinition)
		return NULL;
	serviceDef = (twServiceDef *)cb->characteristicDefinition;
	return serviceDef;
}

void testDefaultBindEventCallback(char *entityName, char isBound, void *userdata) {
	if (isBound) TW_LOG(TW_FORCE, "testDefaultBindEventCallback: Entity %s was Bound", entityName);
	else
		TW_LOG(TW_FORCE, "testDefaultBindEventCallback: Entity %s was Unbound", entityName);
}

void testDefaultAuthEventCallback(char *credType, char *credValue, void *userdata) {
	if (!credType || !credValue) return;
	TW_LOG(TW_FORCE, "testDefaultAuthEventCallback: Authenticated using %s = %s.  Userdata = %p", credType, credValue, userdata);
}

void testDefaultFileEventCallback(char fileRcvd, twFileTransferInfo *info, void *userdata) {
	char startTime[80];
	char endTime[80];
	if (!info) {
		TW_LOG(TW_ERROR, "testDefaultFileEventCallback: Function called with NULL info");
	}
	twGetTimeString(info->startTime, startTime, "%Y-%m-%d %H:%M:%S", 80, 1, 1);
	twGetTimeString(info->endTime, endTime, "%Y-%m-%d %H:%M:%S", 80, 1, 1);
	TW_LOG(TW_DEBUG,
	       "\n\n*****************\nFILE TRANSFER NOTIFICATION:\nSource: %s:%s/%s\nDestination: %s:%s/%s\nSize: %9.0f\nStartTime: %s\nEndTime: %s\nDuration: %d msec\nUser: %s\nState: %s\nMessage: %s\nTransfer ID: %s\n*****************\n",
	       info->sourceRepository, info->sourcePath, info->sourceFile, info->targetRepository, info->targetPath,
	       info->targetFile, info->size, startTime, endTime, info->duration, info->user, info->state, info->message,
	       info->transferId);
}

/** integration test helpers **/
void bindEventCallback(char *entityName, char isBound, void *userdata) {
	if (isBound) TW_LOG(TW_FORCE, "testDefaultBindEventCallback: Entity %s was Bound", entityName);
	else
		TW_LOG(TW_FORCE, "testDefaultBindEventCallback: Entity %s was Unbound", entityName);
}

void authEventCallback(char *credType, char *credValue, void *userdata) {
	if (!credType || !credValue) return;
	TW_LOG(TW_FORCE, "testDefaultAuthEventCallback: Authenticated using %s = %s.  Userdata = %p", credType, credValue, userdata);
}

void createTestApi(char *entityName, char *host, uint16_t port, char *resource, char *app_key, char *gatewayName,
                   uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {
	int i = 0;
	twThread *apiThread = NULL;
	twThread *workerThreads[NUM_WORKER_THREADS] = {NULL};
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(host, port, resource, app_key, gatewayName, messageChunkSize,
	                                          frameSize, autoreconnect));
	twcfg_pointer->max_message_size = 2097152;
	/* TODO: this configuration option may change! */
	twcfg_pointer->offline_msg_queue_size = 2097152;
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();
	twApi_RegisterOnAuthenticatedCallback(authEventCallback, NULL);
	twApi_RegisterBindEventCallback(entityName, bindEventCallback, NULL);
	twApi_BindThing(entityName);
	/* Spin up threads and ensure they're running */
	apiThread = twThread_Create(twApi_TaskerFunction, THREAD_RATE, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		workerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, THREAD_RATE, NULL, TRUE);
	}
	TEST_ASSERT_TRUE(twThread_IsRunning(apiThread));
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		TEST_ASSERT_TRUE(twThread_IsRunning(workerThreads[i]));
	}
}

void deleteTestApi() {
	twThread *apiThread = NULL;
	twThread *workerThreads[NUM_WORKER_THREADS] = {NULL};
	int i = 0;
	twThread_Delete(apiThread);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(workerThreads[i]);
	}
	twApi_Delete();
}

void connectTestApi(char *entityName) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	TEST_ASSERT_TRUE(twApi_IsEntityBound(entityName));
}

void disconnectTestApi(char *entityName, char *reason) {
	TEST_ASSERT_EQUAL(TW_OK, twApi_UnbindThing(entityName));
	TEST_ASSERT_FALSE(twApi_IsEntityBound(entityName));
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect(reason));
	TEST_ASSERT_FALSE(twApi_isConnected());
}
/** end integration test helpers **/

/** offline test helpers **/

/* externally defined helper function */
int enableOfflineMsgStore(char enable, char onDisk);

/* helper functions */
int start_threaded_api(char *host, uint16_t port, char *resource, twPasswdCallbackFunction app_key_function, char *gatewayName,
                       uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {

	int i = 0;

	/* disable logging */
	eatLogs();

	/* initialize */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Initialize(host, port, resource, app_key_function, gatewayName, messageChunkSize, frameSize, autoreconnect));

	/* set connection params */
	twApi_SetSelfSignedOk();
	twApi_DisableCertValidation();

	/* attempt to connect, then check connection */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	TEST_ASSERT_TRUE(twApi_isConnected());
	testUtilsApiThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		testUtilsWorkerThreads[i] = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
	}
	return TW_OK;
}

int tear_down_threaded_api() {
	int i = 0;

	/* disconnect */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("ServiceIntegrationTests Complete"));
	TEST_ASSERT_FALSE(twApi_isConnected());

	/* delete mh threads */
	for (i = 0; i < NUM_WORKER_THREADS; i++) {
		twThread_Delete(testUtilsWorkerThreads[i]);
	}

	/* delete api threads */
	twThread_Delete(testUtilsApiThread);

	/* delete api */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Delete());

	return TW_OK;
}

int enable_msg_store(char onDisk) {
	/* enable offline message store */
	return enableOfflineMsgStore(TRUE, onDisk);
}

void test_write_to_store(char enabled, char cycle_connection, char onDisk, char *host, uint16_t port, char *resource,
                         char *app_key, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                         char autoreconnect) {
	twPrimitive *result = NULL;
	uint64_t f_size = 0;
	DATETIME l_mod;
	char is_dir = FALSE;
	char rd_only = FALSE;

	/* turn on offline msg store if enabled */
	if (enabled) {
		TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));
	}
	/* send a succesful message */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* set the api offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Offline Message Store Integration Test"));

	if (enabled) {
		/* send another message (should be written to msg store) */
		TEST_ASSERT_EQUAL(TW_WROTE_TO_OFFLINE_MSG_STORE, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));
	} else {
		TEST_ASSERT_EQUAL(TW_SERVICE_UNAVAILABLE, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));
	}
	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* check if the msg store is enabled */
	if (enabled) {
		if (onDisk) {
			/* read message store size */
			TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
			TEST_ASSERT_TRUE(f_size > 0);
		} else {
			/* check RAM */
		}
	}

	/* cycle the connection if necessary */
	if (cycle_connection) {
		/* allow threads to execute before shutdown if necessary */
		twSleepMsec(10);

		/* tear down api */
		TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());

		/* bring api back online */
		TEST_ASSERT_EQUAL(TW_OK, start_threaded_api(host, port, resource, app_key, gatewayName, messageChunkSize, frameSize, autoreconnect));

		/* re-enable offline message store */
		if (enabled) {
			TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));
		}

		/* set the api online */
		TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	}

	/* send another message, flushing the message store */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, TRUE));

	/* check if the msg store is enabled */
	if (enabled) {
		if (onDisk) {
			/* read message store size */
			TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
			TEST_ASSERT_TRUE(f_size == 0);
		} else {
			/* check RAM */
		}
	}

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}
}

/**
 * Fills the offline message store with requests to twApi_ReadProperty().
 * @param cycle_connection if TRUE tears down current connection and any associated message processing threads and
 * rebuilds them before performing any tests.
 * @param onDisk Configures offline message store to use a disk based file if TRUE or in ram if FALSE
 * @param size Configures the offline_msg_queue_size
 * @param timeout timeout to use when calling twApi_ReadProperty() to flush the offline message store
 * @param host The address of the name of the thingworx server to flush message to
 * @param port
 * @param resource path to web socket endpoint on server. Usually set to the constant TW_URI
 * @param app_key_function The authentication credential used to connect to host
 * @param gatewayName name of the THing to create to represent the API, usually NULL
 * @param messageChunkSize the maximum size of a message that can be stored in the store or sent to the server.
 * @param frameSize The maximum size of a websocket frame. Ordinarily matches messageChunkSize
 * @param autoreconnect If disconnected when this test starts, attempt to connect before proceeding
 */
void test_fill_store(char cycle_connection, char onDisk, size_t size, int32_t timeout, char *host, uint16_t port,
                     char *resource, twPasswdCallbackFunction app_key_function, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                     char autoreconnect) {
	int err = 0;
	twPrimitive *result = NULL;
	uint64_t f_size = 0;
	DATETIME l_mod;
	char is_dir = FALSE;
	char rd_only = FALSE;

	/* setup offline message store */
	twcfg_pointer->offline_msg_queue_size = size;
	enable_msg_store(onDisk);

	/* send a succesful message */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* set the api offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Offline Message Store Integration Test"));

	/* spam messages until store is full */
	while (err != TW_ERROR_OFFLINE_MSG_STORE_FULL) {
		/* send a message */
		err = twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE);

		/* ensure offline message was written to disk, or offline message exceeds store size */
		TEST_ASSERT_TRUE(TW_WROTE_TO_OFFLINE_MSG_STORE == err || TW_ERROR_OFFLINE_MSG_STORE_FULL == err);

		/* cleanup result to be used again */
		if (result) {
			twPrimitive_Delete(result);
			result = NULL;
		}
	}

	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size >= size);
	} else {
		/* check in ram size */
	}

	/* disconnect and reconnect if necessary */
	if (cycle_connection) {
		/* allow threads to execute before shutdown if necessary */
		twSleepMsec(10);

		/* tear down api */
		TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());

		/* bring api back online */
		TEST_ASSERT_EQUAL(TW_OK, start_threaded_api(host, port, resource, app_key_function, gatewayName, messageChunkSize, frameSize, autoreconnect));

		/* re-enable offline message store */
		TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));

		/* set the api online */
		TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));
	}

	/* send a message to flush storage */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, timeout, TRUE));

	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size == 0);
	} else {
		/* check in ram size */
	}

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}
}

void
test_decrease_store(char onDisk, size_t size, size_t size_2, int32_t timeout, char *host, uint16_t port, char *resource,
                    char *app_key, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                    char autoreconnect) {
	int err = 0;
	twPrimitive *result = NULL;
	uint64_t f_size = 0;
	DATETIME l_mod;
	char is_dir = FALSE;
	char rd_only = FALSE;

	/* setup offline message store */
	twcfg_pointer->offline_msg_queue_size = size;
	enable_msg_store(onDisk);

	/* send a succesful message */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* set the api offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Offline Message Store Integration Test"));

	/* spam messages until store is full */
	while (err != TW_ERROR_OFFLINE_MSG_STORE_FULL) {
		/* send a message */
		err = twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE);

		/* ensure offline message was written to disk, or offline message exceeds store size */
		TEST_ASSERT_TRUE(TW_WROTE_TO_OFFLINE_MSG_STORE == err || TW_ERROR_OFFLINE_MSG_STORE_FULL == err);

		/* cleanup result to be used again */
		if (result) {
			twPrimitive_Delete(result);
			result = NULL;
		}
	}

	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size > size_2);
	} else {
		/* check in ram size */

	}
	/* disconnect and reconnect after resetting size */
	/* allow threads to execute before shutdown if necessary */
	twSleepMsec(10);

	/* tear down api */
	TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());

	/* setup offline message store */
	twcfg_pointer->offline_msg_queue_size = size_2;

	/* bring api back online */
	TEST_ASSERT_EQUAL(TW_OK, start_threaded_api(host, port, resource, app_key, gatewayName, messageChunkSize, frameSize, autoreconnect));

	/* re-enable offline message store */
	TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));

	/* set the api online */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));

	/* send a message to flush storage */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, timeout, FALSE));

	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size == 0);
	} else {
		/* check in ram size */
	}

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}
}

void
test_large_message_decrease_store(char onDisk, size_t size, size_t size_2, int32_t timeout, char *host, uint16_t port,
                                  char *resource, char *app_key, char *gatewayName, uint32_t messageChunkSize,
                                  uint16_t frameSize, char autoreconnect, const char *thingName) {
	int err = 0;
	int i = 0;
	twPrimitive *result = NULL;
	twPrimitive *value = NULL;
	uint64_t f_size = 0;
	DATETIME l_mod;
	char is_dir = FALSE;
	char rd_only = FALSE;
	char *bigString = NULL;

	/* setup offline message store */
	twcfg_pointer->offline_msg_queue_size = size + TEST_MSG_OVERHEAD;
	TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));

	deleteServerThing(TW_THING, thingName);
	importEntityFileFromEtc("Things_UnityIntegrationTestThing.xml");

	/* send a succesful message */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, (char*)thingName, "name", &result, DEFAULT_MESSAGE_TIMEOUT, FALSE));

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}

	/* set the api offline */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Disconnect("Offline Message Store Integration Test"));

	/* create and write a giant message to the store */
	/* recalc size in case the loops arithmatic truncated the true value */
	/* add one for null terminator */
	bigString = (char *) TW_CALLOC(size + 1, sizeof(char));
	for (i = 0; i < size; i++) {
		/* append one char at a time */
		bigString[i] = 'a';
	}

	/* check string length */
	TEST_ASSERT_EQUAL(size, strlen(bigString));

	/* create primitive */
	value = twPrimitive_CreateFromString(bigString, TRUE);

	/* cleanup string */
	TW_FREE(bigString);

	/* send a message */
	err = twApi_WriteProperty(TW_THING, (char*)thingName, "name", value, DEFAULT_MESSAGE_TIMEOUT, FALSE);

	/* delete the primitive if it still exists  */
	if (value) twPrimitive_Delete(value);

	/* ensure offline message was written to disk, or offline message exceeds store size */
	TEST_ASSERT_TRUE(TW_WROTE_TO_OFFLINE_MSG_STORE == err);


	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size > size_2);
	} else {
		/* check in ram size */

	}
	/* disconnect and reconnect after resetting size */
	/* allow threads to execute before shutdown if necessary */
	twSleepMsec(10);

	/* tear down api */
	TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());

	/* setup offline message store */
	twcfg_pointer->offline_msg_queue_size = size_2;

	/* bring api back online */
	TEST_ASSERT_EQUAL(TW_OK, start_threaded_api(host, port, resource, app_key, gatewayName, messageChunkSize, frameSize, autoreconnect));

	/* re-enable offline message store */
	TEST_ASSERT_EQUAL(TW_OK, enable_msg_store(onDisk));

	/* set the api online */
	TEST_ASSERT_EQUAL(TW_OK, twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES));

	/* send a message to flush storage */
	TEST_ASSERT_EQUAL(TW_OK, twApi_ReadProperty(TW_THING, "SystemRepository", "name", &result, timeout, FALSE));

	/* check if the msg store is full */
	if (onDisk) {
		/* read message store size */
		TEST_ASSERT_EQUAL(TW_OK, twDirectory_GetFileInfo(OFFLINE_MSG_STORE_LOCATION_FILE, &f_size, &l_mod, &is_dir, &rd_only));
		TEST_ASSERT_TRUE(f_size == 0);
	} else {
		/* check in ram size */
	}

	/* cleanup result to be used again */
	if (result) {
		twPrimitive_Delete(result);
		result = NULL;
	}
}
/** end offline helpers **/

/** bind test helpers **/
void free_list(void *input) {
	char *tmp = NULL;
	if (input) {
		tmp = (char *) input;
		if (tmp) {
			TW_FREE(tmp);
		}
	}
}

twList *generateBindListLt(uint32_t maxMessageSize) {
	int i = 0;
	uint32_t size = 15 + 6;
	twList *l = NULL;
	char *name = NULL;
	l = twList_Create(free_list);
	while (size + TU_NAME_SIZE < maxMessageSize / 2) {
		name = (char *) TW_CALLOC(TU_NAME_SIZE, sizeof(char));
		snprintf(name, TU_NAME_SIZE, "SteamLT-%04d", i);
		size += TU_NAME_SIZE;
		i++;
		twList_Add(l, name);
	}
	return l;
}

twList *generateBindListEq(uint32_t maxMessageSize) {
	int i = 0;
	uint32_t size = 15 + 6;
	twList *l = NULL;
	char *name = NULL;
	l = twList_Create(free_list);
	while (size + TU_NAME_SIZE < maxMessageSize) {
		name = (char *) TW_CALLOC(TU_NAME_SIZE, sizeof(char));
		snprintf(name, TU_NAME_SIZE, "SteamEQ-%04d", i);
		size += TU_NAME_SIZE;
		i++;
		twList_Add(l, name);
	}
	return l;
}

twList *generateBindListGt(uint32_t maxMessageSize) {
	int i = 0;
	uint32_t size = 15 + 6;
	twList *l = NULL;
	char *name = NULL;
	l = twList_Create(free_list);
	while (size + TU_NAME_SIZE < maxMessageSize / 2 + maxMessageSize) {
		name = (char *) TW_CALLOC(TU_NAME_SIZE, sizeof(char));
		snprintf(name, TU_NAME_SIZE, "SteamGT-%04d", i);
		size += TU_NAME_SIZE;
		i++;
		twList_Add(l, name);
	}
	return l;
}

twList *generateBindListGt2(uint32_t maxMessageSize) {
	int i = 0;
	uint32_t size = 15 + 6;
	twList *l = NULL;
	char *name = NULL;
	l = twList_Create(free_list);
	while (size + TU_NAME_SIZE < maxMessageSize * 2 + 1) {
		name = (char *) TW_CALLOC(TU_NAME_SIZE, sizeof(char));
		snprintf(name, TU_NAME_SIZE, "SteamGT2-%04d", i);
		size += TU_NAME_SIZE;
		i++;
		twList_Add(l, name);
	}
	return l;
}

char verifyIsBoundHandler(ListEntry *le, void *userData) {
	char *thingName = (char *) le->value;
	TEST_ASSERT_TRUE_MESSAGE(twApi_IsEntityBound(thingName), thingName);
	return TRUE;
}

/** end bind helpers **/

char doesServerEntityExist(const char *type, const char *thingName) {
	int res;
	char returnValue = FALSE;
	twInfoTable *itout = NULL;
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it;
	twInfoTableRow *itRow;
	dse = twDataShapeEntry_Create("searchExpression", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("maxSearchItems", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("maxItems", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("types", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	it = twInfoTable_Create(ds);

	itRow = twInfoTableRow_Create(twPrimitive_CreateFromString(thingName, TRUE));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromNumber(100000));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromNumber(1));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromString(type, TRUE));
	twInfoTable_AddRow(it, itRow);
	res = twApi_InvokeService(TW_RESOURCE, "SearchFunctions", "SearchModelEntities", it, &itout, 5000, TRUE);
	if (TW_OK == res) {
		if (NULL != itout) {
			if (1 == itout->rows->count) {
				returnValue = TRUE;
			}
		}
	}

	/* cleanup infotables before returning */
	if (it) twInfoTable_Delete(it);
	if (itout) twInfoTable_Delete(itout);
	return returnValue;
}

char createServerDataShape(const char *dataShapeName, twDataShape *dataShape) {
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	twInfoTableRow *row;
	int resp;

	/* Does this thing already exist? */
	if (doesServerEntityExist("DataShape", dataShapeName))
		return TRUE;

	/* Create Thing */
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING); ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("description", NULL, TW_STRING); twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("fields", NULL, TW_INFOTABLE); twDataShape_AddEntry(ds, dse);

	it = twInfoTable_Create(ds);

	row = twInfoTableRow_Create(twPrimitive_CreateFromString(dataShapeName, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString("", TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromInfoTable(twInfoTable_Create(dataShape)));
	twInfoTable_AddRow(it, row);

	resp = twApi_InvokeService(TW_RESOURCE, "EntityServices", "CreateDataShape", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;
}

char createServerThing(const char *thingName, const char *thingTemplate) {
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	twInfoTableRow *row;
	int resp;

	/* Does this thing already exist? */
	if (doesServerEntityExist("Thing", thingName))
		return TRUE;

	/* Does thing template exist? */
	if (FALSE == doesServerEntityExist("ThingTemplate", thingTemplate))
		return FALSE;

	/* Create Thing */
	dse = twDataShapeEntry_Create("thingTemplateName", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	it = twInfoTable_Create(ds);

	row = twInfoTableRow_Create(twPrimitive_CreateFromString(thingTemplate, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(thingName, TRUE));
	twInfoTable_AddRow(it, row);

	resp = twApi_InvokeService(TW_RESOURCE, "EntityServices", "CreateThing", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		if (TW_OK == twApi_InvokeService(TW_THING, (char*)thingName, "EnableThing", NULL, &itout, 5000, TRUE))
			if (TW_OK == twApi_InvokeService(TW_THING, (char*)thingName, "RestartThing", NULL, &itout, 5000, TRUE))
				returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;

}

char deleteServerThing(enum entityTypeEnum type, const char *thingName) {
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	int resp;
	twPrimitive *p;
	twInfoTableRow *row;

	/* Does this thing already exist? */
	if (TW_THING == type) {
		if (FALSE == doesServerEntityExist("Thing", thingName))
			return TRUE;
	}
	if (TW_THINGTEMPLATES == type) {
		if (FALSE == doesServerEntityExist("ThingTemplate", thingName))
			return TRUE;
	}

	/* Delete Thing */
	dse = twDataShapeEntry_Create("name", NULL, TW_THINGNAME);
	ds = twDataShape_Create(dse);
	it = twInfoTable_Create(ds);

	p = twPrimitive_CreateFromString(thingName, TRUE);

	row = twInfoTableRow_Create(p);
	twInfoTable_AddRow(it, row);
	resp = twApi_InvokeService(TW_RESOURCE, "EntityServices", "DeleteThing", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;

}

char * getEtcDirectory(){
	return configurationDirectory;
}

char importEntityFileFromEtc(const char *fileName) {
	char *httpexecutable;
	char commandBuffer[CMD_BUF_SIZE];
	char fileNameBuffer[FILE_NAME_BUF_SIZE];
	int ret;

	snprintf(fileNameBuffer, FILE_NAME_BUF_SIZE, "\"%s/%s\"",configurationDirectory,fileName);

	twInfoTable_GetString(testConfigInfoTable,"httpuploadexecutable",0,&httpexecutable);
	snprintf(commandBuffer, CMD_BUF_SIZE, httpexecutable, test_app_key, fileNameBuffer, test_host, test_port);
	ret = system(commandBuffer);

	/* cleanup string memory before returning */
	TW_FREE(httpexecutable);

	if (0 == ret)
		return TRUE;
	else
		return FALSE;
}

char importBootstrapEntityFileFromEtc(const char *fileName) {
	char *httpexecutable;
	char commandBuffer[CMD_BUF_SIZE];
	char fileNameBuffer[FILE_NAME_BUF_SIZE];
	int ret;

	snprintf(fileNameBuffer, FILE_NAME_BUF_SIZE, "\"%s/%s\"",configurationDirectory,fileName);

	twInfoTable_GetString(testConfigInfoTable,"bootstraphttpuploadexecutable",0,&httpexecutable);
	snprintf(commandBuffer, CMD_BUF_SIZE, httpexecutable, fileNameBuffer, test_host, test_port);
	ret = system(commandBuffer);

	/* cleanup string memory before returning */
	TW_FREE(httpexecutable);

	if (0 == ret)
		return TRUE;
	else
		return FALSE;
}


char doesServerThingPropertyExist(enum entityTypeEnum type, const char *thingName, const char *propertyTypeName,
                                  const char *propertyName) {
	int res;
	char functionNameBuffer[100];
	char returnValue = FALSE;
	twInfoTable *itout;
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it;
	twInfoTableRow *itRow;

	sprintf(functionNameBuffer, "Get%sPropertyValue", propertyTypeName);

	dse = twDataShapeEntry_Create("propertyName", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	it = twInfoTable_Create(ds);

	itRow = twInfoTableRow_Create(twPrimitive_CreateFromString(propertyName, TRUE));
	twInfoTable_AddRow(it, itRow);
	res = twApi_InvokeService(type, (char*)thingName, functionNameBuffer, it, &itout, 5000, TRUE);
	if (TW_OK == res) {
		if (NULL != itout) {
			if (1 == itout->rows->count) {
				returnValue = TRUE;
			}
		}
	}
	/* cleanup infotables before returning */
	if (it) twInfoTable_Delete(it);
	if (itout) twInfoTable_Delete(itout);
	return returnValue;
}

char removePropertyFromServerEntity(enum entityTypeEnum type, const char *thingName, const char *propertyName) {
	int res;
	char returnValue = FALSE;
	twInfoTable *itout;
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it;
	twInfoTableRow *itRow;

	dse = twDataShapeEntry_Create("name", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	it = twInfoTable_Create(ds);

	itRow = twInfoTableRow_Create(twPrimitive_CreateFromString(propertyName, TRUE));
	twInfoTable_AddRow(it, itRow);
	res = twApi_InvokeService(type, (char*)thingName, "RemovePropertyDefinition", it, &itout, 5000, TRUE);
	if (TW_OK == res) {
		returnValue = TRUE;
	}
	twInfoTable_Delete(it);
	return returnValue;
}

char fileExistsInServerRepo(const char *repoName, const char *path) {
	char res = FALSE;
	twInfoTable *it = NULL;
	twInfoTable *result = NULL;
	it = twInfoTable_CreateFromString("path", (char*)path, TRUE);
	twApi_InvokeService(TW_THING, (char*)repoName, "GetFileInfo", it, &result, 5000, FALSE);
	if (result) {
		res = TRUE;
		twInfoTable_Delete(result);
	}
	twInfoTable_Delete(it);
	return res;
}

char pollFileExistsInServerRepo(const char *repoName, const char *path, int timeout) {
	int time = 0;
	int interval = 1000;
	char exists = FALSE;
	for (time = 0; !exists && time < timeout; time += interval) {
		exists = fileExistsInServerRepo(repoName, path);
		twSleepMsec(interval);
	}
	return exists;
}

char restartServerThing(enum entityTypeEnum type, const char *thingName) {
	int res;
	char returnValue = FALSE;
	twInfoTable *itout;
	res = twApi_InvokeService(type, (char*)thingName, "RestartThing", NULL, &itout, 5000, TRUE);
	if (TW_OK == res) {
		returnValue = TRUE;
	}
	return returnValue;
}

char assignValueStreamToServerThing(const char *thingName, const char *valueStreamName) {
	int res;
	char returnValue = FALSE;
	twInfoTable *itOut;
	twInfoTable *itIn;
	itIn = twInfoTable_CreateFromString("name", (char*)valueStreamName, TRUE);
	res = invokeServiceWithRetries(TW_THING, (char*)thingName, "SetValueStream", itIn, &itOut, 5000, TRUE);

	/* cleanup memory */
	if (itIn) twInfoTable_Delete(itIn);
	if (itOut) twInfoTable_Delete(itOut);

	if (TW_OK == res) {
		restartThing(thingName);
		restartThing(valueStreamName);
		returnValue = TRUE;
	}
	return returnValue;
}

twInfoTable *_HelperQueryPropertyHistory (const char* serviceName, const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query) {
	twDataShapeEntry* dse;
	twDataShape* ds;
	twInfoTable *it, *itout;
	twInfoTableRow* row;
	int resp;

	dse = twDataShapeEntry_Create ("maxItems", NULL, TW_NUMBER);
	ds = twDataShape_Create (dse);
	dse = twDataShapeEntry_Create ("propertyName", NULL, TW_STRING);
	twDataShape_AddEntry (ds, dse);
	dse = twDataShapeEntry_Create ("startDate", NULL, TW_DATETIME);
	twDataShape_AddEntry (ds, dse);
	dse = twDataShapeEntry_Create ("endDate", NULL, TW_DATETIME);
	twDataShape_AddEntry (ds, dse);
	dse = twDataShapeEntry_Create ("oldestFirst", NULL, TW_BOOLEAN);
	twDataShape_AddEntry (ds, dse);
	dse = twDataShapeEntry_Create ("query", NULL, TW_STRING);
	twDataShape_AddEntry (ds, dse);

	it = twInfoTable_Create (ds);
	row = twInfoTableRow_Create (twPrimitive_CreateFromNumber (maxItems));
	twInfoTableRow_AddEntry (row, twPrimitive_CreateFromString (propertyName, TRUE));
	twInfoTableRow_AddEntry (row, twPrimitive_CreateFromDatetime (startDate));
	twInfoTableRow_AddEntry (row, twPrimitive_CreateFromDatetime (endDate));
	twInfoTableRow_AddEntry (row, twPrimitive_CreateFromBoolean (oldestFirst));
	twInfoTableRow_AddEntry (row, twPrimitive_CreateFromString (query, TRUE));
	twInfoTable_AddRow (it, row);

	resp = twApi_InvokeService (TW_THING, (char*)thingName, (char*)serviceName, it, &itout, 10000, TRUE);
	if (TW_OK != resp) {
		itout = NULL;
		}
	twInfoTable_Delete (it);
	return itout;
	}

twInfoTable *queryIntegerPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query) {
	return _HelperQueryPropertyHistory ("QueryIntegerPropertyHistory",
		thingName,
		maxItems,
		propertyName,
		startDate,
		endDate,
		oldestFirst,
		query);
	}

twInfoTable *queryNumberPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query) {
	return _HelperQueryPropertyHistory ("QueryNumberPropertyHistory",
		thingName,
		maxItems,
		propertyName,
		startDate,
		endDate,
		oldestFirst,
		query);
	}

twInfoTable *queryBooleanPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query)
	{
	return _HelperQueryPropertyHistory ("QueryBooleanPropertyHistory",
										thingName,
										maxItems,
										propertyName,
										startDate,
										endDate,
										oldestFirst,
										query);
	}

twInfoTable *queryStringPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query)
	{
	return _HelperQueryPropertyHistory ("QueryStringPropertyHistory",
										thingName,
										maxItems,
										propertyName,
										startDate,
										endDate,
										oldestFirst,
										query);
	}

twInfoTable *queryDateTimePropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate, DATETIME endDate, char oldestFirst, const char *query)
	{
	return _HelperQueryPropertyHistory ("QueryDateTimePropertyHistory",
										thingName,
										maxItems,
										propertyName,
										startDate,
										endDate,
										oldestFirst,
										query);
	}

char purgeAllPropertyHistory(const char *thingName) {
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	twInfoTableRow *row;
	int resp;

	DATETIME startDate = 0;
	DATETIME endDate = twGetSystemTime(TRUE);

	dse = twDataShapeEntry_Create("startDate", NULL, TW_NUMBER);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("endDate", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);

	it = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromDatetime(startDate));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(endDate));
	twInfoTable_AddRow(it, row);

	resp = twApi_InvokeService(TW_THING, (char*)thingName, "PurgeAllPropertyHistory", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;
}

char addServiceDefinitionToServerThing(const char *thingName, const char *name, const char *description,
                                       const char *category, twInfoTable *parameters, twInfoTable *resultType,
                                       char remote, const char *remoteServiceName, int timeout) {

	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	twInfoTableRow *row;
	int resp;

	/* Does this thing already exist? */
	if (FALSE == doesServerEntityExist("Thing", thingName))
		return FALSE;

	/* Datashape (/me Sigh) */
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("description", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("category", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("parameters", NULL, TW_INFOTABLE);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("resultType", NULL, TW_INFOTABLE);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("remote", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("remoteServiceName", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("timeout", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);

	/* Populate InfoTable */
	it = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(name, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(description, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(category, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromInfoTable(parameters));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromInfoTable(resultType));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(remote));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(remoteServiceName, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber(timeout));
	twInfoTable_AddRow(it, row);

	resp = twApi_InvokeService(TW_THING, (char*)thingName, "AddServiceDefinition", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;
}

char addServiceDefinitionToServerThingSimple(const char *thingName, const char *name, twInfoTable *parameters,
                                             twInfoTable *resultType) {
	return addServiceDefinitionToServerThing(thingName, name, "", "", parameters, resultType, TRUE, name, 5000);
}

char addPropertyDefinitionToServerThing(const char *thingName, const char *name, const char *type, char readOnly,
                                        char remote,
                                        const char *remotePropertyName, double timeout, const char *pushType,
                                        double dataChangeThreshold, char logged,
                                        double pushThreshold, const char *dataChangeType, const char *category,
                                        char persistent, const char *dataShapeName,
                                        twPrimitive *defaultValue, const char *description) {

	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTable *it, *itout;
	char returnValue = FALSE;
	twInfoTableRow *row;
	int resp;

	/* Does this thing already exist? */
	if (FALSE == doesServerEntityExist("Thing", thingName))
		return FALSE;

	/* Datashape (/me Sigh) */
	dse = twDataShapeEntry_Create("defaultValue", NULL, defaultValue->type);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("description", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("remotePropertyName", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("pushType", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("dataChangeType", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("category", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("type", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("readOnly", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("remote", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("logged", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("persistent", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("timeout", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("dataChangeThreshold", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("pushThreshold", NULL, TW_NUMBER);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("dataShape", NULL, TW_DATASHAPENAME);
	twDataShape_AddEntry(ds, dse);

	/* Populate InfoTable */
	it = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(defaultValue);
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(description, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(remotePropertyName, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(pushType, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(name, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(dataChangeType, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(category, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(type, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(readOnly));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(remote));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(logged));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(persistent));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber(timeout));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber(dataChangeThreshold));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromNumber(pushThreshold));
	if (dataShapeName) twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(dataShapeName, TRUE));
	twInfoTable_AddRow(it, row);

	resp = twApi_InvokeService(TW_THING, (char*)thingName, "AddPropertyDefinition", it, &itout, 5000, TRUE);
	if (TW_OK == resp)
		returnValue = TRUE;

	twInfoTable_Delete(it);
	return returnValue;

}

char addPropertyDefinitionToServerThingSimple(const char *thingName, const char *name, const char *type,
                                              const char *pushType, twPrimitive *defaultValue) {
	return addPropertyDefinitionToServerThing(thingName, name, type, FALSE, TRUE, name, 5000, pushType, 0, FALSE, 0, pushType, "", FALSE, NULL, defaultValue, "");
}

char createTextFileInServerRepo(const char *repoName, const char *path, size_t size) {
	twDataShapeEntry *dse = NULL;
	twDataShape *ds = NULL;
	twInfoTable *itIn = NULL;
	twInfoTable *itOut = NULL;
	twInfoTableRow *row = NULL;
	char *data = NULL;
	int response = 0;

	/* Data generation */
	data = generateRandomString(size);

	/* Datashape creation */
	dse = twDataShapeEntry_Create("path", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("data", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("overwrite", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);

	/* Populate InfoTable */
	itIn = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(path, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(data, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(TRUE));
	twInfoTable_AddRow(itIn, row);

	response = twApi_InvokeService(TW_THING, (char*)repoName, "CreateTextFile", itIn, &itOut, 10000, TRUE);

	twInfoTable_Delete(itIn);
	TW_FREE(data);

	if (TW_OK == response) return TRUE;
	return FALSE;
}

char *generateRandomString(size_t size) {
	char *str = NULL;
	size_t n = 0;
	const char charset[] = "abcdefghijklmnopqrstuvwxyz";
	str = (char *) TW_MALLOC(sizeof(char) * (size + 1));
	if (size) {
		--size;
		for (n = 0; n < size; n++) {
			int key = rand() % (int) (sizeof charset - 1);
			str[n] = charset[key];
		}
		str[size] = '\0';
	}
	return str;
}

/** file transfer helper functions **/


char fileExists(char *path) {
	return twDirectory_FileExists(path);
}

char pollFileExists(char *path, int timeout) {
	int time = 0;
	int interval = 1000;
	char exists = FALSE;
	for (time = 0; !exists && time < timeout; time += interval) {
		exists = fileExists(path);
		twSleepMsec(interval);
	}
	return exists;
}

const char* getConfigDir()
	{
	return configurationDirectory;
	}

int createDir(char *path) {
#ifdef WIN32
	if (!fileExists(path)) return mkdir(path);
#else
	if (!fileExists(path)) return mkdir(path, 0777);
#endif
	return -1;
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf) {
	int res = 0;
#ifdef WIN32
#else
	res = remove(fpath);
	if (res)
		perror(fpath);
#endif
	return res;
}

int isTransferActive(char *tid) {
	twDataShape *ds = NULL;
	twInfoTable *it = NULL;
	twInfoTableRow *row = NULL;
	twInfoTable *serviceResult = NULL;
	int isActive;
	if (!tid) return -1;
	ds = twDataShape_Create(twDataShapeEntry_Create("tid", NULL, TW_STRING));
	it = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(tid, TRUE));
	twInfoTable_AddRow(it, row);
	twApi_InvokeService(TW_SUBSYSTEM, "FileTransferSubsystem", "IsTransferJobActive", it, &serviceResult, 1000, FALSE);
	twInfoTable_Delete(it);
	twInfoTable_GetInteger(serviceResult, "result", 0, &isActive);
	twInfoTable_Delete(serviceResult);
	return isActive;
}

char *getCurrentDirectory() {
	char *dir = NULL;
#ifdef WIN32
	dir = (char*)malloc(sizeof(char) * (MAX_PATH + 1));
	_getcwd(dir, MAX_PATH);
#else
	dir = malloc(sizeof(char) * (MAX_FILE_PATH + 1));
	getcwd(dir, PATH_MAX);
#endif
	return dir;
}

/** end file transfer helper functions **/


twTest_Data *twTest_CreateData() {
	twTest_Data *tmp = NULL;

	/* allocate memory */
	tmp = (twTest_Data *) TW_MALLOC(sizeof(twTest_Data));
	if (!tmp) return tmp;

	/* defaults for result, isDone, and data */
	tmp->result = 0;
	tmp->isDone = FALSE;
	tmp->data = NULL;

	/* create mutex */
	tmp->mtx = twMutex_Create();
	if (!tmp->mtx) {
		twTest_DeleteData(tmp);
		return NULL;
	}
	return tmp;
}

void twTest_DeleteData(twTest_Data *data) {
	if (data) {

		/* user is responisble for free'ing the results */
		if (data->mtx) {
			twMutex_Lock(data->mtx);
			twMutex_Delete(data->mtx);
		}
		TW_FREE(data);
	}
	return;
}

char *createUniqueServerThing(char *prefix, char *thingTemplate) {
	int i = 0;
	for (i = 0; i < 1000; i++) {
		char *thingName = malloc(sizeof(char) * 1024);
		char *id = twItoa(i);
		strcpy(thingName, prefix);
		strcat(thingName, "_");
		strcat(thingName, id);
		free(id);
		if (!doesServerEntityExist("Thing", thingName)) {
			twLog(TW_DEBUG, "Server thing %s is not in use, creating %s", thingName, thingName);
			createServerThing(thingName, thingTemplate);
			return thingName;
		}
		twLog(TW_DEBUG, "Server thing %s is in use", thingName);
		free(thingName);
	}
	return NULL;
}

twInfoTable *createServiceDefinitionParameterInfoTable(const char *name, const char *type) {
	twDataShapeEntry *dse;
	twDataShape *ds;
	twInfoTableRow *row;
	twInfoTable *it;
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING);
	ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("description", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("isPrimaryKey", NULL, TW_BOOLEAN);
	twDataShape_AddEntry(ds, dse);
	dse = twDataShapeEntry_Create("baseType", NULL, TW_STRING);
	twDataShape_AddEntry(ds, dse);
	it = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(name, TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString("", TRUE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromBoolean(FALSE));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(type, TRUE));
	twInfoTable_AddRow(it, row);
	return it;
}

twDataShape *createAllPropertyBaseTypesDataShape() {
	twDataShape *allPropertyBaseTypes = NULL;
	allPropertyBaseTypes = twDataShape_Create(twDataShapeEntry_Create("Boolean", NULL, TW_BOOLEAN));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Integer", NULL, TW_INTEGER));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Number", NULL, TW_NUMBER));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Datetime", NULL, TW_DATETIME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("String", NULL, TW_STRING));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Text", NULL, TW_TEXT));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("ThingName", NULL, TW_THINGNAME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("GroupName", NULL, TW_GROUPNAME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("UserName", NULL, TW_USERNAME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("MashupName", NULL, TW_MASHUPNAME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("MenuName", NULL, TW_MENUNAME));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Imagelink", NULL, TW_IMAGELINK));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Hyperlink", NULL, TW_HYPERLINK));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("HTML", NULL, TW_HTML));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("XML", NULL, TW_XML));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("JSON", NULL, TW_JSON));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Query", NULL, TW_QUERY));
	twDataShape_AddEntry(allPropertyBaseTypes, twDataShapeEntry_Create("Location", NULL, TW_LOCATION));
	return allPropertyBaseTypes;
}

void registerTestServices(char *entityName) {
	twDataShapeEntry *itEntry = NULL;
	twDataShapeEntry *pushIntegerEntry = NULL;
	twDataShapeEntry *pushStringEntry = NULL;
	twDataShape *dsPushInfoTable = NULL;
	twDataShape *dsAllPropertyBaseTypes = NULL;

	twDataShape *dsPushBoolean = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_BOOLEAN));
	twDataShape *dsPushDatetime = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_DATETIME));
	twDataShape *dsPushGroupName = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_GROUPNAME));
	twDataShape *dsPushHTML = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_HTML));
	twDataShape *dsPushHyperlink = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_HYPERLINK));
	twDataShape *dsPushImage = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_IMAGE));
	twDataShape *dsPushImagelink = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_IMAGELINK));
	twDataShape *dsPushInteger = NULL; /* NULL so that we can add aspects to the entries of this ds */
	twDataShape *dsPushJson = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_JSON));
	twDataShape *dsPushLocation = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_LOCATION));
	twDataShape *dsPushMashupName = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_MASHUPNAME));
	twDataShape *dsPushMenuName = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_MENUNAME));
	twDataShape *dsPushNumber = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_NUMBER));
	twDataShape *dsPushString = NULL; /* NULL so that we can add aspects to the entries of this ds */
	twDataShape *dsPushQuery = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_QUERY));
	twDataShape *dsPushText = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_TEXT));
	twDataShape *dsPushThingName = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_THINGNAME));
	twDataShape *dsPushUserName = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_USERNAME));
	twDataShape *dsPushXML = twDataShape_Create(twDataShapeEntry_Create("value", NULL, TW_XML));

	/* This puts the default value input data shape into the aspect of the input field */
	pushIntegerEntry = twDataShapeEntry_Create("value", NULL, TW_INTEGER);
	twDataShapeEntry_AddAspect(pushIntegerEntry, "defaultValue", twPrimitive_CreateFromInteger(0));
	dsPushInteger = twDataShape_Create(pushIntegerEntry);

	/* This puts the default value input data shape into the aspect of the input field */
	pushStringEntry = twDataShapeEntry_Create("value", NULL, TW_STRING);
	twDataShapeEntry_AddAspect(pushStringEntry, "defaultValue", twPrimitive_CreateFromString("Default String", TRUE));
	dsPushString = twDataShape_Create(pushStringEntry);

	/* This puts the name of the input data shape into the aspect of the input field */
	itEntry = twDataShapeEntry_Create("value", NULL, TW_INFOTABLE);
	twDataShapeEntry_AddAspect(itEntry, "dataShape", twPrimitive_CreateFromString("AllPropertyBaseTypes", TRUE));
	dsPushInfoTable = twDataShape_Create(itEntry);

	/* Push_InfoTable service returns a DataShape that contains fields for all the base types */
	dsAllPropertyBaseTypes = createAllPropertyBaseTypesDataShape();

	/* Expected name for Push_InfoTable return DataShape is 'AllPropertyBaseTypes'. */
	twDataShape_SetName(dsAllPropertyBaseTypes, "AllPropertyBaseTypes");

	twApi_RegisterService(TW_THING, entityName, "Push_Boolean", NULL, dsPushBoolean, TW_BOOLEAN, NULL, pushBoolean, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Datetime", NULL, dsPushDatetime, TW_DATETIME, NULL, pushDatetime, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_GroupName", NULL, dsPushGroupName, TW_GROUPNAME, NULL, pushGroupName, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_HTML", NULL, dsPushHTML, TW_HTML, NULL, pushHTML, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Hyperlink", NULL, dsPushHyperlink, TW_HYPERLINK, NULL, pushHyperlink, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Image", NULL, dsPushImage, TW_IMAGE, NULL, pushImage, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Imagelink", NULL, dsPushImagelink, TW_IMAGELINK, NULL, pushImagelink, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_InfoTable", NULL, dsPushInfoTable, TW_INFOTABLE, dsAllPropertyBaseTypes, pushInfoTable, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Integer", NULL, dsPushInteger, TW_INTEGER, NULL, pushInteger, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Json", NULL, dsPushJson, TW_JSON, NULL, pushJson, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Location", NULL, dsPushLocation, TW_LOCATION, NULL, pushLocation, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_MashupName", NULL, dsPushMashupName, TW_MASHUPNAME, NULL, pushMashupName, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_MenuName", NULL, dsPushMenuName, TW_MENUNAME, NULL, pushMenuName, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Number", NULL, dsPushNumber, TW_NUMBER, NULL, pushNumber, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_String", NULL, dsPushString, TW_STRING, NULL, pushString, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Query", NULL, dsPushQuery, TW_QUERY, NULL, pushQuery, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_Text", NULL, dsPushText, TW_TEXT, NULL, pushText, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_ThingName", NULL, dsPushThingName, TW_THINGNAME, NULL, pushThingName, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_UserName", NULL, dsPushUserName, TW_USERNAME, NULL, pushUserName, NULL);
	twApi_RegisterService(TW_THING, entityName, "Push_XML", NULL, dsPushXML, TW_XML, NULL, pushXML, NULL);
}

void registerTestProperties(const char *entityName) {
	/* Register NEVER push properties */
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Boolean", TW_BOOLEAN, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Integer", TW_INTEGER, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Number", TW_NUMBER, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Datetime", TW_DATETIME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_String", TW_STRING, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Text", TW_TEXT, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_ThingName", TW_THINGNAME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_GroupName", TW_GROUPNAME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_UserName", TW_USERNAME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_MashupName", TW_MASHUPNAME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_MenuName", TW_MENUNAME, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Imagelink", TW_IMAGELINK, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Hyperlink", TW_HYPERLINK, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_HTML", TW_HTML, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_JSON", TW_JSON, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Query", TW_QUERY, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_Location", TW_LOCATION, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "NeverPush_InfoTable", TW_INFOTABLE, NULL, "NEVER", 0.0, DO_NOTHING, NULL);
	/* Register ALWAYS push properties */
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Boolean", TW_BOOLEAN, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Integer", TW_INTEGER, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Number", TW_NUMBER, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Datetime", TW_DATETIME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_String", TW_STRING, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Text", TW_TEXT, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_ThingName", TW_THINGNAME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_GroupName", TW_GROUPNAME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_UserName", TW_USERNAME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_MashupName", TW_MASHUPNAME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_MenuName", TW_MENUNAME, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Imagelink", TW_IMAGELINK, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Hyperlink", TW_HYPERLINK, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_HTML", TW_HTML, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_JSON", TW_JSON, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Query", TW_QUERY, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_Location", TW_LOCATION, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
	twApi_RegisterProperty(TW_THING, entityName, "AlwaysPush_InfoTable", TW_INFOTABLE, NULL, "ALWAYS", 0.0, DO_NOTHING, NULL);
}

void addTestPropertyDefinitionsToServerThing(const char *entityName) {
	twPrimitive *defaultBooleanValue = NULL;
	twPrimitive *defaultIntegerValue = NULL;
	twPrimitive *defaultNumberValue = NULL;
	twPrimitive *defaultDatetimeValue = NULL;
	twPrimitive *defaultStringValue = NULL;
	twPrimitive *defaultTextValue = NULL;
	twPrimitive *defaultThingNameValue = NULL;
	twPrimitive *defaultGroupNameValue = NULL;
	twPrimitive *defaultUserNameValue = NULL;
	twPrimitive *defaultMashupNameValue = NULL;
	twPrimitive *defaultMenuNameValue = NULL;
	twPrimitive *defaultImagelinkValue = NULL;
	twPrimitive *defaultHyperlinkValue = NULL;
	twPrimitive *defaultHTMLValue = NULL;
	twPrimitive *defaultXMLValue = NULL;
	twPrimitive *defaultJSONValue = NULL;
	twPrimitive *defaultQueryValue = NULL;
	twPrimitive *defaultLocationValue = NULL;
	twLocation *location = NULL;

	location = TW_MALLOC(sizeof(twLocation));
	location->latitude = 35.6895;
	location->longitude = 139.6917;
	location->elevation = 131;

	defaultBooleanValue = twPrimitive_CreateFromBoolean(FALSE);
	defaultIntegerValue = twPrimitive_CreateFromInteger(0);
	defaultNumberValue = twPrimitive_CreateFromNumber(0.0);
	defaultDatetimeValue = twPrimitive_CreateFromCurrentTime();
	defaultStringValue = twPrimitive_CreateFromString("DefaultString", TRUE);
	defaultTextValue = twPrimitive_CreateFromString("DefaultText", TRUE);
	defaultThingNameValue = twPrimitive_CreateFromString("DefaultThingName", TRUE);
	defaultGroupNameValue = twPrimitive_CreateFromString("DefaultGroupName", TRUE);
	defaultUserNameValue = twPrimitive_CreateFromString("DefaultUserName", TRUE);
	defaultMashupNameValue = twPrimitive_CreateFromString("DefaultMashupName", TRUE);
	defaultMenuNameValue = twPrimitive_CreateFromString("DefaultMenuName", TRUE);
	defaultImagelinkValue = twPrimitive_CreateFromString("http://DefaultImagelink.net", TRUE);
	defaultHyperlinkValue = twPrimitive_CreateFromString("http://DefaultHyperlink.net", TRUE);
	defaultHTMLValue = twPrimitive_CreateFromString("<html><body>DefaultHTML</body></html>", TRUE);
	defaultXMLValue = twPrimitive_CreateFromString("<xml><body>DefaultHTML</body></xml>", TRUE);
	defaultJSONValue = twPrimitive_CreateFromString("{\"string\":\"default\",\"number\":0}", TRUE);
	defaultQueryValue = twPrimitive_CreateFromString("{\"string\":\"default\",\"number\":0}", TRUE);
	defaultLocationValue = twPrimitive_CreateFromLocation(location);

	/* Add NEVER push properties to server entity */
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Boolean", "BOOLEAN", "NEVER", twPrimitive_FullCopy(defaultBooleanValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Integer", "INTEGER", "NEVER", twPrimitive_FullCopy(defaultIntegerValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Number", "NUMBER", "NEVER", twPrimitive_FullCopy(defaultNumberValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Datetime", "DATETIME", "NEVER", twPrimitive_FullCopy(defaultDatetimeValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_String", "STRING", "NEVER", twPrimitive_FullCopy(defaultStringValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Text", "TEXT", "NEVER", twPrimitive_FullCopy(defaultTextValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_ThingName", "THINGNAME", "NEVER", twPrimitive_FullCopy(defaultThingNameValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_GroupName", "GROUPNAME", "NEVER", twPrimitive_FullCopy(defaultGroupNameValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_UserName", "USERNAME", "NEVER", twPrimitive_FullCopy(defaultUserNameValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_MashupName", "MASHUPNAME", "NEVER", twPrimitive_FullCopy(defaultMashupNameValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_MenuName", "MENUNAME", "NEVER", twPrimitive_FullCopy(defaultMenuNameValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Imagelink", "IMAGELINK", "NEVER", twPrimitive_FullCopy(defaultImagelinkValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Hyperlink", "HYPERLINK", "NEVER", twPrimitive_FullCopy(defaultHyperlinkValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_HTML", "HTML", "NEVER", twPrimitive_FullCopy(defaultHTMLValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_XML", "XML", "NEVER", twPrimitive_FullCopy(defaultXMLValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_JSON", "JSON", "NEVER", twPrimitive_FullCopy(defaultJSONValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Query", "QUERY", "NEVER", twPrimitive_FullCopy(defaultQueryValue));
	addPropertyDefinitionToServerThingSimple(entityName, "NeverPush_Location", "LOCATION", "NEVER", twPrimitive_FullCopy(defaultLocationValue));
	addPropertyDefinitionToServerThing(entityName, "NeverPush_InfoTable", "INFOTABLE", FALSE, TRUE, "NeverPush_InfoTable", 5000.0, "NEVER", 0.0, FALSE, 0.0, "NEVER", "", FALSE, "AllPropertyBaseTypesDataShape", twPrimitive_Create(), "");

	/* Add ALWAYS push properties to server entity */
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Boolean", "BOOLEAN", "ALWAYS", defaultBooleanValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Integer", "INTEGER", "ALWAYS", defaultIntegerValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Number", "NUMBER", "ALWAYS", defaultNumberValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Datetime", "DATETIME", "ALWAYS", defaultDatetimeValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_String", "STRING", "ALWAYS", defaultStringValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Text", "TEXT", "ALWAYS", defaultTextValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_ThingName", "THINGNAME", "ALWAYS", defaultThingNameValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_GroupName", "GROUPNAME", "ALWAYS", defaultGroupNameValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_UserName", "USERNAME", "ALWAYS", defaultUserNameValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_MashupName", "MASHUPNAME", "ALWAYS", defaultMashupNameValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_MenuName", "MENUNAME", "ALWAYS", defaultMenuNameValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Imagelink", "IMAGELINK", "ALWAYS", defaultImagelinkValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Hyperlink", "HYPERLINK", "ALWAYS", defaultHyperlinkValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_HTML", "HTML", "ALWAYS", defaultHTMLValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_XML", "XML", "ALWAYS", defaultXMLValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_JSON", "JSON", "ALWAYS", defaultJSONValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Query", "QUERY", "ALWAYS", defaultQueryValue);
	addPropertyDefinitionToServerThingSimple(entityName, "AlwaysPush_Location", "LOCATION", "ALWAYS", defaultLocationValue);
	addPropertyDefinitionToServerThing(entityName, "AlwaysPush_InfoTable", "INFOTABLE", FALSE, TRUE, "AlwaysPush_InfoTable", 5000.0, "ALWAYS", 0.0, FALSE, 0.0, "ALWAYS", "", FALSE, "AllPropertyBaseTypesDataShape", twPrimitive_Create(), "");

	TW_FREE(location);
}

void addTestServiceDefinitionsToServerThing(char *entityName) {
	twInfoTable *parameters = NULL;
	twInfoTable *resultType = NULL;
	parameters = createServiceDefinitionParameterInfoTable("value", "BOOLEAN");
	resultType = createServiceDefinitionParameterInfoTable("result", "BOOLEAN");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Boolean", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "INTEGER");
	resultType = createServiceDefinitionParameterInfoTable("result", "INTEGER");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Integer", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "NUMBER");
	resultType = createServiceDefinitionParameterInfoTable("result", "NUMBER");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Number", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "DATETIME");
	resultType = createServiceDefinitionParameterInfoTable("result", "DATETIME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Datetime", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "STRING");
	resultType = createServiceDefinitionParameterInfoTable("result", "STRING");
	addServiceDefinitionToServerThingSimple(entityName, "Push_String", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "TEXT");
	resultType = createServiceDefinitionParameterInfoTable("result", "TEXT");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Text", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "THINGNAME");
	resultType = createServiceDefinitionParameterInfoTable("result", "THINGNAME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_ThingName", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "GROUPNAME");
	resultType = createServiceDefinitionParameterInfoTable("result", "GROUPNAME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_GroupName", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "USERNAME");
	resultType = createServiceDefinitionParameterInfoTable("result", "USERNAME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_UserName", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "MASHUPNAME");
	resultType = createServiceDefinitionParameterInfoTable("result", "MASHUPNAME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_MashupName", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "MENUNAME");
	resultType = createServiceDefinitionParameterInfoTable("result", "MENUNAME");
	addServiceDefinitionToServerThingSimple(entityName, "Push_MenuName", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "IMAGELINK");
	resultType = createServiceDefinitionParameterInfoTable("result", "IMAGELINK");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Imagelink", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "HYPERLINK");
	resultType = createServiceDefinitionParameterInfoTable("result", "HYPERLINK");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Hyperlink", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "HTML");
	resultType = createServiceDefinitionParameterInfoTable("result", "HTML");
	addServiceDefinitionToServerThingSimple(entityName, "Push_HTML", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "XML");
	resultType = createServiceDefinitionParameterInfoTable("result", "XML");
	addServiceDefinitionToServerThingSimple(entityName, "Push_XML", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "JSON");
	resultType = createServiceDefinitionParameterInfoTable("result", "JSON");
	addServiceDefinitionToServerThingSimple(entityName, "Push_JSON", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "QUERY");
	resultType = createServiceDefinitionParameterInfoTable("result", "QUERY");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Query", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "LOCATION");
	resultType = createServiceDefinitionParameterInfoTable("result", "LOCATION");
	addServiceDefinitionToServerThingSimple(entityName, "Push_Location", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
	parameters = createServiceDefinitionParameterInfoTable("value", "INFOTABLE");
	resultType = createServiceDefinitionParameterInfoTable("result", "INFOTABLE");
	addServiceDefinitionToServerThingSimple(entityName, "Push_InfoTable", parameters, resultType);
	twInfoTable_Delete(parameters); twInfoTable_Delete(resultType);
}

char *createServerTestThing() {
	char *thingName = NULL;
	twDataShape *allPropertyBaseTypesDataShape = createAllPropertyBaseTypesDataShape();
	createServerDataShape("AllPropertyBaseTypesDataShape", allPropertyBaseTypesDataShape);
	thingName = createUniqueServerThing("c_sdk_test_thing", "RemoteThingWithFileTransfer");
	addTestPropertyDefinitionsToServerThing(thingName);
	registerTestProperties(thingName);
	addTestServiceDefinitionsToServerThing(thingName);
	registerTestServices(thingName);
	return thingName;
}

char *twTest_StringReplace(char *needle, char *haystack, char *replaceWith) {
	int i;
	size_t instring_size, new_size, old_size, diffsize, diffsizeAll, outstring_size;
	char *outstring, *test;

	if (!needle || !haystack || !replaceWith) {
		return (char *) NULL;
	}

	instring_size = strlen(needle);
	new_size = strlen(replaceWith);
	old_size = strlen(haystack);
	diffsize = new_size - old_size;
	diffsizeAll = diffsize;
	outstring_size = instring_size * 2 + 1;

	test=(char*)TW_MALLOC(old_size+1);
	outstring =(char*) calloc(outstring_size, sizeof(char));

	if (!outstring || !test) {
		return (char *) NULL;
	}
	if (instring_size < old_size || old_size == 0) {
		strcpy(outstring, needle);
		free(test);
		return outstring;
	}
	outstring[0] = '\0';
	for (i = 0; i <= instring_size; i++) {
		strncpy(test, (needle + i), old_size);
		test[old_size] = '\0';
		if (strcmp(test, haystack) == 0) {
			if ((instring_size + diffsizeAll) > outstring_size) {
				outstring_size = outstring_size * 2 + 1;
				outstring = realloc(outstring, outstring_size);
				if (!outstring) {
					free(test);
					return (char *) NULL;
				}
			}
			strcat(outstring, replaceWith);
			i = i + old_size - 1;
			diffsizeAll = diffsizeAll + diffsize;
		} else {
			test[1] = '\0';
			strcat(outstring, test);
		}
	}
	TW_FREE(test);
	return outstring;
}

/* These services fall under the "FileSystemServices" thing shape */

twInfoTable *twTest_InvokeService_BrowseDirectory(char *thingName, char *path) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(path)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "BrowseDirectory", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_BrowseDirectory: Failed to invoke \"BrowseDirectory\" service"
	);

	twInfoTable_Delete(params);

	return result;
}

void twTest_InvokeService_DeleteFile(char *thingName, char *path) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(path)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "DeleteFile", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_BrowseFileSystem: Failed to invoke \"BrowseFileSystem\" service"
	);

	twInfoTable_Delete(params);
	twInfoTable_Delete(result);
}

twInfoTable *twTest_InvokeService_GetFileInfo(char *thingName, char *path) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(path)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "GetFileInfo", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_GetFileInfo: Failed to invoke \"GetFileInfo\" service"
	);

	twInfoTable_Delete(params);

	return result;
}

twInfoTable *twTest_InvokeService_ListDirectories(char *thingName, char *path) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(path)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "ListDirectories", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_ListDirectories: Failed to invoke \"ListDirectories\" service"
	);

	twInfoTable_Delete(params);

	return result;
}

twInfoTable *twTest_InvokeService_ListFiles(char *thingName, char *path) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(path)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "ListFiles", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_ListFiles: Failed to invoke \"ListFiles\" service"
	);

	twInfoTable_Delete(params);

	return result;
}

void twTest_InvokeService_MoveFile(char *thingName, char *sourcePath, char *targetPath, char overwrite) {
	twInfoTable *params = NULL;
	twInfoTable *result = NULL;

	params = TW_MAKE_INFOTABLE(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("sourcePath", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("targetPath", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("overwrite", TW_NO_DESCRIPTION, TW_BOOLEAN)
			),
			TW_IT_ROW(
					TW_MAKE_STRING(sourcePath),
					TW_MAKE_STRING(targetPath),
					TW_MAKE_BOOL(overwrite)
			)
	);

	TEST_ASSERT_EQUAL_MESSAGE(TW_OK,
	                          twApi_InvokeService(TW_THING, thingName, "MoveFile", params, &result, 5000, TRUE),
	                          "twTest_InvokeService_MoveFile: Failed to invoke \"MoveFile\" service"
	);

	twInfoTable_Delete(params);
	twInfoTable_Delete(result);
}


/* These services below fall under the "FileTransfer" thing shape.
 *
 * Some of these services have been replaced by services in the "FileSystemServices" thing shape.  We will invoke these
 * "the old way" to serve as an example. */

twInfoTable *twTest_InvokeService_BrowseFileSystem(char *thingName, char *path) {
	int res = TW_UNKNOWN_ERROR;
	twInfoTable *itIn = NULL;
	twInfoTable *itOut = NULL;
	twDataShapeEntry *dse = NULL;
	twDataShape *ds = NULL;
	twInfoTableRow *row = NULL;

	/* Create the data shape */
	dse = twDataShapeEntry_Create("path", NULL, TW_STRING);
	ds = twDataShape_Create(dse);

	/* Create the info table */
	itIn = twInfoTable_Create(ds);
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(path, TRUE));
	twInfoTable_AddRow(itIn, row);

	/* Invoke the service */
	res = twApi_InvokeService(TW_THING, thingName, "BrowseFileSystem", itIn, &itOut, 5000, TRUE);
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, res, "twTest_InvokeService_BrowseFileSystem: Failed to invoke \"BrowseFileSystem\" service");

	/* Clean up */
	twInfoTable_Delete(itIn);

	return itOut;
}

twInfoTable *twTest_InvokeService_GetDirectoryStructure(char *thingName) {
	int res = TW_UNKNOWN_ERROR;
	twInfoTable *itOut = NULL;

	/* Invoke the service */
	res = twApi_InvokeService(TW_THING, thingName, "GetDirectoryStructure", NULL, &itOut, 5000, TRUE);
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, res, "twTest_InvokeService_GetDirectoryStructure: Failed to invoke \"GetDirectoryStructure\" service");

	return itOut;
}



char *getUniqueThingName(char *prefix) {
	int i = 0;
	for (i = 0; i < 1000; i++) {
		char *thingName = malloc(sizeof(char) * 1024);
		char *id = twItoa(i);
		strcpy(thingName, prefix);
		strcat(thingName, "_");
		strcat(thingName, id);
		free(id);
		if (!doesServerEntityExist("Thing", thingName)) {
			twLog(TW_DEBUG, "Server thing %s is not in use, returning %s", thingName, thingName);
			return thingName;
		}
		twLog(TW_DEBUG, "Server thing %s is in use", thingName);
		free(thingName);
	}
	return NULL;
}

char *importAndCloneUniqueTestEntity(char *fileName, char *thingName) {
	char *newThingName = NULL;
	newThingName = getUniqueThingName(thingName);
	if(FALSE==importEntityFileFromEtc(fileName))
		return NULL;
	if(FALSE==cloneThing(thingName, newThingName))
		return NULL;
	if(FALSE==enableThing(newThingName))
		return NULL;
	if(FALSE==restartThing(newThingName))
		return NULL;
	return newThingName;
}

char cloneThing(const char *thingName, const char *newThingName) {
	char res;
	twInfoTable *itIn = NULL;
	twInfoTable* itOut = NULL;
	twDataShapeEntry* dse = NULL;
	twDataShape* ds = NULL;
	twInfoTableRow* itRow = NULL;
	/* Create the service input data shape */
	dse = twDataShapeEntry_Create("name", NULL, TW_STRING); ds = twDataShape_Create(dse);
	dse = twDataShapeEntry_Create("description", NULL, TW_STRING); twDataShape_AddEntry(ds,dse);
	dse = twDataShapeEntry_Create("sourceThingName", NULL, TW_THINGNAME); twDataShape_AddEntry(ds,dse);
	dse = twDataShapeEntry_Create("tags", NULL, TW_TAGS); twDataShape_AddEntry(ds, dse);
	/* Create an info table using this data shape */
	itIn = twInfoTable_Create(ds);
	/* Create an info table row with the necessary service inputs */
	itRow = twInfoTableRow_Create(twPrimitive_CreateFromString(newThingName, TRUE));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromString("Cloned by C SDK test suite", TRUE));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromString(thingName, TRUE));
	twInfoTableRow_AddEntry(itRow, twPrimitive_CreateFromString("", TRUE));
	/* Add the row to our info table */
	twInfoTable_AddRow(itIn, itRow);
	/* Invoke the service with our input info table */
	res = invokeServiceWithRetries(TW_RESOURCE, "EntityServices", "CloneThing", itIn, &itOut, 5000, TRUE);
	/* Clean up */
	if (itOut) twInfoTable_Delete(itOut);
	/* Return TRUE on success, FALSE on failure */
	if (res == TW_OK) return TRUE;
	return FALSE;
}

char enableThing(const char *thingName) {
	char res;
	twInfoTable* itOut = NULL;
	/* Invoke the service */
	res = invokeServiceWithRetries(TW_THING, (char*)thingName, "EnableThing", NULL, &itOut, 5000, TRUE);
	/* Clean up */
	if (itOut) twInfoTable_Delete(itOut);
	/* Return TRUE on success, FALSE on failure */
	if (res == TW_OK) return TRUE;
	return FALSE;
}

char restartThing(const char *thingName) {
	char res;
	twInfoTable* itOut = NULL;
	/* Invoke the service */
	res = invokeServiceWithRetries(TW_THING, (char*)thingName, "RestartThing", NULL, &itOut, 5000, TRUE);
	/* Clean up */
	if (itOut) twInfoTable_Delete(itOut);
	/* Return TRUE on success, FALSE on failure */
	if (res == TW_OK) return TRUE;
	return FALSE;
}

int invokeServiceWithRetries(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	int res = TW_UNKNOWN_ERROR;
	int retries = 0;
	while (res != TW_OK && retries < 10) {
		res = twApi_InvokeService(entityType, entityName, serviceName, params, result, timeout, forceConnect);
		retries++;
	}
	return res;
}

twSubscribedProperty * twSubscribedProperty_Create (char * e, char * n, twPrimitive * v, DATETIME t, char * q, char fold) ;
uint32_t  twSubscribedProperty_GetSize(twSubscribedProperty *p);
int twTest_GetQueueSizeOfOnePropertyChange(char *thingName, char *propertyName){
	int queueSizeOfOnePropertyChange = 0;
	twPrimitive *value = twPrimitive_CreateFromInteger(1);
	twSubscribedProperty *calibrationPropertyChange = twSubscribedProperty_Create(thingName, propertyName, value, NULL, NULL, FALSE);
	queueSizeOfOnePropertyChange = twSubscribedProperty_GetSize(calibrationPropertyChange);
	twSubscribedProperty_Delete(calibrationPropertyChange);
	twPrimitive_Delete(value);
	return queueSizeOfOnePropertyChange;
}

void twTest_DeleteAndSetPersistedBinFileLocations() {
	twDirectory_DeleteFile("subscribed_properties.bin");
	twDirectory_DeleteFile("offline_msgs.bin");
	twcfg_pointer->subscribed_props_dir = ".";
	twcfg_pointer->offline_msg_store_dir = ".";
	twOfflineMsgStore_Enable();
}

DATETIME *twTest_GetConsecutiveTimestamps(DATETIME currentTime, int count) {
	DATETIME *t = NULL;
	int i = 0;
	t = TW_MALLOC(sizeof(DATETIME) * count);
	for (i = 0; i < count; i++) {
		t[i] = currentTime + i;
	}
	return t;
}

int loadCACertFromEtc(const char *fileName){
	char * path_buffer = NULL;
	int ret;
	/* init path buffer */
	path_buffer = (char*)TW_CALLOC(MAX_FILE_PATH, 1);
	snprintf(path_buffer, MAX_FILE_PATH-1, "%s/%s",configurationDirectory,fileName);
	/* load ca cert - only supports .PEM certs */
	ret = twApi_LoadCACert(path_buffer,0);
	/* cleanup string memory before returning */
	TW_FREE(path_buffer);
	return ret;
}

char * joinPath(const char * root, const char * tail) {
#ifdef WIN32
    char os_sep[] = "\\";
#else
    char os_sep[] ="/";
#endif
    char * joined = NULL;
    size_t bufsize = strlen(root) + strlen(os_sep) + strlen(tail) + 1;

    /* Allocate memory. */
    joined = (char *) TW_CALLOC(bufsize, 1);

    /* Concatenate. */
    strcpy(joined, root);
    strcat(joined, os_sep);
    strcat(joined, tail);

    return joined;
}

int inArray(const char *src, const char **array, const size_t items) {
    size_t i;
    for (i = 0; i < items; ++i) {
        if (strcmp(src, array[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

void twcfgResetAll() {
	twcfg_pointer->subscribed_props_enabled =TRUE;
	twcfg_pointer->max_message_size = MAX_MESSAGE_SIZE;
	twcfg_pointer->message_chunk_size = MESSAGE_CHUNK_SIZE;
	twcfg_pointer->default_message_timeout = DEFAULT_MESSAGE_TIMEOUT;
	twcfg_pointer->ping_rate = PING_RATE;
	twcfg_pointer->pong_timeout= DEFAULT_PONG_TIMEOUT;
	twcfg_pointer->stale_msg_cleanup_rate = STALE_MSG_CLEANUP_RATE;
	twcfg_pointer-> connect_timeout = CONNECT_TIMEOUT;
	twcfg_pointer->connect_retries = CONNECT_RETRIES;
	twcfg_pointer->duty_cycle = DUTY_CYCLE;
	twcfg_pointer->duty_cycle_period = DUTY_CYCLE_PERIOD;
	twcfg_pointer->stream_block_size = STREAM_BLOCK_SIZE;
	twcfg_pointer->file_xfer_block_size = FILE_XFER_BLOCK_SIZE;
	twcfg_pointer->file_xfer_max_file_size = FILE_XFER_MAX_FILE_SIZE;
	twcfg_pointer->file_xfer_md5_block_size = FILE_XFER_MD5_BLOCK_SIZE;
	twcfg_pointer->file_xfer_timeout = FILE_XFER_TIMEOUT;
	twcfg_pointer->file_xfer_staging_dir = FILE_XFER_STAGING_DIR;
	twcfg_pointer->offline_msg_queue_size = OFFLINE_MSG_QUEUE_SIZE;
	twcfg_pointer->subscribed_props_queue_size = OFFLINE_MSG_QUEUE_SIZE;
	twcfg_pointer->subscribed_props_queue_size = OFFLINE_MSG_QUEUE_SIZE;
	twcfg_pointer->max_connect_delay = MAX_CONNECT_DELAY;
	twcfg_pointer->connect_retry_interval = CONNECT_RETRY_INTERVAL;
	twcfg_pointer->max_messages = MAX_MESSAGES;
	twcfg_pointer->socket_read_timeout = DEFAULT_SOCKET_READ_TIMEOUT;
	twcfg_pointer->ssl_read_timeout = DEFAULT_SSL_READ_TIMEOUT; 
	twcfg_pointer->frame_read_timeout = FRAME_READ_TIMEOUT;
	twcfg_pointer->initialize_encryption_library = TRUE;
	twcfg_pointer->cipher_set = TW_SSL_DEFAULT_CIPHER_STRING;
	twcfg_pointer->max_string_prop_length = MAX_STRING_PROP_LENGTH;
}

int stub_twWs_Connect_MockSuccess(twWs *ws, uint32_t timeout) {
	tw_api->connectionInProgress = FALSE;
	tw_api->mh->ws->isConnected = TRUE;
	return TW_OK;
}

enum msgCodeEnum stub_sendMessageBlocking_MockSuccess(twMessage **msg, int32_t timeout, twInfoTable **result) {
	return TWX_SUCCESS;
}

twInfoTable *createMockFtiIt() {
	twInfoTable *it = NULL;
	it = TW_MAKE_IT(
			TW_MAKE_DATASHAPE(
					TW_SHAPE_NAME_NONE,
					TW_DS_ENTRY("sourceRepository", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("sourcePath", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("sourceFile", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("sourceChecksum", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("targetRepository", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("targetPath", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("targetFile", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("targetChecksum", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("startTime", TW_NO_DESCRIPTION, TW_DATETIME),
					TW_DS_ENTRY("endTime", TW_NO_DESCRIPTION, TW_DATETIME),
					TW_DS_ENTRY("duration", TW_NO_DESCRIPTION, TW_INTEGER),
					TW_DS_ENTRY("state", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("isComplete", TW_NO_DESCRIPTION, TW_BOOLEAN),
					TW_DS_ENTRY("size", TW_NO_DESCRIPTION, TW_NUMBER),
					TW_DS_ENTRY("transferId", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("user", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("message", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("path", TW_NO_DESCRIPTION, TW_STRING),
					TW_DS_ENTRY("data", TW_NO_DESCRIPTION, TW_STRING)
			),
			TW_IT_ROW(
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_DATETIME(0),
					TW_MAKE_DATETIME(0),
					TW_MAKE_INT(0),
					TW_MAKE_STRING("foo"),
					TW_MAKE_BOOL(TRUE),
					TW_MAKE_NUMBER(0.0),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("foo"),
					TW_MAKE_STRING("path0/abc.txt"),
					TW_MAKE_STRING("foo")
			)
	);
	return it;
}