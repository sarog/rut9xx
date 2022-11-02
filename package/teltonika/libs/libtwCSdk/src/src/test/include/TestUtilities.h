#ifndef TW_C_SDK_TESTUTILITIES_H
#define TW_C_SDK_TESTUTILITIES_H
/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

#include "twApi.h"
#include "twApiStubs.h"
#include "twServices.h"
#include "twProperties.h"
#include "twFileManager.h"

#ifdef USE_GRAFANA
#include "graphite-client.h"
#include <time.h>
#define MARK_START(metric) long start = getTimestamp();
#define MARK_END(metric) long duration = getTimestamp()-start;graphite_send_plain(metric, duration, time(NULL));
#define METRIC(metric,value) graphite_send_plain(metric, value, time(NULL));
#define GRAPHXY(name,x,xunits,y,yunits) ;

#elif defined USE_HTML_GRAPHING
#include "chart-js-client.h"
#include <time.h>
#define MARK_START(metric) long duration =0;long start = getTimestamp();
#define MARK_END(metric) duration = getTimestamp()-start;chartjs_send_plain(metric, duration, time(NULL));
#define METRIC(metric,value) chartjs_send_plain(metric, value, time(NULL));
#define GRAPHXY(name,x,xunits,y,yunits) chartjs_write_graph(name,x,xunits,y,yunits);
#else
#define METRIC(metric, value) TW_LOG(TW_TRACE, "%s %lu", metric,value)
#define MARK_START(metric) long duration;long start = getTimestamp();
#define MARK_END(metric) duration = getTimestamp()-start;TW_LOG(TW_TRACE, "%lu", duration);
#define GRAPHXY(name, x, xunits, y, yunits) ;
#endif

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
extern __declspec(dllimport) twConfig * twcfg_pointer;
#else
extern twApi *tw_api;
extern twConfig twcfg;
extern twConfig *twcfg_pointer;
#endif

#ifndef CI_BUILD_ENV
#define CI_BUILD_ENV 0
#endif

/* err message buffer size */
#define ERR_MSG_SIZE 512

/* Uncomment the line below to enable tests still under development */
/* #define ENABLE_IGNORED_TESTS */

/* integration defines */
#define NUM_WORKER_THREADS 5
#define THREAD_RATE 5
#define FILE_NAME_BUF_SIZE 225
#define CMD_BUF_SIZE 500
#define INTEGRATION_TEST_CONNECT_TIMEOUT 10000
#define INTEGRATION_TEST_CONNECT_RETRIES 3
#define MAX_FILE_PATH 1024
#define INTEGRATION_TEST_SMALL_DELAY 100
#define STEAM_SENSOR_NAME "SteamSensor"
#define STEAM_SENSOR_REPO_NAME "SteamSensorMashupFileRepository"
#define REGRESSION_THING_NAME "RegressionThing"
#define REGRESSION_REPO_NAME "RegressionFileRepository"

/* unit test defs */
#define UNIT_TEST_SMALL_DELAY 10

/* subscribed property defines */
#define SUBSCRIBED_PROPERTY_ITERATIONS 20
#define SUBSCRIBED_PROPERTY_INTERVAL 100
#define SUBSCRIBED_PROPERTY_LOCATION "."
#define SUBSCRIBED_PROPERTY_LOCATION_FILE "./subscribed_properties.bin"

