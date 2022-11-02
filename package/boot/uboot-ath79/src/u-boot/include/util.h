/* Copyright 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/* Various utility functions and macros */
#ifndef __CROS_EC_UTIL_H
#define __CROS_EC_UTIL_H

static inline uint64_t mula32(uint32_t a, uint32_t b, uint32_t c)
{
	uint64_t ret = a;
	ret *= b;
	ret += c;
	return ret;
}
static inline uint64_t mulaa32(uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
	uint64_t ret = a;
	ret *= b;
	ret += c;
	ret += d;
	return ret;
}

#endif  /* __CROS_EC_UTIL_H */
