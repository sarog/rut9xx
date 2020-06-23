/*
 * Copyright 2016-2018, PTC, Inc.
 * All rights reserved.
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <twInfoTable.h>
#include <cJSON.h>
#include "stringUtils.h"
#include "TestUtilities.h"
#include "crossPlatformTestSupport.h"
#include "twPath.h"

#ifdef _WIN32
#include "openssl/applink.c"
#else
#include <libgen.h>
#endif

#include "unity_fixture.h"
#ifdef USE_GRAFANA
#include "graphite-client.h"
#endif
#ifdef USE_HTML_GRAPHING
#include "chart-js-client.h"
#endif

char * configurationFile;
char * configurationDirectory;

twInfoTable* testConfigInfoTable;

int test_port=443;
char* test_app_key;
char * test_host;
int grafana_port=2003;
char * grafana_host;


char enableUnit = TRUE;
char enableIntegration = FALSE;
char enableLongIntegration = FALSE;
char enablePerformance = FALSE;
char enableAcceptance = FALSE;
char enableDockerInstance = FALSE;
const char * programName;

/*
 * Runs a shell script which starts a docker instance of thingworx on the local box
 */
void startDockerInstance(){
	char outbuffer[5000];
	char shellCommand[255];
	char* dockerScript,*dockerPlatform,*dockerPlatformVersion,*dockerUsername,*dockerPassword;

	dockerScript = duplicateString(configurationDirectory);
#ifdef WIN32
	concatenateStrings(&dockerScript, "\\start-docker-thingworx.bat %s %s %s %s %i");
#else
	concatenateStrings(&dockerScript, "/start-docker-thingworx.sh %s %s %s %s %i");
#endif
	twInfoTable_GetString(testConfigInfoTable,"docker-platform",0,&dockerPlatform);
	twInfoTable_GetString(testConfigInfoTable,"docker-platform-version",0,&dockerPlatformVersion);
	twInfoTable_GetString(testConfigInfoTable,"docker-username",0,&dockerUsername);
	twInfoTable_GetString(testConfigInfoTable,"docker-encrypted-password",0,&dockerPassword);

	sprintf(shellCommand,dockerScript,dockerUsername,dockerPassword,dockerPlatform,dockerPlatformVersion,test_port);
	TW_FREE(dockerScript);

	if(xPlat_DockerInstalled()){
		if(TW_OK==xPlat_executeReturnOutput(shellCommand,outbuffer,5000)){
			TW_LOG(TW_INFO,"Docker Thingworx Instance running.");
		} else {
			TW_LOG(TW_INFO,"Docker install failed. Thingworx instance cannot be started.");
			exit(-6);
		}
	} else {
		TW_LOG(TW_INFO,"Docker is not installed. Thingworx instance cannot be started.");
		exit(-6);
	}
}

void runAcceptanceTests(){

    // Do existing components exists?
    // Make working directory
    // Get the js SDK from git
    // git clone https://wreichardt@bitbucket.org/thingworx-ondemand/thingworx-nodejs-agent.git
    // Get the test project from git
    // git clone https://wreichardt@bitbucket.org/thingworx-ondemand/tw-edge-test.git -b localsdk
    // cd to tw-edge-test
    // npm install

    // run the Regression App as a child process

    // run the Acceptance tests

    // kill the Regression App

}

char * readFile(char * filename, uint32_t * returnedSize) {
    char * buffer = NULL;
    long int string_size = 0;
    FILE *fH = fopen(filename, "r");

    if (fH) {
        // Seek the last byte of the file
        fseek(fH, 0, SEEK_END);
        // Offset from the first to the last byte, or in other words, filesize
        string_size = ftell(fH);
        // go back to the start of the file
        rewind(fH);

        // Allocate a string that can hold it all
        buffer = (char *) TW_MALLOC(sizeof(char) * (string_size + 1));

        // Read it all in one operation
        fread(buffer, sizeof(char), string_size, fH);

        // fread doesn't set it so put a \0 in the last position
        // and buffer is now officially a string
        buffer[string_size] = '\0';

        // Always remember to close the file.
        fclose(fH);
    } else {
        printf("Failed to open %s\n", filename);
    }
    *returnedSize=string_size;
    return buffer;
}

