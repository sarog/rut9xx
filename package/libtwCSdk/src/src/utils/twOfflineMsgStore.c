/*
*  Copyright (C) 2015 ThingWorx Inc.
*
*  Portable ThingWorx Offline Message Store
*/
#include <twApiStubsOn.h>
#include "twOfflineMsgStore.h"
#include "twApiStubs.h"
#include "twMessages.h"

#define OFFLINE_MSG_BIN "offline_msgs.bin"
#define OFFLINE_MSG_BIN_WITH_SLASH "/offline_msgs.bin"
#define OFFLINE_MSG_STORE_FILE_OFFSET 8
#define MAX_PATH_LEN 4096
#define MAX_ENTITY_NAME_LEN 512

extern twApi * tw_api;

/* Singleton API structure */
twOfflineMsgStore * tw_offline_msg_store = NULL;
TW_MUTEX twOfflineInitMutex = NULL;

/** Supporting structure and functions **/

typedef struct OfflineMsgStore_TaskParams {
	enum OfflineRequest request_type;	/**< request type **/
	struct twMessage * msg;				/**< Pointer to message structure. **/
	struct twWs * ws;					/**< Pointer to websocket structure. **/
	int status;
} OfflineMsgStore_TaskParams;

/** free memory associated with OfflineMsgStore_TaskParams **/
int twOfflineMsgStore_DeleteTaskParams(OfflineMsgStore_TaskParams ** params) {
	OfflineMsgStore_TaskParams * tmp = *params;
	/* validate inputs */
	if (!params) {
		TW_LOG(TW_WARN,"twOfflineMsgStore_DeleteTaskParams: params is already NULL");
	} else {
		/* delete the message */
		if (tmp->msg) {
			twMessage_Delete(tmp->msg);
			tmp->msg = NULL;
		}
		TW_FREE(tmp);
	}
	/* return result */
	return TW_OK;
}

/** allocates memory for OfflineMsgStore_TaskParams, calling function is responsible for memory management **/
int twOfflineMsgStore_CreateTaskParams(twMessage ** msg, twWs * ws, enum OfflineRequest request_type, OfflineMsgStore_TaskParams ** params){
	int res = TW_OK;
	OfflineMsgStore_TaskParams * tmp = NULL;

	/* check params */
	if (!msg || !*msg || !params) {
		TW_LOG(TW_ERROR, "twOfflineMsgStore_CreateTaskParams: invalid inputs");
		res = TW_INVALID_PARAM;
	}
	if (!res) {
		/* allocate memory */
		tmp = (OfflineMsgStore_TaskParams*)TW_CALLOC(sizeof(OfflineMsgStore_TaskParams), 1);
		if (!tmp) {
			TW_LOG(TW_ERROR, "twOfflineMsgStore_CreateTaskParams: error allocating memory for task params");
			res = TW_ERROR_ALLOCATING_MEMORY;
		}
		if (!res) {
			/* set message params */
			tmp->request_type = request_type;
			tmp->ws = ws;
			tmp->status = TW_OK;

			/* copy/create message */
			switch (request_type) {
			case OFFLINE_MSG_STORE_FLUSH:
				tmp->msg = twMessage_Create((*msg)->code, (*msg)->requestId);
				tmp->msg->type = (*msg)->type;
				break;
			case OFFLINE_MSG_STORE_WRITE:
				tmp->msg = twMessage_ZeroCopy(msg);
				break;
			default:
				/* if there is a bad request type, cleanup params and set return value */
				TW_LOG(TW_ERROR, "twOfflineMsgStore_CreateTaskParams, Unrecognized task type: %ud", request_type);
				twOfflineMsgStore_DeleteTaskParams(&tmp);
				res = TW_INVALID_PARAM;
				break;
			}
			if (!res && !tmp->msg) {
				/* if message was not created cleanup and set return value */
				TW_LOG(TW_ERROR, "twOfflineMsgStore_CreateTaskParams: error allocating memory for task params message");
				twOfflineMsgStore_DeleteTaskParams(&tmp);
				res = TW_ERROR_ALLOCATING_MEMORY;
			}

			*params = tmp;
		}
	}
	return res;
}

/*
 * getMessageRequestId
 *
 * Get the request ID for the message.
 *
 * NOTE: getMessageRequestId is intended to be used only by
 * twOfflineMsgStore_Flush(). Multiple consumers of this function will result
 * in strange side effects due to the static variable, current_requestID.
 */
uint32_t getMessageRequestId(char * msg) {
	/*
	 * We only want a new request ID if:
	 *   * msg is not a multipart message.
	 *   * msg is a multipart message and the current chunk is 1.
	 *
	 * Message header format:
	 *
	 *   Byte(s) Field
	 *   ======= =====
	 *      0    Version
	 *      1    Method/Code
	 *     2-5   Request ID
	 *     6-9   Endpoint ID
	 *    10-13  Session ID
	 *     14    Multipart Marker
	 *
	 *  Multipart header format (byte offset from beginning of main header):
	 *
	 *    Byte(s) Field
	 *    ======= =====
	 *     15-16  Current Chunk
	 *     17-18  Total Chunks
	 *     19-20  Chunk Size
	 */
	static uint32_t current_requestID = 0;
	uint8_t multipart_msg = * (uint8_t *) (msg + 14);
	uint16_t current_chunk;

	memcpy(&current_chunk, (msg + 15), sizeof(current_chunk));
    swap2bytes((char *) &current_chunk);

	if (!multipart_msg || 1 == current_chunk) {
		/* New request ID needed. */
		current_requestID = twMessage_GetRequestId();
	}
	return current_requestID;
}

