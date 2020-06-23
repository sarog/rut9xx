/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable ThingWorx Binary Messaging layer
 */

#include "twOSPort.h"
#include "twMessages.h"
#include "twLogger.h"
#include "twList.h"
#include "stringUtils.h"
#include "twBaseTypes.h"
#include "twInfoTable.h"
#include "twApi.h"
#include "twOfflineMsgStore.h"
#include "twApiStubs.h"

extern TW_MUTEX twInitMutex;

#define MAX_FIELD_LEN 1024
#define MAX_ENTITY_NAME_LEN 1024

uint32_t globalRequestId = 0;


void twDeflateInit(struct twWs * ws) {
	twMutex_Lock (ws->compressionMutex);

	ws->defstream.zalloc = Z_NULL;
	ws->defstream.zfree = Z_NULL;
	ws->defstream.opaque = Z_NULL;

	memset (&ws->defstream, 0, sizeof (ws->defstream));

	deflateInit2(&ws->defstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, ZLIB_WINDOW_BITS_RAW_DEFLATE, ZLIB_DEFAULT_MEM_LEVEL, Z_DEFAULT_STRATEGY);

	ws->bDeflateInitialized = TRUE;

	twMutex_Unlock (ws->compressionMutex);
}

void twDeflateEnd(struct twWs * ws)	{
	twMutex_Lock (ws->compressionMutex);

	if (ws->bDeflateInitialized) {
		deflateEnd (&ws->defstream);
		ws->bDeflateInitialized = FALSE;
	}

	twMutex_Unlock (ws->compressionMutex);
}

int twCompressBytes (char * buf, uint32_t length, twStream* s, struct twWs * ws) {
	/* Buffer to hold the compressed output chunks */
	const char dst[256];
	uint32_t dstLen = sizeof(dst);
	memset(dst, 0, dstLen);

	if (!s || !ws) {
		return TW_PRECONDITION_FAILED;
	}

	/* Has the compression context been initialized? */
	if (!ws->bDeflateInitialized) {
		twDeflateInit (ws);
	}

	/* Aquire this socket's compression context lock */
	twMutex_Lock (ws->compressionMutex);

	/* Set up the input data */
	ws->defstream.avail_in = length;
	ws->defstream.next_in = (Bytef *)buf;

	/* Run deflate () until the entire stream has been processed */
	do {
		uint32_t chunklen = dstLen;
		int rc = Z_OK;
		/* Assign the output buffer to the deflate context */
		ws->defstream.avail_out = dstLen;
		ws->defstream.next_out = (Bytef *)dst;

		/* Run the deflate */
		rc = deflate(&ws->defstream, Z_SYNC_FLUSH);
		chunklen = dstLen - ws->defstream.avail_out;

		if (rc == Z_OK || rc == Z_STREAM_END) {
			/* Move what has been output so far to the destination stream */
			twStream_AddBytes(s, dst, chunklen);
		} else {
			/* Compression failed */
			twMutex_Unlock (ws->compressionMutex);
			TW_LOG(TW_ERROR, "twCompressStream: Error compressing stream");
			return TW_ERROR_SENDING_MSG;
		}
	} while (ws->defstream.avail_out == 0);

	/* Subtract 4 bytes for the DEFLATE block end bytes 00 00 FF FF. See RFC 7692 section 7.2.1 */
	s->length = s->length - DEFLATE_SYNC_TRAILER_SIZE;

	twMutex_Unlock (ws->compressionMutex);

	return TW_OK;
}

uint32_t twMessage_GetRequestId() {
	uint32_t requestId;

	twMutex_Lock(twInitMutex);
	requestId = ++globalRequestId;
	twMutex_Unlock(twInitMutex);

	return requestId;
}

twMessage * twMessage_Create(enum msgCodeEnum code, uint32_t reqId) {
	twMessage * msg = (twMessage *)TW_CALLOC(sizeof(twMessage), 1);
	if (!msg) {
		TW_LOG(TW_ERROR, "twMessage_Create: Error allocating msg storage");
		return NULL;
	}
	msg->version = TW_MSG_VERSION;
	msg->code = code;
	msg->length = MSG_HEADER_SIZE;
	msg->requestId = (reqId != 0) ? reqId : twMessage_GetRequestId();
	return msg;
}

twMessage * twMessage_CreateRequestMsg(enum msgCodeEnum code) {
	twMessage * msg = 0;
	if (code != TWX_GET && code != TWX_PUT && code != TWX_POST && code != TWX_DEL) {
		TW_LOG(TW_ERROR, "twMessage_CreateRequestMsg: Not  valid code for a request messge"); 
		return NULL;
	}
	msg = twMessage_Create(code, 0);
	if (msg) {
		msg->type = TW_REQUEST;
		msg->body = twRequestBody_Create();
		if (!msg->body) {
			TW_LOG(TW_ERROR, "twMessage_CreateRequestMsg: Error allocating body"); 
			twMessage_Delete(msg);
			return NULL;
		}
	}
	return msg;
}

twMessage * twMessage_CreateResponseMsg(enum msgCodeEnum code, uint32_t id, uint32_t sessionId, uint32_t endpointId)  {
	twMessage * msg = NULL;
	if (code < TWX_SUCCESS) {
		TW_LOG(TW_ERROR, "twMessage_CreateResponseMsg: Not valid code for a response messge"); 
		return NULL;
	}
	msg = twMessage_Create(code, 0);
	if (msg) {
		msg->type = TW_RESPONSE;
		msg->code = code;
		msg->requestId = id;
		msg->endpointId = endpointId;
		msg->sessionId = sessionId;
		msg->body = twResponseBody_Create();
		if (!msg->body) {
			TW_LOG(TW_ERROR, "twMessage_CreateResponseMsg: Error allocating body"); 
			twMessage_Delete(msg);
			return NULL;
		}
	}
	return msg;
}

twMessage * twMessage_CreateBindMsg(char * name, char isUnbind)  {
	twBindBody * b = NULL;
	twMessage * msg = twMessage_Create(isUnbind ? TWX_UNBIND : TWX_BIND, 0);
	if (msg) {
		msg->type = TW_BIND;
		b = twBindBody_Create(name);
		if (!b) {
			twMessage_Delete(msg);
			return NULL;
		} 
		twMessage_SetBody(msg, b);
	}
	return msg;
}

