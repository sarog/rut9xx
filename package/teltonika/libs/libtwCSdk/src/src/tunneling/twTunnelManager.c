/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Tunneling Component
 */

#include "twTunnelManager.h"
#include "twLogger.h"
#include "twInfoTable.h"
#include "twDefinitions.h"
#include "twApi.h"
#include "stringUtils.h"
#include "twList.h"
#include "twInfoTable.h"
#include "twWebsocket.h"
#include "twTls.h"
#include "tomcrypt.h"
#include "cJSON.h"

#define MAX_TID_LEN 512
#define MAX_KEY_LENGTH 500

/***************************************/
/*  Tunnel Information struct   */
/***************************************/
enum tunnelType {
	TCP_TUNNEL,
	UDP_TUNNEL,
	SERIAL_TUNNEL,
	FILE_TUNNEL
};

typedef struct twTunnel {
	char * thingName;
	char * peerName;
	char * host;
	uint16_t port;
	DATETIME startTime;
	DATETIME endTime;
	char markForClose;
	int16_t numConnects;
	char isActive;
	char firstPeerConnect;
	uint64_t bytesSent;
	uint64_t bytesRcvd;
	char * tid;
	int32_t chunksize;
	int32_t readTimeout;
	int32_t idleTimeout;
	DATETIME idleTime;
	char * type;
	enum tunnelType typeEnum;
	twWs * ws;
	void * localSocket;
	char * readBuffer;
	char * readBase64;
	char * writeBuffer;
	char disconnectedByPeer;
	char * msg;
	int32_t connectAttempts;
} twTunnel;

/*
twTunnel_Create - Creates a tunnel info structure
Parameters:
    thingName - (Input) name of the thing the tunnel is associated with
	it - (Input) infotable that contains the params for the tunnel
Return:
	twTunnel * - pointer to the allocated structure, 0 if an error occurred
*/
twTunnel * twTunnel_Create(const char * thingName, twInfoTable * it);

/*
twTunnel_Delete - Delete the tunnel info structure
Parameters:
	tunnel - pointer to the structure to delete
Return:
	Nothing
*/
void twTunnel_Delete(void * tunnel_in);


/***************************************/
/* Singleton Tunnel Manager Structure  */
/* NOTE: openWS and closeTunnels should*/
/* never own memory other than its list*/
/* entry structure */
/***************************************/
typedef struct twTunnelManager {
	TW_MUTEX mtx;
	twDict * openTunnels;
	twDict * closeTunnels;
	twDict * callbacks;
	twDict * openWs;
	twConnectionInfo * info;
} twTunnelManager;

twTunnelManager * tm = NULL;

/***************************************/
/*  Private Tunnel Manager Functions   */
/***************************************/
/*
twTunnelManager_AddTunnel - Registers a new tunnel with the manager
Parameters:
	tunnel - (Input) Pointer to a tunnel stucture that has the information required to run the tunnel.
Return:
	int - 0 if successful, positive integral error code (see twErrors.h) if an error was encountered
*/
int twTunnelManager_AddTunnel(twTunnel * tunnel);

/*
twTunnelManager_DeleteWIthPointer - Delete the file manager singleton
Parameters:
	mgr - pointer to the tunnel manager.  If NULL then the tm singleton is used.
Return:
	int - 0 if successful, positive integral error code (see twErrors.h) if an error was encountered
*/
int twTunnelManager_DeleteWithPointer(twTunnelManager * mgr);

/***************************************/
/*           Helper Functions          */
/***************************************/

/* internal tunnel callback struct */
typedef struct twTunnelCallback {
	tunnel_cb cb;
	char * id;
	void * userdata;
} twTunnelCallback;


/**
 * Converts a twTunnelCallback structure into a representative index key for use in a hashmap.
 * These fields are required to parse a callback info item:
 * cb - callback pointer
 * id - tunnel id (unique per tunnel)
 * userdata - pointer to user data
 *
 * @param data callbackInfo struct pointer with the fields listed above set.
 * @return A string representing a valid hash key for the passed in callbackInfo structure.
 */
const char* twTunnelCallbackInfoParser(void * data){
	twTunnelCallback * tcbInfo = (twTunnelCallback *)data;
	char* indexKey;

	/* Make sure all key fields are provided */
	if(!(tcbInfo->cb && tcbInfo->id)) {
		return NULL;
	}

	indexKey = (char*)TW_MALLOC(MAX_KEY_LENGTH);
	snprintf(indexKey, MAX_KEY_LENGTH, "%i|%s|%i", (int)tcbInfo->cb, tcbInfo->id, (int)tcbInfo->userdata);
	return indexKey;
}

const char* twOpenTunnelParser(void * data){
	twTunnel * tunnel = (twTunnel *)data;
	char* indexKey;

	/* Make sure all key fields are provided */
	if(!(tunnel->tid)) {
		return NULL;
	}

	indexKey = (char*)TW_MALLOC(MAX_KEY_LENGTH);
	snprintf(indexKey, MAX_KEY_LENGTH, "%s", tunnel->tid);
	return indexKey;
}

const char* twCloseTunnelParser(void * data){
	twTunnel * closeTunnel = (twTunnel *)data;
	char* indexKey;

	/* Make sure all key fields are provided */
	if(!(closeTunnel) || !(closeTunnel->ws)) {
		return NULL;
	}

	indexKey = (char*)TW_MALLOC(MAX_KEY_LENGTH);
	snprintf(indexKey, MAX_KEY_LENGTH, "%i", (int)closeTunnel->ws);
	return indexKey;

}
const char* twOpenWsParser(void * data){
	twTunnel * tunnelWs = (twTunnel *)data;
	char* indexKey;

	/* Make sure all key fields are provided */
	if(!(tunnelWs) || !(tunnelWs->ws)) {
		return NULL;
	}

	indexKey = (char*)TW_MALLOC(MAX_KEY_LENGTH);
	snprintf(indexKey, MAX_KEY_LENGTH, "%i", (int)tunnelWs->ws);
	return indexKey;
}

twTunnel * getTunnelFromId(char * id) {
	twTunnel * result_tunnel = NULL;
	twTunnel tunnel_query;

	/* check inputs */
	if (!id || !tm || !tm->openTunnels) {
		TW_LOG(TW_ERROR, "getTunnelFromId: NULL input parameter");
	} else {
		/* set query params */
		tunnel_query.tid = id;

		/* find matching tunnel in open tunnel list */
		twDict_Find(tm->openTunnels,(void*)&tunnel_query, (void**)&result_tunnel);
	}
	return result_tunnel;
}

twTunnel * getTunnelFromWebsocket(twWs * ws) {
	twTunnel * result_tunnel = NULL;
	twTunnel tunnel_query;

	/* check inputs */
	if (!ws || !tm || !tm->openWs) {
		TW_LOG(TW_ERROR, "getTunnelFromWebsocket: NULL input parameter");
	} else {
		/* set query params */
		tunnel_query.ws = ws;

		/* find matching tunnel in open tunnel list */
		twDict_Find(tm->openWs,(void*)&tunnel_query, (void**)&result_tunnel);
	}
	return (twTunnel*)(result_tunnel);
}

int sendTunnelCommand(char * thingName, const char * tid, const char * msg) {
	twDataShape * ds = NULL;
	if (!msg || !thingName || !tid) {
		TW_LOG(TW_ERROR, "sendTunnelCommand: NULL message pointer passed in");
		return TW_INVALID_PARAM;
	}
	ds = twDataShape_Create(twDataShapeEntry_Create("tid", NULL, TW_STRING));
	if (ds) {
		twInfoTable *  it = NULL;
		twDataShape_AddEntry(ds, twDataShapeEntry_Create("command", NULL, TW_STRING));
		it = twInfoTable_Create(ds);
		if (it) {
			twInfoTableRow * row = twInfoTableRow_Create(twPrimitive_CreateFromString(tid, TRUE));
			if (row) {
				twInfoTable * result = NULL;
				twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(msg, TRUE));
				twInfoTable_AddRow(it,row);
				twApi_InvokeService(TW_THING, thingName, "TunnelCommandFromEdge", it, &result, -1, FALSE);
				twInfoTable_Delete(it);
				if (result) twInfoTable_Delete(result);
			}
		}
	}
	return TW_OK;
}

