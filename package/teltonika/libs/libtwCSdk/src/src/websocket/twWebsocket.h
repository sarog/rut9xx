/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twWebsocket.h
 *
 * \brief Websocket client abstraction layer
 *
 * Contains structure type definitions and function prototypes for ThingWorx Websockets.
*/
#ifndef TW_WEBSOCKET_H
#define TW_WEBSOCKET_H

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twBaseTypes.h"
#include "twPasswds.h"

#include "zlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Web socket frame op codes */
#define FRAME_OPCODE_CONTINUATION 0x00
#define FRAME_OPCODE_TEXT 0x01
#define FRAME_OPCODE_BINARY 0x02
#define FRAME_OPCODE_CONNECTION_CLOSE 0x08
#define FRAME_OPCODE_PING 0x09
#define FRAME_OPCODE_PONG 0x0A

/* Internal state machine states */
#define READ_HEADER 0
#define READ_CONTROL_FRAME 1
#define READ_TEXT_FRAME 2
#define READ_BINARY_FRAME 3

/* WebSocket compression */
#define ZLIB_WINDOW_BITS_RAW_DEFLATE -15
#define ZLIB_DEFAULT_MEM_LEVEL 8

/*
HTTP Parser declarations for the 
joyent http-parser library
*/
struct http_parser;
struct http_parser_settings;

/* 
Forward declarations of the struct and call back functions 
used by the http-parser library
*/
struct twWs;
typedef int (*ws_cb) (struct twWs * ws);
typedef int (*ws_data_cb) (struct twWs * ws, const char *at, size_t length);
typedef int (*ws_binary_data_cb) (struct twWs * ws, const char *at, size_t length, char bCompressed);

/* 
Helper macros 
*/
#define WS_TLS_CONN(a) (twTlsClient *)a->connection

/**
 * \brief Websocket close reasoning enumeration.
*/
enum close_status {
	 SERVER_CLOSED = 0      /**< 0    - Server closed. **/
	,NORMAL_CLOSE = 1000    /**< 1000 - Normal close. **/
	,GOING_TO_SLEEP         /**< 1001 - Going to sleep. **/
	,PROTOCOL_ERROR         /**< 1002 - Protocol error. **/
	,UNSUPPORTED_DATA_TYPE  /**< 1003 - Unsupported data type. **/
	,RESERVED1              /**< 1004 - RESERVED. **/
	,RESERVED2              /**< 1005 - RESERVED. **/
	,RESERVED3              /**< 1006 - RESERVED. **/
	,INVALID_DATA           /**< 1007 - Invalid data. **/
	,POLICY_VIOLATION       /**< 1008 - Policy violation. **/
	,FRAME_TOO_LARGE        /**< 1009 - Frame too large. **/
	,NO_EXTENSION_FOUND     /**< 1010 - No extension found. **/
	,UNEXPECTED_CONDITION   /**< 1011 - Unexpected condition. **/
};

