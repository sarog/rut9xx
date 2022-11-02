/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twOpenSSL.h
 * \brief Portable ThingWorx OpenSSL wrapper layer
*/

#ifndef TW_OPENSSL_H
#define TW_OPENSSL_H

#include "twDefaultSettings.h"
#include "twOSPort.h"
#include "twLogger.h"
#include "stdio.h"
#include "string.h"
#include "stringUtils.h"

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
/*OpenSSL version header*/
#include <openssl/opensslv.h>

#define openssl_version TW_SSL_VERSION()
/*Set the default openssl cipher string */
#define TW_SSL_DEFAULT_CIPHER_STRING "ALL:!aNULL:!eNULL:!LOW:!3DES:!MD5:!EXP:!PSK:!DSS:!RC4:!SEED:!ADH:!IDEA:!3DES:!SRP"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USING_OPENSSL
#define USING_OPENSSL
#endif

#define TW_SSL_CTX						SSL_CTX
#define TW_SSL							SSL
#define TW_SSL_SESSION_ID_SIZE			sizeof(void *)
#define TW_SSL_SESSION_ID(a)            SSL_get1_session(a)
#define TW_GET_CERT_SIZE				ssl_get_config(SSL_MAX_CERT_CFG_OFFSET)
#define TW_GET_CA_CERT_SIZE				ssl_get_config(SSL_MAX_CA_CERT_CFG_OFFSET)
#define TW_HANDSHAKE_SUCCEEDED(a)		(a && SSL_get_state(a) == TLS_ST_OK)
#define TW_SSL_FREE(a)					SSL_free(a)
#define TW_SSL_CTX_FREE(a)				SSL_CTX_free(a)
#define DATA_AVAILABLE(a,b,c)			(twSocket_WaitFor(a, b) || (c && SSL_pending(c)))
#ifndef OPENSSL_FIPS
#define TW_FIPS_CAPABLE                 FALSE
#else
#define TW_FIPS_CAPABLE                 TRUE
#endif

/*********
* Neither SHA1 nor MD5 are FIPS approved, but we don't use them
* for any crypto/communications functions, however the FIPS library
* will exit the program if we try to use them.
**********/
#if TLS != FIPS
#define TW_SHA1_CTX						SHA_CTX
#define TW_SHA1_INIT(a)					SHA1_Init(a)
#define TW_SHA1_UPDATE(a,b,c)			SHA1_Update(a,b,c)
#define TW_SHA1_FINAL(a,b)				SHA1_Final(a,b)

#define TW_MD5_CTX						MD5_CTX
#define TW_MD5_INIT(a)					MD5_Init(a)
#define TW_MD5_UPDATE(a,b,c)			MD5_Update(a,b,c)
#define TW_MD5_FINAL(a,b)				MD5_Final(a,b)
#endif

static INLINE const char* TW_SSL_VERSION()
{
	static char tls_full_version[64] = {0};    /* The caller isn't expected to FREE what we return */
	const char* raw_tls_full_version = SSLeay_version(SSLEAY_VERSION); /* eg "OpenSSL 1.0.2l-fips  25 May 2017" */

	if(strncmp(SSLeay_version(SSLEAY_VERSION), OPENSSL_VERSION_TEXT,64) != 0){
		TW_LOG(TW_WARN, "TW_SSL_VERSION: Error tls runtime version: %s, does not match compiled version: %s",SSLeay_version(SSLEAY_VERSION),OPENSSL_VERSION_TEXT);
	}

	memcpy (tls_full_version, raw_tls_full_version, strnlen(raw_tls_full_version, 63));

	strtok(tls_full_version, " ");
	return strtok(NULL, " ");
}

static INLINE int TW_USE_CERT_FILE(TW_SSL_CTX * ctx, const char * cert, int type)
{
    int32_t ret = SSL_CTX_use_certificate_file(ctx, cert, type);
    if (ret <= 0) {
	   TW_LOG(TW_ERROR, "TW_USE_CERT_FILE: Error setting the certificate file.");
	   return -1;
	}
	return TW_OK;
}

/**
* \brief Queries the TLS backend to determine whether it is FIPS compatible
*
* \return #TW_OK if successful, positive integral on error code (see
* twErrors.h) if an error was encountered.
*/
static INLINE int TW_IS_FIPS_COMPATIBLE() {
	if (TW_FIPS_CAPABLE == TRUE) {
		return TW_OK;
	}
	return TW_FIPS_MODE_NOT_SUPPORTED;
}

