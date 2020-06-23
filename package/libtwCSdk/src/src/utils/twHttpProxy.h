/***************************************
 *  Copyright 2017, PTC, Inc.
 ***************************************/

/**
 * \file twHttpProxy.h
 * \brief HTTP proxy connection function prototype
*/

#ifndef HTTP_PROXY_H
#define HTTP_PROXY_H

#include "twOSPort.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Opens the socket and connects through the proxy specified in the
 * ::twSocket structure.
 *
 * \param[in]     s                 A pointer to the ::twSocket to connect.
 * \param[in]     authCredentials   Authorization credentials to connect with.
 *                                  If NULL, will use the credentials in the
 *                                  \p s ::twSocket structure.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
 *
 * \note The user should always pass NULL in \p authCredentials.
*/
int connectToProxy(twSocket * s, char * authCredentials);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_PROXY_H */
