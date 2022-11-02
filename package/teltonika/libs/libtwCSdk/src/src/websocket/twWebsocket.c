/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable Websocket Client  abstraction layer
 */

#include "twOSPort.h"
#include "twWebsocket.h"
#include "twDefaultSettings.h"
#include "twErrors.h"
#include "twTls.h"
#include "twLogger.h"
#include "stringUtils.h"
#include "tomcrypt.h"
#include "twApi.h"
#include "twApiStubs.h"
#include "twMessages.h"

#include <string.h>
#include <stdlib.h> 
#include <stdio.h>

#define NOT_SET -1
#define TW_TRUE 1
#define TW_FALSE 0

#define WS_VERSION "13"
#define WS_HEADER_MAX_SIZE 10
#define WS_HEADER_MIN_SIZE 2
#define KEY_LENGTH 16
/* Base 64 encoded key - add the last 1 byte for null termination */
#define ENCODED_KEY_LENGTH (KEY_LENGTH * 2)

#define RCVD_CONNECTION_HEADER 0x01
#define RCVD_UPGRADE_HEADER 0x20
#define VALID_WS_ACCEPT_KEY 0x40


signed char isLittleEndian = NOT_SET;

/**
* Websocket helper functions
**/
int sendCtlFrame(twWs * ws, unsigned char type, char * msg);
int validateAcceptKey(twWs * ws, const char * header_value);

/**
* Websocket compression functions
**/
void twDeflateInit(struct twWs * ws);
void twDeflateEnd(struct twWs * ws);
void twInflateInit(struct twWs * ws);
void twInflateEnd(struct twWs * ws);

/**
* Header callbacks
**/

int32_t ws_on_header_value(twWs * ws, char * header_name, char * header_value) {
	char * line = NULL;
	if (!ws || !header_name || !header_value) {
		TW_LOG(TW_DEBUG,"ws_on_header_value: NULL ws or data value");
		return 1;
	}
	TW_LOG(TW_TRACE,"ws_on_header_value: Header->%s : %s", header_name, header_value);
	if (strcmp(header_name, "upgrade") == 0) {
		/* convert value to lowercase before comparing */
		line = header_value;
		while(*line) {
			if (*line >= 'A' && *line <= 'Z') *line = *line + 32;
			line++;
		}
		if (strcmp(header_value, "websocket") != 0) {
			TW_LOG(TW_ERROR, "ws_on_header_value: Invalid 'upgrade' header: %s", header_value);
			ws->connect_state = -1;
		} else ws->connect_state |= RCVD_UPGRADE_HEADER;
	} else if (strcmp(header_name, "connection") == 0) {
		/* convert value to lowercase before comparing */
		line = header_value;
		while(*line) {
			if (*line >= 'A' && *line <= 'Z') *line = *line + 32;
			line++;
		}
		if (strcmp(header_value, "upgrade") != 0) {
			TW_LOG(TW_ERROR, "ws_on_header_value: Invalid 'connection' header: %s", header_value);
			ws->connect_state = -1;
		} else ws->connect_state |= RCVD_CONNECTION_HEADER;
	} else if (strcmp(header_name, "sec-websocket-accept") == 0) {
		if (validateAcceptKey(ws, header_value) != 0) {
			TW_LOG(TW_ERROR, "ws_on_header_value: Invalid 'sec-websocket-accept' header: %s", header_value);
			ws->connect_state = -1;
		} else ws->connect_state |= VALID_WS_ACCEPT_KEY;
	} else if (strcmp(header_name, "sec-websocket-extensions") == 0) {
		/* Server supports websocket extensions */
		TW_LOG(TW_INFO, "Server supports websocket extensions: %s", header_value);
		/* Not to spec, but we require client context takeover (otherwise we'd have to reset our zlib context after each send). */
		ws->bSupportsPermessageDeflate = (strstr (header_value, "permessage-deflate") != NULL &&			/* Server supports PMCE */
											strstr (header_value, "client_no_context_takeover") == NULL);	/* And isn't requesting we not do client context takeover */
		TW_LOG(TW_TRACE, "PermessageDeflate is [%s].", ws->bSupportsPermessageDeflate ? "enabled" : "disabled");
	}
	return 0;
}
int32_t ws_on_headers_complete(twWs * ws) {
	int state;
	if (!ws) {
		TW_LOG(TW_DEBUG,"ws_on_headers_complete: NULL websocket");
		return 1;
	}
	state = ws->connect_state;
	if (state != -1 && state & RCVD_UPGRADE_HEADER && state & RCVD_CONNECTION_HEADER && state & VALID_WS_ACCEPT_KEY) {
		TW_LOG(TW_DEBUG,"ws_on_headers_complete: Websocket connected!");
		ws->isConnected = TRUE;
		return 0;
	}
	TW_LOG(TW_ERROR,"ws_on_headers_complete: Websocket connection failed.");
	ws->isConnected = FALSE;
	return 1;
}

/**
* Helper functions
**/
int restartSocket(twWs * ws) {
	/* Tear down the socket and create a new one */ 
	int res = 0;
	memset(ws->frameBuffer, 0, twcfg.message_chunk_size + WS_HEADER_MAX_SIZE + 1);
	if (NULL != ws->multiframeRecvStream) {
		twStream_Delete (ws->multiframeRecvStream);
		ws->multiframeRecvStream = NULL;
	}
	ws->connect_state = 0;
	ws->isConnected = FALSE;
    res = s_twTlsClient_Reconnect(ws->connection, ws->host, ws->port);
	ws->frameBufferPtr = ws->frameBuffer;
	ws->headerPtr = ws->ws_header;
	ws->bCompressedMsg = FALSE;
    return res;
}

uint32_t getMaxWebsocketMessageSize (char isTunnelMessage) {
	/* For binary messages, use the max alwaysOn message size with some extra room for any
	* compression overhead in worst cases */
	return isTunnelMessage ? twcfg.max_ws_tunnel_message_size : 3 * twcfg.max_message_size / 2;
	}