/**
* \brief Queries the TLS backend to determine if FIPS mode is enabled
*
* \return #TRUE if FIPS is enabled, #FALSE otherwise
*/
static INLINE int TW_IS_FIPS_MODE_ENABLED() {
	if (TW_FIPS_CAPABLE == TRUE) {
		int ret = 0;
		ret = FIPS_mode();
		if(!ret) {
			return FALSE;
		}
		else {
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * \brief Enables FIPS mode for the entire application
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
static INLINE int TW_ENABLE_FIPS_MODE() {
	int ret = TW_IS_FIPS_COMPATIBLE();
	if (TW_OK == ret) {
		if (TW_IS_FIPS_MODE_ENABLED() != TRUE) {
			ret = FIPS_mode_set(1);  /* FIPS mode *on* */
			if(ret != 1) {
				TW_LOG(TW_ERROR, "TW_ENABLE_FIPS_MODE: FIPS_mode_set(on) failed: %s.", ERR_error_string(ERR_get_error(), NULL));
				ret = TW_ENABLE_FIPS_MODE_FAILED;
			}
		}
	}
	return ret;
}

/**
 * \brief Disable FIPS mode for the entire application
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/


static INLINE int TW_DISABLE_FIPS_MODE() {
	int ret = 0;
	if (TW_IS_FIPS_MODE_ENABLED() == TRUE) {
		ret = FIPS_mode_set(0);  /* FIPS mode *off*/
		if(ret != 1) {
			TW_LOG(TW_ERROR, "TW_DISABLE_FIPS_MODE: FIPS_mode_set(off) failed: %s.", ERR_error_string(ERR_get_error(), NULL));
			return TW_DISABLE_FIPS_MODE_FAILED;
		}
	}
	return TW_OK;
}

/**
 * \brief Creates a new #TW_SSL structure for connection with the specified
 * settings (see SSL_new(), SSL_connect()).
 *
 * \param[in]     ctx           A pointer to the context to associate with the
 * connection.
 * \param[in]     sock          A pointer to the ::twSocket for the connection
 * to use.
 * \param[in]     session_id    An optional session ID to associate with the
 * connection.
 *
 * \return A pointer to the newly allocated #TW_SSL structure.
 *
 * \note The calling function will gain ownership of the returned structure and
 * is responsible for freeing it via TW_SSL_FREE().
*/
static INLINE TW_SSL * TW_NEW_SSL_CLIENT(TW_SSL_CTX * ctx, twSocket * sock, void * session_id, int session_size) {
	TW_SSL * ssl = NULL;
	BIO * bio = NULL;
	signed int res = 0;
	ssl=SSL_new(ctx);
	if (!ssl) {
		TW_LOG(TW_ERROR, "TW_NEW_SSL_CLIENT: Error creating SSL session. Error: %s", ERR_error_string(ERR_get_error(), NULL));
        return NULL;
	}
	if (session_id) SSL_set_session(ssl, (SSL_SESSION *)session_id);
	bio=BIO_new_socket(sock->sock,BIO_NOCLOSE);
	if (!bio) {
		TW_LOG(TW_ERROR, "TW_NEW_SSL_CLIENT: Error creating SSL BIO. Error: %s.", ERR_error_string(ERR_get_error(), NULL));
        TW_SSL_FREE(ssl);
		return NULL;
	}

	SSL_set_bio(ssl,bio,bio);
	res = SSL_connect(ssl);
	if ( res != 1 ) {
		const char * tmp = NULL;
		int index = 0;
		TW_LOG(TW_ERROR,"TW_NEW_SSL_CLIENT: SSL handshake error. Error: %s.", ERR_error_string(ERR_get_error(), NULL));
		  do {
			tmp = SSL_get_cipher_list(ssl,index);
			if (tmp != NULL) {
			  TW_LOG(TW_TRACE,"TW_NEW_SSL_CLIENT: Ciphers Supported: %s", tmp);
			  index++;
			}
		  }
		  while (tmp != NULL);
        TW_SSL_FREE(ssl);
		return NULL;
	}
	return ssl;
}

static const unsigned char s_server_session_id_context[SSL_MAX_SSL_SESSION_ID_LENGTH] = {0};

/**
 * \brief Logs a list of all available ciphers for the ctx to the logs
 *
 * \param[in]     ctx       A pointer to the context to associate with the
 * connection.
 *
 * \return TW_OK if it was able to get a list of ciphers suites
 *
*/
static INLINE int TW_SSL_LIST_CIPHERS(TW_SSL_CTX * ctx) {
	TW_SSL * ssl = NULL;
	char *ciphers = NULL;
	const char *p;
	int i;

	ssl=SSL_new(ctx);

	for (i = 0;; i++) {
		p = SSL_get_cipher_list(ssl, i);
		if (p == NULL) {
			break;
		}
		if (i == 0) {
			ciphers = duplicateString(p);
		}
		else {
			concatenateStrings(&ciphers,":");
			concatenateStrings(&ciphers,p);
		}
	}
	TW_LOG(TW_TRACE, "Listing all available ciphers: %s", ciphers);
    TW_FREE(ciphers);
	TW_SSL_FREE(ssl);
	return TW_OK;
}

/**
 * \brief Creates a new #TW_SSL connection structure (see SSL_new()).
 *
 * \param[in]     ctx       A pointer to the context to associate with the
 * connection.
 * \param[in]     sock      A pointer to the ::twSocket to use for the
 * connection.
 *
 * \return A pointer to the newly allocated #TW_SSL structure.
 *
 * \note The calling function will gain ownership of the returned structure and
 * is responsible for freeing it via TW_SSL_FREE().
*/
static INLINE TW_SSL * TW_NEW_SERVER(TW_SSL_CTX * ctx, twSocket * sock)	{
	TW_SSL * ssl = NULL;
	BIO * bio = NULL;
	RSA * r = NULL;
	X509 * x = NULL;
	unsigned char * p = NULL;
	int ret = 0;

	if (!ctx || !sock) return NULL;
   
	/* Enable ECDH based keys if ctx used for server */
    SSL_CTX_set_ecdh_auto(ctx, 1);
    
	ret = SSL_CTX_check_private_key(ctx);
	if ( ret != 1 ) {
		TW_LOG(TW_ERROR, "TW_NEW_SERVER: Error validating server certificate/key pair. Error: %s", ERR_error_string(ERR_get_error(), NULL));
		return NULL;
	}
	SSL_CTX_set_session_id_context(ctx, s_server_session_id_context,  sizeof(s_server_session_id_context)); 
	ssl=SSL_new(ctx);
	if (!ssl) {
		TW_LOG(TW_ERROR, "TW_NEW_SERVER: Error creating SSL session. Error: %s", ERR_error_string(ERR_get_error(), NULL));
        return NULL;
	}
	bio=BIO_new_socket(sock->sock,BIO_NOCLOSE);
	if (!bio) {
		TW_LOG(TW_ERROR, "TW_NEW_SERVER: Error creating SSL BIO. Error: %s.", ERR_error_string(ERR_get_error(), NULL));
        TW_SSL_FREE(ssl);
		return NULL;
	}
	SSL_set_bio(ssl,bio,bio);
	return ssl;
}

/**
 * \brief Waits for a #TW_SSL client to initiate a handshake with the server.
 * Wrapper function for SSL_accept().
 *
 * \param[in]     s         A pointer to the #TW_SSL client to utilize.
 *
 * \return 0 on success, -1 if an error was encountered.
*/
static INLINE int TW_SSL_ACCEPT(TW_SSL * s)	{
	int res = SSL_accept(s);
	if ( res != 1 ) {
		const char * tmp = NULL;
		int index = 0;
		TW_LOG(TW_ERROR,"TW_SSL_ACCEPT: SSL handshake error. Error: %s.", ERR_error_string(ERR_get_error(), NULL));
		  do {
			tmp = SSL_get_cipher_list(s,index);
			if (tmp != NULL) {
			  TW_LOG(TW_TRACE,"  Ciphers Supported: %s", tmp);
			  index++;
			}
		  }
		  while (tmp != NULL);
		return -1;
	}
	return 0;
}

/**
 * \brief Loads the certificate authority cert chain used to validate the
 * server's certificate in \p file into \p ctx.  Wrapper function for
 * SSL_CTX_use_PrivateKey_file().
 *
 * \param[in]     ctx       The context to load the key into.
 * \param[in]     file      The file to get the certificate from.
 * \param[in]     type      The container format of \p file (should be
 * #SSL_FILETYPE_PEM).
 * \param[in]     passwd    The password callback to use for encrypted PEM file
 * handling.
*/
static INLINE int TW_USE_KEY_FILE(SSL_CTX * ctx, const char * file, int type, char * passwd) {
	uint32_t ret = 0;
    SSL_CTX_set_default_passwd_cb_userdata(ctx, passwd);
    ret = SSL_CTX_use_PrivateKey_file(ctx, file, type);

    if (ret <= 0) {
	   TW_LOG(TW_ERROR, "TW_USE_KEY_FILE: Error setting the key file - %s", ERR_error_string(ERR_get_error(), NULL));
	   return -1;
	}
	return TW_OK;
}


/**
 * \brief sets the default location for trusted CA certs.  Wrapper function for
 * SSL_CTX_load_verify_locations().
 *
 * \param[in]     ctx       The context to load the key into.
 * \param[in]     CAfile    The path of the certificate
 * \param[in]     CApath    The path of the certificate authority
 * handling.
 *
 * \return returns #TW_OK on success or errno on failure
*/
static INLINE int TW_SET_CLIENT_CA_LIST(SSL_CTX *ctx, const char *CAfile,const char *CAPAth) { 
	int ret = 0;
	ret = SSL_CTX_load_verify_locations(ctx, CAfile, CAPAth);
	if (ret == 1) {
		return TW_OK;
	} else {
		TW_LOG (TW_ERROR, "TW_SET_CLIENT_CA_LIST: Loading a certificate authority chain from file into the ctx: %s.", ERR_error_string(ERR_get_error(), NULL));
		return TW_TLS_ERROR_LOADING_FILE;
	}
}

/*#define TW_USE_CERT_CHAIN_FILE(a,b)		SSL_CTX_use_certificate_chain_file(a,b)*/
/**
 * \brief loads a certificate chain from file into ctx. The certificates must be in PEM format.
 *      Wrapper function for SSL_CTX_use_certificate_chain_file().
 *
 * \param[in]     ctx       The context to load the key into.
 * \param[in]     file      The file to get the certificate from.
 *
 * \return returns #TW_OK on success or errno on failure
*/

static INLINE int TW_USE_CERT_CHAIN_FILE(SSL_CTX *ctx, const char *file, int type) { 
	int ret = 0;
	ret = SSL_CTX_use_certificate_chain_file(ctx, file);
	if (ret == 1) {
		return TW_OK;
	} else {
        TW_LOG (TW_ERROR, "TW_USE_CERT_CHAIN_FILE: Error setting the default location for certificate chain: %s.", ERR_error_string(ERR_get_error(), NULL));
		return TW_TLS_ERROR_LOADING_FILE;
	}
}


/**
 * \brief Create a new #SSL_CTX stucture as framework for TLS/SSL enabled
 * functions.  Wrapper function for SSL_CTX_new().
 *
 * \return A pointer to the newly allocated #SSL_CTX structure.
 *
 * \note The calling function will gain ownership of the returned structure and
 * is responsible for freeing it via TW_SSL_CTX_FREE().
*/
static INLINE SSL_CTX * TW_NEW_SSL_CTX_FUNC() {
	const char *cipher_string = NULL;
	int ret = 0;
	SSL_CTX * ctx = NULL;
	if (twcfg.initialize_encryption_library) {
		if (SSL_library_init () != 1)
			{
			TW_LOG (TW_ERROR, "TW_NEW_SSL_CTX_FUNC: Error initializing OpenSSL library: %lx.", ERR_get_error ());
			return NULL;
			}
		OpenSSL_add_all_algorithms (); /* load encryption & hash algorithms for SSL */
		SSL_load_error_strings (); /* load the error strings for good error reporting */
		ERR_load_crypto_strings ();
		}
	ctx = SSL_CTX_new(SSLv23_method());
    /* Limit to TLS only */
	if (ctx) SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
		/* Check if running with FIPS mode enabled */
		if(TW_IS_FIPS_MODE_ENABLED() == TRUE){
		/* If FIPS enabled set to OpenSSL recommended cipher string https://wiki.openssl.org/index.php/FIPS_mode_and_TLS */
			cipher_string = "TLSv1.2+FIPS:kRSA+FIPS:!eNULL:!aNULL";
		} /* If cipher string not set drop to default */
		else if(twcfg.cipher_set == NULL){
			cipher_string = TW_SSL_DEFAULT_CIPHER_STRING;
		} else {
		/* If running without FIPS check for cipher string set in twcfg */
			cipher_string = twcfg.cipher_set;
		}

	if (ctx) {
		ret = SSL_CTX_set_cipher_list(ctx, cipher_string);
		if (ret != 1) {
			TW_LOG(TW_ERROR,"TW_NEW_SSL_CTX_FUNC: Setting SSL cipher string Error: %s", ERR_error_string(ERR_get_error(), NULL));
			TW_SSL_CTX_FREE(ctx);
			return NULL;
			}
		}
		else {
			TW_LOG(TW_INFO,"Setting OpenSSL cipher string to: %s", twcfg.cipher_set);
		}
		ret = TW_SSL_LIST_CIPHERS(ctx);
		if (ret != TW_OK ) {
			TW_LOG(TW_ERROR,"TW_NEW_SSL_CTX_FUNC: Error getting valid cipher list: %s", ERR_error_string(ERR_get_error(), NULL));
		}
	return ctx;
}
#define TW_NEW_SSL_CTX TW_NEW_SSL_CTX_FUNC()

/**
 * \brief Reads \p len bytes of data from \p ssl into \p buf (see SSL_read()).
 *
 * \param[in]     ssl       A pointer to the #TW_SSL connection to read from.
 * \param[out]    buf       A buffer to store the read data.
 * \param[in]     len       The length of data to read.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for
 * I/O before timing out.
 *
 * \return 0 on success, -1 if an error was encountered.
 *
 * \note The calling function will retain ownership of \p buf and is
 * responsible for freeing it.
*/
static INLINE int TW_SSL_READ(TW_SSL * ssl, char * buf, int len, int32_t timeout) {
	int32_t ret = SSL_ERROR_NONE;
	int32_t retries = 0;
	/* Loop until we are unblocked or timeout */
	DATETIME start = twGetSystemTime(TRUE);
	if (!ssl || !buf) return -1;
	while (twTimeLessThan(twGetSystemTime(TRUE), twAddMilliseconds(start,twcfg.default_message_timeout)) && retries < 3) {
		if(!ssl) break;
		ret = SSL_read(ssl, buf, len);
		switch (SSL_get_error(ssl,ret)) {
		case SSL_ERROR_NONE:
			break;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_X509_LOOKUP:
			ret = -1;
			TW_LOG(TW_INFO, "TW_SSL_READ: Read BLOCK on try %d. Error: %s", retries, ERR_error_string(ret, NULL));
			retries++;
			twSleepMsec(5);
			continue;
		case SSL_ERROR_SYSCALL:
		case SSL_ERROR_SSL:
			TW_LOG(TW_ERROR, "TW_SSL_READ: Error reading from SSL stream");
			break;
		case SSL_ERROR_ZERO_RETURN:
			TW_LOG(TW_TRACE, "TW_SSL_READ: Read 0 bytes");
			break;
		}
		break;
	}

	/* Check for a timeout or error */
	if (ret <= 0 || twTimeGreaterThan(twGetSystemTime(TRUE), twAddMilliseconds(start,twcfg.default_message_timeout))) {
		TW_LOG(TW_ERROR, "TW_SSL_READ: Timed out or error waiting reading from socket. Error: %s", ERR_error_string(ret, NULL));
		return TW_TIMEOUT_READING_FROM_SOCKET;
	}
	return ret;
}

/**
 * \brief Writes \p len bytes of data in \p buf to \p ssl.
 *
 * \param[in]     ssl       A pointer to the #TW_SSL connection to write to.
 * \param[out]    buf       A buffer containing the data to be written.
 * \param[in]     len       The length of data to write.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function will retain ownership of \p buf and is
 * responsible for freeing it.
*/
static INLINE int TW_SSL_WRITE(TW_SSL * ssl, char * buf, int len) {
	int retries = 0;
	int result = 0;
	int socketReady = 0;
	BIO * bio;

	TW_SOCKET_TYPE sslSocket;
	fd_set sslSocketFd;

	while (retries < 3) {
		bio = SSL_get_wbio(ssl);
		BIO_get_fd(bio, &sslSocket);

		FD_ZERO(&sslSocketFd);
		FD_SET(sslSocket, &sslSocketFd);

		/* EDGE-2127: Check if the socket is ready before writing to it. Skipping this step may cause the call to SSL_write to hang.*/
		socketReady = select(FD_SETSIZE, 0, &sslSocketFd, 0, 0);

		if(socketReady == 1)
		{
			result = SSL_write(ssl, buf, len);

	                switch (SSL_get_error(ssl,result)) {
                        case SSL_ERROR_NONE:
                                break;
                        case SSL_ERROR_WANT_WRITE:
                        case SSL_ERROR_WANT_READ:
                        case SSL_ERROR_WANT_X509_LOOKUP:
                                result = 0;
                                retries++;
                                TW_LOG(TW_ERROR, "TW_SSL_WRITE: Write BLOCK. Retry %d in 5 msec", retries);
                                twSleepMsec(5);
                                continue;
                        case SSL_ERROR_SYSCALL:
                        case SSL_ERROR_SSL:
                                TW_LOG(TW_ERROR, "TW_SSL_WRITE:  Write failed. Error:  %s", ERR_error_string(result, NULL));
                                break;
                        case SSL_ERROR_ZERO_RETURN:
                                TW_LOG(TW_WARN,"TW_SSL_WRITE: Zero bytes written.");
                                break;
	                }
                	break;
		}
		else
		{
			result = SSL_ERROR_SSL;
			if(socketReady == 0)
			{
				TW_LOG(TW_ERROR, "TW_SSL_WRITE Error: Socket not ready: %d", socketReady);
			}
			else
			{
				TW_LOG(TW_ERROR, "TW_SSL_WRITE Error occurred reading socket state: %d", socketReady);
			}

			retries++;
		}
	}
	return result;
}

/**
 * \brief Validates the certificate of \p ssl.
 *
 * \param[in]     ssl       A pointer to the #TW_SSL to validate the
 * certificate of.
 *
 * \return 0 on success, -1 on error.
*/
static INLINE int TW_VALIDATE_CERT(TW_SSL * ssl, char selfSignedOk) {
	int32_t res = SSL_get_verify_result(ssl);
	if( res != X509_V_OK && !(selfSignedOk && (res == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT || res == X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN))) {
		TW_LOG(TW_ERROR, "TW_VALIDATE_CERT: Certificate rejected.  Code: %d, Reason = %s",res, X509_verify_cert_error_string(res));
		return TW_INVALID_SSL_CERT;
	} else {
		return 0;
	}
}

/**
 * \brief Gets an X509 field of \p ssl.
 *
 * \param[in]     ssl       The #TW_SSL connection to get a field of.
 * \param[in]     field     The field to get.
 *
 * \return A string containing the requested field.  NULL if an error was
 * encountered.
 *
 * \note The calling function will gain ownership of the returned string and is
 * responsible for freeing it.
*/
static INLINE char * TW_GET_X509_FIELD(TW_SSL * ssl, char field) {
	/* Caller will own the returned pointer */
	int nid = 0;
	char tmp[256];
	X509 * cert = NULL;
	X509_NAME * name = NULL;
	if (!ssl) return NULL;
	cert = SSL_get_peer_certificate(ssl);
	if (!cert) return NULL;
	switch (field) {
	case TW_SUBJECT_CN:
		nid = OBJ_txt2nid("CN");
		name = X509_get_subject_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	case TW_SUBJECT_O:
		nid = OBJ_txt2nid("O");
		name = X509_get_subject_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	case TW_SUBJECT_OU:
		nid = OBJ_txt2nid("OU");
		name = X509_get_subject_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	case TW_ISSUER_CN:
		nid = OBJ_txt2nid("CN");
		name = X509_get_issuer_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	case TW_ISSUER_O:
		nid = OBJ_txt2nid("O");
		name = X509_get_issuer_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	case TW_ISSUER_OU:
		nid = OBJ_txt2nid("OU");
		name = X509_get_issuer_name(cert);
		X509_NAME_get_text_by_NID(name, nid, tmp, 255);
		break;
	default:
		tmp[0] = 0x00;
	}
	return duplicateString(tmp);
}
#ifdef __cplusplus
}
#endif

#endif /* TW_OPENSSL_H  */
