/*
 *  Copyright 2018, PTC, Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "twLogger.h"
#include "twErrors.h"

#include "twTls.h"
#include "twOpenSSL.h"

#define SHA256_DIGEST_LENGTH 32
#define SHA256_HEX_DIGEST_LENGTH 65

void TWSHA256BinToHex(unsigned char binary[SHA256_DIGEST_LENGTH],unsigned char hex[SHA256_HEX_DIGEST_LENGTH]){
	int i = 0;
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		char hexText[10];
		snprintf(hexText,10, "%02x", binary[i]);
		hex[(i * 2)]=hexText[0];
		hex[(i * 2)+1]=hexText[1];
	}
	hex[64] = 0;
}

int TWSHA256_sha256_file(char *path, char outputBuffer[SHA256_HEX_DIGEST_LENGTH]){
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	const int bufSize = 32768;
	char *buffer = NULL;
	int bytesRead = 0;
	FILE *file = fopen(path, "rb");

	if(!file)
		return TW_ERROR_READING_FILE;
	
	SHA256_Init(&sha256);
	buffer = (char*)TW_MALLOC(bufSize);
	if(!buffer) return TW_ERROR_ALLOCATING_MEMORY;
	while((bytesRead = fread(buffer, 1, bufSize, file)))
	{
		SHA256_Update(&sha256,buffer, bytesRead);
	}
	SHA256_Final(hash, &sha256);
	TWSHA256BinToHex(hash,(unsigned char*)outputBuffer);

	fclose(file);
	TW_FREE(buffer);
	return TW_OK;
}

int decryptSignatureRsaFile(const char* publicKeyFilePath,char* encryptedData,size_t encryptedDataLength, char decryptedHex[SHA256_HEX_DIGEST_LENGTH],char** commonName){
	X509 * x509Data = NULL;
	RSA *rsa = NULL;
	char decryptedBinary[SHA256_HEX_DIGEST_LENGTH];

	/* Determine if we are dealing with a public cert or a public key */
	if(stringEndsWithSuffix(publicKeyFilePath,"crt")||stringEndsWithSuffix(publicKeyFilePath,"CRT")){
		FILE* pFile = fopen(publicKeyFilePath, "rb");
		EVP_PKEY * pkey = NULL;
		int common_name_loc = -1;
		X509_NAME_ENTRY *common_name_entry = NULL;
		ASN1_STRING *common_name_asn1 = NULL;
		char *common_name_str = NULL;

		if(NULL == PEM_read_X509(pFile, &x509Data, NULL, NULL)){
			char *err;
			ERR_load_crypto_strings();
			err = (char *) TW_MALLOC(130);
			ERR_error_string(ERR_get_error(), err);
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Error opening certificate file (OpenSSL Error): %s.", err);
			TW_FREE(err);
			return TW_ERROR_READING_FILE;
		}
		fclose(pFile);
		pkey = X509_get_pubkey(x509Data);
		if(EVP_PKEY_RSA!=EVP_PKEY_base_id(pkey)){
			EVP_PKEY_free(pkey);
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Certificate file does not contain an RSA key.");
			return TW_ERROR_READING_FILE;
		}
		rsa = EVP_PKEY_get1_RSA(pkey);
		
		// Find the position of the CN field in the Subject field of the certificate
		common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name(x509Data), NID_commonName, -1);
		if (common_name_loc < 0) {
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Common Name could not be found.");
			return TW_ERROR_READING_FILE;
		}

		// Extract the CN field
		common_name_entry = X509_NAME_get_entry(X509_get_subject_name(x509Data), common_name_loc);
		if (common_name_entry == NULL) {
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Could not read common name.");
			return TW_ERROR_READING_FILE;
		}

		// Convert the CN field to a C string
		common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
		if (common_name_asn1 == NULL) {
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Could not convert common name to a string.");
			return TW_ERROR_READING_FILE;
		}
		*commonName = (char *) ASN1_STRING_data(common_name_asn1);

	} else {
		FILE* pFile = fopen(publicKeyFilePath, "rt");
		rsa = PEM_read_RSAPublicKey(pFile, NULL, NULL, NULL);
		if(NULL!=x509Data)
			*commonName=NULL;
		fclose(pFile);
	}

	if (NULL != rsa) {
		if(-1 == RSA_public_decrypt(encryptedDataLength, (const unsigned char*)encryptedData, decryptedBinary, rsa, RSA_PKCS1_PADDING)) {
			char *err;
			ERR_load_crypto_strings();
			err = (char *) TW_MALLOC(130);
			ERR_error_string(ERR_get_error(), err);
			TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Error decrypting message (OpenSSL Error): %s.", err);
			TW_FREE(err);
			return TW_ERROR_READING_FILE;
		} else {
			TWSHA256BinToHex(decryptedBinary,decryptedHex);
			return TW_OK;
		}
	} else {
		char *err;
		ERR_load_crypto_strings();
		err = (char *) TW_MALLOC(130);
		ERR_error_string(ERR_get_error(), err);
		TW_LOG(TW_ERROR, "decryptSignatureRsaFile - Error reading public key %s, %s", publicKeyFilePath,err);
		TW_FREE(err);
		return TW_ERROR_READING_FILE;
	}
}
