/*
 *  Copyright (C) 2015 ThingWorx Inc.
 *
 *  Offline message Store
 */

#ifndef TW_OFFLINE_MSG_STORE_H
#define TW_OFFLINE_MSG_STORE_H

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twMessages.h"
#include "twErrors.h"
#include "twWebsocket.h"
#include "twList.h"
#include "twLogger.h"
#include "twApi.h"
#include "stringUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name Offline Message Store Structure Definition
*/

/* string used to separate messages in the offline message store */
#define PERSISTED_MSG_SEPARATOR "!twMsg!"

/**
 * \brief ThingWorx Offline Message Store structure definition.
 *
 * \note A singleton instance of this structure is automatically created when
 * twOfflineMsgStore_Initialize() is called.  There should be no need to manipulate this
 * structure directly.
*/
typedef struct twOfflineMsgStore {
	TW_MUTEX mtx;                            /**< ThingWorx mutex. **/
	char offlineMsgEnabled;                  /**< Offline message enabled flag. **/
	char onDisk;					/**< Offline message enabled flag. **/
	uint64_t offlineMsgSize;                 /**< The size of the offline message(s). **/
	twList * offlineMsgList;                 /**< Pointer to a ::twList of offline messages. **/
	char * offlineMsgFile;                   /**< The offline message filename. **/
} twOfflineMsgStore;

/**
 * \brief Offline message store request type enumeration
**/
enum OfflineRequest {
	OFFLINE_MSG_STORE_FLUSH, /**< Attempt to flush the offline message store buffer. **/
	OFFLINE_MSG_STORE_WRITE, /**< Attempt tp write a message into the offline message store. **/
};

/**
 * \brief Initialize the offline message store
 *
 * \param[in]     enabled	boolean value to enable/disable the offline message store
 * \param[in]     filePath	path to the offline message store directory
 * \param[in]     size		maximum size of offline message store
 *
 * \return TW_OK on success otherwise an error found in twErrors.h.
*/
int twOfflineMsgStore_Initialize(char enabled, const char * filePath, uint64_t size, char onDisk);

/**
 * \brief set offline message store directory (this is where the offline messages will be stored)
 * 
 * \param[in]	dir	constant character string that represents the path of the offline message store directory
 * 
 * \return TW_OK on success otherwise an error found in twErrors.h.
*/
int twOfflineMsgStore_SetDir(const char * dir);

/**
 * \brief free the memory associated with the offline message store singleton
 * 
 * \return TW_OK on success otherwise an error found in twErrors.h.
*/
int twOfflineMsgStore_Delete();

/**
 * \brief Handles Offline Message Store requests
 *
 * \param[in]     msg			twMessage structure containing the current message
 * \param[in]     ws			twWs structure containing active websocket
 * \param[in]     request_type	see OfflineRequest enum for request types
 *
 * \return TW_OK on success otherwise an error found in twErrors.h.
*/
int twOfflineMsgStore_HandleRequest(struct twMessage ** msg, twWs * ws, enum OfflineRequest request_type);

#ifdef __cplusplus
}
#endif

#endif
