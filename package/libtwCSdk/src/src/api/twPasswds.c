/*
 *  Copyright 2018, PTC, Inc.
 */

#include "twPasswds.h"
#include "twApi.h"

char* twConvertCallbackToPasswd(twPasswdCallbackFunction callback){
	char* buffer = NULL;
	char* callbackTest = (char*)callback;
	buffer = TW_CALLOC(TW_MAX_PASSWORD_LENGTH,sizeof(char));
	if(NULL==buffer) {
		TW_LOG(TW_ERROR,"twConvertCallbackToPasswd - Failed to allocate memory to get app key.");
		return buffer;
	}

	if(NULL==callback) {
		TW_LOG(TW_ERROR,"twConvertCallbackToPasswd - password callback function was null.");
		return buffer;
	}

	if(callbackTest[36]==0&&callbackTest[8]=='-'&&callbackTest[13]=='-'&callbackTest[18]=='-'&callbackTest[23]=='-'){
		TW_LOG(TW_ERROR,"twConvertCallbackToPasswd - If you have received this error you are attempting to use a "
				  "password instead of a password callback. As of C SDK 2.1.3 you must provide a callback "
				  "handler in place of any password including an app key, proxy or digest password. Please see the "
	  "documentation for more information.");
		return buffer;
	}

	callback(buffer,TW_MAX_PASSWORD_LENGTH);
	return buffer;
}

void twFreePasswd(char* twPasswd){
	if(NULL == twPasswd)
		return;
	memset(twPasswd,0,TW_MAX_PASSWORD_LENGTH);
	TW_FREE(twPasswd);
}
