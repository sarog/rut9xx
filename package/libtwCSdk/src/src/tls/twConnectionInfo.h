/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twConnectionInfo.h
 * \brief ThingWorx tunneling connection information structure definition and functions.
*/

#ifndef TW_CONNECTION_INFO_H
#define TW_CONNECTION_INFO_H

#include "twOSPort.h"
#include "twPasswds.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Tunnel Connection Details Structure Definition
*/
typedef struct twConnectionInfo {
	/* Host Info */
	char * ws_host;                /**< The name of the websocket host server. **/
	uint16_t ws_port;               /**< The port the websocket host server is listening on. **/
	twPasswdCallbackFunction appkeyFunction;
	/** < This is a pointer to a function which will be called whenever the client must
	 * 		provide an application key for authentication. You must implement this function
 	 * 		and pass a pointer to it to initialize the API. This function must be of type
 	 * 		twPasswdCallbackFunction and should have a format like the one shown below.
 	 *
  	 * 		myPasswordCallback(char* passwdBuffer,unsigned int maxPasswordLength);
 	 *
	 * 		The implementation of this function must fill passwdBuffer with the current
	 * 		application key. The app key used to authenticate. **/
	/* Proxy info */
	char * proxy_host;             /**< The name of the proxy server. **/
	uint16_t proxy_port;           /**< The port the proxy server is listening on. **/
	char * proxy_user;             /**< The username to use to authenticate with the proxy server. **/
	twPasswdCallbackFunction proxy_pwd; /**< The password callback to use to authenticate with the proxy server. **/
	/* Cert info */
	char * subject_cn;             /**< The common name of the subject in the certificate. **/
	char * subject_o;              /**< The organization of the subject in the certificate. **/
	char * subject_ou;             /**< The organizational unit of the subject in the certificate. **/
	char * issuer_cn;              /**< The common name of the issuer in the certificate. **/
	char * issuer_o;               /**< The organization of the issuer in the certificate. **/
	char * issuer_ou;              /**< The organizational unit of the issuer in the certificate. **/
	char * ca_cert_file;           /**< The certificate authority's cert file. **/
	char * client_cert_file;       /**< The client's cert file. **/
	char * client_key_file;        /**< The client's key file. **/
	twPasswdCallbackFunction client_key_passphrase;  /**< The client's callback to obtain a key's passphrase. **/
	char selfsignedOk;             /**< If #TRUE, accept self-signed certificates. **/
	char doNotValidateCert;        /**< If #TRUE, do not validate certificates (dangerous). **/
	char disableEncryption;        /**< If #TRUE, disable all encryption (very dangerous). **/
} twConnectionInfo;

/**
 * \brief Creates a new ::twConnectionInfo structure, optionally copying
 * settings of an existing ::twConnectionInfo structure.
 *
 * \param[in]     copy      An optional ::twConnectionInfo structure to copy
 *                          (NULL uses default settings).
 *
 * \return A pointer to the newly allocated ::twConnectionInfo structure.
 *
 * \note The calling function will gain ownership of the returned
 * ::twConnectionInfo structure and is responsible for freeing it via
 * twConnectionInfo_Delete().
*/
twConnectionInfo * twConnectionInfo_Create(twConnectionInfo * copy);

/**
 * \brief Frees all memory associated with a ::twConnectionInfo structure and
 * all of its owned substructures.
 *
 * \param[in]     info      A pointer to the ::twConnectionInfo structure to be
 *                          deleted.
 *
 * \return Nothing.
*/
void twConnectionInfo_Delete(void * info);

#ifdef __cplusplus
}
#endif

#endif
