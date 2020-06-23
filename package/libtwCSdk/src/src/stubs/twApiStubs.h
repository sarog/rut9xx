#ifndef TW_API_STUBS_H
#define TW_API_STUBS_H
#include <cfuhash.h>

#ifdef TW_STUBS
#include "twApiStubsOn.h"
#else

#include "twApiStubsOff.h"
#endif
#include "twProperties.h"
#include "twServices.h"
#include "twTls.h"
#include "twApi.h"
#include "twFileManager.h"
#include "twSubscribedProps.h"

/*********************************************************/
/* Internal Functions */
typedef int (*cfuhash_destroyStub)(cfuhash_table_t *);
typedef void (*PropertyChangeListenerFunctionStub)(const char *, const char *,twPrimitive*);
typedef void (*deleteCallbackInfoStub)(void * info);
typedef bindListEntry * (*bindListEntry_CreateStub)(char * entityName);
typedef void (*bindListEntry_DeleteStub)(void * entry);
typedef enum msgCodeEnum (*notifyPropertyUpdateHandlerStub)(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata);
typedef void (*subscribedPropertyUpdateTaskStub)(DATETIME now, void * params);
typedef char (*isFileTransferServiceStub)(char * service);
typedef char (*isTunnelServiceStub)(char * service);
typedef int (*convertMsgCodeToErrorCodeStub)(enum msgCodeEnum code);
typedef ListEntry * (*findRegisteredItemStub)(twList * list, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName);
typedef void * (*getCallbackFromListStub)(twList * list, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName, void ** userdata);
typedef enum msgCodeEnum (*sendMessageBlockingStub)(twMessage ** msg, int32_t timeout, twInfoTable ** result);
typedef enum msgCodeEnum (*makeRequestStub)(enum msgCodeEnum method, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect);
typedef enum msgCodeEnum (*makePropertyRequestStub)(enum msgCodeEnum method, enum entityTypeEnum entityType, char * entityName, char * propertyName, twPrimitive * value, twPrimitive ** result, int32_t timeout, char forceConnect);
typedef int (*twApi_SendResponseStub)(twMessage * msg);
typedef int (*api_requesthandlerStub)(struct twWs * ws, struct twMessage * msg);
typedef enum msgCodeEnum (*getMetadataServiceStub)(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content, void * userdata);
typedef int (*pong_handlerStub)(struct twWs * ws, const char * data, size_t length);
typedef int (*makeAuthOrBindCallbacksStub)(char * entityName, enum entityTypeEnum entityType, char type, char * value);
typedef int (*makeSynchronizedStateCallbacksStub)(char * entityName, enum entityTypeEnum entityType, twInfoTable* subscriptionData);
typedef int (*registerServiceOrEventStub)(enum entityTypeEnum entityType, char * entityName, char * serviceName, char * serviceDescription, twDataShape * inputs, enum BaseType outputType, twDataShape * outputDataShape, service_cb cb, void * userdata, char isService);
typedef int (*AddAspectToEntityStub)(char * entityName, enum characteristicEnum type,  char * characteristicName, char * aspectName, twPrimitive * aspectValue);

/*********************************************************/
/* API Functions */
typedef int (*twApi_InitializeStub)(char * host, uint16_t port, char * resource, char * app_key, char * gatewayName, uint32_t messageChunkSize, uint16_t frameSize, char autoreconnect);
typedef int (*twApi_DeleteStub)(void);
typedef int (*twApi_SetProxyInfoStub)(char * proxyHost, uint16_t proxyPort, char * proxyUser, char * proxyPass);
typedef char * (*twApi_GetVersionStub)(void);
typedef int (*twApi_BindAllStub)(char unbind);
typedef int (*twApi_AuthenticateStub)();
typedef int (*twApi_ConnectStub)(uint32_t timeout, int32_t retries);
typedef int (*twApi_DisconnectStub)(char * reason);
typedef char (*twApi_isConnectedStub)();
typedef char (*twApi_ConnectionInProgressStub)();
typedef int (*twApi_StopConnectionAttemptStub)();
typedef int (*twApi_SetDutyCycleStub)(uint8_t duty_cycle, uint32_t period);
typedef int (*twApi_SetPingRateStub)(uint32_t rate);
typedef int (*twApi_SetConnectTimeoutStub)(uint32_t timeout);
typedef int (*twApi_SetConnectRetriesStub)(signed char retries);
typedef int (*twApi_SetGatewayNameStub)(const char* input_name);
typedef int (*twApi_SetGatewayTypeStub)(const char* input_type);
typedef int (*twApi_BindThingStub)(char * entityName);
typedef int (*twApi_BindThings_Metadata_OptionStub)(char * entityName);
typedef int (*twApi_UnbindThingStub)(char * entityName);
typedef char (*twApi_IsEntityBoundStub)(char * entityName);
typedef void (*twApi_TaskerFunctionStub)(DATETIME now, void * params);
typedef int (*twApi_RegisterPropertyStub)(enum entityTypeEnum entityType, char * entityName, char * propertyName, enum BaseType propertyType, char * propertyDescription, char * propertyPushType, double propertyPushThreshold, property_cb cb, void * userdata);
typedef int (*twApi_UpdatePropertyMetaDataStub)(enum entityTypeEnum entityType, char * entityName, char * propertyName, enum BaseType propertyType, char * propertyDescription, char * propertyPushType, double propertyPushThreshold);
typedef int (*twApi_AddAspectToPropertyStub)(char * entityName, char * propertyName, char * aspectName, twPrimitive * aspectValue);
typedef int (*twApi_RegisterServiceStub)(enum entityTypeEnum entityType, const char * entityName, char * serviceName, char * serviceDescription, twDataShape * inputs, enum BaseType outputType, twDataShape * outputDataShape, service_cb cb, void * userdata);
typedef int (*twApi_AddAspectToServiceStub)(char * entityName, char * serviceName, char * aspectName, twPrimitive * aspectValue);
typedef int (*twApi_RegisterEventStub)(enum entityTypeEnum entityType, char * entityName, char * eventName, char * eventDescription, twDataShape * parameters);
typedef int (*twApi_AddAspectToEventStub)(char * entityName, char * eventName, char * aspectName, twPrimitive * aspectValue);
typedef int (*twApi_RegisterPropertyCallbackStub)(enum entityTypeEnum entityType, char * entityName, char * propertyName, property_cb cb, void * userdata) ;
typedef int (*twApi_RegisterServiceCallbackStub)(enum entityTypeEnum entityType, char * entityName, char * serviceName, service_cb cb, void * userdata);
typedef int (*twApi_UnregisterThingStub)(char * entityName);
typedef int (*twApi_UnregisterCallbackStub)(char * entityName, enum characteristicEnum type, char * characteristicName, void * userdata);
typedef int (*twApi_UnregisterPropertyCallbackStub)(char * entityName, char * propertyName, void * cb);
typedef int (*twApi_UnregisterServiceCallbackStub)(char * entityName, char * serviceName, void * cb);
typedef int (*twApi_RegisterDefaultRequestHandlerStub)(genericRequest_cb cb);
typedef propertyList * (*twApi_CreatePropertyListStub)(char * name, twPrimitive * value, DATETIME timestamp);
typedef int (*twApi_DeletePropertyListStub)(propertyList * list);
typedef int (*twApi_AddPropertyToListStub)(propertyList * proplist, char * name, twPrimitive * value, DATETIME timestamp);
typedef int (*twApi_ReadPropertyStub)(enum entityTypeEnum entityType, char * entityName, char * propertyName, twPrimitive ** result, int32_t timeout, char forceConnect);
typedef int (*twApi_WritePropertyStub)(enum entityTypeEnum entityType, char * entityName, char * propertyName, twPrimitive * value, int32_t timeout, char forceConnect);
typedef int (*twApi_SetSubscribedPropertyVTQStub)(char * entityName, char * propertyName, twPrimitive * value,  DATETIME timestamp, char * quality, char fold, char pushUpdate);
typedef int (*twApi_SetSubscribedPropertyStub)(char * entityName, char * propertyName, twPrimitive * value, char fold, char pushUpdate);
typedef int (*twApi_PushSubscribedPropertiesStub)(char * entityName, char forceConnect);
typedef int (*twApi_PushPropertiesStub)(enum entityTypeEnum entityType, char * entityName, propertyList * properties, int32_t timeout, char forceConnect);
typedef int (*twApi_InvokeServiceStub)(enum entityTypeEnum entityType, char * entityName, char * serviceName, twInfoTable * params, twInfoTable ** result, int32_t timeout, char forceConnect);
typedef int (*twApi_FireEventStub)(enum entityTypeEnum entityType, char * entityName, char * eventName, twInfoTable * params, int32_t timeout, char forceConnect);
typedef int (*twApi_RegisterConnectCallbackStub)(eventcb cb) ;
typedef int (*twApi_RegisterCloseCallbackStub)(eventcb cb);
typedef int (*twApi_RegisterPingCallbackStub)(eventcb cb);
typedef int (*twApi_RegisterPongCallbackStub)(eventcb cb);
typedef int (*twApi_RegisterBindEventCallbackStub)(char * entityName, bindEvent_cb cb, void * userdata);
typedef int (*twApi_UnregisterBindEventCallbackStub)(char * entityName, bindEvent_cb cb, void * userdata);
typedef int (*twApi_RegisterOnAuthenticatedCallbackStub)(authEvent_cb cb, void * userdata);
typedef int (*twApi_UnregisterOnAuthenticatedCallbackStub)(authEvent_cb cb, void * userdata);
typedef int (*twApi_CleanupOldMessagesStub)(void);
typedef int (*twApi_SendPingStub)(char * content);
typedef int (*twApi_CreateTaskStub)(uint32_t runTintervalMsec, twTaskFunction func);
typedef void (*twApi_SetSelfSignedOkStub)();
typedef void (*twApi_DisableCertValidationStub)();
typedef int	(*twApi_LoadCACertStub)(const char *file, int type);
typedef int	(*twApi_LoadClientCertStub)(char *file);
typedef int	(*twApi_SetClientKeyStub)(const char *file, char * passphrase, int type);
typedef int (*twApi_IsFIPSCompatibleStub)();
typedef int (*twApi_EnableFipsModeStub)();
typedef int (*twApi_IsFipsModeEnabledStub)();
typedef void (*twApi_DisableEncryptionStub)();
typedef int (*twApi_SetX509FieldsStub)(char * subject_cn, char * subject_o, char * subject_ou, char * issuer_cn, char * issuer_o, char * issuer_ou);
typedef int	(*twApi_SetOfflineMsgStoreDirStub)(const char *dir);
typedef twConnectionInfo * (*twApi_GetConnectionInfoStub)(void);

