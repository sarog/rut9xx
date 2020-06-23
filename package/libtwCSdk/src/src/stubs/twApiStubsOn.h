#ifndef TW_C_SDK_TWSTUBSON_H
#define TW_C_SDK_TWSTUBSON_H

#include "twApi.h"
#include "twProperties.h"
#include "twServices.h"

#define s_cfuhash_destroy twApi_stub->cfuhash_destroy
#define s_deleteCallbackInfo twApi_stub->deleteCallbackInfo 
#define s_bindListEntry_Create twApi_stub->bindListEntry_Create 
#define s_bindListEntry_Delete twApi_stub->bindListEntry_Delete 
#define s_notifyPropertyUpdateHandler twApi_stub->notifyPropertyUpdateHandler 
#define s_subscribedPropertyUpdateTask twApi_stub->subscribedPropertyUpdateTask 
#define s_isFileTransferService twApi_stub->isFileTransferService 
#define s_isTunnelService twApi_stub->isTunnelService 
#define s_convertMsgCodeToErrorCode twApi_stub->convertMsgCodeToErrorCode 
#define s_findRegisteredItem twApi_stub->findRegisteredItem 
#define s_getCallbackFromList twApi_stub->getCallbackFromList 
#define s_sendMessageBlocking twApi_stub->sendMessageBlocking 
#define s_makeRequest twApi_stub->makeRequest 
#define s_makePropertyRequest twApi_stub->makePropertyRequest 
#define s_twApi_SendResponse twApi_stub->twApi_SendResponse 
#define s_api_requesthandler twApi_stub->api_requesthandler 
#define s_getMetadataService twApi_stub->getMetadataService 
#define s_pong_handler twApi_stub->pong_handler 
#define s_makeAuthOrBindCallbacks twApi_stub->makeAuthOrBindCallbacks
#define s_makeSynchronizedStateCallbacks twApi_stub->makeSynchronizedStateCallbacks
#define s_registerServiceOrEvent twApi_stub->registerServiceOrEvent 
#define s_AddAspectToEntity twApi_stub->AddAspectToEntity 

