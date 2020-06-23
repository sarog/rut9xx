/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twDefaultSettings.h
 * \brief Default settings for ThingWorx C SDK
*/

#ifndef TW_DEFAULT_SETTINGS_H
#define TW_DEFAULT_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

/********************************/
/*   General API Settings       */
/********************************/
/* 
To minimize code footprint, by default logging is enabled for debug builds 
and disabled for release builds.  Use this to over-ride that setting.
*/
/*#define DBG_LOGGING*/ 

/** 
 * \brief The resource portion of the ThingWorx websocket URI.
*/
#define TW_URI			       "/Thingworx/WS"		

/**
 * \brief The maximum size of a complete message whether it is broken up as a
 * multipart message or not.  Messages larger than this will be rejected.
 * Measured in Bytes.
*/
#define MAX_MESSAGE_SIZE			1048576

/** 
 * \brief The maximum size of a message chunk.  Messages large than this will
 * be broken up into a multipart message.  Measured in Bytes.  This value
 * should be the same as the server side configuration which defaults to 8192.
*/
#define MESSAGE_CHUNK_SIZE          8192       

/**
 * \brief The maximum size of a fragmented WebSocket message used for tunneling.
 * This is the largest fragmented text mode WebSocket Message that the SDK will
 * accept. This setting is not likely to require adjustment.
*/
#define MAX_WS_TUNNEL_MESSAGE_SIZE	32768

/**
 * \brief Time to wait for a response to a message from the server.  Measured
 * in milliseconds.
*/
#define DEFAULT_MESSAGE_TIMEOUT		10000		

/**
 * \brief Websocket keep alive rate.  Used to ensure the connection stays open.
 * Measured in milliseconds.  This value should never be greater than the
 * server side setting which defaults to 60000 milliseconds.
*/
#define PING_RATE					55000	 

/**
 * \brief Time to wait for a response to a ping message from the server.
 * Measured in milliseconds.
*/
#define DEFAULT_PONG_TIMEOUT		10000	

/**
 * \brief Periodic cleanup rate for multipart messages that never receive all
 * of the expected number of message chunks.  Measured in milliseconds.
*/
#define STALE_MSG_CLEANUP_RATE		 (DEFAULT_MESSAGE_TIMEOUT * 5)  

/**
 * \brief Time to wait for the websocket connection to be established.
 * Measured in milliseconds.
*/
#define CONNECT_TIMEOUT				10000		

/**
 * \brief Number of retries used to establish a websocket connect.  The
 * twApi_Connect call returns an error after the retries are exhausted.
*/
#define CONNECT_RETRIES				3

/**
 * \brief "ON" time of the duty cycle modulated AlwaysOn connection.
 * Acceptable values are 0-100%.  A value of 100% means the connection always
 * stays alive.
*/
#define DUTY_CYCLE					20			

/**
 * \brief Period of the duty cycle modulated AlwaysOn connection measured in
 * milliseconds.  A value of 0 means the connections always stays alive.  It is
 * recommended that this value be greater than 10 seconds at a minimum.
*/
#define DUTY_CYCLE_PERIOD			0			

/**
 * \brief Incremental block size for dynamically allocated stream (byte array)
 * variables.  When adding bytes to a stream, this is the size of memory
 * allocated if more memory is needed.TW_MAX_TASKS
*/
#define STREAM_BLOCK_SIZE			256

/**
 * \brief Maximum number of tasks allowed for the built in round robin task
 * execution engine.
*/
#define TW_MAX_TASKS 5

/**
 * \brief File transfer Block size (in Bytes)
*/
#define FILE_XFER_BLOCK_SIZE 128000

/**
 * \brief File transfer Max file size (in Bytes)
*/
#define FILE_XFER_MAX_FILE_SIZE 8000000000

/**
 * \brief File transfer MD5 Buffer size (in Bytes - should be multiple of 64)
*/
#define FILE_XFER_MD5_BLOCK_SIZE 6400

/**
 * \brief File transfer timeout for stalled transfers (in milliseconds)
*/
#define FILE_XFER_TIMEOUT 30000

/**
 * \brief File transfer staging directory for received files
*/
#define FILE_XFER_STAGING_DIR "/opt/thingworx/tw_staging"

/**
 * \brief Offline message and subscribed property manager queue max size
*/
#define OFFLINE_MSG_QUEUE_SIZE 16384

/**
 * \brief Offline message store and subscribed property manager default directories
*/
#define OFFLINE_MSG_STORE_DIR "/opt/thingworx"


/**
 * \brief Maximum msec delay before connecting
*/
#define MAX_CONNECT_DELAY 10000

/**
 * \brief Connection retry interval
*/
#define CONNECT_RETRY_INTERVAL 5000

