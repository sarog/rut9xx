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
#include "twNtlm.h"
#include "twTls.h"

#define MAX_TOKEN_LEN 1024
#define MAX_RESPONSE_LEN 4096
#define REQ_SIZE 1024
char * createDigestCredentials(twSocket * s, char * hdr) {
	/*****
	Example
	Host Header
	WWW-Authenticate: Digest realm="testrealm@host.com",
	qop="auth,auth-int",
	nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
	opaque="5ccc069c403ebaf9f0171e9517f40e41"

	Client Header
	Authorization: Digest username="Mufasa",
	realm="testrealm@host.com",
	nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093",
	uri="/dir/index.html",
	qop=auth,
	nc=00000001,
	cnonce="0a4f113b",
	response="6629fae49393a05397450978507c4ef1",
	opaque="5ccc069c403ebaf9f0171e9517f40e41"

	*****/
	char * realm = 0;
	char * nonce = 0;
	char * qop = 0;
	char * opaque = 0;
	char cnonce[9];
	char * response = 0;
	char * token = 0;
	/******
	md5_byte_t mdHA1[16];
	md5_byte_t mdHA2[16];
	md5_byte_t mdHA3[16];
	struct md5_state_s md5;
	*/
	TW_MD5_CTX md5;
	unsigned char mdHA1[16];
	unsigned char mdHA2[16];
	unsigned char mdHA3[16];
	char mdHA1String[33];
	char mdHA2String[33];
	char mdHA3String[33];
	int i = 0;
	char * hdrCopy = duplicateString(hdr);
	char * hdrPtr = hdrCopy;

	/* Initialize things */
	memset(cnonce, 0, 9);
	memset(mdHA1, 0, 16);
	memset(mdHA2, 0, 16);
	memset(mdHA3, 0, 16);
	memset(&md5, 0, sizeof(TW_MD5_CTX));
	memset(mdHA1String, 0, 33);
	memset(mdHA2String, 0, 33);
	memset(mdHA3String, 0, 33);
	response = (char *) TW_CALLOC(REQ_SIZE,1);

	/* Sanity check  */
	if (!response || !s || !hdrCopy || strnlen(hdrCopy, REQ_SIZE) <= strnlen("Digest\r\n", 10)) {
		TW_LOG(TW_ERROR,"createDigestCredentials: Initializaiton error");
		return NULL;
	}
	/* move past the digest word and any whirespace*/
	hdrPtr += 7;
	while (*hdrPtr == ' ' || *hdrPtr == '\t') hdrPtr++;
	TW_LOG(TW_TRACE,"createDigestCredentials: \nAuth Request Header:\n%s\n", hdrPtr);
	/* tokenize the remainder of the header by commas */
	token = strtok(hdrPtr, ",");
    while (token != NULL) {
		char * lastQuote = 0;
		char * tmp = strchr(token,'=');
		if (strnlen(token, MAX_TOKEN_LEN) < 1 || !tmp) {
			token = strtok(NULL, "=");
			continue;
		}
		*tmp = 0;
		tmp++;
		/* Skip the leading whitespace */
		while (token[0] == ' ' || token[0] == '\t') token++;
		/* Make sure to skip the leading and trailing " */
		if (*tmp == '\"') tmp++;
		lastQuote = strrchr(tmp,'\"');
		if (lastQuote) *lastQuote = 0;
		if (strcmp(token, "realm") == 0) {
			realm = duplicateString(tmp);
		} else if (strstr(token, "qop") != 0) {
			qop = duplicateString(tmp);
		} else if (strcmp(token, "nonce") == 0) {
			nonce = duplicateString(tmp);
		} else if (strcmp(token, "opaque") == 0) {
			opaque = duplicateString(tmp);
		} 
		token = strtok(NULL, ",");
	}
	if (!realm || !nonce) {
		TW_LOG(TW_ERROR, "createDigestCredentials: No realm or nonce found in %s", hdr);
		TW_FREE(hdrCopy);
		if (qop) TW_FREE(qop);
		if (nonce) TW_FREE(nonce);
		if (opaque) TW_FREE(opaque);
		return NULL;
	}
	/* Create the cnonce */
	/* only use cnonce if qop is specified */
	if (qop) {
		int now = twGetSystemMillisecondCount();
		snprintf(cnonce, 9, "%08x", now);
	}

	/* Create HA1 */
	twMD5_Init(&md5);
	twMD5_Update(&md5, (unsigned char *)s->proxyUser, strnlen(s->proxyUser, MAX_TOKEN_LEN));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	twMD5_Update(&md5, (unsigned char *)realm, strnlen(realm, MAX_TOKEN_LEN));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	{
		char* proxyPasswd = twConvertCallbackToPasswd(s->proxyPassCallback);
		twMD5_Update(&md5, (unsigned char *)proxyPasswd, strnlen(proxyPasswd, MAX_TOKEN_LEN));
		twFreePasswd(proxyPasswd);
	}
	twMD5_Final(mdHA1, &md5);
	for (i = 0; i < 16; i++) sprintf(&mdHA1String[i * 2], "%02x", (unsigned char)mdHA1[i]);
	TW_LOG(TW_TRACE,"createDigestCredentials: HA1: %s", mdHA1String);

	/* Create HA2 */
	twMD5_Init(&md5);
	twMD5_Update(&md5, (unsigned char *)"CONNECT:", strnlen("CONNECT:", 8));
	twMD5_Update(&md5, (unsigned char *)s->host, strnlen(s->host, MAX_TOKEN_LEN));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	snprintf(mdHA2String, 8, "%d", s->port);
	twMD5_Update(&md5, (unsigned char *)mdHA2String, strnlen(mdHA2String, MAX_TOKEN_LEN));
	twMD5_Final(mdHA2, &md5);
	for (i = 0; i < 16; i++) sprintf(&mdHA2String[i * 2], "%02x", (unsigned char)mdHA2[i]);
	TW_LOG(TW_TRACE,"createDigestCredentials: HA2: %s", mdHA2String);

	/* The MD5 hash of the combined HA1 result, server nonce (nonce),
	   request counter (nc), client nonce (cnonce),
	   quality of protection code (qop) and HA2 result is calculated.
	   The result is the "response" value provided by the client. */
	twMD5_Init(&md5);
	twMD5_Update(&md5, (unsigned char *)mdHA1String, strnlen(mdHA1String, MAX_TOKEN_LEN));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	twMD5_Update(&md5, (unsigned char *)nonce, strnlen(nonce, MAX_TOKEN_LEN));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	twMD5_Update(&md5, "00000001", strnlen("00000001", 8));
	twMD5_Update(&md5, ":", strnlen(":", 1));
	twMD5_Update(&md5, (unsigned char *)cnonce, 8);
	twMD5_Update(&md5, (unsigned char *)":auth:", strnlen(":auth:", 6));
	twMD5_Update(&md5, (unsigned char *)mdHA2String, strnlen(mdHA2String, MAX_TOKEN_LEN));
	twMD5_Final(mdHA3, &md5);
	for (i = 0; i < 16; i++) sprintf(&mdHA3String[i * 2], "%02x", (unsigned char)mdHA3[i]);	

	snprintf(response, REQ_SIZE, "Proxy-Authorization: Digest username=\"%s\",realm=\"%s\",nonce=\"%s\",uri=\"%s:%d\",qop=auth,nc=%s,cnonce=\"%s\",response=\"%s\"",
		s->proxyUser, realm, nonce, s->host, s->port, "00000001",cnonce,mdHA3String);
	if (opaque) {
		strncat(response, ", opaque=\"", REQ_SIZE - strnlen(response, MAX_RESPONSE_LEN));
		strncat(response, opaque, REQ_SIZE - strnlen(response, MAX_RESPONSE_LEN));
		strncat(response, "\"", REQ_SIZE - strnlen(response, MAX_RESPONSE_LEN));
	}
	strncat(response, "\r\n", REQ_SIZE - strnlen(response, MAX_RESPONSE_LEN));
	TW_LOG(TW_TRACE,"createDigestCredentials: \nresponse: \n%s\n", response);
	TW_FREE(hdrCopy);
	if (realm) TW_FREE(realm);
	if (qop) TW_FREE(qop);
	if (nonce) TW_FREE(nonce);
	if (opaque) TW_FREE(opaque);
	return response;
}

