/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twTls.h
 * \brief ThingWorx TLS client abstraction layer
*/

#ifndef TW_TLS_H
#define TW_TLS_H

#include "twOSPort.h"
#include "twErrors.h"

#define TW_SUBJECT_CN 0
#define TW_SUBJECT_O 1
#define TW_SUBJECT_OU 2
#define TW_ISSUER_CN 3
#define TW_ISSUER_O 4
#define TW_ISSUER_OU 5

#define TW_READ_TIMEOUT -333

#include TW_TLS_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief TLS client structure definition.
*/
typedef struct twTlsClient {
	twSocket * connection; 						/**< A pointer to the ::twSocket associated with the client. **/
	TW_SSL_CTX * ctx;      						/**< A pointer to the context structure associated with the client. **/
	TW_SSL * ssl;          						/**< A pointer to the ssl structure associated with the client. **/
	/* void * session; */
	uint32_t options;      						/**< The TLS options of the client. **/
	char ** x509_data;     			   /**< An array of strings containing the X509_data associated with the client. **/
	twPasswdCallbackFunction keypasswdCallback;      /**< A handler to obtain a key password to use to authenticate. **/
	char * read_buf;       						/**< A read buffer associated with the client. **/
	char selfSignedOk;     						/**< If #TRUE, accept self signed certificates. **/
	char validateCert;     						/**< If #TRUE, validate certificates. **/
	char isEncrypted;      						/**< If #TRUE, the client is encrypted. **/
	char isEnabled;        						/**< If #TRUE, the client is enabled. **/
	TW_MUTEX mtx;          						/**< A #TW_MUTEX associated with the client. **/
} twTlsClient;

/**
 * \brief Creates a new ::twTlsClient structure with the specified settings.
 *
 * \param[in]     host      The host name of the server (::twSocket#host).
 * \param[in]     port      The port the server is listening on (::twSocket#port).
 * \param[in]     options   The TLS options of the ::twTlsClient.
 * \param[out]    client    A pointer to a pointer to store the newly allocated
 *                          ::twTlsClient at.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function will gain ownership of \p client and is
 * responsible for freeing it via twTlsClient_Delete().
*/
int twTlsClient_Create(const char * host, int16_t port, uint32_t options, twTlsClient ** client);

/**
 * \brief Connects a ::twTlsClient#connection (see twSocket_Connect()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to connect.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_Connect(twTlsClient * t);

/**
 * \brief Connects a ::twTlsClient#connection (see twSocket_Connect()) with
 * some additional session options.
 *
 * \param[in]     t                 A pointer to the ::twTlsClient to connect.
 * \param[in]     sessionId         The session ID to assign to the connection.
 * \param[in]     sessionLength     The length of the session.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note  Session ID objects keep internal link information about the session
 * cache list, when being inserted into one #TW_SSL_CTX object's session cache.
 * One SSL_SESSION object, regardless of its reference count, must therefore
 * only be used with one #TW_SSL_CTX object (and the SSL objects created from
 * this #TW_SSL_CTX object).
*/
int twTlsClient_ConnectSession(twTlsClient * t, void * sessionId, int sessionLength);

/**
 * \brief Reconnects a ::twTlsClient#connection (see twSocket_Reconnect()).
 *
 * \param[in,out] t     A pointer to the ::twTlsClient to reconnect.
 * \param[in]     host  The host name of the server (::twSocket#host).
 * \param[in]     port  The port the server is listening on (::twSocket#port).
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_Reconnect(twTlsClient* t, const char * host, int16_t port);

/**
 * \brief Closes a ::twTlsClient#connection (see twSocket_Close()).
 *
 * \param[in]     t     A pointer to the ::twTlsClient to close.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_Close(twTlsClient * t);

/**
 * \brief Reads \p len bytes of data from a ::twTlsClient#connection into \p
 * buf (see twSocket_Read()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to read from.
 * \param[out]    buf       A buffer to store the read data.
 * \param[in]     len       The length of data to read.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for
 * I/O before timing out.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function will retain ownership of \p buf and is
 * responsible for freeing it.
*/
int twTlsClient_Read(twTlsClient * t, char * buf, int len, int timeout);

/**
 * \brief Writes \p len bytes of data from \p buf to a ::twTlsClient#connection
 * (see twSocket_Write()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to write to.
 * \param[out]    buf       A buffer containing the data to be written.
 * \param[in]     len       The length of data to write.
 * \param[in]     timeout   The amount of time (in milliseconds) to wait for
 * I/O before timing out.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The calling function will retain ownership of \p buf and is
 * responsible for freeing it.
*/
int twTlsClient_Write(twTlsClient * t, char * buf, int len, int timeout);

