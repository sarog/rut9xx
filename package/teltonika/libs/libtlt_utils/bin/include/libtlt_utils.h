
#ifndef __LIBTLT_SMTP_H
#define __LIBTLT_SMTP_H

#include <libubox/list.h>

typedef enum {
	LUTIL_SUCCESS,
	LUTIL_ERROR,
} lutil_stat;

struct to_list {
	/* global list of recipients */
	struct list_head list;
	/* email address */
	char *addr;
};

struct lutil_smtp {
	/* username for authentication on SMTP server */
	char *username;
	/* password for authentication on SMTP server */
	char *password;
	/* SMTP Server address */
	char *server;
	/* SMTP server port */
	int port;
	/* secure connection. 1 - on, 0 -off */
	int tls;
	/* email subject */
	char *subject;
	/* senders email */
	char *sender;
	/* email text */
	char *message;
	/* linked list of recipients */
	struct list_head *to;
};

/**
 * @brief add recipient to smtp context
 * 
 * @param ctx smtp context
 * @param recipient recipients address
 * @return lutil_stat 
 * @retval LUTIL_SUCCESS on success
 * @retval LUTIL_ERROR on error
 */
lutil_stat lutil_smtp_add_recipient(struct lutil_smtp *ctx,
				    const char *recipient);

/**
 * @brief clean smtp context
 * 
 * @param ctx smtp context
 */
void lutil_smtp_free(struct lutil_smtp *ctx);

/**
 * @brief send email
 * 
 * @param ctx smtp context
 * @return lutil_stat 
 * @retval LUTIL_SUCCESS on success
 * @retval LUTIL_ERROR on error
 */
lutil_stat lutil_smtp_send(struct lutil_smtp *ctx);

/**
 * @brief Base64 encode
 * 
 * @param src Data to be encoded
 * @param len Length of the data to be encoded
 * @param out_len Pointer to output length variable, or NULL if not used
 * @return Allocated buffer of out_len bytes of encoded data,
 * or NULL on failure
 *
 * Helper for b64_encode() in libubox/utils.h. Caller is responsible for
 * freeing the returned buffer. Returned buffer is nul terminated to make
 * it easier to use as a C string. The nul terminator is not included in out_len.
 */
unsigned char *lutil_base64_encode(const unsigned char *src, size_t len,
				   size_t *out_len);

/**
 * @brief Base64 decode
 * 
 * @param src: Data to be decoded
 * @param len: Length of the data to be decoded
 * @param out_len: Pointer to output length variable
 * @return Allocated buffer of out_len bytes of decoded data,
 * or NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. This implementation
 * is about 9.5 times faster than b64_decode() in libubox/utils.h
 */
unsigned char *lutil_base64_decode(const unsigned char *src, size_t len,
				   size_t *out_len);

#endif //__LIBTLT_SMTP_H