#define s_twApi_Delete twApi_stub->twApi_Delete 
#define s_twApi_SetProxyInfo twApi_stub->twApi_SetProxyInfo 
#define s_twApi_GetVersion twApi_stub->twApi_GetVersion 
#define s_twApi_BindAll twApi_stub->twApi_BindAll 
#define s_twApi_Authenticate twApi_stub->twApi_Authenticate 
#define s_twApi_Connect twApi_stub->twApi_Connect 
#define s_twApi_Disconnect twApi_stub->twApi_Disconnect 
#define s_twApi_isConnected twApi_stub->twApi_isConnected 
#define s_twApi_ConnectionInProgress twApi_stub->twApi_ConnectionInProgress 
#define s_twApi_StopConnectionAttempt twApi_stub->twApi_StopConnectionAttempt 
#define s_twApi_SetDutyCycle twApi_stub->twApi_SetDutyCycle 
#define s_twApi_SetPingRate twApi_stub->twApi_SetPingRate 
#define s_twApi_SetConnectTimeout twApi_stub->twApi_SetConnectTimeout 
#define s_twApi_SetConnectRetries twApi_stub->twApi_SetConnectRetries 
#define s_twApi_SetGatewayName twApi_stub->twApi_SetGatewayName 
#define s_twApi_SetGatewayType twApi_stub->twApi_SetGatewayType 
#define s_twApi_BindThing twApi_stub->twApi_BindThing 
#define s_twApi_UnbindThing twApi_stub->twApi_UnbindThing 
#define s_twApi_IsEntityBound twApi_stub->twApi_IsEntityBound 
#define s_twApi_TaskerFunction twApi_stub->twApi_TaskerFunction 
#define s_twApi_RegisterProperty twApi_stub->twApi_RegisterProperty 
#define s_twApi_UpdatePropertyMetaData twApi_stub->twApi_UpdatePropertyMetaData 
#define s_twApi_AddAspectToProperty twApi_stub->twApi_AddAspectToProperty 
#define s_twApi_RegisterService twApi_stub->twApi_RegisterService 
#define s_twApi_AddAspectToService twApi_stub->twApi_AddAspectToService 
#define s_twApi_RegisterEvent twApi_stub->twApi_RegisterEvent 
#define s_twApi_AddAspectToEvent twApi_stub->twApi_AddAspectToEvent 
#define s_twApi_RegisterPropertyCallback twApi_stub->twApi_RegisterPropertyCallback 
#define s_twApi_RegisterServiceCallback twApi_stub->twApi_RegisterServiceCallback 
#define s_twApi_UnregisterThing twApi_stub->twApi_UnregisterThing 
#define s_twApi_UnregisterCallback twApi_stub->twApi_UnregisterCallback 
#define s_twApi_UnregisterPropertyCallback twApi_stub->twApi_UnregisterPropertyCallback 
#define s_twApi_UnregisterServiceCallback twApi_stub->twApi_UnregisterServiceCallback 
#define s_twApi_RegisterDefaultRequestHandler twApi_stub->twApi_RegisterDefaultRequestHandler 
#define s_twApi_CreatePropertyList twApi_stub->twApi_CreatePropertyList 
#define s_twApi_DeletePropertyList twApi_stub->twApi_DeletePropertyList 
#define s_twApi_AddPropertyToList twApi_stub->twApi_AddPropertyToList 
#define s_twApi_ReadProperty twApi_stub->twApi_ReadProperty 
#define s_twApi_WriteProperty twApi_stub->twApi_WriteProperty 
#define s_twApi_SetSubscribedPropertyVTQ twApi_stub->twApi_SetSubscribedPropertyVTQ 
#define s_twApi_SetSubscribedProperty twApi_stub->twApi_SetSubscribedProperty 
#define s_twApi_PushSubscribedProperties twApi_stub->twApi_PushSubscribedProperties 
#define s_twApi_PushProperties twApi_stub->twApi_PushProperties 
#define s_twApi_InvokeService twApi_stub->twApi_InvokeService 
#define s_twApi_FireEvent twApi_stub->twApi_FireEvent 
#define s_twApi_RegisterConnectCallback twApi_stub->twApi_RegisterConnectCallback 
#define s_twApi_RegisterCloseCallback twApi_stub->twApi_RegisterCloseCallback 
#define s_twApi_RegisterPingCallback twApi_stub->twApi_RegisterPingCallback 
#define s_twApi_RegisterPongCallback twApi_stub->twApi_RegisterPongCallback 
#define s_twApi_RegisterBindEventCallback twApi_stub->twApi_RegisterBindEventCallback 
#define s_twApi_UnregisterBindEventCallback twApi_stub->twApi_UnregisterBindEventCallback 
#define s_twApi_RegisterOnAuthenticatedCallback twApi_stub->twApi_RegisterOnAuthenticatedCallback 
#define s_twApi_UnregisterOnAuthenticatedCallback twApi_stub->twApi_UnregisterOnAuthenticatedCallback 
#define s_twApi_CleanupOldMessages twApi_stub->twApi_CleanupOldMessages 
#define s_twApi_SendPing twApi_stub->twApi_SendPing 
#define s_twApi_CreateTask twApi_stub->twApi_CreateTask 
#define s_twApi_SetSelfSignedOk twApi_stub->twApi_SetSelfSignedOk 
#define s_twApi_DisableCertValidation twApi_stub->twApi_DisableCertValidation 
#define s_twApi_LoadCACert twApi_stub->twApi_LoadCACert 
#define s_twApi_LoadClientCert twApi_stub->twApi_LoadClientCert 
#define s_twApi_SetClientKey twApi_stub->twApi_SetClientKey 
#define s_twApi_EnableFipsMode twApi_stub->twApi_EnableFipsMode
#define s_twApi_IsFIPSCompatible twApi_stub->twApi_IsFIPSCompatible
#define s_twApi_IsFipsModeEnabled twApi_stub->twApi_IsFipsModeEnabled
#define s_twApi_DisableEncryption twApi_stub->twApi_DisableEncryption 
#define s_twApi_SetX509Fields twApi_stub->twApi_SetX509Fields 
#define s_twApi_SetOfflineMsgStoreDir twApi_stub->twApi_SetOfflineMsgStoreDir 
#define s_twApi_GetConnectionInfo twApi_stub->twApi_GetConnectionInfo 

