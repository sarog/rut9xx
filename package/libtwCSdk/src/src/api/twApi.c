/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx C SDK API layer
 */

#include "twApi.h"
#include "twOSPort.h"
#include "twLogger.h"
#include "twMessages.h"
#include "twMessaging.h"
#include "stringUtils.h"
#include "twTls.h"
#include "twProperties.h"
#include "twServices.h"
#include "twVersion.h"
#include "tomcrypt_misc.h"
#include "twPasswds.h"

#ifdef ENABLE_FILE_XFER
#include "twFileManager.h"
#endif
#ifdef ENABLE_TUNNELING
#include "twTunnelManager.h"
#endif
#include "twSubscribedProps.h"
#include "twApiStubs.h"
#include "crypto_wrapper.h"
#ifdef TW_STUBS
/*
* cfuhash.h is included here only to add cfuhash names
* to the stubs table. Production usage of libcfu is
* encapsulated through the twDict interface.
*/
#include "cfuhash.h"
#endif

#define TW_NO_DELETION_FUNCTION NULL
#define TW_DONT_DELETE_LIST_VALUE twGenericDoNothingDeletionHandler
#define MAX_ENTITY_NAME_LEN 256

/* Singleton API structure */
/* stubs structure */
#ifdef WIN32
__declspec(dllexport) twApi * tw_api = NULL;
__declspec(dllexport) twApi_Stubs * twApi_stub = NULL;
#else
twApi * tw_api = NULL;
twApi_Stubs* twApi_stub = NULL;
#endif

twList *twList_CreateSearchable(del_func delete_function, parse_func parse_function);
int makeSynchronizedStateCallbacks(char * entityName, enum entityTypeEnum entityType,twInfoTable* subscriptionInfo);
int32_t NTLM_sendType1Msg(twSocket * sock, const char * req, char * domain, char * user, char * password);
int32_t NTLM_parseType2Msg(twSocket * sock, const char * req, char * resp, char * domain, char * username, char * password);
int NTLM_connectToProxy(twSocket * sock, const char * req, const char * resp, char * user, char * password);
int32_t GenerateType1Msg(char **buffer, uint32_t *length);
int32_t GenerateType3Msg(const char * domain, const char * username, const char * password,
                 const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength);
int listDirsInInfoTable(char * entityName, char * virtualPath, twInfoTable * it);

/* Global mutex used during initialization*/
TW_MUTEX twInitMutex;

/****************************************/
/**        Callback data struct        **/
/****************************************/
void deleteCallbackInfo(void * info) {
	if (info) {
		callbackInfo * tmp = (callbackInfo *)info;
		if (tmp->characteristicDefinition) {
			if (tmp->characteristicType == TW_PROPERTIES) twPropertyDef_Delete(tmp->characteristicDefinition);
			else if (tmp->characteristicType == TW_SERVICES) twServiceDef_Delete(tmp->characteristicDefinition);
			else if (tmp->characteristicType == TW_EVENTS) twServiceDef_Delete(tmp->characteristicDefinition);
		}
		TW_FREE(tmp->entityName);
		TW_FREE(tmp->characteristicName);
		TW_FREE(tmp);
	}
}

const char* bindListEntry_Parser (void * data) {
	bindListEntry* blEntry = (bindListEntry*)data;
	const char* name = (const char *)duplicateString (blEntry->name);

	/* Make sure all key fields are provided */
	if (!name)
		return NULL;

	return name;
}

bindListEntry * bindListEntry_Create(char * entityName) {
	bindListEntry * e = NULL;
	if (!entityName) return NULL;
	e = (bindListEntry *)TW_CALLOC(sizeof(bindListEntry), 1);
	if (!e) return NULL;
	e->name = duplicateString(entityName);
	e->needsPropertyUpdate = FALSE;
	return e;
}

void bindListEntry_Delete(void * entry) {
	bindListEntry * e = (bindListEntry *)entry;
	if (!e) return;
	if (e->name) TW_FREE(e->name);
	TW_FREE (e);
}

/* Notify Property Update Handler */
enum msgCodeEnum notifyPropertyUpdateHandler(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	void * result;
	bindListEntry* blQuery;
	if (!entityName) return TWX_BAD_REQUEST;
	twMutex_Lock(tw_api->mtx);

	blQuery = (bindListEntry*)TW_MALLOC(sizeof(bindListEntry));
	blQuery->name = (char*)entityName;

	if(TW_OK == twDict_Find(tw_api->boundList,blQuery,&result)){
		bindListEntry* blEntry = (bindListEntry* )result;
		blEntry->needsPropertyUpdate = TRUE;
	}

	TW_FREE(blQuery);

	twMutex_Unlock(tw_api->mtx);
	return TWX_SUCCESS;
}

int subscribedPropertyUpdateTaskForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
	bindListEntry *blEntry = (bindListEntry *) data;
	if (blEntry->needsPropertyUpdate) {
		/* Make the request to the server */
		twInfoTable *it = NULL;
		int res = TW_OK;
		{
			/* Params to GetPropertySubscriptions are a JSON collection of extra "options" indicating edge SDK
			 * capabilities to the platform. In our case, we inform the platform that we support the DEADBAND
			 * push type on properties.
			 */
			cJSON* jsonOptions = cJSON_CreateObject ();
			twInfoTable* itParams = NULL;
			cJSON_AddBoolToObject (jsonOptions, "hasDeadband", TRUE);
			itParams = twInfoTable_CreateFromJson (jsonOptions, "options");
			cJSON_Delete (jsonOptions);
			res = s_twApi_InvokeService (TW_THING, blEntry->name, "GetPropertySubscriptions", itParams, &it, -1, FALSE);
			twInfoTable_Delete (itParams);
		}
		if (res) {
			TW_LOG(TW_ERROR, "subscribedPropertyUpdateTask - Error getting subscribed properties");
		} else {
			/* Update the metadata */
			int count = 0;
			if (!it) {
				TW_LOG(TW_ERROR, "subscribedPropertyUpdateTask - Error updating metadata for thing %s. Skipping.",
					   blEntry->name);
				return TW_FOREACH_CONTINUE;
			}
			count = twList_GetCount(it->rows);
			while (count > 0) {
				char *propName = NULL;
				char *pushType = NULL;
				double pushThreshold = 0;
				twInfoTable_GetString(it, "edgeName", count - 1, &propName);
				twInfoTable_GetString(it, "pushType", count - 1, &pushType);
				twInfoTable_GetNumber(it, "pushThreshold", count - 1, &pushThreshold);

				/* TW_UNKNOWN_TYPE - keeps the current value, NULL description keeps the current value */
				if (s_twApi_UpdatePropertyMetaData(TW_THING, blEntry->name, propName, TW_UNKNOWN_TYPE, NULL, pushType,
												 pushThreshold)) {
					TW_LOG(TW_ERROR, "subscribedPropertyUpdateTask - Error updating metadata for property %s",
						   propName ? propName : "UNKNOWN");
				} else {
					TW_LOG(TW_TRACE, "subscribedPropertyUpdateTask - Updated metadata for property %s",
						   propName);
				}
				TW_FREE(propName);
				propName = NULL;
				TW_FREE(pushType);
				pushType = NULL;
				count--;
			}

            s_makeSynchronizedStateCallbacks(blEntry->name,TW_THING, it);
            if (it) {
                twInfoTable_Delete(it);
            }
            s_twApi_PushSubscribedProperties(blEntry->name, FALSE);
		}

        blEntry->needsPropertyUpdate = FALSE;

	}
	return TW_FOREACH_CONTINUE;

}

/* Add entry to provided list only if it requires an update */
int findEntriesThatNeedUpdatesForEachHandler(void *key, size_t key_size, void *data, size_t data_size, void *arg) {
	bindListEntry *blEntry = (bindListEntry *)data;
	twDict* needsUpdates = (twDict*)arg;
	bindListEntry* blNewEntry = NULL;

	if (blEntry->needsPropertyUpdate) {
		blEntry->needsPropertyUpdate = FALSE;
		blNewEntry = bindListEntry_Create (blEntry->name);
		if (blNewEntry)	{
			blNewEntry->needsPropertyUpdate = TRUE;
			twDict_Add (needsUpdates, blNewEntry);
		} else {
			TW_LOG(TW_ERROR, "Entity '%s' requires a property update, but could not allocate a binding list entry", blEntry->name);
		}
	}
	return TW_FOREACH_CONTINUE;
}

void subscribedPropertyUpdateTask (DATETIME now, void * params)	{

	twDict* needsUpdates = twDict_Create (bindListEntry_Delete, bindListEntry_Parser);

	if(NULL == tw_api)
		return;

	/* Create a list of all entries that need property updates */
	twDict_Foreach(tw_api->boundList, findEntriesThatNeedUpdatesForEachHandler, needsUpdates);

	/* Service the entries that actually need to be updated, without locking tw_api->boundList */
	twDict_Foreach(needsUpdates,subscribedPropertyUpdateTaskForEachHandler,NULL);

	if (tw_api && FALSE == tw_api->firstSynchronizationComplete)	{
		TW_LOG (TW_TRACE,
				"All property subscriptions have been received from the server. Subscribed properties can now be updated.");
		tw_api->firstSynchronizationComplete = TRUE;
	}

	twDict_Delete (needsUpdates);
}


/****************************************/
/**          Helper functions          **/
/****************************************/
#ifdef ENABLE_FILE_XFER
/* File Callback data structure */
extern void * fileXferCallback;
#endif

char isFileTransferService(char * service) {
	int i = 0;
	if (!service) return FALSE;
	#ifdef ENABLE_FILE_XFER
	while (strcmp(fileXferServices[i],"SENTINEL")) {
		if (!strcmp(fileXferServices[i++],service)) {
			/* This is a file transfer service */
			return TRUE;
		}
	}
	#endif
	return FALSE;
}

char isTunnelService(char * service) {
    int i = 0;
	if (!service) return FALSE;
	#ifdef ENABLE_TUNNELING
	while (strcmp(tunnelServices[i],"SENTINEL")) {
		if (!strcmp(tunnelServices[i++],service)) {
			/* This is a tunneling service */
			return TRUE;
		}
	}
	#endif
	return FALSE;
}

int convertMsgCodeToErrorCode(enum msgCodeEnum code) {
	int err;
	switch (code) {
		case TWX_SUCCESS:
			err = TW_OK;
			break;
		case TWX_BAD_REQUEST:
			err = TW_BAD_REQUEST;
			break;
		case TWX_UNAUTHORIZED:
			err = TW_UNAUTHORIZED;
			break;
		case TWX_BAD_OPTION:
			err = TW_ERROR_BAD_OPTION;
			break;
		case TWX_FORBIDDEN:
			err = TW_FORBIDDEN;
			break;
		case TWX_NOT_FOUND:
			err = TW_NOT_FOUND;
			break;
		case TWX_METHOD_NOT_ALLOWED:
			err = TW_METHOD_NOT_ALLOWED;
			break;
		case TWX_NOT_ACCEPTABLE:
			err = TW_NOT_ACCEPTABLE;
			break;
		case TWX_PRECONDITION_FAILED:
			err = TW_PRECONDITION_FAILED;
			break;
		case TWX_ENTITY_TOO_LARGE:
			err = TW_ENTITY_TOO_LARGE;
			break;
		case TWX_UNSUPPORTED_CONTENT_FORMAT:
			err = TW_UNSUPPORTED_CONTENT_FORMAT;
			break;
		case TWX_INTERNAL_SERVER_ERROR:
			err = TW_INTERNAL_SERVER_ERROR;
			break;
		case TWX_NOT_IMPLEMENTED:
			err = TW_NOT_IMPLEMENTED;
			break;
		case TWX_BAD_GATEWAY:
			err = TW_BAD_GATEWAY;
			break;
		case TWX_SERVICE_UNAVAILABLE:
			err = TW_SERVICE_UNAVAILABLE;
			break;
		case TWX_GATEWAY_TIMEOUT:
			err = TW_GATEWAY_TIMEOUT;
			break;
		case TWX_WROTE_TO_OFFLINE_MSG_STORE:
			err = TW_WROTE_TO_OFFLINE_MSG_STORE;
			break;
		case TWX_OFFLINE_MSG_STORE_FULL:
			err = TW_ERROR_OFFLINE_MSG_STORE_FULL;
			break;
		default:
			err = TW_UNKNOWN_ERROR;
			break;
	}
	return err;
}

ListEntry * findRegisteredItem(twList * list, enum entityTypeEnum entityType, char * entityName,
						enum characteristicEnum characteristicType, char * characteristicName) {
	/* Get based on entity/characteristic pair */
	ListEntry * le = NULL;
	char * target = NULL;
	if (!list || !entityName || !characteristicName) {
		TW_LOG(TW_ERROR, "findRegisteredItem: NULL input parameter found");
		return 0;
	}
	le = twList_Next(list, NULL);
	while (le && le->value) {
		callbackInfo * tmp = (callbackInfo *)(le->value);
		target = tmp->characteristicName;
		if (tmp->entityType != entityType || tmp->characteristicType != characteristicType ||
			strcmp(entityName, tmp->entityName) || strcmp(characteristicName, target)) {
			le = twList_Next(list, le);
			continue;
		} else {
			return le;
		}
	}
	return NULL;
}

void * findCallback(enum entityTypeEnum entityType, char *entityName,
					enum characteristicEnum characteristicType, char *characteristicName, void **userdata) {
	void *results;
	callbackInfo *cbInfoQuery;
	if (!tw_api->callbackList || !entityName || !characteristicName) {
		TW_LOG(TW_ERROR, "findCallback: NULL input parameter found");
		return 0;
	}

	/* Build a query structure */
	cbInfoQuery = (callbackInfo *) TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->entityType = entityType;
	cbInfoQuery->characteristicType = characteristicType;

	/* Search for priority callbacks for Tunneling (*T, *B) */
	if (isTunnelService(characteristicName)) {
		cbInfoQuery->characteristicName = "*T";
		if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
			callbackInfo *cbInfo = (callbackInfo *) results;
			*userdata = cbInfo->userdata;
			TW_FREE(cbInfoQuery);
			return cbInfo->cb;
		}

		cbInfoQuery->characteristicName = "*B";
		if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
			callbackInfo *cbInfo = (callbackInfo *) results;
			*userdata = cbInfo->userdata;
			TW_FREE(cbInfoQuery);
			return cbInfo->cb;
		}
	}

	/* Search for priority callbacks for File Transfer (*F, *B) */
	if (isFileTransferService(characteristicName)) {
		cbInfoQuery->characteristicName = "*F";
		if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
			callbackInfo *cbInfo = (callbackInfo *) results;
			*userdata = cbInfo->userdata;
			TW_FREE(cbInfoQuery);
			return cbInfo->cb;
		}

		cbInfoQuery->characteristicName = "*B";
		if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
			callbackInfo *cbInfo = (callbackInfo *) results;
			*userdata = cbInfo->userdata;
			TW_FREE(cbInfoQuery);
			return cbInfo->cb;
		}
	}

	/* Search for an exact match */
	cbInfoQuery->characteristicName = characteristicName;
	if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
		callbackInfo *cbInfo = (callbackInfo *) results;
		*userdata = cbInfo->userdata;
		TW_FREE(cbInfoQuery);
		return cbInfo->cb;
	}

	/* Search for a generic callback (*) */
	cbInfoQuery->characteristicName = "*";
	if (TW_OK == twDict_Find(tw_api->callbackList, cbInfoQuery, &results)) {
		callbackInfo *cbInfo = (callbackInfo *) results;
		*userdata = cbInfo->userdata;
		TW_FREE(cbInfoQuery);
		return cbInfo->cb;
	}

	TW_FREE(cbInfoQuery);
	/* Fallback to default handlers if possible */
	/* If file transfer is enabled let that system handle any unhandled file transfer services */
#ifdef ENABLE_FILE_XFER
	if (isFileTransferService(characteristicName)) return fileXferCallback;
#endif
	/* If tunneling is enabled let that system handle any unhandled tunnel services */
#ifdef ENABLE_TUNNELING
	if (isTunnelService(characteristicName)) return tunnelServiceCallback;
#endif

	*userdata = NULL;
	return NULL;
}


extern twOfflineMsgStore * tw_offline_msg_store;
int enableOfflineMsgStore(char enable, char onDisk) {
	int err = TW_OK;
	err = twOfflineMsgStore_Initialize(enable, twcfg.offline_msg_store_dir, twcfg.offline_msg_queue_size, onDisk);
	return err;
}

enum msgCodeEnum sendMessageBlocking(twMessage ** msg, int32_t timeout, twInfoTable ** result) {
	DATETIME expirationTime, now;
	unsigned char onlyResponses;
	uint32_t current_request_id = 0;
	int ret = TW_OK;

