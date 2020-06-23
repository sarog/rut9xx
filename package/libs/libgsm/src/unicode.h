#pragma once

bool is_unicode(const char *str);
int utf8_to_utf16be(const unsigned char *in, int *in_len, unsigned char *outb, int *out_len);
int utf16be_to_utf8(const unsigned char *inb, int *inb_len, unsigned char *out, int *out_len);
