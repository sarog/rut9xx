/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Binary Messaging layer
 */

#include "twOSPort.h"
#include "twLogger.h"
#include "twMessaging.h"
#include "twTasker.h"
#include "stringUtils.h"
#include "twApiStubs.h"

#define TW_DUMP_MSGLIST_THRESHOLD 0.75

/* Singleton message handler */
twMessageHandler * msgHandlerSingleton = NULL;

/* Initialize the state of the ZLib Inflate context */
void twInflateInit(struct twWs * ws) {
	twMutex_Lock (ws->decompressionMutex);

	ws->inflstream.zalloc = Z_NULL;
	ws->inflstream.zfree = Z_NULL;
	ws->inflstream.opaque = Z_NULL;

	inflateInit2(&ws->inflstream, ZLIB_WINDOW_BITS_RAW_DEFLATE);

	ws->bInflateInitialized = TRUE;

	twMutex_Unlock (ws->decompressionMutex);
}

/* Reset the state of the ZLib Inflate context. Called when the WebSocket closes*/
void twInflateEnd(struct twWs * ws)	{
	twMutex_Lock (ws->decompressionMutex);

	if (ws->bInflateInitialized) {
		inflateEnd (&ws->inflstream);
		ws->bInflateInitialized = FALSE;
	}

	twMutex_Unlock (ws->decompressionMutex);
}

int twDecompressStream (twStream** s, struct twWs * ws)	{
	/* Add the DEFLATE block end bytes. The server strips these off before it sends the compressed blocks when using context takeover. */
	/* See RFC 7692 section 7.2.2 */
	char blockEnd [] = {0, 0, 0xFF, 0xFF};
	const char dst[256];
	uint32_t dstLen = sizeof(dst);
	twStream* inflatedStream = twStream_Create ();
	int ret = -1;
	int rc = Z_OK;

	memset(dst, 0, dstLen);

	if (!inflatedStream) {
		TW_LOG(TW_ERROR, "twDeompressStream: Error allocating stream");
		return TW_ERROR_ALLOCATING_MEMORY;
	}

	/* Advance the pointer to the end of the data */
	(*s)->ptr = (*s)->data + (*s)->length;

	/* Add the bytes */
	twStream_AddBytes (*s, &blockEnd, sizeof (blockEnd));

	/* Has the decompression context been initialzed? */
	if (!ws->bInflateInitialized) {
		twInflateInit (ws);
	}

	/* Aquire this socket's decompression context lock */
	twMutex_Lock (ws->decompressionMutex);

	/* Set up the input data */
	ws->inflstream.avail_in = twStream_GetLength (*s);
	ws->inflstream.next_in = (Bytef *)(*s)->data;

	/* run inflate() on input until output buffer not full */
	do {
		uint32_t chunklen = dstLen;

		/* Set up the output buffer */
		ws->inflstream.avail_out = dstLen;
		ws->inflstream.next_out = (Bytef *)dst;

		/* Run inflate */
		rc = inflate(&ws->inflstream, Z_SYNC_FLUSH);
		chunklen = dstLen - ws->inflstream.avail_out;

		/* Note: Z_BUF_ERROR while in Z_SYNC_FLUSH mode means we've processed everything available, but the stream isn't complete.*/
		/*       The remainder of the data should be in the following message(s). Finish the decompression of this stream and move on. */
		if (rc == Z_OK || rc == Z_STREAM_END || rc == Z_BUF_ERROR) {
			/* Move what has been output so far to the destination stream */
			twStream_AddBytes(inflatedStream, dst, chunklen);
		} else {
			/* Decompression failed */
			twStream_Delete (inflatedStream);
			twMutex_Unlock (ws->decompressionMutex);
			TW_LOG(TW_ERROR, "twDecompressStream: Error decompressing stream: %d", rc);
			return TW_ERROR_INFLATING_RECEIVED_MSG;
		}
	} while (ws->inflstream.avail_out == 0);

	/* Delete the original stream and replace it with the decompressed stream */
	twStream_Delete(*s);
	*s = inflatedStream;

	twMutex_Unlock (ws->decompressionMutex);

	return 0;
}

