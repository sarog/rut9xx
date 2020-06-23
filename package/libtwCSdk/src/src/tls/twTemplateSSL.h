/**
 * \file twTemplateSSL.h
 * \brief A template for a SSL/TLS wrapper layer
 */

#ifndef TW_C_SDK_TWTEMPLATESSL_H
#define TW_C_SDK_TWTEMPLATESSL_H

#include "twOSPort.h"
#include "twLogger.h"
#include "stringUtils.h"

#include "stdio.h"
#include "string.h"

#include "path/to/my/ssl/header.h"

/**
 * \brief The base SSL structure for your SSL library
 */
#define TW_SSL                            my_ssl_struct
/**
 * \brief Calls a function to free the #TW_SSL structure
 *
 * \param[in]	a	A pointer to the #TW_SSL structure to be freed
 */
#define TW_SSL_FREE(a)                    my_ssl_struct_delete(a)
/**
 * \brief The session ID of the #TW_SSL structure
 *
 * \param[in]	a	A pointer to the #TW_SSL structure to get the ID of
 */
#define TW_SSL_SESSION_ID(a)            my_ssl_struct->session_id

/**
 * \brief The SSL context structure for your SSL library
 */
#define TW_SSL_CTX                        my_ssl_struct_ctx
/**
 * \brief Calls a function to create a new #TW_SSL_CTX structure
 */
#define TW_NEW_SSL_CTX                    my_ssl_struct_ctx_create()
/**
 * \brief Calls a function to free a #TW_SSL_CTX structure
 *
 * \param[in]	a	A pointer to the #TW_SSL_CTX structure to be freed
 */
#define TW_SSL_CTX_FREE(a)                my_ssl_struct_ctx_delete()

/**
 * \brief Calls a function to load the first certificate stored in a file into the #TW_SSL_CTX structure
 *
 * \param[in]	a	A pointer to the #TW_SSL_CTX structure to load the certificate into
 * \param[in]	b	The path of the certificate file to load from
 * \param[in]	c	The container format of the file
 */
#define TW_USE_CERT_FILE(a, b, c)            my_ssl_struct_ctx_load_object(a,b,c)

/**
 * \brief Calls a function to load the first private key stored in a file into the #TW_SSL_CTX structure
 *
 * \param[in]	a	A pointer to the #TW_SSL_CTX structure to load the private key into
 * \param[in]	b	The path of the private key file to load from
 * \param[in]	c	The container format of the file
 */
#define TW_USE_KEY_FILE(a, b, c, d)        my_ssl_struct_ctx_load_object(a,b,c)

/**
 * \brief Calls a function to load the certificate authority cert chain used to validate the server's certificate into
 * the #TW_SSL_CTX structure
 *
 * \param[in]	a	A pointer to the #TW_SSL_CTX structure to load the certificate authority cert chain into
 * \param[in]	b	The path of the certificate authority cert chain file to load from
 * \param[in]	c	The container format of the file
 *
 * \note The certificates must be in PEM format and must be sorted starting with the subject's certificate (actual
 * client or server certificate), followed by intermediate CA certificates if applicable, and ending at the highest
 * level (root)
 */
#define TW_USE_CERT_CHAIN_FILE(a, b, c)    my_ssl_struct_ctx_load_object(a,b,c)

/**
 * \brief Calls a function to load a client certificate authority cert chain into the #TW_SSL_CTX structure
 *
 * \param[in]	a	A pointer to the #TW_SSL_CTX structure to load the client certificate authority cert chain into
 * \param[in]	b	The path of the client certificate authority cert chain file to load from
 *
 * \note The certificates must be in PEM format and must be sorted starting
 * with the subject's certificate (actual client or server certificate),
 * followed by intermediate CA certificates if applicable, and ending at the
 * highest level (root)
 */
#define TW_SET_CLIENT_CA_LIST(a, b)        my_ssl_struct_ctx_load_object(a,b,NULL)