#if defined(USE_NTLM_PROXY) && (!defined(TLS) | (TLS != FIPS))
#define SUPPORTED_AUTH_TYPES_COUNT 3
char * supportAuthTypes[SUPPORTED_AUTH_TYPES_COUNT] = { "ntlm", "digest", "basic" };
#else
#define SUPPORTED_AUTH_TYPES_COUNT 2
char * supportAuthTypes[SUPPORTED_AUTH_TYPES_COUNT] = { "digest", "basic" };
#endif

int connectToProxy(twSocket * s, char * authCredentials) {
	char * req = NULL;
	char * resp = NULL;
	char portStr[8];
	char respCode[8];
	int res = 0;
	twList * authTypeList = 0;

	memset(portStr, 0, 8);
	memset(respCode, 0, 8);
	if (!s) return TW_INVALID_PARAM;
	req = (char *)TW_CALLOC(REQ_SIZE, 1);
	if (!req) {
		return TW_ERROR_ALLOCATING_MEMORY;
	} 
	resp = (char *)TW_CALLOC(REQ_SIZE + 1, 1);
	if (!resp) {
		TW_FREE(req);
		return TW_ERROR_ALLOCATING_MEMORY;
	} 
	strncpy(req,"CONNECT ", REQ_SIZE - 1);
	strncat(req, s->host, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, ":", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	sprintf(portStr, "%u", s->port);
	strncat(req, portStr, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, " HTTP/1.1\r\n" , REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
    strncat(req, "Host: " , REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, s->proxyHost, REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req,"\r\nProxy-Connection: keep-alive\r\nConnection: keep-alive\r\nPragma: no-cache\r\nUser-Agent: ThingWorx C SDK\r\n", REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	if (authCredentials) strncat(req, authCredentials , REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	strncat(req, "\r\n" , REQ_SIZE - strnlen(req, REQ_SIZE) - 1);
	TW_LOG(TW_TRACE,"connectToProxy: Sending Proxy CONNECT Request: \n%s\n", req);

	if (authCredentials) {
		/* Need to play a little trick in case the proxy closed the connection on us on the first connect,
		   we need to reconnect since we can't reopen, but don't want to call CoonectToProxy */
		int res = 0;
		char * tmpHost = s->proxyHost;
		s->proxyHost = NULL;
		if ((res = twSocket_Reconnect(s)) == -1) {
			TW_FREE(req);
			TW_FREE(resp);
			return res;
		}
		s->proxyHost = tmpHost;
	}
	res = twSocket_Write(s, req, strnlen(req, REQ_SIZE), twcfg.connect_timeout);
	if (res != strnlen(req, REQ_SIZE)) {
		TW_FREE(req);
		TW_FREE(resp);
		return TW_ERROR_WRITING_TO_SOCKET;
	}
	if (twSocket_WaitFor(s, twcfg.connect_timeout) <= 0) {
		TW_FREE(req);
		TW_FREE(resp);
		return TW_TIMEOUT_READING_FROM_SOCKET;		
	}
	res = twSocket_Read(s, resp, REQ_SIZE, twcfg.connect_timeout);
	/* Need at least enough for the response code */
	if (res < 13) {
		TW_FREE(req);
		TW_FREE(resp);
		return TW_ERROR_READING_RESPONSE;	
	}
	TW_LOG(TW_TRACE,"connectToProxy: Got Response from Proxy:\n\n%s\n", resp);
	strncpy(respCode, &resp[9], 3);
	if (strcmp(respCode, "200") == 0) {
		TW_FREE(req);
		TW_FREE(resp);
		return 0;
	}
	if (strcmp(respCode, "407") == 0 && !authCredentials) {
		ListEntry * entry = 0;
		int i = 0;
		char foundSupportedType = FALSE;
		/* Need authentication */	
		if (!s->proxyUser || !s->proxyPassCallback) {
			TW_FREE(req);
			TW_FREE(resp);
			return TW_INVALID_PROXY_CREDENTIALS;
		}
		/* Get a list of supported auth types and their params */
		authTypeList = twList_Create(NULL);
		{
			char * token = NULL;
			token = strtok(resp, "\r\n");
			while (token) {
				char * lcToken = lowercase(duplicateString(token));
				char * ptr = strstr(lcToken,"proxy-authenticate: ");
				if (ptr) {
					char * hdr = 0;
					/* use the original case */
					ptr = token + (ptr - lcToken);
					/* strip the www-authenticate and strip white space */
					ptr += strnlen("proxy-authenticate: ", 20);
					while (*ptr == ' ' || *ptr == '\t') ptr++;
					/* Headers can span multiple lines */
					hdr = duplicateString(ptr);
					token = strtok(NULL, "\r\n");
					while (token && (token[0] == ' ' || token[0] == '\t')) {
						char * oldHdr = hdr;
						size_t oldHdrLen = strnlen(oldHdr, REQ_SIZE);
						size_t tokenLen = strnlen(token, MAX_TOKEN_LEN);
						hdr = (char *)TW_CALLOC(oldHdrLen + tokenLen + 1, 1);
						if (!hdr) {
							twList_Delete(authTypeList);
							TW_FREE(req);
							TW_FREE(resp);
							TW_FREE(lcToken);
							return TW_ERROR_ALLOCATING_MEMORY;
						}
						strncpy(hdr, oldHdr, oldHdrLen);
						strncat(hdr, token, tokenLen);
						TW_FREE(oldHdr);
						token = strtok(NULL, "\r\n");
					} 
					twList_Add(authTypeList, hdr);	
					TW_FREE(lcToken);
					continue;
				}
				TW_FREE(lcToken);
				token = strtok(NULL, "\r\n");
			}
		}
		/* Done with the resp */
		/* Look for a supported type */
		for (i = 0; i < SUPPORTED_AUTH_TYPES_COUNT; i++) {
			entry = twList_Next(authTypeList, NULL);
			while (entry && entry->value) {
				char * lcValue = lowercase(duplicateString((char *)entry->value));
				if (strstr(lcValue, supportAuthTypes[i]) == lcValue) {
					foundSupportedType = TRUE;
					TW_FREE(lcValue);
					break;
				}
				TW_FREE(lcValue);
				entry = twList_Next(authTypeList, entry);
			}
			if (foundSupportedType) break;
		}
		if (!foundSupportedType) {
			twList_Delete(authTypeList);
			TW_FREE(req);
			TW_FREE(resp);
			return TW_UNSUPPORTED_PROXY_AUTH_TYPE;
		}
#if defined(USE_NTLM_PROXY) && (!defined(TLS) | (TLS != FIPS))
		if (strcmp(supportAuthTypes[i], "ntlm") == 0) {
			/* Make sure we're connected. Proxy server may have closed connection. */
			int res = 0;
			char * tmpHost = s->proxyHost;
			s->proxyHost = NULL;
			if ((res = twSocket_Reconnect(s)) == -1) {
				twList_Delete(authTypeList);
				TW_FREE(req);
				TW_FREE(resp);
				return res;
			}
			s->proxyHost = tmpHost;
			/* Ntlm Auth */
			{
				char* proxyPasswd = twConvertCallbackToPasswd(s->proxyPassCallback);
				res = NTLM_connectToProxy(s, req, resp, s->proxyUser, proxyPasswd);
				twFreePasswd(proxyPasswd);
			}
		} else 
#endif
		if (strcmp(supportAuthTypes[i], "digest") == 0) {
			/* Digest Auth */
			char * encodedAuth = createDigestCredentials(s, (char *)entry->value);
			if (!encodedAuth) {
				TW_LOG(TW_ERROR,"connectToProxy: Error allocating memory for Digest authentication");
				if (encodedAuth) TW_FREE(encodedAuth);
				TW_FREE(resp);
				TW_FREE(req);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
			TW_LOG(TW_INFO,"connectToProxy: Using Digest authentication");
			res = connectToProxy(s, encodedAuth);
			TW_FREE(encodedAuth);
		} else if (strcmp(supportAuthTypes[i], "basic") == 0) {
			/* Basic Auth */
			char* proxyPasswd = twConvertCallbackToPasswd(s->proxyPassCallback);
			unsigned long len = (strnlen(s->proxyUser, MAX_TOKEN_LEN) + 1 + strnlen(proxyPasswd, MAX_TOKEN_LEN)) * 2 + strnlen("Proxy-Authorization: Basic ", 27);
			char * encodedAuth = 0;
			char * auth = (char *)TW_CALLOC(len, sizeof(*auth));
		    encodedAuth = (char *)TW_CALLOC(len, sizeof(*encodedAuth));
			if (!auth || !encodedAuth) {
				TW_LOG(TW_ERROR,"connectToProxy: Error allocating memory for Basic authentication");
				if (auth) TW_FREE(auth);
				if (encodedAuth) TW_FREE(encodedAuth);
				TW_FREE(resp);
				TW_FREE(req);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
			TW_LOG(TW_INFO,"connectToProxy: Using Basic authentication");
			strcpy(auth, s->proxyUser);
			strncat(auth,":",1);
			strcat(auth,proxyPasswd);
			twFreePasswd(proxyPasswd);

			if (base64_encode((unsigned char *)auth, strnlen(auth, MAX_RESPONSE_LEN), (unsigned char *)encodedAuth, &len)) {
				if (auth) TW_FREE(auth);
				if (encodedAuth) TW_FREE(encodedAuth);
				TW_FREE(resp);
				TW_FREE(req);
				return TW_ERROR_ALLOCATING_MEMORY;
			}
            /* Assemble the credentials header entry */
			strcpy(auth, "Proxy-Authorization: Basic ");
			strcat(auth, encodedAuth);
			strcat(auth, "\r\n");
			res = connectToProxy(s, auth);

			/* Zero out and free memory containing proxy password */
			memset(encodedAuth,0,len*sizeof(*encodedAuth));
			TW_FREE(encodedAuth);
			memset(auth,0,len*sizeof(*auth));
			TW_FREE(auth);
		} else {
			res = TW_UNSUPPORTED_PROXY_AUTH_TYPE;
		}
		twList_Delete(authTypeList);
		TW_FREE(resp);
		TW_FREE(req);
		return res;
	}
	TW_FREE(req);
	return TW_ERROR_CONNECTING_TO_PROXY;	
}