/* Incoming request callback registration structure delete function */
void twRequestCallbackStruct_Delete(void * s) {
	twRequestCallbackStruct * x = NULL;
	if (!s) return;
	x = (twRequestCallbackStruct *)s;
	if (x->entityName) TW_FREE(x->entityName);
	if (x->characteristicName) TW_FREE(x->characteristicName);
	TW_FREE(x);
}

/* Response callback registration structure delete function */
void twResponseCallbackStruct_Delete(void * s) {
	twResponseCallbackStruct * x = NULL;
	if (!s) return;
	x = (twResponseCallbackStruct *)s;
	twInfoTable_Delete(x->content);
	TW_FREE(x);
}

/* Send error response helper function */
void sendErrorResponse(enum msgCodeEnum code, uint32_t id, uint32_t sessionId, uint32_t endpointId , char * reason) {
	/* Send an error response */
	twMessage * resp = NULL;
	if (!msgHandlerSingleton) return;
	resp = twMessage_CreateResponseMsg(code, id, sessionId, endpointId);
	if (resp && msgHandlerSingleton) {
		if (reason) twResponseBody_SetReason((twResponseBody *)(resp->body), reason);
		twMessage_Send(&resp, msgHandlerSingleton->ws);
		twMessage_Delete(resp);
	} else TW_LOG(TW_ERROR,"sendErrorResponse: Error allocating response");
}

