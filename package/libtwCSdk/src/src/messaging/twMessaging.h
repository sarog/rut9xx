/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Binary Messaging layer
 */

#include "twMessages.h"
#include "twWebsocket.h"
#include "twList.h"

#ifndef TW_MESSAGING_H
#define TW_MESSAGING_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************/
/* Message tracking components            */
/******************************************/
typedef int (*message_cb) (struct twWs * ws, struct twMessage * msg);
typedef int (*dumpLog_cb) ();
typedef int (*response_cb) (uint32_t id, enum msgCodeEnum code, char * reason, twInfoTable * content);
typedef int (*eventcb) (struct twWs * ws, const char * data, size_t length);

/***************************************/
/*    Entities below this line are     */
/*    typically not directtly used     */
/*     by application developers       */
/***************************************/

/* Structures used to track request & responses */
typedef struct twRequestCallbackStruct {
	message_cb cb;
	enum entityTypeEnum entityType;
	char * entityName;
	enum characteristicEnum characteristicType;
	char * characteristicName;
} twRequestCallbackStruct;

typedef struct twResponseCallbackStruct {
	char received;
	response_cb cb;
	uint32_t requestId;
	uint32_t sessionId;
	uint64_t expirationTime;
	enum msgCodeEnum code;
	twInfoTable * content;
} twResponseCallbackStruct;

/* Central message handler - this is a singleton */
typedef struct twMessageHandler {
	twWs * ws;
	twList * responseCallbackList;
	twList * incomingRequestCallbacks;
	twList * multipartMessageList;
	message_cb defaultRequestCallback;
	dumpLog_cb dumpIncomingMsgList;
	eventcb on_ws_connected;
	eventcb on_ws_close;
	eventcb on_ping;
	eventcb on_pong;
	TW_MUTEX mtx;
} twMessageHandler;

twMessageHandler * twMessageHandler_Instance(twWs * ws);
int twMessageHandler_Delete(twMessageHandler * handler);
int twMessageHandler_CleanupOldMessages(twMessageHandler * handler);
void twMessageHandler_msgHandlerTask(DATETIME now, void * params);

int twMessageHandler_RegisterConnectCallback(twMessageHandler * handler, eventcb cb);
int twMessageHandler_RegisterCloseCallback(twMessageHandler * handler, eventcb cb);
int twMessageHandler_RegisterPingCallback(twMessageHandler * handler, eventcb cb);
int twMessageHandler_RegisterPongCallback(twMessageHandler * handler, eventcb cb);
int twMessageHandler_RegisterDefaultRequestCallback(twMessageHandler * handler, message_cb cb); 
int twMessageHandler_RegisterDumpIncomingMsgListCallback(twMessageHandler * handler, dumpLog_cb cb); 
int twMessageHandler_RegisterRequestCallback(twMessageHandler * handler, message_cb cb, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName);
int twMessageHandler_RegisterResponseCallback(twMessageHandler * handler, response_cb cb, uint32_t requestId, DATETIME expirationTime); 

twResponseCallbackStruct * twMessageHandler_GetCompletedResponseStruct(twMessageHandler * handler, uint32_t id);
int twMessageHandler_UnegisterRequestCallback(twMessageHandler * handler, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName);
int twMessageHandler_UnegisterResponseCallback(twMessageHandler * handler, uint32_t requestId); 

#ifdef __cplusplus
}
#endif

#endif




