/* 
 * NTLM Authentication functions
 *
 * Copyright (c) 2013 ThingWorx
 *
 * This software is distributed without any warranty.
 */
//-----------------------------------------------------------------------------
// Code based on: http://davenport.sourceforge.net/ntlm.html
//-----------------------------------------------------------------------------

#include "ntlm.h"
#include "stringUtils.h"
#include "crypto_wrapper.h"
#include "tomcrypt.h"

#define NTLM_NegotiateUnicode               0x00000001
#define NTLM_NegotiateOEM                   0x00000002
#define NTLM_RequestTarget                  0x00000004
#define NTLM_Unknown1                       0x00000008
#define NTLM_NegotiateSign                  0x00000010
#define NTLM_NegotiateSeal                  0x00000020
#define NTLM_NegotiateDatagramStyle         0x00000040
#define NTLM_NegotiateLanManagerKey         0x00000080
#define NTLM_NegotiateNetware               0x00000100
#define NTLM_NegotiateNTLMKey               0x00000200
#define NTLM_Unknown2                       0x00000400
#define NTLM_Unknown3                       0x00000800
#define NTLM_NegotiateDomainSupplied        0x00001000
#define NTLM_NegotiateWorkstationSupplied   0x00002000
#define NTLM_NegotiateLocalCall             0x00004000
#define NTLM_NegotiateAlwaysSign            0x00008000
#define NTLM_TargetTypeDomain               0x00010000
#define NTLM_TargetTypeServer               0x00020000
#define NTLM_TargetTypeShare                0x00040000
#define NTLM_NegotiateNTLM2Key              0x00080000
#define NTLM_RequestInitResponse            0x00100000
#define NTLM_RequestAcceptResponse          0x00200000
#define NTLM_RequestNonNTSessionKey         0x00400000
#define NTLM_NegotiateTargetInfo            0x00800000
#define NTLM_Unknown4                       0x01000000
#define NTLM_NegotiateVersion               0x02000000
#define NTLM_Unknown6                       0x04000000
#define NTLM_Unknown7                       0x08000000
#define NTLM_Unknown8                       0x10000000
#define NTLM_Negotiate128                   0x20000000
#define NTLM_NegotiateKeyExchange           0x40000000
#define NTLM_Negotiate56                    0x80000000

// Define the flags we initially send to the server
#define NTLM_TYPE1_FLAGS      \
  (NTLM_NegotiateUnicode |    \
   NTLM_RequestTarget |       \
   NTLM_NegotiateNTLMKey |    \
   NTLM_NegotiateAlwaysSign |  \
   NTLM_NegotiateWorkstationSupplied)

#define NTLM_TYPE1_LENGTH 32
#define NTLM_TYPE2_LENGTH 32
#define NTLM_TYPE3_LENGTH 64

#define LM_RESP_LENGTH 24
#define NTLM_HASH_LENGTH 16
#define NTLM_RESP_LENGTH 24

static const char NTLM_SIGNATURE[] = "NTLMSSP";
static const char NTLM_TYPE1_MSG_ID[] = { 0x01, 0x00, 0x00, 0x00 };
static const char NTLM_TYPE2_MSG_ID[] = { 0x02, 0x00, 0x00, 0x00 };
static const char NTLM_TYPE3_MSG_ID[] = { 0x03, 0x00, 0x00, 0x00 };


// Big Endian byte order swapping
#define SWAP16(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#define SWAP32(x) ((SWAP16((x) & 0xffff) << 16) | (SWAP16((x) >> 16)))

// Predefine the Type2 Message Structure
struct Type2Msg
{
  uint32_t    flags;         
  uint8_t     challenge[8];  
  const char *target;       
  uint32_t    targetLength;     
};

/**
 * Logs the flags indiviually
 */