char *tw_strsep(char **stringp, const char *delim) {
	char *token_start;
    if (*stringp == NULL) { return NULL; }
    token_start = *stringp;
    *stringp = strpbrk(token_start, delim);
    if (*stringp) {
        **stringp = '\0';
        (*stringp)++;
    }
    return token_start;
}

void enableTestSet(const char* testSetName){
    if(strcmp(testSetName,"unit")==0) enableUnit = TRUE;
    if(strcmp(testSetName,"integration")==0) enableIntegration = TRUE;
    if(strcmp(testSetName,"slow")==0) enableLongIntegration = TRUE;
    if(strcmp(testSetName,"longintegration")==0) enableLongIntegration = TRUE;
    if(strcmp(testSetName,"integration-long")==0) enableLongIntegration = TRUE;
    if(strcmp(testSetName,"long-integration")==0) enableLongIntegration = TRUE;
    if(strcmp(testSetName,"performance")==0) enablePerformance = TRUE;
    if(strcmp(testSetName,"acceptance")==0) enableAcceptance = TRUE;
	if(strcmp(testSetName,"docker")==0) enableDockerInstance = TRUE;

    if(strcmp(testSetName,"all")==0){
        enableUnit = TRUE;
        enableIntegration = TRUE;
        enableLongIntegration = TRUE;
        enablePerformance = TRUE;
    }
}