	if (!tw_api || !tw_api->mh || !msg) {
		TW_LOG(TW_ERROR, "api:sendMessageBlocking: NULL api, msg or message handler pointer found");
		return TWX_UNKNOWN;
	}
	expirationTime = twGetSystemMillisecondCount();
	expirationTime = twAddMilliseconds(expirationTime, timeout);
	/* saving the current request ID, because if the message fails,
	* the twMessage struct will be free'd during the write to the
	* offline message store */
	current_request_id = (*msg)->requestId;
	/* Register the response before we send to prevent a race condition */
	twMessageHandler_RegisterResponseCallback(tw_api->mh, 0, current_request_id, expirationTime);
	/* Send the message and wait for it to be received */
	ret = s_twMessage_Send(msg, tw_api->mh->ws);
	if (ret == TW_OK) {
		twResponseCallbackStruct * cb = 0;
		now = twGetSystemMillisecondCount();
		while (twTimeLessThan(now, expirationTime)) {
			if (s_twWs_Receive(tw_api->mh->ws, twcfg.socket_read_timeout)) {
				TW_LOG(TW_WARN,"api:sendMessageBlocking: Receive failed.");
				break;
			}
			onlyResponses = TRUE;
			twMessageHandler_msgHandlerTask(now, &onlyResponses);
			cb = s_twMessageHandler_GetCompletedResponseStruct(tw_api->mh, current_request_id);
			if (cb) break;

			/* the following delay was introduced to solve an issue related to thread contention:
			if sendMessageBlocking is called in a multithreaded environment, it is possible that
			this while loop will never fully release the tlsClient mutex, which will prevent other
			threads from sending AND receiving since AxTls uses the same tlsClient for sending
			and receiving. The thread contention comes into play because this thread (sendMessageBlocking)
			is unlocking the tlsClient mutex within this while loop, but the thread scheduler still sees
			this while loop as having a higher priority (as defined by liunx) than another thread which
			is blocked on the tlsClient mutex lock.

			At a much lower level the cause of the issue here is a feature/flaw within pthreads, specifically
			pthread_mutex's have no enforced queueing for the lock order. The locks rely entirely on the
			thread scheduler, which relies on the priority of threads*/
			twSleepMsec(1);
			now = twGetSystemMillisecondCount();
		}

		if (!cb) {
			TW_LOG(TW_WARN,"api:sendMessageBlocking: Message %d timed out", current_request_id);
			/* writing to the offline message store will result in a twMessage_ZeroCopy, which will
			* set the msg pointer to NULL and retain ownership of the associated memory within the
			* offline message store singleton. The message will be free'd when the message is stored */
			if(tw_offline_msg_store && tw_offline_msg_store->offlineMsgEnabled) {
				ret = twOfflineMsgStore_HandleRequest(msg, tw_api->mh->ws, OFFLINE_MSG_STORE_WRITE);
				if(ret) {
					TW_LOG(TW_ERROR, "sendMessageBlocking: failed to save message to offline message store");
				}
			}
			if(tw_api)
				twMessageHandler_CleanupOldMessages(tw_api->mh);
			return TWX_GATEWAY_TIMEOUT;
		} else {
			enum msgCodeEnum code = cb->code;
			TW_LOG(TW_TRACE,"api:sendMessageBlocking: Received Response to Message %d.  Code: %d", current_request_id, code);
			if (result) *result = cb->content;
			/* If this was an auth request we need to grab the session ID */
			if ((*msg)->type == TW_AUTH) {
				if (code == TWX_SUCCESS) {
					TW_LOG(TW_TRACE,"api:sendMessageBlocking: AUTH Message %d succeeded. Code:%d", current_request_id, code);
					tw_api->mh->ws->sessionId = cb->sessionId;
				} else {
					TW_LOG(TW_WARN,"api:sendMessageBlocking: AUTH Message %d failed. Code:%d", current_request_id, code);
				}
			}
			twMessageHandler_UnegisterResponseCallback(tw_api->mh, current_request_id);
			return code;
		}
	} else {
		/* Message failed to send - remove it from the response callback list */
		twMessageHandler_UnegisterResponseCallback(tw_api->mh, current_request_id);
		if (TW_WROTE_TO_OFFLINE_MSG_STORE == ret) {
			return TWX_WROTE_TO_OFFLINE_MSG_STORE;
		} else if (TW_ERROR_OFFLINE_MSG_STORE_FULL == ret) {
			return TWX_OFFLINE_MSG_STORE_FULL;
		} else if (TW_WEBSOCKET_NOT_CONNECTED == ret) {
			return TWX_SERVICE_UNAVAILABLE;
		} else {
			return TWX_PRECONDITION_FAILED;
		}
	}
}

enum msgCodeEnum sendMessage(twMessage ** msg,response_cb rcb) {
	DATETIME expirationTime;
	uint32_t current_request_id = 0;
	int ret = TW_OK;
	if (!tw_api || !tw_api->mh || !msg || !rcb) {
		TW_LOG(TW_ERROR, "api:sendMessage: NULL api, msg, callback or message handler pointer found");
		return TWX_UNKNOWN;
	}

	/* saving the current request ID, because if the message fails,
	* the twMessage struct will be free'd during the write to the
	* offline message store */
	current_request_id = (*msg)->requestId;

	/* Register the response handler before we send to prevent a race condition */
	expirationTime = twAddMilliseconds(twGetSystemMillisecondCount(), twcfg.default_message_timeout);
	twMessageHandler_RegisterResponseCallback(tw_api->mh, rcb, current_request_id, expirationTime);

	/* Send the message but don't wait for it to be received */
	ret = twMessage_Send(msg, tw_api->mh->ws);
	if (ret == TW_OK) {
		return TWX_SUCCESS;
	} else {
		/* Message failed to send - remove it from the response callback list */
		twMessageHandler_UnegisterResponseCallback(tw_api->mh, current_request_id);
		if (TW_WROTE_TO_OFFLINE_MSG_STORE == ret) {
			return TWX_WROTE_TO_OFFLINE_MSG_STORE;
		} else if (TW_ERROR_OFFLINE_MSG_STORE_FULL == ret) {
			return TWX_OFFLINE_MSG_STORE_FULL;
		} else if (TW_WEBSOCKET_NOT_CONNECTED == ret) {
			return TWX_SERVICE_UNAVAILABLE;
		} else {
			return TWX_PRECONDITION_FAILED;
		}
	}
}


enum msgCodeEnum makeRequest(enum msgCodeEnum method, enum entityTypeEnum entityType, char * entityName,
							 enum characteristicEnum characteristicType, char * characteristicName,
							 twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	enum msgCodeEnum res = TWX_PRECONDITION_FAILED;
	twMessage * msg = NULL;
	if (!tw_api || !entityName || !characteristicName || !result) {
		TW_LOG(TW_ERROR, "api:makeRequest: NULL tw_api, entityName, characteristicName or result pointer");
		return res;
	}
	/* Check to see if we are offline and should attempt to reconnect */
	if (!twApi_isConnected()) {
		if (forceConnect) {
			if (twApi_Connect(twcfg.connect_timeout, twcfg.connect_retries)) {
				TW_LOG(TW_ERROR, "api:makeRequest: Error trying to force a reconnect");
			}
		}
	}
	/* Create the Request message */
	msg = twMessage_CreateRequestMsg(method);
	if (!msg) {
		TW_LOG(TW_ERROR, "api:makeRequest: Error creating request message");
		return res;
	}
	if (timeout < 0) timeout = twcfg.default_message_timeout;
	/* Set the body of the message */
	twRequestBody_SetEntity((twRequestBody *)(msg->body), entityType, entityName);
    twRequestBody_SetCharacteristic((twRequestBody *) (msg->body), characteristicType, characteristicName);
	twRequestBody_SetParams((twRequestBody *)(msg->body), twInfoTable_ZeroCopy(params));

	/* Removed mutex lock around sendMessageBlocking to address a possibility for a deadlock when sending long outbound requests (e.g., synchronous file transfers).
	   When a client binds, the platform sends down NotifyPropertyUpdate and the SDK *would* lock on outgoing requests. If the client were to initiate a long outbound request at this time,
	   it may result in a deadlock. */
	res = s_sendMessageBlocking(&msg, timeout, result);

	if (msg) twMessage_Delete(msg);
	return res;
}

enum msgCodeEnum makeAsyncRequest(enum msgCodeEnum method, enum entityTypeEnum entityType, char * entityName,
							 enum characteristicEnum characteristicType, char * characteristicName,
							 twInfoTable * params, char forceConnect, response_cb cb, uint32_t* messageId) {
	enum msgCodeEnum res = TWX_PRECONDITION_FAILED;
	twMessage * msg = NULL;
	if (!tw_api || !entityName || !characteristicName || !cb ) {
		TW_LOG(TW_ERROR, "api:makeAsyncRequest: NULL tw_api, entityName or characteristicName");
		return res;
	}
	/* Check to see if we are offline and should attempt to reconnect */
	if (!twApi_isConnected()) {
		if (forceConnect) {
			if (twApi_Connect(twcfg.connect_timeout, twcfg.connect_retries)) {
				TW_LOG(TW_ERROR, "api:makeRequest: Error trying to force a reconnect");
			}
		}
	}
	/* Create the Request message */
	msg = twMessage_CreateRequestMsg(method);
	if (!msg) {
		TW_LOG(TW_ERROR, "api:makeRequest: Error creating request message");
		return res;
	}
	/* Set the body of the message */

	twRequestBody_SetEntity((twRequestBody *)(msg->body), entityType, entityName);
    twRequestBody_SetCharacteristic((twRequestBody *) (msg->body), characteristicType, characteristicName);
	twRequestBody_SetParams((twRequestBody *)(msg->body), twInfoTable_ZeroCopy(params));

	res = sendMessage(&msg,cb);
	if(TWX_SUCCESS == res) {
		*messageId = msg->requestId;
	} else {
		*messageId = 0;
	}
	if (msg)
		twMessage_Delete(msg);
	return res;
}

enum msgCodeEnum makePropertyRequest(enum msgCodeEnum method, enum entityTypeEnum entityType, char * entityName,
							 char * propertyName, twPrimitive * value, twPrimitive ** result, int32_t timeout, char forceConnect) {
	twInfoTable * value_it = NULL;
	twInfoTable * result_it = NULL;
	twPrimitive * tmp = NULL;

	int index;

	enum msgCodeEnum res;
	if (!result) return TWX_PRECONDITION_FAILED;
	if (value) {
		/* An InfoTable value could indicate a few different property write scenarios. */
		if (value->type == TW_INFOTABLE) {
			/* If this is a write of several properties at once, pass along the whole InfoTable. User must pass in the */
			/* correct format. */
			if (!strcmp(propertyName,"*")) {
				value_it = twInfoTable_ZeroCopy(value->val.infotable);
			}
			/* Special else if() to catch the special _content_ parameter in the infotable datashape entry. */
			/* This check determines if the input value is an InfoTable, and if the InfoTable has a field named _content_. */
			/* This allows the client to set a property, even if it doesn't know the properties type. */
			/* If it does have a field named _content_, then send over the whole InfoTable. */
			/* The platform knows to treat the _content_ field as a JSON param and decode */
			/* the contents correctly, based on the type of the property being PUT. */
			else if(value->val.infotable && value->val.infotable->ds && value->val.infotable->ds->numEntries == 1 && !twDataShape_GetEntryIndex(value->val.infotable->ds, "_content_", &index)) {
				value_it = twInfoTable_ZeroCopy(value->val.infotable);
			}
			/* If the value's datashape specifies a field whose name matches the propertyName, then assume that the value */
			/* parameter is an InfoTable (with a single field of type InfoTable) that wraps another InfoTable that contains */
			/* the property's new value. */
			else if(value->val.infotable && value->val.infotable->ds && value->val.infotable->ds->numEntries == 1 && !twDataShape_GetEntryIndex(value->val.infotable->ds, propertyName, &index)) {
				value_it = twInfoTable_ZeroCopy(value->val.infotable);
			}
			/* Assume that the value passed in is just the value for the property. In this case we need to wrap it in an */
			/* InfoTable with a single field specifying the property name. */
			else {
				value_it = twInfoTable_CreateFromPrimitive(propertyName, twPrimitive_ZeroCopy(value));
			}
		}
		/* Non-InfoTable properties are simply converted to an InfoTable to be serialized. */
		else {
			value_it = twInfoTable_CreateFromPrimitive(propertyName, twPrimitive_ZeroCopy(value));
		}
	}
	res = makeRequest(method, entityType, entityName, TW_PROPERTIES, propertyName, value_it, &result_it, timeout, forceConnect);
	twInfoTable_Delete(value_it);
	if (result_it) {
		/* if this is a request for all properties we need to treat the result a little differntly */
		if (!strcmp(propertyName,"")) {
			*result = twPrimitive_CreateFromInfoTable(result_it);
		} else {
			twInfoTable_GetPrimitive(result_it, propertyName, 0, &tmp);
			*result = twPrimitive_ZeroCopy(tmp);
		}
		twInfoTable_Delete(result_it);
	}
	return res;
}

int twApi_SendResponse(twMessage * msg) {
	if (tw_api && tw_api->mh && tw_api->mh->ws) return twMessage_Send(&msg, tw_api->mh->ws);
	return TW_ERROR_SENDING_RESP;
}
/**
 * The root request handler for all incoming requests. Understands how to
 * dispatch requests for property values and service invocations that come in
 * over the always on web socket connection.
 * @param ws
 * @param msg
 * @return varies based on how the request is dispatched, one of msgCodeEnum
 * values or TW_INVALID_RESP_MSG is the message could not be processed.
 */
int api_requesthandler(struct twWs * ws, struct twMessage * msg) {
	/*
	Need to look this up to see if we have a property or service callback registered.
	If not we check to see if there is a generic callback
	*/
	int res = TW_UNKNOWN_ERROR;
	twRequestBody * b = NULL;
	twMessage * resp = NULL;
	enum msgCodeEnum respCode = TWX_UNKNOWN;
	twInfoTable * result = NULL;
	void * cb = NULL;
	void * userdata = NULL;
	if (!msg || !tw_api) {
		TW_LOG(TW_ERROR,"api_requesthandler: Null msg pointer");
		return TW_INVALID_PARAM;
	}
	if (msg->type != TW_REQUEST) {
		TW_LOG(TW_ERROR,"api_requesthandler: Non Request message received");
		return TW_INVALID_MSG_TYPE;
	}
	b = (twRequestBody *) (msg->body);
	if (!b || !b->entityName) {
		TW_LOG(TW_ERROR,"api_requesthandler: No valid message body found");
		return TW_INVALID_MSG_BODY;
	}
	cb = findCallback(b->entityType, b->entityName, b->characteristicType, b->characteristicName, &userdata);
	if (cb) {
		switch (b->characteristicType) {
			case TW_PROPERTIES:
				{
					property_cb callback = (property_cb)cb;
					if (msg->code == TWX_PUT) {
						/*  This is a property write */
						if (!b->params) {
							TW_LOG(TW_ERROR,"api_requesthandler: Missing param in PUT message");
							return TW_INVALID_MSG_PARAMS;
						}
						respCode = callback(b->entityName, b->characteristicName, &b->params, TRUE, userdata);
						resp = twMessage_CreateResponseMsg(respCode, msg->requestId, msg->sessionId, msg->endpointId);
					} else {
						/* This is a read */
						respCode = callback(b->entityName, b->characteristicName, &result, FALSE, userdata);
						resp = twMessage_CreateResponseMsg(respCode, msg->requestId, msg->sessionId, msg->endpointId);
						if (resp && result) twResponseBody_SetContent((twResponseBody *)(resp->body), result);
					}
					break;
				}
			case TW_SERVICES:
				{
					service_cb callback = (service_cb)cb;
					respCode = callback(b->entityName, b->characteristicName, b->params, &result, userdata);
					resp = twMessage_CreateResponseMsg(respCode, msg->requestId, msg->sessionId, msg->endpointId);
					if (resp && respCode != TWX_SUCCESS && result) {
						/* try to extract a reason from the result infotable */
						char * reason = NULL;
						twInfoTable_GetString(result, "reason", 0, &reason);
						if (reason) twResponseBody_SetReason((twResponseBody *)resp->body, reason);
						TW_FREE(reason);
					}
					if (resp && result) twResponseBody_SetContent((twResponseBody *)(resp->body), result);
					break;
				}
			default:
				/* Try our generic message handler */
				if (tw_api->defaultRequestHandler) {
					resp = tw_api->defaultRequestHandler(msg);
				} else {
					/* No handler - return a 404 */
					resp = twMessage_CreateResponseMsg(TWX_NOT_FOUND, msg->requestId, msg->sessionId, msg->endpointId);
					if (!resp) TW_LOG(TW_ERROR,"api_requesthandler: Error allocating response message");
				}
		}
	} else {
		/* Try our generic message handler */
		if (tw_api->defaultRequestHandler) {
			resp = tw_api->defaultRequestHandler(msg);
		} else {
			/* No handler - return a 404 */
			TW_LOG(TW_INFO,"api_requesthandler: No handler found.  Returning 404");
			resp = twMessage_CreateResponseMsg(TWX_NOT_FOUND, msg->requestId, msg->sessionId, msg->endpointId);
			if (!resp) TW_LOG(TW_ERROR,"api_requesthandler: Error allocating response message");
		}
	}
	/* Send our response */
	if (resp){
		res = twApi_SendResponse(resp);
		twMessage_Delete(resp);
		return res;
	}
	return TW_INVALID_RESP_MSG;
}

/* internal prototype for dumping the incoming msg list */
int dumpIncomingMsgListCallback();


typedef struct MetadataServiceForEachParams {
	char * entityName;
	cJSON* propJson;
	cJSON* svcJson;
	cJSON* eventJson;

} MetadataServiceForEachParams;

int getMetadataServiceForEach(void *key, size_t key_size, void *data, size_t data_size,void *arg) {
	callbackInfo *currentCb = (callbackInfo *) data;
	MetadataServiceForEachParams *params = (MetadataServiceForEachParams *) arg;
	cJSON *jsonFragment;
	if (strcmp(params->entityName, currentCb->entityName) != 0)
		return TW_FOREACH_CONTINUE;

	if (NULL == currentCb->characteristicDefinition)
		return TW_FOREACH_CONTINUE;

	/* Add the definitions to the JSON */
	jsonFragment = cJSON_CreateObject();
	if (!jsonFragment) {
		TW_LOG(TW_ERROR, "getMetadataService: Error creating JSON object");
		return TW_FOREACH_EXIT;
	}

	switch (currentCb->characteristicType) {
		case TW_PROPERTIES: {
			cJSON_AddStringToObject(jsonFragment, "name", ((twPropertyDef *) currentCb->characteristicDefinition)->name);
			if (((twPropertyDef *) currentCb->characteristicDefinition)->description)
				cJSON_AddStringToObject(jsonFragment, "description",
										((twPropertyDef *) currentCb->characteristicDefinition)->description);
			cJSON_AddStringToObject(jsonFragment, "baseType", baseTypeToString(
					((twPropertyDef *) currentCb->characteristicDefinition)->type));
			/* Add all aspects */
			cJSON_AddItemReferenceToObject(jsonFragment, "aspects",
										   ((twPropertyDef *) currentCb->characteristicDefinition)->aspects);
			/* Add this porperty to the property JSON */
			cJSON_AddItemToObject(params->propJson, ((twPropertyDef *) currentCb->characteristicDefinition)->name, jsonFragment);
			jsonFragment = NULL;
			break;
		}
		case TW_SERVICES : {
			cJSON *aspectsJsonFragment = NULL;
			twServiceDef *svc = (twServiceDef *) currentCb->characteristicDefinition;
			cJSON_AddStringToObject(jsonFragment, "name", svc->name);
			cJSON_AddStringToObject(jsonFragment, "description", svc->description ? svc->description : "");

			/* Add all service level aspects */
			cJSON_AddItemReferenceToObject(jsonFragment, "aspects", svc->aspects);

			/* Inputs */
			if (svc->inputs)
				cJSON_AddItemToObject(jsonFragment, "Inputs", twDataShape_ToJson(svc->inputs, NULL));
			else
				cJSON_AddItemToObject(jsonFragment, "Inputs",
									  cJSON_CreateObject()); /* Need the empty Inputs for Composer to parse */

			aspectsJsonFragment = cJSON_CreateObject();

			if (aspectsJsonFragment) {
				cJSON_AddStringToObject(aspectsJsonFragment, "baseType", baseTypeToString(svc->outputType));
				if (svc->outputDataShape) {
					cJSON *datashapeAspects = cJSON_CreateObject();
					if(datashapeAspects) {
						/* Add the datashape name if the output is an infoTable */
						cJSON_AddStringToObject(datashapeAspects, "dataShape",
												svc->outputDataShape->name ? svc->outputDataShape->name : "");
						cJSON_AddItemToObject(aspectsJsonFragment, "aspects", datashapeAspects);
					}
				}
				/* Add the data shape definitions.  even if there is no datashape the fieldDefinitions element must exist */
				if (svc->outputDataShape)
					twDataShape_ToJson(svc->outputDataShape, aspectsJsonFragment);
				else
					cJSON_AddItemToObject(aspectsJsonFragment, "fieldDefinitions", cJSON_CreateObject());

				/* Data toJson function shape adds a name element at the top level - need to remove that */
				cJSON_DeleteItemFromObject(aspectsJsonFragment, "name");

				/* Set the name of the output to 'result' */
				cJSON_AddStringToObject(aspectsJsonFragment, "name", "result");

				/* Now add the Outputs to the service definition */
				cJSON_AddItemToObject(jsonFragment, "Outputs", aspectsJsonFragment);
			}

			/* Add this service definition to our  list of services */
			cJSON_AddItemToObject(params->svcJson, svc->name, jsonFragment);
			break;
		}

		case TW_EVENTS: {
			/* Event definitons are just service definitions without Outputs */
			twServiceDef *svc = (twServiceDef *) currentCb->characteristicDefinition;
			cJSON *eventData = twDataShape_ToJson(svc->inputs, NULL);
			cJSON *aspects = cJSON_CreateObject();
			if (svc && eventData && aspects) {
				cJSON_AddStringToObject(jsonFragment, "name", svc->name);
				cJSON_AddStringToObject(jsonFragment, "description", svc->description ? svc->description : "");
				/* Add all event level aspects */
				cJSON_AddItemReferenceToObject(jsonFragment, "aspects", svc->aspects);
				/* Add the Datashape name as an aspect */
				cJSON_AddStringToObject(aspects, "dataShape",
										svc->inputs->name ? svc->inputs->name : "");
				cJSON_AddItemToObject(eventData, "aspects", aspects);
				/* Add the dataSHape definition */
				if (svc->inputs) cJSON_AddItemToObject(jsonFragment, "EventData", eventData);
				else
					cJSON_AddItemToObject(jsonFragment, "EventData",
										  cJSON_CreateObject()); /* Need the empty Inputs for Composer to parse */
				/* Add this event definition to our list of events */
				cJSON_AddItemToObject(params->eventJson, svc->name, jsonFragment);
			} else {
				if (eventData) cJSON_Delete(eventData);
				if (aspects) cJSON_Delete(aspects);
			}
			break;
		}

		default: {
			/* nothing of interest, move on */
			cJSON_Delete(jsonFragment);
		}
	}
	return TW_FOREACH_CONTINUE;
}