/* offline message store defines */
#define OFFLINE_MSG_STORE_SIZE 8192
#ifdef WIN32
/* windows does not like relative paths */
#define OFFLINE_MSG_STORE_LOCATION "."
#define OFFLINE_MSG_STORE_LOCATION_FILE "./offline_msgs.bin"
#define OFFLINE_MSG_STORE_LOCATION_BAD "/protected/path/THIS[]/IS)A(*&^%$#BAD!~~~~PAT"
#define OFFLINE_MSG_STORE_LOCATION_DIFFERENT "opt/thingworx"
#define OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE "opt/thingworx/offline_msgs.bin"
#else
/* linux *might* have permission issues with root dirs, so using relative ones except the bad path test */
#define OFFLINE_MSG_STORE_LOCATION "."
#define OFFLINE_MSG_STORE_LOCATION_FILE "./offline_msgs.bin"
#define OFFLINE_MSG_STORE_LOCATION_BAD "/protected/path/THIS[]/IS)A(*&^%$#BAD!~~~~PAT"
#define OFFLINE_MSG_STORE_LOCATION_DIFFERENT "opt/thingworx"
#define OFFLINE_MSG_STORE_LOCATION_DIFFERENT_FILE "opt/thingworx/offline_msgs.bin"
#endif
#define OFFLINE_MSG_TEST_BODY_SIZE 10
#define OFFLINE_MSG_TEST_REQ_ID 1337
#define OFFLINE_MSG_TEST_SESSION_ID 1337
#define OFFLINE_MSG_TEST_ENDPOINT_ID 1337
#define OFFLINE_STORE_DEF 16384
#define OFFLINE_STORE_1M 1024000
#define OFFLINE_STORE_10M 10240000
#define OFFLINE_STORE_MESSAGE_TIMEOUT_DEF 10000
#define OFFLINE_STORE_MESSAGE_TIMEOUT_1M 200000
#define OFFLINE_STORE_MESSAGE_TIMEOUT_10M 2000000
#define OFFLINE_MSG_STORE_SMALL_DELAY 100
#define TEST_MSG_OVERHEAD 2048

/* bind defines */
#define UNITY_INCLUDE_DOUBLE
#define UNITY_DOUBLE_VERBOSE
#define NUM_NAMES 10

#ifdef WIN32
#define PATH_MAX MAX_PATH
#endif

char * twGetParentDirectory(char * path);
/**
 * Use the argv[0] program name to deduce the most likely location of the shared extension directory for
 * loading edge extensions.
 * @return a Path to the extension directory which must be deleted after it is used.
 */
char* twGetPreferedExtensionLoadingDirectory();



/**
 * Only show SDK error logs
 */
void errorLogs();

/**
 * Show all SDK logs
 */
void allLogs();

/**
 * Print the current timestamp
 */
void printTimestamp();

/**
 * Get the current timestamp
 * @return The current timestamp
 */
long getTimestamp();

/**
 * Don't show any SDK logs
 */
void eatLogs();

/**
 * Doesn't do anything
 * @param ptr Useless pointer
 */
void doNothing(void *ptr);

/**
 * Creates a string of a given numerical value
 * @param num The numerical value
 * @return An pointer to the newly allocated string representing the numerical value
 */
char *twItoa(int num);

/**
 * Calculates how many digits a base 10 number is
 * @param num The number to calculate the digits of
 * @return The number of digits in num
 */


int findn(int num);
/**
 * Extracts thing services from the master callback list to see if they are registered.
 */
twServiceDef * findService(char* thingName, char* serviceName);
/**
 * Extracts thing properties from the master callback list to see if they are registered.
 */
twPropertyDef * findProperty(const char* thingName, const char* propertyName);

/**
 * Generates a random string
 * @param size The size of the string to generate
 * @return A pointer to the newly allocated random string
 */
char *generateRandomString(size_t size);
int isShared();

/**
 * Uses a pre initalized API which is connected to a server to verify if an Entity Exists.
 * @param type String values such as Thing ot ThingTemplate
 * @param thingName The name of the thing who's existance you want to verify
 * @return char TRUE if the Thing was found on the server
 */
char doesServerEntityExist(const char *type, const char *thingName);

char createServerDataShape(const char *dataShapeName, twDataShape *dataShape);

/**
 * Uses a pre initalized API which is connected to a server to create a Thing on the server. The created thing is Enabled and Restarted as well.
 * @param thingName The name of the thing to create
 * @param thingTemplate The Thing template to use in creating this thing. Values such as GenericThing or RemoteThing
 * are acceptable.
 * @return char TRUE if the Thing was created.
 */