static void RunAllTests(void)
{

    if(enableUnit) {
        printf("Including Unit Tests\n");
    }

    if(enableIntegration) {
        printf("Including Integration Tests\n");
    }
    if(enableLongIntegration) {
        printf("Including Long Integration Tests\n");
	}

    if(enablePerformance) {
        printf("Including Performance Tests\n");
    }

	if(enablePerformance||enableLongIntegration||enableIntegration) {
		int keyInstallAttempt = 0;
		printf("Installing Security Key..\n");
		while (FALSE == importBootstrapEntityFileFromEtc("IntegrationTestKey.xml") && keyInstallAttempt < 10) {
			twSleepMsec(10000);
			printf("Key Install Retrying....\n");
		}
		if (keyInstallAttempt < 10){
			printf("Security Key Installation Completed.\n");
		} else {
			printf("Security Key Installation Failed.\n");
			exit(-9);
		}
	}

	if(enableUnit) {
		/* Configure SDK connect delay and storage directories */
		twcfg_pointer->max_connect_delay = UNIT_TEST_SMALL_DELAY;
		twcfg_pointer->connect_retry_interval = UNIT_TEST_SMALL_DELAY;
		twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_LOCATION;
		twcfg_pointer->subscribed_props_dir = SUBSCRIBED_PROPERTY_LOCATION;
		/* crossPlatformTestSupport unit tests */
		RUN_TEST_GROUP(unit_xPlat_DockerInstalled);
		RUN_TEST_GROUP(unit_xPlat_executeReturnOutput);
		RUN_TEST_GROUP(unit_xPlat_NodeInstalled);
		/* crypto_wrapper unit tests */
		RUN_TEST_GROUP(unit_EncryptDES);
		RUN_TEST_GROUP(unit_MD4Hash);
		/* stringUtils unit tests */
		RUN_TEST_GROUP(unit_concatenateStrings);
		RUN_TEST_GROUP(unit_duplicateString);
		RUN_TEST_GROUP(unit_lowercase);
		RUN_TEST_GROUP(unit_uppercase);
		/* twApi unit tests */
		RUN_TEST_GROUP(unit_twApi_RegisterInitCallback);
		RUN_TEST_GROUP(unit_twApi_SetOfflineMsgStoreDir);
		RUN_TEST_GROUP(unit_twApi_Initialize);
		RUN_TEST_GROUP(unit_twApi_Connect);
		RUN_TEST_GROUP(unit_twApi_GetVersion);
		RUN_TEST_GROUP(unit_twApi_SetDutyCycle);
		RUN_TEST_GROUP(unit_twApi_SetSelfSignedOk);
		RUN_TEST_GROUP(unit_twApi_DisableCertValidation);
		RUN_TEST_GROUP(unit_twApi_DisableEncryption);
		RUN_TEST_GROUP(unit_twApi_SetX509Fields);
		RUN_TEST_GROUP(unit_twApi_EnableFipsMode);
		RUN_TEST_GROUP(unit_twApi_LoadCACert);
		RUN_TEST_GROUP(unit_twApi_LoadClientCert);
		RUN_TEST_GROUP(unit_twApi_SetClientKey);
		RUN_TEST_GROUP(unit_twApi_SetPingRate);
		RUN_TEST_GROUP(unit_twApi_SetConnectRetries);
		RUN_TEST_GROUP(unit_twApi_SetGatewayName);
		RUN_TEST_GROUP(unit_twApi_SetGatewayType);
		RUN_TEST_GROUP(unit_twApi_SetProxyInfo);
		RUN_TEST_GROUP(unit_twApi_RegisterConnectCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterCloseCallback);
		RUN_TEST_GROUP(unit_twApi_ConnectionInProgress);
		RUN_TEST_GROUP(unit_twApi_StopConnectionAttempt);
		RUN_TEST_GROUP(unit_twApi_BindThing);
		RUN_TEST_GROUP(unit_twApi_BindThings);
		RUN_TEST_GROUP(unit_twApi_RegisterBindEventCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterSynchronizeStateEventCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterOnAuthenticatedCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterProperty);
		RUN_TEST_GROUP(unit_twApi_AddAspectToProperty);
		RUN_TEST_GROUP(unit_twApi_UpdatePropertyMetaData);
		RUN_TEST_GROUP(unit_twApi_RegisterService);
		RUN_TEST_GROUP(unit_twApi_AddAspectToService);
		RUN_TEST_GROUP(unit_twApi_RegisterEvent);
		RUN_TEST_GROUP(unit_twApi_AddAspectToEvent);
		RUN_TEST_GROUP(unit_twApi_RegisterPropertyCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterServiceCallback);
		RUN_TEST_GROUP(unit_twApi_UnregisterThing);
		RUN_TEST_GROUP(unit_twApi_RegisterDefaultRequestHandler);
		RUN_TEST_GROUP(unit_twApi_ReadProperty);
		RUN_TEST_GROUP(unit_twApi_WriteProperty);
		RUN_TEST_GROUP(unit_twApi_PushProperties);
		RUN_TEST_GROUP(unit_twApi_InvokeService);
		RUN_TEST_GROUP(unit_twApi_FireEvent);
		RUN_TEST_GROUP(unit_twApi_PushSubscribedProperties);
		RUN_TEST_GROUP(unit_twApi_SendPing);
		RUN_TEST_GROUP(unit_twApi_RegisterPingCallback);
		RUN_TEST_GROUP(unit_twApi_RegisterPongCallback);
		RUN_TEST_GROUP(unit_twApi_GetApi);
		RUN_TEST_GROUP(unit_twApi_findCallback);
		RUN_TEST_GROUP(unit_twApi_SetSubscribedProperty);
		RUN_TEST_GROUP(unit_twApi_ZipExtractFile);
		/* twBaseTypes unit tests */
		RUN_TEST_GROUP(unit_twBaseTypes_streamToString);
		RUN_TEST_GROUP(unit_twBaseTypes_stringToStream);
		RUN_TEST_GROUP(unit_twBaseTypes_swapbytes);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_Compare);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_Copy);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_Create);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_IsTrue);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_ToJson);
		RUN_TEST_GROUP(unit_twBaseTypes_twPrimitive_ToStream);
		RUN_TEST_GROUP(unit_twBaseTypes_twStream_Create);
		/* twDataShape unit tests */
		RUN_TEST_GROUP(unit_twDataShape_Create);
		RUN_TEST_GROUP(unit_twDataShapeEntry_CreateFromStream);
		/* twDirectory unit tests */
		RUN_TEST_GROUP(unit_twDictionary_Add);
		RUN_TEST_GROUP(unit_twDictionary_Create);
		RUN_TEST_GROUP(unit_twDirectory_IterateEntires);
		/* twExt unit tests */
		RUN_TEST_GROUP(unit_TW_DECLARE_SHAPE);
		RUN_TEST_GROUP(unit_twExt_AddPropertyChangeListener);
		RUN_TEST_GROUP(unit_twExt_CreateThingFromTemplate);
		RUN_TEST_GROUP(unit_twExt_GetCallbackForService);
		RUN_TEST_GROUP(unit_twExt_LoadExtensionLibrary);
		RUN_TEST_GROUP(unit_twExt_RegisterPolledTemplateFunction);
		RUN_TEST_GROUP(unit_twExt_RegisterStandardProperty);
		RUN_TEST_GROUP(unit_twExt_StandardPropertyHandler);
		/* twFileManager unit tests*/
		RUN_TEST_GROUP(unit_twFileManager_FinishFileTransfer);
		RUN_TEST_GROUP(unit_twFileManager_ListVirtualDirs);
		RUN_TEST_GROUP(unit_twFileManager_TransferFile);
		RUN_TEST_GROUP(unit_twFileManager_ListEntities);
		RUN_TEST_GROUP(unit_twFileManager_ListEntities_2);
		RUN_TEST_GROUP(unit_twFileManager_RegisterFileCallback);
		/* twFileTransferCallbacks unit tests */
		RUN_TEST_GROUP(unit_twFileTransfer_twGetDirectoryStructure);
		RUN_TEST_GROUP(unit_twFileTransferCallbacks_listDirsInInfoTable);
		RUN_TEST_GROUP(unit_twFileTransferCallbacks_twCreateBinaryFile);
		RUN_TEST_GROUP(unit_twFileTransferCallbacks_twDeleteFile);
		RUN_TEST_GROUP(unit_twFileTransferCallbacks_twGetDirectoryStructure_2);
		RUN_TEST_GROUP(unit_twFileTransferCallbacks_twMakeDirectory);
		/* twInfoTable unit tests */
		RUN_TEST_GROUP(unit_TW_IT_ROW);
		RUN_TEST_GROUP(unit_TW_MAKE_IT);
		RUN_TEST_GROUP(unit_twInfoTable_Create);
		RUN_TEST_GROUP(unit_twInfoTable_CreateInfoTableFromRows);
		RUN_TEST_GROUP(unit_twInfoTableRow_CreateFromStream);
		/* twList unit tests */
		RUN_TEST_GROUP(unit_twList_Add);
		RUN_TEST_GROUP(unit_twList_Clear);
		RUN_TEST_GROUP(unit_twList_Create);
		RUN_TEST_GROUP(unit_twList_Find);
		RUN_TEST_GROUP(unit_twList_Foreach);
		RUN_TEST_GROUP(unit_twList_GetByIndex);
		RUN_TEST_GROUP(unit_twList_Remove);
		RUN_TEST_GROUP(unit_twList_ReplaceValue);
		/* twLogger unit tests */
		RUN_TEST_GROUP(unit_levelString);
		RUN_TEST_GROUP(unit_twCharacteristicToString);
		RUN_TEST_GROUP(unit_twCodeToString);
		RUN_TEST_GROUP(unit_twEntityToString);
		RUN_TEST_GROUP(unit_twLog);
		RUN_TEST_GROUP(unit_twLogger_Instance);
		RUN_TEST_GROUP(unit_twLogger_SetFunction);
		RUN_TEST_GROUP(unit_twLogger_SetIsVerbose);
		RUN_TEST_GROUP(unit_twLogger_SetLevel);
		RUN_TEST_GROUP(unit_twLogHexString);
		RUN_TEST_GROUP(unit_twLogMessage);
		/* twMap unit tests */
		RUN_TEST_GROUP(unit_twMap_Add);
		RUN_TEST_GROUP(unit_twMap_Clear);
		RUN_TEST_GROUP(unit_twMap_Create);
		RUN_TEST_GROUP(unit_twMap_Delete);
		RUN_TEST_GROUP(unit_twMap_Find);
		RUN_TEST_GROUP(unit_twMap_ForEach);
		RUN_TEST_GROUP(unit_twMap_get);
		RUN_TEST_GROUP(unit_twMap_GetCount);
		RUN_TEST_GROUP(unit_twMap_length);
		RUN_TEST_GROUP(unit_twMap_put);
		RUN_TEST_GROUP(unit_twMap_Remove);
		RUN_TEST_GROUP(unit_twMap_ReplaceValue);
		/* twMessaging unit tests */
		RUN_TEST_GROUP(unit_twBindBody_CreateFromStream);
		RUN_TEST_GROUP(unit_twMessage_Create);
		RUN_TEST_GROUP(unit_twMessage_CreateFromStream);
		RUN_TEST_GROUP(unit_twMessage_GetRequestId);
		RUN_TEST_GROUP(unit_twMessageHandler_CleanupOldMessages);
		RUN_TEST_GROUP(unit_twMessaging_getPreventIncomingMsgListDump);
		/* twNtlm unit tests */
		RUN_TEST_GROUP(unit_NTLM_connectToProxy);
		RUN_TEST_GROUP(unit_NTLM_parseType2Msg);
		RUN_TEST_GROUP(unit_NTLM_sendType1Msg);
		/* twOfflineMessageStore unit tests */
		RUN_TEST_GROUP(unit_twOfflineMsgStore_getMessageRequestId);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_Flush);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_HandleRequest);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_Initialize);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_SetDir);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_Write);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_getOfflineMsgListCount);
		RUN_TEST_GROUP(unit_twOfflineMsgStore_isOfflineMsgListEnabled);
		/* twPasswds.c unit test */
		RUN_TEST_GROUP(unit_twConvertCallbackToPasswd);
		/* twProperty unit tests */
		RUN_TEST_GROUP(unit_twProperty_Create);
		RUN_TEST_GROUP(unit_twPropertyDef_Create);
		/* twServices unit tests */
		RUN_TEST_GROUP(unit_twService_Create);
		RUN_TEST_GROUP(unit_twServiceDef_Create);
		/* twSig unit tests */
		RUN_TEST_GROUP(unit_decryptSignatureRsaFile);
		RUN_TEST_GROUP(unit_TWSHA256_sha256_file);
		RUN_TEST_GROUP(unit_TWSHA256BinToHex);
		/* twStream unit tests */
		RUN_TEST_GROUP(unit_twStream_AddBytes);
		/* twSubscribedProps unit tests */
		RUN_TEST_GROUP(unit_subscribedPropertyUpdateTaskForEachHandler);
		RUN_TEST_GROUP(unit_twSubscribedPropsMgr_Initialize);
		/* twDictionary unit tests */
		RUN_TEST_GROUP(unit_twDictionary_Create);
		RUN_TEST_GROUP(unit_twDictionary_Add);
		/* twTasker unit tests */
		RUN_TEST_GROUP(unit_twTasker_CreateTask);
		/* twThreadUtils unit test */
		RUN_TEST_GROUP(unit_twThreadUtils_twExt_WaitUntilFirstSynchronization);
		/* twTunnelManager unit tests */
		RUN_TEST_GROUP(unit_twTunnelManager_DisableCertValidation);
		RUN_TEST_GROUP(unit_twTunnelManager_DisableEncryption);
		RUN_TEST_GROUP(unit_twTunnelManager_ListActiveTunnels);
		RUN_TEST_GROUP(unit_twTunnelManager_LoadCACert);
		RUN_TEST_GROUP(unit_twTunnelManager_LoadClientCert);
		RUN_TEST_GROUP(unit_twTunnelManager_RegisterTunnelCallback);
		RUN_TEST_GROUP(unit_twTunnelManager_SetClientKey);
		RUN_TEST_GROUP(unit_twTunnelManager_SetProxyInfo);
		RUN_TEST_GROUP(unit_twTunnelManager_SetSelfSignedOk);
		RUN_TEST_GROUP(unit_twTunnelManager_SetX509Fields);
		RUN_TEST_GROUP(unit_twTunnelManager_UpdateTunnelServerInfo);
		/* twWebsocket unit tests */
		RUN_TEST_GROUP(unit_twCompressBytes);
		RUN_TEST_GROUP(unit_twWs_Connect);
		RUN_TEST_GROUP(unit_twWs_Disconnect);
		RUN_TEST_GROUP(unit_twWs_Receive);
		RUN_TEST_GROUP(unit_twWs_SendMessage);
    }

    if(enableIntegration) {
	    /* Configure SDK connect delay and storage directories */
	    twcfg_pointer->max_connect_delay = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->connect_retry_interval = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_LOCATION;
	    twcfg_pointer->subscribed_props_dir = SUBSCRIBED_PROPERTY_LOCATION;
		RUN_TEST_GROUP(SteamSensorIntegration);
		RUN_TEST_GROUP(ExtUseExampleIntegration);
        RUN_TEST_GROUP(FileTransferIntegration);
        RUN_TEST_GROUP(FileSystemServicesIntegration);
        RUN_TEST_GROUP(ServiceIntegration);
        RUN_TEST_GROUP(SubscribedPropertyIntegrationGeneric);
        RUN_TEST_GROUP(SubscribedPropertyIntegrationUnique);
        RUN_TEST_GROUP(TestUtilitiesIntegration);
        RUN_TEST_GROUP(OfflineMsgStoreIntegration);
		RUN_TEST_GROUP(ApiIntegration);
		RUN_TEST_GROUP(MetadataIntegration)
		RUN_TEST_GROUP(SynchronizeStateIntegration);
		RUN_TEST_GROUP(TunnelingIntegration);
		RUN_TEST_GROUP(BindingIntegration);
		RUN_TEST_GROUP(PropertyWriteIntegration);
	    RUN_TEST_GROUP(FipsIntegration);
	    RUN_TEST_GROUP(CertificateIntegration);
#ifndef TW_RELEASE_BUILD
	    RUN_TEST_GROUP(RegressionIntegration);
#endif
    }

    if(enableLongIntegration) {
	    /* Configure SDK connect delay and storage directories */
	    twcfg_pointer->max_connect_delay = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->connect_retry_interval = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_LOCATION;
	    twcfg_pointer->subscribed_props_dir = SUBSCRIBED_PROPERTY_LOCATION;
        RUN_TEST_GROUP(BindingIntegrationSlow);
        RUN_TEST_GROUP(OfflineMsgStoreIntegrationSlow);
    }

    if(enablePerformance) {
	    /* Configure SDK connect delay and storage directories */
	    twcfg_pointer->max_connect_delay = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->connect_retry_interval = UNIT_TEST_SMALL_DELAY;
	    twcfg_pointer->offline_msg_store_dir = OFFLINE_MSG_STORE_LOCATION;
	    twcfg_pointer->subscribed_props_dir = SUBSCRIBED_PROPERTY_LOCATION;
        RUN_TEST_GROUP(KepwarePerformance); /* Measures execution time */
        RUN_TEST_GROUP(BindingPerformance); /* Measures execution time */
		RUN_TEST_GROUP(ServiceExecutionPerformance); /* Measures execution time */
        RUN_TEST_GROUP(ForeachPerformance); /* Measures execution time */
        RUN_TEST_GROUP(PropertyPerformance);
        RUN_TEST_GROUP(FileTransferPerformance);
    }
}