/***************************************/
/*      Tunnel Service Callbacks       */
/***************************************/
enum msgCodeEnum CompleteTunnel(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	char * tid = NULL;
	char * peerName = NULL;
	char * connection = NULL;
	cJSON * connJson = NULL;
	cJSON * tmp = NULL;
	char * msg = NULL;
	twTunnel * tunnel = NULL;
	TW_LOG(TW_DEBUG,"CompleteTunnel: Service callback invoked with entityName: %s", entityName ? entityName : "NULL");
	/* Inputs */
	twInfoTable_GetString(params, "tid", 0, &tid);
	twInfoTable_GetString(params, "peer_name", 0, &peerName);
	twInfoTable_GetString(params, "connection",0,&connection);
	connJson = cJSON_Parse(connection);
	if (connJson) tmp = cJSON_GetObjectItem(connJson, "reason");
	if (tmp && tmp->valuestring) msg = duplicateString(tmp->valuestring);
	cJSON_Delete(connJson);
	TW_FREE(connection);
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	if (!tid || !peerName) {
		TW_LOG(TW_ERROR,"CompleteTunnel: Missing tid");
		if (tid) TW_FREE(tid);
		if (peerName) TW_FREE(peerName);
		if (msg) TW_FREE(msg);
		return TWX_BAD_REQUEST;
	}
	twMutex_Lock(tm->mtx);
	tunnel = getTunnelFromId(tid);
	if (!tunnel) {
		TW_LOG(TW_ERROR,"CompleteTunnel: Error getting tunnel from tid: %s", tid);
		TW_FREE(tid);
		TW_FREE(peerName);
		if (msg) TW_FREE(msg);
		twMutex_Unlock(tm->mtx);
		return TWX_BAD_REQUEST;
	}
	if (peerName && tunnel->peerName) TW_FREE(tunnel->peerName);
	tunnel->peerName = peerName;
	if (msg) {
		if (tunnel->msg) TW_FREE(tunnel->msg);
		tunnel->msg = msg;
	}
	TW_LOG(TW_AUDIT, "TUNNEL ENDED. Entity: %s, Peer: %s, tid: %s, target: %s:%d", entityName, peerName, tunnel->tid, tunnel->host, tunnel->port);
	tunnel->markForClose = TRUE;
	TW_FREE(tid);
	twMutex_Unlock(tm->mtx);
	return TWX_SUCCESS;
}

enum msgCodeEnum StartTunnel(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	twTunnel * tunnel = NULL;
	/* Inputs */

	/* Outputs */
	*content = NULL;
	/* Perform the function */
	tunnel = twTunnel_Create(entityName, params);
	if (!tunnel) {
		TW_LOG(TW_ERROR,"StartTunnel: Error starting tunnel");
		return TWX_INTERNAL_SERVER_ERROR;
	}
	TW_LOG(TW_AUDIT, "TUNNEL CREATED. Entity: %s, tid: %s, target: %s:%d", entityName, tunnel->tid, tunnel->host, tunnel->port);
	return TWX_SUCCESS;
}

void getFormattedTimeString(char * timeStr, int len) {
	twGetSystemTimeString(timeStr, "%Y-%m-%d %H:%M:%S", len, 1, 1);
}

twSocket * createTunnelSocketandLog(twTunnel * tunnel) {
	char timeStr[80] = {0};

	getFormattedTimeString(timeStr, sizeof(timeStr));
	TW_LOG(
		TW_TRACE,
		"TWX_TIME: %s, creating socket to %s:%d for tunnel id: %s",
		timeStr,
		tunnel->host,
		tunnel->port,
		tunnel->tid
	);
	return twSocket_Create(tunnel->host, tunnel->port, 0);
}

int connectTunnelSocketandLog(twTunnel * tunnel) {
	char timeStr[80] = {0};

	getFormattedTimeString(timeStr, sizeof(timeStr));
	TW_LOG(
		TW_TRACE,
		"TWX_TIME: %s, (re)connecting local socket for tid: %s",
		timeStr,
		tunnel->tid
	);

	if (tunnel->numConnects > 0) {
		tunnel->numConnects--;
	}
	return (tunnel->connectAttempts++ == 0) ?
		twSocket_Connect((twSocket *) tunnel->localSocket) :
		twSocket_Reconnect((twSocket *) tunnel->localSocket);
}

int closeTunnelSocketandLog(twTunnel * tunnel) {
	char timeStr[80] = {0};

	getFormattedTimeString(timeStr, sizeof(timeStr));
	TW_LOG(
		TW_TRACE,
		"TWX_TIME: %s, closing local socket for tid: %s",
		timeStr,
		tunnel->tid
	);
	return twSocket_Close((twSocket*)tunnel->localSocket);
}

enum msgCodeEnum TunnelCommandToEdge(const char * entityName, twInfoTable * params, twInfoTable ** content) {
	char * tid = NULL;
	char * command = NULL;
	twTunnel * tunnel = NULL;
	/* Inputs */
	twInfoTable_GetString(params, "tid", 0, &tid);
	twInfoTable_GetString(params, "command", 0, &command);
	/* Outputs */
	*content = NULL;
	/* Perform the function */
	if (!tid || !command) {
		TW_LOG(TW_ERROR,"TunnelCommandToEdge: Missing tid or command");
		if (tid) TW_FREE(tid);
		if (command) TW_FREE(command);
		return TWX_BAD_REQUEST;
	}
	twMutex_Lock(tm->mtx);
	tunnel = getTunnelFromId(tid);
	twMutex_Unlock(tm->mtx);
	if (!tunnel) {
		TW_LOG(TW_ERROR,"TunnelCommandToEdge: Error getting tunnel from tid: %s", tid);
		TW_FREE(tid);
		TW_FREE(command);
		return TWX_BAD_REQUEST;
	}
	if (!strcmp(command,"CONNECT")) {
		tunnel->firstPeerConnect = TRUE;
		/* Execute the command */
		switch (tunnel->typeEnum) {
		case TCP_TUNNEL:
			if (tunnel->numConnects != 0) {
				int res = connectTunnelSocketandLog(tunnel);

				if (res) {
					TW_LOG(TW_ERROR,"TunnelCommandToEdge: Error connecting to %s:%d. tid: %s.  Error: %d", tunnel->host, tunnel->port, tid, res);
					TW_FREE(tid);
					TW_FREE(command);
					return TWX_INTERNAL_SERVER_ERROR;
				}
				tunnel->disconnectedByPeer = FALSE;
				TW_LOG(TW_DEBUG,"TunnelCommandToEdge: Connected to %s:%d. tid: %s.", tunnel->host, tunnel->port, tid);
			} else {
				TW_LOG(TW_WARN,"TunnelCommandToEdge: Exceeded allowable number of connect attempts for tid: %s", tid);
				TW_FREE(tid);
				TW_FREE(command);
				if (tunnel->msg) {
					TW_FREE(tunnel->msg);
					tunnel->msg = NULL;
				}
				tunnel->msg = duplicateString("Exceeded allowable number of connect attempts");
				tunnel->markForClose = TRUE;
				return TWX_BAD_REQUEST;
			}
			break;
		case FILE_TUNNEL:
			tunnel->numConnects = 1;
			tunnel->disconnectedByPeer = FALSE;
			break;
		case UDP_TUNNEL:
		case SERIAL_TUNNEL:
		default:
			/* TODO: support udp & serial */
			twTunnel_Delete(tunnel);
			TW_FREE(tid);
			TW_FREE(command);
			TW_LOG(TW_ERROR, "TunnelCommandToEdge: Unsupported tunnel type: %s", tunnel->type);
			return TWX_BAD_REQUEST;
		}
	} else if (!strcmp(command,"DISCONNECT")) {
		int res = 0;
		switch (tunnel->typeEnum) {
		case TCP_TUNNEL:
			res = closeTunnelSocketandLog(tunnel);
			tunnel->disconnectedByPeer = TRUE;
			if (res) {
				TW_LOG(TW_ERROR,"TunnelCommandToEdge: Error disconnecting from %s:%d. tid: %s.  Error: %d", tunnel->host, tunnel->port, tid, res);
				TW_FREE(tid);
				TW_FREE(command);
				return TWX_INTERNAL_SERVER_ERROR;
			}
			TW_LOG(TW_DEBUG,"TunnelCommandToEdge: Disconnected from %s:%d. tid: %s.", tunnel->host, tunnel->port, tid);
			break;
		case FILE_TUNNEL:
			tunnel->numConnects = 0;
			tunnel->disconnectedByPeer = TRUE;
			break;
		case UDP_TUNNEL:
		case SERIAL_TUNNEL:
		default:
			/* TODO: support udp & serial */
			twTunnel_Delete(tunnel);
			TW_LOG(TW_ERROR, "TunnelCommandToEdge: Unsupported tunnel type: %s", tunnel->type);
			TW_FREE(tid);
			TW_FREE(command);
			return TWX_BAD_REQUEST;
		}
	} else {
		TW_LOG(TW_ERROR,"TunnelCommandToEdge: Unknown command %s", command);
		TW_FREE(tid);
		TW_FREE(command);
		return TWX_BAD_REQUEST;
	}
	TW_FREE(tid);
	TW_FREE(command);
	return TWX_SUCCESS;
}