char createServerThing(const char *thingName, const char *thingTemplate);

/**
 * Deletes a Thing if it exists on the server.
 * @param type Since this can be used on any entity type specify a constant such as TW_THING or TW_THINGTEMPLATE.
 * @param thingName The name of the thing to delete
 * @return char TRUE is the thing did not exist or if it has been sucessfully removed.
 */
char deleteServerThing(enum entityTypeEnum type, const char *thingName);

/**
 * Removes a property from the specified Thing. Note that a call to restartServerThing() is required for any added
 * properties to become visible.
 * @param type Since this function can be used on any entity type specify a constant such as TW_THING or TW_THINGTEMPLATE.
 * @param thingName The name of the thing to remove a property from.
 * @param propertyName The name of the property to create.
 * @return char TRUE if the property is not present or if it was sucessfully deleted.
 */
char removePropertyFromServerEntity(enum entityTypeEnum type, const char *thingName, const char *propertyName);

/**
 * Imports an entity file from the local file system to the server. The test configuration filepath is used to
 * obtain a directory to look for the entity file in.
 * NOTE: This function spawns an external program to perform the import. The program used is defined in the test
 * configuration file and the default is to use curl. If this test is failing, make sure you have curl installed
 * and on your system path.
 * @param fileName The name only of the entity xml file to import to the server.
 * @return char TRUE if the import is sucessfull
 */
char importEntityFileFromEtc(const char *fileName);
/**
 * Returns the path to the current etc directory which contains import files and file which may be used in tests.
 * @return
 */
char * getEtcDirectory();
/**
 * Loads the passed entity file using the default password for a new instance of thingworx.
 * @param fileName
 * @return TRUE if successful
 */
char importBootstrapEntityFileFromEtc(const char *fileName);
/**
 * Adds a property to a pre existing Thing.
 * @param thingName The name of the Thing that a property will be added to
 * @param name The name of the property to add
 * @param type the Type to add. Examples are STRING, BOOLEAN or NUMBER.
 * @param readOnly char Boolean indicating if this property can be written from the server.
 * @param remote char Boolean inficating if this property is remotly bound
 * @param remotePropertyName The property name of the Cache property this proeprty is bound to. Usually the same value
 * as the name parameter
 * @param timeout The timeout in ms to be used when the server attemps to access this property.
 * @param pushType Condition under which this property value should be pushed to the server. Sample value would be ALWAYS or VALUE.
 * @param dataChangeThreshold A number value defining how much a property must change to be recorded to a ValueStream on
 * the server. This value is connected to dataChangeType
 * @param logged char Boolean value indicating that changes in this value should be logged to to a ValueStream on the server.
 * @param pushThreshold A number value indicating how much a property value must change for pushType VALUE to occur.
 * @param dataChangeType Defines how a property must change to be logged. Sample value would be ALWAYS or VALUE.
 * @param category A name used to place this property in a catagory group for sorting and display in the composer.
 * @param persistent char Boolean indicating if this propery should be stored in a database and restored when the server
 * restarts.
 * @param dataShape The name of a datashape to be used on the server if the type of this property is TW_INFOTABLE
 * @param defaultValue A twPrimitive value to use as the default value when this property is first created.
 * @param description A test description to display in the composer when this function appears in a list.
 * @return char Boolean TRUE is this property was created or already exists.
 */
char addPropertyDefinitionToServerThing(const char *thingName, const char *name, const char *type, char readOnly,
                                        char remote,
                                        const char *remotePropertyName, double timeout, const char *pushType,
                                        double dataChangeThreshold, char logged,
                                        double pushThreshold, const char *dataChangeType, const char *category,
                                        char persistent, const char *dataShape,
                                        twPrimitive *defaultValue, const char *description);