/* Properties */
#define s_twPropertyDef_Create twApi_stub->twPropertyDef_Create 
#define s_twPropertyDef_Delete twApi_stub->twPropertyDef_Delete 
#define s_twProperty_Create twApi_stub->twProperty_Create 
#define s_twPropertyVTQ_Create twApi_stub->twPropertyVTQ_Create 
#define s_twProperty_CreateFromStream twApi_stub->twProperty_CreateFromStream 
#define s_twProperty_Delete twApi_stub->twProperty_Delete 

/* Services */
#define s_twServiceDef_Create twApi_stub->twServiceDef_Create 
#define s_twServiceDef_Delete twApi_stub->twServiceDef_Delete 

/* Base Types */
#define s_twStream_Create twApi_stub->twStream_Create 
#define s_twStream_CreateFromCharArray twApi_stub->twStream_CreateFromCharArray 
#define s_twStream_CreateFromCharArrayZeroCopy twApi_stub->twStream_CreateFromCharArrayZeroCopy 
#define s_twStream_Delete twApi_stub->twStream_Delete 
#define s_twStream_GetData twApi_stub->twStream_GetData 
#define s_twStream_GetIndex twApi_stub->twStream_GetIndex 
#define s_twStream_GetLength twApi_stub->twStream_GetLength 
#define s_twStream_AddBytes twApi_stub->twStream_AddBytes 
#define s_twStream_GetBytes twApi_stub->twStream_GetBytes 
#define s_twStream_Reset twApi_stub->twStream_Reset 
#define s_twStream_CreateFromFile twApi_stub->twStream_CreateFromFile 
#define s_swap4bytes twApi_stub->swap4bytes 
#define s_swap8bytes twApi_stub->swap8bytes 
#define s_stringToStream twApi_stub->stringToStream 
#define s_streamToString twApi_stub->streamToString 
#define s_baseTypeFromString twApi_stub->baseTypeFromString 
#define s_baseTypeToString twApi_stub->baseTypeToString 
#define s_twPrimitive_Create twApi_stub->twPrimitive_Create 
#define s_twPrimitive_CreateFromStream twApi_stub->twPrimitive_CreateFromStream 
#define s_twPrimitive_CreateFromStreamTyped twApi_stub->twPrimitive_CreateFromStreamTyped 
#define s_twPrimitive_ZeroCopy twApi_stub->twPrimitive_ZeroCopy 
#define s_twPrimitive_FullCopy twApi_stub->twPrimitive_FullCopy 
#define s_twPrimitive_Delete twApi_stub->twPrimitive_Delete 
#define s_twPrimitive_ToStream twApi_stub->twPrimitive_ToStream 
#define s_twPrimitive_DecoupleStringAndDelete twApi_stub->twPrimitive_DecoupleStringAndDelete 
#define s_twPrimitive_Compare twApi_stub->twPrimitive_Compare 
#define s_twPrimitive_IsTrue twApi_stub->twPrimitive_IsTrue 
#define s_twPrimitive_CreateFromLocation twApi_stub->twPrimitive_CreateFromLocation 
#define s_twPrimitive_CreateFromNumber twApi_stub->twPrimitive_CreateFromNumber 
#define s_twPrimitive_CreateFromInteger twApi_stub->twPrimitive_CreateFromInteger 
#define s_twPrimitive_CreateFromDatetime twApi_stub->twPrimitive_CreateFromDatetime 
#define s_twPrimitive_CreateFromCurrentTime twApi_stub->twPrimitive_CreateFromCurrentTime 
#define s_twPrimitive_CreateFromBoolean twApi_stub->twPrimitive_CreateFromBoolean 
#define s_twPrimitive_CreateFromInfoTable twApi_stub->twPrimitive_CreateFromInfoTable 
#define s_twPrimitive_CreateVariant twApi_stub->twPrimitive_CreateVariant 
#define s_twPrimitive_CreateFromString twApi_stub->twPrimitive_CreateFromString 
#define s_twPrimitive_CreateFromBlob twApi_stub->twPrimitive_CreateFromBlob 
#define s_twPrimitive_CreateFromVariable twApi_stub->twPrimitive_CreateFromVariable 
#define s_twPrimitive_ToJson twApi_stub->twPrimitive_ToJson 
#define s_twPrimitive_CreateFromJson twApi_stub->twPrimitive_CreateFromJson 

