/*
 *  Copyright 2017, PTC, Inc.
 *
 *  HTTP Proxy Connection
 */

#include "twHttpProxy.h"
#include "twDefaultSettings.h"
#include "stringUtils.h"
#include "twList.h"
#include "twLogger.h"
#include "twOSPort.h"
#include "tomcrypt.h"
#include "twApiStubs.h"

#include <ntlm.h>

#define PROXY_TIMEOUT 30000
#define MAX_REQ_LEN 1024
#define MAX_RESP_LEN 1024

int32_t NTLM_sendType1Msg(twSocket * sock, const char * req, char * domain, char * user, char * password) {
	/* Create our NTLM request */
	char * type1Msg = NULL;
	uint32_t type1MsgLen;
	char * encodedAuth = NULL;
	unsigned long encodedAuthLength = 0;
	char * fullRequest = NULL;
	int32_t bytesSent = 0;

	if (!sock || !req || !user || !password) {
		TW_LOG(TW_ERROR,"NTLM_sendType1Msg: NTLM Handshake Requested but no Username or Password is specified");
		return -1;
	}
	s_GenerateType1Msg(&type1Msg, &type1MsgLen);
	if (!type1Msg) {
		TW_LOG(TW_ERROR,"NTLM_sendType1Msg: Error creating Type1 Message");
		return -1;
	}
	TW_LOG(TW_DEBUG, "NTLM_sendType1Msg: Created NTLM Request");
	TW_LOG_HEX(type1Msg, "NTLM: Type 1 Bytes >>>", type1MsgLen);

	/* Base64 encode the type1 message */
	encodedAuthLength = type1MsgLen * 2;
	encodedAuth = (char *)TW_CALLOC(encodedAuthLength, 1);
	if (!encodedAuth) {
		TW_LOG(TW_ERROR,"NTLM_sendType1Msg: Error allocating memory for NTLM authentication");
		TW_FREE(type1Msg);
		return -1;
	}
	TW_LOG(TW_INFO,"NTLM_sendType1Msg: Using NTLM authentication");
	if (s_base64_encode((const unsigned char *)type1Msg, type1MsgLen, (unsigned char *)encodedAuth, &encodedAuthLength)) {
		TW_FREE(encodedAuth);
		TW_FREE(type1Msg);
		return -1;
	}
	fullRequest = (char *)TW_CALLOC(strnlen(req, MAX_REQ_LEN) + encodedAuthLength + 32, 1);
	if (!fullRequest) {
		TW_LOG(TW_ERROR,"NTLM_sendType1Msg: Error allocating memory for NTLM request");
		TW_FREE(encodedAuth);
		TW_FREE(type1Msg);
		return -1;
	}
	/* need to get rid of the last the "\r\n" in the orginal request */
	strncpy(fullRequest, req, strnlen(req, MAX_REQ_LEN) - 2);
	strcat(fullRequest, "Proxy-Authorization: NTLM ");
	strcat(fullRequest, encodedAuth);
	strcat(fullRequest, "\r\n\r\n");
	TW_LOG(TW_DEBUG, "NTLM_sendType1Msg: NTLM Authorization - Type 1 Msg: %s",	fullRequest);
	bytesSent = s_twSocket_Write(sock, fullRequest, strnlen(fullRequest, MAX_REQ_LEN), PROXY_TIMEOUT);
	if (bytesSent != strnlen(fullRequest, MAX_REQ_LEN)) {
		TW_LOG(TW_ERROR, "NTLM_sendType1Msg: Failed sending NTLM Type1 Message - Closing Connection");
		twSocket_Close(sock);
		TW_FREE(fullRequest);
		TW_FREE(encodedAuth);
		TW_FREE(type1Msg);
		return -1;
	}
	TW_FREE(fullRequest);
	TW_FREE(encodedAuth);
	TW_FREE(type1Msg);
	return 0;
}