/** flushes the offline message store **/
int twOfflineMsgStore_Flush(struct twMessage * msg, struct twWs * ws){
	uint32_t tmp;
	int res = TW_OK;
	twStream* s = NULL;

	/* Check to see if offline message store is enabled, we have some queued up messages, and we are online */
	if (tw_offline_msg_store && tw_offline_msg_store->offlineMsgEnabled && tw_api && tw_api->isAuthenticated) {
		/* We are going to have to insert the current session ID into the message */
		uint32_t session = ws->sessionId;
		swap4bytes((char *)&session);

		/* check if we are storing these messages on disk or in RAM */
		if (tw_offline_msg_store->offlineMsgFile) {
			/* Persisted offline message store */
			int64_t persistMsgLen = twcfg.message_chunk_size + MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE + strnlen(PERSISTED_MSG_SEPARATOR, 7);
			char * buf = (char *) TW_CALLOC(persistMsgLen,1);
			size_t bytesRead = 0;
			uint32_t msgLength = 0;
			int64_t curPos = 0;
			TW_FILE_HANDLE f = 0;
			res = TW_OK;
			TW_LOG(TW_TRACE,"twOfflineMsgStore_Flush: Creating buffer for reading offline message store. size = %lli", persistMsgLen);
			if (buf) {
				/* Find the first offline message separator in the file */
				f = TW_FOPEN(tw_offline_msg_store->offlineMsgFile, "rb");
				while (TW_FREAD(buf + curPos, 1, 1, f) == 1 && curPos < persistMsgLen - 1) {
					if (!strstr(buf, PERSISTED_MSG_SEPARATOR)) {
						if (curPos++ < persistMsgLen - 1) continue;
						else {
							TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Couldn't find a persisted message separator in the offline msg store file");
							res = TW_INVALID_MSG_STORE_DIR;
							break;
						}
					}
					/* If we are here we have found the separator */
					TW_LOG(TW_TRACE,"twOfflineMsgStore_Flush: Found end of persisted message separator at index %lli", curPos);
					/* Read in the length */
					msgLength = 0;
					if (TW_FREAD(&msgLength, 1, sizeof(uint32_t), f) != sizeof(uint32_t)) {
						TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Error reading persisted message length from file");
						res = TW_INVALID_MSG_STORE_DIR;
						break;
					}
					TW_LOG(TW_TRACE,"twOfflineMsgStore_Flush: Got persisted message length of %d", msgLength);
					if (msgLength >= persistMsgLen) {
						TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Error persisted message length greater than buffer size");
						res = TW_ERROR_MESSAGE_TOO_LARGE;
						break;
					}
					bytesRead = TW_FREAD(buf, 1, msgLength, f);
					if (bytesRead != msgLength) {
						TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Error reading persisted message from file");
						res = TW_INVALID_MSG_STORE_DIR;
						break;
					}
					curPos = TW_FTELL(f);
					/* Insert a new request ID */
					tmp = getMessageRequestId(buf);
					swap4bytes((char *)&tmp);
					memcpy(&buf[2], (char *)&tmp, 4);
					/* Insert our session ID */
					memcpy(&buf[10], (char *)&session, 4);
					/* we now have the msg in the buffer.  Send it off */
					TW_LOG(TW_DEBUG,"twOfflineMsgStore_Flush: Sending persisted message");

					res = s_twWs_SendMessage(ws, buf, msgLength, 0);

					if (res == TW_WEBSOCKET_NOT_CONNECTED) {
						/* Back off our current position to point to the beginning of this msg again */
						curPos = curPos - msgLength - strnlen(PERSISTED_MSG_SEPARATOR, 7);
						break;
					}
					tw_offline_msg_store->offlineMsgSize -= msgLength;
					/* Rest curPos to check for another message */
					curPos = 0;
					memset(buf, 0, persistMsgLen);
				}
				if (!res || res == TW_WEBSOCKET_NOT_CONNECTED) {
					/* Need to clean up our file now */
					if (tw_offline_msg_store->offlineMsgSize <= 0 || curPos <= 1) {
						TW_FCLOSE(f);
						f = TW_FOPEN(tw_offline_msg_store->offlineMsgFile,"w");
						if (f) TW_FCLOSE(f);
						tw_offline_msg_store->offlineMsgSize = 0;
					} else {
						TW_FILE_HANDLE h = 0;
						size_t len = 0;
						char * tmpFile = NULL;
						len = strnlen(tw_offline_msg_store->offlineMsgFile, MAX_PATH_LEN) + OFFLINE_MSG_STORE_FILE_OFFSET;
						tmpFile = (char *)TW_CALLOC(len,sizeof(char));
						if (tmpFile) {
							strncpy(tmpFile, tw_offline_msg_store->offlineMsgFile, len);
							strncat(tmpFile, ".tmp", OFFLINE_MSG_STORE_FILE_OFFSET);
							h = TW_FOPEN(tmpFile,"wb");
							if (h) {
								TW_FSEEK(f, curPos, SEEK_SET);
								while ((bytesRead = TW_FREAD(buf, 1, persistMsgLen, f)) > 0) {
									TW_FWRITE(buf, 1, bytesRead, h);
								}
								TW_FCLOSE(f);
								TW_FCLOSE(h);
								/* Rename the file */
								twDirectory_MoveFile(tmpFile, tw_offline_msg_store->offlineMsgFile);
							} else {
								TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Opening file %s", tmpFile);
								res = TW_INVALID_MSG_STORE_DIR;
							}
							TW_FREE(tmpFile);
						} else {
							TW_LOG(TW_ERROR,"twOfflineMsgStore_Flush: Error allocating temp filename");
							TW_FCLOSE(f);
						}
					}
					TW_FREE(buf);
				}
			}
		} else if (tw_offline_msg_store->offlineMsgList) {
			/* Memory resident offline message store */
			ListEntry * le = NULL;
			le = twList_Next(tw_offline_msg_store->offlineMsgList, NULL);
			while (le && le->value) {
				ListEntry * tmp = le;
				twStream * ps = (twStream *)le->value;
				/* Insert our session ID */
				if (ps->data) {
					memcpy(&ps->data[10], (char *)&session, 4);
					res = twWs_SendMessage(ws, twStream_GetData(ps), twStream_GetLength(ps), 0);
					tw_offline_msg_store->offlineMsgSize -= ps->length;
				}
				else {
					TW_LOG(TW_ERROR, "twOfflineMsgStore_Flush: NULL pointer in stored message.");
				}
				/* if there is an error sending, stop processing the list, otherwise delete the message */
				if (res) break;
				le = twList_Next(tw_offline_msg_store->offlineMsgList, le);
				twList_Remove(tw_offline_msg_store->offlineMsgList, tmp, TRUE);
			}
		} else {
			TW_LOG(TW_ERROR, "twOfflineMsgStore_Flush: NULL Offline Message storage name found.  No persisted messsges to send.");
			res = TW_INVALID_MSG_STORE_DIR;
		}
	} else {
		if (tw_api && !tw_api->isAuthenticated) {
			res = TW_WEBSOCKET_NOT_CONNECTED;
		} else {
			TW_LOG(TW_ERROR, "twOfflineMsgStore_Flush: tw_offline_msg_store singleton not initialized or disabled.");
			res = TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
		}
	}
	return res;
}