/**
 * Adds a property to a pre existing Thing.
 * @param thingName The name of the Thing that a property will be added to
 * @param name The name of the property to add
 * @param type the Type to add. Examples are STRING, BOOLEAN or NUMBER.
 * @param pushType Condition under which this property value should be pushed to the server. Sample value would be ALWAYS or VALUE.
 * @param defaultValue A twPrimitive value to use as the default value when this property is first created.
 * @return char Boolean TRUE is this property was created or already exists.
 */
char addPropertyDefinitionToServerThingSimple(const char *thingName, const char *name, const char *type,
                                              const char *pushType, twPrimitive *defaultValue);

char createTextFileInServerRepo(const char *repoName, const char *path, size_t size);

/**
 * Check a Thing on the server to see if the specified entity has a property declared on it.
 * @param type an enum type allowing both TW_THING and TW_TEMPLATES to be the target of this check
 * @param thingName The name of the Thing to verify the presence of a property on.
 * @param propertyTypeName The type of the Property to look for and verify the presence of. Sample values would be String or Number
 * @param propertyName The name of the Property to look for and verify the presence of.
 * @return char Boolean TRUE if this property exists on the specified Thing
 */
char doesServerThingPropertyExist(enum entityTypeEnum type, const char *thingName, const char *propertyTypeName,
                                  const char *propertyName);

/**
 * When a Thing has its properties modified, any changes made will not become active until the Thing is restarted. This
 * function performs this restart and should be used after one or more properties are added to a Thing.
 * @param type an enum such as TW_THING or TW_TEMPLATES depending on if you want to restart a Thing or a ThingTemplate
 * @param thingName The name of the Thing you want to restart
 * @return char Boolean TRUE if the Thing was sucessfully restarted.
 */
char restartServerThing(enum entityTypeEnum type, const char *thingName);

/**
 * Uses a pre initalized API which is connected to a server to create a unique test Thing on the server. The function
 * will loop through a set of numerical ID's in the form of prefix_id until it finds a thing that does not exist which
 * will then be created. The created thing is Enabled and Restarted as well.
 * @param prefix The prefix to assign to the thing name in the form of prefix_id.
 * @param thingTemplate The Thing template to use in creating this thing. Values such as GenericThing or RemoteThing
 * are acceptable.
 * @return The unique thing name of the server thing created.
 */
char *createUniqueServerThing(char *prefix, char *thingTemplate);

/**
 * Uses a pre initalized API which is connected to a server to create a unique test Thing on the server. The test Thing
 * will be assigned a unique thing name and a set of standard test properties and services will be dynamically added and
 * registered to the thing. The created thing is Enabled and Restarted as well.
 * @return The unique thing name of the server thing created.
 */
char *createServerTestThing();

/**
 * Adds a service to a pre existing Thing.
 * @param thingName The name of the Thing that a property will be added to
 * @param name The name of the service to add
 * @param description A test description to display in the composer when this function appears in a list.
 * @param category A name used to place this property in a catagory group for sorting and display in the composer.
 * @param parameters An info table describing the parameters used by the service.
 * @param resultType An info table describing the return values used by the service.
 * @param remote char Boolean inficating if this service is remotly bound
 * @param remoteServiceName The name of the remote service this service is bound to. Usually the same value
 * as the name parameter
 * @param timeout The timeout in ms to be used when the server attemps to access this service.
 * @return char Boolean TRUE is this service was created or already exists.
 */
char addServiceDefinitionToServerThing(const char *thingName, const char *name, const char *description,
                                       const char *category, twInfoTable *parameters, twInfoTable *resultType,
                                       char remote, const char *remoteServiceName, int timeout);

/**
 * Adds a service to a pre existing Thing.
 * @param thingName The name of the Thing that a property will be added to
 * @param name The name of the service to add
 * @param parameters An info table describing the parameters used by the service.
 * @param resultType An info table describing the return values used by the service.
 * @return char Boolean TRUE is this service was created or already exists.
 */