enum msgCodeEnum getMetadataService(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata) {
	cJSON * propJson = NULL;
	cJSON * svcJson = NULL;
	cJSON * eventJson = NULL;
	cJSON * completeJson = NULL;
	char * jsonString = NULL;
	MetadataServiceForEachParams* forEachParams;

	completeJson = cJSON_CreateObject();
	TW_LOG(TW_TRACE,"getMetadataService - Function called");
	if (!content || !completeJson ||!tw_api || !tw_api->callbackList || !entityName) {
		TW_LOG(TW_ERROR,"getMetadataService - NULL stream,callback, params or content pointer");
		if (propJson) cJSON_Delete(propJson);
		if (svcJson) cJSON_Delete(svcJson);
		if (eventJson) cJSON_Delete(eventJson);
		return TWX_BAD_REQUEST;
	}
	/* Prep the  complete JSON */
	cJSON_AddStringToObject(completeJson,"name", entityName);
	cJSON_AddStringToObject(completeJson,"description","");
	cJSON_AddFalseToObject(completeJson,"isSystemObject");

	forEachParams = TW_MALLOC(sizeof(MetadataServiceForEachParams));
	forEachParams->entityName = (char*)entityName;
	forEachParams->propJson = cJSON_CreateObject();
	forEachParams->svcJson = cJSON_CreateObject();
	forEachParams->eventJson = cJSON_CreateObject();

	twDict_Foreach(tw_api->callbackList,getMetadataServiceForEach,forEachParams);
	propJson = forEachParams->propJson;
	svcJson = forEachParams->svcJson;
	eventJson = forEachParams->eventJson;
	TW_FREE(forEachParams);

	/* Combine the json  */
	cJSON_AddItemToObject(completeJson, "propertyDefinitions", propJson);
	cJSON_AddItemToObject(completeJson, "serviceDefinitions", svcJson);
	cJSON_AddItemToObject(completeJson, "eventDefinitions", eventJson);

	/* Create the result infotable */
	jsonString = cJSON_PrintUnformatted(completeJson);
	*content = twInfoTable_CreateFromPrimitive("result", twPrimitive_CreateFromVariable(jsonString, TW_JSON, FALSE, 0));
	cJSON_Delete(completeJson);

	if (*content) {
		return TWX_SUCCESS;
	}
	return TWX_INTERNAL_SERVER_ERROR;
}

char receivedPong = FALSE;
int pong_handler (struct twWs * ws, const char * data, size_t length) {
	receivedPong = TRUE;
	return 0;
}

typedef struct twMakeAuthOrBindCallbacksParam {
	char * entityName;
	enum entityTypeEnum entityType;
	char type;
	char * value;
} twMakeAuthOrBindCallbacksParam;

int twMakeAuthOrBindCallbacksForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twMakeAuthOrBindCallbacksParam* options = (twMakeAuthOrBindCallbacksParam*)userData;
	callbackInfo *info = (callbackInfo *)currentValue;
	if (options->type == 0) {
		/* Auth callback */
		if (info->entityType == TW_APPLICATIONKEYS) {
			authEvent_cb cb = (authEvent_cb) info->cb;
			cb(options->entityName, options->value, info->userdata);
		}
	} else {
		if (info->entityType != TW_APPLICATIONKEYS &&
			(!options->entityName || (!info->entityName || !strcmp(info->entityName, options->entityName)))) {
			bindEvent_cb cb = (bindEvent_cb) info->cb;
			cb(info->entityName, (options->type == 1), info->userdata);
		}
	}
	return TW_FOREACH_CONTINUE;
}
int makeAuthOrBindCallbacks(char * entityName, enum entityTypeEnum entityType, char type, char * value) {
	/* Type -> 0 - Auth, 1 - Bind, 2 - Unbind */
	/* Validate all inputs */
	if ((type == 0 && (value == NULL || entityName == NULL)) || !tw_api || !tw_api->bindEventCallbackList) {
		TW_LOG(TW_ERROR, "makeAuthOrBindCallback: Invalid parameter found");
		return TW_INVALID_PARAM;
	} else {
		twMakeAuthOrBindCallbacksParam *options = (twMakeAuthOrBindCallbacksParam *) TW_MALLOC(
				sizeof(twMakeAuthOrBindCallbacksParam));
		options->entityType = entityType;
		options->entityName = entityName;
		options->type = type;
		options->value = value;
		twList_Foreach(tw_api->bindEventCallbackList, twMakeAuthOrBindCallbacksForeachHandler, (void *) options);
		TW_FREE(options);
	}
	return TW_OK;
}

typedef struct twMakeSynchronizedStateCallbacksParam {
	char * entityName;
	enum entityTypeEnum entityType;
	twInfoTable* subscriptionData;
} twMakeSynchronizedStateCallbacksParam;

int twMakeSynchronizedStateCallbacksForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twMakeSynchronizedStateCallbacksParam* options = (twMakeSynchronizedStateCallbacksParam*)userData;
	callbackInfo *info = (callbackInfo *)currentValue;
	if (info->entityType != TW_APPLICATIONKEYS &&
		(!options->entityName || (!info->entityName || !strcmp(info->entityName, options->entityName)))) {
		synchronizeEvent_cb cb = (synchronizeEvent_cb) info->cb;
		cb(options->entityName, options->subscriptionData, info->userdata);
	}
	return TW_FOREACH_CONTINUE;
}

int makeSynchronizedStateCallbacks(char * entityName, enum entityTypeEnum entityType, twInfoTable* subscriptionData) {
	/* Validate all inputs */
	if ( entityName == NULL || !tw_api || !tw_api->synchronizeStateEventCallbackList) {
		TW_LOG(TW_ERROR, "synchronizeStateEventCallback: Invalid parameter found");
		return TW_INVALID_PARAM;
	} else {
		twMakeSynchronizedStateCallbacksParam *options = (twMakeSynchronizedStateCallbacksParam *) TW_MALLOC(
				sizeof(twMakeSynchronizedStateCallbacksParam));
		options->entityType = entityType;
		options->entityName = entityName;
		options->subscriptionData = subscriptionData;
		twList_Foreach(tw_api->synchronizeStateEventCallbackList, twMakeSynchronizedStateCallbacksForeachHandler, (void *) options);
		TW_FREE(options);
	}
	return TW_OK;
}

int registerServiceOrEvent(enum entityTypeEnum entityType, char * entityName, char * serviceName, char * serviceDescription,
						  twDataShape * inputs, enum BaseType outputType, twDataShape * outputDataShape, service_cb cb, void * userdata, char isService) {
	int returnValue;
	callbackInfo * info = NULL;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->callbackList && entityName && serviceName && ((isService && cb) || !isService)) {
		twServiceDef * service = twServiceDef_Create(serviceName, serviceDescription, inputs, outputType,
														outputDataShape);
		info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
		if (!info || !service) {
			TW_LOG(TW_ERROR, "registerServiceOrEvent: Error allocating callback info");
			if (info) TW_FREE(info);
			twServiceDef_Delete(service);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		info->entityType = entityType;
		info->entityName = duplicateString(entityName);
		info->characteristicType = isService ? TW_SERVICES : TW_EVENTS;
		info->characteristicName = duplicateString(service->name);
		info->characteristicDefinition = service;
		info->cb = cb;
		info->userdata = userdata;
		returnValue = twDict_Add(tw_api->callbackList, info);
		return returnValue;
	}
	TW_LOG(TW_ERROR, "registerServiceOrEvent: Invalid params or missing api pointer");
	return TW_INVALID_PARAM;
}

int AddAspectToEntity(char * entityName, enum characteristicEnum type,  char * characteristicName,
							  char * aspectName, twPrimitive * aspectValue) {

	callbackInfo *cbInfoQuery;
	void* result;

	if (!(tw_api )) {
		TW_LOG(TW_ERROR, "twApi_AddAspectToProperty: NULL or invalid api singleton. Call twApi_Initalize() first.");
		return TW_NULL_OR_INVALID_API_SINGLETON;
	}
	if (!(tw_api->callbackList && entityName && characteristicName && aspectName && aspectValue)) {
		TW_LOG(TW_ERROR, "twApi_AddAspectToProperty: Invalid params or missing api pointer");
		return TW_INVALID_PARAM;
	}

	/* Build a query structure */
	cbInfoQuery = (callbackInfo *) TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = type;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = characteristicName;

	if(TW_OK == twDict_Find(tw_api->callbackList,cbInfoQuery,&result)){
		callbackInfo *cb = (callbackInfo *) result;
		switch (type) {
			case TW_PROPERTIES: {
				twPropertyDef *def = (twPropertyDef *) cb->characteristicDefinition;
				twPrimitive_ToJson(aspectName, aspectValue, def->aspects);
				break;
			}
			case TW_SERVICES:
			case TW_EVENTS: {
				twServiceDef *def = (twServiceDef *) cb->characteristicDefinition;
				twPrimitive_ToJson(aspectName, aspectValue, def->aspects);
				break;
			}
			default:
				break;
		}
		twPrimitive_Delete(aspectValue);
		TW_FREE(cbInfoQuery);
		return TW_OK;
	}

	TW_LOG(TW_ERROR, "AddAspectToEntity: Characteristic %s not found in Entity %s", characteristicName, entityName);
	if (aspectValue)
		twPrimitive_Delete(aspectValue);
	TW_FREE(cbInfoQuery);
	return TW_INVALID_PARAM;

}

/**
 * Converts a callbackInfo structure into a representative index key for use in a hashmap.
 * These fields are required to parse a callback info item:
 * entityType - Always should be TW_THING
 * entityName - The name of the Thing you are searching for
 * characteristicName - must be one of TW_PROPERTIES, TW_SERVICES or TW_EVENTS.
 * characteristicType - This will be the name of the property or service you are looking for.
 *
 * @param data callbackInfo struct pointer with the fields listed above set.
 * @return A string representing a valid hash key for the passed in callbackInfo structure.
 */
const char* twCallbackInfoParser(void * data){
	static int maxKeyLength = 500;
	callbackInfo * cbInfo = (callbackInfo *)data;
	char* indexKey;

	/* Make sure all key fields are provided */
	if(!(cbInfo->entityType&&cbInfo->entityName&&cbInfo->characteristicName&&cbInfo->characteristicType))
		return NULL;

	indexKey = TW_MALLOC(maxKeyLength+1);
	snprintf(indexKey,maxKeyLength, "%i|%s|%s|%i", cbInfo->entityType,cbInfo->entityName,cbInfo->characteristicName,cbInfo->characteristicType);
	return indexKey;
}

void twDisplayVersionInformation() {
    TW_LOG(TW_WARN, "SDK Version: %s", twApi_GetVersion());

    TW_LOG(TW_WARN, "TLS Library: OpenSSL");
    TW_LOG(TW_WARN, "TLS Library Version: %s", openssl_version);

#ifdef TW_FIPS_CAPABLE
    TW_LOG(TW_WARN, "FIPS Capable");
#endif
}

void emptyCallback (void * userdata){};

/*********************************************************/
/* API Functions */
int twApi_Initialize(char * host, uint16_t port, char * resource,  twPasswdCallbackFunction app_key_function, char * gatewayName,
						 uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect) {
	int err = TW_UNKNOWN_ERROR;
	int retries = twcfg.connect_retries;
	twWs * ws = NULL;

    /* Need to at least have a callback function registered */
   if (!twcfg.initCallback) {
		twApi_RegisterInitCallback (emptyCallback, NULL);
	}

	twDisplayVersionInformation();
    twcfg.initCallback->cb(twcfg.initCallback->userdata);

	/* Validate all inputs */
	if (!host || 0 == port || !resource || !app_key_function) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Invalid parameter found");
		return TW_INVALID_PARAM;
	}
	/* Create our global initialization mutex */
	if (!twInitMutex) twInitMutex = twMutex_Create();
	if (!twInitMutex) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Error creating initialization mutex");
		return TW_ERROR_CREATING_MTX;
	}
	/* create stubs */
	twApi_CreateStubs();

	twMutex_Lock(twInitMutex);
	/* Check to see if the singleton already exists */
	if (tw_api) {
		TW_LOG(TW_WARN, "twApi_Initialize: API singleton already exists");
		twMutex_Unlock(twInitMutex);
		return TW_OK;
	}
	/* Create the websocket */
	err = twWs_Create(host, port, resource, app_key_function, gatewayName, messageChunkSize, frameSize, &ws);
	while (retries !=0 && err) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Error creating websocket structure, retrying");
		twSleepMsec(twcfg.connect_retry_interval);
		if (retries > 0) {
			retries--;
		}
		err = twWs_Create(host, port, resource, app_key_function, gatewayName, messageChunkSize, frameSize, &ws);
	}
	if (err) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Could not connect after %d attempts", twcfg.connect_retries);
		twMutex_Unlock(twInitMutex);
		return err;
	}
	TW_LOG(TW_DEBUG, "twApi_Initialize: Websocket Established after %d tries", (twcfg.connect_retries - retries));

	/* Allocate space for the structure */
	tw_api = (twApi *)TW_CALLOC(sizeof(twApi), 1);
	if (!tw_api) {
		twWs_Delete(ws);
		TW_LOG(TW_ERROR, "twApi_Initialize: Error allocating api structure");
		twMutex_Unlock(twInitMutex);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	/* Initialize the message handler, mutex, and lists */
	tw_api->mh = twMessageHandler_Instance(ws);
	tw_api->mtx = twMutex_Create();
	tw_api->callbackList = twDict_Create(deleteCallbackInfo,twCallbackInfoParser);
	tw_api->bindEventCallbackList = twList_Create(deleteCallbackInfo);
	tw_api->synchronizeStateEventCallbackList = twList_Create(deleteCallbackInfo);
	tw_api->boundList = twDict_Create(bindListEntry_Delete,bindListEntry_Parser);
	if (!tw_api->mh || !tw_api->mtx || !tw_api->callbackList || !tw_api->boundList || !tw_api->bindEventCallbackList ||!tw_api->synchronizeStateEventCallbackList) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Error initializing api");
        twMutex_Unlock(twInitMutex);
        twApi_Delete();
		return TW_ERROR_INITIALIZING_API;
	}
	twMessageHandler_RegisterDefaultRequestCallback(tw_api->mh, api_requesthandler);
	twMessageHandler_RegisterDumpIncomingMsgListCallback(tw_api->mh, dumpIncomingMsgListCallback);
	tw_api->autoreconnect = autoreconnect;
	tw_api->manuallyDisconnected = TRUE;
	tw_api->isAuthenticated = FALSE;
	tw_api->defaultRequestHandler = NULL;
	tw_api->connectionInProgress = FALSE;
	twApi_SetDutyCycle(twcfg.duty_cycle, twcfg.duty_cycle_period);
	/* Set up our ping/pong handling */
	tw_api->ping_rate = twcfg.ping_rate;
	tw_api->handle_pongs = TRUE;

	/* Mark this field false expecting it to me changed to TRUE once we have received our first notifyPropertyUpdate
	 * message */
	tw_api->firstSynchronizationComplete = FALSE;

	twMessageHandler_RegisterPongCallback(tw_api->mh, pong_handler);

	if (OFFLINE_MSG_STORE == 1 && twcfg.offline_msg_store_dir) {
		err = enableOfflineMsgStore(TRUE, FALSE);
	} else if (OFFLINE_MSG_STORE==2 && twcfg.offline_msg_store_dir) {
		err = enableOfflineMsgStore(TRUE, TRUE);
	}
	if (err) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Error initializing offline message store");
		twMutex_Unlock(twInitMutex);
        twApi_Delete();
		return TW_ERROR_INITIALIZING_API;
	}
#ifdef ENABLE_TASKER
	/* Initalize our tasker */
	twTasker_CreateTask(5, &twApi_TaskerFunction);
	twTasker_Start();
#endif
#ifdef ENABLE_TUNNELING
	/* Create our connection info structure */
	tw_api->connectionInfo = twConnectionInfo_Create(NULL);
	if (tw_api->connectionInfo) {
		tw_api->connectionInfo->ws_host = duplicateString(host);
		tw_api->connectionInfo->ws_port = port;
		tw_api->connectionInfo->appkeyFunction = app_key_function;
		twTunnelManager_Create();
	}