/* InfoTables */
#define s_twDataShapeAspect_Create twApi_stub->twDataShapeAspect_Create 
#define s_twDataShapeAspect_CreateFromStream twApi_stub->twDataShapeAspect_CreateFromStream 
#define s_twDataShapeAspect_Delete twApi_stub->twDataShapeAspect_Delete 
#define s_twDataShapeEntry_Create twApi_stub->twDataShapeEntry_Create 
#define s_twDataShapeEntry_CreateFromStream twApi_stub->twDataShapeEntry_CreateFromStream 
#define s_twDataShapeEntry_Delete twApi_stub->twDataShapeEntry_Delete 
#define s_twDataShapeEntry_AddAspect twApi_stub->twDataShapeEntry_AddAspect 
#define s_twDataShapeEntry_GetLength twApi_stub->twDataShapeEntry_GetLength 
#define s_twDataShapeEntry_ToStream twApi_stub->twDataShapeEntry_ToStream 
#define s_twDataShape_Create twApi_stub->twDataShape_Create 
#define s_twDataShape_CreateFromStream twApi_stub->twDataShape_CreateFromStream 
#define s_twDataShape_Delete twApi_stub->twDataShape_Delete 
#define s_twDataShape_GetLength twApi_stub->twDataShape_GetLength 
#define s_twDataShape_ToStream twApi_stub->twDataShape_ToStream 
#define s_twDataShape_SetName twApi_stub->twDataShape_SetName 
#define s_twDataShape_AddEntry twApi_stub->twDataShape_AddEntry 
#define s_twDataShape_GetEntryIndex twApi_stub->twDataShape_GetEntryIndex 
#define s_twInfoTableRow_Create twApi_stub->twInfoTableRow_Create 
#define s_twInfoTableRow_CreateFromStream twApi_stub->twInfoTableRow_CreateFromStream 
#define s_twInfoTableRow_Delete twApi_stub->twInfoTableRow_Delete 
#define s_twInfoTableRow_GetCount twApi_stub->twInfoTableRow_GetCount 
#define s_twInfoTableRow_GetLength twApi_stub->twInfoTableRow_GetLength 
#define s_twInfoTableRow_AddEntry twApi_stub->twInfoTableRow_AddEntry 
#define s_twInfoTableRow_GetEntry twApi_stub->twInfoTableRow_GetEntry 
#define s_twInfoTableRow_ToStream twApi_stub->twInfoTableRow_ToStream 
#define s_twInfoTable_Create twApi_stub->twInfoTable_Create 
#define s_twInfoTable_CreateFromStream twApi_stub->twInfoTable_CreateFromStream 
#define s_twInfoTable_Delete twApi_stub->twInfoTable_Delete 
#define s_twInfoTable_FullCopy twApi_stub->twInfoTable_FullCopy 
#define s_twInfoTable_ZeroCopy twApi_stub->twInfoTable_ZeroCopy 
#define s_twInfoTable_Compare twApi_stub->twInfoTable_Compare 
#define s_twInfoTable_AddRow twApi_stub->twInfoTable_AddRow 
#define s_twInfoTable_GetEntry twApi_stub->twInfoTable_GetEntry 
#define s_twInfoTable_ToStream twApi_stub->twInfoTable_ToStream 
#define s_twInfoTable_CreateFromPrimitive twApi_stub->twInfoTable_CreateFromPrimitive 
#define s_twInfoTable_CreateFromString twApi_stub->twInfoTable_CreateFromString 
#define s_twInfoTable_CreateFromNumber twApi_stub->twInfoTable_CreateFromNumber 
#define s_twInfoTable_CreateFromInteger twApi_stub->twInfoTable_CreateFromInteger 
#define s_twInfoTable_CreateFromLocation twApi_stub->twInfoTable_CreateFromLocation 
#define s_twInfoTable_CreateFromBlob twApi_stub->twInfoTable_CreateFromBlob 
#define s_twInfoTable_CreateFromDatetime twApi_stub->twInfoTable_CreateFromDatetime 
#define s_twInfoTable_CreateFromBoolean twApi_stub->twInfoTable_CreateFromBoolean 
#define s_twInfoTable_GetString twApi_stub->twInfoTable_GetString 
#define s_twInfoTable_GetNumber twApi_stub->twInfoTable_GetNumber 
#define s_twInfoTable_GetInteger twApi_stub->twInfoTable_GetInteger 
#define s_twInfoTable_GetLocation twApi_stub->twInfoTable_GetLocation 
#define s_twInfoTable_GetBlob twApi_stub->twInfoTable_GetBlob 
#define s_twInfoTable_GetDatetime twApi_stub->twInfoTable_GetDatetime 
#define s_twInfoTable_GetBoolean twApi_stub->twInfoTable_GetBoolean 
#define s_twInfoTable_GetPrimitive twApi_stub->twInfoTable_GetPrimitive 
#define s_twInfoTable_CreateFromJson twApi_stub->twInfoTable_CreateFromJson 
#define s_twDataShape_ToJson twApi_stub->twDataShape_ToJson 
#define s_twInfoTable_ToJson twApi_stub->twInfoTable_ToJson 