void LogFlags(int * tmp, char * mag)
{
	char flagStr[128];
	if (mag == NULL) {
		TW_LOG(TW_ERROR,"NTLM_LogFlags: NULL pointer passed in");
		return;
	}
	memset (flagStr,0,64);
	snprintf(flagStr, 80, "Flags: 0x%x\n", *tmp);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateUnicode: %d\n", (*tmp & NTLM_NegotiateUnicode) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateOEM: %d\n", (*tmp & NTLM_NegotiateOEM) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "RequestTarget: %d\n", (*tmp & NTLM_RequestTarget) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown1: %d\n", (*tmp & NTLM_Unknown1) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateSign: %d\n", (*tmp & NTLM_NegotiateSign) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateSeal: %d\n", (*tmp & NTLM_NegotiateSeal) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateDatagramStyle: %d\n", (*tmp & NTLM_NegotiateDatagramStyle) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateLanManagerKey: %d\n", (*tmp & NTLM_NegotiateLanManagerKey) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateNetware: %d\n", (*tmp & NTLM_NegotiateNetware) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateNTLMKey: %d\n", (*tmp & NTLM_NegotiateNTLMKey) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown2: %d\n", (*tmp & NTLM_Unknown2) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown3: %d\n", (*tmp & NTLM_Unknown3) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateDomainSupplied: %d\n", (*tmp & NTLM_NegotiateDomainSupplied) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateWorkstationSupplied: %d\n", (*tmp & NTLM_NegotiateWorkstationSupplied) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateLocalCall: %d\n", (*tmp & NTLM_NegotiateLocalCall) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateAlwaysSign: %d\n", (*tmp & NTLM_NegotiateAlwaysSign) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "TargetTypeDomain: %d\n", (*tmp & NTLM_TargetTypeDomain) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "TargetTypeServer: %d\n", (*tmp & NTLM_TargetTypeServer) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "TargetTypeShare: %d\n", (*tmp & NTLM_TargetTypeShare) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateNTLM2Key: %d\n", (*tmp & NTLM_NegotiateNTLM2Key) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "RequestInitResponse: %d\n", (*tmp & NTLM_RequestInitResponse) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "RequestAcceptResponse: %d\n", (*tmp & NTLM_RequestAcceptResponse) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "RequestNonNTSessionKey: %d\n", (*tmp & NTLM_RequestNonNTSessionKey) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateTargetInfo: %d\n", (*tmp & NTLM_NegotiateTargetInfo) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown4: %d\n", (*tmp & NTLM_Unknown4) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateVersion: %d\n", (*tmp & NTLM_NegotiateVersion) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown6: %d\n", (*tmp & NTLM_Unknown6) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown7: %d\n", (*tmp & NTLM_Unknown7) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Unknown8: %d\n", (*tmp & NTLM_Unknown8) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Negotiate128: %d\n", (*tmp & NTLM_Negotiate128) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "NegotiateKeyExchange: %d\n", (*tmp & NTLM_NegotiateKeyExchange) ? 1 : 0);
	strcat(mag, flagStr);
	snprintf(flagStr, 64, "Negotiate56: %d\n", (*tmp & NTLM_Negotiate56) ? 1 : 0);
	strcat(mag, flagStr);
}

// Log a binary buffer in hex format
void LogHex(const char *name, const void *token, uint32_t tokenLen)
{
	char * msg = (char *)TW_CALLOC(4096,1);
	int len = tokenLen;
	int i = 0;
	char hex[16];
	if (!msg) {
		TW_LOG(TW_ERROR,"NTLM_LogHex: Error allocating memory");
		return;
	}
	if (tokenLen > 150) len = 150; // Need to break this up because logger cuts it off
	for (i = 0; i < len; i++) {
		sprintf(hex,"%02x", ((const unsigned char *)token)[i]);
		strcat(msg, hex);
	}
	TW_LOG(TW_TRACE, "NTLM_PROXY: %s: 0x%s", name, msg);
	if (tokenLen > 150) {
		uint32_t i = 0;
		strcpy(msg,"");
		for (i = 150; i < tokenLen; i++) {
			sprintf(hex,"%02x", ((const unsigned char *)token)[i]);
			strcat(msg, hex);
		}
		TW_LOG(TW_TRACE, "NTLM_PROXY %s (Part 2): 0x%s", name, msg);
	}
	TW_FREE(msg);
}

