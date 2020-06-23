/*
 *  Copyright (C) 2017 ThingWorx Inc.
 *
 *  Sample Application of starting a connection to ThingWorx and
 *  binding a single Thing.
 */

#include "twExt.h"

#include "SteamThing.h"

/* Server Defaults */
#define TW_HOST "localhost"
#define TW_APP_KEY "e1d78abf-cfd2-47a6-92b7-37dc6dd34618"
#define DATA_COLLECTION_RATE_MSEC 5000
#define DEFAULT_THING_NAME "SteamSensor"
#define RETRY_COUNT 3

#if defined NO_TLS
	#define TW_PORT 8080
#else
	#define TW_PORT 443
#endif

char* appKey = TW_APP_KEY;

void bindEventHandler(char *entityName, char isBound, void *userdata) {
	/* First NULL says "tell me about all things that are bound */
	if (isBound) TW_LOG(TW_FORCE,"bindEventHandler: Entity %s was Bound", entityName);
	else TW_LOG(TW_FORCE,"bindEventHandler: Entity %s was Unbound", entityName);
}

void authEventHandler(char *credType, void *userdata) {
	/* Callbacks only when we have connected & authenticated */
	if (!credType) return;
	TW_LOG(TW_FORCE,"authEventHandler: Authenticated using %s.  Userdata = 0x%x", credType, userdata);
}

void synchronizeStateHandler(char *entityName, twInfoTable *subscriptionInfo, void *userdata){
	/*
	 * Called after binding to notify your application about what fields are bound on the server.
	 * Will also be called each time bindings on a thing are edited.
	 */
	TW_LOG(TW_FORCE,"synchronizeStateHandler: Entity %s was synchronized with your ThingWorx Server", entityName);
}

/**
 * The app key callback function is called whenever the SDK requires the current app key to authenticate
 * In production, this callback should obtain an app key from a secure source.
 * @param appKeyBuffer a buffer allocated to store a copy of the app key retrieved by this function
 * @param maxLength the size of appKeyBuffer. Do not return a password longer than this length.
 */
void appKeyCallback(char* appKeyBuffer,unsigned int maxLength){
	strncpy(appKeyBuffer,appKey,maxLength);
}

int main( int argc, char** argv ) {
	int err = 0;

	/* Allow the user to override the default values for connection settings
	using command line arguments */
	char * thingName = DEFAULT_THING_NAME;
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

    /* Register Event Handlers (optional) */
	twApi_RegisterOnAuthenticatedCallback(authEventHandler, TW_NO_USER_DATA);
	twApi_RegisterBindEventCallback(thingName, bindEventHandler, TW_NO_USER_DATA);
	twApi_RegisterSynchronizeStateEventCallback(NULL, synchronizeStateHandler, TW_NO_USER_DATA);

	createSteamSensorThing(thingName);

	err = twApi_Connect(CONNECT_TIMEOUT, RETRY_COUNT);
	if(TW_OK != err){
		exit(-1);
	}

	/* Add a path to load edge extensions from at runtime */
	putenv("TWXLIB=../ExtUseExample/ext");

	/* Start All SDK Threads, Does not return. If you want control back call twExt_Start() instead. */
	twExt_Idle(DATA_COLLECTION_RATE_MSEC,  TW_THREADING_MULTI, 5);

}