/* Message handler helper function */
int handleMessage(twMessage ** msg) {
	int res = TW_OK;
	char completedMultipartMessage = FALSE;
	if ((*msg)->multipartMarker) {
		if ((*msg)->body) {
			/* Handle Multipart messages */
			twMultipartBody * mp = (twMultipartBody *)((*msg)->body);
			TW_LOG(TW_TRACE, "twMessaging::handleMessage: Handling multipart message. request: %d, chunk %d of %d", (*msg)->requestId, mp->chunkId, mp->chunkCount);
			if (mp->chunkCount * mp->chunkSize > twcfg.max_message_size) {
				TW_LOG(TW_ERROR, "twMessaging::handleMessage: Multipart message would be too large. Discarding. chunkCount: %d, chunkSize: %d, max: %d",
                        mp->chunkCount, mp->chunkSize, twcfg.max_message_size);
				return res;
			}
			*msg = twMultipartMessageStore_AddMessage(*msg);
			if (!(*msg)) {
				/* This is not the last chunk of the multipart message */
				return res;
			}
			completedMultipartMessage = TRUE;
		}
	}
	/* Log this message but make sure we indicate that it was multipart*/
	if (completedMultipartMessage) (*msg)->multipartMarker = TRUE;
	TW_LOG_MSG(*msg, "Recv'd Msg <<<<<<<<<");
	if (completedMultipartMessage) (*msg)->multipartMarker = FALSE;
	/* Handle the complete message */
	if ((*msg)->type == TW_REQUEST) {
		/* See if there is a request handler */
		twRequestCallbackStruct * cb = 0;
		TW_LOG(TW_TRACE,"handleMessage: Received Request Message ID: %d", (*msg)->requestId);
		if (msgHandlerSingleton && msgHandlerSingleton->incomingRequestCallbacks) {
			ListEntry * le = NULL;
			twRequestBody * req = NULL;
			twMutex_Lock(msgHandlerSingleton->mtx);
			le = twList_Next(msgHandlerSingleton->incomingRequestCallbacks, NULL);
			req = (twRequestBody *)((*msg)->body);
			while (le && !cb) {
				if (le->value) {
					twRequestCallbackStruct * tmp = (twRequestCallbackStruct *)(le->value);
					if ((tmp->entityType == req->entityType) && !strcmp(tmp->entityName, req->entityName) &&
						(tmp->characteristicType == req->characteristicType) && !strcmp(tmp->characteristicName, req->characteristicName)) {
							TW_LOG(TW_TRACE,"handleMessage: Callback found for message %d", (*msg)->requestId);
							tmp->cb(msgHandlerSingleton->ws, (*msg));
							break;
					}
				}
			}
			twMutex_Unlock(msgHandlerSingleton->mtx);
		}
		if (!cb) {
			if (msgHandlerSingleton->defaultRequestCallback) {
				/* If there is a Default request handler, use that */
				msgHandlerSingleton->defaultRequestCallback(msgHandlerSingleton->ws, (*msg));
			} else {
				TW_LOG(TW_WARN,"handleMessage: No Callback found for Request ID: %d", (*msg)->requestId);
				sendErrorResponse(TWX_BAD_REQUEST, (*msg)->requestId, (*msg)->sessionId, (*msg)->endpointId, "Message Type not supported");
			}
		}
	} else if ((*msg)->type == TW_RESPONSE) {
		/* See if there is a response handler */
		char FoundCb = FALSE;
		twResponseBody * b = NULL;
		TW_LOG(TW_TRACE,"handleMessage: Received Response to Message ID: %d", (*msg)->requestId);
		if (msgHandlerSingleton->responseCallbackList) {
			ListEntry * le = NULL;
			twMutex_Lock(msgHandlerSingleton->mtx);
			le = twList_Next(msgHandlerSingleton->responseCallbackList, NULL);
			while (le && !FoundCb) {
				if (le->value) {
					twResponseCallbackStruct * tmp = (twResponseCallbackStruct *)(le->value);
					if (tmp->requestId == (*msg)->requestId) {
						TW_LOG(TW_TRACE,"handleMessage: Got response for message %d", (*msg)->requestId);
						FoundCb = TRUE;
						b = (twResponseBody *)((*msg)->body);
						if (b) {
							if ((*msg)->code == TWX_INTERNAL_SERVER_ERROR) {
								TW_LOG(TW_ERROR, "twMessaging::handleMessage: Response ID %d has code %d: %s", (*msg)->requestId, (*msg)->code, b->reason ? b->reason : "None");
							}
							tmp->code = (*msg)->code;
							tmp->content = twInfoTable_ZeroCopy(b->content); /* This content is now owned by tmp */
							tmp->received = TRUE;
							tmp->sessionId = (*msg)->sessionId;
							/* If there is a callback, call it.  Otherwise we expect someone to come pick it up */
							if (tmp->cb) {
								tmp->cb((*msg)->requestId, (*msg)->code, b->reason, tmp->content);
								/* Remove this from the list */
								twList_Remove(msgHandlerSingleton->responseCallbackList, le, TRUE);
							}
							TW_LOG(TW_TRACE,"handleMessage: Marked message %d as received", (*msg)->requestId);
						} else {
							TW_LOG(TW_ERROR,"handleMessage: NULL response body in message %d", (*msg)->requestId);
							twList_Remove(msgHandlerSingleton->responseCallbackList, le, TRUE);
							res = TW_NULL_MSG_BODY;
						}
						break;
					}
				}
				le = twList_Next(msgHandlerSingleton->responseCallbackList, le);
			}
			twMutex_Unlock(msgHandlerSingleton->mtx);
		}
		if (!FoundCb) {
			TW_LOG(TW_TRACE,"handleMessage: Could not find matching request for Response message ID: %d", (*msg)->requestId);
		}
	} else {
		/* Send an error response */
		TW_LOG(TW_WARN,"handleMessage: Received Unhandled Message Type.  Request ID: %d, Type: %d", (*msg)->requestId, (*msg)->type);
		sendErrorResponse(TWX_BAD_REQUEST, (*msg)->requestId, (*msg)->sessionId, (*msg)->endpointId, "Message Type not supported");
	}

	return res;
}

/* Message handling queue */
twList * incomingMsgList = NULL;
char preventIncomingMsgListDump = FALSE;

twList * getIncomingMsgList() {
    return incomingMsgList;
}

void resetIncomingMsgList() {
    twList_Delete(incomingMsgList);
    incomingMsgList = twList_Create(twMessage_Delete);
}

char getPreventIncomingMsgListDump() {
    return preventIncomingMsgListDump;
}

int dumpIncomingMsgListHandler (void *key, size_t key_size, void *data, size_t data_size,void *userData){
	/* print node as a message */
	twMessage * tmp_msg = (twMessage*)data;
	TW_LOG_MSG(tmp_msg,NULL);
	/* continue */
	return TW_FOREACH_CONTINUE;
}

