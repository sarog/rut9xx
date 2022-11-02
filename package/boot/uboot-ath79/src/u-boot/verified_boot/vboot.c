/* Copyright 2017 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * Verify and jump to a RW image if power supply is not sufficient.
 */
#include "vboot.h"

int vboot_verify(const uint8_t *data, uint32_t len,
		 const struct rsa_public_key *key, const uint8_t *sig)
{
	struct sha256_ctx ctx;
	uint8_t *hash;
	uint32_t workbuf[3 * RSANUMBYTES];
	uint32_t *workbuf_ptr = (uint32_t *)&workbuf;
	int err = 0;

	/* Compute hash of the firmware */
	SHA256_init(&ctx);
	SHA256_update(&ctx, data, len);
	hash = SHA256_final(&ctx);

	/* Verify the data */
	if (rsa_verify(key, sig, hash, workbuf_ptr) != 1){
		err = -1;
	}
	return err;
}
