/*
 *  Copyright 2017, PTC, Inc.
 *
 *  NoTLS Client wrapper layer
 */

#ifndef NO_TLS_H
#define NO_TLS_H

#include "twOSPort.h"
#include "tomcrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

int16_t returnValue(int16_t v);
void * mallocByte();

#define TW_SSL_CTX						void
#define TW_NEW_SSL_CTX					mallocByte()

#define TW_SSL							twSocket
#define TW_NEW_SSL_CLIENT(a,b,c,d)		b

#define TW_SSL_SESSION_ID_SIZE			0
#define TW_GET_CERT_SIZE				0
#define TW_GET_CA_CERT_SIZE				0			
#define TW_HANDSHAKE_SUCCEEDED(a)		returnValue(1)
#define TW_SSL_SESSION_ID(a)			NULL
#define TW_NEW_SERVER(a,b,c)			NULL
#define TW_SSL_FREE(a)					returnValue(0)
#define TW_SSL_CTX_FREE(a)				returnValue(0)
#define TW_SSL_ACCEPT(a)				returnValue(0)
#define TW_SSL_WRITE(a,b,c)				twSocket_Write(a, b, c, 0)
#define TW_USE_CERT_FILE(a,b,c)			returnValue(0)
#define TW_USE_KEY_FILE(a,b,c,d)		returnValue(0)
#define TW_USE_CERT_CHAIN_FILE(a,b,c)	returnValue(0)
#define TW_SET_CLIENT_CA_LIST(a,b,c)	returnValue(0)
#define TW_SSL_READ(a,b, c, d)			twSocket_Read(a, b, c, d)
#define TW_VALIDATE_CERT(a,b)			returnValue(0)
#define TW_IS_FIPS_COMPATIBLE()		    returnValue(TW_FIPS_MODE_NOT_SUPPORTED)
#define TW_ENABLE_FIPS_MODE()			returnValue(TW_FIPS_MODE_NOT_SUPPORTED)
#define TW_DISABLE_FIPS_MODE()			returnValue(TW_FIPS_MODE_NOT_SUPPORTED)
#define TW_IS_FIPS_MODE_ENABLED()		returnValue(FALSE)
#define TW_SHA1_CTX						hash_state
#define TW_SHA1_INIT(a)					sha1_init(a)
#define TW_SHA1_UPDATE(a,b,c)			sha1_process(a,b,c)
#define TW_SHA1_FINAL(a,b)				sha1_done(b,a)

#define TW_MD5_CTX						hash_state
#define TW_MD5_INIT(a)					md5_init(a)
#define TW_MD5_UPDATE(a,b,c)			md5_process(a,b,c)
#define TW_MD5_FINAL(a,b)				md5_done(b,a)

#define TW_SSL_VERSION                  returnValue(NULL)
/* Set the cipher string to null incase of noTLS */
#define TW_SSL_DEFAULT_CIPHER_STRING NULL

static INLINE char * TW_GET_X509_FIELD(TW_SSL * ssl, char field) {
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* NO_TLS_H  */