int dumpIncomingMsgListCallback() {
	int res = TW_OK;
	if (incomingMsgList && msgHandlerSingleton) {
        if (!preventIncomingMsgListDump) {
            twList_Foreach(incomingMsgList, dumpIncomingMsgListHandler, NULL);
            preventIncomingMsgListDump = TRUE;
        }
	} else {
		res = TW_NULL_OR_INVALID_MSG_HANDLER;
	}
	return res;
}

/*
 * This function checks the incoming message list for new messages and processes them.
 * The params argument can optionally be a char* representing a boolean.  If the value
 * is true, then this function will only process response messages, and no other types.
 */
void twMessageHandler_msgHandlerTask(DATETIME now, void * params) {
	ListEntry * entry = NULL;
	twMessage * msg = NULL;
	char onlyResponses = FALSE;

	/* See if the params contains a boolean, indicating that this handler
	   invocation should only process responses. */
	if (params) {
		onlyResponses = *((char *) params);
	}

	if (incomingMsgList && msgHandlerSingleton) {
		if (incomingMsgList->count > 0) {
			TW_LOG(TW_TRACE, "twMessageHandler_msgHandlerTask: Processing Incoming Message Queue, Queue Total: %d",
				   incomingMsgList->count);
		}
		twMutex_Lock(msgHandlerSingleton->mtx);
		entry = twList_Next(incomingMsgList, NULL);

		if (entry && entry->value) {
			while (entry && entry->value) {
				msg = (twMessage *)entry->value;

				/* If we're only handling responses, then ignore this message,
				   leave it in the list, and move on to the next one */
				if (onlyResponses && msg->type != TW_RESPONSE && msg->type != TW_MULTIPART_RESP) {
					entry = twList_Next(incomingMsgList, entry);

					if (entry == NULL || entry->value == NULL) {
						/* while loop will exit, so this is the last chance to unlock */
						twMutex_Unlock(msgHandlerSingleton->mtx);
					}
					continue;
				}

				TW_LOG(TW_TRACE,"twMessageHandler_msgHandlerTask: Removing Message ID: %d from Incoming Messages Queue", msg->requestId);
				twList_Remove(incomingMsgList, entry, FALSE);

                /* Is the incomingMsgList's count below the TW_DUMP_MSGLIST_THRESHOLD? */
                if (preventIncomingMsgListDump && incomingMsgList->count < (TW_DUMP_MSGLIST_THRESHOLD * twcfg.max_messages)) {
                    preventIncomingMsgListDump = FALSE;
                }

				twMutex_Unlock(msgHandlerSingleton->mtx);
				TW_LOG(TW_TRACE,"twMessageHandler_msgHandlerTask: Received Binary Message ID: %d, Queue Total: %d", msg->requestId, twList_GetCount(incomingMsgList));
				if (msg) {
					handleMessage(&msg);
					if (msg) twMessage_Delete(msg);
				} else {
					TW_LOG(TW_ERROR,"msgHandlerThread: NULL msg pointer found");
				}

				/* Once we process a message we leave the loop */
				break;
			}
		} else {
			/* No entries in list. Unlock and return */
			twMutex_Unlock(msgHandlerSingleton->mtx);
		}
	}
}

/* Websocket call back functions */
int msgHandlerOnConnect(struct twWs * ws) {
	TW_LOG(TW_TRACE,"msgHandlerOnConnect: WEBSOCKET CONNECTED");
	if (msgHandlerSingleton && msgHandlerSingleton->on_ws_connected) msgHandlerSingleton->on_ws_connected(ws, 0, 0);
	return 0;
}

int msgHandlerOnPing (struct twWs * ws, const char *at, size_t length) {
	TW_LOG(TW_TRACE,"msgHandlerOnPing: Received Ping.  Data: %s", (at ? at : "No Data"));
	if (msgHandlerSingleton && msgHandlerSingleton->on_ping) msgHandlerSingleton->on_ping(ws, at, length);
	return 0;
}

int msgHandlerOnPong (struct twWs * ws, const char *at, size_t length) {
	/* Passed in length is the length of data, not a string length and at isn't null terminated */
	char *msg = (char *)TW_CALLOC(length + 1, 1);
	if (msg) {
		memcpy(msg, at, length);
	}
	TW_LOG(TW_TRACE,"msgHandlerOnPong: Received Pong.  Data: %s", (msg ? msg : "No Data"));
	if (msgHandlerSingleton && msgHandlerSingleton->on_pong) msgHandlerSingleton->on_pong(ws, msg, length);
	TW_FREE(msg);
	return 0;
}