/** attempts to store a message in the offline message store **/
int twOfflineMsgStore_Write(struct twMessage * msg, struct twWs * ws) {
	char byte;
	char stream_added = FALSE;
	char header[MSG_HEADER_SIZE + MULTIPART_MSG_HEADER_SIZE];
	twStream * s = NULL;
	twStream * bodyStream = NULL;
	size_t length = 0;
	size_t bodyBytesRemaining;
	size_t tmp;
	size_t numChunks = 0;
	size_t chunkNumber = 1;
	int res = 0;
	char headerSize = MSG_HEADER_SIZE;
	size_t effectiveChunkSize = twcfg.message_chunk_size - MSG_HEADER_SIZE - MULTIPART_MSG_HEADER_SIZE;

	/* If we're using compression, subtract the size of the deflate sync bytes the server will need to add back in on its end. */
	if (!ws->bDisableCompression && ws->bSupportsPermessageDeflate)	{
		effectiveChunkSize -= DEFLATE_SYNC_TRAILER_SIZE;
	}

	/* check params */
	if (!msg || !msg->body || !ws) {
		TW_LOG(TW_ERROR, "twOfflineMessageStore_Write: invalid params");
		return TW_INVALID_PARAM;
	}
	/* check offline message store singleton and if it is enabled */
	if (!tw_offline_msg_store || !tw_offline_msg_store->offlineMsgEnabled){
		TW_LOG(TW_ERROR, "twOfflineMessageStore_Write: offline message store not enabled or initialized");
		return TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;

	}
	/* Get the length */
	if (msg->type == TW_REQUEST) {
		length = ((twRequestBody *)(msg->body))->length;
		effectiveChunkSize -= 2 + strnlen(((twRequestBody *)msg->body)->entityName, MAX_ENTITY_NAME_LEN);
	} else if (msg->type == TW_BIND) {
		length = ((twBindBody *)(msg->body))->length;
		if (ws->gatewayName && ws->gatewayType) length += (strnlen(ws->gatewayName, MAX_ENTITY_NAME_LEN) + 1 +strnlen(ws->gatewayType, MAX_ENTITY_NAME_LEN) + 1);
	} else if (msg->type == TW_AUTH) {
		msg->sessionId  = -1;
		length = ((twAuthBody *)(msg->body))->length;
	} else if (msg->type == TW_RESPONSE) {
		length = ((twResponseBody *)(msg->body))->length;
	} else {
		TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Unknown message code: %d", msg->code);
		return TW_INVALID_MSG_TYPE;
	}

	/* Create a new stream for the body */
	bodyStream = twStream_Create();
	if (!bodyStream) {
		TW_LOG(TW_ERROR, "twOfflineMessageStore_Write: Error allocating stream");
		return TW_ERROR_ALLOCATING_MEMORY;
	}

	/* Create the header binary representation */
	header[0] = msg->version;
	header[1] = (char)msg->code;
	tmp = msg->requestId;
	swap4bytes((char *)&tmp);
	memcpy(&header[2], (char *)&tmp, 4);
	tmp = msg->endpointId;
	swap4bytes((char *)&tmp);
	memcpy(&header[6], (char *)&tmp, 4);
	if (ws->sessionId && msg->type != TW_AUTH) msg->sessionId = ws->sessionId;
	tmp = msg->sessionId;
	swap4bytes((char *)&tmp);
	memcpy(&header[10], (char *)&tmp, 4);
	/* Check our message size */
	if (length + MSG_HEADER_SIZE > twcfg.max_message_size) {
		TW_LOG(TW_ERROR, "twOfflineMessageStore_Write: Message size %d is larger than max message size %d", length + MSG_HEADER_SIZE, twcfg.max_message_size);
		twStream_Delete(bodyStream);
		return TW_ERROR_MESSAGE_TOO_LARGE;
	}
	if (length + MSG_HEADER_SIZE > twcfg.message_chunk_size) msg->multipartMarker = TRUE;
	header[14] = msg->multipartMarker;

	/* Add the beginning of the body */
	numChunks = length/effectiveChunkSize + 1;
	if (msg->multipartMarker) {
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
	if (msg->type == TW_REQUEST) {
		twRequestBody_ToStream((twRequestBody *)msg->body, bodyStream);
	} else if (msg->type == TW_BIND) {
		twBindBody_ToStream((twBindBody *)msg->body, bodyStream, ws->gatewayName, ws->gatewayType);
	} else if (msg->type == TW_AUTH) {
		twAuthBody_ToStream((twAuthBody *)msg->body, bodyStream);
	} else if (msg->type >= TW_RESPONSE) {
		twResponseBody_ToStream((twResponseBody *)msg->body, bodyStream);
	}

	/* Start writing the message to offline message store */
	bodyBytesRemaining = length;
	while (chunkNumber <= numChunks) {
		/* Create a new stream for the body */
		uint16_t size = effectiveChunkSize;
		s = twStream_Create();
		if (!s) {
			TW_LOG(TW_ERROR, "twOfflineMessageStore_Write: Error allocating stream");
			twStream_Delete(bodyStream);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		res = twStream_AddBytes(s, header, headerSize);
		if (bodyBytesRemaining <= effectiveChunkSize) size = bodyBytesRemaining;
		if (msg->multipartMarker) {
			/* Adjust the chunk number */
			s->data[MSG_HEADER_SIZE] = (unsigned char)(chunkNumber/256);
			s->data[MSG_HEADER_SIZE + 1] = (unsigned char)(chunkNumber%256);
			/* If this is a request we also need to add the entity info unless this is the first chunk which already has it */
			if (!res && (msg->code == TWX_GET || msg->code == TWX_PUT || msg->code == TWX_POST || msg->code == TWX_DEL) && chunkNumber != 1) {
				byte = (char)((twRequestBody *)msg->body)->entityType;
				res = twStream_AddBytes(s, &byte, 1);
				stringToStream(((twRequestBody *)msg->body)->entityName, s);
			}
		}
		/* Add the data */
		if (!res) res = twStream_AddBytes(s,&bodyStream->data[length - bodyBytesRemaining], size);
		if (res) {
			twStream_Delete(s);
			twStream_Delete(bodyStream);
			return res;
		}
		/* Check to see that we don't exceed its max size */
		if (tw_offline_msg_store->offlineMsgSize + twStream_GetLength(s) < twcfg.offline_msg_queue_size) {
			/* Persisted offline message store */
			if (tw_offline_msg_store->offlineMsgFile && tw_offline_msg_store->onDisk) {
				size_t sepLength = strnlen(PERSISTED_MSG_SEPARATOR, 7);
				TW_FILE_HANDLE f = TW_FOPEN(tw_offline_msg_store->offlineMsgFile, "a+b");
				if (!f) {
					TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Error opening offline msg file %s", tw_offline_msg_store->offlineMsgFile);
					msg->multipartMarker = FALSE;
					twStream_Delete(s);
					twStream_Delete(bodyStream);
					return TW_ERROR_WRITING_OFFLINE_MSG_STORE;
				}
				/* Messages are delimited by <PERSISTED_MSG_SEPARATOR><stream length> */
				/* Write the separator */
				if (TW_FWRITE(PERSISTED_MSG_SEPARATOR, 1, sepLength, f) != sepLength) {
					TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Error storing message in offline msg file. RequestId %d", msg->requestId);
					TW_FCLOSE(f);
					msg->multipartMarker = FALSE;
					twStream_Delete(s);
					twStream_Delete(bodyStream);
					return TW_ERROR_WRITING_OFFLINE_MSG_STORE;
				}
				/* Write the length */
				if (TW_FWRITE(&s->length, 1, sizeof(s->length), f) != sizeof(s->length)) {
					TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Error storing message in offline msg file. RequestId %d", msg->requestId);
					TW_FCLOSE(f);
					msg->multipartMarker = FALSE;
					twStream_Delete(s);
					twStream_Delete(bodyStream);
					return TW_ERROR_WRITING_OFFLINE_MSG_STORE;
				} else {
					if (TW_FWRITE(s->data, 1, s->length, f) != s->length) {
						TW_LOG(TW_DEBUG,"twOfflineMessageStore_Write: Error storing message in offline msg file. RequestId %d", msg->requestId);
					} else {
						TW_LOG(TW_DEBUG,"twOfflineMessageStore_Write: Stored message in offline msg file. RequestId %d", msg->requestId);
						tw_offline_msg_store->offlineMsgSize += twStream_GetLength(s);
					}
					TW_FCLOSE(f);
					if (msg->multipartMarker) TW_LOG(TW_TRACE,"twMessage_Send: Chunk %d of %d with RequestId %d stored in offline message store",
						chunkNumber, numChunks, msg->requestId);
					else  TW_LOG(TW_TRACE,"twMessage_Store: Message with RequestId %d stored in offline message store", msg->requestId);
					twStream_Delete(s);
					s = NULL;
					bodyBytesRemaining -= size;
					chunkNumber++;
					continue;
				}
			} else if (tw_offline_msg_store->offlineMsgList) {
				/* Memory resident offline message store */
				if (twList_Add(tw_offline_msg_store->offlineMsgList, s)) {
					TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Error storing message in offline msg queue. RequestId %d", msg->requestId);
					msg->multipartMarker = FALSE;
					twStream_Delete(s);
					twStream_Delete(bodyStream);
					return TW_ERROR_WRITING_OFFLINE_MSG_STORE;
				} else {
					/* set flag to prevent stream delete after listAdd */
					stream_added = TRUE;

					tw_offline_msg_store->offlineMsgSize += twStream_GetLength(s);
					if (msg->multipartMarker) {
						TW_LOG(TW_TRACE,"twMessage_Send: Chunk %d of %d with RequestId %d stored in offline message queue", chunkNumber, numChunks, msg->requestId);
					} else {
						TW_LOG(TW_TRACE,"twOfflineMessageStore_Write: Message with RequestId %d stored in offline message queue", msg->requestId);
					}

					bodyBytesRemaining -= size;
					chunkNumber++;
					continue;
				}
			} else {
				TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Persisted offline msg store enabled but NULL offline msg file name found.  MESSAGE WILL BE LOST");
			}
		} else {
			TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: message size: %ud exceeds space available in offline message store: %llu, message will be lost", twStream_GetLength(s), twcfg.offline_msg_queue_size - tw_offline_msg_store->offlineMsgSize);
			twStream_Delete(s);
			twStream_Delete(bodyStream);
			/* Reset the multipart marker so deletting the message doesn't get confused */
			msg->multipartMarker = FALSE;
			return TW_ERROR_OFFLINE_MSG_STORE_FULL;
		}
		if (msg->multipartMarker) {
			TW_LOG(TW_ERROR,"twOfflineMessageStore_Write: Error sending Chunk %d of %d with RequestId %d", chunkNumber, numChunks, msg->requestId);
			/* Want to unmark this as multi part so the calling function can clean up the entire message properly */
			msg->multipartMarker = FALSE;
		}

		chunkNumber++;
	}
	if (s && !stream_added) {
		twStream_Delete(s);
		s = NULL;
	}
	twStream_Delete(bodyStream);
	/* Reset the multipart marker so deleting the message doesn't get confused */
	msg->multipartMarker = FALSE;
	return TW_OK;
}

/** initializes in memory offline message store list **/
int twOfflineMsgStore_InitList() {
	if (!tw_offline_msg_store) {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: tw_offline_msg_store is not initialized");
		return TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	} else if (tw_offline_msg_store->onDisk) {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: tw_offline_msg_store is not stored in RAM");
		return TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	}

	/* create list */
	tw_offline_msg_store->offlineMsgList = twList_Create(twStream_Delete);

	/* validate existence */
	if (!tw_offline_msg_store->offlineMsgList) {
		TW_LOG(TW_ERROR,"twOfflineMsgStore: Error initializing offline message store list");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	return TW_OK;
}

/** sets the directory where the offline message store file will be created **/
int twOfflineMsgStore_SetDir(const char *dir) {
	int res = TW_OK;

	/* check singleton and params */
	if (!tw_offline_msg_store) {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: tw_offline_msg_store is not initialized");
		res =  TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	} else if (!tw_offline_msg_store->onDisk) {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: tw_offline_msg_store is not stored on disk");
		res =  TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	} else if (!dir) {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: NULL directory string");
		res =  TW_INVALID_PARAM;
	} else {
		/* create variable to save the old offline msg store in case something fails */
		char * old_offline_msg_store_dir = NULL;

		/* Make sure any existing file is not empty */
		if (tw_offline_msg_store->offlineMsgFile) {
			uint64_t size = 0;
			DATETIME lastModified;
			char isDir;
			char isReadOnly;
			char path_match_found = FALSE;
			/* Are we changing the directory at all?  If not just return success */

			/* set an offset if the input directory does not contain a terminating '/' to the path
			the original directory stored in tmp WILL have the slash, so we need to offset the
			string length one character IF the input directory does NOT contain the '/' */
			/** access the last character of the string using the strnlen -1 (null terminator)
			then compare it directly to '/', set offset 0 if the condition is true, 1 otherwise**/
			int offset;
			int last_character_in_string = strnlen(dir, MAX_PATH_LEN) - 1;
			if('/' == dir[last_character_in_string]) {
				offset = 0;
			} else {
				offset = 1;
			}

			/* save previous directory */
			old_offline_msg_store_dir = tw_offline_msg_store->offlineMsgFile;
			if (!old_offline_msg_store_dir) {
				TW_LOG(TW_ERROR,"twOffLineMsgStore_SetDir: could not save original offline message store directory: %s",tw_offline_msg_store->offlineMsgFile);
				res = TW_ERROR_ALLOCATING_MEMORY;
			}

			/* compare paths for the length of the input directory
			then compare lengths to make sure we are not missing
			any additional directories in the original directory */
			{
				/* ints to hold path sizes */
				int old_offline_msg_store_dir_length = 0;
				int offline_bin_filename_length = 0;
				int new_offline_msg_store_dir_length = 0;

				/* ints to hold lengths with offests */
				int old_offline_msg_store_dir_length_with_offset_and_filename_removed = 0;

				/* booleans to hold comparisons */
				char old_directory_contains_new_directory = FALSE;
				char old_directory_length_matches_new_directory = FALSE;


				/* get lengths */
				old_offline_msg_store_dir_length = strnlen(old_offline_msg_store_dir, MAX_PATH_LEN);
				offline_bin_filename_length = strnlen(OFFLINE_MSG_BIN, MAX_PATH_LEN);
				new_offline_msg_store_dir_length = strnlen(dir, MAX_PATH_LEN);

				/* calculate adjusted length factoring in offset and filename */
				old_offline_msg_store_dir_length_with_offset_and_filename_removed = old_offline_msg_store_dir_length - offline_bin_filename_length - offset;

				/* compare strings */
				old_directory_contains_new_directory = !strncmp(old_offline_msg_store_dir, dir, strnlen(dir, MAX_PATH_LEN));

				/* compare lengths */
				old_directory_length_matches_new_directory = (old_offline_msg_store_dir_length_with_offset_and_filename_removed) == (new_offline_msg_store_dir_length);

				/* if strings and lengths match, set to true */
				path_match_found = old_directory_contains_new_directory && old_directory_length_matches_new_directory;
			}
			if (path_match_found) {
				/* The new dir is the same as the old - nothing to do except set the sizes */
				/* Get the size of the file in case there are some persisted messages */
				res = twDirectory_GetFileInfo(dir, &size, &lastModified, &isDir, &isReadOnly);
				if( TW_OK == res ) {
					tw_offline_msg_store->offlineMsgSize = size;
					TW_LOG(TW_DEBUG, "twOffLineMsgStore_SetDir: New dir %s is same as the original", dir);
				}
			} else {
				/* the directories are not the same */
				/* Get the size of the file in case there are some persisted messages.  Error out if its non-zero in length */
				res = twDirectory_GetFileInfo(old_offline_msg_store_dir, &size, &lastModified, &isDir, &isReadOnly);
				if (res){
					TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Error accessing file: %s", tw_offline_msg_store->offlineMsgFile);
				} else if (size > 0) {
					TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Existing offline message store file %s is not empty", tw_offline_msg_store->offlineMsgFile);
					res = TW_MSG_STORE_FILE_NOT_EMPTY;
				}
			}
		}
		if (!res) {
			/* Create the directory and the files if needed */
			if (twDirectory_CreateDirectory((char *)dir)) {
				TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Error creating offline message directory %s",dir);
				res = TW_INVALID_MSG_STORE_DIR;
			} else {

				tw_offline_msg_store->offlineMsgFile = (char *)TW_CALLOC(strnlen(dir, MAX_PATH_LEN) + strnlen(OFFLINE_MSG_BIN, MAX_PATH_LEN) + 2, 1);
				if (tw_offline_msg_store->offlineMsgFile) {
					uint64_t size = 0;
					DATETIME lastModified;
					char isDir;
					char isReadOnly;
					strcpy(tw_offline_msg_store->offlineMsgFile, dir);
					strcat(tw_offline_msg_store->offlineMsgFile,"/");
					strcat(tw_offline_msg_store->offlineMsgFile, OFFLINE_MSG_BIN);
					/* If the file doesn't exist, create it */
					if (!twDirectory_FileExists(tw_offline_msg_store->offlineMsgFile)) {
						if (twDirectory_CreateFile(tw_offline_msg_store->offlineMsgFile)) {
							TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Error creating offline message file %s", tw_offline_msg_store->offlineMsgFile);
							/* free bad tw_offline_msg_store->offlineMsgFile path */
							TW_FREE(tw_offline_msg_store->offlineMsgFile);
							tw_offline_msg_store->offlineMsgFile = NULL;
							res = TW_ERROR_WRITING_FILE;
						} else {
							TW_LOG(TW_INFO, "twOffLineMsgStore_SetDir: Created offline message file %s", tw_offline_msg_store->offlineMsgFile);
						}
					} else {
						/* Get the size of the file in case there are some persisted messages */
						res = twDirectory_GetFileInfo(tw_offline_msg_store->offlineMsgFile, &size, &lastModified, &isDir, &isReadOnly);

						/* if there is an error with the file, reset the path to the old tw_offline_msg_store->offlineMsgFile path */
						if (res) {
							TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Error reading from offline message file %s", tw_offline_msg_store->offlineMsgFile);
							/* free bad tw_offline_msg_store->offlineMsgFile path */
							TW_FREE(tw_offline_msg_store->offlineMsgFile);
							tw_offline_msg_store->offlineMsgFile = NULL;
							res = TW_ERROR_WRITING_FILE;
						}
					}
				} else {
					TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: Error allocating offline message file name");
					res = TW_ERROR_ALLOCATING_MEMORY;
				}
				/* if there are no errors delete and free old tw_offline_msg_store->offlineMsgFile memory */
				if (!res) {
					/* Get rid of the existing file */
					twDirectory_DeleteFile(old_offline_msg_store_dir);
					TW_FREE(old_offline_msg_store_dir);
				} else {
					tw_offline_msg_store->offlineMsgFile = old_offline_msg_store_dir;
				}
			}
		}
	}
	return res;
}

/**
* simple function to switch between in ram storage or on disk
* storage during offline message store initialization
* NOTE: this assumes enabled = TRUE
**/
int twOfflineMsgStore_SetStorage(const char * filePath){
	int res = TW_OK;
	if(tw_offline_msg_store) {
		if(tw_offline_msg_store->onDisk) {
			res = twOfflineMsgStore_SetDir(filePath);
		} else {
			res = twOfflineMsgStore_InitList();
		}
	} else {
		TW_LOG(TW_ERROR, "twOffLineMsgStore_SetDir: tw_offline_msg_store is not initialized");
		res = TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	}
	return res;
}

/** tasker tast to run async with the api **/
void twOfflineMsgStore_Task(DATETIME now, void * params) {
	OfflineMsgStore_TaskParams * tmp = (OfflineMsgStore_TaskParams*)params;

	/** check for valid params **/
	if (!params || !tw_offline_msg_store || !tw_offline_msg_store->mtx) {
		TW_LOG(TW_ERROR, "twOfflineMsgStore_Task: null params");
		return;
	}

	/** check first param for request type **/
	twMutex_Lock(tw_offline_msg_store->mtx);
	switch (tmp->request_type) {
	case OFFLINE_MSG_STORE_FLUSH:
		tmp->status = twOfflineMsgStore_Flush(tmp->msg, tmp->ws);
		break;
	case OFFLINE_MSG_STORE_WRITE:
		tmp->status = twOfflineMsgStore_Write(tmp->msg, tmp->ws);
		break;
	default:
		TW_LOG(TW_ERROR, "twOfflineMsgStore_Task: Request type %ud not recognized", tmp->request_type);
		break;
	}
	twMutex_Unlock(tw_offline_msg_store->mtx);

	if (tmp->status == TW_WEBSOCKET_NOT_CONNECTED) {
		TW_LOG(TW_WARN, "twOfflineMsgStore_Task: websocket not connected, error code: %d", tmp->status);
	} else if (tmp->status) {
		TW_LOG(TW_ERROR, "twOfflineMsgStore_Task: error handling offline message store request, error code: %d", tmp->status);
	}
}

/** returns offline message list count **/
/** returns -1 on error **/
int getOfflineMsgListCount() {
	int result;

	/* check if list exists */
	if (tw_offline_msg_store && tw_offline_msg_store->offlineMsgList) {
		twMutex_Lock(tw_offline_msg_store->mtx);
		result = tw_offline_msg_store->offlineMsgList->count;
		twMutex_Unlock(tw_offline_msg_store->mtx);
	} else {
		TW_LOG(TW_ERROR,"getOfflineMsgListCount: tw_offline_msg_store->offlineMsgList does not exist");
		result = -1;
	}
	return result;
}

/** returns offline message list count **/
/** returns -1 on error **/
uint64_t getOfflineMsgSize() {
	int result;

	/* check if list exists */
	if (tw_offline_msg_store && tw_offline_msg_store->offlineMsgSize) {
		twMutex_Lock(tw_offline_msg_store->mtx);
		result = tw_offline_msg_store->offlineMsgSize;
		twMutex_Unlock(tw_offline_msg_store->mtx);
	} else {
		TW_LOG(TW_ERROR,"getOfflineMsgListCount: tw_offline_msg_store->offlineMsgList does not exist");
		result = -1;
	}
	return result;
}

/** returns offline message list status, TRUE if it exists, FALSE if it does not **/
char isOfflineMsgListEnabled() {
	char result;

	/* check if list exists */
	if (tw_offline_msg_store && tw_offline_msg_store->offlineMsgList) {
		result = TRUE;
	} else {
		result = FALSE;
	}
	return result;
}

/** returns current offline message store directory, NULL if error or does not exist */
/** NOTE caller is responsible for free'ing memory **/
char * getOfflineMsgDirectory() {
	char * str = NULL;
	if(tw_offline_msg_store && tw_offline_msg_store->offlineMsgFile) {
		twMutex_Lock(tw_offline_msg_store->mtx);
		str = duplicateString(tw_offline_msg_store->offlineMsgFile);
		twMutex_Unlock(tw_offline_msg_store->mtx);
	}
	return str;
}

/** End of Supporting Functions **/

int twOfflineMsgStore_Initialize(char enabled, const char * filePath, uint64_t size, char onDisk){
	int res = TW_OK;

	/* Validate inputs */
	if (onDisk && !filePath) {
		TW_LOG(TW_ERROR, "twOfflineMsgStore_Initialize: Invalid parameter found, filePath: %s", filePath ? filePath : "NULL");
		res =  TW_INVALID_PARAM;
	}
	if (TW_OK == res) {
		/* Check the global initialization mutex */
		if (twOfflineInitMutex) {
			TW_LOG(TW_WARN,"twOfflineMsgStoreInitialize: initialization mutex already created");
		} else {
			/* Create the global initialization mutex */
			twOfflineInitMutex = twMutex_Create();
			if (!twOfflineInitMutex) {
				TW_LOG(TW_ERROR, "twOfflineMsgStore_Initialize: Error creating initialization mutex");
				res = TW_ERROR_CREATING_MTX;
			}
		}
		if (TW_OK == res) {
			/* lock init mutex */
			twMutex_Lock(twOfflineInitMutex);

			/* Check to see if the singleton already exists */
			if (tw_offline_msg_store) {
				TW_LOG(TW_WARN, "twOfflineMsgStore_Initialize: Offline Message Store singleton already exists");
				twMutex_Unlock(twOfflineInitMutex);
				res = TW_OK;
			} else {
				/* Allocate space for the structure */
				tw_offline_msg_store = (twOfflineMsgStore *)TW_CALLOC(sizeof(twOfflineMsgStore), 1);
				if (!tw_offline_msg_store) {
					TW_LOG(TW_ERROR, "twOfflineMsgStore_Initialize: Error allocating tw_offline_msg_store structure");
					res = TW_ERROR_ALLOCATING_MEMORY;
				}
				if (TW_OK == res) {

					/* create singleton mutex */
					tw_offline_msg_store->mtx = twMutex_Create();

					if (!tw_offline_msg_store->mtx) {
						res = TW_ERROR_INITIALIZING_OFFLINE_MSG_STORE;
					} else {
						twMutex_Lock(tw_offline_msg_store->mtx);
						/* set initial values */
						tw_offline_msg_store->offlineMsgEnabled = enabled;
						tw_offline_msg_store->offlineMsgSize = 0;
						tw_offline_msg_store->onDisk = onDisk;

						/* set the storage file (on disk or in ram) */
						if (enabled) {
							res = twOfflineMsgStore_SetStorage(filePath);
						}
						twMutex_Unlock(tw_offline_msg_store->mtx);
					}
				}
			}
			/* unlock init mutex */
			twMutex_Unlock(twOfflineInitMutex);
		}
		if(res) {
			/* if error, cleanup associated memory */
			if (tw_offline_msg_store) {
				twOfflineMsgStore_Delete();
			} else if (twOfflineInitMutex) {
				twMutex_Delete(twOfflineInitMutex);
				twOfflineInitMutex = NULL;
			}
		}
	}

	/* return result */
	return res;
}


int twOfflineMsgStore_Delete(){
	int res = TW_OK;

	/* check init mutex */
	if (twOfflineInitMutex) {
		/* lock init mutex to prevent init and delete being called at the same time */
		twMutex_Lock(twOfflineInitMutex);

		/* check tw_offline_msg_store singleton struct */
		if (tw_offline_msg_store) {
			/* check struct mutex */
			if (tw_offline_msg_store->mtx) {

				/* lock mutext and start deleting the struct */
				twMutex_Lock(tw_offline_msg_store->mtx);

				/* delete the offline message list if it exists (storage in ram model)*/
				if(tw_offline_msg_store->offlineMsgList) {
					twList_Delete(tw_offline_msg_store->offlineMsgList);
					tw_offline_msg_store->offlineMsgList = NULL;
				}

				/* delete offline msg file if it exists (storage on disk model */
				if(tw_offline_msg_store->offlineMsgFile) {
					TW_FREE(tw_offline_msg_store->offlineMsgFile);
					tw_offline_msg_store->offlineMsgFile = NULL;
				}

				/* unlock and delete the struct mutex */
				twMutex_Unlock(tw_offline_msg_store->mtx);
				twMutex_Delete(tw_offline_msg_store->mtx);
				tw_offline_msg_store->mtx = NULL;

				/* set result to TW_OK */
				res = TW_OK;
			} else {
				/* if the tw_offline_msg_store mutex does not exist log an error */
				TW_LOG(TW_ERROR, "twOfflineMsgStore_Delete: offline message store mutex does not exist");
				res = TW_UNKNOWN_ERROR;
			}

			/* delete entire offline msg store struct */
			TW_FREE(tw_offline_msg_store);
			tw_offline_msg_store = NULL;
		} else {
			/* if the tw_offline_msg_store  does not exist log an error */
			TW_LOG(TW_ERROR, "twOfflineMsgStore_Delete: offline message store does not exist");
			res = TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
		}

		/* unlock and delete the init mutex */
		twMutex_Unlock(twOfflineInitMutex);
		twMutex_Delete(twOfflineInitMutex);
		twOfflineInitMutex = NULL;
	} else {
		TW_LOG(TW_ERROR, "twOfflineMsgStore_Delete: can not delete Offline Msg Store before it is initialized");
		res = TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	}

	/* return result */
	return res;
}

/** handles incoming requests, task is responsible for blocking/locking and freeing the params **/
int twOfflineMsgStore_HandleRequest(struct twMessage ** msg, struct twWs * ws, enum OfflineRequest request_type){
	int res = TW_OK;
	OfflineMsgStore_TaskParams * tmp = NULL;

	/* validate inputs */
	if (!msg || !*msg || !ws ) {
		/* set error if anything is missing */
		TW_LOG(TW_ERROR, "twOfflineMsgStore_HandleRequest: invalid inputs");
		res =  TW_INVALID_PARAM;
	} else if(!tw_offline_msg_store) {
		/* set error if singleton is not initialized */
		TW_LOG(TW_ERROR, "twOfflineMsgStore_HandleRequest: tw_offline_msg_store is not initialized");
		res =  TW_NULL_OR_INVALID_OFFLINE_MSG_STORE_SINGLETON;
	} else {
		/* create params */
		res = twOfflineMsgStore_CreateTaskParams(msg, ws, request_type, &tmp);

		/* validate param creation */
		if (res) {
			TW_LOG(TW_ERROR, "twOfflineMsgStore_HandleRequest: error creating offline msg store task params");
		} else {
			/* execute offline message store task */
			twOfflineMsgStore_Task(twGetSystemMillisecondCount(), (void *)tmp);

			/* set status */
			res = tmp->status;

			/* cleanup task params */
			twOfflineMsgStore_DeleteTaskParams(&tmp);
		}
	}

	/* return result */
	return res;
}

char twOfflineMsgStore_IsEnabled() {
    if (tw_offline_msg_store) return tw_offline_msg_store->offlineMsgEnabled;
    return FALSE;
}

void twOfflineMsgStore_Disable() {
	twcfg.offline_msg_queue_size = 0;
	if (tw_offline_msg_store) {
		tw_offline_msg_store->offlineMsgEnabled = FALSE;
	}
}

void twOfflineMsgStore_Enable() {
	twcfg.offline_msg_queue_size = OFFLINE_MSG_QUEUE_SIZE;
	if (tw_offline_msg_store) {
		tw_offline_msg_store->offlineMsgEnabled = TRUE;
	}
}

twOfflineMsgStore *getOfflineMsgStore() {
	return tw_offline_msg_store;
}