#endif
#ifdef ENABLE_FILE_XFER
	if (twcfg.max_message_size < twcfg.file_xfer_block_size + twcfg.message_chunk_size) {
		twcfg.max_message_size = twcfg.file_xfer_block_size + twcfg.message_chunk_size;
		TW_LOG(TW_WARN, "Max message size must be larger than the File Transfer Block Size, increasing to %d", twcfg.max_message_size);
	}
#endif

	/* Initialize the Subscribed Properties Manager */

	/* first set the subscribed properties directory */

	err = twSubscribedPropsMgr_Initialize();
	if (err) {
		TW_LOG(TW_ERROR, "twApi_Initialize: Error initializing api");
        twMutex_Unlock(twInitMutex);
        twApi_Delete();
        return TW_ERROR_INITIALIZING_API;
	}
	twMutex_Unlock(twInitMutex);
	return TW_OK;
}

int twApi_Delete() {
	int res = TW_OK;
	twApi * tmp = tw_api;
	if (!tw_api) return TW_OK;
	twApi_Disconnect("Shutting down");
	twApi_StopConnectionAttempt();
#ifdef ENABLE_TASKER
	/* Stop the tasker */
	twTasker_Stop();
#endif
#ifdef ENABLE_FILE_XFER
	twFileManager_Delete();
#endif
#ifdef ENABLE_TUNNELING
	twTunnelManager_Delete();
#endif
	if (tw_api->tw_property_dict) twDict_Delete(tw_api->tw_property_dict);
	if (tw_api->tw_used_property_names) twDict_Delete(tw_api->tw_used_property_names);
	if (tw_api->tw_used_service_names) twDict_Delete(tw_api->tw_used_service_names);
	if (tw_offline_msg_store) twOfflineMsgStore_Delete();
	if (tw_api->connectionInfo) twConnectionInfo_Delete(tw_api->connectionInfo);
	/* Shut down the subscribed property Manager */
	twSubscribedPropsMgr_Delete();
	/* Set the singleton to NULL so no one else uses it */
	twMutex_Lock(twInitMutex);
	twMutex_Lock(tmp->mtx);
	tw_api = NULL;
	if (tmp->mh) twMessageHandler_Delete(NULL);
	if (tmp->callbackList) twDict_Delete(tmp->callbackList);
	if (tmp->bindEventCallbackList) twList_Delete(tmp->bindEventCallbackList);
	if (tmp->synchronizeStateEventCallbackList) twList_Delete(tmp->synchronizeStateEventCallbackList);
	if (tmp->boundList) twMap_Delete(tmp->boundList);
	if (tmp->subscribedPropsFile)TW_FREE(tmp->subscribedPropsFile);
	twMutex_Unlock(tmp->mtx);
	twMutex_Delete(tmp->mtx);
	twMutex_Unlock(twInitMutex);
	twMutex_Delete(twInitMutex);
	twInitMutex = NULL;
	TW_FREE(tmp);
	res = twDict_Cleanup();
	twLogger_Delete();
	return res;
}

#ifdef ENABLE_HTTP_PROXY_SUPPORT
int twApi_SetProxyInfo(char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction proxyPassCallback) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (!tw_api || !tw_api->mh || !tw_api->mh->ws || !tw_api->mh->ws->connection || !tw_api->mh->ws->connection->connection) return TW_SOCKET_INIT_ERROR;
	return twSocket_SetProxyInfo(tw_api->mh->ws->connection->connection, proxyHost, proxyPort, proxyUser, proxyPassCallback);
}

int twApi_ClearProxyInfo() {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (!tw_api || !tw_api->mh || !tw_api->mh->ws || !tw_api->mh->ws->connection || !tw_api->mh->ws->connection->connection) return TW_SOCKET_INIT_ERROR;
	return twSocket_ClearProxyInfo(tw_api->mh->ws->connection->connection);
}
#endif

char * twApi_GetVersion() {
	return C_SDK_VERSION;
}

typedef struct twApi_BindAllForEachHandlerParams {
	twMessage * msg;
	char unbind;
} twApi_BindAllForEachHandlerParams;

int twApi_BindAllForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
	int res = TW_OK;
	int ret = TW_FOREACH_EXIT;
	twApi_BindAllForEachHandlerParams * params = (twApi_BindAllForEachHandlerParams *)arg;
	bindListEntry *e = (bindListEntry *) data;
	if (params->msg == NULL) {
		params->msg = twMessage_CreateBindMsg(e->name, params->unbind);
	} else {
		res = twBindBody_AddName((twBindBody *) (params->msg->body), e->name);
		if (TW_BIND_MESSAGE_FULL == res) {
			enum msgCodeEnum resp = TWX_UNKNOWN;
			/* adding this name would exceed the max message size so it was not added */
			TW_LOG(TW_INFO, "twApi_BindAllForEachHandler: sending partial bind/unbind message");
			/* send what we have */
			resp = sendMessageBlocking(&(params->msg), twcfg.default_message_timeout, NULL);
			if (params->msg) s_twMessage_Delete(params->msg);
			/* create bind message with name that didn't get added to the previously sent msg */
			params->msg = s_twMessage_CreateBindMsg(e->name, params->unbind);
			if (resp != TWX_SUCCESS) {
				TW_LOG(TW_ERROR, "twApi_BindAllForEachHandler: Error sending partial bind/unbind message");
				/* sending the partial bind message failed so bail out of the loop */
				return TW_FOREACH_EXIT;
			}
		}
	}
	if (res == TW_OK || res == TW_BIND_MESSAGE_FULL) ret = TW_FOREACH_CONTINUE;
	return ret;
}

int twApi_BindAll(char unbind) {
	enum msgCodeEnum res = TWX_SUCCESS;
	twApi_BindAllForEachHandlerParams* params;
	twMessage * msg = NULL;
	if (!tw_api || !tw_api->mh || !tw_api->mh->ws || !tw_api->boundList)
		return TW_INVALID_PARAM;
	twMutex_Lock(tw_api->mtx);

	params = (twApi_BindAllForEachHandlerParams*) TW_MALLOC(sizeof(twApi_BindAllForEachHandlerParams));
	params->msg = NULL;
	params->unbind = unbind;
	twDict_Foreach(tw_api->boundList,twApi_BindAllForEachHandler,(void*)params);
	if(params->msg) res = s_sendMessageBlocking(&params->msg, twcfg.default_message_timeout, NULL);
	if(params->msg) twMessage_Delete(params->msg);
		TW_FREE(params);

	twMutex_Unlock(tw_api->mtx);
	if (msg) twMessage_Delete(msg);

	/* Look for any callbacks */
	if (res == TWX_SUCCESS)
		makeAuthOrBindCallbacks(NULL, TW_THING, 1, NULL);

	return convertMsgCodeToErrorCode(res);
}

int twApi_Authenticate() {
	int res = TW_UNKNOWN_ERROR;
	twMessage * msg = NULL;
	char* api_key = NULL;
	if (!tw_api || !tw_api->mh || !tw_api->mh->ws) return TW_INVALID_PARAM;
	twMutex_Lock(tw_api->mtx);
	api_key = twConvertCallbackToPasswd(tw_api->mh->ws->api_key_callback);
	msg = twMessage_CreateAuthMsg("appKey", api_key);
	twFreePasswd(api_key);
	res = convertMsgCodeToErrorCode(s_sendMessageBlocking(&msg, twcfg.default_message_timeout, NULL));
	if (res == TW_OK) tw_api->isAuthenticated = TRUE;
	twMutex_Unlock(tw_api->mtx);
	if (msg) twMessage_Delete(msg);
	return res;
}

int twApi_Connect(uint32_t timeout, int16_t retries) {
	int res = TW_OK;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	tw_api->connect_timeout = timeout;
	tw_api->connect_retries = retries;
#ifdef ENABLE_FILE_XFER
	/* These settings can change between init and connect so check again */
	if (twcfg.max_message_size < twcfg.file_xfer_block_size + twcfg.message_chunk_size) {
		twcfg.max_message_size = twcfg.file_xfer_block_size + twcfg.message_chunk_size;
		TW_LOG(TW_WARN, "Max message size must be larger than the File Transfer Block Size, increasing to %d", twcfg.max_message_size);
	}
#endif
	if (tw_api->mh && tw_api->mh->ws && !tw_api->connectionInProgress) {
		uint32_t delayTime;
		tw_api->connectionInProgress = TRUE;
		tw_api->manuallyDisconnected = TRUE;
		/* Delay a random amount */
		delayTime = (rand() * twcfg.max_connect_delay)/RAND_MAX;
		TW_LOG(TW_TRACE, "twApi_Connect: Delaying %d milliseconds before connecting", delayTime);
		twSleepMsec(delayTime);
		while (retries != 0 && tw_api->connectionInProgress) {
			twMutex_Lock(tw_api->mtx);
			res = s_twWs_Connect(tw_api->mh->ws, timeout);
			/*   if (res == TW_OK) tw_api->manuallyDisconnected = FALSE;  */
			twMutex_Unlock(tw_api->mtx);
			if (!res) res = twApi_Authenticate();
			if (!res) res = twApi_BindAll(FALSE);
			if (!res) break;
			if (retries != -1) retries--;
			twSleepMsec(twcfg.connect_retry_interval);
		}
		if (retries == 0) {
			/* if retries hits 0, that means we have attempted connecting the max number of times and we should warn the user */
			TW_LOG(TW_ERROR, "twApi_Connect: Max number of connect retries: %d, has been reached", tw_api->connect_retries);
		}
		/*
		If we have exhausted out retries we don't want to continue
		attempting to connect
		*/
		if (res == TW_OK) {
			tw_api->manuallyDisconnected = FALSE;
		}
		tw_api->connectionInProgress = FALSE;
	}
	tw_api->firstConnectionComplete = TRUE;
	return res;
}

int twApi_Disconnect(char * reason) {
	int res = TW_UNKNOWN_ERROR;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	twApi_BindAll(TRUE);
	twMutex_Lock(tw_api->mtx);
	if (tw_api->mh){
		tw_api->manuallyDisconnected = TRUE;
		tw_api->isAuthenticated = FALSE;
		res = twWs_Disconnect(tw_api->mh->ws, NORMAL_CLOSE, reason);
	}

	twMutex_Unlock(tw_api->mtx);
	return res;
}

char twApi_isConnected() {
	if (tw_api && tw_api->mh && tw_api->mh->ws) return twWs_IsConnected(tw_api->mh->ws) && tw_api->isAuthenticated;
	return FALSE;
}

char twApi_ConnectionInProgress() {
	if (tw_api) return tw_api->connectionInProgress;
	return FALSE;
}

int twApi_StopConnectionAttempt() {
	if (tw_api) tw_api->connectionInProgress = FALSE;
	return 0;
}

int twApi_SetDutyCycle(uint8_t duty_cycle, uint32_t period) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (duty_cycle > 100) duty_cycle = 100;
	tw_api->duty_cycle = duty_cycle;
	tw_api->duty_cycle_period = period;
	twcfg.duty_cycle = duty_cycle;
	twcfg.duty_cycle_period = period;
	return TW_OK;
}

int twApi_SetPingRate(uint32_t rate) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	tw_api->ping_rate = rate;
	return TW_OK;
}

int twApi_SetConnectTimeout(uint32_t timeout) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	tw_api->connect_timeout = timeout;
	return TW_OK;
}

int twApi_SetConnectRetries(signed char retries) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	tw_api->connect_retries = retries;
	return TW_OK;
}

int twApi_SetGatewayName(const char* input_name){
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if(!input_name) return TW_INVALID_PARAM;
	tw_api->mh->ws->gatewayName = duplicateString(input_name);
	if(!tw_api->mh->ws->gatewayName) return TW_ERROR;
	return TW_OK;
}

int twApi_SetGatewayType(const char* input_type){
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if(!input_type) return TW_INVALID_PARAM;
	tw_api->mh->ws->gatewayType = duplicateString(input_type);
	if(!tw_api->mh->ws->gatewayType) return TW_ERROR;
	return TW_OK;
}

int twApi_BindThingWithoutDefaultServices(char * entityName) {
	int res = TW_UNKNOWN_ERROR;
	enum msgCodeEnum resp;
	twMessage * msg = NULL;
	if (!tw_api || !entityName || !tw_api->bindEventCallbackList) {
		TW_LOG(TW_ERROR, "twApi_BindThing: NULL tw_api or entityName");
		return TW_INVALID_PARAM;
	}
	/* Add it to the list */
	twDict_Add(tw_api->boundList, bindListEntry_Create(entityName));

	/* If we are not connected, we are done */
	if (!twApi_isConnected()) return TW_OK;
	/* Create the bind message */
	msg = twMessage_CreateBindMsg(entityName, FALSE);
	if (!msg) {
		TW_LOG(TW_ERROR, "twApi_BindThing: Error creating Bind message");
		return TW_ERROR_CREATING_MSG;
	}
	twMutex_Lock(tw_api->mtx);
	resp = sendMessageBlocking(&msg, 10000, NULL);
	twMutex_Unlock(tw_api->mtx);
	res = convertMsgCodeToErrorCode(resp);
	if (res != TW_OK) TW_LOG(TW_ERROR, "twApi_BindThing: Error sending Bind message");
	if (msg) twMessage_Delete(msg);
	/* Look for any callbacks */
	if (!res) makeAuthOrBindCallbacks(entityName, TW_THING, 1, NULL);
	return res;
}

int twApi_BindThing(char * entityName) {
	int res = TW_UNKNOWN_ERROR;
	twList * soloList = NULL;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (entityName) {
		soloList = s_twList_Create(TW_NO_DELETION_FUNCTION);
		s_twList_Add(soloList, duplicateString(entityName));
	} else {
		TW_LOG(TW_ERROR, "twApi_BindThing: NULL entityName");
		return TW_INVALID_PARAM;
	}
	res = twApi_BindThings_Metadata_Option(soloList, FALSE);
	twList_Delete(soloList);
	return res;
}

typedef struct twBindThingsForeachParameters {
    twMessage * msg;
    char omitMetadataCallback;
    char duplicateDetected;
} twBindThingsForeachParameters;

twBindThingsForeachParameters * twBindThingsForeachParameters_Create(
    twMessage * msg,
    char omitMetadataCallback
) {
    twBindThingsForeachParameters * params = TW_MALLOC(
        sizeof(twBindThingsForeachParameters)
    );
    if (params) {
        params->msg = msg;
        params->duplicateDetected = FALSE;
        params->omitMetadataCallback = omitMetadataCallback;
    }

    return params;
}

void twBindThingsForeachParameters_Delete(
    twBindThingsForeachParameters * params
) {
    if (NULL == params) return;

    if (params->msg) {
        s_twMessage_Delete(params->msg);
    }
    TW_FREE(params);
}

int twBindThingsForeachHandler(
	void *key,
	size_t key_size,
	void *currentValue,
	size_t currentValue_size,
	void *userData
){
	int res = TW_OK;
	int ret = TW_FOREACH_EXIT;
	void* result;
	twBindThingsForeachParameters* params = (twBindThingsForeachParameters*)userData;
	/* Add it to the list */
	bindListEntry *blEntry = bindListEntry_Create((char *) currentValue);
	if(TW_OK == twDict_Find(tw_api->boundList,blEntry,&result)){
		/* We already have one of these bound */
		params->duplicateDetected = TRUE;
		/* free bindListEntry */
		bindListEntry_Delete(blEntry);
		return TW_FOREACH_EXIT;/* Abort */
	}
	s_twDict_Add(tw_api->boundList, blEntry);
	TW_LOG(TW_DEBUG, "added %s to boundList", blEntry->name);
	/* Register our metadata service handler */
	if(params->omitMetadataCallback == FALSE) {
		s_twApi_RegisterService(TW_THING, (char *) currentValue, "GetMetadata", NULL, NULL, TW_JSON, NULL,
								getMetadataService, NULL);
	}
	s_twApi_RegisterService(TW_THING, (char *) currentValue, "NotifyPropertyUpdate", NULL, NULL, TW_NOTHING, NULL,
							notifyPropertyUpdateHandler, NULL);

	if(params->msg == NULL){
		/* create bind message */
		params->msg = s_twMessage_CreateBindMsg((char *) currentValue, FALSE);
	} else {
		/* add name to bind message */
		res = s_twBindBody_AddName((twBindBody *) (params->msg->body), (char *) currentValue);
		if (res == TW_BIND_MESSAGE_FULL) {
			enum msgCodeEnum resp = TWX_UNKNOWN;
			/* adding this name would exceed the max message size so it was not added */
			TW_LOG(TW_INFO, "twBindThingsForeachHandler: sending partial bind message");
			/* send what we have */
			if (s_twApi_isConnected()) {
				resp = sendMessageBlocking(&params->msg, twcfg.default_message_timeout, NULL);
			} else {
                TW_LOG(TW_DEBUG, "twBindThingsForeachHandler: not currently connected, only binding things to api");
                resp = TWX_SUCCESS;
            }
			if (params->msg) s_twMessage_Delete(params->msg);
			/* create bind message with name that didn't get added to the previously sent msg */
			params->msg = s_twMessage_CreateBindMsg((char *)currentValue, FALSE);
			if (resp != TWX_SUCCESS) {
				TW_LOG(TW_ERROR, "twBindThingsForeachHandler: Error sending partial bind message");
				/* sending the partial bind message failed so bail out of the loop */
				res =  FALSE;
			}
		}
	}
	if (res == TW_OK || res == TW_BIND_MESSAGE_FULL) ret = TW_FOREACH_CONTINUE;
	return ret;
}

int twApi_BindThings(twList *entityNames) {
	return twApi_BindThings_Metadata_Option(entityNames, FALSE);
}

