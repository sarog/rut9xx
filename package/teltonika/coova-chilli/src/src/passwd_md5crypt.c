#include "chilli.h"
#include <assert.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define MD5_CTX MD5_CTX_OpenSSL
#include <openssl/md5.h>
#undef MD5_CTX

static unsigned const char cov_2char[64] = {
	/* from crypto/des/fcrypt.c */
	0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A,
	0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55,
	0x56, 0x57, 0x58, 0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
	0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
	0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A
};

static const char ascii_dollar[] = { 0x24, 0x00 };

size_t OPENSSL_strlcpy(char *dst, const char *src, size_t size)
{
	size_t l = 0;
	for (; size > 1 && *src; size--) {
		*dst++ = *src++;
		l++;
	}
	if (size)
		*dst = CH_ZERO;
	return l + strlen(src);
}

size_t OPENSSL_strlcat(char *dst, const char *src, size_t size)
{
	size_t l = 0;
	for (; size > 0 && *dst; size--, dst++)
		l++;
	return l + OPENSSL_strlcpy(dst, src, size);
}

/*
 * MD5-based password algorithm (should probably be available as a library
 * function; then the static buffer would not be acceptable). For magic
 * string "1", this should be compatible to the MD5-based BSD password
 * algorithm. For 'magic' string "apr1", this is compatible to the MD5-based
 * Apache password algorithm. (Apparently, the Apache password algorithm is
 * identical except that the 'magic' string was changed -- the laziest
 * application of the NIH principle I've ever encountered.)
 */
char *md5crypt(const char *passwd, const char *magic, const char *salt)
{
	/* "$apr1$..salt..$.......md5hash..........\0" */
	static char out_buf[6 + 9 + 24 + 2];
	unsigned char buf[MD5_DIGEST_LENGTH];
	char ascii_magic[5]; /* "apr1" plus '\0' */
	char ascii_salt[9]; /* Max 8 chars plus '\0' */
	char *ascii_passwd = NULL;
	char *salt_out;
	int n;
	unsigned int i;
	EVP_MD_CTX *md = NULL, *md2 = NULL;
	size_t passwd_len, salt_len, magic_len;

	passwd_len = strlen(passwd);

	out_buf[0] = 0;
	magic_len  = strlen(magic);
	OPENSSL_strlcpy(ascii_magic, magic, sizeof(ascii_magic));
#ifdef CHARSET_EBCDIC
	if ((magic[0] & 0x80) != 0) /* High bit is 1 in EBCDIC alnums */
		ebcdic2ascii(ascii_magic, ascii_magic, magic_len);
#endif

	/* The salt gets truncated to 8 chars */
	OPENSSL_strlcpy(ascii_salt, salt, sizeof(ascii_salt));
	salt_len = strlen(ascii_salt);
#ifdef CHARSET_EBCDIC
	ebcdic2ascii(ascii_salt, ascii_salt, salt_len);
#endif

#ifdef CHARSET_EBCDIC
	ascii_passwd = OPENSSL_strdup(passwd);
	if (ascii_passwd == NULL)
		return NULL;
	ebcdic2ascii(ascii_passwd, ascii_passwd, passwd_len);
	passwd = ascii_passwd;
#endif

	if (magic_len > 0) {
		OPENSSL_strlcat(out_buf, ascii_dollar, sizeof(out_buf));

		if (magic_len > 4) /* assert it's  "1" or "apr1" */
			goto err;

		OPENSSL_strlcat(out_buf, ascii_magic, sizeof(out_buf));
		OPENSSL_strlcat(out_buf, ascii_dollar, sizeof(out_buf));
	}

	OPENSSL_strlcat(out_buf, ascii_salt, sizeof(out_buf));

	if (strlen(out_buf) > 6 + 8) /* assert "$apr1$..salt.." */
		goto err;

	salt_out = out_buf;
	if (magic_len > 0)
		salt_out += 2 + magic_len;

	if (salt_len > 8)
		goto err;

	md = EVP_MD_CTX_new();
	if (md == NULL || !EVP_DigestInit_ex(md, EVP_md5(), NULL) ||
	    !EVP_DigestUpdate(md, passwd, passwd_len))
		goto err;

	if (magic_len > 0)
		if (!EVP_DigestUpdate(md, ascii_dollar, 1) ||
		    !EVP_DigestUpdate(md, ascii_magic, magic_len) ||
		    !EVP_DigestUpdate(md, ascii_dollar, 1))
			goto err;

	if (!EVP_DigestUpdate(md, ascii_salt, salt_len))
		goto err;

	md2 = EVP_MD_CTX_new();
	if (md2 == NULL || !EVP_DigestInit_ex(md2, EVP_md5(), NULL) ||
	    !EVP_DigestUpdate(md2, passwd, passwd_len) ||
	    !EVP_DigestUpdate(md2, ascii_salt, salt_len) ||
	    !EVP_DigestUpdate(md2, passwd, passwd_len) ||
	    !EVP_DigestFinal_ex(md2, buf, NULL))
		goto err;

	for (i = passwd_len; i > sizeof(buf); i -= sizeof(buf)) {
		if (!EVP_DigestUpdate(md, buf, sizeof(buf)))
			goto err;
	}
	if (!EVP_DigestUpdate(md, buf, i))
		goto err;

	n = passwd_len;
	while (n) {
		if (!EVP_DigestUpdate(md, (n & 1) ? "\0" : passwd, 1))
			goto err;
		n >>= 1;
	}
	if (!EVP_DigestFinal_ex(md, buf, NULL))
		return NULL;

	for (i = 0; i < 1000; i++) {
		if (!EVP_DigestInit_ex(md2, EVP_md5(), NULL))
			goto err;
		if (!EVP_DigestUpdate(
			    md2, (i & 1) ? (unsigned const char *)passwd : buf,
			    (i & 1) ? passwd_len : sizeof(buf)))
			goto err;
		if (i % 3) {
			if (!EVP_DigestUpdate(md2, ascii_salt, salt_len))
				goto err;
		}
		if (i % 7) {
			if (!EVP_DigestUpdate(md2, passwd, passwd_len))
				goto err;
		}
		if (!EVP_DigestUpdate(
			    md2, (i & 1) ? buf : (unsigned const char *)passwd,
			    (i & 1) ? sizeof(buf) : passwd_len))
			goto err;
		if (!EVP_DigestFinal_ex(md2, buf, NULL))
			goto err;
	}
	EVP_MD_CTX_free(md2);
	EVP_MD_CTX_free(md);
	md2 = NULL;
	md  = NULL;

	{
		/* transform buf into output string */
		unsigned char buf_perm[sizeof(buf)];
		int dest, source;
		char *output;

		/* silly output permutation */
		for (dest = 0, source = 0; dest < 14;
		     dest++, source   = (source + 6) % 17)
			  buf_perm[dest] = buf[source];
		buf_perm[14] = buf[5];
		buf_perm[15] = buf[11];
#ifndef PEDANTIC /* Unfortunately, this generates a "no \
                  * effect" warning */
		assert(16 == sizeof(buf_perm));
#endif

		output = salt_out + salt_len;
		// assert(output == out_buf + strlen(out_buf));
		if (output != out_buf + strlen(out_buf))
			return NULL;

		*output++ = ascii_dollar[0];

		for (i = 0; i < 15; i += 3) {
			*output++ = cov_2char[buf_perm[i + 2] & 0x3f];
			*output++ = cov_2char[((buf_perm[i + 1] & 0xf) << 2) |
					      (buf_perm[i + 2] >> 6)];
			*output++ = cov_2char[((buf_perm[i] & 3) << 4) |
					      (buf_perm[i + 1] >> 4)];
			*output++ = cov_2char[buf_perm[i] >> 2];
		}
		assert(i == 15);
		*output++ = cov_2char[buf_perm[i] & 0x3f];
		*output++ = cov_2char[buf_perm[i] >> 6];
		*output	  = 0;
		assert(strlen(out_buf) < sizeof(out_buf));
#ifdef CHARSET_EBCDIC
		ascii2ebcdic(out_buf, out_buf, strlen(out_buf));
#endif
	}

	return out_buf;

err:
	OPENSSL_free(ascii_passwd);
	EVP_MD_CTX_free(md2);
	EVP_MD_CTX_free(md);
	return NULL;
}