/* Properties */
typedef twPropertyDef * (*twPropertyDef_CreateStub)(char * name, enum BaseType type, char * description, char * pushType, double pushThreshold);
typedef void (*twPropertyDef_DeleteStub)(void * input);
typedef twProperty * (*twProperty_CreateStub)(char * name, twPrimitive * value, DATETIME timestamp);
typedef twProperty * (*twPropertyVTQ_CreateStub)(char * name, twPrimitive * value, DATETIME timestamp, char * quality);
typedef twProperty * (*twProperty_CreateFromStreamStub)(twStream * s);
typedef void (*twProperty_DeleteStub)(void * input);

/* Services */
typedef twServiceDef * (*twServiceDef_CreateStub)(char * name, char * description, twDataShape * inputs, enum BaseType outputType, twDataShape * outputDataShape);
typedef void (*twServiceDef_DeleteStub)(void * input);

/* Base Types */
typedef twStream * (*twStream_CreateStub)();
typedef twStream * (*twStream_CreateFromCharArrayStub)(const char * data, uint32_t length);
typedef twStream * (*twStream_CreateFromCharArrayZeroCopyStub)(const char * data, uint32_t length);
typedef void (*twStream_DeleteStub)(void* s);
typedef char * (*twStream_GetDataStub)(struct twStream * s);
typedef int32_t (*twStream_GetIndexStub)(struct twStream * s);
typedef int32_t (*twStream_GetLengthStub)(struct twStream * s);
typedef int (*twStream_AddBytesStub)(struct twStream * s, void * b, uint32_t count);
typedef int (*twStream_GetBytesStub)(struct twStream * s, void * b, uint32_t count);
typedef int (*twStream_ResetStub)(struct twStream * s);
typedef twStream * (*twStream_CreateFromFileStub)(const char * fname);
typedef void (*swap4bytesStub)(char * bytes);
typedef void (*swap8bytesStub)(char * bytes);
typedef int (*stringToStreamStub)(char * string, twStream * s);
typedef char * (*streamToStringStub)(twStream * s);
typedef enum BaseType (*baseTypeFromStringStub)(const char * s);
typedef const char * (*baseTypeToStringStub)(enum BaseType b);
typedef twPrimitive * (*twPrimitive_CreateStub)();
typedef twPrimitive * (*twPrimitive_CreateFromStreamStub)( twStream * s);
typedef twPrimitive * (*twPrimitive_CreateFromStreamTypedStub)(twStream * s, enum BaseType type);
typedef twPrimitive * (*twPrimitive_ZeroCopyStub)(twPrimitive * p);
typedef twPrimitive * (*twPrimitive_FullCopyStub)(twPrimitive * p);
typedef void (*twPrimitive_DeleteStub)(void * p);
typedef int (*twPrimitive_ToStreamStub)(twPrimitive * p, twStream * s);
typedef char * (*twPrimitive_DecoupleStringAndDeleteStub)(twPrimitive * p);
typedef int (*twPrimitive_CompareStub)(twPrimitive * p1, twPrimitive * p2);
typedef char (*twPrimitive_IsTrueStub)(twPrimitive * p1) ;
typedef twPrimitive * (*twPrimitive_CreateFromLocationStub)(const twLocation * value);
typedef twPrimitive * (*twPrimitive_CreateFromNumberStub)(const double value);
typedef twPrimitive * (*twPrimitive_CreateFromIntegerStub)(const int32_t value);
typedef twPrimitive * (*twPrimitive_CreateFromDatetimeStub)(const DATETIME value);
typedef twPrimitive * (*twPrimitive_CreateFromCurrentTimeStub)();
typedef twPrimitive * (*twPrimitive_CreateFromBooleanStub)(const char value);
typedef twPrimitive * (*twPrimitive_CreateFromInfoTableStub)(struct twInfoTable * it);
typedef twPrimitive * (*twPrimitive_CreateVariantStub)(twPrimitive * input);
typedef twPrimitive * (*twPrimitive_CreateFromStringStub)(const char * value, char duplicate);
typedef twPrimitive * (*twPrimitive_CreateFromBlobStub)(const char * value, int32_t length, char isImage, char duplicate);
typedef twPrimitive * (*twPrimitive_CreateFromVariableStub)(const void * value, enum BaseType type, char duplicateCharArray, uint32_t blobLength);
typedef struct cJSON * (*twPrimitive_ToJsonStub)(char * name, twPrimitive * p, struct cJSON * parent);
typedef twPrimitive * (*twPrimitive_CreateFromJsonStub)(struct cJSON * j, char * name, enum BaseType type);