// Log a Type 1 Message
static void LogType1Msg(const char * buffer, int length) {
	char * msg = (char *)TW_CALLOC(4096,1);
	int i = 0;
	char tmp[64];
	int * flags = 0;
	int type = 0;
	if (!msg || !buffer) {
		TW_LOG(TW_ERROR,"LogType1Msg: Error allocating memory");
		return;
	}
	strcpy(msg,"Signature: ");
	strncat(msg, buffer + i, 7);
	i += 8;
	type = *(int *)(buffer + i);
	strcat(msg,"\nMessage Type: 0x%x");
	snprintf(tmp, 20, "%x\n", type);
	strcat(msg, tmp);
	i += 4;
	flags = (int *)(buffer + i);
	LogFlags(flags, msg);
	TW_LOG(TW_TRACE,"NTLM_PROXY: NTLM Type 1 Message:\n%s", msg);
	TW_FREE(msg);
}

// Log a Type 2 Message
void LogType2Msg(struct Type2Msg * msg) {
	char * tmp = (char *)TW_CALLOC(4096,1);
	int i = 0;
	char * targetBuffer = (char *)(msg->target);
	char buf[16];
	if (!msg || !tmp ) {
		TW_LOG(TW_ERROR,"LogType2Msg: Error allocating memory");
		if (tmp) TW_FREE(tmp);
		return;
	}
	memset(buf,0,16);
	strcpy(tmp, "Target Raw: 0x");
	for (i = 0; i < msg->targetLength; i++) {
		snprintf(buf,2,"%02x", targetBuffer[i]);
		strcat(tmp, buf);
	}
	strcat(tmp, "\nTarget: ");
	for (i = 0; i < msg->targetLength; i++) {
		if (!(i%2)){
			/* Unicode to ascii hack */
			snprintf(buf,2,"%c", targetBuffer[i]);
			strcat(tmp, buf);
		}
	}
	strcat(tmp,"\n");
	LogFlags((int *)&msg->flags, tmp);
	strcat(tmp, "Challenge 0x");
	for (i = 0; i < sizeof(msg->challenge); i++) {
		snprintf(buf,2,"%02x", msg->challenge[i]);
		strcat(tmp, buf);
	}
	strcat(tmp, "\n");
	TW_LOG(TW_TRACE,"NTLM_PROXY: NTLM Type 2 Message:\n%s", tmp);
	TW_FREE(tmp);
}


//-----------------------------------------------------------------------------

char *
AddToMessageBuffer(char *buf, const void *data, uint32_t dataLen)
{
	if (!buf || !data) {
		TW_LOG(TW_ERROR,"NTLM_AddToMessageBuffer: Null pointer passed in");
		return NULL;
	}
	memcpy(buf, data, dataLen);
	return buf + dataLen;
}

char *
WriteFlags(char *buf, uint32_t flags)
{
	if (!buf) {
		TW_LOG(TW_ERROR,"NTLM_WriteFlags: Null pointer passed in");
		return NULL;
	}
#ifdef BIG_ENDIAN_CPU 
	TW_LOG(TW_TRACE,"NTLM_WriteFlags: ############# SWAPPING FLAG BYTES ###########");
    flags = SWAP32(flags);
#endif
    return AddToMessageBuffer(buf, &flags, sizeof(flags));
}

char *
SecurityBuffer(char *buf, uint16_t length, uint32_t offset)
{
	if (!buf) {
		TW_LOG(TW_ERROR,"NTLM_WriteFlags: Null pointer passed in");
		return NULL;
	}
#ifdef BIG_ENDIAN_CPU
	length = SWAP16(length);
	offset = SWAP32(offset);
#endif
	buf = AddToMessageBuffer(buf, &length, sizeof(length));
	buf = AddToMessageBuffer(buf, &length, sizeof(length));
	buf = AddToMessageBuffer(buf, &offset, sizeof(offset));
	return buf;
}

char * convertUnicodeStringToLE(char * input) {
	int i = 0;
	if (!input) {
		TW_LOG(TW_ERROR,"NTLM_convertUnicodeStringToLE: Null pointer passed in");
		return NULL;
	}
	for (i = 0; i < strlen(input); i += 2) {
		char tmp = input[i];
		input[i] = input[i+1];
		input[i+1] = tmp;
	}
	return input;
}

