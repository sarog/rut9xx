/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twNtlm.h
 *
 * \brief NTLM proxy connection function prototype
*/

#ifndef TW_NTLM_H
#define TW_NTLM_H

#include "twOSPort.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Opens the socket and connects through the proxy specified in the
 * ::twSocket structure.
 *
 * \param[in]     sock      A pointer to the ::twSocket to connect
 * \param[in]     msg       The full Proxy Authorization header initial HTTP
 *                          CONNECT attempt.
 * \param[in]     user      The username for the proxy.
 * \param[in]     password  The password for the proxy.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int NTLM_connectToProxy(twSocket * sock, const char * req, const char * resp, char * user, char * password);

#ifdef __cplusplus
}
#endif

#endif /* TW_NTLM_H */
