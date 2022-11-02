/***************************************
 *  Copyright 2018, PTC, Inc.
 ***************************************/


#ifndef TW_C_SDK_TWSIG_H
#define TW_C_SDK_TWSIG_H
#include <stdio.h>
void TWSHA256BinToHex(unsigned char binary[32],unsigned char hex[65]);
int TWSHA256_sha256_file(char *path, char outputBuffer[65]);
int decryptSignatureRsaFile(const char* publicKeyFilePath,char* encryptedData,size_t encryptedDataLength, char decryptedHex[65],char** commonName);
#endif //TW_C_SDK_TWSIG_H