char addServiceDefinitionToServerThingSimple(const char *thingName, const char *name, twInfoTable *parameters,
                                             twInfoTable *resultType);

/**
 * Checks to see if a file exists in a server repository thing.
 * @param repoName The name of the server repository thing
 * @param path The path to check
 * @return TRUE if the file exists, FALSE otherwsie
 */
char fileExistsInServerRepo(const char *repoName, const char *path);

/**
 * Polls a server repo every second up to timeout to see if a file exists.
 * @param repoName The name of the server repository thing
 * @param path The path to check
 * @param timeout How long to poll for
 * @return TRUE if the file exists, FALSE otherwsie
 */
char pollFileExistsInServerRepo(const char *repoName, const char *path, int timeout);

/**
 * Assigns a value stream to a logged server thing
 * @param thingName The name of the logged server thing
 * @param valueStreamName The name of the value stream to assign to the server thing
 * @return TRUE on success, FALSE otherwise
 */
char assignValueStreamToServerThing(const char *thingName, const char *valueStreamName);

/**
 * Queries the property history of an integer property on a logged server thing with an assigned value stream
 * @param thingName The name of the server thing to query
 * @param maxItems The maximum number of property history items to query
 * @param propertyName The name of the property to query
 * @param startDate Start date of property history items to query
 * @param endDate End date of property history items to query
 * @param oldestFirst If true, items will be ordered oldest first
 * @param query Query filter
 * @return An info table containing all requested property history
 */
twInfoTable *
queryIntegerPropertyHistory(const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate,
                            DATETIME endDate, char oldestFirst, const char *query);

/**
* Queries the property history of a number property on a logged server thing with an assigned value stream
* @param thingName The name of the server thing to query
* @param maxItems The maximum number of property history items to query
* @param propertyName The name of the property to query
* @param startDate Start date of property history items to query
* @param endDate End date of property history items to query
* @param oldestFirst If true, items will be ordered oldest first
* @param query Query filter
* @return An info table containing all requested property history
*/
twInfoTable *
queryNumberPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate,
	DATETIME endDate, char oldestFirst, const char *query);

/**
* Queries the property history of a boolean property on a logged server thing with an assigned value stream
* @param thingName The name of the server thing to query
* @param maxItems The maximum number of property history items to query
* @param propertyName The name of the property to query
* @param startDate Start date of property history items to query
* @param endDate End date of property history items to query
* @param oldestFirst If true, items will be ordered oldest first
* @param query Query filter
* @return An info table containing all requested property history
*/
twInfoTable *
queryBooleanPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate,
							DATETIME endDate, char oldestFirst, const char *query);

/**
* Queries the property history of a string property on a logged server thing with an assigned value stream
* @param thingName The name of the server thing to query
* @param maxItems The maximum number of property history items to query
* @param propertyName The name of the property to query
* @param startDate Start date of property history items to query
* @param endDate End date of property history items to query
* @param oldestFirst If true, items will be ordered oldest first
* @param query Query filter
* @return An info table containing all requested property history
*/
twInfoTable *
queryStringPropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate,
							 DATETIME endDate, char oldestFirst, const char *query);

/**
* Queries the property history of a date time property on a logged server thing with an assigned value stream
* @param thingName The name of the server thing to query
* @param maxItems The maximum number of property history items to query
* @param propertyName The name of the property to query
* @param startDate Start date of property history items to query
* @param endDate End date of property history items to query
* @param oldestFirst If true, items will be ordered oldest first
* @param query Query filter
* @return An info table containing all requested property history
*/
twInfoTable *
queryDateTimePropertyHistory (const char *thingName, const double maxItems, const char *propertyName, DATETIME startDate,
							DATETIME endDate, char oldestFirst, const char *query);

/**
 * Returns the current working directory
 * @return
 */
char *getCurrentDirectory();