/* InfoTables */
typedef twDataShapeAspect * (*twDataShapeAspect_CreateStub)(const char * name, twPrimitive * value);
typedef twDataShapeAspect * (*twDataShapeAspect_CreateFromStreamStub)(twStream * s);
typedef void (*twDataShapeAspect_DeleteStub)(void * aspect);
typedef twDataShapeEntry * (*twDataShapeEntry_CreateStub)(const char * name, const char * description, enum BaseType type);
typedef twDataShapeEntry * (*twDataShapeEntry_CreateFromStreamStub)(struct twStream * s);
typedef void (*twDataShapeEntry_DeleteStub)(void * entry);
typedef int (*twDataShapeEntry_AddAspectStub)(struct twDataShapeEntry * entry, const char * name, twPrimitive * value);
typedef uint32_t (*twDataShapeEntry_GetLengthStub)(struct twDataShapeEntry * entry);
typedef int (*twDataShapeEntry_ToStreamStub)(struct twDataShapeEntry * entry, twStream * s);
typedef twDataShape * (*twDataShape_CreateStub)(twDataShapeEntry * firstEntry);
typedef twDataShape * (*twDataShape_CreateFromStreamStub)(struct twStream * s);
typedef void (*twDataShape_DeleteStub)(void * ds);
typedef uint32_t (*twDataShape_GetLengthStub)(struct twDataShape * ds);
typedef int (*twDataShape_ToStreamStub)(struct twDataShape * ds, twStream * s);
typedef int (*twDataShape_SetNameStub)(struct twDataShape * ds, char * name);
typedef int (*twDataShape_AddEntryStub)(struct twDataShape * ds, struct twDataShapeEntry * entry);
typedef int (*twDataShape_GetEntryIndexStub)(struct twDataShape * ds, const char * name, int * index);
typedef twInfoTableRow * (*twInfoTableRow_CreateStub)(twPrimitive * firstEntry);
typedef twInfoTableRow * (*twInfoTableRow_CreateFromStreamStub)(twStream * s);
typedef void (*twInfoTableRow_DeleteStub)(void * row);
typedef int (*twInfoTableRow_GetCountStub)(twInfoTableRow * row);
typedef uint32_t (*twInfoTableRow_GetLengthStub)(twInfoTableRow * row);
typedef int (*twInfoTableRow_AddEntryStub)(twInfoTableRow * row, twPrimitive * entry);
typedef twPrimitive * (*twInfoTableRow_GetEntryStub)(twInfoTableRow * row, int index);
typedef int (*twInfoTableRow_ToStreamStub)(twInfoTableRow * row, twStream * s);
typedef twInfoTable * (*twInfoTable_CreateStub)(twDataShape * shape);
typedef twInfoTable * (*twInfoTable_CreateFromStreamStub)(twStream * s);
typedef void (*twInfoTable_DeleteStub)(void * it);
typedef twInfoTable * (*twInfoTable_FullCopyStub)(twInfoTable * it);
typedef twInfoTable * (*twInfoTable_ZeroCopyStub)(twInfoTable * it);
typedef int (*twInfoTable_CompareStub)(twInfoTable * p1, twInfoTable * p2);
typedef int (*twInfoTable_AddRowStub)(twInfoTable * it, twInfoTableRow * row);
typedef twInfoTableRow * (*twInfoTable_GetEntryStub)(twInfoTable * it, int index);
typedef int (*twInfoTable_ToStreamStub)(twInfoTable * it, twStream * s);
typedef twInfoTable * (*twInfoTable_CreateFromPrimitiveStub)(const char * name, twPrimitive * value);
typedef twInfoTable * (*twInfoTable_CreateFromStringStub)(const char * name, char * value, char duplicate);
typedef twInfoTable * (*twInfoTable_CreateFromNumberStub)(const char * name, double value);
typedef twInfoTable * (*twInfoTable_CreateFromIntegerStub)(const char * name, int32_t value);
typedef twInfoTable * (*twInfoTable_CreateFromLocationStub)(const char * name, twLocation * value);
typedef twInfoTable * (*twInfoTable_CreateFromBlobStub)(const char * name, char * value, int32_t length, char isImage, char duplicate);
typedef twInfoTable * (*twInfoTable_CreateFromDatetimeStub)(const char * name, DATETIME value);
typedef twInfoTable * (*twInfoTable_CreateFromBooleanStub)(const char * name, char value);
typedef int (*twInfoTable_GetStringStub)(twInfoTable * it, const char * name, int32_t row, char ** value);
typedef int (*twInfoTable_GetNumberStub)(twInfoTable * it, const char * name, int32_t row, double * value);
typedef int (*twInfoTable_GetIntegerStub)(twInfoTable * it, const char * name, int32_t row, int32_t * value);
typedef int (*twInfoTable_GetLocationStub)(twInfoTable * it, const char * name, int32_t row, twLocation * value);
typedef int (*twInfoTable_GetBlobStub)(twInfoTable * it, const char * name, int32_t row, char ** value, int32_t * length);
typedef int (*twInfoTable_GetDatetimeStub)(twInfoTable * it, const char * name, int32_t row, DATETIME * value);
typedef int (*twInfoTable_GetBooleanStub)(twInfoTable * it, const char * name, int32_t row, char * value);
typedef int (*twInfoTable_GetPrimitiveStub)(twInfoTable * it, const char * name, int32_t row, twPrimitive ** value);
typedef twInfoTable * (*twInfoTable_CreateFromJsonStub)(struct cJSON * json, char * singleEntryName);
typedef struct cJSON * (*twDataShape_ToJsonStub)(twDataShape * ds, struct cJSON * parent);
typedef struct cJSON * (*twInfoTable_ToJsonStub)(twInfoTable * it);

/* Messages */
typedef twMessage * (*twMessage_CreateStub)(enum msgCodeEnum code, uint32_t reqId); /* Set Reqid to zero to autogenerate ID */
typedef twMessage * (*twMessage_CreateRequestMsgStub)(enum msgCodeEnum code);
typedef twMessage * (*twMessage_CreateResponseMsgStub)(enum msgCodeEnum code, uint32_t id, uint32_t sessionId, uint32_t endpointId);
typedef twMessage * (*twMessage_CreateBindMsgStub)(char * name, char isUnbind);
typedef twMessage * (*twMessage_CreateAuthMsgStub)(char * claimName, char * claimValue);
typedef twMessage * (*twMessage_CreateFromStreamStub)(twStream * s);
typedef void (*twMessage_DeleteStub)(void * input);
typedef int (*twMessage_SendStub)(struct twMessage ** msg, struct twWs * ws);
typedef int (*twMessage_SetBodyStub)(struct twMessage * msg, void * body);
typedef twRequestBody * (*twRequestBody_CreateStub)();
typedef twRequestBody * (*twRequestBody_CreateFromStreamStub)(twStream * s);
typedef int (*twRequestBody_DeleteStub)(struct twRequestBody * body);
typedef int (*twRequestBody_SetParamsStub)(struct twRequestBody * body, twInfoTable * params);
typedef int (*twRequestBody_SetEntityStub)(struct twRequestBody * body, enum entityTypeEnum entityType, char * entityName);
typedef int (*twRequestBody_SetCharacteristicStub)(struct twRequestBody * body, enum characteristicEnum characteristicType, char * characteristicName);
typedef int (*twRequestBody_AddHeaderStub)(struct twRequestBody * body, char * name, char * value);
typedef int (*twRequestBody_ToStreamStub)(struct twRequestBody * body, twStream * s);
typedef twResponseBody * (*twResponseBody_CreateStub)();
typedef twResponseBody * (*twResponseBody_CreateFromStreamStub)(twStream * s);
typedef int (*twResponseBody_DeleteStub)(struct twResponseBody * body);
typedef int (*twResponseBody_SetContentStub)(struct twResponseBody * body, twInfoTable * t);
typedef int (*twResponseBody_SetReasonStub)(struct twResponseBody * body, char * reason);
typedef int (*twResponseBody_ToStreamStub)(struct twResponseBody * body, twStream * s);
typedef twAuthBody * (*twAuthBody_CreateStub)();
typedef twAuthBody * (*twAuthBody_CreateFromStreamStub)(twStream * s);
typedef int (*twAuthBody_DeleteStub)(struct twAuthBody * body);
typedef int (*twAuthBody_SetClaimStub)(struct twAuthBody * body, char * name, char * value);
typedef int (*twAuthBody_ToStreamStub)(struct twAuthBody * body, twStream * s);
typedef twBindBody * (*twBindBody_CreateStub)(char * name);
typedef twBindBody * (*twBindBody_CreateFromStreamStub)(twStream * s);
typedef int (*twBindBody_DeleteStub)(struct twBindBody * body);
typedef int (*twBindBody_AddNameStub)(struct twBindBody * body, char * name);
typedef int (*twBindBody_ToStreamStub)(struct twBindBody * body, twStream * s, char * gatewayName, char * gatewayType);
typedef twMultipartBody * (*twMultipartBody_CreateFromStreamStub)(twStream * s, char isRequest);
typedef void (*twMultipartBody_DeleteStub)(void * body);
typedef mulitpartMessageStoreEntry * (*mulitpartMessageStoreEntry_CreateStub)(twMessage * msg);
typedef void (*mulitpartMessageStoreEntry_DeleteStub)(void * entry);
typedef twMultipartMessageStore * (*twMultipartMessageStore_InstanceStub)();
typedef void (*twMultipartMessageStore_DeleteStub)(void * store);
typedef twMessage * (*twMultipartMessageStore_AddMessageStub)(twMessage * msg);
typedef void (*twMultipartMessageStore_RemoveStaleMessagesStub)();
typedef int (*twCompressBytesStub)(char * buf, uint32_t length, twStream* s, struct twWs * ws);

