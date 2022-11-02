// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 * Copyright (c) 2021, Teltonika Networks
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <endian.h>

#define uswap_16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define uswap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#define _uswap_64(x, sfx) \
	((((x) & 0xff00000000000000##sfx) >> 56) | \
	 (((x) & 0x00ff000000000000##sfx) >> 40) | \
	 (((x) & 0x0000ff0000000000##sfx) >> 24) | \
	 (((x) & 0x000000ff00000000##sfx) >>  8) | \
	 (((x) & 0x00000000ff000000##sfx) <<  8) | \
	 (((x) & 0x0000000000ff0000##sfx) << 24) | \
	 (((x) & 0x000000000000ff00##sfx) << 40) | \
	 (((x) & 0x00000000000000ff##sfx) << 56))
#if defined(__GNUC__)
# define uswap_64(x) _uswap_64(x, ull)
#else
# define uswap_64(x) _uswap_64(x, )
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)
# define cpu_to_be16(x)		uswap_16(x)
# define cpu_to_be32(x)		uswap_32(x)
# define cpu_to_be64(x)		uswap_64(x)
# define be16_to_cpu(x)		uswap_16(x)
# define be32_to_cpu(x)		uswap_32(x)
# define be64_to_cpu(x)		uswap_64(x)
#else
# define cpu_to_le16(x)		uswap_16(x)
# define cpu_to_le32(x)		uswap_32(x)
# define cpu_to_le64(x)		uswap_64(x)
# define le16_to_cpu(x)		uswap_16(x)
# define le32_to_cpu(x)		uswap_32(x)
# define le64_to_cpu(x)		uswap_64(x)
# define cpu_to_be16(x)		(x)
# define cpu_to_be32(x)		(x)
# define cpu_to_be64(x)		(x)
# define be16_to_cpu(x)		(x)
# define be32_to_cpu(x)		(x)
# define be64_to_cpu(x)		(x)
#endif

static void print_help() {
	fprintf(stdout, "Usage:\n"
	"./rsa_pkey_tool input_filename output_filename\n"
	"input_filename - 2048 Bits size RSA public key in .pem format\n"
	"output_filename - header with processed rsa public key values \n"
	"written to struct rsa_public_key \n"
	);
}

/*
 * write_bignum_to_header(): - Write BIGNUM as uint32_t array
 * to header file
 */
static int write_bignum_to_header(FILE *fp, const char *prop_name,
			  BIGNUM *num, int num_bits)
{
	int nwords = num_bits / 32;
	int size;
	uint32_t *buf, *ptr;
	BIGNUM *tmp, *big2, *big32, *big2_32;
	BN_CTX *ctx;
	int i = 0;
	int j = 1;

	tmp = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	big2_32 = BN_new();

	/*
	 * Note: This code assumes that all of the above succeed, or all fail.
	 * In practice memory allocations generally do not fail (unless the
	 * process is killed), so it does not seem worth handling each of these
	 * as a separate case. Technicaly this could leak memory on failure,
	 * but a) it won't happen in practice, and b) it doesn't matter as we
	 * will immediately exit with a failure code.
	 */
	if (!tmp || !big2 || !big32 || !big2_32) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}
	ctx = BN_CTX_new();
	if (!ctx) {
		fprintf(stderr, "Out of memory (bignum context)\n");
		return -ENOMEM;
	}
	BN_set_word(big2, 2L);
	BN_set_word(big32, 32L);
	BN_exp(big2_32, big2, big32, ctx); /* B = 2^32 */

	size = nwords * sizeof(uint32_t);
	buf = (uint32_t *)malloc(size);
	if (!buf) {
		fprintf(stderr, "Out of memory (%d bytes)\n", size);
		return -ENOMEM;
	}

	/* Write out modulus as little endian array of integers */
	ptr = buf;
	while (i < nwords) {
		BN_mod(tmp, num, big2_32, ctx); /* n = N mod B */
		*ptr =  cpu_to_le32(BN_get_word(tmp));
		BN_rshift(num, num, 32); /*  N = N/B */
		ptr++;
		i++;
	}

	fprintf(stdout, "%s length: %d bits\n", prop_name, size*8);
	fprintf(stdout, "%s value:\n", prop_name);
    for (i = 0; i < nwords-1; i++) {
		fprintf(stdout, "0x%08X:", buf[i]);
	}
	fprintf(stdout, "0X%08X", buf[nwords-1]);
	fprintf(stdout,"\n");
	fprintf(stdout,"\n");

	/* Write rsa-modulus or rsa,r-squared to header */
	fprintf(fp, "	{ ");
	for (i = 0; i < nwords-1; i++) {
		fprintf(fp, "0X%08X, ", buf[i]);
		if (j > 4) {
			fprintf(fp, "\n");
			fprintf(fp, "	  ");
			j = 0;
		}
		j++;
	}
	fprintf(fp, "0X%08X", buf[nwords-1]);
	fprintf(fp, " },\n");

	/*
	 * We try signing with successively increasing size values, so this
	 * might fail several times
	 */
	free(buf);
	BN_free(tmp);
	BN_free(big2);
	BN_free(big32);
	BN_free(big2_32);

	return 0;
}


/*
 * rsa_get_exponent(): - Get the public exponent of an RSA public key
 */
static int rsa_get_exponent(RSA *key, uint64_t *e)
{
	int ret;
	BIGNUM *bn_te;
	const BIGNUM *key_e;
	uint64_t te;

	ret = -EINVAL;
	bn_te = NULL;

	if (!e)
		goto cleanup;

	RSA_get0_key(key, NULL, &key_e, NULL);
	if (BN_num_bits(key_e) > 64)
		goto cleanup;

	*e = BN_get_word(key_e);

	if (BN_num_bits(key_e) < 33) {
		ret = 0;
		goto cleanup;
	}

	bn_te = BN_dup(key_e);
	if (!bn_te)
		goto cleanup;

	if (!BN_rshift(bn_te, bn_te, 32))
		goto cleanup;

	if (!BN_mask_bits(bn_te, 32))
		goto cleanup;

	te = BN_get_word(bn_te);
	te <<= 32;
	*e |= te;
	ret = 0;

cleanup:
	if (bn_te)
		BN_free(bn_te);

	return ret;
}

/*
 * rsa_get_params(): - Get the important parameters of an RSA public key
 */

static int rsa_get_params(RSA *key, uint64_t *exponent, uint32_t *n0_invp,
		   BIGNUM **modulusp, BIGNUM **r_squaredp)
{
	BIGNUM *big1, *big2, *big32, *big2_32;
	BIGNUM *n, *r, *r_squared, *tmp;
	const BIGNUM *key_n;
	BN_CTX *bn_ctx = BN_CTX_new();
	int ret = 0;

	/* Initialize BIGNUMs */
	big1 = BN_new();
	big2 = BN_new();
	big32 = BN_new();
	r = BN_new();
	r_squared = BN_new();
	tmp = BN_new();
	big2_32 = BN_new();
	n = BN_new();
	if (!big1 || !big2 || !big32 || !r || !r_squared || !tmp || !big2_32 ||
	    !n) {
		fprintf(stderr, "Out of memory (bignum)\n");
		return -ENOMEM;
	}

	if (0 != rsa_get_exponent(key, exponent))
		ret = -1;

	RSA_get0_key(key, &key_n, NULL, NULL);

	if (!BN_copy(n, key_n) || !BN_set_word(big1, 1L) ||
	    !BN_set_word(big2, 2L) || !BN_set_word(big32, 32L))
		ret = -1;

	/* big2_32 = 2^32 */
	if (!BN_exp(big2_32, big2, big32, bn_ctx))
		ret = -1;

	/* Calculate n0_inv = -1 / n[0] mod 2^32 */
	if (!BN_mod_inverse(tmp, n, big2_32, bn_ctx) ||
	    !BN_sub(tmp, big2_32, tmp))
		ret = -1;
	*n0_invp = BN_get_word(tmp);

	/* Calculate R = 2^(# of key bits) */
	if (!BN_set_word(tmp, BN_num_bits(n)) ||
	    !BN_exp(r, big2, tmp, bn_ctx))
		ret = -1;

	/* Calculate r_squared = R^2 mod n */
	if (!BN_copy(r_squared, r) ||
	    !BN_mul(tmp, r_squared, r, bn_ctx) ||
	    !BN_mod(r_squared, tmp, n, bn_ctx))
		ret = -1;

	*modulusp = n;
	*r_squaredp = r_squared;

	BN_free(big1);
	BN_free(big2);
	BN_free(big32);
	BN_free(r);
	BN_free(tmp);
	BN_free(big2_32);
	if (ret) {
		fprintf(stderr, "Bignum operations failed\n");
		return -ENOMEM;
	}

	return ret;
}

int main(int argc, char **argv) {
	RSA* rsa = NULL;

	BIGNUM *modulus = NULL;
	BIGNUM *r_squared = NULL;
	uint64_t exponent;
	uint32_t n0_inv;
	FILE *fp;
	int ret = 0;
	int bits;
	int i;
	int nwords;
	int size;
	uint32_t n0_inv_tmp;

	if(argc < 3) {
		print_help();
		return -1;
	}

	#if __BYTE_ORDER == __LITTLE_ENDIAN
		fprintf(stdout, "Little endian system\n");
	#elif __BYTE_ORDER == __BIG_ENDIAN
		fprintf(stdout, "Big endian system\n");
	#else
		fprintf(stderr, "Check byte order definitions!\n");
	#endif

	fp = fopen(argv[1], "rb");
	if(fp == NULL) {
		fprintf(stderr,"Unable to open file %s \n",argv[1]);
		return -1;
	}

	rsa = PEM_read_RSA_PUBKEY(fp, &rsa,NULL, NULL);
	if (rsa == NULL) {
		fprintf(stderr, "Couldn't read public key file.\n");
		ret = -1;
		goto exit;
	}

	fclose(fp);

	if(rsa_get_params(rsa, &exponent, &n0_inv_tmp,
						&modulus, &r_squared)) {
		fprintf(stderr, "Couldn't get public key parameters.\n");
		ret = -1;
		goto exit;
	}

	bits = BN_num_bits(modulus);

	fp = fopen(argv[2], "w");
	if(fp == NULL) {
		fprintf(stderr,"Unable to open file for writing %s \n", argv[2]);
		RSA_free(rsa);
		return -1;
	}

	fprintf(fp, "const struct rsa_public_key pubkey = {\n");

	if(write_bignum_to_header(fp, "rsa-modulus", modulus, bits)) {
		fprintf(stderr,"Failed to write rsa-modulus, exiting\n");
		ret = -1;
		goto exit;
	}
	if(write_bignum_to_header(fp, "rsa,r-squared", r_squared, bits)) {
		fprintf(stderr,"Failed to write rsa,r-squared, exiting\n");
		ret = -1;
		goto exit;
	}

	n0_inv = cpu_to_le32(n0_inv_tmp);

	fprintf(stdout, "rsa,n0-inverse value: 0x%08X\n", n0_inv);

	/* Write rsa,n0-inverse to header */
	fprintf(fp, "	  ");
	fprintf(fp, "0X%08X\n", n0_inv);
	fprintf(fp, "};\n");

exit:
	fclose(fp);
	RSA_free(rsa);
	return ret;
}
