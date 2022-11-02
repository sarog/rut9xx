/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twTunnelManager.h
 * \brief ThingWorx tunneling structure definitions and function prototypes.
 *
 * Contains structure type definitions and function prototypes for the ThingWorx API
 * tunneling operations.
*/

#ifndef TUNNELMANAGER_H /* Prevent multiple inclusions. */
#define TUNNELMANAGER_H

#include "twOSPort.h"
#include "twDefinitions.h"
#include "twDefaultSettings.h"
#include "twList.h"
#include "twInfoTable.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Signature of a callback function that is registered to be called when
 * a tunnel state changes.
 *
 * \param[in]     started       #TRUE if the tunnel is started, #FALSE if the
 *                              tunnel has ended.
 * \param[in]     tid           Unique ID of the tunnel.
 * \param[in]     thingName     The name of the thing the tunnel is targeted
 *                              at.
 * \param[in]     peerName      The name of the peer user of the tunnel.
 * \param[in]     host          The hostname of the local connection that is
 *                              tunneled to.
 * \param[in]     port          The port number of the local connection that is
 *                              tunneled to.
 * \param[in]     startTime     The time the tunnel started (0 if it hasn't
 *                              started).
 * \param[in]     endTime       The time the tunnel ended (0 if it hasn't
 *                              ended).
 * \param[in]     bytesSent     The total number of bytes that were sent to the
 *                              peer.
 * \param[in]     bytesRcvd     The total number of bytes that were received
 *                              from the peer.
 * \param[in]     type          The type of the tunnel.
 * \param[in]     userdata      An opaque pointer that is passed in when the
 *                              callback is registered.
 *
 * \return Nothing.
 *
 * \note The calling function will gain ownership of all pointers associated
 * with the tunnel and is responsible for freeing them.
*/
typedef void (*tunnel_cb) (char started, const char * tid, const char * thingName, const char * peerName,
						   const char * host, int16_t port, DATETIME startTime,  DATETIME endTime, 
						   uint64_t bytesSent, uint64_t bytesRcvd, const char * type, const char * msg, void * userdata);

typedef twList twActiveTunnelList;

/**
 * \brief Active tunnel structure definition.
*/
typedef struct twActiveTunnel {
	char * thingName;   /**< Name of the entity associated with the tunnel. **/
	char * tid;         /**< Tunnel ID for this tunnel. **/
	char * peerName;    /**< Name of the entity that initiated the tunnel (may be null if still active **/
	char * host;        /**< Host or file name of the tunnel target. **/
	uint16_t port;      /**< Port of the tunnel target. **/
	DATETIME startTime; /**< Start time of the tunnel. **/
	DATETIME endTime;   /**< End time of the tunnel - should be 0 for an active tunnel. **/
	char * type;        /**< Type of the tunnel - TCP, FILE, UDP, or SERIAL. **/
} twActiveTunnel;

/**
 * \brief Creates the ::twTunnelManager singleton.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The ::twTunnelManager singleton must be freed via
 * twTunnelManager_Delete().
*/
int twTunnelManager_Create();

/**
 * \brief Frees all memory associated with the ::twTunnelManager singleton and all
 * its owned substructures.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTunnelManager_Delete();

/**
 * \brief Executes all functions required for proper operation of tunnels.
 * optimized for the tasker to provide safe single threaded tunnel functionality
 *
 * \param[in]     now       The current timestamp.
 * \param[in]     params    Required by the tasker function signature but
 *                          currently not used.
 *
 * \return Nothing.
*/
void twTunnelManager_TaskerFunction(DATETIME now, void * params);

/**
 * \brief Executes all functions required for proper operation of tunnels.
 * optimized for threads to provide safe performant tunnels
 *
 * \param[in]     now       The current timestamp.
 * \param[in]     params    Required by the tasker function signature but
 *                          currently not used.
 *
 * \return Nothing.
*/
void twTunnelManager_ThreadFunction(DATETIME now, void * params);