/**
 * \brief Frees all memory associated with a ::twTlsClient and all of its owned
 * substructures.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to delete.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_Delete(twTlsClient * t);

/**
 * \brief Checks to see if a ::twTlsClient is encrypted by getting the value of
 * ::twTlsClient#isEncrypted.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to inspect.
 *
 * \return #TRUE if \p t is encrypted, #FALSE otherwise.
*/
int twTlsClient_IsEncrypted(twTlsClient * t);

/**
 * \brief Gets the session ID of a ::twTlsClient
 *
 * \param[in]     t         A pointer to the ::twTlsClient to get the session
 * ID of.
 *
 * \return Nothing.
 *
 * \note  Session ID objects keep internal link information about the session
 * cache list, when being inserted into one #TW_SSL_CTX object's session cache.
 * One #TW_SSL_SESSION object, regardless of its reference count, must
 * therefore only be used with one #TW_SSL_CTX object (and the SSL objects
 * created from this #TW_SSL_CTX object).
*/
void * twTlsClient_GetSessionId(twTlsClient * t);

/**
 * \brief Sets the ::twTlsClient to accept self signed certificates (see
 * twSocket_SetSelfSignedOk()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 *
 * \return Nothing.
 *
 * \warning This option may induce a serious security risk.
*/
void twTlsClient_SetSelfSignedOk(twTlsClient * t);

/**
 * \brief Sets the ::twTlsClient to disable certificate validation (see
 * twSocket_DisableCertValidation()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 *
 * \return Nothing.
 *
 * \warning This option may induce a serious security risk.
*/
void twTlsClient_DisableCertValidation(twTlsClient * t);

/**
 * \brief Sets the ::twTlsClient to disable encryption (see
 * twSocket_DisableEncryption()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 *
 * \return Nothing.
 *
 * \warning This option may induce a serious security risk.
*/
void twTlsClient_DisableEncryption(twTlsClient * t);

/**
 * \brief Tells the ::twTlsClient to validate its ::twTlsClient#ssl
 * certificate (see TW_VALIDATE_CERT()).
 *
 * \param[in]     t         A pointer to the ::twTlsClient to validate the
 * certificate of.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_ValidateCert(twTlsClient * t);

/**
 * \brief Loads the first certificate stored in \p file into ::twTlsClient#ctx.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 * \param[in]     file      The certificate file to use.
 * \param[in]     type      The container format of  \p file.
 *
 * \return 1 on success, updates error stack on failure.
*/
int twTlsClient_UseCertificateFile(twTlsClient * t, const char *file, int type);

/**
 * \brief Loads the first private key stored in \p file into ::twTlsClient#ctx.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 * \param[in]     file      The private key file to use.
 * \param[in]     type      The container format of  \p file.
 *
 * \return 1 on success, updates error stack on failure.
*/
int twTlsClient_UsePrivateKeyFile(twTlsClient * t, const char *file, int type);

/**
 * \brief Loads the certificate authority cert chain used to validate the
 * server's certificate in \p file into ::twTlsClient#ctx (see
 * TW_USE_CERT_CHAIN_FILE().
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 * \param[in]     file      The certificate chain file to use.
 * \param[in]     type      The container format of \p file.
 *
 * \return 1 on success, updates error stack on failure.
 *
 * \note The certificates must be in PEM format and must be sorted starting
 * with the subject's certificate (actual client or server certificate),
 * followed by intermediate CA certificates if applicable, and ending at the
 * highest level (root).
*/
int	twTlsClient_UseCertificateChainFile(twTlsClient * t, const char *file, int type);

/**
 * \brief Loads a client certificate authority cert chain in \p file into
 * ::twTlsClient#ctx.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 * \param[in]     caFile    The certificate chain file to use.
 * \param[in]     caPath    The container format of \p file.
 *
 * \return 1 on success, updates error stack on failure.
 *
 * \note The certificates must be in PEM format and must be sorted starting
 * with the subject's certificate (actual client or server certificate),
 * followed by intermediate CA certificates if applicable, and ending at the
 * highest level (root).
*/
int twTlsClient_SetClientCaList(twTlsClient * t, char * caFile, char * caPath);