int msgHandlerOnClose (struct twWs * ws, const char *at, size_t length) {
	TW_LOG(TW_WARN,"msgHandlerOnClose: WEBSOCKET CLOSED");
	if (msgHandlerSingleton && msgHandlerSingleton->on_ws_close) msgHandlerSingleton->on_ws_close(ws, at, length);
	return 0;
}

int msgHandlerOnBinaryMessage (struct twWs * ws, const char *at, size_t length, char bCompressed) {
	twStream * s = NULL;
	twMessage * msg = NULL;
	if (!incomingMsgList)
		{
		TW_LOG(TW_ERROR, "msgHandlerOnBinaryMessage: NULL incomingMsgList pointer");
		return 1;
		}

	s = s_twStream_CreateFromCharArray(at, length);
	if (!s) {
		TW_LOG(TW_ERROR,"msgHandlerOnBinaryMessage: Error creating twStream from binary message");
	} else if (bCompressed) {
		twDecompressStream (&s, ws);
	}
    
    TW_LOG_HEX(twStream_GetData(s), "msgHandlerOnBinaryMessage: Rcvd Message <<<<\n", twStream_GetLength(s));
    
	msg = s_twMessage_CreateFromStream(s);
	twStream_Delete(s);
	if (!msg)
		{
		TW_LOG(TW_ERROR, "msgHandlerOnBinaryMessage: Error creating message from binary stream");
		return 1;
		}
	twMutex_Lock(msgHandlerSingleton->mtx);
	/* Check our max_message blow off valve */
	if (twList_GetCount(incomingMsgList) >= twcfg.max_messages) {
		TW_LOG(TW_ERROR,"msgHandlerOnBinaryMessage: Too many messages queued, ignoring message ID: %u", msg->requestId);

		/* dump message queue to logs*/
		msgHandlerSingleton->dumpIncomingMsgList();

		twMessage_Delete(msg);
		twMutex_Unlock(msgHandlerSingleton->mtx);
		return 1;
		}
	twList_Add(incomingMsgList, msg);
    TW_LOG(TW_TRACE, "msgHandlerOnBinaryMessage: Inserted message onto queue. ID: %u, Type: %d, Queue Total: %d", msg->requestId, msg->type, twList_GetCount(incomingMsgList));
	twMutex_Unlock(msgHandlerSingleton->mtx);
	return 0;
}


twMessageHandler * twMessageHandler_Instance(twWs * ws) {
	/* Check to see if it already exists */
	twMessageHandler * tmp = NULL;
	if (msgHandlerSingleton) {
		/* Should we update the websocket pointer? */
		if (ws) msgHandlerSingleton->ws = ws;
		return msgHandlerSingleton;
	}
	/* In order to create the singleton we require a websocket */
	if (!ws) {
		TW_LOG(TW_ERROR, "twMessageHandler_Instance: NULL websocket pointer passed in");
		return NULL;
	}
	tmp = (twMessageHandler *)TW_CALLOC(sizeof(twMessageHandler), 1);
	if (!tmp) {
		TW_LOG(TW_ERROR, "twMessageHandler_Instance: Error allocating storage for message handler");
		return NULL;
	}
	tmp->ws = ws;
	/* Create the lists */
	tmp->incomingRequestCallbacks = twList_Create(twRequestCallbackStruct_Delete);
	if (!tmp->incomingRequestCallbacks) {
		TW_LOG(TW_ERROR, "twMessageHandler_Instance: Error creating request callback list");
		twMessageHandler_Delete(tmp);
		return NULL;
	}
	tmp->responseCallbackList = twList_Create(NULL);
	if (!tmp->responseCallbackList) {
		TW_LOG(TW_ERROR, "twMessageHandler_Instance: Error creating response callback list");
		twMessageHandler_Delete(tmp);
		return NULL;
	}

	tmp->mtx = twMutex_Create();
	/* Register our callbacks with the websocket */
	twWs_RegisterConnectCallback(ws, &msgHandlerOnConnect);
	twWs_RegisterPingCallback(ws,&msgHandlerOnPing);
	twWs_RegisterPongCallback(ws,&msgHandlerOnPong);
	twWs_RegisterCloseCallback(ws,&msgHandlerOnClose);
	twWs_RegisterBinaryMessageCallback(ws,&msgHandlerOnBinaryMessage);

	/* Set up the Message Handling Queue */
	incomingMsgList = twList_Create(twMessage_Delete);
	if (!incomingMsgList) {
		TW_LOG(TW_ERROR, "twMessageHandler_Instance: Error creating incomingMsgList ");
		twMessageHandler_Delete(tmp);
		return NULL;
	}
	/* Setup the multipart message storage */
	twMultipartMessageStore_Instance();

#ifdef ENABLE_TASKER
	/* Create our task */
	twTasker_CreateTask(5, &twMessageHandler_msgHandlerTask);
#endif

	msgHandlerSingleton = tmp;
	return msgHandlerSingleton;
}