twMessage * twMessage_CreateAuthMsg(char * claimName, char * claimValue)  {
	twAuthBody * b = NULL;
	twMessage * msg = twMessage_Create(TWX_AUTH, 0);
	if (msg) {
		msg->type = TW_AUTH;
		b = twAuthBody_Create();
		if (!b) {
			twMessage_Delete(msg);
			return NULL;
		} 
		twMessage_SetBody(msg, b);
		twAuthBody_SetClaim(b, claimName, claimValue);
	}
	return msg;
}

twMessage * twMessage_CreateFromStream(twStream * s) {
	twMessage * msg;
	unsigned char code;
	if (!s) {
		TW_LOG(TW_ERROR, "twMessage_CreateFromStream: NULL stream pointer"); 
		return 0; 
	}
	msg = (twMessage *)TW_CALLOC(sizeof(twMessage), 1);
	if (!msg) { TW_LOG(TW_ERROR, "twMessage_CreateFromStream: Error allocating msg storage"); return 0; }
	twStream_Reset(s);
	twStream_GetBytes(s, (char *)&msg->version, 1);
	twStream_GetBytes(s, &code, 1);
	msg->code = (enum msgCodeEnum)code;
	twStream_GetBytes(s, &msg->requestId, 4);
	swap4bytes((char *)&msg->requestId);
	twStream_GetBytes(s, &msg->endpointId, 4);
	swap4bytes((char *)&msg->endpointId);
	twStream_GetBytes(s, &msg->sessionId, 4);
	swap4bytes((char *)&msg->sessionId);
	twStream_GetBytes(s, &msg->multipartMarker, 1);
	msg->length = MSG_HEADER_SIZE;
	if (msg->code == TWX_GET || msg->code == TWX_PUT || msg->code == TWX_POST || msg->code == TWX_DEL) {
		msg->type = TW_REQUEST;
		if (!msg->multipartMarker) {
			msg->body = twRequestBody_CreateFromStream(s);
			msg->length += ((twRequestBody *)(msg->body))->length;
		} else {
			msg->type = TW_MULTIPART_REQ;
			msg->body = twMultipartBody_CreateFromStream(s, TRUE);
			msg->length += ((twMultipartBody *)(msg->body))->length;
		}
	} else if (msg->code == TWX_AUTH) {
		msg->type = TW_AUTH;
		msg->body = twAuthBody_CreateFromStream(s);
		msg->length += ((twRequestBody *)(msg->body))->length;
	} else if ((msg->code == TWX_BIND) || (msg->code == TWX_UNBIND)) {
		msg->type = TW_BIND;
		msg->body = twBindBody_CreateFromStream(s);
		msg->length += ((twRequestBody *)(msg->body))->length;
	} else if (msg->code >= TWX_SUCCESS) {
		msg->type = TW_RESPONSE;
		if (!msg->multipartMarker) {
			msg->body = twResponseBody_CreateFromStream(s);
			msg->length += ((twResponseBody *)(msg->body))->length;
		} else {
			msg->type = TW_MULTIPART_RESP;
			msg->body = twMultipartBody_CreateFromStream(s, FALSE);
			msg->length += ((twMultipartBody *)(msg->body))->length;
		}
	} else {
		TW_LOG(TW_ERROR,"twMessage_CreateFromStream: Unhandled message code: %d", msg->code);
		twMessage_Delete(msg);
		return NULL;
	}
	return msg;
}

void twMessage_Delete(void * input) {
	char * type;
	twMessage * msg = (twMessage *) input;
	if (!msg) {
		TW_LOG(TW_ERROR, "twMessage_Delete: NULL msg pointer"); return;
	}
	/* Clean up */
	/* if there already is a body, free it up */
	if (msg->type == TW_REQUEST) {
		if (!msg->multipartMarker) {
			type = "REQUEST";
			twRequestBody_Delete((twRequestBody *)msg->body);
		} else {
			twMultipartBody_Delete(msg->body);
			type = "MULTIPART REQUEST";
		}
	} else if (msg->type == TW_BIND) {
		twBindBody_Delete((twBindBody *)msg->body);
		type = "BIND";
	} else if (msg->type == TW_AUTH) {
		twAuthBody_Delete((twAuthBody *)msg->body);
		type = "AUTH";
	} else if (msg->type >= TW_RESPONSE) {
		if (!msg->multipartMarker) {
			type = "RESPONSE";
			twResponseBody_Delete((twResponseBody *)msg->body);
		} else {
			twMultipartBody_Delete(msg->body);
			type = "MULTIPART RESPONSE";
		}
	} else {
		TW_LOG(TW_ERROR,"twMessage_Delete: Unknown message code: %d", msg->code);
		TW_FREE(msg->body);
		type = "UNKNOWN";
	}
	TW_LOG(TW_DEBUG, "twMessage_Delete:  Deleting %s Message: %d", type, msg->requestId);
	TW_FREE(msg);
	return;
}

int twMessage_SetBody(struct twMessage * msg, void * body) {
	if (!msg) { TW_LOG(TW_ERROR, "twMessage_SetBody: NULL msg pointer"); return -1; }
	/* if there already is a body, free it up */
	if (msg->body) {
		if (msg->type == TW_REQUEST) {
			twRequestBody_Delete((twRequestBody *)msg->body);
		} else if (msg->type == TW_BIND) {
			twBindBody_Delete((twBindBody *)msg->body);
		} else if (msg->type == TW_AUTH) {
			twAuthBody_Delete((twAuthBody *)msg->body);
		} else if (msg->type == TW_RESPONSE) {
			twResponseBody_Delete((twResponseBody *)msg->body);
		} else {
			TW_LOG(TW_ERROR,"twMessage_SetBody: Unknown message code: %d", msg->code);
			TW_FREE(body);
			return TW_INVALID_MSG_CODE;
		}
	}
	msg->body = body;
	return TW_OK;
}

extern twApi * tw_api;
extern twOfflineMsgStore * tw_offline_msg_store;
#define PERSISTED_MSG_SEPERATOR "!twMsg!"

