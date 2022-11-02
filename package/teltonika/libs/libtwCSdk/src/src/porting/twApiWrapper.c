#include "twApiWrapper.h"
#include "twThreads.h"
#include "twTunnelManager.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ENABLE_TASKER)
#error The default Tasker must be disabled (ENABLE_TASKER must not be defined).
#endif

#if !defined(ENABLE_FILE_XFER)
#error The FileTransfer subsystem must be enabled (define ENABLE_FILE_XFER).
#endif

#if !defined(ENABLE_TUNNELING)
#error The Tunneling subsystem must be enabled (define ENABLE_TUNNELING).
#endif

#if !defined(OFFLINE_MSG_STORE)
#error The OfflineMessageStore subsystem must be enabled (define OFFLINE_MSG_STORE=1|2).
#endif

#if defined(ENABLE_FIPS_MODE)
	#pragma comment (lib, "ssleay32.lib")
	#pragma comment (lib, "libeay32.lib")
//	#pragma comment (lib, "twSDK-openssl.lib")
#endif

#define GETMETADATA_SERVICE "GetMetadata"
#define NOTIFYPROPERTYUPDATE_SERVICE "NotifyPropertyUpdate"

	/* Declarations for our threads */
	twList *twWMsgHandlerThreadList = NULL;
	twList *twWApiTaskerThreadList = NULL;
	twThread *twWTunnelThread = NULL;

	struct  twWCallbacks {
		processPropertyReadRequest_cb on_prop_read_request;
		processPropertyWriteRequest_cb on_prop_write_request;
		processServiceRequest_cb on_service_request;
		tunnel_cb on_tunnel_state_change;
		fileTransfer_cb on_file_transfer;
	} twWCallbacks;


	//**********  T W A P I  *********//

	int twW_InitializeThreadPool(uint32_t msgHandlerThreadCount, uint32_t apiTaskerThreadCount)
	{
		int code = 0;
		uint32_t iter = 0;
		twThread *tmpThread = NULL;

		TW_LOG(TW_TRACE, "twW_InitializeThreadPool: Creating API worker threads, MsgHandlerThreadCount: %d ApiTaskerThreadCount: %d", msgHandlerThreadCount, apiTaskerThreadCount);

		if (!twWMsgHandlerThreadList) {
			// create a list of worker threads to drive the msgHandlerTask
			twWMsgHandlerThreadList = twList_Create(twThread_Delete);
			if (twWMsgHandlerThreadList) {
				for (iter = 0; iter < msgHandlerThreadCount; iter++) {
					tmpThread = twThread_Create(twMessageHandler_msgHandlerTask, 5, NULL, TRUE);
					if (!tmpThread) {
						TW_LOG(TW_ERROR, "twW_InitializeThreadPool: Could not create MsgHandlerTask worker thread number: %d.", iter);
						twList_Delete(twWMsgHandlerThreadList);
						twWMsgHandlerThreadList = 0;
						code = TW_ERROR_ALLOCATING_MEMORY;
						break;
					}

					twList_Add(twWMsgHandlerThreadList, tmpThread);
				}
			} else {
				TW_LOG(TW_ERROR, "twW_InitializeThreadPool: Could not create MsgHandlerTask worker thread list.");
				code = TW_ERROR_ALLOCATING_MEMORY;
			}

			if (code) { return code; }
		}

		if (!twWApiTaskerThreadList) {
			/* Create and start threads for the Api tasker function */
			twWApiTaskerThreadList = twList_Create(twThread_Delete);
			if (twWApiTaskerThreadList) {
				for (iter = 0; iter < apiTaskerThreadCount; iter++) {
					tmpThread = twThread_Create(twApi_TaskerFunction, 5, NULL, TRUE);
					if (!tmpThread) {
						TW_LOG(TW_ERROR, "twW_InitializeThreadPool: Could not create ApiTaskerFunction worker thread number: %d.", iter);
						twList_Delete(twWApiTaskerThreadList);
						twWApiTaskerThreadList = 0;
						twList_Delete(twWMsgHandlerThreadList);
						twWMsgHandlerThreadList = 0;
						code = TW_ERROR_ALLOCATING_MEMORY;
						break;
					}

					twList_Add(twWApiTaskerThreadList, tmpThread);
				}
			} else {
				TW_LOG(TW_ERROR, "twW_InitializeThreadPool: Could not create ApiTaskerFunction worker thread list.");
				
				//delete the other msgHandler thread list too
				twList_Delete(twWMsgHandlerThreadList);
				twWMsgHandlerThreadList = 0;
				code = TW_ERROR_ALLOCATING_MEMORY;
			}

			if (code) { return code; }
		}

		return code;
	}

	void twW_DestroyThreadPool()
	{
		if (twWMsgHandlerThreadList) {
			twList_Delete(twWMsgHandlerThreadList);
			twWMsgHandlerThreadList = 0;
		}

		if (twWApiTaskerThreadList) {
			twList_Delete(twWApiTaskerThreadList);
			twWApiTaskerThreadList = 0;
		}
	}

	void twW_SetStaticApiProperties(
		uint32_t maxMessageSize, uint32_t messageTimeout,
		uint32_t pongTimeout, uint32_t staleMsgCleanupRate,
		uint16_t streamBlockSize,
		uint32_t offlineMsgQueueSize, char *offlineMsgStoreDir,
		uint32_t maxConnectDelay, uint32_t connectRetryInterval, uint32_t maxMessages,
        uint32_t socketReadTimeout, char *fileTransferStagingDir)
	{
		// other config parameters
		twcfg.max_message_size = maxMessageSize;
		twcfg.default_message_timeout = messageTimeout;
		twcfg.pong_timeout = pongTimeout;
		twcfg.stale_msg_cleanup_rate = staleMsgCleanupRate;
		twcfg.stream_block_size = streamBlockSize;
		twcfg.offline_msg_queue_size = offlineMsgQueueSize;
        twcfg.offline_msg_store_dir = duplicateString(offlineMsgStoreDir);
		twcfg.max_connect_delay = maxConnectDelay;
		twcfg.connect_retry_interval = connectRetryInterval;
		twcfg.max_messages = maxMessages;
		twcfg.socket_read_timeout = socketReadTimeout;
        twcfg.file_xfer_staging_dir = duplicateString(fileTransferStagingDir);

		return;
	}

	int twW_SetApiProperties(char allowSelfSignedCert, char disableCertValidation, 
								char *x509FieldSubject_cn, char *x509FieldSubject_o, char *x509FieldSubject_ou,
								char *x509FieldIssuer_cn, char *x509FieldIssuer_o, char *x509FieldIssuer_ou,
								const char *serverCertFilePath, int serverCertType, 
								char *clientCertFilePath,
								const char *clientKeyFilePath, char *clientKeyPassphrase, int clientKeyType,
								uint32_t idlePingRate, uint8_t dutyCycle, uint32_t dutyCyclePeriod,
								uint32_t msgHandlerThreadCount, uint32_t apiTaskerThreadCount)
	{
		int code = 0;

		if (allowSelfSignedCert == 1) { twApi_SetSelfSignedOk(); }
		
#if defined(ENABLE_FIPS_MODE)
			code = twApi_EnableFipsMode();
			if (code) { return code; }
#endif

		if (disableCertValidation == 1)  { twApi_DisableCertValidation(); }

		if (x509FieldSubject_cn || x509FieldSubject_o || x509FieldSubject_ou || 
			x509FieldIssuer_cn || x509FieldIssuer_o || x509FieldIssuer_ou) {
			code = twApi_SetX509Fields(x509FieldSubject_cn, x509FieldSubject_o, x509FieldSubject_ou,
										x509FieldIssuer_cn, x509FieldIssuer_o, x509FieldIssuer_ou);
			if (code) { return code; }
		}

		if (serverCertFilePath) {
			code = twApi_LoadCACert(serverCertFilePath, serverCertType);
			if (code) { return code; }
		}

		if (clientCertFilePath) {
			code = twApi_LoadClientCert(clientCertFilePath);
			if (code) { return code; }
		}

		if (clientKeyFilePath) {
			code = twApi_SetClientKey(clientKeyFilePath, clientKeyPassphrase, clientKeyType);
			if (code) { return code; }
		}

		code = twApi_SetPingRate(idlePingRate);
		if (code) { return code; }
		
		code = twApi_SetDutyCycle(dutyCycle, dutyCyclePeriod);
		if (code) { return code; }

		// create the threadpool to manage messages
		code = twW_InitializeThreadPool(msgHandlerThreadCount, apiTaskerThreadCount);
		
		return code;
	}

	int twW_SetOnBindCallbackFunction(char* entityName,bindEvent_cb onBind) {
		return twApi_RegisterBindEventCallback(entityName, onBind, NULL);
	}

	int twW_SetApiCallbackFunctions(eventcb onConnect, eventcb onClose,
									authEvent_cb onAuthenticated,
									processPropertyReadRequest_cb onPropertyRead, processPropertyWriteRequest_cb onPropertyWrite,
									processServiceRequest_cb onServiceRequest)
	{
		int code = 0;
		if (onConnect) {
			code = twApi_RegisterConnectCallback(onConnect);
			if (code) { return code; }
		}
		
		if (onClose) {
			code = twApi_RegisterCloseCallback(onClose);
			if (code) { return code; }
		}

		if (onAuthenticated) {
			code = twApi_RegisterOnAuthenticatedCallback(onAuthenticated, NULL);
			if (code) { return code; }
		}

		if (onPropertyRead) {
			twWCallbacks.on_prop_read_request = onPropertyRead;
		}

		if (onPropertyWrite) {
			twWCallbacks.on_prop_write_request = onPropertyWrite;
		}

		if (onServiceRequest) {
			twWCallbacks.on_service_request = onServiceRequest;
		}

		return code;
	}

	int twW_SetLoggerProperties(int level, char isVerboseEnabled, log_function onLogMessage)
	{
		int code = twLogger_SetLevel(level);
		if (code) { return code; }

		if (isVerboseEnabled == 0){
			code = twLogger_SetIsVerbose(0);
		}else if (isVerboseEnabled == 1) {
			code = twLogger_SetIsVerbose(1);
		}
		if (code) { return code; }

		if (onLogMessage) {
			code = twLogger_SetFunction(onLogMessage);
			if (code) { return code; }
		}

		return code;
	}

	void twWApi_Delete()
	{
		twW_DestroyThreadPool();

        if (twcfg.offline_msg_store_dir) {
            free((char *)twcfg.offline_msg_store_dir);
            twcfg.offline_msg_store_dir = NULL;
        }

        if (twcfg.file_xfer_staging_dir) {
            free((char *)twcfg.file_xfer_staging_dir);
            twcfg.file_xfer_staging_dir = NULL;
        }

		twApi_Delete();
	}

	int twWApi_BindThingsWithoutDefaultServices(char **entityNames, int size) 
	{
		twList *list = NULL;
		int i = 0;
		list = twList_Create(NULL);
		for (i = 0; i < size; i++) {
			twList_Add(list, entityNames[i]);
		}
		return twApi_BindThings_Metadata_Option(list, TRUE);
	}

	//**********  P R O P E R T Y   H A N D L I N G  *********//

	int twWApi_RegisterProperties(enum entityTypeEnum entityType, char *entityName, twWRegInfoList *regList, void *userdata)
	{
		int code = 0;
		int iter = 0;
		twWPropertyRegInfo *propInfo = NULL;
		twWPropertyRegInfo *firstPropInfo = regList->listPtr;
		if (!entityName || !regList) { return TW_INVALID_PARAM; }
		if (!regList->length) { return code; }

		for (iter = 0; iter < regList->length; iter++) {
			propInfo = &firstPropInfo[iter];
			code = twApi_RegisterProperty(entityType, entityName, propInfo->propertyName, propInfo->propertyType, propInfo->propertyDescription,
				propInfo->propertyPushType, propInfo->propertyPushThreshold, twW_PropertyCallbackDispatcher, userdata);
			if (code) { 
				TW_LOG(TW_ERROR, "twWApi_RegisterProperties: Error while registering PropertyName: %s EntityName: %s aborting registration process.", propInfo->propertyName, entityName);
				break; 
			}
		}

		return code;
	}

	int twWApi_ReadProperty(enum entityTypeEnum entityType, char *entityName, char *propertyName, twWDataBlock **outBlock, int32_t timeout, char forceConnect)
	{
		twPrimitive *result = NULL;
		twWDataBlock *block = NULL;
		int code = 0;
		if (!outBlock) { return TW_INVALID_PARAM; }

		code = twApi_ReadProperty(entityType, entityName, propertyName, &result, timeout, forceConnect);
		if (code) { return code; }

		if (result) {
			code = twWDataBlock_FromPrimitive(result, &block);
			twPrimitive_Delete(result);
			if (code) {
				TW_LOG(TW_ERROR, "twWApi_ReadProperty: Error serializing the returned Primitive object from the Platform ReadProperty invocation. EntityName: %s PropertyName: %s", entityName, propertyName);
				return code;
			}
		}

		*outBlock = block;
		return code;
	}

	int twWApi_WriteProperty(enum entityTypeEnum entityType, char *entityName, char *propertyName, twWDataBlock *paramsBlock, int32_t timeout, char forceConnect)
	{
		twInfoTable *it = NULL;
		twPrimitive *params = NULL;
		char *temp = NULL;
		int code = 0;

		if (paramsBlock && paramsBlock->bytesPtr) {
			it = twWDataBlock_ToInfoTable(paramsBlock);
			if (it) {
				params = twPrimitive_CreateFromInfoTable(it);
				twInfoTable_Delete(it);
			} else {
				TW_LOG(TW_ERROR, "twWApi_WriteProperty: Error deserializing InfoTable object to use in Platform WriteProperty invocation. EntityName: %s PropertyName: %s", entityName, propertyName);
				return TW_PRECONDITION_FAILED;
			}
		} else {
			TW_LOG(TW_WARN, "twWApi_WriteProperty: A WriteProperty request is being made with a NULL property value. EntityName: %s PropertyName: %s", entityName, propertyName);
		}

		code = twApi_WriteProperty(entityType, entityName, propertyName, params, timeout, forceConnect);
		if (params) { twPrimitive_Delete(params); }

		return code;
	}

	enum msgCodeEnum twW_PropertyCallbackDispatcher(const char *entityName, const char *propertyName, twInfoTable **value, char isWrite, void *userdata)
	{
		twWDataBlock *block = NULL;
		twInfoTable *it = NULL;
		enum msgCodeEnum ret = TWX_INTERNAL_SERVER_ERROR;

		// NOTE: it is valid for propertyName to be NULL
		if (!value || !entityName) { return TWX_BAD_REQUEST; }

		if (isWrite){ // ** writes **
			if (!propertyName || !*value) {
				// a write request without a property name or property value?
				// server writing to all properties is not supported?
				ret = TWX_BAD_REQUEST;
			}
			else { // write request for a property
				int code = twWDataBlock_FromInfoTable(*value, &block);
				if (!code && block) {
					if (twWCallbacks.on_prop_write_request) {
						ret = twWCallbacks.on_prop_write_request(entityName, propertyName, block, userdata);
					}else {
						TW_LOG(TW_ERROR, "twW_PropertyCallbackDispatcher-WritePropertyRequest: Error primary on_prop_write_request delegate is NULL (not registered with twApi).");
					}
				} else {
					TW_LOG(TW_ERROR, "twW_PropertyCallbackDispatcher-WritePropertyRequest: Error serializing InfoTable object from Platform WriteProperty request. EntityName: %s PropertyName: %s", entityName, propertyName);
				}

				if (block) {
					// free the dataBlock we created
					twWDataBlock_Free(block);
				}
			}
		}
		else { // ** reads **
			if (twWCallbacks.on_prop_read_request) {
				// read request for a property. a NULL property name represents a read request for all properties
				ret = twWCallbacks.on_prop_read_request(entityName, propertyName, &block, userdata);
				if (block && block->bytesPtr) {
					it = twWDataBlock_ToInfoTable(block);
					if (it) {
						*value = it;
					} else {
						TW_LOG(TW_ERROR, "twW_PropertyCallbackDispatcher-ReadPropertyRequest: Error deserializing InfoTable object to use in Platform ReadProperty request. EntityName: %s PropertyName: %s", entityName, propertyName);
						ret = TWX_INTERNAL_SERVER_ERROR;
					}

					// free the dataBlock it came from a delegate
					twWDataBlock_Free(block);
				} else {
					TW_LOG(TW_WARN, "twW_PropertyCallbackDispatcher-ReadPropertyRequest: on_prop_read_request delegate returned a NULL value for a ReadProperty request, assuming this is intentional. EntityName: %s PropertyName: %s", entityName, propertyName);
					*value = NULL;
					ret = TWX_SUCCESS;
				}
			} else {
				TW_LOG(TW_ERROR, "twW_PropertyCallbackDispatcher-ReadPropertyRequest: Error primary on_prop_read_request delegate is NULL (not registered with twApi).");
			}
		}

		return ret;
	}


	//**********  S E R V I C E  /  E V E N T   H A N D L I N G  *********//

	int twWApi_RegisterServices(enum entityTypeEnum entityType, char *entityName, twWRegInfoList *regList, void *userdata, char autoRegisterStandardServices)
	{
		int code = 0;
		int iter = 0;
		twDataShape *inputDs;
		twDataShape *outputDs;
		twWServiceRegInfo *svcInfo = NULL;
		twWServiceRegInfo *firstSvcInfo = regList->listPtr;
		if (!entityName || !regList) { return TW_INVALID_PARAM; }

		for (iter = 0; iter < regList->length; iter++) {
			inputDs = NULL;
			outputDs = NULL;
			svcInfo = &firstSvcInfo[iter];

			if (svcInfo->inputDataShapeBytesPtr) { 
				inputDs =  twW_BytesToDataShape(svcInfo->inputDataShapeBytesPtr, svcInfo->inputDataShapeLength); 
			}

			if (svcInfo->outputDataShapeBytesPtr) {
				outputDs =  twW_BytesToDataShape(svcInfo->outputDataShapeBytesPtr, svcInfo->outputDataShapeLength);
			}

			code = twApi_RegisterService(entityType, entityName, svcInfo->serviceName, svcInfo->serviceDescription, inputDs, 
											svcInfo->outputType, outputDs, twW_ServiceCallbackDispatcher, userdata);
			if (code) { 
				TW_LOG(TW_ERROR, "twWApi_RegisterServices: Error while registering ServiceName: %s EntityName: %s aborting registration process.", svcInfo->serviceName, entityName);
				break; 
			}
		}

		if (autoRegisterStandardServices == 1) {
			code = twApi_RegisterServiceCallback(entityType, entityName, NOTIFYPROPERTYUPDATE_SERVICE, 
													twW_ServiceCallbackDispatcher, userdata);
			if (code) { TW_LOG(TW_ERROR, "twWApi_RegisterServices: Error while registering ServiceName: %s EntityName: %s", NOTIFYPROPERTYUPDATE_SERVICE, entityName); }

			// Register our own GetMetadata service with the twSDK
			code = twApi_RegisterServiceCallback(entityType, entityName, GETMETADATA_SERVICE,
													twW_ServiceCallbackDispatcher, userdata);
			if (code) { TW_LOG(TW_ERROR, "twWApi_RegisterServices: Error while registering ServiceName: %s EntityName: %s", GETMETADATA_SERVICE, entityName); }
		}

		return code;
	}

	int twWApi_InvokeService(enum entityTypeEnum entityType, char *entityName, char *serviceName, 
								twWDataBlock *paramsBlock, twWDataBlock **outBlock, int32_t timeout, char forceConnect)
	{
		twInfoTable *result = NULL;
		twInfoTable *params = NULL;
		twWDataBlock *block = NULL;
		int code = 0;
		if (!outBlock) { return TW_INVALID_PARAM; }
		if (paramsBlock && paramsBlock->bytesPtr) { 
			params = twWDataBlock_ToInfoTable(paramsBlock); 
			if (!params) {
				TW_LOG(TW_ERROR, "twWApi_InvokeService: Error deserializing InfoTable object to use in service invocation. EntityName: %s ServiceName: %s", entityName, serviceName);
				return TW_PRECONDITION_FAILED;
			}
		}
		
		code = twApi_InvokeService(entityType, entityName, serviceName, params, &result, timeout, forceConnect);
		if (params) { twInfoTable_Delete(params); }

		if (code) {
			TW_LOG(TW_ERROR, "twWApi_InvokeService: Error invoking service on Platform. EntityName: %s ServiceName: %s", entityName, serviceName);
			return code; 
		}

		if (result) {
			code = twWDataBlock_FromInfoTable(result, &block);
			twInfoTable_Delete(result);
			if (code) {
				TW_LOG(TW_ERROR, "twWApi_InvokeService: Error serializing result InfoTable from service invocation on Platform. EntityName: %s ServiceName: %s", entityName, serviceName);
				return code;
			}
		}

		*outBlock = block;
		return code;
	}

	int twWApi_FireEvent(enum entityTypeEnum entityType, char *entityName, char *eventName,
							twWDataBlock *paramsBlock, int32_t timeout, char forceConnect)
	{
		twInfoTable *params = NULL;
		int code = 0;
		if (paramsBlock && paramsBlock->bytesPtr) {
			params = twWDataBlock_ToInfoTable(paramsBlock); 
			if (!params) {
				TW_LOG(TW_ERROR, "twWApi_FireEvent: Error deserializing InfoTable object to use in FireEvent invocation. EntityName: %s EventName: %s", entityName, eventName);
				return TW_PRECONDITION_FAILED;
			}
		}
		
		code = twApi_FireEvent(entityType, entityName, eventName, params, timeout, forceConnect);
		if (params) { twInfoTable_Delete(params); }

		if (code) {
			TW_LOG(TW_ERROR, "twWApi_FireEvent: Error invoking FireEvent on Platform. EntityName: %s EventName: %s", entityName, eventName);
		}

		return code;
	}

	enum msgCodeEnum twW_ServiceCallbackDispatcher(const char *entityName, const char *serviceName, twInfoTable *params, twInfoTable **content, void *userdata)
	{
		twWDataBlock *paramsBlock = NULL;
		twWDataBlock *contentBlock = NULL;
		twInfoTable *it = NULL;
		int code = 0;
		enum msgCodeEnum ret = TWX_INTERNAL_SERVER_ERROR;
		if (!entityName || !serviceName){ return TWX_BAD_REQUEST; }

		if (twWCallbacks.on_service_request) {
			if (params) { // it's legal for params to be NULL
				code = twWDataBlock_FromInfoTable(params, &paramsBlock);
			}

			if (!code) {
				ret = twWCallbacks.on_service_request(entityName, serviceName, paramsBlock, &contentBlock, userdata);
				if (contentBlock && contentBlock->bytesPtr) {
					it = twWDataBlock_ToInfoTable(contentBlock);
					if (!it) {
						TW_LOG(TW_ERROR, "twW_ServiceCallbackDispatcher: Error deserializing InfoTable object to return in Platform Service request. EntityName: %s ServiceName: %s", entityName, serviceName);
						ret = TWX_INTERNAL_SERVER_ERROR;
					}

					// free the dataBlock it came from a delegate
					twWDataBlock_Free(contentBlock);
				}
			} else {
				TW_LOG(TW_ERROR, "twW_ServiceCallbackDispatcher: Error serializing InfoTable object from Platform Service request. EntityName: %s ServiceName: %s", entityName, serviceName);
			}

			if (paramsBlock) {
				// free the dataBlock we created
				twWDataBlock_Free(paramsBlock);
			}
		} else {
			TW_LOG(TW_ERROR, "twW_ServiceCallbackDispatcher: Error primary on_service_request delegate is NULL (not registered with twApi).");
		}

		*content = it;
		return ret;
	}


	//**********  T U N N E L   M A N A G E R  *********//

	void twWTunnelManager_Delete()
	{
		twTunnelManager_Delete();

		if (twWTunnelThread) {
			twThread_Delete(twWTunnelThread);
			twWTunnelThread = 0;
		}

		if (twWCallbacks.on_tunnel_state_change) {
			twTunnelManager_UnregisterTunnelCallback(twWCallbacks.on_tunnel_state_change, "*", NULL);
			twWCallbacks.on_tunnel_state_change = NULL;
		}
	}

	int twWTunnelManager_Create(char *host, int16_t port, char *appkey,
								char allowSelfSignedCert, char disableCertValidation,
								char *x509FieldSubject_cn, char *x509FieldSubject_o, char *x509FieldSubject_ou,
								char *x509FieldIssuer_cn, char *x509FieldIssuer_o, char *x509FieldIssuer_ou,
								const char *serverCertFilePath, int serverCertType,
								char *clientCertFilePath,
								const char *clientKeyFilePath, char *clientKeyPassphrase, int clientKeyType,
								tunnel_cb cb)
	{
		int code = twTunnelManager_Create();
		if (code) { return code; }

		code = twTunnelManager_UpdateTunnelServerInfo(host, port, appkey);
		if (code) { return code; }

		twTunnelManager_SetSelfSignedOk(allowSelfSignedCert);

		twTunnelManager_DisableCertValidation(disableCertValidation);
		if (x509FieldSubject_cn || x509FieldSubject_o || x509FieldSubject_ou ||
			x509FieldIssuer_cn || x509FieldIssuer_o || x509FieldIssuer_ou) {
			twTunnelManager_SetX509Fields(x509FieldSubject_cn, x509FieldSubject_o, x509FieldSubject_ou,
											x509FieldIssuer_cn, x509FieldIssuer_o, x509FieldIssuer_ou);
		}

		if (serverCertFilePath) { twTunnelManager_LoadCACert(serverCertFilePath, serverCertType); }
		if (clientCertFilePath) { twTunnelManager_LoadClientCert(clientCertFilePath); }
		if (clientKeyFilePath) { twTunnelManager_SetClientKey(clientKeyFilePath, clientKeyPassphrase, clientKeyType); }

		if (!twWTunnelThread) {
			/* Create and start a thread for the tunnel manager function */
			twWTunnelThread = twThread_Create(twTunnelManager_TaskerFunction, 5, NULL, TRUE);
			if (!twWTunnelThread) {
				twTunnelManager_Delete();
				TW_LOG(TW_ERROR, "twWTunnelManager_Create: Could not create TunnelManager worker thread.");
				return TW_UNKNOWN_ERROR;
			}
		}

		if (cb) {
			twWCallbacks.on_tunnel_state_change = cb;
			code = twTunnelManager_RegisterTunnelCallback(twWCallbacks.on_tunnel_state_change, "*", NULL);
			if (code) {
				twWTunnelManager_Delete();
				TW_LOG(TW_ERROR, "twWTunnelManager_Create: Could not register primary on_tunnel_state_change Tunnel state change callback.");
			}
		}

		return code;
	}


	//**********  F I L E   T R  A N S F E R  *********//

	void twWFileManager_Delete()
	{
		twFileManager_Delete();

		if (twWCallbacks.on_file_transfer) {
			twFileManager_UnregisterFileCallback(twW_FileTransferCallbackDispatcher, NULL, NULL);
			twWCallbacks.on_file_transfer = NULL;
		}
	}

	int twWFileManager_Create(uint32_t blockSize, uint64_t maxFileSize, uint16_t md5BlockSize, uint32_t timeout, 
								char *stagingDir, fileTransfer_cb cb)
	{
		int code = 0;
		twcfg.file_xfer_block_size = blockSize;
		twcfg.file_xfer_max_file_size = maxFileSize;
		twcfg.file_xfer_md5_block_size = md5BlockSize;
		twcfg.file_xfer_timeout = timeout;

        if (twcfg.file_xfer_staging_dir) {
            free((char *)twcfg.file_xfer_staging_dir);
            twcfg.file_xfer_staging_dir = NULL;
        }
        twcfg.file_xfer_staging_dir = duplicateString(stagingDir);

		code = twFileManager_Create();
		if (code) { return code; }

		if (cb) {
			twWCallbacks.on_file_transfer = cb;
			code = twFileManager_RegisterFileCallback(twW_FileTransferCallbackDispatcher, NULL, 0, NULL);
			if (code) {
				twWFileManager_Delete();
				TW_LOG(TW_ERROR, "twWFileManager_Create: Could not register primary on_file_transfer FileTransfer state change callback.");
			}
		}
		
		return code;
	}

	int twWFileManager_SendFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
		const char * targetRepo, const char * targetPath, const char * targetFile,
		uint32_t timeout, char asynch, char ** tid)
	{
		char *id = NULL;
		size_t len = 0;
		int code = twFileManager_SendFile(sourceRepo, sourcePath, sourceFile, targetRepo, targetPath, targetFile, 
											timeout, asynch, &id);
		if (code) { return code; }

		if (id) {
			if (tid) {
				len = strlen(id) + 1;
#ifdef _WIN32
				*tid = CoTaskMemAlloc(len); // this gets freed by the Marshaler
#else
				*tid = TW_MALLOC(len);
#endif
				if (*tid) {
					memcpy(*tid, id, len); 
				} else {
					TW_LOG(TW_ERROR, "twWFileManager_SendFile: Could not allocate memory to store transfer ID.");
				}
			}
			TW_FREE(id);
		} else {
			TW_LOG(TW_INFO, "twWFileManager_SendFile: The call to twFileManager_SendFile returned success, however, it did not return a transfer ID.");
		}

		return code;
	}

	int twWFileManager_GetFile(const char * sourceRepo, const char * sourcePath, const char * sourceFile,
		const char * targetRepo, const char * targetPath, const char * targetFile,
		uint32_t timeout, char asynch, char ** tid)
	{
		char *id = NULL;
		size_t len = 0;
		int code = twFileManager_GetFile(sourceRepo, sourcePath, sourceFile, targetRepo, targetPath, targetFile,
											timeout, asynch, &id);
		if (code) { return code; }

		if (id) {
			if (tid) {
				len = strlen(id) + 1;
#ifdef _WIN32
				*tid = CoTaskMemAlloc(len); // this gets freed by the Marshaler
#else
				*tid = TW_MALLOC(len);
#endif
				if (*tid) {
					memcpy(*tid, id, len);
				} else {
					TW_LOG(TW_ERROR, "twWFileManager_GetFile: Could not allocate memory to store transfer ID.");
				}
			}
			TW_FREE(id);
		} else {
			TW_LOG(TW_INFO, "twWFileManager_GetFile: The call to twFileManager_GetFile returned success, however, it did not return a transfer ID.");
		}

		return code;
	}


	void twW_FileTransferCallbackDispatcher(char fileRcvd, twFileTransferInfo *info, void *userdata)
	{
		if (twWCallbacks.on_file_transfer) {
			if (info) {
				twWCallbacks.on_file_transfer(fileRcvd, info->sourceRepository, info->sourcePath, info->sourceFile, info->sourceChecksum,
												info->targetRepository, info->targetPath, info->targetFile, info->targetChecksum,
												info->startTime, info->endTime, info->duration, info->state, info->isComplete,
												info->size, info->transferId, info->user, info->message, userdata);
			} else {
				TW_LOG(TW_ERROR, "twW_FileTransferCallbackDispatcher: Received a FileTransfer state changed callback, however the twFileTransferInfo associated with the transfer is NULL.");
			}
		} else {
			TW_LOG(TW_ERROR, "twW_FileTransferCallbackDispatcher: Error primary on_file_transfer delegate is NULL (not registered with twApi). Received a FileTransfer state changed callback, however a delegate handler is not defined for propagating the event.");
		}
	}

	
	//**********  D A T A B L O C K  *********//
	
	twPrimitive * twWDataBlock_ToPrimitive(twWDataBlock *block)
	{
		twPrimitive *p = NULL;
		twStream *s = twStream_CreateFromCharArrayZeroCopy(block->bytesPtr, block->length); // Does NOT copy bytesPtr to stream.
		if (!s) { 
			TW_LOG(TW_ERROR, "twWDataBlock_ToPrimitive: Error creating stream, twStream_CreateFromCharArrayZeroCopy returned NULL.");
			return NULL; 
		}

		p = twPrimitive_CreateFromStream(s); // copies stream bytes to primitive
		twStream_Delete(s);
		return p;
	}

	twInfoTable * twWDataBlock_ToInfoTable(twWDataBlock *block)
	{
		twInfoTable *it = NULL;
		twStream *s = twStream_CreateFromCharArrayZeroCopy(block->bytesPtr, block->length); // Does NOT copy bytesPtr to stream.
		if (!s) { 
			TW_LOG(TW_ERROR, "twWDataBlock_ToInfoTable: Error creating stream, twStream_CreateFromCharArrayZeroCopy returned NULL.");
			return NULL; 
		}

		it = twInfoTable_CreateFromStream(s); // copies stream bytes to internal primitives
		twStream_Delete(s);
		return it;
	}

	int  twWDataBlock_FromPrimitive(twPrimitive *p, twWDataBlock **outBlock)
	{
		return twWDataBlock_FromStruct(p, 1, outBlock);
	}

	int twWDataBlock_FromInfoTable(twInfoTable *it, twWDataBlock **outBlock)
	{
		return twWDataBlock_FromStruct(it, 0, outBlock);
	}

	int twWDataBlock_FromStruct(void *structPtr, char isPrimitive, twWDataBlock **outBlock)
	{
		int code = 0;
		twWDataBlock *out = NULL;
		twStream *s = NULL;
		if (!structPtr || !outBlock) { return TW_INVALID_PARAM; }

		s = twStream_Create();
		if (!s) { 
			TW_LOG(TW_ERROR, "twWDataBlock_FromStruct: Error creating stream, twStream_Create returned NULL.");
			return TW_ERROR_ALLOCATING_MEMORY; 
		}

		if (isPrimitive) {	
			code = twPrimitive_ToStream(structPtr, s); // copies primitive bytes to stream
		} else {
			code = twInfoTable_ToStream(structPtr, s); // copies infotable bytes to stream
		}
		
		if (code) {
			twStream_Delete(s);
			return code;
		}

		twStream_Reset(s);

#ifdef _WIN32
		out = CoTaskMemAlloc(sizeof(twWDataBlock)); // this gets freed by the Marshaler
#else
		out = TW_MALLOC(sizeof(twWDataBlock));
#endif

		if (!out) {
			TW_LOG(TW_ERROR, "twWDataBlock_FromStruct: Error allocating memory, CoTaskMemAlloc returned NULL.");
			twStream_Delete(s);
			return TW_ERROR_ALLOCATING_MEMORY;
		}

		out->isPrimitive = isPrimitive;
		out->length = twStream_GetLength(s);
#ifdef _WIN32
		out->bytesPtr = CoTaskMemAlloc(out->length);
#else
		out->bytesPtr = TW_MALLOC(out->length);
#endif

		if (!out->bytesPtr) {
			twStream_Delete(s);
#ifdef _WIN32
			CoTaskMemFree(out);
#else
            TW_FREE(out);
#endif
			TW_LOG(TW_ERROR, "twWDataBlock_FromStruct: Error allocating memory, CoTaskMemAlloc returned NULL.");
			return TW_ERROR_ALLOCATING_MEMORY;
		}

		memcpy(out->bytesPtr, twStream_GetData(s), out->length); // copies stream bytes to block
		twStream_Delete(s);
		*outBlock = out;
		return code;
	}

	void twWDataBlock_Free(twWDataBlock *block)
	{
		if (block) {
			if (block->bytesPtr) {
#ifdef _WIN32
                CoTaskMemFree(block->bytesPtr);
#else
                TW_FREE(block->bytesPtr);
#endif
			}
#ifdef _WIN32
            CoTaskMemFree(block);
#else
            TW_FREE(block);
#endif
			block = 0;
		}
	}

	twDataShape * twW_BytesToDataShape(unsigned char *bytesPtr, int32_t length)
	{
		twDataShape *ds = NULL;
		twStream *s = twStream_CreateFromCharArrayZeroCopy(bytesPtr, length); // Does NOT copy bytesPtr to stream.
		if (!s) { 
			TW_LOG(TW_ERROR, "twW_BytesToDataShape: Error creating stream, twStream_CreateFromCharArrayZeroCopy returned NULL.");
			return NULL; 
		}

		ds = twDataShape_CreateFromStream(s); // copies stream bytes to primitive
		twStream_Delete(s);
		return ds;
	}

	//**********  U T I L S  *********//

	void twW_DescriptionForErrorCode(int code, char **description)
	{
		char *msg = NULL;
		if (!description) { return; }

		switch (code) 
		{
		case TW_INVALID_PARAM:
			msg = "The parameter is invalid.";
			break;
		case TW_ERROR_ALLOCATING_MEMORY:
			msg = "Could not allocate the required memory.";
			break;
		case TW_UNKNOWN_WEBSOCKET_ERROR:
			msg = "Unknown WebSocket error.";
			break;
		case TW_ERROR_INITIALIZING_WEBSOCKET:
			msg = "Error initializing WebSocket.";
			break;
		case TW_TIMEOUT_INITIALIZING_WEBSOCKET:
			msg = "WebSocket timed out during initialization.";
			break;
		case TW_WEBSOCKET_NOT_CONNECTED:
			msg = "WebSocket is not connected.";
			break;
		case TW_ERROR_PARSING_WEBSOCKET_DATA:
			msg = "Error parsing data from WebSocket.";
			break;
		case TW_ERROR_READING_FROM_WEBSOCKET:
			msg = "Error reading from WebSocket.";
			break;
		case TW_WEBSOCKET_FRAME_TOO_LARGE:
			msg = "The WebSocket frame is too large";
			break;
		case TW_INVALID_WEBSOCKET_FRAME_TYPE:
			msg = "The WebSocket frame Type is invalid.";
			break;
		case TW_WEBSOCKET_MSG_TOO_LARGE:
			msg = "The WebSocket message is too large.";
			break;
		case TW_ERROR_WRITING_TO_WEBSOCKET:
			msg = "Error writing to WebSocket.";
			break;
		case TW_INVALID_ACCEPT_KEY:
			msg = "The Accept_Key is invalid.";
			break;
		case TW_NULL_OR_INVALID_MSG_HANDLER:
			msg = "The message handler is null or invalid.";
			break;
		case TW_INVALID_CALLBACK_STRUCT:
			msg = "The callback is invalid.";
			break;
		case TW_INVALID_MSG_CODE:
			msg = "The message Code is invalid.";
			break;
		case TW_INVALID_MSG_TYPE:
			msg = "The message Type is invalid.";
			break;
		case TW_ERROR_SENDING_MSG:
			msg = "Error sending message.";
			break;
		case TW_ERROR_WRITING_OFFLINE_MSG_STORE:
			msg = "Error writing to offline message store.";
			break;
		case TW_ERROR_MESSAGE_TOO_LARGE:
			msg = "Message too large.";
			break;
		case TW_WROTE_TO_OFFLINE_MSG_STORE:
			msg = "Message saved to offline message store for future delivery up next connection.";
			break;
		case TW_ERROR_ADDING_DATASHAPE_ENTRY:
			msg = "Error adding DataShape entry.";
			break;
		case TW_INDEX_NOT_FOUND:
			msg = "The index was not found.";
			break;
		case TW_ERROR_GETTING_PRIMITIVE:
			msg = "Error retrieving Primitive.";
			break;
		case TW_INVALID_BASE_TYPE:
			msg = "The BaseType is invalid.";
			break;
		case TW_NULL_API_SINGLETON:
		case TW_NULL_OR_INVALID_API_SINGLETON:
			msg = "The API has not been initialized, or, the API singleton is invalid.";
			break;
		case TW_ERROR_SENDING_RESP:
			msg = "Error sending response.";
			break;
		case TW_INVALID_MSG_BODY:
			msg = "The message body is invalid.";
			break;
		case TW_INVALID_MSG_PARAMS:
			msg = "The message params are invalid.";
			break;
		case TW_INVALID_RESP_MSG:
			msg = "The response message is invalid.";
			break;
		case TW_ERROR_CREATING_MSG:
			msg = "Error creating message.";
			break;
		case TW_MAX_TASKS_EXCEEDED:
			msg = "The maximum number of available Tasks have been reached.";
			break;
		case TW_TASK_NOT_FOUND:
			msg = "The Task could not be found.";
			break;
		case TW_NULL_OR_INVALID_LOGGER_SINGLETON:
			msg = "The logger singleton is null, or invalid.";
			break;
		case TW_BASE64_ENCODE_OVERRUN:
			msg = "There was an overrun while encoding to Base64.";
			break;
		case TW_BASE64_DECODE_OVERRUN:
			msg = "There was an overrun while decoding from Base64.";
			break;
		case TW_ERROR_WRITING_TO_SOCKET:
			msg = "Error writing to socket.";
			break;
		case TW_SOCKET_INIT_ERROR:
			msg = "There was an error initializing the Socket.";
			break;
		case TW_INVALID_SSL_CERT:
			msg = "The SSL certificate is invalid.";
			break;
		case TW_SOCKET_NOT_FOUND:
			msg = "The Socket could not be found.";
			break;
		case TW_HOST_NOT_FOUND:
			msg = "Error creating socket. Either host/port information is incorrect, or the server is not reachable.";
			break;
		case TW_ERROR_CREATING_SSL_CTX:
			msg = "Error establishing Client/Server SSL context. Most likely the key/certificate pair could not be loaded.";
			break;
		case TW_ERROR_CREATING_MTX:
			msg = "Error allocating or creating mutex.";
			break;
		case TW_ERROR_INITIALIZING_API:
			msg = "Error initializing the API. See the log for more details.";
			break;
		case TW_BAD_REQUEST:
			msg = "Bad Request.";
			break;
		case TW_UNAUTHORIZED:
			msg = "Unauthorized.";
			break;
		case TW_ERROR_BAD_OPTION:
			msg = "Bad Option.";
			break;
		case TW_FORBIDDEN:
			msg = "Forbidden.";
			break;
		case TW_NOT_FOUND:
			msg = "Not Found.";
			break;
		case TW_METHOD_NOT_ALLOWED:
			msg = "Method Not Allowed.";
			break;
		case TW_NOT_ACCEPTABLE:
			msg = "Not Acceptable.";
			break;
		case TW_PRECONDITION_FAILED:
			msg = "Precondition Failed.";
			break;
		case TW_ENTITY_TOO_LARGE:
			msg = "Request Entity Too Large.";
			break;
		case TW_UNSUPPORTED_CONTENT_FORMAT:
			msg = "Unsupported Content Format.";
			break;
		case TW_INTERNAL_SERVER_ERROR:
			msg = "Internal Server Error.";
			break;
		case TW_NOT_IMPLEMENTED:
			msg = "Not Implemented.";
			break;
		case TW_BAD_GATEWAY:
			msg = "Bad Gateway.";
			break;
		case TW_SERVICE_UNAVAILABLE:
			msg = "Service Unavailable.";
			break;
		case TW_GATEWAY_TIMEOUT:
			msg = "Gateway Timeout.";
			break;
		default:
			msg = "Unknown error.";
			break;
		}

		*description = msg;
	}

/* Thingworx integration - be64toh not defined on windows*/
/* dummy function to force OBJ load for ntohll */
#ifdef _WIN32
#if BYTE_ORDER == LITTLE_ENDIAN 
void twForceLibLoad(uint64_t x){
	be64toh(x);
	return;
}
#endif 
#endif
#ifdef __cplusplus
}
#endif