uint16_t ReadUint16(const uint8_t ** buf)
{
	uint16_t x = 0;
	if (!buf || !*buf) {
		TW_LOG(TW_ERROR,"NTLM_ReadUint16: Null pointer passed in");
		return 0;
	}
	x = ((uint16_t) (*buf)[0]) | ((uint16_t) (*buf)[1] << 8);
	*buf += sizeof(x);
	return x;
}

uint32_t ReadUint32(const uint8_t **buf)
{
	uint32_t x = 0;;
	if (!buf || !*buf) {
		TW_LOG(TW_ERROR,"NTLM_ReadUint32: Null pointer passed in");
		return 0;
	}
	x = ( (uint32_t) (*buf)[0])        |
				(((uint32_t) (*buf)[1]) << 8)  |
				(((uint32_t) (*buf)[2]) << 16) |
				(((uint32_t) (*buf)[3]) << 24);
	*buf += sizeof(x);
	return x;
}

/**
 * Calculate the NTLM Hash
 */
void NTLM_Hash(char * password, unsigned char *hash, uint32_t len)
{
	hash_state md4;
	if (!password || !hash) {
		TW_LOG(TW_ERROR,"NTLM_Hash: Null pointer passed in");
		return;
	}

#ifdef BIG_ENDIAN_CPU
	password = convertUnicodeStringToLE(password);
#endif
	md4_init(&md4);
	md4_process(&md4, (unsigned char *)password, len);
	md4_done(&md4, hash);
	LogHex("NTLM_HASH", hash, 16);
}

//-----------------------------------------------------------------------------

/** 
 * Calculate the response
 */

static void
NTLM_Response(const uint8_t *hash, const uint8_t *challenge, uint8_t *response)
{
  uint8_t keybytes[21], k1[8], k2[8], k3[8];

  memset(keybytes, 0, 21);
  memcpy(keybytes, hash, 16);

  createDESKey(keybytes     , k1);
  createDESKey(keybytes +  7, k2);
  createDESKey(keybytes + 14, k3);

  LogHex("DES Key 1", k1, 8);
  LogHex("DES Key 2", k2, 8);
  LogHex("DES Key 3", k3, 8);

  EncryptDES(k1, response, challenge);
  EncryptDES(k2, response + 8, challenge);
  EncryptDES(k3, response + 16, challenge);

  LogHex("Encrypted Resp 1", response, 8);
  LogHex("Encrypted Resp 2", response + 8, 8);
  LogHex("Encrypted Resp 3", response + 16, 8);

  // Testing
  /*****************
  Uint8 test[8];
  DecryptDES(k1, response, test);
  LogHex("Decrypted Resp 1", test, 8);
  DecryptDES(k2, response + 8, test);
  LogHex("Decrypted Resp 1", test, 8);
  DecryptDES(k3, response + 16, test);
  LogHex("Decrypted Resp 1", test, 8);
  ******************/
}

//-----------------------------------------------------------------------------

uint32_t ParseType2Msg(const void *inputBuf, uint32_t inputLength, struct Type2Msg *msg)
{
  const uint8_t *ptr  = (const uint8_t *) inputBuf;
  uint32_t targetLen = 0;
  uint32_t offset = 0;
  LogHex("Challenge Response Binary", inputBuf, inputLength);

  // Do some sanity checks
  // Correct Length?
  if (inputLength < NTLM_TYPE2_LENGTH) return -2;

  // Correct Signature?
  if (memcmp(ptr, NTLM_SIGNATURE, sizeof(NTLM_SIGNATURE)) != 0) return -2;
  ptr += sizeof(NTLM_SIGNATURE);

  // Correct Msg ID?
  if (memcmp(ptr, NTLM_TYPE2_MSG_ID, sizeof(NTLM_TYPE2_MSG_ID)) != 0)
    return -2;
  ptr += sizeof(NTLM_TYPE2_MSG_ID);

  // Get the Target Length and ignore the Allocated value
  targetLen = ReadUint16(&ptr);
  ReadUint16(&ptr);
  // Get the target offset
  offset = ReadUint32(&ptr);
  // Sanity check to make sure the offset + target length is not greater
  // than the length of the entire buffer
  if (offset < offset + targetLen && offset + targetLen <= inputLength) {
    msg->targetLength = targetLen;
    msg->target = (const char *)inputBuf + offset;
  }  else {
    msg->targetLength = 0;
    msg->target = NULL;
  }
  // Get the flags
  msg->flags = ReadUint32(&ptr);
  // Get the challenge
  memcpy(msg->challenge, ptr, sizeof(msg->challenge));
  ptr += sizeof(msg->challenge);

  // Log the parsed message
  LogType2Msg(msg);
  return 0;
}