/**
 * \brief Calls a function that determines if the SSL library is FIPS mode compatible
 *
 *
 * \returns #TW_OK if FIPS mode is supported,  positive integral on error code (see twErrors.h) if an error was
 * encountered
 */
#define TW_IS_FIPS_COMPATIBLE()        returnValue(TW_FIPS_MODE_NOT_SUPPORTED)

/**
 * \brief Enables FIPS mode
 *
 * \returns #TW_OK if the operation was successful, positive integral on error code (see twErrors.h) if an error was
 * encountered
 */
#define TW_ENABLE_FIPS_MODE()            returnValue(TW_FIPS_MODE_NOT_SUPPORTED)

/**
 * \brief Disables FIPS mode
 *
 * \returns #TW_OK if the operation was successful, positive integral on error code (see twErrors.h) if an error was
 * encountered
 */
#define TW_DISABLE_FIPS_MODE()            returnValue(TW_FIPS_MODE_NOT_SUPPORTED)

/**
 * \brief Calls a function that determines if the SSL library is running with FIPS mode
 *
 *
 * \returns TRUE if FIPS mode is enabled, FALSE if FIPS mode is disabled
 */
#define TW_IS_FIPS_MODE_ENABLED()		returnValue(FALSE)

/**
 * \brief Output ssl library version
 *
 * \returns #Return the numerical version of the ssl library
 */
#define TW_SSL_VERSION()            returnValue(NULL)


/**
 * \brief The structure for your SSL implementation's SHA1 hash
 */
#define TW_SHA1_CTX                        hash_state
/**
 * \brief Calls a function to initalize a SHA1 hash
 *
 * \param[in]	a	A pointer to a #TW_SHA1_CTX to be initalized
 */
#define TW_SHA1_INIT(a)                    sha1_init(a)
/**
 * \brief Calls a function to process a SHA1 hash
 *
 * \param[in]	a	A pointer to a #TW_SHA1_CTX to be processed
 * \param[in]	b	A pointer to the message to add to the hash
 * \param[in]	c	The length of the message
 *
 * /note SHA1 processes an arbitrary-length message into a fixed-length output of 160 bits, typically represented as a
 * sequence of 40 hexadecimal digits
 */
#define TW_SHA1_UPDATE(a, b, c)            sha1_process(a,b,c)
/**
 * \brief Finalize and get the calculated SHA1 digest
 *
 * \param[in]	a	A pointer to the buffer to recieve the digest
 * \param[in]	b	A pointer to a #TW_SHA1_CTX to finalize
 */
#define TW_SHA1_FINAL(a, b)                sha1_done(b,a)

/**
 * \brief The structure for your SSL implementation's MD5 hash
 */
#define TW_MD5_CTX                        hash_state
/**
 * \brief Calls a function to initalize a MD5 hash
 *
 * \param[in]	a	A pointer to a #TW_MD5_CTX to be initalized
 */
#define TW_MD5_INIT(a)                    sha1_init(a)
/**
 * \brief Calls a function to process a MD5 hash
 *
 * \param[in]	a	A pointer to a #TW_MD5_CTX to be processed
 * \param[in]	b	A pointer to the message to add to the hash
 * \param[in]	c	The length of the message
 *
 * \note MD5 processes an arbitrary-length message into a fixed-length output of 128 bits, typically represented as a
 * sequence of 32 hexadecimal digits
 */
#define TW_MD5_UPDATE(a, b, c)            sha1_process(a,b,c)
/**
 * \brief Finalize and get the calculated MD5 digest
 *
 * \param[in]	a	A pointer to the buffer to recieve the digest
 * \param[in]	b	A pointer to a #TW_MD5_CTX to finalize
 */
#define TW_MD5_FINAL(a, b)                sha1_done(b,a)

#endif //TW_C_SDK_TWTEMPLATESSL_H