/**
 * \brief Sets the ::twTlsClient#keypasswdCallback of a ::twTlsClient to \p u.
 *
 * \param[in]     t         A pointer to the ::twTlsClient to modify.
 * \param[in]     u         The password callback function.
 *
 * \return Nothing.
*/
void twTlsClient_SetDefaultPasswdCb(twTlsClient * t, twPasswdCallbackFunction u);

/**
 * \brief Sets the X509 fields of a ::twTlsClient.
 *
 * \param[in,out] t             A pointer to the ::twTlsClient to modify.
 * \param[in]     subject_cn    The common name of the subject.
 * \param[in]     subject_o     The organization of the subject.
 * \param[in]     subject_ou    The organizational unit of the subject.
 * \param[in]     issuer_cn     The common name of the issuer.
 * \param[in]     issuer_o      The organization of the issuer.
 * \param[in]     issuer_ou     The organizational unit of the issuer.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsClient_SetX509Fields(twTlsClient * t, char * subject_cn, char * subject_o, char * subject_ou,
							  char * issuer_cn, char * issuer_o, char * issuer_ou);

/**
 * \brief Creates a new #TW_TLS server and associates it with a
 * ::twTlsClient#ssl.
 *
 * \param[in,out] t         A pointer to the ::twTlsClient to modify.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsServer_Create(twTlsClient * t);

/**
 * \brief Waits for a ::twTlsClient#ssl to initiate a handshake with the
 * server (see TW_SSL_ACCEPT()).
 *
 * \param[in]    t          A pointer to the ::twTlsClient to utilize.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTlsServer_Accept(twTlsClient *t);

/**
 * \note SHA1 for the websocket accept header.  If not defined
 *        by the TLS library, use built in libTomcrypt
*/
#ifndef TW_SHA1_CTX
#include "tomcrypt.h"
#define TW_SHA1_CTX						hash_state
#define TW_SHA1_INIT(a)					sha1_init(a)
#define TW_SHA1_UPDATE(a,b,c)			sha1_process(a,b,c)
#define TW_SHA1_FINAL(a,b)				sha1_done(b,a)
#endif

/**
 * \brief Initializes an SHA1 context.
 *
 * \param[in]    ctx          A pointer to the ctx structure to initialize.
 *
 * \return nothing
*/
void twSHA1_Init(TW_SHA1_CTX * ctx);

/**
 * \brief Add a new entry to an SHA1 hash.
 *
 * \param[in]    ctx          A pointer to the ctx structure.
 * \param[in]    msg          A pointer to the message to add.
 * \param[in]    len          The length of the message.
 *
 * \return nothing
*/
void twSHA1_Update(TW_SHA1_CTX * ctx, const uint8_t * msg, int len);

/**
 * \brief Finalize and get the calculated SHA1 digest.
 *
 * \param[in,out] digest       A pointer to the buffer to receive the digest.
 * \param[in]     ctx          A pointer to the ctx structure.
 *
 * \return nothing
*/
void twSHA1_Final(uint8_t *digest, TW_SHA1_CTX * ctx);

/**
 * \note SHA1 for the websocket accept header.  If not defined
 *        by the TLS library, use built in libTomcrypt
*/
#ifndef TW_MD5_CTX
#include "tomcrypt.h"
#define TW_MD5_CTX						hash_state
#define TW_MD5_INIT(a)					md5_init(a)
#define TW_MD5_UPDATE(a,b,c)			md5_process(a,b,c)
#define TW_MD5_FINAL(a,b)				md5_done(b,a)
#endif

/**
 * \brief Initializes an MD5 context.
 *
 * \param[in]    ctx          A pointer to the ctx structure to initialize.
 *
 * \return nothing
*/
void twMD5_Init(TW_MD5_CTX *);

/**
 * \brief Add a new entry to an MD5 hash.
 *
 * \param[in]    ctx          A pointer to the ctx structure.
 * \param[in]    msg          A pointer to the message to add.
 * \param[in]    len          The length of the message.
 *
 * \return nothing
*/
void twMD5_Update(TW_MD5_CTX *, const uint8_t *msg, int len);

/**
 * \brief Finalize and get the calculated MD5 digest.
 *
 * \param[in,out] digest       A pointer to the buffer to receive the digest.
 * \param[in]     ctx          A pointer to the ctx structure.
 *
 * \return nothing
*/
void twMD5_Final(uint8_t *digest, TW_MD5_CTX *);

#ifdef __cplusplus
}
#endif

#endif