int32_t NTLM_parseType2Msg(twSocket * sock, const char * req, char * resp, char * domain, char * username, char * password) {

	/* Decode the message to give us the challenge */
	char * challengeString = NULL;
	unsigned long challengeStringLength = 0;
	char * outBuf = NULL;
	uint32_t outLen = 0;
	char * encodedAuth = NULL;
	unsigned long encodedAuthLength = 0;
	char * fullRequest = NULL;
	int32_t bytesSent = 0;
	size_t fullRequestLen = 0;
	
	if (!sock || !resp || !username || !password) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Invalid parameter");
		return -1;
	}
	challengeStringLength = strnlen(resp, MAX_RESP_LEN);
	challengeString = (char *)TW_CALLOC(challengeStringLength, 1);
	if (!challengeString) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Error allocating memory");
		return -1;
	}
	if (base64_decode((unsigned char *)resp, strnlen(resp, MAX_RESP_LEN), (unsigned char *)challengeString, &challengeStringLength)) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Error Base64 decoding challenge string");
		TW_FREE(challengeString);
		return -1;
	}
	TW_LOG(TW_DEBUG, "NTLM_parseType2Msg: NTLM Authorization - Type 2 Msg: %s", resp);

	/* Create the type 3 message */
	s_GenerateType3Msg(domain, username, password, challengeString, challengeStringLength, &outBuf, &outLen);
	if (!outBuf) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Error creating Type 3 Msg");
		TW_FREE(challengeString);
		return -1;
	}

	/* Prepare our next HTTP request */
	encodedAuth = (char *)TW_CALLOC(outLen *  2,1);
	if (!encodedAuth) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Error allocating memory for encoded auth");
		TW_FREE(challengeString);
		TW_FREE(outBuf);
		return -1;
	}
	encodedAuthLength = outLen * 2;
	if (s_base64_encode((unsigned char *)outBuf, outLen, (unsigned char *)encodedAuth, &encodedAuthLength)) {
		TW_FREE(challengeString);
		TW_FREE(encodedAuth);
		TW_FREE(outBuf);
		return -1;
	}
	fullRequest = (char *)TW_CALLOC(strnlen(req, MAX_REQ_LEN) + encodedAuthLength + 32, 1);
	if (!fullRequest) {
		TW_LOG(TW_ERROR,"NTLM_parseType2Msg: Error allocating memory for NTLM Type3 request");
		TW_FREE(challengeString);
		TW_FREE(encodedAuth);
		TW_FREE(outBuf);
		return -1;
	}
	
	fullRequestLen = strnlen(req, MAX_REQ_LEN) + encodedAuthLength + 32;
	
	/* Remove the trailing \r\n */
	strncpy(fullRequest, req, strnlen(req, MAX_REQ_LEN) - 2);
	strcat(fullRequest, "Proxy-Authorization: NTLM ");
	strcat(fullRequest, encodedAuth);
	strcat(fullRequest, "\r\n\r\n");
	TW_LOG(TW_DEBUG, "NTLM_parseType2Msg: NTLM Authorization - Type 3 Msg:\n%s", fullRequest);	

	/* CLean up a little now */
	TW_FREE(challengeString);
	TW_FREE(encodedAuth);
	TW_FREE(outBuf);

	/* Send the Type 3 Message */
	bytesSent = s_twSocket_Write(sock, fullRequest, strnlen(fullRequest, fullRequestLen - 1), PROXY_TIMEOUT);
	if (bytesSent != strnlen(fullRequest, fullRequestLen - 1)) {
		TW_LOG(TW_ERROR, "NTLM_parseType2Msg: Failed sending NTLM Type2 Message - Closing Connection");
		twSocket_Close(sock);
		TW_FREE(fullRequest);
		return -1;
	}
	TW_FREE(fullRequest);
	return 0;
}

