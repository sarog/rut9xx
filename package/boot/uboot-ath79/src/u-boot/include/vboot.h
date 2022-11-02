/* Copyright 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/* SHA-256 functions */
#ifndef __CROS_EC_VBOOT_H
#define __CROS_EC_VBOOT_H

#include "sha256.h"
#include "rsa.h"

int vboot_verify(const uint8_t *data, uint32_t len,
		 const struct rsa_public_key *key, const uint8_t *sig);

#endif  /* __CROS_EC_SHA256_H */