/**
 * Purges all property history of a logged server thing
 * @param thingName The server thing to purge the property history of
 * @return
 */
char purgeAllPropertyHistory(const char *thingName);

/**
 * Checks to see if the offline msg store is enabled
 * @return TRUE if the offline msg store is enabled, FALSE otherwise
 */
char twOfflineMsgStore_IsEnabled();

/**
 * Disables the offline msg store
 */
void twOfflineMsgStore_Disable();

/**
 * Enables the offline msg store
 */
void twOfflineMsgStore_Enable();

/**
 * Returns the offline msg store singleton pointer
 */
twOfflineMsgStore *getOfflineMsgStore();

/**
 * Checks to see if a file transfer is active
 * @param tid The transfer id of the file transfer to check the activity of
 * @return TRUE if the file transfer associated with the tid is active, FALSE otherwise
 */
int isTransferActive(char *tid);

/**
 * Checks to see if a file exists
 * @param path The path of the file to check
 * @return TRUE if the file exists, FALSE otherwise
 */
char fileExists(char *path);

/**
 * Polls every second up to specified timeout to see if a file exists
 * @param path The path of the file to check
 * @param timeout How long to poll for
 * @return TRUE if the file exists, FALSE otherwise
 */
char pollFileExists(char *path, int timeout);

/**
 * Removes file transfer test files
 * @param rootPath Root path to the virtual directories
 */
void removeTestFiles(char *rootPath);

/**
* Retrieves the configuration file directory
*/
const char* getConfigDir();

/**
 * Test data struct type for threaded tests
 */
typedef struct twTest_Data {
    void *data;
    int result;
    char isDone;
    TW_MUTEX mtx;
} twTest_Data;

/**
 * Creates a twTest_Data
 * @return A pointer to the newly created twTest_Data
 */
twTest_Data *twTest_CreateData();

/**
 * Deletes a twTest_Data
 * @param data A pointer to the twTest_Data to be deleted
 */
void twTest_DeleteData(twTest_Data *data);

/**
 * Test service to push an integer value to the platform
 * @param thingName The name of the thing
 * @param serviceName The name of the service
 * @param params The parameters of the service
 * @param content The content info table of the service
 * @param userdata User data provided to the service
 * @return The return code of the service
 */
enum msgCodeEnum
pushInteger(const char *thingName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata);

/* integration helpers */
void bindEventCallback(char *entityName, char isBound, void *userdata);

void authEventCallback(char *credType, char *credValue, void *userdata);

void createTestApi(char *entityName, char *host, uint16_t port, char *resource, char *app_key, char *gatewayName,
                   uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect);

void deleteTestApi();

void connectTestApi(char *entityName);

void disconnectTestApi(char *entityName, char *reason);

/* offline helpers */
/* externally defined helper function */
int enableOfflineMsgStore(char enable, char onDisk);

/* helper functions */
int start_threaded_api(char *host, uint16_t port, char *resource,  twPasswdCallbackFunction app_key_function, char *gatewayName,
                       uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect);

int tear_down_threaded_api();

int enable_msg_store(char onDisk);

void test_write_to_store(char enabled, char cycle_connection, char onDisk, char *host, uint16_t port, char *resource,
                         char *app_key, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                         char autoreconnect);

void test_fill_store(char cycle_connection, char onDisk, size_t size, int32_t timeout, char *host, uint16_t port,
                     char *resource, twPasswdCallbackFunction app_key_function, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                     char autoreconnect);

void
test_decrease_store(char onDisk, size_t size, size_t size_2, int32_t timeout, char *host, uint16_t port, char *resource,
                    char *app_key, char *gatewayName, uint32_t messageChunkSize, uint16_t frameSize,
                    char autoreconnect);

void
test_large_message_decrease_store(char onDisk, size_t size, size_t size_2, int32_t timeout, char *host, uint16_t port,
                                  char *resource, char *app_key, char *gatewayName, uint32_t messageChunkSize,
                                  uint16_t frameSize, char autoreconnect, const char *thingName);

