#define NDEBUG
#include "twApi.h"
#include "twTunnelManager.h"
#include "TestUtilities.h"
#include "unity.h"
#include "unity_fixture.h"
#include "integrationTestDefs.h"
#include "twApiStubs.h"
#include "twThreads.h"

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi *tw_api;
extern twApi_Stubs *twApi_stub;
#endif

typedef struct twSocketAcceptServerData {
    int listen_fd;
} twSocketAcceptServerData;

/*      Tunneling Event Callback           */
void IntegrationtunnelCallbackFunc(char started, const char *tid, const char *thingName, const char *peerName,
                                   const char *host, int16_t port, DATETIME startTime, DATETIME endTime,
                                   uint64_t bytesSent, uint64_t bytesRcvd, const char *type, const char *msg,
                                   void *userdata) {
    char startString[100] = "UNKNOWN";
    char endString[100] = "Still in Progress";
    uint32_t duration = endTime ? endTime - startTime : 0;

    if (startTime) twGetTimeString(startTime, startString, "%Y-%m-%d %H:%M:%S", 99, TRUE, FALSE);
    if (endTime) twGetTimeString(endTime, endString, "%Y-%m-%d %H:%M:%S", 99, TRUE, FALSE);

    TW_LOG(TW_AUDIT,
           "\n\n*****************\nTUNNEL NOTIFICATION:\nID: %s\nThingName: %s\nState: %s\nTarget: %s:%d\nStartTime: %s\nEndTime: %s\nDuration: %d msec\nUser: %s\nBytes Sent: %llu\nBytes Rcvd: %llu\nMessage: %s\n*****************\n",
           tid ? tid : "UNKNOWN", thingName ? thingName : "UNKNOWN", started ? "STARTED" : "ENDED",
           host ? host : "UNKNOWN", port, startString, endString, duration, peerName ? peerName : "Unknown", bytesSent,
           bytesRcvd, msg ? msg : "");

}

/* prototype for private tunnel function */
enum msgCodeEnum
tunnelServiceCallback(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content);

/* global tunnel thread used in SETUP and TEAR_DOWN */
twThread * tunnel_integration_thread;

/* echo server function */
/* must be run in seperate thread */
void socket_accept_server(DATETIME now, void *params) {
    int listen_fd, comm_fd;

    twSocketAcceptServerData *data = (twSocketAcceptServerData *)params;
    listen_fd = data->listen_fd;

    comm_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL);
}

TEST_GROUP(TunnelingIntegration);

void test_TunnelingIntegrationAppKey_callback(char *passWd, unsigned int len){
    strncpy(passWd,TW_APP_KEY,len);
}

TEST_SETUP(TunnelingIntegration) {
	/* start api and message handling threads */
	TEST_ASSERT_EQUAL_MESSAGE(TW_OK, start_threaded_api(TW_HOST, TW_PORT, TW_URI, test_TunnelingIntegrationAppKey_callback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE), "TEST_SETUP:OfflineMsgStoreIntegration: Could not start api thread");


    /* register tunnel callback */
    TEST_ASSERT_EQUAL(TW_OK, twTunnelManager_RegisterTunnelCallback(IntegrationtunnelCallbackFunc, NULL, NULL));

	
	/* start tunnel manager thread */
	tunnel_integration_thread = twThread_Create(twTunnelManager_TaskerFunction, 5, NULL, NULL);

}

TEST_TEAR_DOWN(TunnelingIntegration) {
	/* sleep for 100ms to allow tunnel manager to delete idle tunnels */
	twSleepMsec(100);

	/* tear down tunnel manager thread */
	twThread_Delete(tunnel_integration_thread);

	/* tear down api and message handling threads */
	TEST_ASSERT_EQUAL(TW_OK, tear_down_threaded_api());
}

TEST_GROUP_RUNNER(TunnelingIntegration) {
    RUN_TEST_CASE(TunnelingIntegration, test_TestTunnel);
}