/* Messages */
#define s_twMessage_Create twApi_stub->twMessage_Create 
#define s_twMessage_CreateRequestMsg twApi_stub->twMessage_CreateRequestMsg 
#define s_twMessage_CreateResponseMsg twApi_stub->twMessage_CreateResponseMsg 
#define s_twMessage_CreateBindMsg twApi_stub->twMessage_CreateBindMsg 
#define s_twMessage_CreateAuthMsg twApi_stub->twMessage_CreateAuthMsg 
#define s_twMessage_CreateFromStream twApi_stub->twMessage_CreateFromStream 
#define s_twMessage_Delete twApi_stub->twMessage_Delete 
#define s_twMessage_Send twApi_stub->twMessage_Send 
#define s_twMessage_SetBody twApi_stub->twMessage_SetBody 
#define s_twRequestBody_Create twApi_stub->twRequestBody_Create 
#define s_twRequestBody_CreateFromStream twApi_stub->twRequestBody_CreateFromStream 
#define s_twRequestBody_Delete twApi_stub->twRequestBody_Delete 
#define s_twRequestBody_SetParams twApi_stub->twRequestBody_SetParams 
#define s_twRequestBody_SetEntity twApi_stub->twRequestBody_SetEntity 
#define s_twRequestBody_SetCharacteristic twApi_stub->twRequestBody_SetCharacteristic
#define s_twRequestBody_AddHeader twApi_stub->twRequestBody_AddHeader 
#define s_twRequestBody_ToStream twApi_stub->twRequestBody_ToStream 
#define s_twResponseBody_Create twApi_stub->twResponseBody_Create 
#define s_twResponseBody_CreateFromStream twApi_stub->twResponseBody_CreateFromStream 
#define s_twResponseBody_Delete twApi_stub->twResponseBody_Delete 
#define s_twResponseBody_SetContent twApi_stub->twResponseBody_SetContent 
#define s_twResponseBody_SetReason twApi_stub->twResponseBody_SetReason 
#define s_twResponseBody_ToStream twApi_stub->twResponseBody_ToStream 
#define s_twAuthBody_Create twApi_stub->twAuthBody_Create 
#define s_twAuthBody_CreateFromStream twApi_stub->twAuthBody_CreateFromStream 
#define s_twAuthBody_Delete twApi_stub->twAuthBody_Delete 
#define s_twAuthBody_SetClaim twApi_stub->twAuthBody_SetClaim 
#define s_twAuthBody_ToStream twApi_stub->twAuthBody_ToStream 
#define s_twBindBody_Create twApi_stub->twBindBody_Create 
#define s_twBindBody_CreateFromStream twApi_stub->twBindBody_CreateFromStream 
#define s_twBindBody_Delete twApi_stub->twBindBody_Delete 
#define s_twBindBody_AddName twApi_stub->twBindBody_AddName 
#define s_twBindBody_ToStream twApi_stub->twBindBody_ToStream 
#define s_twMultipartBody_CreateFromStream twApi_stub->twMultipartBody_CreateFromStream 
#define s_twMultipartBody_Delete twApi_stub->twMultipartBody_Delete 
#define s_mulitpartMessageStoreEntry_Create twApi_stub->mulitpartMessageStoreEntry_Create 
#define s_mulitpartMessageStoreEntry_Delete twApi_stub->mulitpartMessageStoreEntry_Delete 
#define s_twMultipartMessageStore_Instance twApi_stub->twMultipartMessageStore_Instance 
#define s_twMultipartMessageStore_Delete twApi_stub->twMultipartMessageStore_Delete 
#define s_twMultipartMessageStore_AddMessage twApi_stub->twMultipartMessageStore_AddMessage 
#define s_twMultipartMessageStore_RemoveStaleMessages twApi_stub->twMultipartMessageStore_RemoveStaleMessages 
#define s_twCompressBytes twApi_stub->twCompressBytes