int make_md5_salt(char **salt_p)
{
	if (salt_p == NULL)
		return -1;

	size_t saltlen = 8;
	size_t i;

	*salt_p = malloc(saltlen + 1);

	if (RAND_bytes((unsigned char *)*salt_p, saltlen) <= 0)
		return -1;

	for (i = 0; i < saltlen; i++)
		(*salt_p)[i] = cov_2char[(*salt_p)[i] & 0x3f]; /* 6 bits */
	(*salt_p)[i] = 0;

	return 0;
}

char *hash_md5_with_salt(char *password, char *salt)
{
	if (password != NULL && salt != NULL)
		return strdup(md5crypt(password, "1", salt));

	return NULL;
}

char *hash_md5(char *password)
{
	char *salt = NULL;
	if (!make_md5_salt(&salt)) {
		char *hashed = hash_md5_with_salt(password, salt);
		free(salt);
		return hashed;
	}

	return NULL;
}

char *extract_salt(char *hash)
{
	size_t i	    = 0;
	int salt_offset	    = 0;
	int separator_count = 0;
	char *salt	    = malloc(strlen(hash) + 1);

	if (!salt)
		return NULL;

	for (; i < strlen(hash); i++) {
		if (separator_count == 1) {
			salt_offset = i + 1;
		} else if (separator_count == 2) {
			salt[i - salt_offset] = hash[i];
		} else if (separator_count >= 3) {
			salt[i - salt_offset - 1] = '\0';
			return salt;
		}
		if (hash[i] == '$')
			separator_count++;
	}

	free(salt);
	return NULL;
}
