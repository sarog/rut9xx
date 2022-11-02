/* 
 * NTLM Authentication functions
 *
 * Copyright (c) 2013 ThingWorx
 *
 * This software is distributed without any warranty.
 */


#include "twOSPort.h"
#include "twLogger.h"

#ifndef _NTLM_H_
#define _NTLM_H_

int32_t
GenerateType1Msg(char **buffer, uint32_t *length);

int32_t
GenerateType3Msg(const char * domain, const char * username, const char * password,
                 const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength);

#endif // _NTLM_H_