int twMessageHandler_Delete(twMessageHandler * handler) {
	if (!handler) {
		handler = msgHandlerSingleton;
		msgHandlerSingleton = NULL;
        }
	/* Unregister our websocket callbacks & disconnect */
	if (handler->ws) {
		twWs_RegisterConnectCallback(handler->ws, NULL);
		twWs_RegisterPingCallback(handler->ws, NULL);
		twWs_RegisterPongCallback(handler->ws, NULL);
		twWs_RegisterCloseCallback(handler->ws, NULL);
		twWs_RegisterBinaryMessageCallback(handler->ws, NULL);
		twWs_Disconnect(handler->ws, NORMAL_CLOSE, "Message handler shut down");
		twWs_Delete(handler->ws);
	}
	twMutex_Delete(handler->mtx);
	/* Delete our lists */
	twMultipartMessageStore_Delete(0);
	if (handler->incomingRequestCallbacks) twList_Delete(handler->incomingRequestCallbacks);
	if (handler->responseCallbackList) twList_Delete(handler->responseCallbackList);
	if (handler->multipartMessageList) twList_Delete(handler->multipartMessageList);
	if (incomingMsgList) {
		twList_Delete(incomingMsgList);
		incomingMsgList = NULL;
	}
	/* Free up ourself */
	TW_FREE(handler);
	handler = NULL;
	return TW_OK;
}

typedef struct twMessageHandler_CleanupOldMessagesForEachParams {
	twList *newResponseCallbackList;
	twList *deletedResponseCallbacksList;
} twMessageHandler_CleanupOldMessagesForEachParams;

int twMessageHandler_CleanupOldMessagesForEachHandler(void *key, size_t key_size, void *currentValue, size_t currentValue_size, void *userData) {
	twResponseCallbackStruct *r = (twResponseCallbackStruct*)currentValue;
	twMessageHandler_CleanupOldMessagesForEachParams *params = (twMessageHandler_CleanupOldMessagesForEachParams*)userData;
	uint64_t now = twGetSystemMillisecondCount();
	if (twTimeGreaterThan(now, r->expirationTime)) {
		TW_LOG(TW_INFO,"twMessageHandler_CleanupOldMessages: Message %d timed out", r->requestId);
		if (r->cb) {
			r->cb(r->requestId, TWX_GATEWAY_TIMEOUT, "Message timed out", NULL);
		}
		twList_Add(params->deletedResponseCallbacksList, r);
	} else {
		twList_Add(params->newResponseCallbackList, r);
	}
	return TW_FOREACH_CONTINUE;
}