int twApi_BindThings_Metadata_Option(twList *entityNames, char omitMetadataCallback) {
	int res = TW_UNKNOWN_ERROR;
	enum msgCodeEnum resp = TWX_UNKNOWN;
	twBindThingsForeachParameters *params = NULL;
	if (!tw_api || !tw_api->mh || !tw_api->mh->ws || !tw_api->boundList || !tw_api->bindEventCallbackList) {
		TW_LOG(TW_ERROR, "twApi_BindThing: NULL tw_api or tw_api member");
		return TW_NULL_OR_INVALID_API_SINGLETON;
	}
	if (!entityNames) {
		TW_LOG(TW_ERROR, "twApi_BindThing: NULL entityNames list");
		return TW_INVALID_PARAM;
	}
	twMutex_Lock(tw_api->mtx);

	params = twBindThingsForeachParameters_Create(NULL, omitMetadataCallback);
	if (NULL == params) {
		TW_LOG(TW_ERROR, "Unable to allocate memory for twBindThingsForeachParameters");
		twMutex_Unlock(tw_api->mtx);
		return TW_ERROR_ALLOCATING_MEMORY;
	}

	twList_Foreach(entityNames, twBindThingsForeachHandler, (void *) params);
	if(params->duplicateDetected){
		twBindThingsForeachParameters_Delete(params);
		twMutex_Unlock(tw_api->mtx);
		return TW_ERROR_ITEM_EXISTS;
	}

	/* If we are not connected, we are done */
	if (s_twApi_isConnected()) {
		resp = s_sendMessageBlocking(&params->msg, twcfg.default_message_timeout, NULL);
	} else {
		/*
		if we are not connected, the entities will be bound to the api,
		but they will not be bound to the server until twApi_BindAll() is called
		which will happen automatically after a successful connection
		*/
		TW_LOG(TW_DEBUG, "twApi_BindThings: not currently connected, only binding things to api");
		/* this is not an error, so setting res to TW_OK */
		res = TW_OK;
	}

	twBindThingsForeachParameters_Delete(params);

	twMutex_Unlock(tw_api->mtx);
	/* Look for any callbacks */
	if (resp == TWX_SUCCESS) {
		s_makeAuthOrBindCallbacks(NULL, TW_THING, 1, NULL);
		res = convertMsgCodeToErrorCode(resp);
	}
	if (res != TW_OK) TW_LOG(TW_ERROR, "twApi_BindThing: Error sending Bind message");

	return res;
}

int twApi_UnbindThing(char * entityName) {
	int res = TW_UNKNOWN_ERROR;
	twMessage * msg = NULL;
	enum msgCodeEnum resp;
	void* result;
	bindListEntry* blQuery;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (!tw_api || !entityName || !tw_api->boundList) {
		TW_LOG(TW_ERROR, "twApi_UnbindThing: NULL tw_api or entityName");
		return TW_INVALID_PARAM;
	}
	/* Unregister all call backs for this entity */
	twApi_UnregisterThing(entityName);

	/* Remove it from the Bind list */
	blQuery = (bindListEntry*)TW_MALLOC(sizeof(bindListEntry));
	blQuery->name = entityName;
	if(TW_OK == twDict_Find(tw_api->boundList,blQuery,&result)){
		twDict_Remove(tw_api->boundList, result, TRUE);
	}
	TW_FREE(blQuery);

	/* Create the bind message */
	msg = twMessage_CreateBindMsg(entityName, TRUE);
	if (!msg) {
		TW_LOG(TW_ERROR, "twApi_UnbindThing: Error creating Unbind message");
		return TW_ERROR_CREATING_MSG;
	}
	twMutex_Lock(tw_api->mtx);
	resp = s_sendMessageBlocking(&msg, 10000, NULL);
	twMutex_Unlock(tw_api->mtx);
	res = convertMsgCodeToErrorCode(resp);
	if (res != TW_OK) TW_LOG(TW_ERROR, "twApi_UnbindThing: Error creating sending Unbind message");
	if (msg) twMessage_Delete(msg);
	/* Look for any callbacks */
	if (!res) makeAuthOrBindCallbacks(entityName, TW_THING, 2, NULL);
	return res;
}

char twApi_IsEntityBound(char * entityName) {
	void* result;
	bindListEntry* blQuery;

	if (!tw_api || !entityName || !tw_api->boundList) {
		TW_LOG(TW_ERROR, "twApi_IsEntityBound: NULL tw_api or entityName");
		return FALSE;
	}

	blQuery = (bindListEntry*)TW_MALLOC(sizeof(bindListEntry));
	blQuery->name = entityName;

	if(TW_OK == twDict_Find(tw_api->boundList,blQuery,&result)){
		TW_FREE(blQuery);
		return TRUE;
	}
	TW_FREE(blQuery);
	return FALSE;
}

void twApi_TaskerFunction(DATETIME now, void * params) {
	static DATETIME nextPingTime;
	static DATETIME expectedPongTime;
	static DATETIME nextCleanupTime;
	static DATETIME nextUpdatePropertyCheckTime;
	static DATETIME nextDutyCycleEvent;
	static char madeAuthCallbacks;
	int err = TW_OK;
	if (NULL==tw_api)
		return;
	/* This is the main "loop" of the api */
	if (tw_api && tw_api->mh && tw_api->mh->ws) {

		/* check if we are connected */
		if (twApi_isConnected()) {
			if (twTimeGreaterThan(now, nextPingTime)) {
				/* Time to send our keep alive ping */
				expectedPongTime = twAddMilliseconds(now, twcfg.pong_timeout);
				receivedPong = FALSE;
				err = twWs_SendPing(tw_api->mh->ws,0);
				if(TW_OK != err) {
					/* if the ping fails to send log an error and return so that the api can re-evaluate the conneciton status */
					TW_LOG(TW_ERROR, "twApi_TaskerFunction: failed to send ping, code: %d", err);
					/* Force a ping next time we connect */
					nextPingTime = 0;
					return;
				}
				nextPingTime = twAddMilliseconds(now, tw_api->ping_rate);
			}
			if (tw_api->handle_pongs && !receivedPong && twTimeGreaterThan(now, expectedPongTime)) {
				/* We didn't receive a pong in time */
				TW_LOG(TW_WARN,"apiThread: Did not receive pong in time");
				tw_api->manuallyDisconnected = FALSE;
				if (tw_api->isAuthenticated) TW_LOG(TW_TRACE, "apiThread: setting isAuthenticated from TRUE to FALSE (pong timeout)");
				tw_api->isAuthenticated = FALSE;
				/* This call will likely fail since we are already not connected, but do it just in case */
				twWs_Disconnect(tw_api->mh->ws, NORMAL_CLOSE, "Pong timeout");
				/* Force a ping next time we connect */
				nextPingTime = 0;
			}
			subscribedPropertyUpdateTask(now, NULL);
		}

		/* cleanup old messages */
		if (twTimeGreaterThan(now, nextCleanupTime)) {
			twMessageHandler_CleanupOldMessages(tw_api->mh);
			twMultipartMessageStore_RemoveStaleMessages();
			nextCleanupTime = twAddMilliseconds(now, twcfg.stale_msg_cleanup_rate);
		}

		/* cleanup old file transfers */
#ifdef ENABLE_FILE_XFER
		if (twFileManager_IsEnabled()) {
			twFileManager_CheckStalledTransfers();
		}
#endif

		/* check for property updates */
		if (twTimeGreaterThan(now, nextUpdatePropertyCheckTime)) {
			subscribedPropertyUpdateTask(now, NULL);
			nextUpdatePropertyCheckTime = twAddMilliseconds(now, 500);
		}

		/* check duty cycle status*/
		if (tw_api->firstConnectionComplete && tw_api->duty_cycle_period && tw_api->duty_cycle < 100 && twTimeGreaterThan(now, nextDutyCycleEvent)) {
			if (twApi_isConnected()) {
				TW_LOG(TW_INFO,"apiThread: Entering Duty Cycle OFF state.");
				twApi_Disconnect("Duty cycle off time");
				nextDutyCycleEvent = twAddMilliseconds(now, (tw_api->duty_cycle_period * (100 - tw_api->duty_cycle))/100);
			}  else {
				TW_LOG(TW_INFO,"apiThread: Entering Duty Cycle ON state.");
				twApi_Connect(tw_api->connect_timeout, tw_api->connect_retries);
				nextDutyCycleEvent = twAddMilliseconds(now, (tw_api->duty_cycle_period * tw_api->duty_cycle)/100);
			}
		}

		/* check connection and auth*/
		if (tw_api && tw_api->mh && tw_api->mh->ws && !tw_api->mh->ws->isConnected) {
			if (tw_api->isAuthenticated) TW_LOG(TW_TRACE, "apiThread: setting isAuthenticated from TRUE to FALSE (disconnect detected)");
			tw_api->isAuthenticated = FALSE;
			madeAuthCallbacks = FALSE;
		}

		/* check auth callbacks */
		if (tw_api->isAuthenticated && !madeAuthCallbacks) {
			char* api_key = twConvertCallbackToPasswd(tw_api->mh->ws->api_key_callback);
			makeAuthOrBindCallbacks("appKey", TW_APPLICATIONKEYS, 0, api_key);
			twFreePasswd(api_key);
			madeAuthCallbacks = TRUE;
		}

		/* check websocket status */
		if (!twWs_IsConnected(tw_api->mh->ws) && tw_api && tw_api->autoreconnect && tw_api->manuallyDisconnected == FALSE) {
			if (!tw_api->connectionInProgress) {
                TW_LOG(TW_TRACE, "apiThread: autoreconnecting");
                TW_LOG(TW_TRACE, "apiThread: isAuthenticated: %d", tw_api->isAuthenticated);
            }
		    tw_api->isAuthenticated = FALSE;
			TW_LOG(TW_TRACE, "apiThread: set isAuthenticated: %d before reconnect", tw_api->isAuthenticated);
			twApi_Connect(tw_api->connect_timeout, tw_api->connect_retries);
            TW_LOG(TW_TRACE, "apiThread: isConnected: %d isAuthenticated: %d isConnected(): %d", tw_api->mh->ws->isConnected, tw_api->isAuthenticated, twApi_isConnected());
		}
	}

	/* read available messages*/
	if (tw_api && twWs_IsConnected(tw_api->mh->ws) && tw_api->manuallyDisconnected == FALSE) {
		twWs_Receive(tw_api->mh->ws, twcfg.socket_read_timeout);
	}
}

int twApi_RegisterProperty(enum entityTypeEnum entityType, const char * entityName, const char * propertyName, enum BaseType propertyType,
						   const char * propertyDescription, const char * propertyPushType, double propertyPushThreshold, property_cb cb, void * userdata) {
	callbackInfo * info = NULL;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->callbackList && entityName && propertyName && cb) {
		twPropertyDef * property = twPropertyDef_Create(propertyName, propertyType, propertyDescription, propertyPushType, propertyPushThreshold);
		info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
		if (!info || !property) {
			TW_LOG(TW_ERROR, "twApi_RegisterProperty: Error allocating callback info");
			twPropertyDef_Delete(property);
			if (info) TW_FREE(info);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		info->entityType = entityType;
		info->entityName = duplicateString(entityName);
		info->characteristicType = TW_PROPERTIES;
		info->characteristicName = duplicateString(property->name);
		info->characteristicDefinition = property;
		info->cb = cb;
		info->userdata = userdata;
		return (twDict_Add(tw_api->callbackList, info));
	}
	TW_LOG(TW_ERROR, "twApi_RegisterProperty: Invalid params or missing api pointer");
	return TW_INVALID_PARAM;
}

int twApi_UpdatePropertyMetaData(enum entityTypeEnum entityType, char * entityName, char * propertyName, enum BaseType propertyType,
									char * propertyDescription, char * propertyPushType, double propertyPushThreshold) {
	void * result = NULL;
	callbackInfo *cbInfoQuery;
	callbackInfo * info = NULL;
	twPropertyDef * tmp = NULL;
	if (!tw_api)
		return TW_NULL_OR_INVALID_API_SINGLETON;

	/* Build Search Query */
	cbInfoQuery = (callbackInfo *) TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = TW_PROPERTIES;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->characteristicName = propertyName;

	if(TW_OK!=twDict_Find(tw_api->callbackList,cbInfoQuery,&result)){
		TW_FREE(cbInfoQuery);
		return TW_INVALID_PARAM;
	}
	TW_FREE(cbInfoQuery);

	info = (callbackInfo *)result;
	tmp = twPropertyDef_Create(propertyName,
								(propertyType == TW_UNKNOWN_TYPE) ? ((twPropertyDef *)info->characteristicDefinition)->type : propertyType,
								propertyDescription ? propertyDescription : ((twPropertyDef *)info->characteristicDefinition)->description,
								propertyPushType,
								propertyPushThreshold);
	if (!tmp) {
		TW_LOG(TW_ERROR, "twApi_UpdatePropertyMetaData: Error creating property definition");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	twPropertyDef_Delete(info->characteristicDefinition);
	info->characteristicDefinition = tmp;
	return TW_OK;
}

int twApi_AddAspectToProperty(const char * entityName, const char * propertyName,
							  const char * aspectName, twPrimitive * aspectValue) {
	return AddAspectToEntity(entityName, TW_PROPERTIES, propertyName, aspectName, aspectValue);
}

int twApi_RegisterService(enum entityTypeEnum entityType, const char * entityName, char * serviceName, char * serviceDescription,
						  twDataShape * inputs, enum BaseType outputType, twDataShape * outputDataShape, service_cb cb, void * userdata) {

	return registerServiceOrEvent(entityType, entityName, serviceName, serviceDescription, inputs, outputType, outputDataShape, cb, userdata, TRUE);
}

int twApi_AddAspectToService(char * entityName, char * serviceName,
							  char * aspectName, twPrimitive * aspectValue) {
	return AddAspectToEntity(entityName, TW_SERVICES, serviceName, aspectName, aspectValue);
}

int twApi_RegisterEvent(enum entityTypeEnum entityType, char * entityName, char * eventName, char * eventDescription, twDataShape * parameters) {

	return registerServiceOrEvent(entityType, entityName, eventName, eventDescription, parameters, TW_NOTHING, NULL, NULL, NULL, FALSE);
}

int twApi_AddAspectToEvent(char * entityName, char * eventName,
							  char * aspectName, twPrimitive * aspectValue) {
	return AddAspectToEntity(entityName, TW_EVENTS, eventName, aspectName, aspectValue);
}

int twApi_RegisterPropertyCallback(enum entityTypeEnum entityType, char * entityName, char * propertyName, property_cb cb, void * userdata) {
	callbackInfo * info = NULL;
	void * result;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->callbackList && entityName && propertyName && cb) {
		info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
		if (!info) {
			TW_LOG(TW_ERROR, "twApi_RegisterPropertyCallback: Error allocating callback info");
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		info->entityType = entityType;
		info->entityName = duplicateString(entityName);
		info->characteristicType = TW_PROPERTIES;
		info->characteristicName = duplicateString(propertyName);
		info->characteristicDefinition = NULL;
		info->cb = cb;
		info->userdata = userdata;

		/* Make sure this callback does not already exist */
		if(TW_OK == twDict_Find(tw_api->callbackList,info,&result)){
			TW_FREE(info->entityName);
			TW_FREE(info->characteristicName);
			TW_FREE(info);
			return TW_ERROR_ITEM_EXISTS;
		}

		return (twDict_Add(tw_api->callbackList, info));
	}
	TW_LOG(TW_ERROR, "twApi_RegisterPropertyCallback: Invalid params or missing api pointer");
	return TW_INVALID_PARAM;
}

int twApi_RegisterServiceCallback(enum entityTypeEnum entityType, char * entityName, char * serviceName, service_cb cb, void * userdata) {
	callbackInfo * info = NULL;
	void* result;
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->callbackList && entityName && serviceName && cb) {
		info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
		if (!info) {
			TW_LOG(TW_ERROR, "twApi_RegisterServiceCallback: Error allocating callback info");
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		info->entityType = entityType;
		info->entityName = duplicateString(entityName);
		info->characteristicType = TW_SERVICES;
		info->characteristicName = duplicateString(serviceName);
		info->characteristicDefinition = NULL;
		info->cb = cb;
		info->userdata = userdata;

		/* Make sure this callback is not already registered */
		if(TW_OK == twDict_Find(tw_api->callbackList,info,&result)){
			/* This exact callback has already been registered */
			TW_FREE(info->entityName);
			TW_FREE(info->characteristicName);
			TW_FREE(info);
			return TW_ERROR_ITEM_EXISTS;
		}
		return twDict_Add(tw_api->callbackList, info);
	}
	TW_LOG(TW_ERROR, "twApi_RegisterServiceCallback: Invalid params or missing api pointer");
	return TW_INVALID_PARAM;
}

typedef struct twUnregisterThingParams {
	char * entityName;
	twList * deletionList;
} twUnregisterThingParams;

void twGenericDoNothingDeletionHandler(void *ptr){
	/* Use this deletion handler when maintaining a list of things that you do not own */
}

int twUnregisterThingForEachHandler(void *key, size_t key_size, void *data, size_t data_size,void *arg){
	twUnregisterThingParams* params = (twUnregisterThingParams*)arg;
	callbackInfo *cbInfo = (callbackInfo *) (data);
	if (cbInfo->entityName && strcmp(params->entityName, cbInfo->entityName)) {
		return TW_FOREACH_CONTINUE;
	}
	/* Delete this entry */
	twList_Add(params->deletionList,cbInfo);
	return TW_FOREACH_CONTINUE;
}

int twUnregisterThingRemoveForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twDict_Remove(tw_api->callbackList, currentValue, TRUE);
	return TW_FOREACH_CONTINUE;
}

int twApi_UnregisterThing(char * entityName) {

	if (!tw_api)
		return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->callbackList && entityName) {
		twUnregisterThingParams* params = (twUnregisterThingParams*)TW_MALLOC(sizeof(twUnregisterThingParams));
		params->entityName = entityName;
		params->deletionList = twList_Create(TW_DONT_DELETE_LIST_VALUE);

		/* Create a list of callbacks to delete. */
		twDict_Foreach(tw_api->callbackList,twUnregisterThingForEachHandler,(void*)params);

		/* Delete the items in this list */
		twList_Foreach(params->deletionList, twUnregisterThingRemoveForEachHandler, NULL);

		/* list should be cleared, delete list */
		twList_Delete(params->deletionList);

		/* Remove all pending changes for this thing in the subscribed property manager */
		twSubscribedPropsMgr_PurgeChangesForThing(entityName);
		twSubscribedPropsMgr_PurgeCurrentValuesForThing(entityName);

		/* free params */
		TW_FREE(params);

		return TW_OK;
	}
	TW_LOG(TW_ERROR, "twApi_UnregisterThing: Invalid params or missing api pointer");
	return TW_INVALID_PARAM;
}

int twApi_UnregisterCallback(char * entityName, enum characteristicEnum type, char * characteristicName, void * userdata) {
	void* result;
	callbackInfo *cbInfoQuery;

	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (!(tw_api && tw_api->callbackList && entityName && characteristicName)) {
		TW_LOG(TW_ERROR, "twApi_UnregisterCallback: Invalid params or missing api pointer");
		return TW_INVALID_PARAM;
	}

	cbInfoQuery = (callbackInfo *) TW_MALLOC(sizeof(callbackInfo));
	cbInfoQuery->characteristicType = type;
	cbInfoQuery->entityName = entityName;
	cbInfoQuery->entityType = TW_THING;
	cbInfoQuery->characteristicName = characteristicName;

	if(TW_OK==twDict_Find(tw_api->callbackList,cbInfoQuery,&result)){
		twDict_Remove(tw_api->callbackList,result,TRUE);
		if(TW_PROPERTIES==type) {
			twSubscribedPropsMgr_PurgeChangesForProperty(entityName, characteristicName);
			twSubscribedPropsMgr_PurgeCurrentValueForProperty(entityName, characteristicName);
		}
		TW_FREE(cbInfoQuery);
		return TW_OK;
	}
	TW_FREE(cbInfoQuery);

	return TW_NOT_FOUND;
}