int twMessage_Send(struct twMessage ** msg, struct twWs * ws) {
	char byte;
	char header[MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE];
	twStream * s = NULL;
	twStream * bodyStream = NULL;
	uint32_t length = 0;
	uint32_t bodyBytesRemaining;
	uint32_t tmp;
	uint16_t numChunks = 0;
	uint16_t chunkNumber = 1;
	int res = 0;
	char headerSize = MSG_HEADER_SIZE;
	uint16_t effectiveChunkSize = twcfg.message_chunk_size - MSG_HEADER_SIZE - MULTIPART_MSG_HEADER_SIZE;

	/* If we're using compression, subtract the size of the deflate sync bytes the server will need to add back in on its end. */
	if (!ws->bDisableCompression && ws->bSupportsPermessageDeflate)	{
		effectiveChunkSize -= DEFLATE_SYNC_TRAILER_SIZE;
	}

	/* if there is a message to flush, the request ID of the msg pointer will be stored */
	if(tw_offline_msg_store && tw_offline_msg_store->offlineMsgEnabled && tw_api->isAuthenticated) {
		res = twOfflineMsgStore_HandleRequest(msg, ws, OFFLINE_MSG_STORE_FLUSH);
		if (res && res != TW_WEBSOCKET_NOT_CONNECTED) {
			TW_LOG(TW_ERROR, "twMessage_Send: error flushing offline message store, error: %d", res); 
		}
	}

	if (!msg || !(*msg)->body || !ws) { 
		TW_LOG(TW_ERROR, "twMessage_Send: NULL msg pointer"); 
		return -1; 
	}
	/* Get the length */
	if ((*msg)->type == TW_REQUEST) {
		length = ((twRequestBody *)((*msg)->body))->length;
		effectiveChunkSize -= 2 + strnlen(((twRequestBody *)(*msg)->body)->entityName, MAX_FIELD_LEN);
	} else if ((*msg)->type == TW_BIND) {
		length = ((twBindBody *)((*msg)->body))->length;
		if (ws->gatewayName && ws->gatewayType) length += (strnlen(ws->gatewayName, MAX_FIELD_LEN) + 1 +strnlen(ws->gatewayType, MAX_FIELD_LEN) + 1);
	} else if ((*msg)->type == TW_AUTH) {
		(*msg)->sessionId  = -1;
		length = ((twAuthBody *)((*msg)->body))->length;
	} else if ((*msg)->type == TW_RESPONSE) {
		length = ((twResponseBody *)((*msg)->body))->length;
	} else {
		TW_LOG(TW_ERROR,"twMessage_Send: Unknown message code: %d", (*msg)->code);
		return TW_INVALID_MSG_TYPE;
	}
	/* Create a new stream for the body */
	bodyStream = twStream_Create();
	if (!bodyStream) {
		TW_LOG(TW_ERROR, "twMessage_Send: Error allocating stream"); 
		return TW_ERROR_ALLOCATING_MEMORY; 
	}
	/* Create the header binary representation */
	header[0] = (*msg)->version;
	header[1] = (char)(*msg)->code;
	tmp = (*msg)->requestId;
	swap4bytes((char *)&tmp);
	memcpy(&header[2], (char *)&tmp, 4);
	tmp = (*msg)->endpointId;
	swap4bytes((char *)&tmp);
	memcpy(&header[6], (char *)&tmp, 4);
	if (ws->sessionId && (*msg)->type != TW_AUTH) (*msg)->sessionId = ws->sessionId;
	tmp = (*msg)->sessionId;
	swap4bytes((char *)&tmp);
	memcpy(&header[10], (char *)&tmp, 4);
	/* Check our message size */
	if (length + MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE > twcfg.max_message_size) {
		TW_LOG(TW_ERROR, "twMessage_Send: Message size %d is larger than max message size %d", length + MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE, twcfg.max_message_size); 
		twStream_Delete(bodyStream);
		return TW_ERROR_MESSAGE_TOO_LARGE; 
	}

	if ((length + MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE) > twcfg.message_chunk_size) (*msg)->multipartMarker = TRUE;
	header[14] = (*msg)->multipartMarker;
	/* Log this message before we chunk it up */
	TW_LOG_MSG(*msg, "Sending Msg >>>>>>>>>");
	/* Add the beginning of the body */
	numChunks = length/effectiveChunkSize + 1;
	if ((*msg)->multipartMarker) {
		unsigned char chunkInfo[6];
		chunkInfo[0] = (unsigned char)(chunkNumber / 256);
		chunkInfo[1] = (unsigned char)(chunkNumber % 256);
		chunkInfo[2] = (unsigned char)(numChunks / 256);
		chunkInfo[3] = (unsigned char)(numChunks % 256);
		chunkInfo[4] = (unsigned char)(twcfg.message_chunk_size / 256);
		chunkInfo[5] = (unsigned char)(twcfg.message_chunk_size % 256);
		memcpy(&header[MSG_HEADER_SIZE], chunkInfo, 6);
		headerSize += MULTIPART_MSG_HEADER_SIZE;
	}

	if ((*msg)->type == TW_REQUEST) {
		twRequestBody_ToStream((twRequestBody *)(*msg)->body, bodyStream);
	} else if ((*msg)->type == TW_BIND) {
		twBindBody_ToStream((twBindBody *)(*msg)->body, bodyStream, ws->gatewayName, ws->gatewayType);
	} else if ((*msg)->type == TW_AUTH) {
		twAuthBody_ToStream((twAuthBody *)(*msg)->body, bodyStream);
	} else if ((*msg)->type >= TW_RESPONSE) {
		twResponseBody_ToStream((twResponseBody *)(*msg)->body, bodyStream);
	} 

	/* Start sending the message */
	bodyBytesRemaining = length;
	while (chunkNumber <= numChunks) {
		/* Create a new stream for the body */
		uint16_t size = effectiveChunkSize;
		s = twStream_Create();
		if (!s) {
			TW_LOG(TW_ERROR, "twMessage_Send: Error allocating stream"); 
			twStream_Delete(bodyStream);
			return TW_ERROR_ALLOCATING_MEMORY; 
		}
		twStream_AddBytes(s, header, headerSize);
		if (bodyBytesRemaining <= effectiveChunkSize) size = bodyBytesRemaining;
		if ((*msg)->multipartMarker) {
			/* Adjust the chunk number */
			s->data[MSG_HEADER_SIZE] = (unsigned char)(chunkNumber/256);
			s->data[MSG_HEADER_SIZE + 1] = (unsigned char)(chunkNumber%256);
			/* If this is a request we also need to add the entity info unless this is the first chunk which already has it */
			if (((*msg)->code == TWX_GET || (*msg)->code == TWX_PUT || (*msg)->code == TWX_POST || (*msg)->code == TWX_DEL) && chunkNumber != 1) {
				byte = (char)((twRequestBody *)(*msg)->body)->entityType;
				twStream_AddBytes(s, &byte, 1);
				stringToStream(((twRequestBody *)(*msg)->body)->entityName, s);
			}
		}
		/* Add the data */
		twStream_AddBytes(s,&bodyStream->data[length - bodyBytesRemaining], size);

		/* No point in sending if we haven't been authenticated */
		if (tw_api->isAuthenticated || (*msg)->type == TW_AUTH) {
			/* Send both streams off to the websocket */
			res = s_twWs_SendMessage(ws, twStream_GetData(s), twStream_GetLength(s), 0);
		} else {
			TW_LOG(TW_DEBUG, "twMessage_Send: Not authenticated yet"); 
			res = TW_WEBSOCKET_NOT_CONNECTED;
		}
		if (res) {
			
			if ((*msg)->multipartMarker) {
				TW_LOG(TW_ERROR,"twMessage_Send: Error sending Chunk %d of %d with RequestId %d", chunkNumber, numChunks, (*msg)->requestId);
				/* Want to unmark this as multi part so the calling function can clean up the entire message properly */
				(*msg)->multipartMarker = FALSE;
			} else  TW_LOG(TW_ERROR,"twMessage_Send: Error sending Message with RequestId %d", (*msg)->requestId);
			if ((res == TW_WEBSOCKET_NOT_CONNECTED || res == TW_ERROR_WRITING_TO_WEBSOCKET) && (*msg)->type == TW_REQUEST) {
				TW_LOG(TW_ERROR, "twMessage_Send: Error sending request");
				/* if we are offline store message in offline message store */
				/* writing to the offline message store will result in a twMessage_ZeroCopy, which will 
				* set the msg pointer to NULL and retain ownership of the associated memory within the 
				* offline message store singleton. The message will be free'd when the message is stored */
				
				if(tw_offline_msg_store && tw_offline_msg_store->offlineMsgEnabled) {
					res = s_twOfflineMsgStore_HandleRequest(msg, ws, OFFLINE_MSG_STORE_WRITE);
					if (res) {
						TW_LOG(TW_ERROR, "twMessage_Send: error writing to Offline Message Store, error: %d", res);
					} else {
						res = TW_WROTE_TO_OFFLINE_MSG_STORE;
					}
				}
			}
			twStream_Delete(s);
			twStream_Delete(bodyStream);
			return res; 
		} else {
			if ((*msg)->multipartMarker) TW_LOG(TW_TRACE,"twMessage_Send: Chunk %d of %d with RequestId %d sent successfully", 
				chunkNumber, numChunks, (*msg)->requestId);
			else  TW_LOG(TW_TRACE,"twMessage_Send: Message with RequestId %d sent successfully", (*msg)->requestId);
			twStream_Delete(s);
			bodyBytesRemaining -= size;
		}
		chunkNumber++;
	}
	twStream_Delete(bodyStream);
	/* Reset the multipart marker so deleting the message doesn't get confused */
	(*msg)->multipartMarker = FALSE;
	return TW_OK;
}