int twMessageHandler_CleanupOldMessages(twMessageHandler * handler) {
	twMessageHandler_CleanupOldMessagesForEachParams *params = NULL;
	if (!handler) handler = msgHandlerSingleton;
	if (handler && handler->responseCallbackList) {
		twMutex_Lock(handler->mtx);
		params = TW_MALLOC(sizeof(twMessageHandler_CleanupOldMessagesForEachParams));
		if (!params) {
			TW_LOG(TW_ERROR,"twMessageHandler_CleanupOldMessages: Error allocating twMessageHandler_CleanupOldMessagesForEachParams");
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		params->newResponseCallbackList = twList_Create(handler->responseCallbackList->delete_function);
		if (!params->newResponseCallbackList) {
			TW_LOG(TW_ERROR,"twMessageHandler_CleanupOldMessages: Error allocating params->newResponseCallbackList");
			TW_FREE(params);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		params->deletedResponseCallbacksList = twList_Create(handler->responseCallbackList->delete_function);
		if (!params->deletedResponseCallbacksList) {
			TW_LOG(TW_ERROR,"twMessageHandler_CleanupOldMessages: Error allocating params->deletedResponseCallbacksList");
			twList_Delete(params->newResponseCallbackList);
			TW_FREE(params);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		twList_Foreach(handler->responseCallbackList, twMessageHandler_CleanupOldMessagesForEachHandler, params);
		twList_ClearEntries(handler->responseCallbackList);
		twList_Delete(handler->responseCallbackList);
		twList_Delete(params->deletedResponseCallbacksList);
		handler->responseCallbackList = params->newResponseCallbackList;
		TW_FREE(params);
		twMutex_Unlock(handler->mtx);
	} else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

int twMessageHandler_RegisterConnectCallback(twMessageHandler * handler, eventcb cb) {
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->on_ws_connected = cb;
	return TW_OK;
}

int twMessageHandler_RegisterCloseCallback(twMessageHandler * handler, eventcb cb) {
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->on_ws_close = cb;
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

int twMessageHandler_RegisterDefaultRequestCallback(twMessageHandler * handler, message_cb cb) {
	if (!cb) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterDefaultRequestCallback: NULL callback function specified");
		return 1;
	}
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->defaultRequestCallback = cb;
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

int twMessageHandler_RegisterDumpIncomingMsgListCallback(twMessageHandler * handler, dumpLog_cb cb) {
	if (!cb) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterDumpIncomingMsgListCallback: NULL callback function specified");
		return 1;
	}
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->dumpIncomingMsgList = cb;
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

int twMessageHandler_RegisterRequestCallback(twMessageHandler * handler, message_cb cb, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName) {
	/* Create a new structure */
	twRequestCallbackStruct * s = NULL;
	if (!cb) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterRequestCallback: NULL callback function specified");
		return TW_INVALID_PARAM;
	}
	if (!handler) handler = msgHandlerSingleton;
	s = (twRequestCallbackStruct *)TW_CALLOC(sizeof(twRequestCallbackStruct), 1);
	if (!s) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterRequestCallback: Error allocating RequestCallback structure");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	s->entityType = entityType;
	s->entityName = duplicateString(entityName);
	s->characteristicType = characteristicType;
	s->characteristicName = duplicateString(characteristicName);
	s->cb = cb;
	if (!s->entityName || s->characteristicName) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterRequestCallback: Error duplicating entityName or characteristicName");
		twRequestCallbackStruct_Delete(s);
		return TW_INVALID_CALLBACK_STRUCT;
	}
	twList_Add(handler->incomingRequestCallbacks, s);
	return TW_OK;
}

int twMessageHandler_RegisterResponseCallback(twMessageHandler * handler, response_cb cb, uint32_t requestId, DATETIME expirationTime) {
	/* Create a new structure */
	twResponseCallbackStruct * s = NULL;
	if (!handler) handler = msgHandlerSingleton;
	s = (twResponseCallbackStruct *)TW_CALLOC(sizeof(twResponseCallbackStruct), 1);
	if (!s) {
		TW_LOG(TW_ERROR,"twMessageHandler_RegisterResponseCallback: Error allocating RequestCallback structure");
		return TW_INVALID_CALLBACK_STRUCT;
	}
	s->cb = cb;
	s->requestId = requestId;
	s->expirationTime = expirationTime;
	twMutex_Lock(handler->mtx);
	twList_Add(handler->responseCallbackList, s);
	twMutex_Unlock(handler->mtx);
	return TW_OK;
}

int twMessageHandler_RegisterPingCallback(twMessageHandler * handler, eventcb cb) {
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->on_ping = cb;
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

int twMessageHandler_RegisterPongCallback(twMessageHandler * handler, eventcb cb) {
	if (!handler) handler = msgHandlerSingleton;
	if (handler) handler->on_pong = cb;
	else return TW_NULL_OR_INVALID_MSG_HANDLER;
	return TW_OK;
}

twResponseCallbackStruct * twMessageHandler_GetCompletedResponseStruct(twMessageHandler * handler, uint32_t id) {
	ListEntry * le = NULL;
	if (!handler) handler = msgHandlerSingleton;
	/* Find the entry but only return it if it has been marked as completed */
	if (!handler || !id) {
		TW_LOG(TW_WARN, "twMessageHandler_GetCompletedResponseStruct: NULL Id or MessageHandler");
		return NULL;
	}

	if (!handler->mtx) {
		TW_LOG(TW_ERROR, "twMessageHandler_GetCompletedResponseStruct: NULL mutex in messageHandler");
		return NULL;
	}

	twMutex_Lock(handler->mtx);
	le = twList_Next(handler->responseCallbackList, NULL);
	while (le) {
		if (le->value) {
			twResponseCallbackStruct * tmp = (twResponseCallbackStruct *)(le->value);
			if (tmp->requestId != id) {
				le = twList_Next(handler->responseCallbackList, le);
				continue;
			}
			twMutex_Unlock(handler->mtx);

			/* Return the callback */
			if (tmp->received) {
				TW_LOG(TW_TRACE, "twMessageHandler_GetCompletedResponseStruct: Found Message ID: %d", tmp->requestId);
				return tmp;
			} else return NULL;
		}
	}

	twMutex_Unlock(handler->mtx);
	return NULL;
}

int twMessageHandler_UnegisterRequestCallback(twMessageHandler * handler, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName) {
	ListEntry * le = NULL;
	if (!handler) handler = msgHandlerSingleton;
	if (!handler || !entityName || !characteristicName) {
		TW_LOG(TW_ERROR, "twMessageHandler_UnegisterRequestCallback: NULL input parameter found");
		return TW_INVALID_PARAM;
	}
	if (!handler->mtx) {
		TW_LOG(TW_ERROR, "twMessageHandler_UnregisterResquestCallback: NULL mutex in messageHandler");
		return TW_ERROR_CREATING_MTX;
	}
	twMutex_Lock(handler->mtx);
	le = twList_Next(handler->incomingRequestCallbacks, NULL);
	while (le) {
		if (le->value) {
			twRequestCallbackStruct * tmp = (twRequestCallbackStruct *)(le->value);
			if (tmp->entityType != entityType || strcmp(entityName, tmp->entityName) || tmp->characteristicType != characteristicType || strcmp(characteristicName, tmp->characteristicName)) {
				le = twList_Next(handler->incomingRequestCallbacks, le);
				continue;
			}
			/* Delete the entry */
			twList_Remove(handler->incomingRequestCallbacks, le, TRUE);
			twMutex_Unlock(handler->mtx);
			return TW_OK;
		}
	}
	twMutex_Unlock(handler->mtx);
	return TW_ERROR_CALLBACK_NOT_FOUND;
}

int twMessageHandler_UnegisterResponseCallback(twMessageHandler * handler, uint32_t requestId) {
	ListEntry * le = NULL;
	if (!handler) handler = msgHandlerSingleton;

	/* Find the entry but only return it if it has been marked as completed */
	if (!handler || !requestId) {
		TW_LOG(TW_WARN, "twMessageHandler_UnegisterResponseCallback: NULL Id or MessageHandler");
		return TW_INVALID_PARAM;
	}

	if (!handler->mtx) {
		TW_LOG(TW_ERROR, "twMessageHandler_UnregisterResponseCallback: NULL mutex in messageHandler");
		return TW_ERROR_CREATING_MTX;
	}

	twMutex_Lock(handler->mtx);
	le = twList_Next(handler->responseCallbackList, NULL);
	while (le) {
		if (le->value) {
			twResponseCallbackStruct * tmp = (twResponseCallbackStruct *)(le->value);
			if (tmp->requestId == requestId) {
				twList_Remove(handler->responseCallbackList, le, TRUE);
				break;
			}
			le = twList_Next(handler->responseCallbackList, le);
		}
	}
	twMutex_Unlock(handler->mtx);

	return TW_ERROR_CALLBACK_NOT_FOUND;
}