/********************************/
/* Websocket Callback Functions */
/********************************/

twTunnelCallback * twTunnelCallback_Create(tunnel_cb cb, char * id, void * userdata) {
	twTunnelCallback * tmp = (twTunnelCallback *)TW_CALLOC(sizeof(twTunnelCallback), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twTunnelCallback_Create: Error allocating memory");
		return NULL;
	}
	tmp->cb = cb;
	tmp->id = duplicateString(id);
	tmp->userdata = userdata;
	return tmp;
}

void twTunnelCallback_Delete(void * cb) {
	twTunnelCallback * tmp = (twTunnelCallback *)cb;
	if (!cb) return;
	if (tmp->id) TW_FREE(tmp->id);
	TW_FREE(tmp);
}

int twTunnelManager_OnWsOpenCallback(struct twWs * ws) {
	return 0;
}

int twTunnelManager_OnWsCloseCallback(struct twWs * ws, const char *at, size_t length) {
	twTunnel * tunnel = NULL;
	/* Check to see if we are getting called because we closed this ourselves */
	char msg[128];
	strncpy(msg,at, length > 127 ? 127 : length);
	/* make sure strncpy is placing a null terminator at the end of the string */
	msg[length > 127 ? 127 : length] = NULL;
	if (strstr(msg, "Tunnel stopped by edge")) {
		/* This was closed manually and we should just return */
		return 0;
	}
	/* If we are here, it was closed by the server, so mark it for close now */
	tunnel = getTunnelFromWebsocket(ws);
	if (!tunnel) {
		TW_LOG(TW_WARN,"twTunnelManager_OnWsCloseCallback: couldn't get tunnel id from websocket");
		return 0;
	}
	if (tunnel->msg) {
		TW_FREE(tunnel->msg);
		tunnel->msg = NULL;
	}
	tunnel->msg = duplicateString("Websocket was closed");
	tunnel->markForClose = TRUE;
	return 0;
}

int twTunnelManager_OnWsDataCallback (struct twWs * ws, const char *at, size_t length) {
	twTunnel * tunnel = NULL;
	unsigned long len = 0;
	int res = 0;

	tunnel = getTunnelFromWebsocket(ws);
	if (!tunnel || !at) {
		TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: NULL input params found");
		return 0;
	}
	len = tunnel->chunksize;
	if (base64_decode((const unsigned char *)at, length, (unsigned char *)tunnel->writeBuffer, &len)) {
		TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: Error decoding data");
		return 1;
	}

	switch (tunnel->typeEnum) {
	case TCP_TUNNEL:
		/* Have we connected yet? */
		if (((twSocket *)tunnel->localSocket)->state == CLOSED) {
			if (tunnel->numConnects != 0) {
				res = connectTunnelSocketandLog(tunnel);

				if (res) {
					TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: Error connecting to %s:%d. tid: %s.  Error: %d", tunnel->host, tunnel->port, tunnel->tid, res);
					return 1;
				}
				TW_LOG(TW_DEBUG,"twTunnelManager_OnWsDataCallback: Connected to %s:%d. tid: %s.", tunnel->host, tunnel->port, tunnel->tid);
			} else {
				TW_LOG(TW_WARN,"twTunnelManager_OnWsDataCallback: Exceeded allowable number of connect attempts for tid: %s", tunnel->tid);
				if (tunnel->msg) {
					TW_FREE(tunnel->msg);
					tunnel->msg = NULL;
				}
				tunnel->msg = duplicateString("Exceeded allowable number of connect attempts");
				tunnel->markForClose = TRUE;
				return 1;
			}
		}
		res = twSocket_Write((twSocket *)tunnel->localSocket, tunnel->writeBuffer, len, 1);
		if (res != len) {
			TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: Error writing to %s:%d. tid: %s.  Error: %d", tunnel->host, tunnel->port, tunnel->tid, res);
			if (res < 0) {
				tunnel->disconnectedByPeer = FALSE;
				sendTunnelCommand(tunnel->thingName, tunnel->tid, "DISCONNECT");
			}
			return 0;
		}
		break;
	case FILE_TUNNEL:
		{
		TW_FILE_HANDLE f = 0;
		if (tunnel->localSocket == NULL) {
			TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: NULL file handle pointer");
			return 0;
		}
		if (tunnel->port == 1) {
			TW_LOG(TW_ERROR,"twTunnelManager_OnWsDataCallback: tid: %s. File %s is open for read, but received data", tunnel->tid, tunnel->host);
			return 0;
		}
		f = *(TW_FILE_HANDLE *)tunnel->localSocket;
		if (TW_FWRITE(tunnel->writeBuffer, 1, len, f) != len) {
			TW_LOG(TW_WARN,"twTunnelManager_OnWsDataCallback: Error writing to file %s tid: %s", tunnel->host, tunnel->tid);
			if (tunnel->msg) {
				TW_FREE(tunnel->msg);
				tunnel->msg = NULL;
			}
			tunnel->msg = duplicateString("Error writing to file");
			tunnel->markForClose = TRUE;
			return 1;
		}
		return 0;
		}
		break;
	case UDP_TUNNEL:
	case SERIAL_TUNNEL:
	default:
		/* TODO: support udp & serial */
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Unsupported tunnel type: %s", tunnel->type);
		return 0;
	}

	TW_LOG(TW_TRACE,"twTunnelManager_OnWsDataCallback: Wrote %d bytes to local socket. for tid: %s", res, tunnel->tid);
	tunnel->bytesRcvd += res;
	return 0;
}