void twHeader_Delete(void * value) {
	twHeader * hdr = (twHeader *) value;
	if (hdr) {
		TW_FREE(hdr->name);
		TW_FREE(hdr->value);
	}
}

int twHeader_toStream(twHeader * hdr, twStream * s) {
	int res = TW_UNKNOWN;
	if (hdr && s) {
		res = stringToStream(hdr->name, s);
		res |= stringToStream(hdr->value, s);
	}
	return res;
}

twHeader * twHeader_fromStream(twStream * s) {
	twHeader * hdr;
	int cnt = 0;
	int stringSize = 0;
	if (!s) return 0;
	hdr = (twHeader *)TW_CALLOC(sizeof(twHeader), 1);
	if (!hdr) return 0;

	while (cnt < 2) {
		unsigned char size[4];
		/* Get the first byte to check the size */
		twStream_GetBytes(s, &size[0], 1);
		if (size[0] > 127) {
			/* Need the full 4 bytes */
			twStream_GetBytes(s, &size[1], 3);
			stringSize = size[0] * 0x1000000 + size[1] * 0x10000 + size[2] * 0x100 + size[3];
		} else {
			stringSize = size[0];
		}
		if (cnt) {
			hdr->value = (char *)TW_CALLOC(stringSize + 1, 1);
			if (hdr->value) twStream_GetBytes(s, hdr->value, stringSize);
		} else {
			hdr->name = (char *)TW_CALLOC(stringSize + 1, 1);
			if (hdr->name) twStream_GetBytes(s, hdr->name, stringSize);
		}
		cnt++;
	}
	if (!hdr->name || !hdr->value) {
		TW_LOG(TW_ERROR,"twHeader_fromStream: Error allocating header name or value");
		twHeader_Delete(hdr);
	}
	return NULL;
}

twRequestBody * twRequestBody_Create() {
	twRequestBody * body = (twRequestBody *)TW_CALLOC(sizeof(twRequestBody), 1);
	if (!body) {
		TW_LOG(TW_ERROR, "twRequestBody: Error allocating body");
		return NULL;
	}
	body->headers = twList_Create(twHeader_Delete);
	if (!body->headers) {
		TW_LOG(TW_ERROR, "twRequestBody: Error allocating header list");
		twRequestBody_Delete(body);
		return NULL;
	}
	body->length = 4; /* Ent type + Characteristic TYpe + header count + params type */
	return body;
}

twRequestBody * twRequestBody_CreateFromStream(twStream * s) {
	twRequestBody * body = NULL;
	twPrimitive * prim = NULL;
	char * start = NULL;
	char tmp, i;
	if (!s) {
		TW_LOG(TW_ERROR, "twMessage_CreateFromStream: NULL stream pointer"); 
		return NULL; 
	}
	start = s->ptr;
	body = (twRequestBody *)TW_CALLOC(sizeof(twRequestBody), 1);
	if (!body) {
		TW_LOG(TW_ERROR, "twRequestBody: Error allocating body");
	}
	/* Get the entity type and name */
	twStream_GetBytes(s, &tmp, 1);
	body->entityType = (enum entityTypeEnum)tmp;
	prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
	if (prim && prim->val.bytes.data) {
		body->entityName = twPrimitive_DecoupleStringAndDelete(prim);
	}
	/* Get the characteristic type and name */
	twStream_GetBytes(s, &tmp, 1);
	body->characteristicType = (enum characteristicEnum)tmp;
	prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
	if (prim && prim->val.bytes.data) {
		body->characteristicName = twPrimitive_DecoupleStringAndDelete(prim);
	}
	/* Get the headers */
	twStream_GetBytes(s, &body->numHeaders, 1);
	for (i = 0; i < body->numHeaders; i++) {
		twHeader * hdr = twHeader_fromStream(s);
		if (hdr) twList_Add(body->headers, hdr);
	}
	/* If this has an infotable then parse it */
	twStream_GetBytes(s, &tmp, 1);
	if ((enum BaseType)tmp != TW_INFOTABLE) return body;
	body->params = twInfoTable_CreateFromStream(s);
	body->length = s->ptr - start;
	return body;
}