int main(int argc, const char * argv[]) {
    char* theStreamData;
	char *str = NULL;
    char *token = NULL;
    char * str_tok = NULL;
    cJSON *contentJson = NULL;
    uint32_t length;
    int ret;
	programName = twPath_GetFullPath(argv[0]);

	test_app_key = duplicateString("badbadba-dbad-badb-adba-badbadbadbad");
	test_host = duplicateString("localhost");
	grafana_host = duplicateString("localhost");

    /* Determine if stubs are available, if not, exit */
#ifndef TW_STUBS
	printf("Tests currently will not run against a release build unless TW_STUBS is defined.\nIf building with cmake set cmake option -DBUILD_DEBUG=ON.");
    exit(0);
#endif
	/* Initialize stubs */
	twApi_CreateStubs();
    if(argc>1){
        enableUnit = FALSE;
		str = duplicateString(argv[1]);
        /* iterate over the str_tok, so we do not lose context of the
        original str memory */
        str_tok = str;
        while ((token = tw_strsep(&str_tok, ","))) enableTestSet(token);
        TW_FREE(str);
    }
    if(argc>2){
        configurationFile = duplicateString(argv[2]);
#ifdef _WIN32
		{
		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];
		char fullPath[_MAX_PATH];
		_splitpath(configurationFile, drive, dir, fname, ext);
		sprintf(fullPath,"%s%s",drive,dir);
		configurationDirectory = duplicateString(fullPath);
		}
#else
		/* dirname in linux will strip the file from the
		original pointer, so we first need to duplicate
		the pointer, then strip the file */
		configurationDirectory = duplicateString(configurationFile);
		configurationDirectory = dirname(configurationDirectory);
#endif
        theStreamData = readFile(configurationFile,&length);
        contentJson = cJSON_Parse(theStreamData);
        TW_FREE(theStreamData);
        if(contentJson) {
			char * test_param = NULL;
            printf("Parsing test config JSON\n");
            testConfigInfoTable = twInfoTable_CreateFromJson(contentJson, NULL);
			/* set test port */
            twInfoTable_GetInteger(testConfigInfoTable,"port",0,&test_port);

			/* get test host config value, overwrite global default param if not NULL */
            twInfoTable_GetString(testConfigInfoTable,"host",0,&test_param);
			if(test_param) {
				TW_FREE(test_host);
				test_host = test_param;
				test_param = NULL;
			}

			/* get app key config value, overwrite global default param if not NULL */
			twInfoTable_GetString(testConfigInfoTable,"appKey",0,&test_param);
			if(test_param) {
				TW_FREE(test_app_key);
				test_app_key = test_param;
				test_param = NULL;
			}

			/* get grafana host config value, overwrite global default param if not NULL */
            twInfoTable_GetString(testConfigInfoTable,"grafanaHost",0,&test_param);
			if(test_param) {
				TW_FREE(grafana_host);
				grafana_host = test_param;
				test_param = NULL;
			}

			/* set grafana port */
            twInfoTable_GetInteger(testConfigInfoTable,"grafanaPort",0,&grafana_port);
        } else {
            printf("Failed to parse config file.");
            TW_FREE((void *) programName);
            exit(-1);
        }
    }
#ifdef USE_GRAFANA
    graphite_init(grafana_host, grafana_port);
#endif
	if(enableDockerInstance){
		startDockerInstance();
	}

    ret=UnityMain(argc, argv, RunAllTests);

    if(enableAcceptance){
        runAcceptanceTests();
    }

#ifdef USE_GRAFANA
    graphite_finalize();
#endif
#ifdef USE_HTML_GRAPHING
    chartjs_write_file();
#endif

    /* cleanup memory related to configuration  */
	if (configurationDirectory) TW_FREE(configurationDirectory);
	if (configurationFile) TW_FREE(configurationFile);
	if (testConfigInfoTable) twInfoTable_Delete(testConfigInfoTable);
    if (contentJson) cJSON_Delete(contentJson);
	if (test_app_key ) TW_FREE(test_app_key);
	if (test_host) TW_FREE(test_host);
	if (grafana_host) TW_FREE(grafana_host);
	twApi_DeleteStubs();
    TW_FREE((void *) programName);

    return ret;
}