#define BUFFER_SIZE 1024
int NTLM_connectToProxy(twSocket * sock, const char * req, const char * resp, char * user, char * password) {
	/*  NTLM Authentication */
	char * proxydomain = NULL;
	char * proxyuser = NULL;
	int32_t bytesRead = 0;
	char * buffer = NULL;
	char * authHeader = NULL;
	char * response = NULL;
	if (!sock || !req || !resp || !user || !password) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: NULL parameter passed in");
		return -1;
	}
	/* Need to extract a domain from the user name if it exists */
	proxydomain = duplicateString(user);
	if (!proxydomain) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Error allocating memory to parse domain");
		return -1;
	}
	proxyuser = strstr(proxydomain, "\\\\");
	if (!proxyuser) {
		/* Give people a break and check for just "\\" */
		proxyuser = strstr(proxydomain, "\\");
		if (!proxyuser) {
			/* No domain specified, just use user */
			TW_FREE(proxydomain);
			proxydomain = NULL;
			proxyuser = user;
		} else {
			/* Null terminate the domain */
			proxyuser[0] = 0;
			/* Now set the user pointer past the first '\' */
			proxyuser = proxyuser + 1;
		}
	} else {
		/* Null terminate the domain */
		proxyuser[0] = 0;
		/* Now set the user pointer past the second '\' */
		proxyuser = proxyuser + 2;
	}
	TW_LOG(TW_TRACE,"NTLM_NTLM_connectToProxy: Original User String: %s, Domain: %s, User: %s", user, proxydomain ? proxydomain : "NONE", proxyuser);
	/* Create a buffer to handle the next response */
	buffer = (char *)TW_CALLOC(BUFFER_SIZE,1);
	if (!buffer) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Error allocating buffer for reading");
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Drain any data left over from the previous response */
	while ((bytesRead = s_twSocket_Read(sock, buffer, BUFFER_SIZE - 1, 500)) > 0) {
		
	} 
	/* If we have an error, something is wrong with the socket */
	if (bytesRead < 0) {
		TW_LOG(TW_ERROR, "NTLM_connectToProxy: Error draining the socket");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Prepare and send the Type 1 Message */
	if (s_NTLM_sendType1Msg(sock, req, proxydomain, proxyuser, password)) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Error sending NTLM Type 1 Message");
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Get the response */
	TW_LOG(TW_TRACE,"NTLM_NTLM_connectToProxy: Waiting for response to Type 1 Message");
	if (!s_twSocket_WaitFor(sock, PROXY_TIMEOUT)) {
		TW_LOG(TW_ERROR, "NTLM_connectToProxy: Timeout waitng for Type 2 Message");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	bytesRead = s_twSocket_Read(sock, buffer, BUFFER_SIZE - 1, PROXY_TIMEOUT);
	TW_LOG(TW_TRACE,"NTLM_NTLM_connectToProxy: Got %d bytes in response to Type 1 Message", bytesRead);
	TW_LOG(TW_TRACE,"NTLM_connectToProxy: Got response to Type 1 Msg:\n%s", buffer);
	/*  Need to at least have the response code */
	if (bytesRead < 12) {
		TW_LOG(TW_ERROR, "NTLM_connectToProxy: Response to Type1 Msg too small");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Response should start with "HTTP/1.x XXX" */
	authHeader = strstr(buffer,"Authenticate");
	if (!authHeader || authHeader > buffer + BUFFER_SIZE - 13 - 4) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Could not find Authentication mechanism in Type 2 Message Response");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Get the Authenticate header isolated into a string */
	authHeader += 13;
	/* Strip the NTLM */
	authHeader = strstr(authHeader, "NTLM");
	if (!authHeader) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Could not find Authentication mechanism in Type 2 Message Response");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	response = authHeader + 4;
	authHeader = strstr(response, "\r\n");
	if (authHeader) authHeader[0] = 0;
	/* Last thing - strip off any leading whitespace */
	while (response[0] == ' ' || response[0] == '\t') response++;
	/* Make a copy so we can drain the buffer */
	response = s_duplicateString(response);
	if (!response) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Error copying the NTLM header");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	/* Drain any data left over from the previous response */
	while ((bytesRead = s_twSocket_Read(sock, buffer, BUFFER_SIZE - 1, 500)) > 0) {
		
	} 
	/* If we have an error, something is wrong with the socket */
	if (bytesRead < 0) {
		TW_LOG(TW_ERROR, "NTLM_connectToProxy: Error draining the socket");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		if (response) TW_FREE(response);
		return -1;
	}
	/* Parse the Type 2 message (also sends the Type 3 response) */
	if (s_NTLM_parseType2Msg(sock, req, response, proxydomain, proxyuser, password)) {
		TW_LOG(TW_ERROR,"NTLM_connectToProxy: Error parsing Type 2 Message");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	if (response) TW_FREE(response);
	/* Drain the socket looking for an HTTP response */
	TW_LOG(TW_TRACE,"NTLM_NTLM_connectToProxy: Waiting for response to Type 3 Message");
	if (s_twSocket_WaitFor(sock, PROXY_TIMEOUT)) {
		bytesRead = s_twSocket_Read(sock, buffer, BUFFER_SIZE - 1, PROXY_TIMEOUT);
		TW_LOG(TW_TRACE,"NTLM_NTLM_connectToProxy: Got %d bytes in response to Type 3 Message");
		TW_LOG(TW_TRACE,"NTLM_connectToProxy: Got response to Type 3 Msg:\n%s", buffer);
		if (bytesRead < 12) {
			TW_LOG(TW_ERROR,"NTLM_connectToProxy: Response from Type 3 message too small");
			if (buffer) TW_FREE(buffer);
			if (proxydomain) TW_FREE(proxydomain);
			return -1;
		}
		TW_LOG(TW_TRACE, "NTLM_connectToProxy: Got response\n%s", buffer);
		response = strstr(buffer, "HTTP");
		if (!response) {
			TW_LOG(TW_ERROR,"NTLM_connectToProxy: Response from Type 3  message has no 'HTTP'");
			if (buffer) TW_FREE(buffer);
			if (proxydomain) TW_FREE(proxydomain);
			return -1;
		}
		response = buffer + 9;
		buffer[12] = 0;
		if (strcmp(response,"200")) {
			TW_LOG(TW_WARN, "NTLM_connectToProxy: NTLM Authentication Failed");
			if (buffer) TW_FREE(buffer);
			if (proxydomain) TW_FREE(proxydomain);
			return -1;
		}
	} else {
		TW_LOG(TW_WARN, "NTLM_connectToProxy: NTLM Authentication Failed - Timed out waiting for Proxy Response");
		if (buffer) TW_FREE(buffer);
		if (proxydomain) TW_FREE(proxydomain);
		return -1;
	}
	if (buffer) TW_FREE(buffer);
	if (proxydomain) TW_FREE(proxydomain);
	return 0;
}