/***************************************/
/*           Tunnel Functions          */
/***************************************/
twTunnel * twTunnel_Create(const char * thingName, twInfoTable * it) {
	twTunnel * tunnel = NULL;
	char * connection = NULL;
	cJSON * connJson = NULL;
	cJSON * tmp = NULL;
	int res = 0;
	char * resource = NULL;

	if (!thingName || !it || !tm) {
		TW_LOG(TW_ERROR, "twTunnel_Create: NULL input parameter");
		return NULL;
	}
	tunnel = (twTunnel *)TW_CALLOC(sizeof(twTunnel), 1);
	if (!tunnel) {
		TW_LOG(TW_ERROR, "twTunnel_Create: Error allocating storage");
		return NULL;
	}
	/* set up some defaults */
	tunnel->chunksize = 8192;
	tunnel->idleTimeout = 30000;
	tunnel->readTimeout = 10;
	tunnel->numConnects = 1;
	/* Get our tunnel config params */
	tunnel->thingName = duplicateString(thingName);
	twInfoTable_GetString(it, "tid",0,&tunnel->tid);
	twInfoTable_GetString(it, "type",0,&tunnel->type);
	twInfoTable_GetInteger(it, "chunksize",0,&tunnel->chunksize);
	twInfoTable_GetInteger(it, "idle_timeout",0,&tunnel->idleTimeout);
	twInfoTable_GetInteger(it, "read_timeout",0,&tunnel->readTimeout);
	tunnel->disconnectedByPeer = TRUE;

	/* Allocate our buffers */
	tunnel->readBuffer = (char *)TW_CALLOC(tunnel->chunksize,1);
	tunnel->writeBuffer = (char *)TW_CALLOC(tunnel->chunksize,1);
	tunnel->readBase64 = (char *)TW_CALLOC((tunnel->chunksize * 4)/3,1);
	/* Check some required params */
	if ( !tunnel->tid || !tunnel->type || !tunnel->readBuffer || !tunnel->writeBuffer || !tunnel->readBase64) {
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Missing required params");
		return NULL;
	}
	TW_LOG(TW_DEBUG, "twTunnel_Create: Creating tunnel. tid: ", tunnel->tid);
	/*
	Create the websocket at /Thingworx/WSTunnelServer/" + tid but
	don't connect yet.  Want to leave connection to the tasker so that
	we don't block the original websocket connection to the server
	*/
	resource = (char *)TW_CALLOC(strnlen("/Thingworx/WSTunnelServer/", 26) + strnlen(tunnel->tid, MAX_TID_LEN) + 1, 1);
	if (!resource) {
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Error allocating memory");
		return NULL;
	}
	strcpy(resource,"/Thingworx/WSTunnelServer/");
	strcat(resource, tunnel->tid);
	res = twWs_Create(tm->info->ws_host, tm->info->ws_port, resource, tm->info->appkeyFunction, NULL, (tunnel->chunksize * 4)/3, (tunnel->chunksize * 4)/3, &tunnel->ws);
	TW_FREE(resource);
	if (res) {
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Error creating websocket");
		return NULL;
	}

	/*
	 * CSDK-1218: Forcing disable compression for tunnels. If compression is
	 * wanted for tunneling, we would have to either use binary websockets,
	 * where compression has been implemented, or implement compression on text
	 * sockets because it hasn't been implemented yet. Preferably the latter so
	 * we are in compliance with the spec.
	 */
	tunnel->ws->bDisableCompression = TRUE;

	/* Set up any additional connection info */
	if (tm->info) {
#ifdef ENABLE_HTTP_PROXY_SUPPORT
		if (tm->info->proxy_host && tm->info->proxy_port) twSocket_SetProxyInfo(tunnel->ws->connection->connection, tm->info->proxy_host, tm->info->proxy_port,
																				tm->info->proxy_pwd, tm->info->proxy_user);
#endif
		if (tm->info->ca_cert_file) twTlsClient_SetClientCaList(tunnel->ws->connection, tm->info->ca_cert_file, 0);
		if (tm->info->client_cert_file) twTlsClient_UseCertificateFile(tunnel->ws->connection, tm->info->client_cert_file, 0);
		if (tm->info->client_key_file && tm->info->client_key_passphrase) {
			twTlsClient_SetDefaultPasswdCb(tunnel->ws->connection, tm->info->client_key_passphrase);
			twTlsClient_UsePrivateKeyFile(tunnel->ws->connection, tm->info->client_key_file, 0);
		}
		if (tm->info->disableEncryption) twTlsClient_DisableEncryption(tunnel->ws->connection);
		if (tm->info->selfsignedOk) twTlsClient_SetSelfSignedOk(tunnel->ws->connection);
		if (tm->info->doNotValidateCert) twTlsClient_DisableCertValidation(tunnel->ws->connection);
		twTlsClient_SetX509Fields(tunnel->ws->connection, tm->info->subject_cn, tm->info->subject_o, tm->info->subject_ou,
														tm->info->issuer_cn, tm->info->issuer_o, tm->info->issuer_ou);
	}

	/* Set the type enum */
	lowercase(tunnel->type);
	if (!strcmp(tunnel->type, "file")) tunnel->typeEnum = FILE_TUNNEL;
	else if (!strcmp(tunnel->type, "udp")) tunnel->typeEnum = UDP_TUNNEL;
	else if (!strcmp(tunnel->type, "serial")) tunnel->typeEnum = SERIAL_TUNNEL;
	else tunnel->typeEnum = TCP_TUNNEL;

	/* Create the local connection but don't connect yet */
	twInfoTable_GetString(it, "connection",0,&connection);
	if (!connection) {
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: No connection info found");
		return NULL;
	}
	/* Convert to cJSON struct */
	connJson = cJSON_Parse(connection);
	if (!connJson) {
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Error converting %s to JSON struct", connection);
		TW_FREE(connection);
		return NULL;
	}
	tmp = cJSON_GetObjectItem(connJson, "message");
	if (tmp && tmp->valuestring) tunnel->msg = duplicateString(tmp->valuestring);
	switch (tunnel->typeEnum) {
	case TCP_TUNNEL:
		tmp = cJSON_GetObjectItem(connJson, "host");
		if (tmp && tmp->valuestring) tunnel->host = duplicateString(tmp->valuestring);
		tmp = cJSON_GetObjectItem(connJson, "port");
		if (tmp && tmp->valueint) tunnel->port = tmp->valueint;
		tmp = cJSON_GetObjectItem(connJson, "num_connects");
		if (tmp && tmp->valueint) tunnel->numConnects = tmp->valueint;
		cJSON_Delete(connJson);
		if (!tunnel->host || !tunnel->port) {
			twTunnel_Delete(tunnel);
			TW_LOG(TW_ERROR, "twTunnel_Create: Missing host or port  params");
			TW_FREE(connection);
			return NULL;
		}
		tunnel->localSocket = createTunnelSocketandLog(tunnel);
		if (!tunnel->localSocket) {
			twTunnel_Delete(tunnel);
			TW_LOG(TW_ERROR, "twTunnel_Create: Error creating socket for %s:%d", tunnel->host, tunnel->port);
			TW_FREE(connection);
			return NULL;
		}
		break;
	case FILE_TUNNEL:
		{
		TW_FILE_HANDLE * f = (TW_FILE_HANDLE *)TW_CALLOC(sizeof(TW_FILE_HANDLE), 1);
		char fileExists = FALSE;
		char owrite = FALSE;
		double offset = 0.0;
		/* Use host and port for filename and mode */
		char * mode = NULL;
		char * overwrite = NULL;
		tmp = cJSON_GetObjectItem(connJson, "offset");
		if (tmp && tmp->valuedouble) offset = tmp->valuedouble;
		tmp = cJSON_GetObjectItem(connJson, "mode");
		if (tmp && tmp->valuestring) mode = duplicateString(tmp->valuestring);
		tmp = cJSON_GetObjectItem(connJson, "overwrite");
		if (tmp && tmp->valuestring) overwrite = duplicateString(tmp->valuestring);
		if (!overwrite) overwrite = duplicateString("true");
		tmp = cJSON_GetObjectItem(connJson, "filename");
		if (tmp && tmp->valuestring) tunnel->host = duplicateString(tmp->valuestring);
		cJSON_Delete(connJson);
		if (!tunnel->host || !mode || !f) {
			twTunnel_Delete(tunnel);
			TW_LOG(TW_ERROR, "twTunnel_Create: Missing filename or mode  params");
			TW_FREE(connection);
			return NULL;
		}
		if (!strcmp(mode,"read")) tunnel->port = 1;
		if (!strcmp(overwrite, "true")) owrite = TRUE;
		TW_FREE(mode);
		TW_FREE(overwrite);
		/*
		See if the file exists. If this is a read and it opened we are good, if it exists
		and we are writing, we need to have overwrite enabled
		*/
		fileExists = twDirectory_FileExists(tunnel->host);
		if (!fileExists && tunnel->port == 1) {
			TW_LOG(TW_ERROR, "twTunnel_Create: File %s doesn't exist for reading", tunnel->host);
			twTunnel_Delete(tunnel);
			TW_FREE(connection);
			return NULL;
		}
		if (fileExists && tunnel->port == 0 && !owrite) {
			TW_LOG(TW_ERROR, "twTunnel_Create: File %s exists and overwrite is not enabled", tunnel->host);
			twTunnel_Delete(tunnel);
			TW_FREE(connection);
			return NULL;
		}
		/* Open the file */
		*f = TW_FOPEN(tunnel->host, tunnel->port ? "rb" : "a+b");
		if (f <= 0) {
			TW_LOG(TW_ERROR, "twTunnel_Create: Error opening file: %s for %s", tunnel->host, tunnel->port ? "reading" : "writing");
			twTunnel_Delete(tunnel);
			TW_FREE(connection);
			return NULL;
		}
		res = TW_FSEEK(*f, (uint64_t)offset, SEEK_SET);
		if (res) {
			TW_LOG(TW_ERROR, "twTunnel_Create: Error seeking in file %s to %llu.  Error: %d", tunnel->host, offset, twDirectory_GetLastError());
			twTunnel_Delete(tunnel);
			TW_FREE(connection);
			return NULL;
		}
		tunnel->localSocket = f;
		TW_LOG(TW_DEBUG, "twTunnel_Create: Opened file: %s for %s", tunnel->host, tunnel->port ? "reading" : "writing");
		}
		break;
	case UDP_TUNNEL:
	case SERIAL_TUNNEL:
	default:
		/* TODO: support udp & serial */
		twTunnel_Delete(tunnel);
		TW_LOG(TW_ERROR, "twTunnel_Create: Unsupported tunnel type: %s", tunnel->type);
		TW_FREE(connection);
		cJSON_Delete(connJson);
		return NULL;
	}
	TW_FREE(connection);

	/* Register the callback functions with the websocket */
	twWs_RegisterConnectCallback(tunnel->ws, twTunnelManager_OnWsOpenCallback);
	twWs_RegisterCloseCallback(tunnel->ws, twTunnelManager_OnWsCloseCallback);
	twWs_RegisterTextMessageCallback(tunnel->ws, twTunnelManager_OnWsDataCallback);
	/* Add this tunnel to the tunnel manager's list and start it */
	if (twTunnelManager_AddTunnel(tunnel)) {
		return NULL;
	}
	TW_LOG(TW_DEBUG,"twTunnel_Create: Succesfully created tunnel with tid: %s", tunnel->tid);

	tunnel->connectAttempts = 0;
	return tunnel;
}

