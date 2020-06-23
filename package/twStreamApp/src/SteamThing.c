/*
 *  Copyright (C) 2017 ThingWorx Inc.
 *
 */

#include "twExt.h"
#include "twMacros.h"

static int getCommandOutput(char *buffer, size_t len, char *cmd);

void onSteamSensorProcessScanRequest(char *thingName) {
	TW_LOG(TW_TRACE,"dataCollectionTask: Executing");

	size_t bsize = sizeof(char) * 64;  
	char *buffer;

	if ((buffer = malloc(bsize)) == NULL) {
		printf("error: malloc fail!\r\n");
		return;
	}

	if (getCommandOutput(buffer, bsize, "ip r l 0/0 | head -1 | cut -d' ' -f5") == 0) {
		TW_SET_PROPERTY(thingName, "WAN_Type", TW_MAKE_STRING(buffer));
	}

	memset(buffer, 0, bsize);

	if (getCommandOutput(buffer, bsize, "ip r l 0/0 | head -1 | cut -d' ' -f3") == 0) {
		TW_SET_PROPERTY(thingName, "WAN_IP", TW_MAKE_STRING(buffer));
	}

	memset(buffer, 0, bsize);

	if (getCommandOutput(buffer, bsize, "gsmctl -o") == 0) {
		TW_SET_PROPERTY(thingName, "MobileOperator", TW_MAKE_STRING(buffer));
	}

	memset(buffer, 0, bsize);

	if (getCommandOutput(buffer, bsize, "gsmctl -q") == 0) {
		TW_SET_PROPERTY(thingName, "MobileSignal", TW_MAKE_STRING(buffer));
	}

	memset(buffer, 0, bsize);

	if (getCommandOutput(buffer, bsize, "gsmctl -t") == 0) {
		TW_SET_PROPERTY(thingName, "MobileNetwork", TW_MAKE_STRING(buffer));
	}

	free(buffer);

	// Update the properties on the server
	TW_PUSH_PROPERTIES_FOR(thingName, TW_PUSH_CONNECT_FORCE);
}

/**
 * This function defines a thing. Multiple instances of this thing can be 
 * declared by changing the thingName parameter.
 */
void createSteamSensorThing(char *thingName) {
    // Declare Properties
    TW_MAKE_THING(thingName, TW_THING_TEMPLATE_GENERIC);

    TW_PROPERTY("WAN_Type", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("WAN_Type", TW_ASPECT_ISREADONLY, TRUE);
		TW_ADD_BOOLEAN_ASPECT("WAN_Type", TW_ASPECT_ISLOGGED, TRUE);

    TW_PROPERTY("MobileOperator", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("MobileOperator", TW_ASPECT_ISREADONLY, TRUE);
		TW_ADD_BOOLEAN_ASPECT("MobileOperator", TW_ASPECT_ISLOGGED, TRUE);

    TW_PROPERTY("MobileSignal", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("MobileSignal", TW_ASPECT_ISREADONLY, TRUE);
		TW_ADD_BOOLEAN_ASPECT("MobileSignal", TW_ASPECT_ISLOGGED, TRUE);

    TW_PROPERTY("MobileNetwork", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("MobileNetwork", TW_ASPECT_ISREADONLY, TRUE);
		TW_ADD_BOOLEAN_ASPECT("MobileNetwork", TW_ASPECT_ISLOGGED, TRUE);

    TW_PROPERTY("WAN_IP", TW_NO_DESCRIPTION, TW_STRING);
		TW_ADD_BOOLEAN_ASPECT("WAN_IP", TW_ASPECT_ISREADONLY, TRUE);
		TW_ADD_BOOLEAN_ASPECT("WAN_IP", TW_ASPECT_ISLOGGED, TRUE);

    // Register a function to be called periodically after this thing has been 
    // created
    twExt_RegisterPolledTemplateFunction(
        onSteamSensorProcessScanRequest, TW_THING_TEMPLATE_GENERIC);

    TW_BIND();
}

void destroySteamSensorThing(char *thingName) {
	twApi_UnbindThing(thingName);
	twExt_RemovePolledFunction(onSteamSensorProcessScanRequest);
}

static int getCommandOutput(char *buffer, size_t len, char *cmd) {
	FILE *fp;
	size_t ifsize = sizeof(char) * 16;
	char *ifname = malloc(ifsize);
	int status = -1;

	if (buffer == NULL) {
		printf("error: buffer is null!\r\n");
		return status;
	}

	if (cmd == NULL) {
		printf("error: cmd is null!\r\n");
		return status;
	}

	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("error: popen fail!\r\n");
		return status;
	}

	if (fgets(ifname, ifsize, fp) != NULL) {
		if (strstr(ifname, "") != NULL) {
			snprintf(buffer, len, "%s", ifname);
			status = 0;
		}
	}

	pclose(fp);
	free(ifname);
	return status;
}