TEST(TunnelingIntegration, test_TestTunnel) {
    twInfoTable *params = NULL;
    twInfoTable *content = NULL;
    twThread *echo_thread = NULL;
    int listen_fd;
    struct sockaddr_in servaddr;
    twSocketAcceptServerData *userData = NULL;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(1337);
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listen_fd, 10);
    userData = TW_MALLOC(sizeof(twSocketAcceptServerData));
    userData->listen_fd = listen_fd;

    /* start a tcp echo server on port 1337 */
    echo_thread = twThread_Create(socket_accept_server, 100, userData, TRUE);
    TEST_ASSERT_TRUE(NULL != echo_thread);
    twSleepMsec(100);

    /* bad callbacks */
    TEST_ASSERT_EQUAL_MESSAGE(TWX_BAD_REQUEST, tunnelServiceCallback(NULL, "StartTunnel", params, &content),
                              "Error executing tunnel Service Callback");
    TEST_ASSERT_EQUAL_MESSAGE(TWX_BAD_REQUEST, tunnelServiceCallback(TEST_THINGNAME, NULL, params, &content),
                              "Error executing tunnel Service Callback");
    TEST_ASSERT_EQUAL_MESSAGE(TWX_BAD_REQUEST, tunnelServiceCallback(TEST_THINGNAME, "StartTunnel", NULL, &content),
                              "Error executing tunnel Service Callback");
    TEST_ASSERT_EQUAL_MESSAGE(TWX_BAD_REQUEST, tunnelServiceCallback(TEST_THINGNAME, "StartTunnel", params, NULL),
                              "Error executing tunnel Service Callback");

    /* create start params */
    {
        twDataShapeEntry *tid_entry = NULL;
        twDataShapeEntry *type_entry = NULL;
        twDataShapeEntry *chunksize_entry = NULL;
        twDataShapeEntry *idle_timeout_entry = NULL;
        twDataShapeEntry *read_timeout_entry = NULL;
        twDataShapeEntry *connection_entry = NULL;
        twDataShape *param_shape = NULL;
        twInfoTableRow *param_row = NULL;

        /* create data shape entry */
        tid_entry = twDataShapeEntry_Create("tid", NULL, TW_STRING);
        type_entry = twDataShapeEntry_Create("type", NULL, TW_STRING);
        chunksize_entry = twDataShapeEntry_Create("chunksize", NULL, TW_INTEGER);
        idle_timeout_entry = twDataShapeEntry_Create("idle_timeout", NULL, TW_INTEGER);
        read_timeout_entry = twDataShapeEntry_Create("read_timeout", NULL, TW_INTEGER);
        connection_entry = twDataShapeEntry_Create("connection", NULL, TW_STRING);

        /* create data shape */
        param_shape = twDataShape_Create(tid_entry);
        twDataShape_AddEntry(param_shape, type_entry);
        twDataShape_AddEntry(param_shape, chunksize_entry);
        twDataShape_AddEntry(param_shape, idle_timeout_entry);
        twDataShape_AddEntry(param_shape, read_timeout_entry);
        twDataShape_AddEntry(param_shape, connection_entry);

        params = twInfoTable_Create(param_shape);

        /* create infotable row */
        param_row = twInfoTableRow_Create(twPrimitive_CreateFromString("8675309-8675309", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString("tcp_tunnel", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromInteger(8192));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromInteger(30000));
        /* setting a high read timeout to allow echo server to catch up */
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromInteger(1000));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString(
                "{\"message\":\"never gonna give you up, never gonna let you down\",\"host\": \"127.0.0.1\",\"port\": 1337,\"num_connects\": 1}",
                TRUE));

        /* add row to infotable */
        twInfoTable_AddRow(params, param_row);

    }


    /* start tunnel */
    TEST_ASSERT_EQUAL_MESSAGE(TWX_SUCCESS, tunnelServiceCallback(TEST_THINGNAME, "StartTunnel", params, &content),
                              "Error starting tunnel");

    /* cleanup params and content*/
    if (params) twInfoTable_Delete(params);
    if (content) twInfoTable_Delete(content);

    /* create send command params */

    {
        /* create start params */
        twDataShapeEntry *tid_entry = NULL;
        twDataShapeEntry *command_entry = NULL;
        twDataShape *param_shape = NULL;
        twInfoTableRow *param_row = NULL;

        /* create data shape entry */
        tid_entry = twDataShapeEntry_Create("tid", NULL, TW_STRING);
        command_entry = twDataShapeEntry_Create("command", NULL, TW_STRING);

        /* create data shape */
        param_shape = twDataShape_Create(tid_entry);
        twDataShape_AddEntry(param_shape, command_entry);

        params = twInfoTable_Create(param_shape);

        /* create infotable row */
        param_row = twInfoTableRow_Create(twPrimitive_CreateFromString("8675309-8675309", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString("CONNECT", TRUE));

        /* add row to infotable */
        twInfoTable_AddRow(params, param_row);
    }

    /* send CONNECT command */
    TEST_ASSERT_EQUAL_MESSAGE(TWX_SUCCESS,
                              tunnelServiceCallback(TEST_THINGNAME, "TunnelCommandToEdge", params, &content),
                              "Error sending command to tunnel");

    /* cleanup params and content*/
    if (params) twInfoTable_Delete(params);
    if (content) twInfoTable_Delete(content);

    /* create send command params */

    {
        /* create start params */
        twDataShapeEntry *tid_entry = NULL;
        twDataShapeEntry *command_entry = NULL;
        twDataShape *param_shape = NULL;
        twInfoTableRow *param_row = NULL;

        /* create data shape entry */
        tid_entry = twDataShapeEntry_Create("tid", NULL, TW_STRING);
        command_entry = twDataShapeEntry_Create("command", NULL, TW_STRING);

        /* create data shape */
        param_shape = twDataShape_Create(tid_entry);
        twDataShape_AddEntry(param_shape, command_entry);

        params = twInfoTable_Create(param_shape);

        /* create infotable row */
        param_row = twInfoTableRow_Create(twPrimitive_CreateFromString("8675309-8675309", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString("DISCONNECT", TRUE));

        /* add row to infotable */
        twInfoTable_AddRow(params, param_row);

    }

    /* send DISCONNECT command */
    TEST_ASSERT_EQUAL_MESSAGE(TWX_SUCCESS,
                              tunnelServiceCallback(TEST_THINGNAME, "TunnelCommandToEdge", params, &content),
                              "Error sending command to tunnel");

    /* cleanup params and content*/
    if (params) twInfoTable_Delete(params);
    if (content) twInfoTable_Delete(content);

    /* create complete commands */

    {
        /* create start params */
        twDataShapeEntry *tid_entry = NULL;
        twDataShapeEntry *peer_name_entry = NULL;
        twDataShapeEntry *connection_entry = NULL;
        twDataShape *param_shape = NULL;
        twInfoTableRow *param_row = NULL;

        /* create data shape entry */
        tid_entry = twDataShapeEntry_Create("tid", NULL, TW_STRING);
        peer_name_entry = twDataShapeEntry_Create("peer_name", NULL, TW_STRING);
        connection_entry = twDataShapeEntry_Create("connection", NULL, TW_STRING);

        /* create data shape */
        param_shape = twDataShape_Create(tid_entry);
        twDataShape_AddEntry(param_shape, peer_name_entry);
        twDataShape_AddEntry(param_shape, connection_entry);

        params = twInfoTable_Create(param_shape);

        /* create infotable row */
        param_row = twInfoTableRow_Create(twPrimitive_CreateFromString("8675309-8675309", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString("Unknown", TRUE));
        twInfoTableRow_AddEntry(param_row, twPrimitive_CreateFromString(
                "{\"message\":\"never gonna give you up, never gonna let you down\",\"host\": \"127.0.0.1\",\"port\": 1337,\"num_connects\": 1}",
                TRUE));

        /* add row to infotable */
        twInfoTable_AddRow(params, param_row);

    }

    /* complete tunnel */
    TEST_ASSERT_EQUAL_MESSAGE(TWX_SUCCESS, tunnelServiceCallback(TEST_THINGNAME, "CompleteTunnel", params, &content),
                              "Error completing tunnel");

    /* cleanup params and content*/
    if (params) twInfoTable_Delete(params);
    if (content) twInfoTable_Delete(content);

    if (userData) TW_FREE(userData);
	if (echo_thread) twThread_Delete(echo_thread);
}

