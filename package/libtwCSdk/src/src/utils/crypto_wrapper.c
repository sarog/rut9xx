/* 
 * Wrapper around libtomcrypt DES encryption functions
 *
 * Copyright 2016, PTC, Inc.
 *
 * This software is distributed without any warranty.
 */

#include "twOSPort.h"
#include "twLogger.h"

#include <tomcrypt.h>


int EncryptDES(const unsigned char * key, unsigned char *  ct, const unsigned char *  pt) {

	symmetric_key skey;
	int err;
	/* ... key is loaded appropriately in key ... */
	/* ... load a block of plaintext in pt ... */
	/* schedule the key */
	if ((err = des_setup(
			key, /* the key we will use */
			8, /* key is 8 bytes (64-bits) long */
			0, /* 0 == use default # of rounds */
			&skey) /* where to put the scheduled key */
	) != CRYPT_OK) {
		TW_LOG(TW_ERROR,"EncryptDES: Setup error: %s\n", error_to_string(err));
		return -1;
	}
	/* encrypt the block */
	des_ecb_encrypt(
		pt, /* encrypt this 8-byte array */
		ct, /* store encrypted data here */
		&skey); /* our previously scheduled key */

	// Cleanup
	des_done(&skey);
	return 0;
}

int DecryptDES(const unsigned char * key, const unsigned char *  ct, unsigned char *  pt) {

	symmetric_key skey;
	int err;
	/* ... key is loaded appropriately in key ... */
	/* ... load a block of plaintext in pt ... */
	/* schedule the key */
	if ((err = des_setup(
			key, /* the key we will use */
			8, /* key is 8 bytes (64-bits) long */
			0, /* 0 == use default # of rounds */
			&skey) /* where to put the scheduled key */
	) != CRYPT_OK) {
		TW_LOG(TW_ERROR,"EncryptDES: Setup error: %s\n", error_to_string(err));
		return -1;
	}
	/* decrypt the block */
	des_ecb_decrypt(
		ct, /* decrypt this 8-byte array */
		pt, /* store decrypted data here */
		&skey); /* our previously scheduled key */

	// Cleanup
	des_done(&skey);
	return 0;
}

int MD4Hash(const unsigned char * buf, int length, unsigned char * hash) {

	hash_state md;
	//unsigned char *in = "hello world", out[16];
	/* setup the hash */
	md4_init(&md);
	/* add the message */
	md4_process(&md, buf, length);
	/* get the hash in out[0..15] */
	md4_done(&md, hash);
	return 0;
 
}

/**
 * Applies odd parity to the given byte array.
 *
 * @param bytes The data whose parity bits are to be adjusted for
 * odd parity.
 */
void oddParity(uint8_t * bytes, int length) {
	int i = 0;
    for (i = 0; i < length; i++) {
        uint8_t b = bytes[i];
        char needsParity = (((b >> 7) ^ (b >> 6) ^ (b >> 5) ^
                                (b >> 4) ^ (b >> 3) ^ (b >> 2) ^
                                (b >> 1)) & 0x01) == 0;
        if (needsParity) {
            bytes[i] |= (uint8_t) 0x01;
        } else {
            bytes[i] &= (uint8_t) 0xfe;
        }
    }
}

void createDESKey(const uint8_t * bytes, uint8_t * key) {
	  key[0] = bytes[0];
	  key[1] = (bytes[0] << 7) | (bytes[1] >> 1);
	  key[2] = (bytes[1] << 6) | (bytes[2] >> 2);
	  key[3] = (bytes[2] << 5) | (bytes[3] >> 3);
	  key[4] = (bytes[3] << 4) | (bytes[4] >> 4);
	  key[5] = (bytes[4] << 3) | (bytes[5] >> 5);
	  key[6] = (bytes[5] << 2) | (bytes[6] >> 6);
	  key[7] = (bytes[6] << 1);

	 oddParity(key, 8);
}