/* Messaging */
#define s_twMessageHandler_Instance twApi_stub->twMessageHandler_Instance 
#define s_twMessageHandler_Delete twApi_stub->twMessageHandler_Delete 
#define s_twMessageHandler_CleanupOldMessages twApi_stub->twMessageHandler_CleanupOldMessages 
#define s_twMessageHandler_msgHandlerTask twApi_stub->twMessageHandler_msgHandlerTask 
#define s_twMessageHandler_RegisterConnectCallback twApi_stub->twMessageHandler_RegisterConnectCallback 
#define s_twMessageHandler_RegisterCloseCallback twApi_stub->twMessageHandler_RegisterCloseCallback 
#define s_twMessageHandler_RegisterPingCallback twApi_stub->twMessageHandler_RegisterPingCallback 
#define s_twMessageHandler_RegisterPongCallback twApi_stub->twMessageHandler_RegisterPongCallback 
#define s_twMessageHandler_RegisterDefaultRequestCallback twApi_stub->twMessageHandler_RegisterDefaultRequestCallback 
#define s_twMessageHandler_RegisterDumpIncomingMsgListCallback twApi_stub->twMessageHandler_RegisterDumpIncomingMsgListCallback
#define s_twMessageHandler_RegisterRequestCallback twApi_stub->twMessageHandler_RegisterRequestCallback 
#define s_twMessageHandler_RegisterResponseCallback twApi_stub->twMessageHandler_RegisterResponseCallback 
#define s_twMessageHandler_GetCompletedResponseStruct twApi_stub->twMessageHandler_GetCompletedResponseStruct 
#define s_twMessageHandler_UnegisterRequestCallback twApi_stub->twMessageHandler_UnegisterRequestCallback 
#define s_twMessageHandler_UnegisterResponseCallback twApi_stub->twMessageHandler_UnegisterResponseCallback 