/* Messaging */
typedef twMessageHandler * (*twMessageHandler_InstanceStub)(twWs * ws);
typedef int (*twMessageHandler_DeleteStub)(twMessageHandler * handler);
typedef int (*twMessageHandler_CleanupOldMessagesStub)(twMessageHandler * handler);
typedef void (*twMessageHandler_msgHandlerTaskStub)(DATETIME now, void * params);
typedef int (*twMessageHandler_RegisterConnectCallbackStub)(twMessageHandler * handler, eventcb cb);
typedef int (*twMessageHandler_RegisterCloseCallbackStub)(twMessageHandler * handler, eventcb cb);
typedef int (*twMessageHandler_RegisterPingCallbackStub)(twMessageHandler * handler, eventcb cb);
typedef int (*twMessageHandler_RegisterPongCallbackStub)(twMessageHandler * handler, eventcb cb);
typedef int (*twMessageHandler_RegisterDefaultRequestCallbackStub)(twMessageHandler * handler, message_cb cb);
typedef int (*twMessageHandler_RegisterDumpIncomingMsgListCallbackStub)(twMessageHandler * handler, dumpLog_cb cb);
typedef int (*twMessageHandler_RegisterRequestCallbackStub)(twMessageHandler * handler, message_cb cb, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName);
typedef int (*twMessageHandler_RegisterResponseCallbackStub)(twMessageHandler * handler, response_cb cb, uint32_t requestId, DATETIME expirationTime);
typedef twResponseCallbackStruct * (*twMessageHandler_GetCompletedResponseStructStub)(twMessageHandler * handler, uint32_t id);
typedef int (*twMessageHandler_UnegisterRequestCallbackStub)(twMessageHandler * handler, enum entityTypeEnum entityType, char * entityName, enum characteristicEnum characteristicType, char * characteristicName);
typedef int (*twMessageHandler_UnegisterResponseCallbackStub)(twMessageHandler * handler, uint32_t requestId);

/* Subscribed Properties */
typedef int (*twSubscribedPropsMgr_InitializeStub)();
typedef void (*twSubscribedPropsMgr_DeleteStub)();
typedef void (*twSubscribedPropsMgr_SetFoldingStub)(char fold);
typedef int (*twSubscribedPropsMgr_PushSubscribedPropertiesStub)(char * entityName, char forceConnect);
typedef int (*twSubscribedPropsMgr_SetPropertyVTQStub)(char * entityName, char * propertyName, twPrimitive * value,  DATETIME timestamp, char * quality, char fold, char pushUpdate);
typedef int (*twSubscribedPropsMgr_QueueValueForSendingStub) (twSubscribedProperty * pProp, twDict * pList, char* src);
typedef void (*twSubscribedProperty_DeleteStub)(void * prop);
typedef int (*twSubscribedProperty_ToStreamStub) (twSubscribedProperty * p, struct twStream * s);

/* Offline Message Store */
typedef int (*twOfflineMsgStore_InitializeStub)(char enabled, const char * filePath, uint64_t size, char onDisk);
typedef int (*twOfflineMsgStore_SetDirStub)(const char * dir);
typedef int (*twOfflineMsgStore_DeleteStub)();
typedef int (*twOfflineMsgStore_HandleRequestStub)(struct twMessage ** msg, struct twWs * ws, enum OfflineRequest request_type);

/* Crypto */
typedef int (*EncryptDESStub)(const unsigned char * key, unsigned char *  ct, const unsigned char *  pt);
typedef int (*DecryptDESStub)(const unsigned char * key, const unsigned char *  ct, unsigned char *  pt);
typedef void (*createDESKeyStub)(const uint8_t * bytes, uint8_t * key);
typedef int (*MD4HashStub)(const unsigned char * buf, int length, unsigned char * hash);


/* twList */
typedef twList * (*twList_CreateSearchableStub)(del_func delete_function,parse_func parse_function);
typedef twList * (*twList_CreateStub)(del_func delete_function);
typedef int (*twList_DeleteStub)(struct twList *list);
typedef int (*twList_ClearStub)(struct twList *list);
typedef int (*twList_AddStub)(twList *list, void *value);
typedef int (*twList_RemoveStub)(struct twList *list, struct ListEntry * entry, char deleteValue);
typedef ListEntry * (*twList_NextStub)(struct twList *list, struct ListEntry * entry);
typedef ListEntry * (*twList_GetByIndexStub)(struct twList *list, int index);
typedef int (*twList_GetCountStub)(struct twList *list);
typedef int (*twList_ReplaceValueStub)(struct twList *list, struct ListEntry * entry, void * new_value, char dispose);

/* twMap */
typedef int (*twMap_AddStub)(twMap* in, void *value);
typedef int (*twMap_RemoveStub)(twMap* in, void *value, char deleteValue);

/* twDict */
typedef int (*twDict_AddStub)(twDict* in, void *value);

/* String Utils */
typedef char * (*lowercaseStub)(char *input);
typedef char * (*uppercaseStub)(char *input);
typedef char * (*duplicateStringStub)(const char * input);

/* twProxy */
typedef int (*connectToProxyStub)(twSocket * s, char * authCredentials);
typedef int (*twSocket_WriteStub)(twSocket * s, char * buf, int len, int timeout);
typedef int (*twSocket_WaitForStub)(twSocket * s, int timeout);
typedef int (*twSocket_ReadStub)(twSocket * s, char * buf, int len, int timeout);

/* Logger */
typedef twLogger * (*twLogger_InstanceStub)();
typedef int (*twLogger_DeleteStub)();
typedef int (*twLogger_SetLevelStub)(enum  LogLevel level);
typedef int (*twLogger_SetFunctionStub)(log_function f);
typedef int (*twLogger_SetIsVerboseStub)(char val);
typedef void (*twLogStub)(enum LogLevel level, const char * format, ... );
typedef void (*twLogHexStringStub)(const char * msg, char * preamble, int32_t length);
typedef void (*twLogMessageStub)(void * m, char * preamble);
typedef char * (*twCodeToStringStub)(enum msgCodeEnum m);
typedef char * (*twEntityToStringStub)(enum entityTypeEnum m);
typedef char * (*twCharacteristicToStringStub)(enum characteristicEnum m);

