/***************************************
 *  Copyright 2016, PTC, Inc.
 ***************************************/

/**
 * \file crypto_wrapper.h
 *
 * \brief Wrapper for libtomcrypt DES encryption functions
*/

#ifndef __CRYPTO_WRAPPER_H
#define __CRYPTO_WRAPPER_H

/**
 * \brief Encrypts an 8-byte array using DES.
 *
 * \param[in]     key       A pointer to the 8-byte key.
 * \oaram[out]    ct        A pointer which will store the encrypted data.
 * \param[in]     pt        A pointer to the 8-byte array to be encrypted.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int EncryptDES(const unsigned char * key, unsigned char *  ct, const unsigned char *  pt);

/**
 * \brief Encrypts an 8-byte array using DES.
 *
 * \param[in]     key       A pointer to the 8-byte key.
 * \oaram[in]     ct        A pointer to the 8-byte array to be decrypted.
 * \param[out]    pt        A pointer which will store the decrypted data.
 *
 * \return #TW_OK if successful, positive integral on error code (see
 * twErrors.h) if an error was encountered.
*/
int DecryptDES(const unsigned char * key, const unsigned char *  ct, unsigned char *  pt);

/**
 * \brief Creates an 8-byte DES key.
 *
 * \param[in]     bytes     TBD
 * \param[out]    key       A pointer which will store the 8-byte key.
 *
 * \return Nothing.
*/
void createDESKey(const uint8_t * bytes, uint8_t * key);

/**
 * \brief Generates a MD4 Message-Digest Algorithm.
 *
 * \param[in]     buf       A buffer containing the hash message.
 * \param[in]     length    The length of
 * \param[out]    hash      A pointer which will store the generated MD4 hash.
 *
 * \return Always returns 0.
*/
int MD4Hash(const unsigned char * buf, int length, unsigned char * hash);
#endif