int twRequestBody_Delete(struct twRequestBody * body) {
	if (!body) {
		return TW_INVALID_PARAM; 
	}
	TW_FREE(body->entityName);
	TW_FREE(body->characteristicName);
	twList_Delete(body->headers);
	twInfoTable_Delete(body->params);
	TW_FREE(body);
	return TW_OK;
}

int twRequestBody_SetParams(struct twRequestBody * body, twInfoTable * params) {
	if (!body || body->params) {
		TW_LOG(TW_ERROR, "twRequestBody_SetParams: NULL body pointer or body already has params"); 
		return TW_INVALID_PARAM; 
	}
	body->params = params; /* We own this pointer now */
	if (params) {
		body->length += params->length;
	} 
	return TW_OK;
}

int twRequestBody_SetEntity(struct twRequestBody * body, enum entityTypeEnum entityType, char * entityName) {
	if (!body || body->entityName || !entityName) {
		TW_LOG(TW_ERROR, "twRequestBody_SetEntity: NULL pointer or body already has an entity"); 
		return TW_INVALID_PARAM; 
	}
	body->entityType = entityType;
	body->entityName = duplicateString(entityName); /* We own this pointer now */
	body->length += strnlen(entityName, MAX_ENTITY_NAME_LEN) + 1;
	return TW_OK;
}

int twRequestBody_SetCharacteristic(struct twRequestBody *body, enum characteristicEnum characteristicType,
									char *characteristicName) {
	if (!body || body->characteristicName || !characteristicName) {
		TW_LOG(TW_ERROR, "twRequestBody_SetCharacteristic: NULL pointer or body already has a characteristic");
		return TW_INVALID_PARAM; 
	}
	body->characteristicType = characteristicType;
	body->characteristicName = duplicateString(characteristicName); /* We own this pointer now */
	body->length += strnlen(characteristicName, MAX_FIELD_LEN) + 1;
	return TW_OK;
}

int twRequestBody_AddHeader(struct twRequestBody * body, char * name, char * value) {
	twHeader * hdr = 0;
	if (!body || !body->headers || !name || !value) {
		TW_LOG(TW_ERROR, "twRequestBody_SetCharacteristic: NULL body, headers pointer, name or value"); 
		return TW_INVALID_PARAM; 
	}
	hdr = (twHeader *)TW_CALLOC(sizeof(twHeader), 1);
	hdr->name = duplicateString(name);
	hdr->value = duplicateString(value);
	body->length = strnlen(name, MAX_FIELD_LEN) + 1 + strnlen(value, MAX_FIELD_LEN) + 1;
	twList_Add(body->headers, hdr);
	body->numHeaders++;
	return TW_OK;
}

int twRequestBody_ToStream(struct twRequestBody * body, twStream * s) {
	char byte;
	if (!body || !s) {
		TW_LOG(TW_ERROR, "twRequestBody_ToStream: NULL body or stream pointer"); 
		return TW_INVALID_PARAM; 
	}
	byte = (char)body->entityType;
	twStream_AddBytes(s, &byte, 1);
	stringToStream(body->entityName, s);
	byte = (char)body->characteristicType;
	twStream_AddBytes(s, &byte, 1);
	stringToStream(body->characteristicName, s);
	twStream_AddBytes(s, &body->numHeaders, 1);
	if (body->headers) {
		ListEntry * le = twList_Next(body->headers, NULL);
		while (le) {
			twHeader_toStream((twHeader *)le, s);
			le = twList_Next(body->headers, le);
		}
	}
	if (body->params) {
		byte = (char)TW_INFOTABLE;
		twStream_AddBytes(s, &byte, 1);
		twInfoTable_ToStream(body->params, s);
	} else {
		byte = (char)TW_NOTHING;
		twStream_AddBytes(s, &byte, 1);
	}
	return TW_OK;
}

/**
*	Response Body
**/
twResponseBody * twResponseBody_Create() {
	twResponseBody * body = (twResponseBody *)TW_CALLOC(sizeof(twResponseBody), 1);
	if (!body) {
		TW_LOG(TW_ERROR, "twResponseBody_Create: Error allocating body");
		return NULL;
	}
	/* Default to NOTHING */
	body->length = 2; /* Reason marker + Content base type */
	twResponseBody_SetContent(body, NULL);
	return body;
}

twResponseBody * twResponseBody_CreateFromStream(twStream * s) {
	twResponseBody * body = NULL;
	twPrimitive * prim = NULL;
	char * start;
	char tmp;
	if (!s) {
		TW_LOG(TW_ERROR, "twResponseBody_CreateFromStream: NULL stream pointer"); 
		return 0; 
	}
	start = s->ptr;
	body = (twResponseBody *)TW_CALLOC(sizeof(twResponseBody), 1);
	if (!body) { TW_LOG(TW_ERROR, "twResponseBody_CreateFromStream: Error allocating body storage"); return NULL; }
	twStream_GetBytes(s, &body->reasonMarker, 1);
	if (body->reasonMarker) {
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		if (prim && prim->val.bytes.data) {
			body->reason = twPrimitive_DecoupleStringAndDelete(prim);
		}
	}
	twStream_GetBytes(s, &tmp, 1);
	body->contentType = (enum BaseType)tmp;
	if (body->contentType != TW_INFOTABLE) return body;
	body->content = twInfoTable_CreateFromStream(s);
	body->length = s->ptr - start;
	return body;
}

int twResponseBody_Delete(struct twResponseBody * body) {
	if (!body) {
		return TW_INVALID_PARAM; 
	}
	if (body->reason) TW_FREE(body->reason);
	twInfoTable_Delete(body->content);
	TW_FREE(body);
	return TW_OK;
}

int twResponseBody_SetContent(struct twResponseBody * body, twInfoTable * content) {
	if (!body || body->content) {
		TW_LOG(TW_ERROR, "twResponseBody_SetContent: NULL body pointeror body already has content"); 
		return TW_INVALID_PARAM; 
	}
	body->content = content;  /* We own this pointer now */
	if (content) {
		body->contentType = TW_INFOTABLE;
		body->length += content->length;
	} else {
		body->contentType = TW_NOTHING;
	}
	return TW_OK;
}