/**
 * \brief Maximum number of unhandled messages in the message queue
*/
#define MAX_MESSAGES 500

/**
 * \brief Socket read timeout. 
*/
#define DEFAULT_SOCKET_READ_TIMEOUT	100

/**
 * \brief ssl record read timeout. This determines how long calls to the twApi_TaskerFunction() will wait for data to arrive. 
*/
#define DEFAULT_SSL_READ_TIMEOUT	500

/**
* \brief Maximum time to wait for a full SSL frame during a socket read operation.
*/
#define FRAME_READ_TIMEOUT	10000

/**
* \brief Maximum character count of stringlike properties handled by the SDK before truncation
*/
#define MAX_STRING_PROP_LENGTH (1<<20)

/**
 * \brief Structure to allow overriding of defaults at runtime.
*/
typedef struct twConfig {
	const char tasker_enabled;					/**< Default #TRUE. **/
	const char file_xfer_enabled;				/**< Default #TRUE. **/
	const char tunneling_enabled;				/**< Default #TRUE. **/
	const char offline_msg_store;				/**< Default #TRUE. **/
	char subscribed_props_enabled;				/**< Default #TRUE. Subscribed properties are actually always enabled, but this boolean value controls whether the values will be persisted or not in the event of a disconnect**/
	const char * tw_uri;						/**< Default #TW_URI. **/
	uint32_t max_message_size;					/**< Default #MAX_MESSAGE_SIZE. **/
	uint16_t message_chunk_size;				/**< Default #MESSAGE_CHUNK_SIZE. **/
	uint32_t max_ws_tunnel_message_size;		/**< Default #MAX_WS_TUNNEL_MESSAGE_SIZE. **/
	uint32_t default_message_timeout;			/**< Default #DEFAULT_MESSAGE_TIMEOUT. **/
	uint32_t ping_rate;							/**< Default #PING_RATE. **/
	uint32_t pong_timeout;						/**< Default #DEFAULT_PONG_TIMEOUT. **/
	uint32_t stale_msg_cleanup_rate;			/**< Default #STALE_MSG_CLEANUP_RATE. **/
	uint32_t connect_timeout;					/**< Default #CONNECT_TIMEOUT. **/
	int16_t connect_retries;					/**< Default #CONNECT_RETRIES. **/
	uint8_t duty_cycle;							/**< Default #DUTY_CYCLE. **/
	uint32_t duty_cycle_period;					/**< Default #DUTY_CYCLE_PERIOD. **/
	uint16_t stream_block_size;					/**< Default #STREAM_BLOCK_SIZE. **/
	uint32_t file_xfer_block_size;				/**< Default #FILE_XFER_BLOCK_SIZE. **/
	uint64_t file_xfer_max_file_size;			/**< Default #FILE_XFER_MAX_FILE_SIZE. **/
	uint16_t file_xfer_md5_block_size;			/**< Default #FILE_XFER_MD5_BLOCK_SIZE. **/
	uint32_t file_xfer_timeout;					/**< Default #FILE_XFER_TIMEOUT. **/
	char * file_xfer_staging_dir;				/**< Default #FILE_XFER_STAGING_DIR. **/
	uint32_t offline_msg_queue_size;			/**< Default #OFFLINE_MSG_QUEUE_SIZE. **/
	uint32_t subscribed_props_queue_size;		/**< Default #OFFLINE_MSG_QUEUE_SIZE. **/
	uint32_t max_connect_delay;					/**< Default #MAX_CONNECT_DELAY. **/
	uint32_t connect_retry_interval;			/**< Default #CONNECT_RETRY_INTERVAL. **/
	uint32_t max_messages;						/**< Default #MAX_MESSAGES. **/
	uint32_t socket_read_timeout;				/**< Default #DEFAULT_SOCKET_READ_TIMEOUT. **/
	uint32_t ssl_read_timeout;					/**< Default #DEFAULT_SSL_READ_TIMEOUT. **/
	const char * offline_msg_store_dir;			/**< Default #OFFLINE_MSG_STORE_DIR. **/
	const char * subscribed_props_dir;			/**< Default #OFFLINE_MSG_STORE_DIR. **/
	uint32_t frame_read_timeout;				/**< Default #FRAME_READ_TIMEOUT. **/
	char initialize_encryption_library;			/**< Default #TRUE. Set FALSE to allow the linking application to initialize the library. **/
    const char * cipher_set;                    /**< Default to NULL **/
    struct init_cb *initCallback;
	size_t max_string_prop_length;				/**< Default #MAX_STRING_PROP_LENGTH **/
} twConfig;

extern twConfig twcfg;

#ifdef __cplusplus
}
#endif

#endif