/* NTLM */
typedef int (*NTLM_connectToProxyStub)(twSocket * sock, const char * req, const char * resp, char * user, char * password);

/* twNTLM */
typedef int (*NTLM_parseType2MsgStub)(twSocket * sock, const char * req, char * resp, char * domain, char * username, char * password);
typedef int (*GenerateType3MsgStub)(const char * domain, const char * username, const char * password, const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength);
typedef int (*GenerateType1MsgStub)(char **buffer, uint32_t *length);
typedef int (*NTLM_sendType1MsgStub)(twSocket * sock, const char * req, char * domain, char * user, char * password);

/* Tasker */
typedef void (*twTasker_InitializeStub)();
typedef int (*twTasker_CreateTaskStub)(uint32_t runTimeIntervalMsec, twTaskFunction func);
typedef int (*twTasker_RemoveTaskStub)(int id);

/* Websocket */
typedef int (*twWs_CreateStub)(char * host, uint16_t port, char * resource, char * api_key, char * gatewayName,uint32_t messageChunkSize, uint16_t frameSize, twWs ** entity);
typedef int (*twWs_DeleteStub)(twWs * ws);
typedef int (*twWs_ConnectStub)(twWs * ws, uint32_t timeout);
typedef int (*twWs_DisconnectStub)(twWs * ws, enum close_status code, char * reason);
typedef char (*twWs_IsConnectedStub)(twWs * ws);
typedef int (*twWs_RegisterConnectCallbackStub)(twWs * ws, ws_cb cb);
typedef int (*twWs_RegisterCloseCallbackStub)(twWs * ws, ws_data_cb cb);
typedef int (*twWs_RegisterBinaryMessageCallbackStub)(twWs * ws, ws_data_cb cb);
typedef int (*twWs_RegisterTextMessageCallbackStub)(twWs * ws, ws_data_cb cb);
typedef int (*twWs_RegisterPingCallbackStub)(twWs * ws, ws_data_cb cb);
typedef int (*twWs_RegisterPongCallbackStub)(twWs * ws, ws_data_cb cb);
typedef int (*twWs_ReceiveStub)(twWs * ws, uint32_t timeout);
typedef int (*twWs_SendMessageStub)(twWs * ws, char * buf, uint32_t length, char isText);
typedef int (*twWs_SendPingStub)(twWs * ws, char * msg);
typedef int (*twWs_SendPongStub)(twWs * ws, char * msg);
typedef int (*twWs_SendDataFrameStub)(twWs * ws, char * msg, uint16_t length, char isContinuation, char isFinal, char isText);

/* TLS Client */
typedef int	(*twTlsClient_UseCertificateChainFileStub)(twTlsClient * t, const char *file, int type);
typedef int (*twTlsClient_SetClientCaListStub)(twTlsClient * t, char * caFile, char * caPath);
typedef int (*twTlsClient_UsePrivateKeyFileStub)(twTlsClient * t, const char *file, int type);
typedef int (*twTlsClient_ReadStub)(twTlsClient * t, char * buf, int len, int timeout);
typedef int (*twTlsClient_WriteStub)(twTlsClient * t, char * buf, int len, int timeout);
typedef int (*twTlsClient_ReconnectStub)(twTlsClient * t, const char * host, int16_t port);

/* cJSON */
typedef void (*cJSON_DeleteStub)(cJSON *c);

/* Mutexes */
typedef void (*twMutex_LockStub)(TW_MUTEX m);
typedef void (*twMutex_UnlockStub)(TW_MUTEX m);
typedef TW_MUTEX (*twMutex_CreateStub)();

/* file transfer */
typedef char(*twDirectory_FileExistsStub)(char * name);
typedef int(*twDirectory_GetFileInfoStub)(char * filename, uint64_t * size, DATETIME * lastModified, char * isDirectory, char * isReadOnly);
typedef int(*twDirectory_GetLastErrorStub)();
typedef twFile * (*twFile_CreateStub)();
typedef void(*twFileManager_CloseFileStub)(void * file);
typedef twFile * (*twFileManager_GetOpenFileStub)(const char * thingName, const char * path, const char * filename, const char * tid, char * isTimedOut);
typedef char * (*twFileManager_GetRealPathStub)(const char * thingName, const char * path, const char * filename);
typedef void(* twFileManager_MakeFileCallbackStub)(char rcvd, twFileTransferInfo * fti);
typedef void(*twFile_DeleteStub)(void * f);
typedef TW_FILE_HANDLE(*twFile_FOpenStub)(const char * name, const char * mode);
typedef int(*twDirectory_CreateDirectoryStub)(char * name);
typedef int(*twDirectory_CreateFileStub)(char * name);
typedef int(*twDirectory_DeleteFileStub)(char * name);
typedef int(*listDirsInInfoTableStub)(char * entityName, char * virtualPath, twInfoTable * it);

/* base64_encode */
typedef int(*base64_encodeStub)(const unsigned char *in, unsigned long inlen, unsigned char *out, unsigned long *outlen);