int twResponseBody_SetReason(struct twResponseBody * body, char * reason) {
	if (!body) {
		TW_LOG(TW_ERROR, "twResponseBody_SetReason: NULL body pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (!reason) {
		body->reasonMarker = FALSE;
		return TW_OK;
	}
	TW_FREE(body->reason);
	body->reason = duplicateString(reason);  /* We own this pointer now */
	body->length += strnlen(reason, MAX_FIELD_LEN) + 1;
	return TW_OK;
}

int twResponseBody_ToStream(struct twResponseBody * body, twStream * s) {
	char tempContentType;
	if (!body || !s) {
		TW_LOG(TW_ERROR, "twResponseBody_ToStream: NULL body or stream pointer"); 
		return TW_INVALID_PARAM; 
	}
	twStream_AddBytes(s, &body->reasonMarker, 1);
	if (body->reasonMarker) stringToStream(body->reason, s); 
	tempContentType = (char)body->contentType;
	twStream_AddBytes(s, &tempContentType, 1);
	if (body->contentType == TW_INFOTABLE) twInfoTable_ToStream(body->content, s);
	return TW_OK;
}


/**
*	Auth Body
**/
twAuthBody * twAuthBody_Create() {
	twAuthBody * body = (twAuthBody *)TW_CALLOC(sizeof(twAuthBody), 1);
	if (!body) {
		TW_LOG(TW_ERROR, "twAuthBody_Create: Error allocating body");
		return NULL;
	}
	body->length = 1;
	return body;
}

twAuthBody * twAuthBody_CreateFromStream(twStream * s) {
	twAuthBody * body = NULL;
	twPrimitive * prim = NULL;
	char tmp;
	if (!s) {
		TW_LOG(TW_ERROR, "twAuthBody_CreateFromStream: NULL stream pointer"); 
		return 0; 
	}
	body = (twAuthBody *)TW_CALLOC(sizeof(twAuthBody), 1);
	if (!body) { TW_LOG(TW_ERROR, "twAuthBody_CreateFromStream: Error allocating body storage"); return NULL; }
	twStream_GetBytes(s, &tmp, 1);
	/* We are only supporting one claim */
	if (tmp) {
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		body->name = twPrimitive_DecoupleStringAndDelete(prim);
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		body->value = twPrimitive_DecoupleStringAndDelete(prim);
	}
	body->length = s->ptr - s->data;
	return body;
}

int twAuthBody_Delete(struct twAuthBody * body) {
	if (!body) {
		TW_LOG(TW_ERROR, "twAuthBody_Delete: NULL body or stream pointer"); 
		return TW_INVALID_PARAM; 
	}
	TW_FREE(body->name);
	TW_FREE(body->value);
	TW_FREE(body);
	return TW_OK;
}

int twAuthBody_SetClaim(struct twAuthBody * body, char * name, char * value) {
	if (!body || !name || !value) {
		TW_LOG(TW_ERROR, "twAuthBody_SetClaim: NULL body or name/value pointer"); 
		return TW_INVALID_PARAM; 
	}
	body->name = duplicateString(name); /* We own this pointer now */
	body->value = duplicateString(value); /* We own this pointer now */
	body->length += strnlen(name, MAX_FIELD_LEN) + 1 + strnlen(value, MAX_FIELD_LEN) + 1;
	return TW_OK;
}

int twAuthBody_ToStream(struct twAuthBody * body, twStream * s) {
	char count = 1;
	if (!body || !s) {
		TW_LOG(TW_ERROR, "twAuthBody_ToStream: NULL body or stream pointer"); 
		return TW_INVALID_PARAM; 
	}
	twStream_AddBytes(s, &count, 1);
	stringToStream(body->name, s);
	stringToStream(body->value, s);
	return TW_OK;
}

/**
*	Bind Body
**/
twBindBody * twBindBody_Create(char * name) {
	twBindBody * body = (twBindBody *)TW_CALLOC(sizeof(twBindBody), 1);
	if (!body) {
		TW_LOG(TW_ERROR, "twBindBody_Create: Error allocating body");
		return NULL;
	}
	body->names = twList_Create(0);
	if (!body->names) {
		TW_LOG(TW_ERROR, "twBindBody_Create: Error allocating list");
		twBindBody_Delete(body);
		return NULL;
	}
	body->length = 3; /* Marker for gateway name + count */
	if (name) {
		twList_Add(body->names, duplicateString(name));
		body->count++;
		body->length += strnlen(name, MAX_FIELD_LEN) + 1;
	}
	return body;
}

twBindBody * twBindBody_CreateFromStream(twStream * s) {
	twBindBody * body = NULL;
	twPrimitive * prim = NULL;
	unsigned char tmp;
	uint16_t count = 0;
	if (!s) {
		TW_LOG(TW_ERROR, "twBindBody_CreateFromStream: NULL stream pointer"); 
		return NULL; 
	}
	body = (twBindBody *)TW_CALLOC(sizeof(twBindBody), 1);
	if (!body) { TW_LOG(TW_ERROR, "twBindBody_CreateFromStream: Error allocating body storage"); return NULL; }
	body->names = twList_Create(0);
	if (!body->names) {
		twBindBody_Delete(body);
		TW_LOG(TW_ERROR, "twBindBody_CreateFromStream: Error allocating list");
	}
	twStream_GetBytes(s, &tmp, 1);
	/* Check for a gateway */
	if (tmp) {
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		body->gatewayName = twPrimitive_DecoupleStringAndDelete(prim);
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		body->gatewayType = twPrimitive_DecoupleStringAndDelete(prim);
	}
	/* Get the count */
	twStream_GetBytes(s, &tmp, 1);
	count = tmp * 0x100;
	twStream_GetBytes(s, &tmp, 1);
	count += tmp;
	body->count = count;
	while (count) {
		prim = twPrimitive_CreateFromStreamTyped(s, TW_STRING);
		if (prim) twList_Add(body->names, twPrimitive_DecoupleStringAndDelete(prim));
		count--;
	}
	body->length = s->ptr - s->data;
	return body;
}

int twBindBody_Delete(struct twBindBody * body) {
	if (!body) {
		/* If the bind body is NULL then there is nothing to delete. */
		return TW_OK;
	}
	if (body->gatewayName) TW_FREE(body->gatewayName);
	if (body->gatewayType) TW_FREE(body->gatewayType);
	twList_Delete(body->names);
	TW_FREE(body);
	return TW_OK;
}

int twBindBody_AddName(struct twBindBody *body, char *name) {
	if (!body) {
		TW_LOG(TW_ERROR, "twBindBody_Create: NULL body pointer");
		return TW_INVALID_PARAM;
	}
	if (!body->names) {
		TW_LOG(TW_ERROR, "twBindBody_Create: NULL list point");
		return TW_INVALID_PARAM;
	}
	if (body->length + strnlen(name, MAX_FIELD_LEN) + 1 + MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE >= twcfg.max_message_size) {
		TW_LOG(TW_INFO, "twBindBody_Create: bind name %s would exceed max message size", name);
		return TW_BIND_MESSAGE_FULL;
	}
	if (name) {
		twList_Add(body->names, duplicateString(name));
		body->count++;
		body->length += strnlen(name, MAX_FIELD_LEN) + 1;
	}
	return TW_OK;
}

int twBindBody_ToStream(struct twBindBody * body, twStream * s, char * gatewayName, char * gatewayType) {
	unsigned char tmp = 0;
	ListEntry * entry = NULL;
	char * utf8 = NULL;
	if (!body || !s || !body->names) {
		TW_LOG(TW_ERROR, "twBindBody_ToStream: NULL body or stream pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (gatewayName && gatewayType) {
		/* Add the gateway information */
		tmp = 0x01;
		twStream_AddBytes(s, (char *)&tmp, 1);
		stringToStream(gatewayName, s);
		stringToStream(gatewayType, s);

	} else twStream_AddBytes(s, (char *)&tmp, 1);
	tmp = (unsigned char)(body->count / 0x100);
	twStream_AddBytes(s, (char *)&tmp, 1);
	tmp = (unsigned char)(body->count % 0x100);
	twStream_AddBytes(s, (char *)&tmp, 1);
	entry = twList_Next(body->names, NULL);
	while (entry) {
		utf8 = (char *)entry->value;
		if (utf8) stringToStream(utf8, s);
		entry = twList_Next(body->names, entry);
	}
	return TW_OK;
}

/* Multipart Body */
twMultipartBody * twMultipartBody_CreateFromStream(twStream * s, char isRequest) {
	twMultipartBody * body = (twMultipartBody *)TW_CALLOC(sizeof(twMultipartBody), 1);
	unsigned char tmp[2];
	if (!body || !s) {
		TW_LOG(TW_ERROR, "twMultipartBody_CreateFromStream: Error allocating body or missing stream input");
		return NULL;
	}
	twStream_GetBytes(s, tmp, 2);
	body->chunkId = tmp[0] * 0x100 + tmp[1];
	twStream_GetBytes(s, tmp, 2);
	body->chunkCount = tmp[0] * 0x100 + tmp[1];
	twStream_GetBytes(s, tmp, 2);
	body->chunkSize = tmp[0] * 0x100 + tmp[1];
	if (isRequest) {
		twStream_GetBytes(s, tmp, 1);
		body->entityType = (enum entityTypeEnum)tmp[0];
		body->entityName = streamToString(s);
	}
	/* Allocate storage for the rest of the data in the stream */
	body->length = s->length - (s->ptr - s->data);
	body->data = (char *) TW_CALLOC(body->length, 1);
	if (!body->data) {
		TW_LOG(TW_ERROR,"twMultipartBody_CreateFromStream: Error allocating storage for data");
		twMultipartBody_Delete(body);
		return NULL;
	}
	twStream_GetBytes(s, body->data, body->length);
	return body;
}

void twMultipartBody_Delete(void * body) {
	twMultipartBody * tmp = (twMultipartBody *)body;
	if (!body) return;
	if (tmp->entityName) TW_FREE(tmp->entityName);
	if (tmp->data) TW_FREE(tmp->data);
	TW_FREE(tmp);
}

/**
* Multipart message cache - this is a singleton 
**/
twMultipartMessageStore * mpStore = NULL;

mulitpartMessageStoreEntry * mulitpartMessageStoreEntry_Create(twMessage * msg) {
	mulitpartMessageStoreEntry * tmp = (mulitpartMessageStoreEntry *)TW_CALLOC(sizeof(mulitpartMessageStoreEntry), 1);
	twMultipartBody * mpBody = NULL;
	TW_LOG(TW_TRACE, "mulitpartMessageStoreEntry_Create: Creating message store array.");
	if (!msg || !msg->body) return NULL;
	mpBody = (twMultipartBody *)msg->body;
	/* Make sure the size of this message doesn't exceed our max size */
	if (mpBody->chunkCount * mpBody->chunkSize > twcfg.max_message_size) {
		TW_LOG(TW_ERROR,"mulitpartMessageStoreEntry_Create: Multipart message would exceed maximum message size");
		return NULL;
	}
	/* We want to store the messages in an ordered array for easy reasembly */
	tmp->msgs = (twMessage **)TW_CALLOC(sizeof(twMessage *) * mpBody->chunkCount, 1);
	if (!tmp->msgs) {
		mulitpartMessageStoreEntry_Delete(tmp);
		TW_LOG(TW_ERROR, "mulitpartMessageStoreEntry_Create: Error allocating message store array. request: %d", msg->requestId);
		return NULL;
	}
	tmp->expirationTime = twGetSystemMillisecondCount() + twcfg.stale_msg_cleanup_rate;
	tmp->id = msg->requestId;
	tmp->chunksExpected = mpBody->chunkCount;
	tmp->chunksReceived = 1;
	tmp->msgs[mpBody->chunkId - 1] = msg; /* CHunk IDs are not zero based */
	TW_LOG(TW_TRACE, "mulitpartMessageStoreEntry_Create: Created message store array with chunk %d of %d.", mpBody->chunkId, mpBody->chunkCount);
	return tmp;
}

void mulitpartMessageStoreEntry_Delete(void * entry) {
	uint16_t i = 0;
	mulitpartMessageStoreEntry * tmp = (mulitpartMessageStoreEntry *) entry;
	if (!tmp) return;
	for (i = 0; i < tmp->chunksExpected; i++) {
		if (tmp->msgs[i]) twMessage_Delete(tmp->msgs[i]);
	}
	if (tmp->msgs) TW_FREE(tmp->msgs);
	TW_FREE(tmp);
}

twMultipartMessageStore * twMultipartMessageStore_Instance() {
	/* Check to see if it already exists */
	if (mpStore) {
		return mpStore;
	}
	mpStore = (twMultipartMessageStore *)TW_CALLOC(sizeof(twMultipartMessageStore), 1);
	if (!mpStore) {
		TW_LOG(TW_ERROR,"twMultipartMessageStore_Instance: Error allocating multipart message store");
	} else {
		mpStore->mtx = twMutex_Create();
		mpStore->multipartMessageList = twList_Create(mulitpartMessageStoreEntry_Delete);
		if (!mpStore->mtx || !mpStore->multipartMessageList) {
			TW_LOG(TW_ERROR, "twMultipartMessageStore_Instance: Error allocating memory");
			twMutex_Delete(mpStore->mtx);
			twList_Delete(mpStore->multipartMessageList);
			TW_FREE(mpStore);
			mpStore = 0;
		}
	}
	return mpStore;
}

void twMultipartMessageStore_Delete(void * input) {
	twMultipartMessageStore * tmp = mpStore;
	if (!mpStore) return;
	twMutex_Lock(tmp->mtx);
	/*twMutex_Lock(twInitMutex);*/
	mpStore = NULL;
	/*twMutex_Unlock(twInitMutex);*/
	twMutex_Unlock(tmp->mtx);
	twMutex_Delete(tmp->mtx);
	twList_Delete(tmp->multipartMessageList);
	TW_FREE(tmp);
}

twMessage * twMultipartMessageStore_AddMessage(twMessage * msg) {
	/* Returns the complete message if all chunks have been received */
	ListEntry * le = NULL;
	uint16_t i = 0;
	twMultipartBody * mp = NULL;
	twStream * s = NULL;
	if (!mpStore || !msg || !msg->body){
		TW_LOG(TW_ERROR,"twMultipartMessageStore_AddMessage: No message or message store found");
		return NULL;
	}
	mp = (twMultipartBody *)msg->body;
	twMutex_Lock(mpStore->mtx);
	le = twList_Next(mpStore->multipartMessageList, NULL);
	while (le) {
		twMessage * m = NULL;
		mulitpartMessageStoreEntry * tmp = (mulitpartMessageStoreEntry *)le->value;
		if (tmp && tmp->id == msg->requestId) {
			/* The entry already exists in the list, just add the mesg */
			if (mp->chunkId > tmp->chunksExpected) {
				TW_LOG(TW_ERROR,"twMultipartMessageStore_AddMessage: Chunk Id %d is greater that expected chunks of %d",
					mp->chunkId, tmp->chunksExpected);
				twMessage_Delete(msg);
				twMutex_Unlock(mpStore->mtx);
				return NULL;
			}
			/* Put this message where it belongs */
			tmp->msgs[mp->chunkId - 1] = msg;
			/* Check to see if we have received all the chunks */
			for (i = 0; i < tmp->chunksExpected; i++ ) {
				if (!tmp->msgs[i]) {
					/* We aren't complete, return a NULL */
					twMutex_Unlock(mpStore->mtx);
					TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: msg: %p, Stored Chunk %d  of %d", msg, mp->chunkId, tmp->chunksExpected);
					/* TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: Missing Chunk %d of %d", i + 1, tmp->chunksExpected); */
					return NULL;
				}
			}
			/* If we are here then we have received the entire message */
			m = twMessage_Create(msg->code, msg->requestId);
			s = twStream_Create();
			if (!m || !s) {
				TW_LOG(TW_ERROR,"twMultipartMessageStore_AddMessage: Error allocating memory for complete message");
				twList_Remove(mpStore->multipartMessageList, le, TRUE);
				twMutex_Unlock(mpStore->mtx);
				return NULL;
			}
			TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: Received all %d chunks for message %d", tmp->chunksExpected, msg->requestId);
			m->requestId = msg->requestId;
			m->type = msg->type;
			m->endpointId = msg->endpointId;
			m->sessionId = msg->sessionId;
			/* Reassemble all the chunks */
			for (i = 0; i < tmp->chunksExpected; i++ ) {
				/* Need to add the entity info back in for the first chunk */
				twMultipartBody * b = (twMultipartBody *)tmp->msgs[i]->body;
				if (m->type == TW_MULTIPART_REQ && i == 0) {
					char et = b->entityType;
					twStream_AddBytes(s, &et, 1);
					stringToStream(b->entityName, s);
				}
				twStream_AddBytes(s, b->data, b->length);
				/* Delete the chunk here to preserve RAM  */
				TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: Deleting Multipart Chunk %d", i + 1);
				twMessage_Delete(tmp->msgs[i]);
				tmp->msgs[i] = 0;
			}
			twStream_Reset(s);
			if (m->type == TW_MULTIPART_REQ) {
				m->type = TW_REQUEST;
				m->body = twRequestBody_CreateFromStream(s);
				TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: Reconstituted REQUEST msg %d", m->requestId);
			} else if (m->type == TW_MULTIPART_RESP) {
				m->type = TW_RESPONSE;
				m->body = twResponseBody_CreateFromStream(s);
				TW_LOG(TW_TRACE,"twMultipartMessageStore_AddMessage: Reconstituted RESPONSE msg %d", m->requestId);
			} else {
				TW_LOG(TW_ERROR,"twMultipartMessageStore_AddMessage:Unknown Message type %d for msg %d", m->type, m->requestId);
			}
			twStream_Delete(s);
			/* Remove this entry from the message store */
			twList_Remove(mpStore->multipartMessageList, le, TRUE);
			twMutex_Unlock(mpStore->mtx);
			return m;
		}
		le = twList_Next(mpStore->multipartMessageList, le);
	}
	/* If we are here we didn't find an entry with this request id */
	twList_Add(mpStore->multipartMessageList, mulitpartMessageStoreEntry_Create(msg));
	twMutex_Unlock(mpStore->mtx);
	return NULL;
}

void twMultipartMessageStore_RemoveStaleMessages() {
	ListEntry * le = NULL;
	uint64_t now = twGetSystemMillisecondCount();
	if (!mpStore) return;
	twMutex_Lock(mpStore->mtx);
	le = twList_Next(mpStore->multipartMessageList, NULL);
	while (le) {
		mulitpartMessageStoreEntry * tmp = (mulitpartMessageStoreEntry *)le->value;
		if (twTimeGreaterThan(now, tmp->expirationTime)) {
			TW_LOG(TW_INFO,"Removing stale message with Request Id %d", tmp->id);
			twList_Remove(mpStore->multipartMessageList, le, TRUE);
		}
		le = twList_Next(mpStore->multipartMessageList, le);
	}
	twMutex_Unlock(mpStore->mtx);
}

twMessage * twMessage_ZeroCopy(struct twMessage ** msg) {
	struct twMessage * tmp = NULL;
	tmp = *msg;
	*msg = NULL;
	return tmp;
}

