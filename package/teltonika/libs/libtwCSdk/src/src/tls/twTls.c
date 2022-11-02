/*
 *  Copyright 2017, PTC, Inc.
 *
 *  Portable twTLS Client  abstraction layer
 */

#include "twOSPort.h"
#include "twLogger.h"
#include "twTls.h"
#include "stringUtils.h"

TW_MUTEX tlsClientCreate_Mutex = NULL;

int twTlsClient_Create(const char * host, int16_t port, uint32_t options, twTlsClient ** client) {

	TW_SSL_CTX *ssl_ctx = NULL;
	twTlsClient * tls = NULL;

	/**
    int cert_index = 0, ca_cert_index = 0;
    int cert_size, ca_cert_size;
    char **ca_cert, **cert;
    const char *password = NULL;
	**/

	TW_LOG(TW_DEBUG, "twTlsClient_Create: Initializing TLS Client");
	/****
    cert_size = TW_GET_CERT_SIZE;
    ca_cert_size = TW_GET_CA_CERT_SIZE;
    ca_cert = (char **)TW_CALLOC(1, sizeof(char *)*ca_cert_size);
    cert = (char **)TW_CALLOC(1, sizeof(char *)*cert_size);
	****/

	if (!client) return TW_INVALID_PARAM;
	tls = (twTlsClient *)TW_CALLOC(sizeof(twTlsClient),1);
	if (!tls) { 
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	/* Create the mutex */
	tls->mtx = twMutex_Create();
	if (!tls->mtx) {
		TW_FREE(tls);
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	tls->options = options;
	tls->selfSignedOk = 0;
	tls->validateCert = TRUE;
	tls->isEnabled = TRUE;
	/*  Create our socket */
	tls->connection = twSocket_Create(host, port, 0);
	if (!tls->connection) {
		twTlsClient_Delete(tls);
		return TW_HOST_NOT_FOUND;
	}
	
	/* Create a mutex to wrap the call to TW_NEW_SSL_CTX_FUNC() */
	if(!tlsClientCreate_Mutex) {
		tlsClientCreate_Mutex = twMutex_Create();
	}

	/* Lock the mutex and create the ctx */
	twMutex_Lock(tlsClientCreate_Mutex);
	if (!ssl_ctx) ssl_ctx = TW_NEW_SSL_CTX;
	twMutex_Unlock(tlsClientCreate_Mutex);

	if (ssl_ctx == NULL){
		twTlsClient_Delete(tls);
		return TW_ERROR_CREATING_SSL_CTX;
	}
	tls->ctx = ssl_ctx;
	*client = tls;
	return TW_OK;
}

int twTlsClient_Connect(twTlsClient * t) {
	return twTlsClient_ConnectSession(t, NULL, 0);
}

int twTlsClient_ConnectSession(twTlsClient * t, void * sessionId, int sessionLength) {
	TW_SSL *ssl = NULL;
	int res = 0;
	if (!t) return -1;
	TW_LOG(TW_DEBUG, "twTlsClient_Connect: Connecting to server");
	twMutex_Lock(t->mtx);
	t->isEncrypted = FALSE;
	res = twSocket_Connect(t->connection);
	if (res) {
		TW_LOG(TW_ERROR,"Error intializing socket connection.  Err = %d", res);
		twMutex_Unlock(t->mtx);
		 return TW_SOCKET_INIT_ERROR;
	}
	if (!t->isEnabled) {
		twMutex_Unlock(t->mtx);
		return TW_OK;
	}
	ssl = TW_NEW_SSL_CLIENT(t->ctx, t->connection, sessionId, sessionLength);
	t->ssl = ssl;
	if (TW_HANDSHAKE_SUCCEEDED(t->ssl)) {
        if (t->validateCert) {
			if (twTlsClient_ValidateCert(t)) {
				 TW_LOG(TW_ERROR,"twTlsClient_Connect: Error intializing TLS connection.  Invalid certificate");
				 twMutex_Unlock(t->mtx);
				 return TW_INVALID_SSL_CERT;
			}
			/* Compare any x5089 fields that may have been set */
			if (t->x509_data) {
				char * tmp;
				uint8_t i = 0;
				for (i = 0; i < 6; i++) {
					if (t->x509_data[i]) {
						tmp = TW_GET_X509_FIELD(ssl, i);
						if (tmp) {
							if (strcmp(tmp, t->x509_data[i])) {
								TW_LOG(TW_ERROR,"twTlsClient_Connect: Certificate field %s does not match expected %s. Field %d", tmp, t->x509_data[i], i );
								TW_FREE(tmp);
								twMutex_Unlock(t->mtx);
								return TW_INVALID_SSL_CERT;
							}
							TW_FREE(tmp);
							tmp = NULL;
						}
					}
				}
			}
        }
	 } else {
		TW_LOG(TW_ERROR,"Error intializing SSL connection");
		twMutex_Unlock(t->mtx);
		 return TW_SOCKET_INIT_ERROR;
 	 }
	 TW_LOG(TW_DEBUG, "twTlsClient_Connect: TLS connection established");
	 t->isEncrypted = TRUE;
	 twMutex_Unlock(t->mtx);
	 return TW_OK;
}

int twTlsClient_Reconnect(twTlsClient * t, const char * host, int16_t port) {
	twSocket * old = NULL;
	if (!t || !t->mtx) {
		TW_LOG(TW_ERROR, "twTlsClient_Reconnect: NULL pointer found");
		return TW_INVALID_PARAM;
	}
	/* Close out and delete the existing session and restart */
	twMutex_Lock(t->mtx);
	TW_LOG(TW_DEBUG, "twTlsClient_Reconnect: Re-establishing SSL context");
	if (t->ssl) TW_SSL_FREE(t->ssl);
	if (t->connection){
		old = t->connection;
	}
	t->isEncrypted = FALSE;
	t->ssl = NULL;
	t->connection = NULL;
	t->connection = twSocket_Create(host, port, 0);
	if (!t->connection) {
		twMutex_Unlock(t->mtx);
		return TW_SOCKET_NOT_FOUND;
	}
	if (old) {
		if (old->proxyHost) twSocket_SetProxyInfo(t->connection, old->proxyHost, old->proxyPort, old->proxyUser, old->proxyPassCallback);
		twSocket_Delete(old);
	}
	twMutex_Unlock(t->mtx);
	return twTlsClient_Connect(t);
}

int twTlsServer_Create(twTlsClient * t)  {
	TW_SSL *ssl = NULL;
	if (!t || !t->connection) return -1;
	TW_LOG(TW_DEBUG, "twTlsServer_Create: Connecting to server");
	t->isEncrypted = FALSE;
	if (t->connection->state != OPEN) {
		TW_LOG(TW_ERROR,"twTlsServer_Create: Socket is not open");
		 return TW_SOCKET_INIT_ERROR;
	}
	if (!t->isEnabled) return TW_OK;

	ssl = TW_NEW_SERVER(t->ctx, t->connection);
	if (!ssl) return TW_ERROR;
	t->ssl = ssl;
	TW_LOG(TW_DEBUG, "twTlsServer_Create: TLS server socket established");
	return TW_OK;
}

int twTlsServer_Accept(twTlsClient *t) {
	if (!t || !t->ssl) return -1;
	TW_LOG(TW_DEBUG, "twTlsServer_Accept: Client Handshake in progress");
	t->isEncrypted = FALSE;
	return TW_SSL_ACCEPT(t->ssl);
}

int twTlsClient_Close(twTlsClient * t) {
	if (!t || !t->mtx) return TW_INVALID_PARAM;
	TW_LOG(TW_DEBUG, "twTlsClient_Close: Disconnecting from server");
	twMutex_Lock(t->mtx);
	twSocket_Close(t->connection);
	if (t->ssl) {
		TW_LOG(TW_DEBUG, "twTlsClient_Close: Deleting SSL session");
		TW_SSL_FREE(t->ssl);
		t->ssl = NULL;
	}
	t->isEncrypted = FALSE;
	twMutex_Unlock(t->mtx);
	return TW_OK;
}

#ifndef DATA_AVAILABLE
#define DATA_AVAILABLE(a,b,c) twSocket_WaitFor(a, b)
#endif

int twTlsClient_Read(twTlsClient * t, char * buf, int len, int timeout) {
	int bytesRead = 0;
	if (!t || !t->mtx) return -1;
	twMutex_Lock(t->mtx);
	if (!DATA_AVAILABLE(t->connection, timeout, t->ssl)) {
		twMutex_Unlock(t->mtx);
		return 0;
	}
	/* TW_LOG(TW_TRACE, "twTlsClient_Read: Data available.  Attempting to read %d bytes.", len); */
	if (!t->isEnabled) {
		bytesRead = twSocket_Read(t->connection, buf, len, timeout);
	} else {
		if (!t->ssl) {
			TW_LOG(TW_ERROR, "twTlsClient_Read: NULL t->ssl pointer");
			twMutex_Unlock(t->mtx);
			return -1;
		}
		/* Still some room in the buffer after copying all the leftovers */
		bytesRead = TW_SSL_READ(t->ssl, buf, len, timeout);
		if (bytesRead < 0) {
			if (bytesRead == TW_READ_TIMEOUT) {
				/* TW_LOG(TW_ERROR, "twTlsClient_Read: Timed out after %d milliseconds", timeout); */
				bytesRead = 0; /* Set to 0, since we don't want to treat a timeout as an error in the calling function */
			} else {
				TW_LOG(TW_ERROR, "twTlsClient_Read: Error during read.  Error code: %d", bytesRead);
			}
		} else {
			TW_LOG(TW_TRACE, "twTlsClient_Read: Read: %d, Asked for: %d", bytesRead, len);
		}
	}
	twMutex_Unlock(t->mtx);
	return bytesRead;
}

int twTlsClient_Write(twTlsClient * t, char * buf, int len, int timeout) {
	int res = -1;
	if (!t || !t->mtx) return -1;
	twMutex_Lock(t->mtx);
	if (!t->isEnabled) {
		res = twSocket_Write(t->connection, buf, len, timeout);
	} else {
		if (!t->ssl){
            twMutex_Unlock(t->mtx);
            return -1;
        }
		res = TW_SSL_WRITE(t->ssl, buf, len);
	}
	twMutex_Unlock(t->mtx);
	return res;
}

int twTlsClient_Delete(twTlsClient * t) {
	uint8_t i = 6;
	if (!t || !t->mtx) return TW_INVALID_PARAM;
	twTlsClient_Close(t);
	twMutex_Lock(t->mtx);
	if (t->x509_data) {
		while (i--) if (t->x509_data[i]) TW_FREE(t->x509_data[i]);
		TW_FREE(t->x509_data);
	}
	if (t->ssl) TW_SSL_FREE(t->ssl);
	if (t->ctx) {
		TW_SSL_CTX_FREE(t->ctx);
	}
	if (t->connection) twSocket_Delete(t->connection);
	twMutex_Unlock(t->mtx);
	twMutex_Delete(t->mtx);
	TW_FREE(t);
	return TW_OK;
}

void * twTlsClient_GetSessionId(twTlsClient * t) {
	if (!(TW_SSL_SESSION_ID(t->ssl))) return NULL;
	return TW_SSL_SESSION_ID(t->ssl);
}

void twTlsClient_SetSelfSignedOk(twTlsClient * t) {
	if (!t) return;
	t->selfSignedOk = 1;
}

void twTlsClient_DisableCertValidation(twTlsClient * t) {
	if (!t) return;
	t->validateCert = FALSE;
}

void twTlsClient_DisableEncryption(twTlsClient * t) {
	if (!t) return;
	/* If we are connected we need to close the connection */
	if (t->isEncrypted) twTlsClient_Close(t);
	t->isEnabled = FALSE;
}

int twTlsClient_ValidateCert(twTlsClient * t) {
	if (!t) return 0;
	return TW_VALIDATE_CERT(t->ssl, t->selfSignedOk);
}

int twTlsClient_UseCertificateFile(twTlsClient * t, const char *file, int type) {
	if (!t) return -1;
	return TW_USE_CERT_FILE(t->ctx, file, type);
}

int twTlsClient_UsePrivateKeyFile(twTlsClient * t, const char *file, int type)  {
	int ret = TW_OK;
	char * keypasswd = NULL;
	if (!t) return -1;
	keypasswd = twConvertCallbackToPasswd(t->keypasswdCallback);
	ret = TW_USE_KEY_FILE(t->ctx, file, type, keypasswd);
	twFreePasswd(keypasswd);
	return ret;
}

int	twTlsClient_UseCertificateChainFile(twTlsClient * t, const char *file, int type) {
	if (!t) return -1;
	return TW_USE_CERT_CHAIN_FILE(t->ctx, file, type);
}

int twTlsClient_SetClientCaList(twTlsClient * t, char * caFile, char * caPath)  {
	if (!t) return -1;
	return TW_SET_CLIENT_CA_LIST(t->ctx, caFile, caPath);
}

void twTlsClient_SetDefaultPasswdCb(twTlsClient * t, twPasswdCallbackFunction u) {
	if (t) {
		t->keypasswdCallback = u;
	}
}

int twTlsClient_IsEncrypted(twTlsClient * t) {
	if (!t) return TW_INVALID_PARAM;
	return t->isEncrypted;
}

int twTlsClient_SetX509Fields(twTlsClient * t, char * subject_cn, char * subject_o, char * subject_ou,
							  char * issuer_cn, char * issuer_o, char * issuer_ou) {
	uint8_t i = 6;

	if (!t) return TW_INVALID_PARAM;

	/* free x509 data if present */
	if (t->x509_data) {
		while (i--) if (t->x509_data[i]) TW_FREE(t->x509_data[i]);
		TW_FREE(t->x509_data);
	}

	t->x509_data = (char **)TW_CALLOC(sizeof(char *) * 6 , 1);
	if (!t->x509_data) {
		TW_LOG(TW_ERROR, "twTlsClient_SetX509Fields: Unable to allocate memory for x509 data");
		return TW_ERROR_ALLOCATING_MEMORY;
	}
	if (subject_cn) t->x509_data[TW_SUBJECT_CN] = duplicateString(subject_cn);
	if (subject_o) t->x509_data[TW_SUBJECT_O] = duplicateString(subject_o);
	if (subject_ou) t->x509_data[TW_SUBJECT_OU] = duplicateString(subject_ou);
	if (issuer_cn) t->x509_data[TW_ISSUER_CN] = duplicateString(issuer_cn);
	if (issuer_o) t->x509_data[TW_ISSUER_O] = duplicateString(issuer_o);
	if (issuer_ou) t->x509_data[TW_ISSUER_OU] = duplicateString(issuer_ou);
	return TW_OK;
}

int16_t returnValue(int16_t v) { return v; }
void * mallocByte() { return TW_MALLOC(1); }

/*
SHA1 for the websocket accept header.  If not defined
by the TLS library, use built in libTomcrypt
*/
void twSHA1_Init(TW_SHA1_CTX * ctx) {
	TW_SHA1_INIT(ctx);
}

void twSHA1_Update(TW_SHA1_CTX * ctx, const uint8_t * msg, int len) {
	TW_SHA1_UPDATE(ctx, msg, len);
}

void twSHA1_Final(uint8_t *digest, TW_SHA1_CTX * ctx) {
	TW_SHA1_FINAL(digest, ctx);
}

/*
MD5:  If not defined by the TLS library, use built in libTomcrypt
*/
void twMD5_Init(TW_MD5_CTX * ctx) {
	TW_MD5_INIT(ctx);
}

void twMD5_Update(TW_MD5_CTX * ctx, const uint8_t * msg, int len) {
	TW_MD5_UPDATE(ctx, msg, len);
}

void twMD5_Final(uint8_t *digest, TW_MD5_CTX * ctx) {
	TW_MD5_FINAL(digest, ctx);
}