/**
 * \brief Shuts down all tunnels.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTunnelManager_StopAllTunnels();

/**
 * \brief Updates the host, port, and appkey of the tunnels related to
 * connecting to the server.
 *
 * \param[in]     host      The new host name to use.
 * \param[in]     port      The new port to use.
 * \param[in]     appkey    The new appkey to use.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTunnelManager_UpdateTunnelServerInfo(char * host, uint16_t port, char * appkey);

/**
 * \brief Registers a function to be called when a tunnel state changes.
 *
 * \param[in]     cb        The function to register.
 * \param[in]     id        ID of tunnel to callback on state change ("*" or
 *                          NULL signifies all tunnels).
 * \param[in]     userdata  An opaque pointer that is passed in when the
 *                          callback is registered.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTunnelManager_RegisterTunnelCallback(tunnel_cb cb, char * id, void * userdata);

/**
 * \brief Unregisters a tunnel callback function that was registered via
 * twTunnelManager_RegisterTunnelCallback().
 *
 * \param[in]     cb        Pointer to the function to be unregistered.
 * \param[in]     id        ID of the tunnel that the callback was registered
 *                          to.
 * \param[in]     userdata  An opaque pointer that is passed in when the
 *                          callback is registered.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int twTunnelManager_UnregisterTunnelCallback(tunnel_cb cb, char * id, void * userdata);

/**
 * \brief Return a list of the currently active tunnels.
 *
 * \return A pointer to a ::twList of the currently active tunnels.  NULL if an
 * error was encountered.
 *
 * \note The calling function will gain ownership of the returned ::twList and
 * is responsible for freeing it via twList_Delete().
*/
twActiveTunnelList * twTunnelManager_ListActiveTunnels();

/**
 * \brief Sets the proxy information to be used when making a connection.
 *
 * \param[in]     proxyHost             The host name of the proxy.
 * \param[in]     proxyPort             The port used by the proxy.
 * \param[in]     proxyUser             The username to supply to the proxy (NULL if
 *                                      authentication is disabled).
 * \param[in]     proxyPassCallback     The callback function used to obtain the password to supply
 *                                      to the proxy.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
void twTunnelManager_SetProxyInfo(char * proxyHost, uint16_t proxyPort, char * proxyUser, twPasswdCallbackFunction proxyPassCallback);

/**
 * \brief Passthru to notify the TLS library to accept self-signed
 * certificates.
 *
 * \return Nothing.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_SetSelfSignedOk(char state);

/**
 * \brief Passthru to notify the TLS library to not validate the server
 * certificate.
 *
 * \return Nothing.
 *
 * \warning Disabling certificate validation may induce a serious security
 * risk.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_DisableCertValidation(char state);

/**
 * \brief Passthru to notify the TLS library to disable encryption.
 *
 * \return Nothing.
 *
 * \warning Disabling encryption may induce a serious security risk.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_DisableEncryption(char state);

/**
 * \brief Defines which fields of an X509 certificate will be validated.  NULL
 * parameteres will not be checked against the deceived certificate.
 *
 * \param[in]     subject_cn    The common name of the subject in the
 *                              certificate.
 * \param[in]     subject_o     The organization of the subject in the
 *                              certificate.
 * \param[in]     subject_ou    The organizational unit of the subject in the
 *                              certificate.
 * \param[in]     issuer_cn     The common name of the issuer in the
 *                              certificate.
 * \param[in]     issuer_o      The organization of the issuer in the
 *                              certificate.
 * \param[in]     issuer_ou     The organizational unit of the issuer in the
 *                              certificate.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
void twTunnelManager_SetX509Fields(char * subject_cn, char * subject_o, char * subject_ou,
							  char * issuer_cn, char * issuer_o, char * issuer_ou);

/**
 * \brief Loads the local PEM or DER formatted certificate file used to
 * validate the server.
 *
 * \param[in]     file      The full path to the file containing the
 *                          certificate.
 * \param[in]     type      Definition is dependent on the underlying TLS
 *                          library (may be set to 0 for AxTLS).
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_LoadCACert(const char *file, int type);

/**
 * \brief Loads the local PEM or DER formatted certificate file used to
 * validate the client to the server.
 *
 * \param[in]     file      The full path to the file containing the
 *                          certificate.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_LoadClientCert(char *file);

/**
 * \brief Sets the key of the client.
 *
 * \param[in]     file          The full path to the file containing the key.
 * \param[in]     passphraseCallback    The passphrase used to open the file.
 * \param[in]     type          Definition is dependent on the underlying TLS
 *                              library (may be set to 0 for AxTLS).
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note By default the ::twTunnelManager will utilize the same
 * ::twConnectionInfo structure as the ::twApi and thus all ::twConnectionInfo
 * settings will be shared by the ::twApi and the ::twTunnelManager. If this
 * function is called then a new ::twConnectionInfo structure will be allocated
 * for the ::twTunnelManager, assigned the current values of the ::twApi
 * ::twConnectionInfo structure, and then updated by this function. Therefore,
 * after this function is called, any ::twConnectionInfo settings applied to
 * the ::twApi will *not* be reflected in the ::twTunnelManager's
 * ::twConnectionInfo settings.
*/
void twTunnelManager_SetClientKey(const char *file, twPasswdCallbackFunction passphraseCallback, int type);


/* Tunnel callback for handling all tunneling services. */
enum msgCodeEnum tunnelServiceCallback(const char * entityName, const char * serviceName, twInfoTable * params, twInfoTable ** content) ;

#ifdef __cplusplus
}
#endif

#endif