/**
 * \brief Websocket entity structure definition.
*/
typedef struct twWs{
	struct twTlsClient * connection;        /**< Pointer to a TLS client connection structure. **/
	uint32_t messageChunkSize;              /**< Max size (in bytes) of multipart message chunk. **/
	int32_t bytesNeeded;                    /**< How many bytes we should read next. **/
	char read_state;                        /**< READ_HEADER or READ_BODY. **/
	char previous_read_state;               /**  Used to remember the previous read state if a continuation op code is received **/
	uint16_t frameSize;                     /**< Max size of a websocket frame (not to be confused with max ThingWorx message size .**/
	char * frameBuffer;                     /**< Pointer to a frame buffer. **/
	char * frameBufferPtr;                  /**< A pointer to the websocket's frame buffer.  **/
	unsigned char ws_header[64];            /**< A buffer to receive websocket frame headers.  **/
	unsigned char * headerPtr;              /**< Pointer to a the header buffer. **/
	twStream * multiframeRecvStream;		/**< Stream containing the concatenated payloads of a fragmented WS msg; only present during a fragmented recv. **/
	char * host;                            /**< The host name of the websocket server. **/
	uint16_t port;                          /**< The port that the websocket server is listening on. **/
	twPasswdCallbackFunction api_key_callback;
	/**
	 * This is a pointer to a function which will be called whenever the client must
	 * 	provide an application key for authentication. You must implement this function
	 * 	and pass a pointer to it to initialize the API. This function must be of type
	 * 	twPasswdCallbackFunction and should have a format like the one shown below.
	 *
	 * 	myPasswordCallback(char* passwdBuffer,unsigned int maxPasswordLength);
	 *
	 * 	The implementation of this function must fill passwdBuffer with the current
	 * 	application key.
	 * < The API key that will be used during an ensuing authentication process.
	 **/
	char * gatewayName;                     /**< An optional name if the SDK is being used to develop a gateway application which allows 
                                               multiple Things to connect through it.  If not NULL this is used during the binding process. **/
	char * gatewayType;                     /**< An optional type if the SDK is being used to develop a gateway application which allows 
                                               multiple Things to connect through it.  If not NULL this is used during the binding process. **/
	unsigned char * security_key;           /**< websocket security key. **/
	uint32_t sessionId;                     /**< Unique session ID. **/
	char * resource;                        /**< The HTTP resource of the connection. **/
	TW_MUTEX sendMessageMutex;              /**< A mutex for sending messages. **/
	TW_MUTEX sendFrameMutex;                /**< A mutex for sending frames. **/
	TW_MUTEX recvMutex;                     /**< A mutex for receiving data. **/
	signed char connect_state;              /**< The connection state of the websocket. **/
	signed char isConnected;                /**< TRUE signifies the websocket is connected. **/
	ws_cb on_ws_connected;                  /**< Pointer to a callback function registered to be called when the websocket connection is successfully established. **/
	ws_binary_data_cb on_ws_binaryMessage;  /**< Pointer to a callback function registered to be called when a complete  binary message is received. **/
	ws_data_cb on_ws_textMessage;           /**< Pointer to a callback function registered to be called when a complete text message is received. **/
	ws_data_cb on_ws_ping;                  /**< Pointer to a callback function registered to be called when a Ping is received. **/
	ws_data_cb on_ws_pong;                  /**< Pointer to a callback function registered to be called when a Pong is received. **/
	ws_data_cb on_ws_close;                 /**< Pointer to a callback function registered to be called when the server closes the websocket connection. **/
	z_stream defstream;						/** ZLib deflate stream. **/
	char bDeflateInitialized;				/** State of ZLib deflate context **/
	TW_MUTEX compressionMutex;				/** Lock for ZLib deflate context **/
	z_stream inflstream;					/** ZLib inflate stream. **/
	char bInflateInitialized;				/** State of ZLib inflate context **/
	TW_MUTEX decompressionMutex;			/** Lock for ZLib deflate context **/
	char bCompressedMsg;					/** True when the message being received is a compressed message (text and binary messages). If the message is
											 ** fragmented, this state is preserved across receipt of the continuation frames (which aren't marked as compressed),
											 ** and also across receipt of interleaved control frames (which are themselves never compressed).*/
	char bSupportsPermessageDeflate;		/** True if the server supports Permessage-Deflate (after Upgrade negotiations) **/
	char bDisableCompression;				/** Configurable setting to disable WebSocket compression **/
} twWs;

/**
 * \brief Creates a new websocket struct and the underlying dependent
 * components.
 *
 * \param[in]     host               The hostname of the websocket server.
 * \param[in]     port               The port that the websocket server is
 *                                   listening on.
 * \param[in]     resource           The HTTP resource to use when establishing
 *                                   a connection.
 * \param[in]     twPasswdCallbackFunction            This callback will be used to request an
 *                                   apiKey to be used in the ensuing authentication process.
 * \param[in]     gatewayName        An optional name used if the SDK is being
 *                                   used to develop a gateway application
 *                                   which allows multiple Things to connect
 *                                   through it.  If not NULL, this is used
 *                                   during the binding process.
 * \param[in]     messageChunkSize   The maximum size (in bytes) of a multipart
 *                                   message chunk.
 * \param[in]     frameSize          The maximum websocket frame size (not to
 *                                   be confused with the maximum ThingWorx
 *                                   message size).
 * \param[out]    entity             A pointer to the newly allocated ::twWs
 *                                   structure.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note This function does <b>not</b> attempt to establish a connection.
 * \note The calling function retains ownership of the \p entity pointer and is
 * responsible for freeing it via twWs_Delete().
*/
int twWs_Create(char * host, uint16_t port, char * resource,  twPasswdCallbackFunction app_key_function, char * gatewayName,
				   uint32_t messageChunkSize, uint16_t frameSize, twWs ** entity);