typedef struct twApi_Stubs {
    /* internal functions */
	cfuhash_destroyStub cfuhash_destroy;
    deleteCallbackInfoStub deleteCallbackInfo;
    bindListEntry_CreateStub bindListEntry_Create;
    bindListEntry_DeleteStub bindListEntry_Delete;
    notifyPropertyUpdateHandlerStub notifyPropertyUpdateHandler;
    subscribedPropertyUpdateTaskStub subscribedPropertyUpdateTask;
    isFileTransferServiceStub isFileTransferService;
    isTunnelServiceStub isTunnelService;
    convertMsgCodeToErrorCodeStub convertMsgCodeToErrorCode;
    findRegisteredItemStub findRegisteredItem;
    getCallbackFromListStub getCallbackFromList;
    sendMessageBlockingStub sendMessageBlocking;
    makeRequestStub makeRequest;
    makePropertyRequestStub makePropertyRequest;
    twApi_SendResponseStub twApi_SendResponse;
    api_requesthandlerStub api_requesthandler;
    getMetadataServiceStub getMetadataService;
    pong_handlerStub pong_handler;
    makeAuthOrBindCallbacksStub makeAuthOrBindCallbacks;
    makeSynchronizedStateCallbacksStub makeSynchronizedStateCallbacks;
    registerServiceOrEventStub registerServiceOrEvent;
    AddAspectToEntityStub AddAspectToEntity;

    /* external functions */
    twApi_InitializeStub twApi_Initialize;
    twApi_DeleteStub twApi_Delete;
    twApi_SetProxyInfoStub twApi_SetProxyInfo;
    twApi_GetVersionStub twApi_GetVersion;
    twApi_BindAllStub twApi_BindAll;
    twApi_AuthenticateStub twApi_Authenticate;
    twApi_ConnectStub twApi_Connect;
    twApi_DisconnectStub twApi_Disconnect;
    twApi_isConnectedStub twApi_isConnected;
    twApi_ConnectionInProgressStub twApi_ConnectionInProgress;
    twApi_StopConnectionAttemptStub twApi_StopConnectionAttempt;
    twApi_SetDutyCycleStub twApi_SetDutyCycle;
    twApi_SetPingRateStub twApi_SetPingRate;
    twApi_SetConnectTimeoutStub twApi_SetConnectTimeout;
    twApi_SetConnectRetriesStub twApi_SetConnectRetries;
    twApi_SetGatewayNameStub twApi_SetGatewayName;
    twApi_SetGatewayTypeStub twApi_SetGatewayType;
    twApi_BindThingStub twApi_BindThing;
	twApi_BindThings_Metadata_OptionStub twApi_BindThings_Metadata_Option;
    twApi_UnbindThingStub twApi_UnbindThing;
    twApi_IsEntityBoundStub twApi_IsEntityBound;
    twApi_TaskerFunctionStub twApi_TaskerFunction;
    twApi_RegisterPropertyStub twApi_RegisterProperty;
    twApi_UpdatePropertyMetaDataStub twApi_UpdatePropertyMetaData;
    twApi_AddAspectToPropertyStub twApi_AddAspectToProperty;
    twApi_RegisterServiceStub twApi_RegisterService;
    twApi_AddAspectToServiceStub twApi_AddAspectToService;
    twApi_RegisterEventStub twApi_RegisterEvent;
    twApi_AddAspectToEventStub twApi_AddAspectToEvent;
    twApi_RegisterPropertyCallbackStub twApi_RegisterPropertyCallback;
    twApi_RegisterServiceCallbackStub twApi_RegisterServiceCallback;
    twApi_UnregisterThingStub twApi_UnregisterThing;
    twApi_UnregisterCallbackStub twApi_UnregisterCallback;
    twApi_UnregisterPropertyCallbackStub twApi_UnregisterPropertyCallback;
    twApi_UnregisterServiceCallbackStub twApi_UnregisterServiceCallback;
    twApi_RegisterDefaultRequestHandlerStub twApi_RegisterDefaultRequestHandler;
    twApi_CreatePropertyListStub twApi_CreatePropertyList;
    twApi_DeletePropertyListStub twApi_DeletePropertyList;
    twApi_AddPropertyToListStub twApi_AddPropertyToList;
    twApi_ReadPropertyStub twApi_ReadProperty;
    twApi_WritePropertyStub twApi_WriteProperty;
    twApi_SetSubscribedPropertyVTQStub twApi_SetSubscribedPropertyVTQ;
    twApi_SetSubscribedPropertyStub twApi_SetSubscribedProperty;
    twApi_PushSubscribedPropertiesStub twApi_PushSubscribedProperties;
    twApi_PushPropertiesStub twApi_PushProperties;
    twApi_InvokeServiceStub twApi_InvokeService;
    twApi_FireEventStub twApi_FireEvent;
    twApi_RegisterConnectCallbackStub twApi_RegisterConnectCallback;
    twApi_RegisterCloseCallbackStub twApi_RegisterCloseCallback;
    twApi_RegisterPingCallbackStub twApi_RegisterPingCallback;
    twApi_RegisterPongCallbackStub twApi_RegisterPongCallback;
    twApi_RegisterBindEventCallbackStub twApi_RegisterBindEventCallback;
    twApi_UnregisterBindEventCallbackStub twApi_UnregisterBindEventCallback;
    twApi_RegisterOnAuthenticatedCallbackStub twApi_RegisterOnAuthenticatedCallback;
    twApi_UnregisterOnAuthenticatedCallbackStub twApi_UnregisterOnAuthenticatedCallback;
    twApi_CleanupOldMessagesStub twApi_CleanupOldMessages;
    twApi_SendPingStub twApi_SendPing;
    twApi_CreateTaskStub twApi_CreateTask;
    twApi_SetSelfSignedOkStub twApi_SetSelfSignedOk;
    twApi_DisableCertValidationStub twApi_DisableCertValidation;
    twApi_LoadCACertStub twApi_LoadCACert;
    twApi_LoadClientCertStub twApi_LoadClientCert;
    twApi_SetClientKeyStub twApi_SetClientKey;
    twApi_EnableFipsModeStub twApi_EnableFipsMode;
	twApi_IsFIPSCompatibleStub twApi_IsFIPSCompatible;
    twApi_IsFipsModeEnabledStub twApi_IsFipsModeEnabled;	
	
    twApi_DisableEncryptionStub twApi_DisableEncryption;
    twApi_SetX509FieldsStub twApi_SetX509Fields;
    twApi_SetOfflineMsgStoreDirStub twApi_SetOfflineMsgStoreDir;
    twApi_GetConnectionInfoStub twApi_GetConnectionInfo;

    /* Properties */
    twPropertyDef_CreateStub twPropertyDef_Create;
    twPropertyDef_DeleteStub twPropertyDef_Delete;
    twProperty_CreateStub twProperty_Create;
    twPropertyVTQ_CreateStub twPropertyVTQ_Create;
    twProperty_CreateFromStreamStub twProperty_CreateFromStream;
    twProperty_DeleteStub twProperty_Delete;

    /* Services */
    twServiceDef_CreateStub twServiceDef_Create;
    twServiceDef_DeleteStub twServiceDef_Delete;

    /* Base Types */
    twStream_CreateStub twStream_Create;
    twStream_CreateFromCharArrayStub twStream_CreateFromCharArray;
    twStream_CreateFromCharArrayZeroCopyStub twStream_CreateFromCharArrayZeroCopy;
    twStream_DeleteStub twStream_Delete;
    twStream_GetDataStub twStream_GetData;
    twStream_GetIndexStub twStream_GetIndex;
    twStream_GetLengthStub twStream_GetLength;
    twStream_AddBytesStub twStream_AddBytes;
    twStream_GetBytesStub twStream_GetBytes;
    twStream_ResetStub twStream_Reset;
    twStream_CreateFromFileStub twStream_CreateFromFile;
    swap4bytesStub swap4bytes;
    swap8bytesStub swap8bytes;
    stringToStreamStub stringToStream;
    streamToStringStub streamToString;
    baseTypeFromStringStub baseTypeFromString;
    baseTypeToStringStub baseTypeToString;
    twPrimitive_CreateStub twPrimitive_Create;
    twPrimitive_CreateFromStreamStub twPrimitive_CreateFromStream;
    twPrimitive_CreateFromStreamTypedStub twPrimitive_CreateFromStreamTyped;
    twPrimitive_ZeroCopyStub twPrimitive_ZeroCopy;
    twPrimitive_FullCopyStub twPrimitive_FullCopy;
    twPrimitive_DeleteStub twPrimitive_Delete;
    twPrimitive_ToStreamStub twPrimitive_ToStream;
    twPrimitive_DecoupleStringAndDeleteStub twPrimitive_DecoupleStringAndDelete;
    twPrimitive_CompareStub twPrimitive_Compare;
    twPrimitive_IsTrueStub twPrimitive_IsTrue;
    twPrimitive_CreateFromLocationStub twPrimitive_CreateFromLocation;
    twPrimitive_CreateFromNumberStub twPrimitive_CreateFromNumber;
    twPrimitive_CreateFromIntegerStub twPrimitive_CreateFromInteger;
    twPrimitive_CreateFromDatetimeStub twPrimitive_CreateFromDatetime;
    twPrimitive_CreateFromCurrentTimeStub twPrimitive_CreateFromCurrentTime;
    twPrimitive_CreateFromBooleanStub twPrimitive_CreateFromBoolean;
    twPrimitive_CreateFromInfoTableStub twPrimitive_CreateFromInfoTable;
    twPrimitive_CreateVariantStub twPrimitive_CreateVariant;
    twPrimitive_CreateFromStringStub twPrimitive_CreateFromString;
    twPrimitive_CreateFromBlobStub twPrimitive_CreateFromBlob;
    twPrimitive_CreateFromVariableStub twPrimitive_CreateFromVariable;
    twPrimitive_ToJsonStub twPrimitive_ToJson;
    twPrimitive_CreateFromJsonStub twPrimitive_CreateFromJson;

    /* InfoTables */
    twDataShapeAspect_CreateStub twDataShapeAspect_Create;
    twDataShapeAspect_CreateFromStreamStub twDataShapeAspect_CreateFromStream;
    twDataShapeAspect_DeleteStub twDataShapeAspect_Delete;
    twDataShapeEntry_CreateStub twDataShapeEntry_Create;
    twDataShapeEntry_CreateFromStreamStub twDataShapeEntry_CreateFromStream;
    twDataShapeEntry_DeleteStub twDataShapeEntry_Delete;
    twDataShapeEntry_AddAspectStub twDataShapeEntry_AddAspect;
    twDataShapeEntry_GetLengthStub twDataShapeEntry_GetLength;
    twDataShapeEntry_ToStreamStub twDataShapeEntry_ToStream;
    twDataShape_CreateStub twDataShape_Create;
    twDataShape_CreateFromStreamStub twDataShape_CreateFromStream;
    twDataShape_DeleteStub twDataShape_Delete;
    twDataShape_GetLengthStub twDataShape_GetLength;
    twDataShape_ToStreamStub twDataShape_ToStream;
    twDataShape_SetNameStub twDataShape_SetName;
    twDataShape_AddEntryStub twDataShape_AddEntry;
    twDataShape_GetEntryIndexStub twDataShape_GetEntryIndex;
    twInfoTableRow_CreateStub twInfoTableRow_Create;
    twInfoTableRow_CreateFromStreamStub twInfoTableRow_CreateFromStream;
    twInfoTableRow_DeleteStub twInfoTableRow_Delete;
    twInfoTableRow_GetCountStub twInfoTableRow_GetCount;
    twInfoTableRow_GetLengthStub twInfoTableRow_GetLength;
    twInfoTableRow_AddEntryStub twInfoTableRow_AddEntry;
    twInfoTableRow_GetEntryStub twInfoTableRow_GetEntry;
    twInfoTableRow_ToStreamStub twInfoTableRow_ToStream;
    twInfoTable_CreateStub twInfoTable_Create;
    twInfoTable_CreateFromStreamStub twInfoTable_CreateFromStream;
    twInfoTable_DeleteStub twInfoTable_Delete;
    twInfoTable_FullCopyStub twInfoTable_FullCopy;
    twInfoTable_ZeroCopyStub twInfoTable_ZeroCopy;
    twInfoTable_CompareStub twInfoTable_Compare;
    twInfoTable_AddRowStub twInfoTable_AddRow;
    twInfoTable_GetEntryStub twInfoTable_GetEntry;
    twInfoTable_ToStreamStub twInfoTable_ToStream;
    twInfoTable_CreateFromPrimitiveStub twInfoTable_CreateFromPrimitive;
    twInfoTable_CreateFromStringStub twInfoTable_CreateFromString;
    twInfoTable_CreateFromNumberStub twInfoTable_CreateFromNumber;
    twInfoTable_CreateFromIntegerStub twInfoTable_CreateFromInteger;
    twInfoTable_CreateFromLocationStub twInfoTable_CreateFromLocation;
    twInfoTable_CreateFromBlobStub twInfoTable_CreateFromBlob;
    twInfoTable_CreateFromDatetimeStub twInfoTable_CreateFromDatetime;
    twInfoTable_CreateFromBooleanStub twInfoTable_CreateFromBoolean;
    twInfoTable_GetStringStub twInfoTable_GetString;
    twInfoTable_GetNumberStub twInfoTable_GetNumber;
    twInfoTable_GetIntegerStub twInfoTable_GetInteger;
    twInfoTable_GetLocationStub twInfoTable_GetLocation;
    twInfoTable_GetBlobStub twInfoTable_GetBlob;
    twInfoTable_GetDatetimeStub twInfoTable_GetDatetime;
    twInfoTable_GetBooleanStub twInfoTable_GetBoolean;
    twInfoTable_GetPrimitiveStub twInfoTable_GetPrimitive;
    twInfoTable_CreateFromJsonStub twInfoTable_CreateFromJson;
    twDataShape_ToJsonStub twDataShape_ToJson;
    twInfoTable_ToJsonStub twInfoTable_ToJson;

    /* Messages */
    twMessage_CreateStub twMessage_Create;
    twMessage_CreateRequestMsgStub twMessage_CreateRequestMsg;
    twMessage_CreateResponseMsgStub twMessage_CreateResponseMsg;
    twMessage_CreateBindMsgStub twMessage_CreateBindMsg;
    twMessage_CreateAuthMsgStub twMessage_CreateAuthMsg;
    twMessage_CreateFromStreamStub twMessage_CreateFromStream;
    twMessage_DeleteStub twMessage_Delete;
    twMessage_SendStub twMessage_Send;
    twMessage_SetBodyStub twMessage_SetBody;
    twRequestBody_CreateStub twRequestBody_Create;
    twRequestBody_CreateFromStreamStub twRequestBody_CreateFromStream;
    twRequestBody_DeleteStub twRequestBody_Delete;
    twRequestBody_SetParamsStub twRequestBody_SetParams;
    twRequestBody_SetEntityStub twRequestBody_SetEntity;
    twRequestBody_SetCharacteristicStub twRequestBody_SetCharacteristic;
    twRequestBody_AddHeaderStub twRequestBody_AddHeader;
    twRequestBody_ToStreamStub twRequestBody_ToStream;
    twResponseBody_CreateStub twResponseBody_Create;
    twResponseBody_CreateFromStreamStub twResponseBody_CreateFromStream;
    twResponseBody_DeleteStub twResponseBody_Delete;
    twResponseBody_SetContentStub twResponseBody_SetContent;
    twResponseBody_SetReasonStub twResponseBody_SetReason;
    twResponseBody_ToStreamStub twResponseBody_ToStream;
    twAuthBody_CreateStub twAuthBody_Create;
    twAuthBody_CreateFromStreamStub twAuthBody_CreateFromStream;
    twAuthBody_DeleteStub twAuthBody_Delete;
    twAuthBody_SetClaimStub twAuthBody_SetClaim;
    twAuthBody_ToStreamStub twAuthBody_ToStream;
    twBindBody_CreateStub twBindBody_Create;
    twBindBody_CreateFromStreamStub twBindBody_CreateFromStream;
    twBindBody_DeleteStub twBindBody_Delete;
    twBindBody_AddNameStub twBindBody_AddName;
    twBindBody_ToStreamStub twBindBody_ToStream;
    twMultipartBody_CreateFromStreamStub twMultipartBody_CreateFromStream;
    twMultipartBody_DeleteStub twMultipartBody_Delete;
    mulitpartMessageStoreEntry_CreateStub mulitpartMessageStoreEntry_Create;
    mulitpartMessageStoreEntry_DeleteStub mulitpartMessageStoreEntry_Delete;
    twMultipartMessageStore_InstanceStub twMultipartMessageStore_Instance;
    twMultipartMessageStore_DeleteStub twMultipartMessageStore_Delete;
    twMultipartMessageStore_AddMessageStub twMultipartMessageStore_AddMessage;
    twMultipartMessageStore_RemoveStaleMessagesStub twMultipartMessageStore_RemoveStaleMessages;
	twCompressBytesStub twCompressBytes;

    /* Messaging */
    twMessageHandler_InstanceStub twMessageHandler_Instance;
    twMessageHandler_DeleteStub twMessageHandler_Delete;
    twMessageHandler_CleanupOldMessagesStub twMessageHandler_CleanupOldMessages;
    twMessageHandler_msgHandlerTaskStub twMessageHandler_msgHandlerTask;
    twMessageHandler_RegisterConnectCallbackStub twMessageHandler_RegisterConnectCallback;
    twMessageHandler_RegisterCloseCallbackStub twMessageHandler_RegisterCloseCallback;
    twMessageHandler_RegisterPingCallbackStub twMessageHandler_RegisterPingCallback;
    twMessageHandler_RegisterPongCallbackStub twMessageHandler_RegisterPongCallback;
    twMessageHandler_RegisterDefaultRequestCallbackStub twMessageHandler_RegisterDefaultRequestCallback;
    twMessageHandler_RegisterDumpIncomingMsgListCallbackStub twMessageHandler_RegisterDumpIncomingMsgListCallback;
    twMessageHandler_RegisterRequestCallbackStub twMessageHandler_RegisterRequestCallback;
    twMessageHandler_RegisterResponseCallbackStub twMessageHandler_RegisterResponseCallback;
    twMessageHandler_GetCompletedResponseStructStub twMessageHandler_GetCompletedResponseStruct;
    twMessageHandler_UnegisterRequestCallbackStub twMessageHandler_UnegisterRequestCallback;
    twMessageHandler_UnegisterResponseCallbackStub twMessageHandler_UnegisterResponseCallback;

    /* Subscribed Properties */
    twSubscribedPropsMgr_InitializeStub twSubscribedPropsMgr_Initialize;
    twSubscribedPropsMgr_DeleteStub twSubscribedPropsMgr_Delete;
    twSubscribedPropsMgr_SetFoldingStub twSubscribedPropsMgr_SetFolding;
    twSubscribedPropsMgr_PushSubscribedPropertiesStub twSubscribedPropsMgr_PushSubscribedProperties;
    twSubscribedPropsMgr_SetPropertyVTQStub twSubscribedPropsMgr_SetPropertyVTQ;
    twSubscribedPropsMgr_QueueValueForSendingStub twSubscribedPropsMgr_QueueValueForSending;
    twSubscribedProperty_DeleteStub twSubscribedProperty_Delete;
    twSubscribedProperty_ToStreamStub twSubscribedProperty_ToStream;

	/* Offline Message Store */
	twOfflineMsgStore_HandleRequestStub twOfflineMsgStore_HandleRequest;
	twOfflineMsgStore_InitializeStub twOfflineMsgStore_Initialize;
	twOfflineMsgStore_SetDirStub twOfflineMsgStore_SetDir;
	twOfflineMsgStore_DeleteStub twOfflineMsgStore_Delete;

    /* Crypto */
    EncryptDESStub EncryptDES;
    DecryptDESStub DecryptDES;
    createDESKeyStub createDESKey;
    MD4HashStub MD4Hash;

    /* twList */
    twList_CreateStub twList_Create;
	twList_CreateSearchableStub twList_CreateSearchable;
    twList_DeleteStub twList_Delete;
    twList_ClearStub twList_Clear;
    twList_AddStub twList_Add;
    twList_RemoveStub twList_Remove;
    twList_NextStub twList_Next;
    twList_GetByIndexStub twList_GetByIndex;
    twList_GetCountStub twList_GetCount;
    twList_ReplaceValueStub twList_ReplaceValue;

    /* twMap */
    twMap_AddStub twMap_Add;
    twMap_RemoveStub twMap_Remove;

    /* twMap */
    twDict_AddStub twDict_Add;

    /* String Utils */
    lowercaseStub lowercase;
    uppercaseStub uppercase;
    duplicateStringStub duplicateString;

    /* twProxy */
    connectToProxyStub connectToProxy;
	twSocket_WriteStub twSocket_Write;
	twSocket_WaitForStub twSocket_WaitFor;
	twSocket_ReadStub twSocket_Read;

    /* Logger */
    twLogger_InstanceStub twLogger_Instance;
    twLogger_DeleteStub twLogger_Delete;
    twLogger_SetLevelStub twLogger_SetLevel;
    twLogger_SetFunctionStub twLogger_SetFunction;
    twLogger_SetIsVerboseStub twLogger_SetIsVerbose;
    twLogStub twLog;
    twLogHexStringStub twLogHexString;
    twLogMessageStub twLogMessage;
    twCodeToStringStub twCodeToString;
    twEntityToStringStub twEntityToString;
    twCharacteristicToStringStub twCharacteristicToString;

    /* NTLM */
    NTLM_connectToProxyStub NTLM_connectToProxy;
	
	/* twNTLM */
	NTLM_parseType2MsgStub NTLM_parseType2Msg;
	GenerateType3MsgStub GenerateType3Msg;
	GenerateType1MsgStub GenerateType1Msg;
	NTLM_sendType1MsgStub NTLM_sendType1Msg;

    /* Tasker */
    twTasker_InitializeStub twTasker_Initialize;
    twTasker_CreateTaskStub twTasker_CreateTask;
    twTasker_RemoveTaskStub twTasker_RemoveTask;

    /* Websocket */
    twWs_CreateStub twWs_Create;
    twWs_DeleteStub twWs_Delete;
    twWs_ConnectStub twWs_Connect;
    twWs_DisconnectStub twWs_Disconnect;
    twWs_IsConnectedStub twWs_IsConnected;
    twWs_RegisterConnectCallbackStub twWs_RegisterConnectCallback;
    twWs_RegisterCloseCallbackStub twWs_RegisterCloseCallback;
    twWs_RegisterBinaryMessageCallbackStub twWs_RegisterBinaryMessageCallback;
    twWs_RegisterTextMessageCallbackStub twWs_RegisterTextMessageCallback;
    twWs_RegisterPingCallbackStub twWs_RegisterPingCallback;
    twWs_RegisterPongCallbackStub twWs_RegisterPongCallback;
    twWs_ReceiveStub twWs_Receive;
    twWs_SendMessageStub twWs_SendMessage;
    twWs_SendPingStub twWs_SendPing;
    twWs_SendPongStub twWs_SendPong;
	twWs_SendDataFrameStub twWs_SendDataFrame;

    /* TLS Client */
    twTlsClient_UseCertificateChainFileStub twTlsClient_UseCertificateChainFile;
    twTlsClient_SetClientCaListStub twTlsClient_SetClientCaList;
    twTlsClient_UsePrivateKeyFileStub twTlsClient_UsePrivateKeyFile;
	twTlsClient_ReadStub twTlsClient_Read;
	twTlsClient_WriteStub twTlsClient_Write;
	twTlsClient_ReconnectStub twTlsClient_Reconnect;

    /* cJSON */
    cJSON_DeleteStub cJSON_Delete;

    /* Mutexes */
    twMutex_LockStub twMutex_Lock;
    twMutex_UnlockStub twMutex_Unlock;
    twMutex_CreateStub twMutex_Create;

	/* file transfer */
	twDirectory_FileExistsStub		twDirectory_FileExists;
	twDirectory_GetFileInfoStub		twDirectory_GetFileInfo;
	twDirectory_GetLastErrorStub	twDirectory_GetLastError;
	twFile_CreateStub				twFile_Create;
	twFileManager_CloseFileStub		twFileManager_CloseFile;
	twFileManager_GetOpenFileStub	twFileManager_GetOpenFile;
	twFileManager_GetRealPathStub	twFileManager_GetRealPath;
    twFileManager_MakeFileCallbackStub twFileManager_MakeFileCallback;
	twFile_DeleteStub				twFile_Delete;
	twFile_FOpenStub				twFile_FOpen;
	twDirectory_CreateDirectoryStub twDirectory_CreateDirectory;
	twDirectory_CreateFileStub      twDirectory_CreateFile;
	twDirectory_DeleteFileStub      twDirectory_DeleteFile;
	listDirsInInfoTableStub         listDirsInInfoTable;

	/* base64_encode */
	base64_encodeStub  base64_encode;
} twApi_Stubs;

#ifdef WIN32
extern __declspec(dllimport) twApi_Stubs * twApi_stub;
extern __declspec(dllimport) twApi * tw_api;
#else
extern twApi * tw_api;
extern twApi_Stubs * twApi_stub;
#endif

int twStubs_Use();
int twStubs_Reset();

int twApi_CreateStubs();
int twApi_DeleteStubs();

#endif