int twApi_UnregisterPropertyCallback(char * entityName, char * propertyName, void * cb) {
	return twApi_UnregisterCallback(entityName, TW_PROPERTIES, propertyName, cb);
}

int twApi_UnregisterServiceCallback(char * entityName, char * serviceName, void * userdata) {
	return twApi_UnregisterCallback(entityName, TW_SERVICES, serviceName, userdata);
}

int twApi_RegisterDefaultRequestHandler(genericRequest_cb cb) {
	if (tw_api) {
		tw_api->defaultRequestHandler = cb;
		return TW_OK;
	}
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

propertyList * twApi_CreatePropertyList(char * name, twPrimitive * value, DATETIME timestamp) {
	propertyList * proplist = twList_Create(twProperty_Delete);
	if (!proplist) {
		TW_LOG(TW_ERROR,"twApi_CreatePropertyList: Error allocating property list");
		return NULL;
	}
	if (twList_Add(proplist, twProperty_Create(name, value, timestamp))) {
		TW_LOG(TW_ERROR,"twApi_CreatePropertyList: Error adding initial property  to list");
		twList_Delete(proplist);
		return NULL;
	}
	return proplist;
}

int twApi_DeletePropertyList(propertyList * list) {
	return twList_Delete(list);
}

int twApi_AddPropertyToList(propertyList * proplist, char * name, twPrimitive * value, DATETIME timestamp) {
	return twList_Add(proplist, twProperty_Create(name, value, timestamp));
}

int twApi_ReadProperty(enum entityTypeEnum entityType, char * entityName, char * propertyName, twPrimitive ** result, int32_t timeout, char forceConnect) {
	enum msgCodeEnum res = makePropertyRequest(TWX_GET, entityType, entityName, propertyName, 0, result, timeout, forceConnect);
	return convertMsgCodeToErrorCode(res);
}

int twApi_WriteProperty(enum entityTypeEnum entityType, char * entityName, char * propertyName, twPrimitive * value, int32_t timeout, char forceConnect) {
	twPrimitive * result = NULL;
	enum msgCodeEnum res;
	res = makePropertyRequest(TWX_PUT, entityType, entityName, propertyName, value, &result, timeout, forceConnect);
	if (result) twPrimitive_Delete(result);
	return convertMsgCodeToErrorCode(res);
}

int twApi_SetSubscribedPropertyVTQ(char * entityName, char * propertyName, twPrimitive * value,  DATETIME timestamp, char * quality, char fold, char pushUpdate) {
	return twSubscribedPropsMgr_SetPropertyVTQ(entityName, propertyName, value, timestamp, quality, fold, pushUpdate);
}

int twApi_SetSubscribedProperty(char * entityName, char * propertyName, twPrimitive * value, char fold, char pushUpdate) {
	return twSubscribedPropsMgr_SetPropertyVTQ(entityName, propertyName, value, twGetSystemTime(TRUE), "GOOD", fold, pushUpdate);
}

int twApi_PushSubscribedProperties(char * entityName, char forceConnect) {
	int res = TW_UNKNOWN_ERROR;
	res = twSubscribedPropsMgr_PushSubscribedProperties(entityName, forceConnect, TRUE);
	if (res == TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED) {
		TW_LOG(TW_WARN,"twApi_PushSubscribedProperties: saved properties in subscribed property manager for: %s", entityName);
		res = TW_OK;
	}
	return res;
}

int twApi_PushSubscribedPropertiesAsync(char * entityName, char forceConnect,response_cb cb,twList** messageListRef) {
	int res = TW_UNKNOWN_ERROR;
	res = twSubscribedPropsMgr_PushSubscribedPropertiesAsync(entityName, forceConnect, cb, messageListRef);
	if (res == TW_SUBSCRIBED_PROPERTY_LIST_PERSISTED) {
		TW_LOG(TW_WARN,"twApi_PushSubscribedPropertiesAsync: saved peoperties in subscribed property manager for: %s", entityName);
		res = TW_OK;
	}
	return res;
}


int twPushPropertiesForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twProperty *prop = (twProperty *) currentValue;
	twInfoTable * it = (twInfoTable *)userData;
	twInfoTableRow *row = NULL;
	row = twInfoTableRow_Create(twPrimitive_CreateFromString(prop->name, TRUE));
	if (!row) {
		TW_LOG(TW_ERROR, "twApi_PushProperties: Error creating infotable row");
		return TW_FOREACH_EXIT;
	}
	twInfoTableRow_AddEntry(row, twPrimitive_CreateVariant(twPrimitive_ZeroCopy(prop->value)));
	twInfoTableRow_AddEntry(row, twPrimitive_CreateFromDatetime(prop->timestamp));
	if(NULL==prop->quality)
		twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString("GOOD", TRUE));
	else
		twInfoTableRow_AddEntry(row, twPrimitive_CreateFromString(prop->quality, TRUE));
	twInfoTable_AddRow(it, row);
	return TW_FOREACH_CONTINUE;
}

int twApi_PushProperties(enum entityTypeEnum entityType, char * entityName, propertyList * properties, int32_t timeout, char forceConnect) {
	twDataShape* ds = NULL;
	twInfoTable * it = NULL;
	twInfoTable * values = NULL;
	twInfoTable * result = NULL;
	int res = TW_UNKNOWN_ERROR;
	/* Validate the inputs */
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (!entityName || !properties) {
		TW_LOG(TW_ERROR,"twApi_PushProperties: Missing inputs");
		return TWX_BAD_REQUEST;
	}
	/* Create the data shape */
	ds = twDataShape_Create(twDataShapeEntry_Create("name", NULL, TW_STRING));
	if (!ds) {
		TW_LOG(TW_ERROR,"twApi_PushProperties: Error allocating data shape");
		return TWX_INTERNAL_SERVER_ERROR;
	}
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("value", NULL, TW_VARIANT));
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("time", NULL, TW_DATETIME));
	twDataShape_AddEntry(ds,twDataShapeEntry_Create("quality", NULL, TW_STRING));
	/* Create the infotable */
	it = twInfoTable_Create(ds);
	if (!it) {
		TW_LOG(TW_ERROR,"twApi_PushProperties: Error creating infotable");
		twDataShape_Delete(ds);
		return TWX_INTERNAL_SERVER_ERROR;
	}

	/* Loop through the list and create a row per entry */
	twList_Foreach(properties, twPushPropertiesForeachHandler, (void *) it);

	/* Make the service request */
	values = twInfoTable_Create(twDataShape_Create(twDataShapeEntry_Create("values", NULL, TW_INFOTABLE)));
	twInfoTable_AddRow(values, twInfoTableRow_Create(twPrimitive_CreateFromInfoTable(it)));
	res = s_twApi_InvokeService(entityType, entityName, "UpdateSubscribedPropertyValues", values, &result, timeout, forceConnect);
	twInfoTable_Delete(it);
	twInfoTable_Delete(values);
	twInfoTable_Delete(result);
	return res;
}

int twApi_InvokeService(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect) {
	enum msgCodeEnum res = makeRequest(TWX_POST, entityType, entityName, TW_SERVICES, serviceName, params, result, timeout, forceConnect);
	return convertMsgCodeToErrorCode(res);
}

int twApi_InvokeServiceAsync(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, char forceConnect, response_cb cb, uint32_t* messageId) {
	enum msgCodeEnum res = makeAsyncRequest(TWX_POST, entityType, entityName, TW_SERVICES, serviceName, params, forceConnect,cb,messageId);
	return convertMsgCodeToErrorCode(res);
}

int twApi_FireEvent(enum entityTypeEnum entityType, char * entityName, char * eventName, twInfoTable * params, int32_t timeout, char forceConnect) {
	twInfoTable * result = NULL;
	enum msgCodeEnum res;
	res = makeRequest(TWX_POST, entityType, entityName, TW_EVENTS, eventName, params, &result, timeout, forceConnect);
	twInfoTable_Delete(result);
	return convertMsgCodeToErrorCode(res);
}

int twApi_RegisterConnectCallback(eventcb cb) {
	if (tw_api && tw_api->mh) return twMessageHandler_RegisterConnectCallback(tw_api->mh, cb);
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_RegisterCloseCallback(eventcb cb) {
	if (tw_api && tw_api->mh) return twMessageHandler_RegisterCloseCallback(tw_api->mh, cb);
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_RegisterPingCallback(eventcb cb) {
	if (tw_api && tw_api->mh) return twMessageHandler_RegisterPingCallback(tw_api->mh, cb);
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_RegisterPongCallback(eventcb cb) {
	if (tw_api && tw_api->mh){
		/* We are no longer handling pongs in the api */
		tw_api->handle_pongs = FALSE;
		return twMessageHandler_RegisterPongCallback(tw_api->mh, cb);
	}
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_RegisterBindEventCallback(char * entityName, bindEvent_cb cb, void * userdata) {
	int err = TW_OK;
	callbackInfo * info = NULL;
	if (!tw_api || !tw_api->bindEventCallbackList || !cb) return TW_NULL_OR_INVALID_API_SINGLETON;
	info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
	if (!info) return TW_ERROR_ALLOCATING_MEMORY;
	if (entityName) info->entityName = duplicateString(entityName);
	info->cb = cb;
	info->userdata = userdata;
	err = twList_Add(tw_api->bindEventCallbackList, info);
	if (err) {
		/* might need to delete info since the list will not be managing the memory */
		if (info) deleteCallbackInfo(info);
	}
	return err;
}


typedef struct twBindEventCallbackSearchParam {
	char * entityName;
	bindEvent_cb cb;
	void * userdata;
	void* found;
} twBindEventCallbackSearchParam;


int twUnregisterBindEventCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	callbackInfo *info;
	twBindEventCallbackSearchParam* searchParams = (twBindEventCallbackSearchParam*)userData;
	info = (callbackInfo *) currentValue;
	if (
			info->cb == searchParams->cb &&
			info->userdata == searchParams->userdata &&
			!strcmp(info->entityName, searchParams->entityName)
	) {
		searchParams->found = key;
		return TW_FOREACH_EXIT;
	}
	return TW_FOREACH_CONTINUE;
}

int twApi_UnregisterBindEventCallback(char * entityName, bindEvent_cb cb, void * userdata) {
	int err = TW_NOT_FOUND;
	twBindEventCallbackSearchParam* searchParams;
	if (!tw_api || !tw_api->bindEventCallbackList) return TW_NULL_OR_INVALID_API_SINGLETON;

	searchParams = (twBindEventCallbackSearchParam*)TW_MALLOC(sizeof(twBindEventCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->entityName = entityName;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->bindEventCallbackList,twUnregisterBindEventCallbackForeachHandler,searchParams);
	if(searchParams->found){
		twList_Remove(tw_api->bindEventCallbackList, (ListEntry*)searchParams->found, TRUE);
				err = TW_OK;
	}

	TW_FREE(searchParams);

	return err;
}

int twApi_RegisterSynchronizeStateEventCallback(char * entityName, synchronizeEvent_cb cb, void * userdata) {
	int err = TW_OK;
	callbackInfo * info = NULL;
	if (!tw_api || !tw_api->synchronizeStateEventCallbackList || !cb) return TW_NULL_OR_INVALID_API_SINGLETON;
	info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
	if (!info) return TW_ERROR_ALLOCATING_MEMORY;
	if (entityName) info->entityName = duplicateString(entityName);
	info->cb = cb;
	info->userdata = userdata;
	err = twList_Add(tw_api->synchronizeStateEventCallbackList, info);
	if (err) {
		/* might need to delete info since the list will not be managing the memory */
		if (info) deleteCallbackInfo(info);
	}
	return err;
}

typedef struct twSynchronizeStateEventCallbackSearchParam {
	char * entityName;
	synchronizeEvent_cb cb;
	void * userdata;
	void* found;
} twSynchronizeStateEventCallbackSearchParam;


int twUnregisterSynchronizeStateEventCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	callbackInfo *info;
	twSynchronizeStateEventCallbackSearchParam* searchParams = (twSynchronizeStateEventCallbackSearchParam*)userData;
	info = (callbackInfo *) currentValue;
	if (
			info->cb == searchParams->cb &&
			info->userdata == searchParams->userdata && info->entityName &&
			!strncmp(info->entityName, searchParams->entityName,strnlen(searchParams->entityName, MAX_ENTITY_NAME_LEN))
			) {
		searchParams->found = key;
		return TW_FOREACH_EXIT;
	}
	if(			info->cb == searchParams->cb &&
				   info->userdata == searchParams->userdata && NULL==info->entityName && NULL == searchParams->entityName
	) {
		searchParams->found = key;
		return TW_FOREACH_EXIT;
	}
	return TW_FOREACH_CONTINUE;
}

int twApi_UnregisterSynchronizeStateEventCallback(char * entityName, synchronizeEvent_cb cb, void * userdata) {
	int err = TW_NOT_FOUND;
	twSynchronizeStateEventCallbackSearchParam* searchParams;
	if (!tw_api || !tw_api->synchronizeStateEventCallbackList) return TW_NULL_OR_INVALID_API_SINGLETON;

	searchParams = (twSynchronizeStateEventCallbackSearchParam*)TW_MALLOC(sizeof(twSynchronizeStateEventCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->entityName = entityName;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->synchronizeStateEventCallbackList,twUnregisterSynchronizeStateEventCallbackForeachHandler,searchParams);
	if(searchParams->found){
		twList_Remove(tw_api->synchronizeStateEventCallbackList, (ListEntry*)searchParams->found, TRUE);
		err = TW_OK;
	}

	TW_FREE(searchParams);

	return err;
}

int twApi_RegisterOnAuthenticatedCallback(authEvent_cb cb, void * userdata) {
	int err = TW_OK;
	callbackInfo * info = NULL;
	if (!tw_api || !tw_api->bindEventCallbackList || !cb) return TW_NULL_OR_INVALID_API_SINGLETON;
	info = (callbackInfo *)TW_CALLOC(sizeof(callbackInfo), 1);
	if (!info) return TW_ERROR_ALLOCATING_MEMORY;
	/* Just using TW_APPLICATIONKEYS as a non-zero flag - no special significance */
	info->entityType = TW_APPLICATIONKEYS;
	info->cb = cb;
	info->userdata = userdata;
	err = twList_Add(tw_api->bindEventCallbackList, info);
	if (err) {
		/* need to cleanup callback info since it will not be managed by the list */
		if (info) deleteCallbackInfo(info);
	}
	return err;
}

int twUnregisterOnAuthenticatedCallbackForeachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size,void *userData){
	twBindEventCallbackSearchParam* searchParams = (twBindEventCallbackSearchParam*)userData;
	callbackInfo * info = (callbackInfo *)currentValue;
	if (info->cb == searchParams->cb && info->userdata == searchParams->userdata && info->entityType == TW_APPLICATIONKEYS) {
		searchParams->found=key;
		return TW_FOREACH_EXIT;
	}
	return TW_FOREACH_CONTINUE;
}

int twApi_UnregisterOnAuthenticatedCallback(authEvent_cb cb, void * userdata) {
	int err = TW_NOT_FOUND;
	twBindEventCallbackSearchParam* searchParams;
	if (!tw_api || !tw_api->bindEventCallbackList)
		return TW_NULL_OR_INVALID_API_SINGLETON;
	searchParams = (twBindEventCallbackSearchParam*)TW_MALLOC(sizeof(twBindEventCallbackSearchParam));
	searchParams->cb = cb;
	searchParams->userdata = userdata;
	searchParams->found = NULL;
	twList_Foreach(tw_api->bindEventCallbackList,twUnregisterOnAuthenticatedCallbackForeachHandler,searchParams);
	if(searchParams->found) {
		twList_Remove(tw_api->bindEventCallbackList, searchParams->found, TRUE);
		err = TW_OK;
	}
	TW_FREE(searchParams);
	return err;
}

int twApi_RegisterInitCallback(init_cb_t cb, void * userdata) {
    if (!twcfg.initCallback) {
        twcfg.initCallback = TW_CALLOC(sizeof(init_cb), 1);
    }

    twcfg.initCallback->cb = cb;
    twcfg.initCallback->userdata = userdata;
    return TW_OK;
}

int twApi_CleanupOldMessages() {
	if (tw_api && tw_api->mh) return twMessageHandler_CleanupOldMessages(tw_api->mh);
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_SendPing(char * content) {
	if (tw_api && tw_api->mh && tw_api->mh->ws) return s_twWs_SendPing(tw_api->mh->ws, content);
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

int twApi_CreateTask(uint32_t runTintervalMsec, twTaskFunction func) {
	return twTasker_CreateTask(runTintervalMsec, func);
}

void twApi_SetSelfSignedOk() {
    if (tw_api) {
        if (tw_api->mh && tw_api->mh->ws) {
            twTlsClient_SetSelfSignedOk(tw_api->mh->ws->connection);
        }

        if (tw_api->connectionInfo) {
            tw_api->connectionInfo->selfsignedOk = TRUE;
        }
    }
}

void twApi_DisableCertValidation() {
    if (tw_api) {
        if (tw_api->mh && tw_api->mh->ws) {
            twTlsClient_DisableCertValidation(tw_api->mh->ws->connection);
        }

        if (tw_api->connectionInfo) {
            tw_api->connectionInfo->doNotValidateCert = TRUE;
        }
    }
}

int	twApi_LoadCACert(const char *file, int type) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		if (!file) return TW_INVALID_PARAM;
		if (tw_api->connectionInfo) {
			if (tw_api->connectionInfo->ca_cert_file) {
				TW_FREE(tw_api->connectionInfo->ca_cert_file);
				tw_api->connectionInfo->ca_cert_file = NULL;
			}
			tw_api->connectionInfo->ca_cert_file = duplicateString(file);
		}
		return s_twTlsClient_SetClientCaList (tw_api->mh->ws->connection, (char *) file, NULL);
	}
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
}

int	twApi_LoadClientCert(char *file) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		if (!file) return TW_INVALID_PARAM;
		if (tw_api->connectionInfo) {
			if (tw_api->connectionInfo->client_cert_file) {
				TW_FREE(tw_api->connectionInfo->client_cert_file);
				tw_api->connectionInfo->client_cert_file = NULL;
			}
			tw_api->connectionInfo->client_cert_file = duplicateString(file);
		}
		return s_twTlsClient_UseCertificateChainFile(tw_api->mh->ws->connection, (char *) file, NULL);
	}
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
}

int	twApi_SetClientKey(const char *file, twPasswdCallbackFunction passphraseCallback, int type) {
	if (!tw_api) return TW_NULL_OR_INVALID_API_SINGLETON;
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		if (!passphraseCallback || !file) return TW_INVALID_PARAM;
		if (tw_api->connectionInfo) {
			if (tw_api->connectionInfo->client_key_file) {
				TW_FREE(tw_api->connectionInfo->client_key_file);
				tw_api->connectionInfo->client_key_file = NULL;
			}
			if (tw_api->connectionInfo->client_key_passphrase) {
				tw_api->connectionInfo->client_key_passphrase = NULL;
			}
			tw_api->connectionInfo->client_key_file = duplicateString(file);
			tw_api->connectionInfo->client_key_passphrase = passphraseCallback;
		}
		twTlsClient_SetDefaultPasswdCb(tw_api->mh->ws->connection, passphraseCallback);
		return s_twTlsClient_UsePrivateKeyFile(tw_api->mh->ws->connection, file, type);
	} else return TW_NULL_OR_INVALID_MSG_HANDLER;
}