/**
*	Context manipulation functions
**/
int twWs_Create(char * host, uint16_t port, char * resource, twPasswdCallbackFunction app_key_function, char * gatewayName, uint32_t messageChunkSize, uint16_t frameSize, twWs ** entity) {
	int err = TW_UNKNOWN_ERROR;
	twWs * ws = NULL;

	TW_LOG(TW_DEBUG, "twWs_Create: Initializing Websocket Client for %s:%d/%s", host, port, resource);

	/* Validate our host/port */
	if (!host || !port || !resource || !app_key_function || !entity) {
		TW_LOG(TW_ERROR, "twWs_Create: Missing required parameters");
		return TW_INVALID_PARAM;
	}
	ws = (twWs *)TW_CALLOC(sizeof(twWs), 1);
	if (!ws) {
		TW_LOG(TW_ERROR, "twWs_Create: Error allocating websocket struct");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	ws->isConnected = FALSE;
	/* Create our connection  */
	err = twTlsClient_Create(host, port, 0, &ws->connection);
	if (err) {
		TW_LOG(TW_ERROR, "twWs_Create: Error creating BSD socket to be used for the websocket");
		twWs_Delete(ws);
		return err;
	}
	ws->port = port;
	/* Create copies of any strings passed in */
	ws->host = duplicateString(host);
	if (!ws->host) {
		TW_LOG(TW_ERROR, "twWs_Create: Error allocating storage for websocket host");
		twWs_Delete(ws);
		return TW_ERROR_ALLOCATING_MEMORY;
	}	
	ws->api_key_callback = app_key_function;
	if (!ws->api_key_callback) {
		TW_LOG(TW_ERROR, "twWs_Create: Error allocating storage for websocket api_key_callback");
		twWs_Delete(ws);
		return TW_ERROR_ALLOCATING_MEMORY;
	}	
	ws->resource = duplicateString(resource);
	if (!ws->resource) {
		TW_LOG(TW_ERROR, "twWs_Create: Error allocating storage for websocket resource");
		twWs_Delete(ws);
		return TW_ERROR_ALLOCATING_MEMORY;
	}	
	if (gatewayName) {
		ws->gatewayName = duplicateString(gatewayName);
		if (!ws->gatewayName) {
			TW_LOG(TW_ERROR, "twWs_Create: Error allocating storage for websocket gatewayName");
			twWs_Delete(ws);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
	}
	/* Create the mutexes */
	ws->sendMessageMutex = twMutex_Create();
	ws->sendFrameMutex = twMutex_Create();
	ws->recvMutex = twMutex_Create();

	if (!ws->sendMessageMutex || !ws->recvMutex || !ws->sendFrameMutex) {
		TW_LOG(TW_ERROR, "Error allocating or creating mutex");
		twWs_Delete(ws);
		return TW_ERROR_CREATING_MTX;
	}	
	/* Message Chunks MUST fit into a single frame */
	if (messageChunkSize > frameSize) {
		TW_LOG(TW_ERROR, "twWs_Create: Message chunk size MUST be less than or equal max websocket frame size");
		twWs_Delete(ws);
		return TW_INVALID_PARAM;
	}	
	ws->messageChunkSize = messageChunkSize;
	ws->frameSize = frameSize;
	ws->frameBuffer = (char *)TW_CALLOC((frameSize + WS_HEADER_MAX_SIZE + 1), 1);
	if (!ws->frameBuffer) {
		TW_LOG(TW_ERROR, "twWs_Create: Error allocating frame buffer storage for websocket");
		twWs_Delete(ws);
		return TW_ERROR_ALLOCATING_MEMORY;
	}	
	ws->frameBufferPtr = ws->frameBuffer;
	ws->bytesNeeded = WS_HEADER_MIN_SIZE;
	*entity = ws;

	/* Initialize the WebSocket compression members */
	ws->bDeflateInitialized = FALSE;
	ws->compressionMutex = twMutex_Create ();
	ws->bInflateInitialized = FALSE;
	ws->decompressionMutex = twMutex_Create ();

	return TW_OK;
}

int twWs_Delete(twWs * ws) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_Delete: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (ws->connection) {
		twTlsClient_Delete(ws->connection); 
	}

	TW_FREE(ws->host);
	TW_FREE(ws->frameBuffer);
	if (ws->multiframeRecvStream) {
		twStream_Delete (ws->multiframeRecvStream);
	}
	TW_FREE(ws->resource);
	TW_FREE(ws->security_key);
	TW_FREE(ws->gatewayName);
	TW_FREE(ws->gatewayType);
	twMutex_Delete(ws->sendMessageMutex);
	twMutex_Delete(ws->sendFrameMutex);
	twMutex_Delete(ws->recvMutex);
	twMutex_Delete (ws->compressionMutex);
	twMutex_Delete (ws->decompressionMutex);

	TW_FREE(ws);
	return TW_OK;
}

#define REQ_SIZE 512
int twWs_Connect(twWs * ws, uint32_t timeout) {

	int32_t i = 0;
	int32_t bytesWritten = 0;
	int32_t bytesRead = 0;
	char key[KEY_LENGTH];
	char * req = NULL;
	char max_frame_size[16];
	DATETIME timeouttime = 0;
	DATETIME now = 0;
	unsigned long encodedlen = ENCODED_KEY_LENGTH;
	int appKeyLength = 0;
	int count = 0;

	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_Connect: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (ws->isConnected == TRUE) { 
		TW_LOG(TW_WARN, "twWs_Connect: Already connected");
		return TW_OK; 
	}

	twMutex_Lock(ws->sendMessageMutex);
	ws->connect_state = 0;
	ws->read_state = READ_HEADER;

	/* Create the random key */
	now = twGetSystemTime(TRUE);
	srand(now % 1000);
	for (i = 0; i < KEY_LENGTH; i++) {
		uint8_t r = rand() & 0xff;
		key[i] = r;
	}
	if (!ws->security_key) ws->security_key = (unsigned char *)TW_CALLOC(ENCODED_KEY_LENGTH, 1);
	if (!ws->security_key) {
		TW_LOG(TW_ERROR,"twWs_Connect: Error allocating security key buffer");
		twMutex_Unlock(ws->sendMessageMutex);
		return TW_ERROR_ALLOCATING_MEMORY;
	} 
	base64_encode((const unsigned char *)key, KEY_LENGTH, ws->security_key, &encodedlen);

	/* End the deflate/inflate contexts in case the previous connection was closed on us */
	twDeflateEnd (ws);
	twInflateEnd (ws);

	/* Form the HTTP request */
	req = (char *)TW_CALLOC(REQ_SIZE, 1);
	if (!req) {
		TW_LOG(TW_ERROR,"twWs_Connect: Error allocating request buffer");
		twMutex_Unlock(ws->sendMessageMutex);
		return TW_ERROR_ALLOCATING_MEMORY;
	} 
	strncpy(req,"GET ", REQ_SIZE - 1);
	strncat(req, ws->resource, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, " HTTP/1.1\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "User-Agent: ThingWorx C SDK\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "Upgrade: websocket\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "Connection: Upgrade\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "Host: ", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, ws->host, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "Sec-WebSocket-Version: ", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, WS_VERSION, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "Sec-WebSocket-Key: ", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, (const char *)ws->security_key, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	if (!ws->bDisableCompression) {
		strncat(req, "Sec-WebSocket-Extensions: permessage-deflate", REQ_SIZE - strlen (req) - 1);
		strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	}
	strncat(req, "Max-Frame-Size: ", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	sprintf(max_frame_size, "%u", ws->frameSize);
	strncat(req, max_frame_size, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "appKey: ", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	{
		char* api_key = twConvertCallbackToPasswd(ws->api_key_callback);
		strncat(req, api_key, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
		appKeyLength = strnlen(api_key, strnlen(req, REQ_SIZE));
		twFreePasswd(api_key);
	}
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	
	/* Connect the underlying socket and send the request */
	if (restartSocket(ws)) {
		TW_LOG(TW_ERROR,"twWs_Connect: Error restarting socket.  Error %d", twSocket_GetLastError());
		/* Zero out memory that contains api key */
		memset(req,0,strnlen(req, REQ_SIZE));
		TW_FREE (req);
		twMutex_Unlock(ws->sendMessageMutex);
		return TW_SOCKET_INIT_ERROR;
	}
	bytesWritten = s_twTlsClient_Write(ws->connection, req, strnlen(req, REQ_SIZE), 100);
	if (bytesWritten > 0) TW_LOG(TW_TRACE, "twWs_Connect: Connected to %s:%d", ws->host, ws->port);
	else {
		TW_LOG(TW_ERROR,"twWs_Connect: No bytes written.  Error %d", twSocket_GetLastError());
		/* Zero out memory that contains api key */
		memset(req,0,strnlen(req, REQ_SIZE));
		TW_FREE (req);
		twMutex_Unlock(ws->sendMessageMutex);
		restartSocket(ws);
		return TW_ERROR_WRITING_TO_SOCKET;
	} 
	/* Done with the request, filter all but the first 4 digits of the appKey */
	for (count = appKeyLength; count > 4; count--) {
		int index = strnlen(req, REQ_SIZE) - count;
		if (req[index] == '-') continue;
		req[index] = '*';
	}
	TW_LOG(TW_TRACE, "twWs_Connect: Sent request:\n%s", req);
	/* Zero out memory that contains api key */
	memset(req,0,strnlen(req, REQ_SIZE));
	TW_FREE(req);
	/* Get the response */
	timeouttime = twGetSystemTime(TRUE);
	timeouttime = twAddMilliseconds(timeouttime,timeout);
	now = twGetSystemTime(TRUE);
	while (ws->connect_state >= 0 && ws->isConnected == FALSE && twTimeGreaterThan(timeouttime, now)) {
		bytesRead = twTlsClient_Read(ws->connection, ws->frameBufferPtr, ws->frameSize - (ws->frameBufferPtr - ws->frameBuffer), twcfg.socket_read_timeout);
		if (bytesRead < 0) {
			/* Something is wrong with the socket - give up */
			ws->frameBufferPtr = ws->frameBuffer;
			TW_LOG(TW_ERROR,"twWs_Connect: Error reading from socket.  Error: %d", twSocket_GetLastError());
			twMutex_Unlock(ws->sendMessageMutex);
			return TW_ERROR_INITIALIZING_WEBSOCKET;
		}
		/* Try to parse the response */
		if (bytesRead) {
			char respCode[8];
			char * header_name = NULL;
			char * header_value = NULL;
			char * line = NULL;
			char * headEnd = NULL;
			char gotName = FALSE;
			memset(respCode, 0, 8);
			TW_LOG(TW_TRACE,"twWs_Connect: Got Response from Server:\n\n%s\n", ws->frameBuffer);
			/* Increment our pointer and check for overrun */
			ws->frameBufferPtr = ws->frameBufferPtr + bytesRead;
			if (ws->frameBufferPtr - ws->frameBuffer > ws->frameSize) {
				ws->frameBufferPtr = ws->frameBuffer;
				TW_LOG(TW_ERROR,"twWs_Connect: Connect response too big. Websocket connect failed");
				twMutex_Unlock(ws->sendMessageMutex);
				return TW_ERROR_INITIALIZING_WEBSOCKET;
			}
			/* Check to see if we got the entire header */
			headEnd = strstr(ws->frameBuffer, "\r\n\r\n");
			if (!headEnd) {
				TW_LOG(TW_TRACE,"twWs_Connect: Didn't get the entire header - attempting to read more");
				continue;
			}
			/* Look for the Switching Protocols response */
			strncpy(respCode, &ws->frameBuffer[9], 3);
			if (strcmp(respCode, "101") != 0) {
				/* Something is wrong with the socket - give up */
				ws->frameBufferPtr = ws->frameBuffer;
				TW_LOG(TW_ERROR,"twWs_Connect: Error initializing web socket.  Response code: %s", respCode);
				twMutex_Unlock(ws->sendMessageMutex);
				return TW_ERROR_INITIALIZING_WEBSOCKET;
			}
			/* Look for the required headers.  Beginning of headers is after first \r\n */
			line = strstr(ws->frameBuffer, "\r\n");
			line += 2;
			headEnd += 2;
			/* 
			Walk through the line separate name from value.  While we are
			at it, convert the name to lowercase, and remove whitespace.
			*/
			header_name = line;
			while (line < headEnd) {
				if (!gotName) {
					/* Convert to lower case */
					if (*line >= 'A' && *line <= 'Z') *line = *line + 32;
					/* Find the end of the header name - either whitespace or : */
					if (*line == ' ' || *line == '\t' || *line == ':') {
						*line = 0x00;
						gotName = TRUE;
					}
					line++;
					continue;
				} else {
					/* Get rid of any leading whitespace*/
					if (!header_value && (*line == '\r' || *line == '\n' || *line == ' ' || *line == '\t')) {
						*line = 0x00;
						line++;
						continue;
					} 
					/* Mark the begininng of the value */
					if (!header_value) {
						header_value = line; 
					} 
					/* Fine the end of the value */
					if (*line == '\r' || *line == '\n') {
						*line = 0x00;
						/* Process this name/value pair */
						if (ws_on_header_value(ws, header_name, header_value)) {
							ws->frameBufferPtr = ws->frameBuffer;
							TW_LOG(TW_WARN,"twWs_Connect: Error in HTTP response header: %s : %s.", header_name, header_value);
							twMutex_Unlock(ws->sendMessageMutex);
							return TW_ERROR_INITIALIZING_WEBSOCKET;
						}	
						/* Advance to the next line */
						while (line < headEnd && (*line == 0x00 || *line == '\r' || *line == '\n' || *line == ' ' || *line == '\t')) line++;
						gotName = FALSE;
						header_name = line;
						header_value = NULL;
					} else {
						/* Advance to the next character */
						line++;
					}
					continue;
				}
			}
			/* Reset the frame buffer pointer now */
			ws->frameBufferPtr = ws->frameBuffer;
			/* See if we got what we needed */
			if (ws_on_headers_complete(ws)) {
				TW_LOG(TW_WARN,"twWs_Connect: Error in HTTP response headers. Websocket connection failed");
				twMutex_Unlock(ws->sendMessageMutex);
				return TW_ERROR_INITIALIZING_WEBSOCKET;
			}
		}
		now = twGetSystemTime(TRUE);
	}
	if (twTimeGreaterThan(now, timeouttime)) {
		/* We timed out */
		TW_LOG(TW_ERROR,"twWs_Connect: Timed out trying to connect");
		twMutex_Unlock(ws->sendMessageMutex);
		return TW_TIMEOUT_INITIALIZING_WEBSOCKET;
	}
	if (!(ws->isConnected == TRUE)) {
		TW_LOG(TW_ERROR,"twWs_Connect: Error trying to connect");
		twMutex_Unlock(ws->sendMessageMutex);
		restartSocket(ws);
		return TW_ERROR_INITIALIZING_WEBSOCKET;
	}
	TW_LOG(TW_INFO,"twWs_Connect: Websocket connected!");
	if (ws->on_ws_connected) (ws->on_ws_connected)(ws);
	twMutex_Unlock(ws->sendMessageMutex);
	ws->headerPtr = ws->ws_header;
	ws->bytesNeeded = WS_HEADER_MIN_SIZE;
	ws->read_state = READ_HEADER;
	return TW_OK;
}

char twWs_IsConnected(twWs * ws) { 
	return ((ws && ws->isConnected == TRUE) ? TRUE : FALSE); 
}

#define MAX_MSG_SIZE 60
int twWs_Disconnect(twWs * ws, enum close_status code, char * reason) {
	/* True message length is message size plus two prefix and suffix characters */
	char msg[MAX_MSG_SIZE + 4] = "";
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_Disconnect: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	/*  Send a close message */
	TW_LOG(TW_DEBUG, "Disconnect called.  Code: %d, Reason: %s", code, reason);
	if (code != SERVER_CLOSED) {
		/* Send a close to the server */
		msg[0] = 0x03;
		switch (code) {
			case NORMAL_CLOSE:
				msg[1] = (char)0xE8;
				strncat(msg, "Normal Close", MAX_MSG_SIZE);
				break;
			case GOING_TO_SLEEP:
				msg[1] = (char)0xE9;
				strncat(msg, "Going to Sleep", MAX_MSG_SIZE);
				break;
			case PROTOCOL_ERROR:
				msg[1] = (char)0xEA;
				strncat(msg, "Protocol Error", MAX_MSG_SIZE);
				break;
			case UNSUPPORTED_DATA_TYPE:
				msg[1] = (char)0xEB;
				strncat(msg, "Unsupported Data Type", MAX_MSG_SIZE);
				break;
			case INVALID_DATA:
				msg[1] = (char)0xEF;
				strncat(msg, "Invalid Data", MAX_MSG_SIZE);
				break;
			case POLICY_VIOLATION:
				msg[1] = (char)0xF0;
				strncat(msg, "Policy Violation", MAX_MSG_SIZE);
				break;
			case FRAME_TOO_LARGE:
				msg[1] = (char)0xF1;
				strncat(msg, "Frame too large", MAX_MSG_SIZE);
				break;
			case NO_EXTENSION_FOUND:
				msg[1] = (char)0xF2;
				strncat(msg, "No extension found", MAX_MSG_SIZE);
				break;
			case UNEXPECTED_CONDITION:
			default:
				msg[1] = (char)0xF3;
				strncat(msg, "Unexpected Condition", MAX_MSG_SIZE);
				break;
		}
		strncat(msg, " ", MAX_MSG_SIZE);
		strncat(msg, reason, sizeof (msg) - strnlen(msg, sizeof(msg)) - 1);
		/* Check that msg[] did not get overrun */
		if (msg[MAX_MSG_SIZE + 3] != 0) {
			TW_LOG(TW_ERROR, "twWs_Disconnect: Overflow risk detected. Disconnect aborted.");
			return TW_ERROR_WRITING_TO_WEBSOCKET;
		}
		sendCtlFrame(ws, 0x08, msg);
	}
	ws->isConnected = FALSE;
	twTlsClient_Close(ws->connection);
	if (ws && ws->on_ws_close && msg[0] == 0x03) ws->on_ws_close(ws, msg + 2, strnlen(msg + 2, MAX_MSG_SIZE));
	twDeflateEnd (ws);
	twInflateEnd (ws);
	return TW_OK;
}

int twWs_RegisterConnectCallback(twWs * ws, ws_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterConnectCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	ws->on_ws_connected = cb;
	return TW_OK;
}

int twWs_RegisterCloseCallback(twWs * ws, ws_data_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterCloseCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	ws->on_ws_close = cb;
	return TW_OK;
}

int twWs_RegisterBinaryMessageCallback(twWs * ws, ws_binary_data_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterMessageCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	ws->on_ws_binaryMessage = cb;
	return TW_OK;
}

int twWs_RegisterTextMessageCallback(twWs * ws, ws_data_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterTextMessageCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM;
	}
	ws->on_ws_textMessage = cb;
	return TW_OK;
}

int twWs_RegisterPingCallback(twWs * ws, ws_data_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterPingCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	ws->on_ws_ping = cb;
	return TW_OK;
}

int twWs_RegisterPongCallback(twWs * ws, ws_data_cb cb) {
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_RegisterPongCallback: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	ws->on_ws_pong = cb;
	return TW_OK;
}

/* Receive function for single threaded environments - does not return the data */
int twWs_Receive(twWs * ws, uint32_t timeout) {
	int32_t bytesRead = 0;
	DATETIME maxtime = 0;
	char savedState = READ_HEADER;
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_Receive: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (!ws->isConnected) { 
		TW_LOG(TW_DEBUG, "twWs_Receive: Not connected"); 
		return TW_WEBSOCKET_NOT_CONNECTED; 
	}
	twMutex_Lock(ws->recvMutex);
	/*
	 We never want to read past frame data into another frame
	 so read only the maximum size of a ws header first and then
	 receive whatever remaining bytes are left in the frame.  The
	 remaining bytes are held in the parser's content_length field

	 If we are not already receiving the frame content read just enough for the header
	 else read the number of bytes left in the frame content
	*/
	/* Are we asking for more bytes than we have left in the frame buffer? */
	if (ws->bytesNeeded > (ws->frameSize - (ws->frameBufferPtr - ws->frameBuffer))) {
		TW_LOG(TW_ERROR,"twWs_Receive: Buffer overrun detected, bytes to be read would exceed remaining space in frame buffer");
		TW_LOG(TW_TRACE,"twWs_Receive: Need %d bytes but only have space for %d in frame buffer", ws->bytesNeeded, (ws->frameSize - (ws->frameBufferPtr - ws->frameBuffer)));
		ws->frameBufferPtr = ws->frameBuffer;
		ws->read_state = READ_HEADER;
		ws->bytesNeeded = WS_HEADER_MIN_SIZE;
		ws->headerPtr = &ws->ws_header[0];
	}

	while (ws->read_state == READ_HEADER) {
		int cnt = 0;
		
		bytesRead = s_twTlsClient_Read(ws->connection, ws->headerPtr, ws->bytesNeeded, timeout);
		if (bytesRead > 0) {
			char opcode = 0xff;
			TW_LOG(TW_TRACE,"twWs_Receive: Read %d bytes into header buffer", bytesRead);
			ws->headerPtr += bytesRead;
			ws->bytesNeeded = ws->bytesNeeded - bytesRead;
			/* Do we still need more bytes? */
			if (ws->bytesNeeded > 0) {
				TW_LOG(TW_TRACE,"twWs_Receive: Don't have a full header yet. Still need %d bytes. Will try again", ws->bytesNeeded);
				twMutex_Unlock(ws->recvMutex);
				return TW_OK;
			} else if (ws->bytesNeeded < 0) {
				/* Something is very wrong */
				TW_LOG(TW_WARN,"twWs_Receive: bytesNeed less than zero");
				ws->isConnected = FALSE;
				if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
				twMutex_Unlock(ws->recvMutex);
				restartSocket(ws);
				return TW_ERROR_READING_FROM_WEBSOCKET;
			}
			/* Parse what we have */
			cnt = ws->headerPtr - ws->ws_header;
			if (ws->ws_header[1] == 127) {
				/* We aren't handling frames this large */
				TW_LOG(TW_ERROR,"twWs_Receive: Incoming frame is too large to receive");
				ws->isConnected = FALSE;
				if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
				twMutex_Unlock(ws->recvMutex);
				restartSocket(ws);
				return TW_ERROR_READING_FROM_WEBSOCKET;
			} else if (ws->ws_header[1] == 126) {
				if (cnt < 4) {
					/* Need more bytes for the size */
					ws->bytesNeeded = 4 - cnt;
					continue;
				} else {
					/* We have the entire header */
					TW_LOG(TW_TRACE,"twWs_Receive: Got 2 byte length. 0x%x 0x%x", ws->ws_header[2], ws->ws_header[3] );
					ws->bytesNeeded = (ws->ws_header[2] * 256) + ws->ws_header[3];
					/* Make sure we can handle this */
					if (ws->bytesNeeded > ws->frameSize) {
						TW_LOG(TW_ERROR,"twWs_Receive: Incoming frame is too large to receive.  Size: %d, Max Frame Size: %d", ws->bytesNeeded, ws->frameSize);
						ws->isConnected = FALSE;
						if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
						twMutex_Unlock(ws->recvMutex);
						restartSocket(ws);
						return TW_ERROR_READING_FROM_WEBSOCKET;
					}
				}
			} else {
				/* Length < 126 */
				ws->bytesNeeded = ws->ws_header[1];
			}
			/* We have the entire header */
			opcode = 0xff;
			TW_LOG(TW_TRACE,"twWs_Receive: Got Header: Body length = %d",ws->bytesNeeded);
            TW_LOG_HEX(ws->ws_header, "twWs_Receive: Header Data:\n", ws->headerPtr - ws->ws_header);

			/* set the current read state based on the header op code. This will allow the
			   data after the header to be read in and processed correctly. */
			opcode = ws->ws_header[0] & 0x0f;

			/* Check the compression bit in the header*/
			if (opcode == FRAME_OPCODE_BINARY || opcode == FRAME_OPCODE_TEXT) {
				/* Continuation frames aren't annotated with this flag - the state must be
				** retained from the original frame.
				** Additionally, control frames can be interleaved in fragmented text/binary
				** frames, and control frames aren't compressed.*/
				ws->bCompressedMsg = ws->ws_header[0] & 0x40 ? TRUE : FALSE;
				}

			switch(opcode) {
				case FRAME_OPCODE_CONTINUATION:
					/* Continuation frame */
					if( READ_TEXT_FRAME == ws->previous_read_state ||  READ_BINARY_FRAME == ws->previous_read_state){
						/* "You cannot continue from a state other than a TEXT frame or a BINARY frame" - RFC 6455 5.5
						** (If we had an interleaved control frame, it should have stash and unstashed our old state) */
						ws->read_state = ws->previous_read_state;
					} else {
						/* You cannot continue from a state other than a TEXT frame or a BINARY frame: RFC 6455 5.5*/
						TW_LOG(TW_ERROR,"twWs_Receive: Continuation opcode received without a prior TEXT or BINARY frame preceeding it. Aborting.");
						ws->isConnected = FALSE;
						if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
						twMutex_Unlock(ws->recvMutex);
						restartSocket(ws);
						return TW_ERROR_READING_FROM_WEBSOCKET;
					}
					break;
				case FRAME_OPCODE_TEXT:
					/* Text frame */
					ws->read_state = READ_TEXT_FRAME;
					ws->previous_read_state = READ_TEXT_FRAME;
					break;
				case FRAME_OPCODE_BINARY:
					/* Binary frame */
					ws->read_state = READ_BINARY_FRAME;
					ws->previous_read_state = READ_BINARY_FRAME;
					break;
				case FRAME_OPCODE_CONNECTION_CLOSE:
				case FRAME_OPCODE_PING:
				case FRAME_OPCODE_PONG:
					/* "Control frames can be interjected in the middle of a fragmented message"  -- RFC-6455 5.5 */
					savedState = ws->read_state;
					ws->read_state = READ_CONTROL_FRAME;
					break;
				default:
					TW_LOG(TW_ERROR,"twWs_Receive: Error reading from websocket. Unknown opcode: %d", opcode);
					ws->isConnected = FALSE;
					if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
					twMutex_Unlock(ws->recvMutex);
					restartSocket(ws);
					return TW_ERROR_READING_FROM_WEBSOCKET;
			}
			/* Sanity check - do we need any data? */
			if (!ws->bytesNeeded) {
				TW_LOG(TW_WARN,"twWs_Receive: Got header, but frame size is 0");
				ws->read_state = READ_HEADER;
				ws->bytesNeeded = WS_HEADER_MIN_SIZE;
				ws->headerPtr = ws->ws_header;
				twMutex_Unlock(ws->recvMutex);
				return TW_OK;
			}
		} else {
			if (bytesRead < 0) {
				TW_LOG(TW_DEBUG,"twWs_Receive: Read returned an error value of %d", bytesRead);
				TW_LOG(TW_WARN,"twWs_Receive: Error reading from socket.  Error: %d", twSocket_GetLastError());
				ws->isConnected = FALSE;
				if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
				twMutex_Unlock(ws->recvMutex);
				restartSocket(ws);
				return TW_ERROR_READING_FROM_WEBSOCKET;
			}
			
			twMutex_Unlock(ws->recvMutex);
			return TW_OK;
		}
	}

	maxtime = twGetSystemMillisecondCount() + twcfg.frame_read_timeout;   
	while (ws->bytesNeeded && (ws->read_state == READ_CONTROL_FRAME || ws->read_state == READ_TEXT_FRAME || ws->read_state == READ_BINARY_FRAME)) { /* READ_BODY */

		/* Only attempt to read a frame for so long. */
		if (twTimeGreaterThan(twGetSystemMillisecondCount(), maxtime)) {
			/* We've been trying to read this frame for too long. Give up and reconnect. */
			TW_LOG(TW_ERROR,"twWs_Receive: Timed out attempting to read frame. Got %d bytes.", ws->frameBufferPtr - ws->frameBuffer);
			ws->isConnected = FALSE;
			if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Frame read timeout", strnlen("Frame read timeout", MAX_MSG_SIZE));
			twMutex_Unlock(ws->recvMutex);
			restartSocket(ws);
			return TW_READ_TIMEOUT;
		}

		bytesRead = s_twTlsClient_Read(ws->connection, ws->frameBufferPtr, ws->bytesNeeded,  twcfg.socket_read_timeout);
		if (bytesRead > 0) {
			char opcode = 0xff;
			TW_LOG(TW_TRACE,"twWs_Receive: Read %d bytes into Frame buffer", bytesRead);
			ws->frameBufferPtr = ws->frameBufferPtr + bytesRead;
			ws->bytesNeeded = ws->bytesNeeded - bytesRead;
			/* Do we still need more bytes? */
			if (ws->bytesNeeded > 0) {
				TW_LOG(TW_TRACE,"twWs_Receive: Don't have a full frame yet. Still need %d bytes. Will try again", ws->bytesNeeded);
				continue;
			} else if (ws->bytesNeeded < 0) {
				TW_LOG(TW_ERROR,"twWs_Receive: Error reading from websocket.  Too much data read");
				ws->isConnected = FALSE;
				if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
				twMutex_Unlock(ws->recvMutex);
				restartSocket(ws);
				return TW_ERROR_READING_FROM_WEBSOCKET;
			}
			
            if (ws->bCompressedMsg) {
                TW_LOG_HEX (ws->frameBuffer, "twWs_Receive: Got (Compressed) Body:\n", ws->frameBufferPtr - ws->frameBuffer);
            } else {
                TW_LOG_HEX (ws->frameBuffer, "twWs_Receive: Got Body:\n", ws->frameBufferPtr - ws->frameBuffer);
            }

			/* Check the FIN bit */
			if ((ws->ws_header[0] & 0x80) == 0x00) {
				/* This is not a final message.
				** Set aside its payload and move to the next fragment */
				TW_LOG (TW_TRACE, "twWs_Receive: Handled complete non-FINAL frame. Awaiting continuation frames.");
				if (!ws->multiframeRecvStream) {
					ws->multiframeRecvStream = twStream_Create ();
					if (!ws->multiframeRecvStream) {
						TW_LOG (TW_ERROR, "twWs_Receive: Error allocating fragmentation buffer.");
						ws->isConnected = FALSE;
						if (ws && ws->on_ws_close) ws->on_ws_close (ws, "Allocation failure", strnlen ("Allocation failure", MAX_MSG_SIZE));
						twMutex_Unlock (ws->recvMutex);
						restartSocket (ws);
						return TW_ERROR_READING_FROM_WEBSOCKET;
						}
				}
				twStream_AddBytes (ws->multiframeRecvStream, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);
				memset (ws->ws_header, 0, 16);
				ws->read_state = READ_HEADER;
				ws->headerPtr = ws->ws_header;
				ws->bytesNeeded = WS_HEADER_MIN_SIZE;
				ws->frameBufferPtr = ws->frameBuffer;
				twMutex_Unlock (ws->recvMutex);
				return TW_OK;
			}

			/* Received the final frame of this message, check the op code */
			opcode = ws->ws_header[0] & 0x0f;

			if (opcode == FRAME_OPCODE_CONTINUATION) {
				uint32_t byteCount = 0;
				/* Continuation frame */
				TW_LOG(TW_TRACE,"twWs_Receive: Received FINAL Continuation Frame");
				/* We should have a fragmentation stream by now, but make sure. */
				if (!ws->multiframeRecvStream) {
					TW_LOG (TW_ERROR, "twWs_Receive: Unexpected continuation frame following FINAL frame");
					ws->isConnected = FALSE;
					if (ws && ws->on_ws_close) ws->on_ws_close (ws, "Socket Error", strnlen ("Socket Error", MAX_MSG_SIZE));
					twMutex_Unlock (ws->recvMutex);
					restartSocket (ws);
					return TW_ERROR_READING_FROM_WEBSOCKET;
				}
				/* Apply some sensible defensive maximum here. */
				byteCount = ws->frameBufferPtr - ws->frameBuffer;
				if (twStream_GetLength (ws->multiframeRecvStream) + byteCount > getMaxWebsocketMessageSize (ws->read_state == READ_TEXT_FRAME)) {
					TW_LOG (TW_ERROR, "twWs_Receive: Excessively large fragmented message; aborting.");
					ws->isConnected = FALSE;
					if (ws && ws->on_ws_close) ws->on_ws_close (ws, "Socket Error", strnlen ("Socket Error", MAX_MSG_SIZE));
					twMutex_Unlock (ws->recvMutex);
					restartSocket (ws);
					return TW_ERROR_READING_FROM_WEBSOCKET;
					}
				twStream_AddBytes (ws->multiframeRecvStream, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);

				if (ws->read_state == READ_TEXT_FRAME) {
					TW_LOG(TW_TRACE,"twWs_Receive: Received Multiframe Text Message");
					if (ws->on_ws_textMessage) 
						(*ws->on_ws_textMessage)(ws, ws->multiframeRecvStream->data, ws->multiframeRecvStream->length);
				} else {
					TW_LOG(TW_TRACE,"twWs_Receive: Received Multiframe Binary Message");
					if (ws->on_ws_binaryMessage)
						(*ws->on_ws_binaryMessage)(ws, ws->multiframeRecvStream->data, ws->multiframeRecvStream->length, ws->bCompressedMsg);
				}
				twStream_Delete (ws->multiframeRecvStream);
				ws->multiframeRecvStream = NULL;
			} else if (opcode == FRAME_OPCODE_TEXT) {
				/* Text Message in single Frame */
				TW_LOG(TW_TRACE,"twWs_Receive: Received Text Message in Single Frame");
				if (ws->on_ws_textMessage) 
					(*ws->on_ws_textMessage)(ws, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);
			} else if (opcode == FRAME_OPCODE_BINARY) {
				/* Binary message in single frame */
				TW_LOG(TW_TRACE,"twWs_Receive: Received Binary Message in Single Frame");
				if (ws->on_ws_binaryMessage) 
					(*ws->on_ws_binaryMessage)(ws, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer, ws->bCompressedMsg);
			} else if (opcode == FRAME_OPCODE_CONNECTION_CLOSE) {
				/* Connection close */
				TW_LOG(TW_WARN,"twWs_Receive: Websocket closed!");
				ws->isConnected = FALSE;
				if (ws->on_ws_close) 
					ws->on_ws_close(ws, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);
				ws->read_state = savedState;
			} else if (opcode == FRAME_OPCODE_PING) {
				/* Ping */
				TW_LOG(TW_TRACE,"twWs_Receive: Received Ping");
				if (ws->on_ws_ping) 
					ws->on_ws_ping(ws, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);
				ws->read_state = savedState;
			} else if (opcode == FRAME_OPCODE_PONG) {
				/* Pong */
				TW_LOG(TW_TRACE,"twWs_Receive: Received Pong");
				if (ws->on_ws_pong) 
					ws->on_ws_pong(ws, ws->frameBuffer, ws->frameBufferPtr - ws->frameBuffer);
				ws->read_state = savedState;
			} else {
				TW_LOG(TW_ERROR,"twWs_Receive: Error reading from websocket. Unknown opcode: %d", opcode);
				ws->isConnected = FALSE;
				if (ws && ws->on_ws_close) 
					ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
				twMutex_Unlock(ws->recvMutex);
				restartSocket(ws);
				return TW_ERROR_READING_FROM_WEBSOCKET;
			}

			/* Reset for the next message */
			memset(ws->ws_header,0,16);
			ws->read_state = READ_HEADER;
			ws->headerPtr = ws->ws_header;
			ws->bytesNeeded = WS_HEADER_MIN_SIZE;
			ws->frameBufferPtr = ws->frameBuffer;
			twMutex_Unlock(ws->recvMutex);
			return TW_OK;
		} else if (bytesRead < 0) {
			TW_LOG(TW_WARN,"twWs_Receive: Error reading from socket. bytesRead: %d, Error: %d", bytesRead, twSocket_GetLastError());
			ws->isConnected = FALSE;
			if (ws && ws->on_ws_close) ws->on_ws_close(ws, "Socket Error", strnlen("Socket Error", MAX_MSG_SIZE));
			twMutex_Unlock(ws->recvMutex);
			restartSocket(ws);
			return TW_ERROR_READING_FROM_WEBSOCKET;
		}
		/* If bytesRead == 0 then continue our while loop until we have a frame or we time out. */
	}

	TW_LOG(TW_WARN,"twWs_Receive: read_state is %d, but bytesNeeded is %d.", ws->read_state, ws->bytesNeeded);
	ws->read_state = READ_HEADER;
	ws->bytesNeeded = WS_HEADER_MIN_SIZE;
	ws->headerPtr = ws->ws_header;
	twMutex_Unlock(ws->recvMutex);
	return TW_OK;
}

int twWs_SendMessage(twWs * ws, char * buf, uint32_t length, char isText) {
	char * ptr = buf;
    uint32_t msgLength = length;
	char framesSent = 0;
	twStream* compressed = NULL;
	int res = TW_UNKNOWN_ERROR;

	/* Do some status checks */
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_SendMessage: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (!ws->isConnected) { 
		TW_LOG(TW_WARN, "twWs_SendMessage: Not connected"); 
		return TW_WEBSOCKET_NOT_CONNECTED; 
	}

	/* Make sure we have a message and the original message is under the max AlwaysOn message chunk size */
	if (!buf) { TW_LOG(TW_ERROR, "twWs_SendMessage: NULL msg pointer"); return -1; }
	if (length == 0) { TW_LOG(TW_ERROR, "twWs_SendMessage: Message length is 0.  Not sending"); return -1; }
	if (ws->messageChunkSize < length) { 
		TW_LOG(TW_ERROR, "twWs_SendMessage: Message or multipart message chunk of length %d is too large.  Max AlwaysOn chunk size is %u", 
		length, ws->messageChunkSize);
		return TW_WEBSOCKET_FRAME_TOO_LARGE;
	}

	twMutex_Lock(ws->sendMessageMutex);

	/* Does the data need to be compressed?  If so, make sure to do this behind the
	 * sendMessageMutex so adding data to the compression context's sliding window
	 * and transmitting it are an atomic operation, thread wise. */
	if (!ws->bDisableCompression && ws->bSupportsPermessageDeflate)	{
		compressed = twStream_Create ();
		if (!compressed) {
			twMutex_Unlock(ws->sendMessageMutex);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		/* Compress the stream */
		if (s_twCompressBytes(buf, length, compressed, ws)) {
			twMutex_Unlock(ws->sendMessageMutex);
			twStream_Delete (compressed);
			return TW_ERROR_ALLOCATING_MEMORY;
		}
		ptr = twStream_GetData(compressed);
		length = twStream_GetLength(compressed);
	}

	/* In some cases, applying compression to a message can actually cause it to grow. We therefore may
	 * need to fragment this websocket message, per the WS frame size */
	while (length > 0) {
		if (length > ws->frameSize) {
			if (framesSent) res = s_twWs_SendDataFrame(ws, ptr, ws->frameSize, 1, 0, isText); /* Continuation, not Final */
			else {
				res = s_twWs_SendDataFrame(ws, ptr, ws->frameSize, 0, 0, isText); /* Not Continuation, not Final */
			}
			if (res != 0) {
				TW_LOG(TW_ERROR, "twWs_SendMessage: Error sending frame %d. Error code: %d", framesSent, twSocket_GetLastError());
				twMutex_Unlock(ws->sendMessageMutex);
				if (compressed) twStream_Delete (compressed);
				return res;
			}
			framesSent++;
			ptr = ptr + ws->frameSize;
			length = length - ws->frameSize;
		} else {
			if (framesSent) res = s_twWs_SendDataFrame(ws, ptr, length, 1, 1, isText); /* Continuation, Final */
			else {
				res = s_twWs_SendDataFrame(ws, ptr, length, 0, 1, isText); /* Not Continuation, not Final */
			}
			if (res != 0) {
				TW_LOG(TW_ERROR, "twWs_SendMessage: Error sending frame %d. Error code: %d", framesSent, twSocket_GetLastError());
				twMutex_Unlock(ws->sendMessageMutex);
				if (compressed) twStream_Delete (compressed);
				return res;
			}
			framesSent++;
			ptr = ptr + length;
			length = 0;
		}
	}
	TW_LOG(TW_DEBUG,"twWs_SendMessage: Sent %d bytes using %d frames.", msgLength, framesSent);
	TW_LOG_HEX(buf, "Sent Message >>>>\n", msgLength);
	twMutex_Unlock(ws->sendMessageMutex);
	if (compressed) twStream_Delete (compressed);
	return TW_OK;
}

int twWs_SendPing(twWs * ws, char * msg) {
	char tmp[64];
	memset(tmp, 0, 64);
	if (!msg) {
		twGetSystemTimeString(tmp, "%H:%M:%S", 63, 0, 0);
		msg = tmp;
	} 
	return sendCtlFrame(ws, 0x09, msg);
}

int twWs_SendPong(twWs * ws, char * msg) {
	return sendCtlFrame(ws, 0x0A, msg);
}

/**
* Websocket helper functions
**/
#define MAX_WS_MSG_SIZE 110
#define MAX_CTL_FRAME_SIZE 1024
int sendCtlFrame(twWs * ws, unsigned char type, char * msg) {
	/* Send a control frame */
	int bytesToWrite = 0;
	int bytesWritten = 0;
	int res = 0;
	char frameHeader[128];
	char typeStr[8] = "Unknown";
	if (type == 0x08) strcpy(typeStr,"Close");
	else if (type == 0x09) strcpy(typeStr,"Ping");
	else if (type == 0x0A) strcpy(typeStr,"Pong");

	if (!ws) { 
		TW_LOG(TW_ERROR, "sendCtlFrame: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (!ws->isConnected) { 
		TW_LOG(TW_WARN, "sendCtlFrame: Not connected"); 
		return TW_WEBSOCKET_NOT_CONNECTED; 
	}

	/* Make sure we have a valid message */
	if (!msg) msg = typeStr;

	if (type < 0x08 && type > 0x0a) {
		TW_LOG(TW_ERROR,"sendCtlFrame: Invalid frame type: 0x%x", type);
		return TW_INVALID_WEBSOCKET_FRAME_TYPE;
	}
	if (strnlen(msg, MAX_CTL_FRAME_SIZE) > MAX_WS_MSG_SIZE) {
		TW_LOG(TW_ERROR,"sendCtlFrame: Message too long.  Length = ", strnlen(msg, MAX_CTL_FRAME_SIZE));
		return TW_WEBSOCKET_MSG_TOO_LARGE;
	}
	twMutex_Lock(ws->sendFrameMutex);
	TW_LOG(TW_DEBUG,"sendCtlFrame: >>>>> Sending %s. Msg: %s", typeStr, msg);
	memset(frameHeader,0,6);
	frameHeader[0] = 0x80 + type;
	frameHeader[1] = 0x80 + (char)strnlen(msg, MAX_WS_MSG_SIZE);
	/* Masking is set to 0x00 so nothing else to do */
	strcpy(frameHeader + 6, msg);
	bytesToWrite = strnlen(msg, MAX_WS_MSG_SIZE) + 6;
	bytesWritten = twTlsClient_Write(ws->connection, frameHeader, bytesToWrite, 100);

	if (bytesWritten != bytesToWrite) {
		TW_LOG(TW_WARN,"sendCtlFrame: Error writing to socket.  Error: %d", twSocket_GetLastError());
		ws->isConnected = FALSE;
		restartSocket(ws);
		res = TW_ERROR_WRITING_TO_WEBSOCKET;
	}
	twMutex_Unlock(ws->sendFrameMutex);
	return res;
}

int twWs_SendDataFrame(twWs * ws, char * msg, uint16_t length, char isContinuation, char isFinal, char isText) {

	int bytesToWrite = 0;
	int bytesWritten = 0;
	char frameHeader[12];
	unsigned char headerLength = 6;
	char type = 0x02;  /* Default to Binary complete frame */

	/* Do some status checks */
	if (!ws) { 
		TW_LOG(TW_ERROR, "twWs_SendDataFrame: NULL ws pointer"); 
		return TW_INVALID_PARAM; 
	}
	if (ws->isConnected != TRUE) { 
		TW_LOG(TW_WARN, "twWs_SendDataFrame: Not connected"); 
		return TW_WEBSOCKET_NOT_CONNECTED; 
	}

	/* Make sure we have a message and it fits in a frame */
	if (!msg) { TW_LOG(TW_ERROR, "twWs_SendDataFrame: NULL msg pointer"); return -1; }
	if (ws->frameSize < length) { 
		TW_LOG(TW_WARN, "twWs_SendDataFrame: Frame of length %d is too large.  Max frame size is %u", 
		length, ws->frameSize); 
		return TW_WEBSOCKET_MSG_TOO_LARGE; 
	}

	twMutex_Lock(ws->sendFrameMutex);
	/* Figure out the type */
	if (isText) type = 0x01;
	if (isContinuation) type = 0x00;
	if (isFinal) type = type | 0x80;

	/* Add compression */
	if (ws->bSupportsPermessageDeflate && !ws->bDisableCompression) {
		type = type | 0x40;
	}

	/* Prep the header */
	memset(frameHeader,0,12);
	frameHeader[0] = type;
	/* Set up the length */
	if (length < 126) frameHeader[1] = 0x80 + length;
	else {
		headerLength = 8;
		frameHeader[1] = (char)0xFE; /* (char)(0x80 + 126); */
		frameHeader[2] = (char)(length / 0x100);
		frameHeader[3] = (char)(length % 0x100);
	} 
	/* Masking is set to 0x00 so nothing else to do */
	bytesToWrite = headerLength + length;
	bytesWritten += twTlsClient_Write(ws->connection, frameHeader, headerLength, 100);
	bytesWritten += twTlsClient_Write(ws->connection, msg, length, 100);

	if (bytesWritten != bytesToWrite) {
		TW_LOG(TW_WARN,"twWs_SendDataFrame: Error writing to socket.  Error: %d", twSocket_GetLastError());
		ws->isConnected = FALSE;
        twMutex_Unlock(ws->sendFrameMutex);
		restartSocket(ws);
		return TW_ERROR_WRITING_TO_WEBSOCKET;
	}
	twMutex_Unlock(ws->sendFrameMutex);
	return TW_OK;
}

#define MAX_KEY_LEN 80
int validateAcceptKey(twWs * ws, const char * val) {
	char tmp[MAX_KEY_LEN];
	unsigned char hash[20];
	unsigned char expectedValue[40];
	TW_SHA1_CTX sha;
	unsigned long len = 40;
	if (!ws) { TW_LOG(TW_ERROR, "validateAcceptKey: NULL ws pointer"); return -1; }
	/* Calculate the expected value */
	strcpy(tmp, ws->security_key);
	strcat(tmp, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	twSHA1_Init(&sha);
	twSHA1_Update(&sha, (unsigned char *)tmp, strnlen(tmp, MAX_KEY_LEN));
	twSHA1_Final(hash, &sha);
	memset(expectedValue, 0, 40);
	base64_encode(hash, 20, expectedValue, &len);
	/* Compare the received value */
	if (strcmp(val, (char *)expectedValue)) {
		TW_LOG(TW_ERROR,"validateAcceptKey: Keys don't match. Expected %s, Received %s",
			expectedValue, val);
		return TW_INVALID_ACCEPT_KEY;
	}
	return TW_OK;
}

