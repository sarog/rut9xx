/*
 * LuCI Template - Utility header
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _TEMPLATE_UTILS_H_
#define _TEMPLATE_UTILS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* buffer object */
struct template_buffer {
	unsigned char *data;
	unsigned char *dptr;
	unsigned int size;
	unsigned int fill;
};

char * sanitize_utf8(const char *s, unsigned int l);
char * sanitize_pcdata(const char *s, unsigned int l);

#endif