int twApi_IsFIPSCompatible() {
    return TW_IS_FIPS_COMPATIBLE();
}

int twApi_EnableFipsMode()  {
    return TW_ENABLE_FIPS_MODE();
}

int twApi_DisableFipsMode() {
    return TW_DISABLE_FIPS_MODE();
}

int twApi_IsFipsModeEnabled() {
    return TW_IS_FIPS_MODE_ENABLED();
}


void twApi_DisableEncryption() {
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		if (tw_api->connectionInfo) tw_api->connectionInfo->disableEncryption = TRUE;
		twTlsClient_DisableEncryption(tw_api->mh->ws->connection);
	}
}

void twApi_DisableWebSocketCompression() {
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		tw_api->mh->ws->bDisableCompression = TRUE;
	}
}

int twApi_SetX509Fields(char * subject_cn, char * subject_o, char * subject_ou,
							  char * issuer_cn, char * issuer_o, char * issuer_ou) {
	if (tw_api && tw_api->mh && tw_api->mh->ws) {
		if (tw_api->connectionInfo) {
			if (tw_api->connectionInfo->subject_cn) {
				TW_FREE(tw_api->connectionInfo->subject_cn);
				tw_api->connectionInfo->subject_cn = NULL;
			}
			tw_api->connectionInfo->subject_cn = duplicateString(subject_cn);
			if (tw_api->connectionInfo->subject_o) {
				TW_FREE(tw_api->connectionInfo->subject_o);
				tw_api->connectionInfo->subject_o = NULL;
			}
			tw_api->connectionInfo->subject_o = duplicateString(subject_o);
			if (tw_api->connectionInfo->subject_ou) {
				TW_FREE(tw_api->connectionInfo->subject_ou);
				tw_api->connectionInfo->subject_ou = NULL;
			}
			tw_api->connectionInfo->subject_ou = duplicateString(subject_ou);
			if (tw_api->connectionInfo->issuer_cn) {
				TW_FREE(tw_api->connectionInfo->issuer_cn);
				tw_api->connectionInfo->issuer_cn = NULL;
			}
			tw_api->connectionInfo->issuer_cn = duplicateString(issuer_cn);
			if (tw_api->connectionInfo->issuer_o) {
				TW_FREE(tw_api->connectionInfo->issuer_o);
				tw_api->connectionInfo->issuer_o = NULL;
			}
			tw_api->connectionInfo->issuer_o = duplicateString(issuer_o);
			if (tw_api->connectionInfo->issuer_ou) {
				TW_FREE(tw_api->connectionInfo->issuer_ou);
				tw_api->connectionInfo->issuer_ou = NULL;
			}
			tw_api->connectionInfo->issuer_ou = duplicateString(issuer_ou);
		}
		return twTlsClient_SetX509Fields(tw_api->mh->ws->connection, subject_cn, subject_o, subject_ou, issuer_cn, issuer_o, issuer_ou);
	}
	return TW_NULL_OR_INVALID_API_SINGLETON;
}

twConnectionInfo * twApi_GetConnectionInfo() {
	return tw_api->connectionInfo;
}

/* backwards compatibility method */
int twApi_SetOfflineMsgStoreDir(const char *dir) {
	/* directory changes after offline message store initialization
	will not be realized until the thingworx api is restarted */
	int res = TW_OK;
	if (tw_offline_msg_store && tw_offline_msg_store->mtx) twMutex_Lock(tw_offline_msg_store->mtx);
	res = twOfflineMsgStore_SetDir(dir);
	if (tw_offline_msg_store && tw_offline_msg_store->mtx) twMutex_Unlock(tw_offline_msg_store->mtx);
	return res;
}

twApi * twApi_GetApi(void) {
	return tw_api;
}


int twApi_SetIsAuthenticated(char value) {
	if (!tw_api) {
		return TW_ERROR;
	}
	tw_api->isAuthenticated = value;
	return TW_OK;
}

char twApi_GetIsAuthenticated() {
	return tw_api->isAuthenticated;
}

int twApi_ClearSubscribedPropertyCurrentValues() {
	int res = TW_UNKNOWN_ERROR;
	twSubscribedPropsMgr *spm = twSubscribedPropsMgr_Get();
	if (!spm || !spm->currentValues) {
		return TW_SUBSCRIBEDPROP_MGR_NOT_INTIALIZED;
	}
	twMutex_Lock(spm->mtx);
	res = twDict_Clear(spm->currentValues);
	twMutex_Unlock(spm->mtx);
	return res;
}

/* Structure to allow overriding of defaults at runtime */
twConfig twcfg =  {
#ifdef ENABLE_TASKER
	TRUE,
#else
	FALSE,
#endif
#ifdef ENABLE_FILE_XFER
	TRUE,
#else
	FALSE,
#endif
#ifdef ENABLE_TUNNELING
	TRUE,
#else
	FALSE,
#endif
#if (OFFLINE_MSG_STORE==1) || (OFFLINE_MSG_STORE==2)
	TRUE,
#else
	FALSE,
#endif
	TRUE,
	TW_URI,
	MAX_MESSAGE_SIZE,
	MESSAGE_CHUNK_SIZE,
	MAX_WS_TUNNEL_MESSAGE_SIZE,
	DEFAULT_MESSAGE_TIMEOUT,
	PING_RATE,
	DEFAULT_PONG_TIMEOUT,
	STALE_MSG_CLEANUP_RATE,
	CONNECT_TIMEOUT,
	CONNECT_RETRIES,
	DUTY_CYCLE,
	DUTY_CYCLE_PERIOD,
	STREAM_BLOCK_SIZE,
	FILE_XFER_BLOCK_SIZE,
	FILE_XFER_MAX_FILE_SIZE,
	FILE_XFER_MD5_BLOCK_SIZE,
	FILE_XFER_TIMEOUT,
	FILE_XFER_STAGING_DIR,
	OFFLINE_MSG_QUEUE_SIZE,
	OFFLINE_MSG_QUEUE_SIZE,
	MAX_CONNECT_DELAY,
	CONNECT_RETRY_INTERVAL,
	MAX_MESSAGES,
	DEFAULT_SOCKET_READ_TIMEOUT,
	DEFAULT_SSL_READ_TIMEOUT,
	OFFLINE_MSG_STORE_DIR,
	OFFLINE_MSG_STORE_DIR,
	FRAME_READ_TIMEOUT,
	TRUE,
	NULL,
	NULL,
	MAX_STRING_PROP_LENGTH
};


#ifdef WIN32
__declspec(dllexport) twConfig * twcfg_pointer = &twcfg;
#endif