void twTunnel_Delete(void * tunnel_in) {
	twTunnel * tunnel = (twTunnel *)tunnel_in;
	if (!tunnel) return;
	TW_LOG(TW_TRACE,"twTunnel_Delete: Deleting tunnel. tid: %s", tunnel->tid ? tunnel->tid : "UNKNOWN");
	if (tunnel->ws) {
		if (twWs_IsConnected(tunnel->ws)) twWs_Disconnect(tunnel->ws, NORMAL_CLOSE, "Tunnel stopped by edge");
		twWs_Delete(tunnel->ws);
		tunnel->ws = NULL;
	}
	if (tunnel->localSocket) {
		switch (tunnel->typeEnum) {
			case TCP_TUNNEL:
				twSocket_Delete((twSocket *)tunnel->localSocket);
				tunnel->localSocket = NULL;
				break;
			case FILE_TUNNEL:
				{
				TW_FILE_HANDLE f = *(TW_FILE_HANDLE *)tunnel->localSocket;
				TW_FCLOSE(f);
				TW_FREE(tunnel->localSocket);
				tunnel->localSocket = NULL;
				}
				break;
			case UDP_TUNNEL:
			case SERIAL_TUNNEL:
			default:
				/* TODO: support udp & serial */
				break;
		}
	}
	if (tunnel->thingName) TW_FREE(tunnel->thingName);
	if (tunnel->host) TW_FREE(tunnel->host);
	if (tunnel->tid) TW_FREE(tunnel->tid);
	if (tunnel->type) TW_FREE(tunnel->type);
	if (tunnel->readBuffer) TW_FREE(tunnel->readBuffer);
	if (tunnel->readBase64) TW_FREE(tunnel->readBase64);
	if (tunnel->writeBuffer) TW_FREE(tunnel->writeBuffer);
	if (tunnel->peerName) TW_FREE(tunnel->peerName);
	if (tunnel->msg) TW_FREE(tunnel->msg);
	TW_FREE(tunnel);
}

void twTunnelWs_Delete(void * nothing) {
	/* do nothing */
	return;
}

void twCloseTunnel_Delete(void * nothing) {
	/* do nothing */
	return;
}

int twTunnelManager_DeleteInstance(twTunnelManager * mgr) {
	if (!mgr && tm) mgr = tm;
	if (mgr) {
		if (mgr == tm) twTunnelManager_StopAllTunnels();
		if (mgr->mtx) twMutex_Lock(mgr->mtx);
		if (mgr->openWs) twDict_Delete(mgr->openWs);
		if (mgr->closeTunnels) twDict_Delete(mgr->closeTunnels);
		if (mgr->openTunnels) twDict_Delete(mgr->openTunnels);
		if (mgr->callbacks) twDict_Delete(mgr->callbacks);
		/* Only delete the connection info if we have created our own copy */
		if (mgr->info && mgr->info != twApi_GetConnectionInfo()) twConnectionInfo_Delete(mgr->info);
		if (mgr->mtx) twMutex_Unlock(mgr->mtx);
		if (mgr == tm) tm = NULL;
		twMutex_Delete(mgr->mtx);
		TW_FREE(mgr);
		mgr = NULL;
	}
	return TW_OK;
}