int32_t GenerateType1Msg(char **buffer, uint32_t *length)
{
	char *ptr = NULL;
	if (!buffer || !length) return -1;
	*length = NTLM_TYPE1_LENGTH;
	*buffer = (char *)TW_CALLOC(*length, 1);  // Make sure the caller frees this
	if (!*buffer) return -1;
	ptr = *buffer;

	// Create the Type 1 msg - make sure the caller frees it when done
	// NTLM Signature
	ptr = AddToMessageBuffer(ptr, NTLM_SIGNATURE, sizeof(NTLM_SIGNATURE));
	// Type 1 Message ID
	ptr = AddToMessageBuffer(ptr, NTLM_TYPE1_MSG_ID, sizeof(NTLM_TYPE1_MSG_ID));
	// Flags
	ptr = WriteFlags(ptr, NTLM_TYPE1_FLAGS);
	// Empty Domain buffer
	ptr = SecurityBuffer(ptr, 0, 0);
	// EMpty host security buffer
	ptr = SecurityBuffer(ptr, 0, 0);

	LogType1Msg((char *)*buffer, *length);
	return 0;
}

int32_t GenerateType3Msg(const char * domain, const char * username, const char * password,
                 const void *challenge, uint32_t challengeLength, char **outputBuf, uint32_t *outputLength)
{
	struct Type2Msg msg;
	int32_t ret ;

	uint32_t domainLen, userLen, hostLen;
	int32_t len = 0;
	int i = 0;

	/* Domain */
	char * tmpDomain = NULL;
	char * tmpUser = NULL;
	char * tmpHost = NULL;
	char * tmpPwd = NULL;
	char * hostBuf = "WORKSTATION";

	/* Full message */
	char *ptr = NULL;
	uint32_t offset;
	uint8_t ntlmResp[NTLM_RESP_LENGTH], ntlmHash[NTLM_HASH_LENGTH];

	if (!username || !password || !challenge || !outputBuf || *outputLength) {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: NULL pointer passed in");
		return -1;
	}
	
	ret = ParseType2Msg(challenge, challengeLength, &msg);
	if (ret)  {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error parsing type 2 message");
		return ret;
	}

	/* Domain */
	if (domain) {
		len = strlen(domain);
		tmpDomain = (char *)TW_CALLOC(len * 2, 1);
		if (!tmpDomain) {
			TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error allocating memory");
			return -1;
		}
		for (i = 0; i < len; i++) {
				tmpDomain[i * 2] = domain[i];
				tmpDomain[i * 2 + 1] = (char)0;
		}
		#ifdef BIG_ENDIAN_CPU
		tmpDomain = convertUnicodeStringToLE(tmpDomain);
		#endif
	} else len = 0;
	domainLen = len * 2;

	/* User */
	len = strlen(username);
	tmpUser = (char *)TW_CALLOC(len * 2, 1);
	if (!tmpUser) {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error allocating memory");
		if (tmpDomain) TW_FREE(tmpDomain);
		return -1;
	}
	/* username is ASCII, so we can do a simple zero-pad expansion: */
	for (i = 0; i < len; i++) {
			tmpUser[i * 2] = username[i];
			tmpUser[i * 2 + 1] = (char)0;
	}
	#ifdef BIG_ENDIAN_CPU
	tmpUser= convertUnicodeStringToLE(tmpUser);
	#endif
	userLen = len * 2;

	/* Dummy host name works OK for workstation name */
	len = strlen(hostBuf);
	tmpHost = (char *)TW_CALLOC(len * 2, 1);
	/* hostname is ASCII, so convert it to "unicode": */
	if (!tmpHost) {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error allocating memory");
		if (tmpDomain) TW_FREE(tmpDomain);
		TW_FREE(tmpUser);
		return -1;
	}
	for (i = 0; i < len; i++) {
			tmpHost[i * 2] = hostBuf[i];
			tmpHost[i * 2 + 1] = (char)0;
	}
	#ifdef BIG_ENDIAN_CPU
	tmpHost = convertUnicodeStringToLE(tmpHost);
	#endif
	hostLen = len * 2;;

	/* Allocate space for the message */
	*outputLength = NTLM_TYPE3_LENGTH + hostLen + domainLen + userLen +
			LM_RESP_LENGTH + NTLM_RESP_LENGTH;
	*outputBuf = (char *) TW_CALLOC(*outputLength, 1);
	if (!*outputBuf)   {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error allocating memory");
		if (tmpDomain) TW_FREE(tmpDomain);
		TW_FREE(tmpUser);
		TW_FREE(tmpHost);
		return -1;
	}

	/* Calculate our NTLM Response */
	// Password should be Unicode for the Hash
	len = strlen(password);
	tmpPwd = (char *)TW_CALLOC(len * 2, 1);
	/* hostname is ASCII, so convert it to "unicode": */
	if (!tmpPwd) {
		TW_LOG(TW_ERROR,"NTLM_GenerateType3Msg: Error allocating memory");
		if (tmpDomain) TW_FREE(tmpDomain);
		TW_FREE(tmpUser);
		TW_FREE(tmpHost);
		return -1;
	}
	for (i = 0; i < strlen(password); i++) {
			tmpPwd[i * 2] = password[i];
			tmpPwd[i * 2 +1] = (char)0;
	}
	////LogHex("Pwd", pt::pconst(tmp), pt::length(tmp));
	NTLM_Hash(tmpPwd, ntlmHash, len * 2);
	// Calculate the response
	NTLM_Response(ntlmHash, msg.challenge, ntlmResp);

	/* If we are here we can now put together the full message */
	ptr = *outputBuf;
	// NTLM Signature
	ptr = AddToMessageBuffer(ptr, NTLM_SIGNATURE, sizeof(NTLM_SIGNATURE));
	// Message ID
	ptr = AddToMessageBuffer(ptr, NTLM_TYPE3_MSG_ID, sizeof(NTLM_TYPE3_MSG_ID));
	// LM Response Buffer - This should be the same as the NTLM response buffer
	offset = NTLM_TYPE3_LENGTH + domainLen + userLen + hostLen;
	ptr = SecurityBuffer(ptr, NTLM_RESP_LENGTH, offset);
	memcpy((uint8_t *) *outputBuf + offset, ntlmResp, LM_RESP_LENGTH);
	// NTLM Response Buffer
	offset += LM_RESP_LENGTH;
	ptr = SecurityBuffer(ptr, NTLM_RESP_LENGTH, offset);
	memcpy((uint8_t *) *outputBuf + offset, ntlmResp, NTLM_RESP_LENGTH);
	// Domain Name
	offset = NTLM_TYPE3_LENGTH;
	ptr = SecurityBuffer(ptr, domainLen, offset);
	memcpy((uint8_t *) *outputBuf + offset, tmpDomain, domainLen);
	// User
	offset += domainLen;
	ptr = SecurityBuffer(ptr, userLen, offset);
	memcpy((uint8_t *) *outputBuf + offset, tmpUser, userLen);
	// Host
	offset += userLen;
	ptr = SecurityBuffer(ptr, hostLen, offset);
	memcpy((uint8_t *) *outputBuf + offset, tmpHost, hostLen);
	// Empty Session Key
	ptr = SecurityBuffer(ptr, 0, 0);
	// Flags
	ptr = WriteFlags(ptr, msg.flags & NTLM_TYPE1_FLAGS);

	LogHex("Type3 Message", *outputBuf, *outputLength);

	TW_FREE(tmpDomain);
	TW_FREE(tmpUser);
	TW_FREE(tmpHost);
	return 0;
	}