/* Subscribed Properties */
#define s_twSubscribedPropsMgr_Initialize twApi_stub->twSubscribedPropsMgr_Initialize 
#define s_twSubscribedPropsMgr_Delete twApi_stub->twSubscribedPropsMgr_Delete 
#define s_twSubscribedPropsMgr_SetFolding twApi_stub->twSubscribedPropsMgr_SetFolding 
#define s_twSubscribedPropsMgr_PushSubscribedProperties twApi_stub->twSubscribedPropsMgr_PushSubscribedProperties 
#define s_twSubscribedPropsMgr_SetPropertyVTQ twApi_stub->twSubscribedPropsMgr_SetPropertyVTQ
#define s_twSubscribedPropsMgr_QueueValueForSending twApi_stub->twSubscribedPropsMgr_QueueValueForSending
#define s_twSubscribedProperty_Delete twApi_stub->twSubscribedProperty_Delete
#define s_twSubscribedProperty_ToStream twApi_stub->twSubscribedProperty_ToStream

/* Offline Message Store */
#define s_twOfflineMsgStore_Initialize twApi_stub->twOfflineMsgStore_Initialize
#define s_twOfflineMsgStore_SetDir twApi_stub->twOfflineMsgStore_SetDir
#define s_twOfflineMsgStore_Delete twApi_stub->twOfflineMsgStore_Delete
#define s_twOfflineMsgStore_HandleRequest twApi_stub->twOfflineMsgStore_HandleRequest

/* Crypto */
#define s_EncryptDES twApi_stub->EncryptDES 
#define s_DecryptDES twApi_stub->DecryptDES 
#define s_createDESKey twApi_stub->createDESKey 
#define s_MD4Hash twApi_stub->MD4Hash 

/* twList */
#define s_twList_Create twApi_stub->twList_Create 
#define s_twList_CreateSearchable twApi_stub->twList_CreateSearchable
#define s_twList_Delete twApi_stub->twList_Delete
#define s_twList_Clear twApi_stub->twList_Clear 
#define s_twList_Add twApi_stub->twList_Add 
#define s_twList_Remove twApi_stub->twList_Remove 
#define s_twList_Next twApi_stub->twList_Next 
#define s_twList_GetByIndex twApi_stub->twList_GetByIndex 
#define s_twList_GetCount twApi_stub->twList_GetCount 
#define s_twList_ReplaceValue twApi_stub->twList_ReplaceValue 

/* twMap */
#define s_twMap_Add twApi_stub->twMap_Add
#define s_twMap_Remove twApi_stub->twMap_Remove

/* twDict */
#define s_twDict_Add twApi_stub->twDict_Add

/* String Utils */
#define s_lowercase twApi_stub->lowercase 
#define s_uppercase twApi_stub->uppercase 
#define s_duplicateString twApi_stub->duplicateString 

/* twProxy */
/*#define s_connectToProxy twApi_stub->connectToProxy */
#define s_twSocket_Write twApi_stub->twSocket_Write
#define s_twSocket_WaitFor twApi_stub->twSocket_WaitFor
#define s_twSocket_Read twApi_stub->twSocket_Read

/* Logger */
#define s_twLogger_Instance twApi_stub->twLogger_Instance 
#define s_twLogger_Delete twApi_stub->twLogger_Delete 
#define s_twLogger_SetLevel twApi_stub->twLogger_SetLevel 
#define s_twLogger_SetFunction twApi_stub->twLogger_SetFunction 
#define s_twLogger_SetIsVerbose twApi_stub->twLogger_SetIsVerbose 
#define s_twLog twApi_stub->twLog 
#define s_twLogHexString twApi_stub->twLogHexString 
#define s_twLogMessage twApi_stub->twLogMessage 
#define s_twCodeToString twApi_stub->twCodeToString 
#define s_twEntityToString twApi_stub->twEntityToString 
#define s_twCharacteristicToString twApi_stub->twCharacteristicToString 

/* NTLM */
/*#define s_NTLM_connectToProxy twApi_stub->NTLM_connectToProxy */

/* twNTLM */
#define s_NTLM_parseType2Msg twApi_stub->NTLM_parseType2Msg
#define s_GenerateType3Msg twApi_stub->GenerateType3Msg
#define s_GenerateType1Msg twApi_stub->GenerateType1Msg
#define s_NTLM_sendType1Msg twApi_stub->NTLM_sendType1Msg

