/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Binary Messaging layer
 */

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twDefaultSettings.h"
#include "twErrors.h"
#include "twWebsocket.h"
#include "twBaseTypes.h"
#include "twInfoTable.h"
#include "twList.h"

#ifndef TW_MESSAGES_H
#define TW_MESSAGES_H

#define MSG_HEADER_SIZE 15
#define MULTIPART_MSG_HEADER_SIZE 6

#define DEFLATE_SYNC_TRAILER_SIZE 4

#ifdef __cplusplus
extern "C" {
#endif

/***************************************/
/*    Entities below this line are     */
/*    typically not directly used     */
/*     by application developers       */
/***************************************/

/**
*	Message Header
**/
typedef struct twMessage {
	enum msgType type;
	unsigned char version;
	enum msgCodeEnum code;
	uint32_t requestId;
	uint32_t endpointId;
	uint32_t sessionId;
	char multipartMarker;
	uint32_t length;
	void * body;
} twMessage;

int twCompressBytes (char * buf, uint32_t length, twStream* s, struct twWs * ws);

/* Returns a unique request ID.*/
uint32_t twMessage_GetRequestId();

twMessage * twMessage_Create(enum msgCodeEnum code, uint32_t reqId); /* Set Reqid to zero to autogenerate ID */
twMessage * twMessage_CreateRequestMsg(enum msgCodeEnum code);
twMessage * twMessage_CreateResponseMsg(enum msgCodeEnum code, uint32_t id, uint32_t sessionId, uint32_t endpointId);
twMessage * twMessage_CreateBindMsg(char * name, char isUnbind);
twMessage * twMessage_CreateAuthMsg(char * claimName, char * claimValue);
twMessage * twMessage_CreateFromStream(twStream * s);

/* twMessage_ZeroCopy will return a pointer to the input message structure and it will set the input message
* pointer to NULL, effectively transferring memory control from the input pointer to the output pointer.
* This is very important in the twOfflineMsgStore singleton, because the singleton needs to ensure that a particular
* message is not deleted by the message handler before the message is written to disk
* (esspecially in the case of a multi-threaded environment) */
twMessage * twMessage_ZeroCopy(struct twMessage ** msg);

void twMessage_Delete(void * input);
int twMessage_Send(struct twMessage ** msg, struct twWs * ws);
int twMessage_SetBody(struct twMessage * msg, void * body);

/**
*	Request Body
**/
typedef struct twHeader {
	char * name;
	char * value;
} twHeader;

typedef struct twRequestBody {
	enum entityTypeEnum entityType;
	char * entityName;
	enum characteristicEnum characteristicType;
	char * characteristicName;
	char numHeaders;
	twList * headers;
	twInfoTable * params;
	uint32_t length;
} twRequestBody;

twRequestBody * twRequestBody_Create();
twRequestBody * twRequestBody_CreateFromStream(twStream * s);
int twRequestBody_Delete(struct twRequestBody * body);
int twRequestBody_SetParams(struct twRequestBody * body, twInfoTable * params);
int twRequestBody_SetEntity(struct twRequestBody * body, enum entityTypeEnum entityType, char * entityName);
int twRequestBody_SetCharacteristic(struct twRequestBody *body, enum characteristicEnum characteristicType,
                                    char *characteristicName);
int twRequestBody_AddHeader(struct twRequestBody * body, char * name, char * value);
int twRequestBody_ToStream(struct twRequestBody * body, twStream * s);

/**
*	Response Body
**/
typedef struct twResponseBody {
	char reasonMarker;
	char * reason;
	enum BaseType contentType;
	twInfoTable * content;
	uint32_t length;
} twResponseBody;

twResponseBody * twResponseBody_Create();
twResponseBody * twResponseBody_CreateFromStream(twStream * s);
int twResponseBody_Delete(struct twResponseBody * body);
int twResponseBody_SetContent(struct twResponseBody * body, twInfoTable * t);
int twResponseBody_SetReason(struct twResponseBody * body, char * reason);
int twResponseBody_ToStream(struct twResponseBody * body, twStream * s);

/**
*	Auth Body
**/
typedef struct twAuthBody {
	/* Limit to 1 claim */
	char * name;
	char * value;
	uint32_t length;
} twAuthBody;

twAuthBody * twAuthBody_Create();
twAuthBody * twAuthBody_CreateFromStream(twStream * s);
int twAuthBody_Delete(struct twAuthBody * body);
int twAuthBody_SetClaim(struct twAuthBody * body, char * name, char * value);
int twAuthBody_ToStream(struct twAuthBody * body, twStream * s);


/**
*	Bind Body
**/
typedef struct twBindBody {
	char * gatewayName;
	char * gatewayType;
	uint16_t count;
	struct twList * names;
	uint32_t length;
} twBindBody;

twBindBody * twBindBody_Create(char * name);
twBindBody * twBindBody_CreateFromStream(twStream * s);
int twBindBody_Delete(struct twBindBody * body);
int twBindBody_AddName(struct twBindBody * body, char * name);
int twBindBody_ToStream(struct twBindBody * body, twStream * s, char * gatewayName, char * gatewayType);

/**
*	Multipart Body
**/
typedef struct twMultipartBody {
	uint16_t chunkId;
	uint16_t chunkCount;
	uint16_t chunkSize;
	enum entityTypeEnum entityType;
	char * entityName;
	char * data;
	uint16_t length;
} twMultipartBody;

twMultipartBody * twMultipartBody_CreateFromStream(twStream * s, char isRequest);
void twMultipartBody_Delete(void * body);

typedef struct mulitpartMessageStoreEntry {
	uint64_t expirationTime;
	uint32_t id;
	uint16_t chunksExpected;
	uint16_t chunksReceived;
	twMessage ** msgs; /* Array of message pointers */
} mulitpartMessageStoreEntry;

mulitpartMessageStoreEntry * mulitpartMessageStoreEntry_Create(twMessage * msg);
void mulitpartMessageStoreEntry_Delete(void * entry);

/**
* Multipart message cache - this is a singleton
**/
typedef struct twMultipartMessageStore {
	twList * multipartMessageList;
	TW_MUTEX mtx;
} twMultipartMessageStore;

twMultipartMessageStore * twMultipartMessageStore_Instance();
void twMultipartMessageStore_Delete(void * store);
twMessage * twMultipartMessageStore_AddMessage(twMessage * msg);
void twMultipartMessageStore_RemoveStaleMessages();

#ifdef __cplusplus
}
#endif

#endif
