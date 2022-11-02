/*
 *  Copyright (C) 2017 ThingWorx Inc.
 *
 *  Test application
 */

#include "twExt.h"

/* Defaults */
#define TW_HOST "localhost"
#define TW_APP_KEY "e1d78abf-cfd2-47a6-92b7-37dc6dd34618"
#define DATA_COLLECTION_RATE_MSEC 2000
#if defined NO_TLS
	#define TW_PORT 8080
#else
	#define TW_PORT 443
#endif
char* appKey = TW_APP_KEY;

/**
 * The app key callback function is called whenever the SDK requires the current app key to authenticate
 * In production, this callback should obtain an app key from a secure source.
 * @param appKeyBuffer a buffer allocated and provided to cop
 * @param maxLength the size of appKeyBuffer. Do not return a password longer than this length.
 */
void appKeyCallback(char* appKeyBuffer,unsigned int maxLength){
	strncpy(appKeyBuffer,appKey,maxLength);
}

int main( int argc, char** argv ) {
	int err = 0;

	/*
	 * Allow the user to override the default values for connection settings
	 * using command line arguments
	 */
	char * thingName = "ExtUseExample";
	char* hostname = TW_HOST;
	int16_t port = TW_PORT;

	/* Parse command line parameters */
	if(argc == 1){
		printf("Syntax: %s <hostname> <port> <appKey> <thingname>\n",argv[0]);
		exit(0);
	}
	if(argc > 1){
		hostname = argv[1];
	}
	if(argc > 2){
		port = (short)atoi(argv[2]);
	}
	if(argc > 3){
		appKey = argv[3];
	}
	if(argc > 4){
		thingName = argv[4];
	}

	/* Configure Logging */
	twLogger_SetLevel(TW_TRACE);
	twLogger_SetIsVerbose(TRUE); /* Will show all messages network at TRACE level, reduces performance */
	TW_LOG(TW_FORCE, "Starting up...");

	/* Initialize the API */
	err = twApi_Initialize(hostname, port, TW_URI, appKeyCallback, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
	if (TW_OK != err) {
		TW_LOG(TW_ERROR, "Error initializing the API");
		exit(err);
	}

	/* Allow self signed or unvalidated certs (Neither should be used in production) */
	/* twApi_SetSelfSignedOk(); */ /* Requires a certificate that is self signed */
	twApi_DisableCertValidation(); /* The server certificate will not be validated at all. */

	/* Disable HTTPS support, connect using HTTP only */
	if(80 == port||8080 == port){
		twApi_DisableEncryption();
	}

    /* Dynamically load shape library and add the shape to a Generic Edge Thing */
#ifdef WIN32
	putenv("TWXLIB=../ext/");
#else
	putenv("TWXLIB=./ext/");
#endif
	twExt_LoadExtensionLibrary("libsimpleext");
	twExt_LoadExtensionLibrary("libwarehouseext");
	twExt_CreateThingFromTemplate(thingName, "WarehouseTemplate", "SimpleShape", "AddressShape","InventoryShape",NULL);
    twApi_BindThing(thingName);

	/* Connect to server */
	err = twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES);
	if (err) {
		TW_LOG(TW_ERROR, "Failed to connect.");
		exit(-2);
	}

	/* Choose your threading model */
	twExt_Idle(DATA_COLLECTION_RATE_MSEC, TW_THREADING_SINGLE, 0);
}