/* Tasker */
#ifdef ENABLE_TASKER
#define s_twTasker_Initialize twApi_stub->twTasker_Initialize 
    #define s_twTasker_CreateTask twApi_stub->twTasker_CreateTask 
    #define s_twTasker_RemoveTask twApi_stub->twTasker_RemoveTask 
#endif

/* Websocket */
#define s_twWs_Create twApi_stub->twWs_Create 
#define s_twWs_Delete twApi_stub->twWs_Delete 
#define s_twWs_Connect twApi_stub->twWs_Connect 
#define s_twWs_Disconnect twApi_stub->twWs_Disconnect 
#define s_twWs_IsConnected twApi_stub->twWs_IsConnected 
#define s_twWs_RegisterConnectCallback twApi_stub->twWs_RegisterConnectCallback 
#define s_twWs_RegisterCloseCallback twApi_stub->twWs_RegisterCloseCallback 
#define s_twWs_RegisterBinaryMessageCallback twApi_stub->twWs_RegisterBinaryMessageCallback 
#define s_twWs_RegisterTextMessageCallback twApi_stub->twWs_RegisterTextMessageCallback 
#define s_twWs_RegisterPingCallback twApi_stub->twWs_RegisterPingCallback 
#define s_twWs_RegisterPongCallback twApi_stub->twWs_RegisterPongCallback 
#define s_twWs_Receive twApi_stub->twWs_Receive 
#define s_twWs_SendMessage twApi_stub->twWs_SendMessage 
#define s_twWs_SendPing twApi_stub->twWs_SendPing
#define s_twWs_SendPong twApi_stub->twWs_SendPong
#define s_twWs_SendDataFrame twApi_stub->twWs_SendDataFrame

/* TLS Client */
#define s_twTlsClient_UseCertificateChainFile twApi_stub->twTlsClient_UseCertificateChainFile
#define s_twTlsClient_SetClientCaList twApi_stub->twTlsClient_SetClientCaList
#define s_twTlsClient_UsePrivateKeyFile twApi_stub->twTlsClient_UsePrivateKeyFile
#define s_twTlsClient_Read twApi_stub->twTlsClient_Read
#define s_twTlsClient_Write twApi_stub->twTlsClient_Write
#define s_twTlsClient_Reconnect twApi_stub->twTlsClient_Reconnect

/* cJSON */
#define s_cJSON_Delete twApi_stub->cJSON_Delete

/* Mutexes */
#define s_twMutex_Lock   twApi_stub->twMutex_Lock
#define s_twMutex_Unlock twApi_stub->twMutex_Unlock
#define s_twMutex_Create twApi_stub->twMutex_Create

/* file transfer */
#define s_twDirectory_FileExists		 twApi_stub->twDirectory_FileExists
#define s_twDirectory_GetFileInfo		 twApi_stub->twDirectory_GetFileInfo
#define s_twDirectory_GetLastError		 twApi_stub->twDirectory_GetLastError
#define s_twFile_Create					 twApi_stub->twFile_Create
#define s_twFileManager_CloseFile		 twApi_stub->twFileManager_CloseFile
#define s_twFileManager_GetOpenFile		 twApi_stub->twFileManager_GetOpenFile
#define s_twFileManager_MakeFileCallback twApi_stub->twFileManager_MakeFileCallback
#define s_twFileManager_GetRealPath		 twApi_stub->twFileManager_GetRealPath
#define s_twFile_Delete					 twApi_stub->twFile_Delete
#define s_twFile_FOpen					 twApi_stub->twFile_FOpen
#define s_twDirectory_CreateDirectory    twApi_stub->twDirectory_CreateDirectory
#define s_twDirectory_CreateFile         twApi_stub->twDirectory_CreateFile
#define s_twDirectory_DeleteFile         twApi_stub->twDirectory_DeleteFile
#define s_listDirsInInfoTable			 twApi_stub->listDirsInInfoTable

/* base64_encode */
#define s_base64_encode twApi_stub->base64_encode

#endif /* TW_C_SDK_TWSTUBSON_H */