/**
 * \brief Frees all memory associated with a ::twWs structure and all its owned
 * substructures.
 *
 * \param[in]     ws    The ::twWs structure to delete.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note This function will shut down any connection associated with the
 * structure before deleting it.
*/
int twWs_Delete(twWs * ws);

/**
 * \brief Establishes a websocket connection to the server.
 *
 * \param[in]     ws        The ::twWs structure to connect.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for the
 *                           connection to be established.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_Connect(twWs * ws, uint32_t timeout);

/**
 * \brief Disconnect a websocket connection from the server.
 *
 * \param[in]     ws        The ::twWs structure to disconnect.
 * \oaram[in]     code      Close status code.
 * \param[in]     reason    The reason for the disconnection.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_Disconnect(twWs * ws, enum close_status code, char * reason);

/**
 * \brief Gets the connection status of a websocket entity structure.
 *
 * \param[in]     ws        The websocket entity structure to get the
 *                          connection status of.
 *
 * \return #TRUE if \p ws is connected, #FALSE if it isn't.
*/
char twWs_IsConnected(twWs * ws);

/**
 * \brief Registers a function to be called when the websocket is successfully
 * connected.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterConnectCallback(twWs * ws, ws_cb cb);

/**
 * \brief Registers a function to be called when the websocket is closed by the
 * server.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterCloseCallback(twWs * ws, ws_data_cb cb);

/**
 * \brief Registers a function to be called when the websocket receives a
 * complete binary message.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterBinaryMessageCallback(twWs * ws, ws_binary_data_cb cb);

/**
 * \brief Registers a function to be called when the websocket receives a
 * complete text message.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterTextMessageCallback(twWs * ws, ws_data_cb cb);

/**
 * \brief Registers a function to be called when the websocket receives a Ping
 * message.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterPingCallback(twWs * ws, ws_data_cb cb);

/**
 * \brief Registers a function to be called when the websocket receives a Pong
 * message.
 *
 * \param[in]     ws        The ::twWs structure to register with.
 * \param[in]     cb        A pointer to the function to register.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twWs_RegisterPongCallback(twWs * ws, ws_data_cb cb);

/**
 * \brief Check the websocket for data and drive the state machine of the
 * websocket.
 *
 * \param[in]     ws        The ::twWs structure to utilize.
 * \param[in]     timeout   Time (in miliseconds) to wait for data on the
 *                          socket.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note This function must be called on a regular basis.
 * \note No data is returned as the data is delivered through the state machine
 * callback functions.
*/
int twWs_Receive(twWs * ws, uint32_t timeout);

/**
 * \brief Send a message over the websocket.
 *
 * \param[in]     ws        The ::twWs structure to utilize.
 * \param[in]     buf       A pointer to the buffer containing the message.
 * \param[in]     length    The length of the message.
 * \param[in]     isText    If #TRUE, will be sent as a text message, if #FALSE
 *                          will be sent as a binary message.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The message will be broken up into multiple frames, if necessary.
 * \note The message will be compressed, if required by the connection.
*/
int twWs_SendMessage(twWs * ws, char * buf, uint32_t length, char isText);

/**
 * \brief Send a Ping message over the websocket.
 *
 * \param[in]     ws        The ::twWs structure to utilize.
 * \param[in]     msg       Less than 126 byte NULL-terminated string to send
 *                          with the Ping.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p msg data <b>must</b> be NULL-terminated and less than 126
 * bytes.
*/
int twWs_SendPing(twWs * ws, char * msg);

/**
 * \brief Send a Pong message over the websocket.
 *
 * \param[in]     ws        The ::twWs structure to utilize.
 * \param[in]     msg       Less than 126 byte NULL-terminated string to send
 *                          with the Pong.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The \p msg data <b>must</b> be NULL-terminated and less than 126
 * bytes.
*/
int twWs_SendPong(twWs * ws, char * msg);

/**
* Internal function; exposed to facilitate unit testing
*/
int twWs_SendDataFrame (twWs * ws, char * msg, uint16_t length, char isContinuation, char isFinal, char isText);

#ifdef __cplusplus
}
#endif

#endif