/********************************/
/*        API Functions         */
/********************************/
int twTunnelManager_Create() {
	twTunnelManager * tmp = NULL;
	if (tm) {
		TW_LOG(TW_DEBUG,"twTunnelManager_Create: Tunnel Manager singleton already exists");
		return TW_OK;
	}
	tmp = (twTunnelManager *)TW_CALLOC(sizeof(twTunnelManager), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twTunnelManager_Create: Unable to allocate memory");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	tmp->mtx = twMutex_Create();
	/* Get the current API connection settings */
	tmp->info = twApi_GetConnectionInfo();
	tmp->openTunnels = twDict_Create(twTunnel_Delete, twOpenTunnelParser);
	tmp->closeTunnels = twDict_Create(twCloseTunnel_Delete, twCloseTunnelParser);
	tmp->openWs = twDict_Create(twTunnelWs_Delete, twOpenWsParser);
	tmp->callbacks = twDict_Create(twTunnelCallback_Delete, twTunnelCallbackInfoParser);
	if (!tmp->mtx || !tmp->openTunnels || !tmp->closeTunnels || !tmp->openWs || !tmp->info || !tmp->callbacks ||
		!tmp->info->appkeyFunction || !tmp->info->ws_host || !tmp->info->ws_port || !tmp->info) {
		/* Clean up and get out */
		twTunnelManager_DeleteInstance(tmp);
		return TW_ERROR_ALLOCATING_MEMORY;
	}

#ifdef ENABLE_TASKER
	/* Initalize our tasker functon */
	twApi_CreateTask(5, &twTunnelManager_TaskerFunction);
#endif
	/* Set the singleton */
	tm = tmp;
	return TW_OK;
}

int twTunnelManager_Delete() {
	return twTunnelManager_DeleteInstance(tm);
}

/* internal function to close tunnels */
int twTunnel_CloseTunnel (twTunnel * tunnel) {
	if (tunnel) {
		char * id = duplicateString(tunnel->tid);
		TW_LOG(TW_DEBUG,"twTunnelManager_TaskerFunction: Tunnel %d ws marked for close", tunnel->tid);
		/* Send a message to the server */
		sendTunnelCommand(tunnel->thingName, id, "CANCEL");
		/* Delete this tunnel */
		twTunnelManager_StopTunnel(tunnel);
		TW_FREE(id);
		return TW_OK;
	} else {
		return TW_INVALID_PARAM;
	}
}

/* internal function to start tunnel connections */
int twTunnel_ConnectTunnel (twTunnel * tunnel){
	int res = TW_OK;

	if (tunnel) {
		/* Start the websocket */
		res = twWs_Connect(tunnel->ws, twcfg.connect_timeout);

		if (res) {
			TW_LOG(TW_ERROR,"twTunnelManager_TaskerFunction: Error opening websocket. tid: %s.  Error: %d", tunnel->tid, res);
			/* Send a message to the server */
			sendTunnelCommand(tunnel->thingName, tunnel->tid, "CANCEL");
			/* Delete this tunnel */
			if (tunnel->msg) {
				TW_FREE(tunnel->msg);
				tunnel->msg = NULL;
			}
			tunnel->msg = duplicateString("Websocket connect failed");
			tunnel->markForClose = TRUE;
		} else {
			TW_LOG(TW_AUDIT, "TUNNEL STARTED. Entity: %s, tid: %s, target: %s:%d", tunnel->thingName, tunnel->tid, tunnel->host, tunnel->port);
			tunnel->isActive = TRUE;
		}
	} else {
		res = TW_INVALID_PARAM;
	}

	return res;
}

/* internal function to handle TCP tunnels */
int twTunnel_HandleTCPTunnel (twTunnel * tunnel) {
	int res = TW_OK;
	if (tunnel && tunnel->localSocket) {
		/* Have we connected locally yet? */
		if (((twSocket *)tunnel->localSocket)->state == CLOSED) {
			if (tunnel->disconnectedByPeer) {
				if (tunnel->numConnects != 0) {
					res = connectTunnelSocketandLog(tunnel);

					if (res) {
						TW_LOG(TW_ERROR,"twTunnelManager_TaskerFunction: Error connecting to %s:%d. tid: %s.  Error: %d", tunnel->host, tunnel->port, tunnel->tid, res);
					} else {
						TW_LOG(TW_DEBUG,"twTunnelManager_TaskerFunction: Connected to %s:%d. tid: %s.", tunnel->host, tunnel->port, tunnel->tid);
					}
					tunnel->disconnectedByPeer = FALSE;
				} else {
					TW_LOG(TW_WARN,"twTunnelManager_TaskerFunction: Exceeded allowable number of connect attempts for tid: %s", tunnel->tid);
					sendTunnelCommand(tunnel->thingName, tunnel->tid, "CANCEL");
					/* Delete this tunnel */
					if (tunnel->msg) {
						TW_FREE(tunnel->msg);
						tunnel->msg = NULL;
					}
					tunnel->msg = duplicateString("Exceeded allowable number of connect attempts");
					tunnel->markForClose = TRUE;
					res = TW_OK;
				}
			} else {
				TW_LOG(TW_DEBUG,"twTunnelManager_TaskerFunction: Local socket disconnected for tid: %s", tunnel->tid);
				sendTunnelCommand(tunnel->thingName, tunnel->tid, "DISCONNECT");
				res = TW_OK;;
			}
		}
		/* Check for local data available */
		/* hardcoded twSocket_WaitFor to 1ms timeout to prevent socket blocking delays in multiple tunnel environments */
		if (((twSocket *)tunnel->localSocket)->state == OPEN && twSocket_WaitFor((twSocket *)tunnel->localSocket, 1)) {
			/* We have data on the local socket but need to account for base 64 expansion */
			/* hardcoding reads to 1ms to improve speed */
			int bytesRead = twSocket_Read((twSocket *)tunnel->localSocket, tunnel->readBuffer, tunnel->chunksize - 16, 1);
			if (bytesRead > 0){
				/* Need to base64 encode this and send as text */
				unsigned long len = (tunnel->chunksize * 4)/3;

				if (!base64_encode((const unsigned char *)tunnel->readBuffer, bytesRead, tunnel->readBase64, &len)) {
					twWs_SendMessage(tunnel->ws, tunnel->readBase64, len, TRUE);
					tunnel->bytesSent += bytesRead;
				}

				TW_LOG(TW_TRACE,"twTunnelManager_TaskerFunction: Sent %d bytes over websocket. for tid: %s", bytesRead, tunnel->tid);

				tunnel->idleTime = twGetSystemMillisecondCount() + tunnel->idleTimeout;
			}
		}
	} else {
		res = TW_INVALID_PARAM;
	}

	return res;

}

/* internal funciton to handle file tunnels */
int twTunnel_HandleFileTunnel(twTunnel * tunnel) {
	int res = TW_OK;
	TW_FILE_HANDLE f = 0;
	int bytesRead = 0;
	if (tunnel->localSocket == NULL) {
		TW_LOG(TW_ERROR,"twTunnelManager_TaskerFunction: NULL file handle pointer");
		res = TW_INVALID_PARAM;
	} else {
		if (tunnel->port == 0 || tunnel->disconnectedByPeer) {
			/* This is a write of we are being throttled by server so nothing to do here */
		} else {
			f = *(TW_FILE_HANDLE *)tunnel->localSocket;
			bytesRead = (TW_FREAD(tunnel->readBuffer, 1, tunnel->chunksize - 16, f));
			if (bytesRead > 0){
				/* Need to base64 encode this and send as text */
				unsigned long len = (tunnel->chunksize * 4)/3;
				if (!base64_encode((const unsigned char *)tunnel->readBuffer, bytesRead, (unsigned char *)tunnel->readBase64, &len)) {
					twWs_SendMessage(tunnel->ws, tunnel->readBase64, len, TRUE);
					tunnel->bytesSent += bytesRead;
				}
				TW_LOG(TW_TRACE,"twTunnelManager_TaskerFunction: Sent %d bytes over websocket. for tid: %s", bytesRead, tunnel->tid);
				tunnel->idleTime = twGetSystemMillisecondCount() + tunnel->idleTimeout;
			} else {
				TW_LOG(TW_WARN,"twTunnelManager_TaskerFunction: Error reading from file.  tid: %s.  Shutting tunnel down", tunnel->tid);
				tunnel->markForClose = TRUE;
			}
		}
	}
	return res;
}
int twTunnelManager_TaskerFunction_ForeachHandler (void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	int res = TW_OK;
	twTunnel * tunnel = (twTunnel *) data;
	char * bytesRead = (char*)(arg);

	/* Have we been marked for close? */
	if (!tunnel->markForClose) {
		if (!tunnel->isActive) {
			res = twTunnel_ConnectTunnel(tunnel);
		}
		if (TW_OK == res) {
			/* store initial bytes sent */
			int check_bytes = tunnel->bytesSent;

			switch (tunnel->typeEnum) {
			case TCP_TUNNEL:
				res = twTunnel_HandleTCPTunnel(tunnel);
				break;
			case FILE_TUNNEL:
				twTunnel_HandleFileTunnel(tunnel);
				break;
			case UDP_TUNNEL:
				/* TODO: support UDP tunnels */
			case SERIAL_TUNNEL:
				/* TODO: support Serial tunnels */
			default:
				/* Unknown or unsupported tunnel type */
				TW_LOG(TW_WARN,"twTunnelManager_TaskerFunction_ForeachHandler: unknown or unsupported tunnel type: %i",tunnel->typeEnum);
				break;
			}

			/* compare against initial sent value */
			if (check_bytes < tunnel->bytesSent && bytesRead) {
				/* this means we sent something, set bytes read to TRUE to avoid idle delays */
				*bytesRead = TRUE;
			}
			/* Check for idle tunnels */
			if (twGetSystemMillisecondCount() > tunnel->idleTime) {
				TW_LOG(TW_WARN,"twTunnelManager_TaskerFunction_ForeachHandler: Found idle tunnel.  tid: %s.  Shutting tunnel down", tunnel->tid);
				sendTunnelCommand(tunnel->thingName, tunnel->tid, "CANCEL");
			} else {
				/* store current bytes rcvd */
				int check_bytes = tunnel->bytesRcvd;

				/* Force the read from the websocket */
				/* hardcoding read timeout to 1ms to speed up tunnel operations */
				twWs_Receive(tunnel->ws, 1);

				/* compare against initial rcvd value */
				if (check_bytes < tunnel->bytesSent && bytesRead) {
					/* this means we read something, set bytes read to TRUE to avoid idle delays */
					*bytesRead = TRUE;
				}
			}
		}
	} else {
		/* add to dict */
		twDict_Add(tm->closeTunnels, tunnel);
	}
	return TW_FOREACH_CONTINUE;
}

int twTunnelManager_CleanupHandler (void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	if (data) {
		twTunnel_CloseTunnel((twTunnel*)data);
	}
	return TW_FOREACH_CONTINUE;
}
void twTunnelManager_TaskerFunction(DATETIME now, void * params) {
	/*
	Iterate through our tunnel list and force a read on the websocket
	and read from the local socket. If there is data on the local socket,
	write it out to the websocket.  Need to be careful on the websocket since
	the read may cause a callback and we already have the mutex, so we must
	avoid deadlocks.  Also want to mark idle tunnels for deletion.
	*/
	if (NULL==twApi_GetApi()) {
		TW_LOG(TW_WARN,"twTunnelManager_TaskerFunction() is being called but the api has not been initialized. Call twApi_Initialize() first.");
		return;
	}

	if (tm && tm->openTunnels) {
		/* process tunnels */
		twDict_Foreach(tm->openTunnels, twTunnelManager_TaskerFunction_ForeachHandler, NULL);

		/* clean anything marked for close */
		twDict_Foreach(tm->closeTunnels, twTunnelManager_CleanupHandler, NULL);

		/* clear out the closeTunnel list */
		twDict_Clear(tm->closeTunnels);
	}
	return;
}


void twTunnelManager_ThreadFunction(DATETIME now, void * params) {
	/*
	Iterate through our tunnel list and force a read on the websocket
	and read from the local socket. If there is data on the local socket,
	write it out to the websocket.  Need to be careful on the websocket since
	the read may cause a callback and we already have the mutex, so we must
	avoid deadlocks.  Also want to mark idle tunnels for deletion.
	*/

	/*
	The thread version of the tunnel manager task should only return if the tunnel manager
	is deleted. This will avoid additional overhead of tearing down and spinning up threads
	*/

	while(tm && tm->openTunnels) {
		if(tm->openTunnels) {
			/* Create a flag to determine whether or not we should sleep to relieve
			CPU utilization or continue reading to maintain performant tunnels */
			char bytesRead = FALSE;

			/* process tunnels */
			twDict_Foreach(tm->openTunnels, twTunnelManager_TaskerFunction_ForeachHandler, &bytesRead);

			/* clean anything marked for close */
			twDict_Foreach(tm->closeTunnels, twTunnelManager_CleanupHandler, NULL);

			/* clear out the closeTunnel list */
			twDict_Clear(tm->closeTunnels);

			/* if nothing was read during the twTunnelManager_TaskerFunction_ForeachHandler,
			delay the next round of reads to releive CPU utilization*/
			if(!bytesRead) {
				/* in order to prevent 100% CPU Single core utilization,
				sleep for 1ms if we have not received any data on any tunnel */
				twSleepMsec(10);
			} else {
				/* if bytes were read, reset bytesRead flag for next loop iteration */
				bytesRead = FALSE;
			}
		} else {
			/* if there are no open tunnels delay the next round of reads to releive CPU utilization */
			twSleepMsec(50);
		}
	}

	/* only return from this loop when the tunnel manager does not exist to prevent
	unnecessary resource cost when spinning up and tearing down this thread*/
	return;
}

/* internal foreach handler to find and execute tunnel callbacks based on the tunnel->tid */
int twTunnelManager_AddTunnel_ForeachHandler (void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	/* check pointers */
	if (data && arg) {
		/* Make any callbacks */
		twTunnel * tunnel = (twTunnel*) arg;
		twTunnelCallback * cb = (twTunnelCallback *)data;
		if (cb->id && (!strcmp(tunnel->tid, cb->id) || !strcmp(cb->id,"*"))) {
			cb->cb(TRUE, tunnel->tid, tunnel->thingName, tunnel->peerName, tunnel->host, tunnel->port,
				tunnel->startTime, tunnel->endTime, tunnel->bytesSent, tunnel->bytesRcvd, tunnel->type, tunnel->msg, cb->userdata);
		}
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_AddTunnel_ForeachHandler: null params were passed in to the handler");
	}

	/* continue */
	return TW_FOREACH_CONTINUE;
}

int twTunnelManager_AddTunnel(twTunnel * tunnel) {
	/* check arg */
	if (!tunnel || !tunnel->ws || !tunnel->localSocket || !tm) {
		TW_LOG(TW_ERROR,"twTunnelManager_StartTunnel: Invalid parameter");
		return TW_INVALID_PARAM;
	}

	/* Set the start time to now */
	tunnel->startTime = twGetSystemTime(TRUE);
	tunnel->idleTime = twAddMilliseconds(tunnel->startTime, tunnel->idleTimeout);

	/* Add this to our list */
	twDict_Add(tm->openTunnels, tunnel);
	twDict_Add(tm->openWs, tunnel);

	/* iterate the callback list, execute any matches */
	twDict_Foreach(tm->callbacks, twTunnelManager_AddTunnel_ForeachHandler, tunnel);
	return TW_OK;
}

/* internal struct to pass stop tunnel args */
typedef struct twTunnelManager_StopTunnel_ForeachHandler_Params {
	twTunnel * tunnel;
	char * id;
} twTunnelManager_StopTunnel_ForeachHandler_Params;

/* internal foreach handler to stop tunnels */
int twTunnelManager_StopTunnel_ForeachHandler (void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	/* check pointers */
	if (data && arg) {
		twTunnelManager_StopTunnel_ForeachHandler_Params * tunnel_params = (twTunnelManager_StopTunnel_ForeachHandler_Params*)arg;
		twTunnelCallback * cb = (twTunnelCallback *)data;
		char * id = tunnel_params->id;
		twTunnel * tunnel = tunnel_params->tunnel;

		if (cb->id && (!strcmp(id, cb->id) || !strcmp(cb->id,"*"))) {
			cb->cb(FALSE, tunnel->tid, tunnel->thingName, tunnel->peerName, tunnel->host, tunnel->port,
				tunnel->startTime, tunnel->endTime, tunnel->bytesSent, tunnel->bytesRcvd, tunnel->type, tunnel->msg, cb->userdata);
		}
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_StopTunnel_ForeachHandler: null params were passed in to the handler");
	}

	return TW_FOREACH_CONTINUE;
}


int twTunnelManager_StopTunnel(twTunnel * tunnel) {
	twTunnelManager_StopTunnel_ForeachHandler_Params params;
	twTunnel * tunnel_query = NULL;
	twTunnel * tunnel_entry = NULL;

	if (tunnel ) {
		/* set params */
		tunnel_query = tunnel;
		params.id = tunnel->tid;
		params.tunnel = tunnel;

		/* Set the stop time */
		tunnel->endTime = twGetSystemTime(TRUE);

		/* Make any callbacks that were registered */
		twDict_Foreach(tm->callbacks, twTunnelManager_StopTunnel_ForeachHandler, (void*)&params);

		/* Close the websocket - need to do this now to prevent a deadlock in the onClose callback */
		if (tunnel->ws) twWs_Disconnect(tunnel->ws, NORMAL_CLOSE, "Tunnel stopped by edge");

		/* close the local socket */
		if (tunnel->localSocket && ((twSocket*)tunnel->localSocket)->sock) {
			closeTunnelSocketandLog(tunnel);
		}

		/* find the actual tunnelWs entry so we can free that memeory, not the stack variable in this function */
		twDict_Find(tm->openWs, (void*)tunnel_query, (void**)&tunnel_entry);

		/* Delete the tunnel websocket entry (which closes the websocket as well) */
		if(tunnel_entry && tunnel_entry->ws) {
			twDict_Remove(tm->openWs, tunnel_entry, FALSE);
		}

		/* delete the tunnel */
		twDict_Remove(tm->openTunnels, tunnel, TRUE);
		return TW_OK;
	} else {
		return TW_INVALID_PARAM;
	}
}

int twTunnelManager_StopAllTunnels() {
    TW_LOG(TW_DEBUG,"twTunnelManager_StopAllTunnels: Attempting to stop all tunnels");
	if (!tm) {
		TW_LOG(TW_ERROR,"twTunnelManager_StopAllTunnels: Tunnel Manager not initialized");
		return TW_TUNNEL_MANAGER_NOT_INITIALIZED;
	}
	twMutex_Lock(tm->mtx);
	/* Clear the tunnel list (which  deletes all tunnels as well) */
	twDict_Clear(tm->openWs);
	twDict_Clear(tm->openTunnels);
	twMutex_Unlock(tm->mtx);
	return TW_OK;
}

int twTunnelManager_UpdateTunnelServerInfo(char * host, uint16_t port, char * appkey) {
	if (!appkey || !host || !port) {
		return TW_INVALID_PARAM;
	}
	if (!tm || !tm->info) return TW_TUNNEL_MANAGER_NOT_INITIALIZED;

	if (tm->info->appkeyFunction) {
		tm->info->appkeyFunction = NULL;
	}
	if (tm->info->ws_host) {
		TW_FREE(tm->info->ws_host);
		tm->info->ws_host = NULL;
	}
	tm->info->appkeyFunction = appkey;
	tm->info->ws_host = duplicateString(host);
	tm->info->ws_port = port;

	if (!tm->info->appkeyFunction || !tm->info->ws_host || !tm->info->ws_port) {
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	return TW_OK;
}

int twTunnelManager_RegisterTunnelCallback(tunnel_cb cb, char * id, void * userdata) {
	twTunnelCallback * tmp = NULL;
	if (!tm) {
		TW_LOG(TW_ERROR,"twTunnelManager_RegisterTunnelCallback: Tunnel Manager not initialized");
		return TW_TUNNEL_MANAGER_NOT_INITIALIZED;
	}
	if (!cb) {
		TW_LOG(TW_ERROR,"twTunnelManager_RegisterTunnelCallback: NULL parameters found");
		return TW_INVALID_PARAM;
	}
	tmp = twTunnelCallback_Create(cb, id ? id : "*", userdata);
	if (!tmp) {
		TW_LOG(TW_ERROR,"twTunnelManager_RegisterTunnelCallback: Error allocating memory");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twMutex_Lock(tm->mtx);
	twDict_Add(tm->callbacks, tmp);
	twMutex_Unlock(tm->mtx);
	return TW_OK;
}

/* if (tcb->cb == cb && tcb->userdata == userdata && tcb->id && strcmp(tcb->id, id)==0) { */

int twTunnelManager_UnregisterTunnelCallback(tunnel_cb cb, char * id, void * userdata) {
	twTunnelCallback * result = NULL;
	twTunnelCallback tcb_query;
	if (!cb || !id || !tm || !tm->mtx || !tm->callbacks) {
		TW_LOG(TW_ERROR,"twTunnelManager_UnregisterTunnelCallback: Invalid params or state");
		return TW_INVALID_PARAM;
	}

	tcb_query.cb = cb;
	tcb_query.id = id;
	tcb_query.userdata = userdata;

	while (TW_OK == twDict_Find(tm->callbacks,&tcb_query,(void**)&result)) {
		twDict_Remove(tm->callbacks, result, TRUE);
	}
	return TW_OK;
}


char * tunnelServices[] = {
	"StartTunnel", "CompleteTunnel", "TunnelCommandToEdge",	"SENTINEL"
};

enum msgCodeEnum tunnelServiceCallback(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content) {
	if (!entityName || !serviceName || !params) {
		TW_LOG(TW_ERROR, "tunnelServiceCallback: missing entityName, serviceName, or input params");
		return TWX_BAD_REQUEST;
	}
	if (!content) {
		TW_LOG(TW_ERROR, "tunnelServiceCallback: missing content param");
		return TWX_INTERNAL_SERVER_ERROR;
	}
	if (!strcmp(serviceName, "StartTunnel")) {
		return StartTunnel(entityName, params, content);
	} else 	if (!strcmp(serviceName, "CompleteTunnel")) {
		return CompleteTunnel(entityName, params, content);
	} else 	if (!strcmp(serviceName, "TunnelCommandToEdge")) {
		return TunnelCommandToEdge(entityName, params, content);
	} else {
		TW_LOG(TW_ERROR, "tunnelServiceCallback: Bad serviceName: %s", serviceName);
		return TWX_BAD_REQUEST;
	}
}

twActiveTunnel * twActiveTunnel_Create(twTunnel * tunnel) {
	twActiveTunnel * a = NULL;
	if (!tunnel) return NULL;
	a = (twActiveTunnel *)TW_CALLOC(sizeof (twActiveTunnel), 1);
	if (!a) return NULL;
	if (tunnel->thingName) a->thingName = duplicateString(tunnel->thingName);
	if (tunnel->tid) a->tid = duplicateString(tunnel->tid);
	if (tunnel->peerName) a->peerName = duplicateString(tunnel->peerName);
	if (tunnel->host) a->host = duplicateString(tunnel->host);
	if (tunnel->type) a->type = duplicateString(tunnel->type);
	a->port = tunnel->port;
	a->startTime = tunnel->startTime;
	a->endTime = tunnel->endTime;
	return a;
}

void twActiveTunnel_Delete(void * a) {
	twActiveTunnel * tunnel = (twActiveTunnel *)a;
	if (!tunnel) return;
	if (tunnel->thingName) TW_FREE(tunnel->thingName);
	if (tunnel->tid) TW_FREE(tunnel->tid);
	if (tunnel->peerName) TW_FREE(tunnel->peerName);
	if (tunnel->host) TW_FREE(tunnel->host);
	if (tunnel->type) TW_FREE(tunnel->type);
	TW_FREE(tunnel);
}

/* internal for each handler to populate active tunnel list */
int twTunnelManager_ListActiveTunnels_ForeachHandler (void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	/* check value */
	if (data && arg) {
		/* add entry to input list */
		twList_Add((twList*)arg, twActiveTunnel_Create((twTunnel *)data));
	}

	/* continue */
	return TW_FOREACH_CONTINUE;
}

twActiveTunnelList * twTunnelManager_ListActiveTunnels() {
	twActiveTunnelList * a = NULL;

	/* check tm singleton */
	if (!tm || !tm->openTunnels) return NULL;

	/* create list to populate with active tunnels */
	a = twList_Create(twActiveTunnel_Delete);
	if (!a) return NULL;

	/* iterate the open tunnels list, use the handler to populate the return list */
	twDict_Foreach(tm->openTunnels, twTunnelManager_ListActiveTunnels_ForeachHandler, a);

	/* return list */
	return a;
}

int twTunnelManager_CheckConnectionInfo() {
	if (!tm) {
		TW_LOG(TW_ERROR,"twTunnelManager_CheckConnectionInfo: TunnelManager is not initialized");
		return TW_TUNNEL_MANAGER_NOT_INITIALIZED;
	}
	/* If we are changing the settings we don't want to change the API settings */
	if (tm->info == twApi_GetConnectionInfo()) tm->info = twConnectionInfo_Create(twApi_GetConnectionInfo());
	if (!tm->info) {
		TW_LOG(TW_ERROR,"twTunnelManager_CheckConnectionInfo: Error creating tunnel manager connection info struct");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	return 0;
}

void twTunnelManager_SetProxyInfo(char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction proxyPassCallback) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	if (tm->info->proxy_host) {
		TW_FREE(tm->info->proxy_host);
		tm->info->proxy_host = NULL;
	}
	tm->info->proxy_host = duplicateString(proxyHost);
	tm->info->proxy_port = proxyPort;
	if (tm->info->proxy_user) {
		TW_FREE(tm->info->proxy_user);
		tm->info->proxy_user = NULL;
	}
	tm->info->proxy_user = duplicateString(proxyUser);
	tm->info->proxy_pwd = proxyPassCallback;
}

void twTunnelManager_SetSelfSignedOk(char state) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	tm->info->selfsignedOk = state;
}

void twTunnelManager_DisableCertValidation(char state) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	tm->info->doNotValidateCert = state;
}

void twTunnelManager_DisableEncryption(char state) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	tm->info->disableEncryption = state;
}