/* bind helpers */
void free_list(void *input);

twList *generateBindListLt(uint32_t maxMessageSize);

twList *generateBindListEq(uint32_t maxMessageSize);

twList *generateBindListGt(uint32_t maxMessageSize);

twList *generateBindListGt2(uint32_t maxMessageSize);

char verifyIsBoundHandler(ListEntry *le, void *userData);

char * getVirtualDirPath(const char * thingName, char * vdir);

/**
 * \brief Invokes the BrowseDirectory() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the BrowseDirectory() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return The result info table returned by the BrowseDirectory() service.
 */
twInfoTable *twTest_InvokeService_BrowseDirectory(char *thingName, char *path);

/**
 * \brief Invokes the DeleteFile() service of the DeleteFile thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the DeleteFile() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return Nothing.
 */
void twTest_InvokeService_DeleteFile(char *thingName, char *path);


/**
 * \brief Invokes the GetFileInfo() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the GetFileInfo() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return The result info table returned by the GetFileInfo() service.
 */
twInfoTable *twTest_InvokeService_GetFileInfo(char *thingName, char *path);

/**
 * \brief Invokes the ListDirectories() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the ListDirectories() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return The result info table returned by the ListDirectories() service.
 */
twInfoTable *twTest_InvokeService_ListDirectories(char *thingName, char *path);

/**
 * \brief Invokes the ListFiles() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the ListFiles() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return The result info table returned by the ListFiles() service.
 */
twInfoTable *twTest_InvokeService_ListFiles(char *thingName, char *path);

/**
 * \brief Invokes the MoveFile() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the MoveFile() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return Nothing.
 */
void twTest_InvokeService_MoveFile(char *thingName, char *sourcePath, char *targetPath, char overwrite);

/**
 * \brief Invokes the BrowseFileSystem() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the BrowseFileSystem() service on.
 * \param path[in]			path		The path to pass to the service.
 *
 * \return The result info table returned by the BrowseFileSystem() service.
 */
twInfoTable *twTest_InvokeService_BrowseFileSystem(char *thingName, char *path);

/**
 * \brief Invokes the GetDirectoryStructure() service of the FileSystemServices thing shape included on thing templates
 * "RemoteThingWithFileTransfer" and "RemoteThingWithFileTransferAndTunneling".
 *
 * \param thingName[in]		thingName	The name of the remote thing to invoke the GetDirectoryStructure() service on.
 *
 * \return The result info table returned by the GetDirectoryStructure() service.
 */
twInfoTable *twTest_InvokeService_GetDirectoryStructure(char *thingName);

int restartSocket(twWs * ws);

char * twTest_StringReplace(char *needle,char *haystack,char *replaceWith);

char cloneThing(const char *thingName, const char *newThingName);

char enableThing(const char *thingName);

char restartThing(const char *thingName);

char *getUniqueThingName(char *prefix);

char *importAndCloneUniqueTestEntity(char *fileName, char *thingName);

twDataShape *createAllPropertyBaseTypesDataShape();

int invokeServiceWithRetries(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect);

int twTest_GetQueueSizeOfOnePropertyChange(char *thingName, char *propertyName);

void twTest_DeleteAndSetPersistedBinFileLocations();

DATETIME *twTest_GetConsecutiveTimestamps(DATETIME currentTime, int count);

int loadCACertFromEtc(const char *fileName);

char * joinPath(const char * root, const char * tail);

int inArray(const char *src, const char **array, const size_t items);

void twcfgResetAll();

twInfoTable *createMockFtiIt();

int stub_twWs_Connect_MockSuccess(twWs *ws, uint32_t timeout);

enum msgCodeEnum stub_sendMessageBlocking_MockSuccess(twMessage **msg, int32_t timeout, twInfoTable **result);

#endif //TW_C_SDK_TESTUTILITIES_H