int twApi_CreateStubs() {/* Create Stub Table */
#ifdef TW_STUBS
	if(!twApi_stub) {
		twApi_stub = (twApi_Stubs*)TW_CALLOC(sizeof(twApi_Stubs), sizeof(char));
	}
	twApi_stub->cfuhash_destroy = cfuhash_destroy;
	twApi_stub->deleteCallbackInfo = deleteCallbackInfo;
	twApi_stub->bindListEntry_Create = bindListEntry_Create;
	twApi_stub->bindListEntry_Delete = bindListEntry_Delete;
	twApi_stub->notifyPropertyUpdateHandler = notifyPropertyUpdateHandler;
	twApi_stub->subscribedPropertyUpdateTask = subscribedPropertyUpdateTask;
	twApi_stub->isFileTransferService = isFileTransferService;
	twApi_stub->isTunnelService = isTunnelService;
	twApi_stub->convertMsgCodeToErrorCode = convertMsgCodeToErrorCode;
	twApi_stub->findRegisteredItem = findRegisteredItem;
	twApi_stub->getCallbackFromList = findCallback;
	twApi_stub->sendMessageBlocking = sendMessageBlocking;
	twApi_stub->makeRequest = makeRequest;
	twApi_stub->makePropertyRequest = makePropertyRequest;
	twApi_stub->twApi_SendResponse = twApi_SendResponse;
	twApi_stub->api_requesthandler = api_requesthandler;
	twApi_stub->getMetadataService = getMetadataService;
	twApi_stub->pong_handler = pong_handler;
	twApi_stub->makeAuthOrBindCallbacks = makeAuthOrBindCallbacks;
    twApi_stub->makeSynchronizedStateCallbacks = makeSynchronizedStateCallbacks;
	twApi_stub->registerServiceOrEvent = registerServiceOrEvent;
	twApi_stub->AddAspectToEntity = AddAspectToEntity;

	twApi_stub->twApi_Delete = twApi_Delete;
	twApi_stub->twApi_SetProxyInfo = twApi_SetProxyInfo;
	twApi_stub->twApi_GetVersion = twApi_GetVersion;
	twApi_stub->twApi_BindAll = twApi_BindAll;
	twApi_stub->twApi_Authenticate = twApi_Authenticate;
	twApi_stub->twApi_Connect = twApi_Connect;
	twApi_stub->twApi_Disconnect = twApi_Disconnect;
	twApi_stub->twApi_isConnected = twApi_isConnected;
	twApi_stub->twApi_ConnectionInProgress = twApi_ConnectionInProgress;
	twApi_stub->twApi_StopConnectionAttempt = twApi_StopConnectionAttempt;
	twApi_stub->twApi_SetDutyCycle = twApi_SetDutyCycle;
	twApi_stub->twApi_SetPingRate = twApi_SetPingRate;
	twApi_stub->twApi_SetConnectTimeout = twApi_SetConnectTimeout;
	twApi_stub->twApi_SetConnectRetries = twApi_SetConnectRetries;
	twApi_stub->twApi_SetGatewayName = twApi_SetGatewayName;
	twApi_stub->twApi_SetGatewayType = twApi_SetGatewayType;
	twApi_stub->twApi_BindThing = twApi_BindThing;
	twApi_stub->twApi_UnbindThing = twApi_UnbindThing;
	twApi_stub->twApi_IsEntityBound = twApi_IsEntityBound;
	twApi_stub->twApi_TaskerFunction = twApi_TaskerFunction;
	twApi_stub->twApi_RegisterProperty = twApi_RegisterProperty;
	twApi_stub->twApi_UpdatePropertyMetaData = twApi_UpdatePropertyMetaData;
	twApi_stub->twApi_AddAspectToProperty = twApi_AddAspectToProperty;
	twApi_stub->twApi_RegisterService = twApi_RegisterService;
	twApi_stub->twApi_AddAspectToService = twApi_AddAspectToService;
	twApi_stub->twApi_RegisterEvent = twApi_RegisterEvent;
	twApi_stub->twApi_AddAspectToEvent = twApi_AddAspectToEvent;
	twApi_stub->twApi_RegisterPropertyCallback = twApi_RegisterPropertyCallback;
	twApi_stub->twApi_RegisterServiceCallback = twApi_RegisterServiceCallback;
	twApi_stub->twApi_UnregisterThing = twApi_UnregisterThing;
	twApi_stub->twApi_UnregisterCallback = twApi_UnregisterCallback;
	twApi_stub->twApi_UnregisterPropertyCallback = twApi_UnregisterPropertyCallback;
	twApi_stub->twApi_UnregisterServiceCallback = twApi_UnregisterServiceCallback;
	twApi_stub->twApi_RegisterDefaultRequestHandler = twApi_RegisterDefaultRequestHandler;
	twApi_stub->twApi_CreatePropertyList = twApi_CreatePropertyList;
	twApi_stub->twApi_DeletePropertyList = twApi_DeletePropertyList;
	twApi_stub->twApi_AddPropertyToList = twApi_AddPropertyToList;
	twApi_stub->twApi_ReadProperty = twApi_ReadProperty;
	twApi_stub->twApi_WriteProperty = twApi_WriteProperty;
	twApi_stub->twApi_SetSubscribedPropertyVTQ = twApi_SetSubscribedPropertyVTQ;
	twApi_stub->twApi_SetSubscribedProperty = twApi_SetSubscribedProperty;
	twApi_stub->twApi_PushSubscribedProperties = twApi_PushSubscribedProperties;
	twApi_stub->twApi_PushProperties = twApi_PushProperties;
	twApi_stub->twApi_InvokeService = twApi_InvokeService;
	twApi_stub->twApi_FireEvent = twApi_FireEvent;
	twApi_stub->twApi_RegisterConnectCallback = twApi_RegisterConnectCallback;
	twApi_stub->twApi_RegisterCloseCallback = twApi_RegisterCloseCallback;
	twApi_stub->twApi_RegisterPingCallback = twApi_RegisterPingCallback;
	twApi_stub->twApi_RegisterPongCallback = twApi_RegisterPongCallback;
	twApi_stub->twApi_RegisterBindEventCallback = twApi_RegisterBindEventCallback;
	twApi_stub->twApi_UnregisterBindEventCallback = twApi_UnregisterBindEventCallback;
	twApi_stub->twApi_RegisterOnAuthenticatedCallback = twApi_RegisterOnAuthenticatedCallback;
	twApi_stub->twApi_UnregisterOnAuthenticatedCallback = twApi_UnregisterOnAuthenticatedCallback;
	twApi_stub->twApi_CleanupOldMessages = twApi_CleanupOldMessages;
	twApi_stub->twApi_SendPing = twApi_SendPing;
	twApi_stub->twApi_CreateTask = twApi_CreateTask;
	twApi_stub->twApi_SetSelfSignedOk = twApi_SetSelfSignedOk;
	twApi_stub->twApi_DisableCertValidation = twApi_DisableCertValidation;
	twApi_stub->twApi_LoadCACert = twApi_LoadCACert;
	twApi_stub->twApi_LoadClientCert = twApi_LoadClientCert;
	twApi_stub->twApi_SetClientKey = twApi_SetClientKey;
	twApi_stub->twApi_EnableFipsMode = twApi_EnableFipsMode;
	twApi_stub->twApi_DisableEncryption = twApi_DisableEncryption;
	twApi_stub->twApi_SetX509Fields = twApi_SetX509Fields;
	twApi_stub->twApi_SetOfflineMsgStoreDir = twApi_SetOfflineMsgStoreDir;
	twApi_stub->twApi_GetConnectionInfo = twApi_GetConnectionInfo;

	/* Properties */
	twApi_stub->twPropertyDef_Create = twPropertyDef_Create;
	twApi_stub->twPropertyDef_Delete = twPropertyDef_Delete;
	twApi_stub->twProperty_Create = twProperty_Create;
	twApi_stub->twPropertyVTQ_Create = twPropertyVTQ_Create;
	twApi_stub->twProperty_CreateFromStream = twProperty_CreateFromStream;
	twApi_stub->twProperty_Delete = twProperty_Delete;

	/* Services */
	twApi_stub->twServiceDef_Create = twServiceDef_Create;
	twApi_stub->twServiceDef_Delete = twServiceDef_Delete;

	/* Base Types */
	twApi_stub->twStream_Create = twStream_Create;
	twApi_stub->twStream_CreateFromCharArray = twStream_CreateFromCharArray;
	twApi_stub->twStream_CreateFromCharArrayZeroCopy = twStream_CreateFromCharArrayZeroCopy;
	twApi_stub->twStream_Delete = twStream_Delete;
	twApi_stub->twStream_GetData = twStream_GetData;
	twApi_stub->twStream_GetIndex = twStream_GetIndex;
	twApi_stub->twStream_GetLength = twStream_GetLength;
	twApi_stub->twStream_AddBytes = twStream_AddBytes;
	twApi_stub->twStream_GetBytes = twStream_GetBytes;
	twApi_stub->twStream_Reset = twStream_Reset;
	twApi_stub->twStream_CreateFromFile = twStream_CreateFromFile;
	twApi_stub->swap4bytes = swap4bytes;
	twApi_stub->swap8bytes = swap8bytes;
	twApi_stub->stringToStream = stringToStream;
	twApi_stub->streamToString = streamToString;
	twApi_stub->baseTypeFromString = baseTypeFromString;
	twApi_stub->baseTypeToString = baseTypeToString;
	twApi_stub->twPrimitive_Create = twPrimitive_Create;
	twApi_stub->twPrimitive_CreateFromStream = twPrimitive_CreateFromStream;
	twApi_stub->twPrimitive_CreateFromStreamTyped = twPrimitive_CreateFromStreamTyped;
	twApi_stub->twPrimitive_ZeroCopy = twPrimitive_ZeroCopy;
	twApi_stub->twPrimitive_FullCopy = twPrimitive_FullCopy;
	twApi_stub->twPrimitive_Delete = twPrimitive_Delete;
	twApi_stub->twPrimitive_ToStream = twPrimitive_ToStream;
	twApi_stub->twPrimitive_DecoupleStringAndDelete = twPrimitive_DecoupleStringAndDelete;
	twApi_stub->twPrimitive_Compare = twPrimitive_Compare;
	twApi_stub->twPrimitive_IsTrue = twPrimitive_IsTrue;
	twApi_stub->twPrimitive_CreateFromLocation = twPrimitive_CreateFromLocation;
	twApi_stub->twPrimitive_CreateFromNumber = twPrimitive_CreateFromNumber;
	twApi_stub->twPrimitive_CreateFromInteger = twPrimitive_CreateFromInteger;
	twApi_stub->twPrimitive_CreateFromDatetime = twPrimitive_CreateFromDatetime;
	twApi_stub->twPrimitive_CreateFromCurrentTime = twPrimitive_CreateFromCurrentTime;
	twApi_stub->twPrimitive_CreateFromBoolean = twPrimitive_CreateFromBoolean;
	twApi_stub->twPrimitive_CreateFromInfoTable = twPrimitive_CreateFromInfoTable;
	twApi_stub->twPrimitive_CreateVariant = twPrimitive_CreateVariant;
	twApi_stub->twPrimitive_CreateFromString = twPrimitive_CreateFromString;
	twApi_stub->twPrimitive_CreateFromBlob = twPrimitive_CreateFromBlob;
	twApi_stub->twPrimitive_CreateFromVariable = twPrimitive_CreateFromVariable;
	twApi_stub->twPrimitive_ToJson = twPrimitive_ToJson;
	twApi_stub->twPrimitive_CreateFromJson = twPrimitive_CreateFromJson;

	/* InfoTables */
	twApi_stub->twDataShapeAspect_Create = twDataShapeAspect_Create;
	twApi_stub->twDataShapeAspect_CreateFromStream = twDataShapeAspect_CreateFromStream;
	twApi_stub->twDataShapeAspect_Delete = twDataShapeAspect_Delete;
	twApi_stub->twDataShapeEntry_Create = twDataShapeEntry_Create;
	twApi_stub->twDataShapeEntry_CreateFromStream = twDataShapeEntry_CreateFromStream;
	twApi_stub->twDataShapeEntry_Delete = twDataShapeEntry_Delete;
	twApi_stub->twDataShapeEntry_AddAspect = twDataShapeEntry_AddAspect;
	twApi_stub->twDataShapeEntry_GetLength = twDataShapeEntry_GetLength;
	twApi_stub->twDataShapeEntry_ToStream = twDataShapeEntry_ToStream;
	twApi_stub->twDataShape_Create = twDataShape_Create;
	twApi_stub->twDataShape_CreateFromStream = twDataShape_CreateFromStream;
	twApi_stub->twDataShape_Delete = twDataShape_Delete;
	twApi_stub->twDataShape_GetLength = twDataShape_GetLength;
	twApi_stub->twDataShape_ToStream = twDataShape_ToStream;
	twApi_stub->twDataShape_SetName = twDataShape_SetName;
	twApi_stub->twDataShape_AddEntry = twDataShape_AddEntry;
	twApi_stub->twDataShape_GetEntryIndex = twDataShape_GetEntryIndex;
	twApi_stub->twInfoTableRow_Create = twInfoTableRow_Create;
	twApi_stub->twInfoTableRow_CreateFromStream = twInfoTableRow_CreateFromStream;
	twApi_stub->twInfoTableRow_Delete = twInfoTableRow_Delete;
	twApi_stub->twInfoTableRow_GetCount = twInfoTableRow_GetCount;
	twApi_stub->twInfoTableRow_GetLength = twInfoTableRow_GetLength;
	twApi_stub->twInfoTableRow_AddEntry = twInfoTableRow_AddEntry;
	twApi_stub->twInfoTableRow_GetEntry = twInfoTableRow_GetEntry;
	twApi_stub->twInfoTableRow_ToStream = twInfoTableRow_ToStream;
	twApi_stub->twInfoTable_Create = twInfoTable_Create;
	twApi_stub->twInfoTable_CreateFromStream = twInfoTable_CreateFromStream;
	twApi_stub->twInfoTable_Delete = twInfoTable_Delete;
	twApi_stub->twInfoTable_FullCopy = twInfoTable_FullCopy;
	twApi_stub->twInfoTable_ZeroCopy = twInfoTable_ZeroCopy;
	twApi_stub->twInfoTable_Compare = twInfoTable_Compare;
	twApi_stub->twInfoTable_AddRow = twInfoTable_AddRow;
	twApi_stub->twInfoTable_GetEntry = twInfoTable_GetEntry;
	twApi_stub->twInfoTable_ToStream = twInfoTable_ToStream;
	twApi_stub->twInfoTable_CreateFromPrimitive = twInfoTable_CreateFromPrimitive;
	twApi_stub->twInfoTable_CreateFromString = twInfoTable_CreateFromString;
	twApi_stub->twInfoTable_CreateFromNumber = twInfoTable_CreateFromNumber;
	twApi_stub->twInfoTable_CreateFromInteger = twInfoTable_CreateFromInteger;
	twApi_stub->twInfoTable_CreateFromLocation = twInfoTable_CreateFromLocation;
	twApi_stub->twInfoTable_CreateFromBlob = twInfoTable_CreateFromBlob;
	twApi_stub->twInfoTable_CreateFromDatetime = twInfoTable_CreateFromDatetime;
	twApi_stub->twInfoTable_CreateFromBoolean = twInfoTable_CreateFromBoolean;
	twApi_stub->twInfoTable_GetString = twInfoTable_GetString;
	twApi_stub->twInfoTable_GetNumber = twInfoTable_GetNumber;
	twApi_stub->twInfoTable_GetInteger = twInfoTable_GetInteger;
	twApi_stub->twInfoTable_GetLocation = twInfoTable_GetLocation;
	twApi_stub->twInfoTable_GetBlob = twInfoTable_GetBlob;
	twApi_stub->twInfoTable_GetDatetime = twInfoTable_GetDatetime;
	twApi_stub->twInfoTable_GetBoolean = twInfoTable_GetBoolean;
	twApi_stub->twInfoTable_GetPrimitive = twInfoTable_GetPrimitive;
	twApi_stub->twInfoTable_CreateFromJson = twInfoTable_CreateFromJson;
	twApi_stub->twDataShape_ToJson = twDataShape_ToJson;
	twApi_stub->twInfoTable_ToJson = twInfoTable_ToJson;

	/* Messages */
	twApi_stub->twMessage_Create = twMessage_Create;
	twApi_stub->twMessage_CreateRequestMsg = twMessage_CreateRequestMsg;
	twApi_stub->twMessage_CreateResponseMsg = twMessage_CreateResponseMsg;
	twApi_stub->twMessage_CreateBindMsg = twMessage_CreateBindMsg;
	twApi_stub->twMessage_CreateAuthMsg = twMessage_CreateAuthMsg;
	twApi_stub->twMessage_CreateFromStream = twMessage_CreateFromStream;
	twApi_stub->twMessage_Delete = twMessage_Delete;
	twApi_stub->twMessage_Send = twMessage_Send;
	twApi_stub->twMessage_SetBody = twMessage_SetBody;
	twApi_stub->twRequestBody_Create = twRequestBody_Create;
	twApi_stub->twRequestBody_CreateFromStream = twRequestBody_CreateFromStream;
	twApi_stub->twRequestBody_Delete = twRequestBody_Delete;
	twApi_stub->twRequestBody_SetParams = twRequestBody_SetParams;
	twApi_stub->twRequestBody_SetEntity = twRequestBody_SetEntity;
	twApi_stub->twRequestBody_SetCharacteristic = twRequestBody_SetCharacteristic;
	twApi_stub->twRequestBody_AddHeader = twRequestBody_AddHeader;
	twApi_stub->twRequestBody_ToStream = twRequestBody_ToStream;
	twApi_stub->twResponseBody_Create = twResponseBody_Create;
	twApi_stub->twResponseBody_CreateFromStream = twResponseBody_CreateFromStream;
	twApi_stub->twResponseBody_Delete = twResponseBody_Delete;
	twApi_stub->twResponseBody_SetContent = twResponseBody_SetContent;
	twApi_stub->twResponseBody_SetReason = twResponseBody_SetReason;
	twApi_stub->twResponseBody_ToStream = twResponseBody_ToStream;
	twApi_stub->twAuthBody_Create = twAuthBody_Create;
	twApi_stub->twAuthBody_CreateFromStream = twAuthBody_CreateFromStream;
	twApi_stub->twAuthBody_Delete = twAuthBody_Delete;
	twApi_stub->twAuthBody_SetClaim = twAuthBody_SetClaim;
	twApi_stub->twAuthBody_ToStream = twAuthBody_ToStream;
	twApi_stub->twBindBody_Create = twBindBody_Create;
	twApi_stub->twBindBody_CreateFromStream = twBindBody_CreateFromStream;
	twApi_stub->twBindBody_Delete = twBindBody_Delete;
	twApi_stub->twBindBody_AddName = twBindBody_AddName;
	twApi_stub->twBindBody_ToStream = twBindBody_ToStream;
	twApi_stub->twMultipartBody_CreateFromStream = twMultipartBody_CreateFromStream;
	twApi_stub->twMultipartBody_Delete = twMultipartBody_Delete;
	twApi_stub->mulitpartMessageStoreEntry_Create = mulitpartMessageStoreEntry_Create;
	twApi_stub->mulitpartMessageStoreEntry_Delete = mulitpartMessageStoreEntry_Delete;
	twApi_stub->twMultipartMessageStore_Instance = twMultipartMessageStore_Instance;
	twApi_stub->twMultipartMessageStore_Delete = twMultipartMessageStore_Delete;
	twApi_stub->twMultipartMessageStore_AddMessage = twMultipartMessageStore_AddMessage;
	twApi_stub->twMultipartMessageStore_RemoveStaleMessages = twMultipartMessageStore_RemoveStaleMessages;
	twApi_stub->twCompressBytes = twCompressBytes;

	/* Messaging */
	twApi_stub->twMessageHandler_Instance = twMessageHandler_Instance;
	twApi_stub->twMessageHandler_Delete = twMessageHandler_Delete;
	twApi_stub->twMessageHandler_CleanupOldMessages = twMessageHandler_CleanupOldMessages;
	twApi_stub->twMessageHandler_msgHandlerTask = twMessageHandler_msgHandlerTask;
	twApi_stub->twMessageHandler_RegisterConnectCallback = twMessageHandler_RegisterConnectCallback;
	twApi_stub->twMessageHandler_RegisterCloseCallback = twMessageHandler_RegisterCloseCallback;
	twApi_stub->twMessageHandler_RegisterPingCallback = twMessageHandler_RegisterPingCallback;
	twApi_stub->twMessageHandler_RegisterPongCallback = twMessageHandler_RegisterPongCallback;
	twApi_stub->twMessageHandler_RegisterDefaultRequestCallback = twMessageHandler_RegisterDefaultRequestCallback;
	twApi_stub->twMessageHandler_RegisterDumpIncomingMsgListCallback = twMessageHandler_RegisterDumpIncomingMsgListCallback;
	twApi_stub->twMessageHandler_RegisterRequestCallback = twMessageHandler_RegisterRequestCallback;
	twApi_stub->twMessageHandler_RegisterResponseCallback = twMessageHandler_RegisterResponseCallback;
	twApi_stub->twMessageHandler_GetCompletedResponseStruct = twMessageHandler_GetCompletedResponseStruct;
	twApi_stub->twMessageHandler_UnegisterRequestCallback = twMessageHandler_UnegisterRequestCallback;
	twApi_stub->twMessageHandler_UnegisterResponseCallback = twMessageHandler_UnegisterResponseCallback;

	/* Subscribed Properties */
	twApi_stub->twSubscribedPropsMgr_Initialize = twSubscribedPropsMgr_Initialize;
	twApi_stub->twSubscribedPropsMgr_Delete = twSubscribedPropsMgr_Delete;
	twApi_stub->twSubscribedPropsMgr_SetFolding = twSubscribedPropsMgr_SetFolding;
	twApi_stub->twSubscribedPropsMgr_PushSubscribedProperties = twSubscribedPropsMgr_PushSubscribedProperties;
	twApi_stub->twSubscribedPropsMgr_SetPropertyVTQ = twSubscribedPropsMgr_SetPropertyVTQ;
	twApi_stub->twSubscribedPropsMgr_QueueValueForSending = twSubscribedPropsMgr_QueueValueForSending;
	twApi_stub->twSubscribedProperty_Delete = twSubscribedProperty_Delete;
	twApi_stub->twSubscribedProperty_ToStream = twSubscribedProperty_ToStream;

	/* Offline Message Store */
	twApi_stub->twOfflineMsgStore_HandleRequest = twOfflineMsgStore_HandleRequest;
	twApi_stub->twOfflineMsgStore_Initialize = twOfflineMsgStore_Initialize;
	twApi_stub->twOfflineMsgStore_SetDir = twOfflineMsgStore_SetDir;
	twApi_stub->twOfflineMsgStore_Delete = twOfflineMsgStore_Delete;

	/* Crypto */
	/* these are commented out because they break the openssl build
	twApi_stub->EncryptDES = EncryptDES;
	twApi_stub->DecryptDES = DecryptDES;
	twApi_stub->createDESKey = createDESKey;
	twApi_stub->MD4Hash = MD4Hash;
	*/

	/* twList */
	twApi_stub->twList_Create = twList_Create;
	twApi_stub->twList_CreateSearchable = twList_CreateSearchable;
	twApi_stub->twList_Delete = twList_Delete;
	twApi_stub->twList_Clear = twList_Clear;
	twApi_stub->twList_Add = twList_Add;
	twApi_stub->twList_Remove = twList_Remove;
	twApi_stub->twList_Next = twList_Next;
	twApi_stub->twList_GetByIndex = twList_GetByIndex;
	twApi_stub->twList_GetCount = twList_GetCount;
	twApi_stub->twList_ReplaceValue = twList_ReplaceValue;

	/* twMap */
	twApi_stub->twMap_Add = twMap_Add;
	twApi_stub->twMap_Remove = twMap_Remove;

	/* twDict */
	twApi_stub->twDict_Add = twDict_Add;


/* String Utils */
	twApi_stub->lowercase = lowercase;
	twApi_stub->uppercase = uppercase;
	twApi_stub->duplicateString = duplicateString;

/* twProxy */
	/*twApi_stub->connectToProxy = connectToProxy;*/

/* twSocket */
	twApi_stub->twSocket_Read = twSocket_Read;
	twApi_stub->twSocket_Write = twSocket_Write;
	twApi_stub->twSocket_WaitFor = twSocket_WaitFor;

/* twNtlm */
	twApi_stub->NTLM_parseType2Msg = NTLM_parseType2Msg;
	twApi_stub->NTLM_connectToProxy = NTLM_connectToProxy;
	twApi_stub->NTLM_sendType1Msg = NTLM_sendType1Msg;

/* ntlm */
	twApi_stub->GenerateType1Msg = GenerateType1Msg;
	twApi_stub->GenerateType3Msg = GenerateType3Msg;

/* base64_encode */
	twApi_stub->base64_encode = base64_encode;

/* Logger */
	twApi_stub->twLogger_Instance = twLogger_Instance;
	twApi_stub->twLogger_Delete = twLogger_Delete;
	twApi_stub->twLogger_SetLevel = twLogger_SetLevel;
	twApi_stub->twLogger_SetFunction = twLogger_SetFunction;
	twApi_stub->twLogger_SetIsVerbose = twLogger_SetIsVerbose;
	twApi_stub->twLog = twLog;
	twApi_stub->twLogHexString = twLogHexString;
	twApi_stub->twLogMessage = twLogMessage;
	twApi_stub->twCodeToString = twCodeToString;
	twApi_stub->twEntityToString = twEntityToString;
	twApi_stub->twCharacteristicToString = twCharacteristicToString;

/* TLS Client */
	twApi_stub->twTlsClient_UseCertificateChainFile		= twTlsClient_UseCertificateChainFile;
	twApi_stub->twTlsClient_SetClientCaList				= twTlsClient_SetClientCaList;
	twApi_stub->twTlsClient_UsePrivateKeyFile			= twTlsClient_UsePrivateKeyFile;
	twApi_stub->twTlsClient_Read						= twTlsClient_Read;
	twApi_stub->twTlsClient_Write						= twTlsClient_Write;
	twApi_stub->twTlsClient_Reconnect					= twTlsClient_Reconnect;

/* Tasker */
#ifdef ENABLE_TASKER
	twApi_stub->twTasker_Initialize = twTasker_Initialize;
	twApi_stub->twTasker_CreateTask = twTasker_CreateTask;
	twApi_stub->twTasker_RemoveTask = twTasker_RemoveTask;
#endif

/* Websocket */
	twApi_stub->twWs_Create							= twWs_Create;
	twApi_stub->twWs_Delete							= twWs_Delete;
	twApi_stub->twWs_Connect						= twWs_Connect;
	twApi_stub->twWs_Disconnect						= twWs_Disconnect;
	twApi_stub->twWs_IsConnected					= twWs_IsConnected;
	twApi_stub->twWs_RegisterConnectCallback		= twWs_RegisterConnectCallback;
	twApi_stub->twWs_RegisterCloseCallback			= twWs_RegisterCloseCallback;
	twApi_stub->twWs_RegisterBinaryMessageCallback	= twWs_RegisterBinaryMessageCallback;
	twApi_stub->twWs_RegisterTextMessageCallback	= twWs_RegisterTextMessageCallback;
	twApi_stub->twWs_RegisterPingCallback			= twWs_RegisterPingCallback;
	twApi_stub->twWs_RegisterPongCallback			= twWs_RegisterPongCallback;
	twApi_stub->twWs_Receive						= twWs_Receive;
	twApi_stub->twWs_SendMessage					= twWs_SendMessage;
	twApi_stub->twWs_SendPing						= twWs_SendPing;
	twApi_stub->twWs_SendPong						= twWs_SendPong;
	twApi_stub->twWs_SendDataFrame						= twWs_SendDataFrame;

/* cJSON */
	twApi_stub->cJSON_Delete = cJSON_Delete;

/* Mutexes*/
	twApi_stub->twMutex_Lock	= twMutex_Lock;
	twApi_stub->twMutex_Unlock	= twMutex_Unlock;
	twApi_stub->twMutex_Create  = twMutex_Create;

/* file transfer */
	twApi_stub->twDirectory_FileExists         = twDirectory_FileExists;
	twApi_stub->twDirectory_GetFileInfo        = twDirectory_GetFileInfo;
	twApi_stub->twDirectory_GetLastError       = twDirectory_GetLastError;
	twApi_stub->twFile_Create                  = twFile_Create;
	twApi_stub->twFileManager_CloseFile        = twFileManager_CloseFile;
	twApi_stub->twFileManager_GetOpenFile      = twFileManager_GetOpenFile;
	twApi_stub->twFileManager_GetRealPath      = twFileManager_GetRealPath;
	twApi_stub->twFileManager_MakeFileCallback = twFileManager_MakeFileCallback;
	twApi_stub->twFile_Delete				   = twFile_Delete;
	twApi_stub->twFile_FOpen                   = twFile_FOpen;

/* file transferCallback*/
	twApi_stub->twDirectory_CreateDirectory    = twDirectory_CreateDirectory;
	twApi_stub->twDirectory_CreateFile         = twDirectory_CreateFile;
	twApi_stub->twDirectory_DeleteFile         = twDirectory_DeleteFile;
	twApi_stub->listDirsInInfoTable            = listDirsInInfoTable;
#endif
	return TW_OK;
}

int twApi_DeleteStubs() {
#ifdef TW_STUBS
	if(twApi_stub) {
		TW_FREE(twApi_stub);
		twApi_stub = NULL;
	}
#endif
	return TW_OK;
}