void twTunnelManager_SetX509Fields(char * subject_cn, char * subject_o, char * subject_ou,
							  char * issuer_cn, char * issuer_o, char * issuer_ou) {
	if (twTunnelManager_CheckConnectionInfo()) return;

	if (!subject_cn) {
		if (tm->info->subject_cn) {
			TW_FREE(tm->info->subject_cn);
			tm->info->subject_cn = NULL;
		}
		tm->info->subject_cn = duplicateString(subject_cn);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in subject_cn");
	}

	if (!subject_o) {
		if (tm->info->subject_o) {
			TW_FREE(tm->info->subject_o);
			tm->info->subject_o = NULL;
		}
		tm->info->subject_o = duplicateString(subject_o);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in subject_o");
	}

	if (!subject_ou) {
		if (tm->info->subject_ou) {
			TW_FREE(tm->info->subject_ou);
			tm->info->subject_ou = NULL;
		}
		tm->info->subject_ou = duplicateString(subject_ou);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in subject_ou");
	}

	if (!issuer_cn) {
		if (tm->info->issuer_cn) {
			TW_FREE(tm->info->issuer_cn);
			tm->info->issuer_cn = NULL;
		}
		tm->info->issuer_cn = duplicateString(issuer_cn);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in issuer_cn");
	}

	if (!issuer_o) {
		if (tm->info->issuer_o) {
			TW_FREE(tm->info->issuer_o);
			tm->info->issuer_o = NULL;
		}
		tm->info->issuer_o = duplicateString(issuer_o);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in issuer_o");
	}

	if (!issuer_ou) {
		if (tm->info->issuer_ou) {
			TW_FREE(tm->info->issuer_ou);
			tm->info->issuer_ou = NULL;
		}
		tm->info->issuer_ou = duplicateString(issuer_ou);
	} else {
		TW_LOG(TW_WARN,"twTunnelManager_SetX509Fields: NULL argument passed in issuer_ou");
	}
}

void twTunnelManager_LoadCACert(const char *file, int type) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	if (tm->info->ca_cert_file) {
		TW_FREE(tm->info->ca_cert_file);
		tm->info->ca_cert_file = NULL;
	}
	tm->info->ca_cert_file = duplicateString(file);
}

void twTunnelManager_LoadClientCert(char *file) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	if (tm->info->client_cert_file) {
		TW_FREE(tm->info->client_cert_file);
		tm->info->client_cert_file = NULL;
	}
	tm->info->client_cert_file = duplicateString(file);
}

void twTunnelManager_SetClientKey(const char *file, twPasswdCallbackFunction passphraseCallback, int type) {
	if (twTunnelManager_CheckConnectionInfo()) return;
	if (tm->info->client_key_file) {
		TW_FREE(tm->info->client_key_file);
		tm->info->client_key_file = NULL;
	}
	tm->info->client_key_file = duplicateString(file);
	if (tm->info->client_key_passphrase) {
		tm->info->client_key_passphrase = NULL;
	}
	tm->info->client_key_passphrase = passphraseCallback;
}
